#ident	"@(#)librpcsvc:klm_prot.x	1.1"
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
 * Kernel/lock manager protocol definition
 * Copyright (C) 1986 Sun Microsystems, Inc.
 *
 * protocol used between the UNIX kernel (the "client") and the
 * local lock manager.  The local lock manager is a deamon running
 * above the kernel.
 */
program KLM_PROG {
	version KLM_VERS {

		klm_testrply	KLM_TEST (struct klm_testargs) =	1;

		klm_stat	KLM_LOCK (struct klm_lockargs) =	2;

		klm_stat	KLM_CANCEL (struct klm_lockargs) =	3;
		/* klm_granted=> the cancel request fails due to lock is already granted */
		/* klm_denied=> the cancel request successfully aborts
lock request  */

		klm_stat	KLM_UNLOCK (struct klm_unlockargs) =	4;
	} = 1;
} = 100020;

const	LM_MAXSTRLEN = 1024;

/*
 * lock manager status returns
 */
enum klm_stats {
	klm_granted = 0,	/* lock is granted */
	klm_denied = 1,		/* lock is denied */
	klm_denied_nolocks = 2, /* no lock entry available */
	klm_working = 3, 	/* lock is being processed */
	klm_deadlck = 5		/* EINTR = 5,is used in krpc/klm_lockmgr.c */
};

/*
 * lock manager lock identifier
 */
struct klm_lock {
	string server_name<LM_MAXSTRLEN>;
	netobj fh;		/* a counted file handle */
	int base;		/* beginning offset of the lock */
	int length;		/* byte length of the lock;
				 * zero means through end of file */
	int type;
	int granted;
	int color;
	int LockID;
	int pid;                /* holder of the lock */
	int class;
	long rsys;
	long rpid;
};

/*
 * lock holder identifier
 */
struct klm_holder {
	bool exclusive;		/* FALSE if shared lock */
	int base;               /* beginning offset of the lock */
        int length;             /* byte length of the lock;
                                 * zero means through end of file */
	int type;
	int granted;
        int color;
        int LockID;
	int pid;                /* holder of the lock (pid) */
        int class;
	long rsys;
	long rpid;
};

/*
 * reply to KLM_LOCK / KLM_UNLOCK / KLM_CANCEL
 */
struct klm_stat {
	klm_stats stat;
};

/*
 * reply to a KLM_TEST call
 */
union klm_testrply switch (klm_stats stat) {
	case klm_denied:
		struct klm_holder holder;
	default: /* All other cases return no arguments */
		void;
};

/*
 * arguments to KLM_LOCK
 */
struct klm_lockargs {
	bool block;
	bool exclusive;
	struct klm_lock alock;
};

/*
 * arguments to KLM_TEST
 */
struct klm_testargs {
	bool exclusive;
	struct klm_lock alock;
};

/*
 * arguments to KLM_UNLOCK
 */
struct klm_unlockargs {
	struct klm_lock alock;
};
