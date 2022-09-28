#ident	"@(#)sdb:libC/common/_new.C	1.1"
/*ident	"@(#)ctrans:lib/new/_new.c	1.2.2.1" */
/**************************************************************************
			Copyright (c) 1984 AT&T
	  		  All Rights Reserved  	

	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
	
	The copyright notice above does not evidence any   	
	actual or intended publication of such source code.

*****************************************************************************/

typedef void (*PFVV)();
extern PFVV _new_handler;
extern char* malloc(unsigned);

extern void* operator new(long size)
{
void* _last_allocation;

	while ( (_last_allocation=malloc(unsigned(size)))==0 ) {
		if(_new_handler)
			(*_new_handler)();
		else
			return 0;
	}
	return _last_allocation;
}
