/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1986, 1987  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	
#ifndef _SYS_ATCSMP_H
#define _SYS_ATCSMP_H

#ident	"@(#)mbus:uts/i386/sys/atcsmp.h	1.3.1.1"

/* codes returned as command completion Status */
#define ATCSSuccess 0x00
#define ATCSFailure 0x80

#define ATCSSendImmediateC 0x01
#define ATCSSIMaxChar 18
struct ATCSSendImmediateReq	{
	unchar Type;		/* ATCSSendImmediateC */
	unchar Count;		/* number of charaters included */
	char Data[ATCSSIMaxChar];/* characters to output */
};
struct ATCSSendImmediateResp {
	unchar Type;		/* ATCSSendImmediateC */
	unchar Status;		/* 00-success, 80-error */
	unchar reserved01;
	unchar reserved02;
};

#define ATCSBlockSendC 0x02
struct ATCSBlockSendReq	{
	unchar Type;		/* ATCSBlockSendC */
	unchar reserved01[3];
};
struct ATCSBlockSendResp {
	unchar Type;		/* ATCSBlockSendC */
	unchar Status;		/* 00-success, 80-error */
	unchar reserved01[2];
};

#define ATCSReceiveC 0x03
struct ATCSReceiveReq {
	unchar Type;		/* ATCSReceiveC */
	unchar reserved01;
	ushort MaxCount;	/* maximum characters to read */
	ushort MinCount;	/* minimum characters to read */
	unchar Timeout;		/* 10ms ticks to wait for MinCount */
	unchar IdleTime;	/* 10ms ticks to wait between chars */
	unchar NumReplySlots;	/* unsolicited flow control */
	unchar reserved02[3];
};
struct ATCSReceiveResp {
	unchar Type;		/* ATCSReceiveC */
	unchar Status;		/* 00-success, 80-error */
	unchar reserved[2];
};
/* control only response contains the characters */
#define ATCSRSmallChars 18
struct ATCSReceiveUResp {
	unchar Type;		/* ATCSReceiveC */
	unchar Count;		/* number of bytes herein */
	char Data[ATCSRSmallChars];		/* received chars */
};

#define ATCSWaitForSendCompleteC 0x04
struct ATCSWaitForSendCompleteReq {
	unchar Type;		/* ATCSWaitForSendCompleteC */
	unchar reserved01;
	ushort LowWaterMark;	/* count to send reply for */
};
struct ATCSWaitForSendCompleteResp {
	unchar Type;		/* ATCSWaitForSendCompleteC */
	unchar Status;		/* 00-success, 80-error */
	unchar reserved01[2];
};

#define ATCSOutputControlC 0x05
struct ATCSOutputControlReq {
	unchar Type;		/* ATCSOutputControlC */
	unchar reserved01;
	unchar Command;		/* ATCSOCxxxx */
	unchar reserved02;
};
#define ATCSOCFlushOutput 0x00
#define ATCSOCSuspendOutput 0x01
#define ATCSOCResumeOutput 0x02
struct ATCSOutputControlResp {
	unchar Type;		/* ATCSOutputControlC */
	unchar Status;		/* 00-success, 80-error */
	unchar reserved[2];
};

#define ATCSFlushInputC	0x06
struct ATCSFlushInputReq {
	unchar Type;		/* ATCSFlushInputC */
	unchar reserved01[3];
};
struct ATCSFlushInputResp {
	unchar Type;		/* ATCSFlushInputC */
	unchar Status;		/* 00-success, 80-error */
	unchar reserved01[2];
};

#define ATCSSetModemSignalC 0x07
struct ATCSSetModemSignalReq {
	unchar Type;		/* ATCSSetModemSignalC */
	unchar reserved01;
	unchar RTSCommand;	/* ATCSSMSxxxx */
	unchar DTRCommand;	/* ATCSSMSxxxx */
	unchar BreakCommand;	/* ATCSSMSxxxx */
	unchar reserved02[3];
};
#define ATCSSMSNoChange 0x00
#define ATCSSMSSet 0x01
#define ATCSSMSClear 0x02
struct ATCSSetModemSignalResp {
	unchar Type;		/* ATCSSetModemSignalC */
	unchar Status;		/* 00-success, 80-error */
	unchar reserved01[2];
};

#define ATCSSetLineParametersC 0x08
struct ATCSSetLineParametersReq {
	unchar Type;		/* ATCSSetLineParametersC */
	unchar reserved01;
	unchar Link;		/* ATCSLxxxx */
	unchar InputBaudRate;	/* ATCSBxxxx */
	unchar OutputBaudRate;	/* ATCSBxxxx */
	unchar Mode0;		/* ATCSMxxxx */
	unchar SpecialChar[4];	/* special characters */
	unchar Mode1;		/* ATCSMxxxx */
	unchar reserved02;
};
#define ATCSL5Bits 0x00		/* Link: five data bits */
#define ATCSL6Bits 0x01		/* Link: six data bits */
#define ATCSL7Bits 0x02		/* Link: seven data bits */
#define ATCSL8Bits 0x03		/* Link: eight data bits */
#define ATCSL1Stop 0x00		/* Link: one stop bit */
#define ATCSL15Stop 0x04	/* Link: 1.5 stop bits */
#define ATCSL2Stop 0x08		/* Link: two stop bits */
#define ATCSLNoParity 0x00	/* Link: no output parity */
#define ATCSLEvenParity 0x10	/* Link: even output parity */
#define ATCSLOddParity 0x20	/* Link: odd output parity */
#define ATCSLEnable 0x00	/* Link: unconditionally enable data in/out */
#define ATCSLCDEnable 0x40	/* Link: enable data in/out with CTS and CD */
#define ATCSBHANG 0x00
#define ATCSB50 0x01
#define ATCSB75 0x02
#define ATCSB110 0x03
#define ATCSB134_5 0x04
#define ATCSB150 0x05
#define ATCSB200 0x06
#define ATCSB300 0x07
#define ATCSB600 0x08
#define ATCSB1200 0x09
#define ATCSB1800 0x0a
#define ATCSB2400 0x0b
#define ATCSB4800 0x0c
#define ATCSB9600 0x0d
#define ATCSB19200 0x0e
#define ATCSB38400 0x0f
#define ATCSB76800 0x10
#define ATCSMXON 0x01		/* Mode0: output XON/XOFF process enabled */
#define ATCSMStrip 0x02		/* Mode0: strip bit 7 off incoming chars */
#define ATCSMResumeMode1 0x00	/* Mode0: restart XOFF only with XON */
#define ATCSMResumeMode2 0x04	/* Mode0: restart XOFF with amy char */
#define ATCSMSpCharEnable 0x08	/* Mode0: Enable special character recognition */
#define ATCSMInErrNULL 0x00	/* Mode0: mark input errors with NULLs */
#define ATCSMInErrDiscard 0x10	/* Mode0: discard input errors */
#define ATCSMInErrEsc 0x20	/* Mode0: mark input errors with 0xff,0x00 */
#define ATCSMInErrTop 0x30	/* Mode0: mark input errors with bit 7 on */
#define ATCSMTandem 0x40	/* Mode0: enable input XON/XOFF processing*/
#define ATCSMInErrIgnore 0x80	/* Mode0: ignore input errors (pass chars) */
#define ATCSMReturnXON 0x01	/* Mode1: return XON if not suspended */
#define	ATCSDetectBreak	0x02	/* Mode1: Detect Break */
struct ATCSSetLineParametersResp {
	unchar Type;		/* ATCSSetLineParametersC */
	unchar Status;		/* 00-success, 01-flushed, 80-error */
	unchar reserved01[2];
};

#define ATCSWaitForCarrierC 0x09
struct ATCSWaitForCarrierReq {
	unchar Type;		/* ATCSWaitForCarrierC */
	unchar reserved01;
	unchar State;		/* wait for asserted (01) or deasserted (02) */
	unchar reserved02;
};
#define ATCSWFCOn 0x01
#define ATCSWFCOff 0x02
struct ATCSWaitForCarrierResp {
	unchar Type;		/* ATCSWaitForCarrierC */
	unchar Status;		/* 00-success, 01-flushed, 80-error */
	unchar State;		/* ATCSWFCxxx */
	unchar reserved01;
};

#define ATCSWaitForSpecialCharC 0x0a
struct ATCSWaitForSpecialCharReq {
	unchar Type;		/* ATCSWaitForSpecialCharC */
	unchar reserved01[3];
};
#define	ATCSBreak	0x1
struct ATCSWaitForSpecialCharResp {
	unchar Type;		/* ATCSWaitForSpecialCharC */
	unchar Status;		/* 00-success, 80-error */
	unchar SpecialChar;	/* special character detected */
	unchar Flags;		/* Bit 0 set if BREAK sensed */
};

#define ATCSTandemControlC 0X0b
struct ATCSTandemControlReq {
	unchar Type;		/* ATCSTandemControlC */
	unchar reserved01;
	unchar Command;		/* ATCSTMCxxxx */
	unchar reserved02;
};
#define ATCSTMCSuspendInput 0x01
#define ATCSTMCResumeInput 0x02
struct ATCSTandemControlResp {
	unchar Type;		/* ATCSTandemControlC */
	unchar Status;		/* 00-success, 80-error */
	unchar reserved01[2];
};
#endif /* _SYS_ATCSMP_H */
