/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bkreg.d/hdrs/errors.h	1.7.3.1"

/* 
	This file contains error definitions for all error messages in bkreg 
	command.
*/

/* "Option \"%c\" is invalid.\n" */
#define	ERROR0	0

/* "Argument \"%s\" is invalid.\n" */
#define	ERROR1	1

/* "All option arguments are invalid.\n" */
#define ERROR2	2

/* "\"%s\" is an invalid argument for the %c option.\n" */
#define ERROR3	3

/* "Could not save table %s (return code %d).\n" */
#define ERROR4	4

/* "Cannot allocate memory.\n" */
#define ERROR5	5

/* "Could not read ROTATION period in %s (return code %d).\n" */
#define ERROR6	6

/* "Could not read ROTATION STARTED in %s (return code %d).\n" */
#define ERROR7	7

/* "Could not read table entry number %d (return code %d).\n" */
#define ERROR8	8

/* "Unable to add this backup.\n" */
#define	ERROR9	9

/* "Cannot open table %s (return code %d).\n" */
#define	ERROR10	10

/* "Tag %s already exists in table %s.\n" */
#define ERROR11 11

/* "Tag %s does not exist in table %s.\n" */
#define ERROR12 12

/* "Unable to assign value \"%s\" to entry number %d (return code %d).\n" */
#define ERROR13 13

/* "Unable to remove entry %d (return code %d).\n" */
#define ERROR14 14

/* "Unable to insert %s comment into table %s (return code %d).\n" */
#define ERROR15 15

/* "Unable to sort table %s on originating device field, errno = %d.\n" */
#define ERROR16 16

/* "Warning: unable to remove temporary file %s, errno = %d.\n" */
#define ERROR17 17

/* "The %c argument must be greater than 0 and less than or equal to %d.\n" */
#define ERROR18	18

/* "Search of table %s failed (return code %d).\n" */
#define ERROR19	19

/* "Cannot open table %s (errno = %d).\n" */
#define ERROR20	20

/* "Could not save table %s (errno = %d).\n" */
#define ERROR21	21

/* "Warning: All the week(s) of backup should be less than ROTATION period = %d.\n" */
#define ERROR22 22

