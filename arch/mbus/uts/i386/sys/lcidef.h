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

#ident	"@(#)mbus:uts/i386/sys/lcidef.h	1.3.1.1"

#include <sys/mps.h>
#include <sys/fcntl.h>

typedef unsigned long    dword;
typedef unsigned short     WORD;
typedef unsigned char     BYTE;
typedef unsigned char    uchar;
typedef WORD (*FUNC_PTR)();

#define LCI_RB_PORT            0x960
#define LCI_XMIT_PORT        0x961
#define LCI_RCV_PORT        0x962

#define LCI_INIT_EID_INDEX    18    /* start of ethernet id in unsol msg */ 

#define INIT_PENDING	0x20
#define INIT_DONE	0x40

#define LCI_SERVER_PORT        0x0505
#define LCI_RCV_MAXBUFS        2
#define LCI_MAX_RX_SIZE      0x10000

#define LCI_INIT_OPCODE        31416
#define LCI_RBSEND_OPCODE    31417
#define LCI_TXDATA_OPCODE    31418
#define LCI_RXADR_OPCODE    31419
#define LCI_RXDATA_OPCODE    31420
#define LCI_RXADRDT_OPCODE    31422
#define LCI_RBRET_OPCODE    31417    /* same as RBSEND */

#define E_MPC_XMITDATA        0x11
#define E_MPC_RCVDATA1        0x21
#define E_MPC_RCVDATA2        0x22

#define E_MPC_MG_TERR        0x80

#define E_LCI_INIT        0x01
#define E_LCI_OPEN_CHAN        0x02
#define E_LCI_BAD_OPCODE	0x03


#define ICS_LOCAL_RESET 0x80
#define ICS_GCR			0x19		/* General Control Register */

typedef    struct adma_buf_str        {
                ushort             len;
                ulong             buffer_adr;
                 ulong            next_adr;
                }    adma_buf_str;

typedef    struct buf_req_str    {
                uchar            dest_adr;
                uchar            src_adr;
                uchar            msg_type;
                uchar            req_id;
                uchar            hw_preserved;
                uchar 		 msg_len1;
		uchar		 msg_len2;
                uchar            msg_len3;
                uchar            proto_id;
                uchar            xmission_ctl;
                ushort            dest_port_id;
                ushort            src_port_id;
                uchar            trans_id;
                uchar            trans_ctl;
                ushort            op_code;    /* start of LCI defined fields */
                dword            remote_adr;
                uchar            user_data[10];
                }    buf_req_str;

typedef    struct rx_buf_req_str    {
                uchar            dest_adr;
                uchar            src_adr;
                uchar            msg_type;
                uchar            req_id;
                uchar            hw_preserved;
                uchar 		 msg_len1;
		uchar		 msg_len2;
                uchar            msg_len3;
                uchar            proto_id;
                uchar            xmission_ctl;
                ushort            dest_port_id;
                ushort            src_port_id;
                uchar            trans_id;
                uchar            trans_ctl;
                ushort            op_code;    /* start of LCI defined fields */
		ushort		 buf_len;
                uchar            fpointer[4];
                uchar            user_data[8];
                }    rx_buf_req_str;

typedef    struct buf_grant_str    {
                uchar            dest_adr;
                uchar            src_adr;
                uchar            msg_type;
                uchar            req_id;
                uchar            not_used;
                uchar            slid;        /* sender liaison id */
                uchar            duty_cycle;
                uchar            lsb_length;
                }    buf_grant_str;

typedef struct unsol_msg_str    {
                uchar            dest_adr;
                uchar            src_adr;
                uchar            msg_type;
                uchar            req_id;
                uchar            proto_id;
                uchar            xmission_ctl;
                ushort            dest_port_id;
                ushort            src_port_id;
                uchar            trans_id;
                uchar            trans_ctl;
                ushort            op_code;    /* start of LCI defined fields */
                ushort            msg_len;
                dword            remote_adr;
                ushort            rb_port_id;   /* port id to send rb response*/
                ushort            data_port_id; /* port id to send data to */
                ushort            num_user_data; /* max number of user buffers 
                                            that can be sent in one message */
                ushort            size_user_data; /* max size of data that
                                                can be handled in one xfer */
                uchar            user_data[4];
    
                }    unsol_msg_str;


typedef    struct    host_adr_array_str    {
                dword        user_adr;
                ushort        user_len;
                }    host_adr_array_str;

typedef    struct rb_buf_req_str    {
                uchar            dest_adr;
                uchar            src_adr;
                uchar            msg_type;
                uchar            req_id;
                uchar            hw_preserved;
                uchar		 msg_len1;
		uchar		 msg_len2;
		uchar		 msg_len3;
                uchar            proto_id;
                uchar            xmission_ctl;
                ushort            dest_port_id;
                ushort            src_port_id;
                uchar            trans_id;
                uchar            trans_ctl;
                ushort            op_code;    /* start of LCI defined fields */
                ushort            rb_size;    
                ushort            ta_size;    /* size of transport address */
                ushort            csd_size;    /* size of cospatial data */
                ushort            ina_port;    /* ina port to give rb to */
		uchar             name_idx;
                uchar            user_data[5];
                }    rb_buf_req_str;


typedef struct unsol_init_msg_str    {
                uchar            dest_adr;
                uchar            src_adr;
                uchar            msg_type;
                uchar            req_id;
                uchar            proto_id;
                uchar            xmission_ctl;
                ushort            dest_port_id;
                ushort            src_port_id;
                uchar            trans_id;
                uchar            trans_ctl;
                ushort            op_code;    /* start of LCI defined fields */
                ushort            host_id;
                ushort            rb_port_id;   /* port id to send rb response*/
                ushort            tx_port_id; /* host transmit data port id */
                ushort            rx_port_id;   /* host receive data port id*/
                ushort            dl_port_id;   /* host delete user port id*/
                ushort            num_user_data; /* max number of user buffers 
                                            that can be sent in one message */
                ulong            size_user_data; /* max size of data that can be
                                                can be handled in one xfer */
                uchar            reserved[2];    
                }    unsol_init_msg_str;

typedef struct unsol_init_ack_msg_str    {
                uchar            dest_adr;
                uchar            src_adr;
                uchar            msg_type;
                uchar            req_id;
                uchar            proto_id;
                uchar            xmission_ctl;
                ushort            dest_port_id;
                ushort            src_port_id;
                uchar            trans_id;
                uchar            trans_ctl;
                ushort            op_code;    /* start of LCI defined fields */
                ushort            lci_version_id;
                ushort            lci_host_code;
                uchar            ethernet_adr[6];
                uchar            reserved[8];    
                }    unsol_init_ack_msg_str;

typedef    struct    erb_header_str    {
                dword            rb_adr;
                ushort            host_id;  
                ushort            rb_size;
                ushort            ta_size;
                ushort            csd_size;
                ushort            rb_ina_port;
		uchar		  name_index;
                uchar            reserved;
                }    erb_header_str;

typedef struct lci_init_str {
                ushort    opcode;
                ushort  hostid;    
                ushort  rbport;
                ushort    txport;
                ushort    rxport;
                ushort    delport;
                ushort    maxbufs;
                ulong    maxrxsz;
                }  lci_init_str;


typedef struct lci_rxadr_str {
                char    buf_len[2];
                char    buf_p[4];
                ushort    reserved;
                }    lci_rxadr_str;
