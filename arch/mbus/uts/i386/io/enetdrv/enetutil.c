/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1985, 1986, 1987, 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/io/enetdrv/enetutil.c	1.3.1.1"

/*
 *   enetutil.c: generic subroutines for enet driver.
 */

/*	
 *   Modification log
 *
 *	I000	7/17/87		DF 	Intel ISO
 *		Add a case in ioctl_check() to return enetinfo.
 *	I001	8/14/87		DF	Intel ISO
 *		Added a case in ioctl_check() to handle enet full reset.
 *	I002	8/27/87		DF	Intel ISO
 *		Added changes to correct a problem with enet_full_reset
 *		which did not release the buffer posted.
 *	I003	9/22/87		DF	Intel ISO
 *		Added code to handle situation when the driver runs out
 *		of request blocks.
 *	I004	9/22/87		DF	Intel ISO
 *		Moved RBTABSIZE, PENDTABSIZE, enet_rb_list, enet_pend_list
 *		to space.c.
 *	I005	6/28/88		rjs		Intel OMSO
 *		Removed check to allowing endpoint to support both Connection
 *		and Connectionless (i.e. datagram) modes.
 *	I006	11/23/88	rjs		Intel OMSO
 *		Modified enet_make_addr algorithm allowing hardware dependent
 *		portion of driver to determine base TSAP for building
 *		addresses.
 *	I007	07/11/89	rjf		Intel
 *		Made lint fixes.
 */

#define DEBUG 1
#include "sys/enet.h"

/*
 * The following are major variables defined in the /etc/master.d
 * file for enet:
 *
 *	enet_endpoints: One for each STREAM
 *	enet_n_endpoints: Max number of endpoints the driver can handle
 */
extern endpoint		enet_endpoints[];
extern int		enet_n_endpoints;
extern struct enetboard	enet_boards[];
extern struct enetinf	enet_inform[];     /* I000 */
extern int		enet_n_boards;
extern int		enet_rbtabsize, enet_pendtabsize; 	/* I004 */
extern unsigned short	enet_base_tsap;	/* I006 */
/* 
 * enet statistics structure defined in enet.c
 */
extern ulong		enet_stat[enet_SCNT];

extern	int		enet_rb_hiwat;		/* I003 */
extern	int		enet_rb_lowat;		/* I003 */

int			enet_rbs_used;		/* I003 */
int			enet_rb_hi = 0;		/* I003 */

int			enet_resetting = 0; /* set in the ioctl enetfull_reset*/

/*
 * TPI state transition matrix, defined in the STREAMS system
 * module timod
 */
extern char ti_statetbl[TE_NOEVENTS][TS_NOSTATES];

/*
 * Local buffer queue data structures
 */
int take_rb, give_rb, take_pend, give_pend;

extern	struct req_blk	*enet_rb_tab[];
extern	pend_list	*enet_pend_tab[];
extern	struct req_blk	enet_rb_list[];
extern	pend_list	enet_pend_list[];

/*
 * enet debug level defined in enet.c
 */
extern int		enet_debug;

/*
 * discon_ind
 *
 * Generate a T_DISCON_IND message to the local user.
 *
 * This function returns TRUE if the message was written, and FALSE
 * otherwise.
 *
 * The message will only be generated if the state of the virtual
 * curcuit would permit it.
 *
 * The state of the virtual endpoint is updated to reflect the message
 * if it is generted.
 */
int
discon_ind(ep, reason, seqnbr)
endpoint *ep;
int reason, seqnbr;
{
	int nextstate, i;
	register mblk_t *mptr;
	struct T_discon_ind *dis;

	DEBUGP(DEB_CALL,(CE_CONT, "discon_ind()\n"));
	i = (ep->nbr_pend == 0) ? TE_DISCON_IND1
			       : ((ep->nbr_pend == 1) ? TE_DISCON_IND2
						     : TE_DISCON_IND3);
	if((nextstate = ti_statetbl[i][ep->tli_state]) == TS_INVALID)
	{
		DEBUGP(3,(CE_CONT, "discon_ind(): TS_INVALID\n"));
		return(FALSE);
	}			
	/*
	 * A T_DISCON_IND to the local user is valid.
	 */
	mptr = getmptr(sizeof(struct T_discon_ind), BPRI_HI, ep);
	if(mptr == NULL) {
		DEBUGP(3,(CE_CONT, "discon_ind(): getmptr failed\n"));	
		return(FALSE);
	}
	mptr->b_datap->db_type = M_PROTO;
	mptr->b_wptr = mptr->b_rptr + sizeof(struct T_discon_ind);
	dis = (struct T_discon_ind *)mptr->b_rptr;
	dis->PRIM_type	   = T_DISCON_IND;
	dis->DISCON_reason = reason;
	dis->SEQ_number	   = seqnbr;
	/*
	 * If necessary, generate a M_FLUSH.
	 */
	if(ep->tli_state == TS_DATA_XFER)
		enet_send_flush(ep);
	putnext(ep->rd_q, mptr);
	ep->tli_state = nextstate;
	DEBUGP(DEB_CALL,(CE_CONT, "discon_ind => TRUE\n"));
	return(TRUE);
}

/*
 * enet_check_opts
 *
 * Check the supplied options, returning T_SUCCESS or T_FAILURE,
 * depending on whether or not they are all valid.
 *
 * If "optaddr" is not NULL, a valid set of options, based on those
 * given by "options" and biased towards the enet defaults, will be
 * returned using this pointer.
 */
opts
enet_check_opts(options, optaddr)
register opts options;
opts *optaddr;
{
	register ushort result;

	DEBUGP(DEB_CALL,(CE_CONT, "enet_check_opts()\n"));
	result = T_SUCCESS;

	/*					 I005
	 * if(options & OPT_COTS) {
	 *	if(options & OPT_CLTS) {
	 *		result = T_FAILURE;
	 *		options &= ~OPT_CLTS;
	 *	}
	 * }
	 */

/* This is from the body of this routine from urp
 * Something along these lines is needed if/when any real mode options are
 * setup
	if( options & U_CHAR_MODE ) {
		if( options & U_BLOCK_MODE ) {
			result = T_FAILURE;
			options &= ~U_BLOCK_MODE;
			}
		if( options & OPT_CLTS ) {
			result = T_FAILURE;
			options &= ~U_CHAR_MODE;
			}
		}
	else if( options & U_BLOCK_MODE ) {
		if( options & OPT_CLTS ) {
			result = T_FAILURE;
			options &= ~U_BLOCK_MODE;
			}
		}
	else if( ! (options & OPT_CLTS) )
		options |= U_CHAR_MODE;
*/
/* This is also from urp.  It seems redundant.  If the user doesn't want
 * a flushing disconnect, let him not set the bit
	if( options & U_F_DISCON ) {
		if( options & U_D_DISCON ) {
			result = T_FAILURE;
			options &= ~U_D_DISCON;
			}
		}
	else if( ! (options & U_D_DISCON) )
		options |= U_F_DISCON;
	if( options & U_NONU_ADVER ) {
		if( options & U_U_ADVER ) {
			result = T_FAILURE;
			options &= ~U_U_ADVER;
			}
		}
	else if( ! (options & U_U_ADVER) )
		options |= U_NONU_ADVER;
*/
	if(optaddr != NULL)
		*optaddr = options;
	DEBUGP(DEB_CALL,(CE_CONT, "enet_check_opts => %x\n", result));
	return(result);
}

/*
 * enet_set_opts
 *
 * Set the options on a given endpoint
 */
void
enet_set_opts(ep, options)
register endpoint *ep;
opts options;
{
	DEBUGP(DEB_CALL,(CE_CONT, "enet_set_opts()\n"));
	/* NOTE: This can only happen in T_IDLE state, so if the user wants
	 * to choose a service type, that's fine with us.  He just has to
	 * live with the choice later
	 */
	ep->options = options;
	DEBUGP(DEB_CALL,(CE_CONT, "enet_set_opts => NULL\n"));
}

/*
 * enet_pferr
 *
 * A fatal error has occurred on virtual endpoint "ep".
 */
void
enet_pferr(ep)
register endpoint *ep;
{
	mblk_t *mptr;

	DEBUGP(DEB_CALL,(CE_CONT, "enet_pferr()\n"));
	DEBUGP(DEB_STOP,(CE_CONT, "enet_pferr: STOP\n"));
	DEBUGP(DEB_STOP,(CE_CONT, "ep = %x\n", ep));
	if(enet_debug >= DEB_STOP) {
		asm("int $1");
	}
	DEBUGC('f');
	/*
	 * Watch out for "rolling" and "recursive" errors: Only do this once.
	 */
	if(ep->str_state & C_ERROR)
		return;
	ep->str_state |= C_ERROR;
	/*
	 * Take down a connection:
	 *
	 *	1. Notify all remote endpoints that this endpoint is down.
	 *		Note that a "flushing disconnect" is used.
	 *	2. Notify the local user that this endpoint is down. Either:
	 *		a. Generate a T_DISCON_IND, if such a message would be
	 *			legal.
	 *		b. Otherwise generate a M_ERROR of type EPROTO.
	 */
	enet_abort(ep);
	if(discon_ind(ep, EPROTO, -1)) {
		/*
		 * A T_DISCON_IND is valid.
		 */
		return;
	}
	/*
	 * Can only report the error to the local user through a M_ERROR
	 * message.
	 */
	if((mptr = allocb(1, BPRI_HI)) == NULL) {
		enet_stat[ST_ALFA]++;
		return;
	}
	mptr->b_datap->db_type  = M_ERROR;
	mptr->b_wptr = mptr->b_rptr + sizeof(char);
	/*
	 * There is only one fatal TPI message.
	 */
	*mptr->b_rptr = EPROTO;
	putnext(ep->rd_q, mptr);
	DEBUGP(DEB_CALL,(CE_CONT, "enet_pferr => NULL\n"));
}

static char
enet_ntoa(i)
unsigned char i;
{
	if ((i>=0) && (i<=9))
		return(i+0x30);
	else
		return(i+0x57);
}

void
enet_print_addr(eaddr)
unsigned char eaddr[];
{
	int i;
	char a_eaddr[18];

	for (i=0; i<6; i++)
	{
		a_eaddr[i*3]   = enet_ntoa(eaddr[i]>>4&0xf);
		a_eaddr[i*3+1] = enet_ntoa(eaddr[i]&0xf);
		a_eaddr[i*3+2] = ':';
	}
	a_eaddr[17] = '\0';
	cmn_err(CE_CONT, "--- Ethernet Address: %s\n", a_eaddr);
}

void
init_std_addr(ep, addr)
endpoint *ep;
char *addr;
{
	struct enetboard *board_p;

	T_NA_LEN(addr) = 10;
	TS_NA_AFI(addr) = 0x49;
	TS_NA_SUBNETNO(addr) = 0;
	board_p = &enet_boards[ep->bnum];
	TS_NA_SUBNETADDR(addr)[0] = board_p->eaddr[0] & 0377;
	TS_NA_SUBNETADDR(addr)[1] = board_p->eaddr[1] & 0377;
	TS_NA_SUBNETADDR(addr)[2] = board_p->eaddr[2] & 0377;
	TS_NA_SUBNETADDR(addr)[3] = board_p->eaddr[3] & 0377;
	TS_NA_SUBNETADDR(addr)[4] = board_p->eaddr[4] & 0377;
	TS_NA_SUBNETADDR(addr)[5] = board_p->eaddr[5] & 0377;
	T_TSAP_LEN(addr) = 2;
	T_TSAP(addr) = 0;
}

/*
 * enet_make_addr		I006
 *
 * A new address needs to be provided to the user.  Use the minor device
 * number (must be unique) OR'd with "enet_base_tsap", a hardware-dependent
 * ushort variable.  The value of enet_base_tsap must be chosen such that crafted
 * TSAPs don't collide with actual TSAPs bound by users to endpoints.
 *
 * The length of the address is fixed at 2 bytes
 *
 * Return the new T_BIND_REQ message upon success. Return NULL on any
 * type of failure.
 */
mblk_t *
enet_make_addr(ep, mptr, bind)
endpoint *ep;
register mblk_t *mptr;
register struct T_bind_req *bind;
{
	DEBUGP(DEB_CALL,(CE_CONT, "enet_make_addr()\n"));
	if((mptr->b_rptr+sizeof(struct T_bind_req)+STD_NET_ADDR_LEN)
			> mptr->b_datap->db_lim) {
		ulong tmp;
		/*
		 * Need some more space: Allocate a new block.
		 */
		tmp = bind->CONIND_number;
		freemsg(mptr);
		mptr = getmptr(sizeof(struct T_bind_req)+STD_NET_ADDR_LEN,
				BPRI_MED,ep);
		if(mptr == NULL)
			return(NULL);
		mptr->b_datap->db_type = M_PROTO;
		bind = (struct T_bind_req *)mptr->b_rptr;
		bind->PRIM_type     = T_BIND_REQ;
		bind->CONIND_number = tmp;
	}
	bind->ADDR_offset = sizeof(struct T_bind_req);
	/*
	 * Find the minor device number
	 */
	{
	register int i;

	init_std_addr(ep, (char *)bind+bind->ADDR_offset);
	for(i = 0; i < enet_n_endpoints; i++)
		if(ep == &enet_endpoints[i])
			break;
	T_TSAP((char *)bind + bind->ADDR_offset) = enet_base_tsap + i;
	bind->ADDR_length = STD_NET_ADDR_LEN;
	mptr->b_wptr = (unchar *)bind+sizeof(struct T_bind_req)+STD_NET_ADDR_LEN;
	DEBUGP(DEB_CALL,(CE_CONT, "enet_make_addr => %x, %x, %x\n",
			mptr, i, T_TSAP((char *)bind + bind->ADDR_offset)));
	}
	return(mptr);
}

/*
 * enet_pterr
 *
 * Issue an error message for a non-fatal TLI error condition.
 *
 * "mptr" is the transport user TPI message that was being processed.
 */
void
enet_pterr(ep, mptr, tli_err, unx_err)
endpoint *ep;
register mblk_t *mptr;
int tli_err, unx_err;
{
	long tli_type;
	register struct T_error_ack *ack;

	DEBUGP(DEB_CALL,(CE_CONT, "enet_pterr()\n"));
	DEBUGP(DEB_CALL,(CE_CONT, "ep = %x, tli_err = %x, unx_err = %d\n", 
			ep, tli_err, unx_err));
	DEBUGP(DEB_STOP,(CE_CONT, "enet_pterr: STOP\n"));
	if(enet_debug >= DEB_STOP) {
		asm("int $1");
	}
	tli_type = ((union T_primitives *)mptr->b_rptr)->type;
	/*
	 * See if the original message is large enough to be used to construct a
	 * T_ERROR_ACK message.
	 */
	if((mptr->b_datap->db_lim - mptr->b_datap->db_base) < 
	     sizeof(struct T_error_ack)) {
		/*
		 * The original message is too small: Get a new message
		 * and free up the original.
		 */
		freemsg(mptr);
		mptr = getmptr(sizeof(struct T_error_ack), BPRI_HI, ep);
		if(mptr == NULL)
			return;
	}
	/*
	 * Construct the T_ERROR_ACK message.
	 */
	mptr->b_datap->db_type  = (tli_err&HIGH_PRI)?M_PCPROTO:M_PROTO;
	DEBUGP(DEB_FULL,(CE_CONT, "enet_pterr: error message of type %x\n",
			mptr->b_datap->db_type));
	mptr->b_rptr  = mptr->b_datap->db_base;
	mptr->b_wptr = mptr->b_rptr + sizeof(struct T_error_ack);
	ack = (struct T_error_ack *)mptr->b_rptr;
	ack->PRIM_type  = T_ERROR_ACK;
	ack->ERROR_prim = tli_type;
	ack->TLI_error  = tli_err&~HIGH_PRI;
	ack->UNIX_error = unx_err;
	if(mptr->b_cont) {
		freemsg(mptr->b_cont);
		mptr->b_cont = 0;
	}
	putnext(ep->rd_q, mptr);
	DEBUGP(DEB_CALL,(CE_CONT, "enet_pterr => NULL\n"));
}

/*
 * enet_send_flush
 *
 * Issue a M_FLUSH, to flush both the read and writer queues.
 */
void
enet_send_flush(ep)
register endpoint *ep;
{
	register mblk_t *mptr;

	DEBUGP(DEB_CALL,(CE_CONT, "enet_send_flush()\n"));
	if((mptr = allocb(1, BPRI_HI)) == NULL) {
		enet_stat[ST_ALFA]++;
		return;
	}
	mptr->b_datap->db_type = M_FLUSH;
	*mptr->b_rptr = FLUSHRW;
	mptr->b_wptr++;
	putnext(ep->rd_q, mptr);
	DEBUGP(DEB_CALL,(CE_CONT, "enet_send_flush => NULL\n"));
}

/*
 * enet_ok_ack
 *
 * Issue a T_OK_ACK message back to the transport user, to 
 * acknowledge succesfull processing of a TPI primitive.
 *
 * "mptr" is the transport user TPI message that was being processed.
 * "flag" is 1 if this is to be a PCPROTO message
 */
void
enet_ok_ack(ep, mptr, flag)
endpoint *ep;
register mblk_t *mptr;
int flag;
{
	long tli_type;
	register struct T_ok_ack *ack;

	DEBUGP(DEB_CALL,(CE_CONT, "enet_ok_ack()\n"));
	DEBUGP(DEB_FULL,(CE_CONT, "enet_ok_ack: ep = %x, mptr = %x\n", ep, mptr));
	tli_type = ((union T_primitives *)mptr->b_rptr)->type;
	/*
	 * See if the original message is large enough to be used to construct
	 * the T_OK_ACK message.
	 */
	if((mptr->b_datap->db_lim - mptr->b_datap->db_base) < 
	      sizeof(struct T_ok_ack)) {
		/*
		 * The original message is too small: Get a new message
		 * and free up the original.
		 */
		freemsg(mptr);
		mptr = getmptr(sizeof(struct T_ok_ack), BPRI_HI, ep);
		if(mptr == NULL)
			return;
	}
	else
		/*
		 * Only the first block of the original message is
		 * needed.
		 */
		freemsg(unlinkb(mptr));
	/*
	 * Construct the T_OK_ACK message.
	 */
	mptr->b_datap->db_type  = flag?M_PCPROTO:M_PROTO;
	mptr->b_rptr  = mptr->b_datap->db_base;
	mptr->b_wptr = mptr->b_rptr + sizeof(struct T_ok_ack);
	ack = (struct T_ok_ack *)mptr->b_rptr;
	ack->PRIM_type    = T_OK_ACK;
	ack->CORRECT_prim = tli_type;
	putnext(ep->rd_q, mptr);
	DEBUGP(DEB_CALL,(CE_CONT, "enet_ok_ack => NULL\n"));
}

/*
 * enet_bind_ack
 *
 * Issue a T_BIND_ACK message back to the transport user, to 
 * acknowledge succesfull processing of a TPI primitive.
 *
 * "mptr" is the transport user TPI message that was being processed.
 */
void
enet_bind_ack(ep, mptr)
endpoint *ep;
register mblk_t *mptr;
{
	register struct T_bind_ack *ack;
	char *tabufp;

	DEBUGP(DEB_CALL,(CE_CONT, "enet_bind_ack()\n"));
	/*
	 * Construct the T_BIND_ACK message.
	 */
	mptr->b_rptr  = mptr->b_datap->db_base;
	ack = (struct T_bind_ack *)mptr->b_rptr;
#ifdef CHANGE_RET_ADDR
	if((mptr->b_datap->db_lim - mptr->b_datap->db_base) < 
			sizeof(struct T_bind_ack) + STD_NET_ADDR_LEN) {
		/*
		 * The buffer isn't big enough, get another one
		 */
		freemsg(mptr);
		mptr = getmptr(sizeof(struct T_bind_ack)+STD_NET_ADDR_LEN,
				BPRI_MED,
				ep);
		if(mptr == NULL)
			return;
		mptr->b_rptr  = mptr->b_datap->db_base;
		ack = (struct T_bind_ack *)mptr->b_rptr;
	}
#endif
	mptr->b_datap->db_type  = M_PROTO;
	ack->PRIM_type    = T_BIND_ACK;
	ack->ADDR_offset  = sizeof(struct T_bind_ack);
#ifdef CHANGE_RET_ADDR
	tabufp = (char *)ack+ack->ADDR_offset;
	init_std_addr(ep, tabufp);
	T_TSAP(tabufp) = ep->tsap;
	ack->ADDR_length = STD_NET_ADDR_LEN;
	mptr->b_wptr = mptr->b_rptr + sizeof(struct T_bind_ack) + STD_NET_ADDR_LEN;
#endif
	putnext(ep->rd_q, mptr);
	DEBUGP(DEB_CALL,(CE_CONT, "enet_bind_ack => NULL\n"));
}

/*
 * enet_conn_con
 *
 * Issue a T_CONN_CON message back to the transport user, to 
 * acknowledge succesfull processing of a TPI primitive.
 *
 * "mptr" is the transport user TPI message that was being processed.
 */
void
enet_conn_con_ack(ep, mptr, tabufp)
endpoint *ep;
register mblk_t *mptr;
char *tabufp;
{
	register struct T_conn_con *conn;
	int	addr_length;

	DEBUGP(DEB_CALL,(CE_CONT, "enet_conn_con_ack()\n"));
	mptr = growmptr(sizeof(struct T_conn_con) + MAX_NAME_LEN, 
			BPRI_MED, ep, mptr);
	if(mptr == NULL) {
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_conn_con_ack: alloc1 failed\n"));
		DEBUGP(DEB_CALL,(CE_CONT, "enet_conn_con_ack => NULL\n"));
		return;
	}
	mptr->b_datap->db_type = M_PROTO;
	addr_length = ADDR_LENGTH(REM_ADDR(tabufp));
	conn = (struct T_conn_con *)mptr->b_rptr;
	conn->PRIM_type  = T_CONN_CON;
	conn->RES_length = addr_length;
	conn->RES_offset = sizeof(struct T_conn_con);
	conn->OPT_length = 0;
	conn->OPT_offset = 0;
	memcpy((char *)mptr->b_rptr+sizeof(struct T_conn_con),
	    REM_ADDR(tabufp),
	    addr_length);
	mptr->b_wptr=mptr->b_rptr + sizeof(struct T_conn_con)
			+ addr_length + COPT_LEN;
	putnext(ep->rd_q, mptr);
	DEBUGP(DEB_CALL,(CE_CONT, "enet_conn_con_ack => NULL\n"));
}

void
enet_info_ack(ep, mptr, tsdu_size, etsdu_size, cdata_size, ddata_size, addr_size, opt_size, tidu_size, tli_state, serv_type)
endpoint *ep;
mblk_t *mptr;
long tsdu_size, etsdu_size, cdata_size, ddata_size, addr_size;
long opt_size, tidu_size, tli_state, serv_type;
{
	struct T_info_ack *ack;

	mptr = growmptr(sizeof(struct T_info_ack), BPRI_MED, ep, mptr);
	if(mptr == NULL)
		return;
	/*
	 * Build a T_INFO_ACK message.
	 */
	mptr->b_datap->db_type  = M_PROTO;
	mptr->b_rptr  = mptr->b_datap->db_base;
	mptr->b_wptr = mptr->b_rptr + sizeof(struct T_info_ack);
	ack = (struct T_info_ack *)mptr->b_rptr;
	ack->PRIM_type     = T_INFO_ACK;
	ack->TSDU_size     = tsdu_size;
	ack->ETSDU_size    = etsdu_size;
	ack->CDATA_size    = cdata_size;
	ack->DDATA_size    = ddata_size;
	ack->ADDR_size     = addr_size;
	ack->OPT_size      = opt_size;
	ack->TIDU_size     = tidu_size;
	ack->CURRENT_state = tli_state;
	ack->SERV_type     = serv_type;
	putnext(ep->rd_q, mptr);
}

void
enet_optmgmt_ack(ep, mptr, flags, len, options)
endpoint *ep;
mblk_t *mptr;
int flags;
int len;
opts options;
{
	struct T_optmgmt_req *opt_req;

	/*
	 * Make sure there is enough space to return options to
	 * the user.
	 */
	mptr = growmptr((int)(sizeof(struct T_optmgmt_ack) + len), BPRI_MED,
			ep, mptr);
	if(mptr == NULL)
		return;
	opt_req = (struct T_optmgmt_req *)mptr->b_rptr;
	mptr->b_datap->db_type = M_PROTO;
	opt_req->PRIM_type  = T_OPTMGMT_ACK;
	opt_req->OPT_length = len;
	opt_req->OPT_offset = (len==0)?0:sizeof(struct T_optmgmt_ack);
	opt_req->MGMT_flags = flags;
	if(len)
		*(opts *)(mptr->b_rptr + sizeof(struct T_optmgmt_ack)) = 
			options;
	mptr->b_wptr = mptr->b_rptr + sizeof(struct T_optmgmt_ack) + len;
	putnext(ep->rd_q, mptr);
}

void
ioctl_check(ep, mptr, wr_q)
endpoint	*ep;
mblk_t		*mptr;
queue_t		*wr_q;
{
	struct iocblk *ioc;

	DEBUGP(DEB_CALL,(CE_CONT, "enet_ioctl_check()\n"));
	ioc = (struct iocblk *)mptr->b_rptr;
	switch(ioc->ioc_cmd) {
	case 0:
		DEBUGP(DEB_FULL,(CE_CONT, "enetwput: outb(%d) requested\n",
				*(char *)mptr->b_cont->b_rptr));
		/* First make sure this is the administrator entry */
		if(ep != &enet_endpoints[0]) {
			DEBUGP(DEB_ERROR,(CE_CONT, "ioctl_check: wrong ep\n"));
			mptr->b_datap->db_type = M_IOCNAK;
			ioc->ioc_count = 0;
			ioc->ioc_error = EACCES;
			qreply(wr_q, mptr);
			break;
		}
		if(!mptr->b_cont) {
			mptr->b_datap->db_type = M_IOCNAK;
			ioc->ioc_count = 0;
			ioc->ioc_error = EINVAL;
			qreply(wr_q, mptr);
			return;
		}
		outb((ushort)enet_boards[ep->bnum].port,
		     *(char *)mptr->b_cont->b_rptr);
		mptr->b_datap->db_type = M_IOCACK;
		ioc->ioc_count = 0;
		ioc->ioc_error = 0;
		qreply(wr_q, mptr);
		break;
	case 1:
		DEBUGP(DEB_FULL,(CE_CONT, "enetwput: stats requested\n"));
		if(!mptr->b_cont) {
			mptr->b_datap->db_type = M_IOCNAK;
			ioc->ioc_count = 0;
			ioc->ioc_error = EINVAL;
			qreply(wr_q, mptr);
			return;
		}
		if((mptr->b_cont->b_datap->db_lim
			  - mptr->b_cont->b_datap->db_base)
			 < sizeof(enet_stat)) {
			mptr->b_datap->db_type = M_IOCNAK;
			ioc->ioc_count = 0;
			ioc->ioc_error = EINVAL;
			qreply(wr_q, mptr);
			return;
		}
		mptr->b_datap->db_type = M_IOCACK;
		mptr->b_cont->b_rptr = mptr->b_cont->b_datap->db_base;
		mptr->b_cont->b_wptr = mptr->b_cont->b_rptr
					 + sizeof(enet_stat);
		memcpy((char *)mptr->b_cont->b_rptr,
			(char *)enet_stat,
			sizeof(enet_stat));
		ioc->ioc_count = sizeof(enet_stat);
		ioc->ioc_error = 0;
		qreply(wr_q, mptr);
		break;
	case 2:
		DEBUGP(DEB_FULL,(CE_CONT, "enetwput: set debug level to %d\n",
				*(int *)mptr->b_cont->b_rptr));
		if(!mptr->b_cont) {
			mptr->b_datap->db_type = M_IOCNAK;
			ioc->ioc_count = 0;
			ioc->ioc_error = EINVAL;
			qreply(wr_q, mptr);
			return;
		}
		enet_debug = 0;
		*(char *)&enet_debug = *mptr->b_cont->b_rptr;
		mptr->b_datap->db_type = M_IOCACK;
		ioc->ioc_count = 0;
		ioc->ioc_error = 0;
		qreply(wr_q, mptr);
		break;
	case 3:
		DEBUGP(DEB_FULL,(CE_CONT, "enetwput: reseting board\n"));
		/* First make sure this is the administrator entry */
		if(ep != &enet_endpoints[0]) {
			DEBUGP(DEB_ERROR,(CE_CONT, "ioctl_check: wrong ep\n"));
			mptr->b_datap->db_type = M_IOCNAK;
			ioc->ioc_count = 0;
			ioc->ioc_error = EACCES;
			qreply(wr_q, mptr);
			break;
		}
		enetreset(ep->bnum);
		mptr->b_datap->db_type = M_IOCACK;
		ioc->ioc_count = 0;
		ioc->ioc_error = 0;
		qreply(wr_q, mptr);
		break;
#if DEBUG
	case 4:
		DEBUGP(DEB_FULL,(CE_CONT, "enetwput: reseting debugtxt\n"));
		{ int i;
		for(i=0;i<10001;i++)
			debugtxt[i] = 0;
		debugptr = debugtxt;
		mptr->b_datap->db_type = M_IOCACK;
		ioc->ioc_count = 0;
		ioc->ioc_error = 0;
		qreply(wr_q, mptr);
		break;
		}
#endif
	case 5:			/* I000 */	

		DEBUGP(DEB_FULL,(CE_CONT, "enetwput: enetinfo requested\n"));   
		if(!mptr->b_cont) {
			mptr->b_datap->db_type = M_IOCNAK;
			ioc->ioc_count = 0;
			ioc->ioc_error = EINVAL;
			qreply(wr_q, mptr);
			return;
		}
		if((mptr->b_cont->b_datap->db_lim
			  - mptr->b_cont->b_datap->db_base)
			 < ((sizeof(struct enetinf)) * enet_n_boards)) {
			mptr->b_datap->db_type = M_IOCNAK;
			ioc->ioc_count = 0;
			ioc->ioc_error = EINVAL;
			qreply(wr_q, mptr);
			return;
		}
		mptr->b_datap->db_type = M_IOCACK;
		mptr->b_cont->b_rptr = mptr->b_cont->b_datap->db_base;
		mptr->b_cont->b_wptr = mptr->b_cont->b_rptr
				 + (sizeof(struct enetinf)) * enet_n_boards;
		memcpy((char *)mptr->b_cont->b_rptr,
			(char *)&enet_inform[0],
			(int)((sizeof(struct enetinf)) * enet_n_boards));
		ioc->ioc_count = (sizeof(struct enetinf)) * enet_n_boards;
		ioc->ioc_error = 0;
		qreply(wr_q, mptr);
		break;
						/* I001 */
	case 6:		
		DEBUGP(DEB_FULL,(CE_CONT, "enetwput: iNA/enet full reset\n"));
		enet_resetting = 1;
		enetfull_reset(ep->bnum);
		mptr->b_datap->db_type = M_IOCACK;
		ioc->ioc_count = 0;
		ioc->ioc_error = 0;
		qreply(wr_q, mptr);
		break;
	default:
		DEBUGP(DEB_ERROR,(CE_CONT, "enetwput: unknown ioctl request %x\n", ioc->ioc_cmd));
		/*
		 * No other type of ioctl should ever get here.
		 */
		mptr->b_datap->db_type = M_IOCNAK;
		ioc->ioc_count = 0;
		ioc->ioc_error = EACCES;
		qreply(wr_q, mptr);
		break;
	}
	DEBUGP(DEB_CALL,(CE_CONT, "enet_ioctl_check => NULL\n"));
}

/*
 * For the following routines, give_xxx==take_xxx implies an empty
 * buffer ring; give_xxx==xxxTABSIZE implies a full buffer ring.
 */
struct req_blk *
getrb(ep)
endpoint *ep;
{
	register	s;
	struct req_blk	*rb_p;

	s = SPL();
	/* if empty, we've got a problem */
	if(take_rb == give_rb) {
		cmn_err(CE_WARN, "Ethernet Driver: getrb - rb pool empty\n");
		splx(s);
		u.u_error = ENOSPC;
		enet_stat[ST_RBFA]++;
		enet_pferr(ep);
		DEBUGP(DEB_CALL,(CE_CONT, "getrb => NULL\n"));
		return(NULL);
	}
	if(give_rb == enet_rbtabsize)
		/* Was full, mark non-full */
		give_rb = take_rb;
	rb_p = enet_rb_tab[take_rb++];
	if(take_rb >= enet_rbtabsize)
		take_rb = 0;
	DEBUGP(DEB_CALL,(CE_CONT, "getrb => %x\n", rb_p));
	rb_p->in_use = 1; 				/* I002 */
	rb_p->databuf = 0;				/* I002 */
	enet_rbs_used++;					/* I003 */
	splx(s);
/*	cmn_err(CE_WARN, "getrb(): rbs used %d\n", enet_rbs_used); */
	if ((enet_rbs_used == enet_rb_hiwat) && !enet_rb_hi)	/* I003 */
	{
		enet_rb_hi = 1;
		DEBUGP(3,(CE_CONT, "rb hi water %d mark reached, rbs used %d.\n", 
		enet_rb_hiwat, enet_rbs_used));
	}
		
	return(rb_p);
}

void
relrb(rb_p)
struct req_blk	*rb_p;
{
	register	s;
	endpoint	*ep;

	DEBUGP(DEB_CALL,(CE_CONT, "relrb(%x)\n", rb_p));
	if(rb_p == NULL) {
		DEBUGP(DEB_CALL,(CE_CONT, "relrb => NULL\n"));
		return;
	}
	s = SPL();
	if((give_rb >= enet_rbtabsize) || 
	   (rb_p < enet_rb_list) ||
	   (rb_p > (struct req_blk *)&enet_rb_list[enet_rbtabsize])) {
		cmn_err(CE_WARN, "Ethernet Driver: release unallocated request block\n");
		splx(s);
		if(enet_debug >= DEB_STOP)
			asm("int $1");
		DEBUGP(DEB_CALL,(CE_CONT, "relrb => NULL\n"));
		return;
	}
	rb_p->in_use = 0;			/* I002 */
	enet_rb_tab[give_rb++] = rb_p;
	if(give_rb >= enet_rbtabsize)
		give_rb = 0;
	if(give_rb == take_rb)
		/* mark as full */
		give_rb = enet_rbtabsize;
	enet_rbs_used--;
	splx(s);
	if (enet_rb_hi && (enet_rbs_used <= enet_rb_lowat))
	{
		/* reenable all write queues */
		enet_rb_hi = 0;
		for (ep=&enet_endpoints[0];
				ep<&enet_endpoints[enet_n_endpoints]; ep++)
			if (ep->str_state != C_IDLE)
			{
				DEBUGP(3,(CE_CONT, "Low water mark %d reached.\n",enet_rb_lowat));
				qenable(WR(ep->rd_q));
			}		
	}			
/*	cmn_err(CE_NOTE, "rel_rb(): rb used %d\n", enet_rbs_used); */
	return;
}

pend_list *
getpend(ep)
endpoint *ep;
{
	register	s;
	pend_list	*pending;

	DEBUGP(DEB_CALL,(CE_CONT, "getpend()\n"));
	s = SPL();
	/* if empty, signal that, but it's not fatal */
	if(take_pend == give_pend) {
		DEBUGP(DEB_ERROR,(CE_CONT, "getpend: empty, ep = %x\n", ep));
		splx(s);
		enet_stat[ST_PDFA]++;
		DEBUGP(DEB_CALL,(CE_CONT, "getpend => NULL\n"));
		return(NULL);
	}
	if(give_pend == enet_pendtabsize)
		/* Was full, mark non-full */
		give_pend = take_pend;
	pending = enet_pend_tab[take_pend++];
	if(take_pend >= enet_pendtabsize)
		take_pend = 0;
	splx(s);
	DEBUGP(DEB_CALL,(CE_CONT, "getpend => %x\n", pending));
	return(pending);
}

void
relpend(pending)
pend_list	*pending;
{
	register	s;

	DEBUGP(DEB_CALL,(CE_CONT, "relpend(%x)\n", pending));
	if(pending == NULL) {
		DEBUGP(DEB_CALL,(CE_CONT, "relpend => NULL\n"));
		return;
	}
	s = SPL();
	if(give_pend >= enet_pendtabsize) {
		cmn_err(CE_WARN, "Ethernet Driver: release unallocated pending connect block\n");
		splx(s);
		DEBUGP(DEB_CALL,(CE_CONT, "relpend => NULL\n"));
		return;
	}
	enet_pend_tab[give_pend++] = pending;
	if(give_pend >= enet_pendtabsize)
		give_pend = 0;
	if(give_pend == take_pend)
		/* mark as full */
		give_pend = enet_pendtabsize;
	splx(s);
	DEBUGP(DEB_CALL,(CE_CONT, "relpend => NULL\n"));
	return;
}

void
enet_init_buffers()
{
	register i;

	for(i=0; i < enet_rbtabsize; i++)
	{
		enet_rb_tab[i] = &enet_rb_list[i];
		enet_rb_tab[i]->in_use = 0;		/* I002 */
		enet_rb_tab[i]->databuf = 0;		/* I002 */
	}
	take_rb = 0;
	give_rb = enet_rbtabsize;
	for(i=0; i < enet_pendtabsize; i++)
		enet_pend_tab[i] = &enet_pend_list[i];
	take_pend = 0;
	give_pend = enet_pendtabsize;
	enet_rb_hiwat = enet_rbtabsize * 72 / 100;	/* I003 */
	enet_rb_lowat = enet_rbtabsize * 55 / 100;	/* I003 */
	enet_rbs_used = 0;				/* I003 */
	enet_rb_hi = 0;
}

mblk_t *
getmptr(size, pri, ep)
int	size;
int	pri;
endpoint *ep;
{
	mblk_t	*mptr;

	if((mptr = allocb(size, (uint)pri)) == NULL) {
		DEBUGP(DEB_ERROR,(CE_CONT, "getmptr: alloc failed\n"));
		enet_stat[ST_ALFA]++;
		u.u_error = ENOSR;
		enet_pferr(ep);
		return (mptr);
	}
	DEBUGP(DEB_CALL,(CE_CONT, "getmptr => %x\n", mptr));
	*(unsigned short *)mptr->b_wptr = 0xffff;
	*(unsigned short *)(mptr->b_wptr+2) = size;
	if((mptr->b_datap->db_lim - mptr->b_datap->db_base) >= 28) {
		mblk_t **wptr = (mblk_t **)mptr->b_wptr;
		unsigned long *sp = (unsigned long *)&mptr;
		int i;

		wptr++;
		*wptr++ = mptr;
		sp++;
		for(i=0;i<5;i++) {
			sp++;
			*wptr++ = *(mblk_t **)sp;
			sp--;
			if(sp < *(unsigned long **)sp)
				break;
			sp = *(unsigned long **)sp;
		}
	}
	return(mptr);
}

mblk_t *
growmptr(size, pri, ep, mptr)
int	size;
int	pri;
endpoint *ep;
mblk_t *mptr;
{
	mblk_t *tmp;

	if((mptr->b_datap->db_lim - mptr->b_datap->db_base) < size) {
		DEBUGP(DEB_FULL,(CE_CONT, "growmptr: %x not big enough\n", mptr));
		tmp = getmptr(size, pri, ep);
		if(tmp == NULL) {
			DEBUGP(DEB_CALL,(CE_CONT, "growmptr: => NULL (getmpr <= NULL)\n"));
			freemsg(mptr);
			return(NULL);
		}
		tmp->b_datap->db_type = mptr->b_datap->db_type;
		((union T_primitives *)tmp->b_datap->db_base)->type =
			((union T_primitives *)mptr->b_datap->db_base)->type;
		freemsg(mptr);
		DEBUGP(DEB_FULL,(CE_CONT, "growmptr => %x\n",tmp));
		return(tmp);
	}
	else
		return(mptr);
}
