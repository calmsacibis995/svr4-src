/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-i386:sys/shmsys.c	1.1"
#ifdef __STDC__
	#pragma weak shmat = _shmat
	#pragma weak shmctl = _shmctl
	#pragma weak shmdt = _shmdt
	#pragma weak shmget = _shmget
#endif
#include	"synonyms.h"
#include	"sys/types.h"
#include	"sys/ipc.h"
#include	"sys/shm.h"

#define	SHMSYS	52

#define	SHMAT	0
#define	SHMCTL	1
#define	SHMDT	2
#define	SHMGET	3

extern long syscall();

VOID *
shmat(shmid, shmaddr, shmflg)
int shmid;
VOID *shmaddr;
int shmflg;
{
	return((char *)syscall(SHMSYS, SHMAT, shmid, shmaddr, shmflg));
}

#ifdef __STDC__

	#include "stdarg.h"

	int
	shmctl(int shmid, int cmd, ...)
	{
		struct shmid_ds *buf;
		va_list ap;

		va_start(ap, cmd);
		buf = va_arg(ap, struct shmid_ds *);
		va_end(ap);

		return(syscall(SHMSYS, SHMCTL, shmid, cmd, buf));
	}

#else	/* pre-ANSI version */

	int
	shmctl(shmid, cmd, buf)
	int shmid, cmd;
	struct shmid_ds *buf;
	{
		return(syscall(SHMSYS, SHMCTL, shmid, cmd, buf));
	}

#endif	/* __STDC__ */

int
shmdt(shmaddr)
char *shmaddr;
{
	return(syscall(SHMSYS, SHMDT, shmaddr));
}

int
shmget(key, size, shmflg)
key_t key;
int size, shmflg;
{
	return(syscall(SHMSYS, SHMGET, key, size, shmflg));
}
