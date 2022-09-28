/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:common/newscn.c	1.4"


#ifdef __STDC__
	#pragma weak	elf_newscn = _elf_newscn
#endif


#include "syn.h"
#include "libelf.h"
#include "decl.h"
#include "error.h"


Elf_Scn *
elf_newscn(elf)
	register Elf		*elf;
{
	register Snode		*s;
	register Elf_Scn	*tl;

	if (elf == 0)
		return 0;

	/*	if no sections yet, the file either isn't cooked
	 *	or it truly is empty.  Then allocate shdr[0]
	 */

	if (elf->ed_hdscn == 0 && _elf_cook(elf) != OK_YES)
		return 0;
	if (elf->ed_ehdr == 0)
	{
		_elf_err = ESEQ_EHDR;
		return 0;
	}
	if (elf->ed_hdscn == 0)
	{
		if ((s = _elf_snode()) == 0)
			return 0;
		s->sb_scn.s_elf = elf;
		elf->ed_hdscn = elf->ed_tlscn = &s->sb_scn;
		s->sb_scn.s_uflags |= ELF_F_DIRTY;
	}
	if ((s = _elf_snode()) == 0)
		return 0;
	tl = elf->ed_tlscn;
	s->sb_scn.s_elf = elf;
	s->sb_scn.s_index = tl->s_index + 1;
	elf->ed_tlscn = tl->s_next = &s->sb_scn;
	elf->ed_ehdr->e_shnum = tl->s_index + 2;
	s->sb_scn.s_uflags |= ELF_F_DIRTY;
	return &s->sb_scn;
}
