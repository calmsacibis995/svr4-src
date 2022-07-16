/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_MB2TAIUSR_H
#define _SYS_MB2TAIUSR_H

/*	Copyright (c) 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/sys/mb2taiusr.h	1.3"

#define UNSOL_LENGTH 	20
#define SOL_LENGTH 	16

#define MB2_SYNC_MODE	1
#define MB2_ASYNC_MODE	2

#define MB2_EOT		1
#define MB2_NOTEOT	0
#define MB2_RECV_CMD	99
#define MB2_OPTDEFAULT	0xffffffff

#define MB2_MORE_DATA	999	/* this value should not conflict with errno */

/*	primitives issued by the user library */

#define MB2_HOSTID_REQ	1
#define MB2_BIND_REQ 	2
#define MB2_UNBIND_REQ	3
#define MB2_OPTMGMT_REQ 4
#define MB2_INFO_REQ	5
#define MB2_SEND_REQ	6
#define MB2_RSVP_REQ	7
#define MB2_FRAG_REQ	8
#define MB2_REPLY_REQ	9
#define MB2_CANCEL_REQ	10
#define MB2_BRDCST_REQ	11
#define MB2_RECV_REQ	12
#define MB2_SYNC_CMD	13

/*	
 *	primitives issued by the service provider
 */

#define MB2_HOSTID_ACK	21
#define MB2_BIND_ACK	22
#define MB2_UNBIND_ACK	23
#define MB2_OPTMGMT_ACK 24
#define MB2_INFO_ACK	25
#define MB2_NTRAN_MSG	26
#define MB2_REQ_MSG	27
#define MB2_REQFRAG_MSG	28
#define MB2_FRAGRES_MSG	29
#define MB2_RESP_MSG	30
#define MB2_CANCEL_MSG	31
#define MB2_BRDCST_MSG	32
#define MB2_STATUS_MSG	33


/*  structure defining a MBII socket		*/

typedef struct {
	ushort hostid;
	ushort portid;
} mb2_socket;

/*  structure defining the info associated with a message received */
 
typedef struct {
	mb2_socket socketid;
	unsigned char tid;
	ulong msgtyp;
	ulong reference;
} mb2_msginfo;



typedef struct {
	int maxlen;
	int len;
	char *buf;
} mb2_buf;

/*	user opts available to be set			*/

typedef struct {
	ulong tosendrsvp;
	ulong torcvfrag;
	ulong fcsendside;
	ulong fcrcvside;
} mb2_opts;

/* 	info about the endpoint				*/

typedef struct {
	mb2_socket socketid;
	mb2_opts opts;
} mb2_info;

/*	primitives used by the service provider		*/


struct mb2_hostid_req {
	ulong PRIM_type;		/* always MB2_HOSTID_REQ */
};

struct mb2_hostid_ack {
	ulong PRIM_type;		/* always MB2_HOSTID_ACK */
	ulong HOST_id;			/* the hostid		*/
};

struct mb2_bind_req { 
	ulong PRIM_type;		/* always MB2_BIND_REQ */
	ulong SRC_port;			/* port id  */
};

struct mb2_bind_ack {
	ulong PRIM_type;		/* always MB2_BIND_ACK */
	ulong RET_code;			/* return code	*/
};

struct mb2_unbind_req { 
	ulong PRIM_type;		/* always MB2_UNBIND_REQ */
};

struct mb2_unbind_ack { 
	ulong PRIM_type;		/* always MB2_UNBIND_ACK */
	ulong RET_code;			/* return code */
};

struct mb2_optmgmt_req {
	ulong PRIM_type;		/* always MB2_OPTMGMT_REQ */
	ulong RSVP_tout;		/* RSVP timeout value */
	ulong FRAG_tout;		/* receive frag timeout value */
	ulong SEND_flow;		/* sendside flow control value */
	ulong RECV_flow;		/* receive side flow control value */
};

struct mb2_optmgmt_ack {
	ulong PRIM_type;		/* always MB2_OPTMGMT_ACK */
	ulong RET_code;			/* return code		 */
};

struct mb2_info_req {
	ulong PRIM_type;		/* always MB2_INFO_REQ	*/
};

struct mb2_info_ack {
	ulong PRIM_type;		/* always MB2_INFO_ACK	*/
	ulong SRC_addr;			/* source socketid	*/
	ulong RSVP_tout;		/* RSVP timeout value */
	ulong FRAG_tout;		/* receive frag timeout value */
	ulong SEND_flow;		/* sendside flow control value */
	ulong RECV_flow;		/* receive side flow control value */
};

struct mb2_send_req {
	ulong PRIM_type;		/* always MB2_NTMSG_REQ */
	ulong DEST_addr;		/* destination socketid */
	ulong DATA_length;		/* specifies wether sol or unsol */
	ulong CTRL_length;		/* length of control user data */
	ulong CTRL_offset;		/* offset of control user data */
};


struct mb2_rsvp_req {
	ulong PRIM_type;		/* always MB2_RSVP_REQ */
	ulong DEST_addr;		/* destination socketid */
	ulong REF_value;		/* reference value */
	ulong DATA_length;		/* length of output data */
	ulong RESP_length;		/* response data length */
	ulong CTRL_length;		/* length of control user data */
	ulong CTRL_offset;		/* offset of control user data */
};


struct mb2_frag_req {
	ulong PRIM_type;		/* always MB2_FRAG_REQ */
	ulong DEST_addr;		/* destination socketid */
	ulong TRAN_id;			/* transaction id	*/
	ulong REF_value;		/* reference value */
	ulong DATA_length;		/* length of data to be received */
};

struct mb2_reply_req {
	ulong PRIM_type;		/* always MB2_REPLY_REQ */
	ulong DEST_addr;		/* destination socketid */
	ulong TRAN_id;			/* transaction id	*/
	ulong EOT_flag;			/* last frag flag 	*/
	ulong DATA_length;		/* length of data to be sent */
	ulong CTRL_length;		/* length of control user data */
	ulong CTRL_offset;		/* offset of control user data */
};

struct mb2_cancel_req {
	ulong PRIM_type;		/* always MB2_CANCEL_REQ */
	ulong DEST_addr;		/* destination socketid */
	ulong TRAN_id;			/* transaction id	*/
};

struct mb2_brdcst_req {
	ulong PRIM_type;		/* always MB2_NTMSG_REQ */
	ulong DEST_addr;		/* destination portid */
	ulong CTRL_length;		/* length of control user data */
	ulong CTRL_offset;		/* offset of control user data */
};

struct mb2_msg_ind {		
	ulong PRIM_type;	   	/* MB2_NTRAN_MSG or MB2_REQ_MSG 
						etc ... */
	ulong SRC_addr;			/* source socketid */
	ulong TRAN_id;			/* transaction id	*/
	ulong REF_value;		/* refernce value 	*/	
	ulong DATA_length;		/* length of incoming data */
	ulong CTRL_length;		/* length of control user data */
	ulong CTRL_offset;		/* offset of control user data */
};


union primitives {	/* union of all primitives */
	ulong type;
	struct mb2_hostid_req hostid_req;
	struct mb2_bind_req bind_req;
	struct mb2_bind_ack bind_ack;
	struct mb2_unbind_req unbind_req;
	struct mb2_unbind_ack unbind_ack;
	struct mb2_optmgmt_req optmgmt_req;
	struct mb2_optmgmt_ack optmgmt_ack;
	struct mb2_info_req info_req;
	struct mb2_info_ack info_ack;
	struct mb2_send_req send_req;
	struct mb2_rsvp_req rsvp_req;
	struct mb2_frag_req frag_req;
	struct mb2_reply_req reply_req;
	struct mb2_cancel_req cancel_req;
	struct mb2_brdcst_req brdcst_req;
	struct mb2_msg_ind msg_ind;
};

/*	primitive passed to the Sync module  requesting a receive */

struct mb2_recv_req {
	ulong PRIM_type;		/* always MB2_RECV_REQ */
	ulong CTRL_length;		/* length of ctrl user data */
};

/*	definitions of library procedures */

extern ushort mb2_gethostid ();
extern int mb2s_closeport ();
extern int mb2s_getinfo ();
extern int mb2s_getreqfrag ();
extern int mb2s_openport ();
extern int mb2s_receive ();
extern int mb2s_send ();
extern int mb2s_sendcancel ();
extern int mb2s_sendreply ();
extern int mb2s_sendrsvp ();
extern int mb2a_closeport ();
extern int mb2a_getinfo ();
extern int mb2a_getreqfrag ();
extern int mb2a_openport ();
extern int mb2a_receive ();
extern int mb2a_send ();
extern int mb2a_sendcancel ();
extern int mb2a_sendreply ();
extern int mb2a_sendrsvp ();

#endif	/* _SYS_MB2TAIUSR_H */
