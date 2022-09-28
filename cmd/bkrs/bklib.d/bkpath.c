/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bklib.d/bkpath.c	1.12.2.1"

#include	<string.h>
#include	<backup.h>

extern int sprintf();

/* Where things live */
#ifdef DEBUG
extern char *br_get_oambase();
static char oambase[ BKFNAME_SZ ];
#endif

static char path[ BKFNAME_SZ ];

/* get the path of where the methods reside */
char *
bk_get_method_path( method_name )
char *method_name;
{
#ifdef DEBUG
	if( !*oambase ) (void) strcpy( oambase, br_get_oambase() );
	(void) sprintf( path, "%s/bkrs/method/%s", oambase, method_name );
#else
	(void) sprintf( path, "/etc/bkup/method/%s", method_name );
#endif
	return( path );
}

/* return path of the history log */
char *
bk_get_histlog_path()
{
	
#ifdef DEBUG
	if( !*oambase ) (void) strcpy( oambase, br_get_oambase() );
	(void) sprintf( path, "%s/bkrs/tables/%s", oambase, BK_HISTLOG );
#else
	(void) sprintf( path, "/etc/bkup/%s", BK_HISTLOG );
#endif
	return( path );
}

/* return path of the status table */
char *
bk_get_statlog_path()
{
	
#ifdef DEBUG
	if( !*oambase ) (void) strcpy( oambase, br_get_oambase() );
	(void) sprintf( path, "%s/bkrs/tables/%s", oambase, BK_STATLOG );
#else
	(void) sprintf( path, "/etc/bkup/%s", BK_STATLOG );
#endif
	return( path );
}

/* return path of the rsnotify table */
char *
bk_get_notify_path()
{
	
#ifdef DEBUG
	if( !*oambase ) (void) strcpy( oambase, br_get_oambase() );
	(void) sprintf( path, "%s/bkrs/tables/%s", oambase, RS_NTFYLOG );
#else
	(void) sprintf( path, "/etc/bkup/%s", RS_NTFYLOG );
#endif
	return( path );
}

char *
bk_get_bkdaemon_path()
{
#ifdef	DEBUG
	if( !*oambase ) (void) strcpy( oambase, br_get_oambase() );
	if( getenv( "OAMBASE" ) ) {
		(void) sprintf( path, "%s/bkrs/bin/bkdaemon", oambase );
		return( path );
	}
#endif
	return( "/usr/sadm/bkup/bin/bkdaemon" );
}

char *
bk_get_bkreg_path()
{
#ifdef DEBUG
	if( !*oambase ) (void) strcpy( oambase, br_get_oambase() );
	(void) sprintf( path, "%s/bkrs/tables/bkreg.tab", oambase );
	return( path );
#else
	return( "/etc/bkup/bkreg.tab" );
#endif
}

char *
bk_get_bkoper_path()
{
#ifdef	DEBUG
	if( !*oambase ) (void) strcpy( oambase, br_get_oambase() );
	if( getenv( "OAMBASE" ) ) {
		(void) sprintf( path, "%s/bkrs/bin/bkoper", oambase );
		return( path );
	}
#endif
	return( "/usr/sbin/bkoper" );
}

/* get path to sed file for bkexcept translation option */
char *
bk_get_sedfile()
{
#ifdef DEBUG
	if( !*oambase ) (void) strcpy( oambase, br_get_oambase() );
	(void) sprintf( path, "%s/bin/exconv.sed", oambase );
	return( path );
#else
	return( "/usr/sadm/bkup/bin/exconv.sed" );
#endif
}

/* get path to restore strategy file */
char *
bk_get_strat_path()
{
#ifdef DEBUG
	if( !*oambase ) (void) strcpy( oambase, br_get_oambase() );
	(void) sprintf( path, "%s/bkrs/tables/rsstrat.tab", oambase );
	return( path );
#else
	return( "/etc/bkup/rsstrat.tab" );
#endif
}

/* get path to restore method description file */
char *
bk_get_rsmethod_path()
{
#ifdef DEBUG
	if( !*oambase ) (void) strcpy( oambase, br_get_oambase() );
	(void) sprintf( path, "%s/bkrs/tables/rsmethod.tab", oambase );
	return( path );
#else
	return( "/etc/bkup/rsmethod.tab" );
#endif
}

/* get path to rsoper */
char *
bk_get_rsoper_path()
{
#ifdef DEBUG
	if( !*oambase ) (void) strcpy( oambase, br_get_oambase() );
	if( getenv( "OAMBASE" ) ) {
		(void) sprintf( path, "%s/bkrs/bin/rsoper", oambase );
		return( path );
	}
#endif
	return( "/sbin/rsoper" );
}
