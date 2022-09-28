/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mailx:hdr/rcv.h	1.2.5.1"

/*
 * mailx -- a modified version of a University of California at Berkeley
 *	mail program
 *
 * This file is included by normal files which want both
 * globals and declarations.
 */

/*
 */

#define	USG	1			/* System V */
#define	USG_TTY	1			/* termio(7) */

#include "def.h"
#include "glob.h"
