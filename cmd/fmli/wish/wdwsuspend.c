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
#ident	"@(#)fmli:wish/wdwsuspend.c	1.5"

#include <stdio.h>
#include "wish.h"
#include "token.h"
#include "slk.h"
#include "actrec.h"
#include "terror.h"
#include "ctl.h"
#include "menudefs.h"
#include "moremacros.h"

extern	menu_id menu_make();

static int
proc_list_ctl(a, cmd, arg1, arg2, arg3, arg4, arg5, arg6)
struct actrec *a;
int cmd, arg1, arg2, arg3, arg4, arg5, arg6;
{
	struct actrec *menline_to_ar();

	if (cmd == CTGETARG) {
		int line;
		struct actrec *rec, *menline_to_proc();

		(void) menu_ctl(a->id, CTGETPOS, &line);
		rec = menline_to_proc(line);
		**((char ***)(&arg1)) = strsave(rec->path);
		return(SUCCESS);
	} else
		return(menu_ctl(a->id, cmd, arg1, arg2, arg3, arg4, arg5, arg6));
}

proc_list_create()
{
	struct actrec a;
	vt_id vid;
	extern struct actrec *Proc_list;

	struct menu_line proc_menudisp();

	a.id = (int) menu_make(-1, "Suspended Activities", VT_CENTER|MENU_NONUM,
		 0, 0, proc_menudisp, NULL);
	ar_menu_init(&a);
	a.lifetime = AR_PERMANENT;
	a.fcntbl[AR_CTL] = proc_list_ctl;
	a.flags = 0;

	return(ar_current(Proc_list = ar_create(&a), FALSE)); /* abs k15 */
}
