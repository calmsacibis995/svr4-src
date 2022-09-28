/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */

#ident	"@(#)fmli:oh/partab.c	1.5"

/* Note: this file created with tabstops set to 4.
 *
 * Definition of the Object Parts Table (OPT).
 *
 */

#include <stdio.h>
#include <sys/types.h>		/* EFT abs k16 */
#include "wish.h"
#include "but.h"
#include "typetab.h"
#include "ifuncdefs.h"
#include "optabdefs.h"
#include "partabdefs.h"


/*** NOTE: the ordering of the objects in this table must be the same
 *** as the order in the object operations table (In optab.c), as this table is
 *** used as an index into that table.
 ***/

struct opt_entry Partab[MAX_TYPES] =
{
	{ "DIRECTORY",	"File folder",	CL_DIR,  "?", "?", "?", "?", "?", 0, 2},
	{ "ASCII",	"Standard file",CL_DOC,	 "?", "?", "?", "?", "?", 2, 1},
	{ "MENU",	"Menu",	CL_DYN | CL_FMLI,"?", "?", "?", "?", "?", 3, 1},
	{ "FORM",	"Form",		CL_FMLI, "?", "?", "?", "?", "?", 4, 1},
	{ "TEXT",	"Text",		CL_FMLI, "?", "?", "?", "?", "?", 5, 1},
	{ "EXECUTABLE",	"Executable",	CL_FMLI, "?", "?", "?", "?", "?", 7, 1},
	{ "TRANSFER",	"Foreign file",	CL_OEU,  "?", "?", "?", "?", "?", 6, 1},
	{ "UNKNOWN",	"Data file",	NOCLASS, "?", "?", "?", "?", "?", 7, 1},
	{ "", 		"", 	   NOCLASS, NULL, NULL, NULL, NULL, NULL, 0, 0}
};

/* the "magic" numbers in the "%.ns" below (2nd field) are based on 
 * a max file name size of 255.
 */
struct one_part Parts[MAXPARTS] = 
{
        {"1",	"%.255s", 	PRT_DIR},	/* 0  DIRECTORY */
	{"2",	"%.249s/.pref",	PRT_FILE|PRT_OPT}, /* 1            */
	{"1",	"%.255s", 	PRT_FILE},	/* 2  ASCII     */
	{"1",   "Menu.%.250s", 	PRT_FILE},	/* 3  MENU      */
	{"1",   "Form.%.250s", 	PRT_FILE},	/* 4  FORM      */
	{"1",   "Text.%.250s", 	PRT_FILE},	/* 5  TEXT      */
	{"1",	"%.255s",	PRT_FILE|PRT_BIN}, /* 6  TRANSFER  */
	{"1",	"%.255s", 	PRT_FILE|PRT_BIN}, /* 7  UNKNOWN/EXEC*/
	{"",	"",		0}
};
