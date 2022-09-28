/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:restore.d/file.c	1.3.2.1"

#include	<stdio.h>

extern unsigned int strlen();
extern void *malloc();
extern void free();

/*
	back up ptr over the previous directory name in the string.  E.g.
	if begin and ptr point to a string as follows:
	begin         ptr
	  v            v
	  /abc/def/ghi/

	then return:
	begin     ptr
	  v        v
	  /abc/def/
*/
static char *
dotdot( begin, ptr )
char *begin, *ptr;
{
	if( ptr == begin + 1 ) return( ptr );
	if( *(ptr - 1) == '/' ) ptr--;
	while( ptr != begin + 1 && *(ptr - 1) != '/' ) ptr--;
	return( ptr );
}
		

/*
	Convert a relative path to an absolute one, this function returns a
	malloc'd address.
*/
char *
f_rel_to_abs( path )
char *path;
{
	register size, cwdsz = 0;
	register char *to, *from, c;
	char *getcwd(), *cwd, *after;
	
	if( *path != '/' ) {
		cwd = getcwd( (char *)0, 512 ),
		cwdsz = strlen( cwd );
	}
	size = cwdsz + strlen( path ) + 2;

	/* Absolute path will not be longer than the relative path */
	if( !(after = (char *)malloc( (unsigned int) size ) ) ) {
		free( cwd );
		return( (char *)0 );
	}

	/*
		getcwd() returns an absolute path; therefore, that portion is already
		done.
	*/
	from = path;
	if( *path != '/' ) {
		(void) sprintf( after, "%s/", cwd );
		to = after + cwdsz + 1;
	} else to = after;

	for( c = *from++; c; )
		switch( c ) {
		case '\0':
			break;

		case '/':
			*to++ = c;
			while( (c = *from++) == '/' && c ) ;
			break;

		case '.':
			switch( *from ) {
			case '\0':
				continue;

			case '/':
				/* CURRENT directory case */
				from++;
				c = *from++;
				continue;

			case '.':
				switch( *(from + 1) ) {
				case '/':
					/* PARENT directory case */
					from++;
					to = dotdot( after, to );
					from++;
					c = *from++;
					continue;

				/* Default case handled below */
				}

			/* Default case handled below */
			}

		/*NO BREAK*/
		default:
			*to++ = c;
			while( (c = *from++) != '/' && c ) *to++ = c;
			break;
		}

	*to = '\0';

	return( after );
}
