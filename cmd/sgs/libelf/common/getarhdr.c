/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:common/getarhdr.c	1.7"


#ifdef __STDC__
	#pragma weak	elf_getarhdr = _elf_getarhdr
#endif


#include <ar.h>
#include "syn.h"
#include "libelf.h"
#include "decl.h"
#include "member.h"
#include "error.h"


Elf_Arhdr *
elf_getarhdr(elf)
	Elf		*elf;
{
	register Member	*mh;
	if (elf == 0)
		return 0;
	if ((mh = elf->ed_armem) == 0)
	{
		_elf_err = EREQ_AR;
		return 0;
	}
	if (mh->m_err != 0)
		_elf_err = mh->m_err;
	return &elf->ed_armem->m_hdr;
}
