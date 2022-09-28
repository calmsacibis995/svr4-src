/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:backup.d/hdrs/errors.h	1.4.2.1"

/* 
	This file contains error definitions for all error messages in backup 
	command.
*/

/* "%s: option \"%c\" is invalid.\n" */
#define	ERROR0	0

/* "%s: argument \"%s\" is invalid.\n" */
#define	ERROR1	1

/* "\"%s\% is not a valid user name.\n" */
#define	ERROR2	2

/* "All option arguments are invalid.\n" */
#define ERROR3	3

/* "\"%s\" is an invalid argument for the %c option.\n" */
#define ERROR4	4

/* "File name \"%s\" is too long.\n" */
#define	ERROR5	5

/* A week or day range may not be specified with the \"%c\" option.\n" */
#define	ERROR6	6

/* Cannot use both the \"%c\" and \"%c\" options with the \"%c\" option.\n" */
#define	ERROR7	7

/* "Backup daemon process has unexpectedly terminated - trying to restart... */
#define	ERROR8	8

/* "Backup daemon process has unexpectedly terminated - cannot %s ... */
#define ERROR9	9

/* "Cannot keep backup daemon process alive - exiting.\n" */
#define ERROR10	10

/* "The backup jod id is back-%d\n" */
#define	ERROR11	11

/* "Unable to get current working directory\n" */
#define ERROR12	12

/* "Path name for file %s/%s is too long\n" */
#define	ERROR13	13

/* "Cannot access %s\n" */
#define	ERROR14	14

/* "Cannot RESUME/CANCEL/SUSPEND someone else's backups.\n" */
#define	ERROR15	15

/* "Unable to invoke bkoper: %s" */
#define	ERROR16	16

/* "Unable to open backup register %s - %s.\n" */
#define ERROR17 17

/* "Table has no rotation period.\n" */
#define ERROR18 18

/* "Rotation not found , set to 1.\n" */
#define ERROR19 19

/* "Rotation %d greater than %d, set to %d.\n" */
#define ERROR20 20

/* "No ROTATION STARTED in register table.\n" */
#define ERROR21 21

/* "Tag %s has invalid week/day specified.\n" */
#define ERROR22 22

/* "Tag %s: specified priority %s illegal, set to %d.\n" */
#define ERROR23 23

/* "Tag %s: tag has appeared previously, first used.\n" */
#define ERROR24 24

/* "Tag %s: week %d day %d illegal combination.\n" */
#define ERROR25 25

/* "Tag %s: oname %s odevice %s dup backups as follows:\n\t%s\n." */
#define ERROR26 26

/* "TLread of bkreg returned %d - %s.\n" */
#define ERROR27 27

/* "Tag: %s depends on nonexistent tag %s.\n"  */
#define ERROR28 28

/* "Tag: %s cannot execute due to dependency conflicts\n\t" */
#define ERROR29 29

/* "%s .\n" */
#define ERROR30 30

/* "No valid entries in table. \n" */
#define ERROR31 31

/* "There are no backup operations to %s" */
#define ERROR32 32
