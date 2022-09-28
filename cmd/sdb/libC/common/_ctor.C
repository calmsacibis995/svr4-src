#ident	"@(#)sdb:libC/common/_ctor.C	1.1"
/*ident	"@(#)ctrans:lib/static/_ctor.c	1.1" */
/**************************************************************************
			Copyright (c) 1984 AT&T
	  		  All Rights Reserved  	

	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
	
	The copyright notice above does not evidence any   	
	actual or intended publication of such source code.

*****************************************************************************/
typedef void (*PFV)();
extern PFV _ctors[] = { 0 };
