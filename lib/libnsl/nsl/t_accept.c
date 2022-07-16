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


#ident	"@(#)libnsl:nsl/t_accept.c	1.5.4.1"
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
extern int _t_is_event();
extern void (*sigset())();
extern int ioctl();


t_accept(fd, resfd, call)
int fd;
int resfd;
struct t_call *call;
{
	char *buf;
	register struct T_conn_res *cres;
	struct strfdinsert strfdinsert;
	int size;
	int retval;
	register struct _ti_user *tiptr;
	register struct _ti_user *restiptr;
	void (*sigsave)();

	if ((tiptr = _t_checkfd(fd)) == NULL)
		return(-1);

	if ((restiptr = _t_checkfd(resfd)) == NULL)
		return(-1);

	if (tiptr->ti_servtype == T_CLTS) {
		t_errno = TNOTSUPPORT;
		return(-1);
	}

	if (fd != resfd)
	{
		if ((retval = ioctl(resfd,I_NREAD,&size)) < 0)
		{
			t_errno = TSYSERR;
			return(-1);
		}
		if (retval)
		{
			t_errno = TBADF;
			return(-1);
		}
	}

	sigsave = sigset(SIGPOLL, SIG_HOLD);
	if (_t_is_event(fd, tiptr)) {
		sigset(SIGPOLL, sigsave);
		return(-1);
	}

	buf = tiptr->ti_ctlbuf;
	cres = (struct T_conn_res *)buf;
	cres->PRIM_type = T_CONN_RES;
	cres->OPT_length = call->opt.len;
	cres->OPT_offset = 0;
	cres->SEQ_number = call->sequence;
	size = sizeof(struct T_conn_res);

	if (call->opt.len) {
		_t_aligned_copy(buf, call->opt.len, size,
			     call->opt.buf, &cres->OPT_offset);
		size = cres->OPT_offset + cres->OPT_length;
	}


	strfdinsert.ctlbuf.maxlen = tiptr->ti_ctlsize;
	strfdinsert.ctlbuf.len = size;
	strfdinsert.ctlbuf.buf = buf;
	strfdinsert.databuf.maxlen = call->udata.maxlen;
	strfdinsert.databuf.len = (call->udata.len? call->udata.len: -1);
	strfdinsert.databuf.buf = call->udata.buf;
	strfdinsert.fildes = resfd;
	strfdinsert.offset = sizeof(long);
	strfdinsert.flags = 0;      /* could be EXPEDITED also */

	if (ioctl(fd, I_FDINSERT, &strfdinsert) < 0) {
		if (errno == EAGAIN)
			t_errno = TFLOW;
		else
			t_errno = TSYSERR;
		sigset(SIGPOLL, sigsave);
		return(-1);
	}

	if (!_t_is_ok(fd, tiptr, T_CONN_RES)) {
		sigset(SIGPOLL, sigsave);
		return(-1);
	}

	sigset(SIGPOLL, sigsave);

	if (tiptr->ti_ocnt == 1) {
		if (fd == resfd)
			tiptr->ti_state = TLI_NEXTSTATE(T_ACCEPT1, tiptr->ti_state);
		else {
			tiptr->ti_state = TLI_NEXTSTATE(T_ACCEPT2, tiptr->ti_state);
			restiptr->ti_state = TLI_NEXTSTATE(T_PASSCON, restiptr->ti_state);
		}
	}
	else {
		tiptr->ti_state = TLI_NEXTSTATE(T_ACCEPT3, tiptr->ti_state);
		restiptr->ti_state = TLI_NEXTSTATE(T_PASSCON, restiptr->ti_state);
	}

	tiptr->ti_ocnt--;
	return(0);
}
