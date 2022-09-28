/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:user/groups.c	1.5.7.1"

#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <grp.h>
#include <unistd.h>
#include <userdefs.h>
#include "users.h"

struct group *getgrnam();
struct group *getgrgid();
struct group *fgetgrent();
extern void putgrent();
extern long strtol();
extern pid_t getpid();
extern int strcmp(), copyfile(), unlink();

int
edit_group( login, new_login, gids, overwrite )
char *login, *new_login;
gid_t gids[];		/* group id to add login to */
int overwrite;	/* overwrite != 0 means replace existing ones */
{
	char **memptr, t_name[ 20 ];
	FILE *e_fptr, *t_fptr;
	struct group *g_ptr;	/* group structure from fgetgrent */
	register i, modified = 0;

	if( (e_fptr = fopen(GROUP, "r")) == NULL ) {
		return( EX_UPDATE );
	}
	
	(void) sprintf( t_name, "/tmp/%ld", getpid() );

	if( (t_fptr = fopen( t_name, "w+")) == NULL ) {
		return( EX_UPDATE );
	}

	/* Make TMP file look like we want GROUP file to look */
	while( (g_ptr = fgetgrent( e_fptr )) != NULL ) {
		
		/* first delete the login from the group, if it's there */
		if( overwrite || !gids )
			for( memptr = g_ptr->gr_mem; *memptr; memptr++ )
				if( !strcmp( *memptr, login ) ) {
					/* Delete this one */
					char **from = memptr + 1;

					do {
						*(from - 1) = *from;
					} while( *from++ );

					modified++;
					break;
				}
		
		/* now check to see if group is one to add to */
		if( gids )
			for( i = 0; gids[ i ] != -1; i++ ) 
				if(g_ptr->gr_gid == gids[i]) {
					/* Find end */
					for( memptr = g_ptr->gr_mem; *memptr; memptr++ )
						;

					*memptr++ = new_login? new_login: login;
					*memptr = NULL;

					modified++;
				}

		putgrent( g_ptr, t_fptr );

	}

	(void) fclose( e_fptr );
	(void) fclose( t_fptr );
	
	/* Now, update GROUP file, if it was modified */
	if( modified && copyfile( t_name, GROUP ) < 0 ) {
		(void) unlink( t_name );
		return( EX_UPDATE );
	}

	(void) unlink( t_name );
		
	return( EX_SUCCESS );
}
