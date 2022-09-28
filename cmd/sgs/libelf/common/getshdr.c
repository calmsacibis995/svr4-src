/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:common/getshdr.c	1.4"


#ifdef __STDC__
	#pragma weak	elf32_getshdr = _elf32_getshdr
#endif


#include "syn.h"
#include "libelf.h"
#include "decl.h"
#include "error.h"


Elf32_Shdr *
elf32_getshdr(scn)
	Elf_Scn	*scn;
{
	if (scn == 0)
		return 0;
	if (scn->s_elf->ed_class != ELFCLASS32)
	{
		_elf_err = EREQ_CLASS;
		return 0;
	}
	return scn->s_shdr;
}
