/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:interface/print_err.c	1.7.1.2"

#include <stdio.h>
#include <string.h>
#include <varargs.h>
#include "print_err.h"

static char	*msg[] = {
/* S_USAGE */	"invalid syntax.\nUsage:  sysadm [menu name | task name]",
/* P_USAGE */	"invalid syntax.\nUsage:  powerdown [-y | -Y]",
/* M_USAGE */	"invalid syntax.\nUsage:  mountfsys [-y] [-r]",
/* U_USAGE */	"invalid syntax.\nUsage:  umountfsys [-y]",
/* OAM_LOC */	"unable to invoke interface.  Interface structure not found.",
/* ENV_PROB */	"unable to invoke interface.  Unable to obtain memory to expand environment.",
/* NO_EXPR */	"unable to invoke express mode.  \nExpress lookup file not available.",
/* NOFMLI */	"unable to invoke interface.  Fmli is not available.",
/* O_USAGE */	"invalid syntax.\nUsage:  '%s'",
/* NOTUNIQ */	"'%s' is not a unique menu or task name.\nType sysadm and proceed through menus to find it.",
/* NOT_FOUND */	"menu or task '%s' does not exist.",
/* NO_COMND */	"cannot execute command: '%s'",
/* INV_ENT */	"invalid express file entry for '%s'",
/* P_HOLDER */	"'%s' is empty and not available for use.",
/* INVNAME */	"'%s' is not a valid command name.",
/* NOT_OPEN */	"cannot open file: '%s'",
/* RENAMED */	"'%s' has been renamed.\nUse '%s' in the future.",
/* INVPATH */	"'%s' will not be valid in the future.\nUse '%s' in the future.",
/*EXPR_SYNTAX*/	"invalid express seed file syntax for name: '%s' link: '%s'"
};

#define NMESGS	(sizeof(msg)/sizeof(char *))

/*VARARGS*/
void
print_err(va_alist)
va_dcl
{
	va_list ap;
	char *prog;		/* calling program name */
	int err_no;		/* error to print */
	char	*pt;

	va_start(ap);
	err_no = va_arg(ap, int);
	prog = va_arg(ap, char *);

	if((pt = strrchr(prog, '/')) != NULL)
		prog = pt+1;

	(void) fprintf(stderr, "UX:%s:", prog);

	if(err_no == RENAMED)
		(void) fprintf(stderr, "INFO:");
	else if(err_no == INVPATH)
		(void) fprintf(stderr, "WARNING:");
	else
		(void) fprintf(stderr, "ERROR:");

	if((err_no < 0) || (err_no >= NMESGS))
		(void) fprintf(stderr, "unknown error.");
	else
		(void) vfprintf(stderr, msg[err_no], ap);
	va_end(ap);
	(void) putc('\n', stderr);
}
