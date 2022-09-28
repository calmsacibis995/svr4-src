/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bkdaemon.d/events.c	1.4.2.1"

#include	<sys/types.h>
#include	<backup.h>
#include	<bkdaemon.h>
#include	<bkmsgs.h>
#include	<bkerrors.h>

extern method_t	*methodtab;
extern int	methodtabsz;

extern void *malloc();
extern void free();
extern void incoming();
extern void brlog();
extern int ev_schedule();
extern int q_inject();
extern int md_terminate();
extern void q_mdactivate();

static void ev_incoming();

void
ev_msg( originator, type, data, seconds )
pid_t originator;
int type, seconds;
bkdata_t *data;
{
	queued_msg_t *msg;

	if( !(msg = (queued_msg_t *)malloc( sizeof( queued_msg_t ))) ) {
		brlog( "ev_msg(): unable to malloc memory for retry of msg type %d from pid %ld",
			type, originator );
		return;
	}

	msg->data = *data;
	msg->originator = originator;
	msg->type = type;

#ifdef TRACE
	brlog( "ev_msg(): reschedule message type %d from pid %ld", 
		type, originator );
#endif
	(void) ev_schedule( ev_incoming, (long)msg, seconds );
}

static void
ev_incoming( msg )
queued_msg_t *msg;
{

#ifdef TRACE
	brlog( "ev_incoming() event" );
#endif

	incoming( msg );
	free( msg );
}

void
ev_resched( m_slot )
int m_slot;
{
	register rc;
	register method_t *method;
	register dep_t *dep, *t_dep;

#ifdef TRACE
	brlog( "ev_resched(): reschedule m_slot %d's waiting methods", m_slot );
#endif
	
	if( !(method = MD_SLOT( m_slot )) ) {
		brlog( "ev_resched(): given bad m_slot %d - ignored" );
		return;
	}

	for( dep = method->m_waiting; dep; dep = t_dep ) {

		t_dep = dep->next;

		if( (rc = q_inject( dep->m_slot )) != BKSUCCESS ) {

			brlog( "ev_resched(): unable to put m_slot %d back on runqueue",
				dep->m_slot );
			(void) md_terminate( dep->m_slot, MDT_FAIL, rc,
				"Unable to reschedule method", 0 );

		} else
			q_mdactivate( dep->m_slot );

		free( dep );

	}

	method->m_waiting = 0;
}
