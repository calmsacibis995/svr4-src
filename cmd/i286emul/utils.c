/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)i286emu:utils.c	1.1"

#include <stdio.h>
#include "vars.h"
#include <setjmp.h>
/*
 * read "count" bytes from "offset" in file opened on "fd" to addr "base"
 */
seekto( fd, offset )
	int fd;
	long offset;
{
	if ( lseek( fd, offset, 0 ) < 0 ) {
		emprintf(  "Can't lseek to byte %d\n", offset );
		exit(1);
	}
}

readfrom( fd, base, count )
	int fd, count;
	char * base;
{
	int got;

	got = read( fd, base, count );
	if ( got != count ) {
		emprintf(  "Can't read executable file\n" );
		exit(1);
	}
}

char *
getmem( size )
{
	register char * cp;

	size = ctob(btoc(size));        /* round up to clock boundry */

	cp = malloc( size );
	if ( cp == 0 ) {
		emprintf(  "Malloc of %d byte failed\n", size );
		exit(1);
	}
	clearmem( cp, size );
	return cp;
}

emprintf( FMT, arg0, arg1, arg2, arg3, arg4, arg5 )
	char *FMT;
	long arg0, arg1, arg2, arg3, arg4, arg5;
{
	fprintf( stderr, "i286emul: " );
	fprintf( stderr, FMT, arg0, arg1, arg2, arg3, arg4, arg5 );
}

dprintf( a,b,c,d,e,f,g,h,i,j,k,l,m,n )
{
#ifdef DEBUG
	printf( a,b,c,d,e,f,g,h,i,j,k,l,m,n );
#endif
}

/*
 * convert 286 selector:offset to 386 pointer
 */
char * cvtptr( seloff )
	unsigned int seloff;
{
	int index, off;

	if ( seloff == 0 )
		return 0;
	off = seloff & 0xFFFF;
	index = (seloff>>19) & 0x1FFF;

	if ( index >= MAXDSEGS )
		return BAD_ADDR;
	return dsegs[index].base + off;
}

/*
 * convert 286 selector:offset to 386 pointer, and check for legality
 */
extern jmp_buf bad_pointer;

char * cvtchkptr( seloff )
	unsigned int seloff;
{
	int index, off;
	long answer;

	if ( seloff == 0 )
		longjmp( bad_pointer, 1 );
	off = seloff & 0xFFFF;
	index = (seloff>>19) & 0x1FFF;

	if ( index >= MAXDSEGS )
		answer = (unsigned long)BAD_ADDR;
	else
		answer = (long)dsegs[index].base;
	if ( answer == (unsigned long)BAD_ADDR )
		longjmp( bad_pointer, 1 );
	return (char *)answer + off;
}

