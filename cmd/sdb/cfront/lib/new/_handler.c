/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/lib/new/_handler.c	1.1"
/*ident	"@(#)cfront:lib/new/_handler.c	1.3" */

typedef void (*PFVV)();

extern PFVV _new_handler = 0;

extern PFVV set_new_handler(PFVV handler)
{
	PFVV rr = _new_handler;
	_new_handler = handler;
	return rr;
}
