/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-i386:fp/fp_data.c	1.3"
/*
 * contains the definitions
 * of the global symbols used
 * by the floating point environment
 */
#include "synonyms.h"
#ifdef __STDC__
#include <math.h>
const _h_val __huge_val = 
#else
unsigned long _huge_val[sizeof(double)/sizeof(unsigned long)] =
#endif /* __STDC__ */
	{ 0x0,0x7ff00000 };
