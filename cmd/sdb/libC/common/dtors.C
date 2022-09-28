#ident	"@(#)sdb:libC/common/dtors.C	1.1"
/*ident	"@(#)ctrans:lib/static/dtors.c	1.1" */
/**************************************************************************
			Copyright (c) 1984 AT&T
	  		  All Rights Reserved  	

	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
	
	The copyright notice above does not evidence any   	
	actual or intended publication of such source code.

*****************************************************************************/

typedef void (*PFV)();

void dtors()
{
	extern PFV _dtors[];
	static ddone;
	if (ddone == 0) {	// once only
		ddone = 1;
		PFV* pf = _dtors;
		while (*pf) pf++;
		while (_dtors < pf) (**--pf)();
	}
}
