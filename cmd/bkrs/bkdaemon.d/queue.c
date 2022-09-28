/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bkdaemon.d/queue.c	1.8.2.1"

#include	<sys/types.h>
#include	<backup.h>
#include	<bkmsgs.h>
#include	<bkdaemon.h>
#include	<bkerrors.h>
#include	<bkstatus.h>

#define	MAX_CONCURRENT	10
#define	TIMEOUT	120

extern	int nactive;
extern	method_t	*methodtab;
extern	int	methodtabsz;
extern	owner_t	*ownertab;
extern	int	ownertabsz;
extern	int state;
extern	int	runqueue;
extern	int runpriority;

extern void ev_resched();
extern void brlog();
extern void st_set();
extern int md_chk_deps();
extern int md_terminate();
extern int md_w_insert();
extern int ev_sleep();
extern int md_spawn();
extern int ev_schedule();

/*
	This file contains routines that manage the various queues.
*/

/* 
	merge a particular schedule of methods into the system one.
	q_md_merge() uses a "zipper"-like algorithm.  That is, the local
	method schedule queue is merged into the global run queue in a
	way that is reminiscent of a zipper being closed.

	The general idea is as follows:
		- the "g_" variables refer to the global run queue;
		- the "l_" variables refer to the local method schedule queue;
		- g_method points to the spot in the global list that items from
			the local queue will be inserted AFTER;  this is a pointer to
			a method slot number;
		- g_method_p points to the method structure in the global queue
			AFTER g_method.  This is a pointer to a method structure.
			For example, if the global queue contains method A pointing
			to method B, then if g_method points to A's link element
			(next_g: in this case, m_slot of B), then g_method_p
			points to B.
		- g_priority is the priority of method, g_method_p.
		- l_method, l_method_p, and l_priority  work like g_method,
			g_method_p, and g_priority except that they are used on
			the local method schedule queue.
		- The algorithm proceeds by deciding if method, l_method_p, should
			be inserted before g_method_p.  If so, it is.  If not, the
			global pointers are cycled to the next element in the global
			run queue;
		- A "lowest" priority of -1 is used to force all local methods to
			be inserted when the end of the global queue is reached.
*/
int
q_md_merge( o_slot )
int o_slot;
{
	register owner_t *owner;
	register method_t *l_method_p = NULL, *g_method_p = NULL;
	register int *l_method, *g_method, l_priority, g_priority, limit;

	if( !(owner = O_SLOT( o_slot ) ) ) {
		brlog( "q_md_merge(): given bad o_slot: %d", o_slot );
		return( BKINTERNAL );
	}

	/* Initialize LOCAL pointers - if no processes, then DONE */
	if( *(l_method = &(owner->methods)) ) {
		if( !(l_method_p = MD_SLOT( *l_method )) ) {
			brlog(
				"q_md_merge(): o_slot %d has bad m_slot in private schedule",
				o_slot, *l_method );
			return( BKINTERNAL );
		}
		l_priority = l_method_p->entry.priority;
	} else return( BKSUCCESS );

	/* Initialize GLOBAL pointers */
	if( *(g_method = &runqueue) ) {
		if( !(g_method_p = MD_SLOT( *g_method )) ) {
			brlog(
				"q_md_merge(): bad m_slot (%d) on global run queue", runqueue );
			return( BKINTERNAL );
		}
		g_priority = g_method_p->entry.priority;
	} else {
		g_priority = -1;
		g_method_p = NULL;
	}

	limit = methodtabsz;
	while( limit-- > 0 ) {

		/* if just getting an estimate, don't modify status table */
		if( (owner->options & (S_ESTIMATE|S_NO_EXECUTE))
			!= (S_ESTIMATE|S_NO_EXECUTE) )
			st_setpending( *l_method );

		if( l_priority > g_priority ) {
			/* INSERT local into global */
			l_method_p->next_g = *g_method;
			*g_method = *l_method;
			g_method = &(l_method_p->next_g);
			l_method_p->use_count++;

			/* CYCLE local */
			if( *(l_method = &(l_method_p->next_l)) ) {
				if( !(l_method_p = MD_SLOT( *l_method )) ) {
					brlog(
						"q_md_merge(): o_slot %d has bad m_slot in it's private schedule",
						o_slot, *l_method );
					return( BKINTERNAL );
				} 
				l_priority = l_method_p->entry.priority;
			} else 
				/* End of the LOCAL list */
				break;
		} else {
			/* CYCLE global */
			if( *(g_method = &(g_method_p->next_g)) ) {
				if( !(g_method_p = MD_SLOT( *g_method )) ) {
					brlog( "q_md_merge(): bad m_slot in global runqueue %d",
						*g_method );
					return( BKINTERNAL );
				} 
				g_priority = g_method_p->entry.priority;
			} else {
				/* END of the global list */
				g_priority = -1;
				g_method_p = NULL;
			}
		}
	}
	if( limit < 0 ) {
		/* Cycle in local schedule! */
		brlog(
			"q_md_merge(): cycle detected in o_slot (%d)'s private method schedule",
			o_slot );
		/* XXX - cleanup ? */
		return( BKINTERNAL );
	}
	return( BKSUCCESS );
}

/* Start up those methods that can be started. */
int
q_run()
{
	register method_t *g_method_p = NULL;
	register int *g_method, rc;
	int next_method, tmp;
	void q_mdactivate();

#ifdef TRACE
	brlog( "q_run(): attempt to start methods (%d already active)", nactive );
#endif

	/* If no active methods, then service at any priority */
	if( !nactive ) runpriority = LOWEST_PRIORITY;

	/* If enough started, don't start more */
	if( nactive >= MAX_CONCURRENT )
		return( BKSUCCESS );

	/* Only have one method running at LOWEST_PRIORITY */
	if( !runpriority && nactive > 0 )
		return( BKSUCCESS );

	/* Initialize GLOBAL pointers */
	if( *(g_method = &runqueue) ) {
		if( !(g_method_p = MD_SLOT( *g_method )) ) {
			brlog(
				"q_run(): bad m_slot (%d) on global run queue", runqueue );
			return( BKINTERNAL );
		}
	} else return( BKSUCCESS );

	while( *g_method && nactive < MAX_CONCURRENT ) {
		if( g_method_p->entry.priority < runpriority )
			break;
		else runpriority = g_method_p->entry.priority;

		/* Pick next method to run */
		if( g_method_p->status == MD_HALTED
			|| g_method_p->state & MD_SLEEPING ) {

			/*
				Don't start NON-PENDING methods or ones on the
				event queue 
			*/
			g_method = &(g_method_p->next_g);

		} else {

			/* Remove this method from the queue */
			next_method = *g_method;
			*g_method = g_method_p->next_g;
			g_method_p->next_g = 0;
			g_method_p->use_count--;

			if( (rc = md_chk_deps( next_method ) ) == -1 ) {
#ifdef TRACE
				brlog( "q_run(): m_slot %d: some dependencies failed", next_method );
#endif

				/* Some dependencies have failed */
				(void) md_terminate( next_method, MDT_FAIL, 0,
					"Unable to spawn method - some dependencies failed", 0 );
				goto cycle;

			} else if( rc == -2 ) {

				/* some other error occured */
				(void) md_terminate( next_method, MDT_FAIL, BKINTERNAL,
					"Unable to spawn method", 0 );
				goto cycle;

			} else if( rc > 0 ) {

#ifdef TRACE
				brlog( "q_run(): m_slot %d: wait for dependencies", next_method );
#endif
				/* Put this method on the other method's 'wait for' list */
				if( md_w_insert( rc, next_method ) != BKSUCCESS ) {
					brlog(
						"md_w_insert(): unable to make m_slot %d wait for m_slot %d",
						next_method, rc );
					(void) md_terminate( next_method, MDT_FAIL, BKINTERNAL,
						"unable to wait for dependencies", 0 );
				} else {

					/* Some dependencies have not been run yet */
					g_method_p->state |= MD_SLEEPING;
					g_method_p->ev_fcn = ev_resched;
					g_method_p->ev_arg = rc;

					(void) ev_sleep( ev_resched, (long)rc );

				}
				goto cycle;

			}

			/* Spawn it */
			switch( rc = md_spawn( next_method ) ) {
			case BKBUSY:
				/*
					Let the process sleep for now.  Also, put
					it back on the run queue so that it is involved
					with runpriority calculations.
				*/
				tmp = *g_method;
				*g_method = next_method;
				g_method_p->next_g = tmp;
				g_method_p->use_count++;
				g_method_p->state |= MD_SLEEPING;
				g_method_p->ev_fcn = q_mdactivate;
				g_method_p->ev_arg = next_method;
				(void) ev_schedule( q_mdactivate, (long) next_method, TIMEOUT );
				g_method = &(g_method_p->next_g);
				break;
				
			case BKSUCCESS:
				break;

			case BKBADDGROUP:
				(void) md_terminate( next_method, MDT_FAIL, rc, "Bad device group", 0 );
				break;

			default:
				/* Couldn't spawn it - send error to owner */
				(void) md_terminate( next_method, MDT_FAIL, rc,
					"Unable to spawn method", 0 );
				break;
			}

			/*
				If maximum number of methods is running, or if the current
				run-time priority level is 0 (i.e. run methods sequentially,
				then quit.
			*/
			if( (nactive > MAX_CONCURRENT) || (!runpriority && nactive) )
				break;
		}

cycle:

		/* End of the list - quit */
 		if( !(*g_method) ) break;

		if( !(g_method_p = MD_SLOT( *g_method )) ) {
			brlog(
				"q_run(): bad m_slot (%d) on global run queue", runqueue );
			return( BKINTERNAL );
		}
	}

	return( BKSUCCESS );
}

/*
	Insert a method in queue behind other methods of the same priority.
*/
int
q_inject( m_slot )
int m_slot;
{
	register method_t *method, *g_method_p, *g_tail_p;
	register g_method, g_priority;
	


	if( !(method = MD_SLOT( m_slot )) ) {
		brlog( "q_inject(): given bad m_slot %d", m_slot );
		return( BKINTERNAL );
	}

#ifdef TRACE
	brlog( "q_inject(): insert m_slot %d in global run queue at priority %d",
		m_slot, method->entry.priority );
#endif

	if( g_method = runqueue ) {

		if( !(g_method_p = MD_SLOT( g_method )) ) {
			brlog( "q_inject(): bad m_slot (%d) on global run queue",
				runqueue );
			return( BKINTERNAL );
		}

		g_priority = g_method_p->entry.priority;

		if( g_priority < method->entry.priority ) {
			/* method is higher priority than queued methods */

			method->next_g = runqueue;
			runqueue = m_slot;

		} else if( g_method_p->next_g ) {
			/* Multiple methods in queue and some are higher priority */

			do {

				g_method = g_method_p->next_g;
				g_tail_p = g_method_p;

				if( !(g_method_p = MD_SLOT( g_method )) ) {
					brlog( "q_inject(): global queue has bad m_slot (%d) in it",
						g_method );
					return( BKINTERNAL );
				}
                         /*  reset g_priority to that of next in chain */
			g_priority = g_method_p->entry.priority;		

			} while( (g_priority >= method->entry.priority )
                                 && (g_method_p->next_g)  );

			/* Insert AFTER g_tail_p */

			method->next_g = g_method;
			g_tail_p->next_g = m_slot;

		} else {

			/* Only 1 element in queue - insert AFTER */
			g_method_p->next_g = m_slot;
			method->next_g = 0;

		}

	} else {

		/* Empty runqueue */
		runqueue = m_slot;
		method->next_g = 0;

	}

	method->use_count++;
	st_setpending( m_slot );
	return( BKSUCCESS );
}

/*
	Remove a method from the global runqueue.  If it isn't there, return 
	SUCCESS anyway.
*/
int
q_remove( m_slot )
int m_slot;
{
	register method_t *method, *g_method_p;
	
	if( !(method = MD_SLOT( m_slot )) ) {
		brlog( "q_remove(): given bad m_slot %d", m_slot );
		return( BKINTERNAL );
	}

#ifdef TRACE
	brlog( "q_remove(): remove m_slot %d from global run queue.", m_slot );
#endif

	if( runqueue ) {

		if( runqueue == m_slot ) {
			/* Method is at head of list */
			runqueue = method->next_g;
			method->next_g = 0;
			method->use_count--;
			return( BKSUCCESS );
		}

		/* Not at head of list - go look for it */
		if( !(g_method_p = MD_SLOT( runqueue )) ) {
			brlog( "q_remove(): bad m_slot (%d) on global run queue", runqueue );
			return( BKINTERNAL );
		}

		while( g_method_p->next_g ) {
			if( g_method_p->next_g == m_slot ) {
				/* Found it */
				g_method_p->next_g = method->next_g;
				method->next_g = 0;
				method->use_count--;

				return( BKSUCCESS );
			}
		
			if( !(g_method_p = MD_SLOT( g_method_p->next_g )) ) {
				brlog( "q_remove(): bad m_slot (%d) on global run queue", runqueue );
				return( BKINTERNAL );
			}
		}

		/* It wasn't on the runqueue */
		method->next_g = 0;

	}

	return( BKSUCCESS );
}

/* Activate this method */
void
q_mdactivate( m_slot )
int m_slot;
{
	register method_t *method;

	if( !(method = MD_SLOT( m_slot )) ) {
		brlog( "q_mdactivate(): given bad m_slot %d", m_slot );
		return;
	}

#ifdef TRACE
	brlog( "Activate m_slot %d", m_slot );
#endif
	method->state &= ~MD_SLEEPING;
	method->ev_fcn = (void (*)())0;
	method->ev_arg = 0;
}
