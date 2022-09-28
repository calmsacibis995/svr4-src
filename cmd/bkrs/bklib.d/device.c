/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bklib.d/device.c	1.5.2.1"

#include	<sys/types.h>
#include	<bktypes.h>
#include	<backup.h>
#include	<bkerrors.h>
#include	<devmgmt.h>
#include	<errno.h>

extern void *realloc();
extern void *malloc();
extern void free();
extern pid_t getpid();
extern void brlog();
extern char *strchr();
extern int strcmp();
extern unsigned int strlen();
extern int sprintf();

extern argv_t	*s_to_argv();
extern char	*argv_to_s(), *bkstrdup();
extern void	argv_free();

static pid_t	dpid;

#define	INCREMENT	10

argv_t *
d_getdgrp( group )
char *group;
{
	return( (argv_t *)listdgrp( group ) );
}

/*
	Attempt to allocate devices for a particular method.
	Returns a malloc'd buffer containing the name of the
	final ddevice.
*/
int
d_allocate( odevice, dgroup, ddevice )
char *odevice, **ddevice;
argv_t *dgroup;
{
	argv_t *ddev, *result;
	char *list[3], *odev[2];
	register rc, need_group = TRUE;

	if( !dpid ) dpid = getpid();

	if( !ddevice ) {
		brlog( "d_allocate(): No place to put ddevice" );
		return( BKINTERNAL );
	}

	if( *ddevice ) {
		if( !(ddev = (argv_t *)malloc( 2 * sizeof( char * ) ) ) ) {
			brlog( "d_allocate(): Unable to malloc memory" );
			return( BKNOMEMORY );
		}

		need_group = FALSE;

		(*ddev)[ 0 ] = (char *)*ddevice;
		(*ddev)[ 1 ] = (char *)0;

	} else if( !dgroup ) {

		brlog( "d_allocate(): given neither ddevice nor dgroup to allocate" );
		return( BKINTERNAL );

	} else ddev = dgroup;


	odev[ 0 ] = (char *)odevice;
	odev[ 1 ] = (char *)0;

	list[ 0 ] = (char *)odev;
	list[ 1 ] = (char *)ddev;
	list[ 2 ] = (char *)0;

	result = (argv_t *)devreserv( dpid, list );

	if( !result ) {

#ifdef TRACE
		brlog( "d_allocate(): devreserv() returns errno %d", errno );
#endif

		if( errno == ENOMEM )
			rc = BKNOMEMORY;
		else if( errno == EAGAIN )
			rc = BKBUSY;
		else rc = BKSUCCESS;
	}

	if( result )
		*ddevice = bkstrdup( (*result)[1] );

	/* Free stuff up */
	if( result ) argv_free( result );
	if( !need_group ) free( (void *) ddev );

	return( rc );
}

/* Free devices for a particular method */
void
d_free( odevice, ddevice )
char *odevice, *ddevice;
{
#ifdef TRACE
	register rc;

	brlog( "Free odev: %s ddev: %s", odevice, ddevice );

	if( odevice && *odevice ) {
		if( (rc = devfree( dpid, odevice )) == -1 )
			brlog( "d_free(): devfree() of %s returns errno %d",
				odevice, errno );
	}

	if( ddevice && *ddevice ) {
		if( (rc = devfree( dpid, ddevice )) == -1 )
			brlog( "d_free(): devfree() of %s returns errno %d",
				ddevice, errno );
	}
#else

	if( odevice && *odevice )
		(void) devfree( dpid, odevice );
	if( ddevice && *ddevice )
		(void) devfree( dpid, ddevice );
#endif
}

/*
	return the name of the attribute string (up to BKFNAME_SZ) chars.
	if the string is not of the form "name=value" return NULL.
*/
static
char *
d_getattrname( value )
char *value;
{
	static char buffer[BKFNAME_SZ];
	register char *to;
	register i;

	to = buffer;
	for( i = 0, to = buffer;
		i < BKFNAME_SZ && *value && *value != '=';
		value++, to++ )
		*to = *value;

	if( !(i < BKFNAME_SZ) ) {

		if( !strchr( value, '=' ) )
			return( (char *)0 );

	} else if( !*value )
		return( (char *)0 );

	*to = '\0';

	return( buffer );
}

/* Is attribute 'name' in 'list'? */
static
d_inlist( list, name )
argv_t *list;
char *name;
{
	register i;
	register char *attr, *a_name;
	
	for( i = 0, attr = (*list)[i]; attr = (*list)[i]; i++ )
		if( *attr && (a_name = d_getattrname( attr ))
			&& !strcmp( name, a_name ) )
				return( TRUE );

	return( FALSE );
}

/*
	resolve dchars from device mgmt and command line.
*/
char *
d_resolve( ddevice, dchar )
char *ddevice, *dchar;
{
	register i, argc, size;
	argv_t *attrs, *dchar_argv, *argv;
	char *result, *a_name, *tmp, *tmp1;

	/* listdev() returns NAMES of attributes */
	if( !(attrs = (argv_t *)listdev( ddevice ) ) )
		return( bkstrdup( dchar ) );

	dchar_argv = s_to_argv( dchar, " ," );

	if( !(argv = (argv_t *) malloc( INCREMENT * sizeof( char * ) ) ) ) {
		argv_free( dchar_argv );
		return( bkstrdup( dchar ) );
	}
	size = INCREMENT;

	/*
		argv gets all those attributes that are in attr and NOT
		in dchar_argv plus everything in dchar_argv.
	*/
	for( i = 0, argc = 0; a_name = (*attrs)[i]; i++ ) {

		if( d_inlist( dchar_argv, a_name ) )
			/* Ignore attributes in attrs and in dchar_argv */
			continue;

		if( argc == size ) {
			if( !(argv = (argv_t *)realloc( (void *) argv,
				(size + INCREMENT) * sizeof( char * ))) ) {
				argv_free( dchar_argv );
				return( bkstrdup( dchar ) );
			}
			size += INCREMENT;
		}

		/* Build <attribute_name>=<value> string */
		if( !(tmp1 = devattr( ddevice, a_name ) ) ) {
			argv_free( dchar_argv );
			return( bkstrdup( dchar ) );
		}

		if( (*argv)[argc] = (char *)malloc( strlen( a_name ) + strlen( tmp1 ) + 2 ) )
			(void) sprintf( (*argv)[argc++], "%s=%s", a_name, tmp1 );

	}

	/* Concatenate dchar_argv to argv */
	for( i = 0; tmp = (*dchar_argv)[i] ; i++ ) {

		if( argc == size ) {
			if( !(argv = (argv_t *)realloc( (void *) argv,
				(size + INCREMENT) * sizeof( char * ))) ) {
				argv_free( dchar_argv );
				return( bkstrdup( dchar ) );
			}
			size += INCREMENT;
		}

		if( *tmp )
			(*argv)[ argc++ ] = bkstrdup( tmp );
	}

	if( argc == size ) {
		if( !(argv = (argv_t *)realloc( (char *) argv,
			(size + 1) * sizeof( char * ) ) ) ) {
			argv_free( dchar_argv );
			return( bkstrdup( dchar ) );
		}
	}
	(*argv)[ argc++ ] = (char *)0;

	/* Nothing left */
	if( argc <= 1 ) {
		argv_free( argv );
		argv_free( dchar_argv );
		return( bkstrdup( dchar ) );
	}

	result = argv_to_s( argv, ',' );

	argv_free( argv );
	argv_free( dchar_argv );
	return( result );
}

/* Merge two dchar lists, attributes in list1 override those in list2 */
char *
d_merge( list1, list2 )
char *list1, *list2;
{
	register size, i, argc;
	argv_t *l_argv1, *l_argv2, *argv;
	char *tmp, *result;

	if( !(argv = (argv_t *) malloc( INCREMENT * sizeof( char * ) ) ) ) {
		return( bkstrdup( list1 ) );
	}
	size = INCREMENT;

	l_argv1 = s_to_argv( list1, "," );
	l_argv2 = s_to_argv( list2, "," );

	/*
		argv gets all those attributes that are in list2 and NOT
		in list1 plus everything in list1.
	*/
	for( i = 0, argc = 0; tmp = (*l_argv2)[i]; i++ ) {
		if( !*tmp )
			continue;

		if( d_inlist( l_argv1, d_getattrname( tmp ) ) )
			/* Ignore attributes in attrs and in dchar_argv */
			continue;

		if( argc == size ) {
			if( !(argv = (argv_t *)realloc( (void *) argv,
				(size + INCREMENT) * sizeof( char * ))) ) {
				argv_free( l_argv1 );
				argv_free( l_argv2 );
				return( bkstrdup( list1 ) );
			}
			size += INCREMENT;
		}

		(*l_argv1)[argc++] = bkstrdup( tmp );

	}

	/* Concatenate dchar_argv to argv */
	for( i = 0; tmp = (*l_argv1)[i]; i++ ) {

		if( argc == size ) {
			if( !(argv = (argv_t *)realloc( (void *) argv,
				(size + INCREMENT) * sizeof( char * ))) ) {
				argv_free( l_argv1 );
				argv_free( l_argv2 );
				return( bkstrdup( list1 ) );
			}
			size += INCREMENT;
		}

		if( *tmp )
			(*argv)[ argc++ ] = bkstrdup( tmp );
	}

	if( argc == size ) {
		if( !(argv = (argv_t *)realloc( (char *)argv,
			(size + 1) * sizeof( char * ) ) ) ) {
			argv_free( l_argv1 );
			argv_free( l_argv2 );
			return( bkstrdup( list1 ) );
		}
	}
	(*argv)[ argc++ ] = (char *)0;

	/* Nothing left */
	if( argc <= 1 ) {
		argv_free( argv );
		argv_free( l_argv1 );
		argv_free( l_argv2 );
		return( bkstrdup( list1 ) );
	}

	result = argv_to_s( argv, ',' );

	argv_free( argv );
	argv_free( l_argv1 );
	argv_free( l_argv2 );

	return( result );
}
