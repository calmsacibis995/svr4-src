/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)alint:common/xl_byte.c	1.1"
#include <stdio.h>
#include "p1.h"
#include "lnstuff.h"
#include "xl_byte.h"

int xl_swap;	/* true if host and target have different byte orders */

void
xl_init()
/* Initialize xl_swap. */
{
	
	union {
		int i;
		char c[sizeof(int)];
	} u;

	u.i = 1;
#ifdef LTORBYTES
	xl_swap = u.c[0] == 1;
#else
	xl_swap = u.c[0] == 0;
#endif
#ifndef NODBG
	if (ln_dbflag) fprintf(stderr, xl_swap ? 
		"Byte order different on target machine\n" :
		"Byte order same on target machine\n");
#endif
}

unsigned long
xl_xlate(var, size)
unsigned long var;
{
	unsigned int val;
	static int mask = 0xff;	

	switch (size) {	
	case 4:
		val = ((var >> 24) & mask)
		    | (((var >> 16) & mask) << 8)
		    | (((var >> 8) & mask) << 16)
		    | ((var & mask) << 24);
		break;
	case 2:
		val = ((var >> 8) & mask)
		    | ((var & mask) << 8);
		break;
	case 1:	
		val = var;
		break;
	default:
		lerror(0, "bad size in XL_BODY");
		exit(0);
	}						

#ifndef NODBG
	if (ln_dbflag) 
	    fprintf(stderr, "translation %d bytes: %#x : %#x\n", size, var, val);
#endif
	return val;
}

FLENS *
xl_t_flens(f)
FLENS *f;
/* Call xl_ routines on each part of f, return pointer to translated 
** structure. 
*/
{
	static FLENS tmp;
#ifndef NODBG
	if (ln_dbflag) fprintf(stderr, "in xl_t_flens()\n");
#endif
	tmp.f1 = XL_XLATE(f->f1);
	tmp.f2 = XL_XLATE(f->f2);
	tmp.f3 = XL_XLATE(f->f3);
	tmp.f4 = XL_XLATE(f->f4);
	tmp.ver = XL_XLATE(f->ver);
	tmp.mno = XL_XLATE(f->mno);
	return &tmp;
}

ATYPE *
xl_t_atype(a, flag)
ATYPE *a;
int flag;
/* Call xl_ routines on each part of a.  Since there is a union, we
** do not know whether to swap a->extra.pos or a->extra.ty.  We rely
** on the flag for this information.  Return a pointer to the
** translated structure.
*/
{
	static ATYPE tmp;
#ifndef NODBG
	if (ln_dbflag) fprintf(stderr, "in xl_t_atype()\n");
#endif
	tmp.aty = XL_XLATE(a->aty);
	tmp.dcl_mod = XL_XLATE(a->dcl_mod);
	tmp.dcl_con = XL_XLATE(a->dcl_con);
	tmp.dcl_vol = XL_XLATE(a->dcl_vol);
	if (flag & XL_POS)
		tmp.extra.pos = XL_XLATE(a->extra.pos);
	else if (flag & XL_TY)
		tmp.extra.ty = XL_XLATE(a->extra.ty);
	else
		tmp.extra = a->extra;	/* No translation */
	return &tmp;
}

LINE *
xl_t_line(line, flag)
LINE *line;
int flag;
/* Translate a LINE.  Pass flag to xl_t_atype().  Return pointer to translated
** LINE.
*/
{
	static LINE tmp;
#ifndef NODBG
	if (ln_dbflag) fprintf(stderr, "in xl_t_line()\n");
#endif
	tmp.decflag = XL_XLATE(line->decflag);
	tmp.nargs = XL_XLATE(line->nargs);
	tmp.fline = XL_XLATE(line->fline);
	tmp.type = *xl_t_atype(&line->type, flag);
	return &tmp;
}

union rec *
xl_t_rec(r, flag)
union rec *r;
int flag;
/* Translate a union rec.  Use the flag field to determine which type
** of record this is.  Return a pointer to a rec.
*/
{
	static union rec tmp;
#ifndef NODBG
	if (ln_dbflag) fprintf(stderr, "in xl_t_rec()\n");
#endif
	if (flag & XL_LINE)
		tmp.l = *(xl_t_line(&r->l, flag));
	else if (flag & XL_F) {
		tmp.f.decflag = XL_XLATE(r->f.decflag);
/*
		tmp.f.fn = (char *)XL_XLATE((long)r->f.fn); 
*/
	}
	else
		tmp = *r;	/* Don't do any translation */
	return &tmp;
}
