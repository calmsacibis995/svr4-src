/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:sys/semsys.c	1.6.1.6"
#ifdef __STDC__
	#pragma weak semctl = _semctl
	#pragma weak semget = _semget
	#pragma weak semop = _semop
#endif
#include	"synonyms.h"
#include	"sys/types.h"
#include	"sys/ipc.h"
#include	"sys/sem.h"

#define	SEMSYS	53

#define	SEMCTL	0
#define	SEMGET	1
#define	SEMOP	2

extern long syscall();

#ifdef __STDC__

	#include "stdarg.h"

	int
	semctl(int semid, int semnum, int cmd, ...)
	{
		void *arg;
		va_list ap;

		va_start(ap, cmd);
		arg = va_arg(ap, void *);
		va_end(ap);

		return(syscall(SEMSYS, SEMCTL, semid, semnum, cmd, arg));
	}

#else	/* pre-ANSI version */

	union semun {
		int val;
		struct semid_ds *buf;
		ushort *array;
	};

	int
	semctl(semid, semnum, cmd, arg)
	int semid, semnum, cmd;
	union semun arg;
	{
		return(syscall(SEMSYS, SEMCTL, semid, semnum, cmd, arg));
	}

#endif	/* __STDC__ */

int
semget(key, nsems, semflg)
key_t key;
int nsems, semflg;
{
	return(syscall(SEMSYS, SEMGET, key, nsems, semflg));
}

int
semop(semid, sops, nsops)
int semid;
struct sembuf *sops;
unsigned int nsops;
{
	return(syscall(SEMSYS, SEMOP, semid, sops, nsops));
}
