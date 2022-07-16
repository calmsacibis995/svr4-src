/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_CCIMP_H
#define _SYS_CCIMP_H

/*	Copyright (c) 1986, 1987  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/sys/ccimp.h	1.3"

#ifndef CCI_PORT_ID
#define CCI_PORT_ID 0x0506
#endif
#ifndef CCI_BRDCST_PORT
#define CCI_BRDCST_PORT 0x2405
#endif

#define CCISuccess 0x00
#define CCISwitched 0x01
#define CCIFailure 0x80

#define CCICreateC 0x01
struct CCICreateReq {
	unchar Type;		/* CCICreateC */
	unchar reserved01;
	unchar LineDiscID;	/* ID of line discipline script */
	unchar reserved02;
	long MemorySize;	/* bytes being requested for script */
};
struct CCICreateResp {
	unchar Type;		/* CCICreateC */
	unchar Status;		/* 00-mem alloc, 01-already loaded, 80-error */
	unchar LineDiscID;	/* ID of line discipline script */
	unchar reserved01;
	long reserved02;
	long BaseAddress;	/* base of allocated memory segment */
};

#define CCIDownloadC 0x02
struct CCIDownloadReq {
	unchar Type;		/* CCIDownloadC */
	unchar reserved01;
	unchar LineDiscID;	/* ID of line discipline script */
	unchar reserved02;
	long reserved03;
	long Offset;		/* offset to place data */
	long PSBAddress;	/* data buffer if shared memory */
};
struct CCIDownloadResp {
	unchar Type;		/* CCIDownloadC */
	unchar Status;		/* 00-success, 80-error */
	unchar LineDiscID;	/* ID of line discipline script */
	unchar reserved01;
};

#define CCISetParametersC 0x03
struct CCISetParametersReq {
	unchar Type;		/* CCISetParametersC */
	unchar reserved01;
	unchar LineDiscID;	/* ID of line discipline script */
	unchar reserved02;
	long DataSegmentSize;	/* memory to allocate to job in a Bind */
	long StartAddress;	/* absolute starting address */
};
struct CCISetParametersResp {
	unchar Type;		/* CCISetParametersC */
	unchar Status;		/* 00-success, 80-error */
	unchar LineDiscID;	/* ID of line discipline script */
	unchar reserved01;
};

#define CCIFreeC 0x04
struct CCIFreeReq {
	unchar Type;		/* CCIFreeC */
	unchar reserved01;
	unchar LineDiscID;	/* ID of line discipline script */
	unchar reserved02;
};
struct CCIFreeResp {
	unchar Type;		/* CCIFreeC */
	unchar Status;		/* 00-success, 80-error */
	unchar LineDiscID;	/* ID of line discipline script */
	unchar reserved01;
};

#define CCIBindC 0x10
struct CCIBindReq {
	unchar Type;		/* CCIBindReq */
	unchar reserved01;
	unchar LineDiscID;	/* ID of line discipline script */
	unchar LineNumber;	/* line number to bind */
};
struct CCIBindResp {
	unchar Type;		/* CCIBindReq */
	unchar Status;		/* 00-success, 80-error */
	unchar LineDiscID;	/* ID of line discipline script */
	unchar LineNumber;	/* line number bound */
	ushort NumSubchannels;	/* number of subchannels available */
	ushort reserved01;
};

#define CCIUnbindC 0x11
struct CCIUnbindReq {
	unchar Type;		/* CCIUnbindC */
	unchar reserved01;
	unchar reserved02;
	unchar LineNumber;	/* line number to unbind */
};
struct CCIUnbindResp {
	unchar Type;		/* CCIUnbindC */
	unchar Status;		/* 00-success, 80-error */
	unchar reserved01;
	unchar LineNumber;	/* line unbound */
};

#define CCIAttachC 0x20
struct CCIAttachReq {
	unchar Type;		/* CCIAttachC */
	unchar reserved01;
	unchar reserved02;
	unchar LineNumber;	/* line containing the subchannel */
	ushort Subchannel;	/* subchannel to attach */
	ushort DefaultPortID;	/* receiver for subchannel messages */
};
struct CCIAttachResp {
	unchar Type;		/* CCIAttachC */
	unchar Status;		/* 00-success, 01-switched to, 80-error */
	unchar reserved01;
	unchar LineNumber;	/* line containing subchannel */
	ushort Subchannel;	/* subchannel attached */
	ushort PortID;		/* portID allocated for this subchannel */
	unchar SessionStatus[4];/* information from previous host */
	ushort PrevHost;	/* hostID of host that switched from */
	unchar ScriptInfo[4];	/* script specific initialization information */
};

#define CCIDetachC 0x21
struct CCIDetachReq {
	unchar Type;		/* CCIDetachC */
	unchar reserved01;
	unchar reserved02;
	unchar LineNumber;	/* line containing subchannel */
	ushort Subchannel;	/* subchannel to detach */
	ushort reserved03;
	unchar SessionStatus[4];/* info from previous host */
};
struct CCIDetachResp {
	unchar Type;		/* CCIDetachC */
	unchar Status;		/* 00-success, 80-error */
	unchar reserved01;
	unchar LineNumber;	/* line containing subchannel */
	ushort Subchannel;	/* subchannel detached */
};

#define CCISwitchC 0x22
struct CCISwitchReq {
	unchar Type;		/* CCISwitchC */
	unchar reserved01;
	unchar reserved02;
	unchar LineNumber;	/* line containing subchannel */
	ushort Subchannel;	/* subchannel to switch */
	ushort reserved03;
	unchar SessionStatus[4];/* info to pass to next host */
	ushort NewHost;		/* host to pass subchannel to */
};
struct CCISwitchResp {
	unchar Type;		/* CCISwitchC */
	unchar Status;		/* 00-success, 01-switched to, 80-error */
	unchar reserved01;
	unchar LineNumber;	/* line containing subchannel */
	ushort Subchannel;	/* subchannel switched */
	ushort reserved02;
	unchar SessionStatus[4];/* info passed by previous host */
	ushort PrevHost;	/* portID of previous host */
};

#define CCIGetServerInfoC 0x30
struct CCIGetServerInfoReq {
	unchar Type;		/* CCIGetServerInfoC */
	unchar reserved01;
	unchar reserved02;
	unchar reserved03;
};
struct CCIGetServerInfoResp {
	unchar Type;		/* CCIGetServerInfoC */
	unchar Status;		/* 00-success, 80-error */
	ushort NumLines;	/* number of lines supported */
};

#define CCIGetLineDisciplineListC 0x31
struct CCIGetLineDisciplineListReq {
	unchar Type;		/* CCIGetLineDisciplineList */
	unchar reserved01;
	unchar reserved02;
	unchar reserved03;
	unchar reserved[12];
	long PSBAddress;	/* reply buffer (shared memory only) */
};
struct CCIGetLineDisciplineListResp {
	unchar Type;
	unchar Status;		/* 00-success, 80-error */
	unchar NumLineDiscID;	/* number of line disciplines loaded */
	unchar reserved01;
};

#define CCIGetLineDisciplineInfoC 0x32
struct CCIGetLineDisciplineInfoReq {
	unchar Type;		/* CCIGetLineDisciplineInfoC */
	unchar reserved01;
	unchar LineDiscID;	/* line discipline to get info for */
	unchar reserved02;
	unchar reserved03[12];
	long PSBAddress;	/* reply buffer (shared memory only) */
};
struct CCIGetLineDisciplineInfoResp {
	unchar Type;		/* CCIGetLineDisciplineInfoC */
	unchar Status;		/* 00-success, 80-error */
	unchar LineDiscID;	/* line disipline info gotten for */
	unchar State;		/* 00-not present, 01-downloading, 02-present */
	unchar NumHosts;	/* num hosts issuing Creates for this */
	unchar reserved01;
	ushort NumLines;	/* num lines bound to this discipline */
};

#define CCIGetLineInfoC 0x33
struct CCIGetLineInfoReq {
	unchar Type;		/* CCIGetLineInfoC */
	unchar reserved01;
	unchar LineNumber;	/* line to get info for */
	unchar reserved02;
	unchar reserved03[12];
	long PSBAddress;	/* reply buffer (shared memory only) */
};
struct CCIGetLineInfoResp {
	unchar Type;		/* CCIGetLineInfoC */
	unchar Status;		/* 00-success, 80-error */
	unchar LineNumber;	/* line info gotten for */
	unchar State;		/* 00-not bound, 01-bound */
	unchar NumHosts;	/* num hosts bound to line */
	unchar LineDiscID;	/* line discipline bound to line */
	ushort NumSubchannels;	/* num subchannels on line */
};

#define CCIGetSubchannelInfoC 0x34
struct CCIGetSubchannelInfoReq {
	unchar Type;		/* CCIGetSubchannelInfoC */
	unchar reserved01;
	unchar LineNumber;	/* line containing subchannel */
	unchar reserved;
	ushort Subchannel;	/* subchannel to get info for */
	unchar reserved02[10];
	long PSBAddress;	/* reply buffer (shared memory only) */
};
struct CCIGetSubchannelInfoResp {
	unchar Type;		/* CCIGetSubchannelInfoC */
	unchar Status;		/* 00-success, 80-error */
	unchar LineNumber;	/* line containing subchannel */
	unchar reserved01;
	ushort Subchannel;	/* subchannel info gotten for */
	unchar State;		/* 00-not attached, 01-attached */
	unchar NumHosts;	/* number of waiting hosts */
	ushort ActiveHost;	/* hostID of active host */
};
#endif	/* _SYS_CCIMP_H */
