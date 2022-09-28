/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:intf_remove/rm_err.c	1.1.1.2"

#include <stdio.h>
#include "rm_err.h"

union in_arg {
	int int_arg;
	char *str_arg;
};

void
rm_err(type, cmd, err_no, err_arg1, err_arg2)
int type;		/* WARNING or ERROR */
char *cmd;		/* name of the command */
int err_no;		/* error to print */
union in_arg err_arg1;
union in_arg err_arg2;

{
static char *msg[] = {
/* USAGE */	"invalid syntax.\nusage:  intf_remove menu_file | expr_file\n",
/* INV_FILE */	"Invalid menu file entry:  '%s'\n",
/* FILE_OPN */	"Package interface removal error.  Unable to open %s.\n",
/* FILE_RD */	"Package interface removal error.  Unable to read %s.\n",
/* FILE_WR */	"Package interface removal error.  Unable to write %s.\n",
/* D_CREAT */	"Package interface removal error.  Unable to create %s.\n",
/* FILE_CLS */	"Package interface removal error.  Unable to close %s.\n",
/* RM_ERR */	"Package interface removal error.  Cannot remove %s.\n"
};

	if(type == WARN)
		fprintf(stderr, "UX:%s:WARNING:", cmd);
	else fprintf(stderr, "UX:%s:ERROR:", cmd);

	if(err_no < HAS_STR)	/* no arguments */
		fprintf(stderr, msg[err_no]);
	else if(err_no < HAS_TWO)	/* single string argument */
		fprintf(stderr, msg[err_no], err_arg1.str_arg);
	else fprintf(stderr, msg[err_no], err_arg1.str_arg, err_arg2.str_arg);

}
