/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/cci.d/dl.d/interface.c	1.3"

#ifdef MBIIU
#include <sys/types.h>
#include <sys/mb2taiusr.h>
#include <sys/signal.h>
#endif

#include <stdio.h>
#include "trans.h"
#include "xprt.h"
#include "410.h"

extern FILE *ofile;	
extern unsigned short portid;
extern unsigned char slot_num;
extern void exit(); 
extern char *defread();
extern int defopen();
extern unsigned char *progname;
int ic_fd;
int xprt_fd;
TOKEN port_tok;
FILE *fnull;
socket_rec dest_socket;

/****************************************************************************/
/*                                                                          */
/*                     SYP MDP INITIALIZATION       				        */
/*                     ----------------------				                */
/*                                                                          */
/****************************************************************************/

#ifdef MBIIU
void mdp_init()
{
	WORD reg;
	BYTE value;
	WORD status;
	
	xprt_fd = mb2s_openport(portid, NULL);
	if (xprt_fd == -1) {
		fprintf(stderr,"\n%s: Transport not Initialized\n",progname);
		perror("init");
		exit(1);
	}
	
	ic_fd = open("/dev/ics", O_RDWR);
	if (ic_fd == -1) {
		fprintf(stderr,"\n%s: Interconnect not Initialized\n",progname);
		perror("open");
		exit(1);
	}
	dest_socket.hostid = slot_num;
	dest_socket.portid = DEST_PORT_ID; 
	}
#endif
	
/****************************************************************************/
/*                                                                          */
/*                     RMX INITIALIZATION       					        */
/*                     ------------------					                */
/*                                                                          */
/****************************************************************************/

#ifdef RMX286 
void rmx_init()
{
	p_info_rec port_buffer;
	WORD status;
	WORD reg;
	BYTE value;
	
	port_buffer.portid = 0;
	port_buffer.type = TRANSPORT_SERVICE;
	port_buffer.reserved = 0;
	port_buffer.flags = RMX_FLAG;
	port_tok = rq$create$port(10, &port_buffer, &status);
	if (status != 0) {
		fprintf(stderr,"\n%s: Transport Not Initialized\n",progname);
		exit(1);
	}
	dest_socket.ids[0] = slot_num;
	dest_socket.ids[1] = DEST_PORT_ID; 
	}
#endif
	
/****************************************************************************/
/*                                                                          */
/*                     TRANSPORT INITIALIZATION     		  		        */
/*                     ------------------------				                */
/*                                                                          */
/****************************************************************************/

void transport_init()
{
#ifdef MBIIU 
	mdp_init();
#else 
	rmx_init();
#endif

}

/****************************************************************************/
/*                                                                          */
/*                     GET PORT ID       		    					    */
/*                     -----------							                */
/*                                                                          */
/****************************************************************************/

unsigned short get_portid()
{
	unsigned short ret_value;
	WORD status;
	p_attr_rec p_buffer;
	int ret;

if (portid != 0)
	return(portid);
else {

#ifdef MBIIU 
	mb2u_info info_buf;

	ret = mb2s_getinfo(xprt_fd, &info_buf);
	return(info_buf.socketid.portid);
#else
	rq$get$port$attributes(port_tok, &p_buffer, &status);
	if (status != 0) {
		fprintf(stderr,"\n%s: Get Attributes Error\n",progname);
		exit(1);
	}
	return(p_buffer.portid);
#endif
	}	
} 

/****************************************************************************/
/*                                                                          */
/*                     GET INTERCONNECT       						        */
/*                     ----------------						                */
/*                                                                          */
/****************************************************************************/

unsigned char get_ic(slot, reg)
BYTE slot;
WORD reg;
{
	unsigned char value;
	WORD status;
	
#ifdef MBIIU 
	status = ics_read(ic_fd, slot, reg, &value, 1);
#else
	value = rq$get$interconnect(slot, reg, &status);
#endif

	if (status != 0) {
		fprintf(stderr,"\n%s: Interconnect Read Error\n",progname);
		exit(1);
	}

	return(value);
} 

/****************************************************************************/
/*                                                                          */
/*                     PUT INTERCONNECT       						        */
/*                     ----------------						                */
/*                                                                          */
/****************************************************************************/

void put_ic(slot, reg, value)
BYTE slot;
WORD reg;
BYTE value;
{
	WORD status;
	
#ifdef MBIIU 
	status = ics_write(ic_fd, slot, reg, &value,1);
#else
	rq$set$interconnect(value, slot, reg, &status);
#endif

	if (status != 0) {
		fprintf(stderr,"\n%s: Interconnect Write Error\n",progname);
		exit(1);
	}
}

/****************************************************************************/
/*                                                                          */
/*                     				DO SEND 			      		        */
/*                     				-------		        			        */
/*                                                                          */
/****************************************************************************/

void do_send(data_p, data_len)
unsigned char *data_p;
unsigned short data_len;
{
	char d_buf[16];
	int status;
	int count, i;
	WORD trans_id;

#ifdef MBIIU	
	mb2_buf ctl_out_buf;
	mb2_buf data_out_buf;

	ctl_out_buf.maxlen = (int)sizeof(d_buf);
	ctl_out_buf.len = (int)sizeof(d_buf);
	ctl_out_buf.buf = &(d_buf[0]);
	data_out_buf.maxlen = (int)data_len;
	data_out_buf.len = (int)data_len;
	data_out_buf.buf = (char *)data_p;

	status=mb2s_send(xprt_fd, &dest_socket, &ctl_out_buf, &data_out_buf);

#else
	for (i = 0; i < sizeof(d_buf); i++) 
		d_buf[i] = 0;
		
	trans_id = rq$send(port_tok, dest_socket.socket, &(d_buf[0]), data_p, 
	                   (long)data_len, 0, &status);	
#endif

	if (status != 0) {
		fprintf(stderr,"\n%s: MPC Transport Error\n",progname);
		fprintf(ofile,"stat is %08xH\n",status);
		exit(1);
	}
}

/****************************************************************************/
/*                                                                          */
/*                     			DO SLEEP 				      		        */
/*                     			--------		        			        */
/*                                                                          */
/****************************************************************************/

void do_sleep(count)
unsigned char count;
{
	WORD status;

#ifdef MBIIU
	nap(count);
#else
	rq$sleep(count, &status);
#endif
}

/****************************************************************************/
/*                                                                          */
/*                     GET SLOT ID       		    					    */
/*                     -----------							                */
/*                                                                          */
/****************************************************************************/

unsigned char get_slot_id()		
{
	
	WORD reg;
	unsigned char value;
	WORD status;

#ifdef MBIIU	
	reg = ics_find_rec(ic_fd, MY_SLOT, PSB_CONTROL_REC);
	if (reg == -1) {
		fprintf(stderr,"\n%s: Interconnect Record Not Found\n",progname);
		exit(1);
	}

	status = ics_read(ic_fd, MY_SLOT, (reg + SLOT_ID_OFFSET), &value, 1);
	value = value >> 3;
	return(value); 
#else
	return(rq$get$host$id(&status));
#endif
}

/****************************************************************************/
/*                                                                          */
/*                     OPEN BIT BUCKET      		    				    */
/*                     ---------------						                */
/*                                                                          */
/****************************************************************************/

FILE *open_bit_bucket()
{
	char *s;

#ifdef MBIIU
	s = "/dev/null";
#else
	s = ":BB:";
#endif

	if ((fnull = fopen(s, "w")) ==NULL) {
		fprintf(stderr,"\n%s: Can't Open %s\n",progname,s);
		exit(1);
	}
	return(fnull);
}

/****************************************************************************/
/*                                                                          */
/*                     STRING MOVE  	    		    				    */
/*                     -----------							                */
/*                                                                          */
/****************************************************************************/

void string_move(s, t)
char *s;
char *t;
{
	
#ifdef RMX286
		cstr(s, t);
#else				
		*s++;
		while (*t++ = *s++)
			;
#endif
}

/****************************************************************************/
/*                                                                          */
/*                     FIND RECORD		       						        */
/*                     -----------							                */
/*                                                                          */
/****************************************************************************/

short find_rec(slot, req_rec_type)

unsigned char slot;
unsigned char req_rec_type;

{
	unsigned char rec_type;
	unsigned short	curr_reg;

#ifdef MBIIU 

	return(ics_find_rec(ic_fd, slot, req_rec_type));
#else

	curr_reg = IC_FIRST_REC_OFFSET;

	rec_type = get_ic(slot, curr_reg);

	while (rec_type != IC_EOT) {
		if (rec_type == req_rec_type)
			return(curr_reg);
	
		else {
			curr_reg = curr_reg + get_ic(slot, curr_reg + IC_LEN_OFFSET) + 2;
			rec_type = get_ic(slot, curr_reg);
		}
	}
	return(-1);

#endif
}
			
/****************************************************************************/
/*                                                                          */
/*                    CATCH INTERRUPT           				            */
/*                    ---------------				                        */
/*                                                                          */
/****************************************************************************/

void catch_interrupt()
{

void on_intr();

#ifdef MBIIU
	signal(SIGINT, on_intr);
#endif

}

/****************************************************************************/
/*                                                                          */
/*                    ON INTERRUPT           					            */
/*                    ------------					                        */
/*                                                                          */
/****************************************************************************/
#ifdef MBIIU

void on_intr()
{

	WORD status;

	fprintf(stderr,"\n%s: INTERRUPT\n",progname);

	mb2s_closeport(xprt_fd);

	exit(1);
}
#endif

/****************************************************************************/
/*                                                                          */
/*                     SET DELAY		       						        */
/*                     -----------							                */
/*                                                                          */
/****************************************************************************/

unsigned int set_delay()

{
	char *t_delay;
	unsigned int time_delay;

#ifdef MBIIU 

	time_delay = DEFAULT_DELAY;	

	if (defopen(DEFFILE) == 0) {
		if (t_delay = defread("DELAY="))
			time_delay = atoi(t_delay);
	}
#else
	time_delay = DEFAULT_DELAY;	
#endif
	return(time_delay);
}
