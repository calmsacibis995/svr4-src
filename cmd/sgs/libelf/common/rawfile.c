/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:common/rawfile.c	1.4"


#ifdef __STDC__
	#pragma weak	elf_rawfile = _elf_rawfile
#endif


#include "syn.h"
#include "libelf.h"
#include "decl.h"


char *
elf_rawfile(elf, ptr)
	register Elf	*elf;
	register size_t	*ptr;
{
	register size_t	sz;
	char		*p = 0;

	if (elf == 0 || (sz = elf->ed_fsz) == 0)
	{
		if (ptr != 0)
			*ptr = 0;
		return 0;
	}
	if (elf->ed_raw != 0)
		p = elf->ed_raw;
	else if (elf->ed_status == ES_COOKED)
	{
		if ((p = _elf_read(elf->ed_fd, elf->ed_baseoff, sz)) != 0)
		{
			elf->ed_raw = p;
			elf->ed_myflags |= EDF_RAWALLOC;
		}
		else
			sz = 0;
	}
	else
	{
		p = elf->ed_raw = elf->ed_ident;
		elf->ed_status = ES_FROZEN;
		if (_elf_vm(elf, (size_t)0, elf->ed_fsz) != OK_YES)
		{
			p = 0;
			sz = 0;
		}
	}
	if (ptr != 0)
		*ptr = sz;
	return p;
}
