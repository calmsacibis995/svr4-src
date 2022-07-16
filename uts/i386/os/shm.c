/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *		PROPRIETARY NOTICE (Combined)
 *
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 *
 *
 *
 *		Copyright Notice
 *
 * Notice of copyright on this source code product does not indicate
 * publication.
 *
 *	(c) 1986,1987,1988,1989	 Sun Microsystems, Inc
 *	(c) 1983,1984,1985,1986,1987,1988,1989	AT&T.
 *		  All rights reserved.
 *
 */

#ident	"@(#)kern-os:shm.c	1.3.2.1"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/cred.h"
#include "sys/errno.h"
#include "sys/time.h"
#include "sys/signal.h"
#include "sys/tss.h"
#include "sys/kmem.h"
#include "sys/user.h"
#include "sys/ipc.h"
#include "sys/immu.h"
#include "sys/proc.h"
#include "sys/systm.h"
#include "sys/sysmacros.h"
#include "sys/shm.h"
#include "sys/debug.h"
#include "sys/tuneable.h"
#include "sys/cmn_err.h"
#include "sys/vm.h"
#include "sys/mman.h"

#include "vm/hat.h"
#include "vm/seg.h"
#include "vm/as.h"
#include "vm/seg_vn.h"
#include "vm/anon.h"
#include "vm/page.h"
#include "vm/vpage.h"

extern struct shmid_ds	shmem[];	/* shared memory headers */
extern struct shminfo	shminfo;	/* shared memory info structure */

/* XENIX Support */
	/* The structure 'xshmid_ds' is the XENIX 386 version of 'shmid_ds'.
	 * All x.out executables use 'xshmid_ds'.  The shmctl() routine
	 * must convert xshmid_ds <==> shmid_ds for x.out executables.
	 */

struct	xshmid_ds {
	struct	o_ipc_perm shm_perm;	/* operation permission struct */
	int	shm_segsz;		/* segment size */
	ushort	shm_ptbl;		/* addr of sd segment */
	ushort	shm_lpid;		/* pid of last shared mem op */
	ushort	shm_cpid;		/* creator pid */
	ushort	shm_nattch;		/* current # attached */
	ushort	shm_cnattch;		/* in-core # attached */
	time_t	shm_atime;		/* last attach time */
	time_t	shm_dtime;		/* last detach time */
	time_t	shm_ctime;		/* last change time */
};		

#define INT16_MAX	32767		/* For 286 compatibility, shared mem
					 * identifiers should only use the low
					 * 15 bits.
					 */

/* End XENIX Support */

STATIC int kshmdt();
STATIC int shm_lock();
STATIC int shm_unlock();
STATIC segacct_t *sa_find();
STATIC int sa_add();
STATIC int sa_del();
STATIC void shm_rm_amp();

/*
 * Argument vectors for the various flavors of shmsys().
 */

#define	SHMAT	0
#define	SHMCTL	1
#define	SHMDT	2
#define	SHMGET	3

struct shmsysa {
	int		opcode;
};

struct shmata {
	int		opcode;
	int		shmid;
	caddr_t		addr;
	int		flag;
};

struct shmctla {
	int		opcode;
	int		shmid;
	int		cmd;
	struct shmid_ds	*arg;
};

struct shmdta {
	int		opcode;
	caddr_t		addr;
};

struct shmgeta {
	int		opcode;
	key_t		key;
	int		size;
	int		shmflg;
};

/*
 * Convert user supplied shmid into a ptr to the associated
 * shared memory header.
 */
STATIC int
shmconv(s, spp)
	register int	s;		/* shmid */
	struct shmid_ds	**spp;		/* shared-mem header to be returned */
{
	register struct shmid_ds *sp;	/* ptr to associated header */

	if (s < 0)
		return EINVAL;
	sp = &shmem[s % shminfo.shmmni];
	SHMLOCK(sp);
	if (!(sp->shm_perm.mode & IPC_ALLOC)  
	  || s / shminfo.shmmni != sp->shm_perm.seq) {
		SHMUNLOCK(sp);
		return EINVAL;
	}
	*spp = sp;
	return 0;
}

/*
 * Shmat (attach shared segment) system call.
 */
STATIC int
shmat(uap, rvp)
	register struct shmata *uap;
	rval_t *rvp;
{
	struct shmid_ds	*sp;
	addr_t	addr;
	register uint	size;
	register int	error = 0;
	register proc_t *pp = u.u_procp;
	struct segvn_crargs	crargs;	/* segvn create arguments */

	if (error = shmconv(uap->shmid, &sp))
		return error;
	if (error = ipcaccess(&sp->shm_perm, SHM_R, u.u_cred))
		goto errret;
	if ((uap->flag & SHM_RDONLY) == 0
	  && (error = ipcaccess(&sp->shm_perm, SHM_W, u.u_cred)))
		goto errret;

	if (u.u_nshmseg >= shminfo.shmseg) {
		error = EMFILE;
		goto errret;
	}

	size = sp->shm_amp->size;
	addr = uap->addr;
	if (addr == 0) {
		/* Let the system pick the attach address */
		map_addr(&addr, size, (off_t)0, 1);
		if (addr == NULL) {
			error = ENOMEM;
			goto errret;
		}
	} else {
		/* Use the user-supplied attach address */
		addr_t base;
		uint len;

		if (uap->flag & SHM_RND)
			addr = (addr_t)((ulong)addr & ~(SHMLBA - 1));
		/*
		 * Check that the address range
		 *  1) is properly aligned
		 *  2) is correct in unix terms (i.e., not in the u-area)
		 *  3) is within an unmapped address segment
		 */
		base = addr;
		len = size;		/* use aligned size */
		if (((uint)base & PAGEOFFSET) || 
		    (valid_usr_range(base,len) == 0) ||
		    as_gap(pp->p_as, len, &base, &len,
			    AH_LO, (addr_t)NULL) != 0) {
			error = EINVAL;
			goto errret;
		}
	}
	
	/* Initialize the create arguments and map the segment */
	crargs = *(struct segvn_crargs *)zfod_argsp;	/* structure copy */
	crargs.offset = 0;
	crargs.type = MAP_SHARED;
	crargs.amp = sp->shm_amp;
	crargs.maxprot = crargs.prot;
	crargs.prot = (uap->flag & SHM_RDONLY) ?
	    (PROT_ALL & ~PROT_WRITE) : PROT_ALL;

	error = as_map(pp->p_as, addr, size, segvn_create, (caddr_t)&crargs);
	if (error)
		goto errret;

	/* record shmem range for the detach */
	sa_add(pp, addr, (size_t)size, sp->shm_amp);
	sp->shm_amp->refcnt++;		/* keep amp until shmdt and IPC_RMID */

	rvp->r_val1 = (int) addr;
	u.u_nshmseg++;
	sp->shm_atime = hrestime.tv_sec;
	sp->shm_lpid = pp->p_pid;

errret:
	SHMUNLOCK(sp);

	return error;
}


/*
 * Shmctl system call.
 */
/* ARGSUSED */
STATIC int
shmctl(uap, rvp)
	register struct shmctla *uap;
	rval_t *rvp;
{
	struct shmid_ds		*sp;	/* shared memory header ptr */
	struct o_shmid_ds	ods;	/* hold area for SVR3 IPC_O_SET */
	struct shmid_ds		ds;	/* hold area for SVR4 IPC_SET */
	/* XENIX Support */
	struct xshmid_ds		xds;	/* hold for XENIX shmid_ds */
	/* End XENIX Support */
	register int		error;

	if (error = shmconv(uap->shmid, &sp))
		return error;

	switch (uap->cmd) {

	/* Remove shared memory identifier. */
	case IPC_O_RMID:
	case IPC_RMID:
		if (u.u_cred->cr_uid != sp->shm_perm.uid
		  && u.u_cred->cr_uid != sp->shm_perm.cuid
		  && !suser(u.u_cred)) {
			error = EPERM;
			break;
		}
	/* XENIX Support */
		if (((unsigned)(++(sp->shm_perm.seq) * shminfo.shmmni + (sp - shmem))) > INT16_MAX)
	/* End XENIX Support - UNIX SVR3 Code was deleted */
			sp->shm_perm.seq = 0;

		/*
		 * When we created the shared memory segment,
		 * we set the refcnt to 2. When we shmat (and shmfork)
		 * we bump the refcnt.  We decrement it in
		 * kshmdt (called from shmdt, shmexec and shmexit),
		 * and in IPC_RMID.  Thus we use a refcnt
		 * of 1 to mean that there are no more references.
		 * We do this so that the anon_map will not
		 * go away until we are ready, even if a process
		 * munmaps it's shared memory.
		 */
                if (--sp->shm_amp->refcnt == 1) {       /* if no attachments */
			shm_rm_amp(sp->shm_amp, sp->shm_lkcnt);
                }
		sp->shm_lkcnt = 0;
		sp->shm_segsz = 0;
                sp->shm_amp = NULL;
                SHMUNLOCK(sp);
                sp->shm_perm.mode = 0;
                return (0);

	/* Set ownership and permissions. */
	case IPC_O_SET:
		if (u.u_cred->cr_uid != sp->shm_perm.uid
		  && u.u_cred->cr_uid != sp->shm_perm.cuid
		  && !suser(u.u_cred)) {
			error = EPERM;
			break;
		}
	/* XENIX Support */
		if (VIRTUAL_XOUT) {
			/* Copy in XENIX version of xshmid_ds.  Note that
			 * IPC_SET only really looks at fields in the
			 * ipc_perm portion of shmid_ds, and at that point
			 * the xshmid_ds and shmid_ds structs are the same.
			 * Don't need to kludge IPC_SET to use xds instead of
			 * ds, because the structs agree on shm_perm.  However,
			 * we DO want to ensure that we only copy sizeof(xds).
			 */
			if (copyin((caddr_t)uap->arg, (caddr_t)&ods, sizeof(xds))) {
				error = EFAULT;
				break;
			}
		}
		else
	/* End XENIX Support */
		if (copyin((caddr_t)uap->arg, (caddr_t)&ods, sizeof(ods))) {
			error = EFAULT;
			break;
		}
		if (ods.shm_perm.uid > MAXUID || ods.shm_perm.gid > MAXUID){
			error = EINVAL;
			break;
		}
		sp->shm_perm.uid = ods.shm_perm.uid;
		sp->shm_perm.gid = ods.shm_perm.gid;
		sp->shm_perm.mode =
		  (ods.shm_perm.mode & 0777) | (sp->shm_perm.mode & ~0777);
		sp->shm_ctime = hrestime.tv_sec;
		break;

	case IPC_SET:
		if (u.u_cred->cr_uid != sp->shm_perm.uid
		  && u.u_cred->cr_uid != sp->shm_perm.cuid
		  && !suser(u.u_cred)) {
			error = EPERM;
			break;
		}
		if (copyin((caddr_t)uap->arg, (caddr_t)&ds, sizeof(ds))) {
			error = EFAULT;
			break;
		}
		if (ds.shm_perm.uid < (uid_t)0 || ds.shm_perm.uid > MAXUID ||
			ds.shm_perm.gid < (gid_t)0 || ds.shm_perm.gid > MAXUID){
			error = EINVAL;
			break;
		}
		sp->shm_perm.uid = ds.shm_perm.uid;
		sp->shm_perm.gid = ds.shm_perm.gid;
		sp->shm_perm.mode =
		  (ds.shm_perm.mode & 0777) | (sp->shm_perm.mode & ~0777);
		sp->shm_ctime = hrestime.tv_sec;
		break;

	/* Get shared memory data structure. */
	case IPC_O_STAT:
		if (error = ipcaccess(&sp->shm_perm, SHM_R, u.u_cred))
			break;
	/* XENIX Support */
		if (VIRTUAL_XOUT) {
			/* Kludge up the XENIX version of shmid_ds. */
			xds.shm_perm.uid = (o_uid_t) sp->shm_perm.uid;
			xds.shm_perm.gid = (o_gid_t) sp->shm_perm.gid;
			xds.shm_perm.cuid = (o_uid_t) sp->shm_perm.cuid;
			xds.shm_perm.cgid = (o_gid_t) sp->shm_perm.cgid;
			xds.shm_perm.mode = (o_mode_t) sp->shm_perm.mode;
			xds.shm_perm.seq = (ushort) sp->shm_perm.seq;
			xds.shm_perm.key = sp->shm_perm.key;
			xds.shm_segsz = sp->shm_segsz;
			xds.shm_ptbl = (ushort)sp->shm_amp;
			xds.shm_lpid = sp->shm_lpid;
			xds.shm_cpid = sp->shm_cpid;
			xds.shm_nattch = sp->shm_amp->refcnt;
			xds.shm_cnattch = sp->shm_nattch;
			xds.shm_atime = sp->shm_atime;
			xds.shm_dtime = sp->shm_dtime;
			xds.shm_ctime = sp->shm_ctime;

			if (copyout((caddr_t)&xds, (caddr_t)uap->arg, sizeof(xds)))
				error = EFAULT;
		}
		else {
	/* End XENIX Support */

		/*
		 * We set refcnt to 2 in shmget.
		 * It is bumped twice for every attach.
		 */

		sp->shm_nattch = (sp->shm_amp->refcnt >> 1) - 1;
		sp->shm_cnattch = sp->shm_nattch;

	/* copy expanded shmid_ds struct to SVR3 o_shmid_ds. 
	** The o_shmid_ds data structure supports SVR3 applications.
	** EFT applications use struct shmid_ds.
	*/
		if (sp->shm_perm.uid > USHRT_MAX || sp->shm_perm.gid > USHRT_MAX ||
		    sp->shm_perm.cuid > USHRT_MAX || sp->shm_perm.cgid > USHRT_MAX ||
		    sp->shm_perm.seq > USHRT_MAX || sp->shm_lpid > SHRT_MAX ||
		    sp->shm_cpid > SHRT_MAX || sp->shm_nattch > USHRT_MAX || 
		    sp->shm_cnattch > USHRT_MAX){
			error = EOVERFLOW;
			break;
		}
		ods.shm_perm.uid = (o_uid_t) sp->shm_perm.uid;
		ods.shm_perm.gid = (o_gid_t) sp->shm_perm.gid;
		ods.shm_perm.cuid = (o_uid_t) sp->shm_perm.cuid;
		ods.shm_perm.cgid = (o_gid_t) sp->shm_perm.cgid;
		ods.shm_perm.mode = (o_mode_t) sp->shm_perm.mode;
		ods.shm_perm.seq = (ushort) sp->shm_perm.seq;
		ods.shm_perm.key = sp->shm_perm.key;
		ods.shm_segsz = sp->shm_segsz;
		ods.shm_amp = NULL;	/* kernel addr */
		ods.shm_lkcnt = sp->shm_lkcnt;
		ods.pad[0] = 0; 	/* initialize SVR3 reserve pad */
		ods.pad[1] = 0;
		ods.shm_lpid = (o_pid_t) sp->shm_lpid;
		ods.shm_cpid = (o_pid_t) sp->shm_cpid;
		ods.shm_nattch = (ushort) sp->shm_nattch;
		ods.shm_cnattch = (ushort) sp->shm_cnattch;
		ods.shm_atime = sp->shm_atime;
		ods.shm_dtime = sp->shm_dtime;
		ods.shm_ctime = sp->shm_ctime;
		

		if (copyout((caddr_t)&ods, (caddr_t)uap->arg, sizeof(ods)))
			error = EFAULT;
	/* XENIX Support */
		}
	/* End XENIX Support */
		break;

	case IPC_STAT:
		if (error = ipcaccess(&sp->shm_perm, SHM_R, u.u_cred))
			break;

		/*
		 * We set refcnt to 2 in shmget.
		 * It is bumped twice for every attach.
		 */
		sp->shm_nattch = (sp->shm_amp->refcnt >> 1) - 1;
		sp->shm_cnattch = sp->shm_nattch;

		if (copyout((caddr_t)sp, (caddr_t) uap->arg, sizeof(*sp)))
			error = EFAULT;
		break;

	/* Lock segment in memory */
	case SHM_LOCK:
		if (!suser(u.u_cred)) {
			error = EPERM;
			break;
		}
		if (sp->shm_lkcnt++ == 0) {
			if (error = shm_lock(sp->shm_amp)) {
				cmn_err(CE_NOTE,
				  "shmctl - couldn't lock %d pages into memory",
				   sp->shm_amp->size);
				error = ENOMEM;
				sp->shm_lkcnt--;
				shm_unlock(sp->shm_amp, 0);
			}
		}
		break;

	/* Unlock segment */
	case SHM_UNLOCK:
		if (!suser(u.u_cred)) {
			error = EPERM;
			break;
		}
		if (sp->shm_lkcnt && (--sp->shm_lkcnt == 0))
			shm_unlock(sp->shm_amp, 1);
		break;

	default:
		error = EINVAL;
		break;
	}

	SHMUNLOCK(sp);

	return error;
}

/*
 * Detach shared memory segment.
 */
/* ARGSUSED */
STATIC int
shmdt(uap, rvp)
	register struct shmdta *uap;
	rval_t *rvp;
{
	return kshmdt(uap->addr);
}


STATIC int
kshmdt(addr)
	register addr_t	addr;
{
	register struct shmid_ds	*sp;
	register struct anon_map	*amp;
	register proc_t *pp = u.u_procp;
	register segacct_t *sap;
#ifdef ASYNCIO
	caddr_t  sb, se, ab, ae;
#endif /* ASYNC IO */

	/*
         * Do not allow shmdt() if there are any
         * outstanding async operations.
         */

        if (pp->p_aiocount)
                return EINVAL;

	/*
	 * Is addr a shared memory segment?
	 */
	sap = sa_find(pp, addr);
	if (sap == NULL)
		return EINVAL;

#ifdef ASYNCIO
	/*
	 * Refuse a detach if part of this segment
	 * is locked for raw disk async i/o.
	 */

	if (ab = u.u_raioaddr) {
		sb = addr;
		se = sb + sap->sa_len;
		ae = ab + u.u_raiosize; 

		if ((ab <= sb && sb < ae) || (ab <= se && se < ae)) 
			return(EBUSY);

		if ((sb <= ab && ab < se) || (sb <= ae && ae < se)) 
			return(EBUSY);
	}
#endif /* ASYNCIO */

	/*
	 * A refcnt of 1 means that the IPC_RMID
	 * has already been done, and there are
	 * no other refernces.  We have to free
	 * up everything here.
	 */

	(void) as_unmap(pp->p_as, addr, sap->sa_len);

	amp = sap->sa_amp;

	ASSERT(u.u_nshmseg > 0);
	u.u_nshmseg--;

	/* remove our accounting record */
	sa_del(pp, addr);

	/*
	 * We increment refcnt for every shmat
	 * (and shmfork) and decrement for every 
	 * detach (shmdt, shmexec, shmexit).
	 * If the refcnt is now 1, there are no
	 * more references, and the IPC_RMID has
	 * been done.
	 */
	if (--amp->refcnt == 1) {
		shm_rm_amp(amp, 0);
		return 0;
	}


	/*
	 * Find shmem anon_map ptr in system-wide table.
	 * If not found, IPC_RMID has already been done.
	 * If found, SHMLOCK prevents races in IPC_STAT and IPC_RMID.
	 */
	for (sp = shmem; sp < &shmem[shminfo.shmmni]; sp++)
		if (sp->shm_amp == amp) {
			SHMLOCK(sp);
			if (sp->shm_amp == amp)	{	/* still there? */
				sp->shm_dtime = hrestime.tv_sec;
				sp->shm_lpid = pp->p_pid;
			}
			SHMUNLOCK(sp);
			break;
		}

	return 0;
}

/*
 * Shmget (create new shmem) system call.
 */
STATIC int
shmget(uap, rvp)
	register struct shmgeta *uap;
	rval_t *rvp;
{
	struct shmid_ds	*sp;	/* shared memory header ptr */
	register uint	npages; /* how many pages */
	int		s;	/* ipcget status */
	register int	size = uap->size;
	register int	error;

	if (error = ipcget(uap->key, uap->shmflg, (struct ipc_perm *)shmem,
	  shminfo.shmmni, sizeof(*sp), &s, (struct ipc_perm **)&sp))
		return error;
	if (s) {
		/*
		 * This is a new shared memory segment.
		 * Allocate an anon_map structure and anon array and
		 * finish initialization.
		 */
		if (size < shminfo.shmmin || size > shminfo.shmmax) {
			sp->shm_perm.mode = 0;
			return EINVAL;
		}

		/*
		 * Fail if we cannot get anon space.
		 */
		if (anon_resv((uint)size) == 0) {
			sp->shm_perm.mode = 0;
			return ENOMEM;
		}

		/*
		 * Get number of pages required by this segment (round up).
		 */
		npages = btopr(size);

		/*
		 * Lock shmid in case kmem_zalloc() sleeps and a
		 * bogus request comes in with this shmid.
		 */
		SHMLOCK(sp);
		sp->shm_amp = (struct anon_map *)
		    kmem_zalloc(sizeof (struct anon_map), KM_SLEEP);
		sp->shm_amp->anon = (struct anon **)
		    kmem_zalloc(npages * sizeof (struct anon *), KM_SLEEP);
		sp->shm_amp->swresv = sp->shm_amp->size = ptob(npages);
		/*
		 * We set the refcnt to 2 so that the anon_map
		 * will stay around even if we IPC_RMID
		 * and as_unmap (instead of shmdt) the shm.
		 * In that case we catch this in kshmdt,
		 * and free up the anon_map there.
		 */
		sp->shm_amp->refcnt = 2;

		/*
		 * Store the original user's requested size, in bytes,
		 * rather than the page-aligned size.  The former is
		 * used for IPC_STAT and shmget() lookups.  The latter
		 * is saved in the anon_map structure and is used for
		 * calls to the vm layer.
		 */

		sp->shm_segsz = size;
		sp->shm_atime = sp->shm_dtime = 0;
		sp->shm_ctime = hrestime.tv_sec;
		sp->shm_lpid = 0;
		sp->shm_cpid = u.u_procp->p_pid;

		/* initialize reserve area */
		{
		int i;
			for(i=0;i<4;i++)
				sp->shm_perm.pad[i] = 0;
			sp->shm_pad1 = 0;
			sp->shm_pad2 = 0;
			sp->shm_pad3 = 0;
			for(i=0;i<4;i++)
				sp->shm_pad4[i] = 0;
		}

		SHMUNLOCK(sp);
	} else {
		/*
		 * Found an existing segment.  Check size
		 */
		if (size && size > sp->shm_segsz)
			return EINVAL;
	}

	rvp->r_val1 = sp->shm_perm.seq * shminfo.shmmni + (sp - shmem);
	return 0;
}

/*
 * System entry point for shmat, shmctl, shmdt, and shmget system calls.
 */

int
shmsys(uap, rvp)
	register struct shmsysa *uap;
	rval_t *rvp;
{
	register int error;

	switch (uap->opcode) {
	case SHMAT:
		error = shmat((struct shmata *)uap, rvp);
		break;
	case SHMCTL:
		error = shmctl((struct shmctla *)uap, rvp);
		break;
	case SHMDT:
		error = shmdt((struct shmdta *)uap, rvp);
		break;
	case SHMGET:
		error = shmget((struct shmgeta *)uap, rvp);
		break;
	default:
		error = EINVAL;
		break;
	}

	return error;
}

/* XXX why is this here?  No one calls it */
int
shmseg()
{
	return shminfo.shmseg;
}

/*
 * add this record to the segacct list.
 */
STATIC int
sa_add(pp, addr, len, amp)
	struct proc *pp;
	register caddr_t addr;
	size_t len;
	struct anon_map *amp;
{
	register segacct_t *nsap, **sapp;

	nsap = (segacct_t *) kmem_alloc(sizeof(*nsap), KM_SLEEP);
	nsap->sa_addr = addr;
	nsap->sa_len  = len;
	nsap->sa_amp  = amp;

	/* add this to the sorted list */
	sapp = (segacct_t **)&pp->p_segacct;
	while ((*sapp != NULL) && ((*sapp)->sa_addr < addr))
		sapp = &((*sapp)->sa_next);

	ASSERT((*sapp == NULL) || ((*sapp)->sa_addr >= addr + len ));
	nsap->sa_next = *sapp;
	*sapp = nsap;
	return 0;
}

/*
 * Delete this record from the segacct list.
 */
STATIC int
sa_del(pp, addr)
	register struct proc *pp;
	register caddr_t addr;
{
	register segacct_t *osap, **sapp;

	sapp = (segacct_t **)&pp->p_segacct;
	while ((*sapp != NULL) && ((*sapp)->sa_addr < addr))
		sapp = &((*sapp)->sa_next);

	ASSERT((*sapp != NULL) && ((*sapp)->sa_addr == addr));

	osap = *sapp;	/* save pointer to structure for kmem_free */

	*sapp = (*sapp)->sa_next;

	kmem_free(osap, sizeof(*osap));

	return 0;
}

STATIC segacct_t *
sa_find(pp, addr)
	register struct proc *pp;
	register caddr_t addr;
{
	register segacct_t *sap = (segacct_t *)pp->p_segacct;

	while (sap != NULL) {
		if (sap->sa_addr == addr)
			return(sap);
		sap = sap->sa_next;
	}

	return NULL;
}

/* 
 * Duplicate parents segacct records in child.
 */
void
shmfork(ppp, cpp)
	struct proc *ppp;	/* parent proc pointer */
	struct proc *cpp;	/* childs proc pointer */
{
	register segacct_t *sap = (segacct_t *)ppp->p_segacct;

	while (sap != NULL) {
		sa_add(cpp, sap->sa_addr, sap->sa_len, sap->sa_amp);
		/* increment for every shmat */
		sap->sa_amp->refcnt++;
		sap = sap->sa_next;
	}
}

/*
 * Detach shared memory segments from process doing exit.
 */
void
shmexit(pp)
	struct proc *pp;
{
	register segacct_t *sap;
	int error;

	while ((sap=(segacct_t *)pp->p_segacct) != NULL) {
		error = kshmdt(sap->sa_addr);
		if (error != 0) {
			/* delete the record  so we can continue */
			sa_del(pp, sap->sa_addr);
		}
		/*
		 * shmdt has deleted the record, so 
		 * pp->p_segacct now points to the
		 * next record to delete.
		 */
	}
}

/*
 * Detach shared memory segments from process doing exec.
 * We may need to do something different.
 */
void
shmexec(pp)
	struct proc *pp;
{
	shmexit(pp);
}

/*
 * At this time pages should be in memory, so just lock them.
 */

STATIC void 
lock_again(npages, app)
	uint npages;
	struct anon **app;
{
	register struct page *pp;
	struct vnode *vp;
	uint off;

	for (; npages > 0; app++, npages--) {
		ALOCK(*app);
		swap_xlate(*app, &vp, &off);
		pp = page_lookup(vp, off);
		if (pp == NULL)
			cmn_err (CE_PANIC,
			       "lock_again: page not in the system");
		(void) page_pp_lock(pp, 0, 0); 
		AUNLOCK(*app);
	}
}

/* check if this segment is already locked. */

STATIC int 
check_locked(svd, npages)
	struct segvn_data *svd;
	uint npages;
{
	register struct vpage *vpp = svd->vpage;
	register uint i;
	if (svd->vpage == NULL)
		return (0);		/* unlocked */
	for (i = 0; i < npages; i++)
		if ((vpp++)->vp_pplock != 1) 
			return(1);	/* partialy locked */
	return (2);			/* locked */
}
	
 

/*
 * Attach the share memory segment to process
 * address space and lock the pages.
 */

STATIC int
shm_lock(amp)
	register struct anon_map *amp;
{
	register struct anon **app = amp->anon;
	register uint npages = btop(amp->size);
	register struct seg *seg, *sseg;
	struct segvn_crargs crargs;
	struct segvn_data *svd;
	proc_t *p = u.u_procp;
	addr_t addr;
	uint lckflag, error, ret;

	/* check if shared memory is already attached */
	sseg = seg = p->p_as->a_segs; 
	do {
		svd = (struct segvn_data *)seg->s_data;
		if ((seg->s_ops == &segvn_ops) && (svd->amp == amp) && 
		    (amp->size == seg->s_size)) {
			switch (ret = check_locked(svd, npages)) {
			case 0:			/* unlocked */
				if ((error = as_ctl(p->p_as, seg->s_base, 
				     seg->s_size, MC_LOCK, 0,
				     (caddr_t)NULL, (ulong *)NULL,
				     (size_t)NULL)) == 0) 
					lock_again(npages, app);
				(void) as_ctl(p->p_as, seg->s_base, seg->s_size, 
				     MC_UNLOCK, 0, (caddr_t)NULL,
				     (ulong *)NULL, (size_t)NULL);
				return(error);
			case 1:			/* partialy locked */
				break;
			case 2:			/* locked */
				lock_again(npages, app);
				return (0);
			default:
				cmn_err( CE_WARN, "swith(): default %d\n", ret);
				break;
			}
		}
	} while ((seg = seg->s_next) != sseg);

	/* attach shm segment to our address space */

	map_addr(&addr, amp->size, (off_t)0, 1);
	if (addr == NULL)
		return (ENOMEM);

	/* Initialize the create arguments and map the segment */
	crargs = *(struct segvn_crargs *)zfod_argsp;	/* structure copy */
	crargs.offset = 0;
	crargs.type = MAP_SHARED;
	crargs.amp = amp;
	crargs.maxprot = PROT_ALL;
	crargs.prot =  PROT_ALL;

	if (!p->p_as->a_paglck) {
		lckflag = 1;
		p->p_as->a_paglck = 1;
	}
	error = as_map(p->p_as, addr, amp->size, segvn_create, 
		       (caddr_t)&crargs);
	if (!error) {
		lock_again(npages, app);
		(void) as_unmap(p->p_as, addr, amp->size);
	}
	if (lckflag)
		p->p_as->a_paglck = 0;
	return error;
}


/* Unlock shared memory */

STATIC int
shm_unlock(amp, lck) 
	register struct anon_map *amp;
	register uint lck;
{
	register struct anon **app = amp->anon;
	register uint npages = btop(amp->size);
	struct vnode *vp;
	struct page *pp;
	uint off;

	for (; npages != 0; app++, npages--) {
		if (*app == NULL) {
			if (lck)
				cmn_err (CE_PANIC,
				       "shm_unlock: null app");
			continue;
		}
		ALOCK(*app);
		swap_xlate(*app, &vp, &off);
		pp = page_lookup(vp, off);
		if (pp == NULL) { 
			AUNLOCK(*app);
			if (lck)
				cmn_err (CE_PANIC,
				       "shm_unlock: page not in the system");
			continue;
		}
		if (pp->p_lckcnt)
			page_pp_unlock(pp, 0, 0);
		AUNLOCK(*app);
	}
	return 0;
}

/*
 * We call this routine when we have
 * removed all references to this amp.
 * This means all shmdt's and the
 * IPC_RMID have been done.
 */
STATIC void
shm_rm_amp(amp, lckflag)
	register struct anon_map *amp;
	uint lckflag;
{

	/*
	 * If we are finally deleting the
	 * shared memory, and if no one did 
	 * the SHM_UNLOCK, we must do it now.
	 */
	shm_unlock(amp, lckflag);

	/*
	 * Free up the anon_map.
	 */
	anon_unresv(amp->swresv);
	anon_free(amp->anon, amp->size);
	kmem_free((caddr_t)amp->anon,
	    ((amp->size >> PAGESHIFT) * sizeof (struct anon *)));
	kmem_free((caddr_t)amp, sizeof (struct anon_map));

}
