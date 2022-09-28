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
#ident	"@(#)fmli:sys/ar_mfuncs.c	1.3"

#include <stdio.h>
#include <varargs.h>
#include "wish.h"
#include "token.h"
#include "slk.h"
#include "actrec.h"

/* This file contains standard menu functions that can be used for
 * popup internal menus.  They simply take the activation record
 * pointer's id field and call the equivalent menu function.
 */

int
AR_MEN_CLOSE(a)
struct actrec *a;
{ return(menu_close(a->id)); }

int
AR_MEN_CURRENT(a)
struct actrec *a;
{ return(menu_current(a->id)); }

int
AR_MEN_NONCUR(a)
struct actrec *a;
{ return(menu_noncurrent(a->id)); }

int
AR_NOP(a)
struct actrec *a;
{ return(SUCCESS); }

int
AR_NOHELP(a)
struct actrec *a;
{
	mess_temp("No help available here");
	return(SUCCESS);
}

int
AR_MEN_CTL(a, cmd, arg1, arg2, arg3, arg4, arg5, arg6)
struct actrec *a;
int cmd, arg1, arg2, arg3, arg4, arg5, arg6;
{ return(menu_ctl(a->id, cmd, arg1, arg2, arg3, arg4, arg5, arg6)); }

token
AR_MEN_ODSH(a, t)
struct actrec *a;
token t;
{ 
	token menu_stream();

	if (t == TOK_CANCEL)
		t = TOK_CLOSE;
	return(menu_stream(t));
}

void
ar_menu_init(a)
struct actrec *a;
{
	extern struct slk Echslk[];

	a->lifetime = AR_SHORTERM;
	a->path = NULL;
	a->odptr = NULL;
	a->slks = &Echslk[0];
	a->flags = 0;
	a->fcntbl[AR_CLOSE] = AR_MEN_CLOSE;
	a->fcntbl[AR_REREAD] = AR_NOP;
	a->fcntbl[AR_REINIT] = AR_NOP;
	a->fcntbl[AR_CURRENT] = AR_MEN_CURRENT;
	a->fcntbl[AR_TEMP_CUR] = AR_MEN_CURRENT;  /* abs k16 */
	a->fcntbl[AR_NONCUR] = AR_MEN_NONCUR;
	a->fcntbl[AR_CTL] = AR_MEN_CTL;
	a->fcntbl[AR_ODSH] = (int (*)())AR_MEN_ODSH; /* added cast abs */
	a->fcntbl[AR_HELP] = AR_NOHELP;
}
