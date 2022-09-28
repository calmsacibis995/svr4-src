/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:common/getdata.c	1.16"


#ifdef __STDC__
	#pragma weak	elf_getdata = _elf_getdata
#endif


#include "syn.h"
#include "libelf.h"
#include "decl.h"
#include "error.h"


/* 	Convert data from file format to memory format.
 */


static const size_t	align[] =
{
	1,			/* ELF_T_BYTE */
	sizeof(Elf32),		/* ELF_T_ADDR */
	sizeof(Elf32),		/* ELF_T_DYN */
	sizeof(Elf32),		/* ELF_T_EHDR */
	sizeof(Elf32_Half),	/* ELF_T_HALF */
	sizeof(Elf32),		/* ELF_T_OFF */
	sizeof(Elf32),		/* ELF_T_PHDR */
	sizeof(Elf32),		/* ELF_T_RELA */
	sizeof(Elf32),		/* ELF_T_REL */
	sizeof(Elf32),		/* ELF_T_SHDR */
	sizeof(Elf32),		/* ELF_T_SWORD */
	sizeof(Elf32),		/* ELF_T_SYM */
	sizeof(Elf32),		/* ELF_T_WORD */
};

#define Nalign	(sizeof(align)/sizeof(align[0]))


Elf_Data *
elf_getdata(scn, data)
	register Elf_Scn	*scn;
	Elf_Data		*data;
{
	register Dnode		*d = (Dnode *)data;
	register Elf		*elf;
	Elf_Data		src;

	/*	trap null args, end of list, previous buffer.
	 *	SHT_NULL sections have no buffer list, so they
	 *	fall out here too.
	 */

	if (scn == 0)
		return 0;
	if (d == 0)
		d = scn->s_hdnode;
	else
		d = d->db_next;
	if (scn->s_err != 0)
	{
		_elf_err = scn->s_err;
		return 0;
	}
	if (d == 0)
		return 0;
	if (d->db_scn != scn)
	{
		_elf_err = EREQ_DATA;
		return 0;
	}
	if (d->db_myflags & DBF_READY)
		return &d->db_data;
	elf = scn->s_elf;

	/*	Prepare return buffer.  The data come from the memory
	 *	image of the file.  "Empty" regions get an empty buffer.
	 *
	 *	Only sections of an ELF_C_READ file can be not READY here.
	 *	Furthermore, the input file must have been cooked or
	 *	frozen by now.  Translate cooked files in place if possible.
	 */

	d->db_data.d_version = _elf_work;
	if (d->db_off == 0 || d->db_fsz == 0)
	{
		d->db_myflags |= DBF_READY;
		return &d->db_data;
	}

	{
		Elf32_Shdr	*sh = scn->s_shdr;
		size_t		sz = sh->sh_entsize;
		Elf_Type	t = d->db_data.d_type;

		if (t != ELF_T_BYTE
		&& sz > 1
		&& sz != elf32_fsize(t, 1, elf->ed_version))
		{
			_elf_err = EFMT_ENTSZ;
			return 0;
		}
	}

	/*	validate the region
	 */

	if (d->db_off < 0
	|| d->db_off >= elf->ed_fsz
	|| elf->ed_fsz - d->db_off < d->db_fsz)
	{
		_elf_err = EFMT_DATA;
		return 0;
	}

	/*	set up translation buffers and validate
	 */

	src.d_buf = (Elf_Void *)(elf->ed_ident + d->db_off);
	src.d_size = d->db_fsz;
	src.d_type = d->db_data.d_type;
	src.d_version = elf->ed_version;
	if (_elf_vm(elf, (size_t)d->db_off, d->db_fsz) != OK_YES)
		return 0;

	/*	decide where to put destination
	 */

	switch (elf->ed_status)
	{
	case ES_COOKED:
		if ((size_t)d->db_data.d_type >= Nalign)
		{
			_elf_err = EBUG_COOKTYPE;
			return 0;
		}

		/*	If the destination size (memory) is at least as
		 *	big as the source size (file), reuse the space.
		 */

		if (d->db_data.d_size <= src.d_size
		&& d->db_off % align[d->db_data.d_type] == 0)
		{
			d->db_data.d_buf = (Elf_Void *)(elf->ed_ident + d->db_off);
			break;
		}
		/*FALLTHRU*/
	case ES_FROZEN:
		if ((d->db_buf = malloc(d->db_data.d_size)) == 0)
		{
			_elf_err = EMEM_DATA;
			return 0;
		}
		d->db_data.d_buf = d->db_buf;
		break;

	default:
		_elf_err = EBUG_COOKSTAT;
		return 0;
	}
	if (elf32_xlatetom(&d->db_data, &src, elf->ed_encode) == 0)
		return 0;
	d->db_myflags |= DBF_READY;
	return &d->db_data;
}
