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

#ident	"@(#)mbus:lib/libmb2/mb2a_prim.c	1.3"

#include <sys/types.h>
#include <sys/param.h>
#include <fcntl.h>
#include <sys/errno.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/mps.h>
#include <sys/mb2taiusr.h>
#include "_def.h"


int
mb2a_closeport (fd)
int fd;
{
	int ret;

	if (_mb2_check_fd (fd, MB2_ASYNC_MODE) < 0)
		return (-1);

	if (_mb2_do_unbind (fd, MB2_ASYNC_MODE) < 0) 
		return (-1);
	ret = close (fd);
	return (ret);
}



int
mb2a_getinfo (fd, mb2infoptr)
int fd;
mb2_info *mb2infoptr;
{

	if (mb2infoptr == NULL) {
		errno = EFAULT;
		return (-1);
	}
	if (_mb2_check_fd (fd, MB2_ASYNC_MODE) < 0)
		return (-1);

	return (_mb2_do_info (fd, mb2infoptr, MB2_ASYNC_MODE));
} 

int
mb2a_getreqfrag (fd, socketidptr, tid, fraglen, reference)
int fd;
mb2_socket *socketidptr;
unchar tid;
ulong fraglen;
ulong reference;
{

	if (_mb2_check_fd (fd, MB2_ASYNC_MODE) < 0) 
		return (-1);
	
	if (socketidptr == NULL) {
		errno = EINVAL;
		return (-1);
	}

	if (tid == 0) {
		errno = EINVAL;
		return (-1);
	}


	if (_mb2_do_fragreq (fd, socketidptr, tid, fraglen, reference) < 0)
		return (-1);

	return (0);

}


int
mb2a_openport (portid, optptr)
ushort portid;
mb2_opts *optptr;
{
	int fd, ret;
	int save_errno;
	
	if ((fd = open (a_dev_name, O_RDWR|O_NDELAY)) < 0) 
		return (-1);

	if ((ret = _mb2_do_bind (fd, portid, MB2_ASYNC_MODE)) < 0) {
		save_errno = errno;
		close (fd);
		errno = save_errno;
		return (ret);
	}

	if (optptr != NULL) {
		if ((ret = _mb2_do_optmgmt (fd, optptr, MB2_ASYNC_MODE)) < 0) {
			save_errno = errno;
			close (fd);
			errno = save_errno;
			return (ret);
		}
	}
	return (fd);
}


int
mb2a_receive (fd, msginfoptr, ctlbufptr, databufptr)
int fd;
mb2_msginfo *msginfoptr;
mb2_buf *ctlbufptr;
mb2_buf *databufptr;
{
	if (_mb2_check_fd (fd, MB2_ASYNC_MODE) < 0) 
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
		if (_mb2_do_receive (fd, msginfoptr, ctlbufptr, databufptr, MB2_ASYNC_MODE) < 0) {
			return (-1);
		}
	}
	return (0);
}
	

int
mb2a_send (fd, socketidptr, ctlbufptr, databufptr)
int fd;
mb2_socket *socketidptr;
mb2_buf *ctlbufptr;
mb2_buf *databufptr;
{


	if (_mb2_check_fd (fd, MB2_ASYNC_MODE) < 0) 
		return (-1);

	if (_mb2_check_range (ctlbufptr, databufptr) < 0)
		return (-1);

	if (_mb2_do_send (fd, socketidptr, ctlbufptr, databufptr) < 0)
		return (-1);
	return (0);
}



int
mb2a_brdcst (fd, portid, ctlbufptr)
int fd;
unsigned short portid;
mb2_buf *ctlbufptr;
{

	if (_mb2_check_fd (fd, MB2_ASYNC_MODE) < 0) 
		return (-1);

	if (_mb2_check_range (ctlbufptr, (mb2_buf *)NULL) < 0)
		return (-1);

	if (_mb2_do_brdcst (fd, portid, ctlbufptr) < 0)
		return (-1);
	return (0);
}


int
mb2a_sendcancel (fd, socketidptr, tid)
int fd;
mb2_socket *socketidptr;
unchar tid;
{

	if (_mb2_check_fd (fd, MB2_ASYNC_MODE) < 0) 
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
mb2a_sendreply (fd, socketidptr, ctlbufptr, databufptr, tid, lastflag)
int fd;
mb2_socket *socketidptr;
mb2_buf *ctlbufptr;
mb2_buf *databufptr;
unchar tid;
int lastflag;
{

	if (_mb2_check_fd (fd, MB2_ASYNC_MODE) < 0) 
		return (-1);

	if (socketidptr == NULL) {
		errno = EINVAL;
		return (-1);
	}

	if (_mb2_check_range (ctlbufptr, databufptr) < 0)
		return (-1);

	if (((lastflag != MB2_EOT) && (lastflag != MB2_NOTEOT)) ||
		((databufptr == NULL) && (lastflag != MB2_EOT))) {
		errno = EINVAL;
		return (-1);
	}
	if (tid == 0) {
		errno = EINVAL;
		return (-1);
	}
	if (_mb2_do_sendreply (fd, socketidptr, ctlbufptr, databufptr, tid, lastflag) < 0)
		return (-1);
	return (0);
}


int
mb2a_sendrsvp (fd, socketidptr, reqcbufptr, reqdbufptr, resplen, reference)
int fd;
mb2_socket *socketidptr;
mb2_buf *reqcbufptr;
mb2_buf *reqdbufptr;
ulong resplen;
ulong reference;
{


	if (_mb2_check_fd (fd, MB2_ASYNC_MODE) < 0) 
		return (-1);
	
	if (socketidptr == NULL) {
		errno = EINVAL;
		return (-1);
	}

	if (_mb2_check_range (reqcbufptr, reqdbufptr) < 0)
		return (-1);

	if (_mb2_do_sendrsvp (fd, socketidptr, reqcbufptr,
				reqdbufptr, resplen, reference) < 0)
		return (-1);
	return (0);	
}
