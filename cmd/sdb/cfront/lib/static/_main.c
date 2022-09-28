/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/lib/static/_main.c	1.1"
/*ident	"@(#)cfront:lib/static/_main.c	1.4" */
extern void _main()
{
	typedef void (*PFV)();
	extern PFV _ctors[];
	for (PFV* pf=_ctors; *pf; pf++) {
		(**pf)();
		*pf = 0; // permits main to be called recursively
	}
}
