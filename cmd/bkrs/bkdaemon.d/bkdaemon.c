/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bkdaemon.d/bkdaemon.c	1.10.2.1"

#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <backup.h>
#include <bkmsgs.h>
#include <bkrs.h>
#include <bkdaemon.h>
#include <errno.h>

/* Does a process exist? */
#define proc_exist( pid )	(!kill( pid, 0 ) || errno != ESRCH)

extern int state, bklevels, runqueue, nactive;

extern void *malloc(), free(), exit(), st_stop(), hst_truncate();
extern void brloginit(), brlog(), ev_stop(), ev_start(), ev_service();
extern void incoming(), ev_cancel(), p_terminate(), p_check();
extern void pause();
extern int getppid(), getpid(), setuid(), setpgrp(), wait();
extern int bkm_start(), st_start(), q_run(), ev_schedule();
extern int bkm_exit(), bkm_receive();

static void check_parent(), new_message(), waitrtn();

static int first_msg = FALSE;

void
main()
{
	register pnum;
	int	done = FALSE, status, parentpid;
	queued_msg_t	msg;
	register rc;

	brloginit( "bkdaemon", BACKUP_T );
	parentpid = getppid();

#ifdef TRACE
	brlog( "spawned..." );
#endif

	/* divorce bkdaemon from the backup command that spawned it. */
	(void) setuid(0);
	(void) setpgrp();
	(void) sigignore( SIGHUP );
	(void) sigignore( SIGINT );
	(void) sigset( SIGCLD, SIG_DFL );

	/* set up signal handler on SIGUSR1 */
	(void) sigset( SIGUSR1, new_message );

	/* If there is already a bkdaemon running, give up */
	if( bkm_start( BKNAME, FALSE ) == -1 ) {
		/* Start up parent, anyway */
		(void) kill( getppid(), SIGUSR1 );
		brlog( "bkdaemon pid %d: Could not start up messaging - errno: %d",
			getpid(),  errno );
		exit( 1 );
	}

	if( !st_start() ) {
		/* Unable to initialize status table */
		exit( 1 );
	}

	/* truncate history log */
	hst_truncate();

	/*
		Start up parent - sending a SIGUSR1 to it tells it that it
		may send its START message.  Keep checking on it in the case
		that it hasn't sent the message yet.
	*/
	ev_stop();
	if( !ev_schedule( check_parent, (long)parentpid, 15 )) {
		brlog( "Unable to start event queues." );
		(void) sigignore( SIGUSR1 );
		(void) sigignore( SIGALRM );
		(void) sigignore( SIGCLD );

		/* Remove the message queue */
		bkm_exit( BKNAME );
		st_stop();
		exit( 1 );
	}
	(void) kill( parentpid, SIGUSR1 );
	ev_start();

	/* Wait for a message to come in */
	while( !done ) {

#ifdef TRACE
		brlog( "top of while loop; state is 0x%x", state );
#endif
		if( state & NEW_EVENTS ) {

			/* Service Events */
			ev_service();
			(void) q_run();

		} else if( state & NEW_MESSAGES ) {

			/* Service new messages */
			BEGIN_CRITICAL_REGION;
	
			if( (rc = bkm_receive( &(msg.originator), &(msg.type),
				&(msg.data) )) == -1 ) {

				state &= ~NEW_MESSAGES;

				switch( errno ) {
				case EEXIST:
					/* check to see that all processes are still alive */
					p_check();
					break;
				default:
					if( errno != ENOMSG )
						brlog( "bkm_receive returns: %s", brerrno( errno ) );

					break;
				}
			} else if( rc > 0 ) {
#ifdef TRACE
				brlog( "New message orig %d type %s nbytes %d",
					msg.originator, msgname( msg.type ), rc );
#endif
				END_CRITICAL_REGION;

				if( !first_msg ) {
					ev_cancel( check_parent, (long)parentpid );
					first_msg = TRUE;
				}

				incoming( &msg );

				BEGIN_CRITICAL_REGION;
			} 
			END_CRITICAL_REGION;

		} else if( state & ACTIVE_METHODS ) {

			(void) ev_schedule( waitrtn, 0, 30 );
			pnum = wait( &status );
			ev_cancel( waitrtn, 0 );

			if( pnum  != -1 ) {

				if( (status & 0177) == 0177 )
					/* Traced child got a signal */
					continue;

				/*
					Before processing the terminated method,
					check to see if it sent any messages before
					it exited.
				*/
				/* Service new messages */
				BEGIN_CRITICAL_REGION;
		
				if( (rc = bkm_receive( &(msg.originator), &(msg.type),
					&(msg.data) )) == -1 ) {

					state &= ~NEW_MESSAGES;

					switch( errno ) {
					case EEXIST:
						/* check to see that all processes are still alive */
						p_check();
						break;
					default:
						if( errno != ENOMSG )
							brlog( "bkm_receive returns: %s", brerrno( errno ) );

						break;
					}
				} else if( rc > 0 ) {
#ifdef TRACE
					brlog( "New message orig %d type %s nbytes %d",
						msg.originator, msgname( msg.type ), rc );
#endif
					END_CRITICAL_REGION;

					if( !first_msg ) {
						ev_cancel( check_parent, (long)parentpid );
						first_msg = TRUE;
					}

					incoming( &msg );

					BEGIN_CRITICAL_REGION;
				} 
				END_CRITICAL_REGION;

				p_terminate( pnum, status );

			} else if( errno == ECHILD ) 
				state &= ~ACTIVE_METHODS;

			/* Check to see if any more methods can be started */
			(void) q_run();

		} else if( runqueue ) {
#ifdef	TRACE
			brlog( "runqueue: %d", runqueue );
#endif

			(void) q_run();

			if( state & EVENTS ) {
#ifdef TRACE
				brlog( "Waiting for SLEEPING events" );
				ev_trace();
#endif
				(void) ev_schedule( waitrtn, 0, 30 );
				(void) sigpause( SIGUSR1 );
				ev_cancel( waitrtn, 0 );
			}

		} else if( state & EVENTS ) {

#ifdef TRACE
			brlog( "Waiting for SLEEPING events" );
#endif
			pause();

		} else if( !(state & SOME_DONE) || (state & TALKING) ) {

#ifdef	TRACE
			brlog( "Wait for message" );
#endif
			(void) ev_schedule( waitrtn, 0, 30 );
			(void) sigpause( SIGUSR1 );
			ev_cancel( waitrtn, 0 );
				
		} else {
			/* We get here when: 
				- no new messages;
				- no active methods;
				- and the methods have completed;
				therefore, we are done.
			*/
			(void) sigignore( SIGUSR1 );
			(void) sigignore( SIGALRM );
			(void) sigignore( SIGCLD );

			/* Remove the message queue */
			bkm_exit( BKNAME );
			done = TRUE;
		}
	}

	/* close the status table */
	st_stop();

	exit( 0 );
}

static
void
check_parent( pid )
{
	static void check_parent();

#ifdef TRACE
	brlog( "check_parent() event" );
#endif

	if( !first_msg ) {
		if( !proc_exist( pid ) ) {
			brlog( "Parent backup process has gone away - give up" );

			(void) sigignore( SIGUSR1 );
			(void) sigignore( SIGALRM );
			(void) sigignore( SIGCLD );

			/* Remove the message queue */
			bkm_exit( BKNAME );

			/* close the status table */
			st_stop();

			exit( 1 );
		}

		(void) ev_schedule( check_parent, (long) pid, 15 );
	}
}

/*VARARGS0*/
static void
new_message( sig )
int sig;
{

#ifdef TRACE
	brlog( "Signal %d arrived", sig );
#endif
	
	state |= NEW_MESSAGES;

}

/*ARGSUSED*/
static void
waitrtn()
{
#ifdef	TRACE
	brlog( "alarm went off." );
#endif
}
