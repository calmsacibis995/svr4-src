/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)fmli:form/fclose.c	1.5"

#include	<stdio.h>
#include        <curses.h>
#include	"wish.h"
#include	"token.h"
#include	"winp.h"
#include	"form.h"
#include	"vtdefs.h"
#include	"var_arrays.h"

int
form_close(fid)
form_id	fid;
{
	register int i;
	register char *argptr;
	register struct form *fptr;
	formfield ff, (*disp)();

	if (fid < 0 || !(FORM_array[fid].flags & FORM_USED)) {
#ifdef _DEBUG
		_debug(stderr, "form_close(%d) - bad form number\n", fid);
#endif
		return(FAIL);
	}
	fptr = &FORM_array[fid];
	disp = fptr->display;
	argptr = fptr->argptr;
	for (i = 0, ff = (*disp)(0, argptr); ff.name != NULL; ff = (*disp)(++i, argptr)) 
		if (*(ff.ptr))
			endfield(*(ff.ptr));
	if (FORM_curid == fid)
		FORM_curid = -1;
	fptr->flags = 0;
	vt_close(fptr->vid);	/* close the window associated with the form */
	return(SUCCESS);
}
