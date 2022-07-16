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


#ident	"@(#)libnsl:nsl/t_rcvconnect.c	1.3.2.1"
#include "sys/types.h"
#include "sys/timod.h"
#include "sys/tiuser.h"
#include "sys/param.h"
#include "_import.h"

extern int t_errno;
extern rcv_conn_con();
extern struct _ti_user *_t_checkfd();


t_rcvconnect(fd, call)
int fd;
struct t_call *call;
{
	register struct _ti_user *tiptr;
	int retval;

	if ((tiptr = _t_checkfd(fd)) == NULL)
		return(-1);

	if (((retval = _rcv_conn_con(fd, call)) == 0) || (t_errno == TBUFOVFLW))
		tiptr->ti_state = TLI_NEXTSTATE(T_RCVCONNECT, tiptr->ti_state);
	return(retval);
}
