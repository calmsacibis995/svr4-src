/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_RCIMP_H
#define _SYS_RCIMP_H

/*	Copyright (c) 1986, 1987  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/sys/rcimp.h	1.3"

#ifndef RCI_PORT_ID
#define RCI_PORT_ID 0x507
#endif

/* codes returned as command completion Status */
#define RCISuccess		0x01
#define RCIFailure		0x81
#define RCINoInput		RCISuccess
#define RCIHaveInput	0x11

#define RCILocateServerC 0x01
struct RCILocateServerReq	{
	unchar Type;			/* RCILocateServerC */
	unchar Slot;			/* Client Slot ID */
	ushort Register;		/* Client ICS Register Offset */
	char Reserved[16];		/* characters to output */
};
struct RCILocateServerResp {
	unchar Status;			/* RCISuccess, RCIFailure */
	unchar HostIdL;			/* Low byte of Hostid of RCI server */
	unchar HostIdH;			/* High byte of Hostid of RCI server */
};

#define RCIEnableLineC 0x02
struct RCIEnableLineReq {
	unchar Type;			/* RCIEnableLineC */
	unchar Slot;			/* Client Slot ID */
	ushort Register;		/* Client ICS Register Offset */
	ushort LineID;			/* Line ID to enable */
	unchar Reserved[14];
};
struct RCIEnableLineResp {
	unchar Status;			/* RCISuccess, RCIFailure */
};

#define RCIDisableLineC 0x03
struct RCIDisableLineReq {
	unchar Type;			/* RCIDisableLineC */
	unchar Slot;			/* Client Slot ID */
	ushort Register;		/* Client ICS Register Offset */
	ushort LineID;			/* Line ID to Disable */
	unchar Reserved[14];
};
struct RCIDisableLineResp {
	unchar Status;			/* RCISuccess, RCIFailure */
};

#define RCITransmitC 0x04
#define RCITxMaxChar 13
struct RCITransmitReq	{
	unchar Type;			/* RCITransmitC */
	unchar Slot;			/* Client Slot ID */
	ushort Register;		/* Client ICS Register Offset */
	ushort LineID;			/* Line ID to Output On */
	unchar Count;			/* number of charaters included */
	char Data[RCITxMaxChar];/* characters to output */
};
struct RCITransmitResp {
	unchar Status;			/* RCISuccess, RCIFailure */
};

#define RCIReceiveC 0x05
struct RCIReceiveReq {
	unchar Type;			/* RCIReceiveC */
	unchar Slot;			/* Client Slot ID */
	ushort Register;		/* Client ICS Register Offset */
	ushort LineID;			/* Line ID to Input from */
	unchar Reserved[14];
};
struct RCIReceiveResp {
	unchar Status;			/* RCISuccess, RCIFailure */
	char Data;				/* Received Character */
};

#define RCISenseInputC 0x06
struct RCISenseInputReq {
	unchar Type;			/* RCISenseInputC */
	unchar Slot;			/* Client Slot ID */
	ushort Register;		/* Client ICS Register Offset */
	ushort LineID;			/* Line ID to Input from */
	unchar Reserved[14];
};
struct RCISenseInputResp {
	unchar Status;			/* RCINoInput, RCIHaveInput, RCIFailure */
	char Data;				/* Received Character, if any */
};

#define RCISetLineParametersC 0x07
struct RCISetLineParametersReq {
	unchar Type;			/* RCISetLineParametersC */
	unchar Slot;			/* Client Slot ID */
	ushort Register;		/* Client ICS Register Offset */
	ushort LineID;			/* Line ID to Set Up */
	unchar Link;			/* RCILxxxx */
	unchar InputBaudRate;	/* RCIBxxxx */
	unchar OutputBaudRate;	/* RCIBxxxx */
	unchar Mode;			/* RCIMxxxx */
	unchar Reserved[10];
};
struct RCISetLineParametersResp {
	unchar Status;			/* RCISuccess, RCIFailure */
};

#define RCIL5Bits 0x00		/* Link: five data bits */
#define RCIL6Bits 0x01		/* Link: six data bits */
#define RCIL7Bits 0x02		/* Link: seven data bits */
#define RCIL8Bits 0x03		/* Link: eight data bits */
#define RCIL1Stop 0x00		/* Link: one stop bit */
#define RCIL15Stop 0x04		/* Link: 1.5 stop bits */
#define RCIL2Stop 0x08		/* Link: two stop bits */
#define RCILNoParity 0x00	/* Link: no output parity */
#define RCILEvenParity 0x10	/* Link: even output parity */
#define RCILOddParity 0x20	/* Link: odd output parity */
#define RCILEnable 0x00		/* Link: unconditionally enable data in/out */
#define RCILCDEnable 0x40	/* Link: enable data in/out with CTS and CD */

#define RCIB50		0x01	/* BaudRate: 50 Baud */
#define RCIB75		0x02	/* BaudRate: 75 Baud */
#define RCIB110		0x03	/* BaudRate: 110 Baud */
#define RCIB134		0x04	/* BaudRate: 134.5 Baud */
#define RCIB150		0x05	/* BaudRate: 150 Baud */
#define RCIB200		0x06	/* BaudRate: 200 Baud */
#define RCIB300		0x07	/* BaudRate: 300 Baud */
#define RCIB600		0x08	/* BaudRate: 600 Baud */
#define RCIB1200	0x09	/* BaudRate: 1200 Baud */
#define RCIB1800	0x0a	/* BaudRate: 1800 Baud */
#define RCIB2400	0x0b	/* BaudRate: 2400 Baud */
#define RCIB4800	0x0c	/* BaudRate: 4800 Baud */
#define RCIB9600	0x0d	/* BaudRate: 9600 Baud */
#define RCIB19200	0x0e	/* BaudRate: 19200 Baud */
#define RCIB38400	0x0f	/* BaudRate: 38400 Baud */
#define RCIB76800	0x10	/* BaudRate: 76800 Baud */

#define RCIMErrDetect 0x00	/* Mode: Handle Input Errors */
#define RCIMErrIgnore 0x01	/* Mode: Ignore Input Errors */
#define RCIMErrNULL 0x00	/* Mode: Replace Input Error with NUL */
#define RCIMErrDiscard 0x02	/* Mode: discard input errors */
#define RCIMErrEsc 0x04		/* Mode: mark input errors with 0xff,0x00 */
#define RCIMErrTop 0x06		/* Mode: mark input errors with bit 7 on */

#endif	/* _SYS_RCIMP_H */
