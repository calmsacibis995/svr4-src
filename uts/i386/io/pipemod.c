/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)kern-io:pipemod.c	1.3"
/*
 * This module switches the read and write flush bits for each
 * M_FLUSH control message it receives. It's intended usage is to
 * properly flush a STREAMS-based pipe.
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
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/vnode.h"
#include "sys/file.h"

int pipeopen(), pipeclose(), pipeput();

static struct module_info pipe_info = {
	1003,
	"pipe",
	0,
	INFPSZ,
	STRHIGH,
	STRLOW };
static struct qinit piperinit = { 
	pipeput,
	NULL,
	pipeopen,
	pipeclose,
	NULL,
	&pipe_info,
	NULL };
static struct qinit pipewinit = { 
	pipeput,
	NULL,
	NULL,
	NULL,
	NULL,
	&pipe_info,
	NULL};
struct streamtab pipeinfo = { &piperinit, &pipewinit };

/*ARGSUSED*/
int
pipeopen(rqp, dev, flag, sflag)
queue_t *rqp;
dev_t dev;
int flag;
int sflag;
{
	return (0);
}

/*ARGSUSED*/
int
pipeclose(q)
queue_t *q;
{
	return (0);
}

/*
 * Use same put procedure for write and read queues.
 * If mp is an M_FLUSH message, switch the FLUSHW to FLUSHR and
 * the FLUSHR to FLUSHW and send the message on.  If mp is not an
 * M_FLUSH message, send it on with out processing.
 */
int
pipeput(qp, mp)
queue_t *qp;
mblk_t *mp;
{
	switch(mp->b_datap->db_type) {
		case M_FLUSH:
			if (!(*mp->b_rptr & FLUSHR && *mp->b_rptr & FLUSHW)) {
				if (*mp->b_rptr & FLUSHW) {
					*mp->b_rptr |= FLUSHR;
					*mp->b_rptr &= ~FLUSHW;
				} else {
					*mp->b_rptr |= FLUSHW;
					*mp->b_rptr &= ~FLUSHR;
				}
			}
			break;

		default:
			break;
	}
	putnext(qp,mp);
	return (0);
}
