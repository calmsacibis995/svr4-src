/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bkdaemon.d/operator.c	1.5.2.1"

#include	<sys/types.h>
#include	<backup.h>
#include	<bkdaemon.h>
#include	<bkerrors.h>
#include	<bkmsgs.h>

extern	proc_t	*proctab;
extern	int	proctabsz;

extern pid_t jobidtopid();
extern void brlog();
extern int p_findproc();
extern char *typename();
extern char *msgname();
extern int md_findtag();
extern int md_send();
extern int lbl_insert();

static int op_forward();

/* See if this FAILED message is destined for a method, if so, forward it */
int
op_failed( msg )
bkdata_t *msg;
{
	return( op_forward( &(msg->failed.method_id), FAILED, msg ) );
}

/* See if this VOLUME message is destined for a method, is so forward it */
int
op_volume( msg )
bkdata_t *msg;
{
	return( op_forward( &(msg->volume.method_id), VOLUME, msg ) );
}

/* Forward a message on to a method */
static int
op_forward( method_id, type, msg )
method_id_t *method_id;
int type;
bkdata_t *msg;
{
	register pid_t pid;
	register p_slot, m_slot;
	register proc_t *proc;

	if( !(pid = jobidtopid( method_id->jobid )) ) {
		brlog( "op_forward(): given method id with bad jobid format: %s",
			method_id->jobid );
		return( BKBADID );
	}
	
	if( (p_slot = p_findproc( pid )) == -1 ) {
		brlog( "op_forward(): unable to find OWNER for job id %s",
			method_id->jobid );
		return( BKBADID );
	}
	if( !(proc = P_SLOT( p_slot )) ) {
		brlog( "op_forward(): p_findproc() returns bad slot number %d",
			p_slot );
		return( BKINTERNAL );
	}
	if( proc->type != OWNER_P ) {
		brlog(
		"op_forward(): jobid %s belongs to a %s process - ignore %s request",
			method_id->jobid, typename( proc->type ), msgname( type ) );
		return( BKBADID );
	}
	if( !(m_slot = md_findtag( proc->slot, method_id->tag )) ) {
		brlog(
		"op_forward(): Unable to find tag %s for o_slot %s - ignore %s request",
			method_id->tag, proc->slot, msgname( type ) );
		return( BKEXIST );
	}

	if( !md_send( m_slot, type, msg ) )
		return( BKERREXIT );

	/* Mark the label "used" */
	if( type == VOLUME )
		(void) lbl_insert( m_slot, msg->volume.label );

	return( BKSUCCESS );
}
