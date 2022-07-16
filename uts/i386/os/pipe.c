/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-os:pipe.c	1.3"

#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/immu.h"
#include "sys/tss.h"
#include "sys/cred.h"
#include "sys/user.h"
#include "sys/fstyp.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/file.h"
#include "sys/debug.h"
#include "sys/conf.h"
#include "sys/fs/fifonode.h"
#include "sys/stream.h"
#include "sys/strsubr.h"


/*
 * pipe(2) system call.
 * Create a pipe by connecting two streams together. Associate
 * each end of the pipe with a vnode, a file descriptor and 
 * one of the streams.
 */
/*ARGSUSED*/
int
pipe(uap, rvp)
	char *uap;
	rval_t *rvp;
{
	extern int fifo_stropen(), strclean();
	extern int strclose();
	extern struct vnode *makepipe();
	struct vnode *vp1, *vp2;
	extern ushort fifogetid();
	struct file *fp1, *fp2;
	register int error = 0;
	int fd1, fd2;
	static ushort pipeino = 1;

	/*
	 * Allocate and initialize two vnodes. 
	 */
	if ((vp1 = makepipe()) == NULL)
		return ENOMEM;
	if ((vp2 = makepipe()) == NULL) {
		VN_RELE(vp1);
		return ENOMEM;
	}
	/*
	 * Allocate and initialize two file table entries and two
	 * file pointers. Each file pointer is open for read and
	 * write.
	 */
	if (error = falloc(vp1, FWRITE|FREAD, &fp1, &fd1)) {
		VN_RELE(vp1);
		VN_RELE(vp2);
		return (error);
	}
	if (error = falloc(vp2, FWRITE|FREAD, &fp2, &fd2))
		goto out2;

	/*
	 * Create two stream heads and attach to each vnode.
	 */
	if (error = fifo_stropen(&vp1, 0, fp1->f_cred))
		goto out;

	if (error = fifo_stropen(&vp2, 0, fp2->f_cred)) {
		strclean(vp1);
		strclose(vp1, 0, fp1->f_cred);
		goto out;
	}
	/*
	 * Twist the stream head queues so that the write queue
	 * points to the other stream's read queue.
	 */
	vp1->v_stream->sd_wrq->q_next = RD(vp2->v_stream->sd_wrq);
	vp2->v_stream->sd_wrq->q_next = RD(vp1->v_stream->sd_wrq);
	/*
	 * Tell each pipe about its other half.
	 */
	VTOF(vp1)->fn_mate = vp2;
	VTOF(vp2)->fn_mate = vp1;
	pipeino = fifogetid(pipeino);
	VTOF(vp1)->fn_ino = VTOF(vp2)->fn_ino = pipeino;

	/*
	 * Return the file descriptors to the user. They now
	 * point to two different vnodes which have different
	 * stream heads.
	 */
	rvp->r_val1 = fd1;
	rvp->r_val2 = fd2;
	return (0);
out:
	unfalloc(fp2);
	setf(fd2, NULLFP);
out2:
	unfalloc(fp1);
	setf(fd1, NULLFP);
	VN_RELE(vp1);
	VN_RELE(vp2);
	return (error);
}
