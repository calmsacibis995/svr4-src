/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:lib/getkval.c	1.2.7.1"



#include	<fcntl.h>
#include	<nlist.h>

static int kfd = 0;
static struct nlist kvalue[ 2 ];

extern int read();
extern char *strncpy();
extern long lseek();

/* Get a value for a kernel variable */
int
getkval( name, size, result )
char *name;
unsigned size;
char *result;
{
	(void) strncpy( (char *)kvalue, "", 2*sizeof( struct nlist ) );
	kvalue[ 0 ].n_name = name;
	if( nlist( "/unix", kvalue ) )
		return( 0 );
	
	if( !kfd && (kfd = open( "/dev/kmem", O_RDONLY )) == -1 )
		return( 0 );

	if( lseek( kfd, kvalue[0].n_value, 0 ) == -1
		|| read( kfd, result, size ) != size )
		return( 0 );

	return( 1 );
}
