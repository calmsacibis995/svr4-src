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

#ident	"@(#)x286emul:shm.c	1.1"

/*
 * shared memory
 */
#include "vars.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>

/* char * shmat();          why doesn't shm.h do this? */

#define SHMAT   0
#define SHMCTL  1
#define SHMDT   2
#define SHMGET  3

/*
 * shmget() has only argument conversion, so it is dispatched directly
 * from the xenix sysent table
 */

Shmctl( shmid, cmd, buf2 )
	int shmid, cmd;
	char * buf2;			/* 286 version of struct shmid_ds */
{
	int rv;
	struct	{
		struct	ipc_perm shm_perm;	/* operation perms struct */
		int	shm_segsz;		/* segment size */
		ushort	shm_ptbl;		/* addr of sd segment */
		ushort	shm_lpid;		/* pid of last shared mem op */
		ushort	shm_cpid;		/* creator pid */
		ushort	shm_nattch;		/* current # attached */
		ushort	shm_cnattch;		/* in-core # attached */
		time_t	shm_atime;		/* last attach time */
		time_t	shm_dtime;		/* last detach time */
		time_t	shm_ctime;		/* last change time */
	} shmid_x386;			/* 386 XENIX version of shmid_ds */

	if (buf2 == BAD_ADDR) {
		errno = EFAULT;
		return -1;
	}

	shmid &= 0xffff;	/* guarantee unsigned short */

	/*
	 * Because we've told the MP kernel that we're an x.out binary,
	 * we have to pass a 386 Xenix shmid_ds struct (which is different,
	 * of course, from the 286 Xenix and 386 Unix structs).
	 */

	if (cmd == IPC_SET) {
		shmid_x386.shm_perm = *(struct ipc_perm *)buf2;
		shmid_x386.shm_segsz = *(ushort *)(buf2+16);
			/* copy tail of struct, starting with shm_ptbl */
		copymem( buf2+18, (char *)&shmid_x386.shm_ptbl, 10 );
		copymem( buf2+28, (char *)&shmid_x386.shm_atime, 12 );
	}

	rv = shmctl( shmid, cmd, &shmid_x386 );

	if (cmd == IPC_STAT) {
		*(struct ipc_perm *)buf2 = shmid_x386.shm_perm;
		*(ushort *)(buf2+16) = shmid_x386.shm_segsz;
			/* copy tail of struct, starting with shm_ptbl */
		copymem( (char *)&shmid_x386.shm_ptbl, buf2+18, 10 );
		copymem( (char *)&shmid_x386.shm_atime, buf2+28, 12 );
	}

	return rv;
}

/*
 * Segmap contains the 286 selectors for those areas of 386 address space
 * that have shared memory segments attached to them.
 */
short	segmap[ MAXSHMSEGS ]; 

/*
 * Idmap has entries that correspond to those of segmap and indicate the
 * shm id value associated with the segment.  It is necessary for the
 * emulator to record these because Unix allows multiple attaches of a
 * given segmentwhile Xenix allows but one.
 */
unsigned idmap[ MAXSHMSEGS ];

Shmat( shmid, seloff, shmflg )
	int shmid;
	int seloff;
	int shmflg;
{
	int sel, off;
	int slot;
	int segno;			/* number of segment */
	char *  shmaddr;
	int 	shmsize;		/* size of segment */

	sel = SEL(seloff);
	off = OFF(seloff);

	shmid &= 0xffff;		/* guarantee unsigned short */

	/*
	 * Locate a place in 386 address space to stick segment.  While
	 * we're at it, be sure that this id isn't already attached.
	 */
	for ( slot = MAXSHMSEGS, segno = 0; segno < MAXSHMSEGS; segno++ ) {
		if ( segmap[segno] == 0 ) {
			/* open slot, take it if we need it */
			if (slot == MAXSHMSEGS) {
				slot = segno;
			}
		} else {
			/* besure we aren't trying to use an extant id */
			if (idmap[segno] == shmid) {
				errno = EINVAL;
				return -1;
			}
		}
	}
	if ( slot >= MAXSHMSEGS  ) {
		errno = ENOMEM;
		return -1;
	}
	/*
	 * Find out where the segment should go in 286 address space
	 */
	if ( sel != 0 || off != 0 ) {
		if ( shmflg & SHM_RND ) {
			off = 0;
		} else if ( off ) {	/* must be at the start of a seg */
			errno = EINVAL;
			return -1;
		}
		segno = SELTOIDX(sel);
		if ( segno >= Numdsegs ) {
			if ( moredsegs( segno - Numdsegs + 1 ) == 0 ) {
				errno = ENOMEM;
				return -1;
			}
			/* make sure segno is not already used */
		} else if ((Dsegs[segno].base != BAD_ADDR) &&
			    			(Dsegs[segno].lsize != 0)) {
			errno = EINVAL;
			return -1;
		}
	} else {
		/*
		 * use first available data segment
		 */
		segno = nextfreedseg(1,ANYWHERE);
	}
	/*
	 * Attach the shared memory segment to the emulator at address
	 * SHMSEG(slot)
	 */
	shmaddr = shmat( shmid, SHMSEG(slot), shmflg );
	if ( shmaddr != SHMSEG(slot) ) {
		int saverr = errno;

		if ( (unsigned long)shmaddr != (unsigned long)-1 ) {
			shmdt(shmaddr);
		}
		Nodeath--;
		errno = saverr;
		return -1;
	}
	segmap[slot] = IDXTOSEL(segno);
	shmsize = getsize( shmid );
	Nodeath++;
	if ( ! setsegdscr( segmap[slot], shmaddr, shmsize, shmsize, (2<<16)|1 ) ) {
		shmdt( shmaddr );
		segmap[slot] = 0;
		Nodeath--;
		errno = EINVAL;
		return -1;
	}
	Nodeath--;
	idmap[slot] = shmid;
	return MAKEPTR(segmap[slot], 0);
}
	
Shmdt( off, sel )
	int sel, off;
{
	int i;
	int rv;

	if (off) {
		errno = EINVAL;
		return -1;
	}
	for ( i = 0; i < MAXSHMSEGS; i++ )
		if ( segmap[i] == sel )
			break;
	if ( i >= MAXSHMSEGS ) {
		errno = EINVAL;
		return -1;
	}
	rv = shmdt( SHMSEG(i) );
	if ( rv < 0 ) {
		return rv;
	}
	setsegdscr( segmap[i], 0, 0, 0, 2 );
	Dsegs[SELTOIDX(sel)].base = BAD_ADDR;
	segmap[i] = 0;
	return 0;
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
	if ( buf.shm_segsz > NBPS )
		return NBPS;
	return buf.shm_segsz;
}
