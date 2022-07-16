/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_LIHDR_H
#define _SYS_LIHDR_H

#ident	"@(#)head.sys:sys/lihdr.h	11.2.7.1"

/*
 * Data Link Level Interface standard definitions.
 */


/*
 *	Primitive that are initiated by the Link-Layer user
 */

#define	DL_INFO_REQ	0	/* data link layer protocol parameter sizes*/
#define DL_BIND_REQ	1	/* bind protocol address request 	   */
#define	DL_UNBIND_REQ	2	/* unbind protocol address request 	   */
#define	DL_UNITDATA_REQ	7	/* unit_data send request 		   */


/*
 *	Primitives that are initiated by the Link-Layer provider
 */

#define	DL_INFO_ACK	3	/* protocol information acknowledgement */
#define	DL_BIND_ACK	4	/* protocol bind acknowledgement 	*/
#define	DL_ERROR_ACK	5	/* error acknowledgement 		*/
#define	DL_OK_ACK	6	/* success acknowledgement 		*/
#define	DL_UNITDATA_IND	8	/* unitdata receive indication 		*/
#define	DL_UDERROR_IND	9	/* unitdata receive indication 		*/


/*
 *	Primitive Non-fatal error return codes
 */

#define DLBADSAP	0	/* bad LSAP selector			 */
#define	DLACCES		2	/* inproper permissions 		 */
#define	DLOUTSTATE	3	/* Link layer interface out of state     */
#define	DLSYSERR	4	/* Unix system error 			 */


/*
 *	Subnetwork types
 */

#define	DL_CSMACD	0	/* CSMA/CD network (802.3)   */
#define	DL_TPB		1	/* Token Passing Bus (802.4) */
#define	DL_TPR		2	/* Token Ring Bus (802.5)    */
#define	DL_METRO	3	/* Metro Net (802.6)         */
#define	DL_ETHER	4	/* ETHERNET bus 	     */


/*
 *	Link-Layer service classes
 */

#define	DL_NOSERV	0	/* No service class    */
#define	DL_CLASSES	1	/* Has a service class */


/*
 *	Link-Layer current state definitions
 */

#define	DL_UNBND	0	/* LL not bound 	     */
#define	DL_WACK_B	1	/* LL waiting for bind ack   */
#define	DL_WACK_U	2	/* LL waiting for unbind ack */
#define	DL_IDLE		3	/* LL is active		     */


/*
 * The following structure definitions define the format of the 
 * streams message blocks used to define the Link-Layer Interface.
 */


/* 
 *	User Originated Primitives
 */

struct DL_info_req {
	long	PRIM_type;		/* always DL_INFO_REQ */
};


struct	DL_bind_req {
	long	PRIM_type;		/* always DL_BIND_REQ */
	long	LLC_sap;		/* the LSAP selector */
	long	GROWTH_field[2];	/* 802.2 llc type 2 fields */
};


struct DL_unbind_req {
	long	PRIM_type;		/* always DL_UNBIND_REQ */
};


struct DL_unitdata_req {
	long	PRIM_type;		/* always DL_UNITDATA_REQ */
	long	RA_length;		/* dest LSAP addr length */
	long	RA_offset;		/* dest LSAP addr offset */
	long	SERV_class;		/* service class */
	long	FILLER_field;		/* 802.2 LLC2 field */
};


/* 
 *	Provider Originated Primitives
 */

struct DL_info_ack {
	long	PRIM_type;		/* always DL_INFO_ACK */
	long	SDU_max;		/* max lsdu size */
	long	SDU_min;		/* min lsdu size */
	long	ADDR_length;		/* LSAP address length in bytes */
	long	SUBNET_type;		/* subnet type */
	long	SERV_class;		/* service class */
	long	CURRENT_state;		/* link layer state */
	long	GROWTH_field[2];	/* 802.2 LLC2 fields */
};


struct DL_bind_ack {
	long	PRIM_type;		/* always DL_BIND_ACK */
	long	LLC_sap;		/* lsap selector */
	long	ADDR_length;		/* LSAP address length in bytes */
	long	ADDR_offset;		/* LSAP address offset in the message */
	long	GROWTH_field[2];	/* 802.2 LLC2 fields */
};


struct DL_error_ack {
	long	PRIM_type;		/* always DL_ERROR_ACK */
	long	ERROR_prim;		/* primitive in error */
	long	LLC_error;		/* LLC error code */
	long	UNIX_error;		/* UNIX error code */
};


struct DL_ok_ack {
	long	PRIM_type;		/* always DL_OK_ACK */
	long	CORRECT_prim;		/* correct primitive */
};


struct DL_unitdata_ind {
	long	PRIM_type;		/* always DL_UNITDATA_IND */
	long	RA_length;		/* dest LSAP address length in bytes */
	long	RA_offset;		/* dest offset LSAP into message */
	long	LA_length;		/* src LSAP address length in bytes */
	long	LA_offset;		/* src offset LSAP into message */
	long	SERV_class;		/* service class */
};


struct DL_uderror_ind {
	long	PRIM_type;		/* always DL_UDERROR_IND */
	long	RA_length;		/* dest LSAP address length in bytes */
	long	RA_offset;		/* dest LSAP offset into msg in bytes */
	long	SERV_class;		/* service class */
	long	ERROR_type;		/* error type */
};


/*
 * 	The following is a union of all the primitives.
 */

union DL_primitives {
	long			prim_type;
	struct DL_info_req	info_req;
	struct DL_bind_req	bind_req;
	struct DL_unbind_req	unbind_req;
	struct DL_unitdata_req	data_req;
	struct DL_info_ack	info_ack;
	struct DL_bind_ack	bind_ack;
	struct DL_error_ack	error_ack;
	struct DL_ok_ack	ok_ack;
	struct DL_unitdata_ind	data_ind;
	struct DL_uderror_ind	error_ind;
};


/*
 *	Structure/union size constants.
 */

#define DL_INFO_REQ_SIZE	sizeof(struct DL_info_req)
#define DL_BIND_REQ_SIZE	sizeof(struct DL_bind_req)
#define DL_UNBIND_REQ_SIZE	sizeof(struct DL_unbind_req)
#define DL_INFO_ACK_SIZE	sizeof(struct DL_info_ack)
#define DL_BIND_ACK_SIZE	sizeof(struct DL_bind_ack)
#define DL_ERROR_ACK_SIZE	sizeof(struct DL_error_ack)
#define DL_OK_ACK_SIZE		sizeof(struct DL_ok_ack)
#define DL_UNITDATA_REQ_SIZE	sizeof(struct DL_unitdata_req)
#define DL_UNITDATA_IND_SIZE	sizeof(struct DL_unitdata_ind)
#define DL_UDERROR_IND_SIZE	sizeof(struct DL_uderror_ind)
#define DL_PRIMITIVES_SIZE	sizeof(union DL_primitives)


#endif	/* _SYS_LIHDR_H */
