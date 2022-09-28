/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:common/getscn.c	1.10"


#ifdef __STDC__
	#pragma weak	elf_getscn = _elf_getscn
#endif


#include "syn.h"
#include "libelf.h"
#include "decl.h"
#include "error.h"


Elf_Scn *
elf_getscn(elf, index)
	register Elf		*elf;
	register size_t		index;
{
	register Elf_Scn	*s;
	register size_t		j = index;

	if (elf == 0)
		return 0;
	if (elf->ed_hdscn == 0 && _elf_cook(elf) != OK_YES)
		return 0;
	for (s = elf->ed_hdscn; s != 0; s = s->s_next)
	{
		if (j == 0)
		{
			if (s->s_index == index)
				return s;
			_elf_err = EBUG_SCNLIST;
			return 0;
		}
		--j;
	}
	_elf_err = EREQ_NDX;
	return 0;
}
