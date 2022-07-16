/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_MSG_H
#define _SYS_MSG_H

#ident	"@(#)head.sys:sys/msg.h	11.16.3.1"

/*
 * IPC Message Facility.
 */

/*
 * Implementation Constants.
 */

#define	PMSG	(PZERO + 2)	/* message facility sleep priority */

/*
 * Permission Definitions.
 */

#define	MSG_R	0400	/* read permission */
#define	MSG_W	0200	/* write permission */

/*
 * ipc_perm Mode Definitions.
 */

#define	MSG_RWAIT	01000	/* a reader is waiting for a message */
#define	MSG_WWAIT	02000	/* a writer is waiting to send */

/*
 * Message Operation Flags.
 */

#define	MSG_NOERROR	010000	/* no error if big message */

/*
 * There is one msg queue id data structure for each q in the system.
 */

/* Applications that read /dev/mem must be built like the kernel. A new
** symbol "_KMEMUSER" is defined for this purpose.
*/

#if defined(_KERNEL) || defined(_KMEMUSER)
/* expanded msqid_ds structure */

struct msqid_ds {
	struct ipc_perm msg_perm;	/* operation permission struct */
	struct msg	*msg_first;	/* ptr to first message on q */
	struct msg	*msg_last;	/* ptr to last message on q */
	ulong		msg_cbytes;	/* current # bytes on q */
	ulong		msg_qnum;	/* # of messages on q */
	ulong		msg_qbytes;	/* max # of bytes on q */
	pid_t		msg_lspid;	/* pid of last msgsnd */
	pid_t		msg_lrpid;	/* pid of last msgrcv */
	time_t		msg_stime;	/* last msgsnd time */
	long		msg_pad1;	/* reserved for time_t expansion */
	time_t		msg_rtime;	/* last msgrcv time */
	long		msg_pad2;	/* time_t expansion */
	time_t		msg_ctime;	/* last change time */
	long		msg_pad3;	/* time expansion */
	long		msg_pad4[4];		/* reserve area */
};

/* SVR3 structure */

struct o_msqid_ds {
	struct o_ipc_perm msg_perm;	/* operation permission struct */
	struct msg	*msg_first;	/* ptr to first message on q */
	struct msg	*msg_last;	/* ptr to last message on q */
	ushort		msg_cbytes;	/* current # bytes on q */
	ushort		msg_qnum;	/* # of messages on q */
	ushort		msg_qbytes;	/* max # of bytes on q */
	o_pid_t		msg_lspid;	/* pid of last msgsnd */
	o_pid_t		msg_lrpid;	/* pid of last msgrcv */
	time_t		msg_stime;	/* last msgsnd time */
	time_t		msg_rtime;	/* last msgrcv time */
	time_t		msg_ctime;	/* last change time */
};
#else	/* user definition */

#if !defined(_STYPES) 		/* EFT system */
/* this maps to the kernel struct msgid_ds */

struct msqid_ds {
	struct ipc_perm	msg_perm;	/* operation permission struct */
	struct msg	*msg_first;	/* ptr to first message on q */
	struct msg	*msg_last;	/* ptr to last message on q */
	ulong		msg_cbytes;	/* current # bytes on q */
	ulong		msg_qnum;	/* # of messages on q */
	ulong		msg_qbytes;	/* max # of bytes on q */
	pid_t		msg_lspid;	/* pid of last msgsnd */
	pid_t		msg_lrpid;	/* pid of last msgrcv */
	time_t		msg_stime;	/* last msgsnd time */
	long		msg_pad1;	/* reserved for time_t expansion */
	time_t		msg_rtime;	/* last msgrcv time */
	long		msg_pad2;	/* time_t expansion */
	time_t		msg_ctime;	/* last change time */
	long		msg_pad3;	/* time_t expansion */
	long		msg_pad4[4];	/* reserve area */
};
#else	/* NON EFT */

/* SVR3 structure - maps to kernel structure o_msqid_ds */

struct msqid_ds {
	struct ipc_perm	msg_perm;	/* operation permission struct */
	struct msg	*msg_first;	/* ptr to first message on q */
	struct msg	*msg_last;	/* ptr to last message on q */
	ushort		msg_cbytes;	/* current # bytes on q */
	ushort		msg_qnum;	/* # of messages on q */
	ushort		msg_qbytes;	/* max # of bytes on q */
	o_pid_t		msg_lspid;	/* pid of last msgsnd */
	o_pid_t		msg_lrpid;	/* pid of last msgrcv */
	time_t		msg_stime;	/* last msgsnd time */
	time_t		msg_rtime;	/* last msgrcv time */
	time_t		msg_ctime;	/* last change time */
};

#endif	/* !defined(_STYPES) */
#endif  /* defined(_KERNEL) */

/*
 * There is one msg structure for each message that may be in the system.
 */

struct msg {
	struct msg	*msg_next;	/* ptr to next message on q */
	long		msg_type;	/* message type */
	ushort		msg_ts;		/* message text size */
	short		msg_spot;	/* message text map address */
};

/*
 * User message buffer template for msgsnd and msgrecv system calls.
 */

struct msgbuf {
	long	mtype;		/* message type */
	char	mtext[1];	/* message text */
};

/*
 * Message information structure.
 */

struct msginfo {
	int	msgmap;	/* # of entries in msg map */
	int	msgmax;	/* max message size */
	int	msgmnb;	/* max # bytes on queue */
	int	msgmni;	/* # of message queue identifiers */
	int	msgssz;	/* msg segment size (should be word size multiple) */
	int	msgtql;	/* # of system message headers */
	ushort	msgseg;	/* # of msg segments (MUST BE < 32768) */
};

/*
 * We have to be able to lock a message queue since we can
 * sleep during message processing due to a page fault in
 * copyin/copyout or iomove.  We cannot add anything to the
 * msqid_ds structure since this is used in user programs
 * and any change would break object file compatibility.
 * Therefore, we allocate a parallel array, msglock, which
 * is used to lock a message queue.  The array is defined
 * in the msg master file.  The following macro takes a
 * pointer to a message queue and returns a pointer to the
 * lock entry.  The argument must be a pointer to a msgqid
 * structure.
 */

#define	MSGLOCK(X)	&msglock[X - msgque]

#if defined(__STDC__) && !defined(_KERNEL)
int msgctl(int, int, ...);
int msgget(key_t, int);
int msgrcv(int, void *, size_t, long, int);
int msgsnd(int, const void *, size_t, int);
#endif

#endif	/* _SYS_MSG_H */
