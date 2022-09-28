/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bkdaemon.d/control.c	1.8.2.1"

#include	<sys/types.h>
#include	<backup.h>
#include	<bkerrors.h>
#include	<bkmsgs.h>
#include	<bkdaemon.h>

#define	C_BUNCH	10	/* Allocate this many owner slots at a time */
#define	c_exit( c_slot ) \
	(void) strncpy( (char *)(controltab + c_slot), "", sizeof( control_t ) )

extern void free();
extern void *malloc();
extern void *realloc();
extern char *strncpy();
extern void brlog();
extern int p_findslot();
extern int p_exit();
extern int m_send();
extern int md_terminate();
extern void md_free();
extern void st_set();
extern void st_resume();

extern	control_t *controltab;
extern	int controltabsz;
extern	owner_t	*ownertab;
extern	int ownertabsz;
extern	proc_t	*proctab;
extern	int	proctabsz;
extern	method_t	*methodtab;
extern	int	methodtabsz;
extern	int state;
extern	int ntalking;

/* Find an unused slot in the CONTROL table */
static int
c_findslot()
{
	register i;
	register control_t *control;

	/* Slot 0 is not used */
	if( controltabsz ) {
		for( i = 1, control = C_SLOT( 1 ); i < controltabsz; i++, control++ )
			if( !(control->state & C_ALLOCATED ) ) {
				control->state |= C_ALLOCATED;
				return( i );
			}

		/* Didn't find an open slot - grow the table */
		if( !(controltab = (control_t *)realloc( controltab,
			(controltabsz + C_BUNCH) * sizeof( control_t ) ) ) ) {
			brlog( "Cannot reallocate control table" );
			return( -1 );
		}
		/* Initialize the new portion of the table */
		(void) strncpy( (char *)(controltab + controltabsz), "",
			C_BUNCH * sizeof( control_t ) );

		i = controltabsz;

	} else {
		/* New table - grow it */
		if( !(controltab = (control_t *)malloc( C_BUNCH * sizeof(control_t)))) {
			brlog( "Cannot allocate control table" );
			return( -1 );
		}
		(void) strncpy( (char *)controltab, "", C_BUNCH * sizeof( control_t ) );

		i = 1;
	} 

	controltab[ i ].state |= C_ALLOCATED;
	controltabsz += C_BUNCH;

	/* Return 1st slot in new portion of table */
	return( i );
}

/* Allocate a PROCESS and CONTROL slot */
int
c_malloc()
{
	int c_slot, p_slot;
	register proc_t *proc;
	register control_t *control;

	if( (p_slot = p_findslot( CONTROL_P ) ) == -1 ) {
		brlog( "c_malloc(): cannot allocate a process slot" );
		return( -1 );
	}
	if( !(proc = P_SLOT( p_slot ) ) ) {
		brlog( "c_malloc(): p_findslot() returned a bad slot no. (%d)",
			p_slot );
		return( -1 );
	}
	if( (c_slot = c_findslot()) == -1 ) {
		brlog( "c_malloc(): cannot allocate a control slot" );
		(void) p_exit( p_slot );
		return( -1 );
	}
	if( !(control = C_SLOT( c_slot )) ) {
		brlog( "c_malloc(): c_findslot() returned a bad slot no. (%d)",
			c_slot );
		(void) p_exit( p_slot );
		c_exit( c_slot );
		return( -1 );
	}
	proc->slot = c_slot;
	control->p_slot = p_slot;
	return( c_slot );
}

/* Free up an CONTROL slot and it's associated PROCESS slot */
void
c_free( c_slot )
int c_slot;
{
	register control_t *control;

	if( !(control = C_SLOT( c_slot )) ) {
		brlog( "c_free(): given bad slot number to free (%d)", c_slot );
		return;
	}
	(void) p_exit( control->p_slot );
	c_exit( c_slot );
}

/* Empty out a CONTROL process' cqueue and c_exit */
void
c_terminate( c_slot )
int c_slot;
{
	register control_t *control;
	register method_t *method;
	register cqueue_t *cqptr, *tmp;

	if( !(control = C_SLOT( c_slot )) ) {
		brlog( "c_free(): given bad slot number to free (%d)", c_slot );
		return;
	}

	/* Empty out control queue */
	for( cqptr = control->cqueue; cqptr; ) {
		if( !(method = MD_SLOT( cqptr->m_slot )) ) {
			brlog(
				"c_terminate(): bad m_slot %d in c_slot %d's control queue",
				cqptr->m_slot, c_slot );
			return;
		}

		/* 'Unmark' method as being controlled */
		method->c_slot = 0;

		/* Free method if no longer in use */
		if( --(method->use_count) <= 0 ) md_free( cqptr->m_slot );
		tmp = cqptr;
		cqptr = cqptr->next_c;
		free( tmp );
	}
	c_free( c_slot );
}

/*
	Check for existence of methods with particular uid/pid. "-1" means "all".
*/
int
c_mcheck( uid, pid )
uid_t uid;
pid_t pid;
{
	register i, some = 0;
	register owner_t *owner;
	register proc_t *proc;

	/* Search through OWNER table looking for a match */
	for( i = 1, owner = O_SLOT( 1 ); i < ownertabsz; i++, owner++ )
		if( owner->state & O_ALLOCATED ) {
			if( !(proc = P_SLOT( owner->p_slot )) ) {
				brlog( "c_mcheck(): o_slot %d has bad p_slot %d", i,
					owner->p_slot );
				return( BKINTERNAL );
			}
			if( uid == -1 || uid == proc->uid ) {
				if( pid == -1 || pid == proc->pid ) 
					/*
						If the capability to control individual methods is
						introduced, the following check would have to change.
					*/
					if( owner->controling > 0 ) return( BKBUSY );
					else some++;
			}
		}
	return( some? BKSUCCESS: BKNONE );
}

/* Insert an OWNER's local queue methods into a CONTROL's control queue */
static int
c_cq_insert( cqptr, m_slot )
cqueue_t ***cqptr;
int	 m_slot;
{
	register method_t *method;
	register int slot, some = 0;
	register cqueue_t *tmpcqptr;

	for( slot = m_slot; slot; slot = method->next_l ) {
		if( !(method = MD_SLOT( slot )) ) {
			brlog( "c_cq_insert(): bad m_slot %d in local schedule", slot );
			return( BKINTERNAL );
		}

		if( !(tmpcqptr = (cqueue_t *)malloc( sizeof( cqueue_t ) ) ) ) {
			brlog( "c_cq_insert(): unable to malloc memory" );
			return( BKNOMEMORY );
		}
		(void) strncpy( (char *)tmpcqptr, "", sizeof( cqueue_t ) );
		method->use_count++;	/* Method is on yet another list */
		tmpcqptr->m_slot = slot;
		**cqptr = tmpcqptr;
		*cqptr = &(tmpcqptr->next_c);
		some++;
	}
	return( some );
}

/* Find methods to put in CONTROL's method list */
int
c_schedule( c_slot )
int c_slot;
{
	register i, rc, some = 0;
	register control_t *control;
	register owner_t *owner;
	register proc_t *proc;
	cqueue_t **cqptr;
	
	if( !(control = C_SLOT( c_slot )) ) {
		brlog( "c_schedule(): given bad c_slot %d", c_slot );
		return( BKINTERNAL );
	} 

	cqptr = &(control->cqueue);

	/* Search through OWNER table looking for a match */
	for( i = 1, owner = O_SLOT( 1 ); i < ownertabsz; i++, owner++ )
		if( owner->state & O_ALLOCATED ) {
			if( !(proc = P_SLOT( owner->p_slot )) ) {
				brlog( "c_schedule(): o_slot %d has bad p_slot %d", i,
					owner->p_slot );
				return( BKINTERNAL );
			}

			if( control->uid == -1 || control->uid == proc->uid ) {
				if( control->pid == -1 || control->pid == proc->pid ) 

					/* Found one - insert this owner's methods in the cqueue */
					if( (rc = c_cq_insert( &cqptr, owner->methods ) )
						< 0 )
						return( rc );
					else some += rc;
			}
		}
	*cqptr = 0;
	return( some? BKSUCCESS: BKNONE );
}

/* Send a CONTROL message to all methods in a CONTROL's control queue */
int
c_broadcast( c_slot )
int c_slot;
{
	register control_t *control;
	register cqueue_t *cqptr;
	register owner_t *owner;
	register method_t *method;
	register proc_t *proc;
	register some = 0;

	if( !(control = C_SLOT( c_slot )) ) {
		brlog( "c_broadcast(): given bad c_slot %d", c_slot );
		return( BKINTERNAL );
	}

	control->ntodo = 0;

	for( cqptr = control->cqueue; cqptr; cqptr = cqptr->next_c ) {
		if( !(method = MD_SLOT( cqptr->m_slot )) ) {
			brlog( "c_broadcast(): bad m_slot %d in c_slot %d's cqueue",
				cqptr->m_slot, c_slot );
			return( BKINTERNAL );
		}
		if( !(owner = O_SLOT( method->o_slot )) ) {
			brlog( "c_broadcast(): m_slot %d has bad o_slot %d",
				cqptr->m_slot, method->o_slot );
			return( BKINTERNAL );
		}
		if( !(proc = P_SLOT( method->p_slot )) ) {
			brlog( "c_broadcast(): m_slot %d has bad p_slot %d",
				cqptr->m_slot, method->p_slot );
			return( BKINTERNAL );
		}

		switch( control->type ) {
		case CANCEL:
			owner->state &= ~O_TALKING;
			if( !(--ntalking) )
				state &= ~TALKING;

			switch( method->status ) {
			case MD_ACTIVE:
			case MD_WAITING:
			case MD_HALTED:
				/* Send Message telling METHOD to CANCEL itself */
				if( m_send( proc->pid, control->type, NULL ) ) {
					method->state |= MD_CONTROLLING;
					method->c_slot = c_slot;
					owner->controling++;
					control->ntodo++;
					some++;
				}
				break;

			case MD_PENDING:
				/* Terminate the method */
				method->c_slot = c_slot;
				(void) md_terminate( cqptr->m_slot, MDT_CANCEL, 0, 0, 0 );
				some++;
				break;
			}
			break;

		case SUSPEND:
			switch( method->status ) {
			case MD_ACTIVE:
			case MD_WAITING:
				/* Send Message to METHOD to SUSPEND itself */
				if( m_send( proc->pid, control->type, NULL ) ) {
					method->state |= MD_CONTROLLING;
					method->c_slot = c_slot;
					owner->controling++;
					control->ntodo++;
					some++;
				}
				break;

			case MD_PENDING:
				st_setsuspended( cqptr->m_slot );
				some++;
				break;
			}
			break;

		case RESUME:
			switch( method->status ) {
			case MD_ACTIVE:
			case MD_HALTED:
			case MD_WAITING:
				/* Send message to METHOD to RESUME itself */
				if( m_send( proc->pid, control->type, NULL ) ) {
					method->state |= MD_CONTROLLING;
					method->c_slot = c_slot;
					owner->controling++;
					control->ntodo++;
					some++;
				}
				break;

			case MD_PENDING:
				st_resume( cqptr->m_slot );
				some++;
				break;
			}
			break;
		}
	}
	return( some? BKSUCCESS: BKNONE );
}

/* Is this method on this cqueue? */
static int
c_on_cqueue( cqptr, m_slot )
cqueue_t *cqptr;
int	m_slot;
{
	for( ; cqptr; cqptr = cqptr->next_c ) 
		if( m_slot == cqptr->m_slot ) return( TRUE );
	return( FALSE );
}

/*
	Process a response to a CONTROL message - respond to ALL controllers
	of the expecting a response of this type
*/
int
c_response( m_slot, type, cleanup )
int m_slot, type, cleanup;
{
	register rc = BKSUCCESS;
	register method_t *method;
	register owner_t *owner;
	register proc_t *proc;
	register control_t *control;

	if( !(method = MD_SLOT( m_slot )) ) {
		brlog( "c_response(): given bad m_slot %d", m_slot );
		return( BKINTERNAL );
	}
	if( !(owner = O_SLOT( method->o_slot )) ) {
		brlog( "c_response(): m_slot %d has bad o_slot %d",
			m_slot, method->o_slot );
		return( BKINTERNAL );
	}
	/*
		ASSERT: a method can only be responding to one CONTROL at a time.
	*/
	if( !(control = C_SLOT( method->c_slot )) ) {
		brlog( "c_response(): m_slot %d has a bad c_slot %d", m_slot,
			method->c_slot );
		return( BKINTERNAL );
	}

	/* Perform consistency checks */
	if( !(method->state & MD_CONTROLLING) || !method->c_slot ) {
		brlog(
			"c_response(): msg type %d from not-CONTROLed method - ignored",
			type );
		goto c_resp_done;
	}
	if( type != control->type ) {
		/* Note: DON'T mark this method "not controlled" */
		brlog(
			"c_response(): type of m_slot %d's response doesn't match c_slot %d's type - response ignored",
			m_slot, method->c_slot );
		return( BKSUCCESS );
	}
	if( !c_on_cqueue( control->cqueue, m_slot ) ) {
		brlog( "c_response(): m_slot %d is NOT on c_slot %d's queue",
			m_slot, method->c_slot );
		rc = BKINTERNAL;
		goto c_resp_done;
	}

	if( !cleanup ) 
		switch( type ) {
		case SUSPEND:
			/* XXX - free devices associated with this method */
			st_setsuspended( m_slot );
			break;
		case RESUME:
			/* XXX - re-allocate devices associated with this method */
			st_resume( m_slot );
			break;
		}

	/* If this is the last responding method for this CONTROL, terminate it. */
	if( --(control->ntodo) <= 0 ) {
		if( !control->o_slot ) {
			/* This CONTROL is not an OWNER - send DONE message */
			if( !(proc = P_SLOT( control->p_slot )) ) {
				brlog( "c_response(): c_slot %d has bad p_slot %d",
					method->c_slot, control->p_slot );
				rc = BKINTERNAL;
				goto c_resp_done;
			}
			(void) m_send( proc->pid, DONE, NULL );
		}
		c_terminate( method->c_slot );
	}

c_resp_done:
	/* Mark response in the OWNER and METHOD structures */
	owner->controling--;
	method->state &= ~MD_CONTROLLING;
	method->c_slot = 0;

	return( rc );
}
