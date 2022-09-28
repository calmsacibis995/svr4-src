/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nfs.cmds:nfs/lockd/prot_pklm.c	1.8.4.1"
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
	 * prot_pklm.c
	 * consists of all procedures called by klm_prog
	 */

#include <stdio.h>
#include "prot_lock.h"

#define DEBUG_ON        57
#define DEBUG_OFF       58
#define KILL_IT         59
 
int			tmp_ck;         /* flag to trigger tmp check */
int			klm;
FILE			*fp;
SVCXPRT 		*klm_transp;	/* export klm transport handle */

extern int 		grace_period, LockID;
extern int 		lock_len, res_len;
 
extern remote_result 	*get_res();
extern reclock 		*get_reclock();
 
extern remote_result 	res_nolock;
extern remote_result 	res_working;
extern remote_result 	res_grace;

extern int 		debug;
extern msg_entry 	*klm_msg;	/* record last klm msg */
extern msg_entry 	*msg_q;
extern msg_entry 	*retransmitted();
extern msg_entry	*queue();
extern reclock 		*search_block_lock();
extern klm_testrply 	*get_klm_testrply();
extern klm_stat 	*get_klm_stat();
extern bool_t           remote_data();

remote_result 		*remote_test();
remote_result 		*remote_lock();
remote_result 		*remote_unlock();
remote_result 		*remote_cancel();

void 			klm_lockargstoreclock();
void 			klm_unlockargstoreclock();
void 			klm_testargstoreclock();

proc_klm_test(a)
	reclock *a;
{
	klm_msg_routine(a, KLM_TEST, remote_test);
}

proc_klm_lock(a)
	reclock *a;
{
	klm_msg_routine(a, KLM_LOCK, remote_lock);
}

proc_klm_cancel(a)
	reclock *a;
{
	tmp_ck = 1;
	klm_msg_routine(a, KLM_CANCEL, remote_cancel);
}

proc_klm_unlock(a)
	reclock *a;
{
	klm_msg_routine(a, KLM_UNLOCK, remote_unlock);
}

/*
 * common routine to handle msg passing form of communication;
 * klm_msg_routine is shared among all klm procedures:
 * proc_klm_test, proc_klm_lock, proc_klm_cancel, proc_klm_unlock;
 * proc specifies the name of the routine to branch to for reply purpose;
 * local and remote specify the name of routine that handles the call
 *
 * when a msg arrives, it is first checked to see
 *   if retransmitted;
 *	 if a reply is ready,
 *	   a reply is sent back and msg is erased from the queue
 *	 or msg is ignored!
 *   else if this is a new msg;
 *	 if data is remote
 *		a rpc request is send and msg is put into msg_queue,
 *	 else (request lock or similar lock)
 *		reply is sent back immediately.
 */
klm_msg_routine(a, proc, remote)
	reclock *a;
	int proc;
	remote_result *(*remote)();
{
	struct msg_entry *msgp;
	remote_result *result;
	reclock *nl, *reqp;

	if (debug) {
		printf("\nenter klm_msg_routine(proc =%d): op=%d, (%d, %d) by ",
			 proc, a->lck.op, a->lck.lox.lld.l_start, a->lck.lox.lld.l_len);
		pr_oh(&a->lck.oh);
		printf("\n");
		pr_lock(a);
		(void) fflush(stdout);
	}

	if ((msgp = retransmitted(a, proc)) != NULL) { /* retransmitted msg */
		if (debug)
			printf("retransmitted msg!\n");
		a->rel = 1;
		if (msgp->reply == NULL) {
			klm_msg = msgp;	/* record last received klm msg */
			return;
		}
		else {
			if (msgp->reply->lstat != blocking) {
				klm_reply(proc, msgp->reply);
				dequeue(msgp);
			}
			else {
				klm_msg = msgp;
			}
			return;
		}
	}
	else {
		/* tmp check inconsistency of process! */
		msgp = msg_q;
		while (msgp != NULL) {
			reqp = msgp->req;
			if (tmp_ck == 0 && proc != KLM_CANCEL &&
			    msgp->proc != NLM_LOCK_RECLAIM  &&
			    same_proc(reqp, a)) {
				fprintf(stderr, "*****warning:*******process issues request %x (proc = %d) before obtaining response for %x\n",
				 a, proc, msgp->req);
			}
			msgp = msgp ->nxt;
		}

		if (proc == KLM_LOCK && !remote_data(a) &&
		    (nl = search_block_lock(a)) != NULL) {
			/*
			 * set up entry in msg queue
			 * retransmitted local block lock
			 */
			if (debug)
				printf("retransmitted local block lock (%x)\n, nl");
			a->rel = 1;
			msgp = queue(nl, KLM_LOCK);
			/*
			 * o.k., if queue returns NULL;
			 */
			klm_msg = msgp;
			return;
		}
	}

	result = remote(a, MSG); /* specify msg passing type of comm */
	if ((result == NULL) && (proc == KLM_CANCEL)) {
		if ((result = get_res()) != NULL)
			result->lstat = nlm_granted;
	}
	if (result != NULL) {
		klm_reply(proc, result);
	}
	if (debug)
		printf("klm_msg_routine() exiting, proc = %d\n", proc);
}

/*
 * klm_reply send back reply from klm to requestor(kernel):
 * proc specify the name of the procedure return the call;
 * corresponding xdr routines are then used;
 */
klm_reply(proc, reply)
	int proc;
	remote_result *reply;
{
	bool_t (*xdr_reply)();
	int oldmask;
	klm_testrply *args;
	klm_stat *stat_args;

	oldmask = sigblock ( 1 <<( SIGALRM-1));

	switch (proc) {
	case KLM_TEST:
	case NLM_TEST_MSG:	/* record in msgp->proc */
		xdr_reply = xdr_klm_testrply;
		if ((args = get_klm_testrply()) == NULL) {
			printf("rpc.lockd: unable to allocate space for klm_testrply.\n");
			(void) sigsetmask(oldmask);
			return;
		}
		if (reply->lstat == nlm_granted) {
			args->stat = klm_granted;
		} else {
			if (reply->lstat == nlm_denied)
				args->stat = klm_denied;
			else if (reply->lstat == nlm_denied_nolocks)
				args->stat = klm_denied_nolocks;
			else if (reply->lstat == nlm_blocked)
				args->stat = klm_working;
			args->klm_testrply_u.holder.pid =
				reply->stat.nlm_testrply_u.holder.svid;
			args->klm_testrply_u.holder.base =
				reply->stat.nlm_testrply_u.holder.l_offset;
			args->klm_testrply_u.holder.length =
				reply->stat.nlm_testrply_u.holder.l_len;
			if (reply->stat.nlm_testrply_u.holder.exclusive)
				args->klm_testrply_u.holder.exclusive = TRUE;
			else
				args->klm_testrply_u.holder.exclusive = FALSE;
		}
		if (debug) {
			printf("KLM_REPLY : svid=%d l_offset=%d l_len=%d\n",
				args->klm_testrply_u.holder.pid,
				args->klm_testrply_u.holder.base,
				args->klm_testrply_u.holder.length);
		}
		if (!svc_sendreply(klm_transp, xdr_reply, args))
			svcerr_systemerr(klm_transp);
		break;
	case KLM_LOCK:
	case NLM_LOCK_MSG:
	case NLM_LOCK_RECLAIM:
	case KLM_CANCEL:
	case NLM_CANCEL_MSG:
	case KLM_UNLOCK:
	case NLM_UNLOCK_MSG:
		xdr_reply = xdr_klm_stat;
		if ((stat_args = get_klm_stat()) == NULL) {
			printf("rpc.lockd: unable to allocate space for klm_stat.\n");
			(void) sigsetmask(oldmask);
			return;
		}
		if (reply->lstat == nlm_denied)
			stat_args->stat = klm_denied;
		else if (reply->lstat == nlm_granted)
			stat_args->stat = klm_granted;
		else if (reply->lstat == nlm_denied_nolocks)
			stat_args->stat = klm_denied_nolocks;
		else if (reply->lstat == nlm_deadlck)
			stat_args->stat = klm_deadlck;
		else if (reply->lstat == nlm_blocked)
			stat_args->stat = klm_working;
		if (debug) {
			printf("KLM_REPLY : stat=%d\n",
				stat_args->stat);
		}
		if (!svc_sendreply(klm_transp, xdr_reply, stat_args))
			svcerr_systemerr(klm_transp);
		break;
	default:
		xdr_reply = xdr_void;
		printf("unknown klm_reply proc(%d)\n", proc);
		if (!svc_sendreply(klm_transp, xdr_reply, &reply->stat))
			svcerr_systemerr(klm_transp);
	}
	if (debug)
		printf("klm_reply: stat=%d\n", reply->lstat);
	(void) sigsetmask(oldmask);
	return;
}

void
klm_lockargstoreclock(from, to)
	struct klm_lockargs *from;
	struct reclock *to;
{
	to->lck.server_name = from->alock.server_name;
	if (from->alock.fh.n_len)
		obj_copy(&to->lck.fh, &from->alock.fh);
	else
		to->lck.fh.n_len = 0;
	to->lck.lox.lld.l_start = from->alock.base;
	if (debug)
		printf("klm_lockargstoreclock(): from->alock.length=%d\n",from->alock.length);
	if (from->alock.length == 0 || from->alock.length == 0x7fffffff) 
		to->lck.lox.lld.l_len = 0x7fffffff - from->alock.base;
	else
		to->lck.lox.lld.l_len = from->alock.length;

	if (from->exclusive) {
		to->lck.lox.lld.l_type = F_WRLCK;
		to->exclusive = TRUE;
	} else {
		to->lck.lox.lld.l_type = F_RDLCK;
		to->exclusive = FALSE;
	}
	if (from->block) {
		to->block = TRUE;
	} else {
		to->block = FALSE;
	}
	to->lck.lox.granted = 0;
	to->lck.lox.color = 0;
	to->lck.lox.LockID = 0;
	to->lck.lox.lld.l_pid = from->alock.pid;
	to->lck.lox.class = 0;
	to->cookie.n_len = from->alock.pid;
	to->lck.lox.lld.l_sysid = 0;
}

void
klm_unlockargstoreclock(from, to)
	struct klm_unlockargs *from;
	struct reclock *to;
{
	to->lck.server_name = from->alock.server_name;
	if (from->alock.fh.n_len)
		obj_copy(&to->lck.fh, &from->alock.fh);
	else
		to->lck.fh.n_len = 0;
	to->lck.lox.lld.l_start = from->alock.base;
	if (from->alock.length == 0 || from->alock.length == 0x7fffffff) 
		to->lck.lox.lld.l_len = 0x7fffffff - from->alock.base;
	else
		to->lck.lox.lld.l_len = from->alock.length;
	to->lck.lox.granted = 0;
	to->lck.lox.color = 0;
	to->lck.lox.LockID = 0;
	to->lck.lox.lld.l_pid = from->alock.pid;
	to->lck.lox.class = 0;
	to->lck.lox.lld.l_sysid = 0;
	to->cookie.n_len = from->alock.pid;
}

void
klm_testargstoreclock(from, to)
	struct klm_testargs *from;
	struct reclock *to;
{
	to->lck.server_name = from->alock.server_name;
	if (from->alock.fh.n_len)
		obj_copy(&to->lck.fh, &from->alock.fh);
	else
		to->lck.fh.n_len = 0;
	to->lck.lox.lld.l_start = from->alock.base;
	if (from->alock.length == 0 || from->alock.length == 0x7fffffff) 
		to->lck.lox.lld.l_len = 0x7fffffff - from->alock.base;
	else
		to->lck.lox.lld.l_len = from->alock.length;
	if (from->exclusive) {
		to->lck.lox.lld.l_type = F_WRLCK;
		to->exclusive = TRUE;
	} else {
		to->lck.lox.lld.l_type = F_RDLCK;
		to->exclusive = FALSE;
	}
	to->block = TRUE;
	to->lck.lox.granted = 0;
	to->lck.lox.color = 0;
	to->lck.lox.LockID = 0;
	to->lck.lox.lld.l_pid = from->alock.pid;
	to->lck.lox.class = 0;
	to->lck.lox.lld.l_sysid = 0;
	to->cookie.n_len = from->alock.pid;
}

void
klm_prog(Rqstp, Transp)
	struct svc_req *Rqstp;
	SVCXPRT *Transp;
{
	int			oldmask;
	char			*klm_req;
	struct reclock		*req;
	msg_entry		*msgp;
	bool_t 			(*xdr_Argument)(), (*xdr_Result)();
	char 			*(*Local)();
	klm_lockargs		*klm_lockargs_a;
	klm_unlockargs		*klm_unlockargs_a;
	klm_testargs		*klm_testargs_a;

	extern klm_lockargs 	*get_klm_lockargs();
	extern klm_unlockargs 	*get_klm_unlockargs();
	extern klm_testargs 	*get_klm_testargs();

	if (debug)
		printf("KLM_PROG+++ version %d proc %d\n",
			Rqstp->rq_vers, Rqstp->rq_proc);

	oldmask = sigblock (1 << (SIGALRM -1));
	klm_transp = Transp;
	klm_msg = NULL;

	switch (Rqstp->rq_proc) {
	case NULLPROC:
		svc_sendreply(Transp, xdr_void, NULL);
		(void) sigsetmask(oldmask);
		return;

	case DEBUG_ON:
		debug = 2;
		svc_sendreply(Transp, xdr_void, NULL);
		(void) sigsetmask(oldmask);
		return;

	case DEBUG_OFF:
		debug = 0;
		svc_sendreply(Transp, xdr_void, NULL);
		(void) sigsetmask(oldmask);
		return;

	case KILL_IT:
		svc_sendreply(Transp, xdr_void, NULL);
		fprintf(stderr, "rpc.lockd killed upon request\n");
		exit(2);

	case KLM_TEST:
		xdr_Argument = xdr_klm_testargs;
		xdr_Result = xdr_klm_testrply;
		Local = (char *(*)()) proc_klm_test;
		if ((klm_testargs_a = get_klm_testargs()) == NULL) {
			printf("rpc.lockd: unable to allocate space for klm_req.\n");
			(void) sigsetmask(oldmask);
			return;
		}
		klm_req = (char *) klm_testargs_a;
		break;

	case KLM_LOCK:
		xdr_Argument = xdr_klm_lockargs;
		xdr_Result = xdr_klm_stat;
		Local = (char *(*)()) proc_klm_lock;
		if ((klm_lockargs_a = get_klm_lockargs()) == NULL) {
			printf("rpc.lockd: unable to allocate space for klm_req.\n");
			(void) sigsetmask(oldmask);
			return;
		}
		klm_req = (char *) klm_lockargs_a;
		break;

	case KLM_CANCEL:
		xdr_Argument = xdr_klm_lockargs;
		xdr_Result = xdr_klm_stat;
		Local = (char *(*)()) proc_klm_cancel;
		if ((klm_lockargs_a = get_klm_lockargs()) == NULL) {
			printf("rpc.lockd: unable to allocate space for klm_req.\n");
			(void) sigsetmask(oldmask);
			return;
		}
		klm_req = (char *) klm_lockargs_a;
		break;

	case KLM_UNLOCK:
		xdr_Argument = xdr_klm_unlockargs;
		xdr_Result = xdr_klm_stat;
		Local = (char *(*)()) proc_klm_unlock;
		if ((klm_unlockargs_a = get_klm_unlockargs()) == NULL) {
			printf("rpc.lockd: unable to allocate space for klm_req.\n");
			(void) sigsetmask(oldmask);
			return;
		}
		klm_req = (char *) klm_unlockargs_a;
		break;

	default:
		svcerr_noproc(Transp);
		(void) sigsetmask(oldmask);
		return;
	}

	if ((req = get_reclock()) != NULL) {
		if (!svc_getargs(Transp, xdr_Argument, klm_req)) {
			svcerr_decode(Transp);
			release_reclock(req);
			(void) sigsetmask(oldmask);
			return;
		}
		switch (Rqstp->rq_proc) {
		case KLM_TEST:
			klm_testargstoreclock((struct klm_testargs *)klm_req, req);
			break;
		case KLM_LOCK:
		case KLM_CANCEL:
			klm_lockargstoreclock((klm_lockargs *)klm_req, req);
			break;
		case KLM_UNLOCK:
			klm_unlockargstoreclock((struct klm_unlockargs *)klm_req, req);
			break;
		}
		req->lck.lox.LockID = LockID++;

		if (debug == 3){
			if (fwrite(&klm, sizeof (int), 1, fp) == 0)
				fprintf(stderr, "fwrite klm error\n");
			if (fwrite(&Rqstp->rq_proc, sizeof (int), 1, fp) == 0)
				fprintf(stderr, "fwrite klm_proc error\n");
			(void) fflush(fp);
		}

		if (map_kernel_klm(req) == -1)
			goto abnormal;
		if (grace_period > 0 && !(req->reclaim)) {
			/*
			 * put msg in queue and delay reply, unless there is
			 * no queue space
			 */
			if (debug)
				printf("during grace period, please retry later\n");
			if ((msgp = queue(req, (int) Rqstp->rq_proc)) == NULL) {
				klm_reply(Rqstp->rq_proc, &res_working);
				req->rel = 1;
				release_reclock(req);
				switch (Rqstp->rq_proc) {
				case KLM_TEST:
					release_klm_testargs(klm_req);
					break;
				case KLM_LOCK:
				case KLM_CANCEL:
					release_klm_lockargs(klm_req);
					break;
				case KLM_UNLOCK:
					release_klm_unlockargs(klm_req);
					break;
				}
				(void) sigsetmask(oldmask);
				return;
			}
			if (debug)
				pr_all();
			req->rel = 1;
			release_reclock(req);
			switch (Rqstp->rq_proc) {
			case KLM_TEST:
				release_klm_testargs(klm_req);
				break;
			case KLM_LOCK:
			case KLM_CANCEL:
				release_klm_lockargs(klm_req);
				break;
			case KLM_UNLOCK:
				release_klm_unlockargs(klm_req);
				break;
			}
			klm_msg = msgp;
			(void) sigsetmask(oldmask);
			if (debug)
				printf("klm_msg=%x \n", klm_msg->req);
			return;
		}
		if (grace_period >0 && debug)
			printf("accept reclaim request\n");
		if (Rqstp->rq_proc == KLM_LOCK)
			if (add_mon(req, 1) == -1) {
				req->rel = 1;
				release_reclock(req);
				switch (Rqstp->rq_proc) {
				case KLM_TEST:
					release_klm_testargs(klm_req);
					break;
				case KLM_LOCK:
				case KLM_CANCEL:
					release_klm_lockargs(klm_req);
					break;
				case KLM_UNLOCK:
					release_klm_unlockargs(klm_req);
					break;
				}
				fprintf(stderr,
					"req discard due status monitor problem\n");
				(void) sigsetmask(oldmask);
				return;
			}

		(*Local)(req);
		release_reclock(req);
	}
	else { /* malloc failure */
		klm_reply((int) Rqstp->rq_proc, &res_nolock);
	}
	(void) sigsetmask(oldmask);
	if (debug)
		printf("EXITING FROM KLM_PROG....\n");
	return;

abnormal:

	klm_reply((int) Rqstp->rq_proc, &res_nolock);
	req->rel = 1;
	release_reclock(req);
	switch (Rqstp->rq_proc) {
	case KLM_TEST:
		release_klm_testargs(klm_req);
		break;
	case KLM_LOCK:
	case KLM_CANCEL:
		release_klm_lockargs(klm_req);
		break;
	case KLM_UNLOCK:
		release_klm_unlockargs(klm_req);
		break;
	}
	(void) sigsetmask(oldmask);
	return;
}
