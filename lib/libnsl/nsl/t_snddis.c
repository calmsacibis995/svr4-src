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


#ident	"@(#)libnsl:nsl/t_snddis.c	1.4.5.1"
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
extern int ioctl(), putmsg();


t_snddis(fd, call)
int fd;
struct t_call *call;
{
	struct T_discon_req dreq;
	struct strbuf ctlbuf;
	struct strbuf databuf;
	register struct _ti_user *tiptr;
	void (*sigsave)();


	if ((tiptr = _t_checkfd(fd)) == NULL)
		return(-1);

	if (tiptr->ti_servtype == T_CLTS) {
		t_errno = TNOTSUPPORT;
		return(-1);
	}
	
	/*
         * look at look buffer to see if there is a discon there
	 */

	if (t_look(fd) == T_DISCONNECT) {
		t_errno = TLOOK;
		return(-1);
	}

	tiptr->ti_lookflg = 0;

	if (ioctl(fd, I_FLUSH, FLUSHW) < 0) {
		t_errno = TSYSERR;
		return(-1);
	}

	sigsave = sigset(SIGPOLL, SIG_HOLD);

	dreq.PRIM_type = T_DISCON_REQ;
	dreq.SEQ_number = (call? call->sequence: -1);


	ctlbuf.maxlen = sizeof(struct T_discon_req);
	ctlbuf.len = sizeof(struct T_discon_req);
	ctlbuf.buf = (caddr_t)&dreq;

	databuf.maxlen = (call? call->udata.len: 0);
	databuf.len = (call? call->udata.len: 0);
	databuf.buf = (call? call->udata.buf: NULL);

	if (putmsg(fd, &ctlbuf, (databuf.len? &databuf: NULL), 0) < 0) {
		sigset(SIGPOLL, sigsave);
		t_errno = TSYSERR;
		return(-1);
	}

	if (!_t_is_ok(fd, tiptr, T_DISCON_REQ)) {
		sigset(SIGPOLL, sigsave);
		return(-1);
	}

	tiptr->ti_flags &= ~MORE;
	sigset(SIGPOLL, sigsave);

	if (tiptr->ti_ocnt <= 1) {
		if (tiptr->ti_state == T_INCON)
			tiptr->ti_ocnt--;
		tiptr->ti_state = TLI_NEXTSTATE(T_SNDDIS1, tiptr->ti_state);
	}
	else {
		if (tiptr->ti_state == T_INCON)
			tiptr->ti_ocnt--;
		tiptr->ti_state = TLI_NEXTSTATE(T_SNDDIS2, tiptr->ti_state);
	}	
	return(0);
}
