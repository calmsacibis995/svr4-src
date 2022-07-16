/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:lib/libmb2/mb2s_prim.c	1.3"

#include <sys/types.h>
#include <sys/param.h>
#include <fcntl.h>
#include <sys/errno.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/mps.h>
#include <sys/mb2taiusr.h>
#include <sys/inline.h>
#include "_def.h"



int
mb2s_closeport (fd)
int fd;
{
	int ret;

	if (_mb2_check_fd (fd, MB2_SYNC_MODE) < 0)
		return (-1);

	if (_mb2_do_unbind (fd, MB2_SYNC_MODE) < 0)
		return (-1);
	ret = close (fd);
	return (ret);
}



int
mb2s_getinfo (fd, mb2infoptr)
int fd;
mb2_info *mb2infoptr;
{

	if (mb2infoptr == NULL) {
		errno = EFAULT;
		return (-1);
	}

	if (_mb2_check_fd (fd, MB2_SYNC_MODE) < 0)
		return (-1);

	return (_mb2_do_info (fd, mb2infoptr, MB2_SYNC_MODE));
} 

int
mb2s_getreqfrag (fd, msginfoptr, databufptr)
int fd;
mb2_msginfo *msginfoptr;
mb2_buf *databufptr;
{
	struct strbuf ctl;
	struct mb2_msg_ind msg_ind;
	int ret, flags;
	long type;

	if (_mb2_check_fd (fd, MB2_SYNC_MODE) < 0) 
		return (-1);

	if (msginfoptr == NULL) {
		errno = EINVAL;
		return (-1);
	}

	if (msginfoptr->tid == 0) {
		errno = EINVAL;
		return (-1);
	}

	if (databufptr->maxlen < databufptr->len) {
		errno = ERANGE;
		return (-1);
	}

	

	if (_mb2_do_fragreq (fd, &msginfoptr->socketid, msginfoptr->tid,
			     (ulong) databufptr->len, MPS_MG_DFBIND) < 0)
		return (-1);

	ctl.maxlen = sizeof (struct mb2_msg_ind);
	ctl.len = 0;
	ctl.buf = (char *) &msg_ind;
	flags = 0;

	errno = 0;
	if ((ret = getmsg (fd, &ctl, databufptr, &flags)) < 0) {
		/*
		 * we need to check if we were interrupted if so
		 * sync up the module
		 */
		if (errno == EINTR) {
			(void) _mb2_do_sync (fd);
			errno = EINTR;
		}
		return (-1);
	}
	if (ret) {	/* error because there is more on streamhead */ 
		/* 
		 * this cannot happen unless the module screwed up
		 * or somebody screwed up the streamhead
		 */
		errno = EIO;
		return (-1);
	}

	type = msg_ind.PRIM_type;
	if ((type != MB2_FRAGRES_MSG) && (type != MB2_STATUS_MSG)) {
		errno = EIO;
		return (-1);
	}

	return (0);	
}


int
mb2s_openport (portid, optptr)
ushort portid;
mb2_opts *optptr;
{
	int fd;
	int ret;
	int save_errno;
	
	if ((fd = open (s_dev_name, O_RDWR)) < 0) 
		return (-1);

	/* 
	 * find if the module has been pushed, if not push it
	 */

	if ((ret = ioctl(fd, I_FIND, mod_name)) < 0) {
		save_errno = errno;
		close (fd);
		errno = save_errno;
		return (ret);
	}

	if (ret == 0) { 
		if (ioctl (fd, I_PUSH, mod_name) < 0) {
			save_errno = errno;
			close (fd);
			errno = save_errno;
			return (-1);
		}
	}


	if ((ret = _mb2_do_bind (fd, portid, MB2_SYNC_MODE)) < 0) {
		save_errno = errno;
		close (fd);
		errno = save_errno;
		return (ret);
	}

	if (optptr != NULL) {
		if ((ret = _mb2_do_optmgmt (fd, optptr, MB2_SYNC_MODE)) < 0) {
			save_errno = errno;
			close (fd);
			errno = save_errno;
			return (ret);
		}
	}
	return (fd);
}



int
mb2s_receive (fd, msginfoptr, ctlbufptr, databufptr)
int fd;
mb2_msginfo *msginfoptr;
mb2_buf *ctlbufptr;
mb2_buf *databufptr;
{
	struct strioctl strioc;
	struct mb2_recv_req *recv_req;


	if (_mb2_check_fd (fd, MB2_SYNC_MODE) < 0) 
		return (-1);

	if (msginfoptr == NULL) {
		/*
		 * this is the second receive
		 */
		if ((ctlbufptr != NULL) || (databufptr == NULL)) {
			errno = EINVAL;
			return (-1);
		}
		if (_mb2_do_second_recv (fd, databufptr) < 0)
			return (-1);
	} else {
		/*
	 	 * we assume that this is the first time request
		 * the getmsg in _mb2_do_receive will fail if not so
	 	 */
		recv_req = (struct mb2_recv_req *)mb2_user;
		recv_req->PRIM_type = MB2_RECV_REQ;
		if (ctlbufptr != NULL) 
			recv_req->CTRL_length = RNDUP (ctlbufptr->maxlen);
		else
			recv_req->CTRL_length = 0;
		
		strioc.ic_cmd = MB2_RECV_CMD;
		strioc.ic_timout = -1;
		strioc.ic_len = sizeof (struct mb2_recv_req);
		strioc.ic_dp = (char *)recv_req;
		
		errno = 0;	
		if (ioctl (fd, I_STR, &strioc) < 0) {
			/*
			 * we need to check if it was interrupted and if
			 * so send a SYNC cmd
			 */
			if (errno == EINTR) {
				(void) _mb2_do_sync (fd);
				errno = EINTR;
			}
			return (-1);
		}
		if (_mb2_do_receive (fd, msginfoptr, ctlbufptr, databufptr, MB2_SYNC_MODE) < 0) {
			return (-1);
		}
	
	} 
	return (0);
}
	

int
mb2s_send (fd, socketidptr, ctlbufptr, databufptr)
int fd;
mb2_socket *socketidptr;
mb2_buf *ctlbufptr;
mb2_buf *databufptr;
{

	if (_mb2_check_fd (fd, MB2_SYNC_MODE) < 0) 
		return (-1);

	if (_mb2_check_range (ctlbufptr, databufptr) < 0)
		return (-1);

	if (_mb2_do_send (fd, socketidptr, ctlbufptr, databufptr) < 0)
		return (-1);
	return (0);
}


int
mb2s_brdcst (fd, portid, ctlbufptr)
int fd;
unsigned short portid;
mb2_buf *ctlbufptr;
{

	if (_mb2_check_fd (fd, MB2_SYNC_MODE) < 0) 
		return (-1);

	if (_mb2_check_range (ctlbufptr, (mb2_buf *)NULL) < 0)
		return (-1);

	if (_mb2_do_brdcst (fd, portid, ctlbufptr) < 0)
		return (-1);
	return (0);
}


int
mb2s_sendcancel (fd, socketidptr, tid)
int fd;
mb2_socket *socketidptr;
unchar tid;
{

	if (_mb2_check_fd (fd, MB2_SYNC_MODE) < 0) 
		return (-1);

	if (tid == 0) {
		errno = EINVAL;
		return (-1);
	}

	if (_mb2_do_sendcancel (fd, socketidptr, tid) < 0)
		return (-1);
	return (0);
}


int
mb2s_sendreply (fd, socketidptr, ctlbufptr, databufptr, tid, lastflag)
int fd;
mb2_socket *socketidptr;
mb2_buf *ctlbufptr;
mb2_buf *databufptr;
unchar tid;
int lastflag;
{

	if (_mb2_check_fd (fd, MB2_SYNC_MODE) < 0) 
		return (-1);

	if (socketidptr == NULL) {
		errno = EINVAL;
		return (-1);
	}

	if (_mb2_check_range (ctlbufptr, databufptr) < 0)
		return (-1);

	if (tid == 0) {
		errno = EINVAL;
		return (-1);
	}

	if (((lastflag != MB2_EOT) && (lastflag != MB2_NOTEOT)) ||
		((databufptr == NULL) && (lastflag != MB2_EOT))) {
		errno = EINVAL;
		return (-1);
	}
	if (_mb2_do_sendreply (fd, socketidptr, ctlbufptr, databufptr, tid, lastflag) < 0)
		return (-1);
	return (0);
}



int
mb2s_sendrsvp (fd, msginfoptr, reqcbufptr, reqdbufptr, rescbufptr, resdbufptr)
int fd;
mb2_msginfo *msginfoptr;
mb2_buf *reqcbufptr;
mb2_buf *reqdbufptr;
mb2_buf *rescbufptr;
mb2_buf *resdbufptr;
{
	struct mb2_msg_ind *msg_ind;
	ulong resplen, type;
	struct strbuf ctl;
	int ret, flags;

	if (_mb2_check_fd (fd, MB2_SYNC_MODE) < 0) 
		return (-1);

	if (msginfoptr == NULL) {
		errno = EINVAL;
		return (-1);
	}

	if (_mb2_check_range (reqcbufptr, reqdbufptr) < 0)
		return (-1);

	
	resplen = 0;
	if (resdbufptr != NULL)
		resplen = resdbufptr->maxlen;


	if (_mb2_do_sendrsvp (fd, &(msginfoptr->socketid), reqcbufptr,
				reqdbufptr, resplen, MPS_MG_DFBIND) < 0)
		return (-1);

	msg_ind = (struct mb2_msg_ind *) mb2_user;
	ctl.maxlen = sizeof (struct mb2_msg_ind) + UNSOL_LENGTH;
	ctl.len = 0;
	ctl.buf = (char *) msg_ind;
	flags = 0;

	errno = 0;
	if ((ret = getmsg (fd, &ctl, resdbufptr, &flags)) < 0) {
		/*
		 * we need to check if this call was interrupted. If so
		 * we sync up the module
		 */
		if (errno == EINTR) {
			(void) _mb2_do_sync (fd);
			errno = EINTR;
		}
		return (-1);
	}

	if (ret) {	/* error because there is more on streamhead */ 
		/* 
		 * this cannot happen unless the module or driver screwed up
		 * or somebody screwed up the streamhead
		 */
		errno = EIO;
		return (-1);
	}

	type = msg_ind->PRIM_type;
	if ((type != MB2_RESP_MSG) && (type != MB2_CANCEL_MSG) &&
		(type != MB2_STATUS_MSG)) {
		errno = EIO;
		return (-1);
	}
	if (type == MB2_STATUS_MSG) {
		errno = ETIME;
		return (-1);
	}
	msginfoptr->msgtyp = type;
	if (rescbufptr != NULL) {
		(void) copy_bytes ((caddr_t)((char *)msg_ind + msg_ind->CTRL_offset),
				rescbufptr->buf, 
				(int)min (rescbufptr->maxlen, msg_ind->CTRL_length));
		rescbufptr->len = min (rescbufptr->maxlen, msg_ind->CTRL_length);
	}
	return (0);	
}
