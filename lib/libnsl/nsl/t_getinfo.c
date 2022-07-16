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


#ident	"@(#)libnsl:nsl/t_getinfo.c	1.5.2.1"
#include "sys/errno.h"
#include "sys/types.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/tihdr.h"
#include "sys/timod.h"
#include "sys/tiuser.h"
#include "sys/signal.h"
#include "_import.h"


extern int t_errno;
extern int errno;
extern void (*sigset())();

t_getinfo(fd, info)
int fd;
register struct t_info *info;
{
	struct T_info_ack inforeq;
	int retlen;
	void (*sigsave)();

	if (_t_checkfd(fd) == 0)
		return(-1);

	sigsave = sigset(SIGPOLL, SIG_HOLD);
	inforeq.PRIM_type = T_INFO_REQ;

	if (!_t_do_ioctl(fd, (caddr_t)&inforeq, sizeof(struct T_info_req), TI_GETINFO, &retlen)) {
		sigset(SIGPOLL, sigsave);
		return(-1);
	}
		
	sigset(SIGPOLL, sigsave);
	if (retlen != sizeof(struct T_info_ack)) {
		errno = EIO;
		t_errno = TSYSERR;
		return(-1);
	}

	info->addr = inforeq.ADDR_size;
	info->options = inforeq.OPT_size;
	info->tsdu = inforeq.TSDU_size;
	info->etsdu = inforeq.ETSDU_size;
	info->connect = inforeq.CDATA_size;
	info->discon = inforeq.DDATA_size;
	info->servtype = inforeq.SERV_type;

	return(0);
}
