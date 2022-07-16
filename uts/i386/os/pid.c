/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-os:pid.c	1.3"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/debug.h"
#include "sys/proc.h"
#include "sys/kmem.h"
#include "sys/tuneable.h"
#include "sys/inline.h"
#include "sys/var.h"
#include "sys/debug.h"
#include "sys/cmn_err.h"
#include "sys/cred.h"
#include "sys/file.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/user.h"
#include "sys/systm.h"
#include "sys/session.h"
#include "sys/sysinfo.h"

/* directory entries for /proc */
union procent {
	proc_t *pe_proc;
	union procent *pe_next;
};

/* active process chain */
proc_t *practive;

struct pid pid0 = {
	0,		/* pid_prinactive */
	1,		/* pid_pgorphaned */
	3,		/* pid_ref	*/
	0,		/* pid_prslot	*/
	0,		/* pid_id	*/
	NULL,		/* pid_pglink	*/
	NULL		/* pid_link	*/
};

#define HASHSZ		64
#define HASHPID(pid)	(pidhash[((pid)&(HASHSZ-1))])

STATIC u_int nproc;
STATIC struct pid **pidhash;
STATIC pid_t minpid;	
STATIC pid_t mpid;
STATIC union procent *procdir;
STATIC union procent *procentfree;

STATIC struct pid *
pid_lookup(pid)
	register pid_t pid;
{
	register struct pid *pidp;
	for (pidp = HASHPID(pid); pidp; pidp = pidp->pid_link) {
		if (pidp->pid_id == pid) {
			ASSERT(pidp->pid_ref > 0);
			break;
		}
	}
	return pidp;
}

void
pid_setmin()
{
	minpid = mpid + 1;
}

/*
 * This function assigns a pid for use in a fork request.  It checks
 * to see that there is an empty slot in the proc table, that the
 * requesting user does not have too many processes already active,
 * and that the last slot in the proc table is not being allocated to
 * anyone who should not use it.
 *
 * After a proc slot is allocated, it will try to allocate a proc
 * structure for the new process. 
 *
 * If all goes well, pid_assign() will return a new pid and set up the
 * proc structure pointer for the child process.  Otherwise it will
 * return -1.
 */

pid_t
pid_assign(cond, pp)
	int	cond;	/* allow assignment of last slot? */
	proc_t	**pp;	/* child process proc structure pointer */
{
	register struct pid *pidp;
	register proc_t *prp;
	union procent *pep;

	/*
	 * If NPROC processes already running,
	 * or NPROC-1 processes running and NP_NOLAST not used, 
	 * fail immediately
	 */

	if (nproc >= v.v_proc - 1) {
		if (nproc == v.v_proc) {
			syserr.procovf++;
			return -1;
		}
		if (cond & NP_NOLAST)
			return -1;
	}

	/*
	 * If not super-user then make certain that the maximum
	 * number of children don't already exist.
	 */

	if (u.u_cred->cr_uid && u.u_cred->cr_ruid) {
		register uid_t ruid = u.u_cred->cr_ruid;
		int uid_procs = 0;
		for (prp = practive; prp != NULL; prp = prp->p_next)
			if (prp->p_uid == ruid)
				uid_procs++;
		if (uid_procs >= v.v_maxup)
			return -1;
	}

	prp = (proc_t *)kmem_zalloc(sizeof(*prp), KM_NOSLEEP);
	pidp = (struct pid *)kmem_zalloc(sizeof(*pidp), KM_NOSLEEP);

	if (prp == NULL || pidp == NULL) {
		if ((cond & NP_FAILOK) == 0)
			cmn_err(CE_PANIC, "newproc - fork failed\n");
		if (prp != NULL)
			kmem_free(prp, sizeof(*prp));
		if (pidp != NULL)
			kmem_free(pidp, sizeof(*pidp));
		return -1;
	}

	/*
	 * Allocate a pid
	 */

	do  {
		if (++mpid == MAXPID)
			mpid = minpid;
	} while (pid_lookup(mpid) != NULL);

	/* 
	 * Allocate a /proc directory entry
	 */

	ASSERT(procentfree != NULL);

	pep = procentfree;
	procentfree = procentfree->pe_next;
	pep->pe_proc = prp;

	PID_HOLD(pidp);
	pidp->pid_id = mpid;
	pidp->pid_prslot = pep - procdir;
	pidp->pid_link = HASHPID(mpid);
	HASHPID(mpid) = pidp;

	prp->p_stat = SIDL;
	prp->p_opid = mpid;
	prp->p_pidp = pidp;
	prp->p_next = practive;
	practive = prp;

	*pp = prp;
	nproc++;
	return mpid;
}

/*
 * decrement the reference count for pid
 */

int
pid_rele(pidp)
	register struct pid *pidp;
{
	register struct pid **pidpp;
	register int s;

	ASSERT(pidp != &pid0);

	for (pidpp = &HASHPID(pidp->pid_id); ; pidpp = &(*pidpp)->pid_link) {
		ASSERT(*pidpp != NULL);
		if (*pidpp == pidp)
			break;
	}

	s = splhi();
	*pidpp = pidp->pid_link;
	kmem_free(pidp, sizeof(*pidp));
	splx(s);

	return 0;
}

void
pid_exit(prp)
	register proc_t *prp;
{
	register proc_t **prpp;
	register struct pid *pidp;
	register int s;

	pgexit(prp);

	SESS_RELE(prp->p_sessp);

	pidp = prp->p_pidp;
	pidp->pid_prinactive = 1;
	procdir[pidp->pid_prslot].pe_next = procentfree;
	procentfree = &procdir[pidp->pid_prslot];

	PID_RELE(pidp);

	for (prpp = &practive; ; prpp = &(*prpp)->p_next) {
		ASSERT(*prpp != NULL);
		if (*prpp == prp)
			break;
	}

	s = splhi();
	*prpp = prp->p_next;
	kmem_free(prp, sizeof(*prp));
	splx(s);

	nproc--;
}

/*
 * find a process given its process ID
 */

proc_t *
prfind(pid)
	register pid_t pid;
{
	struct pid *pidp;

	pidp = pid_lookup(pid);
	if (pidp != NULL && pidp->pid_prinactive == 0)
		return procdir[pidp->pid_prslot].pe_proc;
	return NULL;
}

/*
 * return the list of processes in whose process group ID is 'pgid',
 * or NULL, if no such process group
 */

proc_t *
pgfind(pgid)
	register pid_t pgid;
{
	register proc_t *prp;

	struct pid *pidp;

	pidp = pid_lookup(pgid);
	if (pidp != NULL)
		return pidp->pid_pglink;
			
	return NULL;
}

void
pid_init()
{
	register i;

	pidhash = (struct pid **)
	  kmem_zalloc(sizeof(struct pid *)*HASHSZ, KM_NOSLEEP);

	procdir = (union procent *)
	  kmem_alloc(sizeof(union procent)*v.v_proc, KM_NOSLEEP);

	if (pidhash == NULL || procdir == NULL)
		cmn_err(CE_PANIC, "Could not allocate space for pid tables\n");

	nproc = 1;
	practive = proc_sched;
	proc_sched->p_next = NULL;
	procdir[0].pe_proc = proc_sched;

	procentfree = &procdir[1];
	for (i = 1; i < v.v_proc - 1; i++)
		procdir[i].pe_next = &procdir[i+1];
	procdir[i].pe_next = NULL;

	HASHPID(0) = &pid0;
}

proc_t *
pid_entry(slot)
	int slot;
{
	register union procent *pep;
	register proc_t *prp;

	ASSERT(slot >= 0 && slot < v.v_proc);

	pep = procdir[slot].pe_next;
	if ((pep >= procdir && pep < &procdir[v.v_proc]) || pep == NULL)
		return NULL;
	prp = procdir[slot].pe_proc;
	if (prp->p_stat == SIDL)
		return NULL;
	return prp;
}

/*
 * Send the specified signal to all processes whose process group ID is
 * equal to 'pgid'
 */

void
signal(pgid, sig)
	pid_t pgid;
	int sig;
{
	register struct pid *pidp;

	if (pgid == 0 || (pidp = pid_lookup(pgid)) == NULL)
		return;

	pgsignal(pidp, sig);
}

/*
 * Send the specified signal to the specified process
 */

void
prsignal(pidp, sig)
	register struct pid *pidp;
	int sig;
{
	if (!(pidp->pid_prinactive))
		psignal(procdir[pidp->pid_prslot].pe_proc, sig);
}
