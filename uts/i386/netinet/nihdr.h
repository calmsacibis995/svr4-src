/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _NETINET_NIHDR_H
#define _NETINET_NIHDR_H

#ident	"@(#)kern-inet:nihdr.h	1.3"

/*
 *  		PROPRIETARY NOTICE (Combined)
 *  
 *  This source code is unpublished proprietary information
 *  constituting, or derived under license from AT&T's Unix(r) System V.
 *  In addition, portions of such source code were derived from Berkeley
 *  4.3 BSD under license from the Regents of the University of
 *  California.
 *  
 *  
 *  
 *  		Copyright Notice 
 *  
 *  Notice of copyright on this source code product does not indicate 
 *  publication.
 *  
 *  	(c) 1986,1987,1988,1989  Sun Microsystems, Inc.
 *  	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *  	          All rights reserved.
 */


/*
 * System V STREAMS TCP - Release 3.0 
 *
 * Copyright 1987, 1988, 1989 Lachman Associates, Incorporated (LAI) 
 * All Rights Reserved. 
 *
 * The copyright above and this notice must be preserved in all copies of this
 * source code.  The copyright above does not evidence any actual or intended
 * publication of this source code. 
 *
 * This is unpublished proprietary trade secret source code of Lachman
 * Associates.  This source code may not be copied, disclosed, distributed,
 * demonstrated or licensed except as expressly authorized by Lachman
 * Associates. 
 *
 * System V STREAMS TCP was jointly developed by Lachman Associates and
 * Convergent Technologies. 
 */

/*
 * Network Level Interface standard definitions. Note: I did these in
 * preparation for what I felt AT&T will do in the future.  This file will
 * undoubtably have to be significantly modified if AT&T ever actually does
 * publish a network level interface. This interface is in fact based heavily
 * on the link level interface and represents basically the same
 * functionality. 
 */


/*
 * Primitives that are initiated by the Network-Layer user 
 */

#define	N_INFO_REQ	0	/* data link layer protocol parameter sizes */
#define N_BIND_REQ	1	/* bind protocol address request 	   */
#define	N_UNBIND_REQ	2	/* unbind protocol address request 	   */
#define	N_UNITDATA_REQ	7	/* unit_data send request 		   */


/*
 * Primitives that are initiated by the Link-Layer provider 
 */

#define	N_INFO_ACK	3	/* protocol information acknowledgement */
#define	N_BIND_ACK	4	/* protocol bind acknowledgement 	 */
#define	N_ERROR_ACK	5	/* error acknowledgement 		 */
#define	N_OK_ACK	6	/* success acknowledgement 		 */
#define	N_UNITDATA_IND	8	/* unitdata receive indication 		 */
#define	N_UDERROR_IND	9	/* unitdata receive indication 		 */


/*
 * Primitive Non-fatal error return codes 
 */

#define NBADSAP		0	/* bad NSAP selector			 */
#define	NACCES		2	/* inproper permissions 		 */
#define	NOUTSTATE	3	/* Network layer interface out of state     */
#define	NSYSERR		4	/* Unix system error 			 */


/*
 * Network-Layer current state definitions 
 */

#define	N_UNBND		0	/* NL not bound 	     */
#define	N_WACK_B	1	/* NL waiting for bind ack   */
#define	N_WACK_U	2	/* NL waiting for unbind ack */
#define	N_IDLE		3	/* NL is active		     */


/*
 * The following structure definitions define the format of the streams
 * message blocks used to define the Link-Layer Interface. 
 */


/*
 * User Originated Primitives 
 */

struct N_info_req {
	long            PRIM_type;	/* always N_INFO_REQ */
};


struct N_bind_req {
	long            PRIM_type;	/* always N_BIND_REQ */
	long            N_sap;	/* the NSAP selector */
	long            GROWTH_field[2];	/* 802.2 llc type 2 fields */
};


struct N_unbind_req {
	long            PRIM_type;	/* always N_UNBIND_REQ */
};


struct N_unitdata_req {
	long            PRIM_type;	/* always N_UNITDATA_REQ */
	long            RA_length;	/* dest NSAP addr length */
	long            RA_offset;	/* dest NSAP addr offset */
	long            SERV_class;	/* service class */
	long            FILLER_field;	/* 802.2 LLC2 field */
};


/*
 * Provider Originated Primitives 
 */

struct N_info_ack {
	long            PRIM_type;	/* always N_INFO_ACK */
	long            SDU_max;/* max lsdu size */
	long            SDU_min;/* min lsdu size */
	long            ADDR_length;	/* NSAP address length in bytes */
	long            SUBNET_type;	/* subnet type */
	long            SERV_class;	/* service class */
	long            CURRENT_state;	/* link layer state */
};


struct N_bind_ack {
	long            PRIM_type;	/* always N_BIND_ACK */
	long            N_sap;	/* NSAP selector */
	long            ADDR_length;	/* NSAP address length in bytes */
	long            ADDR_offset;	/* NSAP address offset in the message */
};


struct N_error_ack {
	long            PRIM_type;	/* always N_ERROR_ACK */
	long            ERROR_prim;	/* primitive in error */
	long            N_error;/* N error code */
	long            UNIX_error;	/* UNIX error code */
};


struct N_ok_ack {
	long            PRIM_type;	/* always N_OK_ACK */
	long            CORRECT_prim;	/* correct primitive */
};


struct N_unitdata_ind {
	long            PRIM_type;	/* always N_UNITDATA_IND */
	long            RA_length;	/* dest NSAP address length in bytes */
	long            RA_offset;	/* dest offset NSAP into message */
	long            LA_length;	/* src NSAP address length in bytes */
	long            LA_offset;	/* src offset NSAP into message */
	long            SERV_class;	/* service class */
};


struct N_uderror_ind {
	long            PRIM_type;	/* always N_UDERROR_IND */
	long            RA_length;	/* dest NSAP address length in bytes */
	long            RA_offset;	/* dest NSAP offset into msg in bytes */
	long            SERV_class;	/* service class */
	long            ERROR_type;	/* error type */
};


/*
 * The following is a union of all the primitives. 
 */

union N_primitives {
	long            prim_type;
	struct N_info_req info_req;
	struct N_bind_req bind_req;
	struct N_unbind_req unbind_req;
	struct N_unitdata_req data_req;
	struct N_info_ack info_ack;
	struct N_bind_ack bind_ack;
	struct N_error_ack error_ack;
	struct N_ok_ack ok_ack;
	struct N_unitdata_ind data_ind;
	struct N_uderror_ind error_ind;
};


/*
 * Structure/union size constants. 
 */

#define N_INFO_REQ_SIZE		sizeof(struct N_info_req)
#define N_BIND_REQ_SIZE		sizeof(struct N_bind_req)
#define N_UNBIND_REQ_SIZE	sizeof(struct N_unbind_req)
#define N_INFO_ACK_SIZE		sizeof(struct N_info_ack)
#define N_BIND_ACK_SIZE		sizeof(struct N_bind_ack)
#define N_ERROR_ACK_SIZE	sizeof(struct N_error_ack)
#define N_OK_ACK_SIZE		sizeof(struct N_ok_ack)
#define N_UNITDATA_REQ_SIZE	sizeof(struct N_unitdata_req)
#define N_UNITDATA_IND_SIZE	sizeof(struct N_unitdata_ind)
#define N_UDERROR_IND_SIZE	sizeof(struct N_uderror_ind)
#define N_PRIMITIVES_SIZE	sizeof(union N_primitives)
#endif	/* _NETINET_NIHDR_H */
