/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:coff/coffscn.c	1.4"


#include "syn.h"
#include "filehdr.h"
#include "scnhdr.h"
#include "reloc.h"
#include "libelf.h"
#include "decl.h"
#include "coff.h"
#include "error.h"


/* COFF section conversion
 *	This file converts COFF (Common Object File Format) sections
 *	to their elf equivalents.
 *
 *	If the coff section has relocation, create the elf section.
 */


Okay
_elf_coffscn(elf, info)
	Elf		*elf;
	Info		*info;
{
	Elf_Data	crel;
	SCNHDR		*ch, *ech;
	Elf_Scn		*s, *rscn;
	size_t		sz;

	if ((sz = info->i_filehdr->f_nscns) == 0)
		return OK_YES;
	ch = info->i_coffscn + 1;
	ech = ch + sz;
	s = 0;
	for (; ch < ech; ++ch)
	{
		if ((s = elf_nextscn(elf, s)) == 0)
			return OK_NO;
		if (ch->s_relptr <= 0 || ch->s_nreloc <= 0)
			continue;
		sz = ch->s_nreloc;
		if (_elf_vm(elf, (size_t)ch->s_relptr, sz * RELSZ) != OK_YES
		|| (rscn = elf_newscn(elf)) == 0)
			return OK_NO;
		if (info->i_symtab == 0)
		{
			rscn->s_err = ECOFF_RELSYM;
			continue;
		}
		crel.d_buf = (Elf_Void *)(elf->ed_ident + ch->s_relptr);
		crel.d_size = sz * RELSZ;
		if ((*info->i_coff->c_rel)(elf, info, rscn, s, &crel) != (int)OK_YES)
			return OK_NO;
	}
	return OK_YES;
}
