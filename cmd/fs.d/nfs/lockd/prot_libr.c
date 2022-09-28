/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nfs.cmds:nfs/lockd/prot_libr.c	1.6.3.1"
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
	 * prot_libr.c
	 * consists of routines used for initialization, mapping and debugging
	 */

#include <stdio.h>
#include <sys/param.h>
#include <memory.h>
#include "prot_lock.h"
#include "prot_time.h"

char hostname[MAXNAMELEN];		/* for generating oh */
int pid;				/* id for monitor usage */
int host_len;				/* for generating oh */
int lock_len;
int res_len;
int msg_len;
int grace_period;
remote_result res_nolock;
remote_result res_working;
remote_result res_grace;

int cookie;				/* monitonically increasing # */

extern lm_vnode *fh_q;
extern struct filock *sleeplcks;
extern struct reclock *sleeplcks_reclox;
extern msg_entry *msg_q;
extern int debug, used_reclock;

char *xmalloc();

init()
{
	int i;

	(void) gethostname(hostname, MAXNAMELEN);
	/* used to generate owner handle */
	host_len = strlen(hostname) +1;
	msg_len = sizeof (msg_entry);
	lock_len = sizeof (struct reclock);
	res_len = sizeof (remote_result);
	pid = getpid(); /* used to generate return id for status monitor */
	res_nolock.lstat = nolocks;
	res_working.lstat = blocking;
	res_grace.lstat = grace;
	grace_period = LM_GRACE;
	cancel_mon();
}

/*
 * map input (from kenel) to lock manager internal structure
 * returns -1 if cannot allocate memory;
 * returns 0 otherwise
 */
int
map_kernel_klm(a)
	reclock *a;
{
	/*
	 * common code shared between map_kernel_klm and map_klm_nlm
	 * generate op
	 */
	if (a->lck.lox.lld.l_type == F_WRLCK) {
		a->lck.op = LOCK_EX;
	} else if (a->lck.lox.lld.l_type == F_RDLCK) {
		a->lck.op = LOCK_SH;
	}
	if (a->block == FALSE)
		a->lck.op = a->lck.op | LOCK_NB;
	if (a->lck.lox.lld.l_len > MAXLEN) {
		fprintf(stderr, " len(%d) greater than max len(%d)\n",
			a->lck.lox.lld.l_len, MAXLEN);
		a->lck.lox.lld.l_len = MAXLEN;
	}

	/*
	 * generate svid holder
	 */
	if (!a->lck.lox.lld.l_pid)
		a->lck.lox.lld.l_pid = getpid();
	a->lck.svid = a->lck.lox.lld.l_pid;

	/*
	 * owner handle == (hostname, pid);
	 * cannot generate owner handle use obj_alloc
	 * because additioanl pid attached at the end
	 */
	a->lck.oh_len = host_len + sizeof (int);
	if ((a->lck.oh_bytes = xmalloc(a->lck.oh_len) ) == NULL)
		return (-1);
	(void) strcpy(a->lck.oh_bytes, hostname);
	memcpy(&a->lck.oh_bytes[host_len], (char *) &a->lck.lox.lld.l_pid,
		sizeof (int));

	/*
	 * generate cookie
	 * cookie is generated from monitonically increasing #
	 */
	cookie++;
	if (obj_alloc(&a->cookie, (char *) &cookie, sizeof (int))== -1)
		return (-1);

	/*
	 * generate clnt_name
	 */
	if ((a->lck.clnt= xmalloc(host_len)) == NULL)
		return (-1);
	(void) strcpy(a->lck.clnt, hostname);
	a->lck.caller_name = a->lck.clnt; 	/* ptr to same area */
	return (0);
}


/*
 * nlm map input from klm to lock manager internal structure
 * return -1, if cannot allocate memory!
 * returns 0, otherwise
 */
int
map_klm_nlm(a)
	reclock *a;
{
	/*
	 * common code shared between map_kernel_klm and map_klm_nlm
	 * generate op
	 */
	if (a->lck.lox.lld.l_type == F_WRLCK) {
		a->lck.op = LOCK_EX;
	} else if (a->lck.lox.lld.l_type == F_RDLCK) {
		a->lck.op = LOCK_SH;
	}
	if (a->block == FALSE)
		a->lck.op = a->lck.op | LOCK_NB;

	/*
	 * generate svid holder
	 */
	if (!a->lck.lox.lld.l_pid)
		a->lck.lox.lld.l_pid = getpid();
	a->lck.svid = a->lck.lox.lld.l_pid;

	a->lck.l_offset = a->lck.lox.lld.l_start;
	a->lck.l_len = a->lck.lox.lld.l_len;

 	/*
	 * normal klm to nlm calls
	 */
	if ((a->lck.svr = xmalloc(host_len)) == NULL) {
		return (-1);
	}
	(void) strcpy(a->lck.svr, hostname);
	a->lck.clnt = a->lck.caller_name;
	return (0);
}

pr_oh(a)
	netobj *a;
{
	int i;
	int j;
	unsigned p = 0;

	if (a->n_len - sizeof (int) > 4 )
		j = 4;
	else
		j = a->n_len - sizeof (int);

	/*
	 * only print out part of oh
	 */
	for (i = 0; i< j; i++) {
		printf("%c", a->n_bytes[i]);
	}
	for (i = a->n_len - sizeof (int); i< a->n_len; i++) {
		p = (p << 8) | (((unsigned)a->n_bytes[i]) & 0xff);
	}
	printf("%u", p);
}

pr_fh(a)
	netobj *a;
{
	int i;

	for (i = 0; i< a->n_len; i++) {
		printf("%02x", (a->n_bytes[i] & 0xff));
	}
}


pr_lock(a)
	reclock *a;
{
	if (a != NULL) {
		printf("(%x), oh= ", a);
		pr_oh(&a->lck.oh);
		printf(", svr= %s, fh = ", a->lck.svr);
		pr_fh(&a->lck.fh);
		if (a->block)
			printf(" block=TRUE ");
		else
			printf(" block=FALSE ");
		if (a->exclusive)
			printf(" exclusive=TRUE ");
		else
			printf(" exclusive=FALSE ");
		printf(" rel=%d w_flag=%d type=%d pid=%d class=%d granted=%d rsys=%x LockID=%d ",
			a->rel, a->w_flag, a->lck.lox.lld.l_type,
			a->lck.lox.lld.l_pid, a->lck.lox.class, a->lck.lox.granted,
			a->lck.lox.lld.l_sysid, a->lck.lox.LockID);
		printf(", op=%d, ranges= [%d, %d)\n",
 			a->lck.op,
			a->lck.lox.lld.l_start, a->lck.lox.lld.l_start + a->lck.lox.lld.l_len);
	} else {
		printf("RECLOCK is NULL.\n");
	}
}

pr_vfilock(a)
        struct filock *a;
{
        if (a != NULL) {
                printf("(%x), ", a);
                printf(" type=%d, whence=%d, pid=%d, sysid=%x, ",
                        a->set.l_type, a->set.l_whence, a->set.l_pid,
                        a->set.l_sysid);
                printf("ranges= [%d, %d) wakeflg=%d stat.blk.sysid=%d stat.blk.pid=%d\n",
                        a->set.l_start, a->set.l_start + a->set.l_len,
			a->stat.wakeflg, a->stat.blk.sysid, a->stat.blk.pid);
        } else {
                printf("VFILOCK is NULL.\n");
        }
}

pr_all()
{
	struct reclock *tf;
	struct lm_vnode *fl;
	struct filock *ff;
	msg_entry *msgp;
	int i, ii;

	if (debug < 2)
		return;


	/*
	 * print msg queue
	 */
	if (msg_q != NULL) {
		printf("***** MSG QUEUE *****\n");
		msgp= msg_q;
		while (msgp != NULL) {
			printf("(%x) : ", msgp->req);
			printf(" (%x, ", msgp->req);
			if (msgp->reply != NULL)
				printf(" lstat =%d), ", msgp->reply->lstat);
			else
				printf(" NULL), ");
			msgp = msgp->nxt;
		}
		printf("\n");
	}
	else
		printf("***** NO MSG IN MSG QUEUE *****\n");

	/*
	 * print fh_q
	 */
	if (fh_q != NULL) {
		printf("\n***** FILE HANDLE LIST *****");
		for (fl = fh_q; fl; fl = fl->next) {
			if (fl->filocks != NULL) {
                                printf("\n***** V_FILOCKS : %x *****\n",
                                        fl);
                                for (ff = fl->filocks; ff; ff = ff->next) {
                                        pr_vfilock(ff);
                                }
                        } else
                                printf("\n***** NO V_FILOCKS : %x****",
                                        fh_q);
			if (fl->reclox != NULL) {
				printf("\n***** RECLOX LOCKS : %x *****\n", fl);
				for (tf = fl->reclox; tf; tf = tf->next) {
					pr_lock(tf);
				}
			} else
				printf("\n***** NO RECLOX LOCKS : %x\n****",
					fl);
		}
	}
	else
		printf("\n***** NO FILE HANDLE STRUCT ****\n");
	/* 
         * print sleeplcks 
         */ 
        if (sleeplcks != NULL) { 
                printf("\n***** SLEEP LCK LIST *****"); 
                for (ff = sleeplcks; ff; ff = ff->next) { 
                        pr_vfilock(ff); 
		}
	} else
                printf("\n***** NO SLEEPLCKS : %x****\n", sleeplcks); 

	/*
         * print sleeplcks_reclox
         */
        if (sleeplcks_reclox != NULL) {
                printf("\n***** SLEEPLCKS_RECLOX LIST *****");
                for (tf = sleeplcks_reclox; tf; tf = tf->next) {
                        pr_lock(tf);
                }
        } else
                printf("\n***** NO SLEEPLCKS_RECLOX : %x****\n", 
			sleeplcks_reclox);

	printf("used_reclock=%d\n", used_reclock);
	(void) fflush(stdout);
}

up(x)
	int x;
{
	return ((x % 2 == 1) || (x %2 == -1));
}

kill_process(a)
	reclock *a;
{
	fprintf(stderr, "kill process (%d)\n", a->lck.lox.lld.l_pid);
	(void) kill(a->lck.lox.lld.l_pid, SIGLOST);
}
