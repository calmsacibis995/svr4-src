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
#ident	"@(#)fmli:wish/wdwcreate.c	1.6"

#include <stdio.h>
#include "wish.h"
#include "token.h"
#include "slk.h"
#include "actrec.h"
#include "terror.h"
#include "ctl.h"
#include "menudefs.h"
#include "vtdefs.h"
#include "sizes.h"

extern char *Args[];
extern int Arg_count;

int
glob_create()
{
    char path[PATHSIZ], *errstr;
    static char *argv[3];
    extern char *Filecabinet;
    char *path_to_full(), *bsd_path_to_title(), *cur_path();

    argv[0] = argv[1] = argv[2] = NULL;
    if (parse_n_in_fold(&argv[1], &argv[0]) == FAIL)
	return(TOK_CREATE);
    if (eqwaste(argv[0]))
	return(FAIL);
    if (isfolder(argv[0]) == FALSE) {
	mess_temp("You can only create new objects inside File folders");
	return(FAIL);
    }
    if (access(argv[0], 02) < 0) {
	mess_temp(nstrcat("You don't have permission to create objects in ",
	    bsd_path_to_title(argv[0], MESS_COLS-47), NULL));
	return(FAIL);
    }
    if (argv[1] == NULL) {
	enter_getname("create", "", argv);
	return(TOK_NOP);
    }
    if (namecheck(argv[0], argv[1], NULL, &errstr, TRUE) == FALSE) {
	mess_temp(errstr);
	argv[1] = NULL;
	enter_getname("create", "", argv);
	return(TOK_NOP);
    }
    Create_create(argv);
    return(TOK_NOP);
}

int
Create_create(argv)
char *argv[];
{
	char *bsd_path_to_title();
	char *path;

	working(TRUE);
	path = bsd_path_to_title(argv[1], (COLS-30)/2);
	return(objop("OPEN", "MENU", "$VMSYS/OBJECTS/Menu.create",
	    argv[0], argv[1], path,
	    bsd_path_to_title(argv[0], COLS - strlen(path)), NULL));
}

static int
eqwaste(str)
char *str;
{
	extern char *Wastebasket;

	if (strncmp(str, Wastebasket, strlen(Wastebasket)) == 0) {
		mess_temp("You cannot create objects in your WASTEBASKET");
		return(1);
	}
	return(0);
}
