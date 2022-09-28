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
#ident	"@(#)lp:lib/lpNet/networkMgmt.c	1.6.2.1"

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/utsname.h>
#include	<rpc/rpc.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<libgen.h>
#include	<fcntl.h>
#include	<errno.h>
#include	<malloc.h>
#include	<string.h>
#include	"networkMgmt.h"
#include	"errorMgmt.h"
#include	"lists.h"
#include	"memdup.h"
#include	"debug.h"


	boolean		JobPendingFlag	= False;
struct	jobControl	JobControl;

extern	int		errno;
/*==================================================================*/

/*==================================================================*/
/*
*/
int
SendJob (cip, dataList_p, srcFileList_p, destFileList_p)

connectionInfo	*cip;
list		*dataList_p;
list		*srcFileList_p;
list		*destFileList_p;
{
	/*----------------------------------------------------------*/
	/*
	*/
		int	i,
			jobId,
			listLength;
		boolean	endOfJob    = True;

	static	char	FnName []	= "SendJob";


	ENTRYP
	/*----------------------------------------------------------*/
	/*
	**	Check our args.
	*/
	if (cip == NULL)
	{
		errno = EINVAL;
		TRACE (errno)
		EXITP
		return	-1;
	}
	if (!Connected (cip))
	{
		errno = ENOLINK;
		TRACE (errno)
		EXITP
		return	-1;
	}
	if (dataList_p == NULL && srcFileList_p == NULL)
	{
		errno = EINVAL;
		TRACE (errno)
		EXITP
		return	-1;
	}


	/*----------------------------------------------------------*/
	/*
	*/
	if ((jobId = NegotiateJobClearance (cip)) == -1)
	{
		TRACE (errno)
		EXITP
		return	-1;
	}


	/*----------------------------------------------------------*/
	/*
	**	Send data.
	*/
	endOfJob = False;

	listLength = LengthOfList (dataList_p);

	for (i=0; i < listLength-1; i++) {
		if (! SendData (cip, endOfJob,
		      ListMember (dataList_p, i),
		      SizeofListMember (dataList_p, i)))
		{
			int	save = errno;
			if (Connected (cip))
				SendJobControlMsg (cip, JobAborted);
			errno = save;
			TRACE (errno)
			EXITP
			return	-1;
		}
	}
	if (srcFileList_p == NULL)
	{
		endOfJob = True;
	}
	if (! SendData (cip, endOfJob, ListMember (dataList_p, i),
	      SizeofListMember (dataList_p, i)))
	{
		int	save = errno;
		if (Connected (cip))
			SendJobControlMsg (cip, JobAborted);
		errno = save;
		TRACE (errno)
		EXITP
		return	-1;
	}
	if (endOfJob)
	{
		EXITP
		return	jobId;
	}


	/*----------------------------------------------------------*/
	/*
	**	Send files.
	*/
	listLength = LengthOfList (srcFileList_p);

	for (i=0; i < listLength-1; i++) {
		if (! SendFile (cip, endOfJob,
		      ListMember (srcFileList_p, i),
		      destFileList_p == NULL ? NULL :
		      ListMember (destFileList_p, i)))
		{
			int	save = errno;
			if (Connected (cip))
				SendJobControlMsg (cip, JobAborted);
			errno = save;
			TRACE (errno)
			EXITP
			return	-1;
		}
	}
	endOfJob = True;

	if (! SendFile (cip, endOfJob,
	      ListMember (srcFileList_p, i),
	      destFileList_p == NULL ? NULL :
	      ListMember (destFileList_p, i)))
	{
		int	save = errno;
		if (Connected (cip))
			SendJobControlMsg (cip, JobAborted);
		errno = save;
		TRACE (errno)
		EXITP
		return	-1;
	}
	EXITP
	return	jobId;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
int
ReceiveJob (cip, dataList_pp, srcFileList_pp)

connectionInfo	*cip;
list		**dataList_pp;
list		**srcFileList_pp;
{
	/*----------------------------------------------------------*/
	/*
	*/
		int		jobId,
				msgCount;
		char		*fileName_p;
		void		*data_p,
				*networkMsg_p;
		boolean		endOfJob;
		networkMsgTag	*networkMsgTag_p;

	static	char		FnName []	= "ReceiveJob";


	ENTRYP
	/*----------------------------------------------------------*/
	/*
	**	Check our args.
	*/
	if (cip == NULL)
	{
		errno = EINVAL;
		TRACE (errno)
		EXITP
		return	-1;
	}
	if (!Connected (cip))
	{
		errno = ENOLINK;
		TRACE (errno)
		EXITP
		return	-1;
	}
	if (dataList_pp == NULL && srcFileList_pp == NULL)
	{
		errno = EINVAL;
		TRACE (errno)
		EXITP
		return	-1;
	}
	/*----------------------------------------------------------*/
	/*
	*/
	msgCount = 0;
	endOfJob = False;
do {
	networkMsgTag_p = ReceiveNetworkMsg (cip, &networkMsg_p);

	if (networkMsgTag_p == NULL)
	{
		TRACE (errno)
		goto	errorReturn_1;
	}
	msgCount++;
	if (networkMsgTag_p->msgType == JobControlMsg)
	{
		switch (networkMsgTag_p->jobControlp->controlCode)
		{
		case	RequestToSendJob:
			TRACEP ("Received 'RequestToSendJob'.");
			if (msgCount != 1)
			{
				TrapError (NonFatal, Internal, FnName,
				"Network messages out of sync.");
				goto	errorReturn_1;
			}
			TRACEP ("Sending 'ClearToSendJob'.")
			jobId = networkMsgTag_p->jobControlp->jobId;
			if (! SendJobControlMsg (cip, ClearToSendJob))
				goto	errorReturn_1;
			break;
	
		case	JobAborted:
			TRACEP ("Received 'JobAborted'.");
			goto	errorReturn_1;
	
		default:
			TRACE (networkMsgTag_p->jobControlp->controlCode)
			TrapError (NonFatal, Internal, FnName,
				"Unexpected network message.");
			errno = EPROTO;
			TRACE (errno)
			EXITP
			return	-1;
		}
		continue;
	}
	endOfJob = networkMsgTag_p->jobControlp->endOfJob;

	switch (networkMsgTag_p->msgType)
	{
	case 	DataPacketMsg:
		if (*dataList_pp == NULL)
		{
			*dataList_pp = NewList (PointerList, 0);
			if (*dataList_pp == NULL)
			{
				TrapError (Fatal, Unix, FnName, "NewList");
			}
		}
		data_p = memdup (
			((dataPacketMsg *) networkMsg_p)->data.data_val,
			((dataPacketMsg *) networkMsg_p)->data.data_len);

		if (data_p == NULL)
		{
			TrapError (Fatal, Unix, FnName, "memdup");
		}
		if (! AppendToList (*dataList_pp, data_p,
		      ((dataPacketMsg *) networkMsg_p)->data.data_len))
		{
			TrapError (Fatal, Unix, FnName, "AppendToList");
		}
		FreeNetworkMsg (DataPacketMsg, &networkMsg_p);
		break;

	case 	FileFragmentMsg:
		if (*srcFileList_pp == NULL)
		{
			*srcFileList_pp = NewList (StringList, 0);
			if (*srcFileList_pp == NULL)
			{
				TrapError (Fatal, Unix, FnName, "NewList");
			}
		}
		/*
		**	NOTE:  The networkMsg_p is unusable
		**	when after a return from ReceiveFile.
		**	Therefore, do not free.
		*/
		fileName_p =
		ReceiveFile (cip, (fileFragmentMsg *) networkMsg_p);

		networkMsg_p = NULL;

		if (fileName_p == NULL)
		{
			goto	errorReturn_1;
		}
		if (! AppendToList (*srcFileList_pp, fileName_p, 0))
		{
			TrapError (Fatal, Unix, FnName, "AppendToList");
		}
		break;


	default:
		TRACE (networkMsgTag_p->msgType)
		TrapError (NonFatal, Internal, FnName,
			"Unknown message from remote.");
		free (networkMsg_p);
		goto	errorReturn_1;
	}
} while (! endOfJob);


	JobPendingFlag = False;
	EXITP
	return	jobId;


	/*----------------------------------------------------------*/
	/*
	*/
errorReturn_1:
	FreeList (dataList_pp);

	if (*srcFileList_pp != NULL) {
		(void)
		ApplyToList (*srcFileList_pp, (void *(*) ()) unlink,
			     EmptyList, 0);
		FreeList (srcFileList_pp);
	}
	JobPendingFlag = False;
	errno = ECOMM;
	TRACE (errno)
	EXITP
	return	-1;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
boolean
SendFile (cip, lastFileOfJob, srcPath_p, destPath_p)

connectionInfo	*cip;
boolean		lastFileOfJob;
char		*srcPath_p;
char		*destPath_p;
{
	/*----------------------------------------------------------*/
	/*
	*/
		int		fd,
				nBytes,
				endOfJob = False;
		struct stat	statInfo;
		fileFragmentMsg	fragmentMsg;

	static	int	FragmentBufferSize	= 32768;
	static	char	*FragmentBuffer_p	= NULL;
	static	char	FnName []		= "SendFile";


	ENTRYP
	TRACE  (cip)
	TRACEs (srcPath_p);
	TRACEs (destPath_p);
	/*---------------------------------------------------------*/
	/*
	*/
	if (cip == NULL || srcPath_p == NULL) {
		errno = EINVAL;
		TRACE (errno);
		EXITP
		return	False;
	}


	/*----------------------------------------------------------*/
	/*
	*/
	if (FragmentBuffer_p == NULL)
	{
		FragmentBuffer_p = malloc (FragmentBufferSize);

		if (FragmentBuffer_p == NULL)
		{
			TrapError (Fatal, Unix, FnName, "malloc");
		}
	}


	/*----------------------------------------------------------*/
	/*
	*/
	if (stat (srcPath_p, &statInfo) == -1)
	{
		TRACE (errno)
		EXITP
		return	False;
	}
	TRACE (statInfo.st_size)

	if ((fd = open (srcPath_p, O_RDONLY)) == -1)
	{
		TRACE (errno)
		EXITP
		return	False;
	}


	/*----------------------------------------------------------*/
	/*
	*/
	fragmentMsg.sizeOfFile		  = statInfo.st_size;
	fragmentMsg.fragment.fragment_val = FragmentBuffer_p;
	fragmentMsg.destPathp		  =
		destPath_p == NULL ? srcPath_p : destPath_p;

	nBytes = 0;
	do {
		fragmentMsg.fragment.fragment_len =
			read (fd, FragmentBuffer_p, FragmentBufferSize);

		if (fragmentMsg.fragment.fragment_len == -1)
		{
			int	save = errno;
			(void) close (fd);
			errno = save;
			TRACE (errno)
			EXITP
			return	False;
		}
		nBytes += fragmentMsg.fragment.fragment_len;

		if (nBytes < statInfo.st_size)
		{
			fragmentMsg.endOfFile = False;
		}
		else
		{
			fragmentMsg.endOfFile = True;
			if (lastFileOfJob)
			{
				endOfJob = True;
			}
		}
		if (! SendFileFragmentMsg (cip, endOfJob, &fragmentMsg))
		{
			int	save = errno;
			(void)	close (fd);
			errno = save;
			TRACE (errno)
			EXITP
			return	False;
		}
	} while (! fragmentMsg.endOfFile);
	/*----------------------------------------------------------*/


	(void)	close (fd);
	EXITP
	return	True;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
#define	END_OF_FILE		fileFragmentMsg_p->endOfFile
#define	DEST_PATH		fileFragmentMsg_p->destPathp
#define	SIZE_OF_FILE		fileFragmentMsg_p->sizeOfFile
#define	FRAGMENT_LEN		fileFragmentMsg_p->fragment.fragment_len
#define	FRAGMENT_VAL		fileFragmentMsg_p->fragment.fragment_val


char *
ReceiveFile (cip, fileFragmentMsg_p)

connectionInfo	*cip;
fileFragmentMsg	*fileFragmentMsg_p;
{
	/*----------------------------------------------------------*/
	/*
	*/
		int		fd,
				nBytes;
		char		*destPath_p,
				*basename_p,
				*dirname_p;
		boolean		done;
		networkMsgTag	*networkMsgTag_p;

	static	char	FnName []	= "ReceiveFile";


	/*----------------------------------------------------------*/
	/*
	**	We should probably check the file system to make
	**	sure we have enough space to hold the entire file.
	**
	*/

	/*----------------------------------------------------------*/
	/*
	*/
	TRACEs (DEST_PATH)
	TRACE  (SIZE_OF_FILE)

	if ((destPath_p = strdup (DEST_PATH)) == NULL)
	{
		TrapError (Fatal, Unix, FnName, "strdup");
	}
	basename_p = basename (DEST_PATH);
	TRACEs (basename_p)

	if (basename_p == DEST_PATH)
		goto	_open;

	dirname_p = dirname (DEST_PATH);
	TRACEs (dirname_p)

	if (mkdirp (dirname_p, 0755) == -1 && errno != EEXIST)
	{
		TrapError (NonFatal, Unix, FnName, "mkdirp");
		goto	errorReturn_1;
	}

_open:
	TRACEP ("_open")
	TRACEs (destPath_p)
	if ((fd = open (destPath_p, O_WRONLY|O_CREAT|O_EXCL, 0600)) == -1)
	{
		TrapError (NonFatal, Unix, FnName, "open");
		goto	errorReturn_1;
	}


	/*----------------------------------------------------------*/
	/*
	*/
	done = False;

	do {
		nBytes = write (fd, FRAGMENT_VAL, FRAGMENT_LEN);

		if (nBytes == -1)
		{
			TrapError (NonFatal, Unix, FnName, "write");
			goto	errorReturn_2;
		}
		if (nBytes != FRAGMENT_LEN)
		{
			TrapError (NonFatal, Unix, FnName, "write");
			goto	errorReturn_2;
		}
		done = END_OF_FILE;

		FreeNetworkMsg (FileFragmentMsg, (void **)&fileFragmentMsg_p);

		if (done)
			continue;

		networkMsgTag_p =
			ReceiveNetworkMsg (cip, (void **)&fileFragmentMsg_p);

		if (networkMsgTag_p == NULL ||
		    networkMsgTag_p->msgType != FileFragmentMsg)
		{
			goto	errorReturn_2;
		}
	} while (! done);


	/*----------------------------------------------------------*/
	/*
	**	Normal cleanup and return;
	*/
	(void)	close (fd);
	return	destPath_p;


	/*----------------------------------------------------------*/
	/*
	**	Error clean-up and return.
	*/
errorReturn_2:
	(void)	close (fd);
	(void)	unlink (destPath_p);

errorReturn_1:
	free (destPath_p);
	FreeNetworkMsg (FileFragmentMsg, (void **)&fileFragmentMsg_p);
	return	NULL;
}

#undef	FRAGMENT_VAL
#undef	FRAGMENT_LEN
#undef	PATH_OF_FILE
#undef	END_OF_FILE
/*==================================================================*/

/*==================================================================*/
/*
*/
boolean
SendData (cip, endOfJob, data_p, sizeOfData)

connectionInfo	*cip;
boolean		endOfJob;
void		*data_p;
int		sizeOfData;
{
	/*----------------------------------------------------------*/
	/*
	*/
		dataPacketMsg	msg;
	static	char		FnName []	= "SendData";


	ENTRYP
	TRACEs (data_p)
	TRACE  (sizeOfData)
	/*----------------------------------------------------------*/
	/*
	*/
	msg.endOfPacket   = (int) True;
	msg.data.data_len = sizeOfData;
	msg.data.data_val = (char *) data_p;


	/*---------------------------------------------------------*/
	/*
	*/
	JobControl.controlCode = NormalJobMsg;
	JobControl.endOfJob    = endOfJob;

	if (! EncodeNetworkMsgTag (cip, DataPacketMsg))
	{
		return	False;
	}
	if (! PutIntoXdrStream (cip, xdr_dataPacketMsg, &msg)) 
	{
		return	False;
	}
	if (! _SendNetworkMsg (cip))
	{
		return	False;
	}
	return	True;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
void
SetJobPriority (priority)

int	priority;
{
	JobControl.priority = priority;
	return;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
boolean
EncodeNetworkMsgTag (cip, msgType)

connectionInfo	*cip;
networkMsgType	msgType;
{
	/*----------------------------------------------------------*/
	/*
	*/
	static	int		MsgId		= 0;
	static	char		FnName []	= "EncodeNetworkMsgTag";
	static	networkMsgTag	NetworkMsgTag;


	/*---------------------------------------------------------*/
	/*
	*/
	NetworkMsgTag.versionMajor = MSGS_VERSION_MAJOR;
	NetworkMsgTag.versionMinor = MSGS_VERSION_MINOR;

	NetworkMsgTag.routeControl.sysId = 0;
	NetworkMsgTag.routeControl.msgId = ++MsgId;
	NetworkMsgTag.msgType            = msgType;

	if (msgType != SystemIdMsg)
	{
		NetworkMsgTag.jobControlp = &JobControl;
	}
	else
		NetworkMsgTag.jobControlp = NULL;
		
	/*---------------------------------------------------------*/
	/*
	*/
	ResetXdrStream (cip, XDR_ENCODE);

	if (! PutIntoXdrStream (cip, xdr_networkMsgTag, &NetworkMsgTag))
	{
		return	False;
	}
	return	True;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
boolean
JobPending (cip)

connectionInfo	*cip;
{
	if (cip == NULL)
	{
		errno = EINVAL;
		return	False;
	}
	if (cip->fd == -1)
	{
		errno = ENOLINK;
		return	False;
	}
	return	JobPendingFlag;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
int
NegotiateJobClearance (cip)

connectionInfo	*cip;
{
	void		*netMsgp;
	networkMsgTag	*netMsgTagp;

	static	int	JobId	  = 0;
	static	char	FnName [] = "NegotiateJobClearance";

	ENTRYP

	JobControl.endOfJob = False;

	TRACEP ("Sending 'RequestToSendJob'.")
	if (! SendJobControlMsg (cip, RequestToSendJob))
	{
		EXITP
		return	-1;
	}
receiveNetworkMsg:
	netMsgTagp = ReceiveNetworkMsg (cip, &netMsgp);
	if (netMsgTagp == NULL)
	{
		EXITP
		return	-1;
	}
	if (netMsgTagp->msgType != JobControlMsg)
	{
		EXITP
		return	-1;
	}
	switch (netMsgTagp->jobControlp->controlCode)
	{
	case	ClearToSendJob:
		TRACEP ("Received 'ClearToSendJob'.")
		break;

	case	RequestDenied:
		TRACEP ("Received 'RequestDenied'.");
		errno = EBUSY;
		EXITP
		return	-1;

	case	RequestToSendJob:
		JobPendingFlag = True;
		TRACEP ("Received 'RequestToSendJob'.")
		if (netMsgTagp->jobControlp->priority > JobControl.priority)
		{
			TRACEP ("Sending 'ClearToSendJob'.")
			JobControl.controlCode = ClearToSendJob;
			(void)	EncodeNetworkMsgTag (cip, JobControlMsg);
			(void)	_SendNetworkMsg (cip);
			goto	receiveNetworkMsg;
		}
		else
		{
			TRACEP ("Sending 'RequestDenied'.")
			JobControl.controlCode = RequestDenied;
			(void)	EncodeNetworkMsgTag (cip, JobControlMsg);
			(void)	_SendNetworkMsg (cip);
			goto	receiveNetworkMsg;
		}
		break;

	default:
		TRACE (netMsgTagp->jobControlp->controlCode)
		TrapError (NonFatal, Internal, FnName,
		"Network messages out of sync.");
		EXITP
		return	-1;
	}
	EXITP
	return	++JobId;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
boolean
SendJobControlMsg (cip, controlCode)

connectionInfo	*cip;
jobControlCode	controlCode;
{
	static	char	FnName [] = "SendJobControlMsg";

	ENTRYP

	JobControl.controlCode = controlCode;

	if (! EncodeNetworkMsgTag (cip, JobControlMsg))
	{
		return	False;
	}
	if (! _SendNetworkMsg (cip))
	{
		return	False;
	}
	return	True;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
boolean
SendSystemIdMsg (cip, data_p, sizeOfData)

connectionInfo	*cip;
void		*data_p;
int		sizeOfData;
{
	/*----------------------------------------------------------*/
	/*
	*/
		systemIdMsg	msg;
		struct utsname	utsName;

	static	char	FnName[]	= "SendSystemIdMsg";


	/*---------------------------------------------------------*/
	/*
	*/
	(void)	uname (&utsName);
	msg.systemNamep = utsName.nodename;
	msg.data.data_val = (char *) data_p;
	msg.data.data_len = sizeOfData;

	if (! EncodeNetworkMsgTag (cip, SystemIdMsg))
	{
		return	False;
	}
	if (! PutIntoXdrStream (cip, xdr_systemIdMsg, &msg)) 
	{
		return	False;
	}
	if (! _SendNetworkMsg (cip))
	{
		return	False;
	}
	return	True;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
boolean
SendFileFragmentMsg (cip, endOfJob, fileFragmentMsg_p)

connectionInfo	*cip;
boolean		endOfJob;
fileFragmentMsg	*fileFragmentMsg_p;
{
	/*----------------------------------------------------------*/
	/*
	*/
	static	char	FnName []	= "SendFileFragmentMsg";


	/*---------------------------------------------------------*/
	/*
	*/
	JobControl.controlCode = NormalJobMsg;
	JobControl.endOfJob    = endOfJob;

	if (! EncodeNetworkMsgTag (cip, FileFragmentMsg))
	{
		return	False;
	}
	if (! PutIntoXdrStream (cip, xdr_fileFragmentMsg, fileFragmentMsg_p)) 
	{
		return	False;
	}
	if (! _SendNetworkMsg (cip))
	{
		return	False;
	}
	return	True;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
networkMsgTag *
ReceiveNetworkMsg (cip, networkMsg_pp)

connectionInfo	*cip;
void		**networkMsg_pp;
{
	/*---------------------------------------------------------*/
	/*
	*/
		networkMsgTag	networkMsgTag_p;
	static	char		FnName []	= "ReceiveNetworkMsg";


	/*---------------------------------------------------------*/
	/*
	*/
	if (! _ReceiveNetworkMsg (cip))
	{
		return	NULL;
	}


	return	DecodeNetworkMsg (cip, networkMsg_pp);
}
/*==================================================================*/

/*==================================================================*/
/*
**
*/
networkMsgTag *
DecodeNetworkMsg (cip, networkMsg_pp)

connectionInfo	*cip;
void		**networkMsg_pp;
{
	/*---------------------------------------------------------*/
	/*
	*/
#define	ALLOCATE_NETWORK_MSG(msgType)	\
	*networkMsg_pp = (void *)  calloc (1, sizeof (msgType));	\
	if (*networkMsg_pp == NULL)					\
		TrapError (Fatal, Unix, FnName, "calloc")

#define	DECODE_NETWORK_MSG(xdrFnName)	\
	if (! GetFromXdrStream (cip, xdrFnName, *networkMsg_pp))\
		return	NULL
		

	/*---------------------------------------------------------*/
	/*
	*/
	static	char		FnName []	= "DecodeNetworkMsg";
	static	networkMsgTag	NetworkMsgTag;


	/*---------------------------------------------------------*/
	/*
	*/
	if (NetworkMsgTag.jobControlp)
	{
		free (NetworkMsgTag.jobControlp);
		NetworkMsgTag.jobControlp = NULL;
	}
	ResetXdrStream (cip, XDR_DECODE);
	if (! GetFromXdrStream (cip, xdr_networkMsgTag, &NetworkMsgTag))
	{
		return	NULL;
	}
	TRACE (NetworkMsgTag.msgType)
	switch (NetworkMsgTag.msgType)
	{
	case	JobControlMsg:
		*networkMsg_pp = NULL;
		break;

	case	SystemIdMsg:
		ALLOCATE_NETWORK_MSG (systemIdMsg);
		DECODE_NETWORK_MSG (xdr_systemIdMsg);
		break;

	case	PacketBundleMsg:
		ALLOCATE_NETWORK_MSG (packetBundleMsg);
		DECODE_NETWORK_MSG (xdr_packetBundleMsg);
		break;

	case	DataPacketMsg:
		ALLOCATE_NETWORK_MSG (dataPacketMsg);
		DECODE_NETWORK_MSG (xdr_dataPacketMsg);
		break;

	case	FileFragmentMsg:
		ALLOCATE_NETWORK_MSG (fileFragmentMsg);
		DECODE_NETWORK_MSG (xdr_fileFragmentMsg);
		break;

	default:
		TrapError (NonFatal, Internal, FnName,
			"Unknown network message.  Could not decode.");
		return	NULL;
	}


	return	&NetworkMsgTag;
}

#undef	DECODE_NETWORK_MSG
#undef	ALLOCATE_NETWORK_MSG
/*==================================================================*/

/*==================================================================*/
/*
*/
void
FreeNetworkMsg (msgType, networkMsg_pp)

networkMsgType	msgType;
void		**networkMsg_pp;
{
	/*---------------------------------------------------------*/
	/*
	*/
	static	char		FnName[]	= "FreeNetworkMsg";


	/*---------------------------------------------------------*/
	/*
	*/
	if (networkMsg_pp == NULL || *networkMsg_pp == NULL)
		return;

	free (*networkMsg_pp);

	*networkMsg_pp = NULL;

	return;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
dataPacket *
NewDataPacket (size)

int	size;
{
	/*----------------------------------------------------------*/
	/*
	*/
	register	dataPacket	*dataPacket_p;
	static		char		FnName []	= "NewDataPacket";


	/*----------------------------------------------------------*/
	/*
	*/
	dataPacket_p = (dataPacket *)  calloc (1, sizeof (dataPacket));

	if (dataPacket_p == NULL)
		TrapError (Fatal, Unix, FnName, "calloc");

	if (size <= 0)
		return	dataPacket_p;

	dataPacket_p->data_p = (void *) calloc (size, sizeof (char));

	if (dataPacket_p->data_p == NULL)
		TrapError (Fatal, Unix, FnName, "calloc");

	dataPacket_p->size = size;

	return	dataPacket_p;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
void
FreeDataPacket (dataPacket_pp)

register
dataPacket	**dataPacket_pp;
{
	/*----------------------------------------------------------*/
	/*
	*/
	if (dataPacket_pp == NULL || *dataPacket_pp == NULL)
		return;

	if ((*dataPacket_pp)->data_p != NULL) {
		free ((*dataPacket_pp)->data_p);
		(*dataPacket_pp)->data_p = NULL;
	}
	free (*dataPacket_pp);

	*dataPacket_pp = NULL;

	return;
}
/*==================================================================*/
