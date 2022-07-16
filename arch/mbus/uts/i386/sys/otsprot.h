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

#ident	"@(#)mbus:uts/i386/sys/otsprot.h	1.3"

/*
** ABSTRACT:	OTS protocol structures and defines
**
** MODIFICATIONS:
*/

#define USEND_SIZE		18	/* UNSOL data send size */
#define OTS_VERSION		1	/* OTS version supported by driver */
#define TP_TYPE_SIZE		12	/* transport provider type name size */
#define TP_TYPE		"OTS        "	/* 11 characters in length */

/* OTS Protocol control port commands */

#define O_CONNECT		0x01	/* connect request */
#define O_ACCEPT		0x02	/* accept connection request */
#define O_ACCEPT_CANCEL		0x03	/* cancel accept request */
#define O_DISCONNECT		0x11	/* disconnect request */
#define O_ORD_RELEASE		0x12	/* orderly release request */
#define O_DATAGRAM		0x21	/* send datagram request */
#define O_DATAGRAM_OPEN		0x22	/* accept datagrams request */
#define O_DATAGRAM_CANCEL	0x23	/* cancel accept datagram request */
#define O_GET_INFO		0x31	/* get network controller info */
#define O_NEGOTIATE_VER		0x32	/* negotiate OTS protocol version */

/* OTS Protocol control port responses */

#define O_CONNECT_RSP		0x81	/* connect response */
#define O_ACCEPT_RSP		0x82	/* accept connection response */
#define O_ACCEPT_CANCEL_RSP	0x83	/* cancel accept response */
#define O_DISCONNECT_RSP	0x91	/* disconnect response */
#define O_ORD_RELEASE_RSP	0x92	/* orderly release response */
#define O_DATAGRAM_RSP		0xA1	/* send datagram response */
#define O_DATAGRAM_OPEN_RSP	0xA2	/* accept datagrams response */
#define O_DATAGRAM_CANCEL_RSP	0xA3	/* cancel accept datagram response */
#define O_GET_INFO_RSP		0xB1	/* get network controller info */
#define O_NEGOTIATE_VER_RSP	0xB2	/* negotiate OTS protocol version */

#define O_CONTROL_RESPONSE	0x80	/* command bit flagging response */

/* OTS Protocol control port responses */

#define O_DATA			0x01	/* partial data message */
#define O_EOM_DATA		0x02	/* complete or last data message */
#define O_EXPEDITED_DATA	0x11	/* expedited message */
#define O_NOEOM_EXPEDITED_DATA	0x12	/* partial expedited message */
#define O_TRANSACTION_RSP	0x81	/* transaction response */

/* OTS Protocol message structures */

struct o_connect
{
	uchar	command;	/* command code: O_CONNECT */
	uchar	version;	/* protocol version number */
	ushort	qlen;		/* unsolicited message queue length */
	ushort	buffer;		/* recommended solicited buffer size */
	ushort	lport;		/* local data port */
	ushort	options;	/* transport provider options length */
	ushort	laddr;		/* local network address length */
	ushort	raddr;		/* remote network address length */
	ushort	userdata;	/* user data length */
};

struct o_connect_rsp
{
	uchar	command;	/* command code: O_CONNECT_RSP */
	uchar	errcode;	/* error code */
	ushort	qlen;		/* unsolicited message queue length */
	ushort	buffer;		/* recommended solicited buffer size */
	ushort	lport;		/* local data port */
	ushort	rport;		/* remote data port */
	ushort	options;	/* transport provider options length */
	ushort	raddr;		/* remote network address length */
	ushort	userdata;	/* user data length */
};

struct o_accept
{
	uchar	command;	/* command code: O_ACCEPT */
	uchar	version;	/* protocol version number */
	ushort	options;	/* transport provider options length */
	ushort	laddr;		/* local network address length */
	ushort	raddr;		/* remote network address length */
};

struct o_accept_rsp
{
	uchar	command;	/* command code: O_ACCEPT_RSP */
	uchar	errcode;	/* error code */
	ushort	options;	/* transport provider options length */
};

struct o_cancel_accept
{
	uchar	command;	/* command code: O_CANCEL_ACCEPT */
	uchar	reason;		/* the reason for the request */
	ushort	laddr;		/* local network address length */
};

struct o_cancel_accept_rsp
{
	uchar	command;	/* command code: O_CANCEL_ACCEPT_RSP */
	uchar	errcode;	/* error code */
};

struct o_negotiate_ver
{
	uchar	command;	/* command code: O_NEGOTIATE_VER */
	uchar	reserved;	/* unused */
	ushort	list;		/* version list length */
};

struct o_negotiate_ver_rsp
{
	uchar	command;	/* command code: O_NEGOTIATE_VER_RSP */
	uchar	errcode;	/* error code */
	uchar	version;	/* negotiated version */
};

struct o_disconnect
{
	uchar	command;	/* command code: O_DISCONNECT */
	uchar	reason;		/* the reason for the request */
	ushort	lport;		/* local data port */
	ushort	rport;		/* remote data port */
	ushort	userdata;	/* user data length */
};

struct o_disconnect_rsp
{
	uchar	command;	/* command code: O_DISCONNECT_RSP */
	uchar	errcode;	/* error code */
	ushort	lport;		/* local data port */
	ushort	rport;		/* remote data port */
};

struct o_release
{
	uchar	command;	/* command code: O_RELEASE */
	uchar	reason;		/* the reason for the request */
	ushort	lport;		/* local data port */
	ushort	rport;		/* remote data port */
};

struct o_release_rsp
{
	uchar	command;	/* command code: O_RELEASE_RSP */
	uchar	errcode;	/* error code */
	ushort	lport;		/* local data port */
	ushort	rport;		/* remote data port */
};

struct o_datagram
{
	uchar	command;	/* command code: O_DATAGRAM */
	uchar	version;	/* protocol version number */
	ushort	userdata;	/* user data length */
	ushort	options;	/* transport provider options length */
	ushort	laddr;		/* local network address length */
	ushort	raddr;		/* remote network address length */
	ushort	reserved[3];	/* reserved for future use by OTS */
};

struct o_datagram_rsp
{
	uchar	command;	/* command code: O_DATAGRAM_RSP */
	uchar	errcode;	/* error code */
};

struct o_datagram_open
{
	uchar	command;	/* command code: O_DATAGRAM_OPEN */
	uchar	version;	/* protocol version number */
	ushort	options;	/* transport provider options length */
	ushort	laddr;		/* local network address length */
	ushort	raddr;		/* remote network address length */
};

struct o_datagram_open_rsp
{
	uchar	command;	/* command code: O_DATAGRAM_OPEN_RSP */
	uchar	errcode;	/* error code */
	ushort	options;	/* transport provider options length */
};

struct o_datagram_cancel
{
	uchar	command;	/* command code: O_DATAGRAM_CANCEL */
	uchar	reason;		/* reason for the request */
	ushort	laddr;		/* local network address length */
};

struct o_datagram_cancel_rsp
{
	uchar	command;	/* command code: O_DATAGRAM_CANCEL_RSP */
	uchar	errcode;	/* error code */
};

struct o_get_info
{
	uchar	command;	/* command code: O_GET_INFO */
	uchar	version;	/* protocol version number */
};

struct o_get_info_rsp
{
	uchar	command;	/* command code: O_GET_INFO_RSP */
	uchar	errcode;	/* error code */
	ushort	data;		/* standard response data length */
	ushort	options;	/* transport provider options length */
};

struct o_solicited_data
{
	char	data[14];	/* user data */
	uchar	dlen;		/* user data length (always zero) */
	uchar	type;		/* O_(EOM_)DATA, O_EXPEDITED_DATA */
};

struct o_unsolicited_data
{
	char	data[USEND_SIZE];	/* user data */
	uchar	dlen;			/* user data length */
	uchar	type;			/* O_(EOM_)DATA, O_EXPEDITED_DATA */
};

struct o_transaction_rsp
{
	char	reserved[16];	/* unused */
	ushort	qlen;		/* free slots left in local input queue */
	uchar	errcode;	/* error code */
	uchar	type;		/* data message type */
};

struct get_info_data
{
	ulong	netaddr;	/* max size of a network address */
	ulong	options;	/* max size of protocol specific options */
	ulong	tsdu_size;	/* Transport Service Data Unit size */
	ulong	etsdu_size;	/* Expedited Transport Service Data Unit size */
	ulong	connect;	/* max size of user data in a connect request */
	ulong	discon;		/* max size of user data in a discon request */
	ulong	datagram;	/* max datagram size */
	ulong	service;	/* services supported by transport provider */
	char	tp_type[TP_TYPE_SIZE];	/* transport provider type */
};

union ots_messages
{
	uchar command;
	struct o_connect co;
	struct o_connect_rsp cor;
	struct o_accept ac;
	struct o_accept_rsp acr;
	struct o_cancel_accept ca;
	struct o_cancel_accept_rsp car;
	struct o_negotiate_ver nv;
	struct o_negotiate_ver_rsp nvr;
	struct o_disconnect di;
	struct o_disconnect_rsp dir;
	struct o_release re;
	struct o_release_rsp rer;
	struct o_datagram da;
	struct o_datagram_rsp dar;
	struct o_datagram_open dao;
	struct o_datagram_open_rsp daor;
	struct o_datagram_cancel dac;
	struct o_datagram_cancel_rsp dacr;
	struct o_get_info gi;
	struct o_get_info_rsp gir;
	struct o_solicited_data sd;
	struct o_unsolicited_data ud;
	struct o_transaction_rsp tr;
};

typedef union ots_messages	 otsstr;

struct cbreq_info
{
	otsstr		control;
	mb2socid_t	rsoc;
	connect		*ce;
	int		tid;
};

struct dbreq_info
{
	connect		*de;
	uchar		type;
};

union control_header
{
	struct T_unitdata_ind	ud;
	struct cbreq_info	breq;
};

union data_header
{
	union qm		q;
	struct T_exdata_ind	ex;
	struct dbreq_info	breq;
};

extern mblk_t *O_alloc_dsolbuf();
extern mblk_t *O_alloc_csolbuf();
