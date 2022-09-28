/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/lib/new/_new.c	1.1"
/*ident	"@(#)cfront:lib/new/_new.c	1.3" */

typedef void (*PFVV)();

extern PFVV _new_handler;

extern void* operator new(long size)
{
	extern char* malloc(unsigned);
	char* p;

	while ( (p=malloc(unsigned(size)))==0 ) {
		if(_new_handler)
			(*_new_handler)();
		else
			return 0;
	}
	return (void*)p;
}
