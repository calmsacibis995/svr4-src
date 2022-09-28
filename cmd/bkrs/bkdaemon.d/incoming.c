/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bkdaemon.d/incoming.c	1.10.2.1"

#include	<sys/types.h>
#include	<backup.h>
#include	<bkmsgs.h>
#include	<bkdaemon.h>
#include	<bkstatus.h>
#include	<bkerrors.h>

extern void hst_invalidate();
extern int p_findproc();
extern void brlog();
extern char *msgname();
extern void m_send_pid_failed();
extern int c_mcheck();
extern int m_send();
extern void ev_msg();
extern int c_malloc();
extern void c_free();
extern int c_schedule();
extern void m_send_done();
extern void c_terminate();
extern int c_broadcast();
extern char *typename();
extern int md_flagset();
extern void o_terminate();
extern int o_send();
extern int md_terminate();
extern char *strcpy();
extern void o_kill();
extern int op_failed();
extern int sprintf();
extern void st_set();
extern int lbl_insert();
extern int send_mail_waiting();
extern int hst_update();
extern int c_response();
extern int f_exists();
extern int o_malloc();
extern void o_free();
extern int md_schedule();
extern void m_pr_methods();
extern int q_md_merge();
extern int q_run();
extern int op_volume();

extern proc_t	*proctab;
extern int	proctabsz;
extern owner_t	*ownertab;
extern int	ownertabsz;
extern control_t	*controltab;
extern int	controltabsz;
extern method_t	*methodtab;
extern int	methodtabsz;
extern int state;
extern int ntalking;

static
void
do_control_m( originator, type, msg )
pid_t originator;
int type;
control_m *msg;
{
	register p_slot, breaksent = FALSE, rc, c_slot;
	register proc_t *proc;
	register control_t *control = 0;

	/*
		If bkdaemon already knows about this originator, then the only
		allowable case is a CANCEL message from an OWNER.
	*/
	if( (p_slot = p_findproc( originator )) != -1 ) {
		if( !(proc = P_SLOT( p_slot )) ) {
			brlog( "do_control_m(): p_findproc() returns bad slot number %d",
				p_slot );
			return;
		}

		breaksent = ((type == CANCEL) && (proc->type == OWNER_P));
		if( !breaksent ) {
			brlog(
				"do_control_m(): received %s message from process %ld",
				msgname( type ), originator );
			m_send_pid_failed( originator, NULL, BKBADMESSAGE, 0 );
			return;
		}
	}

#ifdef TRACE
	brlog( "do_control_m: flags: 0x%x uid: %ld pid: %ld", 
		msg->flags, msg->uid, msg->pid );
#endif

	/* Verify consistency of control message */
	switch( c_mcheck( ((msg->flags & CTL_UID)? msg->uid: -1),
		((msg->flags & CTL_PID)? msg->pid: -1) ) ) {

	case BKEXIST:
	case BKNONE:
		m_send_pid_failed( originator, NULL, BKEXIST, "No backups to control" );
#ifdef TRACE
		brlog( "do_control_m(): no methods to control (c_mcheck())" );
#endif
		if( !breaksent ) (void) m_send( originator, DONE, (char *)NULL );
		return;
			
	case BKBUSY:
		/* Reschedule this message for later */
#ifdef TRACE
		brlog( "do_control_m(): c_mcheck() returns BKBUSY" );
#endif
		ev_msg( originator, type, (bkdata_t *)msg, 5 );
		return;

	case BKINTERNAL:
#ifdef TRACE
		brlog( "do_control_m(): c_mcheck() returns BKINTERNAL" );
#endif
		m_send_pid_failed( originator, NULL, BKINTERNAL, NULL );
		if( !breaksent ) (void) m_send( originator, DONE, (char *)NULL );
		return;
	}

	/* Set up new CONTROL process structure */
	if( (c_slot = c_malloc()) == -1 ) {
		brlog( "do_control_m(): Unable to allocate CONTROL slot" );
		m_send_pid_failed( originator, NULL, BKNOMEMORY,
			"Unable to malloc memory" );
		if( !breaksent ) (void) m_send( originator, DONE, (char *)NULL );
		return;
	}
	if( !(control = C_SLOT( c_slot )) ) {
		brlog( "do_control_m(): c_malloc returns bad c_slot", c_slot );
		m_send_pid_failed( originator, NULL, BKINTERNAL, "Internal error" );
		if( !breaksent ) (void) m_send( originator, DONE, (char *)NULL );
		c_free( c_slot );
		return;
	}

	if( !(proc = P_SLOT( control->p_slot )) ) {
		brlog( "do_control_m(): c_slot %d has bad p_slot %d", c_slot, p_slot );
		m_send_pid_failed( originator, NULL, BKINTERNAL, "Internal error" );
		if( !breaksent ) (void) m_send( originator, DONE, (char *)NULL );
		c_free( c_slot );
		return;
	}

	/* Fill in Control Structure */
	control->pid = (msg->flags & CTL_PID)? msg->pid: -1;
	control->uid = (msg->flags & CTL_UID)? msg->uid: -1;
	control->type = type;
	control->o_slot = (breaksent? proc->slot: 0);

	proc->pid = originator;

	if( rc = c_schedule( c_slot ) ) {
		if( rc != BKNONE )
			brlog(
				"do_control_m(): Unable to schedule CONTROL messages for backup pid %ld",
				originator );
		else brlog( "do_control_m(): No backups to CONTROL (c_schedule())" );
		if( !breaksent)
			m_send_done( control->p_slot );
		c_terminate( c_slot );
		return;
	}
	if( rc = c_broadcast( c_slot ) ) {
		if( rc != BKNONE )
			brlog( 
				"do_control_m(): Unable to CONTROL messages for backup pid %ld",
				originator );
		else brlog( "do_control_m(): No backups to CONTROL (c_broadcast())" );

		if( !breaksent ) 
			m_send_done( control->p_slot );

		c_terminate( c_slot );
		return;
	}
}

static
void
do_disconnected_m( originator )
pid_t originator;
{
	register p_slot;
	register proc_t *proc;
	register owner_t *owner;

	if( (p_slot = p_findproc( originator )) == -1 ) {
		brlog(
			"do_disconnected_m(): received DISCONNECTED message from unknown process pid %ld",
			originator );
		return;
	}
	if( !(proc = P_SLOT( p_slot )) ) {
		brlog( "do_disconnected_m(): p_findproc() returns bad slot number %d",
			p_slot );
		return;
	}
	if( proc->type != OWNER_P ) 
		brlog( "do_disconnected_m(): received a DISCONNECTED message from process type %s",
			typename( proc->type ) );
	else if( !(owner = O_SLOT( proc->slot )) )
		brlog( "do_disconnected_m(): p_slot %d has bad o_slot %d",
			p_slot, proc->slot );
	else {
		owner->state &= ~O_TALKING;
		if( !(--ntalking) )
			state &= ~TALKING;
	}

	if( md_alldone( proc->slot ) ) {
#ifdef TRACE
		brlog( "do_disconnect_m(): ALL DONE - terminate o_slot %d",
			proc->slot );
#endif
		o_terminate( proc->slot );
	}
}

static
void
do_dot_m( originator )
int originator;
{
	register p_slot;
	register proc_t *proc;
	register method_t *method;

	if( (p_slot = p_findproc( originator )) == -1 ) {
		brlog(
			"do_dot_m(): received DOT message from unknown process pid %ld",
			originator );
		return;
	}
	if( !(proc = P_SLOT( p_slot )) ) {
		brlog( "do_dot_m(): p_findproc() returns bad slot number %d",
			p_slot );
		return;
	}
	if( proc->type != METHOD_P ) 
		brlog( "do_dot_m(): received a DOT message from process type %s",
			typename( proc->type ) );
	else if( !(method = MD_SLOT( proc->slot )) ) 
		brlog( "do_dot_m(): p_slot %d points to bad m_slot %d",
			p_slot, proc->slot );
	else if( o_send( method->o_slot, DOT, (bkdata_t *)0 ) != BKSUCCESS )
		brlog( "do_dot_m(): unable to send DOT msg to o_slot %d",
			method->o_slot );
}

static
void
do_done_m( originator, done )
pid_t originator;
done_m *done;
{
	register p_slot;
	register proc_t *proc;

	if( (p_slot = p_findproc( originator )) == -1 ) {
		brlog(
			"do_done_m(): received DONE message from unknown process pid %ld",
			originator );
		return;
	}

	if( !(proc = P_SLOT( p_slot )) ) {
		brlog( "do_done_m(): p_findproc() returns bad slot number %d",
			p_slot );
		return;
	}

	if( proc->type != METHOD_P ) 
		brlog( "do_done_m(): received a DONE message from process type %s",
			typename( proc->type ) );
	else (void) md_terminate( proc->slot, MDT_DONE, 0, 0, done->nblocks );
}

static
void
do_estimate_m( originator, msg )
pid_t originator;
bkdata_t	*msg;
{
	register p_slot;
	register proc_t *proc;
	register owner_t *owner;
	register method_t *method;

#ifdef	TRACE
	brlog( "ESTIMATE of %d volumes, %d blocks.", msg->estimate.volumes,
		msg->estimate.blocks );
#endif

	if( (p_slot = p_findproc( originator )) == -1 ) {
		brlog( "do_estimate_m(): received ESTIMATE msg from unknown process pid %ld - ignored",
			originator );
		return;
	}
	if( !(proc = P_SLOT( p_slot )) ) {
		brlog( "do_estiamte_m(): p_findproc() returns bad slot number %d",
			p_slot );
		return;
	}

	if( proc->type != METHOD_P ) {
		brlog(
			"do_estimate_m(): received a ESTIMATE message from process type %s",
			typename( proc->type ) );
		return;
	}

	/* Send estimate to OWNING backup */
	if( !(method = MD_SLOT( proc->slot )) ) {
		brlog( "do_estimate_m(): p_slot %d points to bad m_slot %d",
			p_slot, proc->slot );
		return;
	}
	if( !(owner = O_SLOT( method->o_slot ) ) ) {
		brlog( "do_estimate_m(): m_slot %d has bad o_slot %d", proc->slot,
			method->o_slot );
		return;
	}

	/* Record estimate in method structure and add tag name to message */
	method->e_volumes = msg->estimate.volumes;
	method->e_blocks = msg->estimate.blocks;

	/*  don't send if NOEXECUTE */
	if( !(owner->options & S_NO_EXECUTE) ) {
		(void) strcpy( msg->estimate.method_id.tag, (char *)method->entry.tag );

		/* Forward message to owner */
		if( o_send( method->o_slot, ESTIMATE, msg ) ) {
			o_kill( method->o_slot );
		}
	}
}

static
void
do_failed_m( originator, msg )
pid_t originator;
bkdata_t	*msg;
{
	register p_slot;
	register proc_t *proc;
	if( (p_slot = p_findproc( originator )) == -1 ) {
		/* see if this may be a FAILED message for a GET_VOLUME request */
		if( op_failed( msg ) ) 
			brlog(
				"do_failed_m(): unable to forward FAILED msg from pid %ld - ignored",
				originator );
		return;
	}
	if( !(proc = P_SLOT( p_slot )) ) {
		brlog( "do_failed_m(): orig %ld has bad slot number %d",
			originator, p_slot );
		return;
	}
	if( proc->type != METHOD_P ) {
		brlog(
			"do_failed_m(): FAILED message from process type %s - ignored",
			typename( proc->type ) );
		return;
	}
	(void) md_terminate( proc->slot, MDT_FAIL, msg->failed.reason,
		msg->failed.errmsg, 0 );
}

static 
void
do_get_volume_m( originator, msg )
pid_t originator;
getvolume_m *msg;
{
	register p_slot;
	register proc_t *proc;
	register method_t *method;
	register owner_t *owner;

	if( (p_slot = p_findproc( originator )) != -1 ) {
		if( !(proc = P_SLOT( p_slot )) ) {
			brlog( "do_get_volume_m(): orig %ld has bad slot number %d",
				originator, p_slot );
			return;
		}
	} else {
		brlog(
		"do_get_volume_m(): unexpected GET_VOLUME msg from pid %ld - ignored",
			originator );
		return;
	}
	if( !proc->type == METHOD_P ) {
		brlog(
			"do_get_volume_m(): GET_VOLUME msg from process type %s - ignored",
			typename( proc->type ) );
		return;
	}
	if( !(method = MD_SLOT( proc->slot )) ) {
		brlog( "do_get_volume_m(): p_slot %d has bad m_slot %d",
			p_slot, proc->slot );
		return;
	}
	if( !(owner = O_SLOT( method->o_slot )) ) {
		brlog( "do_get_volume_m(): m_slot %d has bad o_slot %d",
			proc->slot, method->o_slot );
		return;
	}

#ifdef TRACE
	brlog( "do_get_volume(): o_slot %d m_slot %d volume %s flags 0x%x",
		method->o_slot, proc->slot, msg->label, msg->flags );
#endif

	if( msg->flags & GV_OVERRIDE ) {

		/* Tell bkoper that override is allowed */
		char buffer[ BKLABEL_SZ + 3 ];
		(void) sprintf( buffer, "%s,O", msg->label );
		st_setwaiting( proc->slot, (unsigned char *)buffer );

	} else st_setwaiting( proc->slot, (unsigned char *)msg->label );

	/* Mark this label "used" */
	(void) lbl_insert( proc->slot, msg->label );

	if( owner->options & S_INTERACTIVE ) {
		if( owner->state & O_TALKING )
			return;

		/* interactive backup - forward get_volume message */
		if( o_send( method->o_slot, GET_VOLUME, (bkdata_t *)msg ) ) {
			o_kill( method->o_slot );
			return;
		}
		owner->state |= O_TALKING;
		state |= TALKING;
		ntalking++;
	} else {
		/* Not an interactive backup - mail message to owner */
		if( send_mail_waiting( proc->slot, msg->label ) ) {
			o_kill( method->o_slot );
			return;
		}
	}
}

static
void
do_history_m( originator, msg )
pid_t originator;
history_m *msg;
{
	register proc_t *proc;
	register p_slot;

	if( (p_slot = p_findproc( originator )) != -1 ) {
		if( !(proc = P_SLOT( p_slot )) ) {
			brlog( "do_history_m(): orig %ld has bad slot number %d",
				originator, p_slot );
			return;
		}
	}
	if( p_slot == -1 || proc->type != METHOD_P ) {
		brlog( "do_history_m(): received HISTORY message from orig %ld - ignored",
			originator );
		return;
	}
	(void) hst_update( proc->slot, msg );
}

static
void
do_inval_m( originator, msg )
pid_t originator;
inv_lbls_m *msg;
{
	register proc_t *proc;
	register p_slot;
	if( (p_slot = p_findproc( originator )) != -1 ) {
		if( !(proc = P_SLOT( p_slot )) ) {
			brlog( "do_inval_m(): orig %ld has bad slot number %d",
				originator, p_slot );
			return;
		}
	}
	if( p_slot == -1 || proc->type != METHOD_P ) {
		brlog( "do_inval_m(): received INVALIDATE_LBLS message from orig %ld - ignored",
			originator );
		return;
	}
	hst_invalidate( proc->slot, msg->label );
}

static
void
do_response_m( originator, type )
pid_t originator;
int type;
{
	register proc_t *proc;
	register p_slot;
	if( (p_slot = p_findproc( originator )) != -1 ) {
		if( !(proc = P_SLOT( p_slot )) ) {
			brlog( "do_response_m(): orig %ld has bad slot number %d",
				originator, p_slot );
			return;
		}
	}
	if( p_slot == -1 || proc->type != METHOD_P ) {
		brlog( "do_response_m(): received message type %d from orig %ld - ignored",
			type, originator );
		return;
	}
	(void) c_response( proc->slot, type, FALSE );
}

static
void
do_start_m( originator, msg )
pid_t originator;
start_m	*msg;
{
	register o_slot;
	register owner_t *owner;
	register proc_t *proc;

#ifdef TRACE
	brlog(
	"do_start_m(): fname %s table %s uid %ld gid %ld options 0x%x week %d day %d",
		msg->fname, msg->table, msg->my_uid, msg->my_gid, msg->options, 
		msg->week, msg->day);
#endif

	/* START messages should *never* come from processes
		already in the proc table */
	if( P_EXISTS( originator ) ) {
		brlog(
			"Received START message from an existing process; pid %ld - ignored",
			originator );
		return;
	}

	/* validity checks e.g. table, fname */
	if( *(msg->fname) && !f_exists( msg->fname ) )
		m_send_pid_failed( originator, NULL, BKNOFILE, msg->fname );

	if( *(msg->table) && !f_exists( msg->table ) )
		m_send_pid_failed( originator, NULL, BKNOFILE, msg->table );

	/* Allocate an OWNER and PROC slot */
	if( (o_slot = o_malloc()) == -1 ) {
		brlog( "Unable to allocate OWNER slot" );
		return;
	}
	if( !(owner = O_SLOT( o_slot ) ) ) {
		brlog( "do_start_m(): o_findslot returned a bad owner slot: %d",
			o_slot );
		o_free( o_slot );
		return;
	}
	if( !(proc = P_SLOT( owner->p_slot ) ) ) {
		brlog( "do_start_m(): o_findslot returned a bad process slot: %d",
			owner->p_slot );
		o_free( o_slot );
		return;
	}

	/* Fill in the PROC and OWNER structures */
	proc->uid = msg->my_uid;
	proc->gid = msg->my_gid;
	proc->pid = originator;

	owner->options = msg->options;
	(void) strcpy( owner->table, msg->table );
	(void) strcpy( owner->owner, msg->user );
	(void) strcpy( owner->fname, msg->fname );
	owner->week = msg->week;
	owner->day = msg->day;

#ifdef	TRACE
	brlog( "do_start_m(): o_slot: %d owner: 0x%x table: %s",
		o_slot, owner,  owner->table );
#endif

	/* Set up the schedule of methods */
	if( md_schedule( o_slot ) ) {
		brlog( "do_start_m(): Unable to schedule methods for backup pid %ld",
			originator );
		m_send_done( owner->p_slot );
		o_free( o_slot );
		return;
	}

	/* If NO_EXECUTE and no ESTIMATE, then only the order of the methods is
		important - the methods do not have to be started */
	if( ( owner->options & S_NO_EXECUTE )
		&& !( owner->options & S_ESTIMATE ) ) {

		/* PRINT out order of methods for owner */
		m_pr_methods( o_slot, FALSE );
		m_send_done( owner->p_slot );
		o_free( o_slot );

	} else if( !owner->methods ) {

		/* Nothing to do right now */
		m_send_done( owner->p_slot );
		o_free( o_slot );

	} else {

		/* Set start time and merge the local schedule with the global one */
		(void) q_md_merge( o_slot );
		(void) q_run();

	}
}

static
void
do_text_m( originator, msg )
pid_t originator;
bkdata_t *msg;
{
	register p_slot;
	register proc_t *proc;
	register method_t *method;

	if( (p_slot = p_findproc( originator )) == -1 ) {
		brlog(
			"do_text_m(): received TEXT message from unknown process pid %ld",
			originator );
		return;
	}
	if( !(proc = P_SLOT( p_slot )) ) {
		brlog( "do_text_m(): p_findproc() returns bad slot number %d",
			p_slot );
		return;
	}
	if( proc->type != METHOD_P ) 
		brlog( "do_text_m(): received a TEXT message from process type %s",
			typename( proc->type ) );
	else if( !(method = MD_SLOT( proc->slot )) ) 
		brlog( "do_text_m(): p_slot %d points to bad m_slot %d",
			p_slot, proc->slot );
	else if( o_send( method->o_slot, TEXT, msg ) != BKSUCCESS )
		brlog( "do_text_m(): unable to send TEXT msg to o_slot %d",
			method->o_slot );
}

static
void
do_volume_m( originator, msg )
pid_t originator;
bkdata_t *msg;
{
	register proc_t *proc;
	register p_slot;

	if( (p_slot = p_findproc( originator )) != -1 ) {
		if( !(proc = P_SLOT( p_slot )) ) {
			brlog( "do_volume_m(): orig %ld has bad slot number %d",
				originator, p_slot );
			return;
		}
		if( proc->type != OPERATOR_P ) {
			brlog(
				"do_volume_m(): received VOLUME msg from orig %ld - ignored",
				originator );
			return;
		}
	}
	(void) op_volume( msg );
}

/* Handle incoming messages */
void
incoming( msg )
queued_msg_t	*msg;
{
	if( !msg ) return;
#ifdef TRACE
	brlog( "incoming(): type: %s orig %ld", msgname( msg->type ), msg->originator );
#endif
	switch( msg->type ) {
	case CANCEL:
	case RESUME:
	case SUSPEND:
		do_control_m( msg->originator, msg->type, &(msg->data.control) );
		break;

	case SUSPENDED:
		do_response_m( msg->originator, SUSPEND );
		break;

	case RESUMED:
		do_response_m( msg->originator, RESUME );
		break;

	case CANCELED:
		do_response_m( msg->originator, CANCEL );
		break;

	case DOT:
		do_dot_m( msg->originator );
		break;

	case DISCONNECTED:
		do_disconnected_m( msg->originator );
		break;

	case DONE:
		do_done_m( msg->originator, &(msg->data.done) );
		break;

	case ESTIMATE:
		do_estimate_m( msg->originator, &(msg->data) );
		break;

	case FAILED:
		do_failed_m( msg->originator, &(msg->data) );
		break;

	case GET_VOLUME:
		do_get_volume_m( msg->originator, &(msg->data.getvolume) );
		break;

	case HISTORY:
		do_history_m( msg->originator, &(msg->data.history) );
		break;

	case INVL_LBLS:
		do_inval_m( msg->originator, &(msg->data.inv_lbls) );
		break;

	case START:
		do_start_m( msg->originator, &(msg->data.start) );
		break;

	case TEXT:
		do_text_m( msg->originator, &(msg->data) );
		break;

	case VOLUME:
		do_volume_m( msg->originator, &(msg->data) );
		break;

	default:
		brlog( "Unknown message type received: %d, message ignored\n",
			msg->type );
		break;
	}
	state |= SOME_DONE;
}
