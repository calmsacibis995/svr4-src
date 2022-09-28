/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:i386/expand2.c	1.2"

#include "expand2.h"


/*	Information for span-dependent instruction types
 */


ITINFO	itinfo[NITYPE] =
{
	/* binc			no hinc */
	{UJMP_WSZ - UJMP_BSZ,	0,},	/* UJMP */
	{CJMP_WSZ - CJMP_BSZ,	0,},	/* CJMP */
	{LOOP_WSZ - LOOP_BSZ,	0,},	/* LOOP */
};
