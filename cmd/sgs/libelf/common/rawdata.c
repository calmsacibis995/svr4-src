/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:common/rawdata.c	1.3"


#ifdef __STDC__
	#pragma weak	elf_rawdata = _elf_rawdata
#endif


#include "syn.h"
#include "libelf.h"
#include "decl.h"
#include "error.h"


Elf_Data *
elf_rawdata(scn, data)
	register Elf_Scn	*scn;
	Elf_Data		*data;
{
	register Dnode		*d = (Dnode *)data;
	register Dnode		*raw;
	register Elf		*elf;

	if (scn == 0)
		return 0;
	if (d == 0)
		d = scn->s_hdnode;
	else
		d = d->db_next;
	if (d == 0)
		return 0;
	if (d->db_scn != scn)
	{
		_elf_err = EREQ_DATA;
		return 0;
	}

	/*	The data may come from a previously constructed Dbuf,
	 *	from the file's raw memory image, or the file system.
	 *	"Empty" regions get an empty buffer.
	 */

	if (d->db_raw != 0)
		return &d->db_raw->db_data;

	if ((raw = _elf_dnode()) == 0)
		return 0;
	raw->db_myflags |= DBF_READY;
	if (d->db_off == 0 || d->db_fsz == 0)
	{
		d->db_raw = raw;
		raw->db_data.d_size = d->db_shsz;
		return &raw->db_data;
	}

	/*	validate the region
	 */

	elf = scn->s_elf;
	if (d->db_off < 0
	|| d->db_off >= elf->ed_fsz
	|| elf->ed_fsz - d->db_off < d->db_fsz)
	{
		_elf_err = EFMT_DATA;
		free(raw);
		return 0;
	}
	raw->db_data.d_size = d->db_fsz;
	if (elf->ed_raw != 0)
	{
		raw->db_data.d_buf = (Elf_Void *)(elf->ed_raw + d->db_off);
		d->db_raw = raw;
		return &raw->db_data;
	}
	raw->db_buf = (Elf_Void *)_elf_read(elf->ed_fd,
			elf->ed_baseoff + d->db_off, d->db_fsz);
	if (raw->db_buf == 0)
	{
		free(raw);
		return 0;
	}
	raw->db_data.d_buf = raw->db_buf;
	d->db_raw = raw;
	return &raw->db_data;
}
