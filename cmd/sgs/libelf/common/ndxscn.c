/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:common/ndxscn.c	1.2"


#ifdef __STDC__
	#pragma weak	elf_ndxscn = _elf_ndxscn
#endif


#include "syn.h"
#include "libelf.h"
#include "decl.h"


size_t
elf_ndxscn(scn)
	register Elf_Scn	*scn;
{

	if (scn == 0)
		return SHN_UNDEF;
	return scn->s_index;
}
