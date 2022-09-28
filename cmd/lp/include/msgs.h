/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:include/msgs.h	1.18.3.1"

# include	<sys/types.h>
# include	<poll.h>
# include	<stdarg.h>
# include	<stropts.h>

#if	!defined(_LP_MSGS_H)
# define	_LP_MSGS_H

/*
 * THE DISPATCH TABLE DEPENDS ON EACH R_... MESSAGE FOLLOWING
 * IMMEDIATELY AFTER ITS CORRESPONDING S_... COUNTERPART.
 * I.E R_... MESSAGE FOR A S_... MESSAGE IS (S_... + 1)
 */
# define	R_BAD_MESSAGE			0
# define	S_NEW_QUEUE			1
# define	R_NEW_QUEUE			2
# define	S_ALLOC_FILES			3
# define	R_ALLOC_FILES			4
# define	S_PRINT_REQUEST			5
# define	R_PRINT_REQUEST			6
# define	S_START_CHANGE_REQUEST		7
# define	R_START_CHANGE_REQUEST		8
# define	S_END_CHANGE_REQUEST		9
# define	R_END_CHANGE_REQUEST		10
# define	S_CANCEL_REQUEST		11
# define	R_CANCEL_REQUEST		12
# define	S_INQUIRE_REQUEST		13
# define	R_INQUIRE_REQUEST		14
# define	S_LOAD_PRINTER			15
# define	R_LOAD_PRINTER			16
# define	S_UNLOAD_PRINTER		17
# define	R_UNLOAD_PRINTER		18
# define	S_INQUIRE_PRINTER_STATUS	19
# define	R_INQUIRE_PRINTER_STATUS	20
# define	S_LOAD_CLASS			21
# define	R_LOAD_CLASS			22
# define	S_UNLOAD_CLASS			23
# define	R_UNLOAD_CLASS			24
# define	S_INQUIRE_CLASS			25
# define	R_INQUIRE_CLASS			26
# define	S_MOUNT				27
# define	R_MOUNT				28
# define	S_UNMOUNT			29
# define	R_UNMOUNT			30
# define	S_MOVE_REQUEST			31
# define	R_MOVE_REQUEST			32
# define	S_MOVE_DEST			33
# define	R_MOVE_DEST			34
# define	S_ACCEPT_DEST			35
# define	R_ACCEPT_DEST			36
# define	S_REJECT_DEST			37
# define	R_REJECT_DEST			38
# define	S_ENABLE_DEST			39
# define	R_ENABLE_DEST			40
# define	S_DISABLE_DEST			41
# define	R_DISABLE_DEST			42
# define	S_LOAD_FILTER_TABLE		43
# define	R_LOAD_FILTER_TABLE		44
# define	S_UNLOAD_FILTER_TABLE		45
# define	R_UNLOAD_FILTER_TABLE		46
# define	S_LOAD_PRINTWHEEL		47
# define	R_LOAD_PRINTWHEEL		48
# define	S_UNLOAD_PRINTWHEEL		49
# define	R_UNLOAD_PRINTWHEEL		50
# define	S_LOAD_USER_FILE		51
# define	R_LOAD_USER_FILE		52
# define	S_UNLOAD_USER_FILE		53
# define	R_UNLOAD_USER_FILE		54
# define	S_LOAD_FORM			55
# define	R_LOAD_FORM			56
# define	S_UNLOAD_FORM			57
# define	R_UNLOAD_FORM			58
# define	S_GETSTATUS			59
# define	R_GETSTATUS			60
# define	S_QUIET_ALERT			61
# define	R_QUIET_ALERT			62
# define	S_SEND_FAULT			63
# define	R_SEND_FAULT			64
# define	S_SHUTDOWN			65
# define	R_SHUTDOWN			66
# define	S_GOODBYE			67
# define	S_CHILD_DONE			68

/*
**	These are for use by the scheduler only
*/
# define	I_GET_TYPE			69
# define	I_QUEUE_CHK			70
# define	R_CONNECT			71

# define	S_GET_STATUS			72
# define	R_GET_STATUS			73
# define	S_INQUIRE_REQUEST_RANK		74
# define	R_INQUIRE_REQUEST_RANK		75
# define	S_CANCEL			76
# define	R_CANCEL			77
# define	S_NEW_CHILD			78
# define	R_NEW_CHILD			79
# define	S_SEND_JOB			80
# define	R_SEND_JOB			81
# define	S_JOB_COMPLETED			82
# define	R_JOB_COMPLETED			83
# define	S_INQUIRE_REMOTE_PRINTER	84
# define	R_INQUIRE_REMOTE_PRINTER	20
# define	S_CHILD_SYNC			85
# define	S_LOAD_SYSTEM			86
# define	R_LOAD_SYSTEM			87
# define	S_UNLOAD_SYSTEM			88
# define	R_UNLOAD_SYSTEM			89
/*
**	Last available message
*/
# define	LAST_MESSAGE			89

/*
**      These are the possible status codes returned by the scheduler
*/
# define	MOK		 0
# define	MOKMORE		 1
# define	MOKREMOTE	 2
# define	MMORERR		 3
# define	MNODEST		 4
# define	MERRDEST	 5
# define	MDENYDEST	 6
# define	MNOMEDIA	 7
# define	MDENYMEDIA	 8
# define	MNOFILTER	 9
# define	MNOINFO		10
# define	MNOMEM		11
# define	MNOMOUNT	12
# define	MNOOPEN		13
# define	MNOPERM		14
# define	MNOSTART	15
# define	MUNKNOWN	16
# define	M2LATE		17
# define	MNOSPACE	18
# define	MBUSY		19
# define	MTRANSMITERR	20
# define	MNOMORE		21
# define	MGONEREMOTE	22

/*
** Offsets and lengths of the various elements of the message header.
**
**	Macro		Data Type	Size	Comment
**
**	HEAD_RESYNC	2 bytes		(2)	*
**	HEAD_AUTHCODE	short + long	(6)	*
**
**	HEAD_SIZE	4 bytes		(4)	\
**	HEAD_TYPE	4 bytes		(4)	 > message propper
**	HEAD_DATA	n bytes		(n)	/
**
**	TAIL_CHKSUM	4 bytes		(4)	*
**	TAIL_ENDSYNC	2 bytes		(2)	*
**
**	Items marked with an asterisk are only used with the 3.2
**	Spooler protocol.
*/

/*
**	3.2 Protocol Header Information:
**		2-byte message introduction
**		6-byte client authorization data
*/
#define	HEAD_RESYNC		(0)
#define HEAD_RESYNC_LEN		2
#define HEAD_AUTHCODE		(HEAD_RESYNC + HEAD_RESYNC_LEN)
#define HEAD_AUTHCODE_LEN		(sizeof(short) + sizeof(long))

/*
**	3.2 Protocol Message Information:
**		4-byte message size
**		4-byte message type
**		n-byte message data
*/
#define HEAD_SIZE		(HEAD_AUTHCODE + HEAD_AUTHCODE_LEN)
#define HEAD_SIZE_LEN			4
#define HEAD_TYPE		(HEAD_SIZE + HEAD_SIZE_LEN)
#define HEAD_TYPE_LEN			4
#define HEAD_DATA		(HEAD_TYPE + HEAD_TYPE_LEN)

/*
**	3.2 Protocol Size of non-data header information
*/
#define HEAD_LEN		HEAD_DATA

/*
**	Equivalents for 4.0 protocol
*/
#define MESG_SIZE		(0)
#define MESG_SIZE_LEN			4
#define MESG_TYPE		(MESG_SIZE + MESG_SIZE_LEN)
#define MESG_TYPE_LEN			4
#define MESG_DATA		(MESG_TYPE + MESG_TYPE_LEN)

#define MESG_LEN		MESG_DATA

/*
**	3.2 Protocol Trailer Information:
**		4-byte message check sum
**		2-byte message closing identifier
**
**	"N" is the decoded value of buffer[HEAD_SIZE].  This must
**	be provided because messages are variable length.
*/
#define	TAIL_ENDSYNC_LEN		2
#define	TAIL_ENDSYNC(N)		(N - TAIL_ENDSYNC_LEN)
#define TAIL_CHKSUM_LEN			4
#define TAIL_CHKSUM(N)		(TAIL_ENDSYNC(N) - TAIL_CHKSUM_LEN)

/*
**	3.2 Protocol Size of non-data trailer information
*/
#define	TAIL_LEN		(TAIL_CHKSUM_LEN + TAIL_ENDSYNC_LEN)

/*
**	3.2 Protocol Size of all non-data information
**	(This is also the minimum size for 3.2 protocol messages)
*/
#define	CONTROL_LEN		(HEAD_LEN + TAIL_LEN)

/*
**	Size of excess data induced by 3.2 Protocol.
**	(This is also the size differance between 3.2 & 4.0 protocols)
*/
#define	EXCESS_3_2_LEN		(HEAD_SIZE + TAIL_LEN)
/**
 ** Checksum:
 **/
#define CALC_CHKSUM(B,SZ,RC) \
if (SZ >= CONTROL_LEN) \
{ \
    register unsigned char	*p = (unsigned char *)B, \
			    *pend = p + SZ - TAIL_LEN; \
    RC = 0; \
    while (p < pend) \
	RC += *p++;  /* let it overflow */ \
} \
else \
    return ((errno = EINVAL, -1))

/*
**      Largest size permitted for any given message
*/
# define	MSGMAX		2048

/*
**      Possible values of the type field of S_QUIET_ALERT
*/
# define	QA_FORM		1
# define	QA_PRINTER	2
# define	QA_PRINTWHEEL	3

typedef	struct	strbuf	strbuf_t;	/*  STREAMS buffer */

typedef	struct mque
{
    struct mque	  *next;
    struct strbuf *dat;
} MQUE;

/*
**	Definition of a message descriptor
*/
typedef struct
{
    short	type;			/* type of connection */
    int		readfd;			/* STREAM fd to read from */
    int		writefd;		/* STREAM fd to write to */
    int		wait;			/* number of systems waiting for */
    char	*file;			/* pipe name if type==MD_FIFO */
    short	state;			/* Current state of client */
    short	admin;			/* Non zero if admin  */
    short	event;			/* Event returned from poll */
    MQUE *	mque;			/* backlogged message ptr */
    uid_t	uid;			/* Clients UID */
    gid_t	gid;			/* Clients GID */
    void	(**on_discon)();	/* Clean up functions */
} MESG;

# define	MDSIZE	(sizeof(MESG))

/*
**	Possible values of MESG.state
*/
# define	MDS_IDLE	0

# define	MDS_32PROTO	320
# define	MDS_32CONNECT	321

/*
**	Possible values of MESG.type
*/
# define	MD_UNKNOWN	0	/* We don't know just yet */
# define	MD_STREAM	1	/* 4.0 STREAMS pipe protocol */
# define	MD_BOUND	2	/* 4.0 STREAMS fd protocol */
# define	MD_SYS_FIFO	3	/* 3.2 named-pipe protocol */
# define	MD_USR_FIFO	4	/* 3.2 named-pipe protocol */
# define	MD_MASTER	5	/* MD_STREAM used by lpsched */
# define	MD_CHILD	6	/* MD_STREAM to a child process */

/*
**	Definition for a FIFO buffer (used
**	in read_fifo.
*/
typedef struct
{
	int	full;
	char	save [MSGMAX],
		*psave,
		*psave_end;
} fifobuffer_t;

#if	defined(__STDC__)
/*
**      Definitions for the rest of the world and lint
*/
/*
**	Server functions in order of usage
*/
MESG		* mcreate ( char * );
int		mlisteninit ( MESG * );
MESG		* mlisten ( void );
int		mlistenadd ( MESG *, short );
int		mon_discon ( MESG *, void (*)());
MESG		* mlistenreset ( void );
int		mdestroy ( MESG * );

/*
**	Client functions in order of typical usage
*/
MESG		* mconnect ( char *, int, int );
int		mgetm ( MESG *, int, ... );
int		mwrite ( MESG *, char * );
int		mputm ( MESG *, int, ... );
int		mread ( MESG *, char *, int );
short		msize ( char * );
short		mpeek ( MESG * );
int		mdisconnect ( MESG * );

/*
**	This may be called to deallocate internal buffers allocated
**	by mgetm and mputm.  Probably not useful except right before
**	a fork().
*/
void		__mbfree ( void );

/*
**	Client functions for pre-4.0 compatability
*/
int		mclose ( void );
int		mneeds ( void );
int		mopen ( void );
int		mrecv ( char *, int );
int		msend ( char * );

int		Putmsg (MESG *, strbuf_t *, strbuf_t *, int);
int		Getmsg (MESG *, strbuf_t *, strbuf_t *, int *);
int		read3_2 (MESG * md, char *msgbuf, int size);
int		write3_2 (MESG *, char *, int);
int		read_fifo (int, char *, unsigned int);
int		write_fifo (int, char *, unsigned int);
int		ResetFifoBuffer (int);
fifobuffer_t	*GetFifoBuffer (int);

/*
**	General purpose message manipulating functions
*/
char		* htos ( char *, unsigned short );
char		* ltos ( char *, unsigned long );
unsigned long	stol ( char * );
unsigned short	stoh ( char * );
int		_getmessage ( char *, short, va_list );
int		_putmessage ( char *, short, va_list );
int		getmessage ( char *, short, ... );
int		putmessage ( char *, short, ... );

#else	/* defined(__STDC__) */

/*
**      Definitions for the rest of the world and lint
*/
/*
**	Server functions in order of usage
*/
MESG		* mcreate();
int		mlisteninit();
MESG		* mlisten();
int		mlistenadd();
int		mon_discon();
MESG		* mlistenreset();
int		mdestroy();

/*
**	Client functions in order of typical usage
*/
MESG		* mconnect();
int		mgetm();
int		mwrite();
int		mputm();
int		mread();
short		msize();
short		mpeek();
int		mdisconnect();

/*
**	This may be called to deallocate internal buffers allocated
**	by mgetm and mputm.  Probably not useful except right before
**	a fork().
*/
void		__mbfree();

/*
**	Client functions for pre-4.0 compatability
*/
int		mclose();
int		mneeds();
int		mopen();
int		mrecv();
int		msend();

int		Getmsg();
int		Putmsg();
int		read3_2();
int		write3_2();
int		read_fifo();
int		write_fifo();
int		ResetFifoBuffer ();
fifobuffer_t	*GetFifoBuffer ();

/*
**	General purpose message manipulating functions
*/
char		* htos();
char		* ltos();
unsigned long	stol();
unsigned short	stoh();
int		_getmessage();
int		_putmessage();
int		getmessage();
int		putmessage();

#endif	/* defined(__STDC__) */
/*
**	This will yield the type of a message
*/
# define	mtype(buffer)	(getmessage(buffer, I_GET_TYPE))

/*
**	This will yeild the size of a message
*/
# define	msize(buffer)	(stoh(buffer))

/*
**	Pass this for the request-id argument of S_CANCEL
**	to obtain the effect of the 3.2 S_CANCEL_REQUEST.
*/
# define	CURRENT_REQ	"current"

#endif	/* !defined (_LP_MSGS_H) */
