/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ifndef	_NETWORK_MGMT_H
#define	_NETWORK_MGMT_H
/*=================================================================*/
/*
*/
#ident	"@(#)lp:include/_networkMgmt.h	1.6.2.1"

#include	<sys/utsname.h>
#include	<time.h>
#include	<tiuser.h>
#include	<stropts.h>
#include	<poll.h>
#include	<netconfig.h>
#include	<netdir.h>
#include	<rpc/rpc.h>
#include	"_xdrMsgs.h"
#include	"boolean.h"


#define	CONNECTEDP(cp)	(cp == NULL ? False : (cp->fd == -1 ? False : True))


#ifndef	_T_BIND_T
#define	_T_BIND_T
typedef	struct	t_bind	t_bind_t;
#endif

#ifndef	_T_INFO
#define	_T_INFO
typedef	struct	t_info	t_info;
#endif

#ifndef	_T_CALL
#define	_T_CALL
typedef	struct	t_call	t_call;
#endif

#ifndef	_XDR_OP
#define	_XDR_OP
typedef	enum	xdr_op	xdr_op;
#endif

#ifndef	_NETCONFIG
#define	_NETCONFIG
typedef	struct	netconfig	netconfig;
#endif

#ifndef	_ND_HOSTSERV
#define	_ND_HOSTSERV
typedef	struct	nd_hostserv	nd_hostserv;
#endif

#ifndef	_ND_ADDRLIST
#define	_ND_ADDRLIST
typedef	struct	nd_addrlist	nd_addrlist;
#endif


/*---------------------------------------------------------*/
/*
*/
typedef	enum
{
	UnknownSystem	= 1,
	SystemVSystem	= 2,
	BerkeleySystem	= 3,

}  systemType;


typedef	struct
{
	char			*systemName_p;
	systemType		systemType;
	char			*systemPassword_p;
	int			timeout;
	int			retry;
	char			*ncProtoFamily_p;
	nd_hostserv		ndHostServ;
	int			ndOption;

}  systemInfo;


typedef	struct
{
	int		fd;
	int		timeout;
	int		logicalMsgSize;
	int		logicalMsgBufferSize;
	int		physicalMsgBufferSize;
	int		xdrSizeofPhysicalMsgTag;
	XDR		*xdrStream1_p;
	XDR		*xdrStream2_p;
	time_t		lastTransmissionTime;
	char		*physicalMsgBuffer_p;
	char		*logicalMsgBuffer_p;
	t_info		*providerInfo_p;
	t_bind_t	*returnBinding_p;
	t_call		*sendCall_p;
	t_call		*receiveCall_p;
	netconfig	*netConfig_p;
	nd_addrlist	*ndAddrList_p;
	physicalMsgTag	physicalMsgTag;

}  connectionInfo;


/*---------------------------------------------------------*/
/*
**	Interface definition.
*/
#ifdef	__STDC__

extern	int		FormatProtocolMsg (char *, char	*);
extern	int		PollNetworkEvent (connectionInfo *, int, int);
extern	char		*TransportStateDescription (int);
extern	char		*TransportEventDescription (int);
extern	void		DisconnectSystem (connectionInfo *);
extern	void		ResetXdrStream (connectionInfo *, xdr_op);
extern	void		FreeSystemInfo (systemInfo **);
extern	void		FreeConnectionInfo (connectionInfo **);
extern	void		FreeConnectionInfoMembers (connectionInfo *);
extern	boolean		ConnectToService (connectionInfo *, char *);
extern	boolean		Connected (connectionInfo *);
extern	boolean		DataWaiting (connectionInfo *);
extern	boolean		_SendNetworkMsg (connectionInfo *);
extern	boolean		_ReceiveNetworkMsg (connectionInfo *);
extern	boolean		OpenNetworkInfoFile (char *);
extern	boolean		PutIntoXdrStream (connectionInfo *,
					  bool_t (*)(), void *);
extern	boolean		GetFromXdrStream (connectionInfo *,
					  bool_t (*)(), void *);
extern	systemInfo	*GetSystemInfoByName (char *);
extern	systemInfo	*GetSystemInfoByAddr (connectionInfo *, char *);
extern	systemInfo	*NewSystemInfo (void);
extern	connectionInfo	*ConnectToSystem (systemInfo *);
extern	connectionInfo	*AcceptConnection (int);
extern	connectionInfo	*NewConnectionInfo (void);

#else

extern	int		FormatProtocolMsg ();
extern	int		PollNetworkEvent ();
extern	char		*TransportStateDescription ();
extern	char		*TransportEventDescription ();
extern	void		DisconnectSystem ();
extern	void		ResetXdrStream ();
extern	void		FreeSystemInfo ();
extern	void		FreeConnectionInfo ();
extern	void		FreeConnectionInfoMembers ();
extern	boolean		ConnectToService ();
extern	boolean		Connected ();
extern	boolean		DataWaiting ();
extern	boolean		_SendNetworkMsg ();
extern	boolean		_ReceiveNetworkMsg ();
extern	boolean		OpenNetworkInfoFile ();
extern	boolean		PutIntoXdrStream ();
extern	boolean		GetFromXdrStream ();
extern	systemInfo	*GetSystemInfoByName ();
extern	systemInfo	*GetSystemInfoByAddr ();
extern	systemInfo	*NewSystemInfo ();
extern	connectionInfo	*ConnectToSystem ();
extern	connectionInfo	*AcceptConnection ();
extern	connectionInfo	*NewConnectionInfo ();

#endif
/*=================================================================*/
#endif
