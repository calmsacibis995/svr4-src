/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)x286emul:msgsys.c	1.1"

/*
 * This file contains the support routines for the IPC system calls.
 */

#include "vars.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>


Msgctl( msqid, cmd, buf )
	int msqid, cmd;
	char *buf;		/* ptr to x286 struct msqid_ds */
{
	int rv;
	struct msqid_ds msg;

	if (buf == BAD_ADDR) {
		errno = EFAULT;
		return -1;
	}

	msqid &= 0xffff;	/* ensure unsigned int value */

	switch (cmd) {
	case IPC_STAT:
		msg.msg_perm = *(struct ipc_perm *)buf;
		msg.msg_qbytes = *(ushort *)(buf+24);

		rv = msgctl( msqid, cmd, &msg );

		*(struct ipc_perm *)buf = msg.msg_perm;
		*(ushort *)(buf+20) = msg.msg_cbytes;
		*(ushort *)(buf+22) = msg.msg_qnum;
		*(ushort *)(buf+24) = msg.msg_qbytes;
		*(ushort *)(buf+26) = msg.msg_lspid;
		*(ushort *)(buf+28) = msg.msg_lrpid;
		*(time_t *)(buf+30) = msg.msg_stime;
		*(time_t *)(buf+34) = msg.msg_rtime;
		*(time_t *)(buf+38) = msg.msg_ctime;
		return rv;

	case IPC_SET:
		msg.msg_perm = *(struct ipc_perm *)buf;
		msg.msg_qbytes = *(ushort *)(buf+24);
			/* FALL THRU */

	case IPC_RMID:
		return msgctl( msqid, cmd, &msg );

	default:
		errno = EINVAL;
		return -1;
	}
}


Semctl(semid, semnum, cmd, arg)
	int semid, semnum, cmd;
	unsigned short *arg;
{
	char *cvtarg;
	struct semid_ds sem386;
	long argl;
	int rv;

	semid &= 0xffff;	/* ensure unsigned int value */

	switch( cmd ) {
		/* arg is a short */
	case SETVAL:
		if ( (argl = *(ushort *)arg) <= 32767) {
			return semctl(semid, semnum, cmd, argl);
		} else {
			errno = ERANGE;
			return -1;
		}

		/* no conversion needed for these commands */
	case GETVAL:
	case GETPID:
	case GETNCNT:
	case GETZCNT:
	case IPC_RMID:
		return semctl(semid, semnum, cmd, 0);

		/* buffer address conversion needed */
	case GETALL:
	case SETALL:
	case IPC_STAT:
	case IPC_SET:
		if (!Ldata) {
			cvtarg = cvtptr(MAKEPTR( Stacksel, *arg ) );
		} else {
			cvtarg = cvtptr(*(long *)arg);
		}
		if (cvtarg == BAD_ADDR) {
			errno = EFAULT;
			return -1;
		}
		break;
	default:
		errno = EINVAL;
		return -1;
	}

	switch( cmd ) {
		/* only needed correct address to sys call */
	case GETALL:
	case SETALL:
		/*
		 * This one works 'cause the kernel only looks at a
		 * few fields in the ipc_perm sub-struct
		 */
	case IPC_SET:
		return semctl(semid, semnum, cmd, cvtarg);

	case IPC_STAT:
		rv = semctl(semid, semnum, cmd, &sem386);
			/* get the ipc_perm sub-struct */
		copymem(&sem386, cvtarg, sizeof(struct ipc_perm));
			/* get sem_nsems */
		*(ushort *)(cvtarg + sizeof(struct ipc_perm) + 2) =
			sem386.sem_nsems;
			/* get the time fields */
		copymem(&sem386.sem_otime, 
			cvtarg + sizeof(struct ipc_perm) + 4,
				2*sizeof(time_t) );
		return rv;
	}
}
