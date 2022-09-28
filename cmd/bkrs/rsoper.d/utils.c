/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:rsoper.d/utils.c	1.8.2.1"

#include	<sys/types.h>
#include	<time.h>
#include	<string.h>
#include	<stdio.h>
#include	<pwd.h>
#include	<bktypes.h>
#include	<brarc.h>
#include	<bkrs.h>
#include 	"libadmIO.h"

#ifndef	FALSE
#define TRUE 1
#define FALSE 0
#endif

#define HDR "   machine   method  file system                device   volume              date seq volsz archsz \n"

extern int ascftime();
extern void free();
extern argv_t *s_to_argv();
extern void argv_free();
extern struct passwd *getpwuid();
extern struct passwd *getpwnam();
extern int get_hdr();

uid_t
uname_to_uid( username )
char *username;
{
	struct passwd *pw;
	if( !(pw = getpwnam( username )) ) return( -1 );
	return( pw->pw_uid );
}

char *
uid_to_uname( uid )
uid_t uid;
{
	struct passwd *pw;
	if( !(pw = getpwuid( uid )) ) return( (char *)0 );
	return( pw->pw_name );
}

/* Read the archive information from a volume - do not override current info */
void
get_arch_info( ddevice, dchar, machine, oname, odevice, method, label, toc )
char *ddevice, *dchar, **machine, **oname, **odevice, **method, **label;
int *toc;
{
	archive_info_t ai;
	char buffer[ 40 ];

	strncpy( buffer, "", 40 );

	if( get_hdr( ddevice, dchar, *label, &ai, BR_PROMPT_ALLWD, (GFILE **)0 ) ) {
#ifdef TRACE
		brlog( "get_arch_info(): found label: %s", ai.br_mname );
#endif
		
		if( !*machine && !**machine )
			*machine = strdup( ai.br_sysname );
		
		if( !*oname && !**oname )
			*oname = strdup( ai.br_fsname );
		
		if( !*odevice && !**odevice )
			*odevice = strdup( ai.br_dev );
		
		if( !*method && !**method )
			*method = strdup( ai.br_method );
		
		if( !*label && !**label )
			*label = strdup( ai.br_mname );
		
		if( !*toc )
			*toc = (ai.br_flags & BR_IS_TOC);
	}
#ifdef TRACE
	else brlog( "get_arch_info(): no label found" );
#endif

#ifdef TRACE
	brlog( "get_arch_info(): return; buffer[0] = 0x%x", buffer[0] );
#endif
}

int
p_arch_info( ddevice, dchar, dmnames )
char *ddevice, *dchar, *dmnames;
{
	archive_info_t ai;
	char *ptr, buffer[ 40 ];

	if( ptr = strchr( dmnames, ',' ) )
		*ptr = '\0';

	if( get_hdr( ddevice, dchar, dmnames, &ai, BR_PROMPT_ALLWD, (GFILE **)0 ) ) {
		
		ascftime( buffer, "%b %d %R %Y", localtime( &(ai.br_date) ) );

		(void) fprintf( stdout, HDR );
		
		(void) fprintf( stdout,
			"%10.10s %8.8s %12.12s %21.21s %8.8s %17.17s %3d %5d %6d %s\n",
				ai.br_sysname, ai.br_method, ai.br_fsname, ai.br_dev,
				ai.br_mname, buffer, ai.br_seqno, ai.br_media_cap,
				(int) ai.br_blk_est, ((ai.br_flags & BR_IS_TOC)? "TOC": "") );

		return( 1 );
	} 

	(void) fprintf( stdout, "No archive information available.\n" );
	return( 0 );
}

/* is list 'a' a sublist of 'b'? */
int
sublist( a, b, separators )
char *a, *b, *separators;
{
	argv_t *a_argv, *b_argv;
	register i, j, is_in;

	/* Make private copies */
	if( !a || !(a = strdup( a )) ) return( FALSE );
	if( !b || !(b = strdup( b )) ) return( FALSE );

	if( !(a_argv = s_to_argv( a, separators ) ) ) {
		free( a );
		free( b );
		return( FALSE );
	}

	if( !(b_argv = s_to_argv( b, separators ) ) ) {
		free( a );
		free( b );
		(void) argv_free( a_argv );
		return( FALSE );
	}

	for( i = 0; (*a_argv)[i]; i++ ) {

		is_in = FALSE;

		for( j = 0; (*b_argv)[j]; j++ ) 

			if( !strcmp( (*a_argv)[i], (*b_argv)[i] ) ) {
				is_in = TRUE;
				break;
			}

		if( !is_in ) {
			free( a );
			free( b );
			(void) argv_free( a_argv );
			(void) argv_free( b_argv );
			return( FALSE );
		}
	}

	free( a );
	free( b );
	(void) argv_free( a_argv );
	(void) argv_free( b_argv );
	return( TRUE );
}
