/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:rsstatus.d/hdrs/errors.h	1.4.2.1"

/* 
	This file contains error definitions for all error messages in rsstatus 
	command.
*/

/* "Option \"%c\" is invalid.\n" */
#define	ERROR0	0

/* "Warning: invalid argument \"%s\" ignored.\n" */
#define ERROR1	1

/* "Cannot open %s (errno = %d).\n" */
#define ERROR2	2

/* "Unable to read table entry number %d (return code = %d).\n" */
#define	ERROR3	3

/* "Open of table %s failed, return code = %d.\n" */
#define ERROR4	4

/* "Warning: table %s has different format than expected.\n" */
#define ERROR5	5

/* "Unable to allocate memory for reading table entry.\n" */
#define ERROR6	6

/* "Unable to allocate memory for login-to-userid conversion.\n" */
#define ERROR7	7

/* "Illegal field separator specification \"%s\".\n" */
#define ERROR8	8

/* "Argument \"%s\" is invalid.\n" */
#define ERROR9	9

/* "Unable to allocate memory for TOC volume label processing.\n" */
#define ERROR10	10
