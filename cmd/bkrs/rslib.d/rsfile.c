/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:rslib.d/rsfile.c	1.1.2.1"

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<stdio.h>
#include	<sys/mnttab.h>
#include	<errno.h>

static int bld_fstable(), cmp();

extern unsigned int strlen();
extern int strncmp();
extern void *malloc();
extern int read();
extern void qsort();
extern void free();
extern FILE *fopen();
extern char *strdup();

static struct mnttab **fstable = 0;
static int n_mounted = 0;

/*
	Return the file system that this file resides on (mounted file systems
	only are considered).
*/
char *
f_to_fs( fname )
char *fname;
{
	register i, size;

	if( !fstable && !bld_fstable() ) return( 0 );

	for( i = 0; i < n_mounted; i++ ) {
		size = strlen( fstable[i]->mnt_mountp );

		/*
			Find first file system that is a substring of fname
			(up to a '/' - /usr/abc is not the file system for
			/usr/abcde)
		*/
		if( !(strncmp( fstable[i]->mnt_mountp, fname, (unsigned int) size ) )
			&& ( !fname[ size ] || fname[ size ] == '/' ) )
			return( fstable[i]->mnt_mountp );
	}

	return( 0 );
}

static int
bld_fstable()
{
	register i;
	FILE *fptr;
	struct stat statbuf;
	struct mnttab mnt;
	char buffer[ 1024 ], *ptr;

	/* Open mount table */
	if( !(fptr = fopen( MNTTAB, "r" )) ) {
		(void) fprintf( stderr, "Unable to open %s: errno %d\n", MNTTAB, errno );
		return( 0 );
	}

	/*
		Count the number of lines in the mount table to figure number of entries
	*/

	while( (i = getc( fptr )) != EOF )
		if( i == (int)'\n' ) n_mounted++;
	
	fseek( fptr, 0, 0 );

	fstable = (struct mnttab **)malloc( n_mounted * sizeof(struct mnttab *) );

	if( !fstable ) {
		(void) fprintf( stderr, "Unable to get memory for file system table\n" );
		return( 0 );
	}
	
	/* Fill the table */
	for( i = 0; i < n_mounted; i++ ) {
		if( !(fstable[ i ] = (struct mnttab *)malloc( sizeof( struct mnttab )) ) ) {
			(void) fprintf( stderr, "Unable to get memory for file system table\n" );
			return( 0 );
		}
		if( getmntent( fptr, &mnt ) ) {
			(void) fprintf( stderr, "Unable to read %s entry number %d\n",
				MNTTAB, i );
			return( 0 );
		}

		fstable[ i ]->mnt_special = strdup( mnt.mnt_special );
		fstable[ i ]->mnt_mountp = strdup( mnt.mnt_mountp );
		fstable[ i ]->mnt_fstype = strdup( mnt.mnt_fstype );
		fstable[ i ]->mnt_mntopts = strdup( mnt.mnt_mntopts );
		fstable[ i ]->mnt_time = strdup( mnt.mnt_time );
	}

	qsort( (char *)fstable, (unsigned int) n_mounted, sizeof( struct mnttab *), cmp );

	return( 1 );
}

static int
cmp( a, b )
struct mnttab **a, **b;
{
	register i = 0;
	while( (*a)->mnt_mountp[ i ]
		&& (*b)->mnt_mountp[ i ]
		&& (*a)->mnt_mountp[ i ] == (*b)->mnt_mountp[ i ] )
		i++;

	if( !(*a)->mnt_mountp[ i ] ) 
		return( (*b)->mnt_mountp[ i ] != 0 );

	if( !(*b)->mnt_mountp[ i ] )
		return( -1 );

	return( (int) (*a)->mnt_mountp[ i ] - (int) (*b)->mnt_mountp[ i ] );
		
}

int
f_exists( fname )
char *fname;
{
	struct stat buf;
	return( !stat( fname, &buf ) );
}
