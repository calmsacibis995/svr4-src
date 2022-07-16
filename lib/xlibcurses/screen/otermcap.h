/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/otermcap.h	1.2"
#define TBUFSIZE	2048		/* double the norm */

/* externs from libtermcap.a */
extern int otgetflag (), otgetnum (), otgetent ();
extern char *otgetstr ();
extern char *tskip ();			/* non-standard addition */
extern int TLHtcfound;			/* non-standard addition */
extern char TLHtcname[];		/* non-standard addition */
