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
#ident	"@(#)fmli:oh/path_to_vp.c	1.3"

#include <stdio.h>
#include <sys/types.h>		/* EFT abs k16 */
#include "wish.h"
#include "typetab.h"
#include "optabdefs.h"
#include "obj.h"
#include "sizes.h"

extern struct operation Ascii_cv, Unknown_cv, Illeg_op, No_op;
static char Holdpath[PATHSIZ];

char *
path_to_vpath(path)
char *path;
{
	struct ott_entry *entry;
	extern char *Oasys;
	static char *viewdir = "/info/OH/view/";
	struct operation **obj_to_oot();
	struct operation *convert;
	struct ott_entry *path_to_ott();

	if ((entry = path_to_ott(path)) == NULL)
		return(NULL);
	convert = obj_to_oot(entry->objtype)[OF_MV];

	if (entry->objmask & M_EN) {
		strcpy(Holdpath, Oasys);
		strcat(Holdpath, viewdir);
		strcat(Holdpath, "scram.view");
	} else if (convert == &Ascii_cv)		/* ascii convert uses file itself*/
		return(path);
	else if (convert == &Unknown_cv)	/* unknown convert uses object type*/
		sprintf(Holdpath, "%s%sv.%s", Oasys, viewdir, entry->objtype);
	else if (convert == &Illeg_op || convert == &No_op)
		return(NULL);
	else
		sprintf(Holdpath, "%s/.v%s", entry->dirpath, entry->name);

	return(Holdpath);
}
