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
#ident	"@(#)fmli:proc/pctl.c	1.4"

#include <stdio.h>
#include <varargs.h>
#include <sys/types.h>   /* EFT abs k16 */
#include "wish.h"
#include "token.h"
#include "slk.h"
#include "actrec.h"
#include "proc.h"
#include "procdefs.h"
#include "terror.h"
#include "ctl.h"
#include "sizes.h"


extern struct proc_rec PR_all[];

int
proc_ctl(rec, cmd, arg1, arg2, arg3, arg4, arg5, arg6)
struct actrec *rec;
int cmd;
int arg1, arg2, arg3, arg4, arg5, arg6;
{
    static char title[MAX_WIDTH];
    int p = rec->id;
    register int len, i;

    switch (cmd) {
    case CTGETITLE:
	if (rec->odptr) {
	    **((char ***)(&arg1)) = rec->odptr;
	} else {
	    len = sprintf(title, "%.*s ", MAX_TITLE, PR_all[p].name);
	    i = 1;
	    while (len<MAX_TITLE  && i<MAX_ARGS && PR_all[p].argv[i]) {
		len += sprintf(title+len, "%.*s ", MAX_TITLE-len,
			       filename(PR_all[p].argv[i]));
		i++;
	    }
	    **((char ***)(&arg1)) = &title[0];
	}
	return(SUCCESS);
    case CTGETPID:
	*((pid_t *)arg1) = PR_all[rec->id].respid; /* EFT abs k16 */
	return(SUCCESS);		/* miked k17 */
    case CTSETPID:
	PR_all[rec->id].respid = (pid_t)arg1;      /* EFT abs k16 */
	return(SUCCESS);
    default:
	return(FAIL);
    }
}
