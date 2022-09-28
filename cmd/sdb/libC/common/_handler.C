#ident	"@(#)sdb:libC/common/_handler.C	1.1"
/*ident	"@(#)ctrans:lib/new/_handler.c	1.2" */
/**************************************************************************
			Copyright (c) 1984 AT&T
	  		  All Rights Reserved  	

	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
	
	The copyright notice above does not evidence any   	
	actual or intended publication of such source code.

*****************************************************************************/

typedef void (*PFVV)();
extern PFVV _new_handler = 0;

extern PFVV set_new_handler(PFVV handler)
{
	PFVV rr = _new_handler;
	_new_handler = handler;
	return rr;
}
