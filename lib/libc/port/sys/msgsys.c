/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:sys/msgsys.c	1.6.1.7"
#ifdef __STDC__
	#pragma weak msgctl = _msgctl
	#pragma weak msgget = _msgget
	#pragma weak msgrcv = _msgrcv
	#pragma weak msgsnd = _msgsnd
#endif
#include	"synonyms.h"
#include	"sys/types.h"
#include	"sys/ipc.h"
#include	"sys/msg.h"

#define	MSGSYS	49

#define	MSGGET	0
#define	MSGCTL	1
#define	MSGRCV	2
#define	MSGSND	3

extern long syscall();

int
msgget(key, msgflg)
key_t key;
int msgflg;
{
	return(syscall(MSGSYS, MSGGET, key, msgflg));
}

#ifdef __STDC__

	#include "stdarg.h"

	int
	msgctl(int msqid, int cmd, ...)
	{
		struct msqid_ds *buf;
		va_list ap;

		va_start(ap, cmd);
		buf = va_arg(ap, struct msqid_ds *);
		va_end(ap);

		return(syscall(MSGSYS, MSGCTL, msqid, cmd, buf));
	}

#else	/* pre-ANSI version */

	int
	msgctl(msqid, cmd, buf)
	int msqid, cmd;
	struct msqid_ds *buf;
	{
		return(syscall(MSGSYS, MSGCTL, msqid, cmd, buf));
	}

#endif	/* __STDC__ */

int
msgrcv(msqid, msgp, msgsz, msgtyp, msgflg)
int msqid;
void *msgp;
size_t msgsz;
long msgtyp;
int msgflg;
{
	return(syscall(MSGSYS, MSGRCV, msqid, msgp, msgsz, msgtyp, msgflg));
}

int
msgsnd(msqid, msgp, msgsz, msgflg)
int msqid;
const void *msgp;
size_t msgsz;
int msgflg;
{
	return(syscall(MSGSYS, MSGSND, msqid, msgp, msgsz, msgflg));
}
