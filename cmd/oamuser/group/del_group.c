/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:group/del_group.c	1.6.7.1"



#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <grp.h>
#include <unistd.h>
#include <userdefs.h>
#include <errno.h>
#include "users.h"

struct group *getgrnam();
struct group *getgrgid();
struct group *fgetgrent();
void putgrent();

extern pid_t getpid();
extern int strcmp(), unlink(), copyfile();

/* Delete a group from the GROUP file */
int
del_group( group )
char *group;
{
	register deleted;
	FILE *e_fptr, *t_fptr;
	struct group *grpstruct;
	char tname[ 20 ];

	if( (e_fptr = fopen(GROUP, "r")) == NULL )
		return( EX_UPDATE );

	(void) sprintf( tname, "/tmp/%ld", getpid() );

	errno = 0;

	if( (t_fptr = fopen( tname, "w+" ) ) == NULL )
		return( EX_UPDATE );

	/* loop thru GROUP looking for the one to delete */
	for( deleted = 0; (grpstruct = fgetgrent( e_fptr )); ) {
		
		/* check to see if group is one to delete */
		if( !strcmp( grpstruct->gr_name, group ) )
			deleted = 1;

		else putgrent( grpstruct, t_fptr );

	}

	(void) fclose( e_fptr );
	(void) fclose( t_fptr );

	if( errno == EINVAL ) {
		/* GROUP file contains bad entries */
		(void) unlink( tname );
		return( EX_UPDATE );
	}
	
	/* If deleted, update GROUP file */
	if( deleted && copyfile( tname, GROUP ) < 0 ) {
		(void) unlink( tname );
		return( EX_UPDATE );
	}

	(void) unlink( tname );

	return( deleted? EX_SUCCESS: EX_NAME_NOT_EXIST );
}
