/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988, 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/io/enetdrv/lci.c	1.3.1.1"

/*
**  Intel Corporation
**  Microcomputer Communications Development
**
** MODIFICATIONS:
**	I000	June 24, 1988	rjs
**		Don't send out init message to non-existent board
**	I001	8/28/77		RJS		Intel
**		Save RB ptr inside RB because of phystokv() <=> kvtophys()
**		weirdness.
**	I002 	01/29/89	rjs
**		During boot-up, the board appears to accept our first RSVP
**		sent in lciinit() and then locks up.  We need to cancel
**		this transaction and free it's tid when lci_reinit() is
**		called from timeout.  Otherwise, we'll lose the tid forever
**		and never be able to close the channel.
**	I003	07/06/89	rjf		Intel
**		Added EDL support.
**	I004	07/11/89	rjf		Intel
**		Made lint fixes.
**	I005 	08/09/89	rjf,vish	Intel
**		Added register_RB to connect, accept, close.
*/
/* 
* TITLE: enet LCI Device Driver
*
* DATE: May 12, 1987
*
*
*      The enet LCI device driver provides the functions necessary to 
* communicate with the LCI on the enet board.  For this purpose it
* provides the following routines:
*
* a) lciinit: send a init message (unsol message) to the LCI server on the enet
* b) mipsend: send a data message (frame) to the LCI server on the enet
*
*
* Warning: This code assumes that the kernel data segment is in the
*          first megabyte of memory.  It will return an error if this
*          is not the case.   Also this code will have to be modified
*          to work under large model.
*
* ROUTINES:  
* 
*             mipsend 
*             lciinit
*
*/
#define DEBUG 1
#include "sys/enet.h"
#include "sys/lcidef.h"
#include <sys/immu.h>

extern    void enet_rcv_rb();
extern    void enet_send_data();
extern    void enet_rcv_data();

extern    unsigned long lci_rb_chan;
extern    unsigned long lci_xmit_chan;
extern    unsigned long lci_rcv_chan;
extern    unsigned char ics_myslotid();
extern	  ulong init_tid;

extern  int g_lci_error;
extern  int mps_max_tran;				/* I002 */

extern struct dma_buf	*lci_rcv_datbuf_p;

extern struct enetboard	enet_boards[];
extern struct enetcfg	enet_cfg[];
extern struct req_blk	enet_rb_list[];
extern pend_list	enet_pend_list[];
extern int		enet_rbtabsize;

/*
 * enet debug level defined in enet.c
 */
extern int		enet_debug;

void
lci_reinit()
    { 
    int i;
    extern int enet_n_boards;    /* number of enet boards to initialize    */
    mps_msgbuf_t *mbp;
    lci_init_str    init_data;
    unsigned long socket_id;
    long ret;


    /* send the init message to the lci server on the enet board */
    for (i=0; i < enet_n_boards; i++)
    {
           if ((enet_boards[i].state & INIT_PENDING) == INIT_PENDING)
       {
        mbp = mps_get_msgbuf(KM_NOSLEEP);
	if (init_tid < mps_max_tran)				/* I002 */
	{
		ret = mps_AMPcancel((long)lci_rb_chan,
				mps_mk_mb2socid(ics_myslotid(), LCI_RB_PORT),
				(unchar)init_tid);
		ret = mps_free_tid((long)lci_rb_chan, (unchar)init_tid);
	}
        init_tid = mps_get_tid((long)lci_rb_chan);
        DEBUGP(DEB_FULL,(CE_CONT, "lci_reinit 1: got trans id; tid=%x\n",init_tid));
        socket_id = ((ulong)enet_cfg[i].slot_id << 16);
        socket_id |= enet_cfg[i].lcis_rb_port;    
        init_data.opcode = LCI_INIT_OPCODE; /* 31416 */
        init_data.hostid = ics_myslotid();
        init_data.rbport = LCI_RB_PORT;
        init_data.txport = LCI_XMIT_PORT;
        init_data.rxport = LCI_RCV_PORT;
        init_data.delport = 0;
        init_data.maxbufs = 1;
        init_data.maxrxsz = 0xffff;
	DEBUGP(DEB_FULL,(CE_CONT, "lci_reinit 2: about to make unsol msg,socket_id=%x\n",socket_id));
        mps_mk_unsol (mbp,socket_id,(uchar)init_tid,(unsigned char *)&init_data,sizeof(init_data));
        DEBUGP(DEB_FULL,(CE_CONT, "lci_reinit(): About to resend init_msg. Socket id=%x\n",socket_id));
        if (mps_AMPsend_rsvp((long)lci_rb_chan,mbp,(struct dma_buf *)NULL,(struct dma_buf *)NULL) == -1)
            {
            /* log init error */
            DEBUGP(DEB_FULL,(CE_CONT, "lci_reinit(): Error response from mps_AMPsend_rsvp()\n"));
            g_lci_error = E_LCI_INIT;
            mps_free_msgbuf(mbp);
            ret = mps_free_tid((long)lci_rb_chan, (unchar)init_tid);
	    init_tid = mps_max_tran;
            }
        DEBUGP(DEB_FULL,(CE_CONT, "lci_reinit(): Returned from re_init_msg. Waiting for response\n"));
        enet_boards[i].state |= INIT_PENDING;
        timeout(lci_reinit, (caddr_t)NULL, 500);
            }
       }
    }

/*
* lciinit 
*   Initialize lci queues     
*
* Inputs:
*     none
*
* Outputs:
*     none
*/
lciinit(bnum)
int	bnum;
{
    int i;
    extern int enet_n_boards;    /* number of enet boards to initialize    */
    mps_msgbuf_t *mbp;
    lci_init_str    init_data;
    unsigned long socket_id;
    long    ret;

/*
    for (i = 0; i < enet_n_boards; i++) {
        enet_boards[i].qxmt.d_empty = M_NO_CHANGE;
        enet_boards[i].qxmt.d_full = M_NO_CHANGE;
        enet_boards[i].qrcv.d_empty = M_NO_CHANGE;
        enet_boards[i].qrcv.d_full = M_NO_CHANGE;
    }
*/
    g_lci_error = 0;
    DEBUGP(DEB_CALL,(CE_CONT, "lciinit(): Entry Point; %d boards to initialize\n",enet_n_boards));
        
    /* allocate a datbuf chain of length LCI_RCV_MAXBUFS to receive
        incoming data 
    */
/*    lci_rcv_datbuf_p = mps_get_dmabuf(LCI_RCV_MAXBUFS,DMA_NOSLEEP); */

    /* attach the interrupt handlers to the appropriate ports */
    if ((lci_rb_chan = mps_open_chan(LCI_RB_PORT, (int (*)())enet_rcv_rb, MPS_BLKPRIO)) == -1)
        g_lci_error = E_LCI_OPEN_CHAN;
    if ((lci_xmit_chan = mps_open_chan(LCI_XMIT_PORT, (int (*)())enet_send_data, MPS_BLKPRIO)) == -1)
        g_lci_error = E_LCI_OPEN_CHAN;
    if ((lci_rcv_chan = mps_open_chan(LCI_RCV_PORT, (int (*)())enet_rcv_data, MPS_BLKPRIO)) == -1)
        g_lci_error = E_LCI_OPEN_CHAN;

    if (g_lci_error !=0)
        {
        cmn_err(CE_WARN, "LCI Initialization Error: mps_open_chan().\n");
        return; 
        }

    DEBUGP(DEB_CALL,(CE_CONT, "lciinit(): Interrupt Handlers attached.\n"));

    /* send the init message to the lci server on the enet board */
    for (i=0; i < enet_n_boards; i++)
    {
        if (!(enet_boards[i].state & PRESENT))		/* I000 */
		continue;
        mbp = mps_get_msgbuf(KM_NOSLEEP);
        init_tid = mps_get_tid((long)lci_rb_chan);
        DEBUGP(DEB_CALL,(CE_CONT, "lciinit 3: got trans id; tid=%x\n",init_tid));
        socket_id = ((ulong)enet_cfg[i].slot_id << 16);
        socket_id |= enet_cfg[i].lcis_rb_port;    
        init_data.opcode = LCI_INIT_OPCODE; /* 31416 */
        init_data.hostid = ics_myslotid();
        init_data.rbport = LCI_RB_PORT;
        init_data.txport = LCI_XMIT_PORT;
        init_data.rxport = LCI_RCV_PORT;
        init_data.delport = 0;
        init_data.maxbufs = 1;
        init_data.maxrxsz = 0xffff;
        DEBUGP(DEB_CALL,(CE_CONT, "lciinit 4: about to make unsol msg,socket_id=%x\n",socket_id));
        mps_mk_unsol (mbp,socket_id,(uchar)init_tid,
				(unsigned char *)&init_data,sizeof(init_data));
        DEBUGP(DEB_CALL,(CE_CONT, "lciinit(): About to send init_msg. Socket id=%x\n",socket_id));
        if (mps_AMPsend_rsvp((long)lci_rb_chan,mbp,(struct dma_buf *)NULL,(struct dma_buf *)NULL) == -1)
            {
            /* log init error */
            DEBUGP(DEB_CALL,(CE_CONT, "lciinit(): Error response from mps_AMPsend_rsvp()\n"));
            g_lci_error = E_LCI_INIT;
            mps_free_msgbuf(mbp);
            ret = mps_free_tid((long)lci_rb_chan,(unchar)init_tid);
	    init_tid = mps_max_tran;
            }
        enet_boards[i].state |= INIT_PENDING;
        DEBUGP(DEB_CALL,(CE_CONT, "lciinit(): Returned from init_msg. Waiting for response\n"));
        timeout(lci_reinit, (caddr_t)NULL, 500);
        DEBUGP(DEB_CALL,(CE_CONT, "lciinit(): After timeout().\n"));
    }
}

/*
* mipsend 
*     Transmit Frame to enet via lci  
*
* INPUTS:
*     bnum: index in enet_cfg[] of the number of the enet board to which 
*       the frame is to be sent
*     port: the iNA port number to which the frame is to be directed. 
*     frame: pointer to a frame to be sent to the enet
*     length: length in bytes of frame
*
* OUTPUTS:
*     None.
*
* RETURNS:
*     TRUE:   if the frame was sent to the enet
*     FALSE:  if the frame could not be sent
*
* ASSUMPTIONS:
*
* Assumes caller completely filled out frame.
*
* Assumes frame lives in Kernel data-space.
*
*
*/
mipsend(bnum, port, frame, length)
char bnum;
ushort port;
char *frame;
ushort length;

{
    ulong           status;
    register        x;
    mps_msgbuf_t        *mbuf_p;
    struct dma_buf        *buf_p;
    erb_header_str  *lci_rb_hdr;  
    mblk_t          *mptr;
    rb_buf_req_str     *buf_req;
    uchar	    ta_present;  /* TRUE if trans address is a part of rb */
    register unchar	    ina_opcode;
    ulong	    ta_adr;
    unchar name_idx;

    DEBUGP(DEB_CALL,(CE_CONT, "mipsend(): Entered\n"));
    
    if((frame < (char *)enet_rb_list) || 
       (frame > (char *)&enet_rb_list[enet_rbtabsize])) {
        cmn_err(CE_WARN, "Ethernet Driver: mipsend - frame out of bounds %x\n", frame);
        return(FALSE);
    }
	/*
	 * save original address of RB
	 */
	((struct req_blk *)frame)->rb = (caddr_t)frame;	/* I001 */

    /*
     * Note: must mutex against usage by interrupts while using shared
     * rqe structure.
     */
    x = SPL();

    if ((mptr = allocb(sizeof(erb_header_str),BPRI_HI)) == (mblk_t *)NULL) 
	{
        cmn_err(CE_WARN, "Ethernet Driver: mipsend - no buffer for rb; allocb failed");
	return (FALSE);
        }
    /* create the extended rb header */

    lci_rb_hdr = (erb_header_str *)(mptr->b_datap->db_base);
    lci_rb_hdr->rb_size = length+sizeof(erb_header_str);
    lci_rb_hdr->csd_size = 0;
    lci_rb_hdr->rb_ina_port = port;
    lci_rb_hdr->rb_adr = (ulong)kvtophys((caddr_t)frame);

    name_idx = 0xff;
    ina_opcode = ((crbh *)frame)->c_opcode;
    ta_present = FALSE;
    ta_adr = 0;

    DEBUGP(DEB_FULL,(CE_CONT, "mipsend: rb_ptr=%x, ina opcode= %x\n",frame,ina_opcode));
    /* monitor(); */

#ifdef NS
  if (((crbh *)frame)->c_subsys == SUB_NS)
  {
 		if ((name_idx =
 			register_RB((struct _crbh *) frame)) == 0xff)
 				cmn_err(CE_WARN, "Ethernet Driver: Unable to register name\n");
  }
  else if (((crbh *)frame)->c_subsys == SUB_NMF)
  {
 		if ((name_idx =
 			register_RB((struct _crbh *) frame)) == 0xff)
 				cmn_err(CE_WARN, "Ethernet Driver: Unable to register name\n");
  }
  else
  {
#endif
    switch(ina_opcode) {
         case OP_OPEN:
            break;

         case OP_ACR:
         case OP_SCR:
         case OP_ACRU:
	    ta_present = TRUE;
            ta_adr = *((ulong *)(((struct crrb *)frame)->cr_tabufp));
		/* I005 Start */
		if (((struct crrb *) frame) -> cr_u_len != 0)
 		{
 			if ((name_idx = register_RB((struct _crbh *)frame))
					== (unchar)0xff)
 				cmn_err(CE_WARN, "Ethernet Driver: Unable to register name\n");
 		}
		/* I005 End */
	    break;

         case OP_RD:
         case OP_SD:
         case OP_EOM_SD:
         case OP_EX_SD:
         case OP_EX_RD:
         case OP_CLOSE:					/* I005 */
         case OP_ACLOSE:				/* I005 */
 		if (((struct vcrb *) frame) -> vc_nblks != 0)
 		{
 			if ((name_idx = register_RB((struct _crbh *)frame))
					== (unchar)0xff)
 				cmn_err(CE_WARN, "Ethernet Driver: Unable to register name\n");
 		}
	    break;

         case OP_SDGM:
         case OP_RDGM:
         case OP_WDGM:
	    ta_present = TRUE;
		ta_adr = *((ulong *)(((struct drb *)frame)->dr_tabufp));
 		if (((struct drb *) frame) -> dr_nblks != 0)
 		{
 			if ((name_idx = register_RB((struct _crbh *)frame))
					== (unchar)0xff)
 				cmn_err(CE_WARN, "Ethernet Driver: Unable to register name\n");
 		}
	    break;

#ifdef EDL
         case OP_EDL_CONN:
         case OP_EDL_DISC:
	    break;
         case OP_EDL_RAWRECV:
 		if ((name_idx =
 			register_RB((struct postrb *) frame)) == 0xff)
 				cmn_err(CE_WARN, "Ethernet Driver: Unable to register name\n");
	    break;
         case OP_EDL_RAWTRAN:
	        ta_present = TRUE;
		ta_adr =
		     *((ulong *)(((struct tranrb *)frame)->tr_dst_addr_ptr));
 		if ((name_idx = 
 			register_RB((struct tranrb *) frame)) == 0xff)
 				cmn_err(CE_WARN, "Ethernet Driver: Unable to register name\n");
            break;
#endif

         default:
         /* 
         case OP_ACRT:
         case OP_WRB:
         case OP_WEB:
         case OP_STAT:
         case OP_DEFSTAT:
         case OP_WDB:
         case OP_NMF_RO:
         case OP_NMF_RACO:
         case OP_NMF_SO:
         case OP_NMF_RMEM:
         case OP_NMF_SMEM:
         case OP_NMF_DUMP:
         case OP_NMF_ECHO:
         case OP_NMF_FLOAD:
         case OP_NMF_SUPPLYB:
         case OP_NMF_TAKEB:
         */
            cmn_err(CE_WARN, "Ethernet Driver: Unexpected request block from TLI (%x) \n",ina_opcode);
            relrb((struct req_blk *)frame);
            break;
    }
#ifdef NS
  }
#endif
    /*
     * Create the data buffer linked chain to be transmitted.
     */
    /*DEBUGP(DEB_FULL,(CE_CONT, "mipsend: Afer case stmt; ta_present=%x\n",ta_present)); */
    if (ta_present == TRUE)
       {
       buf_p = mps_get_dmabuf(4,DMA_NOSLEEP);
       lci_rb_hdr->ta_size = sizeof(tabuf); 
       }
    else
       {
       buf_p = mps_get_dmabuf(3, DMA_NOSLEEP);
       lci_rb_hdr->ta_size = 0; 
       }

    /* DEBUGP(DEB_FULL,(CE_CONT, "mipsend: Got dbuf;dbuf_p=%x\n",buf_p)); */
    buf_p->count = sizeof(erb_header_str);
    buf_p->address = (ulong)kvtophys((caddr_t)lci_rb_hdr);
    (buf_p->next_buf)->count = length;
    (buf_p->next_buf)->address = (ulong)kvtophys((caddr_t)frame);

    if (ta_present == TRUE) 
	{
        /* DEBUGP(DEB_FULL,(CE_CONT, "mipsend: ta_present: Forming buf_p, ta_adr=%x\n",ta_adr));  */
        ((buf_p->next_buf)->next_buf)->count = sizeof(tabuf);
        ((buf_p->next_buf)->next_buf)->address = ta_adr;
        (((buf_p->next_buf)->next_buf)->next_buf)->next_buf = (struct dma_buf *)NULL;
        (((buf_p->next_buf)->next_buf)->next_buf)->count = 0;
        (((buf_p->next_buf)->next_buf)->next_buf)->address = NULL;
        /* DEBUGP(DEB_FULL,(CE_CONT, "mipsend: ta_present: buf_p=%x\n",buf_p)); */
	}
    else
	{
        /* DEBUGP(DEB_FULL,(CE_CONT, "mipsend: ta_not_present: Forming buf_p\n")); */
        ((buf_p->next_buf)->next_buf)->next_buf = (struct dma_buf *)NULL;
        ((buf_p->next_buf)->next_buf)->count = 0;
        ((buf_p->next_buf)->next_buf)->address = NULL;
	}
/*************************
      DEBUGP(DEB_CALL,(CE_CONT, "mipsend(): data buffer chain created\n")); 
      DEBUGP(DEB_CALL,(CE_CONT, "mipsend(): erb_adr=%x; erb_len=%x; rb_adr=%x; rb_len=%x
        \n",buf_p->address,buf_p->count,(buf_p->next_buf)->address,
        (buf_p->next_buf)->count));
    DEBUGP(DEB_CALL,(CE_CONT, "mipsend(): lci_rb_hdr->rbsize=%x,tasize=%x,csdsize=%x,
        rbport=%x\n",lci_rb_hdr->rb_size, lci_rb_hdr->ta_size,lci_rb_hdr->
	csd_size,lci_rb_hdr->rb_ina_port));

    DEBUGP(DEB_CALL,(CE_CONT, "mipsend(): board num to send rb to =%x\n",enet_cfg[bnum].slot_id));
    DEBUGP(DEB_CALL,(CE_CONT, "mipsend():src portid=%x;dest portid=%x;ina_opcode=%x\n",LCI_RB_PORT,LCI_SERVER_PORT,((crbh *)frame)->c_opcode));
****************/
    /*
     * Create a buffer request message.
    */
    mbuf_p = mps_get_msgbuf(KM_NOSLEEP);
    mbuf_p->mb_bind = (unsigned long) mptr;
    buf_req = (rb_buf_req_str *) (mbuf_p->mb_data);
    buf_req->dest_adr = enet_cfg[bnum].slot_id;   /* dest_adr = dest host id */
    buf_req->src_adr = ics_myslotid();

    buf_req->msg_type = MPS_MG_BREQ;
    buf_req->msg_len1 = (length + sizeof(erb_header_str) +lci_rb_hdr->ta_size) & 0xff;
    buf_req->msg_len2 = ((length + sizeof(erb_header_str) +lci_rb_hdr->ta_size) >> 8) & 0xff;
    buf_req->msg_len3 = '\0';
    buf_req->req_id = 0x80;
    buf_req->hw_preserved = 0;
    buf_req->op_code = LCI_RBSEND_OPCODE;
    buf_req->rb_size = length + sizeof(erb_header_str);
    buf_req->ta_size = lci_rb_hdr->ta_size; 
    buf_req->csd_size = 0;
    buf_req->ina_port = port; 
    buf_req->name_idx = name_idx; 
    buf_req->proto_id = 0;
    buf_req->xmission_ctl = 0;
    buf_req->src_port_id = LCI_RB_PORT;
    buf_req->dest_port_id = LCI_SERVER_PORT;
    buf_req->trans_id = 0;

    mps_mk_sol(mbuf_p,((enet_cfg[bnum].slot_id << 16) | LCI_SERVER_PORT),0,
				(unsigned char *)&(buf_req->op_code),16);
    /* transmit the frame to the enet */
/*****
    DEBUGP(DEB_CALL,(CE_CONT, "mipsend(): About to send rb to enet. mbuf_p=%x; buf_p=%x\n",mbuf_p,buf_p));
*****/
    status = mps_AMPsend_data((long)lci_rb_chan,mbuf_p,buf_p);
    if (status != -1)
    {
        DEBUGP(DEB_CALL,(CE_CONT, "mipsend(): rb sent to enet; ok return.\n"));
        status = TRUE;
    }
    else
    {
        DEBUGP(DEB_CALL,(CE_CONT, "mipsend(): mps_AMPsend_data ERROR. rb not sent.\n"));
        mps_free_msgbuf(mbuf_p);
	mps_free_dmabuf(buf_p);
	status = FALSE;
    }

    /* monitor(); */
    splx(x);
    return (status);
}

