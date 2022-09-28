/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:rsnotify.d/hdrs/errors.h	1.2.2.1"

/* 
	This file contains error definitions for all error messages in bkstatus 
	command.
*/

/* "Option \"%c\" is invalid.\n" */
#define ERROR0	0

/* "Argument \"%s\" is invalid.\n" */
#define ERROR1	1

/* "Unable to allocate memory for reading table entry.\n" */
#define ERROR2	2

/* "Unable to read table entry number %d (return code = %d).\n" */
#define ERROR3	3

/* "Unable to assign %s value to table entry number %d (return code = %d).\n" */
#define ERROR4	4

/* "Warning: table %s has different format than expected.\n" */
#define ERROR5	5

/* "Open of table %s failed (return code = %d).\n" */
#define ERROR6	6
