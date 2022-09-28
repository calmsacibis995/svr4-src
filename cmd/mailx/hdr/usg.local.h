/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mailx:hdr/usg.local.h	1.4.5.1"
/*
 * Declarations and constants specific to an installation.
 */
 
/*
 * mailx -- a modified version of a University of California at Berkeley
 *	mail program
 *
 */

#define	LOCAL		EMPTYID		/* Dynamically determined local host */
#ifdef preSVr4
# define	MAIL	"/bin/rmail"	/* Mail delivery agent */
#else
# define	MAIL	"/usr/bin/rmail"/* Mail delivery agent */
#endif
#define	EDITOR		"ed"		/* Name of text editor */
#define	VISUAL		"vi"		/* Name of display editor */
#define	PG		(value("PAGER") ? value("PAGER") : "pg -e")
					/* Standard output pager */
#define	LS		(value("LISTER") ? value("LISTER") : "ls")
					/* Name of directory listing prog*/
#ifdef preSVr4
# define	SHELL	"/bin/sh"	/* Standard shell */
#else
# define	SHELL	"/usr/bin/sh"	/* Standard shell */
#endif
#define	HELPFILE	libpath("mailx.help")
					/* Name of casual help file */
#define	THELPFILE	libpath("mailx.help.~")
					/* Name of casual tilde help */
#ifdef preSVr4
# define	MASTER	libpath("mailx.rc")
#else
# define	MASTER	"/etc/mail/mailx.rc"
#endif
#define	APPEND				/* New mail goes to end of mailbox */
#define CANLOCK				/* Locking protocol actually works */
#define	UTIME				/* System implements utime(2) */
