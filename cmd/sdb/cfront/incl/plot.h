/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/incl/plot.h	1.1"
/*ident	"@(#)cfront:incl/plot.h	1.5"*/

#ifndef PLOTH
#define PLOTH

extern int arc (int, int, int, int, int, int),
           circle (int, int, int),
           closepl (),
           cont (int, int),
           erase (),
           label (const char*),
           line (int, int, int, int),
           linemod (const char*),
           move (int, int),
           openpl (),
           point (int, int),
           space (int, int, int, int);

#endif
