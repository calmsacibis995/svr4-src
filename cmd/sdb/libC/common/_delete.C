#ident	"@(#)sdb:libC/common/_delete.C	1.1"
/*ident	"@(#)ctrans:lib/new/_delete.c	1.1.2.1" */
/**************************************************************************
			Copyright (c) 1984 AT&T
	  		  All Rights Reserved  	

	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
	
	The copyright notice above does not evidence any   	
	actual or intended publication of such source code.

*****************************************************************************/
extern free(char*);

extern void operator delete(void* p)
{
	if (p) free( (char*)p );
}
