/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)i286emu:ipc.c	1.1"

/*
 * deal with shared IPC system calls
 */
#include "vars.h"

/*#include <sys/types.h>*/      /* included in "vars.h" */
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <stdio.h>
#include <errno.h>

/***********************************************************************/
/***********************                       *************************/
/***********************     SHARED MEMORY     *************************/
/***********************                       *************************/
/***********************************************************************/


/* char * shmat();         who doesn't shm.h do this? */

#define SHMAT   0
#define SHMCTL  1
#define SHMDT   2
#define SHMGET  3

#define MAXSEGS 30              /* how many shared memory segs we can deal
				 * with.  should be larger than any 286
				 * proc can have */
char    segmap[ MAXSEGS ];      /* which segs are used */
unsigned short segmap286[ MAXSEGS ];


#define BASE0   ((char *)(0x90000000 - (MAXSEGS << 22)))
#define BASE(i) (BASE0+((i)<<22))


Shmsys( ap )
	unsigned short * ap;
{
	switch ( ap[0] ) {
	case SHMGET:
	{
		int size;               /* size of segment */
		int shmflg;             /* flags */
		key_t key;              /* key */

		key = (ap[2] << 16) | ap[1];
		size = ap[3];
		shmflg = ap[4];
		return shmget( key, size, shmflg );
		break;
	}
	case SHMCTL:
	{
		int shmid;              /* id of segment */
		int cmd;                /* what operation */
		struct shmid_ds buf;
		char * userbuf;
		int retval;

		shmid = ap[1];
		cmd   = ap[2];
		if ( cmd != IPC_RMID ) {
			userbuf = (char *)cvtchkptr( (ap[4]<<16) | ap[3] );
			shmid_ds_2to3( userbuf, &buf );
		}
		retval = shmctl( shmid, cmd, &buf );
		if ( cmd != IPC_RMID )
			shmid_ds_3to2( &buf, userbuf );
		return retval;
	}
	case SHMDT:
	{
		char * shmaddr;
		int  retval;
		int i, remove;

		shmaddr = cvtptr( (ap[2]<<16) | ap[1] );
		retval = shmdt( shmaddr );
		if ( retval < 0 )
			return retval;
		/*
		 * free slot in segmap
		 */
		for ( i = 0; i < MAXSEGS; i++ )
			if ( BASE(i) == shmaddr ) {
				remove = segmap[i];
				segmap[i] = 0;
				break;
			}
		if ( i >= MAXSEGS ) {
			emprintf(  "286 emulator bug: shmdt: lost seg\n" );
			errno = EAGAIN;
			return -1;
		}
		setsegdscr( remove*8+7, 0, 0, 2 );
		return 0;
		break;
	}
	case SHMAT:
	{
		int shmid, shmflg;
		unsigned short sel, off;
		char * shmaddr;
		int slot;               /* which slot in 386 space to use */
		int seg286i;            /* which 286 segment to use */

		shmid = ap[1];
		shmflg = ap[4];
		sel = ap[3]; off = ap[2];

		/*
		 * find a place in 386 address space for seg
		 */
		for ( slot = 0; slot < MAXSEGS; slot++ )
			if ( segmap[slot] == 0 )
				break;
		if ( slot >= MAXSEGS ) {
			errno = ENOMEM;
			return -1;
		}
		/*
		 * find a place in 286 address space for seg
		 */
		if ( sel != 0 || off != 0 ) {
			if ( shmflg & SHM_RND )
				off = 0;
			if ( off != 0 ) {
				errno = EINVAL;
				return -1;
			}
			seg286i = ( sel >> 3 ) & 0x1fff;
		} else {
			for ( seg286i=64/*MAXDSEGS-1*/; seg286i >= 0; --seg286i )
				if ( dsegs[seg286i].size == 0 )
					break;
		}
		if ( seg286i <= lastds ) {
			errno = EINVAL;
			return -1;
		}
		/*
		 * place the segment at 286 segment seg286i, and at 386
		 * virtual address BASE(slot)
		 */
		shmaddr = shmat( shmid, BASE(slot), shmflg );
		if ( shmaddr != BASE(slot) ) {
			if ( shmaddr != (char *)-1 )
				shmdt( shmaddr );
			return -1;
		}
		segmap[slot] = seg286i;
		segmap286[slot] = seg286i;
		nodeath++;
		if ( !setsegdscr( seg286i*8+7, shmaddr, getsize(shmid), 1 )){
			shmdt( shmaddr );
			segmap[slot] = 0;
			segmap286[slot] = 0;
			nodeath--;
			errno = EINVAL;
			return -1;
		}
		nodeath--;
		return ((seg286i*8+7)<<16);
		break;
	}
	default:
		emprintf(  "shmsys: bad arg in switch\n" );
		return -1;
		break;
	}
}

/*
 * return the size of a shared memory segment
 */
getsize( shmid )
{
	struct shmid_ds buf;

	if ( shmctl( shmid, IPC_STAT, &buf) < 0 ) {
		emprintf(  "286 emulator: shmat: getsize fails!\n" );
		return 2;
	}
	if ( buf.shm_segsz > 65536 )
		return 65536;
	return buf.shm_segsz;
}

/***********************************************************************/
/***********************                       *************************/
/***********************        MESSAGES       *************************/
/***********************                       *************************/
/***********************************************************************/

#define MSGGET  0
#define MSGCTL  1
#define MSGRCV  2
#define MSGSND  3

Msgsys( ap )
	unsigned short * ap;
{
	switch ( ap[0] ) {
	case MSGGET:
	{
		key_t key;
		int   msgflg;

		key = (ap[2] << 16) | ap[1];
		msgflg = ap[3];

		return msgget( key, msgflg );
		break;
	}
	case MSGCTL:
	{
		struct msqid_ds buf;
		char * ptr;
		int msqid, cmd;

		msqid = (short)ap[1];
		cmd = (short)ap[2];
		if ( cmd != IPC_RMID )
			ptr = cvtchkptr( (ap[4]<<16) | ap[3] );

		if ( cmd == IPC_SET ) {
			copymem( ptr, &buf, 34 );
			copymem( ptr+34, (char *)&buf + 36, 12 );
		}
		if ( msgctl( msqid, cmd, &buf ) < 0 )
			return -1;
		if ( cmd == IPC_STAT ) {
			copymem( &buf, ptr, 34 );
			copymem( (char *)&buf + 36, ptr+34, 12 );
		}
		return 0;
		break;
	}
	case MSGRCV:
	{
		int msqid, msgsz, msgflg, msgtyp;
		struct msgbuf *msgp;

		msqid = (short)ap[1];
		msgp = (struct msgbuf *)cvtptr( (ap[3]<<16) | ap[2] );
		msgsz = (short)ap[4];
		msgtyp = (ap[6]<<16) | ap[5];
		msgflg = ap[7];

		return msgrcv( msqid, msgp, msgsz, msgtyp, msgflg );
		break;
	}
	case MSGSND:
	{
		int msqid, msgsz, msgflg;
		struct msgbuf *msgp;

		msqid = (short)ap[1];
		msgp = (struct msgbuf *)cvtptr( (ap[3]<<16) | ap[2] );
		msgsz = (short)ap[4];
		msgflg = (short)ap[5];

		return msgsnd( msqid, msgp, msgsz, msgflg );
		break;
	}
	default:
		emprintf(  "msgsys: bad arg in switch\n" );
		return -1;
		break;
	}
}

/***********************************************************************/
/***********************                       *************************/
/***********************       SEMAPHORES      *************************/
/***********************                       *************************/
/***********************************************************************/

#define SEMCTL  0
#define SEMGET  1
#define SEMOP   2

Semsys( ap )
	unsigned short * ap;
{
	switch ( ap[0] ) {
	case SEMGET:
	{
		key_t key;
		int nsems, semflg;

		key = (ap[2] << 16) + ap[1];
		nsems = ap[3]; semflg = ap[4];

		return semget( key, nsems, semflg );
		break;
	}
	case SEMCTL:
	{
		int semid, semnum, cmd;
		int ret;
		struct semid_ds s;
		char *src, *dst;
		union {
			int val;
			char * buf;
			unsigned short * array;
		} arg;

		semid  = ap[1];
		semnum = ap[2];
		cmd    = ap[3];
		/*
		 * set up arg
		 */
		switch ( cmd ) {
		case SETVAL:
			arg.val = ap[4]; break;
		case GETALL:
		case SETALL:
			arg.array = (unsigned short *)
					 cvtchkptr( (ap[5]<<16) | ap[4] );
			break;
		case IPC_SET:
			src = cvtchkptr( (ap[5]<<16) | ap[4] );
			dst = (char *)&s;
			copymem( src, dst, 22 );
			copymem( src+22, dst+24, 8 );
			/*
			 * fall through!
			 */
		case IPC_STAT:
			arg.buf = (char *)&s; break;
		}
		ret = semctl( semid, semnum, cmd, arg.buf );
		if ( ret < 0 )
			return ret;
		if ( cmd == IPC_STAT ) {
			src = (char *)&s;
			dst = cvtchkptr( (ap[5]<<16) | ap[4] );
			copymem( src, dst, 22 );
			copymem( src+24, dst+22, 8 );
		}
		return ret;
		break;
	}
	case SEMOP:
	{
		int semid, nsops;
		char *sops;

		semid = ap[1];
		sops  = cvtchkptr( (ap[3]<<16) | ap[2] );
		nsops = ap[4];

		return semop( semid, (struct sembuf *)sops, nsops );

		break;
	}
	default:
		emprintf(  "semsys: bad arg in switch\n" );
		return -1;
		break;
	}
}

/*
 * convert a 286 shmid_ds to a 386 shmid_ds
 */
shmid_ds_2to3( src, dst )
	char * src, * dst;
{
	copymem( src, dst, sizeof(struct ipc_perm) );
	src += sizeof(struct ipc_perm);
	dst += sizeof(struct ipc_perm);
	*(int *)dst = *(unsigned short *)src;
	copymem( src+4, dst+12, 20 );
}

shmid_ds_3to2( src, dst )
	char * src, * dst;
{
	copymem( src, dst, sizeof(struct ipc_perm) );
	src += sizeof(struct ipc_perm);
	dst += sizeof(struct ipc_perm);
	*(unsigned short *)dst = *(int *)src;
	copymem( src+12, dst+4, 20 );
}
