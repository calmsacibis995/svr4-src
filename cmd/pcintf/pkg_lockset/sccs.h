/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:pkg_lockset/sccs.h	1.1"
/*------------------------------------------------------------------------
 *
 *	sccs.h - sccs definitions 
 *
 *-------------------------------------------------------------------------
 */

#define SCCSID(idString)		static char Sccsid[] = idString

#ifdef	X_SCCS_H
#define SCCSID_H(idString, file)	static char file[] = idString;
#else
#define SCCSID_H(idString, file)
#endif

/* Keep this comment */ SCCSID_H("@(#)sccs.h	1.2	15:42:49	11/20/89", sccs_h)
