/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bkstatus.d/hdrs/errors.h	1.5.2.1"

/* 
	This file contains error definitions for all error messages in bkstatus 
	command.
*/

/* "Option \"%c\" is invalid.\n" */
#define ERROR0	0

/* "Argument \"%s\" is invalid.\n" */
#define ERROR1	1

/* "No other options may be specified with the \"%c\" option.\n" */
#define ERROR2	2

/* "The \"%c\" and \"%c\" options are mutually exclusive.\n" */
#define ERROR3	3

/* "Warning: \"%s\" is not a valid field separator, using default.\n" */
#define ERROR4	4

/* "Unable to insert ROTATION comment into %s (return code = %d).\n" */
#define ERROR5	5

/* "Open of table %s failed (return code = %d).\n" */
#define ERROR6	6

/* "Unable to read table entry number %d (return code = %d).\n" */
#define ERROR7	7

/* "Unable to allocate memory for login-to-userid conversion.\n" */
#define ERROR8	8

/* "Warning: table %s has different format than expected.\n" */
#define ERROR9	9

/* "Warning: invalid argument %s ignored.\n" */
#define ERROR10	10

/* "Warning: invalid state %c ignored.\n" */
#define ERROR11	11

/* "All option arguments are invalid.\n" */
#define ERROR12	12

/* "Illegal state character \"%c\" encountered.\n" */
#define ERROR13	13

/* "Unable to allocate memory for table entry.\n" */
#define ERROR14	14

/* "Table file %s does not exist or is not accessible.\n" */
#define ERROR15	15

/* "Period value must be at least 1 and not greater than %d.\n" */
#define ERROR16	16

/* "Unable to assign status field value in table entry %d (return code = %d).\n" */
#define ERROR17	17
