/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-io:connld.c	1.3.1.2"
/*
 * This module establishes a unique connection on
 * a STREAMS-based pipe.
 */
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/immu.h"
#include "sys/user.h"
#include "sys/fstyp.h"
#include "sys/stropts.h"
#include "sys/stream.h"
#include "sys/strsubr.h"
#include "sys/vnode.h"
#include "sys/file.h"
#include "sys/fs/fifonode.h"

/*
 * Define local and external routines.
 */
int connopen(), connclose(), connput();
struct vnode *connvnode();
extern int fifo_close(), fifo_stropen();
extern int strclose(), strioctl(), falloc();
extern void setf(), unfalloc();
extern struct qinit strdata;

/*
 * Define STREAMS header information.
 */
static struct module_info conn_info = {
	1003, 
	"conn", 
	0, 
	INFPSZ, 
	STRHIGH, 
	STRLOW 
};
static struct qinit connrinit = { 
	connput, 
	NULL, 
	connopen, 
	connclose, 
	NULL, 
	&conn_info, 
	NULL 
};
static struct qinit connwinit = { 
	connput, 
	NULL, 
	NULL, 
	NULL, 
	NULL, 
	&conn_info, 
	NULL
};
struct streamtab conninfo = { 
	&connrinit, 
	&connwinit 
};

/*
 * For each invokation of connopen(), create a new pipe. One end of the pipe 
 * is sent to the process on the other end of this STREAM. The vnode for 
 * the other end is returned to the open() system call as the vnode for 
 * the opened object.
 *
 * On the first invokation of connopen(), a flag is set and the routine 
 * retunrs 0, since the first open corresponds to the pushing of the module.
 */
/*ARGSUSED*/
int
connopen(rqp, dev, flag, sflag)
queue_t *rqp;
dev_t dev;
int flag;
int sflag;
{
	register int error = 0;
	struct vnode *vp1;
	struct vnode *vp2;
	struct vnode *streamvp = NULL; 
	struct vnode *makepipe();
	struct file *filep;
	struct fifonode *streamfnp = NULL;
	struct fifonode *matefnp = NULL;
	int fd, rvalp;

	if ((streamvp = connvnode(rqp)) == NULL) {
		u.u_error = EINVAL;
		return(OPENFAIL);
	}
	/*
	 * CONNLD is only allowed to be pushed onto a "pipe" that has both 
	 * of its ends open.
	 */
	if (streamvp->v_type != VFIFO) {
		u.u_error = EINVAL;
		return(OPENFAIL);
	}
	if (!(VTOF(streamvp)->fn_flag & ISPIPE) || 
		!(VTOF(streamvp)->fn_mate)) {
			u.u_error = EPIPE;
			return(OPENFAIL);
	}
	/*
	 * If this is the first time CONNLD was opened while on this stream,
	 * it is being pushed. Therefore, set a flag and return 0.
	 */
	if ((int)rqp->q_ptr == 0) { 
		rqp->q_ptr = (caddr_t)1;
		return (0);
	}
	/*
	 * Get two vnodes that will represent the pipe ends for the new pipe.
	 */
	if ((vp1 = makepipe()) == NULL)
		return (ENOMEM);
	if ((vp2 = makepipe()) == NULL) {
		VN_RELE(vp1);
		return (ENOMEM);
	}
	/*
	 * Allocate a file descriptor and file pointer for one of the pipe 
	 * ends. The file descriptor will be used to send that pipe end to 
	 * the process on the other end of this stream.
	 */
	if (error = falloc(vp1, FWRITE|FREAD, &filep, &fd)) {
		VN_RELE(vp1);
		VN_RELE(vp2);
		return (error);
	}
	/*
	 * Create two new stream heads and attach them to the two vnodes for
	 * the new pipe.
	 */
	if ((error = fifo_stropen(&vp1, 0, filep->f_cred)) || 
		(error = fifo_stropen(&vp2, 0, filep->f_cred))) {
			if (vp1->v_stream)
				strclose(vp1, 0, filep->f_cred);
			unfalloc(filep);
			setf(fd, NULLFP);
			VN_RELE(vp1);
			VN_RELE(vp2);
			return(error);
	}
	/*
	 * Twist queue pointers so that write queue points to read queue.
	 */
	vp1->v_stream->sd_wrq->q_next = RD(vp2->v_stream->sd_wrq);
	vp2->v_stream->sd_wrq->q_next = RD(vp1->v_stream->sd_wrq);
	/*
	 * Tell each pipe end about its mate.
	 */
	VTOF(vp1)->fn_mate = vp2;
	VTOF(vp2)->fn_mate = vp1;
	/*
	 * Send one end of the new pipe to the process on the other 
	 * end of this pipe and block until the other process
	 * received it.
	 * If the other process exits without receiving it, fail this open
	 * request.
	 */
	streamfnp = VTOF(streamvp);
	matefnp = VTOF(streamfnp->fn_mate);
	matefnp->fn_flag |= FIFOSEND;
	error = strioctl(streamvp, I_SENDFD, fd, flag, K_TO_K, filep->f_cred,
	    &rvalp);
	if (error != 0)
		goto out;

	while (matefnp->fn_flag & FIFOSEND) {
		if (sleep((caddr_t) &matefnp->fn_unique, PPIPE|PCATCH)) {
			error = OPENFAIL;
			goto out;
		}
		if (streamfnp->fn_mate == NULL) {
			error = OPENFAIL;
			goto out;
		}
	}
	/*
	 * all is okay...return new pipe end to user
	 */
	streamfnp->fn_unique = vp2;
	streamfnp->fn_flag |= FIFOPASS;
	closef(filep);
	setf(fd, NULLFP);
	return 0;
out:
	streamfnp->fn_unique = NULL;
	streamfnp->fn_flag &= ~FIFOPASS;
	matefnp->fn_flag &= ~FIFOSEND;
	fifo_close(vp2, 0, 0, 0, filep->f_cred);
	VN_RELE(vp2);
	closef(filep);
	setf(fd, NULLFP);
	return(error);
}

/*ARGSUSED*/
int
connclose(q)
queue_t *q;
{
	return (0);
}

/*
 * Use same put procedure for write and read queues.
 */
int
connput(q, bp)
queue_t *q;
mblk_t *bp;
{
	putnext(q, bp);
	return (0);
}

/*
 * Get the vnode for the stream connld is push onto. Follow the 
 * read queue until the stream head is reached. The vnode is taken 
 * from the stdata structure, which is obtaine from the q_ptr field 
 * of the queue.
 */
struct vnode *
connvnode(qp)
queue_t *qp;
{
	queue_t *tempqp;
	struct vnode *streamvp = NULL;
	
	for(tempqp = qp; tempqp->q_next; tempqp = tempqp->q_next)
		;
	if (tempqp->q_qinfo != &strdata)
		return (NULL);
	streamvp = ((struct stdata *)(tempqp->q_ptr))->sd_vnode;
	return (streamvp);
}
