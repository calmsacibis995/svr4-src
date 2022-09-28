/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:libbrmeth.d/brhistory.c	1.6.2.1"

#include	<sys/types.h>
#include	<time.h>
#include	<bkrs.h>
#include	<backup.h>
#include	<bkmsgs.h>
#include	<errno.h>

extern int brtype;
extern char *brcmdname;
extern pid_t bkdaemonpid;

extern int bkm_send();
extern void brlog();
extern char *strncpy();
extern unsigned int strlen();
extern char *strcpy();

static int fill_buffer();

/*
	This routine sends a message to bkdaemon to modify the history table.
*/
int
brhistory( oname, odevice, time, size, volumes, nvolumes, flags, tocname )
char *oname, *odevice, *volumes[], *tocname;
int size, nvolumes, flags;
time_t time;
{
	bkdata_t msg;
	int count = 0;

	if( brtype != BACKUP_T ) return( 0 );

#ifdef TRACE
	brlog( "brhistory(): oname %s odevice %s time 0x%x size %d flags 0x%x",
		oname, odevice, time, size, flags );
#endif

	(void) strncpy( (char *) &msg, "", sizeof( bkdata_t ) );

	if( tocname )
		if( strlen( tocname ) > BKFNAME_SZ ) {
			brlog( "Unable to write history table - TOC filename too long." );
			return( 0 );
		} else
			(void) strcpy( msg.history.tocname, tocname );

	if( oname ) 
		if( strlen( oname ) > BKDEVICE_SZ ) {
			brlog( "Unable to write history table - oname: %s is too long", oname );
			return( 0 );
		} else
			(void) strcpy( msg.history.oname, oname );

	if( odevice )
		if( strlen( odevice ) > BKDEVICE_SZ ) {
			brlog( "Unable to write history table - odevice: %s is too long", odevice );
			return( 0 );
		} else
			(void) strcpy( msg.history.odevice, odevice );

	msg.history.nvolumes = nvolumes;
	msg.history.size = size;
	msg.history.time = time;

	if( flags & BR_ARCHIVE_TOC ) msg.history.flags |= HST_DO_ARCHIVE_TOC;
	if( flags & BR_IS_OLD_ENTRY ) msg.history.flags |= HST_MODIFY;
	if( flags & BR_IS_TMNAMES ) msg.history.flags |= HST_IS_TOC;

	while( fill_buffer( msg.history.labels, BKTEXT_SZ + 1,
		volumes, nvolumes, &count ) ) {
		if( bkm_send( bkdaemonpid, HISTORY, &msg ) == -1 ) {
			brlog( "Unable to send HISTORY message to bkdaemon; errno %d", errno );
			return( 0 );
		}
		msg.history.flags |= HST_CONTINUE;
	}
	return( 1 );
}

static int
fill_buffer( buffer, buffersz, volumes, nvolumes, count )
char *buffer, *volumes[];
int buffersz, nvolumes, *count;
{
	register bsize = 0, sz;
	register char *bptr = buffer;

#ifdef TRACE
	brlog( "fill_buffer(): count: %d nvolumes %d buffersz %d",
		*count, nvolumes, buffersz );
#endif

	if( *count >= nvolumes ) return( 0 );

	for( ; *count < nvolumes; ) {
		sz = strlen( volumes[ *count ] );
		if( sz > buffersz ) {
			brlog( "Unable to update HISTORY: volume name %s is too big",
				volumes[ *count ] );
			return( 0 );
		}

		if( bsize + sz + 1 > buffersz ) {
			/* This buffer is full */
			*(bptr - 1) = '\0';
			return( 1 );
		}

		(void) strcpy( bptr, volumes[ *count ] );
		bptr += sz;
		*bptr++ = ',';
		(*count)++;
	} 
	*(bptr - 1) = '\0';
	return( 1 );
}
