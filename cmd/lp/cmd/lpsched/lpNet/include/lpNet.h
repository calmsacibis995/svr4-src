/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ifndef	LP_NET_H
#define	LP_NET_H
/*=================================================================*/
/*
*/
#ident	"@(#)lp:cmd/lpsched/lpNet/include/lpNet.h	1.5.2.1"

#include	<sys/utsname.h>
#include	"networkMgmt.h"
#include	"msgs.h"

#ifndef	NULL
#define	NULL	0
#endif


/*-----------------------------------------------------------------*/
/*
*/
#define	LP_SERVICE_CODE	"lp"
#define	LP_HOME_DIR	"/var/spool/lp"
#define	LP_LOGS_DIR	"/var/lp/logs"
#define	LP_ETC_DIR	"/etc/lp"
#define	FILE_LPNET_LOG	"/var/lp/logs/lpNet"
#define	FILE_LPNET_DATA	"/etc/lp/Systems"
#define	PIPE_LISTEN_S5	"/var/spool/lp/fifos/listenS5"
#define	PIPE_LISTEN_BSD	"/var/spool/lp/fifos/listenBSD"

#define	SIP			ProcessInfo.remoteSystemInfo_p
#define	CIP			ProcessInfo.remoteConnectionInfo_p
#define	REMOTE_SYSTEM_FD	(CIP == NULL ? -1 : CIP->fd)
#define	CONNECTED_TO_REMOTE	CONNECTEDP(CIP)


/*-----------------------------------------------------------------*/
/*
*/
typedef	enum	{ 

	ParentProcess, 
	ChildProcess

}  processType;


typedef	enum	{

	ParentRank,
	MasterChild,
	SlaveChild,

}  processRank;

typedef	struct	{

	int		processId;
	char		*systemName_p;
	int		lpExec;
	MESG		*lpExecMsg_p;
	int		listenS5;
	int		listenBSD;
	systemInfo	*remoteSystemInfo_p;
	processType	processType;
	processRank	processRank;
	connectionInfo	*remoteConnectionInfo_p;

}  processInfo;


/*-----------------------------------------------------------------*/
/*
*/
extern	processInfo	ProcessInfo;


/*-----------------------------------------------------------------*/
/*
**	Global functions.
*/
#ifdef	__STDC__

extern	void	Exit (int);

#else

extern	void	Exit ();

#endif
/*=================================================================*/
#endif
