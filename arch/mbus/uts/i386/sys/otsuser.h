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

#ident	"@(#)mbus:uts/i386/sys/otsuser.h	1.3"

/*
** ABSTRACT:	User structures and defines for System V/386 OTS driver
**
** MODIFICATIONS:
*/

/*
 * otsmod specific statistics counts are as follows:
 *
 *	ots_stat[ST_ALFA]	Count of STREAMS buffer allocation failures
 *	ots_stat[ST_RMSG]	Count of received messages
 *	ots_stat[ST_REXP]	Count of received expedited messages
 *	ots_stat[ST_RUNI]	Count of received datagrams
 *	ots_stat[ST_SMSG]	Count of sent messages
 *	ots_stat[ST_SEXP]	Count of sent expedited messages
 *	ots_stat[ST_SUNI]	Count of sent datagrams
 *	ots_stat[ST_BRCV]	Count of data bytes passed to users
 *	ots_stat[ST_URCV]	Count of datagram bytes passed to users
 *	ots_stat[ST_ERCV]	Count of expedited data bytes passed to users
 *	ots_stat[ST_BSNT]	Count of data bytes written for users
 *	ots_stat[ST_USNT]	Count of datagram bytes written for users
 *	ots_stat[ST_ESNT]	Count of expedited data bytes written for users
 *	ots_stat[ST_SPCK]	Count of sent packets (streams message blocks)
 *	ots_stat[ST_EPCK]	Count of sent expedited packets
 *	ots_stat[ST_UPCK]	Count of sent datagram packets
 *	ots_stat[ST_CURO]	Number of currently open endpoints
 *	ots_stat[ST_TOTO]	Total number of opens done
 *	ots_stat[ST_CURC]	Number of currently connected endpoints
 *	ots_stat[ST_TOTC]	Total number of connections made
 *	ots_stat[ST_BADCON1]	Connect failure count: no Transport resources
 *	ots_stat[ST_BADCON2]	Connect failure count: no driver resources
 *	ots_stat[ST_BADCON3]	Connect failure count: no data ports
 *	ots_stat[ST_TKIRETRY]	Retries on transport sends
 *	ots_stat[ST_TKIFAILS]	Failures to send or receive data
 *	ots_stat[ST_TKIMBLK]	Failures to allocate message blocks
 *	ots_stat[ST_TKIDBLK]	Failures to allocate data buffer descriptors
 *	ots_stat[ST_TKITID]	Failures to allocate transaction ids
 */

#define ST_ALFA		0
#define ST_RMSG		1
#define ST_REXP		2
#define ST_RUNI		3
#define ST_SMSG		4
#define ST_SPCK		5
#define ST_SEXP		6
#define ST_EPCK		7
#define ST_SUNI		8
#define ST_UPCK		9
#define ST_BRCV		10
#define ST_ERCV		11
#define ST_URCV		12
#define ST_BSNT		13
#define ST_ESNT		14
#define ST_USNT		15
#define ST_CURO		16
#define ST_TOTO		17
#define ST_CURC		18
#define ST_TOTC		19
#define ST_BADCON1	20
#define ST_BADCON2	21
#define ST_BADCON3	22
#define ST_TKIRETRY	23
#define ST_TKIFAILS	24
#define ST_TKIMBLK	25
#define ST_TKIDBLK	26
#define ST_TKITID	27
#define OTS_SCNT	28

/* Options Info */
typedef unsigned long opts;
#define COPT_LEN sizeof(opts) 
#define OPT_COTS	0x1
#define OPT_EXP		0x2
#define OPT_ORD		0x4
#define OPT_CLTS	0x8
#define U_OPTIONS	(opts)(OPT_EXP|OPT_ORD|OPT_CLTS|OPT_COTS)
