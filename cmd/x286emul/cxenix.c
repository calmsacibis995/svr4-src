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

#ident	"@(#)x286emul:cxenix.c	1.1"

/* 
 * These are subcommand of the cxenix system call.  They are dispatched
 * via the table Xsysent[], in sysent.c.  Argument conversion is specified
 * in the table.
 */

#include "vars.h"
#include "h/brk.h"
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/fcntl.h>
#include <sys/sysi86.h>

Brkctl( cmd, change, seloff )
	int cmd;
	long change, seloff;
{
	int sel;
	int i;

	/*
	 * The Xenix libraries are pretty weird here.  In small model, the
	 * selector shows up in the low word.  In large model, a large pointer
	 * is passed, so the selector is in the high word.
	 */
	if (Ldata) {
		sel = SEL(seloff);
	} else {
		sel = OFF(seloff);
	}

#ifdef DEBUG
	fprintf(dbgfd, "cmd=%d, change=%d (0x%x), seloff=0x%x\n", cmd,change,change,seloff );
#endif

	/*
	 * It's illegal to ask for more memory than a full segment's worth.
	 */
	if (change > NBPS) {
		errno = EINVAL;
		return -1;
	}

	switch ( cmd ) {
	case BR_ARGSEG:		/* change size of specified segment */
	{
		int index;
		char * oldbase, * newbase;
		unsigned long ret;

		index = SELTOIDX(sel);

		if ( index >= Numdsegs ) {	/* this segment not mapped */
			errno = EINVAL;
			return -1;
		}

		if ( !change ) {
			return MAKEPTR(sel, Dsegs[index].lsize);
		} else if ( change < 0 ) {
			int oldsize;

			change = -change;
			oldsize = Dsegs[index].lsize;

				/* can't give back more than is in the seg */
			if ( change > oldsize ) {
				errno = EINVAL;
				return -1;
			}

			change = oldsize - change;

				/* don't let user give back part of stack */
			if ( (index == SELTOIDX(Stacksel)) &&
				(change < (Stackbase + Stacksize)) ) {
				errno = EINVAL;
				return -1;
			}

			setsegdscr( sel, Dsegs[index].base, change,
				Dsegs[index].psize, 1 );
			return MAKEPTR(sel, change);

		} else {	/* change > 0 */
			long oldsize;

			oldsize = Dsegs[index].lsize;
			if ( oldsize + change > NBPS ) {
				errno = EINVAL;
				return -1;
			}
			GrowSeg( index, oldsize + change );
			setsegdscr( sel, Dsegs[index].base, oldsize + change,
				Dsegs[index].psize, 1 );
			return MAKEPTR(sel, oldsize);
		}
	}

	case BR_NEWSEG:		/* new segment */
newseg:
	{
		int i;
		if ( !(i = nextfreedseg(1, ANYWHERE)) ) {
			errno = ENOMEM;
			return -1;
		}
		if (change == 0) {
			/*
			 * Don't want to call setsegdscr(), because it
			 * calls the kernel sysi86(), which we don't want
			 * to do.  Instead, we duplicate the portions of
			 * setsegdscr() that we DO want to happen here.
			 *
			 * We go through all this hassle because in XENIX
			 * the brkctl() routine will allocate a selector
			 * for a new segment of size 0, but it is not
			 * valid to use the selector (because we return
			 * from brkseg() early if oldsize == 0 == newsize).
			 */

			Dsegs[i].base = (char *)getmem(NBPS);
			Dsegs[i].lsize = 0;
			Dsegs[i].psize = NBPS;
			Dsegs[i].type = 1;
		}
		else
			setsegdscr( IDXTOSEL(i), getmem(NBPS), 
					(unsigned long) change, NBPS, 1 );
		return MAKEPTR(IDXTOSEL(i), 0);
	}

	case BR_IMPSEG:		/* implied segment - last data segment */
	{
		int i;
		int os;

		i = ldsegidx();
			/* avoid shared memory/data segments */
		if ( (Dsegs[i].type & 0xffff) > 1) {
			/*
		 	 * Skip past shared data/memory segments and
			 * mapped-in video display memory segments.
		 	 */
			os = 0;
			i++;
			if (change != 0) {
				if ( i != nextfreedseg(1, ANYWHERE) ) {
					errno = ENOMEM;
					return -1;
				}
				setsegdscr( IDXTOSEL(i), getmem(NBPS), 
								0L, NBPS, 1); 
			}
		}
		else	
			os = Dsegs[i].lsize;

		if (!change) {				/* get break value */
			if (os == NBPS) {
				i++;
				os = 0;
			}
			return MAKEPTR(IDXTOSEL(i), os);

		} else if ( change > 0 ) {		/* growing segment */
			if ( os + change > NBPS ) {
				/*
				 * This segment is too large already. Create new
				 * segments instead
				 */
				goto newseg;
			}
			GrowSeg( i, os + change );
			setsegdscr( IDXTOSEL(i), Dsegs[i].base, os + change,
				Dsegs[i].psize, 1 );
			return MAKEPTR(IDXTOSEL(i), os);

		} else { /* change < 0 */		/* shrinking segment */
			int j, tsize;

			change = -change;
			/*
			 * see how much we can give away
			 */
			tsize = 0;
			for ( j = i; j >= 0 && tsize < change; j-- ) {
				switch( Dsegs[i].type & 0xffff ) {
				case 1: /* data */
					tsize += Dsegs[j].lsize;
					break;
				case 0:	/* text */
					errno = EINVAL;
					return -1;
				default: /* other seg types */
					break;
				}
			}

			/*
			 * give away segments
			 */
			while ( change >= Dsegs[i].lsize ) {
				while (Dsegs[i].type != 1) --i;
#ifdef TRACE
				if (systrace)
					fprintf(dbgfd,
					"seg 0x%x: give away %d bytes of %d\n",
					i*8+7,change,Dsegs[i].lsize);
#endif
				change -= Dsegs[i].lsize;
				setsegdscr( IDXTOSEL(i), 0, 0, 0, 2 );
				i--;
				if ( change == 0 )
					break;
			}
			/*
			 * shrink the last segment, if needed
			 */
			if ( change > 0 ) {
				setsegdscr(IDXTOSEL(i), Dsegs[i].base,
					Dsegs[i].lsize-change,
					Dsegs[i].psize, 1);
			}
			return MAKEPTR(IDXTOSEL(i), Dsegs[i].lsize);
		}
		break;
	}

	case BR_FREESEG:	/* free the specified segment */
	{
		int nsegs, startseg, size, i;

		startseg = SELTOIDX(sel);
		nsegs = change ? (change + NBPS - 1) / NBPS : 1;
		if ( (startseg + (nsegs-1) > ldsegidx()) ) {
			errno = EINVAL;
			return -1;
		}

		/* loop through segments to be freed checking for validity */
		size = 0;
		for (i = startseg; i < startseg + nsegs; i++) {
				/* valid segment */
			if (Dsegs[i].base == BAD_ADDR ||
					/* not shared or mapped */
					(Dsegs[i].type >> 16) > 1) {
				errno = EINVAL;
				return -1;
			}
			size += Dsegs[i].lsize;
		}
		if (size != change) {
			errno = EINVAL;
			return -1;
		}
		/* now do the actual freeing of the segments */
		for (i = startseg; i < startseg + nsegs; i++) {
			if (Dsegs[i].psize) {
				free(Dsegs[i].base);
			}
			setsegdscr( IDXTOSEL(i), 0, 0, 0, 2 );
		}

		return MAKEPTR(sel, 0);
		break;
	}

	default:
		errno = EINVAL;
		return -1;
	}
}


/*
 * find the last data segment
 */
ldsegidx() {
	int i;

	for ( i = Numdsegs-1; i >= 0; i-- )
		if ( Dsegs[i].base != BAD_ADDR && (Dsegs[i].type & 0xffff) )
			break;
	if ( i < 0 ) {
		emprintf( "internal error: lost data segments\n" );
		exit(1);
	}

	return i;
}
				
GrowSeg( index, newsize )
	int index;
	long newsize;
{
	if ( newsize > Dsegs[index].psize ) {
		/*
		 * segment not big enough
		 */
		char * newbase;
		char * oldbase;

		newbase = getmem( NBPS );
		oldbase = Dsegs[index].base;
		copymem( oldbase, newbase, Dsegs[index].lsize );
		Dsegs[index].base = newbase;
		Dsegs[index].psize = NBPS;
		if ( !ISMAPPED(oldbase) )
			free( oldbase );
	} else {
		char *cp, *limit;

		limit = Dsegs[index].base + newsize;
		for ( cp = Dsegs[index].base + Dsegs[index].lsize;
							cp < limit; cp++) {
			*cp = 0;
		}
	}
	Dsegs[index].lsize = newsize;
}

Locking( fd, fcn, size )
	int fd, fcn;
	long size;
{
	struct flock l;		/* arg for use in fcntl() */
	int rv;
	struct { short s1, s2; long l1; } x; /* stack format for Fcntl() */

	switch(fcn) {
	case 2:		/* Xenix LK_NBLK */
		fcn = 20;	/* replace with new value */
		/* FALLTHRU */

	default:	/* real locking() call */
		return locking(fd, fcn, size);

/*
 * Old locking() used to do double duty as fcntl;  the Xenix
 * library translated fcntl()'s into equivalent locking() commands
 * that were not normally visible to users.  However, the MP doesn't
 * support this, so we must translate back to fcntl()!
 */

	/*
	 * The following three cases build 286 fcntl system calls and pass
	 * them directly on to Fcntl() in the emulator to handle.  This is
	 * because the 286 program supplies its own struct flock and we need
	 * only convert the commands to the correct values.
	 */
	case 5:		/* Xenix LK_GETLK */
		x.s2 = F_GETLK;
		goto doFcntl;

	case 6:		/* Xenix LK_SETLK */
		x.s2 = F_SETLK;
		goto doFcntl;

	case 7:		/* Xenix LK_SETLKW */
		x.s2 = F_SETLKW;

doFcntl:
		x.s1 = fd;	/* file descriptor */
		x.l1 = size;	/* actually, 286 struct flock * */

		rv = Fcntl(&x);

		return rv;

	/*
	 * The following call is a conversion to a real fcntl() call.
	 * We use the info supplied in the locking() call to build a 386
	 * struct flock and we do the call directly.  This code is nearly
	 * identical to the 386 lockf() library routine.
	 */
	case 8:		/* Xenix LK_TESTLK */

			/* build correct struct flock for request */
		l.l_whence = 1;	/* make the request relative to current pos */
		if (size < 0) {
			l.l_start = size;
			l.l_len = -size;
		} else {
			l.l_start = 0L;
			l.l_len = size;
		}
		l.l_type = F_WRLCK;

		rv = fcntl(fd, F_GETLK, &l);

		if (rv >= 0) {
			if (l.l_type == F_UNLCK || l.l_type == 0) {
				return 0;
			} else {
				/*
				 * lock(ba_os) applications usage
				 * svid vol 1 p 100
				 */
				errno = EAGAIN;
				rv = -1;
			}
		} else {
			switch(errno) {
			case EMFILE:
			case ENOSPC:
			case ENOLCK:
				/*
				 * A deadlock error is given if we run out
				 * of resources, in compliance with
				 * /usr/group standards.
				 */
				errno = EDEADLK;
				break;
			}
		}
		return rv;
	}
}

/*
 * Xenix shared data
 *
 * When the 286 program wants to create a shared data segment, the emulator
 * will create one.
 *
 * When the 286 program attaches the shared data segment to its address
 * space, the action the emulator takes depends on the model of the 286
 * program.
 *
 * If it is a small model program, the emulator must figure out where
 * the segment goes in the address space of the data segment of the
 * program.  The emulator then tells this to the kernel.  The kernel
 * will see to copying the contents of the shared data segment into
 * the 286 program when it starts execution, and copying any changes
 * back to the shared segment when the 286 program loses the CPU.
 *
 * For a larger model program, the emulator just maps the shared data
 * segment into the LDT where the 286 program expects it to be.
 */

/*
 * sdmap[] is an array that tells what shared data segments are where in
 * the address space of the 286.
 *
 * AddToMap( sel, off, a3 ) inserts an entry into the table for a shared
 * data segment at 286 address a3, which is mapped to 286 address sel:off.
 *
 * RemoveFromTable( sel, off, a3 ) removes from the table any entries that
 * were inserted by AddToMap( sel, off, a3 ).  Any of the arguments may
 * be zero to indicate a "don't care" field.  For example, to remove from
 * the table the shared data segment at 386 address a3, the call
 * RemoveFromMap( 0, 0, a3 ) would work.
 */
#define	SDMAPMAX 10

struct sdmap {
	unsigned int	sel;			/* 286 selector for segment */
	unsigned int	off;			/* 286 offset for segment */
	char *		a3;			/* 386 address of segment */
} sdmap[SDMAPMAX];

AddToMap( sel, off, a3 )
	int sel, off;
	char * a3;
{
	int i;
	for ( i = 0; i < SDMAPMAX; i++ )
		if ( sdmap[i].sel == 0 ) {
			sdmap[i].sel = sel;
			sdmap[i].off = off;
			sdmap[i].a3  = a3;
			return 1;
		}
	return 0;
}

RemoveFromTable( sel, off, a3 )
	int sel, off;
	char * a3;
{
	int i;
	for ( i = 0; i < SDMAPMAX; i++ ) {
		if ( (sel == 0 || sdmap[i].sel == sel)
		  && (off == 0 || sdmap[i].off == off)
		  && (a3  == 0 || sdmap[i].a3  == a3)
		) {
			sdmap[i].sel = 0;
			sdmap[i].off = 0;
			sdmap[i].a3  = 0;
		}
	}
}

FindInTable( Psel, Poff, Pa3 )
	int * Psel, *Poff;
	char ** Pa3;
{
	int i;
	for ( i = 0; i < SDMAPMAX; i++ ) {
		if ( (Psel == 0 || *Psel == 0 || sdmap[i].sel == *Psel)
		  && (Poff == 0 || *Poff == 0 || sdmap[i].off == *Poff)
		  && (Pa3  == 0 || *Pa3  == 0 || sdmap[i].a3  == *Pa3)
		) {
			if ( Psel) *Psel = sdmap[i].sel;
			if ( Poff) *Poff = sdmap[i].off;
			if ( Pa3 ) *Pa3  = sdmap[i].a3;
			break;
		}
	}
}

/*
 * Have already used SI86BADVISE to turn on sd context switch copying.
 * This variable incidentally counts the number of shared data segments,
 * in small model ONLY.
 */
static int SDSWTCH_on;

/*
 * SDget handles an sdget() call from the 286 program
 */
SDget( path, flags, size, mode )
	char * path;				/* path name for segment */
	int    flags;				/* flags */
	long   size;				/* size of segment */
	int    mode;				/* mode */
{
	char * a3;				/* 386 address of segment */
	char * sdget();

	size += 1;	/* compensate for 286 runtime decrement of size */
	a3 = sdget( path, flags, size, mode );

#ifdef DEBUG
	fprintf(dbgfd, "%d sdget(\"%s\",0x%x,0x%x(%d),0%o) == %d (0x%x)\n", getpid(),
	path, flags, size, size, mode, a3, a3 );
#endif

	if ( (long)a3 == -1 )
		return -1;

#define	SD_CREATE	0x02
	if ( ! (flags & SD_CREATE) ) {
		/*
		 * User is not creating new segment, so we need to find out
		 * the size and the mode information from an already
		 * existing segment
		 */
		struct xsdbuf x;

		x.xsd_cmd = SI86SHR_SZ;
		x.xsd_386vaddr = a3;
		if ( sysi86(SI86SHRGN,&x) == -1 ) {
			/* unattach segment */
			return -1;
		}
		size = x.xsd_un.xsd_size;
#ifdef DEBUG
	fprintf(dbgfd, "size is %d\n", size );
#endif
	}

	/*
	 * find a place to put the segment in the 286 address space
	 */
	if ( Ldata ) {		/* large model, just choose next free seg */
		int i;

		i = nextfreedseg(1, ANYWHERE);
		AddToMap( IDXTOSEL(i), 0, a3);
		setsegdscr( IDXTOSEL(i), a3, size, size, (2<<16)|1 );
		return MAKEPTR(IDXTOSEL(i), 0);
	} else {			/* small model */
		int i;
		int os,			/* old size of data segment */
		    ns;			/* new size of data segment */
		char *touchp;
		int dmy;
		struct xsdbuf x;

		i = SELTOIDX(Stacksel);
		os = Dsegs[i].lsize;
		ns = os + size;
		if ( ns >= NBPS ) {
			/* no room */
			errno = ENOMEM;
			return -1;
		}
		if ( ns > Dsegs[i].psize )
			GrowSeg( i, ns );
		setsegdscr( IDXTOSEL(i), Dsegs[i].base, ns, Dsegs[i].psize, 1 );
		x.xsd_cmd = SI86SHR_CP;
		x.xsd_386vaddr = a3;
		x.xsd_un.xsd_286vaddr = Dsegs[i].base + os;

			/* force a copyout of each shared data segment */
		if (SDSWTCH_on) {
			BADVISE_flags &= ~SI86B_XSDSWTCH;
			sysi86(SI86BADVISE,SI86B_SET|BADVISE_flags);
		}

		/*
		 * Touch each page in the shared data segment so that it
		 * will fault into memory.  The kernel can hang if we don't
		 * do this because it could try to context switch and cause
		 * an infinite recursion.
		 */
		for (touchp = a3; touchp < a3 + size; touchp += NBPC) {
			dmy += *touchp;
		}
		dmy += *(a3 + size - 1);	/* be sure to touch last page */

			/* add the new shared segment */
		sysi86(SI86SHRGN,&x);
		AddToMap( IDXTOSEL(i), os, a3 );

			/* force a copyin of each shared data segment */
		SDSWTCH_on++;
		BADVISE_flags |= SI86B_XSDSWTCH;
		sysi86(SI86BADVISE,SI86B_SET|BADVISE_flags);
#ifdef DEBUG
	fprintf(dbgfd, "%d segment mapped to 0x%x:%x\n", getpid(), i*8+7, os );
#endif
		return os;
	}
}

SDfree( off, sel )
	int sel, off;
{
	char * a3;

	a3 = 0;
	FindInTable( &sel, (int *)0, &a3 );
	if ( a3 == 0 ) {
		extern Shmdt();
		/*
		 * Maybe it was a shared memory segment?
		 */
		return Shmdt( off, sel );
	}

#ifdef DEBUG
	fprintf(dbgfd, "%d sdfree(0x%x:0x%x\n", getpid(), sel, off );
#endif
	sdfree(a3);

	if ( Ldata ) {
		setsegdscr( sel, 0, 0, 0, (1<<16)|2 );
	} else if (!--SDSWTCH_on) {
		BADVISE_flags &= ~SI86B_XSDSWTCH;
		sysi86(SI86BADVISE,SI86B_SET|BADVISE_flags);
	}

	RemoveFromTable( sel, 0, a3 );

	return 0;
}

SDgetv( seloff )
	int seloff;
{
	int sel, off;
	char * a3;

	off = OFF(seloff);
	sel = SEL(seloff);
	a3 = 0;
	FindInTable( &sel, &off, &a3 );
	if ( a3 == 0 ) {
		errno = EINVAL;
		return -1;
	}

#ifdef DEBUG
	fprintf(dbgfd, "%d sdgetv(0x%x:%x) returns...\n", getpid(), sel, off );
#endif

	sel = sdgetv(a3);

#ifdef DEBUG
	fprintf(dbgfd, "%d ... %d\n", getpid(), sel );
#endif

	return sel;
}

SDwaitv( seloff, vnum )
	int seloff, vnum;
{
	int sel, off;
	char * a3;

	off = OFF(seloff);
	sel = SEL(seloff);
	a3 = Dsegs[SELTOIDX(sel)].base + off;

#ifdef DEBUG
	fprintf(dbgfd, "%d 286 data:0x%x 0x%x\n", getpid(), a3[0], a3[1] );
#endif

	a3 = 0;
	FindInTable( &sel, &off, &a3 );

#ifdef DEBUG
	fprintf(dbgfd, "%d 386 data:0x%x 0x%x\n", getpid(), a3[0], a3[1] );
#endif

	if ( a3 == 0 ) {
		errno = EINVAL;
		return -1;
	}

#ifdef DEBUG
	fprintf(dbgfd, "%d sdwaitv(0x%x:%x,%d) returns...\n", getpid(), sel, off, vnum );
#endif

	sel = sdwaitv(a3,vnum);

#ifdef DEBUG
	fprintf(dbgfd, "%d ... %d\n", getpid(), sel );
#endif

	return sel;
}

SDleave( seloff )
	int seloff;
{
	int sel, off;
	char * a3;

	off = OFF(seloff);
	sel = SEL(seloff);
	a3 = Dsegs[SELTOIDX(sel)].base + off;

#ifdef DEBUG
	fprintf(dbgfd, "%d 286 data:0x%x 0x%x\n", getpid(), a3[0], a3[1] );
#endif

	a3 = 0;
	FindInTable( &sel, &off, &a3 );

#ifdef DEBUG
	fprintf(dbgfd, "%d 386 data:0x%x 0x%x\n", getpid(), a3[0], a3[1] );
#endif

	if ( a3 == 0 ) {
		errno = EINVAL;
#ifdef DEBUG
	fprintf(dbgfd, "%d sdleave failed\n", getpid() );
#endif
		return -1;
	}

	sel = sdleave( a3 );
	return sel;
}

SDenter( seloff , flags )
	int seloff;
	int flags;
{
	char * a3;
	int sel, off;

	off = OFF(seloff);
	sel = SEL(seloff);
	a3 = 0;
	FindInTable( &sel, &off, &a3 );

	if ( a3 == 0 ) {
		errno = EINVAL;
#ifdef DEBUG
	fprintf(dbgfd, "sdenter: failed\n" );
#endif
		return -1;
	}

	return sdenter( a3, flags );
}
