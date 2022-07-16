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

#ident	"@(#)mbus:lib/libmb2/_mb2_utility.c	1.3"

#ifndef lint
static char _mb2_utilitycopyright[] = "Copyright 1988 Intel Corporation 462651";
#endif

#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/errno.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/mps.h>
#include <sys/inline.h> 
#include <sys/mb2taiusr.h>
#include "_def.h"


int
_mb2_check_fd (fd, flag)
int fd;
int flag;
{
	
#ifdef FOR_PERFORMANCE
	struct stat statbuf;
	int ret;

	if  (fstat (fd, &statbuf) < 0) {
		return (-1);
	}

	if (flag == MB2_SYNC_MODE) {
		if ((ret = ioctl(fd, I_FIND, mod_name)) < 0) 
			return (-1);
		if (ret != 1)  {
			errno = EBADF;
			return (-1);
		}
	}
#endif
	return (0);
}

_mb2_check_range (ctlbufptr, databufptr)
mb2_buf *ctlbufptr;
mb2_buf *databufptr;
{
	int clen, dlen;

	dlen = 0;
	if (databufptr != NULL) 
		dlen = databufptr->len;	
	clen = 0;
	if (ctlbufptr != NULL)
		clen = ctlbufptr->len;

	if ((clen > UNSOL_LENGTH) ||
		((dlen != 0) && (clen > SOL_LENGTH))) {
		errno = ERANGE;
		return (-1);
	}
	return (0);
}

void
_mb2_do_sync (fd)
int fd;
{
	struct strioctl strioc;

	/*
	 * it may be necessary to disable all signals here. But I dont
	 * think it necessary
	 */
	strioc.ic_cmd = MB2_SYNC_CMD;
	strioc.ic_timout = 0;
	strioc.ic_len = 0;
	strioc.ic_dp = NULL;
	if (ioctl (fd, I_STR, &strioc) < 0) {
		/*
		 * there is nothing that can be done
		 */
	}
	return;
}
	

ushort
mb2_gethostid ()
{
	int ret, fd, flags;
	struct mb2_hostid_req hostid_req;
	struct mb2_hostid_ack hostid_ack;
	struct strbuf ctl;

	/*
	 * this is a very kludgy way of getting the hostid
	 * but serves the present purpose
	 */
	
	if ((fd = open (a_dev_name, O_RDWR)) < 0) 
		return (0xffff);   /* equivalent -1 return for ushort's */

	hostid_req.PRIM_type = MB2_HOSTID_REQ;
	ctl.len = sizeof (struct mb2_hostid_req);
	ctl.buf = (char *) &hostid_req;
	flags = RS_HIPRI;

	if (putmsg (fd, &ctl, NULL, flags) < 0) {
		close (fd);
		return (0xffff);
	}

	ctl.maxlen = sizeof (struct mb2_hostid_ack);
	ctl.buf = (char *) &hostid_ack;

	if ((ret = getmsg (fd, &ctl, (mb2_buf *)NULL, &flags)) < 0) {
		close (fd);
		return (0);
	}

	if ((ret > 0) || (hostid_ack.PRIM_type != MB2_HOSTID_ACK)) { 
		/* 
		 *  we are expecting an ack and we got something else 
		 */
		errno = ENXIO;
		close (fd);
		return (0xffff);
	}
	close (fd);
	return (hostid_ack.HOST_id);
}


int
_mb2_do_bind (fd, portid, mode)
int fd;
ushort portid;
int mode;
{
	int ret, flags;
	struct mb2_bind_req bind_req;
	struct mb2_bind_ack bind_ack;
	struct strbuf ctlbuf;

	bind_req.PRIM_type = MB2_BIND_REQ;
	bind_req.SRC_port = portid;
	ctlbuf.len = sizeof (struct mb2_bind_req);
	ctlbuf.buf = (char *) &bind_req;
	flags = RS_HIPRI;
	
	if (putmsg (fd, &ctlbuf, NULL, flags) < 0) 
		return (-1);

	ctlbuf.maxlen = sizeof (struct mb2_bind_ack);
	ctlbuf.buf = (char *) &bind_ack;

	while (1) {
		if ((ret = getmsg (fd, &ctlbuf, (mb2_buf *)NULL, 
						&flags)) < 0) {
			if (mode == MB2_SYNC_MODE) {
				return (-1);
			} else {
				if (errno != EAGAIN)
					return (-1);
			}
		} else
			break;
	}

	if ((ret > 0) || (bind_ack.PRIM_type != MB2_BIND_ACK)) { 
		/* 
		 *  we are expecting an ack and we got something else 
		 */
		errno = ENXIO;
		return (-1);
	}

	if (bind_ack.RET_code) {
		errno = bind_ack.RET_code;
		return (-1);
	}
	return (0);
}


int
_mb2_do_unbind (fd, mode)
int fd;
int mode;
{
	int ret, flags;
	struct mb2_unbind_req unbind_req;
	struct mb2_unbind_ack unbind_ack;
	struct strbuf ctlbuf;

	unbind_req.PRIM_type = MB2_UNBIND_REQ;
	ctlbuf.len = sizeof (struct mb2_unbind_req);
	ctlbuf.buf = (char *) &unbind_req;
	flags = RS_HIPRI;
	
	if (putmsg (fd, &ctlbuf, NULL, flags) < 0) 
		return (-1);

	ctlbuf.maxlen = sizeof (struct mb2_unbind_ack);
	ctlbuf.buf = (char *) &unbind_ack;

	while (1) {
		if ((ret = getmsg (fd, &ctlbuf, (mb2_buf *)NULL, 
						&flags)) < 0) {
			if (mode == MB2_SYNC_MODE) {
				return (-1);
			} else {
				if (errno != EAGAIN)
					return (-1);
			}
		} else
			break;
	}

	if ((ret > 0) || (unbind_ack.PRIM_type != MB2_UNBIND_ACK)) { 
		/* 
		 *  we are expecting an ack and we got something else 
		 */
		errno = EIO;
		return (-1);
	}
	if (unbind_ack.RET_code) {
		errno = unbind_ack.RET_code;
		return (-1);
	}
	return (0);
}



_mb2_do_optmgmt (fd, optptr, mode)
int fd;
mb2_opts *optptr;
int mode;
{
	int ret, flags;
	struct mb2_optmgmt_req optm_req;
	struct mb2_optmgmt_ack optm_ack;
	struct strbuf ctlbuf;

	optm_req.PRIM_type = MB2_OPTMGMT_REQ;
	optm_req.RSVP_tout = optptr->tosendrsvp;
	optm_req.FRAG_tout = optptr->torcvfrag;
	optm_req.SEND_flow = optptr->fcsendside;
	optm_req.RECV_flow = optptr->fcrcvside;

	ctlbuf.len = sizeof (struct mb2_optmgmt_req);
	ctlbuf.buf = (char *) &optm_req;
	flags = RS_HIPRI;

	if (putmsg (fd, &ctlbuf, NULL, flags) < 0) 
		return (-1);

	ctlbuf.maxlen = sizeof (struct mb2_optmgmt_ack);
	ctlbuf.buf = (char *) &optm_ack;

	while (1) {
		if ((ret = getmsg (fd, &ctlbuf, (mb2_buf *)NULL, 
						&flags)) < 0) {
			if (mode == MB2_SYNC_MODE) {
				return (-1);
			} else {
				if (errno != EAGAIN)
					return (-1);
			}
		} else
			break;
	}

	if ((ret > 0) || (optm_ack.PRIM_type != MB2_OPTMGMT_ACK)) { 
		/* 
	 	 *  we are expecting an ack and we got something else 
	 	 */
		errno = ENXIO;
		return (-1);
	}

	if (optm_ack.RET_code) {
		errno = optm_ack.RET_code;
		return (-1);
	}
	return (0);
}


int
_mb2_do_info (fd, mb2infoptr, mode)
int fd;
mb2_info *mb2infoptr;
int mode;
{
	int ret, flags;
	struct mb2_info_req info_req;
	struct mb2_info_ack info_ack;
	struct strbuf ctlbuf;

	info_req.PRIM_type = MB2_INFO_REQ;
	ctlbuf.len = sizeof (struct mb2_info_req);
	ctlbuf.buf = (char *) &info_req;
	flags = RS_HIPRI;
	
	if (putmsg (fd, &ctlbuf, NULL, flags) < 0) 
		return (-1);

	ctlbuf.maxlen = sizeof (struct mb2_info_ack);
	ctlbuf.buf = (char *) &info_ack;
	flags = RS_HIPRI;

	while (1) {
		if ((ret = getmsg (fd, &ctlbuf, (mb2_buf *)NULL, 
						&flags)) < 0) {
			if (mode == MB2_SYNC_MODE) {
				return (-1);
			} else {
				if (errno != EAGAIN)
					return (-1);
			}
		} else
			break;
	}

	if ((ret > 0) || (info_ack.PRIM_type != MB2_INFO_ACK)) { 
		/* 
	 	 *  we are expecting an ack and we got something else 
	 	 */
		errno = EIO;
		return (-1);
	}
	mb2infoptr->socketid.hostid =  mps_mk_mb2soctohid (info_ack.SRC_addr);
	mb2infoptr->socketid.portid =  mps_mk_mb2soctopid (info_ack.SRC_addr);
	mb2infoptr->opts.tosendrsvp = info_ack.RSVP_tout;
	mb2infoptr->opts.torcvfrag = info_ack.FRAG_tout;
	mb2infoptr->opts.fcsendside = info_ack.SEND_flow;
	mb2infoptr->opts.fcrcvside = info_ack.RECV_flow;
	return (0);
}


int
_mb2_do_send (fd, socketidptr, ctlbufptr, databufptr)
int fd;
mb2_socket *socketidptr;
mb2_buf *ctlbufptr;
mb2_buf *databufptr;
{
	struct mb2_send_req *send_req;
	struct strbuf ctl;
	char *cbuf;

	send_req = (struct mb2_send_req *)mb2_user;
	
	send_req->PRIM_type = MB2_SEND_REQ;
	send_req->DEST_addr = (long) mps_mk_mb2socid (socketidptr->hostid, 
						 socketidptr->portid);
	send_req->DATA_length = 0;
	if (databufptr)
		send_req->DATA_length = databufptr->len;

	send_req->CTRL_length = 0;
	cbuf = NULL;
	if (ctlbufptr) {
		send_req->CTRL_length = ctlbufptr->len;
		cbuf = ctlbufptr->buf;
	}

	send_req->CTRL_offset = sizeof (struct mb2_send_req);
	if (send_req->CTRL_length != 0)
		(void) copy_bytes (cbuf, (char *)send_req + send_req->CTRL_offset, 
			(int)send_req->CTRL_length); 

	ctl.len = sizeof(struct mb2_send_req) + send_req->CTRL_length;
	ctl.buf = (char *) send_req;
		
	if (putmsg (fd, &ctl, databufptr, 0) < 0) 
		return (-1);
	return (0);
}


int
_mb2_do_brdcst (fd, portid, ctlbufptr)
int fd;
unsigned short portid;
mb2_buf *ctlbufptr;
{
	struct mb2_brdcst_req *brdcst_req;
	struct strbuf ctl;
	char *cbuf;

	brdcst_req = (struct mb2_brdcst_req *)mb2_user;
	
	brdcst_req->PRIM_type = MB2_BRDCST_REQ;
	brdcst_req->DEST_addr = (ulong) portid;

	brdcst_req->CTRL_length = 0;
	cbuf = NULL;
	if (ctlbufptr) {
		brdcst_req->CTRL_length = ctlbufptr->len;
		cbuf = ctlbufptr->buf;
	}

	brdcst_req->CTRL_offset = sizeof (struct mb2_brdcst_req);
	if (brdcst_req->CTRL_length != 0)
		(void) copy_bytes (cbuf, (char *)brdcst_req + brdcst_req->CTRL_offset, 
			(int)brdcst_req->CTRL_length); 

	ctl.len = sizeof(struct mb2_brdcst_req) + brdcst_req->CTRL_length;
	ctl.buf = (char *) brdcst_req;
		
	if (putmsg (fd, &ctl, NULL, 0) < 0) 
		return (-1);
	return (0);
}




int
_mb2_do_second_recv (fd, databufptr)
int fd;
mb2_buf *databufptr;
{
	int ret;
	struct strbuf ctl;
	int flags;

	ctl.maxlen = MAX_PSIZE;
	ctl.buf = (char *)mb2_user;
	flags = 0;

	if ((ret = getmsg (fd, &ctl, databufptr, &flags)) < 0)  {
		return (-1);
	}
	if (ctl.len != -1) {
		errno = EINVAL;
		return (-1);
	}
	if (ret > 0) {
		/* 
		 * there is data which has not been retreived
		 */
		errno = MB2_MORE_DATA;
		return (-1);
	}
	return (0);
}


int
_mb2_do_receive (fd, msginfoptr, ctlbufptr, databufptr, mode)
int fd;
mb2_msginfo *msginfoptr;
mb2_buf *ctlbufptr;
mb2_buf *databufptr;
int mode;
{
	struct mb2_msg_ind *msg_ind;
	struct strbuf ctl;
	int flags;
	long type;

	msg_ind = (struct mb2_msg_ind *)mb2_user;
	if (msginfoptr != NULL) {
		/*
		 * this is an an assumed  first time receive 
		 */
		ctl.maxlen = sizeof (struct mb2_msg_ind) + UNSOL_LENGTH;
		ctl.buf = (char *) msg_ind;
		flags = 0;
	
		if (getmsg (fd, &ctl, (mb2_buf *)NULL, &flags) < 0) {
			return (-1);
		}
		/* 
		 * we check the length of the returned message. 
		 * if the user had specified an errant msginfo value
		 * we will know here.
		 */
		if (ctl.len == 0) {
			errno = EINVAL;
			return (-1);
		}
	
		type = msg_ind->PRIM_type;
		if (mode == MB2_SYNC_MODE) {
			if ((type != MB2_REQ_MSG) && (type != MB2_NTRAN_MSG) &&
	 			(type != MB2_REQFRAG_MSG) && (type != MB2_STATUS_MSG)
					&& (type != MB2_BRDCST_MSG)) {

				/* this cannot happen if nobody screwed up */
	
				errno = EIO;
				return (-1);
			}
		}
		msginfoptr->msgtyp = type;
		msginfoptr->socketid.hostid =  mps_mk_mb2soctohid (msg_ind->SRC_addr);
		msginfoptr->socketid.portid =  mps_mk_mb2soctopid (msg_ind->SRC_addr);
		msginfoptr->tid = msg_ind->TRAN_id;
		msginfoptr->reference = msg_ind->REF_value;

		if (ctlbufptr == NULL) {
			if (msg_ind->CTRL_length != 0) {
				errno = EINVAL;
				return (-1);
			}
		} else {
			if (msg_ind->CTRL_length > RNDUP(ctlbufptr->maxlen)) {
				errno = EINVAL;
				return (-1);
			}
			if (msg_ind->CTRL_length != 0)
				(void) copy_bytes ((char *)msg_ind + 
						msg_ind->CTRL_offset, 
						ctlbufptr->buf, 
						(int)min (msg_ind->CTRL_length, ctlbufptr->maxlen));
			ctlbufptr->len = min (msg_ind->CTRL_length, ctlbufptr->maxlen);
		}

		if (msg_ind->PRIM_type == MB2_REQFRAG_MSG) {
			if (databufptr != NULL) 
				databufptr->len = msg_ind->DATA_length;
			return (0);
		}

		if (databufptr == NULL) {
			if (msg_ind->DATA_length != 0) {
				errno = MB2_MORE_DATA;
				return (-1);
			}
			return (0);
		} else {
			databufptr->len = msg_ind->DATA_length;
			if (msg_ind->DATA_length > RNDUP(databufptr->maxlen)) {
				errno = MB2_MORE_DATA;
				return (-1);
			}
			if (msg_ind->DATA_length != 0) {
				if (_mb2_do_second_recv (fd, databufptr) < 0)
					return (-1);
			} 
		}
	}
	return (0);	
}


int
_mb2_do_fragreq (fd, socketidptr, tid, fraglen, reference)
int fd;
mb2_socket *socketidptr;
unchar tid;
ulong fraglen;
ulong reference;
{
	struct mb2_frag_req frag_req;
	struct strbuf ctl;

	frag_req.PRIM_type = MB2_FRAG_REQ;
	frag_req.DEST_addr = mps_mk_mb2socid (socketidptr->hostid,
					  socketidptr->portid);
	frag_req.TRAN_id = tid;
	frag_req.REF_value = reference;
	frag_req.DATA_length = fraglen;

	ctl.len = sizeof (struct mb2_frag_req);
	ctl.buf = (char *)&frag_req;

	if (putmsg (fd, &ctl, NULL, 0) < 0)
		return (-1);
	return (0);
}



_mb2_do_sendrsvp (fd, socketidptr, reqcbufptr, reqdbufptr, resplen, reference)
int fd;
mb2_socket *socketidptr;
mb2_buf *reqcbufptr;
mb2_buf *reqdbufptr;
ulong resplen;
ulong reference;
{

	struct mb2_rsvp_req *rsvp_req;
	struct strbuf ctl;
	char *reqcbuf;
	
	rsvp_req = (struct mb2_rsvp_req *) mb2_user;
	rsvp_req->PRIM_type = MB2_RSVP_REQ;
	rsvp_req->DEST_addr = mps_mk_mb2socid (socketidptr->hostid,
					  socketidptr->portid);

	rsvp_req->REF_value = reference;
	rsvp_req->DATA_length = 0;
	if (reqdbufptr)
		rsvp_req->DATA_length = reqdbufptr->len;

	rsvp_req->RESP_length = resplen;
	rsvp_req->CTRL_length = 0;
	reqcbuf = NULL;
	if (reqcbufptr) {
		rsvp_req->CTRL_length  = reqcbufptr->len;
		reqcbuf = reqcbufptr->buf;
	}
	rsvp_req->CTRL_offset = sizeof (struct mb2_rsvp_req);

	if (rsvp_req->CTRL_length != 0)
		(void) copy_bytes (reqcbuf, (char *)rsvp_req + rsvp_req->CTRL_offset, 
			(int)rsvp_req->CTRL_length); 

	ctl.len = sizeof (struct mb2_rsvp_req) + rsvp_req->CTRL_length;
	ctl.buf = (char *)rsvp_req;

	if (putmsg (fd, &ctl, reqdbufptr, 0) < 0)
		return (-1);
	return (0);
}



_mb2_do_sendreply (fd, socketidptr, ctlbufptr, databufptr, tid, lastflag)
int fd;
mb2_socket *socketidptr;
mb2_buf *ctlbufptr;
mb2_buf *databufptr;
unchar tid;
int lastflag;
{
	struct mb2_reply_req *reply_req;
	struct strbuf ctl;
	char *cbuf;

	reply_req = (struct mb2_reply_req *) mb2_user;
	reply_req->PRIM_type = MB2_REPLY_REQ;
	reply_req->DEST_addr = mps_mk_mb2socid (socketidptr->hostid,
					   socketidptr->portid);
	reply_req->TRAN_id = tid;
	reply_req->EOT_flag = lastflag;

	reply_req->DATA_length = 0;
	if (databufptr != NULL)
		reply_req->DATA_length = databufptr->len;

	reply_req->CTRL_length = 0;
	cbuf = NULL;
	if (ctlbufptr != NULL) {
		reply_req->CTRL_length = ctlbufptr->len;
		cbuf = ctlbufptr->buf;
	}
	reply_req->CTRL_offset = sizeof (struct mb2_reply_req);
	if (reply_req->CTRL_length != 0)	
		(void) copy_bytes (cbuf, (char *)reply_req + reply_req->CTRL_offset, 
			(int)reply_req->CTRL_length); 
		/*
		 * note that in the above case it is okay to convert from a
		 * long to an int because the mb2_buf has a maxlen declared 
		 * as an int. If that changes however this and all other
		 * conversions should be changed.
		 */

	ctl.len = sizeof (struct mb2_reply_req) + reply_req->CTRL_length;
	ctl.buf = (char *) reply_req;

	if (putmsg (fd, &ctl, databufptr, 0) < 0)
		return (-1);
	return (0);
}

int
_mb2_do_sendcancel (fd, socketidptr, tid)
int fd;
mb2_socket *socketidptr;
unchar tid;
{
	struct mb2_cancel_req cancel_req;
	struct strbuf ctl;

	cancel_req.PRIM_type = MB2_CANCEL_REQ;
	cancel_req.DEST_addr = mps_mk_mb2socid (socketidptr->hostid,
						socketidptr->portid);
	cancel_req.TRAN_id = tid;
	
	ctl.len = sizeof (struct mb2_cancel_req);
	ctl.buf = (char *) &cancel_req;

	if (putmsg (fd, &ctl, NULL, 0) < 0)
		return (-1);
	return (0);
}
