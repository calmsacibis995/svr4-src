/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident  "@(#)xx:io/xxinode.c	1.2.2.4"

#include "sys/types.h"
#include "sys/buf.h"
#include "sys/cmn_err.h"
#include "sys/conf.h"
#include "sys/cred.h"
#include "sys/debug.h"
#include "sys/errno.h"
#include "sys/time.h"
#include "sys/file.h"
#include "sys/kmem.h"
#include "sys/open.h"
#include "sys/param.h"
#include "sys/stat.h"
#include "sys/swap.h"
#include "sys/sysinfo.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/var.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/mode.h"
#include "sys/dnlc.h"
#include "sys/user.h"

#include "sys/proc.h"	/* XXX -- needed for user-context kludge in ILOCK */
#include "sys/disp.h"	/* XXX */

#include "sys/fs/s5param.h"
#include "sys/fs/s5dir.h"
#include "sys/fs/s5ino.h"
#include "sys/fs/s5inode.h"
#include "sys/fs/s5macros.h"

#include "vm/page.h"
#include "vm/pvn.h"
#include "vm/seg.h"

#include "fs/fs_subr.h"

extern struct seg *segkmap;
extern long int xxninode;
/*
 * inode hashing.
 */

#define	NHINO	128
#define xxihash(X)	(&xxhinode[(int) (X) & (NHINO-1)])

struct	hinode	{
	struct	inode	*i_forw;
	struct	inode	*i_back;
} xxhinode[NHINO];

struct inode *xxinode;
struct ifreelist xxifreelist;


STATIC void xxipfree();
void xxiupdat(), xxiunhash(), xxtloop();
int  xxitrunc();
extern struct vnodeops xxvnodeops;
extern struct vfsops xxvfsops;

/*
 * Allocate and initialize inodes.
 */
void
xxinoinit()
{
	register struct inode *ip;
	register int i;

	if ((xxinode = (inode_t *)kmem_zalloc(xxninode*sizeof(inode_t), KM_SLEEP))
	  == NULL)
		cmn_err(CE_PANIC, "xxinoinit: no memory for inodes");
	xxifreelist.av_forw = xxifreelist.av_back = (inode_t *) &xxifreelist;
	for (i = 0; i < NHINO; i++)
		xxhinode[i].i_forw = xxhinode[i].i_back = (inode_t *) &xxhinode[i];
	for (i = 0, ip = xxinode; i < xxninode; i++, ip++) {
		ip->i_forw = ip->i_back = ip;
		xxifreelist.av_forw->av_back = ip;
		ip->av_forw = xxifreelist.av_forw;
		xxifreelist.av_forw = ip;
		ip->av_back = (struct inode *) &xxifreelist;
	}
}

/*
 * Look up an inode by vfs and i-number.  If it's in core, honor
 * the locking protocol.  If it's not in core, read it in from the
 * associated device.  In all cases, a pointer to a locked inode
 * structure is returned.
 */
int
xxiget(vfsp, ino, ipp)
	register struct vfs *vfsp;
	register int ino;
	struct inode **ipp;
{
	register struct inode *ip;
	register struct hinode *hip;
	register struct vnode *vp;
	register struct inode *iq;
	int error;

	sysinfo.iget++;
	*ipp = NULL;
loop:
	hip = xxihash(ino);
	for (ip = hip->i_forw; ip != (struct inode *) hip; ip = ip->i_forw) {
		vp = ITOV(ip);
		if (ino == ip->i_number && vfsp == vp->v_vfsp)
			goto found;
	}

	/*
	 * XXX -- If inode freelist is empty, toss out name cache entries
	 * in an attempt to reclaim some inodes.  Give up only when
	 * there are no more name cache entries to purge.
	 */
	while (xxifreelist.av_forw == (struct inode *) &xxifreelist
	  && dnlc_purge1() == 1)
		;
		
	if ((ip = xxifreelist.av_forw) == (struct inode *) &xxifreelist) {
		cmn_err(CE_WARN, "xxiget - inode table overflow");
		syserr.inodeovf++;
		return ENFILE;
	}
	vp = ITOV(ip);
	if (ip->i_mode != 0 && vp->v_pages != NULL)
		sysinfo.s5ipage++;
	else
		sysinfo.s5inopage++;

	/* ASSERT(vp->v_count == 0); */
	/*
	 * Remove inode from free list.  Leave it on its hash chain
	 * until after xxsyncip() has been applied.
	 */
	ASSERT(ip == ip->av_back->av_forw);
	ASSERT(ip == ip->av_forw->av_back);
	ip->av_back->av_forw = ip->av_forw;
	ip->av_forw->av_back = ip->av_back;

/*
 * The following code checks to be sure that putpages from the page layer
 * have not activated the vnode while the inode is on the free list. If
 * we hit this case we put the inode back on the tail of the free list
 * and try again. If there are not other inodes on the free list then
 * we put the inode back and must call preempt so that some other process
 * can do work to free an inode.
 */
	if (ITOV(ip)->v_count > 0) {
		/*
		 *	Put inode on end of freelist.
		 */
		xxifreelist.av_back->av_forw = ip;
		ip->av_forw = (struct inode *) &xxifreelist;
		ip->av_back = xxifreelist.av_back;
		xxifreelist.av_back = ip;

		if (xxifreelist.av_forw == ip ) {
			/*
			 *	only 1 inode left!
			 */
			preempt();
		}

		goto loop;
	}

	ASSERT((ip->i_flag & ILOCKED) == 0);
	ILOCK(ip);

	/*
	 * When the inode was put on the free list in xxinactive(),
	 * we did an asynchronous xxsyncip() there.  Here we call
	 * xxsyncip() to synchronously wait for any pages that are
	 * still in transit, to invalidate all the pages on the vp,
	 * and finally to write back the inode to disk.  Since
	 * xxsyncip() may sleep, someone may find and try to acquire
	 * the inode in the meantime; if so we put it back on the
	 * free list and loop around to find another free inode.
	 */
	if (((vp)->v_vfsp && xxsyncip(ip, B_INVAL) != 0)
	  || (ip->i_flag & IWANT)) {
		xxipfree(ip);
		IUNLOCK(ip);
		goto loop;
	}

	/*
	 * Since we may have slept, we need to check to
	 * see if someone else has entered this inode.
	 * If we find it, free ip, and use the found ip.
	 */
	hip = xxihash(ino);
	for (iq = hip->i_forw; iq != (struct inode *) hip; iq = iq->i_forw) {
		register struct vnode *qvp;

		qvp = ITOV(iq);
		if (ino == iq->i_number && vfsp == qvp->v_vfsp) {
			xxipfree(ip);
			IUNLOCK(ip);
			ip = iq;
			vp = qvp;
			goto found;
		}
	}
        if (vp->v_count != 0) {
		cmn_err(CE_CONT, "vp count not 0 vp: %x	 ip: %x\n", vp, ip);
		call_demon();
	}

	if (vp->v_pages != NULL) {
		cmn_err(CE_CONT, "vp: %x  ip: %x\n", vp, ip);
		call_demon();
	}
	/* ASSERT(vp->v_pages == NULL); */

	/*
	 * Remove from old hash chain and insert into new one.
	 */
	ip->i_back->i_forw = ip->i_forw;
	ip->i_forw->i_back = ip->i_back;
	hip->i_forw->i_back = ip;
	ip->i_forw = hip->i_forw;
	hip->i_forw = ip;
	ip->i_back = (struct inode *) hip;
	
	if (ip->i_map)
		xxfreemap(ip);

	/*
	 * Fill in the rest.
	 */
	vp->v_flag = 0;
	vp->v_count = 1;
	vp->v_vfsmountedhere = NULL;
	vp->v_op = &xxvnodeops;
	vp->v_vfsp = vfsp;
	vp->v_stream = NULL;
	vp->v_pages = NULL;
	vp->v_data = (caddr_t)ip;
	vp->v_filocks = NULL;
	ip->i_vcode = 0;
	ip->i_mapcnt = 0;
	if ((error = xxiread(ip, ino))
	  || ((vp->v_type = IFTOVT((int)ip->i_mode)) == VREG
	    && ((error = fs_vcode(vp, &ip->i_vcode)) != 0))) {
		xxiunhash(ip);
		xxipfree(ip);
		vp->v_data = NULL;
		vp->v_count = 0;
		IUNLOCK(ip);
	} else {
		vp->v_rdev = ip->i_rdev;
		*ipp = ip;
	}
	return error;

found:
	if ((ip->i_flag & (IRWLOCKED|ILOCKED))
	  && ip->i_owner != curproc->p_slot) {	/* XXX */
		ip->i_flag |= IWANT;
		sleep((caddr_t) ip, PINOD);
		goto loop;
	}

	if (vp->v_count == 0) {
		/*
		 * Remove from freelist.
		 */
		ASSERT(ip->av_back->av_forw == ip);
		ASSERT(ip->av_forw->av_back == ip);
		ip->av_back->av_forw = ip->av_forw;
		ip->av_forw->av_back = ip->av_back;
	}
	VN_HOLD(vp);
	ASSERT((ip->i_flag & ILOCKED) == 0);
	ILOCK(ip);
	*ipp = ip;
	return 0;
}

/*
 * Decrement reference count of an inode structure.
 * On the last reference, write the inode out and if necessary,
 * truncate and deallocate the file.
 */
void
xxiput(ip)
	register struct inode *ip;
{
	struct vnode *vp = ITOV(ip);

	ASSERT(ip->i_flag & ILOCKED);
	ASSERT(vp->v_count > 0);
	ITIMES(ip);
	IUNLOCK(ip);
	VN_RELE(vp);
}

/* ARGSUSED */
void
xxiinactive(ip, cr)
	register struct inode *ip;
	struct cred *cr;
{

	/* if inode has already been freed, just return */
	if (ip->av_back->av_forw == ip || ip->av_forw->av_back == ip)
		return;

        /*
         * Mark iinactive in progress.	This allow VOP_PUTPAGE to abort
	 * a concurrent attempt to flush a page due to pageout/fsflush.
	 */
	ASSERT((ip->i_flag & IINACTIVE) == 0);
	ip->i_flag |= IINACTIVE;

	/* xxitrunc may take some time, so preempt */
	/* PREEMPT(); this is the wrong place, may corrupt inode */
	ILOCK(ip);
	if (ip->i_nlink <= 0) {
		ip->i_gen++;
		(void)xxitrunc(ip);
		ip->i_flag |= IUPD|ICHG;
		xxifree(ip);
	} else if (!IS_SWAPVP(ITOV(ip))) {
		/*
		 * Do an async write (B_ASYNC) of the pages
		 * on the vnode and put the pages on the free
		 * list when we are done (B_FREE).  This action
		 * will cause all the pages to be written back
		 * for the file now and will allow update() to
		 * skip over inodes that are on the free list.
		 */
		(void) xxsyncip(ip, B_FREE | B_ASYNC);
	}

	if (ip->i_flag & (IACC|IUPD|ICHG|IMOD)) {
		if ((ip->i_flag & IUPD)
		  && (ip->i_mode & IFMT) == IFREG && ip->i_map)
			xxfreemap(ip);
		/*
		 * Only call xxiupdat if an ifree has not been done; this
		 * avoids a race whereby an ifree could put an inode on
		 * the freelist, the inode could be allocated, and then
		 * the xxiupdat could put outdated information into the
		 * disk inode.
		 */
		xxiupdat(ip);
	}

        /* Clear the IINACTIVE flag */
	ip->i_flag &= ~IINACTIVE;

        ASSERT((ITOV(ip))->v_count == 0);
	xxipfree(ip);

	IUNLOCK(ip);
	PREEMPT();
}

/*
 * Purge any cached inodes on the given VFS.  If "force" is 0,
 * -1 is returned if an active inode (other than the filesystem root)
 * is found, otherwise 0.  If "force" is non-zero, the search
 * doesn't stop if an active inode is encountered.
 */
int
xxiflush(vfsp, force)
	register struct vfs *vfsp;
	int force;
{
	register struct inode *ip;
	register struct vnode *vp, *rvp = S5VFS(vfsp)->vfs_root;
	register int i;
	dev_t dev = vfsp->vfs_dev;

	ASSERT(rvp != NULL);
	/*
	 * This search should run through the hash chains (rather
	 * than the entire inode table) so that we only examine
	 * inodes that we know are currently valid.
	 */
	for (i = 0, ip = xxinode; i < xxninode; i++, ip++) {
		if (ip->i_dev == dev) {
			vp = ITOV(ip);
			if (vp == rvp) {
				if (vp->v_count > 1 && force == 0)
					return -1;
				ILOCK(ip);
				(void) xxsyncip(ip, B_INVAL);
				IUNLOCK(ip);
				continue;
			}
			if (vp->v_count == 0) {
				if (vp->v_vfsp == 0)
					continue;
				if ((ip->i_flag & IRWLOCKED)
				  || (ip->i_flag & ILOCKED)) {
					if (force)
						continue;
					return -1;
				}
				/*
				 * Thoroughly dispose of this inode.  Flush
				 * any associated pages and remove it from
				 * its hash chain.
				 */
				ILOCK(ip);	/* Won't sleep */
				if (ip->i_map)
					xxfreemap(ip);
				(void) xxsyncip(ip, B_INVAL);
				if (ip->i_flag & IWANT) {
					IUNLOCK(ip);
					if (force)
						continue;
					return -1;
				}
				IUNLOCK(ip);
				xxiunhash(ip);
			} else if (force == 0)
				return -1;
		}
	}
	return 0;
}

/*
 * Put an in-core inode on the free list.
 */
STATIC void
xxipfree(ip)
	register struct inode *ip;
{
	ASSERT(ip->av_back->av_forw != ip && ip->av_forw->av_back != ip);

	if ((ip->i_mode == 0 || ITOV(ip)->v_pages == NULL)
	  && xxifreelist.av_forw != (struct inode *)&xxifreelist) {
		ip->av_forw = xxifreelist.av_forw;
		xxifreelist.av_forw->av_back = ip;
		ip->av_back = (struct inode *)&xxifreelist;
		xxifreelist.av_forw = ip;
	} else {
		xxifreelist.av_back->av_forw = ip;
		ip->av_forw = (struct inode *) &xxifreelist;
		ip->av_back = xxifreelist.av_back;
		xxifreelist.av_back = ip;
	}
}

/*
 * Remove an inode from its hash list.
 */
void
xxiunhash(ip)
	register struct inode *ip;
{
	ip->i_back->i_forw = ip->i_forw;
	ip->i_forw->i_back = ip->i_back;
	ip->i_forw = ip->i_back = ip;
}

int
xxiread(ip, ino)
	register struct inode *ip;
	int ino;
{
	register char *p1, *p2;
	register struct vnode *vp = ITOV(ip);
	register struct dinode *dp;
	struct buf *bp;
	register unsigned i;
	register struct vfs *vfsp;
	register struct s5vfs *s5vfsp;
	int error;

	vfsp = vp->v_vfsp;
	s5vfsp = S5VFS(vfsp);
	i = vfsp->vfs_bsize;
	bp = bread(vfsp->vfs_dev, FsITOD(s5vfsp, ino), i);
	if (error = geterror(bp)) {
		brelse(bp);
		return error;
	}
	dp = (struct dinode *)bp->b_un.b_addr;
	dp += FsITOO(s5vfsp, ino);
	ip->i_nlink = dp->di_nlink;
	ip->i_uid = dp->di_uid;
	ip->i_gid = dp->di_gid;
	ip->i_size = dp->di_size;
	ip->i_mode = dp->di_mode;
	ip->i_atime = dp->di_atime;
	ip->i_mtime = dp->di_mtime;
	ip->i_ctime = dp->di_ctime;
	ip->i_number = (o_ino_t)ino;
	ip->i_dev = vfsp->vfs_dev;
	ip->i_nextr = 0;
	ip->i_gen = dp->di_gen;
	p1 = (char *) ip->i_addr;
	p2 = (char *) dp->di_addr;
	for (i = 0; i < NADDR; i++) {
#ifndef i386
		*p1++ = 0;
#endif
		*p1++ = *p2++;
		*p1++ = *p2++;
		*p1++ = *p2++;
#ifdef i386
		*p1++ = 0;
#endif
	}

	if (ip->i_mode & IFBLK || ip->i_mode == IFCHR) {
		if (ip->i_bcflag & NDEVFORMAT)
			ip->i_rdev = makedevice(ip->i_major, ip->i_minor);
		else
			ip->i_rdev = expdev(ip->i_oldrdev);
	} else if (ip->i_mode & IFNAM)
		ip->i_rdev = ip->i_oldrdev;

	brelse(bp);
	return 0;
}

/*
 * Flush inode to disk, updating timestamps if requested.
 */
void
xxiupdat(ip)
	register struct inode *ip;
{
	struct buf *bp;
	register struct vnode *vp = ITOV(ip);
	register struct s5vfs *s5vfsp = S5VFS(vp->v_vfsp);
	register struct dinode *dp;
	register char *p1;
	register char *p2;
	register unsigned i;

	ASSERT(ip->i_flag & ILOCKED);
	if (vp->v_vfsp->vfs_flag & VFS_RDONLY)
		return;
	i = VBSIZE(vp);
	bp = bread(ip->i_dev, FsITOD(s5vfsp, ip->i_number), i);
	if (bp->b_flags & B_ERROR) {
		brelse(bp);
		return;
	}
	dp = (struct dinode *)bp->b_un.b_addr;
	dp += FsITOO(s5vfsp, ip->i_number);
	dp->di_mode = ip->i_mode;
	dp->di_nlink = ip->i_nlink;
	dp->di_uid = ip->i_uid;
	dp->di_gid = ip->i_gid;
	dp->di_size = ip->i_size;
	dp->di_gen = ip->i_gen;
	p1 = (char *)dp->di_addr;
	p2 = (char *)ip->i_addr;
	for (i = 0; i < NADDR; i++) {
#ifndef i386
		p2++;
#endif
		*p1++ = *p2++;
		*p1++ = *p2++;
		*p1++ = *p2++;
#ifdef i386
		p2++;
#endif
	}
	if (ip->i_flag & IACC)
		ip->i_atime = hrestime.tv_sec;
	if (ip->i_flag & IUPD) 
		ip->i_mtime = hrestime.tv_sec;
	if (ip->i_flag & ICHG) 
		ip->i_ctime = hrestime.tv_sec;
	dp->di_atime = ip->i_atime;
	dp->di_mtime = ip->i_mtime;
	dp->di_ctime = ip->i_ctime;
	if (ip->i_flag & ISYN)
		bwrite(bp);
	else
		bdwrite(bp);
	ip->i_flag &= ~(IACC|IUPD|ICHG|ISYN|IMOD);
}

/*
 * Update times on inode.
 */
void
xxiuptimes(ip)
	register struct inode *ip;
{
	if (ip->i_flag & (IACC|IUPD|ICHG)) {
		ip->i_flag |= IMOD;
		if (ip->i_flag & IACC)
			ip->i_atime = hrestime.tv_sec;
		if (ip->i_flag & IUPD) 
			ip->i_mtime = hrestime.tv_sec;
		if (ip->i_flag & ICHG) 
			ip->i_ctime = hrestime.tv_sec;
		ip->i_flag &= ~(IACC|IUPD|ICHG);
	}
}

/*
 * Free all the disk blocks associated with the specified inode
 * structure.  The blocks of the file are removed in reverse order.
 * This FILO algorithm will tend to maintain a contiguous free list
 * much longer than FIFO.
 *
 * Update inode first with zero size and block addrs to ensure sanity.
 * Save blocks addrs locally to free.
 */
int
xxitrunc(ip)
	register struct inode *ip;
{
	register int i, type;
	register struct vnode *vp = ITOV(ip);
	register struct vfs *vfsp;
	register daddr_t bn;
	daddr_t save[NADDR];

	ASSERT(ip->i_flag & ILOCKED);
	type = ip->i_mode & IFMT;
	if (type != IFREG && type != IFDIR && type != IFLNK)
		return 0;

	/*
	 * If file is currently in use for swap, disallow truncate-down.
	 */
	if (ip->i_size > 0 && IS_SWAPVP(vp))
		return EBUSY;


	if ((ip->i_mode & IFMT) == IFREG && ip->i_map)
		xxfreemap(ip);

	/*
	 * Update the pages associated with the file.
	 */
	pvn_vptrunc(vp, 0, (u_int) 0);

	vfsp = vp->v_vfsp;
	ip->i_size = 0;
	for (i = NADDR - 1; i >= 0; i--) {
		save[i] = ip->i_addr[i];
		ip->i_addr[i] = 0;
	}
	ip->i_flag |= IUPD|ICHG|ISYN;
	xxiupdat(ip);

	for (i = NADDR - 1; i >= 0; i--) {
		if ((bn = save[i]) == 0)
			continue;

		switch (i) {

		default:
			xxblkfree(vfsp, bn);
			break;

		case NADDR-3:
			xxtloop(vfsp, bn, 0, 0);
			break;

		case NADDR-2:
			xxtloop(vfsp, bn, 1, 0);
			break;

		case NADDR-1:
			xxtloop(vfsp, bn, 1, 1);
		}
	}
	return(0);
}

void
xxtloop(vfsp, bn, f1, f2)
	register struct vfs *vfsp;
	daddr_t bn;
{
	dev_t dev;
	register i;
	register struct buf *bp;
	register daddr_t *bap;
	register daddr_t nb;
	struct s5vfs *s5vfsp = S5VFS(vfsp);

	dev = vfsp->vfs_dev;
	bp = NULL;
	for (i = s5vfsp->vfs_nindir-1; i >= 0; i--) {
		if (bp == NULL) {
			bp = bread(dev, bn, vfsp->vfs_bsize);
			if (bp->b_flags & B_ERROR) {
				brelse(bp);
				return;
			}
			bap = bp->b_un.b_daddr;
		}
		nb = bap[i];
		if (nb == (daddr_t)0)
			continue;
		/*
		 * Move following 2 lines out of "if" so that buffer
		 * guaranteed to be released before calling mfree, thus
		 * avoiding the rare deadlock whereby we would have a
		 * buffer locked here but couldn't get the super block lock,
		 * and someone in alloc would have the super block lock and
		 * would not be able to get the buffer lock that is locked
		 * here.
		 */
		brelse(bp);
		bp = NULL;
		if (f1)
			xxtloop(vfsp, nb, f2, 0);
		else
			xxblkfree(vfsp, nb);
	}
	if (bp != NULL)
		brelse(bp);
	xxblkfree(vfsp, bn);
}

/*
 * Lock an inode.
 */
void
xxilock(ip)
	register struct inode *ip;
{
	ILOCK(ip);
}

/*
 * Unlock an inode.
 */
void
xxiunlock(ip)
	register struct inode *ip;
{
	IUNLOCK(ip);
}

int xxfstype;

void
xxinit(vswp, fstype)
	struct vfssw *vswp;
	int fstype;
{
	xxinoinit();
	vswp->vsw_vfsops = &xxvfsops;
	xxfstype = fstype;
}

/*
 * Check mode permission on inode.  Mode is READ, WRITE or EXEC.
 * In the case of WRITE, the read-only status of the file system
 * is checked.  Also in WRITE, prototype text segments cannot be
 * written.  The mode is shifted to select the owner/group/other
 * fields.  The super user is granted all permissions.
 */
int
xxiaccess(ip, mode, cr)
	register struct inode *ip;
	register int mode;
	register struct cred *cr;
{
	register struct vnode *vp = ITOV(ip);

	if ((mode & IWRITE) && (vp->v_vfsp->vfs_flag & VFS_RDONLY))
		return EROFS;
	if (cr->cr_uid == 0)
		return 0;
	if (cr->cr_uid != ip->i_uid) {
		mode >>= 3;
		if (!groupmember(ip->i_gid, cr))
			mode >>= 3;
	}
	if ((ip->i_mode & mode) == mode)
		return 0;
	if ((ip->i_mode&IEXEC == IEXEC) && is286EMUL)
		return 0;
	return EACCES;
}

/*
 * Flush all the pages associated with an inode using the given flags,
 * then force inode information to be written back.
 */
int
xxsyncip(ip, flags)
	register struct inode *ip;
	int flags;
{
	int error;
	register struct vnode *vp = ITOV(ip);

	if (vp->v_pages == NULL || vp->v_type == VCHR)
		error = 0;
	else
		error = VOP_PUTPAGE(vp, 0, 0, flags, (struct cred *)0);
	if (ip->i_flag & (IUPD|IACC|ICHG|IMOD)) {
		if ((flags & B_ASYNC) == 0)
			ip->i_flag |= ISYN;
		xxiupdat(ip);
	}
	return error;
}

void
xxinull(vfsp)
	register struct vfs *vfsp;
{
	register struct inode *ip;
	register struct vnode *vp, *rvp = S5VFS(vfsp)->vfs_root;
	register int i;
	dev_t dev = vfsp->vfs_dev;

	ASSERT(rvp != NULL);

	for (i = 0, ip = xxinode; i < xxninode; i++, ip++)
		if (ip->i_dev == dev) {
			vp = ITOV(ip);
			if (vp == rvp)
				continue;
			vp->v_vfsp = 0;
		}

}
