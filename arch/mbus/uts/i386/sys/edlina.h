/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/sys/edlina.h	1.3"

#define	MAC_ADD_SIZE	6	/* size of a MAC address */
#define MAX_PACK_SIZE	1500	/* maximum size of an ethernet packet */

/*
 *  Typedefs that they forgot to put in lihdr.h
 */
typedef struct  DL_info_req            DL_info_req_t ;
typedef struct  DL_bind_req            DL_bind_req_t ;
typedef struct  DL_unbind_req          DL_unbind_req_t ;
typedef struct  DL_unitdata_req        DL_unitdata_req_t ;
typedef struct  DL_info_ack            DL_info_ack_t ;
typedef struct  DL_bind_ack            DL_bind_ack_t ;
typedef struct  DL_error_ack           DL_error_ack_t ;
typedef struct  DL_ok_ack              DL_ok_ack_t ;
typedef struct  DL_unitdata_ind        DL_unitdata_ind_t;

/*
 *  Constants and structs specific to this driver
/*
#define LSAP_FREE	0	/* edlinaopen has not allocated this SAP */
#define LSAP_ATTACHED 	1	/* an upstream module rcvs at the given SAP */
#define N_LSAP		32	/* number of SAP's per installable board */
#define DL_DEAD         0xffff
#define TRUE            1     
#define FALSE           0

typedef struct {
	int		maj_dev;	/* mknod and space.c -> edlinainit() */
	int		board_num;	/* board number */	
	int		opens;		/* number of open streams */
	endpoint	*p_ep;
	unsigned char	*mac_addr;	/* enet host id */
} info_t;

/*
 *  To receive connectionless data, sap_state must be set to DL_IDLE,
 *  sap must contain proper SAP, and the read q ptr must point to the
 *  module that will be consuming data on this SAP.
 */
typedef struct {
	int		sap;
	int		sap_state; 
	queue_t		*read_q;
	queue_t		*write_q;
	info_t		*p_info;
} sap_t ; 

/*
 *  This struct contains all information needed to use the edlina driver.
 */
typedef struct {
	sap_t		sap[ N_LSAP ];
	info_t		info;
} struct_edlina_t ;

#ifdef DEBUG
#define EDEBUGP(level,args) if(edlina_debug >= level) cmn_err args;
#else
#define EDEBUGP(level,args)
#endif
