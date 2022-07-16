/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-os:fio.c	1.3"

#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/tss.h"
#include "sys/immu.h"
#include "sys/cred.h"
#include "sys/user.h"
#include "sys/conf.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/file.h"
#include "sys/proc.h"
#include "sys/buf.h"
#include "sys/var.h"
#include "sys/sysinfo.h"
#include "sys/acct.h"
#include "sys/open.h"
#include "sys/cmn_err.h"
#include "sys/evecb.h"
#include "sys/hrtcntl.h"
#include "sys/priocntl.h"
#include "sys/procset.h"
#include "sys/events.h"
#include "sys/evsys.h"
#include "sys/asyncsys.h"
#include "sys/debug.h"
#include "sys/kmem.h"

unsigned int filecnt;
STATIC struct file *file;

/*
 * Convert a user supplied file descriptor into a pointer to a file
 * structure.  Only task is to check range of the descriptor (soft
 * resource limit was enforced at open time and shouldn't be checked
 * here).
 */
int
getf(fd, fpp)
	register int fd;
	struct file **fpp;
{
	register struct ufchunk *ufp;
	register struct file *fp;

	if (fd < 0)
		return EBADF;

	/*
	 * Most commonly fd will be small -- optimize this case.  Note
	 * that code elsewhere must guarantee (u.u_nofiles >= NFPCHUNK).
	 */
	if (fd < NFPCHUNK)  {
		if ((fp = u.u_flist.uf_ofile[fd]) == NULLFP)
			return EBADF;
		*fpp = fp;
		return 0;
	}

	if (fd >= u.u_nofiles)
		return EBADF;
	
	ufp = &u.u_flist;
	do  {
		fd -= NFPCHUNK;
		ufp = ufp->uf_next;
	} while (fd >= NFPCHUNK);

	if ((fp = ufp->uf_ofile[fd]) == NULLFP)
		return EBADF;

	*fpp = fp;
	return 0;
}

void
closeall(flag)
	register int flag;
{
	register i;
	file_t *fp;

	for (i = 0; i < u.u_nofiles; i++) {
		if (getf(i, &fp) == 0) {
			closef(fp);
			if (flag)
				setf(i, NULLFP);
		}
	}
}

/*
 * Internal form of close.  Decrement reference count on file
 * structure.  Decrement reference count on the vnode following
 * removal of the referencing file structure.
 */
int
closef(fp)
	register struct file *fp;
{
	register struct vnode *vp;
	register int error;

	/*
	 * Sanity check.
	 */
	if (fp == NULL || fp->f_count <= 0)
		return 0;

 	stop_aio(u.u_procp, fp);

	vp = fp->f_vnode;
	error =
	  VOP_CLOSE(vp, fp->f_flag, fp->f_count, fp->f_offset, fp->f_cred);
	if ((unsigned)fp->f_count > 1) {
		fp->f_count--;
		return error;
	}

	/* XENIX Support */
	if (vp->v_type == VXNAM)
		closesem(fp, vp);
	vp->v_flag &= ~VXLOCKED;
	/* End XENIX Support */

	VN_RELE(vp);
	crfree(fp->f_cred);
	fp->f_count = 0;
	if (fp->f_prev)
		fp->f_prev->f_next = fp->f_next;
	else
		file = fp->f_next;
	if (fp->f_next)
		fp->f_next->f_prev = fp->f_prev;
	kmem_free((caddr_t)fp, sizeof(file_t));
	filecnt--;
	return error;
}

/*
 * Allocate a user file descriptor greater than or equal to "start" (supplied).
 */
int
ufalloc(start, fdp)
	int start;
	int *fdp;
{
	register struct ufchunk *ufp;
	register int j;
	register int count;
	register int i = start;

	/*
	 * First look for an unused entry.
	 */
	if (i < u.u_nofiles) {
		count = i / NFPCHUNK;
		for (ufp = &u.u_flist; count > 0; count--)
			ufp = ufp->uf_next;
		count = i / NFPCHUNK;
		for (; i < u.u_nofiles; i++) {
			if (i >= u.u_rlimit[RLIMIT_NOFILE].rlim_cur)
				return EMFILE;
			j = i / NFPCHUNK;
			if (j > count) {
				count = j;
				ufp = ufp->uf_next;
			}
			j = i % NFPCHUNK;
			if (ufp->uf_ofile[j] == NULL) {
				/*
			 	* We have to leave a pebble in u.u_rval1 for
			 	* certain old things which expect to find it
			 	* there.
			 	*/
				u.u_rval1 = i;		/* XXX */
				ufp->uf_pofile[j] = 0;
				*fdp = i;
				return 0;
			}
		}
		i = start;
	} else {
		if (i >= u.u_rlimit[RLIMIT_NOFILE].rlim_cur)
			return EMFILE;
	}

	/*
	 * We need to allocate more memory.
	 */
	if (u.u_nofiles >= u.u_rlimit[RLIMIT_NOFILE].rlim_cur)
		return EMFILE;
	for (ufp = &u.u_flist; ufp->uf_next; ufp = ufp->uf_next)
		;
	if (i == 0) {		/* just allocate one more chunk */
		if ((ufp->uf_next = (struct ufchunk *)kmem_zalloc(
		    sizeof(struct ufchunk), 0)) == NULL)
			return ENOMEM;
		u.u_rval1 = u.u_nofiles;		/* XXX */
		*fdp = u.u_nofiles;
		u.u_nofiles += NFPCHUNK;
	} else {

		/*
		 * We could enter this path under two different circumstances.
		 * First, "start" could have been less than u.u_nofiles, but
		 * greater than zero, and there were no free fds.  Here, we
		 * just want to allocate one more chunk and return the first
		 * slot in the chunk.  The second case is when "start" is
		 * greater than or equal to u.u_nofiles.  In this case, we
		 * need to allocate enough chunks to return fd number "start".
		 *
		 * The following calculation computes the number of chunks to
		 * be allocated (it can be less than zero, but this is okay).
		 * The do-while loop allocates at least one chunk.
		 *
		 * Alternatively, we could just jump back to the beginning
		 * of this routine, but that would be less efficient.
		 */
		j = ((i / NFPCHUNK) + 1) - (u.u_nofiles / NFPCHUNK);
		count = u.u_nofiles;
		do {
			if ((ufp->uf_next = (struct ufchunk *)kmem_zalloc(
			    sizeof(struct ufchunk), 0)) == NULL)
				return ENOMEM;
			u.u_nofiles += NFPCHUNK;
			ufp = ufp->uf_next;
		} while (--j > 0);
		if (i < count) {
			u.u_rval1 = count;
			*fdp = count;
		} else {
			u.u_rval1 = i;			/* XXX */
			*fdp = i;
		}
	}
	return 0;
}

/*
 * Allocate a user file descriptor and a file structure.
 * Initialize the descriptor to point at the file structure.
 *
 * file table overflow -- if there are no available file structures.
 */
int
falloc(vp, flag, fpp, fdp)
	struct vnode *vp;
	int flag;
	struct file **fpp;
	int *fdp;
{
	register struct file *fp;
	int fd;
	register int error;

	if (error = ufalloc(0, &fd))
		return error;
	if ((fp = (file_t *)kmem_zalloc(sizeof(file_t), 0)) == NULL) {
		cmn_err(CE_NOTE, "KERNEL: Could not allocate file table entry");
		syserr.fileovf++;
		return ENFILE;
	}
	if (file)
		file->f_prev = fp;
	fp->f_next = file;
	fp->f_prev = NULLFP;
	file = fp;
	filecnt++;
	setf(fd, fp);
	fp->f_count++;
	fp->f_flag = flag;
	fp->f_vnode = vp;
	fp->f_offset = 0;
	fp->f_aiof = (aioreq_t *)&fp->f_aiof;
	fp->f_aiob = (aioreq_t *)&fp->f_aiof;
	crhold(u.u_cred);
	fp->f_cred = u.u_cred;
	*fpp = fp;
	*fdp = fd;
	return 0;
}

void
finit()
{
	file = NULLFP;
	filecnt = 0;
}

void
unfalloc(fp)
	register struct file *fp;
{
	if (--fp->f_count <= 0) {
		if (fp->f_prev)
			fp->f_prev->f_next = fp->f_next;
		else
			file = fp->f_next;
		if (fp->f_next)
			fp->f_next->f_prev = fp->f_prev;
		crfree(fp->f_cred);
		kmem_free(fp, sizeof(file_t));
		filecnt--;
	}
}

/*
 * Given a file descriptor, set the user's
 * file pointer to the given parameter.
 */
void
setf(fd, fp)
	int fd;
	struct file *fp;
{
	register int i;
	register struct ufchunk *ufp;

	ASSERT(0 <= fd && fd < u.u_nofiles);
	ufp = &u.u_flist;
	for (i = (fd / NFPCHUNK); i > 0; i--)
		ufp = ufp->uf_next;
	i = fd % NFPCHUNK;
	ufp->uf_ofile[i] = fp;
}

/*
 * Given a file descriptor, return the user's file flags.
 */
char
getpof(fd)
	int fd;
{
	register int i;
	register struct ufchunk *ufp;

	if (fd >= u.u_nofiles)
		return 0;
	ufp = &u.u_flist;
	for (i = (fd / NFPCHUNK); i > 0; i--)
		ufp = ufp->uf_next;
	i = fd % NFPCHUNK;
	return ufp->uf_pofile[i];
}

/*
 * Given a file descriptor and file flags,
 * set the user's file flags.
 */

void
#ifdef __STDC__
setpof(int fd, char flags)
#else
setpof(fd, flags)
	int fd;
	char flags;
#endif
{
	register int i;
	register struct ufchunk *ufp;

	ASSERT(0 <= fd && fd < u.u_nofiles);
	ufp = &u.u_flist;
	for (i = (fd / NFPCHUNK); i > 0; i--)
		ufp = ufp->uf_next;
	i = fd % NFPCHUNK;
	ufp->uf_pofile[i] = flags;
}

/*
 * Search the file table looking for a file with the
 * same vnode and see if someone has it open for writing
 * before we try to exec it.  Return 1 if the file is
 * found and 0 otherwise.
 */
int
filesearch(vp)
	register struct vnode *vp;
{
	register struct file *fp;

	for (fp = file; fp; fp = fp->f_next)
		if (fp->f_count && fp->f_vnode == vp
		  && (fp->f_flag & FWRITE))
			return 1;
	return 0;
}

/*
 * Allocate a file descriptor and assign it to the vnode "*vpp",
 * performing the usual open protocol upon it and returning the
 * file descriptor allocated.  It is the responsibility of the
 * caller to dispose of "*vpp" if any error occurs.
 */
int
fassign(vpp, mode, fdp)
	struct vnode **vpp;
	int mode;
	int *fdp;
{
	struct file *fp;
	register int error;
	int fd;

	if (error = falloc((struct vnode *)NULL, mode & FMASK, &fp, &fd))
		return error;
	if (error = VOP_OPEN(vpp, mode, u.u_cred)) {
		setf(fd, NULLFP);
		unfalloc(fp);
		return error;
	}
	fp->f_vnode = *vpp;
	*fdp = fd;
	return 0;
}
