/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/src/alloc.c	1.1"
/*ident	"@(#)cfront:src/alloc.c	1.5" */
#include "cfront.h"
#include "size.h"

extern void free(char*);
extern char* malloc(unsigned);
extern char* calloc(unsigned,unsigned);
int Nchunk;

void print_free()
{
	fprintf(stderr,"free store: %d bytes alloc()=%d free()=%d\n",Nfree_store,Nalloc, Nfree);
	fprintf(stderr,"%d chunks: %d (%d)\n",Nchunk,CHUNK,Nchunk*CHUNK);
}


void* chunk(int i)	// get memory that is not to be freed
{
	register char* cp = malloc(i*CHUNK-8);
	if (cp == 0) {			// no space
		free((char*)gtbl);	// get space for error message
		if (Nspy) print_free();
		error('i',"free store exhausted");
	}
	Nchunk += i;
	Nfree_store += i*CHUNK;
	return cp;
}

void* operator new(long sz)	// get memory that might be freed
{
	char* p = calloc(sz,1);

//fprintf(stderr,"alloc(%d)->%d\n",sz,p);
	if (p == 0) {			// no space
		free((char*)gtbl);	// get space for error message
		if (Nspy) print_free();
		error('i',"free store exhausted");
	}
	Nalloc++;
	Nfree_store += sz+sizeof(int*);
	return p;
}

int NFn, NFtn, NFbt, NFpv, NFf, NFe, NFs, NFc;

void operator delete (void* p)
{
	if (p == 0) return;

//fprintf(stderr,"free(%d) %d\n",p,((int*)p)[-1]-(int)p-1+sizeof(int*));

if (Nspy) {
	Pname pp = (Pname) p;
	TOK t = pp->base;
	Nfree++;
	Nfree_store -= ((int*)p)[-1]-(int)p-1+sizeof(int*);
	switch (t) {	// can be fooled by character strings
	case INT: case CHAR: case TYPE: case VOID: case SHORT: case LONG:
	case FLOAT: case DOUBLE: case COBJ: case EOBJ: case FIELD:
			NFbt++; break;

	case PTR: case VEC:
			NFpv++; break;

	case FCT:	NFf++; break;

	case ICON: case CCON: case STRING: case FCON: case THIS:
			NFc++; break;
	}
}
	free((char*)p);
}
