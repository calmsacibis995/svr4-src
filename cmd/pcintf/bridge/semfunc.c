/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/semfunc.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)semfunc.c	3.15	LCC);	/* Modified: 11/17/89 18:31:22 */

/*****************************************************************************

	Copyright (c) 1986 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#include "pci_types.h"
#include <errno.h>

#ifdef   RD3	 	   /* ipc include files for Xenix */
#include	<ipc.h>
#include	<sd.h>
#endif

#ifdef  RD5 		/* ipc for Sys V */
/*#include	<sys/immu.h>*/
#include	<sys/ipc.h>
#include	<sys/sem.h>
#include	<sys/shm.h>
#endif

/*
 * Semaphore functions for reliable delivery.
 *
 *	Reliable delivery will be useing semaphores for synchronization
 *	of ACK and DATA packets to the PC.
 *
 *	This module contains the definitions and functions for the
 *	current implemetation using semaphores.
 *
 *	Author: Richard W. Patterson	3/24/86
 *
 *  Currently this is only for SCO Xenix System V (using System III IPC)
 *			   and ATT Unix  System V (using System  V  IPC)
 *
 */

extern int rd_flag;

#ifdef	RD3                 /* XENIX Sys III */
#define sem_template	"/tmp/%06dRDrp.SEM"
extern int errno;
static char rdsem_name[20]; 		/* semaphore name */
extern char *sdget();
#endif

#ifdef	RD5
#define	PCI_KEY	(key_t) (((long)'P' << 16) | (long)(('C' << 8) | 'I'))

		/* ipc for Sys V */
        	/* system V sem structures */
struct	sembuf decr[] = {
			 {0, -1, SEM_UNDO & IPC_NOWAIT}
			};

struct	sembuf incr[] = {
			 {0, 1, SEM_UNDO & IPC_NOWAIT}
			};

union	semnum {
		int	val;
		struct	semid_ds *buf;
		ushort	array[1];
	       };


static	union semnum ctl_arg;		/* semctl argument union */
#endif



/*
 * RDSEM_INIT		Initializes a reliable delivery semaphore in
 *			the form xxxxxxRDrpSEM.  Where xxxxxx is the PID
 *			of the creating process.  For SCO Xenix.
 *
 *	Entry: None
 *
 *	Returns:
 *		-1	Failure
 *		 0	Success
 *
 */
rdsem_init()
{
	int	pid,		/* process id */
		rdsem;		/* semaphore number for reliable delivery */
	int	err;		/* saves value of errno */

	pid = getpid();

#ifdef	RD3                 /* XENIX Sys III */
	sprintf(rdsem_name, sem_template, pid);	/* create the name */
	if ((rdsem = creatsem(rdsem_name, 0777)) != -1) {
		log("rdsem_init: init of semaphore completed.\n");
		return(rdsem);
	}
	log("rdsem_init: init of semaphore failed.\n");

#else /* RD5 */
#ifdef RD5
	if ((rdsem = semget(PCI_KEY + pid, 1, IPC_CREAT|0777)) != -1) {
		ctl_arg.val = 1;		/* init sem to 1 */
		if (semctl(rdsem, 0, SETVAL, ctl_arg) < 0) {
			log("semctl: errno: %d\n", errno);
			return(-1);		/* call failed */
		}
		return(rdsem);
	}
	err = errno;
	log("semget: errno: %d\n", errno);
	serious("Cannot create semaphore, errno = %d\n", err);
#endif  /* RD5 */
#endif  /* RD3 */
	return(-1);
}


/*
 * RDSEM_OPEN		This function is called by other processes
 *			requiring access to a semaphore.
 *
 *	Entry:
 *		None
 *
 *	Returns:
 *		-1	Failure
 *		 0	Success
 *
 */
rdsem_open()
{
	int	semn;
	int	ppid;

	ppid = getppid();

#ifdef	RD3                 /* XENIX Sys III */
	sprintf(rdsem_name, sem_template, getppid());	/* create the name */
	if ((semn = opensem(rdsem_name)) != -1) {
#else	/* RD5 */
	if ((semn = semget(PCI_KEY + ppid, 1, 0777)) != -1) {
#endif 	/* RD3 */
		log("rdsem_open: success\n");
		return(semn);
	}
	log("rdsem_open: failure\n");
}
	

/*
 * RDSEM_ACCESS		Returns whether a process can/has gained access
 *			to a particular semaphore.
 *
 *	Entry:
 *		sem_id		The desired semaphore to gain access to.
 *
 *	Return:
 *		-1		Semaphore is busy or non-existent
 *
 *
 */
rdsem_access(sem_id)
register int sem_id;
{
#ifdef	RD3                 /* XENIX Sys III */
	if (nbwaitsem(sem_id) != -1) {
		log("rdsem_access: gained access.\n");
		return(0);
	}
	log("rdsem_access: access denied.\n");

	switch (errno) {
		case ENAVAIL : log("rdsem_access: sem not available.\n");
			       break;

		default : log("rdsem_access: errno: %d\n", errno);
	}

	return(-1);
#endif
#ifdef  RD5
	if (semop(sem_id, decr, 1) < 0) {
		log("rdsem_access: semop errno: %d\n", errno);
		return(-1);
	}

	log("rdsem_access: gained access.\n");
	return(0);
#endif
#if	!defined(RD5) && !defined(RD3)
	log("rdsem_access: Semaphores not supported presently.\n");
#endif
	
}



/*
 * RDSEM_RELINQ		Relinquishes control of the passed semaphore.
 *
 *	Entry:
 *		sem_num		The desired semaphore to relinquish control.
 *
 *	Return:
 *		-1		Semaphore is busy or non-existent
 *
 */
rdsem_relinq(sem_num)
register int sem_num;
{

#ifdef	RD3                 /* XENIX Sys III */
	return(sigsem(sem_num));

#endif
#ifdef  RD5
	if (semop(sem_num, incr, 1) < 0) {
		log("rdsem_relinq: semop errno: %d\n", errno);
		return(-1);
	}

	log("rdsem_relinq: released.\n");
	return(0);
#endif
#if (!defined(RD3)  && !defined(RD5))
	log("rdsem_relinq: Semaphores not supported presently.\n");
#endif
}


/*
 * RDSEM_UNLNK		Deletes the previously created reliable delivery
 *			semaphore.
 *
 *	Entry:
 *		None
 *
 *	Returns:
 *		Nothing
 *
 */
rdsem_unlnk(sem_id)
register int sem_id;
{
	if (!rd_flag)
		return;

#ifdef	RD3                 /* XENIX Sys III */
	unlink(rdsem_name);
#endif
#ifdef   RD5
	/* under ULTRIX 3.1 the following returns -1 and sets errno */
	/* to EINVAL even though it works! */
	if (semctl(sem_id, 0, IPC_RMID, 0L) < 0)
		log("rdsem_unlnk: semctl errno: %d\n", errno);
#endif
}


/*
 * Shared memory functions for reliable delivery.
 *
 *	This module contains the definitions and functions for the
 *	current implemetation using shared memory.
 *
 *	Author: Richard W. Patterson	4/16/86
 *
 *
 */

#ifdef	RD3                 /* XENIX Sys III */
#define shm_template	"/tmp/%06dRDrp.SHM"
static char	RDshmpath[20];		/* for shared memory file. */
#endif

#if	defined(RD5) && !defined(SYS5_4)	/* ipc for Sys V */
extern	char *shmat();
#endif

/*
 * RD_SHMINIT		This procedure initializes the shared memory for
 *			reliable delivery.  It sets up the necessary
 *			structures so other processes can access it.
 *
 *	Entry:	None
 *
 *	Returns:
 *		shmid	Returns a shared memory descriptor
 *		   -1	Could not create the shared memory
 *
 */
#ifdef	RD3                 /* XENIX Sys III */
char *
#endif
rd_sdinit()
{

#ifdef	RD3                 /* XENIX Sys III */
	char	*shmid;		/* shared memory descriptor */

	sprintf(RDshmpath, shm_template, getpid());
	if ((shmid = (char *) sdget(RDshmpath, SD_CREAT|SD_UNLOCK|SD_WRITE,
			((long) sizeof(struct rd_shared_mem)), 0777)) < 0)
		return((char *) -1);
	return(shmid);
#endif

#ifdef  RD5			/* ipc for Sys V */
	int	shmid;		/* shared memory descriptor */
	key_t	key;		/* key for shared memory */
	int	err;		/* saves value of errno */

	key = (key_t) getpid();	/* use process ID for key */
	if ((shmid = shmget(key, sizeof(struct rd_shared_mem), 
					IPC_CREAT|SHM_R|SHM_W)) < 0) {
		err = errno;
		log("shmget: errno: %d\n", errno);
		serious("Cannot create shared mem segment, errno = %d\n", err);
		return(-1);
	}
	return(shmid);

#endif
}


/*
 * RD_SDOPEN		This procedure essentially "opens" an already
 *			existing shared memory segment.  It returns a
 *			descriptor to the calling process.  The calling
 *			process will pass its parent's PID.  The parent's
 *			PID is used as a watermark.
 *
 *	Entry:  None
 *
 *	Returns:
 *		shmid		This processes shared mem ID
 *		   -1		Could not access/open this segment
 *
 */
#ifdef	RD3                 /* XENIX Sys III */
char *
#endif
rd_sdopen()
{

#ifdef	RD3                 /* XENIX Sys III */
	char	*shmid;		/* shared memory descriptor */
	int	pid;

	pid = getppid();	/* use parent's process id */
	sprintf(RDshmpath, shm_template, pid);
	if ((shmid = (char *) sdget(RDshmpath, SD_WRITE,
			((long) sizeof(struct rd_shared_mem)), 0777)) < 0)
		return((char *) -1);
	return(shmid);
#endif

#ifdef  RD5			/* ipc for Sys V */
	int	shmid;		/* shared memory descriptor */
	key_t	key;		/* key for shared memory */

	key = (key_t) getppid();	/* use parent's process ID */
	if ((shmid = shmget(key, sizeof(struct rd_shared_mem), 
				SHM_R|SHM_W)) < 0) {
		log("shmget: errno: %d\n", errno);
		return(-1);
	}
	return(shmid);
#endif

}


/*
 * RD_SDENTER		This procedure requests that the system attach the
 *			desinated shared memory to the process' addr space.
 *
 *	Entry:
 *		shmdesc 	Desciptor for the shared memory
 *
 *	Returns:
 *		>0		Ptr to shared memory segment. (Unix Sys V)
 *		 0		Attach failed.
 *
 */
char *
rd_sdenter(shmdesc)
#ifdef	RD3                 /* XENIX Sys III */
char *shmdesc;			/* Xenix Sys III IPC uses char ptr. */
#endif

#ifdef  RD5			/* ipc for Sys V */
int	shmdesc;		/* Unix Sys V IPC uses integer. */
#endif
{

#ifdef	RD3                 /* XENIX Sys III */
	if (sdenter(shmdesc, SD_WRITE) < 0)
		return(0);
	return(shmdesc);
#endif

#ifdef  RD5			/* ipc for Sys V */
	char	*ptr;
	if ((ptr = shmat(shmdesc, 0, SHM_R|SHM_W)) == (char *)-1) {
		log("shmat: errno: %d\n", errno);
		return(0);
	}
	return(ptr);
#endif
}


/*
 * RD_SDLEAVE		This procedures requests that the system deatach
 *			the specified shared memory from the calling
 *			process' address space.
 *
 *	Entry:
 *		shmaddr		Descriptor (segment addr) for the shared memory.
 *
 *	Returns:
 *		 0		The memory was detached.
 *		-1		Error in detaching the memory.
 *
 */
rd_sdleave(shmaddr)
char *shmaddr;
{

#ifdef	RD3                 /* XENIX Sys III */
	if (sdleave(shmaddr) < 0)
		return(-1);
	return(0);
#endif

#ifdef  RD5			/* ipc for Sys V */
	if (shmdt(shmaddr) < 0) {
		log("shmdt: errno: %d\n", errno);
		return(-1);
	}
	return(0);
#endif

}

/*
 * RD_SHMDEL			This procedure removes the shared memory
 *				segment from the system.  Never to be found
 *				again.  This is called ONLY by the process
 *				that created the shared memory segment.
 *
 *	Entry:
 *		shmid		Desciptor of the shared memory
 *
 *	Returns:
 *		Nothing
 *
 */
rd_shmdel(shmid)
#ifdef	RD3                 /* XENIX Sys III */
char *shmid;			/* Xenix Sys III IPC uses char ptr. */
#endif

#ifdef  RD5			/* ipc for Sys V */
int	shmid;			/* Unix Sys V IPC uses integer. */
#endif
{
	if (!rd_flag)
		return;

#ifdef	RD3                 /* XENIX Sys III */
	if (sdfree(shmid) < 0)
		log("rd_shmdel: couldn't release shared memory.\n");
	sprintf(RDshmpath, shm_template, getpid());
	unlink(RDshmpath);
#endif

#ifdef  RD5			/* ipc for Sys V */
	/* under ULTRIX 3.1 the following returns -1 and sets errno */
	/* to EINVAL even though it works! */
	if (shmctl(shmid, IPC_RMID, 0L) < 0)
		log("rd_shmdel: shmctl errno: %d\n", errno);
#endif
}
