/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988, 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:cmd/bootutils/bootserver.d/bootserver.c	1.3.1.2"

static char bootserver_copyright[] = "Copyright 1988, 1989 Intel Corp. 462650";

/*
#define DEBUG
 */
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <signal.h>
#include <errno.h>
#include <sys/stropts.h>			/* *************** */
#include <sys/mb2taiusr.h>	 		/* *************** */
#include "bootserver.h"


static struct HostInfo	*hosthead = NULL; 	/* Head of link list */
static struct HostInfo	*workhost; 		/* Working host */

static char	cbuffer[MAX_UNSOLMSG];		/* Buffer for control message */
static mb2_buf	ctlbuffer =
	{ MAX_UNSOLMSG, 0, cbuffer};	/* buffer for TAI control part */
static mb2_buf	databuffer =
	{ 0,0, NULL};			/* buffer for TAI data part */
static char	datachange = NONE;		/* switch for allocating data msg? */

static ushort		portid = BOOT_PORT_ID; 
static int		portfd;			/* file descriptor of bootserver port */
static ushort		clientid;		/* client's host id */
static unchar		state;			/* state of client */
static mb2_msginfo	msginfo;		/* message info for TAI lib */

static char	ConfigFile[MAX_LENGTH] = "";
static char	LogFileName[MAX_LENGTH];		/* Default is standard output */
int	LogLevel = L_ERROR|L_STARTUP|L_LOCATE|L_CONNECT|L_OPEN;

static int	DoLocate(), DoDiscAndLocate(), DoConnect(), DoDisconnect();
static int	DoOpen(), DoSeek(), DoRead(), DoClose(), DoCloseAndOpen();
static int	DoGetBPS(), DoError();
static void 	DoDump();

/*
 *	Table for routines that handle the in-coming message.
 */
static int	(*Routine[NUM_COMMAND][NUM_STATE])() =
	{{DoLocate,	DoDiscAndLocate,	DoDiscAndLocate},
	 {DoLocate,	DoDiscAndLocate,	DoDiscAndLocate},
	 {DoConnect,	DoError,		DoError},
	 {DoError,	DoOpen,			DoError},
	 {DoGetBPS,	DoGetBPS,		DoGetBPS},
	 {DoError,	DoOpen,			DoCloseAndOpen},
	 {DoError,	DoError,		DoRead},
	 {DoError,	DoError,		DoSeek},
	 {DoError,	DoError,		DoClose},
	 {DoError,	DoDisconnect,		DoError}};
#ifdef DEBUG
#define	OPTIONS	"c:l:x:D:p:" 
char		Debug;
#define DDEBUG(x) if (Debug) DoLog x 
#else	 		/* DEBUG */
#define	OPTIONS	"c:l:x:p:"
#define DDEBUG(x) 
#endif			/* DEBUG */
static char		errflag = 0;		/* error flag for options */

extern int LogInit();
extern void DoLog();
extern int ConfigInit();
extern char *GetBPString();
extern char *GetBootLoader();
extern int GetHostAndMethod();
extern	struct passwd *getpwnam();
extern	struct group *getgrnam();

extern unsigned short mb2_gethostid ();
extern int mb2s_closeport ();
extern int mb2s_getinfo ();
extern int mb2s_getreqfrag ();
extern int mb2s_openport ();
extern int mb2s_receive ();
extern int mb2s_send ();
extern int mb2s_sendcancel ();
extern int mb2s_sendreply ();

extern	void *malloc();
extern int	close();	
extern void	free();	
extern char	*optarg;		
extern int	optind, opterr;	
extern long	lseek();	
extern time_t	time();	
extern void	exit();	

void
main(argc, argv)
int argc;
char **argv;
{
	int response;
	ushort command;
	static ushort VerifyCmd();
	static unchar SearchClientState();
	static void Options();
	static int Mb2BigSendReply();
	static void CreateChild();
	static void InitializeEnv();
	static void HandleReceiveError();
	static void HandleFragment();

	Options(argc,argv);
#ifdef DEBUG
	if (!Debug)
#endif /* DEBUG */
		CreateChild();
	if ((portfd = mb2s_openport(portid,(mb2_opts *)NULL)) == -1) { 
		if (errno == EBUSY) {
			DoLog (L_BAD,"Could not open port, check for existing bootserver process.");
			exit(X_PRESENT); 
		}
		DoLog (L_ERROR,"EXIT: Internal Error on open port. errno=0x%x",errno);
		exit(X_ERR_OPN);
	}
	InitializeEnv();
	DoLog (L_STARTUP,"Initialize Bootserver using,");
	DoLog (L_STARTUP,"Portid = 0x%x,",portid);
	DoLog (L_STARTUP,"Configuration file = %s,", ConfigFile);
	if (strlen(LogFileName)) {
		DoLog(L_STARTUP,"Log File = %s.",LogFileName);
	} else {
		DoLog(L_STARTUP,"No Log File.");
	}
	/* CONSTANTCONDITION */
	while (TRUE) {
		DDEBUG((L_DEBUG,"Ready to receive request message.\n"));
		msginfo.tid=(unchar)0;
		
		if ((mb2s_receive(portfd,&msginfo,&ctlbuffer,&databuffer)) == -1) {
			HandleReceiveError(errno);
		}
		if (msginfo.msgtyp == MB2_REQFRAG_MSG) {
			HandleFragment();
		}
		DoLog (L_DEBUG,"Message is");
		DoDump(L_DEBUG,ctlbuffer.buf,ctlbuffer.len);
		if ( databuffer.len ) {
			DoLog (L_DEBUG,"with Data portion");
			DoDump(L_DEBUG,databuffer.buf,databuffer.len);
		}
		if ((command = VerifyCmd(&ctlbuffer)) == 0) {
			DoLog(L_ERROR,"IGNORE: Unknown protocol message is received.");
			DoDump(L_ERROR,ctlbuffer.buf,ctlbuffer.len);
		} else {
			clientid = msginfo.socketid.hostid;
			state = SearchClientState(clientid);
			DDEBUG((L_DEBUG,"Call subroutine of Command=0x%x,state=0x%x\n",command,state));
			response = (*Routine[command-1][state])();
			DDEBUG((L_DEBUG,"Return from routine, response=0x%x\n",response));
			switch (response) {
			 case NO_REPLY:
				break;
			 case REPLY:
				DoLog (L_DEBUG,"Reply Message is");
				DoDump(L_DEBUG,ctlbuffer.buf,ctlbuffer.len);
				if (databuffer.len) {
					DoLog (L_DEBUG,"with data,size %d bytes.",databuffer.len);
				}
				(void)Mb2BigSendReply(portfd,&msginfo.socketid,
				     &ctlbuffer,&databuffer,msginfo.tid);
				break;
			 case REPLY_LOCATE:
				DoLog (L_DEBUG,"Reply Message is");
				DoDump(L_DEBUG,ctlbuffer.buf,ctlbuffer.len);
				if ((mb2s_send(portfd,&msginfo.socketid,
				     &ctlbuffer,&databuffer)) == -1) {
					DoLog(L_ERROR,"EXIT: Cannot send reply to hostid=0x%x, errno=0x%x.",clientid,errno);
					exit(X_ERR_SND);
				}
				break;
			}
		}
		DDEBUG((L_DEBUG,"datachange=0x%x,ptr=0x%x,length=0x%x\n",
			datachange,databuffer.buf,databuffer.maxlen));
		switch (datachange) {
			default:
				break;
			case ALLOC:
				free(databuffer.buf);
				databuffer.maxlen = 0;
				databuffer.len = 0;
				datachange = NONE;
				break;
			case CHANGE:
				datachange = NONE;
				databuffer.maxlen = 0;
				databuffer.len = 0;
				databuffer.buf = (char *)NULL;
				break;
		}
	}
}

/*
 *	Routine: Options
 *	input:	 argc, argv
 *	return:	 none
 *	parse command line options.
 */
static void
Options(argc,argv)
int argc;
char **argv;
{
	extern int getopt();
	extern int setgid();
	extern int setuid();
	struct passwd *pwdptr;
	struct group *grpptr;
	int	ch;   				
	DDEBUG(("L_DEBUG,Execute Options.\n"));
	while ((ch = getopt(argc, argv,OPTIONS)) != -1) {
		switch (ch)	{
		case 'c':
			(void)strcpy(ConfigFile,optarg);
			break;
		case 'l':
			(void)strcpy(LogFileName,optarg);
			break;
		case 'x':
			if (sscanf(optarg,"%d",&LogLevel) != 1) {
				DoLog(L_BAD,"Invalid log level, value=%s",optarg);
				errflag++;
			}
			switch (LogLevel) {
			case 0:
				LogLevel= L_ERROR|L_STARTUP;
				break;
			case 1:
				LogLevel= L_ERROR|L_STARTUP|L_LOCATE|L_CONNECT|L_OPEN;
				break;
			case 2:
				LogLevel= L_ERROR|L_STARTUP|L_LOCATE|L_CONNECT|L_OPEN|L_SEEK|L_READ|L_GETBPS;
				break;
			case 9:
				LogLevel= L_ERROR|L_STARTUP|L_LOCATE|L_CONNECT|L_OPEN|L_SEEK|L_READ|L_GETBPS|L_DEBUG;
				break;
			default:
				DoLog(L_BAD,"Invalid log level, value=%s",optarg);
				errflag++;
				break;
			}
			break;
#ifdef DEBUG
		case 'D':
			if (strcmp(optarg,"ebug") == 0) {
				Debug++;
				break;
			}
			/* Fall through */
#endif /* DEBUG */
		case '?':
			errflag++;
			break;
		}			/* end switch */
	}
	if (errflag || (optind < argc))  {
		(void)printf(M_USAGE);
		exit(X_INVALID);
	}
	if ((pwdptr=getpwnam(USERNAME)) == NULL) {
		DoLog(L_ERROR,"EXIT: No entry for %s in /etc/passwd file.",USERNAME);
		exit(X_SET_ID);
	}
	if ((grpptr=getgrnam(GROUPNAME)) == NULL) {
		DoLog(L_ERROR,"EXIT: No entry for %s in /etc/group file.",GROUPNAME);
		exit(X_SET_ID);
	}
	if ((setgid(grpptr->gr_gid) < 0)||(setuid(pwdptr->pw_uid) < 0)) {
		DoLog(L_ERROR,"EXIT: Can not set userid and groupid to Bootserver.");
		exit(X_SET_ID);
	}

	/*
	 * If config file is not specified on the command line, ask BPS driver
	 * to pass one. 
	 */
	if (strlen (ConfigFile) == 0) {
		int		BpsDev				;
		char	FileName[MAX_LENGTH];
		int	rc						;

		if ( bpsopen (O_RDONLY) == -1) {
			DoLog(L_GETBPS,"Cannot open BPS driver")	;
			(void)strcpy(ConfigFile,"/stand/config");
		}
		else { /* Ask BPS driver */
			rc = bps_get_val ("BL_CONFIG_FILE", MAX_LENGTH, FileName) ;
			if ((rc == ENODATA) || (rc == -1) ) {
				(void)strcpy(ConfigFile,"/stand/config");
			}
			else {				
				(void)strcpy(ConfigFile, FileName)	;
			}

			if (bpsclose () == -1) {
				DoLog(L_GETBPS, "Cannot close BPS driver");
			}
		}
				
	}

	if (ConfigInit(ConfigFile) == -1)  {
		DoLog(L_BAD,"Cannot open Configuration file, %s",ConfigFile);
		exit(X_ERR_CONFIG);
	}
#ifdef DEBUG
	if (Debug)
		LogLevel=ALL_LEVEL;
#endif
		
}

/*
 *	Routine: CreateChild
 *	input:	 none
 *	return:	 none
 *	create child process and detach child from parent process
 */
static void
CreateChild()
{
	extern pid_t setpgrp();
	extern pid_t fork(); 
	int child;
	int count = 0;
	DDEBUG(("L_DEBUG,Execute CreateChild\n"));
	while (child = (int)fork()) {
		if (child == -1) {
			if (count++ >= 10) {
				DoLog(L_BAD,"Internal error, cannot fork child,errno=0x%x",errno);
				exit(X_INTERNAL); 
			}
		} else {
			DDEBUG((L_DEBUG,"Parent exits.\n"));
			exit(X_COMPLETE);
		}
	}
	(void)setpgrp();	/* detach process */
}

/*
 *	Routine: InitializeEnv
 *	input:	 none
 *	return:	 none
 *	Initialize environment for child process:
 *	- Signal
 *	- bit mask
 *	- File descriptors.
 */
static void
InitializeEnv()
{
	int fd;
	extern mode_t umask();
	static void caught_hup(), caught_term();

	DDEBUG((L_DEBUG,"Execute InitializeEnv.\n"));
	(void)umask(022);
	(void)signal(SIGHUP, caught_hup);
#ifdef DEBUG
	if (!Debug) {
#endif /* DEBUG */
	(void)signal(SIGINT, SIG_IGN);
	(void)signal(SIGQUIT, SIG_IGN);
#ifdef DEBUG
	} else {
	(void)signal(SIGINT, caught_hup);
	(void)signal(SIGQUIT, caught_hup);
	}
#endif /* DEBUG */
	(void)signal(SIGTERM, caught_term);

	for (fd=3; fd < _NFILE; fd++) {
		if (fd != portfd) {
			(void)close(fd); /* Close all file descriptors except 0, 1, 2 */
		}
	}
	(void)freopen ("/dev/null","r",stdin);
	(void)freopen ("/dev/null","w",stderr);
	if (LogInit(LogFileName) == -1) {
		DoLog(L_BAD,"WARNING: Can not initialize log file (%s).",LogFileName);
		(void)strcpy(LogFileName,"");
	}
}
static void
caught_hup()
{
	static void shut_down();

	DoLog (L_STARTUP,"Receive SIGHUP, Bootserver terminated.");
	shut_down();
	exit(X_COMPLETE);
}
static void
caught_term()
{
	static void shut_down();

	DoLog (L_STARTUP,"Receive SIGTERM, Bootserver terminated.");
	shut_down();
	exit(X_COMPLETE);
}
static void
shut_down()
{
	mb2_socket	socketid;
	unchar	tid;
	if ((tid=msginfo.tid) != 0) {
		socketid = msginfo.socketid;
		(void)mb2s_sendcancel(portfd,&socketid,tid);
	}
	(void)mb2s_closeport(portfd);
}

/*
 *	Routine: VerifyCmd
 *	input:	 pointer to control portion buffer of TAI 
 *	return:	 bootserver command 
 *	Verify the command in receive message.
 */
static ushort
VerifyCmd(ctlbufptr)
mb2_buf	*ctlbufptr;
{
	ushort	*cmdptr;

	DDEBUG((L_DEBUG,"Execute VerifyCmd.\n"));
	if (ctlbufptr->len) {
		cmdptr = (ushort *)ctlbufptr->buf;
		if ((*cmdptr>=FIRST_COMMAND) && (*cmdptr<=LAST_COMMAND))  {
			return(*cmdptr);
		}
	}
	return((ushort)0);
}

/*
 *	Routine: SearchClientState
 *	Global variables used: workhost
 *	input:	 hostid of client
 *	return:	 state with pointer to HostInfo structure of the client
 *	Find the state of the server-client connection.
 */
static unchar
SearchClientState(id)
ushort	id;
{
	DDEBUG((L_DEBUG,"Execute SearchClientState.\n"));
	workhost = hosthead;
	while (workhost) {
		if (workhost->HostID == id) {
			return(workhost->State);	/* with workhost ptr */
		}
		workhost = workhost->Next;
	}
	return(ST_DISC);	/* with workhost = NULL */
}

/*
 *	Routine: DoLocate
 *	Global variables used: ctlbuffer, databuffer, clientid
 *	input:	 
 *	return:	 Response flag and message to send back if response.
 *	Handle the Locate Bootserver and Locate ConfigServer protocol.
 *
 */
int
DoLocate()
{
	struct LocateServerReq	*msg;
	struct LocateServerResp	*rmsg;
	int GetHostAndMethod();
	static int SupportVersion();

	DDEBUG((L_DEBUG,"Execute DoLocate.\n"));
	msg = (struct LocateServerReq *) ctlbuffer.buf;
	rmsg = (struct LocateServerResp *) ctlbuffer.buf;
	switch (GetHostAndMethod(clientid)) {
		 case NO_HOSTID:
			DDEBUG((L_DEBUG,"No hostid in Config File.\n"));
			return(NO_REPLY);
		 case NO_METHOD:
			DDEBUG((L_DEBUG,"No BL_method specified in Config File.\n"));
			break;
		 case DEP_METHOD:
			if (msg->Command != LocateBootS) {
				DDEBUG((L_DEBUG,"Dependent boot specified in Config File.\n"));
				return(NO_REPLY);
			}
			break;
		 case QUA_METHOD:
			if (msg->Command != LocateConfigS) {
				DDEBUG((L_DEBUG,"Quasi boot specified in Config File.\n"));
				return(NO_REPLY);
			}
			break;
	}
	if (!SupportVersion(msg->ClientVersion)) {
		DDEBUG((L_DEBUG,"Version 0x%x is not supported.\n",
			msg->ClientVersion));
		return(NO_REPLY);
	}
	rmsg->Command |= RESPONSE_PROTOCOL;
	rmsg->ServerVersion = VERSION;
	/* Force status to zero rather than return data from client's
	   LOCATE request message. */
	rmsg->Status = 0;
	ctlbuffer.len = sizeof(struct LocateServerResp);
	databuffer.len = 0;
	DoLog(L_LOCATE,"Service LOCATE from host 0x%x ",clientid);
	return(REPLY_LOCATE);
}

/*
 *	Routine: DoConnect
 *	Global variables used: ctlbuffer, databuffer, clientid, workhost
 *	input:	 
 *	return:	 Response flag and message to send back if response.
 *	Handle the Connect Bootserver protocol.
 */
int
DoConnect()
{
	struct ConnectServerResp	*rmsg;
	static struct HostInfo *CreateHostInfo();

	DDEBUG((L_DEBUG,"Execute DoConnect.\n"));
	rmsg = (struct ConnectServerResp *)ctlbuffer.buf;
	if ((workhost=CreateHostInfo(clientid)) == 0) {
		DoLog(L_ERROR,"WARNING: Can not create Host Info for hostid 0x%x.",clientid);
		return (DoError());
	}
	rmsg->Command |= RESPONSE_PROTOCOL;
	rmsg->Status = E_OK;
	ctlbuffer.len = sizeof(struct ConnectServerResp);
	databuffer.len = 0;
	workhost->HostID = clientid;
	workhost->State = ST_CONN;
	workhost->ConnectTime = time((long *)0);
	DoLog(L_CONNECT,"Service CONNECT from host 0x%x state 0x%x",clientid,workhost->State);
	return(REPLY);
}

/*
 *	Routine: DoDisconnect
 *	Global variables used: clientid, workhost
 *	input:	 
 *	return:	 Response flag 
 *	Handle the Disconnect Bootserver protocol.
 */
int
DoDisconnect()
{
	static void FreeHostInfo();

	DDEBUG((L_DEBUG,"Execute DoDisconnect.\n"));
	if (workhost != NULL) {
		FreeHostInfo(workhost);	/* workhost from SearchClientState */
	}
	databuffer.maxlen = databuffer.len = 0;
	DoLog(L_CONNECT,"Service DISCONNECT from host 0x%x",clientid);
	return(NO_REPLY);
}

/*
 *	Routine: DoOpen
 *	Global variables used: ctlbuffer, databuffer, clientid, workhost
 *	input:	 
 *	return:	 Response flag and message to send back if response.
 *	Handle the Open Second Stage Bootserver protocol and
 *	the Open File Bootserver protocol.
 */
int
DoOpen()
{
	struct OpenFileReq	*msg;
	struct OpenFileResp	*rmsg;
	char *pathname = NULL;
	static ushort CheckFileExistance();
	char *GetBootLoader();

	DDEBUG((L_DEBUG,"Execute DoOpen.\n"));
	msg = (struct OpenFileReq  *)ctlbuffer.buf;
	rmsg = (struct OpenFileResp  *)ctlbuffer.buf;
	if (databuffer.len && !((databuffer.len==1)&&(databuffer.buf[0]== '\0'))) {
		/* If we follow SGI functional spec, this code will be used.
		 * But it is different from the implementation in firmware.
		struct	OpenFileData	*data;
		data = (struct OpenFileData *)databuffer.buf;
		if (data->Length) {
			pathname = data->PathName;
		}
		*/
		pathname = databuffer.buf;
	} else {
		if (msg->Command == OpenSecondStageC) {
			pathname = GetBootLoader(clientid);
		} else {
			DoLog(L_ERROR,"ERROR: No filename to open.");
			return(DoError());
		}
	}
	switch (rmsg->Status = CheckFileExistance(pathname)) {
	case E_FNEXIST:
		break;
	case E_ACCESS:
		break;
	case E_OK:
		rmsg->FileDesc = (ulong)workhost;
		workhost->State = ST_CON_OPN;
		(void)strcpy(workhost->OpenFile,pathname);
		workhost->FilePosition = 0;
		workhost->OpenTime = time((long *)0);
		break;
	}
	rmsg->Command |= RESPONSE_PROTOCOL;
	ctlbuffer.len = sizeof(struct OpenFileResp);
	databuffer.len = 0;
	DoLog(L_OPEN,"Service OPEN (%s) from host 0x%x, Status=0x%x",
		pathname,clientid,rmsg->Status);
	return(REPLY);
}

/*
 *	Routine: CheckFileExistance
 *	Global variables used: 
 *	input:	pathname of requested file 
 *	return:	Status of file 
 *	Check the status of the requested file.
 */
static ushort
CheckFileExistance(name)
char *name;
{
	extern int access();

	DDEBUG((L_DEBUG,"Execute CheckFileExistance.\n"));
	if (name == NULL) {
		return(E_FNEXIST);
	} 
	if (access(name,READ) == -1) {
		if (errno == EACCES)
			return(E_ACCESS);
		else
			return(E_FNEXIST);
	} 
	return(E_OK);
}

/*
 *	Routine: DoSeek
 *	Global variables used: ctlbuffer, databuffer, clientid, workhost
 *	input:	 
 *	return:	 Response flag and message to send back if response.
 *	Handle the Seek File Bootserver protocol.
 */
int
DoSeek()
{
	struct SeekFileReq	*msg;
	struct SeekFileResp	*rmsg;

	DDEBUG((L_DEBUG,"Execute DoSeek.\n"));
	msg = (struct SeekFileReq *)ctlbuffer.buf;
	rmsg = (struct SeekFileResp *)ctlbuffer.buf;
	if (msg->FileDesc == (ulong)workhost) {
		workhost->FilePosition = msg->Offset;
		rmsg->Status = E_OK;
	} else {
		rmsg->Status = E_IO;
	}
	rmsg->Command |= RESPONSE_PROTOCOL;
	ctlbuffer.len = sizeof(struct SeekFileResp);
	databuffer.len = 0;
	DoLog(L_SEEK,"Service SEEK from host 0x%x, Offset=0x%x, Status=0x%x",
		clientid,workhost->FilePosition, rmsg->Status);
	return(REPLY);
}

/*
 *	Routine: DoRead
 *	Global variables used: ctlbuffer, databuffer, clientid, workhost
 *	input:	 
 *	return:	 Response flag and message and data to send back if response.
 *	Handle the Read Bootserver protocol.
 */
int
DoRead()
{
	struct ReadFileReq	*msg;
	struct ReadFileResp	*rmsg;
	static ushort ReadFile();

	DDEBUG((L_DEBUG,"Execute DoRead.\n"));
	msg = (struct ReadFileReq *)ctlbuffer.buf;
	rmsg = (struct ReadFileResp *)ctlbuffer.buf;
	if (msg->FileDesc == (ulong)workhost) {
		DoLog(L_READ,"READ msg of hostid (0x%x), byte requested=%d",
			clientid,msg->Count);
		switch (rmsg->Status = ReadFile(msg->Count)) {
		case E_OK:
			workhost->ReadTime = time((long *)0);
			workhost->FilePosition += databuffer.len;
			break;
		case E_IO:
			databuffer.len = 0;
			break;
		}
	} else {
		rmsg->Status = E_IO;
		databuffer.len = 0;
	}
	rmsg->Command |= RESPONSE_PROTOCOL;
	rmsg->Actual = (ulong)databuffer.len;
	ctlbuffer.len = sizeof(struct ReadFileResp);
	DoLog(L_READ,"Service READ from host 0x%x, Status=0x%x, nbyte=%d",
		clientid,rmsg->Status,rmsg->Actual);
	return(REPLY);
}

/*
 *	Routine: ReadFile
 *	Global variables used: databuffer, clientid, workhost
 *	input:	 size of data 
 *	return:	Status of read file 
 *
 *	This routine is doing all activities depending on the
 *	information kept in HostInfo Structure. The activities are
 *	open file, seek file, read file, and close file including
 *	space allocation for data area.
 */
static ushort
ReadFile(size)
ulong	size;
{
	extern int read();
	char	*allocptr;
	int	fd;

	DDEBUG((L_DEBUG,"Execute ReadFile.\n"));
	if ((allocptr = (char *)malloc((unsigned)size)) == NULL) {
		return (E_IO);
	}
	databuffer.maxlen = size;
	databuffer.buf = allocptr;
	datachange = ALLOC;
	if ((fd=open(workhost->OpenFile,O_RDONLY)) == -1) {
		return(E_IO);
	}
	if (lseek(fd,workhost->FilePosition,0) == -1L) {
		(void)close(fd);
		return(E_IO);
	}
	if ((databuffer.len=read(fd,allocptr,(unsigned)size)) == -1) {
		databuffer.len=0;
		(void)close(fd);
		return(E_IO);
	}
	(void)close(fd);
	return(E_OK);
}

/*
 *	Routine: DoClose
 *	Global variables used: ctlbuffer, workhost
 *	input:	 
 *	return:	 Response flag 
 *	Handle the Close File Bootserver protocol.
 */
int
DoClose()
{
	struct CloseFileReq	*msg;
	static void CloseFile();

	DDEBUG((L_DEBUG,"Execute DoClose.\n"));
	msg = (struct CloseFileReq *)ctlbuffer.buf;
	if (msg->FileDesc == (ulong)workhost) {
		CloseFile();
	}
	databuffer.len = 0;
	DoLog(L_OPEN,"Service CLOSE from host 0x%x", clientid);
	return(NO_REPLY);
}

/*
 *	Routine: CloseFile
 *	Global variables used: workhost
 *	input:	none 
 *	return:	 none
 *	Clean up OpenFile and file position in HostInfo structure
 */
static void
CloseFile()
{
	DDEBUG((L_DEBUG,"Execute CloseFile.\n"));
	(void)strcpy(workhost->OpenFile,"");
	workhost->FilePosition = 0;
	workhost->State = ST_CONN;
}

/*
 *	Routine: DoGetBPS
 *	Global variables used: ctlbuffer, databuffer, clientid, workhost
 *	input:	 
 *	return:	 Response flag and message and data to send back if response.
 *	Handle the Get Host BPS Bootserver protocol.
 */
int
DoGetBPS()
{
	struct GetHostBPSReq	*msg;
	struct GetHostBPSResp	*rmsg;
	char *ptr;
	int    size;
	char *GetBPString();

	DDEBUG((L_DEBUG,"Execute DoGetBPS.\n"));
	msg = (struct GetHostBPSReq *)ctlbuffer.buf;
	rmsg = (struct GetHostBPSResp *)ctlbuffer.buf;
	if ((ptr=GetBPString(clientid)) == NULL) {
		rmsg->Status = E_OVERFLOW;
	} else {
		size = strlen(ptr)+1;
		if (size > (int)msg->BufSize) {
			rmsg->Status = E_OVERFLOW;
		} else {
			rmsg->Status = E_OK;
		}
	}
	switch (rmsg->Status) {
	case E_OK:
		datachange = CHANGE;
		databuffer.maxlen = databuffer.len=size;
		databuffer.buf = ptr;
		rmsg->BpSize = (ulong)size;
		break;
	case E_OVERFLOW:
		databuffer.len = 0;
		rmsg->BpSize = (ulong)0;
		break;
	}
	rmsg->Command |= RESPONSE_PROTOCOL;
	ctlbuffer.len = sizeof(struct GetHostBPSResp);
	DoLog(L_GETBPS,"Service GETHOSTBPS from host 0x%x, status=0x%x",
		clientid,rmsg->Status);
	return(REPLY);
}

/*
 *	Routine: DoDiscAndLocate
 *	input:	 
 *	return:	 Response flag and message to send back if response.
 *	Handle the Locate Bootserver protocol if the state of connection
 *	is connect or connect&open.
 */
int
DoDiscAndLocate()
{
	DDEBUG((L_DEBUG,"Execute DoDiscAndLocate.\n"));
	(void)DoDisconnect();
	return(DoLocate());
}

/*
 *	Routine: DoCloseAndOpen
 *	input:	 
 *	return:	 Response flag and message to send back if response.
 *	Handle the Open Bootserver protocol if the state of connection
 *	is connect&open. The bootserver will close the opened file and
 *	then open the new request file.
 */
int
DoCloseAndOpen()
{
	void CloseFile();

	DDEBUG((L_DEBUG,"Execute DoCloseAndOpen.\n"));
	CloseFile();
	return(DoOpen());
}

/*
 *	Routine: DoError
 *	Global variables used: ctlbuffer, databuffer, clientid, workhost
 *	input:	 
 *	return:	 Response flag and message to send back if response.
 *	Handle the out of order messages. 
 */
DoError()
{
	struct anycommand{
		ushort	cmd;
		ushort	status;
	} *ptr;
	DoLog(L_ERROR,"ERROR: Out of sync msg, state=%x, clientid=%x",state,clientid);
	DoDump(L_ERROR,ctlbuffer.buf,ctlbuffer.len);
	(void)DoDisconnect();
	ptr = (struct anycommand *)ctlbuffer.buf;
	if ((ptr->cmd == CloseFileC) || (ptr->cmd == DisconnectServerC)) {
		return(NO_REPLY);
	}
	DoLog(L_ERROR,"\t Reply Out of sync msg to clientid=%x, Command=%x",
		clientid,ptr->cmd);
	ptr->cmd |= RESPONSE_PROTOCOL;
	ptr->status = E_INCOMPLETE;
	databuffer.len = 0;
	return(REPLY);
}

/*
 *	Routine: CreateHostInfo
 *	Global variables used: hosthead
 *	input:	client id (hostid)
 *	return:	 pointer to HostInfo structure for the client 
 *	Create and initialize HostInfo structure for the client.
 */
static struct HostInfo *
CreateHostInfo(id)
ushort	id;
{
	struct HostInfo	*current, *new;

	DDEBUG((L_DEBUG,"Execute CreateHostInfo.\n"));
	if ((new = (struct HostInfo *)malloc(sizeof(struct HostInfo))) == NULL) {
		DDEBUG((L_DEBUG,"Exit CreateHostInfo.(not create)\n"));
		return (NULL);
	}
	if (hosthead == NULL) {
		hosthead = new;
	} else {
		current = hosthead;
		while (current->Next) {
			current = current->Next;
		}
		current->Next = new;
	}
	new->Next = NULL;
	new->HostID = id;
	new->State = ST_DISC;
	new->OpenFile[0] = '\0';
	new->FilePosition = 0;
	new->ConnectTime = (time_t) 0;
	new->OpenTime = (time_t) 0;
	new->ReadTime = (time_t) 0;
	DDEBUG((L_DEBUG,"Exit CreateHostInfo(create).\n"));
	return(new);	
}

/*
 *	Routine: FreeHostInfo
 *	Global variables used: hosthead
 *	input:	 pointer to HostInfo structure that will be free
 *	return:	 none
 *	Free memory space of HostInfo structure and delete the entry
 *	from the link list.
 */
static void
FreeHostInfo(ptr)
struct HostInfo *ptr;
{
	struct HostInfo	 *previous;
	DDEBUG((L_DEBUG,"Execute FreeHostInfo.\n"));
	if (hosthead == NULL) {
		DDEBUG((L_DEBUG,"Exit FreeHostInfo(Hosthead=NULL).\n"));
		return;
	}

	if (hosthead == ptr) {
		hosthead = ptr->Next;
	} else {
		previous = hosthead;
		while (previous->Next != ptr)  {
			if (previous->Next== NULL) {
				DDEBUG((L_DEBUG,"Exit FreeHostInfo(Not Found).\n"));
				return;
			} else {
				previous = previous->Next;
			}
		}
		previous->Next = ptr->Next;
	}
	free(ptr);
	DDEBUG((L_DEBUG,"Exit FreeHostInfo(Found).\n"));
	return;
}

/*
 *	Routine: DoDump
 *	input:	 log_level, starting pointer, length 	 
 *	return:	 none
 *	Dump in hexadecimal 
 */
void
DoDump(level,ptr,len)
int level;
char *ptr;
int	len;
{  
	int	i;
	char buff[MAXLOGLINE];
	char *bp;
	if (level && ((level & LogLevel) == 0))
		return;
	i=0;
	bp = buff;
	while (i < len) {
		(void)sprintf (bp,"%2x ",ptr[i++]&0xFF);
		 bp +=3;
	}
	DoLog(level,"%s",buff);
}

/*
 *	Routine: SupportVersion
 *	input:	 Version number 
 *	return:	 flag for supporting or not
 *	Check client version number
 */
static int
SupportVersion(version)
ulong	version;
{	/* Since we just have only one version of protocol, return true */
	DDEBUG((L_DEBUG,"Execute SupportVersion.\n"));
	if (version != 1) {
		return(0);
	}
	return(TRUE);
}

/*
 *	Routine: HandleReceiveError
 *	Global variables used: ctlbuffer, databuffer, datachange
 *	input:	 errno
 *	return:	 none
 *	Handle the error from receiving message. The special case is
 *	MB2_MORE_DATA error which means that bootserver receives 
 *	data portion. This rotuine will allocate memory space and
 *	call mb2s_receive again.
 */
static void
HandleReceiveError(code)
int code;
{
	char *ptr;

	DDEBUG((L_DEBUG,"Execute HandleReceiveError.\n"));
	if (code != MB2_MORE_DATA) {
		DoLog (L_ERROR,"EXIT: Internal error, cannot receive message, errno=0x%x.",errno);
		exit(X_ERR_RCV);
	}
	if ((ptr = (char *)malloc(databuffer.len)) == NULL) {
		DoLog (L_ERROR,"EXIT: Internal error, insufficient space to allocate.");
		exit(X_INTERNAL);
	}
	databuffer.maxlen = databuffer.len;
	databuffer.buf = ptr;

	if ((mb2s_receive(portfd,(mb2_msginfo *)NULL,(mb2_buf *)NULL,&databuffer)) == -1) {
		DoLog (L_ERROR,"EXIT: Internal error, cannot receive message, errno=0x%x.",errno);
		exit(X_ERR_RCV);
	}
	datachange=ALLOC;
}

/* Since limitation of new TAI library is 32k bytes of data,
 * Then we need special routine to handle.
 */
static int
Mb2BigSendReply(fd,sptr,c,d,tid)
int fd;
mb2_socket *sptr;
mb2_buf *c, *d;
unchar tid;
{
	mb2_buf	tmpbuf;
	int	count;
	DDEBUG((L_DEBUG,"Execute Mb2BigSendReply of New TAI.\n"));
	tmpbuf.buf = d->buf;
	tmpbuf.maxlen = MAX_DATA_SENT;
	tmpbuf.len = MAX_DATA_SENT;
	count = d->len;
	DoLog (L_DEBUG,"Data part, size %d bytes (0x%x).",
			count,tmpbuf.buf);
	while (count > MAX_DATA_SENT) {
		if ((mb2s_sendreply(fd,sptr,(mb2_buf *)NULL,&tmpbuf,tid,MB2_NOTEOT)) == -1) {
			DoLog(L_ERROR,"EXIT: Cannot send reply to hostid=0x%x, errno=0x%x.",clientid,errno);
			exit(X_ERR_SND);
		}
		count -= MAX_DATA_SENT;
		tmpbuf.buf += MAX_DATA_SENT;
		DoLog (L_DEBUG,"Data fragment, send %d bytes,remain %d bytes (0x%x)",
			tmpbuf.len,count,tmpbuf.buf);
	}
	tmpbuf.len = count;
	DoLog (L_DEBUG,"Send Control part and Last fragment, %d bytes (0x%x)",
		tmpbuf.len,tmpbuf.buf);
	if ((mb2s_sendreply(fd,sptr,c,&tmpbuf,tid,MB2_EOT)) == -1) {
		DoLog(L_ERROR,"EXIT: Cannot send reply to hostid=0x%x, errno=0x%x.",clientid,errno);
		exit(X_ERR_SND);
	}
	return(0);
}
static void
HandleFragment()
{
	char *ptr;

	DDEBUG((L_DEBUG,"Execute HandleFragment.\n"));
	if ((ptr = (char *)malloc((unsigned)databuffer.len)) == NULL) {
		DoLog (L_ERROR,"EXIT: Internal error, insufficient space to allocate.");
		exit(X_INTERNAL);
	}
	databuffer.maxlen = databuffer.len;
	databuffer.buf = ptr;
	if ((mb2s_getreqfrag(portfd,&msginfo,&databuffer)) == -1) {
		DoLog (L_ERROR,"EXIT: Internal error, cannot get fragment message, errno=0x%x.",errno);
		exit(X_ERR_RCV);
	}
	datachange=ALLOC;
}
