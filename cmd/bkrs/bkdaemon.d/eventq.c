/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bkdaemon.d/eventq.c	1.4.3.1"

#include	<sys/types.h>
#include	<signal.h>
#include	<backup.h>
#include	<bkdaemon.h>

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

extern int state;
extern int bklevels;

static event_t eventq = { 0, 0, 0, 0, 0 };
static stopped = FALSE;

static event_t sleepq = { 0, 0, 0, 0, 0 };

extern unsigned int alarm();
extern time_t time();
extern void *malloc();
extern void free();
extern void brlog();

static void ev_handler();
static void starttimer();
void	ev_service();

static void
ev_handler()
{

#ifdef TRACE
	brlog( "Event timer went off." );
#endif

	BEGIN_CRITICAL_REGION;
	state |= NEW_EVENTS;
	END_CRITICAL_REGION;
}

/* Schedule fcn to be called in 'seconds' seconds from now */
int
ev_schedule( fcn, arg, seconds )
void	(*fcn)();
long	arg;
int	seconds;
{
	time_t curtime;
	register event_t	*eventp, *new_p;

	/*
		prevent the signal handler from going off -
		this prevents the eventq from getting hosed.
	*/
	(void) alarm(0);
	(void) sigset( SIGALRM, (void (*)())ev_handler );

#ifdef TRACE
	brlog( "ev_schedule(): schedule fcn 0x%x arg 0x%x seconds %d",
		(long) fcn, arg, seconds );
#endif

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
	(void) time( &curtime );
	new_p->time = curtime + seconds;

	for( eventp = &eventq; eventp->forward; eventp = eventp->forward )
		if( eventp->forward->time > new_p->time ) 
			break;

	/* assert: eventp points to just ahead in list of where we want to insert */
	new_p->forward = eventp->forward;
	eventp->forward = new_p;

	starttimer();

	BEGIN_CRITICAL_REGION;
	state |= EVENTS;
	END_CRITICAL_REGION;

	return( TRUE );
}

/*
	only start a signal if there is another event in the queue. By only
	starting a signal for the first event on the queue, race conditions
	are avoided.
*/
static void
starttimer() 
{ 
	time_t curtime; 
	register sleeptime;

	(void) alarm(0);
	if( !stopped && eventq.forward ) {
		(void) time( &curtime ); 
		/* shouldn't happen, but we cannot afford 2 timers at once */
		sleeptime =  curtime < eventq.forward->time?
			eventq.forward->time - curtime: 1;

		(void) alarm( (unsigned int) sleeptime );
	}
}

/* service some events */
void
ev_service()
{
	long	curtime;
	register event_t *eventp, *cur_p;

	/* disable other timers (for safety sake) */
	(void) alarm(0);

	(void) time( &curtime );
	for( eventp = &eventq; eventp && eventp->forward;
		eventp = eventp->forward ) {

		if( eventp->forward->time <= curtime ) {

			/* Remove this event structure from the queue */
			cur_p = eventp->forward;
			eventp->forward = cur_p->forward;

			/* CALL this function */
			(void) (*(cur_p->fcn))( cur_p->arg );
			free( cur_p );

		} else break;
	}

	BEGIN_CRITICAL_REGION;
	state &= ~NEW_EVENTS;
	if( !eventq.forward && !sleepq.forward )
		state &= ~EVENTS;
	END_CRITICAL_REGION;

	starttimer();
}

/* Remove an event from the timeout and sleep queues */
void
ev_cancel( fcn, arg )
void	(*fcn)();
long	arg;
{
	register event_t *eventp, *cur_p;
	
	/* Disable the timer */
	(void) alarm(0);

#ifdef TRACE
	brlog( "ev_cancel(): fcn 0x%x arg 0x%x", (long)fcn, arg );
#endif

	for( eventp = &eventq; eventp; eventp = eventp->forward ) {

		cur_p = eventp->forward;

		if( (long)fcn == (long)cur_p->fcn && arg == cur_p->arg ) {

			/* Remove this event structure from the queue */
			eventp->forward = cur_p->forward;

			free( cur_p );
		}
	}

	for( eventp = &sleepq; eventp; eventp = eventp->forward ) {

		cur_p = eventp->forward;

		if( (long)fcn == (long)cur_p->fcn && arg == cur_p->arg ) {

			/* Remove this event structure from the queue */
			eventp->forward = cur_p->forward;

			free( cur_p );
		}
	}

	BEGIN_CRITICAL_REGION;
	if( !eventq.forward && !sleepq.forward )
		state &= ~EVENTS;
	END_CRITICAL_REGION;
}

/* Stop scheduling events */
void
ev_stop()
{
	(void) alarm(0);
	stopped = TRUE;
}

/* Start up events again */
void
ev_start()
{
/*
	brlog( "ev_start()" );
*/
	stopped = FALSE;
	starttimer();
}

#ifdef TRACE
ev_trace()
{
	register event_t *eventp;

	brlog( "EVENTQ:" );

	for( eventp = eventq.forward; eventp; eventp = eventp->forward ) 
		brlog( "(FCN 0x%x ARG 0x%x) ->", (long)eventp->fcn, eventp->arg );

	brlog( "END" );

	brlog( "SLEEPQ:" );

	for( eventp = sleepq.forward; eventp; eventp = eventp->forward ) 
		brlog( "(FCN 0x%x ARG 0x%x) ->", (long)eventp->fcn, eventp->arg );

	brlog( "END" );
}
#endif

/*
	Place an event on the 'sleep' queue - the function will not be called
	until explicitly awakened.
*/
int
ev_sleep( fcn, arg )
void	(*fcn)();
long	arg;
{
	register event_t *new_p, *eventp;

	/* fill in a new event structure */
	if( !(new_p = (event_t *)malloc( sizeof( event_t )) ) ) {
		brlog( "ev_sleep(): unable to malloc memory" );
		return( FALSE );
	}

	new_p->fcn = fcn;
	new_p->arg = arg;

	for( eventp = &sleepq; eventp->forward; eventp = eventp->forward )
		if( (unsigned long)(eventp->forward->fcn) >= (unsigned long)(new_p->fcn) ) 
			break;

	/* assert: eventp points to just ahead in list of where we want to insert */
	new_p->forward = eventp->forward;
	eventp->forward = new_p;

	BEGIN_CRITICAL_REGION;
	state |= EVENTS;
	END_CRITICAL_REGION;

	return( TRUE );
}

/*
	Wake up an event.
*/
void
ev_wakeup( fcn, arg )
void	(*fcn)();
long	arg;
{
	register event_t *eventp, *cur_p;

	for( eventp = &sleepq;
		eventp && eventp->forward
			&& (unsigned long)(eventp->forward->fcn) <= (unsigned long)(fcn);
		eventp = eventp->forward ) {

		if( (unsigned long)(eventp->forward->fcn) == (unsigned long)(fcn)
			&& (unsigned long)(eventp->forward->arg) == (unsigned long)arg ) {

			/* Remove this event structure from the queue */
			cur_p = eventp->forward;
			eventp->forward = cur_p->forward;

			/* CALL this function */
			(void) (*(cur_p->fcn))( cur_p->arg );
			free( cur_p );

			break;

		} else if( (unsigned long)(eventp->forward->fcn) < (unsigned long)(fcn) )
			break;
	}

	BEGIN_CRITICAL_REGION;
	if( !eventq.forward && !sleepq.forward )
		state &= ~EVENTS;
	END_CRITICAL_REGION;
}
