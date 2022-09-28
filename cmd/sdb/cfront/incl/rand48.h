/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/incl/rand48.h	1.1"
/*ident	"@(#)cfront:incl/rand48.h	1.4"*/

#ifndef RAND48H
#define RAND48H

extern double drand48 (),
              erand48 (unsigned short);

extern long lrand48 (),
            nrand48 (unsigned short),
            mrand48 (),
            jrand48 (unsigned short);

extern void lcong48 (unsigned short);
extern void srand48 (long);

extern unsigned short *seed48 (unsigned short);

#endif
