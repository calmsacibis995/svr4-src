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
#ident	"@(#)fmli:oh/dispfuncs.c	1.6"

#include <stdio.h>
#include <sys/types.h>		/* EFT abs k16 */
#include "wish.h"
#include "typetab.h"
#include "partabdefs.h"
#include "var_arrays.h"
#include "terror.h"
#include	"moremacros.h"

#define START_OBJS	20

static char **All_objtypes;
static char **All_displays;

char *
def_display(objtype)
char *objtype;
{
	register int i, size;
	struct opt_entry *opt, *obj_to_parts();

	if (!All_objtypes) {
		All_objtypes = (char **)array_create(sizeof(char *), START_OBJS);
		All_displays = (char **)array_create(sizeof(char *), START_OBJS);
	}
	size = array_len(All_objtypes);
	for (i = 0; i < size; i++)
		if (strcmp(All_objtypes[i], objtype) == 0)
			return(All_displays[i]);

	/* not found, append new */
	All_objtypes = (char **)array_append(All_objtypes, NULL);

	All_objtypes[size] = strsave(objtype);

	if (opt = obj_to_parts(objtype)) {
		All_displays = (char **)array_append(All_displays, NULL);
		All_displays[size] = strsave(opt->objdisp);
	} else {
		All_displays = (char **)array_append(All_displays, NULL);
		All_displays[size] = "Data file";
	}

	return(All_displays[size]);
}

char *
def_objtype(objtype)
char *objtype;
{
	register int i, size;
	struct opt_entry *opt, *obj_to_parts();

	if (!All_objtypes) {
		All_objtypes = (char **)array_create(sizeof(char *), START_OBJS);
		All_displays = (char **)array_create(sizeof(char *), START_OBJS);
	}

	size = array_len(All_objtypes);

	for (i = 0; i < size; i++)
		if (strcmp(All_objtypes[i], objtype) == 0)
			return(All_objtypes[i]);

	/* not found, append new */

	All_objtypes = (char **)array_append(All_objtypes, NULL);
	/* ehr 3
	if (All_objtypes[size])
		free(All_objtypes[size]);
	*/
	All_objtypes[size] = strsave(objtype);

	if (opt = obj_to_parts(objtype)) {
		All_displays = (char **)array_append(All_displays, NULL);
		/* ehr3
		if (All_objtypes[size])
			free(All_objtypes[size]);
		*/
		All_displays[size] = strsave(opt->objdisp);
	} else {
		All_displays = (char **)array_append(All_displays, NULL);
		All_displays[size] = "Data file";
	}

	return(All_objtypes[size]);
}
