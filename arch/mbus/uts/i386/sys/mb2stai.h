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

#ident	"@(#)mbus:uts/i386/sys/mb2stai.h	1.3"

/*	debug variables			*/

extern	int  mb2t_dbg_level;
extern	unsigned char ics_myslotid();


#define DBG_NONE	0
#define DBG_CALL	1
#define DBG_ERR		2
#define DBG_FULL	4

#ifdef DEBUG
#define	DEBUGS(x,y)		 if (dbg_level > x) dri_printf y
#else
#define DEBUGS(x,y)		
#endif

extern ulong mb2t_stat[];
/*
 * statistic bitmap in mb2t_stat
 */

#define NO_SEND_SENT	0
#define NO_SEND_COMP	1
#define NO_SEND_QUED	2
#define NO_RSVP_SENT	3
#define NO_RSVP_COMP	4
#define NO_RSVP_QUED	5
#define NO_FRAG_SENT	6
#define NO_FRAG_COMP	7
#define NO_FRAG_QUED	8
#define NO_REPLY_SENT	9
#define NO_REPLY_COMP	10
#define NO_REPLY_QUED	11
#define NO_CANCEL_SENT	12
#define NO_CANCEL_COMP	13
#define NO_CANCEL_QUED	14
#define NO_BRDCST_SENT	15
#define NO_BRDCST_COMP	16
#define NO_BRDCST_QUED	17
#define NO_COMP_MSGS	18
#define NO_INC_MSGS	19
#define NO_MSGS_RECV	20
#define NO_FRAGMENTED	21
#define NO_MSGS_REJ_FLO 22
#define NO_MSGS_UNDEL	23
#define NO_RSVP_TOUT	24
#define NO_FRAG_TOUT	25

#define MAX_STAT	50

#define SPL 	splstr 

/*	sleep priority				*/

#define SWRPRI	PZERO+3


/* definitions of sync module			*/

/* linked list of buffered messages		*/

typedef struct	mb2_msg_que {
	mblk_t	*mq_head;		/* head of the list */
	mblk_t	*mq_tail;		/* tail of the list */
} mb2_msg_que;

/* the modules private data structure */

typedef struct mb2_modep {
	queue_t *m_rdq;			/* pointer to read queue_t structure */
	mblk_t *m_iocmptr;		/* saved ioctl message pointer */
	ulong m_flag;			/* see below */
	ulong m_type;			/* type of request */
	ulong m_curref;			/* reference value */
	ulong m_ref_counter;		/* reference counter. 0 is an
					/* invalid value for this counter.
					 */
	mb2_msg_que m_msgq;		/* list of queued messages */
} mb2_modep;	

extern mb2_modep mb2s_modep[];	/* the modep table */

/*	m_flag values		*/

#define MB2_WAIT_MSG	0x1


/* definitions for the Async driver */

#define mb2t_NUMBER 	5
#define mb2t_NAME 	"mb2t"
#define mb2t_STRMIN 	0
#define mb2t_STRMAX 	32767
#define mb2t_HIWAT 	256
#define mb2t_LOWAT 	0
#define MB2_INCOMING 	0x100
#define MB2_OUTGOING	0x200
#define MB2_DEF_RSVPTOUT 0xffff	
#define MB2_DEF_FRAGTOUT 0xffff	
#define HIGH_PRI	0x100



/* primitive bind structure used for maintaining state */

struct mb2_prim_bind {
	ulong 		pb_flags;		/* described below */
	ulong 		pb_uref;		/* the users bind reference */
	ulong 		pb_tid;			/* the transaction id */
	ulong 		pb_datalen;		/* datalength */
	int 		pb_tval;		/* value returned from timout */
	struct _end_point *pb_endp;	/* pointer to endpoint struct */
	mblk_t 		*pb_mptr;		/* outgoing msg pointer */
	struct dma_buf *pb_odbuf;		/* outdoing mb2_datbuf ptr */
	mblk_t 		*pb_imptr;		/* stream msg incoming ptr */
	struct dma_buf *pb_idbuf;		/* incoming mb2_datbuf ptr */
	struct mb2_prim_bind *pb_next;		/* pointer to next on busy list */
	struct mb2_prim_bind *pb_prev;		/* pointer to the prev on busy list */
	struct mb2_prim_bind *pb_free;		/* list of free struct */
};


extern struct mb2_prim_bind mb2t_pbind [];	/* the prim_bind pool */

/* the endpoint structure */
typedef struct _end_point {
	queue_t *e_rdq;			/* pointer to the read queue_t struct */
	ulong e_strstate;		/* state of the stream	*/
	ulong e_taistate;		/* state of the TAI endpoint */
	long e_chan;			/* channel no */
	mb2socid_t e_socket;	/* source socket id */
	ulong e_rsvptout;		/* rsvp timeout */
	ulong e_fragtout;		/* frag req timeout */
	struct mb2_prim_bind e_pb;	/* maintain a list of outstanding trans */
} end_point;

extern end_point mb2t_endps[];		/* the endpoint table */

extern ushort mb2t_ports [];		/* the pool of allocatable ports */
	
/* stream states */

#define S_IDLE	0x1

/*  endpoint states	*/

#define T_BOUND 	0x1
#define T_CLOSING 	0x2
#define T_WAITPBIND	0x4
#define T_WAITTID	0x8


/*	configurable constants	for driver */

extern int mb2t_no_endps;
extern int mb2t_defrcv_hiwat;
extern int mb2t_defrcv_lowat;
extern int mb2t_defsnd_hiwat;
extern int mb2t_defsnd_lowat;
extern ulong mb2t_max_send_hiwat;
extern ulong mb2t_max_recv_hiwat;
extern ushort mb2t_first_port;
extern ulong mb2t_maxmsgs;
extern ulong mb2t_max_rsvp_tout;
extern ulong mb2t_def_rsvptout;
extern ulong mb2t_max_frag_tout;
extern ulong mb2t_def_fragtout;


/*	Synchronous module procedures 	*/


extern void mb2s_init_modep ();
extern void mb2s_send_flush ();
extern void mb2s_do_dnstr_msg ();
extern void mb2s_do_optmgmt ();
extern void mb2s_do_upstr_msg ();
extern int mb2s_match_type ();
extern ulong mb2s_allocref ();

/*	Async Driver procedures 	*/

extern int mb2topen(); 
extern int mb2tclose();
extern int mb2twput();
extern int mb2twsrv();
extern int mb2t_do_closing ();
extern int mb2trsrv();
extern void mb2tinit();
extern void mb2t_send_flush();
extern void mb2t_send_queue_msg();
extern int mb2t_val_send_req(); 
extern int mb2t_val_rsvp_req(); 
extern int mb2t_val_frag_req(); 
extern int mb2t_val_reply_req(); 
extern void mb2t_do_cntrl_msg ();
extern void mb2t_hostid_req();
extern void mb2t_hostid_ack();
extern void mb2t_bind_req();
extern void mb2t_bind_ack ();
extern void mb2t_unbind_req();
extern void mb2t_unbind_ack ();
extern void mb2t_optmgmt_req();
extern void mb2t_optmgmt_ack();
extern void mb2t_info_req();
extern void mb2t_info_ack();
extern void mb2t_putq ();
extern int mb2t_can_send_upstr ();
extern int mb2t_send_send_req(); 
extern int mb2t_send_unsol ();
extern int mb2t_send_sol ();
extern int mb2t_send_rsvp_req ();
extern int mb2t_send_frag_req ();
extern int mb2t_send_reply_req ();
extern int mb2t_send_cancel_req ();
extern int mb2t_send_brdcst_req ();
extern void mb2t_cancel_outtrans ();
extern int mb2t_intr ();
extern void mb2t_handle_comp ();
extern void mb2t_do_comp_close ();
extern void mb2t_send_req_comp ();
extern void mb2t_reply_req_comp ();
extern void mb2t_frag_req_comp ();
extern void mb2t_rsvp_req_comp ();
extern void mb2t_handle_inreq ();
extern void mb2t_msg_ind_comp ();
extern int mb2t_send_reqfrag_msg ();
extern void mb2t_send_status_msg ();
extern struct mb2_prim_bind *mb2t_get_unsol_buf ();
extern struct mb2_prim_bind *mb2t_get_sol_buf();
extern void mb2t_handle_undelmsg();
extern mblk_t *mb2t_getmptr();
extern end_point *mb2t_getendp ();
extern void mb2t_do_ioctl_msg ();
extern void mb2t_fill_pb ();
extern mblk_t *mb2t_get_strbuf ();
extern int mb2t_getsclass ();
extern struct dma_buf *mb2t_mk_dbuf ();
extern void mb2t_free_resource ();
extern void mb2t_fill_msg ();
extern void mb2t_rsvp_tout ();
extern void mb2t_frag_tout ();
extern struct mb2_prim_bind *mb2t_alloc_pb();
extern void mb2t_free_pb ();
extern ushort mb2t_alloc_port ();
extern void mb2t_free_port ();
extern void mb2t_init_endp ();
extern void mb2t_init_pb ();
extern void mb2t_await_resource ();
extern int mb2t_do_bufcall ();

/*	MB II transport routines				*/

extern struct dma_buf *mb2t_mk_datbuf();

/*	OS routines					*/

extern void splx ();
extern void wakeup ();
extern void untimeout ();
extern int timeout ();
extern void bcopy ();

/*	some redefinitions and macros		*/

#define memcpy(d, s, c) bcopy(s, d, c)
#define MSG_ISERR(mbp)  ((mps_msg_iserror(mbp) || (((mbp)->mb_flags&MPS_MG_ESOL)&&1)))
