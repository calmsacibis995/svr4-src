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

#ident	"@(#)libnsl:nsl/_utility.c	1.11.3.1"
#include "sys/param.h"
#include "sys/types.h"
#include "sys/errno.h"
#include "sys/stropts.h"
#include "sys/stream.h"
#include "sys/tihdr.h"
#include "sys/timod.h"
#include "sys/tiuser.h"
#include "sys/fcntl.h"
#include "sys/signal.h"
#include "_import.h"

#define DEFSIZE 2048

extern struct _ti_user *_ti_user;
extern int t_errno;
extern int errno;
extern long openfiles;
extern char *calloc();
extern char *memcpy();
extern void free();
extern ioctl(), getmsg();
extern long ulimit();
extern int fcntl();
extern void (*sigset())();

/*
 * Checkfd - checks validity of file descriptor
 */

struct _ti_user *
_t_checkfd(fd)
int fd;
{
	extern int _t_restore_state();

	if (fd < 0) {
		t_errno = TBADF;
		return(NULL);
	}
	if ((fd > (openfiles -1)) || (_ti_user == NULL) ||
	    !(_ti_user[fd].ti_flags & USED)) {
		/*
		 * try to get state back.
		 * _t_restore_state sets t_errno.
		 */
		if (_t_restore_state(fd) < 0) {
			return(NULL);
		}
	}
	return(&_ti_user[fd]);
}

/* 
 * copy data to output buffer and align it as in input buffer
 * This is to ensure that if the user wants to align a network
 * addr on a non-word boundry then it will happen.
 */
void
_t_aligned_copy(buf, len, init_offset, datap, rtn_offset)
char *buf;
char *datap;
long *rtn_offset;
{
		*rtn_offset = ROUNDUP(init_offset) + ((unsigned int)datap&0x03);
		memcpy((char *)(buf + *rtn_offset), datap, (int)len);
}


/*
 * Max - return max between two ints
 */
_t_max(x, y)
int x;
int y;
{
	if (x > y)
		return(x);
	else 
		return(y);
}

/* 
 * put data and control info in look buffer
 * 
 * The only thing that can be in look buffer is a T_discon_ind,
 * T_ordrel_ind or a T_uderr_ind.
 */
void
_t_putback(tiptr, dptr, dsize, cptr, csize)
struct _ti_user *tiptr;
caddr_t dptr;
int dsize;
caddr_t cptr;
int csize;
{
	memcpy(tiptr->ti_lookdbuf, dptr, dsize);
	memcpy(tiptr->ti_lookcbuf, cptr, csize);
	tiptr->ti_lookdsize = dsize;
	tiptr->ti_lookcsize = csize;
	tiptr->ti_lookflg++;

}

/*
 * Is there something that needs attention?
 */

_t_is_event(fd, tiptr)
int fd;
struct _ti_user *tiptr;
 {
	int size, retval;

	if ((retval = ioctl(fd, I_NREAD, &size)) < 0) {
		t_errno = TSYSERR;
		return(1);
	}

	if (retval || tiptr->ti_lookflg) {
		t_errno = TLOOK;
		return(1);
	}

	return(0);
}

/* 
 * wait for T_OK_ACK
 */
_t_is_ok(fd, tiptr, type)
int fd;
register struct _ti_user *tiptr;
long type;
{

	struct strbuf ctlbuf;
	struct strbuf rcvbuf;
	register union T_primitives *pptr;
	int flags = 0;
	int retval, cntlflag;
	int size;

	cntlflag = fcntl(fd,F_GETFL,0);
	fcntl(fd,F_SETFL,fcntl(fd,F_GETFL,0) & ~(O_NDELAY | O_NONBLOCK));

	ctlbuf.len = 0;
	ctlbuf.buf = tiptr->ti_ctlbuf;
	ctlbuf.maxlen = tiptr->ti_ctlsize;
	rcvbuf.maxlen = tiptr->ti_rcvsize;
	rcvbuf.len = 0;
	rcvbuf.buf = tiptr->ti_rcvbuf;
	flags = RS_HIPRI;

	while ((retval = getmsg(fd, &ctlbuf, &rcvbuf, &flags)) < 0) {
		if (errno == EINTR)
			continue;
		t_errno = TSYSERR;
		return(0);
	}

	/* did I get entire message */
	if (retval) {
		t_errno = TSYSERR;
		errno = EIO;
		return(0);
	}

	/* 
	 * is ctl part large enough to determine type?
	 */
	if (ctlbuf.len < sizeof(long)) {
		t_errno = TSYSERR;
		errno = EPROTO;
		return(0);
	}

	fcntl(fd,F_SETFL,cntlflag);

	pptr = (union T_primitives *)ctlbuf.buf;

	switch(pptr->type) {
		case T_OK_ACK:
			if ((ctlbuf.len < sizeof(struct T_ok_ack)) ||
			    (pptr->ok_ack.CORRECT_prim != type)) {
				t_errno = TSYSERR;
				errno = EPROTO;
				return(0);
			}
			return(1);

		case T_ERROR_ACK:
			if ((ctlbuf.len < sizeof(struct T_error_ack)) ||
			    (pptr->error_ack.ERROR_prim != type)) {
				t_errno = TSYSERR;
				errno = EPROTO;
				return(0);
			}
			/*
			 * if error is out of state and there is something
			 * on read queue, then indicate to user that
			 * there is something that needs attention
			 */
			if (pptr->error_ack.TLI_error == TOUTSTATE) {
				if ((retval = ioctl(fd, I_NREAD, &size)) < 0) {
					t_errno = TSYSERR;
					return(0);
				}
				if (retval)
					t_errno = TLOOK;
				else
					t_errno = TOUTSTATE;
			} else {
				t_errno = pptr->error_ack.TLI_error;
				if (t_errno == TSYSERR)
					errno = pptr->error_ack.UNIX_error;
			}
			return(0);

		default:
			t_errno = TSYSERR;
			errno = EPROTO;
			return(0);
	}
}

/*
 * timod ioctl
 */
_t_do_ioctl(fd, buf, size, cmd, retlen)
char *buf;
int *retlen;
{
	int retval;
	struct strioctl strioc;

	strioc.ic_cmd = cmd;
	strioc.ic_timout = -1;
	strioc.ic_len = size;
	strioc.ic_dp = buf;

	if ((retval = ioctl(fd, I_STR, &strioc)) < 0) {
		t_errno = TSYSERR;
		return(0);
	}

	if (retval) {
		t_errno = retval&0xff;
		if (t_errno == TSYSERR)
			errno = (retval >>  8)&0xff;
		return(0);
	}
	if (retlen)
		*retlen = strioc.ic_len;
	return(1);
}

/*
 * alloc scratch buffers and look buffers
 */

/* ARGSUSED */
_t_alloc_bufs(fd, tiptr, info)
register struct _ti_user *tiptr;
struct T_info_ack info;
{
	unsigned size1, size2;
	unsigned csize, dsize, asize, osize;
	char *ctlbuf, *rcvbuf;
	char *lookdbuf, *lookcbuf;

	csize = _t_setsize(info.CDATA_size);
	dsize = _t_setsize(info.DDATA_size);

	size1 = _t_max(csize,dsize);

	if (size1 > 0) {
		if ((rcvbuf = calloc(1, size1)) == NULL)
			return(-1);
		if ((lookdbuf = calloc(1, size1)) == NULL) {
			(void)free(rcvbuf);
			return(-1);
		}
	} else {
		rcvbuf = NULL;
		lookdbuf = NULL;
	}

	asize = _t_setsize(info.ADDR_size);
	osize = _t_setsize(info.OPT_size);

	size2 = sizeof(union T_primitives) + asize + sizeof(long) + osize + sizeof(long);

	if ((ctlbuf = calloc(1, size2)) == NULL) {
		if (size1 > 0) {
			(void)free(rcvbuf);
			(void)free(lookdbuf);
		}
		return(-1);
	}

	if ((lookcbuf = calloc(1, size2)) == NULL) {
		if (size1 > 0) {
			(void)free(rcvbuf);
			(void)free(lookdbuf);
		}
		(void)free(ctlbuf);
		return(-1);
	}


	tiptr->ti_rcvsize = size1;
	tiptr->ti_rcvbuf = rcvbuf;
	tiptr->ti_ctlsize = size2;
	tiptr->ti_ctlbuf = ctlbuf;
	tiptr->ti_lookcbuf = lookcbuf;
	tiptr->ti_lookdbuf = lookdbuf;
	tiptr->ti_lookcsize = 0;
	tiptr->ti_lookdsize = 0;
	tiptr->ti_lookflg = 0;
	tiptr->ti_flags = USED;
	if (info.PROVIDER_flag & TP_SNDZERO)
		tiptr->ti_flags |= SENDZERO;
	if (info.PROVIDER_flag & TP_EXPINLINE)
		tiptr->ti_flags |= EXPINLINE;
	tiptr->ti_maxpsz = info.TIDU_size;
	tiptr->ti_servtype = info.SERV_type;
	tiptr->ti_state = T_UNINIT;
	tiptr->ti_ocnt = 0;
	return(0);
}

/*
 * set sizes of buffers
 */

_t_setsize(infosize)
long infosize;
{
	switch(infosize)
	{
		case -1: return(DEFSIZE);
		case -2: return(0);
		default: return(infosize);
	}
}

void
_null_tiptr(tiptr)
struct _ti_user *tiptr;
{
	tiptr->ti_flags = 0;
	tiptr->ti_rcvsize = 0;
	tiptr->ti_rcvbuf = NULL;
	tiptr->ti_ctlsize = 0;
	tiptr->ti_ctlbuf = NULL;
	tiptr->ti_lookdbuf = NULL;
	tiptr->ti_lookcbuf = NULL;
	tiptr->ti_lookdsize = 0;
	tiptr->ti_lookcsize = 0;
	tiptr->ti_maxpsz = 0;
	tiptr->ti_servtype = 0;
	tiptr->ti_lookflg = 0;
	tiptr->ti_state = 0;
	tiptr->ti_ocnt = 0;
}

/*
 * _t_restore_state - try to restore the state of the library
 * after an exec to avoid users needing to do a t_sync.  Code
 * was stolen from t_sync.c.
 */
int
_t_restore_state(fd)
int fd;
{
	int retval;
	struct T_info_ack info;
	register struct _ti_user *tiptr;
	int retlen;
	void (*sigsave)();
	int arg,rval;
	int didalloc = 0;

	openfiles = OPENFILES;
	if (fd > (openfiles - 1)) {
		t_errno = TBADF;
		return(-1);
	}
	if (!_ti_user) {
		if ((_ti_user = (struct _ti_user *)calloc(1, (unsigned)(openfiles*sizeof(struct _ti_user)))) == NULL) {
			t_errno = TSYSERR;
			return(-1);
		}
		didalloc = 1;
	}
	sigsave = sigset(SIGPOLL, SIG_HOLD);
	info.PRIM_type = T_INFO_REQ;
	if ((retval = ioctl(fd, I_FIND, "timod")) < 0) {
		sigset(SIGPOLL, sigsave);
		if (didalloc) {
			(void)free(_ti_user);
			_ti_user = NULL;
		}
		t_errno = TBADF;
		return(-1);
	}
	if (!retval) {
		sigset(SIGPOLL, sigsave);
		if (didalloc) {
			(void)free(_ti_user);
			_ti_user = NULL;
		}
		t_errno = TBADF;
		return(-1);
	}
	if (!_t_do_ioctl(fd, (caddr_t)&info, sizeof(struct T_info_req), TI_GETINFO, &retlen) < 0) {
		sigset(SIGPOLL, sigsave);
		if (didalloc) {
			(void)free(_ti_user);
			_ti_user = NULL;
		}
		return(-1);
	}
	sigset(SIGPOLL, sigsave);
	if (retlen != sizeof(struct T_info_ack)) {
		if (didalloc) {
			(void)free(_ti_user);
			_ti_user = NULL;
		}
		errno = EIO;
		t_errno = TSYSERR;
		return(-1);
	}
	tiptr = &_ti_user[fd];
	if (_t_alloc_bufs(fd, tiptr, info) < 0) {
		_null_tiptr(tiptr);
		if (didalloc) {
			(void)free(_ti_user);
			_ti_user = NULL;
		}
		t_errno = TSYSERR;
		return(-1);
	}
	if (info.CURRENT_state == TS_DATA_XFER) {
		tiptr->ti_state = T_DATAXFER;
	} else  {
		if (info.CURRENT_state == TS_IDLE) {
			if((rval = ioctl(fd,I_NREAD,&arg)) < 0)  {
				if (didalloc) {
					(void)free(_ti_user);
					_ti_user = NULL;
				}
				t_errno = TSYSERR;
				return(-1);
			}
			if(rval == 0 )
				tiptr->ti_state = T_IDLE;
			else
				tiptr->ti_state = T_DATAXFER;
		} else {
			tiptr->ti_state = T_FAKE;
		}
	}
	return(0);
}

