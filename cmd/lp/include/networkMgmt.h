/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ifndef	NETWORK_MGMT_H
#define	NETWORK_MGMT_H
/*==================================================================*/
/*
*/
#ident	"@(#)lp:include/networkMgmt.h	1.4.2.1"

#include	"_networkMgmt.h"
#include	"xdrMsgs.h"
#include	"lists.h"
#include	"boolean.h"

typedef	struct
{
	int	size;
	void	*data_p;

}  dataPacket;

/*------------------------------------------------------------------*/
/*
**	Interface definition.
*/
#ifdef	__STDC__

int		SendJob (connectionInfo *, list *, list *, list *);
int		NegotiateJobClearance (connectionInfo *);
int		ReceiveJob (connectionInfo *, list **, list **);
char		*ReceiveFile (connectionInfo *, fileFragmentMsg *);
void		SetJobPriority (int);
void		FreeNetworkMsg (networkMsgType, void **);
void		FreeDataPacket (dataPacket **);
boolean		JobPending (connectionInfo *);
boolean		SendFile (connectionInfo *, boolean, char *, char *);
boolean		SendData (connectionInfo *, boolean, void *, int);
boolean		EncodeNetworkMsgTag (connectionInfo *, networkMsgType);
boolean		SendJobControlMsg (connectionInfo *, jobControlCode);
boolean		SendSystemIdMsg (connectionInfo *, void *, int);
boolean		SendFileFragmentMsg (connectionInfo *, boolean,
			fileFragmentMsg *);
dataPacket	*NewDataPacket (int);
networkMsgTag	*ReceiveNetworkMsg (connectionInfo *, void **);
networkMsgTag	*DecodeNetworkMsg (connectionInfo *, void **);

#else

int		SendJob ();
int		NegotiateJobClearance ();
int		ReceiveJob ();
char		*ReceiveFile ();
void		SetJobPriority ();
void		FreeNetworkMsg ();
void		FreeDataPacket ();
boolean		JobPending ();
boolean		SendFile ();
boolean		SendData ();
boolean		EncodeNetworkMsgTag ();
boolean		SendJobControlMsg ();
boolean		SendSystemIdMsg ();
boolean		SendFileFragmentMsg ();
dataPacket	*NewDataPacket ();
networkMsgTag	*ReceiveNetworkMsg ();
networkMsgTag	*DecodeNetworkMsg ();

#endif
/*==================================================================*/
#endif
