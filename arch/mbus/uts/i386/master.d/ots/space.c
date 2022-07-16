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

#ident	"@(#)mbus:uts/i386/master.d/ots/space.c	1.3"

#include <sys/ots.h>

#define	NVC			25		/* VC endpoints */
#define NDG			5		/* datagram endpoints */
#define MAX_PEND		5		/* max connection indications */
#define TSDU_SIZE		0x10000		/* max TSDU size */
#define ETSDU_SIZE		0		/* max ETSDU size */
#define CDATA_SIZE		64		/* max connect user data */
#define DDATA_SIZE		64		/* max disconnect user data */
#define DATAGRAM_SIZE		4096		/* max datagram size */
#define OTS_RD_HIWAT		1024		/* OTS downstream queue hwm */
#define OTS_RD_LOWAT		512		/* OTS downstream queue lwm */
#define OTS_WR_HIWAT		1024		/* OTS upstream queue hwm */
#define OTS_WR_LOWAT		512		/* OTS upstream queue lwm */

#define U_VCDEFAULTS		(opts)(OPT_COTS|OPT_EXP)
#define U_DGDEFAULTS		(opts)(OPT_CLTS)

/*------------------------------------------------------
|	Only modify parameters above this notice	|
-------------------------------------------------------*/

#define QUEUE_LEN		6	/* flow control queue size */
#define N_XMT_RETRIES		10	/* max transmit retries */
#define ADDR_SIZE		4	/* max tranport address size */
#define OPTS_SIZE		4	/* max options size */
#define BUFFER_SIZE		0x1000	/* negotiated buffer size */
#define MAX_FRAGMENT		0	/* maximum fragment size */
#define SH_HIWAT		256	/* STREAMS head hi water mark */
#define SH_LOWAT		0	/* STREAMS head low water mark*/
#define FIRST_MBII_PORT		0x0420		/* first reserved port */
#define N_MBII_PORTS		NDG+(NVC*2)	/* number of reserved ports */

#include "config.h"			/* for overriding above parameters */

#define CONNECT_SIZE	NDG + (NVC * 2)	/* size of connect table */
#define NEP		NVC + NDG + 1	/* total number of endpoints */

endpoint ots_endpoints[NEP];
connect ots_connects[CONNECT_SIZE];
char ots_address[NEP][ADDR_SIZE];
ushort ots_p_index[N_MBII_PORTS];

struct otscfg otscfg =
{
	(opts) U_VCDEFAULTS,
	(opts) U_DGDEFAULTS,
	(ulong) TSDU_SIZE,
	(ulong) MAX_FRAGMENT,
	(ushort) ETSDU_SIZE,
	(ushort) DATAGRAM_SIZE,
	(ushort) FIRST_MBII_PORT,
	(ushort) OPTS_SIZE,
	(ushort) ADDR_SIZE,
	(ushort) CDATA_SIZE,
	(ushort) DDATA_SIZE,
	(ushort) QUEUE_LEN,
	(ushort) BUFFER_SIZE,
	(ushort) CONNECT_SIZE,
	(ushort) N_MBII_PORTS,
	(ushort) NEP,
	(ushort) NVC,
	(ushort) NDG,
	(ushort) N_XMT_RETRIES,
	(ushort) OTS_RD_HIWAT,
	(ushort) OTS_RD_LOWAT,
	(ushort) OTS_WR_HIWAT,
	(ushort) OTS_WR_LOWAT,
	(ushort) SH_HIWAT,
	(ushort) SH_LOWAT,
	(uchar) MAX_PEND
};
