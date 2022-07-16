/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:llib-lform.c	1.1"
/*LINTLIBRARY*/
#include "message.h"
#include "tam.h"
#include "tamwin.h"
#include "form.h"
#include "wind.h"
#include "kcodes.h"

int	form(f, op) form_t *f; int op; {return 0;}
int	form_2(f, op) form_t *f; int op; {return 0;}
int	field(w, f, fl, tf, bf, doin, pval, okr, okc, width)
	int w;
	form_t *f;
	field_t *fl;
	char tf, bf, doin;
	int *pval, okr, okc;
	short width;
	{return 0;}
int	flmenu(fl) field_t *fl; {return 0;}
field_t	*fright(f, cfl) form_t *f; field_t *cfl; {return cfl;}
field_t	*fleft(f, cfl) form_t *f; field_t *cfl; {return cfl;}
field_t	*fdown(f, cfl) form_t *f; field_t *cfl; {return cfl;}
field_t	*fup(f, cfl) form_t *f; field_t *cfl; {return cfl;}
void	disp_ins(w, label, width) char *label; short width; {}
