/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:common/nextscn.c	1.2"


#ifdef __STDC__
	#pragma weak	elf_nextscn = _elf_nextscn
#endif


#include "syn.h"
#include "libelf.h"
#include "decl.h"


Elf_Scn *
elf_nextscn(elf, scn)
	register Elf		*elf;
	register Elf_Scn	*scn;
{

	if (elf == 0)
		return 0;
	if (scn != 0)
		return scn->s_next;
	if (elf->ed_hdscn == 0)
		(void)_elf_cook(elf);
	if ((scn = elf->ed_hdscn) != 0)
		return scn->s_next;
	return 0;
}
