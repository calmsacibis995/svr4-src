/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bkdaemon.d/owner.c	1.5.2.1"

#include	<sys/types.h>
#include	<backup.h>
#include	<bktypes.h>
#include	<bkmsgs.h>
#include	<bkdaemon.h>
#include	<bkerrors.h>

#define	O_BUNCH	10	/* Allocate this many owner slots at a time */
#define o_exit( o_slot ) \
		(void) strncpy( (char *)(ownertab + o_slot), "", sizeof( owner_t ) )

extern	owner_t *ownertab;
extern	int ownertabsz;
extern	method_t	*methodtab;
extern	int	methodtabsz;
extern	proc_t	*proctab;
extern	int	proctabsz;

extern argv_t *s_to_argv();
extern char *strdup();
extern void *realloc();
extern char *strncpy();
extern void *malloc();
extern int strcmp();
extern void free();
extern void brlog();
extern int p_findslot();
extern int p_exit();
extern int md_free_local();
extern void send_mail_done();
extern void m_send_done();
extern int m_send();
extern int md_d_insert();
extern void argv_free();

/* Find an unused slot in the OWNER table */
static int
o_findslot()
{
	register i;
	register owner_t *owner;

	/* Slot 0 is not used */
	if( ownertabsz ) {
		for( i = 1, owner = O_SLOT( 1 ); i < ownertabsz; i++, owner++ )
			if( !(owner->state & O_ALLOCATED ) ) {
				owner->state |= O_ALLOCATED;
				return( i );
			}

		/* Didn't find an open slot - grow the table */
		if( !(ownertab = (owner_t *)realloc( ownertab,
			(ownertabsz + O_BUNCH) * sizeof( owner_t ) ) ) ) {
			brlog( "Cannot reallocate owner table" );
			return( -1 );
		}
		/* Initialize the new portion of the table */
		(void) strncpy( (char *)(ownertab + ownertabsz), "", O_BUNCH * sizeof( owner_t ) );

		i = ownertabsz;

	} else {
		/* New table - grow it */
		if( !(ownertab = (owner_t *)malloc( O_BUNCH * sizeof(owner_t) ) ) ) {
			brlog( "Cannot allocate owner table" );
			return( -1 );
		}
		(void) strncpy( (char *)ownertab, "", O_BUNCH * sizeof( owner_t ) );

		i = 1;
	} 

	ownertab[ i ].state |= O_ALLOCATED;
	ownertabsz += O_BUNCH;

	/* Return 1st slot in new portion of table */
	return( i );
}

/* Allocate a PROCESS and OWNER slot */
int
o_malloc()
{
	int o_slot, p_slot;
	register proc_t *proc;
	register owner_t *owner;

	if( (p_slot = p_findslot( OWNER_P ) ) == -1 ) {
		brlog( "o_malloc(): cannot allocate a process slot" );
		return( -1 );
	}
	if( !(proc = P_SLOT( p_slot ) ) ) {
		brlog( "o_malloc(): p_findslot() returned a bad slot no. (%d)",
			p_slot );
		return( -1 );
	}
	if( (o_slot = o_findslot()) == -1 ) {
		brlog( "o_malloc(): cannot allocate an owner slot" );
		(void) p_exit( p_slot );
		return( -1 );
	}
	if( !(owner = O_SLOT( o_slot )) ) {
		brlog( "o_malloc(): o_findslot() returned a bad slot no. (%d)",
			o_slot );
		(void) p_exit( p_slot );
		o_exit( o_slot );
		return( -1 );
	}
	proc->slot = o_slot;
	owner->p_slot = p_slot;
	return( o_slot );
}

/* Free up an OWNER slot and it's associated PROCESS slot */
void
o_free( o_slot )
int o_slot;
{
	register owner_t *owner;
	register p_slot;
	if( !(owner = O_SLOT( o_slot )) ) {
		brlog( "o_free(): given bad slot number to free (%d)", o_slot );
		return;
	}
	(void) md_free_local( o_slot );
	p_slot = owner->p_slot;
	o_exit( o_slot );
	(void) p_exit( p_slot );
}

/* All this owner's methods have finished - clean up */
void
o_terminate( o_slot )
int o_slot;
{
	register owner_t *owner;

	if( !(owner = O_SLOT( o_slot )) ) {
		brlog( "o_terminate(): given bad slot number to free (%d)", o_slot );
		return;
	}

	/* If owner is still in bkoper, don't terminate yet */
	if( owner->state & O_TALKING )
		return;

	/* If "-m" option - then send mail */
	if( *owner->owner )
		send_mail_done( o_slot );

	/* Tell the OWNER that it is done */
	m_send_done( owner->p_slot );

	/* Free methods on local list */
	(void) md_free_local( o_slot );

	o_free( o_slot );
}

/* Increment the tally of number of methods failed for this methods owner */
void
o_incfailed( m_slot )
int m_slot;
{
	register owner_t *owner;
	register method_t *method;
	if( !(method = MD_SLOT( m_slot )) ) {
		brlog( "o_incfailed(): given bad m_slot (%d)", m_slot );
		return;
	}
	if( !(owner = O_SLOT( method->o_slot )) ) {
		brlog( "o_incfailed(): m_slot %d has bad o_slot %d", m_slot,
			method->o_slot );
		return;
	}
	owner->m_failed++;
}

/* Inc the tally of number of successful methods for this method's owner */
void
o_incsucceeded( m_slot )
int m_slot;
{
	register owner_t *owner;
	register method_t *method;
	if( !(method = MD_SLOT( m_slot )) ) {
		brlog( "o_incsucceeded(): given bad m_slot (%d)", m_slot );
		return;
	}
	if( !(owner = O_SLOT( method->o_slot )) ) {
		brlog( "o_incsucceeded(): m_slot %d has bad o_slot %d", m_slot,
			method->o_slot );
		return;
	}
	owner->m_succeeded++;
}

/* Kill all methods of an owner */
/*VARARGS0*/
void
o_kill( o_slot )
int o_slot;
{
	/* XXX */
}

/*ARGSUSED*/
/* Send a message to an owner - if it is TALKING, queue the message for it */
int
o_send( o_slot, type, msg )
int o_slot, type;
bkdata_t *msg;
{
	register owner_t *owner;
	register proc_t *proc;
	if( !(owner = O_SLOT( o_slot )) ) {
		brlog( "o_send(): given bad o_slot %d", o_slot );
		return( BKINTERNAL );
	}
	if( !(proc = P_SLOT( owner->p_slot )) ) {
		brlog( "o_send(): o_slot %d has bad p_slot %d", o_slot,
			owner->p_slot );
		return( BKINTERNAL );
	}
	return( m_send( proc->pid, type, msg )? BKSUCCESS: BKEXIST );
}

/* Find this tag on the owner's method list and return a pointer */
static
method_t *
o_findtag( owner, tag )
owner_t *owner;
char *tag;
{
	register m_slot;
	register method_t *method;

	if( !owner || !tag || !*tag )
		return( (method_t *)NULL );

	for( m_slot = owner->methods; m_slot > 0; m_slot = method->next_l ) {

		if( !(method = MD_SLOT( m_slot )) ) {
			brlog( "o_findtag(): bad m_slot %d in o_slot %d's private MD sched",
				m_slot, O_SLOTNO( owner ) );

			return( (method_t *)NULL );
		}

		if( !strcmp( (char *)method->entry.tag, tag ) )
			return( method );
	}

	return( (method_t *)NULL );
}

/* Fix dependency chains for dependencies */
int
o_deps( owner )
owner_t *owner;
{
	register m_slot, i, rc;
	register method_t *method, *m_dep;
	argv_t *deps;
	char *tmp;

	for( m_slot = owner->methods; m_slot > 0; m_slot = method->next_l ) {

		if( !(method = MD_SLOT( m_slot )) ) {
			brlog( "o_deps(): bad m_slot %d in o_slot %d's private MD sched",
				m_slot, O_SLOTNO( owner ) );
			return( BKINTERNAL );
		}

		if( !method->entry.dependencies )
			continue;

		tmp = strdup( (char *)method->entry.dependencies );

		if( !(deps = s_to_argv( tmp, " ," )) )
			return( BKNOMEMORY );

		for( i = 0; (*deps)[i]; i++ )
			if( m_dep = o_findtag( owner, (*deps)[i] ) )
				if( (rc = md_d_insert( method, MD_SLOTNO( m_dep ) ) ) != BKSUCCESS )
					return( rc );

		argv_free( deps );
		free( tmp );
		
	}

	return( BKSUCCESS );
}
