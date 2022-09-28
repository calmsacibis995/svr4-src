/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:common/getphdr.c	1.10"


#ifdef __STDC__
	#pragma weak	elf32_getphdr = _elf32_getphdr
#endif


#include "syn.h"
#include "libelf.h"
#include "decl.h"
#include "error.h"


Elf32_Phdr *
elf32_getphdr(elf)
	Elf	*elf;
{
	if (elf == 0)
		return 0;
	if (elf->ed_class != ELFCLASS32)
	{
		_elf_err = EREQ_CLASS;
		return 0;
	}
	if (elf->ed_phdr == 0)
		(void)_elf_cook(elf);
	return (Elf32_Phdr *)elf->ed_phdr;
}
