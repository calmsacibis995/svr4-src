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


#ident	"@(#)libnsl:nsl/t_sync.c	1.4.6.1"
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


extern struct _ti_user *_ti_user;
extern int t_errno;
extern int errno;
extern long openfiles;
extern struct _ti_user *_t_checkfd();
extern void free();
extern char *calloc();
extern void (*sigset())();
extern int ioctl();
extern long ulimit();
extern _null_tiptr();


t_sync(fd)
int fd;
{
	int retval;
	struct T_info_ack info;
	register struct _ti_user *tiptr;
	int retlen;
	void (*sigsave)();
	int arg,rval;

	/*
	 * Initialize "openfiles" - global variable
  	 * containing number of open files allowed
	 * per process.
	 */
	openfiles = OPENFILES;

	/*
         * if needed allocate the ti_user structures
	 * for all file desc.
	 */
	 if (!_ti_user) 
		if ((_ti_user = (struct _ti_user *)calloc(1, (unsigned)(openfiles*sizeof(struct _ti_user)))) == NULL) {
			t_errno = TSYSERR;
			return(-1);
		}

	sigsave = sigset(SIGPOLL, SIG_HOLD);
	info.PRIM_type = T_INFO_REQ;


	if ((retval = ioctl(fd, I_FIND, "timod")) < 0) {
		sigset(SIGPOLL, sigsave);
		t_errno = TBADF;
		return(-1);
	}

	if (!retval) {
		sigset(SIGPOLL, sigsave);
		t_errno = TBADF;
		return(-1);
	}
	if (!_t_do_ioctl(fd, (caddr_t)&info, sizeof(struct T_info_req), TI_GETINFO, &retlen) < 0) {
		sigset(SIGPOLL, sigsave);
		return(-1);
	}
	sigset(SIGPOLL, sigsave);
			
	if (retlen != sizeof(struct T_info_ack)) {
		errno = EIO;
		t_errno = TSYSERR;
		return(-1);
	}

	/* 
	 * Range of file desc. is OK, the ioctl above was successful!
	 * Check for fork/exec case.
	 */

	if ((tiptr = _t_checkfd(fd)) == NULL) {

		tiptr = &_ti_user[fd];

		if (_t_alloc_bufs(fd, tiptr, info) < 0) {
			_null_tiptr(tiptr);
			t_errno = TSYSERR;
			return(-1);
		}
		/****** Hack to fix exec user level state problem *********/
		/****** DATAXFER and DISCONNECT cases are covered *********/
		/****** Solves the problems for execed servers*************/

			if (info.CURRENT_state == TS_DATA_XFER)
				tiptr->ti_state = T_DATAXFER;
			else  {
				if (info.CURRENT_state == TS_IDLE) {
					if((rval = ioctl(fd,I_NREAD,&arg)) < 0)  {
						t_errno = TSYSERR;
						return(-1);
					}
					if(rval == 0 )
						tiptr->ti_state = T_IDLE;
					else
						tiptr->ti_state = T_DATAXFER;
				}
				else
					tiptr->ti_state = T_FAKE;
	       	 }			
	}

	switch (info.CURRENT_state) {

	case TS_UNBND:
		return(T_UNBND);
	case TS_IDLE:
		return(T_IDLE);
	case TS_WRES_CIND:
		return(T_INCON);
	case TS_WCON_CREQ:
		return(T_OUTCON);
	case TS_DATA_XFER:
		return(T_DATAXFER);
	case TS_WIND_ORDREL:
		return(T_OUTREL);
	case TS_WREQ_ORDREL:
		return(T_INREL);
	default:
		t_errno = TSTATECHNG;
		return(-1);
	}
}
