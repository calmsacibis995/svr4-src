/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_MPS_H
#define _SYS_MPS_H

/*	Copyright (c) 1986, 1987  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/sys/mps.h	1.3.3.1"

#ifndef _SYS_DMA_H
#include "sys/dma.h"
#endif
#ifndef _SYS_KMEM_H
#include "sys/kmem.h"
#endif

/* Structures and Macros for MB II Transport implementation */

/* Message Buffer */
#define MPS_MAXMSGSZ 32
typedef struct msgbuf {
	unsigned char mb_data[MPS_MAXMSGSZ];
	unsigned long mb_count;
	unsigned long mb_flags;
	int (*mb_intr)();
	unsigned long mb_bind;
	struct msgbuf *mb_next;
} mps_msgbuf_t;

/* Flags for mps_msgbuf_t.mb_flags field */
#define MPS_MG_TERR 0x01
#define MPS_MG_DONE 0x02
#define MPS_MG_ESOL 0x04

/* default for the mb_bind field.
 * Set for all incoming messages that are not completion message
 * for a request
 */
#define MPS_MG_DFBIND ((unsigned)0xFFFFFFFF)

typedef unsigned long mb2socid_t;
#define mps_mk_mb2socid(hostID, portID) ((mb2socid_t)((unsigned long)(hostID)<<16|(portID)))
#define mps_mk_mb2soctopid(sock) ((unsigned short)((sock)&0xffff))
#define mps_mk_mb2soctohid(sock) ((unsigned short)((sock)>>16))


/* MPC hardware message types */
#define MPS_MG_UNSOL	0x00
#define MPS_MG_BRDCST	0x01
#define MPS_MG_BREQ		0x24
#define MPS_MG_BREJ		0x34
#define MPS_MG_BGRANT	0x35

/* Offsets into the message of different fields */
#define MPS_MG_DA 0		/* Destination Address */
#define MPS_MG_SA 1		/* Source Address */
#define MPS_MG_MT 2		/* Message Type */
#define MPS_MG_RI 3		/* Request ID */
#define MPS_MG_BRLI MPS_MG_RI

/*Buffer Grants */
#define MPS_MG_BGLI 5	/* Liaison ID */
#define MPS_MG_BGDC 6	/* Duty Cycle */
#define MPS_MG_BGLL 7	/* LSB of Length */

/* Buffer Rejects */
#define MPS_MG_BJLI 5	/* Liaison ID */

/* Transport Fields */
/* Unsolicited Messages */
#define MPS_MG_UMPI 4	/* Unsolicited Message Protocol ID */
#define MPS_MG_UMDP 6	/* Unsolicited Message Destination Port ID */
#define MPS_MG_UMSP 8	/* Unsolicited Message Source Port ID */
#define MPS_MG_UMTI 10	/* Unsolicited Message Transaction ID */
#define MPS_MG_UMTC 11	/* Unsolicited Message Transaction Control */
#define MPS_MG_UMUD	12	/* Unsolicited Message User Data */

/* Transaction Control Bits */
#define MPS_MG_RRMSK 0x01	/* Request Response Mask */
#define MPS_MG_REQ   0x01	/* request message */
#define MPS_MG_RES	 0x00	/* response message */
#define MPS_MG_RRTMSK 0x06	/* Request response types mask */
#define MPS_MG_RQNF	 0x00	/* Request: No Message Fragmentation */
#define MPS_MG_RQMF	 0x02	/* Request: Message Fragmantation */
#define MPS_MG_RQSF	 0x04	/* Request: Send Next Fragment */
#define MPS_MG_RQXF	 0x06	/* Request: Next Fragment */
#define MPS_MG_RSET  0x00	/* Response: EOT */
#define MPS_MG_RSNE	 0x02	/* Response: Not EOT */
#define MPS_MG_RSCN	 0x04	/* Response: Cancel */

/* Buffer Request Message */
#define MPS_MG_BRML 5	/* Message Length (24 bits) */
#define MPS_MG_BRPI 8	/* Buffer Request Protocol ID */
#define MPS_MG_BRDP 10	/* Buffer Request Destination Port */
#define MPS_MG_BRSP 12	/* BUffer Request Source Port */
#define MPS_MG_BRTI 14	/* Buffer Request Transaction ID */
#define MPS_MG_BRTC 15	/* Buffer Request Transaction Control */
#define MPS_MG_BRUD 16	/* Buffer Request User Data */

/* Send Next Fragment Message */
#define MPS_MG_NFPI 4	/* SNF: Protocol ID */
#define MPS_MG_NFDP 6	/* SNF: Destination Port */
#define MPS_MG_NFSP 8	/* SNF: Source Port */
#define MPS_MG_NFTI 10	/* SNF: Transaction ID */
#define MPS_MG_NFTC 11	/* SNF: Transaction Control */
#define MPS_MG_NFFL	12	/* SNF: Fragment Length (32 bits) */

/* overhead of Transport message passing */
#define MPS_MG_UMOVHD 12
#define MPS_MG_BROVHD 16
#define MPS_MG_BGOVHD 8
#define MPS_MG_BJOVHD 8
#define MPS_MG_NFOVHD 16
#define LOW(x)	((short)(x))
#define HIW(x)	((short)((x)>>16))
#define MPS_LOB(x)  ((char)(x))
#define MPS_HIB(x)	((char)((x)>>8))

/* Implementation Defined Constants */
#define MPS_MG_DUTY1C 0x1E
#define MPS_MG_DUTY2C 0x8A


#define mps_set_rid(mbp) ((mbp)->mb_data[MPS_MG_RI]=RID_MASK&(mps_requestid()))
#define mps_msg_getrid(mbp) ((mbp)->mb_data[MPS_MG_RI])

/* Macros to decode Messages */

#define mps_msg_getsrcmid(mbp) ((mbp)->mb_data[MPS_MG_SA])
#define mps_msg_getdstmid(mbp) ((mbp)->mb_data[MPS_MG_DA])
#define mps_msg_getmsgtyp(mbp) ((mbp)->mb_data[MPS_MG_MT])
#define mps_msg_getreqid(mbp)  ((mbp)->mb_data[MPS_MG_RI])
#define mps_msg_getlsnid(mbp)  ((mbp)->mb_data[MPS_MG_MT]==MPS_MG_BREQ?(mbp)->mb_data[MPS_MG_BRLI]:(mbp)->mb_data[MPS_MG_BGLI])
#define mps_msg_getsrcpid(mbp) ((mbp)->mb_data[MPS_MG_MT]==MPS_MG_BREQ?*(unsigned short *)(&(mbp)->mb_data[MPS_MG_BRSP]):*(unsigned short *)(&(mbp)->mb_data[MPS_MG_UMSP]))
#define mps_msg_getdstpid(mbp) ((mbp)->mb_data[MPS_MG_MT]==MPS_MG_BREQ?*(unsigned short *)(&(mbp)->mb_data[MPS_MG_BRDP]):*(unsigned short *)(&(mbp)->mb_data[MPS_MG_UMDP]))
#define mps_msg_gettrnsid(mbp) ((mbp)->mb_data[MPS_MG_MT]==MPS_MG_BREQ?(mbp)->mb_data[MPS_MG_BRTI]:(mbp)->mb_data[MPS_MG_UMTI])
#define mps_msg_gettransctl(mbp) ((mbp)->mb_data[MPS_MG_MT]==MPS_MG_BREQ?(mbp)->mb_data[MPS_MG_BRTC]:(mbp)->mb_data[MPS_MG_UMTC])
#define mps_msg_isreq(mbp) ((mps_msg_gettrnsid(mbp)!=0)&&(((mbp)->mb_data[MPS_MG_MT]==MPS_MG_BREQ)?(((mbp)->mb_data[MPS_MG_BRTC]&MPS_MG_RRMSK)==MPS_MG_REQ):(((mbp)->mb_data[MPS_MG_UMTC]&MPS_MG_RRMSK)==MPS_MG_REQ)))
#define mps_msg_iserror(mbp) (!!((mbp)->mb_flags&MPS_MG_TERR))
#define mps_msg_iscompletion(mbp) (!!((mbp)->mb_flags&MPS_MG_DONE))
#define mps_msg_iscancel(mbp) ((mps_msg_getmsgtyp(mbp)==MPS_MG_UNSOL)&&(((mbp)->mb_data[MPS_MG_UMTI]!=0)&&(((mbp)->mb_data[MPS_MG_UMTC]&(MPS_MG_RRMSK|MPS_MG_RRTMSK))==(MPS_MG_RES|MPS_MG_RSCN))))
#define mps_msg_iseot(mbp) ((mps_msg_gettrnsid(mbp)!=0)&&((mbp)->mb_data[MPS_MG_MT]==MPS_MG_BREQ?(((mbp)->mb_data[MPS_MG_BRTC]&(MPS_MG_RRTMSK|MPS_MG_RRMSK))==(MPS_MG_RES|MPS_MG_RSET)):(((mbp)->mb_data[MPS_MG_UMTC]&(MPS_MG_RRTMSK|MPS_MG_RRMSK))==(MPS_MG_RES|MPS_MG_RSET))))
#define mps_msg_getprotid(mbp) ((mbp)->mb_data[MPS_MG_MT]==MPS_MG_BREQ?(mbp)->mb_data[MPS_MG_BRPI]:(mbp)->mb_data[MPS_MG_UMPI])
#define mps_msg_setsrcpid(mbp,pid) \
{ \
	if((mbp)->mb_data[MPS_MG_MT]==MPS_MG_BREQ) \
		*(unsigned short *)&(mbp)->mb_data[MPS_MG_BRSP] = (pid); \
	else \
		*(unsigned short *)&(mbp)->mb_data[MPS_MG_UMSP] = (pid); \
}
#define mps_msg_setbrlen(mbp,len) \
{ \
	*(unsigned short *)&(mbp)->mb_data[MPS_MG_BRML] = LOW(len); \
	(mbp)->mb_data[MPS_MG_BRML+2] = MPS_LOB(HIW(len)); \
}
#define mps_msg_getbrlen(mbp) ((*(unsigned long *)&(mbp)->mb_data[MPS_MG_BRML])&0x00ffffff)
#define mps_msg_getfraglen(mbp) ((*(unsigned long *)&(mbp)->mb_data[MPS_MG_NFFL])&0x00ffffff)
#define mps_msg_setcancel(mbp) ((mbp)->mb_data[MPS_MG_UMTC] |= MPS_MG_RSCN)
#define mps_msg_setbglen(mbp,len) ((mbp)->mb_data[MPS_MG_BGLL] = len)
#define mps_msg_setduty(mbp,duty) ((mbp)->mb_data[MPS_MG_BGDC] = duty)
#define mps_msg_setrid(mbp) ((mbp)->mb_data[MPS_MG_RI]=mps_requestid()&RID_MASK)
#define mps_msg_getudp(mbp) \
	((mbp)->mb_data[MPS_MG_MT]==MPS_MG_BREQ?&(mbp)->mb_data[MPS_MG_BRUD]:&(mbp)->mb_data[MPS_MG_UMUD])


#define MPS_MB2_TPDT 0x2 	/* Multibus 2 Transport Protocol
			 *  for Data Transmission
			 */

/* pre defined priorities that can be associated with ports
 * (supplied to mps_open_chan() calls)
 */
#define MPS_CLKPRIO 0x7
#define MPS_BLKPRIO 0x6
#define MPS_SRLPRIO 0x5
#define MPS_NRMPRIO 0x0

#ifdef _KERNEL
extern unsigned char mps_requestid();
extern unsigned short mps_lhid();
#endif

#define MPS_FP_PORT 0x3666


/* Structures internal to MB II Transport Implementation */

typedef struct port {
	unsigned long pr_flags;
	int (*pr_intr)();		/* interrupt handler */
	unsigned long pr_level;		/* interrupt priority level */
	unsigned char pr_ltid;		/* last tid used */
} port_t;

typedef struct tinfo {
	unsigned short t_state;	/* Transaction State */
	unsigned short t_flags;	/* Transaction flags */
	unsigned long t_lcid;	/* local channel ID */
	mb2socid_t t_lsocid;	/* local socket ID */
	mb2socid_t t_rsocid;	/* remote socket ID */
	mps_msgbuf_t *t_omsg;	/* Outgoing message */
	mps_msgbuf_t *t_imsg;	/* Incoming Message */
	struct dma_buf *t_obuf;	/* Outgoing data buffer */
	unsigned long t_ocnt;	/* Outgoing data count */
	unsigned long t_orem;	/* outgoing data remainder count */
	struct dma_buf *t_ibuf;	/* Incoming */
	unsigned long t_icnt;	/*    ,,    */
	unsigned long t_irem;	/*    ,,    */
	struct tinfo *t_next;	/* ptr to next in queue */
} tinfo_t;

/* Flags and states */
#define MG_RSVP	0x1	/* rsvp transaction entry */
#define MG_FRAG 0x2	/* receive fragmentation entry */
#define MG_RCV	0x4	/* receive entry */
#define MG_SDATA 0x8	/* send_data entry(non-transaction */
#define MG_SRPLY 0x10	/* send_reply entry */
#define MG_ENTRY 0x1F	/* entry type mask */
#define MG_IALGN 0x20	/* aligned sol input */
#define MG_OALGN 0x40	/* aligned sol output */

/*states */
#define MG_INITST 0x1	/* init state */

					/* send_rsvp */
#define MG_RS_RS 0x2	/* request sent */
#define MG_RS_FR 0x3	/* fragmentation */
#define MG_RS_RC 0x4	/* request complete */
#define MG_RS_PRR 0x5	/* partial response received */
					/* receive fragmentation */
#define MG_FR_BRR 0x2	/* buffer request received */
					/* send_data */
#define MG_SD_N	0x2	/* no states */
					/* receive */
#define MG_RC_N 0x2	/* no states */
					/* send_reply */
#define MG_RC_N 0x2	/* no states */

/* error types */
#define EDELIV   0x1	/* msg could not be delivered to port */
#define ENOENTRY 0x2	/* transaction entry to receive msg doesn't exist */
#define ESTATE   0x4	/* msg received out of state */

/* transactions */
#define MG_REMTRN 0x1		/* remote transactions-fragmentation */
#define MG_LOCTRN 0x2		/* transactions initiated locally */

/* Priority message queues */
typedef struct msgque {
	mps_msgbuf_t *m_qhead;
	mps_msgbuf_t *m_qtail;
	int m_count;
} msgque_t;

#define SOC 0x1		/* indicates solicited output */
#define SIC 0x2		/* idicates solicited input */


#if defined __STDC__
/*
 * Since prototype definitions do allow less than word width parameters all 
 * char's and short's have been changed to int's. 
 */
extern long 	mps_AMPsend(long,mps_msgbuf_t *);
extern long 	mps_AMPsend_rsvp(long, mps_msgbuf_t *, struct dma_buf *, 
					struct dma_buf *);
extern long 	mps_AMPreceive(long, mb2socid_t, mps_msgbuf_t *, 
					struct dma_buf *);
extern long		mps_AMPreceive_frag(long, mps_msgbuf_t *, mb2socid_t, 
					unsigned int, struct dma_buf *);
extern long 	mps_AMPsend_reply(long, mps_msgbuf_t *, struct dma_buf *);
extern long 	mps_AMPsend_data(long, mps_msgbuf_t *, struct dma_buf *);
extern long 	mps_AMPcancel(long, mb2socid_t, unsigned int);
extern void 	mps_msg_proc(mps_msgbuf_t *);
extern void 	mps_msg_comp(tinfo_t *,int );
extern void 	mps_msg_dispatch();
extern long 	mps_open_chan(unsigned int, int (*)(), unsigned int);
extern long 	mps_close_chan(long);
extern unsigned short mps_lhid();
extern void 	mpsinit();
extern void 	mps_free_msgbuf(mps_msgbuf_t *);
extern mps_msgbuf_t *mps_get_msgbuf(int);
extern void 	mps_free_dmabuf (struct dma_buf *);
extern struct dma_buf *mps_get_dmabuf(unsigned long,unsigned int);
extern struct dma_buf *mps_dmabuf_breakup(struct dma_buf *,long);
extern struct dma_buf *mps_dmabuf_join(struct dma_buf *,struct dma_buf *);
extern unsigned char mps_get_tid(long);
extern long 	mps_free_tid(long,unsigned int);
extern int 	mps_find_transaction(mb2socid_t,unsigned int, int);
extern long 	mps_buf_count(struct dma_buf *);
extern void 	mps_msg_showmsg(mps_msgbuf_t *);
extern void 	mps_mk_unsol(mps_msgbuf_t *, mb2socid_t, unsigned int, 
					unsigned char *,unsigned long);
extern void 	mps_mk_brdcst(mps_msgbuf_t *, unsigned int, 
					unsigned char *, unsigned long);
extern void 	mps_mk_sol(mps_msgbuf_t *, mb2socid_t, unsigned int, 
					unsigned char *, unsigned long);
extern void 	mps_mk_unsolrply(mps_msgbuf_t *, mb2socid_t, unsigned int, 
					unsigned char *, unsigned long);
extern void 	mps_mk_solrply(mps_msgbuf_t *, mb2socid_t, unsigned int, 
					unsigned char *, unsigned long, 
					unsigned int);
extern void 	mps_mk_bgrant(mps_msgbuf_t *, mb2socid_t, unsigned int, 
					unsigned long);
extern void 	mps_mk_breject(mps_msgbuf_t *,mb2socid_t, unsigned int);
extern void 	mps_mk_snf(mps_msgbuf_t *, mb2socid_t, unsigned int, 
					unsigned long);
extern void 	mps_get_unsoldata(mps_msgbuf_t *, unsigned char *, 
					unsigned long);
extern void 	mps_get_soldata(mps_msgbuf_t *, unsigned char *, 
					unsigned long);
extern unsigned char mps_requestid();
#else
extern long 	mps_AMPsend();
extern long 	mps_AMPsend_rsvp();
extern long 	mps_AMPreceive();
extern long 	mps_AMPreceive_frag();
extern long 	mps_AMPsend_reply();
extern long 	mps_AMPsend_data();
extern long 	mps_AMPcancel();
extern void 	mps_msg_proc();
extern void 	mps_msg_comp() ;
extern void 	mps_msg_dispatch();
extern long 	mps_open_chan();
extern long 	mps_close_chan();
extern unsigned short mps_lhid();
extern void 	mpsinit();
extern void 	mps_free_msgbuf();
extern mps_msgbuf_t *mps_get_msgbuf();
extern void 	mps_free_dmabuf ();
extern struct dma_buf *mps_get_dmabuf();
extern struct dma_buf *mps_dmabuf_breakup();
extern struct dma_buf *mps_dmabuf_join();
extern unsigned char mps_get_tid();
extern long 	mps_free_tid();
extern int 	mps_find_transaction();
extern long 	mps_buf_count();
extern void 	mps_msg_showmsg();
extern void 	mps_mk_unsol();
extern void 	mps_mk_brdcst();
extern void 	mps_mk_sol();
extern void 	mps_mk_unsolrply();
extern void 	mps_mk_solrply();
extern void 	mps_mk_bgrant();
extern void 	mps_mk_breject();
extern void 	mps_mk_snf();
extern void 	mps_get_unsoldata();
extern void 	mps_get_soldata();
extern unsigned char mps_requestid();
#endif

#endif	/* _SYS_MPS_H */
