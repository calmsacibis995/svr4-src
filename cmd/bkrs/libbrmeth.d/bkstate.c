/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:libbrmeth.d/bkstate.c	1.8.2.1"

#include	<sys/types.h>
#include	<signal.h>
#include	<time.h>
#include	<backup.h>
#include	<bkmsgs.h>
#include	<bkrs.h>
#include	<bkoper.h>
#include	<errno.h>

/* 
	This file contains routines that implement a state machine for incoming
	messages.  Most of the time, the state machine will be in the ST_NORMAL
	state.  However, when the method does a brgetvolume() call, a conversation
	of several messages takes place.  Most of the rest of these states handle
	this conversation.  Since the state machine works at signal catching 
	level, communication between the signal handler and the brgetvolume()
	routine is done through global variables.
*/

/* Names of the various states */
#define	ST_NORMAL	1
#define	ST_SUSPENDING	2
#define	ST_SUSPENDED	3
#define	ST_GET_VOLUME	4
#define	ST_CANCELING	5
#define	ST_CANCELED		6
#define	ST_NSTATES	6

/* State to char string conversion */
#define	STNAME(state)	((state > 0 && state <= ST_NSTATES)? \
			stnames[ state - 1 ]: "UNKNOWN" )

static char *stnames[] = {
	"ST_NORMAL", "ST_SUSPENDING", "ST_SUSPENDED", "ST_GET_VOLUME",
	"ST_CANCELING", "ST_CANCELED"
};

/* Strings for some local error messages */
static char *errmsg =
	"Unable to send %s message to bkdaemon: errno %ld - %s";

extern int bklevels;
extern pid_t bkdaemonpid;

extern int sprintf();
extern unsigned int strlen();
extern char *strcpy();
extern char *strncpy();
extern int bkm_send();
extern void brlog();
extern int bkm_receive();
static int bkcurrent = ST_NORMAL;
static char *brresult;
int	brstate = BR_PROCEED, bksuspended;
static int brdone, brrc;

bkgetvolume( volume, override, automated, result )
char *volume, *result;
int override, automated;
{
	register rc, current;
	bkdata_t msg;

#ifdef TRACE
	brlog( "bkgetvolume(): volume %s; override - %s; automated - %s",
		volume, (override? "yes": "no" ), (automated? "yes": "no" ) );
#endif

	BEGIN_CRITICAL_REGION;
	current = bkcurrent;
	END_CRITICAL_REGION;

	switch( current ) {
	case ST_NORMAL:

		/* Fill in message */
		if( strlen( volume ) < BKLABEL_SZ + 1 )
			(void) strcpy( msg.getvolume.label, volume );
		else {
			(void) strncpy( msg.getvolume.label, volume, strlen( volume ) - 1 );
			(msg.getvolume.label)[ strlen( volume ) - 1 ] = '\0';
		}

		msg.getvolume.flags = 0;
		if( override ) 
			msg.getvolume.flags |= GV_OVERRIDE;

		if( bkm_send( bkdaemonpid, GET_VOLUME, &msg ) == -1 ) {
			brlog( "Unable to send GET_VOLUME message to bkdaemon; errno %ld",
				errno );
			bkcurrent = ST_CANCELING;
			brstate = BR_CANCEL;
			return( BRFATAL );
		}

		BEGIN_CRITICAL_REGION;
		bkcurrent = ST_GET_VOLUME;
		brresult = result;
		brdone = FALSE;
		END_CRITICAL_REGION;

		/* Wait for state machine to cycle thru conversation */
		while( !brdone )	(void) sigpause( SIGUSR1 );

		return( brrc );
		/*NOTREACHED*/
		break;

	case ST_SUSPENDED:
	case ST_CANCELED:
	case ST_GET_VOLUME:
		/* Can't? happen */
		BEGIN_CRITICAL_REGION;
		brstate = BR_CANCEL;
		END_CRITICAL_REGION;

		rc = BRFATAL;
		break;

	case ST_CANCELING:
		brstate = BR_CANCEL;
		rc = BRCANCELED;
		break;

	case ST_SUSPENDING:
		brstate = BR_SUSPEND;
		rc = BRSUSPENDED;
		break;

	default:
		brlog( "bkgetvolume(): in unknown state: %d", bkcurrent );
		brstate = BR_CANCEL;
		rc = BRFATAL;
		break;
	}

	return( rc );
}

/* Suspend this method */
bksuspend()
{
	register current, rc = BRSUCCESS;

	BEGIN_CRITICAL_REGION;
	current = bkcurrent;
	END_CRITICAL_REGION;

	switch( current ) {
	case ST_NORMAL:
		break;

	case ST_SUSPENDING:
		if( bkm_send( bkdaemonpid, SUSPENDED, NULL ) == -1 ) {
			brlog( "Unable to send SUSPENDED message to bkdaemon; errno %ld",
				errno );
			brstate = BR_CANCEL;
			rc = BRFATAL;
			break;
		} else {
			BEGIN_CRITICAL_REGION;
			bkcurrent = ST_SUSPENDED;
			END_CRITICAL_REGION;
		}
		/* NO BREAK */
		
	case ST_SUSPENDED:
		BEGIN_CRITICAL_REGION;
		bksuspended = TRUE;
		END_CRITICAL_REGION;

		/* Wait for state machine to cycle thru conversation */
		while( bksuspended )	(void) sigpause( SIGUSR1 );
		break;

	case ST_GET_VOLUME:
	case ST_CANCELED:
		/* Can't? happen */
		BEGIN_CRITICAL_REGION;
		brstate = BR_CANCEL;
		END_CRITICAL_REGION;

		rc = BRFATAL;
		break;
		
	case ST_CANCELING:
		BEGIN_CRITICAL_REGION;
		brstate = BR_CANCEL;
		END_CRITICAL_REGION;

		rc = BRCANCELED;
		break;

	default:
		brlog( "bksuspend(): in unknown state: %d", current );
		brstate = BR_CANCEL;
		rc = BRFATAL;
		break;
	}
	return( rc );
}

/* Cancel this method */
bkcancel()
{
	register current, rc = BRSUCCESS;

	BEGIN_CRITICAL_REGION;
	current = bkcurrent;
	END_CRITICAL_REGION;

	switch( current ) {
	case ST_NORMAL:
	case ST_SUSPENDING:
	case ST_SUSPENDED:
	case ST_GET_VOLUME:
	case ST_CANCELED:
		break;

	case ST_CANCELING:
		if( bkm_send( bkdaemonpid, CANCELED, NULL ) == -1 )
			brlog( "Unable to send CANCELED message to bkdaemon; errno %ld",
				errno );
		break;
		
	default:
		brlog( "bkcancel(): in unknown state: %d", current );
		brstate = BR_CANCEL;
		break;
	}
	
	return( rc );
}

/* bkmessage handles incoming messages according to a state machine */
void
bkmessage( signal )
int signal;
{
	bkdata_t	msg;
	pid_t originator;
	int	type;
	if( signal != SIGUSR1 ) return;

	BEGIN_SIGNAL_HANDLER;

#ifdef TRACE
	brlog( "received signal %d", signal );
#endif

	/* Read message from queue */
	if( bkm_receive( &originator, &type, &msg ) == -1 ) {
		brlog( "Unable to receive message; errno %ld - STOPPING", errno );
		brrc = BRFATAL;
		bkcurrent = ST_CANCELED;
		brstate = BR_CANCEL;
		brdone = TRUE;
		bksuspended = FALSE;
		END_SIGNAL_HANDLER;
		return;
	}

#ifdef TRACE
	brlog( "State machine: enter at state %s: type %s: orig: %ld",
		STNAME( bkcurrent ), brmsgname( type ), originator );
#endif

	/* Now process message according to the state machine */
	switch( bkcurrent ) {
	case ST_NORMAL:
		switch( type ) {
		case RESUME:
			bksuspended = FALSE;
			if( bkm_send( bkdaemonpid, RESUMED, NULL ) == -1 ) {
				brlog( "Unable to send RESUMED message to bkdaemon; errno %ld",
					errno );
				brrc = BRFATAL;
			}
			break;

		case SUSPEND:
			bkcurrent = ST_SUSPENDING;
			brdone = TRUE;
			brrc = BRSUSPENDED;
			brstate = BR_SUSPEND;
			break;

		case CANCEL:
			bkcurrent = ST_CANCELING;
			brdone = TRUE;
			brrc = BRCANCELED;
			brstate = BR_CANCEL;
			break;

		default:
			brlog( "received unexpected message type %s - ignored", brmsgname( type ) );
			break;
		}
		break;

	case ST_GET_VOLUME:
		switch( type ) {
		case RESUME:
			bksuspended = FALSE;
			if( bkm_send( bkdaemonpid, RESUMED, NULL ) == -1 ) {
				brlog( "Unable to send RESUMED message to bkdaemon; errno %ld",
					errno );
				brrc = BRFATAL;
			}
			break;

		case VOLUME:
			(void) strcpy( brresult, msg.volume.label );
			brdone = TRUE;
			brstate = BR_PROCEED;
			bkcurrent = ST_NORMAL;
			brrc = BRSUCCESS;
			break;

		case FAILED:
			brdone = TRUE;
			brrc = BRFAILED;
			bkcurrent = ST_NORMAL;
			brstate = BR_PROCEED;
			break;

		case SUSPEND:
			brdone = TRUE;
			brrc = BRSUSPENDED;
			bkcurrent = ST_SUSPENDING;
			brstate = BR_SUSPEND;
			break;

		case CANCEL:
			brdone = TRUE;
			brrc = BRCANCELED;
			bkcurrent = ST_CANCELING;
			brstate = BR_CANCEL;
			break;

		default:
			brlog( "received unexpected message type %s - ignored", brmsgname( type ) );
			break;
		}
		break;

	case ST_SUSPENDING:
		switch( type ) {
		case RESUME:
			bksuspended = FALSE;
			bkcurrent = ST_NORMAL;
			brrc = BRSUCCESS;
			brstate = BR_PROCEED;
			if( bkm_send( bkdaemonpid, RESUMED, NULL ) == -1 ) {
				brlog( "Unable to send RESUMED message to bkdaemon; errno %ld",
					errno );
				brrc = BRFATAL;
			}
			break;

		case CANCEL:
			bksuspended = FALSE;
			brrc = BRCANCELED;
			bkcurrent = ST_CANCELING;
			brstate = BR_CANCEL;
			break;
			
		default:
			brlog( "received unexpected message type %s - ignored", brmsgname( type ) );
			break;
		}
		break;

	case ST_SUSPENDED:
		switch( type ) {
		case RESUME:
			bksuspended = FALSE;
			bkcurrent = ST_NORMAL;
			brrc = BRSUCCESS;
			brstate = BR_PROCEED;
			if( bkm_send( bkdaemonpid, RESUMED, NULL ) == -1 ) {
				brlog( "Unable to send RESUMED message to bkdaemon; errno %ld",
					errno );
				brrc = BRFATAL;
			}
			break;

		case CANCEL:
			bksuspended = FALSE;
			bkcurrent = ST_CANCELING;
			brrc = BRCANCELED;
			break;

		case SUSPEND:
			if( bkm_send( bkdaemonpid, SUSPENDED, NULL ) == -1 )
				brlog( "Unable to send SUSPENDED message to bkdaemon; errno %ld",
					errno );
			break;
			
		default:
			brlog( "received unexpected message type %s - ignored", brmsgname( type ) );
			break;
		}
		break;

	case ST_CANCELING:
		brlog( "received unexpected message type %s - ignored", brmsgname( type ) );
		break;

	case ST_CANCELED:
		switch( type ) {
		case CANCEL:
			if( bkm_send( bkdaemonpid, CANCELED, NULL ) == -1 )
				brlog( "Unable to send CANCELED message to bkdaemon; errno %ld",
					errno );
			break;

		default:
			brlog( "received unexpected message type %s - ignored", brmsgname( type ) );
			break;
		}
		break;

	default:
		brlog( "unknown current state: %d", bkcurrent );
		brrc = BRFATAL;
		brstate = BRFAILED;
		bksuspended = FALSE;
		brdone = TRUE;
	}

#ifdef TRACE
	brlog( "State machine: New state %s", STNAME( bkcurrent ) );
#endif
	END_SIGNAL_HANDLER;
}
