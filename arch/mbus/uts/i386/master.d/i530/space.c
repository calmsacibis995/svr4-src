/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988, 1989, 1990  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/master.d/i530/space.c	1.3.2.1"

#include "sys/enet.h"

#define	N_ENET			1
#define	NVC			101
#define	N_ENDPOINTS 		101
#define	DATA_BUF_LEN 		4096
#define	MAX_BUFS_POSTED		1
#define	MAXCOPYSIZ		128
#define MAX_DATA_RQ		1
#define SEND_DELAY		0
#define SH_HIWAT		2048	
#define SH_LOWAT		1024
#define MAX_PEND 		NVC
#define PENDTABSIZE		(MAX_PEND*3)
#define RBTABSIZE		(MAX_PEND*4)
#define DATAGRAM_SIZE		64
#define U_DEFAULTS  		(opts)(OPT_DISCON|OPT_COTS)

#include "config.h"	/* for overriding above parameters */

int enet_n_boards	= N_ENET;	/* number of enet boards */

int enet_nvc		= NVC;		/* number of virtual circuits */

int enet_data_buf_len	= DATA_BUF_LEN;	/* receive buffer size */
int enet_max_bufs_posted = MAX_BUFS_POSTED; /* no. of bufs posted during open */
int enet_maxcopysiz	= MAXCOPYSIZ;	/* max msg size to copy to new buffer */
int enet_max_data_rq	= MAX_DATA_RQ;	/* max number of concurrent sends */
int enet_sh_hiwat	= SH_HIWAT;	/* Streams head high water mark */
int enet_sh_lowat	= SH_LOWAT;	/* Streams head low water mark */
int enet_sdelay		= SEND_DELAY;	/* # 10-msecs between data sends */
int enet_datagram_size	= DATAGRAM_SIZE; /* size of posted datagram buffer */
opts enet_u_defaults	= U_DEFAULTS;	/* default value of endpoint options */

int enet_rb_hiwat;	/* request block high water mark for flow control*/
int enet_rb_lowat;	/* request block low water mark for flow control*/
int enet_rbtabsize	= RBTABSIZE;
int enet_pendtabsize	= PENDTABSIZE;
int enet_max_pend	= MAX_PEND;

struct req_blk *enet_rb_tab[RBTABSIZE];
pend_list	*enet_pend_tab[PENDTABSIZE];
struct req_blk	enet_rb_list[RBTABSIZE];
pend_list	enet_pend_list[PENDTABSIZE];

struct enetboard enet_boards[N_ENET];

struct enetinf enet_inform[N_ENET];

/*
 * slotid is now just a place holder; its value is set at system initialization
 *	time when the driver scans the backplane to locate the first 186/530
 */

struct enetcfg enet_cfg[N_ENET] = {
/*      {slotid, lcis_rb_port, lcis_xmit_port, lcis_rcv_port} */
	{0, 0x0505, 0x0505, 0x0505},     /* one line per board here */
};

char *enet_ics_name_ptr[] = {"186/530", "MIX386/560", 0};

int enet_n_endpoints	= N_ENDPOINTS;
endpoint enet_endpoints[N_ENDPOINTS];

int enet_majtobnum[256]	= {
/*0*/	 0,
/*1*/	 0,
/*2*/	 0,
/*3*/	 0,
/*4*/	 0,
/*5*/	 0,
/*6*/	 0,
/*7*/	 0,
/*8*/	 0,
/*9*/	 0,
/*10*/	 0,
/*11*/	 0,
/*12*/	 0,
/*13*/	 0,
/*14*/	 0,
/*15*/	 0,
/*16*/	 0,
/*17*/	 0,
/*18*/	 0,
/*19*/	 0,
/*20*/	 0,
/*21*/	 0,
/*22*/	 0,
/*23*/	 0,
/*24*/	 0,
/*25*/	 0,
/*26*/	 0,
/*27*/	 0,
/*28*/	 0,
/*29*/	 0,
/*30*/	 0,
/*31*/	 0,
/*32*/	 0,
/*33*/	 0,
/*34*/	 0,
/*35*/	 0,
/*36*/	 0,
/*37*/	 0,
/*38*/	 0,
/*39*/	 0,
/*40*/	 0,
/*41*/	 0,
/*42*/	 0,
/*43*/	 0,
/*44*/	 0,
/*45*/	 0,
/*46*/	 0,
/*47*/	 0,
/*48*/	 0,
/*49*/	 0,
/*50*/	 0,
/*51*/	 0,
/*52*/	 0,
/*53*/	 0,
/*54*/	 0,
/*55*/	 0,
/*56*/	 0,
/*57*/	 0,
/*58*/	 0,
/*59*/	 0,
/*60*/	 0,
/*61*/	 0,
/*62*/	 0,
/*63*/	 0,
/*64*/	 0,
/*65*/	 0,
/*66*/	 0,
/*67*/	 0,
/*68*/	 0,
/*69*/	 0,
/*70*/	 0,
/*71*/	 0,
/*72*/	 0,
/*73*/	 0,
/*74*/	 0,
/*75*/	 0,
/*76*/	 0,
/*77*/	 0,
/*78*/	 0,
/*79*/	 0,
/*80*/	 0,
/*81*/	 0,
/*82*/	 0,
/*83*/	-1,     /* last initializer must be -1, enetinit inits rest of table */
};

#if defined(IEDL)
extern void iedl_connect_complete();
extern void iedl_disconnect_complete();
extern void iedl_send_complete();
extern void iedl_post_complete();
void (*edl_conn_complete)() = iedl_connect_complete;
void (*edl_disc_complete)() = iedl_disconnect_complete;
void (*edl_rawtran_complete)() = iedl_send_complete;
void (*edl_rawrecv_complete)() = iedl_post_complete;
#elif defined(EDL)
extern void edlina_conn_complete();
extern void edlina_disc_complete();
extern void edlina_rawtran_complete();
extern void edlina_rawrecv_complete();
void (*edl_conn_complete)() = edlina_conn_complete;
void (*edl_disc_complete)() = edlina_disc_complete;
void (*edl_rawtran_complete)() = edlina_rawtran_complete;
void (*edl_rawrecv_complete)() = edlina_rawrecv_complete;
#else
void edl_stub_complete() {}
void (*edl_conn_complete)() = edl_stub_complete;
void (*edl_disc_complete)() = edl_stub_complete;
void (*edl_rawtran_complete)() = edl_stub_complete;
void (*edl_rawrecv_complete)() = edl_stub_complete;
#endif
