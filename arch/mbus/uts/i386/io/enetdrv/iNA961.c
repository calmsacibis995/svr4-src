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

#ident	"@(#)mbus:uts/i386/io/enetdrv/iNA961.c	1.3.1.1"

 /*
 *   iNA961.c: routines for talking to the iNA961 firmware on the enet board
 *
 *	I000	7/14/1987	DF 	Intel
 *		Added support for dynamic iNA961 seletcion (between R2.0 and
 *		R1.3).
 *	I001	7/20/1987	DF	Intel
 *		Added support for enetinfo.
 *	I002	8/25/1987	DF	Intel
 *		Moved DATA_BUF_LEN definition from enet.h to space.c and 
 *		changed DATA_BUF_LEN to data_buf_len.
 *	I003	9/21/87		DF	Intel
 *		Moved all enet hardware dependent modules to enethwdep.c.
 *		Added compile switch for enet driver. This is done so that
 *		the code is common to both MBI and MBII drivers.
 *	I004	04/13/88	RS/SLH		Intel
 *		Made DATAGRAM_SIZE configurable by user.
 *	I005	07/07/88	NL	Intel
 *		Modified #ifdef to include conditional for AT386
 *		BUS type.
 *	I006	09/08/88	rjs		Intel
 *		Identify when iNA961 R3.0 is being downloaded to enet board.
 *	I005	07/11/89	rjf		Intel
 *		Made lint fixes.
*/

#define DEBUG 1
#include "sys/enet.h"
#include <sys/immu.h>

extern int		enet_data_buf_len;	/* I002 */
extern int		enet_datagram_size;		/* I004 */

int			enet_ina_ver;			/* I000 */
char	ver2[] = "V2.0";		/* I000 */
char	ver3[] = "V3.0";		/* I006 */
char	ver31[] = "V3.1";		/* I006 */

/* 
 * enet statistics structure defined in enet.c
 */
extern ulong		enet_stat[enet_SCNT];

/*
 * enet debug level defined in enet.c
 */
extern int		enet_debug;

void
init_crbh(crbhp)
register crbh *crbhp;
{
	crbhp->c_user[0] = 0;
	crbhp->c_user[1] = 0;
	crbhp->c_resp_port = 0xff;
	crbhp->c_dev_id = L_DEV_ID;
	crbhp->c_port_id = PORT_RETURN_RB;
}

void
init_tabuf(tabufp)
register char *tabufp;
{
	register int i;

	DEBUGC('i');
	T_LNA_LEN(tabufp) = 0;
	T_LTSAP_LEN(tabufp) = 2;
#if defined(MB1) || defined(AT386)		/* I005 */
	if (enet_ina_ver == 1)			/* I003, I006 */
		T_RNA_LEN(tabufp) = 12;
	else
		T_RNA_LEN(tabufp) = 15;
#else
	T_RNA_LEN(tabufp) = 15;
#endif

/* There is no bzero which will deal with non-aligned data (!?!?)
	bzero(&tabufp[5],15);
*/
	for(i=0;i<15;i++)
		tabufp[5+i] = 0;
	T_RTSAP_LEN(tabufp) = 2;
	T_RTSAP(tabufp) = 0;
}

int
iNA961_open(ep, open_rb)
register endpoint *ep;
register struct orb *open_rb;
{
	DEBUGC('O');
	init_crbh(&open_rb->or_crbh);
	open_rb->or_crbh.c_len = ORB_SIZE;
	open_rb->or_crbh.c_subsys = SUB_VC;
	open_rb->or_crbh.c_opcode = OP_OPEN;
	if(!(mipsend(ep->bnum, PORT_960, open_rb, open_rb->or_crbh.c_len))) {
		u.u_error = ENXIO;
		enet_pferr(ep);
		return(1);
	}
	return(0);
}

iNA961_send_connect_req(ep, address, addr_length, scr_rb, databuf, datalen)
register endpoint *ep;
char *address;
int addr_length;
struct crrb *scr_rb;
unchar *databuf;
int datalen;
{
	char *tabufp;

	init_crbh(&scr_rb->cr_crbh);
	scr_rb->cr_crbh.c_len = CRRB_SIZE;
	scr_rb->cr_crbh.c_subsys = SUB_VC;
	scr_rb->cr_crbh.c_opcode = OP_SCR;
	scr_rb->cr_ep = ep;
	tabufp = (char *)scr_rb->cr_tabuf;
	init_tabuf(tabufp);
	T_LTSAP(tabufp) = ep->tsap;
	memcpy(REM_ADDR(tabufp),address,addr_length);
	*(char **)scr_rb->cr_tabufp = (char *)kvtophys((caddr_t)tabufp);
	*(ushort *)scr_rb->cr_pc = 50;
	*(ushort *)scr_rb->cr_ato = 0;	/* use default (configured) timeout */
	*(ushort *)scr_rb->cr_reference = ep->c_reference;
	scr_rb->cr_qos = 0;
	*(opts *)scr_rb->cr_neg_ops = 0;
	*(unchar **)scr_rb->cr_u_buf = (unchar *)kvtophys((caddr_t)databuf);
	scr_rb->cr_u_len = datalen;
	if(!(mipsend(ep->bnum, PORT_960, scr_rb, scr_rb->cr_crbh.c_len))) {
		u.u_error = ENXIO;
		enet_pferr(ep);
		return(1);
	}
	return(0);
}

iNA961_await_conn_req(ep, acr_rb, databuf)
register endpoint *ep;
struct crrb *acr_rb;
unchar *databuf;
{ 
	register char *tabufp;

	DEBUGC('L');
	init_crbh(&acr_rb->cr_crbh);
	acr_rb->cr_crbh.c_len = CRRB_SIZE;
	acr_rb->cr_crbh.c_subsys = SUB_VC;
	acr_rb->cr_crbh.c_opcode = OP_ACRU;
	acr_rb->cr_ep = ep;
	tabufp = (char *)acr_rb->cr_tabuf;
	init_tabuf(tabufp);
	T_LTSAP(tabufp) = ep->tsap;
	*(char **)acr_rb->cr_tabufp = (char *)kvtophys((caddr_t)tabufp);
	*(ushort *)acr_rb->cr_ato = 0;	/* use default (configured) timeout */
	*(ushort *)acr_rb->cr_reference = ep->l_reference;
	acr_rb->cr_qos = 0;
	*(opts *)acr_rb->cr_neg_ops = 0;/* use default (configured) options */
	*(unchar **)acr_rb->cr_u_buf = (unchar *)kvtophys((caddr_t)databuf);
	acr_rb->cr_u_len = CONN_BUF_LEN;
	if(!(mipsend(ep->bnum, PORT_960, acr_rb, acr_rb->cr_crbh.c_len))) {
		u.u_error = ENXIO;
		enet_pferr(ep);
		return(1);
	}
	return(0);
}

/*ARGSUSED*/
iNA961_accept_conn_req(ep, acr_rb, reference, databuf, datalen)
register endpoint *ep;
struct crrb *acr_rb;
ushort reference;
unchar *databuf;
int datalen;
{
	register char *tabufp;

	init_crbh(&acr_rb->cr_crbh);
	acr_rb->cr_crbh.c_len = CRRB_SIZE;
	acr_rb->cr_crbh.c_subsys = SUB_VC;
	acr_rb->cr_crbh.c_opcode = OP_ACR;
	acr_rb->cr_ep = ep;
	tabufp = (char *)acr_rb->cr_tabuf;
	init_tabuf(tabufp);
	*(char **)acr_rb->cr_tabufp = (char *)kvtophys((caddr_t)tabufp);
	*(ushort *)acr_rb->cr_reference = reference;
	if (acr_rb->cr_u_len = datalen)
		*(unchar **)acr_rb->cr_u_buf = (unchar *)kvtophys((caddr_t)databuf);
	if(!(mipsend(ep->bnum, PORT_960, acr_rb, acr_rb->cr_crbh.c_len))) {
		u.u_error = ENXIO;
		enet_pferr(ep);
		return(1);
	}
	return(0);
}

iNA961_send_data(ep, data, len, more, dr_rb)
register endpoint *ep;
unchar *data;
int len;
int more;
struct vcrb *dr_rb;
{
	init_crbh(&dr_rb->vc_crbh);
	dr_rb->vc_crbh.c_len = VCRB_SIZE;
	dr_rb->vc_crbh.c_subsys = SUB_VC;
	if(more) {
		DEBUGC('%');
		dr_rb->vc_crbh.c_opcode = OP_SD;
	}
	else {
		DEBUGC('!');
		dr_rb->vc_crbh.c_opcode = OP_EOM_SD;
	}
	dr_rb->vc_ep = ep;
	dr_rb->vc_reference = ep->c_reference;
	dr_rb->vc_qos = 0;
	dr_rb->vc_nblks = 1;
	*(unchar **)dr_rb->vc_bufp = (unchar *)kvtophys((caddr_t)data);
	dr_rb->vc_buflen = len;
	DEBUGC((char)data[0]);
	DEBUGC((char)((len>>8)&0xff));
	DEBUGC((char)(len&0xff));
	if(!(mipsend(ep->bnum, PORT_960, dr_rb, dr_rb->vc_crbh.c_len))) {
		u.u_error = ENXIO;
		enet_pferr(ep);
		return(1);
	}
	return(0);
}

iNA961_receive_data(wait_ep, error_ep, rd_rb, databuf)
register endpoint *wait_ep, *error_ep;
struct vcrb *rd_rb;
unchar *databuf;
{
	init_crbh(&rd_rb->vc_crbh);
	rd_rb->vc_crbh.c_len = VCRB_SIZE;
	rd_rb->vc_crbh.c_subsys = SUB_VC;
	rd_rb->vc_crbh.c_opcode = OP_RD;
	rd_rb->vc_ep = wait_ep;
	rd_rb->vc_reference = wait_ep->c_reference;
	rd_rb->vc_qos = 0;
	rd_rb->vc_nblks = 1;
	*(unchar **)rd_rb->vc_bufp = (unchar *)kvtophys((caddr_t)databuf);
	rd_rb->vc_buflen = enet_data_buf_len;		/* I002 */
	if(!(mipsend(wait_ep->bnum, PORT_960, rd_rb, rd_rb->vc_crbh.c_len))) {
		u.u_error = ENXIO;
		enet_pferr(error_ep);
		return(1);
	}
	return(0);
}

iNA961_send_expedited_data(ep, data, len, dr_rb)
register endpoint *ep;
unchar *data;
int len;
struct vcrb *dr_rb;
{
	init_crbh(&dr_rb->vc_crbh);
	dr_rb->vc_crbh.c_len = VCRB_SIZE;
	dr_rb->vc_crbh.c_subsys = SUB_VC;
	DEBUGC('E');
	dr_rb->vc_crbh.c_opcode = OP_EX_SD;
	dr_rb->vc_ep = ep;
	dr_rb->vc_reference = ep->c_reference;
	dr_rb->vc_qos = 0;
	dr_rb->vc_nblks = 1;
	*(unchar **)dr_rb->vc_bufp = (unchar *)kvtophys((caddr_t)data);
	dr_rb->vc_buflen = len;
	if(!(mipsend(ep->bnum, PORT_960, dr_rb, dr_rb->vc_crbh.c_len))) {
		u.u_error = ENXIO;
		enet_pferr(ep);
		return(1);
	}
	return(0);
}

iNA961_receive_expedited_data(wait_ep, error_ep, rd_rb, databuf)
register endpoint *wait_ep, *error_ep;
struct vcrb *rd_rb;
unchar *databuf;
{
	init_crbh(&rd_rb->vc_crbh);
	rd_rb->vc_crbh.c_len = VCRB_SIZE;
	rd_rb->vc_crbh.c_subsys = SUB_VC;
	rd_rb->vc_crbh.c_opcode = OP_EX_RD;
	rd_rb->vc_ep = wait_ep;
	rd_rb->vc_reference = wait_ep->c_reference;
	rd_rb->vc_qos = 0;
	rd_rb->vc_nblks = 1;
	*(unchar **)rd_rb->vc_bufp = (unchar *)kvtophys((caddr_t)databuf);
	rd_rb->vc_buflen = ETSDU_SIZE;
	if(!(mipsend(wait_ep->bnum, PORT_960, rd_rb, rd_rb->vc_crbh.c_len))) {
		u.u_error = ENXIO;
		enet_pferr(error_ep);
		return(1);
	}
	return(0);
}

void
iNA961_close(ep, reference, c_rb, rsn, databuf, datalen)
endpoint *ep;
ushort reference;
struct vcrb *c_rb;
int rsn;
unchar *databuf;
int datalen;
{
	DEBUGC('C');
	init_crbh(&c_rb->vc_crbh);
	c_rb->vc_crbh.c_len = VCRB_SIZE;
	c_rb->vc_crbh.c_subsys = SUB_VC;
	c_rb->vc_crbh.c_opcode = OP_CLOSE;
	c_rb->vc_ep = ep;
	c_rb->vc_reference = reference;
	c_rb->vc_iso_rc = rsn;
	c_rb->vc_qos = 0;
	c_rb->vc_nblks = (datalen == 0)?0:1;
	if(datalen == 0)
		*(unchar **)c_rb->vc_bufp = (unchar *)0;
	else 
		*(unchar **)c_rb->vc_bufp = (unchar *)kvtophys((caddr_t)databuf);
	c_rb->vc_buflen = datalen;
	if(!(mipsend(ep->bnum, PORT_960, c_rb, c_rb->vc_crbh.c_len))) {
		if(c_rb->vc_databuf)
			freemsg(c_rb->vc_databuf);
		enet_pferr(ep);
		relrb((struct req_blk *)c_rb);
	}
	return;
}

iNA961_await_close(ep, reference, ac_rb, databuf)
register endpoint *ep;
ushort reference;
struct vcrb *ac_rb;
unchar *databuf;
{
	init_crbh(&ac_rb->vc_crbh);
	ac_rb->vc_crbh.c_len = VCRB_SIZE;
	ac_rb->vc_crbh.c_subsys = SUB_VC;
	ac_rb->vc_crbh.c_opcode = OP_ACLOSE;
	ac_rb->vc_ep = ep;
	ac_rb->vc_reference = reference;
	ac_rb->vc_qos = 0;
	ac_rb->vc_nblks = 1;
	*(unchar **)ac_rb->vc_bufp = (unchar *)kvtophys((caddr_t)databuf);
	ac_rb->vc_buflen = CLOSE_BUF_LEN;
	if(!(mipsend(ep->bnum, PORT_960, ac_rb, ac_rb->vc_crbh.c_len))) {
		u.u_error = ENXIO;
		enet_pferr(ep);
		return(1);
	}
	return(0);
}

iNA961_send_datagram(ep, address, d_rb, databuf, datalen)
register endpoint *ep;
char *address;
struct drb *d_rb;
unchar *databuf;
int datalen;
{
	register char *tabufp;

#if DEBUG
	int i;

	DEBUGP(DEB_FULL,(CE_CONT, " address="));
	for (i=0;i<15;i++)
		DEBUGP(DEB_FULL,(CE_CONT, " %x", (char)*(address+i)));
	DEBUGP(DEB_FULL,(CE_CONT, "\n data="));
	for (i=0;i<datalen;i++)
		DEBUGP(DEB_FULL,(CE_CONT, " %x", (char)*(databuf+i)));
	DEBUGP(DEB_FULL,(CE_CONT, "\n"));
#endif
	DEBUGC('G');
	init_crbh(&d_rb->dr_crbh);
	d_rb->dr_crbh.c_len = DRB_SIZE;
	d_rb->dr_crbh.c_subsys = SUB_DG;
	d_rb->dr_crbh.c_opcode = OP_SDGM;
	d_rb->dr_ep = ep;
	tabufp = (char *)d_rb->dr_tabuf;
	init_tabuf(tabufp);
	T_LTSAP(tabufp) = ep->tsap;
	memcpy(REM_ADDR(tabufp),address,ADDR_LENGTH(REM_ADDR(tabufp)));
	*(char **)d_rb->dr_tabufp = (char *)kvtophys((caddr_t)tabufp);
	d_rb->dr_nblks = 1;
	*(unchar **)d_rb->dr_bufp = (unchar *)kvtophys((caddr_t)databuf);
	d_rb->dr_buflen = datalen;
#if DEBUG
	DEBUGP(DEB_FULL,(CE_CONT, " OUTGOING REQUEST BLOCK:\n"));
	for (i=0;i<80;i++)
	{
		if (i%16 == 0)
			DEBUGP(DEB_FULL,(CE_CONT, "\n"));
		DEBUGP(DEB_FULL,(CE_CONT, " %x", (unsigned char) *((char*)d_rb+i)));
	}
	DEBUGP(DEB_FULL,(CE_CONT, "\n"));
#endif
	if(!(mipsend(ep->bnum, PORT_960, d_rb, d_rb->dr_crbh.c_len))) {
		u.u_error = ENXIO;
		enet_pferr(ep);
		return(1);
	}
	return(0);
}

iNA961_receive_datagram(ep, d_rb, databuf)
register endpoint *ep;
struct drb *d_rb;
unchar *databuf;
{ 
	register char *tabufp;

	init_crbh(&d_rb->dr_crbh);
	d_rb->dr_crbh.c_len = DRB_SIZE;
	d_rb->dr_crbh.c_subsys = SUB_DG;
	d_rb->dr_crbh.c_opcode = OP_RDGM;
	d_rb->dr_ep = ep;
	tabufp = (char *)d_rb->dr_tabuf;
	init_tabuf(tabufp);
	T_LTSAP(tabufp) = ep->tsap;
	*(char **)d_rb->dr_tabufp = (char *)kvtophys(tabufp);
	d_rb->dr_qos = 0;
	d_rb->dr_nblks = 1;
	*(unchar **)d_rb->dr_bufp = (unchar *)kvtophys((caddr_t)databuf);
	d_rb->dr_buflen = enet_datagram_size;
	if(!(mipsend(ep->bnum, PORT_960, d_rb, d_rb->dr_crbh.c_len))) {
		u.u_error = ENXIO;
		enet_pferr(ep);
		return(1);
	}
	return(0);
}

iNA961_withdraw_datagram(ep, d_rb)
register endpoint *ep;
struct drb *d_rb;
{ 
	register char *tabufp;

	init_crbh(&d_rb->dr_crbh);
	d_rb->dr_crbh.c_len = DRB_SIZE;
	d_rb->dr_crbh.c_subsys = SUB_DG;
	d_rb->dr_crbh.c_opcode = OP_WDGM;
	d_rb->dr_ep = ep;
	tabufp = (char *)d_rb->dr_tabuf;
	init_tabuf(tabufp);
	T_LTSAP(tabufp) = ep->tsap;
	*(char **)d_rb->dr_tabufp = (char *)kvtophys((caddr_t)tabufp);
	d_rb->dr_qos = 0;
	d_rb->dr_nblks = 1;
	*(unchar **)d_rb->dr_bufp = 0;
	d_rb->dr_count[0] = 0xff;
	d_rb->dr_count[1] = 0xff;
	if(!(mipsend(ep->bnum, PORT_960, d_rb, d_rb->dr_crbh.c_len))) {
		u.u_error = ENXIO;
		enet_pferr(ep);
		return(1);
	}
	return(0);
}


#if defined(MB1) || defined(AT386)	/* I005 */
/* I000 I003 */
/* This routine search the buffer for string "V2.0". It returns 2 if the
   string is found, else it returns 1 */

ina_version(buf_p, buf_siz)

 char 	 *buf_p;
 int     buf_siz;
{
   int    i;

   i = 0;
   while (i < buf_siz-4)
    {
      if (str_compare((char *)&ver2[0], buf_p, 4) == 0)
      {
        cmn_err(CE_CONT, "Loading iNA961 Release 2.0\n");
        return (20);
      }
      else if (str_compare((char *)&ver3[0], buf_p, 4) == 0)
      {
        cmn_err(CE_CONT, "Loading iNA961 Release 3.0\n");
        return (30);
      }
      else if (str_compare((char *)&ver31[0], buf_p, 4) == 0)
      {
        cmn_err(CE_CONT, "Loading iNA961 Release 3.1\n");
        return (31);
      }

      i++;
      buf_p++;
    }
   
    cmn_err(CE_CONT, "Loading iNA961\n");
    return (10);
}

str_compare(s1, s2, n)
char *s1; 
char *s2;
int n;
{
 char *t;

  t = s2;
  while (--n >= 0 && *s1 == *t++)
    if (*s1++ == '\0')
      return(0);
  return(n<0 ? 0 : *s1 - *--t);
}
#endif
