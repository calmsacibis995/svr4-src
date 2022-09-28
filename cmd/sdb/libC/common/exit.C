#ident	"@(#)sdb:libC/common/exit.C	1.1"
/*ident	"@(#)ctrans:lib/static/exit.c	1.1.2.1" */
/**************************************************************************
			Copyright (c) 1984 AT&T
	  		  All Rights Reserved  	

	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
	
	The copyright notice above does not evidence any   	
	actual or intended publication of such source code.

*****************************************************************************/

extern void _exit(int);
extern void _cleanup();
extern void dtors();

extern void exit(int i);

extern void exit(int i)
{
	dtors();
	_cleanup();
	_exit(i);
}
