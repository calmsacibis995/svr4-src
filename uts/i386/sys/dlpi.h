/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_DLPI_H
#define _SYS_DLPI_H

#ident	"@(#)head.sys:sys/dlpi.h	1.2.7.1"

/*
 * dlpi.h header for Data Link Provider Interface
 */

/*
 * This header file has encoded the values so an existing driver 
 * or user which was written with the Logical Link Interface(LLI)
 * can migrate to the DLPI interface in a binary compatible manner.
 * Any fields which require a specific format or value are flagged
 * with a comment containing the message LLI compatibility.
 */

/*
 * Primitives for Local Management Services
 */
#define DL_INFO_REQ		0x00	/* Information Req, LLI compatibility */
#define DL_INFO_ACK		0x03	/* Information Ack, LLI compatibility */
#define DL_ATTACH_REQ		0x0b	/* Attach a PPA */
#define DL_DETACH_REQ		0x0c	/* Detach a PPA */
#define DL_BIND_REQ		0x01	/* Bind dlsap address, LLI compatibility */
#define DL_BIND_ACK		0x04	/* Dlsap address bound, LLI compatibility */
#define DL_UNBIND_REQ		0x02	/* Unbind dlsap address, LLI compatibility */
#define DL_OK_ACK		0x06	/* Success acknowledgment, LLI compatibility */
#define DL_ERROR_ACK		0x05	/* Error acknowledgment, LLI compatibility */
#define DL_SUBS_BIND_REQ	0x1b	/* Bind Subsequent DLSAP address */
#define DL_SUBS_BIND_ACK	0x1c	/* Subsequent DLSAP address bound */

/*
 * Primitives used for Connectionless Service
 */
#define DL_UNITDATA_REQ		0x07	/* datagram send request, LLI compatibility */
#define DL_UNITDATA_IND		0x08	/* datagram receive indication, LLI compatibility */
#define DL_UDERROR_IND		0x09	/* datagram error indication, LLI compatibility */
#define DL_UDQOS_REQ		0x0a	/* set QOS for subsequent datagram transmissions */

/*
 * Primitives used for Connection-Oriented Service
 */
#define DL_CONNECT_REQ		0x0d	/* Connect request */
#define DL_CONNECT_IND		0x0e	/* Incoming connect indication */
#define DL_CONNECT_RES		0x0f	/* Accept previous connect indication */
#define DL_CONNECT_CON		0x10	/* Connection established */

#define DL_TOKEN_REQ		0x11	/* Passoff token request */
#define DL_TOKEN_ACK		0x12	/* Passoff token ack */

#define DL_DISCONNECT_REQ	0x13	/* Disconnect request */
#define DL_DISCONNECT_IND	0x14	/* Disconnect indication */

#define DL_RESET_REQ		0x17	/* Reset service request */
#define DL_RESET_IND		0x18	/* Incoming reset indication */
#define DL_RESET_RES		0x19	/* Complete reset processing */
#define DL_RESET_CON		0x1a	/* Reset processing complete */


/*
 * DLPI interface states
 */
#define	DL_UNATTACHED		0x04	/* PPA not attached */
#define DL_ATTACH_PENDING	0x05	/* Waiting ack of DL_ATTACH_REQ */
#define DL_DETACH_PENDING	0x06	/* Waiting ack of DL_DETACH_REQ */
#define	DL_UNBOUND		0x00	/* PPA attached, LLI compatibility */
#define	DL_BIND_PENDING		0x01	/* Waiting ack of DL_BIND_REQ, LLI compatibility */
#define	DL_UNBIND_PENDING	0x02	/* Waiting ack of DL_UNBIND_REQ, LLI compatibility */
#define	DL_IDLE			0x03	/* dlsap bound, awaiting use, LLI compatibility */
#define DL_UDQOS_PENDING	0x07	/* Waiting ack of DL_UDQOS_REQ */
#define	DL_OUTCON_PENDING	0x08	/* outgoing connection, awaiting DL_CONN_CON */
#define	DL_INCON_PENDING	0x09	/* incoming connection, awaiting DL_CONN_RES */
#define DL_CONN_RES_PENDING	0x0a	/* Waiting ack of DL_CONNECT_RES */
#define	DL_DATAXFER		0x0b	/* connection-oriented data transfer */
#define	DL_USER_RESET_PENDING	0x0c	/* user initiated reset, awaiting DL_RESET_CON */
#define	DL_PROV_RESET_PENDING	0x0d	/* provider initiated reset, awaiting DL_RESET_RES */
#define DL_RESET_RES_PENDING	0x0e	/* Waiting ack of DL_RESET_RES */
#define DL_DISCON8_PENDING	0x0f	/* Waiting ack of DL_DISC_REQ when in DL_OUTCON_PENDING */
#define DL_DISCON9_PENDING	0x10	/* Waiting ack of DL_DISC_REQ when in DL_INCON_PENDING */
#define DL_DISCON11_PENDING	0x11	/* Waiting ack of DL_DISC_REQ when in DL_DATAXFER */
#define DL_DISCON12_PENDING	0x12	/* Waiting ack of DL_DISC_REQ when in DL_USER_RESET_PENDING */
#define DL_DISCON13_PENDING	0x13	/* Waiting ack of DL_DISC_REQ when in DL_DL_PROV_RESET_PENDING */
#define DL_SUBS_BIND_PND	0x14	/* Waiting ack of DL_SUBS_BIND_REQ */


/*
 * DL_ERROR_ACK error return values
 */
#define	DL_ACCESS	0x02	/* Improper permissions for request, LLI compatibility */
#define	DL_BADADDR	0x01	/* DLSAP address in improper format or invalid */
#define	DL_BADCORR	0x05	/* Sequence number not from outstanding DL_CONN_IND */
#define	DL_BADDATA	0x06	/* User data exceeded provider limit */
#define	DL_BADPPA	0x08	/* Specified PPA was invalid */
#define DL_BADPRIM	0x09	/* Primitive received is not known by DLS provider */
#define DL_BADQOSPARAM	0x0a	/* QOS parameters contained invalid values */
#define DL_BADQOSTYPE	0x0b	/* QOS structure type is unknown or unsupported */
#define	DL_BADSAP	0x00	/* Bad LSAP selector, LLI compatibility */
#define DL_BADTOKEN	0x0c	/* Token used not associated with an active stream */
#define DL_BOUND	0x0d	/* Attempted second bind with dl_max_conind or  */
				/*	dl_conn_mgmt > 0 on same DLSAP or PPA */
#define	DL_INITFAILED	0x0e	/* Physical Link initialization failed */
#define DL_NOADDR	0x0f	/* Provider couldn't allocate alternate address */
#define	DL_NOTINIT	0x10	/* Physical Link not initialized */
#define	DL_OUTSTATE	0x03	/* Primitive issued in improper state, LLI compatibility */
#define	DL_SYSERR	0x04	/* UNIX system error occurred, LLI compatibility */
#define	DL_UNSUPPORTED	0x07	/* Requested service not supplied by provider */
#define DL_UNDELIVERABLE 0x11	/* Previous data unit could not be delivered */
#define DL_NOTSUPPORTED  0x12	/* Primitive is known but not supported by DLS provider */


/*
 * DLPI media types supported
 */
#define	DL_CSMACD	0x0	/* IEEE 802.3 CSMA/CD network, LLI Compatibility */
#define	DL_TPB		0x1	/* IEEE 802.4 Token Passing Bus, LLI Compatibility */
#define	DL_TPR		0x2	/* IEEE 802.5 Token Passing Ring, LLI Compatibility */
#define	DL_METRO	0x3	/* IEEE 802.6 Metro Net, LLI Compatibility */
#define	DL_ETHER	0x4	/* Ethernet Bus, LLI Compatibility */
#define	DL_HDLC		0x05	/* ISO HDLC protocol support, bit synchronous */
#define DL_CHAR		0x06	/* Character Synchronous protocol support, eg BISYNC */
#define	DL_CTCA		0x07	/* IBM Channel-to-Channel Adapter */


/*
 * DLPI provider service supported.
 * These must be allowed to be bitwise-OR for dl_service_mode in
 * DL_INFO_ACK.
 */
#define DL_CODLS	0x01	/* support connection-oriented service */
#define DL_CLDLS	0x02	/* support connectionless data link service */
#define	DL_CL_ETHER	0x04	/* support Ethernet service class */


/*
 * DLPI provider style.
 * The DLPI provider style which determines whether a provider
 * requires a DL_ATTACH_REQ to inform the provider which PPA
 * user messages should be sent/received on.
 */
#define	DL_STYLE1	0x0500	/* PPA is implicitly bound by open(2) */
#define	DL_STYLE2	0x0501	/* PPA must be explicitly bound via DL_ATTACH_REQ */

/*
 * DLPI flag for MORE IDU's for a single SDU
 */
#define	DL_MORE		0x08

/*
 * DLPI Originator for Disconnect and Resets
 */
#define	DL_PROVIDER	0x0700
#define	DL_USER		0x0701

/*
 * DLPI Disconnect Reasons
 */
#define	DL_CONREJ_DEST_UNKNOWN			0x0800
#define	DL_CONREJ_DEST_UNREACH_PERMANENT	0x0801
#define	DL_CONREJ_DEST_UNREACH_TRANSIENT	0x0802
#define	DL_CONREJ_QOS_UNAVAIL_PERMANENT		0x0803
#define	DL_CONREJ_QOS_UNAVAIL_TRANSIENT		0x0804
#define	DL_CONREJ_PERMANENT_COND		0x0805
#define	DL_CONREJ_TRANSIENT_COND		0x0806
#define	DL_DISC_ABNORMAL_CONDITION		0x0807
#define	DL_DISC_NORMAL_CONDITION		0x0808
#define DL_DISC_PERMANENT_CONDITION		0x0809
#define	DL_DISC_TRANSIENT_CONDITION		0x080a
#define	DL_DISC_UNSPECIFIED			0x080b

/*
 * DLPI Reset Reasons
 */
#define	DL_RESET_FLOW_CONTROL	0x0900
#define	DL_RESET_LINK_ERROR	0x0901
#define	DL_RESET_RESYNCH	0x0902

/*
 * DLPI Quality Of Service definition for use in QOS structure definitions.
 * The QOS structures are used in connection establishment, DL_INFO_ACK,
 * and setting connectionless QOS values.
 */

/*
 * Throughput
 *
 * This parameter is specified for both directions.
 */
typedef struct {
		long	dl_target_value;	/* desired bits/second desired */
		long	dl_accept_value;	/* min. acceptable bits/second */
} dl_through_t;

/*
 * transit delay specification 
 *
 * This parameter is specified for both directions.
 * expressed in milliseconds assuming a DLSDU size of 128 octets.
 * The scaling of the value to the current DLSDU size is provider dependent.
 */
typedef struct {
		long	dl_target_value;	/* desired value of service */
		long	dl_accept_value;	/* min. acceptable value of service */
} dl_transdelay_t;

/*
 * priority specification
 * priority range is 0-100, with 0 being highest value.
 */
typedef struct {
		long	dl_min;
		long	dl_max;
} dl_priority_t;


/*
 * protection specification
 *
 */
#define DL_NONE			0x0B01	/* no protection supplied */
#define DL_MONITOR		0x0B02	/* protection against passive monitoring */
#define DL_MAXIMUM		0x0B03	/* protection against modification, replay, */
					/* addition, or deletion */

typedef struct {
		long	dl_min;
		long	dl_max;
} dl_protect_t;


/*
 * Resilience specification
 * probabilities are scaled by a factor of 10,000 with a time interval
 * of 10,000 seconds.
 */
typedef struct {
		long	dl_disc_prob;	/* probability of provider init DISC */
		long	dl_reset_prob;	/* probability of provider init RESET */
} dl_resilience_t;


/*
 * QOS type definition to be used for negotiation with the
 * remote end of a connection, or a connectionless unitdata request.
 * There are two type definitions to handle the negotiation 
 * process at connection establishment. The typedef dl_qos_neg_t
 * is used to present a range for parameters. This is used
 * in the DL_CONNECT_REQ and DL_CONNECT_IND messages. The typedef
 * dl_qos_sel_t is used to select a specific value for the QOS
 * parameters. This is used in the DL_CONNECT_RES, DL_CONNECT_CON,
 * and DL_INFO_ACK messages to define the selected QOS parameters
 * for a connection.
 *
 * NOTE
 *	A DataLink provider which has unknown values for any of the fields
 *	will use a value of DL_UNKNOWN for all values in the fields.
 *
 * NOTE
 *	A QOS parameter value of DL_QOS_DONT_CARE informs the DLS
 *	provider the user requesting this value doesn't care 
 *	what the QOS parameter is set to. This value becomes the
 *	least possible value in the range of QOS parameters.
 *	The order of the QOS parameter range is then:
 *
 *		DL_QOS_DONT_CARE < 0 < MAXIMUM QOS VALUE
 */
#define DL_UNKNOWN		-1
#define DL_QOS_DONT_CARE	-2

/*
 * Every QOS structure has the first 4 bytes containing a type
 * field, denoting the definition of the rest of the structure.
 * This is used in the same manner has the dl_primitive variable
 * is in messages.
 *
 * The following list is the defined QOS structure type values and structures.
 */
#define DL_QOS_CO_RANGE1	0x0101
#define DL_QOS_CO_SEL1		0x0102
#define DL_QOS_CL_RANGE1	0x0103
#define DL_QOS_CL_SEL1		0x0104

typedef struct {
		ulong		dl_qos_type;
		dl_through_t	dl_rcv_throughput;
		dl_transdelay_t	dl_rcv_trans_delay;
		dl_through_t	dl_xmt_throughput;
		dl_transdelay_t	dl_xmt_trans_delay;
		dl_priority_t	dl_priority;
		dl_protect_t	dl_protection;
		long		dl_residual_error;
		dl_resilience_t	dl_resilience;
}	dl_qos_co_range1_t;

typedef struct {
		ulong		dl_qos_type;
		long		dl_rcv_throughput;
		long		dl_rcv_trans_delay;
		long		dl_xmt_throughput;
		long		dl_xmt_trans_delay;
		long		dl_priority;
		long		dl_protection;
		long		dl_residual_error;
		dl_resilience_t	dl_resilience;
}	dl_qos_co_sel1_t;

typedef struct {
		ulong		dl_qos_type;
		dl_transdelay_t	dl_trans_delay;
		dl_priority_t	dl_priority;
		dl_protect_t	dl_protection;
		long		dl_residual_error;
}	dl_qos_cl_range1_t;

typedef struct {
		ulong		dl_qos_type;
		long		dl_trans_delay;
		long		dl_priority;
		long		dl_protection;
		long		dl_residual_error;
}	dl_qos_cl_sel1_t;

/*
 * DLPI interface primitive definitions.
 *
 * Each primitive is sent as a stream message.  It is possible that
 * the messages may be viewed as a sequence of bytes that have the
 * following form without any padding. The structure definition
 * of the following messages may have to change depending on the
 * underlying hardware architecture and crossing of a hardware
 * boundary with a different hardware architecture.
 *
 * Fields in the primitives having a name of the form
 * dl_reserved cannot be used and have the value of
 * binary zero, no bits turned on.
 *
 * Each message has the name defined followed by the
 * stream message type (M_PROTO, M_PCPROTO, M_DATA)
 */

/*
 *	LOCAL MANAGEMENT SERVICE PRIMITIVES
 */

/*
 * DL_INFO_REQ, M_PCPROTO type
 */
typedef struct {
	ulong	dl_primitive;
} dl_info_req_t;

/*
 * DL_INFO_ACK, M_PCPROTO type
 */
typedef struct {
	ulong		dl_primitive;
	ulong		dl_max_sdu;
	ulong		dl_min_sdu;
	ulong		dl_addr_length;
	ulong		dl_mac_type;
	ulong		dl_reserved;
	ulong		dl_current_state;
	ulong		dl_reserved2;
	ulong		dl_service_mode;
	ulong		dl_qos_length;
	ulong		dl_qos_offset;
	ulong		dl_qos_range_length;
	ulong		dl_qos_range_offset;
	long		dl_provider_style;
	ulong 		dl_addr_offset;
	ulong		dl_growth;
} dl_info_ack_t;

/*
 * DL_ATTACH_REQ, M_PROTO type
 */
typedef struct {
	ulong		dl_primitive;
	ulong		dl_ppa;
} dl_attach_req_t;

/*
 * DL_DETACH_REQ, M_PROTO type
 */
typedef struct {
	ulong	dl_primitive;
} dl_detach_req_t;

/*
 * DL_BIND_REQ, M_PROTO type
 */
typedef struct {
	ulong	dl_primitive;
	ulong	dl_sap;
	ulong	dl_max_conind;
	ushort	dl_service_mode;
	ushort	dl_conn_mgmt;
} dl_bind_req_t;

/*
 * DL_BIND_ACK, M_PCPROTO type
 */
typedef struct {
	ulong	dl_primitive;
	ulong	dl_sap;
	ulong	dl_addr_length;
	ulong	dl_addr_offset;
	ulong	dl_max_conind;
	ulong	dl_growth;
} dl_bind_ack_t;

/*
 * DL_SUBS_BIND_REQ, M_PROTO type
 */
typedef struct {
	ulong	dl_primitive;
	ulong 	dl_subs_sap_offset;
	ulong	dl_subs_sap_len;
} dl_subs_bind_req_t;

/*
 * DL_SUBS_BIND_ACK, M_PCPROTO type
 */
typedef struct {
	ulong dl_primitive;
	ulong dl_subs_sap_offset;
	ulong dl_subs_sap_len;
} dl_subs_bind_ack_t;

/*
 * DL_UNBIND_REQ, M_PROTO type
 */
typedef struct {
	ulong	dl_primitive;
} dl_unbind_req_t;

/*
 * DL_OK_ACK, M_PCPROTO type
 */
typedef struct {
	ulong	dl_primitive;
	ulong	dl_correct_primitive;
} dl_ok_ack_t;

/*
 * DL_ERROR_ACK, M_PCPROTO type
 */
typedef struct {
	ulong	dl_primitive;
	ulong	dl_error_primitive;
	ulong	dl_errno;
	ulong	dl_unix_errno;
} dl_error_ack_t;


/*
 *	CONNECTION-ORIENTED SERVICE PRIMITIVES
 */

/*
 * DL_CONNECT_REQ, M_PROTO type
 */
typedef struct {
	ulong			dl_primitive;
	ulong			dl_dest_addr_length;
	ulong			dl_dest_addr_offset;
	ulong			dl_qos_length;
	ulong			dl_qos_offset;
	ulong			dl_growth;
} dl_connect_req_t;

/*
 * DL_CONNECT_IND, M_PROTO type
 */
typedef struct {
	ulong			dl_primitive;
	ulong			dl_correlation;
	ulong			dl_called_addr_length;
	ulong			dl_called_addr_offset;
	ulong			dl_calling_addr_length;
	ulong			dl_calling_addr_offset;
	ulong			dl_qos_length;
	ulong			dl_qos_offset;
	ulong			dl_growth;
} dl_connect_ind_t;

/*
 * DL_CONNECT_RES, M_PROTO type
 */
typedef struct {
	ulong			dl_primitive;
	ulong			dl_correlation;
	ulong			dl_resp_token;
	ulong			dl_qos_length;
	ulong			dl_qos_offset;
	ulong			dl_growth;
} dl_connect_res_t;

/*
 * DL_CONNECT_CON, M_PROTO type
 */
typedef struct {
	ulong			dl_primitive;
	ulong			dl_resp_addr_length;
	ulong			dl_resp_addr_offset;
	ulong			dl_qos_length;
	ulong			dl_qos_offset;
	ulong			dl_growth;
	
} dl_connect_con_t;

/*
 * DL_TOKEN_REQ, M_PCPROTO type
 */
typedef struct {
	ulong		dl_primitive;
} dl_token_req_t;

/*
 * DL_TOKEN_ACK, M_PCPROTO type
 */
typedef struct {
	ulong		dl_primitive;
	ulong		dl_token;
}dl_token_ack_t;

/*
 * DL_DISCONNECT_REQ, M_PROTO type
 */
typedef struct {
	ulong		dl_primitive;
	ulong		dl_reason;
	ulong		dl_correlation;
} dl_disconnect_req_t;

/*
 * DL_DISCONNECT_IND, M_PROTO type
 */
typedef struct {
	ulong	dl_primitive;
	ulong	dl_originator;
	ulong	dl_reason;
	ulong	dl_correlation;
} dl_disconnect_ind_t;

/*
 * DL_RESET_REQ, M_PROTO type
 */
typedef struct {
	ulong	dl_primitive;
} dl_reset_req_t;

/*
 * DL_RESET_IND, M_PROTO type
 */
typedef struct {
	ulong	dl_primitive;
	ulong	dl_originator;
	ulong	dl_reason;
} dl_reset_ind_t;

/*
 * DL_RESET_RES, M_PROTO type
 */
typedef struct {
	ulong	dl_primitive;
} dl_reset_res_t;

/*
 * DL_RESET_CON, M_PROTO type
 */
typedef struct {
	ulong	dl_primitive;
} dl_reset_con_t;


/*
 *	CONNECTIONLESS SERVICE PRIMITIVES
 */

/*
 * DL_UNITDATA_REQ, M_PROTO type, with M_DATA block(s)
 */
typedef struct {
	ulong	dl_primitive;
	ulong	dl_dest_addr_length;
	ulong	dl_dest_addr_offset;
	ulong	dl_reserved[2];
} dl_unitdata_req_t;

/*
 * DL_UNITDATA_IND, M_PROTO type, with M_DATA block(s)
 */
typedef struct {
	ulong	dl_primitive;
	ulong	dl_dest_addr_length;
	ulong	dl_dest_addr_offset;
	ulong	dl_src_addr_length;
	ulong	dl_src_addr_offset;
	ulong	dl_reserved;
} dl_unitdata_ind_t;

/*
 * DL_UDERROR_IND, M_PROTO type
 * 	(or M_PCPROTO type if LLI-based provider)
 */
typedef struct {
	ulong	dl_primitive;
	ulong	dl_dest_addr_length;
	ulong	dl_dest_addr_offset;
	ulong	dl_reserved;
	ulong	dl_errno;
} dl_uderror_ind_t;

/*
 * DL_UDQOS_REQ, M_PROTO type
 */
typedef struct {
	ulong	dl_primitive;
	ulong	dl_qos_length;
	ulong	dl_qos_offset;
} dl_udqos_req_t;

union DL_primitives {
	ulong			dl_primitive;
	dl_info_req_t		info_req;
	dl_info_ack_t		info_ack;
	dl_attach_req_t		attach_req;
	dl_detach_req_t		detach_req;
	dl_bind_req_t		bind_req;
	dl_bind_ack_t		bind_ack;
	dl_unbind_req_t		unbind_req;
	dl_subs_bind_req_t	subs_bind_req;
	dl_subs_bind_ack_t	subs_bind_ack;
	dl_ok_ack_t		ok_ack;
	dl_error_ack_t		error_ack;
	dl_connect_req_t	connect_req;
	dl_connect_ind_t	connect_ind;
	dl_connect_res_t	connect_res;
	dl_connect_con_t	connect_con;
	dl_token_req_t		token_req;
	dl_token_ack_t		token_ack;
	dl_disconnect_req_t	disconnect_req;
	dl_disconnect_ind_t	disconnect_ind;
	dl_reset_req_t		reset_req;
	dl_reset_ind_t		reset_ind;
	dl_reset_res_t		reset_res;
	dl_reset_con_t		reset_con;
	dl_unitdata_req_t	unitdata_req;
	dl_unitdata_ind_t	unitdata_ind;
	dl_uderror_ind_t	uderror_ind;
	dl_udqos_req_t		udqos_req;
};

#define	DL_INFO_REQ_SIZE	sizeof(dl_info_req_t)
#define	DL_INFO_ACK_SIZE	sizeof(dl_info_ack_t)
#define	DL_ATTACH_REQ_SIZE	sizeof(dl_attach_req_t)
#define	DL_DETACH_REQ_SIZE	sizeof(dl_detach_req_t)
#define	DL_BIND_REQ_SIZE	sizeof(dl_bind_req_t)
#define	DL_BIND_ACK_SIZE	sizeof(dl_bind_ack_t)
#define	DL_UNBIND_REQ_SIZE	sizeof(dl_unbind_req_t)
#define DL_SUBS_BIND_REQ_SIZE	sizeof(dl_subs_bind_req_t)
#define DL_SUBS_BIND_ACK_SIZE	sizeof(dl_subs_bind_ack_t)
#define	DL_OK_ACK_SIZE		sizeof(dl_ok_ack_t)
#define	DL_ERROR_ACK_SIZE	sizeof(dl_error_ack_t)
#define	DL_CONNECT_REQ_SIZE	sizeof(dl_connect_req_t)
#define	DL_CONNECT_IND_SIZE	sizeof(dl_connect_ind_t)
#define	DL_CONNECT_RES_SIZE	sizeof(dl_connect_res_t)
#define	DL_CONNECT_CON_SIZE	sizeof(dl_connect_con_t)
#define	DL_TOKEN_REQ_SIZE	sizeof(dl_token_req_t)
#define	DL_TOKEN_ACK_SIZE	sizeof(dl_token_ack_t)
#define	DL_DISCONNECT_REQ_SIZE	sizeof(dl_disconnect_req_t)
#define	DL_DISCONNECT_IND_SIZE	sizeof(dl_disconnect_ind_t)
#define	DL_RESET_REQ_SIZE	sizeof(dl_reset_req_t)
#define	DL_RESET_IND_SIZE	sizeof(dl_reset_ind_t)
#define	DL_RESET_RES_SIZE	sizeof(dl_reset_res_t)
#define	DL_RESET_CON_SIZE	sizeof(dl_reset_con_t)
#define	DL_UNITDATA_REQ_SIZE	sizeof(dl_unitdata_req_t)
#define	DL_UNITDATA_IND_SIZE	sizeof(dl_unitdata_ind_t)
#define	DL_UDERROR_IND_SIZE	sizeof(dl_uderror_ind_t)
#define	DL_UDQOS_REQ_SIZE	sizeof(dl_udqos_req_t)
 
#endif /* _SYS_DLPI_H */
