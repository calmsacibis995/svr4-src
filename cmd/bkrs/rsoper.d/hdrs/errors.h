/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:rsoper.d/hdrs/errors.h	1.2.2.1"

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

/* "The \"%c\" option may not be used with the \"%c\" option.\n" */
#define	ERROR5	5

/* "Unable to open a temporary file (%s). Mail was not sent to %s.\n" */
#define	ERROR6	6

/* "Unable to read pending request entry for restore id %s.\n" */
#define	ERROR7	7

/* "Restore id %s does not exist.\n" */
#define	ERROR8	8

/* "Pending restore request table has bad format.\n" */
#define	ERROR9	9

/* "Must have the same effective uid to cancel restore id %s.\n" */
#define	ERROR10 10

/* "Unable to %s restore id %s.\n" */
#define	ERROR11	11

/* "Unable to open pending request table.\n" */
#define	ERROR12	12

/* "Must have one of the following options: \"%s\".\n" */
#define	ERROR13	13

/* "Unable to read pending restore request table.\n" */
#define	ERROR14	14

/* "Unable to satisfy any restore requests; the following information is needed: %s"*/
#define	ERROR15 15

/* "The \"%c\" option is not yet implemented.\n" */
#define	ERROR16	16

/* "Restore request %s for %s was not completed.\n" */
#define	ERROR17 17

/* "Unable to read pending restore request table: %s.\n" */
#define	ERROR18	18

/* "Unable to spawn %s method\n" */
#define	ERROR19	19

/* "Unable to read label from %s.\n" */
#define	ERROR20 20

/* "Unable to read label from %s.\n" */
#define	ERROR21	21

/* "Unable to spawn %s method: %s.\n" */
#define	ERROR22	22

/* "\"%s\" is not a valid jobid.\n" */
#define ERROR23 23

/* "Unable to reserve devices %s and %s to service restore reqests.\n" */
#define	ERROR24 24

/* "Devices %s and/or %s are currently busy.  Please try later.\n" */
#define	ERROR25 25
