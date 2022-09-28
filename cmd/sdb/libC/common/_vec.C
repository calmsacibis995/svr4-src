#ident	"@(#)sdb:libC/common/_vec.C	1.1"
/*ident	"@(#)ctrans:lib/new/_vec.c	1.1" */
/**************************************************************************
			Copyright (c) 1984 AT&T
	  		  All Rights Reserved  	

	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
	
	The copyright notice above does not evidence any   	
	actual or intended publication of such source code.

*****************************************************************************/
typedef void* PV;
typedef void (*PF)(PV);
typedef void (*PFI)(PV,int);

PV _vec_new(PV op, int n, int sz, PV f)
/*
	allocate a vector of "n" elements of size "sz"
	and initialize each by a call of "f"
*/
{
	if (op == 0) op = PV( new char[n*sz] );
	register char* p = (char*) op;
	if (f) for (register int i=0; i<n; i++) ( *PF(f) )( PV(p+i*sz) );
	return PV(p);
}

void _vec_delete(PV op, int n, int sz, PV f, int del)
{
	if (op == 0) return;
	if (f) {
		register char* p = (char*) op;
		for (register int i=0; i<n; i++) ( *PFI(f) )(PV(p+i*sz),0);
	}
	if (del) delete op;
}
