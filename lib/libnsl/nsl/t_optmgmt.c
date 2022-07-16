/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libnsl:nsl/t_optmgmt.c	1.3.6.1"
#include "sys/param.h"
#include "sys/types.h"
#include "sys/errno.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/tihdr.h"
#include "sys/timod.h"
#include "sys/tiuser.h"
#include "sys/signal.h"
#include "_import.h"


extern int t_errno;
extern int errno;
extern struct _ti_user *_t_checkfd();
extern void (*sigset())();
extern char *memcpy();


t_optmgmt(fd, req, ret)
int fd;
struct t_optmgmt *req;
struct t_optmgmt *ret;
{
	int size;
	register char *buf;
	register struct T_optmgmt_req *optreq;
	register struct _ti_user *tiptr;
	void (*sigsave)();


	if ((tiptr = _t_checkfd(fd)) == NULL)
		return(-1);

	sigsave = sigset(SIGPOLL, SIG_HOLD);
	buf = tiptr->ti_ctlbuf;
	optreq = (struct T_optmgmt_req *)buf;
	optreq->PRIM_type = T_OPTMGMT_REQ;
	optreq->OPT_length = req->opt.len;
	optreq->OPT_offset = 0;
	optreq->MGMT_flags = req->flags;
	size = sizeof(struct T_optmgmt_req);

	if (req->opt.len) {
		_t_aligned_copy(buf, req->opt.len, size,
			     req->opt.buf, &optreq->OPT_offset);
		size = optreq->OPT_offset + optreq->OPT_length;
	}

	if (!_t_do_ioctl(fd, buf, size, TI_OPTMGMT, NULL)) {
		sigset(SIGPOLL, sigsave);
		return(-1);
	}
	sigset(SIGPOLL, sigsave);


	if (optreq->OPT_length > ret->opt.maxlen) {
		t_errno = TBUFOVFLW;
		return(-1);
	}

	memcpy(ret->opt.buf, (char *) (buf + optreq->OPT_offset),
	       (int)optreq->OPT_length);
	ret->opt.len = optreq->OPT_length;
	ret->flags = optreq->MGMT_flags;

	tiptr->ti_state = TLI_NEXTSTATE(T_OPTMGMT, tiptr->ti_state);
	return(0);
}

