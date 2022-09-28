/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:common/hash.c	1.4"


#ifdef __STDC__
	#pragma weak	elf_hash = _elf_hash
#endif


#include "syn.h"
#include "libelf.h"


#define MASK	(~(unsigned long)0<<28)


unsigned long
elf_hash(name)
	const char			*name;
{
	register unsigned long		g, h = 0;
	register const unsigned char	*nm = (unsigned char *)name;

	while (*nm != '\0') {
		h = (h << 4) + *nm++;
		if ((g = h & MASK) != 0)
			h ^= g >> 24;
		h &= ~MASK;
	}
	return h;
}
