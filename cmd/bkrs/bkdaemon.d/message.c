/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bkdaemon.d/message.c	1.8.2.1"

#include	<sys/types.h>
#include	<backup.h>
#include	<bkmsgs.h>
#include	<bkdaemon.h>
#include	<errno.h>

/* Formats for -n and -ne headers */
#define NE_HDR \
"       Tag     Orig. Name              Orig. Device Dest. Group              Dest. Device Pri Vols.     Blocks Depends On"
#define N_HDR \
"       Tag     Orig. Name              Orig. Device Dest. Group              Dest. Device Pri Depends On"

extern	proc_t	*proctab;
extern	int	proctabsz;
extern	owner_t	*ownertab;
extern	int	ownertabsz;
extern	method_t	*methodtab;
extern	int	methodtabsz;

extern int sprintf();
extern unsigned int strlen();
extern char *strncpy();
extern char *strcpy();
extern int bkm_send();
extern void brlog();
extern void bkstrncpy();
extern int md_flagset();

int
m_send( destination, type, data )
pid_t destination;
int type;
bkdata_t *data;
{
	register rc;
	while( TRUE ) {
		if( (rc = bkm_send( destination, type, data )) == -1 ) {
			if( errno == EEXIST )
				return( 0 );
			brlog( "m_send(): bkm_send fails: errno %d", errno );
		}
		/* If the send was interrupted, keep trying to send this message */
		if( rc != -1 || errno != EINTR )
			break;
	}
	return( 1 );
}

void
m_send_done( pslot )
int pslot;
{
	register proc_t *proc;
	if( !(proc = P_SLOT( pslot ) ) ) {
		brlog( "m_send_done(): given bad pslot: %d", pslot );
		return;
	}
	(void) m_send( proc->pid, DONE, (bkdata_t *)NULL );
}

void
m_send_failure( pslot, tag, reason, text )
int pslot, reason;
char *tag, *text;
{
	register proc_t	*proc;
	bkdata_t data;
	if( !(proc = P_SLOT( pslot )) ) {
		brlog( "m_send_failure(): given bad pslot: %d", pslot );
		return;
	}
#ifdef TRACE
	brlog( "m_send_failure(): p_slot %d", pslot );
#endif

	if( tag ) {
		(void) sprintf( (char *)data.failed.method_id.jobid, "back-%ld", proc->pid );
		if( strlen( tag ) > BKTAG_SZ ) {
			(void) strncpy( data.failed.method_id.tag, tag, BKTAG_SZ );
			data.failed.method_id.tag[ BKTAG_SZ - 1 ] = '\0';
		} else (void) strcpy( data.failed.method_id.tag, tag );
	} else {
		data.failed.method_id.jobid[ 0 ] = '\0';
		data.failed.method_id.tag[ 0 ] = '\0';
	}
	
	data.failed.reason = reason;
	if( text && *text )
		if( strlen( text ) < BKTEXT_SZ )
			(void) strcpy( data.failed.errmsg, text );
		else {
			(void) strncpy( data.failed.errmsg, text, BKTEXT_SZ - 1 );
			data.failed.errmsg[ BKTEXT_SZ - 1 ] = '\0';
		}
	else data.failed.errmsg[ 0 ] = '\0';
	(void) m_send( proc->pid, FAILED, &data );
}

/* SEND Failure message to this method's owner */
void
m_send_mdfail( m_slot, reason, text )
int m_slot, reason;
char *text;
{
	register method_t *method;
	register owner_t *owner;

	if( !(method = MD_SLOT( m_slot )) ) {
		brlog( "m_send_mdfail(): given bad m_slot %d", m_slot );
		return;
	}

	if( !(owner = O_SLOT( method->o_slot )) ) {
		brlog( "m_send_mdfail(): m_slot %d has bad o_slot %d",
			m_slot, method->o_slot );
		return;
	}

	m_send_failure( owner->p_slot, (char *)method->entry.tag, reason, text );
}

/* Send "SUCCESS" message to this method's owner */
void
m_send_mdsuccess( m_slot, nblocks )
int m_slot, nblocks;
{
	register method_t *method;
	register owner_t *owner;
	register proc_t *proc;
	bkdata_t data;

	if( !(method = MD_SLOT( m_slot )) ) {
		brlog( "m_send_mdsuccess(): given bad m_slot %d", m_slot );
		return;
	}
	if( !(owner = O_SLOT( method->o_slot )) ) {
		brlog( "m_send_mdsuccess(): m_slot %d has bad o_slot %d",
			m_slot, method->o_slot );
		return;
	}
	if( !(proc = P_SLOT( owner->p_slot )) ) {
		brlog( "m_send_mdsuccess(): o_slot %d has bad p_slot %d",
			method->o_slot, owner->p_slot );
		return;
	}

	(void) sprintf( (char *)data.text.text, "%s: completed: %d blocks.",
		method->entry.tag, nblocks );
	
	(void) m_send( proc->pid, TEXT, &data );
}

void
m_send_pid_failed( pid, tag, reason, text )
pid_t pid;
int reason;
char *tag, *text;
{
	bkdata_t data;

	if( tag ) {
		(void) sprintf( (char *)data.failed.method_id.jobid, "back-%ld", pid );
		bkstrncpy( (char *)data.failed.method_id.tag, BKTAG_SZ, tag, strlen( tag ) );
	} else data.failed.method_id.tag[ 0 ] = '\0';

	data.failed.reason = reason;

	if( text && *text )
		bkstrncpy( data.failed.errmsg, BKTEXT_SZ, text, strlen( text ) );
	else data.failed.errmsg[ 0 ] = '\0';

	(void) m_send( pid, FAILED, &data );
}

/* Send PRINT messages that show the order of the scheduled methods */
void
m_pr_methods( o_slot, pr_estimate )
int o_slot, pr_estimate;
{
	register owner_t *owner;
	register method_t *l_method_ptr;
	register int *l_method;
	register proc_t *proc;
	bkdata_t data;

	if( !(owner = O_SLOT( o_slot )) ) {
		brlog( "m_pr_methods(): given bad o_slot %d", o_slot );
		return;
	}

	if( !(proc = P_SLOT( owner->p_slot ) ) ) {
		brlog( "m_pr_methods(): o_slot %d has bad p_slot %d", o_slot,
			owner->p_slot );
		return;
	}

	/* Only print for SUCCESSFUL methods */
	if( pr_estimate && !md_anysucceeded( o_slot ) ) return;

	/* Find first one on the list */
	if( *(l_method = &(owner->methods) ) ) {
		if( !(l_method_ptr = MD_SLOT( *l_method )) ) {
			brlog(
				"m_pr_methods(): o_slot %d has bad m_slot in its methods list",
				o_slot, *l_method );
			return;
		}
	} else return;

	/* First send HEADER */
	if( pr_estimate )
		(void) sprintf( (char *)data.text.text, "%s", NE_HDR );
	else (void) sprintf( (char *)data.text.text, "%s", N_HDR );
	(void) m_send( proc->pid, TEXT, &data );

	while( TRUE ) {

		if( !(l_method_ptr->state & MD_FAIL) ) {
			if( pr_estimate )
				(void) sprintf( data.text.text,
					"%10.10s %14.14s %25.25s %11.11s %25.25s %3d %5d %10d %s",
						l_method_ptr->entry.tag, l_method_ptr->entry.oname,
						l_method_ptr->entry.odevice, l_method_ptr->entry.dgroup,
						l_method_ptr->entry.ddevice, l_method_ptr->entry.priority,
						l_method_ptr->e_volumes, l_method_ptr->e_blocks,
						l_method_ptr->entry.dependencies );
			else (void) sprintf( data.text.text,
				"%10.10s %14.14s %25.25s %11.11s %25.25s %3d %s",
					l_method_ptr->entry.tag, l_method_ptr->entry.oname,
					l_method_ptr->entry.odevice, l_method_ptr->entry.dgroup,
					l_method_ptr->entry.ddevice, l_method_ptr->entry.priority,
					l_method_ptr->entry.dependencies );

			(void) m_send( proc->pid, TEXT, &data );
		}
		
		if( *(l_method = &(l_method_ptr->next_l) ) ) {
			if( !(l_method_ptr = MD_SLOT( *l_method )) ) {
				brlog(
				"m_pr_methods(): o_slot %d has bad m_slot in its methods list",
					o_slot, *l_method );
				return;
			}
		} else return;
	}
}
