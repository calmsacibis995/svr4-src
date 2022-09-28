/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)fmli:sys/terrmess.c	1.2"

#include	<stdio.h>
#include	"wish.h"
#include	"terror.h"

/*
 * NOTE: these error messages depend upon the order of error numbers in
 * errno.  When that changes, so must this array and the list of defines
 * in terror.h
 */
char	*Errlist[] = {
	nil,
	"Permissions are wrong",
	"File does not exist",
	nil,
	nil,
	"Hardware error",
	nil,
	"Arguments are too long",
	"File has been corrupted",
	"Software error",
	nil,
	"Can't create another process",
	"Out of memory",
	"Permissions are wrong",
	nil,
	nil,
	nil,
	"File already exists",
	nil,
	nil,
	"Improper name",
	"It is a directory",
	nil,
	"Too many files in use on system",
	"Too many files in use by program",
	nil,
	nil,
	nil,
	"System out of disk space",
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
};

/*
 * NOTE: this array depends on the numbering scheme in terror.h
 * If you add an element to this array, add it at the end and change
 * terror.h to define the new value. Also, don't forget to change
 * TS_NERRS and add a line to Use_errno.
 */
char	*What[TS_NERRS] = {
	nil,
	"Can't open file",
	"Invalid arguments",
	"Data has been corrupted",
	"Some necessary information is missing",
	"Software failure error",
	"Can't execute the program",
	"Can't create or remove file",
	"Input is not valid",
	"Frame not updated: definition file missing or not readable",
	"Can't open frame: definition file missing or not readable"
};

/*
 * This array indicates whether or not errno may be considered
 * valid when this type of error occurs
 */
bool	Use_errno[TS_NERRS] = {
	FALSE,
	TRUE,
	FALSE,
	FALSE,
	FALSE,
	TRUE,
	TRUE,
	TRUE,
	FALSE,
	FALSE,
	FALSE
};
