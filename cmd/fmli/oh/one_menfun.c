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
#ident	"@(#)fmli:oh/one_menfun.c	1.3"

#include <stdio.h>
#include "wish.h"
#include "but.h"
#include "obj.h"
#include "terror.h"
#include "moremacros.h"
#include "sizes.h"

#define MAX_PATHS	(4)	/* max number of OBJPATH's */
#ifndef WISH
extern bool Inroot, Fromroot;
#endif
extern char *Objpaths[];

char *strtok();

int
one_menfunc(objtype, object, args, amask, nmask, fmask)
char *objtype, *object, *args;
long amask, nmask, *fmask;
{
	register char *p;
	register char *name, *path;
	register int i, numargs, retcode;
	char *argbuf[MAX_ARGS];

	char *filename();

	argbuf[0] = object;
	if (args && (argbuf[1] = strtok(args, "\t\n"))) {
		numargs = 2;
		while (numargs < (MAX_ARGS-1)) {
			argbuf[numargs] = strtok(NULL, "\t\n");
			if (argbuf[numargs] == NULL)
				break;
			numargs++;
		}
	} else
		numargs = 1;
	argbuf[numargs] = NULL;

	if (retcode = menu_args(argbuf)) {
		free_args(argbuf);
		return(retcode);
	}
	object = argbuf[0];

	if (strcmp(objtype, "NOP") == 0) {
		free_args(argbuf);
		return(NOPAINT);
	} else if (strcmp(objtype, "RUN") == 0 ) {
		return(do_run(argbuf));
	} else if (strcmp(objtype, "MODE") == 0) {
		return(do_mode(argbuf, amask, nmask, fmask));
#ifndef WISH
	} else if (strcmp(objtype, "SCREENSIZE") == 0) {
		return(do_screensize(argbuf));
	} else if (strcmp(objtype, "DIAL") == 0) {
		return(do_dial(argbuf));
	} else if (strcmp(objtype, "PRINT") == 0) {
		return(do_print(argbuf));
	} else if (strcmp(objtype, "ECHO") == 0) {
		return(do_echo(argbuf));
	} else if (strcmp(objtype, "SENDMAIL") == 0) {
		int retcode, inroot = Inroot, fromroot = Fromroot;
		Fromroot = Inroot;
		Inroot = FALSE;
		retcode = newsend(argbuf);
		free_args(argbuf);
		Inroot = inroot;
		Fromroot = fromroot;
		return(retcode);
	} else if (strcmp(objtype, "READENV") == 0) {
		oh_init(FALSE);
		force_reread();
		free_args(argbuf);
		return(NOPAINT);
	} else if (strcmp(objtype, "RESALIAS") == 0) {
		if (argbuf[0]) {
			no_resume(atoi(argbuf[0]));
		} else
			error(MISSING, "Argument to RESALIAS");
		free_args(argbuf);
		return(NOPAINT);
	} else if (strcmp(objtype, "DUMPOBJ") == 0) {
		if (argbuf[1])
				ootwrite(argbuf[0], argbuf[1]);
		else if (argbuf[0] && strcmp(argbuf[0], "ALL") == 0) {
			ootwrite(NULL,NULL);
		} else
			error(MISSING, "Argument to DUMPOBJ");
		free_args(argbuf);
		return(NOPAINT);
#else	/** WISH Ignores Certain TeleSystem Menu Commands for now **/
	} else if (strcmp(objtype, "SCREENSIZE") == 0 ||
				strcmp(objtype, "DIAL") == 0 ||
				strcmp(objtype, "DUMPOBJ") == 0 ||
				strcmp(objtype, "PRINT") == 0 ||
				strcmp(objtype, "ECHO") == 0 ||
				strcmp(objtype, "SENDMAIL") == 0 ||
				strcmp(objtype, "READENV") == 0 ||
				strcmp(objtype, "RESALIAS") == 0) {
		free_args(argbuf);
		return(NOPAINT);
#endif
	} else if (strcmp(objtype, "SET") == 0) {
		return(do_set(argbuf));
	} else if (strcmp(objtype, "PREREAD") == 0) {
		register int i;
		for (i = 0; argbuf[i]; i++)
			ott_get(argbuf[i], 0, 0, 0, 0);
		return(NOPAINT);
	} else {
	/* The only remaining possibility is that this is an object
	 * that should be processed.  So, create an object entry and
	 * do it.
	 */
		p = argbuf[0];
		name = filename(p);
		path = NULL;
		if (*p == '/')
			path = strsave(parent(p));
		else {
			char buf[PATHSIZ];

			for (i = 0; i < MAX_PATHS && Objpaths[i]; i++) {
				strcpy(buf, Objpaths[i]);
				strcat(buf, "/");
				strcat(buf, object);
				if (access(buf, 0) == 0) {
					path = strsave(Objpaths[i]);
					break;
				}
			}
			if (!path)
				path = strsave(".");
		}

		if (strcmp(objtype, "OBJECT") == 0) 	/* type not known */
			objtype = NULL;

#ifndef WISH
		{
			long strtol();
			long newamask = amask, newnmask = nmask, newfmask = *fmask;
			bool inroot = Inroot, fromroot = Fromroot;	/* stack globals */

			Fromroot = Inroot;
			Inroot = FALSE;
			if (argbuf[1] != NULL) {
				newamask = strtol(argbuf[1],NULL,16);
				if (argbuf[2] != NULL) {
					newnmask = strtol(argbuf[2],NULL,16);
					if (argbuf[3] != NULL)
						newfmask = strtol(argbuf[3],NULL,16);
				}
			}
			if (!(retcode = make_object(path, name, objtype, FORCE, newamask, 
							newnmask, newfmask))) {	/* returns NULL on error */
				wprintf(DWH, "Unable to access %s %s\n",
					objtype?objtype:"object", name);
				beep();
				flush_one_window(DWH);
				sleep(4);
				retcode = REREAD;
			}
			Inroot = inroot;		/*restore globals */
			Fromroot = fromroot;
		}
#else
		retcode = object_open(name, path, objtype);
#endif
		/* p need not be freed, since it is argbuf[0] */
		free_args(argbuf);
		return(retcode);
	}
}
