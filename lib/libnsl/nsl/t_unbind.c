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


#ident	"@(#)libnsl:nsl/t_unbind.c	1.3.6.1"
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
extern int ioctl();


t_unbind(fd)
int fd;
{
	register struct _ti_user *tiptr;
	struct T_unbind_req *unbind_req;
	void (*sigsave)();

	if ((tiptr = _t_checkfd(fd)) == NULL)
		return(-1);


	sigsave = sigset(SIGPOLL, SIG_HOLD);
	if (_t_is_event(fd, tiptr)) {
		sigset(SIGPOLL, sigsave);
		return(-1);
	}

	unbind_req = (struct T_unbind_req *)tiptr->ti_ctlbuf;
	unbind_req->PRIM_type = T_UNBIND_REQ;

	if (!_t_do_ioctl(fd, (caddr_t)unbind_req, sizeof(struct T_unbind_req), TI_UNBIND, NULL)) {
		sigset(SIGPOLL, sigsave);
		return(-1);
	}
	sigset(SIGPOLL, sigsave);

	if (ioctl(fd, I_FLUSH, FLUSHRW) < 0) {
		t_errno = TSYSERR;
		return(-1);
	}

	/*
	 * clear more data in TSDU bit
	 */
	tiptr->ti_flags &= ~MORE;

	tiptr->ti_state = TLI_NEXTSTATE(T_UNBIND, tiptr->ti_state);
	return(0);
}
