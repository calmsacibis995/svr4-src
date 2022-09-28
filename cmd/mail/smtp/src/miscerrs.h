/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:smtp/src/miscerrs.h	1.3.3.1"
/* ident "@(#)miscerrs.h	1.2 'attmail mail(1) command'" */
/* these error numbers are local to the smtp programs, used by bomb() */
#define	E_USAGE		-1
#define E_NOHOST	-2
#define	E_CANTOPEN	-3
#define E_OSFILE	-4
#define E_IOERR		-5
#define E_TEMPFAIL	-6
