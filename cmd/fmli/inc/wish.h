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
 *
 */

#ident	"@(#)fmli:inc/wish.h	1.7"

#include	<malloc.h>

#define MALLOC

/* abs: shut lint up.
char *calloc();
*/
#ifndef TRUE
#define TRUE	(1)
#define FALSE	(0)
#endif
#define UNDEFINED (-1)

#ifndef TYPE_BOOL
/* curses.h also  does a typedef bool */
#ifndef CURSES_H
#define TYPE_BOOL
typedef char bool;
#endif
#endif

#define FAIL	(-1)
#define SUCCESS	(0)

/* abs reverse order of args to calloc() 
#define new(X)		((X *) ((_tmp_ptr = calloc(sizeof(X), 1)) == NULL ? (char *) fatal(NOMEM, nil) : _tmp_ptr))
*/
#define new(X)		((X *) ((_tmp_ptr = calloc(1, sizeof(X))) == NULL ? (char *) fatal(NOMEM, nil) : _tmp_ptr))
#define _debug0		(!(_Debug & 1)) ? 0 : fprintf
#define _debug1		(!(_Debug & 2)) ? 0 : fprintf
#define _debug2		(!(_Debug & 4)) ? 0 : fprintf
#define _debug3		(!(_Debug & 8)) ? 0 : fprintf
#define _debug4		(!(_Debug & 16)) ? 0 : fprintf
#define _debug5		(!(_Debug & 32)) ? 0 : fprintf
#define _debug		_debug5
#define max(A, B)	((A) > (B) ? (A) : (B))
#define min(A, B)	((A) < (B) ? (A) : (B))

extern int	_Debug;
extern char	nil[];
extern char	*_tmp_ptr;

typedef int	vt_id;
typedef int	menu_id;
typedef int	form_id;
