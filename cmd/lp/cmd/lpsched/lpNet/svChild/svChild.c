/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/




#ident	"@(#)lp:cmd/lpsched/lpNet/svChild/svChild.c	1.9.3.1"

/*=================================================================*/
/*
*/
#include	<unistd.h>
#include	<stdlib.h>
#include	<signal.h>
#include	<errno.h>
#include	<string.h>
#include	"lpNet.h"
#include	"networkMgmt.h"
#include	"logMgmt.h"
#include	"errorMgmt.h"
#include	"boolean.h"
#include	"lists.h"
#include	"memdup.h"
#include	"debug.h"

#ifndef	_POLLFD_T
typedef	struct	pollfd		pollfd_t;
#endif

static	boolean	AlarmTrapFlag	= False;
extern	int	errno;
extern	int	Lp_NTBase;

/*-----------------------------------------------------------------*/
/*
*	Local functions.
*/
#ifdef	__STDC__

static	int	SendJobToRemote (char *);
static	void	LpExecEvent (pollfd_t *);
static	void	RemoteSystemEvent (pollfd_t *);
static	void	Shutdown (boolean);
static	void	NotifyLpExecOfJobStatus (int, char *);
static	void	SoftwareTerminationTrap (int);
static	void	AlarmTrap (int);
static	list	*SendJobToLpSched (list *);
static	boolean	ConnectToRemoteChild (void);

extern	char	**getjobfiles (char *);

#else

static	int	SendJobToRemote ();
static	void	LpExecEvent ();
static	void	RemoteSystemEvent ();
static	void	Shutdown ();
static	void	NotifyLpExecOfJobStatus ();
static	void	SoftwareTerminationTrap ();
static	void	AlarmTrap ();
static	list	*SendJobToLpSched ();
static	boolean	ConnectToRemoteChild ();

extern	char	**getjobfiles ();

#endif
/*=================================================================*/

/*=================================================================*/
/*
*/
void
svChild ()
{
	/*----------------------------------------------------------*/
	/*
	*/
		enum {
			lpExec		= 0,
			remoteSystem	= 1
		} pipeIds;

		int	i,
			nFiles,
			timeoutValue,
			nEvents,
			save;
		char	msgbuf [MSGMAX];
		short	status;
		pollfd_t	pipeEvents [2];
	static	char	FnName []	= "svChild";


	ENTRYP
	/*---------------------------------------------------------*/
	/*
	**	Some initialization stuff:
	**
	**	o  Trap the 'SIGTERM' signal.
	*/
	(void)	sigset (SIGTERM, SoftwareTerminationTrap);
	(void)	sigset (SIGALRM, AlarmTrap);

	WriteLogMsg ("Starting.");

	if (mread (ProcessInfo.lpExecMsg_p, msgbuf, sizeof (msgbuf)) == -1)
		TrapError (Fatal, Unix, FnName, "mread");

	TRACE (mtype (msgbuf))
	if (mtype (msgbuf) != S_CHILD_SYNC)
		TrapError (Fatal, Internal, FnName, 
			"Bad message from lpExec.  Expected S_CHILD_SYNC.");

	if (getmessage (msgbuf, S_CHILD_SYNC, &status) == -1)
		TrapError (Fatal, Unix, FnName, "getmessage");


	if (status != MOK)
	{
		WriteLogMsg ("Child services aborted.");
		Shutdown (False);
	}


	/*---------------------------------------------------------*/
	/*
	**	This is the main loop of the child.
	**	Some points of interest:
	**	
	**	o  We always poll the lpExec pipe but we don't
	**	   always have a connection to the remote system
	**	   we service.
	**
	**	o  The timeoutValue to poll is dependent upon
	**	   whether we have a connection or not.  If we
	**	   have a connection then the timeoutValue reflects
	**	   when we should shutdown the child for inactivity.
	**	   If we don't have a connection the timeoutValue
	**	   reflects when we should try to connect to the remote
	**	   system again.  Until we reach the remote system
	**	   the child is considered inactive and may timeout
	**	   anyway.
	*/
	if (ProcessInfo.processRank == MasterChild)
		SetJobPriority (2);
	else
		SetJobPriority (1);

	pipeEvents [lpExec].fd		= ProcessInfo.lpExec;
	pipeEvents [lpExec].events	= POLLIN;
	nFiles = 1;

	if (Connected (CIP) || ConnectToRemoteChild ())
	{
		pipeEvents [remoteSystem].fd	 = CIP->fd;
		pipeEvents [remoteSystem].events = POLLIN;
		nFiles = 2;
	}
	timeoutValue = -1;

do
{
	/*-------------------------------------------------*/
	/*
	**  For nEvents:
	**	==  0:  we timed-out.
	**	== -1:  an error occurred.
	**	>   0:  the number of fd's that have
	**		events.
	**  Turn off the alarm immediately.
	*/
	TRACEd (nFiles)
	TRACEd (timeoutValue)

	nEvents = poll (pipeEvents, nFiles, timeoutValue);
	save = errno;
	(void)	alarm (0);

	TRACEd (nEvents)
	TRACEd ((errno=save))

	if (nEvents == -1)
	switch (errno=save) {
	case	EAGAIN:
		continue;

	case	EINTR:
		if (AlarmTrapFlag)
		{
			TRACEP ("Alarm caught in poll.");
			AlarmTrapFlag = False;
			if (ConnectToRemoteChild ())
			{
				pipeEvents [remoteSystem].fd	 = CIP->fd;
				pipeEvents [remoteSystem].events = POLLIN;
				nFiles = 2;
				timeoutValue = SIP->timeout;
				if (timeoutValue > 0)
					timeoutValue *= 60000;
				LpExecEvent (NULL);
			}
		}
		continue;

	case	EFAULT:
	case	EINVAL:
	default:
		TrapError (Fatal, Unix, FnName, "poll");
	}
	if (nEvents == 0)
		Shutdown (True);


	/*-------------------------------------------------*/
	/*
	*/
	for (i=nFiles-1; i >= 0; i--)
	{
		if (pipeEvents [i].revents)
		switch (i) {
		case	lpExec:
			LpExecEvent (&pipeEvents [i]);
			break;

		case	remoteSystem:
			RemoteSystemEvent (&pipeEvents [i]);
			break;
		}
	}
	/*-------------------------------------------------*/
	/*
	**  If we are connected then we are all set,
	**  otherwise, we may have lost our connection so
	**  reset nFiles and timeoutValue.  Then,
	**  schedule a retry.
	*/
	if (Connected (CIP))
	{
		continue;
	}
	else
	if (ProcessInfo.processRank = SlaveChild)
	{
		Shutdown (True);
	}
	if (CIP)
	{
		FreeConnectionInfo (&CIP);
	}
	nFiles = 1;
	timeoutValue = -1;

	if (SIP->retry == -1)
		Shutdown (True);

	if (SIP->retry == 0)
	{
		if (ConnectToRemoteChild ())
		{
			LpExecEvent (NULL);
		}
		else
		{
			(void)	alarm (60);
		}
		continue;
	}
	else
		(void)	alarm (SIP->retry);

} while (ProcessInfo.processType == ChildProcess);  /*  Always true.  */
	/*---------------------------------------------------------*/


	EXITP
	return;
}
/*=================================================================*/

/*==================================================================*/
/*
*/
boolean
ConnectToRemoteChild ()
{
	/*----------------------------------------------------------*/
	/*
	*/
	static	char	FnName []	= "ConnectToRemoteChild";


	ENTRYP
	/*---------------------------------------------------------*/
	/*
	*/
	CIP = ConnectToSystem (SIP);

	if (CIP == NULL)
		goto	errorReturn_2;

	if (! ConnectToService (CIP, LP_SERVICE_CODE))
		goto	errorReturn_1;

	if (! SendSystemIdMsg (CIP, NULL, 0))
		goto	errorReturn_1;

	WriteLogMsg ("Connected to remote child.");

	EXITP
	return	True;


	/*----------------------------------------------------------*/
	/*
	*/
errorReturn_1:
	FreeConnectionInfo (&CIP);

errorReturn_2:
	WriteLogMsg ("Could not connect to remote child.");
	EXITP
	return	False;
}
/*==================================================================*/

/*==================================================================*/
/*
**	lpExec sent a message to the child and this is
**	the function that handles it.
*/
void
LpExecEvent (pipeEvents_p)

pollfd_t	*pipeEvents_p;
{
	/*----------------------------------------------------------*/
	/*
	*/
		int		i,
				jobId,
				listLength;
		char		msgbuf [MSGMAX],
				*msgp;
		list		*dataList_p = NULL,
				*fileList_p = NULL;
		boolean		jobPending,
				badEvent    = False;

	register	uint	revents   = 0;
	static		list	*MsgListp = NULL;
	static		char	FnName [] = "LpExecEvent";


	ENTRYP
	/*---------------------------------------------------------*/
	/*
	**	We are most concerned w/ POLLIN.  POLLIN can occur
	**	w/ POLLHUP and POLLERR.
	*/
	if (pipeEvents_p == NULL)
	{
		if (Connected (CIP))
		{
			if (MsgListp)
			{
				msgp = (char *)PopListMember (MsgListp);
				if (LengthOfList (MsgListp) == 0)
					FreeList (&MsgListp);
				goto	_switch;
			}
		}
		return;
	}
	else
	{
		revents = (unsigned int)  pipeEvents_p->revents;
		TRACE (revents)
	}
_POLLIN:
	if (! (revents & POLLIN))
		goto	_POLLPRI;


	/*---------------------------------------------------------*/
	/*
	*/
	if (mread (ProcessInfo.lpExecMsg_p, msgbuf, sizeof (msgbuf)) == -1)
	{
		TrapError (Fatal, Unix, FnName, "mread");
	}
	if (mtype (msgbuf) == S_SHUTDOWN)
	{
		WriteLogMsg ("Instructed to shutdown.");
		Shutdown (False);
	}
	if (Connected (CIP))
	{
		if (MsgListp)
		{
			msgp = (char *)memdup (msgbuf, msize (msgbuf));
			if (!msgp)
				TrapError (Fatal, Unix, FnName, "memdup");
			if (! AppendToList (MsgListp, msgp, msize (msgp)))
				TrapError (Fatal, Unix, FnName,
				"AppendToList");
			msgp = (char *)PopListMember (MsgListp);
		}
		else
			msgp = msgbuf;
	}
	else
	{
		msgp = (char *)memdup (msgbuf, msize (msgbuf));
		if (! MsgListp)
		{
			MsgListp = NewList (PointerList, 0);
			if (MsgListp == NULL)
				TrapError (Fatal, Unix, FnName, "NewList");
		}
		if (! AppendToList (MsgListp, msgp, msize (msgp)))
			TrapError (Fatal, Unix, FnName, "AppendToList");
		return;
	}
_switch:
	switch (mtype (msgp)) {
	case	S_SEND_JOB:
s_send_job:
		/*
		**  If 'jobId' is -1 then there may be a job from the
		**  remote that has a greater priority than we do.
		*/
		jobId = SendJobToRemote (msgp);
		jobPending = JobPending (CIP);
		if (jobId == -1)
		{
			if (Connected (CIP))
			{
				/*
				**  Our job may have failed because
				**  we lost out in the job-priority
				**  negotiations.  Therefore, there
				**  is an immediate job pending so
				**  fake the remote system event.
				*/
				if (jobPending)
				{
					RemoteSystemEvent (NULL);
					goto	s_send_job;
				}
			}
			else
				FreeConnectionInfo (&CIP);
			NotifyLpExecOfJobStatus (MTRANSMITERR, 0);
			break;
		}
		jobId = ReceiveJob (CIP, &dataList_p, &fileList_p);

		if (jobId == -1)
		{
			if (!Connected (CIP))
				FreeConnectionInfo (&CIP);
			NotifyLpExecOfJobStatus (MTRANSMITERR, 0);
			break;
		}
		listLength = LengthOfList (dataList_p);

		TRACE (listLength)
		for (i=0; i < listLength-1; i++)
			NotifyLpExecOfJobStatus (MOKMORE,
				(char *)ListMember (dataList_p, i));

		TRACE (i)
		TRACEs (ListMember (dataList_p, i))
		NotifyLpExecOfJobStatus (MOK,
			(char *)ListMember (dataList_p, i));

		FreeList (&dataList_p);
		FreeList (&fileList_p);
		/*
		**  Check and see if we pre-empted a job during
		**  the underlying job priority negotiations.
		**  If we did then immediately fake a 
		**  remote system event.
		*/
		if (jobPending)
		{
			RemoteSystemEvent (NULL);
		}
		break;

	case	S_SHUTDOWN:
		break;

	default:
		TrapError (Fatal, Internal, FnName, 
			"Received unknown message from lpExec.");
	}


	/*---------------------------------------------------------*/
	/*
	*/
_POLLPRI:
	if (revents & POLLPRI)
	{
		TRACEP ("POLLPRI")
		badEvent = True;
		TrapError (NonFatal, Internal, FnName,
		"Bad event (POLLPRI) on lpExec pipe.");
	}
_POLLERR:
	if (revents & POLLERR)
	{
		TRACEP ("POLLERR")
		badEvent = True;
		TrapError (NonFatal, Internal, FnName,
		"Bad event (POLLERR) on lpExec pipe.");
	}
_POLLNVAL:
	if (revents & POLLNVAL)
	{
		TRACEP ("POLLNVAL")
		badEvent = True;
		TrapError (NonFatal, Internal, FnName,
		"Bad event (POLLNVAL) on lpExec pipe.");
	}
_POLLHUP:
	if (revents & POLLHUP)
	{
		TRACEP ("POLLHUP")
		badEvent = True;
		TrapError (Fatal, Internal, FnName,
		"Broken lpExec pipe.");
	}
	if (badEvent)
		TrapError (Fatal, Internal, FnName, "Cannot recover.");


	EXITP
	return;
}
/*=================================================================*/

/*==================================================================*/
/*
**	The remote system has sent us a message.
*/
void
RemoteSystemEvent (pipeEvents_p)

pollfd_t	*pipeEvents_p;
{
	/*----------------------------------------------------------*/
	/*
	*/
		int		jobId;
		list		*listOfReplies_p,
				*dataList_p	= NULL,
				*fileList_p	= NULL;
		boolean		badEvent	= False;

	register	uint	revents   = 0;
	static		char	FnName [] = "RemoteSystemEvent";


	ENTRYP
	/*---------------------------------------------------------*/
	/*
	**	We are most concerned w/ POLLIN.  POLLIN can occur
	**	w/ POLLHUP and POLLERR.
	*/
	if (pipeEvents_p == NULL)
	{
		revents = POLLIN;
	}
	else
	{
		revents = (unsigned int)  pipeEvents_p->revents;
	}
	TRACE (revents)

_POLLIN:
	if (! (revents & POLLIN))
		goto	_POLLPRI;


	/*---------------------------------------------------------*/
	/*
	**	Before we assume a job is waiting we want to
	**	check the state of our connection.  On some
	**	transports we get POLLIN to signify an event
	**	has occurred - the event being an
	**	'Orderly release indication'.  Therefore, we
	**	want to check for a usable connection not just
	**	a POLLIN.
	*/
	TRACEP ("POLLIN")
	if (! Connected (CIP))
	{
		DisconnectSystem (CIP);
		FreeConnectionInfo (&CIP);
		goto	_POLLPRI;
	}
	jobId = ReceiveJob (CIP, &dataList_p, &fileList_p);

	if (jobId == -1)
	{
		if (!Connected (CIP))
		{
			DisconnectSystem (CIP);
			FreeConnectionInfo (&CIP);
		}
		goto	_POLLPRI;
	}
	TRACEs (ListMember (dataList_p, 0))
	TRACEs (ListMember (fileList_p, 0))

#ifdef	DEBUG2
if (getenv ("DEBUG_SVCHILD") == NULL)
{
#endif
	listOfReplies_p = SendJobToLpSched (dataList_p);

	if (listOfReplies_p == NULL)
		goto	freeData;

#ifdef	DEBUG2
}
else
{
	char	*p;

	p = strdup ("Hello, world.");

	listOfReplies_p = NewList (StringList, 0);

	(void)	AppendToList (listOfReplies_p, p, 0);
}
#endif

	/*---------------------------------------------------------*/
	/*
	**  'jobid' should only be -1 if the connection was lost.
	*/
	jobId = SendJob (CIP, listOfReplies_p, NULL, NULL);

	if (jobId == -1)
	{
		if (! Connected (CIP))
			FreeConnectionInfo (&CIP);
	}
freeData:
	FreeList (&dataList_p);
	FreeList (&fileList_p);
	FreeList (&listOfReplies_p);


	/*---------------------------------------------------------*/
	/*
	*/
_POLLPRI:
	if (revents & POLLPRI)
	{
		TRACEP ("POLLPRI")
		badEvent = True;
		TrapError (NonFatal, Internal, FnName,
		"Bad event (POLLPRI) on remote pipe.");
	}
_POLLERR:
	if (revents & POLLERR) {
		TRACEP ("POLLERR")
		badEvent = True;
		TrapError (NonFatal, Internal, FnName,
		"Bad event (POLLERR) on remote pipe.");
	}
_POLLNVAL:
	if (revents & POLLNVAL) {
		TRACEP ("POLLNVAL")
		badEvent = True;
		TrapError (NonFatal, Internal, FnName,
		"Bad event (POLLNVAL) on remote pipe.");
	}
_POLLHUP:
	if (revents & POLLHUP)
	{
		TRACEP ("POLLHUP")
		;
	}

	if (badEvent)
		TrapError (Fatal, Internal, FnName, "Cannot recover.");


	EXITP
	return;
}
/*==================================================================*/

/*==================================================================*/
/*
**	This function takes the HPI message pointed to by
**	jobMsg_p and sends it to the remote along with
**	any files that are part of the job.
**
**	It does not wait for an answer to the job.
*/
int
SendJobToRemote (jobMsg_p)

char	*jobMsg_p;
{
	/*----------------------------------------------------------*/
	/*
	*/
		int		i,
				jobId;
		char		*remoteMsg_p,
				*systemName_p,
				*requestFileName_p,
				**fileList_pp,
				*srcFilePath_p,
				*destFilePath_p;
		list		*dataList_p	= NULL,
				*srcFileList_p	= NULL,
				*destFileList_p	= NULL;
		short		msgSize,
				fileType;
		boolean		good;

	static	char	FnName []	= "SendJobToRemote";

	ENTRYP
	/*----------------------------------------------------------*/
	/*
	**
	*/
	if (getmessage (jobMsg_p, S_SEND_JOB, &systemName_p,
		&fileType, &requestFileName_p, &msgSize,
		&remoteMsg_p) == -1)
		TrapError (Fatal, Unix, FnName, "getmessage");

	if ((remoteMsg_p = (char *)memdup (remoteMsg_p, msgSize)) == NULL)
		TrapError (Fatal, Unix, FnName, "memdup");

	if ((dataList_p = NewList (PointerList, 0)) == NULL)
		TrapError (Fatal, Unix, FnName, "NewList");

	if (! AppendToList (dataList_p, remoteMsg_p, msgSize))
		TrapError (Fatal, Unix, FnName, "AppendToList");


	/*----------------------------------------------------------*/
	/*
	**	Check to make sure the system name matches with
	**	the remote system we are connected to.
	*/
	if (strcmp (systemName_p, SIP->systemName_p) != 0)
		TrapError (Fatal, Internal, FnName, "lpExec confused.");


	/*----------------------------------------------------------*/
	/*
	**	If filetype is zero then the 'requestFile' is
	**	a data file that is to be sent uninterpreted.
	**
	**	If the fileType is 1 then the 'requestFile' is
	**	a true request-file and needs to be parsed and
	**	all the files it names need to be sent to the
	**	remote.
	*/
	if (strlen (requestFileName_p) == 0)
		goto	sendJob;

	switch (fileType) {
	case	0:
		requestFileName_p = strdup (requestFileName_p);

		if (requestFileName_p == NULL)
			TrapError (Fatal, Unix, FnName, "strdup");

		srcFileList_p  = NewList (StringList, 0);

		if (srcFileList_p == NULL)
			TrapError (Fatal, Unix, FnName, "NewList");

		if (! AppendToList (srcFileList_p, requestFileName_p, 0))
			TrapError (Fatal, Unix, FnName, "AppendToList");

		destFileList_p = NULL;
/*
#ifdef	DEBUG
{
		char	path [128];

		(void)	sprintf (path, "/tmp/lpNet/%s",
			(*requestFileName_p == '/' ?
			(requestFileName_p+1) : requestFileName_p));

		requestFileName_p = strdup (path);

		if (requestFileName_p == NULL)
			TrapError (Fatal, Unix, FnName, "strdup");

		destFileList_p = NewList (StringList, 0);

		if (destFileList_p == NULL)
			TrapError (Fatal, Unix, FnName, "NewList");

		if (! AppendToList (destFileList_p, requestFileName_p, 0))
			TrapError (Fatal, Unix, FnName, "AppendToList");

}
#endif
*/
		break;

	case	1:
		srcFileList_p  = NewList (StringList, 0);
		destFileList_p = NewList (StringList, 0);

		if (srcFileList_p == NULL || destFileList_p == NULL)
			TrapError (Fatal, Unix, FnName, "NewList");
		fileList_pp = getjobfiles (requestFileName_p);

		if (fileList_pp == NULL)
			TrapError (Fatal, Unix, FnName, "getjobfiles");

		for (i=0; fileList_pp [i] != NULL; i++) {
			srcFilePath_p   = fileList_pp [i];
			fileList_pp [i] = NULL;
			destFilePath_p  = strdup (srcFilePath_p + Lp_NTBase);

			if (destFilePath_p == NULL)
			TrapError (Fatal, Unix, FnName, "strdup");

			TRACEs (srcFilePath_p)
			if (!AppendToList (srcFileList_p, srcFilePath_p, 0))
			TrapError (Fatal, Unix, FnName, "AppendToList");

			TRACEs (destFilePath_p)
			if (!AppendToList (destFileList_p, destFilePath_p, 0))
			TrapError (Fatal, Unix, FnName, "AppendToList");
		}
		break;

	default:
		TrapError (NonFatal, Internal, FnName,
			"Unknown file-type in S_SEND_JOB message.");
		EXITP
		return	-1;
	}


	/*----------------------------------------------------------*/
	/*
	*/
sendJob:
	jobId = SendJob (CIP, dataList_p, srcFileList_p, destFileList_p);

	if (jobId != -1)
	{
		(void)	ApplyToList (srcFileList_p, (void *(*)())unlink,
				     EmptyList, 0);
	}
	FreeList (&srcFileList_p);
	FreeList (&destFileList_p);


	EXITP
	return	jobId;
}
/*==================================================================*/

/*==================================================================*/
/*
**	This function sends the job to lpSched and gets the reply.
*/
list *
SendJobToLpSched (msgs_p)

list	*msgs_p;
{
	/*----------------------------------------------------------*/
	/*
	*/
		int	i,
			listLength,
			msgType,
			msgSize;
		char	msgBuffer [512],
			*member_p,
			*fooChar_p;
		list	*listOfReplies_p;
		long	fooLong;
		short	statusCode;
		short	fooShort;
		boolean	done;

	static	boolean	PipeNotOpened	= True;
	static	char	FnName []	= "SendJobToLpSched";


	ENTRYP
	/*----------------------------------------------------------*/
	/*
	*/
	if (PipeNotOpened) {
		if (mopen () == -1) {
			TrapError (NonFatal, Unix, FnName, "mopen");
			EXITP
			return	NULL;
		}

		PipeNotOpened = False;
	}
	listLength = LengthOfList (msgs_p);  TRACE (listLength)
	for (i=0; i < listLength; i++)
	{
		
		if (msend (ListMember (msgs_p, i)) == -1)
		{
			TrapError (NonFatal, Unix, FnName, "msend");
			goto	errorReturn_1;
		}
	}


	/*----------------------------------------------------------*/
	/*
	*/
	listOfReplies_p = NewList (PointerList, 0);

	if (listOfReplies_p == NULL)
		TrapError (Fatal, Unix, FnName, "NewList");

	done = False;

	do
	{
		msgType = mrecv (msgBuffer, sizeof (msgBuffer));

		if (msgType == -1)
		{
			TrapError (NonFatal, Unix, FnName, "mrecv");
			goto	errorReturn_2;
		}

		msgSize  = msize (msgBuffer);
		member_p = (char *)  memdup (msgBuffer, msgSize);

		if (member_p == NULL)
			TrapError (Fatal, Unix, FnName, "memdup");

		if (! AppendToList (listOfReplies_p, member_p, msgSize))
			TrapError (Fatal, Unix, FnName, "AppendToList");

		switch (msgType) {
		case	R_PRINT_REQUEST:
			done = True;
			break;

		case	R_JOB_COMPLETED:
			done = True;
			break;

		case	R_CANCEL:
			/*
			**	Can have more than one reply.
			*/
			if (getmessage (msgBuffer, R_CANCEL,
			    &statusCode, &fooLong, &fooChar_p) == -1)
			{
				TrapError (Fatal, Unix, FnName, "getmessage");
			}
			if (statusCode != MOKMORE)
				done = True;
			break;

		case	R_INQUIRE_PRINTER_STATUS:
			/*
			**	Can have more than one reply.
			*/
			if (getmessage (msgBuffer, R_INQUIRE_PRINTER_STATUS,
			    &statusCode,
			    &fooChar_p, &fooChar_p, &fooChar_p, &fooChar_p,
			    &fooChar_p, &fooShort, &fooChar_p, &fooChar_p,
			    &fooChar_p) == -1)
			{
				TrapError (Fatal, Unix, FnName, "getmessage");
			}
			if (statusCode != MOKMORE)
				done = True;
			break;

		case	R_GET_STATUS:
			done = True;
			break;

		case	R_INQUIRE_REQUEST:
			if (getmessage (msgBuffer, R_INQUIRE_REQUEST,
			    &statusCode, &fooChar_p, &fooChar_p, &fooLong,
			    &fooLong, &fooShort, &fooChar_p, &fooChar_p,
			    &fooChar_p) == -1)
			{
				TrapError (Fatal, Unix, FnName, "getmessage");
			}
			if (statusCode != MOKMORE)
				done = True;
			break;

		case	R_INQUIRE_REQUEST_RANK:
			if (getmessage (msgBuffer, R_INQUIRE_REQUEST_RANK,
			    &statusCode, &fooChar_p, &fooChar_p, &fooLong,
			    &fooLong, &fooShort, &fooChar_p, &fooChar_p,
			    &fooChar_p, &fooShort) == -1)
			{
				TrapError (Fatal, Unix, FnName, "getmessage");
			}
			if (statusCode != MOKMORE)
				done = True;
			break;

		default:
			TRACEP ("Bad message from lpsched.")
			TRACE (msgType)
			TrapError (Fatal, Internal, FnName,
			"Unknown message received from lpSched.");
		}
	} while (! done);


	EXITP
	return	listOfReplies_p;


	/*----------------------------------------------------------*/
	/*
	*/
errorReturn_2:
	FreeList (&listOfReplies_p);

errorReturn_1:
	(void)	mclose ();
	PipeNotOpened = True;


	EXITP
	return	NULL;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
void
Shutdown (notifyLpExec)

boolean	notifyLpExec;
{
	/*----------------------------------------------------------*/
	/*
	*/
		char	msgBuffer [128];
	static	char	FnName []	= "Shutdown";

	ENTRYP
	/*----------------------------------------------------------*/
	/*
	*/
	if (Connected (CIP))
	{
		DisconnectSystem (CIP);
		FreeConnectionInfo (&CIP);
	}
	if (notifyLpExec &&
	    mputm (ProcessInfo.lpExecMsg_p, S_SHUTDOWN, 0) == -1)
		TrapError (NonFatal, Unix, FnName, "mputm");

	EXITP
	Exit (0);
}
/*==================================================================*/

/*==================================================================*/
/*
*/
void
NotifyLpExecOfJobStatus (status, msg_p)

int	status;
char	*msg_p;
{
	/*----------------------------------------------------------*/
	/*
	*/
	static	char	FnName []	= "NotifyLpExecOfJobStatus";


	ENTRYP
	/*----------------------------------------------------------*/
	/*
	*/
#ifdef	DEBUG
if (getenv ("DEBUG_SVCHILD") == NULL) {
#endif
	if (mputm (ProcessInfo.lpExecMsg_p, R_SEND_JOB,
	    SIP->systemName_p, (short) status,
	    msg_p == NULL ? 0 : (short) msize (msg_p),
	    msg_p == NULL ? "" : msg_p) == -1)
	{
		TrapError (Fatal, Unix, FnName, "mputm");
	}
#ifdef	DEBUG
}
else
	if (mputm (ProcessInfo.lpExecMsg_p, R_SEND_JOB,
	    SIP->systemName_p, (short) status,
	    msg_p == NULL ? 0 : strlen(msg_p)+1,
	    msg_p == NULL ? "" : msg_p) == -1)
	{
		TrapError (Fatal, Unix, FnName, "mputm");
	}
#endif

	EXITP
	return;
}
/*==================================================================*/

/*==================================================================*/
/*
**	This function is used to trap the SIGTERM signal.
**
**	An lpNet child may receive it from the lpNet parent
**	if the parent can't send lpExec the file-descriptor
**	to talk with the child.
**
*/
void
SoftwareTerminationTrap (signalNumber)

int	signalNumber;
{
	/*----------------------------------------------------------*/
	/*
	*/
	signalNumber = signalNumber;

	WriteLogMsg ("Received 'SIGTERM' signal, exiting.");

	Shutdown (True);	/*  Does not return.  */
}
/*==================================================================*/

/*==================================================================*/
void
AlarmTrap (signal)

int	signal;
{
	AlarmTrapFlag = True;
	return;
}
/*==================================================================*/
