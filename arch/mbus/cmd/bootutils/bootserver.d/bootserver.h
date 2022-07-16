/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:cmd/bootutils/bootserver.d/bootserver.h	1.3"

#define VERSION		1		/* Version of bootserver */
#define BOOT_PORT_ID	0x0500		/* well known bootserver port */
#define USERNAME	"daemon"		/* user name of bootserver */
#define GROUPNAME	"daemon"	/* group name of bootserver */

#define FIRST_COMMAND	0x0001		/* First valid command */
#define LAST_COMMAND	0x000A		/* Last valid command */

#define TRUE		1
#define NONE		0		/* No data buffer allocated */
#define CHANGE		1		/* data buffer uses someone's area */
#define ALLOC		2		/* data buffer allocated */

#define	X_COMPLETE	0
#define	X_INVALID	1
#define	X_ERR_CONFIG	2
#define	X_PRESENT	3
#define	X_SET_ID	4
#define	X_ERR_OPN	5
#define	X_ERR_RCV	6
#define	X_ERR_SND	7
#define	X_INTERNAL	9

#define E_OK 		0x00		/* status code of Bootserver protocol */
#define E_ACCESS	0x11
#define E_FNEXIST	0x22
#define E_IO		0x24
#define E_OVERFLOW	0x2A
#define E_INCOMPLETE	0xFE

#define MAXLOGLINE 512
#define ARGS a1,a2,a3,a4,a5
#define DUMP_LENGTH	16		/* Length for dump lines */
#define ALL_LEVEL	0xFF
#define L_BAD		0x00		/* Log level */
#define L_ERROR		0x00
#define L_STARTUP	0x01
#define L_LOCATE	0x02
#define L_CONNECT	0x04
#define L_OPEN		0x08
#define L_SEEK		0x10
#define L_READ		0x20
#define L_GETBPS	0x40
#define L_DEBUG		0x80

#define M_USAGE	"usage: bootserver [-c config_file] [-l log_file] [-x level]\n"

#define NUM_COMMAND	10		/* Number of Command */
#define NUM_STATE	3		/* Number of State */
#define ST_DISC		0x0000		/* Disconnect state */
#define ST_CONN		0x0001		/* Connect state */
#define ST_CON_OPN	0x0002		/* Connect&Open state */

#define MAX_DATA_SENT	28672	/* Max length of data part (28k) */
#define MAX_UNSOLMSG	20		/* Max length of ctl part */
#define MAX_LENGTH	256		/* Max length of path name */
/*
 * Structure for each client that has connection.
 */
struct HostInfo {
	struct HostInfo		*Next;
	ushort			HostID;
	unchar			State;
	char			OpenFile[MAX_LENGTH];
	long			FilePosition;
	time_t			ConnectTime;
	time_t			OpenTime;
	time_t			ReadTime;
};

#define RESPONSE_PROTOCOL	0x8000	
#define REPLY_LOCATE		2	
#define REPLY			1	
#define NO_REPLY		0	
#define READ			04	

#define NO_HOSTID		0
#define NO_METHOD		1
#define DEP_METHOD		2
#define QUA_METHOD		3
#define MAX_BL_METH		3	/* First 3 char. of BL_method are checked */

#define LocateBootS	0x0001
#define LocateConfigS	0x0002
struct LocateServerReq {
	ushort		Command;	/* LocateServerC */
	ushort		reserved01;
	ulong		ClientVersion;	/* Client protocol version */
	double 		Context;	/*Remote client context */
};
struct LocateServerResp	{
	ushort		Command;	/* LocateServerR */
	ushort		Status;
	ulong		ServerVersion;	/* Server protocol version */
	double 		Context;	/*Remote client context */
};

struct ConnectServerReq	{
	ushort		Command;	/* ConnectServerC */
	ushort		reserved01;
	double 		Context;	/*Remote client context */
};
struct ConnectServerResp	{
	ushort		Command;	/* ConnectServerR */
	ushort		Status;
	double 		Context;	/*Remote client context */
};

#define OpenSecondStageC	0x0004
struct OpenFileReq	{
	ushort		Command;	/* OpenFileC */
	ushort		reserved01;
	double 		Context;	/*Remote client context */
};	/* 2bytes for length, 256 data bytes for Path name */
struct	OpenFileData	{
	short		Length;
	char		PathName[MAX_LENGTH];
};
struct OpenFileResp	{
	ushort		Command;	/* OpenFileR */
	ushort		Status;
	ulong		FileDesc;
	double 		Context;	/*Remote client context */
};

struct GetHostBPSReq	{
	ushort		Command;	/* GetHostBPSC */
	ushort		reserved01;
	ulong		BufSize;
	double 		Context;	/*Remote client context */
};
struct GetHostBPSResp	{
	ushort		Command;	/* GetHostBPSC */
	ushort		Status;
	ulong		BpSize;
	double 		Context;	/*Remote client context */
};	/* data bytes for actual BPS data */

struct ReadFileReq	{
	ushort		Command;	/* ReadFileC */
	ushort		reserved01;
	ulong		FileDesc;
	ulong		Count;
};
struct ReadFileResp	{
	ushort		Command;	/* ReadFileR */
	ushort		Status;
	long		reserved01;
	ulong		Actual;
}; /* more data byte read */

struct SeekFileReq	{
	ushort		Command;	/* SeekFileC */
	ushort		reserved01;
	ulong		FileDesc;
	long		Offset;
};
struct SeekFileResp	{
	ushort		Command;	/* SeekFileR */
	ushort		Status;
}; 

#define CloseFileC		0x0009
struct CloseFileReq	{
	ushort		Command;	/* CloseFileC */
	ushort		reserved01;
	ulong		FileDesc;
};

#define DisconnectServerC	0x000A
struct DisconnectServerReq	{
	ushort		Command;	/* DisconnectServerR */
	double 		Context;	/*Remote client context */
};
