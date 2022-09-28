/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nfs.cmds:nfs/lockd/prot_msg.c	1.4.3.1"
/*
 *  		PROPRIETARY NOTICE (Combined)
 *  
 *  This source code is unpublished proprietary information
 *  constituting, or derived under license from AT&T's Unix(r) System V.
 *  In addition, portions of such source code were derived from Berkeley
 *  4.3 BSD under license from the Regents of the University of
 *  California.
 *  
 *  
 *  
 *  		Copyright Notice 
 *  
 *  Notice of copyright on this source code product does not indicate 
 *  publication.
 *  
 *  	(c) 1986,1987,1988,1989  Sun Microsystems, Inc.
 *  	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *  	          All rights reserved.
 */
	/*
	 * prot_msg.c
	 * consists all routines handle msg passing
	 */

#include <memory.h>
#include "prot_lock.h"
#include "prot_time.h"

extern int debug;
extern int grace_period;
extern int msg_len;
extern remote_result res_working;
char *xmalloc();
void xtimer();

msg_entry *klm_msg;	/* ptr last msg to klm in msg queue */
msg_entry *msg_q;	/* head of msg queue */

/*
 * retransmitted search through msg_queue to determine if "a" is
 * retransmission of a previously received msg;
 * it returns the addr of the msg entry if "a" is found
 * otherwise, it returns NULL
 */
msg_entry *
retransmitted(a, proc)
	struct reclock *a;
	int proc;
{
	msg_entry *msgp;

	msgp = msg_q;
	while (msgp != NULL) {
		if (same_lock(msgp->req, a) || simi_lock(msgp->req, a)) {
			/*
			 * 5 is the constant diff between rpc calls and msg
			 * passing
			 */
			if ((msgp->proc == NLM_LOCK_RECLAIM &&
			    (proc == KLM_LOCK || proc == NLM_LOCK_MSG)) ||
				msgp->proc == proc + 5 || msgp->proc == proc)
				return (msgp);
		}
		msgp = msgp->nxt;
	}
	return (NULL);
}

/*
 * match response's cookie with msg req
 * either return msgp or NULL if not found
 */
msg_entry *
search_msg(resp)
	remote_result *resp;
{
	msg_entry *msgp;
	struct reclock *req;

	msgp = msg_q;
	while (msgp != NULL) {
		req = msgp->req;
		if (obj_cmp(&req->cookie, &resp->cookie))
			return (msgp);
		msgp = msgp->nxt;
	}
	return (NULL);
}


/*
 * add a to msg queue; called from nlm_call: when rpc call is succ and reply is needed
 * proc is needed for sending back reply later
 * if case of error, NULL is returned;
 * otherwise, the msg entry is returned
 */
msg_entry *
queue(a, proc)
	struct reclock *a;
	int proc;
{
	msg_entry *msgp;

	if ((msgp = (msg_entry *) xmalloc(msg_len)) == NULL)
		return (NULL);
	(void) memset((char *) msgp, 0, msg_len);
	msgp->req = a;
	msgp->proc = proc;
	msgp->t.exp = 1;

	/* insert msg into msg queue */
	if (msg_q == NULL) {
		msgp->nxt = msgp->prev = NULL;
		msg_q = msgp;
		/* turn on alarm only when there are msgs in msg queue */
		if (grace_period == 0)
			(void) alarm(LM_TIMEOUT);
	}
	else {
		msgp->nxt = msg_q;
		msgp->prev = NULL;
		msg_q->prev = msgp;
		msg_q = msgp;
	}

	if ( proc != NLM_LOCK_RECLAIM )
		klm_msg = msgp;			/* record last msg to klm */
	return (msgp);
}

/*
 * dequeue remove msg from msg_queue;
 * and deallocate space obtained  from malloc
 * lockreq is release only if a->rel == 1;
 */
dequeue(msgp)
	msg_entry *msgp;
{
	/*
	 * First, delete msg from msg queue since dequeue(),
	 * FREELOCK() and dequeue_lock() are recursive.
	 */
	if (msgp->prev != NULL)
		msgp->prev->nxt = msgp->nxt;
	else
		msg_q = msgp->nxt;
	if (msgp->nxt != NULL)
		msgp->nxt->prev = msgp->prev;

	if (msgp->req != NULL)
		release_reclock(msgp->req);
	if (msgp->reply != NULL)
		release_res(msgp->reply);

	(void) memset((char *) msgp, 0, sizeof (*msgp));
	free((char *) msgp);
}

/*
 * Find a reclock and dequeue it.  But do not actually free reclock here.
 */
void
dequeue_lock(a)
	struct reclock *a;
{
	msg_entry *msgp;

	msgp = msg_q;
	while (msgp != NULL) {
		if (a == msgp->req) {
			msgp->req = NULL;  /* don't free here; caller does it */
			dequeue(msgp);
			dequeue_lock(a); /* is there another ? */
			return;
		}
		msgp = msgp->nxt;
	}
}

/*
 * if resp is not NULL, add reply to msg_entyr and reply if msg is last req;
 * otherwise, reply working
 */
add_reply(msgp, resp )
	msg_entry *msgp;
	remote_result *resp;
{
	if (debug)
		printf("enter add_reply ...\n");

	if ( resp != NULL) {
		msgp->t.curr = 0; /* reset timer counter to record old msg */
		msgp->reply = resp;
		if (debug) {
			if (klm_msg && klm_msg->req)
				printf("klm_msg->req=%x\n", klm_msg->req);
			if (msgp && msgp->req)
				printf("msgp->req=%x\n", msgp->req);
		}
		if (klm_msg == msgp) { /* reply immed */
			klm_reply(msgp->proc, resp);
			/*
			 * prevent timer routine reply "working" to already
			 * replied req
			 */
			klm_msg = NULL;
			if (resp->lstat != blocking)
				dequeue(msgp);
		}
	} else {	/* res == NULL, used by xtimer */
		if (debug)
			printf(" RESP == NULL  klm_msg->req=%x msgp->req=%x\n",
				klm_msg->req, msgp->req);
		if (klm_msg == msgp) {
			if (debug)
				printf("xtimer reply to (%x): ", msgp->req);
			klm_reply(msgp->proc, &res_working);
			klm_msg = NULL;
			dequeue(msgp);
		}
	}
}

/*
 * signal handler:
 * wake up periodically to check retransmiting status and reply to last req
 */
void
xtimer()
{
	msg_entry *msgp, *next;

	if (debug)
		printf("\nalarm! enter xtimer:\n");

	(void) signal(SIGALRM, SIG_IGN);
	if (grace_period > 0) {
		/* reduce the remaining grace period */
		grace_period--;
		if (grace_period == 0) {
			if (debug) {
				printf("**********end of grace period\n");
				pr_all();
			}
			/* remove proc == klm_xxx in msg queue */
			next = msg_q;
			while ((msgp = next) != NULL) {
				next = msgp->nxt;
				if (msgp->proc == KLM_LOCK ||
				    msgp->proc == KLM_UNLOCK ||
				    msgp->proc == KLM_TEST ||
				    msgp->proc == KLM_CANCEL) {
					if (debug)
						printf("remove grace period msg (%x) from msg queue\n", msgp);
					dequeue(msgp);
				}
			}
		}
	}

	next = msg_q;
	while ((msgp = next) != NULL) {
		next = msgp->nxt;
		if (msgp->reply == NULL) { /* check for retransimssion */
			if (msgp->proc != KLM_LOCK) {
				/* KLM_LOCK is for local blocked locks */
				if (msgp->t.exp == msgp->t.curr) {
					/* retransmit */
					if (debug)
						printf("xtimer retransmit: ");
					(void) nlm_call(msgp->proc, msgp->req, 1);
					msgp->t.curr = 0;
					msgp->t.exp = 2 * msgp->t.exp;
					/* double timeout period */
					if (msgp->t.exp > MAX_LM_TIMEOUT_COUNT){
						msgp->t.exp = MAX_LM_TIMEOUT_COUNT;
					}
				}
				else 	/* increment current count */
					msgp->t.curr++;
			}
		}
		else { /* check if reply is sitting there too long */
			if (msgp->reply->lstat != blocking) {
				klm_reply(msgp->proc, msgp->reply);
/*
				if (msgp->t.curr > OLDMSG)
					dequeue(msgp);
				else
					msgp->t.curr++;
*/
				dequeue(msgp);
			}
		}
	}

	/* send back working reply for last req received in klm */
	if (klm_msg != NULL && klm_msg->req != 0) {
		if (debug)
			printf("klm_msg = %x\n", klm_msg->req);
/*
		klm_reply(klm_msg->proc, &res_working);
		dequeue(klm_msg);
*/
		add_reply(klm_msg, (remote_result *)NULL);
	} else if (debug)
		printf("klm_msg == NULL\n");
	(void) signal(SIGALRM, xtimer);
	if (grace_period != 0 || msg_q != NULL) {
		(void) alarm(LM_TIMEOUT);
	}
}
