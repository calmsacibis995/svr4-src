/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nfs.cmds:nfs/lockd/prot_proc.c	1.16.4.1"
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
	 * prot_proc.c
	 * consists all local, remote, and continuation routines:
	 * local_xxx, remote_xxx, and cont_xxx.
	 */

#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/errno.h>
#include <memory.h>
#include "prot_lock.h"
#include <nfs/nfs.h>
#include <nfs/nfssys.h>

remote_result nlm_result;		/* local nlm result */
remote_result *nlm_resp = &nlm_result;	/* ptr to klm result */

remote_result *remote_cancel();
remote_result *remote_lock();
remote_result *remote_test();
remote_result *remote_unlock();
remote_result *local_test();
remote_result *local_lock();
remote_result *local_cancel();
remote_result *local_unlock();
remote_result *cont_test();
remote_result *cont_lock();
remote_result *cont_unlock();
remote_result *cont_cancel();
remote_result *cont_reclaim();

msg_entry *search_msg();
msg_entry *retransmitted();
struct lm_vnode *search_fh();
struct filock *blocked(), *insflck();
struct reclock *blocked_reclox(), *insflck_reclox();

extern int debug, errno;
extern int res_len;
extern lm_vnode *fh_q;
extern msg_entry *msg_q;
extern msg_entry *klm_msg;
extern struct filock  *sleeplcks;	   /* head of chain of sleeping locks */
extern struct reclock *sleeplcks_reclox;   /* head of chain of sleeping locks */
extern bool_t obj_cmp();
extern int obj_copy();

#define FDTABLE	1000

struct {
	netobj fh;
	int fd;
} fd_table[FDTABLE];

int used_fd;

print_fdtable()
{
	int i, ii;

	if (debug)
		printf("In print_fdtable()....used_fd=%d\n", used_fd);

	for (i=0; i < FDTABLE; i++) {
		if (fd_table[i].fd) {
			printf("%d : ID=%d\n", i, fd_table[i].fd);
			for (ii = 0; ii < fd_table[i].fh.n_len; ii++) {
				printf("%02x",
					(fd_table[i].fh.n_bytes[ii] & 0xff));
			}
			printf("\n");
		}
	}
}

remove_fd(a)
	struct reclock *a;
{
	int i, ii;

	if (debug)
		printf("In remove_fd() ...\n");

	for (i=0; i < FDTABLE; i++) {
		if (fd_table[i].fh.n_len &&
			obj_cmp(&fd_table[i].fh, &a->lck.fh) &&
			fd_table[i].fd) {
			if (debug) {
				for (ii = 0; ii < a->lck.fh.n_len; ii++) {
					printf("%02x",
						(a->lck.fh.n_bytes[ii] & 0xff));
				}
				printf("\n");
			}
			(void) memset(fd_table[i].fh.n_bytes, 0,
				sizeof (fd_table[i].fh.n_bytes));
			fd_table[i].fh.n_len = 0;
			fd_table[i].fd = 0;
			used_fd--;
			break;
		}
	}
	if (debug)
		print_fdtable();
}

int
get_fd(a)
	struct reclock *a;
{
	int fd, cmd, i, ii;
	struct {
		char    *fh;
		int	filemode;
		int	*fd;
	} fa;

	if (debug)
		printf("enter get_fd ....\n");

	for (i=0; i < FDTABLE; i++) {
		if (obj_cmp(&(fd_table[i].fh), &(a->lck.fh)) &&
			fd_table[i].fd) {
			if (debug) {
				printf("Found fd entry : a = ");
				for (ii = 0; ii < a->lck.fh.n_len; ii++) {
					printf("%02x",
						(a->lck.fh.n_bytes[ii] & 0xff));
				}
				printf("\nfd_table[i].fh = ");
				for (ii = 0; ii < 32; ii++) {
					printf("%02x",
						(fd_table[i].fh.n_bytes[ii] & 0xff));
				}
				printf("\n");
			}
			return (fd_table[i].fd);
		}
	}
	/*
	 * convert fh to fd
	 */
	cmd = NFS_CNVT;
	fa.fh = a->lck.fh.n_bytes;
	if (debug) {
		printf("Convert fd entry : ");
		for (i = 0; i < a->lck.fh.n_len; i++) {
			printf("%02x", (a->lck.fh.n_bytes[i] & 0xff));
		}
		printf("\n");
	}
	fa.filemode = O_RDWR;
	fa.fd = &fd;
	if ((i = _nfssys(cmd, &fa)) == -1) {
		perror("fcntl");
		printf("rpc.lockd: unable to do cnvt.\n");
		if (errno == ENOLCK)
			return (-1);
		else
			return (-2);
	}

	if (debug)
		printf("_nfssys returns fd %d\n", fd);

	for (i=0; i < FDTABLE; i++) {
		if (!fd_table[i].fh.n_len) {
			obj_copy(&fd_table[i].fh, &a->lck.fh);
			fd_table[i].fd = fd;
			used_fd++;
			break;
		}
	}

	if (debug)
		print_fdtable();

	return (fd_table[i].fd);
}

remote_result *
local_lock(a)
	struct reclock *a;
{
	int fd, err, cmd;
	struct flock fld;

	if (debug)
		printf("enter local_lock()...\n");

	/*
	 * convert fh to fd
	 */
	if ((fd = get_fd(a)) < 0) {
		if (fd == -1)
			nlm_resp->lstat = nlm_denied_nolocks;
		else
			nlm_resp->lstat = denied;
		return (nlm_resp);
	}

	/*
	 * set the lock
	 */
	if (debug) {
		printf("enter local_lock...FD=%d\n", fd);
		pr_lock(a);
		(void) fflush(stdout);
	}
	if (a->block)
		cmd = F_RSETLKW;
	else
		cmd = F_RSETLK;
	if (a->exclusive)
		fld.l_type = F_WRLCK;
	else
		fld.l_type = F_RDLCK;
	fld.l_whence = 0;
	fld.l_start = a->lck.lox.lld.l_start;
	fld.l_len = a->lck.lox.lld.l_len;
	fld.l_pid = a->lck.lox.lld.l_pid;
	fld.l_sysid = a->lck.lox.lld.l_sysid;
	if (debug) {
		printf("fld.l_start=%d fld.l_len=%d fld.l_sysid=%x\n",
			fld.l_start, fld.l_len, fld.l_sysid);
	}
	if ((err = fcntl(fd, cmd, &fld)) == -1) {
		if (errno != EINTR) 
			perror("fcntl");
		if (errno == EINTR) {
			nlm_resp->lstat = blocking;
			a->w_flag = 1;
		} else if (errno == EDEADLK) {
			nlm_resp->lstat = deadlck;
			a->w_flag = 0;
		} else if (errno == ENOLCK) {
			printf("rpc.lockd: out of lock.\n");
			nlm_resp->lstat = nlm_denied_nolocks;
		} else {
			printf("rpc.lockd: unable to set a lock. \n");
			nlm_resp->lstat = denied;
		}
	} else {
		nlm_resp->lstat = nlm_granted;
	}
	return (nlm_resp);
}

/*
 * choice == RPC; rpc calls to remote;
 * choice == MSG; msg passing calls to remote;
 */
remote_result *
remote_lock(a, choice)
	struct reclock *a;
	int choice;
{
	struct lm_vnode *fp;
	msg_entry *msgp;
	struct filock  *sf, *ff, *found, *insrt = NULL;
	struct reclock  *sr;
        int i, retval, sleeping;

	if (debug) {
		printf("enter remote_lock\n");
		pr_lock(a);
		(void) fflush(stdout);
	}

	/*
	 * Create/get fh entry
	 */
	if ((fp = search_fh(a, 1)) == (struct lm_vnode *)-1) {
		a->lck.lox.granted = 1;
		a->rel = 1;
		nlm_resp->lstat = nlm_granted;
		return (nlm_resp);
	} else {
		/*
		 * Check deadlock condition
		 */
		if ((fp->filocks == NULL) || (fp->filocks &&
			(found = blocked(fp->filocks, &(a->lck.lox.lld), 
				&insrt)) == NULL)) {
			a->lck.lox.granted = 0;
                        if (choice == MSG) {
                                if (nlm_call(NLM_LOCK_MSG, a, 0) == -1)
                                        a->rel = 1;
                        } else {
                                printf("rpc not supported\n");
                                a->rel = 1;
                        }
		} else {
			if (!a->block) {
				a->lck.lox.granted = 0;
				a->rel = 1;
				nlm_resp->lstat = nlm_denied;
				return (nlm_resp);
                        }
			/* do deadlock detection here */
                        if (deadflck(found, &(a->lck.lox.lld))) {
                                nlm_resp->lstat = nlm_deadlck;
                                a->lck.lox.granted = 0;
                                a->rel = 1;
                                return (nlm_resp);
                        }
			sleeping = 0;
                        for (sf = sleeplcks; sf != NULL; sf = sf->next) {
                                if (same_lock(a, sf)) {
                                        sleeping = 1;
                                        break;
                                }
                        }
			if (!sleeping) { 
			     	if ((sf=insflck(&sleeplcks, &(a->lck.lox.lld),
			                (struct filock *)NULL)) == (struct filock *)NULL) {
					nlm_resp->lstat = nlm_denied_nolocks;
                                	a->lck.lox.granted = 0;
                                	a->rel = 1;
                                	return (nlm_resp);
				}
				sf->stat.wakeflg = found->stat.wakeflg++;
                                sf->stat.blk.pid = found->set.l_pid;
                                sf->stat.blk.sysid = found->set.l_sysid;
				if ((sr=insflck_reclox(&sleeplcks_reclox, a,
                                        (struct reclock *)NULL)) == (struct reclock *)NULL) {
                                        nlm_resp->lstat = nlm_denied_nolocks;
                                        a->lck.lox.granted = 0;
                                        a->rel = 1;
                                        return (nlm_resp);
                                }
                                sr->lck.lox.filocks.stat.wakeflg = found->stat.wakeflg;
                                sr->lck.lox.filocks.stat.blk.pid = found->set.l_pid;
                                sr->lck.lox.filocks.stat.blk.sysid = found->set.l_sysid;
			}
			a->lck.lox.granted = 0;
                        if (choice == MSG) {    /* msg passing */
                                if (nlm_call(NLM_LOCK_MSG, a, 0) == -1)
                                        a->rel = 1;
                                nlm_resp->lstat = nlm_blocked;
                                msgp = msg_q;
                                while (msgp != NULL) {
                                        if (msgp->req->lck.lox.LockID == a->lck.lox.LockID) {
                                                break;
                                        }
                                        msgp = msgp->nxt;
                                }
                                if (msgp != NULL) {
                                        msgp->t.curr = 0;
                                        msgp->reply = nlm_resp;
                                        add_reply(msgp, nlm_resp);
                                }
                                return (nlm_resp);
                        } else {                /* rpc */
                                printf("rpc not supported\n");
                                a->rel = 1;     /* rpc error, discard */
                        }
		}
	}
	return (NULL);			/* no reply available */
}

remote_result *
local_unlock(a)
	struct reclock *a;
{
	int fd, cmd;
	struct flock fld;

	if (debug)
		printf("enter local_unlock...................\n");
	/*
	 * convert fh to fd
	 */
	if ((fd = get_fd(a)) < 0) {
		if (fd == -1)
			nlm_resp->lstat = nlm_denied_nolocks;
		else
			nlm_resp->lstat = denied;
		return (nlm_resp);
	}

	/*
	 * set the lock
	 */
	if (a->block)
		cmd = F_RSETLKW;
	else
		cmd = F_RSETLK;
	fld.l_type = F_UNLCK;
	fld.l_whence = 0;
	fld.l_start = a->lck.lox.lld.l_start;
	fld.l_len = a->lck.lox.lld.l_len;
	fld.l_pid = a->lck.lox.lld.l_pid;
	fld.l_sysid = a->lck.lox.lld.l_sysid;
	if (debug) {
		printf("fld.l_start=%d fld.l_len=%d fld.l_pid=%d fld.l_rsys=%x\n",
			fld.l_start, fld.l_len, fld.l_pid, fld.l_sysid);
	}
	if (fcntl(fd, cmd, &fld) == -1) {
		perror("fcntl");
		if (errno == EINTR) {
			nlm_resp->lstat = blocking;
			a->w_flag = 1;
		} else if (errno == ENOLCK) {
			printf("rpc.lockd: out of lock.\n");
			nlm_resp->lstat = nlm_denied_nolocks;
		} else {
			printf("rpc.lockd: unable to unlock a lock. \n");
			nlm_resp->lstat = denied;
		}
	} else {
		nlm_resp->lstat = nlm_granted;
		/*
		 * Update fd table
		 */
		remove_fd(a);
		close(fd);
	}
	return (nlm_resp);
}

remote_result *
remote_unlock(a, choice)
	struct reclock *a;
	int choice;
{
	if (debug)
		printf("enter remote_unlock\n");

	if (search_fh(a, 0) == (struct lm_vnode *)NULL) {
		a->rel = 1;
		nlm_resp->lstat = nlm_granted;
		return (nlm_resp);
	} else {
		if (choice == MSG) {
			if (nlm_call(NLM_UNLOCK_MSG, a, 0) == -1)
				a->rel = 1;	/* rpc error, discard */
		} else {
			printf("rpc not supported\n");
			a->rel = 1;		/* rpc error, discard */
		}
		return (NULL);			/* no reply available */
	}
}


remote_result *
local_test(a)
	struct reclock *a;
{
	int fd, cmd;
	struct flock fld;

	if (debug)
		printf("enter local_test()...\n");

	/*
	 * convert fh to fd
	 */
	if ((fd = get_fd(a)) < 0) {
		if (fd == -1)
			nlm_resp->lstat = nlm_denied_nolocks;
		else
			nlm_resp->lstat = denied;
		nlm_resp->lstat = denied;
		return (nlm_resp);
	}

	/*
	 * test the lock
	 */
	cmd = F_RGETLK;
	if (a->exclusive)
		fld.l_type = F_WRLCK;
	else
		fld.l_type = F_RDLCK;
	fld.l_whence = 0;
	fld.l_start = a->lck.lox.lld.l_start;
	fld.l_len = a->lck.lox.lld.l_len;
	fld.l_pid = a->lck.lox.lld.l_pid;
	fld.l_sysid = a->lck.lox.lld.l_sysid;
	if (fcntl(fd, cmd, &fld) == -1) {
		perror("fcntl");
		if (errno == EINTR) {
			nlm_resp->lstat = blocking;
			a->w_flag = 1;
		} else if (errno == ENOLCK) {
			printf("rpc.lockd: out of lock.\n");
			nlm_resp->lstat = nlm_denied_nolocks;
		} else {
			printf("rpc.lockd: unable to test a lock. \n");
			nlm_resp->lstat = denied;
		}
	} else {
		if (fld.l_type == F_UNLCK) {
			nlm_resp->lstat = nlm_granted;
			a->lck.lox.lld.l_type = fld.l_type;
		} else {
			nlm_resp->lstat = nlm_denied;
			a->lck.lox.lld.l_type = fld.l_type;
			a->lck.lox.lld.l_start = fld.l_start;
			a->lck.lox.lld.l_len = fld.l_len;
			a->lck.lox.lld.l_pid = fld.l_pid;
			a->lck.lox.lld.l_sysid = fld.l_sysid;
		}
	}
	if (debug) {
		printf("fld.l_start=%d fld.l_len=%d fld.l_sysid=%x\n",
			fld.l_start, fld.l_len, fld.l_sysid);
	}
	return (nlm_resp);
}

remote_result *
remote_test(a, choice)
	struct reclock *a;
	int choice;
{
	if (debug)
		printf("enter remote_test\n");

	if (search_fh(a, 1) != (struct lm_vnode *)NULL) {
		if (choice == MSG) {
			if (nlm_call(NLM_TEST_MSG, a, 0) == -1)
				a->rel = 1;
		} else {
			printf("rpc not supported\n");
			a->rel = 1;
		}
		return (NULL);
	} else {
		a->rel = 1;
		nlm_resp->lstat = denied;
		return (nlm_resp);
	}
}

remote_result *
local_cancel(a)
	struct reclock *a;
{
	msg_entry *msgp;

	if (debug)
		printf("enter local_cancel(%x)\n", a);
	a->rel = 1;
	if (search_fh(a, 0) == (struct lm_vnode *)NULL)
		nlm_resp->lstat = denied;
	else {
		if (a->w_flag == 0)
			nlm_resp->lstat = nlm_granted;
		else {
			a->w_flag = 0;
			a->rel = 1;
			if (!remote_clnt(a)) {
				if ((msgp = retransmitted(a, KLM_LOCK))
					!= NULL) {
					if (debug)
						printf("local_cancel: dequeue(%x)\n",
							msgp->req);
					dequeue(msgp);
				} else {
					release_reclock(a);
				}
			} else
				release_reclock(a);
			nlm_resp->lstat = denied;
		}
	}
	return (nlm_resp);
}

remote_result *
remote_cancel(a, choice)
	struct reclock *a;
	int choice;
{
	msg_entry *msgp;

	if (debug)
		printf("enter remote_cancel(%x)\n", a);

	if (search_fh(a, 0) == (struct lm_vnode *)NULL) {
		if ((msgp = retransmitted(a, KLM_LOCK)) == NULL) {
			/* msg was never received */
			a->rel = 1;
			nlm_resp->lstat = denied;
			return (nlm_resp);
		} else {
			/* msg is being processed */
			if (debug)
				printf("remove msg (%x) due to remote cancel\n",
					msgp->req);
			msgp->req->rel = 1;
			dequeue(msgp);
		}
	} else {
		if (a->w_flag == 0) {
			a->rel = 1;
			nlm_resp->lstat = nlm_granted;
			return (nlm_resp);
		}
	}

	if (choice == MSG){
		if (nlm_call(NLM_CANCEL_MSG, a, 0) == -1)
			a->rel = 1;
	} else { /* rpc case */
		printf("rpc not supported\n");
		a->rel = 1;
	}
	return (NULL);
}

remote_result *
cont_lock(a, resp)
	struct reclock *a;
	remote_result *resp;
{
	struct process_locks *t;
	struct lm_vnode *lox;
	struct reclock *sr, *found_reclox, *insrt_reclox = NULL;
	struct filock *sf, *found, *insrt = NULL;
	int i, retval, sleeping;

	if (debug) {
		printf("enter cont_lock (%x) ID=%d \n", a, a->lck.lox.LockID);
	}
	switch (resp->lstat) {
	case nlm_granted:
		a->rel = 0;
		/*
		 * Update fh struct table
		 */
		if ((lox = search_fh(a, 0)) == (struct lm_vnode *)NULL) {
			a->rel = 1;
			release_res(resp);
			printf("rpc.lockd: unable to alloc fh entry.\n");
			return (NULL);
		}
		retval = flckadj(&(lox->filocks), (struct filock *)NULL,
                        &(a->lck.lox.lld));
                retval = flckadj_reclox(&(lox->reclox), (struct reclock *)NULL,
			 a);
		if (debug) {
			printf("cont_lock: after updating table :\n");
			pr_all();
		}
		if (add_mon(a, 1) == -1)
			printf("rpc.lockd: add_mon failed in cont_lock.\n");
		return (resp);
	case denied:
	case nolocks:
		/*
		 * Update fh struct table
		 */
		if ((lox = search_fh(a, 0)) == (struct lm_vnode *)NULL) {
			a->rel = 1;
			release_res(resp);
			printf("rpc.lockd: unable to alloc fh entry.\n");
			return (NULL);
		}
		a->rel = 1;
		a->block = FALSE;
		a->lck.lox.granted = 0;
		if (debug) {
                        printf("cont_lock: after updating table :\n");
                        pr_all();
                }
		return (resp);
	case deadlck:
		/*
		 * Update fh struct table
		 */
		if ((lox = search_fh(a, 0)) == (struct lm_vnode *)NULL) {
			a->rel = 1;
			release_res(resp);
			printf("rpc.lockd: unable to alloc fh entry.\n");
			return (NULL);
		}
		a->rel = 1;
		a->block = TRUE;
		a->lck.lox.granted = 0;
		if (debug) {
                        printf("cont_lock: after updating table :\n");
                        pr_all();
                }
		return (resp);
	case blocking:
		/*
                 * Update fh struct table
                 */
                if ((lox = search_fh(a, 0)) == (struct lm_vnode *)NULL) {
                        a->rel = 1;
                        release_res(resp);
                        printf("rpc.lockd: unable to alloc fh entry.\n");
                        return (NULL);
                }
		a->rel = 0;
		a->w_flag = 1;
		a->block = TRUE;
                sleeping = 0;
                for (sf = sleeplcks; sf != NULL; sf = sf->next) {
                        if (same_lock(a, sf)) {
                                sleeping = 1;
                                break;
                        }
                }
                if (!sleeping) {
			found = blocked(lox->filocks, &(a->lck.lox.lld), &insrt);
                        if ((sf=insflck(&sleeplcks, &(a->lck.lox.lld),
                                (struct filock *)NULL)) == NULL) {
                                nlm_resp->lstat = nlm_denied_nolocks;
                                a->lck.lox.granted = 0;
                                a->rel = 1;
                                return (nlm_resp);
                        }
			if (found) {
				sf->stat.wakeflg = found->stat.wakeflg;
                        	sf->stat.blk.pid = found->set.l_pid;
                        	sf->stat.blk.sysid = found->set.l_sysid;
			}
			found_reclox = blocked_reclox(lox->reclox, 
				&(a->lck.lox.lld), &insrt_reclox);
			if ((sr=insflck_reclox(&sleeplcks_reclox, a,
                                (struct reclock *)NULL)) == (struct reclock *)NULL) {                                                               
                                nlm_resp->lstat = nlm_denied_nolocks;
                                a->lck.lox.granted = 0;
                                a->rel = 1;
                                return (nlm_resp);
                        }                         
			if (found_reclox) {
                        	sr->lck.lox.filocks.stat.wakeflg = 
				found_reclox->lck.lox.filocks.stat.wakeflg++;
                        	sr->lck.lox.filocks.stat.blk.pid = 
				found_reclox->lck.lox.lld.l_pid;
                        	sr->lck.lox.filocks.stat.blk.sysid = 
				found_reclox->lck.lox.lld.l_sysid;
			}
                }
		return (resp);
	case grace:
		a->rel = 0;
		release_res(resp);
		return (NULL);
	default:
		a->rel = 1;
		release_res(resp);
		printf("unknown lock return: %d\n", resp->lstat);
		return (NULL);
	}
}


remote_result *
cont_unlock(a, resp)
	struct reclock *a;
	remote_result *resp;
{
	struct process_locks *t;
	struct lm_vnode *lox;
	struct filock *sf;
	struct reclock *sr;
	int retval, sleeping;

	if (debug)
		printf("enter cont_unlock\n");

	a->rel = 1;
	switch (resp->lstat) {
		case nlm_granted:
			/*
			 * Update fh struct table
			 */
			if ((lox = search_fh(a, 0)) == (struct lm_vnode *)NULL) {
				a->rel = 1;
				resp->lstat = nlm_granted;
				return (resp);
			}
                	retval = flckadj(&(lox->filocks), (struct filock *)NULL,
				&(a->lck.lox.lld));
			retval = flckadj_reclox(&(lox->reclox), 
				(struct reclock *)NULL, a);

			for (sf = sleeplcks; sf != NULL; sf = sf->next) {
                                if (a->lck.lox.lld.l_pid == sf->stat.blk.pid &&
                                        a->lck.lox.lld.l_sysid == sf->stat.blk.sysid) {
                                        break;
                                }
                        }
			for (sr = sleeplcks_reclox; sr != NULL; sr = sr->next) {
                                if (a->lck.lox.lld.l_pid == sr->lck.lox.filocks.stat.blk.pid &&
                                        a->lck.lox.lld.l_sysid == sr->lck.lox.filocks.stat.blk.sysid) {                          
                                        break;
                                }
                        }
			if (!sf && !sr) {
				sleeping = 0;
                        	for (sf = sleeplcks; sf != NULL; sf = sf->next) {
                                	if (same_lock(a, sf)) {
                                        	sleeping = 1;
                                        	break;
                                	}
                        	}
				if (sleeping) {
                                	delflck(&sleeplcks, sf);
					delflck_reclox(&sleeplcks_reclox, sr);
				}
			} else {
                		for (sf = sleeplcks; sf != NULL; sf = sf->next) {
                       			if (a->lck.lox.lld.l_pid == sf->stat.blk.pid &&
                       				a->lck.lox.lld.l_sysid == sf->stat.blk.sysid) {
						a->lck.lox.granted = 0;
						delflck(&sleeplcks, sf);
                        			retval = flckadj(&(lox->filocks), (struct filock *)NULL, &(a->lck.lox.lld));
						wakeup(sf);
					}
				}
				for (sr = sleeplcks_reclox; sr != NULL; 
					sr = sr->next) {
                                        if (a->lck.lox.lld.l_pid == sr->lck.lox.filocks.stat.blk.pid &&
                                                a->lck.lox.lld.l_sysid == sr->lck.lox.filocks.stat.blk.sysid) {
                                                a->lck.lox.granted = 0;
                                                delflck_reclox(&sleeplcks_reclox, sr);
                                                retval = flckadj_reclox(&(lox->reclox),                                          
                                                        (struct reclock *)NULL,
a);                                                     
                                        }        
                                }
			}

			if (!lox->filocks && !lox->reclox) {
				remove_fh(lox);
                                release_fh(lox);
                        }
			resp->lstat = nlm_granted;
			if (debug) {
                                printf("cont_unlock: after updating the tables\n");
                                pr_all();
                        }
			return (resp);
		case denied:		/* impossible */
		case nolocks:
			return (resp);
		case blocking:		/* impossible */
			a->w_flag = 1;
			return (resp);
		case grace:
			a->rel = 0;
			release_res(resp);
			return (NULL);
		default:
			a->rel = 0;
			release_res(resp);
			fprintf(stderr,
				"rpc.lockd: unkown rpc_unlock return: %d\n",
				resp->lstat);
			return (NULL);
	}
}

remote_result *
cont_test(a, resp)
	struct reclock *a;
	remote_result *resp;
{
	if (debug)
		printf("enter cont_test\n");

	a->rel = 1;
	switch (resp->lstat) {
	case grace:
		a->rel = 0;
		release_res(resp);
		return (NULL);
	case nlm_granted:
	case denied:
		if (debug)
			printf("lock blocked by %d, (%d, %d)\n",
				resp->lholder.svid, resp->lholder.l_offset,
				resp->lholder.l_len);
		return (resp);
	case nolocks:
		return (resp);
	case blocking:
		a->w_flag = 1;
		return (resp);
	default:
		fprintf(stderr, "rpc.lockd: cont_test: unknown return: %d\n",
			resp->lstat);
		release_res(resp);
		return (NULL);
	}
}

remote_result *
cont_cancel(a, resp)
	struct reclock *a;
	remote_result *resp;
{
	struct process_locks *t;
	msg_entry *msgp;
	register struct filock  *sf;
	register struct reclock  *sr;
        struct  filock *insrt = (struct filock *)NULL;

	if (debug)
		printf("enter cont_cancel\n");

	a->rel = 1;
	switch (resp->lstat) {
	case nlm_granted:
		if (search_fh(a, 0) == (struct lm_vnode *)NULL) { /* lock is not found */
			if (debug)
				printf("cont_cancel: msg must be removed from msg queue due to remote_cancel, now has to be put back\n");

			a->rel = 0;
			return (resp);
		}
		return (resp);
	case blocking:		/* should not happen */
		a->w_flag = 1;
	case nolocks:
		return (resp);
	case grace:
		a->rel = 0;
		release_res(resp);
		return (NULL);
	case denied:
		if ((search_fh(a, 0) != (struct lm_vnode *)NULL) && (a->w_flag == 1)) {
			a->w_flag = 0;
			a->rel = 1;
			if ((msgp = retransmitted(a, KLM_LOCK)) != NULL) {
				if (debug)
					printf("cont_cancel: dequeue(%x)\n",
						msgp->req);
				dequeue(msgp);
			}
			else {
				fprintf(stderr,
					"rpc.lockd: cont_cancel: cannot find blocked lock request in msg queue! \n");
				release_reclock(a);
			}
			return (resp);
		}
		else if ( a != NULL && a->w_flag == 0) {
			/* remote and local lock tbl inconsistent */
			printf("remote and local lock tbl inconsistent\n");
			release_res(resp);	/* discard this msg */
			return (NULL);
		}
		else {
			return (resp);
		}
	default:
		fprintf(stderr, "rpc.lockd: unexpected remote_cancel %d\n",
			resp->lstat);
		release_res(resp);
		return (NULL);
	}		/* end of switch */
}

remote_result *
cont_reclaim(a, resp)
	struct reclock *a;
	remote_result *resp;
{
	remote_result *local;

	if (debug)
		printf("enter cont_reclaim\n");
	switch (resp->lstat) {
	case nlm_granted:
	case denied:
	case nolocks:
	case blocking:
		local = resp;
		break;
	case grace:
		if (a->reclaim)
			fprintf(stderr, "rpc.lockd: reclaim lock req(%x) is returned due to grace period, impossible\n", a);
		local = NULL;
		break;
	default:
		printf("unknown cont_reclaim return: %d\n", resp->lstat);
		local = NULL;
		break;
	}

	if (local == NULL)
		release_res(resp);
	return (local);
}
