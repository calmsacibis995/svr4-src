/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ipc:/ipcrm.c	1.7.6.3"

/*
 * ipcrm - IPC remove
 *
 * Remove specified message queues,
 * semaphore sets and shared memory ids.
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define NULL_MSG	((struct msqid_ds *)NULL)
#define NULL_SEM	((struct semid_ds *)NULL)
#define NULL_SHM	((struct shmid_ds *)NULL)

char opts[] = "q:m:s:Q:M:S:";	/* allowable options for getopt */
extern char	*optarg;	/* arg pointer for getopt */
extern int	optind;		/* option index for getopt */
extern int	errno;		/* error return */

main(argc, argv)
int	argc;
char	**argv;
{
	register int	o;	/* option flag */
	register int	err;	/* error count */
	register int	ipc_id;	/* id to remove */
	register key_t	ipc_key;/* key to remove */
	key_t getkey();
	void oops();

	/*
	 * If one or more of the IPC modules is not
	 * included in the kernel, the corresponding
	 * system calls will incur SIGSYS.  Ignoring
	 * that signal makes the system call appear
	 * to fail with errno == EINVAL, which can be
	 * interpreted appropriately in oops().
	 */

	(void) signal(SIGSYS, SIG_IGN);

	/*
	 * Go through the options.
	 */

	err = 0;
	while ((o = getopt(argc, argv, opts)) != EOF)
	{
		switch (o)
		{
		case 'q':	/* message queue */
			ipc_id = atoi(optarg);
			if (msgctl(ipc_id, IPC_RMID, NULL_MSG) == -1)
			{
				oops("msqid", optarg);
				err++;
			}
			break;

		case 'm':	/* shared memory */
			ipc_id = atoi(optarg);
			if (shmctl(ipc_id, IPC_RMID, NULL_SHM) == -1)
			{
				oops("shmid", optarg);
				err++;
			}
			break;

		case 's':	/* semaphores */
			ipc_id = atoi(optarg);
			if (semctl(ipc_id, 0, IPC_RMID, NULL_SEM) == -1)
			{
				oops("semid", optarg);
				err++;
			}
			break;

		case 'Q':	/* message queue (by key) */
			if ((ipc_key = getkey(optarg)) == 0)
			{
				err++;
				break;
			}
			if ((ipc_id = msgget(ipc_key, 0)) == -1
				|| msgctl(ipc_id, IPC_RMID, NULL_MSG) == -1)
			{
				oops("msgkey", optarg);
				err++;
			}
			break;

		case 'M':	/* shared memory (by key) */
			if ((ipc_key = getkey(optarg)) == 0)
			{
				err++;
				break;
			}
			if ((ipc_id = shmget(ipc_key, 0, 0)) == -1
				|| shmctl(ipc_id, IPC_RMID, NULL_SHM) == -1)
			{
				oops("shmkey", optarg);
				err++;
			}
			break;

		case 'S':	/* semaphores (by key) */
			if ((ipc_key = getkey(optarg)) == 0)
			{
				err++;
				break;
			}
			if ((ipc_id = semget(ipc_key, 0, 0)) == -1
				|| semctl(ipc_id, 0, IPC_RMID, NULL_SEM) == -1)
			{
				oops("semkey", optarg);
				err++;
			}
			break;

		case '?':	/* anything else */
		default:
			err++;
			break;
		}
	}
	if (err || (optind < argc))
	{
		(void) fprintf(stderr,
		   "usage: ipcrm [ [-q msqid] [-m shmid] [-s semid]\n%s\n",
		   "	[-Q msgkey] [-M shmkey] [-S semkey] ... ]");
		err++;
	}
	exit(err);
	/*NOTREACHED*/
}

void
oops(thing, arg)
char *thing;
char *arg;
{
	char *e;

	switch (errno)
	{
	case ENOENT:	/* key not found */
	case EINVAL:	/* id not found */
		e = "not found";
		break;

	case EPERM:
		e = "permission denied";
		break;
	default:
		e = "unknown error";
	}

	(void) fprintf(stderr, "ipcrm: %s(%s): %s\n", thing, arg, e);
}

key_t
getkey(kp)
register char *kp;
{
	key_t k;
	char *tp;	/* will point to char that terminates strtol scan */
	extern long strtol();

	if((k = (key_t)strtol(kp, &tp, 0)) == IPC_PRIVATE || *tp != '\0') {
		(void) fprintf(stderr, "illegal key: %s\n", kp);
		return 0;
	}
	return k;
}
