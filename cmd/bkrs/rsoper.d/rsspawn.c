/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:rsoper.d/rsspawn.c	1.1.2.1"

#include	<stdio.h>
#include	<sys/types.h>
#include	<errno.h>
#include	<bkrs.h>
#include	<backup.h>
#include	<restore.h>
#include	<rsoper.h>
#include	<errors.h>

#define	MAXARGS	512

rsspawn( list, oname, odevice, ddev, dchar, dmnames, method, flags, toc )
rs_entry_t *list;
char *oname, *odevice, *ddev, *dchar, *dmnames, *method;
int flags, toc;
{
	register i = 1, partial;
	char *argv[ MAXARGS ], *ddevice, *item();

	/* Pre-pend a ':' to ddev */
	if( !(ddevice = (char *)malloc( strlen( ddev )
		+ strlen( dchar ) + strlen( dmnames ) + 4 ) ) ) {
		bkerror( stderr, ERROR19, method, brerrno( ENOMEM ) );
		return( 0 );
	}
	sprintf( ddevice, ":%s:%s:%s", ddev, dchar, dmnames );

	if( toc ) {

		/* Use incfile F to restore all TOC files */
		argv[ 0 ] = (char *)bk_get_method_path( TOC_METHOD );

		/* restore complete table of contents */
		argv[ i++ ] = "-RC";

		if( flags & sFLAG ) argv[ i++ ] = "-S";
		else if( flags & vFLAG ) argv[ i++ ] = "-V";

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
		partial = !strcmp(list->type, (unsigned char *)R_FILE_TYPE )
			|| !strcmp( list->type, (unsigned char *)R_DIRECTORY_TYPE );
		argv[ i++ ] = partial? "-RF": "-RC";

		if( flags & sFLAG ) argv[ i++ ] = "-S";
		else if( flags & vFLAG ) argv[ i++ ] = "-V";

		argv[ i++ ] = (oname? oname: "");
		argv[ i++ ] = (odevice? odevice: "");
		argv[ i++ ] = ddevice;

		if( partial ) {
			while( list && i < MAXARGS - 2 ) {
				argv[ i++ ] = item( list );
				list = list->next;
			}

		} else {
			/* a FULL restore request */
			argv[ i++ ] = (char *)(list->refsname? list->refsname: (unsigned char *)"");
			argv[ i++ ] = (char *)(list->redev? list->redev: (unsigned char *)"");
			argv[ i++ ] = (char *)list->jobid;
		}

		argv[ i++ ] = (char *)0;
	}

	if( bkspawn( argv[0], "-", "-", "-", 0, 0, BKARGV, argv ) == -1 ) {
		bkerror( stderr, ERROR19, method, brerrno( errno ) );
		return( 0 );
	}

	return( 1 );
}

/*
	Build a "rsjobid:uid:date:type:name:rename:inode" character string
	from a rs_entry_t structure;
*/
char *
item( entry )
rs_entry_t *entry;
{
	char *buffer;
	register size;

	size = strlen( entry->jobid ) + strlen( entry->object ) 
		+ strlen( entry->type );

	if( entry->target ) size += strlen( entry->target );

	size += 5 /* UID */ + 15 /* DATE */ + 10 /* INODE */ + 6 /* :'s */;

	if( !(buffer = (char *)malloc( size + 1 )) )
		return( (char *)0 );

	if( entry->target ) {

		if( entry->inode ) {

			sprintf( buffer, "%s:%d:0x%x:%s:%s:%s:%d", entry->jobid, entry->uid,
				entry->fdate, entry->type, entry->object, entry->target,
				entry->inode );

		} else sprintf( buffer, "%s:%d:0x%x:%s:%s:%s", entry->jobid, entry->uid,
			entry->fdate, entry->type, entry->object, entry->target );

	} else if( entry->inode ) {

		sprintf( buffer, "%s:%d:0x%x:%s:%s::%d", entry->jobid, entry->uid,
			entry->fdate, entry->type, entry->object, entry->inode );

	} else sprintf( buffer, "%s:%d:0x%x:%s:%s", entry->jobid, entry->uid,
		entry->fdate, entry->type, entry->object );

	return( buffer );
}
