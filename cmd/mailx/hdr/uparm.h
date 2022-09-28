/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mailx:hdr/uparm.h	1.3.5.1"
/*
 * mailx -- a modified version of a University of California at Berkeley
 *	mail program
 *
 */

extern char *libpath();

#ifdef preSVr4
# define LIBPATH          "/usr/lib/mailx"
#else
# define LIBPATH          "/usr/share/lib/mailx"
#endif
