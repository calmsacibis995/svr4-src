/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:tmeth.d/event.c	1.1.2.1"

#include	<sys/types.h>
#include	<signal.h>

#ifndef TRUE
#define	TRUE 1
#define	FALSE 0
#endif

typedef	struct event_s {
	struct event_s *forward;
	struct event_s *backward;
	time_t time;
	void	(*fcn)();
	long	arg;
} event_t;

static event_t eventq;
static stopped = FALSE;

char *malloc();
void free();
void starttimer();

/* Schedule fcn to be called in 'seconds' seconds from now */
ev_schedule( fcn, arg, seconds )
void	(*fcn)();
long	arg;
int	seconds;
{
	void	ev_service();
	time_t curtime;
	register event_t	*eventp, *new_p;

	/*
		prevent the signal handler from going off -
		this prevents the eventq from getting hosed.
	*/
	alarm(0);
	sigset( SIGALRM, ev_service );
	if( seconds < 0 ) {
		brlog( "ev_schedule(): called with %d seconds.", seconds );
		return( FALSE );
	}

	/* fill in a new event structure */
	if( !(new_p = (event_t *)malloc( sizeof( event_t )) ) ) {
		brlog( "ev_schedule(): unable to malloc memory" );
		return( FALSE );
	}

	new_p->fcn = fcn;
	new_p->arg = arg;
	time( &curtime );
	new_p->time = curtime + seconds;

	for( eventp = &eventq; eventp->forward; eventp = eventp->forward )
		if( eventp->forward->time >= new_p->time ) 
			break;

	/* assert: eventp points to just ahead in list of where we want to insert */
	new_p->forward = eventp->forward;
	eventp->forward = new_p;

	starttimer();
	return( TRUE );
}

/*
	only start a signal if there is another event in the queue. By only
	starting a signal for the first event on the queue, race conditions
	are avoided.
*/
static
void
starttimer() 
{ 
	time_t curtime; 
	register sleeptime;

	alarm(0);
	if( !stopped && eventq.forward ) {
		time( &curtime ); 
		/* shouldn't happen, but we cannot afford 2 timers at once */
		sleeptime =  curtime < eventq.forward->time?
			eventq.forward->time - curtime: 1;
/*
		brlog( "starttimer(): alarm(%d)", sleeptime );
*/
		alarm( sleeptime );
	}
/*
		else brlog( "starttimer(): no events started" ); 
*/
}

/* service some events */
static
void
ev_service()
{
	int	curtime, more;
	register event_t *eventp, *cur_p;

	/* disable other timers (for safety sake) */
	alarm(0);

	more = TRUE;
	do {
		time( &curtime );
		for( eventp = &eventq; more && eventp->forward;
			eventp = eventp->forward ) {

			if( eventp->forward->time <= curtime ) {

				/* Remove this event structure from the queue */
				cur_p = eventp->forward;
				eventp->forward = cur_p->forward;

				/* CALL this function */
				(void) (*(cur_p->fcn))( cur_p->arg );
				(void) free( cur_p );

			} else more = FALSE;
		}
	} while( more );

	starttimer();
}

/* Stop scheduling events */
ev_stop()
{
	alarm(0);
/*
	brlog( "ev_stop()" );
*/
	stopped = TRUE;
}

/* Start up events again */
ev_start()
{
/*
	brlog( "ev_start()" );
*/
	stopped = FALSE;
	starttimer();
}
