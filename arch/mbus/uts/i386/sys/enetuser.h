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

#ident	"@(#)mbus:uts/i386/sys/enetuser.h	1.3.1.1"

#ifndef ENETUSERH
#define ENETUSERH

/* Address structure */
/*
struct net_addr {
	unsigned char net_addr_len;
	union {
		struct {
			unsigned char length;
			unsigned char afi;		- always 0x49
			unsigned short subnet_no;
			unsigned char subnet_addr[7];	- host addr, 0
			unsigned char rem_nsap_id;	- optional
		} subnet_addr;
		struct {
			unsigned char length;
			unsigned char afi;		- always 0x49
			unsigned char areaid[5];
			unsigned char subnet_no;
			unsigned char subnet_addr[7];	- host addr, 0
			unsigned char rem_nsap_id;	- optional
		} area_addr;
		struct {
			unsigned char length;		- always 12
			unsigned long subnet_no;	- always 1
			unsigned char subnet_addr[6];	- host addr
			unsigned short rem_nsap_id;	- always 1
		} compat_addr;
	unsigned char tsap_len;				- always 2
	unsigned short tsap;
};
*/
#define STD_NET_ADDR_LEN 14	/* Subnet addressing, no rem_nsap_id */
#define MAX_NET_ADDR_LEN 19

#define T_NA_LEN(na) ((na)[0])

#define TS_NA_AFI(na) ((na)[1])
#define TS_NA_SUBNETNO(na) (*(unsigned short *)&(na)[2])
#define TS_NA_SUBNETADDR(na) ((unsigned char *)&(na)[4])
#define TS_NA_NSAP_ID(na) ((na)[11])

#define TA_NA_AFI(na) ((na)[1])
#define TA_NA_AREAID(na) ((unsigned char *)&(na)[2])
#define TA_NA_SUBNETNO(na) ((na)[7])
#define TA_NA_SUBNETADDR(na) ((unsigned char *)&(na)[8])
#define TA_NA_NSAP_ID(na) ((na)[15])

#define TC_NA_SUBNETNO(na) (*(unsigned long *)&(na)[1])
#define TC_NA_SUBNETADDR(na) ((unsigned char *)&(na)[5])
#define TC_NA_NSAP(na) (*(unsigned short *)&(na)[11])

#define T_TSAP_LEN(na) ((na)[(na)[0]+1])
#define T_TSAP(na) (*(unsigned short *)&(na)[(na)[0]+2])

/* Boot Image header structure */
/*
struct inabimg {
	unsigned char	bi_command;
	unsigned long	bi_loadsa;
	unsigned short	bi_length;
	unsigned long	bi_execsa;
};
*/
typedef unsigned char inabimg[11];
#define BIMG_CMD(x) x[0]
#define BIMG_LOADA(x) *(unsigned long *)&x[1]
#define BIMG_LENGTH(x) *(unsigned short *)&x[5]
#define BIMG_EXECA(x) *(unsigned long *)&x[7]

/* iNA961 object code module codes */
#define  ANOTHER_MOD    01
#define  EXECUTE        02

/* Options Info */
typedef unsigned short opts;
#define COPT_LEN sizeof(opts) 
#define OPT_DISCON	0x01
#define OPT_CLTS	0x02
#define OPT_COTS	0x04
#define OPT_EXCL	0x08	/* exclusive open of device */
#define OPT_EXPD	0x10	/* support expedited data */
#define U_OPTIONS (opts)(OPT_DISCON|OPT_CLTS|OPT_COTS|OPT_EXCL|OPT_EXPD)

/* NS primitives */
#define NS_ADD_NAME_REQ	0x50
#define NS_DEL_NAME_REQ	0x51
#define NS_GET_VAL_REQ	0x52
#define NS_CHNG_VAL_REQ	0x53
#define NS_DEL_PROP_REQ	0x54
#define NS_GET_NAME_REQ	0x55
#define NS_GET_SPOK_REQ	0x56
#define NS_LIST_TAB_REQ	0x57

struct NS_req {
	long	PRIM_type;
	long	NAME_length;
	long    UNIQUE_flag;
	long    PROPERTY_type;
	long    PROPERTY_value;
	long	VALUE_length;
	long	EXTRA_length;
};

#define NS_ADD_NAME_RES	0x60
#define NS_DEL_NAME_RES	0x61
#define NS_GET_VAL_RES	0x62
#define NS_CHNG_VAL_RES	0x63
#define NS_DEL_PROP_RES	0x64
#define NS_GET_NAME_RES	0x65
#define NS_GET_SPOK_RES	0x66
#define NS_LIST_TAB_RES	0x67

struct NS_res {
	long	PRIM_type;
	long	RESP_code;
	long	EXTRA_length;
};

/* NMF primitives */
#define NMF_READ_O_REQ	0x70
#define NMF_RDCLR_O_REQ	0x71
#define NMF_SET_O_REQ	0x72

struct NMF_req {
	long	PRIM_type;
	long	CMD_length;
	long	RESP_length;
};

#define NMF_READ_O_RES	0x80
#define NMF_RDCLR_O_RES	0x81
#define NMF_SET_O_RES	0x82

struct NMF_res {
	long	PRIM_type;
	long	RESP_code;
	long	RESP_length;
};
#endif
