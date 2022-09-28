/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



/*==================================================================*/
/*
*/
#ident	"@(#)lp:lib/lpNet/_networkMgmt.c	1.6.3.1"

#include	<unistd.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<memory.h>
#include	<string.h>
#include	<fcntl.h>
#include	<listen.h>
#include	<rpc/rpc.h>
#include	"_networkMgmt.h"
#include	"errorMgmt.h"
#include	"debug.h"

#define	PROTOCOL_FORMAT		"NLPS:000:001:%s"
#define	PHYSICAL_MSG_SIZE(cp)	(cp)->logicalMsgSize + \
				(cp)->xdrSizeofPhysicalMsgTag

#ifndef	_POLLFD_T
typedef	struct	pollfd	pollfd_t;
#endif

#ifndef	_NETBUF_T
typedef	struct	netbuf	netbuf_t;
#endif

#ifndef	_NETCONFIG_T
typedef	struct	netconfig	netconfig_t;
#endif

#ifndef	_ND_HOSTSERVLIST_T
typedef	struct	nd_hostservlist	nd_hostservlist_t;
#endif

/*------------------------------------------------------------------*/
/*
*/
static	int	DefaultNetworkMsgBufferSize = 64*1024;
static	FILE	*NetworkInfoFile_p	    = NULL;

extern	int	errno;
extern	int	t_errno;


/*-----------------------------------------------------------------*/
/*
**	Local functions.
*/
#ifdef	__STDC__

static	int	PacketSize (boolean (*)(), connectionInfo *);

#else

static	int	PacketSize ();

#endif


/*-----------------------------------------------------------------*/
/*
**	netconfig functions
*/
#ifdef	__STDC__

extern	int		endnetpath (void *);
extern	int		endnetconfig (void *);
extern	void		*setnetpath (void);
extern	void		*setnetconfig (void);
extern	netconfig	*getnetpath (void *);
extern	netconfig	*getnetconfig (void *);
extern	netconfig	*getnetconfigent (char *);
extern	void		freenetconfigent (netconfig *);

#else

extern	int		endnetpath ();
extern	int		endnetconfig ();
extern	void		*setnetpath ();
extern	void		*setnetconfig ();
extern	netconfig	*getnetpath ();
extern	netconfig	*getnetconfig ();
extern	netconfig	*getnetconfigent ();
extern	void		freenetconfigent ();

#endif
/*-----------------------------------------------------------------*/
/*
**	Debug section.
*/
#ifdef	DEBUG

#ifdef	__STDC__
void	PrintSystemInfo (systemInfo *);
void	PrintStreamModules (int);
void	PrintTransportInfo (t_info *);
void	PrintTransportState (int);
void	PrintTransportEvent (int);
#else
void	PrintSystemInfo ();
void	PrintStreamModules ();
void	PrintTransportInfo ();
void	PrintTransportState ();
void	PrintTransportEvent ();
#endif

#else

#define	PrintSystemInfo(p)
#define	PrintStreamModules(fd)
#define	PrintTransportInfo(p)
#define	PrintTransportState(fd)
#define	PrintTransportEvent(fd)

#endif
/*==================================================================*/

/*=================================================================*/
/*
*/
boolean
OpenNetworkInfoFile (filePath_p)

char	*filePath_p;
{
	/*----------------------------------------------------------*/
	/*
	*/
		char	msgBuffer [128];
	static	char	FnName []	= "OpenNetworkInfoFile";


	/*---------------------------------------------------------*/
	/*
	*/
	if (filePath_p == NULL)
		return	False;

	if (NetworkInfoFile_p != NULL) {
		(void)
		fclose (NetworkInfoFile_p);
		NetworkInfoFile_p = NULL;
		return	True;
	}

	NetworkInfoFile_p = fopen (filePath_p, "r");

	if (NetworkInfoFile_p == NULL) {
		TrapError (NonFatal, Unix, FnName, "fopen");
		return	False;
	}
	/*---------------------------------------------------------*/


	return	True;
}
/*=================================================================*/

/*=================================================================*/
/*
*/
systemInfo *
GetSystemInfoByName (systemName_p)

char	*systemName_p;
{
	/*---------------------------------------------------------*/
	/*
	*/
		int		i;
		char		buffer [128],
				*token_p;
		boolean		found = False;
		systemInfo	*sip;

	static	char		TokenSeparators [] = ": \t\n";
	static	char		FnName []	   = "GetSystemInfoByName";


	ENTRYP
	/*---------------------------------------------------------*/
	/*
	*/
	if (systemName_p == NULL)
	{
		EXITP
		return	NULL;
	}


	/*---------------------------------------------------------*/
	/*
	*/
	if (NetworkInfoFile_p == NULL)
	{
		EXITP
		return	NULL;
	}
	else
		(void)	
		rewind (NetworkInfoFile_p);

	sip = NewSystemInfo ();


	/*----------------------------------------------------------*/
	/*
	*/
	do {
		if (fgets (buffer, sizeof (buffer), NetworkInfoFile_p) == NULL)
			goto	errorReturn_1;

		/*
		TRACEs (buffer) */


		/*-------------------------------------------------*/
		/*
		*/
		token_p = strtok (buffer, TokenSeparators);
		/*
		TRACEs (token_p) */

		if (*token_p == '#') {
			/*
			TRACEP ("COMMENT") */
			continue;
		}
		if (strcmp (systemName_p, token_p) == 0) {
			/*
			TRACEP ("MATCH")*/
			TRACEP ("System-name")
			TRACEs (token_p)
			if ((sip->systemName_p = strdup (token_p)) == NULL)
				TrapError (Fatal, Unix, FnName, "strdup");
			found = True;
			continue;
		}
	} while (! found);



	/*---------------------------------------------------------*/
	/*
	*/
	/* Password */
	token_p = strtok (NULL, TokenSeparators);
	TRACEP ("Password")
	TRACEs (token_p)
	if ((sip->systemPassword_p = strdup (token_p)) == NULL)
		TrapError (Fatal, Unix, FnName, "strdup");


	/*---------------------------------------------------------*/
	/*
	*/
	/* Reserved field */
	token_p = strtok (NULL, TokenSeparators);
	TRACEP ("Reserved")
	TRACEs (token_p)


	/*---------------------------------------------------------*/
	/*
	*/
	token_p = strtok (NULL, TokenSeparators);
	TRACEP ("System-Type")
	TRACEs (token_p)

	if (strcmp (token_p, "sv") == 0)
		sip->systemType = SystemVSystem;
	else
	if (strcmp (token_p, "s5") == 0)
		sip->systemType = SystemVSystem;
	else
	if (strcmp (token_p, "bsd") == 0)
		sip->systemType = BerkeleySystem;
	else
	if (strcmp (token_p, "ucb") == 0)
		sip->systemType = BerkeleySystem;
	else
		sip->systemType = UnknownSystem;


	/*---------------------------------------------------------*/
	/*
	*/
	/* Reserved field */
	token_p = strtok (NULL, TokenSeparators);
	TRACEP ("Reserved")
	TRACEs (token_p)


	/*---------------------------------------------------------*/
	/*
	*/
	token_p = strtok (NULL, TokenSeparators);
	TRACEP ("Timeout")
	TRACEs (token_p)

	if (*token_p == '\0')
		sip->timeout = -1;	/* Default:  never */
	if (*token_p == 'n')
		sip->timeout = -1;	/* Never timeout */
	else
	if (sscanf (token_p, "%d", &sip->timeout) != 1)
		sip->timeout = -1;	/* Default:  never */


	/*---------------------------------------------------------*/
	/*
	*/
	token_p = strtok (NULL, TokenSeparators);
	TRACEP ("Retry")
	TRACEs (token_p)

	if (*token_p == '\0')
		sip->retry = 10;	/* Default:  10 minutes */
	if (*token_p == 'n')
		sip->retry = -1;	/* Never */
	else
	if (sscanf (token_p, "%d", &sip->retry) != 1)
		sip->retry = 10;	/* Default:  10 minutes */


	/*---------------------------------------------------------*/
	/*
	**	Reserved field
	*/
	token_p = strtok (NULL, TokenSeparators);
	TRACEP ("Reserved")
	TRACEs (token_p)


	/*---------------------------------------------------------*/
	/*
	/*	Reserved field
	*/
	token_p = strtok (NULL, TokenSeparators);
	TRACEP ("Reserved")
	TRACEs (token_p)


	/*---------------------------------------------------------*/
	/*
	**	Comment field
	*/
	token_p = strtok (NULL, "\n");
	TRACEP ("Comment")
	TRACEs (token_p)


	sip->ncProtoFamily_p = NULL;
	sip->ndHostServ.h_host = strdup (sip->systemName_p);
	sip->ndHostServ.h_serv = strdup ("listen");
	sip->ndOption = 0;

	EXITP
	return	sip;


	/*----------------------------------------------------------*/
	/*
	*/
errorReturn_1:
	FreeSystemInfo (&sip);


	EXITP
	return	NULL;
}
/*=================================================================*/

/*=================================================================*/
/*
*/
systemInfo *
GetSystemInfoByAddr (cip, netidp)

connectionInfo	*cip;
char		*netidp;
{
	/*---------------------------------------------------------*/
	/*
	*/
		int			i,
					foo;
		systemInfo		*sip;
		netconfig_t		*configp;
		nd_hostservlist_t	*hostservlistp;

	static	char	FnName [] = "GetSystemInfoByAddr";


	ENTRYP
	/*---------------------------------------------------------*/
	/*
	*/
	if (!cip || !netidp)
	{
		errno = EINVAL;
		EXITP
		return	NULL;
	}
	if (!cip->receiveCall_p)
	{
		errno = ENOLINK;
		EXITP
		return	NULL;
	}
	configp = getnetconfigent (netidp);
	if (!configp)
	{
		EXITP
		return	NULL;
	}
	TRACEP ("After getnetconfigent.")

	foo = netdir_getbyaddr (configp, &hostservlistp,
				&(cip->receiveCall_p->addr));
	TRACEd (foo)
	TRACEd (errno)
	TRACE  (hostservlistp)
	TRACEd (hostservlistp->h_cnt)

	if (foo || !hostservlistp || !hostservlistp->h_cnt)
	{
		freenetconfigent (configp);
		EXITP
		return	NULL;
	}
	for (i=0; i < hostservlistp->h_cnt; i++)
	{
		TRACEs (hostservlistp->h_hostservs[i].h_host)
		sip =
		GetSystemInfoByName (hostservlistp->h_hostservs[i].h_host);
		if (sip)
		{
			TRACEP ("Found.")
			break;
		}
	}
	netdir_free (hostservlistp, ND_HOSTSERVLIST);
	freenetconfigent (configp);

	return	sip;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
connectionInfo *
ConnectToSystem (sip)

systemInfo	*sip;
{
	/*----------------------------------------------------------*/
	/*
	*/
		int		i;
		
		int		foo;
		void		*handle_p;
		netbuf_t	*netbuf_p;
		boolean		goodConnect = False;
		connectionInfo	*cip;

	static	char		FnName []	= "ConnectToSystem";


	ENTRYP
	/*---------------------------------------------------------*/
	/*
	*/
	cip = NewConnectionInfo ();

	if ((handle_p = setnetpath ()) == NULL)
		TrapError (Fatal, Unix, FnName, "setnetconfig");

	while ((cip->netConfig_p = getnetpath (handle_p)) != NULL) {
		if (cip->netConfig_p->nc_semantics == NC_TPI_CLTS) {
			TRACEP ("Wrong semantics.")
			cip->netConfig_p = NULL;
			continue;
		}
		if (sip->ncProtoFamily_p != NULL &&
		    strcmp (sip->ncProtoFamily_p,
			cip->netConfig_p->nc_protofmly) != 0) {
			TRACEP ("Wrong proto-family.")
			cip->netConfig_p = NULL;
			continue;
		}
		if ((foo = netdir_getbyname (cip->netConfig_p,
			&sip->ndHostServ,
			&cip->ndAddrList_p)) != 0) {
			TRACE (foo)
			cip->netConfig_p = NULL;
			continue;
		}
		TRACEP ("Address found.")


		/*-------------------------------------------------*/
		/*
		**	This section opens the network provider in
		**	'/dev'.
		*/
#ifdef	DEBUG
TRACEs (cip->netConfig_p->nc_device)
#endif
		cip->fd = t_open (cip->netConfig_p->nc_device, O_RDWR,
			cip->providerInfo_p);

		if (cip->fd == -1) {
			TrapError (NonFatal, TLI, FnName, "t_open");
			goto	errorReturn_1;
		}
#ifdef	DEBUG
PrintTransportInfo (cip->providerInfo_p);
#endif

		/*-------------------------------------------------*/
		/*
		*/
		cip->returnBinding_p = (t_bind_t *)
			t_alloc (cip->fd, T_BIND, T_ADDR);

		if (cip->returnBinding_p == NULL) {
			TrapError (NonFatal, TLI, FnName, "t_alloc");
			goto	errorReturn_1;
		}
		if (sip->ndOption != 0) {
			TRACE (sip->ndOption)
			if (netdir_options (cip->netConfig_p,
				sip->ndOption,
				cip->fd, NULL) != 0) {
				TrapError (NonFatal, Unix,
					FnName, "netdir_options");
				goto	errorReturn_1;
			}
			if (t_getname (cip->fd, cip->returnBinding_p,
				LOCALNAME) == -1) {
				TrapError (NonFatal, TLI, FnName,
					"t_getname");
				goto	errorReturn_1;
			}
		}
		else
		if (t_bind (cip->fd, NULL, cip->returnBinding_p) == -1) {
			TrapError (NonFatal, TLI, FnName, "t_bind");
			goto	errorReturn_1;
		}
#ifdef	DEBUG
TRACE (cip->returnBinding_p->addr.maxlen)
TRACE (cip->returnBinding_p->addr.len)
TRACEb (cip->returnBinding_p->addr.buf, cip->returnBinding_p->addr.len)
#endif

		/*-------------------------------------------------*/
		/*
		**	This section connects to the remote system.
		*/
		/*
		*/
		cip->sendCall_p = (t_call *)
			t_alloc (cip->fd, T_CALL, T_ALL);

		if (cip->sendCall_p == NULL)
		{
			TrapError (NonFatal, TLI, FnName, "t_alloc");
			goto	errorReturn_1;
		}
		cip->receiveCall_p = (t_call *)
			t_alloc (cip->fd, T_CALL, T_ALL);

		if (cip->receiveCall_p == NULL)
		{
			TrapError (NonFatal, TLI, FnName, "t_alloc");
			goto	errorReturn_1;
		}
		netbuf_p = cip->ndAddrList_p->n_addrs;

		for (i=0; i < cip->ndAddrList_p->n_cnt; i++)
		{
			if (netbuf_p->len > cip->sendCall_p->addr.maxlen)
			{
				TrapError (NonFatal, Internal, FnName,
				"netbuf_p->len > cip->sendCall_p->addr.maxlen");
				goto	errorReturn_1;
			}
			(void) memcpy (cip->sendCall_p->addr.buf,
				netbuf_p->buf, netbuf_p->len);
			cip->sendCall_p->addr.len = netbuf_p->len;
			if (t_connect (cip->fd, cip->sendCall_p,
				cip->receiveCall_p) == 0)
			{
#ifdef	DEBUG
PrintStreamModules (cip->fd);
TRACE (cip->receiveCall_p->addr.maxlen)
TRACE (cip->receiveCall_p->addr.len)
TRACEb (cip->receiveCall_p->addr.buf, cip->receiveCall_p->addr.len)
{
	char		buf [128];
	netbuf_t	nbuf;
	nbuf.len = 0;
	nbuf.maxlen = sizeof (buf);
	nbuf.buf = buf;
	if (t_getname (cip->fd, &nbuf, LOCALNAME) == -1)
	{
		TRACE (errno)
		TRACE (t_errno)
	}
	else
	{
		TRACEP ("t_getname")
		TRACE (nbuf.maxlen)
		TRACE (nbuf.len)
		TRACEb (nbuf.buf, nbuf.len)
	}
}
#endif
					goodConnect = True;
					break;
			}
#ifdef	DEBUG
/*---------------------------------------------------------*/
/*
**	This section tries to explain why 't_connect' failed.
*/
else {
	TRACEP ("t_connect failed.")
	TRACE (t_errno)
	PrintTransportEvent (cip->fd);
	PrintTransportState (cip->fd);

	switch (t_errno) {
	case	TLOOK:
		switch (t_look (cip->fd)) {
		case	-1:
			TrapError (NonFatal, TLI, FnName, "t_look");
			return	NULL;

		case	T_LISTEN:
		case	T_CONNECT:
		case	T_DATA:
		case	T_EXDATA:
		case	T_DISCONNECT:
		case	T_ERROR:
		case	T_UDERR:
		case	T_ORDREL:
			break;
		default:
			TrapError (NonFatal, Internal, FnName,
			"Unknown 't_look' event.");
		}
		break;

	case	TSYSERR:
	case	TBADF:
	case	TOUTSTATE:
	case	TNODATA:
	case	TBADADDR:
	case	TBADOPT:
	case	TBADDATA:
	case	TACCES:
	case	TBUFOVFLW:
	case	TNOTSUPPORT:
	default:
		TrapError (NonFatal, TLI, FnName, "t_connect");
	}
}
#endif
		} /* for */


		/*-------------------------------------------------*/
		/*
		*/
		if (goodConnect == True)
			break;
		else {
			cip->netConfig_p = NULL;
			netdir_free (cip->ndAddrList_p, ND_ADDRLIST);
			cip->ndAddrList_p = NULL;
		}
	} /* while */
	(void)	endnetpath (handle_p);
	handle_p =
	cip->netConfig_p = NULL;  /* unusabe after 'endnetpath' */

	if (goodConnect) {
		EXITP
		return	cip;
	}
	else {
		FreeConnectionInfo (&cip);
		EXITP
		return	NULL;
	}


	/*---------------------------------------------------------*/
	/*
	**	This section handles cleaning-up for the return call
	**	in the case of an error.
	*/
errorReturn_1:
	if (handle_p != NULL) {
		(void)	endnetpath (handle_p);
		handle_p =
		cip->netConfig_p = NULL;  /* unusabe after 'endnetpath' */
	}
	FreeConnectionInfo (&cip);


	EXITP
	return	NULL;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
connectionInfo *
AcceptConnection (fd)

int	fd;
{
	/*----------------------------------------------------------*/
	/*
	*/
		connectionInfo	*cip	= NULL;
	static	char		FnName []		= "AcceptConnection";

#ifdef	DEBUG
int	flags;
ENTRYP
PrintStreamModules (fd);
PrintTransportState (fd);
PrintTransportEvent (fd);

if ((flags = fcntl (fd, F_GETFL, &flags)) == -1)
	TrapError (NonFatal, Unix, FnName, "fcntl");

TRACE (flags)
#endif
	/*----------------------------------------------------------*/
	/*
	*/
	cip = NewConnectionInfo ();

	cip->fd = fd;

	if (t_sync (fd) == -1) {
		TrapError (NonFatal, TLI, FnName, "t_sync");
		goto	errorReturn_1;
	}
	if (t_getinfo (fd, cip->providerInfo_p) == -1) {
		TrapError (NonFatal, TLI, FnName, "t_getinfo");
		goto	errorReturn_1;
	}
#ifdef	DEBUG
PrintTransportInfo (cip->providerInfo_p);
#endif


	/*---------------------------------------------------------*/
	/*
	**	This section sets up the t_call structure to contain
	**	the address of the remote-system on the network.
	*/
	cip->receiveCall_p = (t_call *)
		t_alloc (fd, T_CALL, T_ADDR);

	if (cip->receiveCall_p == NULL) {
		TrapError (NonFatal, TLI, FnName, "t_alloc");
		goto	errorReturn_1;
	}
	if (t_getname (fd, &(cip->receiveCall_p->addr), REMOTENAME) == -1) {
#ifdef	DEBUG
TRACE (t_errno)
TRACE (errno)
#endif
		TrapError (NonFatal, TLI, FnName, "t_getname");
		PrintTransportState (fd);
		PrintTransportEvent (fd);
		goto	errorReturn_1;
	}
#ifdef	DEBUG
TRACEP ("t_getname")
TRACE (cip->receiveCall_p->addr.maxlen)
TRACE (cip->receiveCall_p->addr.len)
TRACEb (cip->receiveCall_p->addr.buf, cip->receiveCall_p->addr.len)
EXITP
#endif
	return	cip;


	/*---------------------------------------------------------*/
	/*
	**	This section handles cleaning-up for the return call
	**	in the case of an error.
	*/
errorReturn_1:
	FreeConnectionInfo (&cip);

EXITP
	return	NULL;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
void
DisconnectSystem (cip)

connectionInfo	*cip;
{
	/*----------------------------------------------------------*/
	/*
	*/
	static	char	FnName []	= "DisconnectSystem";


	ENTRYP
	/*----------------------------------------------------------*/
	/*
	*/
	if (cip == NULL || cip->fd == -1)
		return;

	(void)	t_close (cip->fd);

	cip->fd = -1;

	EXITP
	return;
}
/*==================================================================*/

/*=================================================================*/
/*
*/
boolean
ConnectToService (cip, serviceCode_p)

connectionInfo	*cip;
char		*serviceCode_p;
{
	/*----------------------------------------------------------*/
	/*
	*/
		char	msgBuffer [128];
	static	char	FnName []		= "ConnectToService";
	extern	char	*_nlsrmsg;

	/*----------------------------------------------------------*/
	/*
	*/
	switch (nlsrequest (cip->fd, serviceCode_p)) {
	case	-1:
		TrapError (NonFatal, TLI, FnName, "nlsrequest");
		return	False;

	case	NLSSTART:
		break;

	case	NLSFORMAT:
	case	NLSUNKNOWN:
	case	NLSDISABLED:
		TrapError (NonFatal, Internal, FnName, _nlsrmsg);
		return	False;

	default:
		TrapError (NonFatal, Internal, FnName,
		"Unknown return code from 'nlsrequest()'.");
		return	False;
	}


	return	True;
}
/*=================================================================*/

/*=================================================================*/
/*
*/
int
FormatProtocolMsg (logicalMsgBuffer_p, serviceCode_p)

char	*logicalMsgBuffer_p;
char	*serviceCode_p;
{
	static	char	ProtocolFormat[] = PROTOCOL_FORMAT;

	(void)	sprintf (logicalMsgBuffer_p, ProtocolFormat, serviceCode_p);

	return	strlen (logicalMsgBuffer_p)+1;
}
/*=================================================================*/

/*==================================================================*/
/*
**	Testable select events 
**
**	POLLIN		01	fd is readable
**	POLLPRI		02	priority info at fd
**	POLLOUT		04	fd is writeable (won't block)
**
**	Non-testable poll events (may not be specified in events field,
**	but may be returned in revents field).
**
**	POLLERR		010	fd has error condition
**	POLLHUP		020	fd has been hung up on
**	POLLNVAL	040	invalid pollfd entry
*/
int
PollNetworkEvent (cip, events, timeout)

connectionInfo	*cip;
int		events;
int		timeout;
{
	/*----------------------------------------------------------*/
	/*
	*/
		pollfd_t	pfd;
	static	char	FnName [] =	"PollNetworkEvent";


	/*----------------------------------------------------------*/
	/*
	*/
start:
	pfd.fd	= cip->fd;
	pfd.events	= (short) events;
	pfd.revents	= 0;

	switch	(poll (&pfd, 1, timeout)) {
	case	1:
		if (pfd.revents && POLLHUP)
			DisconnectSystem (cip);
		if (pfd.revents && POLLERR)
			DisconnectSystem (cip);

		return	(int)	pfd.revents;

	case	0:
		return	0;

	case	-1:
		if (errno == EAGAIN)
			goto	start;

		if (errno == EINTR)
			goto	start;

		return	-1;
	}

	return	0;
}
/*==================================================================*/

/*=================================================================*/
/*
*/
boolean
SetTimeout (cip, timeout)

connectionInfo	*cip;
{
		int	state,
			event;
	static	char	FnName [] = "Connected";

	ENTRYP
	TRACE (cip)
	if (cip == NULL)
	{
		errno = EINVAL;
		EXITP
		return	False;
	}
	TRACE (cip->fd)
	if (cip->fd == -1)
	{
		errno = ENOLINK;
		EXITP
		return	False;
	}
	if (timeout < -1)
		timeout = -1;
	cip->timeout = timeout;
	EXITP
	return	True;
}
/*=================================================================*/

/*==================================================================*/
/*
*/
boolean
Connected (cip)

connectionInfo	*cip;
{
		int	state,
			event;
	static	char	FnName [] = "Connected";

	ENTRYP
	TRACE (cip)
	if (cip == NULL)
	{
		errno = EINVAL;
		EXITP
		return	False;
	}
	TRACE (cip->fd)
	if (cip->fd == -1)
	{
		errno = ENOLINK;
		EXITP
		return	False;
	}
	while ((state = t_getstate (cip->fd)) == -1)
	{
		TRACE (t_errno)
		if (t_errno == TSTATECHNG)
			continue;
		else
		{
			EXITP
			return	False;
		}
	}
	TRACEs (TransportStateDescription (state))
	if ((event = t_look (cip->fd)) == -1)
	{
		TRACE (t_errno)
		EXITP
		return	False;
	}
	TRACEs (TransportEventDescription (event))
	if (state == T_DATAXFER &&
	   (event == 0 || event == T_DATA || event == T_EXDATA))
	{
		EXITP
		return	True;
	}
	EXITP
	return	False;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
boolean
DataWaiting (cip)

connectionInfo	*cip;
{
		int	state,
			event;
	static	char	FnName [] = "DataWaiting";

	ENTRYP
	TRACE (cip)
	if (cip == NULL)
	{
		errno = EINVAL;
		EXITP
		return	False;
	}
	TRACE (cip->fd)
	if (cip->fd == -1)
	{
		errno = ENOLINK;
		EXITP
		return	False;
	}
	while ((state = t_getstate (cip->fd)) == -1)
	{
		TRACE (t_errno)
		if (t_errno == TSTATECHNG)
			continue;
		else
		{
			EXITP
			return	False;
		}
	}
	TRACEs (TransportStateDescription (state))
	if ((event = t_look (cip->fd)) == -1)
	{
		TRACE (t_errno)
		EXITP
		return	False;
	}
	TRACEs (TransportEventDescription (event))
	if (state == T_DATAXFER &&
	   (event == T_DATA || event == T_EXDATA))
	{
		EXITP
		return	True;
	}
	EXITP
	return	False;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
boolean
_SendNetworkMsg (cip)

connectionInfo	*cip;
{
	/*----------------------------------------------------------*/
	/*
	*/
		int		state,
				event,
				flags,
				nBytes,
				nPackets,
				byteCount,
				packetSize,
				physicalMsgSize;
		pollfd_t	pfd;

	static	char	FnName []	= "_SendNetworkMsg";


	ENTRYP
	/*---------------------------------------------------------*/
	/*
	*/
	if (cip == NULL) {
		TRACE (cip)
		return	False;
	}
	if (cip->fd == -1) {
		TRACE (cip->fd)
		return	False;
	}
	if (cip->logicalMsgSize <= 0) {
		TRACE (cip->logicalMsgSize)
		return	False;
	}


	/*---------------------------------------------------------*/
	/*
	*/
	PrintTransportState (cip->fd);
	while ((state = t_getstate (cip->fd)) == -1)
	{
		if (t_errno == TSTATECHNG)
			continue;
		else
		{
			TrapError (NonFatal, TLI, FnName, "t_getstate");
			EXITP
			return	False;
		}
	}
	switch (state) {
	case	T_DATAXFER:
		break;

	case	T_UNBND:
	case	T_IDLE:
	case	T_OUTCON:
	case	T_INCON:
	case	T_OUTREL:
	case	T_INREL:
		TrapError (NonFatal, Internal, FnName,
			"Bad transport state.");
		TrapError (NonFatal, Internal, FnName, 
			TransportStateDescription (state));
		DisconnectSystem (cip);
		EXITP
		return	False;

	default:
		TrapError (NonFatal, Internal, FnName,
		"Unknown transport state.");
		DisconnectSystem (cip);
		EXITP
		return	False;
	}


	/*---------------------------------------------------------*/
	/*
	*/
#ifdef	DEBUG
PrintTransportEvent (cip->fd);
#endif

 	event = t_look (cip->fd);

	switch (event) {
	case	-1:
		TrapError (NonFatal, TLI, FnName, "t_look");
		EXITP
		return	False;

	case	0:		/*  No event.	*/
	case	T_DATA:		/*  This is OK.	*/
		break;
	
	case	T_UDERR:
	case	T_LISTEN:
	case	T_CONNECT:
	case	T_EXDATA:
	case	T_ORDREL:
	case	T_ERROR:
		TrapError (NonFatal, Internal, FnName, 
			"Bad TLI event.");
		TrapError (NonFatal, Internal, FnName, 
			TransportEventDescription (event)); 
		DisconnectSystem (cip);
		EXITP
		return	False;

	case	T_DISCONNECT:
		DisconnectSystem (cip);
		EXITP
		return	False;

	default:
		TrapError (NonFatal, Internal, FnName,
		"Unknown transport event.");
		DisconnectSystem (cip);
		EXITP
		return	False;
	}


	/*---------------------------------------------------------*/
	/*
	**	Send an atomic transmission.
	*/
	physicalMsgSize	=
	cip->physicalMsgTag.physicalMsgSize = PHYSICAL_MSG_SIZE (cip);

	ResetXdrStream (cip, XDR_ENCODE);

	if (! PutIntoXdrStream (cip, xdr_physicalMsgTag, &cip->physicalMsgTag))
	{
		EXITP
		return	False;
	}
	packetSize = PacketSize (_SendNetworkMsg, cip);

	TRACE (cip->logicalMsgSize)
	TRACE (packetSize)
	byteCount = 0;
	nPackets = 0;
	do {
		/*--------------------------------------------------*/
		/*
		**	If we have more than one packet then
		**	we should wait for the stream to drain.
		*/
		if (nPackets > 0) {
			TRACEP ("Before poll")
			PrintTransportEvent (pfd.fd);
			PrintTransportState (pfd.fd);

			pfd.fd	= cip->fd;
			pfd.events	= POLLOUT;

			event = poll (&pfd, 1, -1);

			PrintTransportEvent (pfd.fd);
			PrintTransportState (pfd.fd);
			TRACEP ("After poll")
			TRACE (event)
			TRACE (pfd.revents)
			if (event == -1) {
				TrapError (NonFatal, Unix, FnName, "poll");
				DisconnectSystem (cip);
				EXITP
				return	False;
			}
			if (! (pfd.revents & POLLOUT)) {
				TrapError (NonFatal, Internal, FnName,
				"Bad event on transport.");
				PrintTransportEvent (pfd.fd);
				PrintTransportState (pfd.fd);
				DisconnectSystem (cip);
				EXITP
				return	False;
			}
		}


		/*--------------------------------------------------*/
		/*
		*/
		if ((physicalMsgSize - byteCount) < packetSize)
			packetSize = physicalMsgSize - byteCount;

		TRACEP ("Before t_snd")
		PrintTransportEvent (cip->fd);
		PrintTransportState (cip->fd);
		TRACE (packetSize)
		nBytes = t_snd (cip->fd,
			cip->physicalMsgBuffer_p + byteCount,
			packetSize, 0);
		PrintTransportEvent (cip->fd);
		PrintTransportState (cip->fd);
		TRACEP ("After t_snd")
		TRACE (nBytes)

		if (nBytes == -1) {
			TrapError (NonFatal, TLI, FnName, "t_snd");
			EXITP
			return	False;
		}
		byteCount += nBytes;
		nPackets++;
	} while (byteCount != physicalMsgSize);
	TRACE (byteCount)
	TRACE (nPackets)

	cip->lastTransmissionTime = time ((long *) 0);


	EXITP
	return	True;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
boolean
_ReceiveNetworkMsg (cip)

register
connectionInfo	*cip;
{
	/*---------------------------------------------------------*/
	/*
	*/
		int		state,
				event,
				flags,
				nBytes,
				nPackets,
				byteCount,
				packetSize,
				physicalMsgSize;
		pollfd_t	pfd;

	static	char	FnName []	= "_ReceiveNetworkMsg";


	ENTRYP
	/*---------------------------------------------------------*/
	/*
	*/
	if (cip == NULL)
	{
		EXITP
		return	False;
	}
	if (cip->fd == -1)
	{
		EXITP
		return	False;
	}


	/*---------------------------------------------------------*/
	/*
	*/
	PrintTransportState (cip->fd);
	while ((state = t_getstate (cip->fd)) == -1)
	{
		if (t_errno == TSTATECHNG)
			continue;
		else
		{
			TrapError (NonFatal, TLI, FnName, "t_getstate");
			EXITP
			return	False;
		}
	}
	switch (state) {
	case	T_DATAXFER:
		break;

	case	T_UNBND:
	case	T_IDLE:
	case	T_OUTCON:
	case	T_INCON:
	case	T_OUTREL:
	case	T_INREL:
		TrapError (NonFatal, Internal, FnName,
			"Bad transport state.");
		TrapError (NonFatal, Internal, FnName, 
			TransportStateDescription (state));
		DisconnectSystem (cip);
		EXITP
		return	False;

	default:
		TrapError (NonFatal, Internal, FnName,
		"Unknown transport state.");
		DisconnectSystem (cip);
		EXITP
		return	False;
	}


	/*---------------------------------------------------------*/
	/*
	*/
	PrintTransportEvent (cip->fd);

 	event = t_look (cip->fd);

	switch (event) {
	case	-1:
		TrapError (NonFatal, TLI, FnName, "t_look");
		EXITP
		return	False;

	case	0:
		/*
		**	No event.
		*/
		pfd.fd	= cip->fd;
		pfd.events	= POLLIN;
		event = poll (&pfd, 1, cip->timeout);
		if (event == -1) {
			TrapError (NonFatal, Unix, FnName, "poll");
			DisconnectSystem (cip);
			EXITP
			return	False;
		}
		if (event == 0) {
			TRACEP ("timeout")
			PrintTransportState (cip->fd);
			PrintTransportEvent (cip->fd);
			EXITP
			return	False;
		}

		EXITP
		return	_ReceiveNetworkMsg (cip);

	
	case	T_UDERR:
	case	T_LISTEN:
	case	T_CONNECT:
	case	T_EXDATA:
	case	T_ORDREL:
	case	T_ERROR:
		TrapError (NonFatal, Internal, FnName, 
			"Bad TLI event.");
		TrapError (NonFatal, Internal, FnName, 
			TransportEventDescription (event)); 

	case	T_DISCONNECT:
		DisconnectSystem (cip);
		EXITP
		return	False;

	case	T_DATA:
		break;

	default:
		TrapError (NonFatal, Internal, FnName,
		"Unknown 't_look' event.");
		EXITP
		return	False;
	}


	/*---------------------------------------------------------*/
	/*
	**	Receive an atomic transmission.
	**
	**	NOTE:
	**	Byte stream providers are trickey.  We could
	**	get two messages with one t_rcv and that would
	**	screw up the logic of the application using us.
	**	So, we do the first t_rcv to get the physicalMsgSize
	**	and then make sure we only read that many bytes
	**	forcing the underlying layers to buffer the messages.
	*/
	packetSize		= PacketSize (_ReceiveNetworkMsg, cip);
	nPackets		= 0;
	byteCount		= 0;
	cip->logicalMsgSize	= 0;
	physicalMsgSize		= packetSize;

	if (cip->providerInfo_p->tsdu == 0) {
		TRACEP ("Byte Stream transport.")
		TRACEP ("Before t_rcv")
		PrintTransportEvent (cip->fd);
		PrintTransportState (cip->fd);
		flags = 0;
		while (byteCount < cip->xdrSizeofPhysicalMsgTag)
		{
			nBytes = t_rcv (cip->fd,
				cip->physicalMsgBuffer_p + byteCount,
				cip->xdrSizeofPhysicalMsgTag-byteCount,
				&flags);
			PrintTransportEvent (cip->fd);
			PrintTransportState (cip->fd);
			TRACEP ("After t_rcv")
			TRACE (nBytes)
			TRACE (flags)
			if (nBytes == -1)
			{
				TrapError (NonFatal, TLI, FnName, "t_rcv");
				EXITP
				return	False;
			}
			byteCount += nBytes;
		}
		nPackets++;

		ResetXdrStream (cip, XDR_DECODE);
		if (! GetFromXdrStream (cip, xdr_physicalMsgTag,
			&cip->physicalMsgTag)) {
			EXITP
			return	False;
		}
		physicalMsgSize = cip->physicalMsgTag.physicalMsgSize;

		if (physicalMsgSize > cip->physicalMsgBufferSize) {
			TrapError (NonFatal, Internal, FnName,
				"Buffer too small.");
			EXITP
			return	False;
		}
	}
	TRACEP ("Before 'do'.")
	TRACE (packetSize)
	TRACE (nPackets)
	TRACE (byteCount)
	do {
		if ((physicalMsgSize - byteCount) < packetSize)
			packetSize = physicalMsgSize - byteCount;

		TRACEP ("Before t_rcv")
		PrintTransportEvent (cip->fd);
		PrintTransportState (cip->fd);
		flags = 0;
		nBytes = t_rcv (cip->fd,
			cip->physicalMsgBuffer_p + byteCount,
			packetSize, &flags);
		PrintTransportEvent (cip->fd);
		PrintTransportState (cip->fd);
		TRACEP ("After t_rcv")
		TRACE (nBytes)
		TRACE (flags)

		if (nBytes == -1) {
			TrapError (NonFatal, TLI, FnName, "t_rcv");
			EXITP
			return	False;
		}

		byteCount += nBytes;
		nPackets++;

		if (nPackets == 1) {
			ResetXdrStream (cip, XDR_DECODE);
			if (! GetFromXdrStream (cip, xdr_physicalMsgTag,
				&cip->physicalMsgTag)) {
				EXITP
				return	False;
			}
			physicalMsgSize = cip->physicalMsgTag.physicalMsgSize;

			if (physicalMsgSize > cip->physicalMsgBufferSize) {
				TrapError (NonFatal, Internal, FnName,
					"Buffer too small.");
				EXITP
				return	False;
			}
		}
	} while (byteCount != physicalMsgSize);
	TRACE (byteCount)
	TRACE (nPackets)

	cip->logicalMsgSize =
		cip->physicalMsgTag.physicalMsgSize -
		cip->xdrSizeofPhysicalMsgTag;

	cip->lastTransmissionTime = time ((long *) 0);


	EXITP
	return	True;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
static int
PacketSize (function, cip)

boolean		(*function) ();
connectionInfo	*cip;
{
	/*----------------------------------------------------------*/
	/*
	*/
		register
		int	packetSize;
	static	char	FnName []	= "PacketSize";


	/*----------------------------------------------------------*/
	/*
	*/
	packetSize = cip->providerInfo_p->tsdu;

	if (packetSize == -2)
		TrapError (Fatal, Internal, FnName,
			"Unsupported transport mode.");
	else
	if (packetSize == -1)
		if (function == _SendNetworkMsg)
			packetSize = PHYSICAL_MSG_SIZE (cip);
		else
			packetSize = cip->physicalMsgBufferSize;
	else
 	if (packetSize == 0)
		packetSize = 2048;
	else
	if (packetSize > 1024)
		packetSize = (packetSize / 1024) * 1024;


	return	packetSize;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
void
ResetXdrStream (cip, xdrOperation)

connectionInfo	*cip;
xdr_op		xdrOperation;
{
	/*----------------------------------------------------------*/
	/*
	*/
	static	char	FnName []	= "ResetXdrStream";


	/*----------------------------------------------------------*/
	/*
	*/
	if (cip == NULL)
		return;


	/*----------------------------------------------------------*/
	/*
	*/
	xdr_setpos (cip->xdrStream1_p, 0);
	cip->xdrStream1_p->x_op = xdrOperation;

	xdr_setpos (cip->xdrStream2_p, 0);
	cip->xdrStream2_p->x_op = xdrOperation;


	return;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
boolean
PutIntoXdrStream (cip, xdrFn_p, data_p)

connectionInfo	*cip;
bool_t		(*xdrFn_p) ();
void		*data_p;
{
	/*---------------------------------------------------------*/
	/*
	*/
		XDR	*xdrStream_p;
	static	char	FnName []	= "PutIntoXdrStream";


	/*----------------------------------------------------------*/
	/*
	*/
	if (cip == NULL)
		return	False;


	/*---------------------------------------------------------*/
	/*
	*/
	if (xdrFn_p == xdr_physicalMsgTag)
		xdrStream_p = cip->xdrStream1_p;
	else
		xdrStream_p = cip->xdrStream2_p;

	if (! (*xdrFn_p) (xdrStream_p, data_p)) {
		TrapError (NonFatal, XdrEncode, FnName, NULL);

		return	False;
	}
	if (xdrFn_p != xdr_physicalMsgTag)
		cip->logicalMsgSize = xdr_getpos (xdrStream_p);

	return	True;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
boolean
GetFromXdrStream (cip, xdrFn_p, data_p)

connectionInfo	*cip;
bool_t		(*xdrFn_p) ();
void		*data_p;
{
	/*---------------------------------------------------------*/
	/*
	*/
		XDR	*xdrStream_p;
	static	char	FnName []	= "GetFromXdrStream";


	/*----------------------------------------------------------*/
	/*
	*/
	if (cip == NULL)
		return	False;


	/*---------------------------------------------------------*/
	/*
	*/
	if (xdrFn_p == xdr_physicalMsgTag)
		xdrStream_p = cip->xdrStream1_p;
	else
		xdrStream_p = cip->xdrStream2_p;

	if (! (*xdrFn_p) (xdrStream_p, data_p)) {
		TrapError (NonFatal, XdrDecode, FnName, NULL);

		return	False;
	}


	return	True;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
systemInfo *
NewSystemInfo ()
{
	/*----------------------------------------------------------*/
	/*
	*/
	register	systemInfo	*sip;
	static	char	FnName []	= "NewSystemInfo";

	/*----------------------------------------------------------*/
	/*
	*/
	sip = (systemInfo *)  
		calloc (1, sizeof (systemInfo));

	if (sip == NULL)
		TrapError (Fatal, Unix, FnName, "calloc");


	return	sip;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
void
FreeSystemInfo (sipp)

systemInfo	**sipp;
{
	if (sipp == NULL || *sipp == NULL)
		return;

	if ((*sipp)->systemName_p != NULL)
		free ((*sipp)->systemName_p);
	if ((*sipp)->systemPassword_p != NULL)
		free ((*sipp)->systemPassword_p);
	if ((*sipp)->ndHostServ.h_host != NULL)
		free ((*sipp)->ndHostServ.h_host);
	if ((*sipp)->ndHostServ.h_serv != NULL)
		free ((*sipp)->ndHostServ.h_serv);
	free (*sipp);

	*sipp = NULL;

	return;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
connectionInfo *
NewConnectionInfo ()
{
	/*----------------------------------------------------------*/
	/*
	*/
	register	connectionInfo	*cip;
	static	char	FnName []	= "NewConnectionInfo";


	/*----------------------------------------------------------*/
	/*
	*/
	cip = (connectionInfo *)  
		calloc (1, sizeof (connectionInfo));

	if (cip == NULL)
		TrapError (Fatal, Unix, FnName, "calloc");

	cip->fd			= -1;	/*  Bad fd.		*/
	cip->timeout		= -1;	/*  Never.		*/
	cip->logicalMsgSize	= 0;	/*  0			*/

	cip->xdrStream1_p = (XDR *)
		calloc (1, sizeof (XDR));

	if (cip->xdrStream1_p == NULL)
		TrapError (Fatal, Unix, FnName, "calloc");

	cip->xdrStream2_p = (XDR *)
		calloc (1, sizeof (XDR));

	if (cip->xdrStream2_p == NULL)
		TrapError (Fatal, Unix, FnName, "calloc");

	cip->physicalMsgBuffer_p	=
	cip->logicalMsgBuffer_p	=
		calloc (1, DefaultNetworkMsgBufferSize);

	if (cip->physicalMsgBuffer_p == NULL)
		TrapError (Fatal, Unix, FnName, "calloc");

	cip->physicalMsgBufferSize =
		DefaultNetworkMsgBufferSize;

	xdrmem_create (cip->xdrStream1_p, cip->physicalMsgBuffer_p,
		cip->physicalMsgBufferSize, XDR_ENCODE);

	if (xdr_physicalMsgTag (cip->xdrStream1_p,
		&cip->physicalMsgTag))
		cip->xdrSizeofPhysicalMsgTag =
			xdr_getpos (cip->xdrStream1_p);
	else
		cip->xdrSizeofPhysicalMsgTag =
			2*sizeof(physicalMsgTag);

	cip->logicalMsgBuffer_p +=
		cip->xdrSizeofPhysicalMsgTag;

	cip->logicalMsgBufferSize =
		cip->physicalMsgBufferSize - cip->xdrSizeofPhysicalMsgTag;

	xdrmem_create (cip->xdrStream2_p, cip->logicalMsgBuffer_p,
		cip->logicalMsgBufferSize, XDR_ENCODE);

	cip->providerInfo_p = (t_info *)
		calloc (1, sizeof (t_info));

	if (cip->providerInfo_p == NULL)
		TrapError (Fatal, Unix, FnName, "calloc");


	return	cip;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
void
FreeConnectionInfo (cipp)

register
connectionInfo	**cipp;
{
	/*---------------------------------------------------------*/
	/*
	*/
	if (cipp == NULL || *cipp == NULL)
		return;

	FreeConnectionInfoMembers (*cipp);

	free (*cipp);

	*cipp = NULL;


	return;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
void
FreeConnectionInfoMembers (cip)

register
connectionInfo	*cip;
{
	/*----------------------------------------------------------*/
	/*
	*/
	static	char	FnName []	= "FreeConnectionInfoMembers";


	/*----------------------------------------------------------*/
	/*
	*/
	if (cip == NULL)
		return;

	if (cip->fd != -1)
		DisconnectSystem (cip);

	if (cip->netConfig_p != NULL) {
		cip->netConfig_p = NULL;
	}
	if (cip->ndAddrList_p != NULL) {
		netdir_free (cip->ndAddrList_p, ND_ADDRLIST);
		cip->ndAddrList_p = NULL;
	}
	if (cip->providerInfo_p != NULL) {
		free (cip->providerInfo_p);
		cip->providerInfo_p = NULL;
	}
	if (cip->returnBinding_p != NULL) {
		t_free (cip->returnBinding_p, T_BIND);
		cip->returnBinding_p = NULL;
	}
	if (cip->sendCall_p != NULL) {
		t_free (cip->sendCall_p, T_CALL);
		cip->sendCall_p = NULL;
	}
	if (cip->receiveCall_p != NULL) {
		t_free (cip->receiveCall_p, T_CALL);
		cip->receiveCall_p = NULL;
	}
	if (cip->xdrStream1_p != NULL) {
		xdr_destroy (cip->xdrStream1_p);
		free (cip->xdrStream1_p);
		cip->xdrStream1_p = NULL;
	}
	if (cip->xdrStream2_p != NULL) {
		xdr_destroy (cip->xdrStream2_p);
		free (cip->xdrStream2_p);
		cip->xdrStream2_p = NULL;
	}
	if (cip->physicalMsgBuffer_p != NULL) {
		free (cip->physicalMsgBuffer_p);
		cip->physicalMsgBuffer_p	= NULL;
		cip->logicalMsgBuffer_p		= NULL;
		cip->physicalMsgBufferSize	= 0;
		cip->logicalMsgBufferSize	= 0;
		cip->logicalMsgSize		= 0;
	}


	return;
}
/*==================================================================*/

/*=================================================================*/
/*
*/
char *
TransportStateDescription (state)

int	state;
{
	/*---------------------------------------------------------*/
	/*
	*/
		char	*state_p;


	/*---------------------------------------------------------*/
	/*
	*/
	switch (state) {
	case	T_UNBND:
		state_p = "Unbound.";
		break;

	case	T_IDLE:
		state_p = "Idle.";
		break;

	case	T_OUTCON:
		state_p = "Outgoing connection pending.";
		break;

	case	T_INCON:
		state_p = "Incoming connection pending.";
		break;

	case	T_DATAXFER:
		state_p = "Data transfer.";
		break;

	case	T_OUTREL:
		state_p = "Outgoing orderly release.";
		break;

	case	T_INREL:
		state_p = "Incoming orderly release.";
		break;

	default:
		state_p = "Unknown state.";
	}


	return	state_p;
}
/*=================================================================*/

/*=================================================================*/
/*
*/
char *
TransportEventDescription (event)

int	event;
{
	/*---------------------------------------------------------*/
	/*
	*/
		char	*event_p;


	/*---------------------------------------------------------*/
	/*
	*/
	switch (event) {
	case	0:
		event_p = "No event pending.";
		break;

	case	T_LISTEN:
		event_p = "Connection indication received.";
		break;

	case	T_CONNECT:
		event_p = "Connect confirmation received.";
		break;

	case	T_DATA:
		event_p = "Normal data received.";
		break;

	case	T_EXDATA:
		event_p = "Expedited data received.";
		break;

	case	T_DISCONNECT:
		event_p = "Disconnect received.";
		break;

	case	T_ERROR:
		event_p = "Fatal error indication.";
		break;

	case	T_UDERR:
		event_p = "Datagram error indication.";
		break;

	case	T_ORDREL:
		event_p = "Orderly release indication.";
		break;

	default:
		event_p = "Unknown event.";
	}


	return	event_p;
}
/*=================================================================*/

#ifdef	DEBUG
/*=================================================================*/
/*
*/
void
PrintSystemInfo (sip)

systemInfo	*sip;
{
	/*---------------------------------------------------------*/
	/*
	*/
	char	*systemType_p;


	/*---------------------------------------------------------*/
	/*
	*/
	if (sip == NULL)
	{
		(void)	fprintf (_Debugp,
			"PrintSystemInfo: sip == NULL");
		(void)	fflush (_Debugp);
		return;
	}
	(void)	fprintf (_Debugp, "systemName          =: %s\n", 
		(sip->systemName_p == NULL ? "<NULL>" : sip->systemName_p));

	(void)	fprintf (_Debugp, "systemPassword      =: %s\n",
		(sip->systemPassword_p == NULL ? "<NULL>" :
		sip->systemPassword_p));

	switch (sip->systemType) {
	case	UnknownSystem:
		systemType_p = "unknown";
		break;

	case	SystemVSystem:
		systemType_p = "s5";
		break;

	case	BerkeleySystem:
		systemType_p = "bsd";
		break;

	default:
		systemType_p = "**undefined**";
		break;
	}

	(void)	fprintf (_Debugp, "systemType          =: %s\n", systemType_p);
	(void)	fprintf (_Debugp, "timeout (mins)      =: %d\n", sip->timeout);
	(void)	fprintf (_Debugp, "retry (mins)        =: %d\n", sip->retry);
	(void)	fprintf (_Debugp, "ncProtoFamily       =: %s\n", 
		(sip->ncProtoFamily_p == NULL ? "<Null>"
		: sip->ncProtoFamily_p));
	(void)	fprintf (_Debugp, "ndHostServ.h_host   =: %s\n", 
		(sip->ndHostServ.h_host == NULL ? "<Null>"
		: sip->ndHostServ.h_host));
	(void)	fprintf (_Debugp, "ndHostServ.h_serv   =: %s\n", 
		(sip->ndHostServ.h_serv == NULL ? "<Null>"
		: sip->ndHostServ.h_serv));
	(void)	fprintf (_Debugp, "ndOption            =: %d\n", sip->ndOption);
	(void)	fflush (_Debugp);


	return;
}
/*=================================================================*/

/*=================================================================*/
/*
**	For I_LIST ioctl.
**
**
**	struct str_mlist {
**		char l_name[FMNAMESZ+1];
**	};
**
**	struct str_list {
**		int sl_nmods;
**		struct str_mlist *sl_modlist;
**	};
**
*/
void
PrintStreamModules (fd)

int	fd;
{
	/*---------------------------------------------------------*/
	/*
	*/
		int	i;
		struct
		str_mlist	mlist [32];
		struct
		str_list	strlist;
	extern	int	errno;


	/*---------------------------------------------------------*/
	/*
	*/
	strlist.sl_nmods = 32;
	strlist.sl_modlist = mlist;

	if (ioctl (fd, I_LIST, &strlist) == -1) {
		(void)	fprintf (_Debugp,
		"ERROR: trace=(PrintStreamModules/ioctl), errno = %d\n",
		errno);
		(void)	fflush (_Debugp);
		return;
	}


	/*---------------------------------------------------------*/
	/*
	*/
	(void)	fprintf (_Debugp, "NModules = %d\n", strlist.sl_nmods);

	for (i=0; i < strlist.sl_nmods; i++)
		(void)	fprintf (_Debugp,
		"[%02d] %s\n", i, strlist.sl_modlist[i].l_name);

	(void)	fflush (_Debugp);


	return;
}
/*=================================================================*/

/*=================================================================*/
/*
*/
void
PrintTransportInfo (transportInfo_p)

t_info	*transportInfo_p;
{
	(void)	fprintf (_Debugp,
		"addr       : %ld\n", transportInfo_p->addr);
	(void)	fprintf (_Debugp,
		"options    : %ld\n", transportInfo_p->options);
	(void)	fprintf (_Debugp,
		"tsdu       : %ld\n", transportInfo_p->tsdu);
	(void)	fprintf (_Debugp,
		"etsdu      : %ld\n", transportInfo_p->etsdu);
	(void)	fprintf (_Debugp,
		"connect    : %ld\n", transportInfo_p->connect);
	(void)	fprintf (_Debugp,
		"disconnect : %ld\n", transportInfo_p->discon);
	(void)	fprintf (_Debugp,
		"servtype   : %ld\n", transportInfo_p->servtype);
	(void)	fflush (_Debugp);

	return;
}
/*=================================================================*/

/*=================================================================*/
/*
*/
void
PrintTransportState (fd)

int	fd;
{
	/*---------------------------------------------------------*/
	/*
	*/
		int	state;
	static	char	FnName []	= "PrintTransportState";
	extern	int	errno;
	extern	int	t_errno;


	/*---------------------------------------------------------*/
	/*
	*/
	state = t_getstate (fd);
	if (state == -1) {
		if (t_errno == TSYSERR) {
			(void)	fprintf (_Debugp,
			"ERROR:  trace=(%s/t_getstate), errno = %d\n",
			FnName, errno);
		}
		else {
			(void)	fprintf (_Debugp,
			"ERROR:  trace=(%s/t_getstate), t_errno = %d\n",
			FnName, t_errno);
		}
		(void)	fflush (_Debugp);
		return;
	}

	(void)
	fprintf (_Debugp, "Transport state:  %s\n", 
		TransportStateDescription (state));
	(void)	fflush (_Debugp);

	return;
}
/*=================================================================*/

/*=================================================================*/
/*
*/
void
PrintTransportEvent (fd)

int	fd;
{
	/*---------------------------------------------------------*/
	/*
	*/
		int	event;
	static	char	FnName []	= "PrintTransportEvent";
	extern	int	errno;
	extern	int	t_errno;


	/*---------------------------------------------------------*/
	/*
	*/
	event = t_look (fd);

	if (event == -1)
	{
		if (t_errno == TSYSERR) {
			(void)	fprintf (_Debugp,
			"ERROR:  trace=(%s/t_look), errno= %d\n",
			FnName, errno);
		}
		else {
			(void)	fprintf (_Debugp,
			"ERROR:  trace=(%s/t_look), t_errno= %d\n",
			FnName, t_errno);
		}
		(void)	fflush (_Debugp);
		return;
	}
	(void)	fprintf (_Debugp, "Transport event:  %s\n",
		TransportEventDescription (event));
	(void)	fflush (_Debugp);


	return;
}
/*=================================================================*/
#endif
