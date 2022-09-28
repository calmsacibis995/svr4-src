/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:rslib.d/rsspawn.c	1.6.2.1"

#include	<stdio.h>
#include	<signal.h>
#include	<sys/types.h>
#include	<errno.h>
#include	<bkrs.h>
#include	<backup.h>
#include	<bkmsgs.h>
#include	<restore.h>
#include	<rsstatus.h>
#include	<bkerrors.h>

#define	MAXARGS	512

extern char *d_resolve();
extern void *malloc();
extern unsigned int strlen();
extern void free();
extern char *bk_get_method_path();
extern int strcmp();
extern pid_t getpid();
extern int bkm_start();
extern int bkspawn();
extern pid_t wait();
extern int bkm_exit();
extern int bkm_receive();
extern void brlog();
extern void in_enqueue();
extern int in_dequeue();
extern char *strcpy();
extern char *strdup();

static void incoming();
static void new_message();
static char *item();

int bklevels;

int
rsspawn( list, oname, odevice, ddev, dchar, dmnames, method, flags, tocname )
rs_entry_t *list;
char *oname, *odevice, *ddev, *dchar, *dmnames, *method, *tocname;
int flags;
{
	register i = 1, partial, j = 0, rc = BKSUCCESS;
	char *argv[ MAXARGS ], *freelist[ MAXARGS ], *ddevice, name[ 30 ];
	int status;
	rs_entry_t *save = list;

	/* resolve dchar vs. dvmgt dchar */
	dchar = d_resolve( ddev, dchar );

	/* Pre-pend a ':' to ddev */
	if( !(ddevice = (char *)malloc( (unsigned int) strlen( ddev )
		+ (unsigned int) strlen( dchar ) + (unsigned int) strlen( dmnames )
		+ 4 ) ) ) {
		free( dchar );
		return( BKNOMEMORY );
	}
	(void) sprintf( ddevice, ":%s:%s:%s", ddev, dchar, dmnames );

	if( flags & RS_ISTOC ) {

		/* Use incfile F to restore all TOC files */
		argv[ 0 ] = (char *)bk_get_method_path( TOC_METHOD );

		/* restore complete table of contents */
		argv[ i++ ] = "-RCT";

		if( flags & RS_SFLAG ) argv[ i++ ] = "-S";
		else if( flags & RS_VFLAG ) argv[ i++ ] = "-V";

		argv[ i++ ] = (oname? oname: "");
		argv[ i++ ] = (odevice? odevice: "");
		argv[ i++ ] = ddevice;
		argv[ i++ ] = "";	/* refsname */
		argv[ i++ ] = "";	/* redev */
		argv[ i++ ] = ""; /* XXX - rsjobid? */
		argv[ i++ ] = (char *)0;

	} else {
	
		argv[ 0 ] = (char *)bk_get_method_path( method );

		/* File and directories map to -F */
		partial = !strcmp( (char *)list->type, (char *)R_FILE_TYPE )
			|| !strcmp( (char *) list->type, (char *)R_DIRECTORY_TYPE );
		argv[ i++ ] = partial? "-RF": "-RC";

		if( flags & RS_SFLAG ) argv[ i++ ] = "-S";
		else if( flags & RS_VFLAG ) argv[ i++ ] = "-V";

		argv[ i++ ] = (oname? oname: "");
		argv[ i++ ] = (odevice? odevice: "");
		argv[ i++ ] = ddevice;

		if( partial ) {
			while( list && i < MAXARGS - 2 ) {
				freelist[ j++ ] = argv[ i++ ] = item( list );
				list = list->next;
			}

		} else {
			/* a FULL restore request */
			argv[ i++ ] = (char *)(list->refsname? list->refsname: (unsigned char *)"");
			argv[ i++ ] = (char *)(list->redev? list->redev: (unsigned char *)"");
			argv[ i++ ] = (char *)list->jobid;
		}

		freelist[ j++ ] = argv[ i++ ] = (char *)0;
	}

	(void) sprintf( name, "RSQ%ld", getpid() );
	if( bkm_start( name, FALSE ) == -1 ) {
		/* Start up parent, anyway */
		rc = BKNOTSPAWNED;
		goto out;
	} 

	(void) sigset( SIGUSR1, (void (*)())new_message );

	if( bkspawn( argv[0], "-", "-", "-", 0, 0, BKARGV, argv ) == -1 ) {
		rc = BKNOTSPAWNED;
		goto out;
	}

	/* Wait for methods to get done */
	while( wait( &status ) == -1 && (status & 0177) == 0177 ) ;

	while( TRUE ) {
		if( wait( &status ) == -1 ) {
			if( status & 0177 )
				continue;
			if( errno == EINTR )
				incoming( save, tocname );
			else if( errno == ECHILD )
				break;
		}
	}

	incoming( save, tocname );

out:
	/* free stuff up */
	for( j = 0; freelist[ j ]; j++ )
		free( freelist[ j ] );

	free( dchar );
	free( ddevice );

	bkm_exit( name );

	return( rc );
}

/*
	Build a "rsjobid:uid:date:type:name:rename:inode" character string
	from a rs_entry_t structure;
*/
static char *
item( entry )
rs_entry_t *entry;
{
	char *buffer;
	register size;

	size = strlen( (char *) entry->jobid ) + strlen( (char *) entry->object ) 
		+ strlen( (char *) entry->type );

	if( entry->target ) size += strlen( (char *) entry->target );

	size += 10 /* UID */ + 15 /* DATE */ + 10 /* INODE */ + 6 /* :'s */;

	if( !(buffer = (char *)malloc( (unsigned int) size + 1 )) )
		return( (char *)0 );

	if( entry->target ) {

		if( entry->inode ) {

			(void) sprintf( buffer, "%s:%ld:0x%lx:%s:%s:%s:%ld", entry->jobid, entry->uid,
				(int)entry->fdate, entry->type, entry->object, entry->target,
				entry->inode );

		} else (void) sprintf( buffer, "%s:%ld:0x%lx:%s:%s:%s", entry->jobid, entry->uid,
			(int)entry->fdate, entry->type, entry->object, entry->target );

	} else if( entry->inode ) {

		(void) sprintf( buffer, "%s:%ld:0x%lx:%s:%s::%ld", entry->jobid, entry->uid,
			(int)entry->fdate, entry->type, entry->object, entry->inode );

	} else (void) sprintf( buffer, "%s:%ld:0x%lx:%s:%s", entry->jobid, entry->uid,
		(int)entry->fdate, entry->type, entry->object );

	return( buffer );
}

/*VARARGS*/
static void
new_message( sig )
int sig;
{
	register more = TRUE;
	int type, orig;
	bkdata_t data;
	
	BEGIN_SIGNAL_HANDLER;

#ifdef TRACE
	brlog( "Signal %d arrived", sig );
#endif

	while( more ) {
		if( bkm_receive( &orig, &type, &data ) == -1 ) {
			more = FALSE;
			if( errno == EINTR )
				brlog( "no message received yet" );
			else if( errno != ENOMSG )
				brlog( "bkm_receive returns -1; errno %ld", errno );
#ifdef BOZO
			else brlog( "new_message(): no message received." );
#endif
		} else in_enqueue( type, &data );
	}
	END_SIGNAL_HANDLER;
}
/*ARGSUSED*/

static void
incoming( list, tocname )
rs_entry_t *list;
char *tocname;
{
	int type;
	bkdata_t data;
	rs_entry_t *lptr;

	while( in_dequeue( &type, &data ) ) {
			
#ifdef TRACE
		brlog( "incoming(): got a %d msg.", type );
#endif

		switch( type ) {
		case RSTOC:
			if( tocname )
				(void) strcpy( tocname, data.rst_o_c.tocname );
			break;

		case RSRESULT:
			if( !tocname )
				/* If this is a table of contents volume - ignore RESULT msgs */
				break;

			for( lptr = list; lptr; lptr = lptr->next ) {
				if( !strcmp( data.rsret.jobid, (char *) lptr->jobid ) ) {
					if( data.rsret.retcode == BRSUCCESS )
						lptr->status = 
							(unsigned char *)strdup( (char *) RST_SUCCESS );
					else {
						lptr->status = 
							(unsigned char *)strdup( (char *) RST_FAILED );
						lptr->explanation = 
							(unsigned char *)strdup( data.rsret.errmsg );
					}
					break;
				}
			}
			break;

		default:
			/* Ignore 'unknown' messages */
			brlog( "incoming(): received an 'unknown' message - ignored" );
			break;
		}
	}
}
