/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)listen:lsfiles.h	1.3.3.1"

/*
 *	lsfiles.h:	listener files stuff
 */


/*
 * initialization constants for file checking/usage
 */

#define LOGOFLAG	(O_WRONLY | O_APPEND | O_CREAT)
#define LOGMODE		(0666)

#define PIDOFLAG	(O_WRONLY | O_CREAT)
#define PIDMODE		(0644)

#define NETOFLAG	(O_RDWR)

