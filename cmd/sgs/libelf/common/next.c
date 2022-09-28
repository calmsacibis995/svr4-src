/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:common/next.c	1.5"


#ifdef __STDC__
	#pragma weak	elf_next = _elf_next
#endif


#include "syn.h"
#include "libelf.h"
#include "decl.h"


Elf_Cmd
elf_next(elf)
	Elf	*elf;
{
	Elf	*parent;

	if (elf == 0
	|| (parent = elf->ed_parent) == 0
	|| elf->ed_siboff >= parent->ed_fsz)
		return ELF_C_NULL;
	parent->ed_nextoff = elf->ed_siboff;
	return ELF_C_READ;
}
