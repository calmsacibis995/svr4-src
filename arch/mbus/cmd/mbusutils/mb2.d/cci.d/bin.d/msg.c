/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/cci.d/bin.d/msg.c	1.3"

#define MSGFILE


#include <stdio.h>
#include "common.h"
#include <sys/types.h>
#include <sys/mb2taiusr.h>
#include "cci.h"
#include "utils.h"
#include "main.h"


/*
 *	Global Data Structures
 */

	static unsigned short	dest_hostid;
	static mb2_minfo		msgbuf;
	static mb2_buf			req_buf;
	static mb2_buf			reply_buf;			
	static int				mb2_fd;
	static short 			my_port_id; /* used in myexit */
	

/****************************************************************************/
/*                                                                          */
/*                          MOV BYTE	                                    */
/*                          --------                                        */
/*                                                                          */
/****************************************************************************/

void movb(src_p, dest_p, len)

	char			*src_p;
	char			*dest_p;
	unsigned short	len;


{

	if (src_p == NULL || dest_p == NULL) 
		return;

	while (len > 0) {
		*dest_p = *src_p;
		++src_p;
		++dest_p;
		--len;
	}

}
/****************************************************************************/
/*                                                                          */
/*                          READ    	   		                            */
/*                          -----                                           */
/*                                                                          */
/****************************************************************************/

unsigned long myread(fp, buf_p, loc, size)

FILE			*fp;
unsigned char	*buf_p;
unsigned long	loc;
unsigned long	size;

{
	int status;
	int count;
	
	status = fseek(fp, loc, 0);

	count = fread(buf_p, 1, (int)size, fp);
	if (count != (int)size)
		return(0);
		
	return(size);
}
	
/****************************************************************************/
/*                                                                          */
/*                          INIT    	   		                            */
/*                          ----                                            */
/*                                                                          */
/****************************************************************************/

cci_init(dest_host,port_id)

unsigned short	dest_host;
unsigned short	port_id;


{
	dest_hostid = dest_host;
	my_port_id = port_id;

	mb2_fd = mb2s_openport(port_id, NULL);
	if (mb2_fd == -1) {
		perror("open");
		exit(1);
	}	

	return(E_OK);
		
}

/****************************************************************************/
/*                                                                          */
/*                          SEND CCI CMD		                            */
/*                          ------------                                    */
/*                                                                          */
/****************************************************************************/

unsigned long cci_send_cmd (dest_portid,
							req_buf_p, reply_buf_p, sol_data_p, sol_len, 
								rsvp_buf_p, rsvp_len, status_p)

unsigned short	dest_portid;
req_rec			*req_buf_p;
char 			*reply_buf_p;
char			*sol_data_p;
unsigned long	sol_len;
char			*rsvp_buf_p;
unsigned long	rsvp_len;
int 			*status_p;
{
	mb2_buf req_ctl_buf;
	mb2_buf resp_ctl_buf;
	char resp_buf;

	req_ctl_buf.buf = (char *)req_buf_p;
	req_ctl_buf.maxlen = (sol_len == 0) ? 20 : 16;  
	req_ctl_buf.len = (sol_len == 0) ? 20 : 16;
	req_buf.maxlen = sol_len;
	req_buf.len = sol_len;
	req_buf.buf = sol_data_p;

	resp_ctl_buf.buf = reply_buf_p;
	resp_ctl_buf.maxlen = CCI_MSG_SIZE;  
	reply_buf.maxlen = rsvp_len;
	reply_buf.buf = rsvp_buf_p;

	msgbuf.socketid.hostid = dest_hostid;
	msgbuf.socketid.portid = dest_portid;

	*status_p = mb2s_sendrsvp(mb2_fd, &msgbuf, &req_ctl_buf, 
					&req_buf, &resp_ctl_buf, &reply_buf);
	
	if (*status_p != 0) {
		perror("sendrsvp");
		myexit(1);
	}

	if ((msgbuf.socketid.hostid != dest_hostid) || 
		(msgbuf.msgtyp == MB2_CANCEL_MSG) ||  
		(msgbuf.msgtyp == MB2_STATUS_MSG))  {
		*status_p = E_ERROR;
		return(0L);
	}
	return(resp_ctl_buf.len);
}

myexit(status)
short status;

{
	int my_fd;

	if ((my_fd = mb2s_openport(my_port_id, NULL)) == -1)
		mb2s_closeport(mb2_fd);
	else
		mb2s_closeport(my_fd);
	exit(status);
}

void
on_intr()
{
	printf("\n ------------Interrupt------------\n");
	myexit(1);
}
