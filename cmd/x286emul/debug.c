/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)x286emul:debug.c	1.1"

#include "vars.h"

#if defined(DEBUG) || defined(TRACE)
dump( count, data, base )
	int count;		/* how many bytes */
	char * data;		/* the data itself */
	long base;		/* address in file of data */
{
	int linecount = 0;

	if (data == BAD_ADDR) return;
	while ( count > 0 ) {
		int i;
		fprintf(dbgfd, "0x%.8x: ", base );	/* ADDR field */
		for ( i = 0; i < 16; i++ ) {
			if ( i < count )
				pr2x( data[i] );
			else
				fprintf(dbgfd, ".." );
			if ( i&1 )
				fprintf(dbgfd, " " );
		}
		fprintf(dbgfd, "  " );
		for ( i = 0; i < 16; i++ ) {
			if ( i < count ) {
				if ( data[i] > ' ' && data[i] < 128 )
					fprintf(dbgfd, "%c", data[i] );
				else
					fprintf(dbgfd, "." );
			} else
				fprintf(dbgfd, "." );
		}
		count -= 16;
		data += 16;
		base += 16;
		if ( linecount == 16 ) {
			char x;
			fflush(stdout);
			eatline();
			linecount = 1;
		} else {
			fprintf(dbgfd, "\n" );
			linecount++;
		}
	}
}

pr2x( data )
	int data;
{
	fputc( "0123456789abcdef"[(data>>4)&0xf], dbgfd );
	fputc( "0123456789abcdef"[data&0xf], dbgfd );
}

eatline() {
	int c;

	while ( (c = getchar()) != EOF )
		if ( c == '\n' )
			return;
}
#endif
