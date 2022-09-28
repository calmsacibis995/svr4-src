/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:libbrmeth.d/brpath.c	1.11.2.1"

#include	<sys/types.h>
#include	<stdio.h>
#include	<time.h>
#include	<backup.h>
#include	<restore.h>
#include	<rsstatus.h>
#include	<bkrs.h>
#include	<bkexcept.h>
#include	<errno.h>

extern char *strcpy();
extern int ascftime();
extern int mkdirp();

/* Where things live */
static char brtmp[ BKFNAME_SZ ];

#ifdef	DEBUG
char broambase[ BKFNAME_SZ ];

/* find out the base name of where the oam stuff resides */
char *
br_get_oambase( )
{
	char *ptr, *getenv();
	if( ptr = getenv( "OAMBASE" ) )
		return( ptr );
	else
		return( "/usr/oam" );
}
#endif

/* Get path name to the log file */
char *
brlogpath( type )
int type;
{
#ifdef DEBUG
	if( !*broambase ) (void) strcpy( broambase, br_get_oambase() );
	switch( type ) {
	case BACKUP_T:
		(void) sprintf( brtmp, "%s/bkrs/logs/%s", broambase, BKLOGFILE );
		break;
	case RESTORE_T:
		(void) sprintf( brtmp, "%s/bkrs/logs/%s", broambase, RSLOGFILE );
		break;
	default:
		(void) sprintf( brtmp, "%s/bkrs/logs/bkrs", broambase );
	}
	return( brtmp );
#else
	switch( type ) {
	case BACKUP_T:
		return( "/var/sadm/bkup/logs/bklog" );
	case RESTORE_T:
		return( "/var/sadm/bkup/logs/rslog" );
	default:
		return( "/var/sadm/bkup/logs/bkrs" );
	}
#endif
}

/* return path of the history table */
char *
br_get_histlog_path()
{
#ifdef DEBUG
	if( !*broambase ) (void) strcpy( broambase, br_get_oambase() );
	(void) sprintf( brtmp, "%s/bkrs/tables/%s", broambase, BK_HISTLOG );
	return( brtmp );
#else
	return( "/etc/bkup/bkhist.tab" );
#endif
}

char *
br_get_toc_path( jobid, pid )
char *jobid;
pid_t pid;
{
	char date[10];
	time_t clock;
	struct tm *t;

	(void) time(&clock);
	t = localtime(&clock);
	(void) ascftime(date, "%y%m%d", t);

#ifdef DEBUG
	if( !*broambase ) (void) strcpy( broambase, br_get_oambase() );
	(void) sprintf( brtmp, "%s/bkrs/tables/toc/%s/%s", broambase, date, jobid );
#else
	(void) sprintf( brtmp, "/var/sadm/bkup/toc/%s/%s", date, jobid );
#endif

	errno = EEXIST;
	if( mkdirp( brtmp, 0700 ) == -1 && errno != EEXIST ) return( (char *)0 );
	
	(void) sprintf( brtmp, "%s/p%ld", brtmp, pid );

	return( brtmp );
}

char *
br_get_except_path()
{
#ifdef DEBUG
	if( !*broambase ) (void) strcpy( broambase, br_get_oambase() );

	(void) sprintf( brtmp, "%s/bkrs/tables/%s", broambase, BK_EXTAB );

	return( brtmp );
#else
	return( "/etc/bkup/bkexcept.tab" );
#endif
}

/* return path of the restore request table */
char *
br_get_rsstatlog_path()
{
#ifdef DEBUG
	if( !*broambase ) (void) strcpy( broambase, br_get_oambase() );
	(void) sprintf( brtmp, "%s/bkrs/tables/%s", broambase, RS_STATLOG );
	return( brtmp );
#else
	return( "/etc/bkup/rsstatus.tab" );
#endif
}
