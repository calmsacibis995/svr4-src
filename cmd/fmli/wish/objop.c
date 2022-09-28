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
#ident	"@(#)fmli:wish/objop.c	1.12"

#include <stdio.h>
#include <varargs.h>
#include <sys/types.h>		/* EFT abs k16 */
#include "wish.h"
#include "typetab.h"
#include "optabdefs.h"
#include "terror.h"
#include "procdefs.h"
#include "moremacros.h"
#include "message.h"
#include "sizes.h"

#define NUM_REQ_OPS	(4)

/*
 * Caution: MAX_ARGS is defined in other files and should ultimately reside 
 * in wish.h
 */
#define MAX_ARGS	(25)

extern int (*Function[])();
bool       isfolder();


int
objop(op, objtype, va_alist)
char *op, *objtype;
va_dcl
{
	register int i, argc;
	char *argv[MAX_ARGS];
	va_list a;

	/* decode the args into the argv structure */

	va_start(a);
	for (argc = 0; argc < MAX_ARGS-1 && (argv[argc] = va_arg(a,char *)); argc++)
#ifdef _DEBUG
		_debug(stderr, "%s ", argv[argc]);
#else
		;
#endif
	argv[MAX_ARGS-1] = NULL;
	va_end(a);

	return(objopv(op, objtype, argv));
}

int
objopv(op, objtype, argv)
char *op, *objtype;
char *argv[];
{
    register int ret, i, argc, opindex;
    register bool found = FALSE;
    char *object;
    char *path, *p;
    char *argvhold[MAX_ARGS];
    struct operation **oot;
    struct ott_entry *ott = NULL, *path_to_ott();
    char	*bsd_path_to_title();

    struct operation **obj_to_oot();
    char *path_to_full();

    object = path_to_full(argv[0]);

    /*
     * Test to see if the object is "/" ... if it is
     * then specify its objtype (DIRECTORY) since there is
     * no "parent" ott in which to determine its type
     */
    if (object && strcmp(object, "/") == 0) {
	if (!objtype || strCcmp(objtype, "OBJECT") == 0)
	    objtype = "DIRECTORY";
    }
    argvhold[0] = object;
    for (argc = 0; argv[argc]; argc++)
	;

    /* if no object type given, find it by getting ott */

    if (!objtype || strCcmp(objtype, "OBJECT") == 0) {
	if ((ott = path_to_ott(object)) == NULL)
	    return(FAIL);
	objtype = ott->objtype;
    }

    if ((oot = obj_to_oot(objtype)) == NULL) {
	mess_temp("I do not recognize that kind of object.");
	return(FAIL);
    }

    for (opindex = NUM_REQ_OPS; oot[opindex]; opindex++) {
	if (strCcmp(op, oot[opindex]->opername) == 0) {
	    found = TRUE;
	    break;
	}
    }
    if (!found) {
	char msg[MESSIZ];

	sprintf(msg, "No %s operation defined for this object.", op);
	mess_temp(msg);
	return(FAIL);
    }
    if (oot[opindex]->none_mask & M_EN && strcmp(object, "-i") != 0)	{ /* not allowed on encrypted */
	if (ott == NULL && (ott = path_to_ott(object)) == NULL)
	    return(FAIL);
	if (ott->objmask & M_EN) {
	    char msg[MESSIZ];

	    sprintf(msg, "Cannot %s a scrambled object.  Unscramble it first.", op);
	    mess_temp(msg);
	    return(FAIL);
	}
    }

    /* gather up the arguments */

    switch (oot[opindex]->op_type & (OP_CUR|OP_NEW|OP_SNG|OP_DIR)) {
    case OP_SNG:		/* Single, no other args needed than the first */
	for (i = 1; argv[i]; i++)
	    argvhold[i] = strsave(argv[i]);
	argvhold[i] = NULL;
	break;
    case OP_NEW|OP_DIR:		/* copy-like */
    case OP_NEW|OP_CUR:		/* copy-like */
    case OP_DIR:		/* move-like */
    case OP_CUR:
	if (argc <= 1) {
	    enter_browse(op, objtype, argv);
	    return(SUCCESS);
	} else {
	    if (strCcmp(argv[1], "to") == 0) {
		for (i = 1; argvhold[i] = argv[i+1]; i++)
		    ;
		argc--;
	    } else
		for (i = 1; argvhold[i] = argv[i]; i++)
		    ;
	} 
	/* first two args are paths, third arg is alternate name */
	for (i = 1; argvhold[i]; i++) {
	    if (i < 2)
		argvhold[i] = path_to_full(argvhold[i]);
	    else
		argvhold[i] = strsave(argvhold[i]);
	}

	if (argc == 2) {
	    char *errstr;
	    char oldname[ONAMESIZ];
			
	    if (isfolder(argvhold[1]) == FALSE)
		return(FAIL);
	    if (ott == NULL && (ott = path_to_ott(object)) == NULL)
		return(FAIL);
	    strcpy(oldname, ott->name);
	    if (namecheck(argvhold[1], oldname,
			  objtype, &errstr, TRUE) == FALSE) {
		mess_temp(errstr);
		enter_getname(op, objtype, argvhold);
		return(SUCCESS);
	    }
	}
	break;
    case OP_NEW:		/* rename-like */
	if (argc <= 1) {
	    enter_getname(op, objtype, argv);
	    return(SUCCESS);
	} else {
	    if (strcmp(argv[1], "as") == 0 || strcmp(argv[1], "to") == 0) {
		for (i = 1; argv[i+1]; i++)
		    argvhold[i] = strsave(argv[i+1]);
	    } else
		for (i = 1; argv[i]; i++)
		    argvhold[i] = strsave(argv[i]);
	    argvhold[i] = NULL;
	}
	break;
    default:
	for (i = 1; argv[i]; i++)
	    argvhold[i] = strsave(argv[i]);
	argvhold[i] = NULL;
#ifdef _DEBUG
	_debug(stderr, "Unimplemented object operation type");
#endif
	break;
    }

#ifdef _DEBUG
    _debug(stderr, "OBJOP about to execute %s %s ", op, objtype);
    for (i = 0; argvhold[i]; i++)
	_debug(stderr, "%s ", argvhold[i]);
    _debug(stderr, "\n");
#endif

    /* execute the function */

    switch (oot[opindex]->func_type) {
    case F_NOP:
	ret = SUCCESS;
	break;
    case F_ILL:
	ret = FAIL;
	break;
    case F_INT:
	ret = (*(Function[oot[opindex]->intern_func]))(argvhold);
	break;
    case F_EXEC:
    case F_SHELL:
    {
	char command[BUFSIZ];
	char title[BUFSIZ];
	int length;

	sprintf(title, "Suspended %s", bsd_path_to_title(argvhold[0], COLS-14));
	length = sprintf(command, "%s ", oot[opindex]->extern_func);
	for (i = 0; argv[i]; i++)
	    length += sprintf(command + length, "%s ", argvhold[i]);
	proc_opensys(PR_ERRPROMPT, title, NULL, command);
	if (ott == NULL && (ott = path_to_ott(object)) == NULL)
	    ret = FAIL;
	else {
	    ott_mtime(ott);	/* update internal mod time */
	    ret = SUCCESS;
	}
    }
	break;
    default:
#ifdef _DEBUG
	_debug(stderr, "Unimplemented object operation type");
#endif
	ret = FAIL;
	break;
    }

    for (i = 0; argvhold[i]; i++)
	free(argvhold[i]);

    return(ret);
}

bool
isfolder(path)
char *path;
{
    struct ott_entry *ott;
    struct ott_entry *path_to_ott();
    char	*bsd_path_to_title();

    if ((ott = path_to_ott(path)) == NULL) {
	return(FALSE);
    } else if ((ott->objmask & CL_DIR) == 0) {
	mess_temp(nstrcat("Object ",
			  bsd_path_to_title(ott_to_path(ott), MESS_COLS-23), 
			  " is not a folder", NULL));
	return(FALSE);
    }
    return(TRUE);
}
