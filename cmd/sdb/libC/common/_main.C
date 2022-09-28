#ident	"@(#)sdb:libC/common/_main.C	1.1"
/*ident	"@(#)ctrans:lib/static/_main.c	1.1.2.1" */
/**************************************************************************
			Copyright (c) 1984 AT&T
	  		  All Rights Reserved  	

	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
	
	The copyright notice above does not evidence any   	
	actual or intended publication of such source code.

*****************************************************************************/
extern void _main();

extern void _main()
{
	typedef void (*PFV)();
	extern PFV _ctors[];
	for (PFV* pf=_ctors; *pf; pf++) {
		(**pf)();
		*pf = 0; // permits main to be called recursively
	}
}
