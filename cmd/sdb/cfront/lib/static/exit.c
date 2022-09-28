/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/lib/static/exit.c	1.1"
/*ident	"@(#)cfront:lib/static/exit.c	1.5" */

extern void _exit(int);
extern void _cleanup();
extern void dtors();

extern void exit(int i)
{
	dtors();
	_cleanup();
	_exit(i);
}
