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

#ident	"@(#)x286emul:miscsys.c	1.1"

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>

#include <sys/sysi86.h>
#include <sys/utsname.h>

#include <sys/ustat.h>
#include "vars.h"
#include "sysent.h"

extern int errno;

/*
 * This file contains miscellaneous special system call interface routines
 */

Fork()
{
	int rv;

	if ((rv = fork()) != -1) {
		if (!rv) {
			/* For child, set %ax to parent's PID, %bx to zero */
			rv = My_PID & 0xffff;
			My_PID = getpid();	/* get new PID for emulator */
		} else {
			rv &= 0xffff;	/* be sure PID only takes 16 bits */
			rv |= 0x10000;	/* parent must have nonzero %bx */
		}
	}
	return rv;
}

Getpid()
{
	extern int getppid();

	return (My_PID & 0xffff) | (getppid() << 16);
}

Getuid() {
	extern int getuid(), geteuid();

	return (getuid() & 0xffff) | (geteuid() << 16);
}

Getgid() {
	extern int getgid(), getegid();

	return (getgid() & 0xffff) | (getegid() << 16);
}

Uname(name)
	char *name;
{
	int rv;
	struct xutsname xun;

	if (name == BAD_ADDR) {
		errno = EFAULT;
		return -1;
	}

	rv = uname((struct utsname *)&xun);


	if ((BADVISE_flags & SI86B_PRE_SV) != SI86B_PRE_SV)
		copymem(xun.sysname, name, sizeof(struct xutsname));
	else {
		/* Pre-System V binaries have uts3name struct */
		copymem( xun.sysname, name, 36 );
		copymem( (char *) &xun.sysorigin, name+36, 8);
	}

	return rv;
}

Ustat(dev, buf)
	int dev;
	char *buf;
{
	int rv;
	struct ustat us;

	if (buf == BAD_ADDR) {
		errno = EFAULT;
		return -1;
	}

	rv = ustat(dev, &us);
	*(long *)buf = us.f_tfree;
	*(short *)(buf+4) = us.f_tinode;
	copymem( us.f_fname, buf+6, 12 );

	return rv;
}

Ulimit(cmd, arg)
	int cmd;
	long arg;
{
	switch (cmd) {

	/*
	 * XENIX extension:  UL_GTEXTOFF
	 * Return total text size up to pointer passed as arg.
	 * For small & compact models, this is just offset part; for
	 * large text models, we must count size of preceeding segments.
	 */
	case 64:
		if (!Ltext) {
			return (arg & 0xffff);
		} else {
			int i, size = 0;
			int seg = (unsigned short)(arg >> 16);
			
			for ( i = 0; i < Xnumsegs; i++ ) {
				struct xseg * xp;

				if ( (xp = Xs+i)->xs_type == XS_TTEXT) {
					if (xp->xs_seg == seg) {
						return size + (arg & 0xffff);
					} else {
						size += xp->xs_vsize;
					}
				}
			}
			errno = EINVAL;
			return -1;
		}

	/*
	 * Special handling for ulimit(3,--)--get memory limits.  If we
	 * are small data, we are limited to 64k.  If we are large data,
	 * we use min(2^29, ulimit(3,--)).  (2^29 is max 286 user
	 * addressability.)  The kernel returns us the max proc size, and
	 * we must convert that into a maximum 286 break value, that takes
	 * into account the space we are using up.
	 */
#define TWO29	0x20000000	/* max 286 data addressability */

	case 3:
		if (Ldata) {
			int i, j;

				/* get the real limit */
			i = ulimit(cmd, arg);

				/* check for max 286 addressability */
			if (i > TWO29) i = TWO29;

			/*
			 * Turn size into a 286 address.
			 * The address will be one less than the size;
			 * if this crosses a segment boundary, we will
			 * have to reduce the selector by one.  We will
			 * reserve two 64k segments for the emulator.
			 */
			j = OFF(i) - 1;
			i = SEL(i) - 2;
			if ( j < 0 ) {
				j = 0xffff;
				--i;
			}
			return MAKEPTR(IDXTOSEL(i), j);
		} else {
			return 0xffff;
		}

	default:
		return ulimit(cmd, arg);
	}
}

#include <sys/stat.h>

struct stat sb386;

Stat(name,sbp286)
	char * name;
	char * sbp286;
{

	int ret;

	if (sbp286 == BAD_ADDR) {
		errno = EFAULT;
		return -1;
	}

	if ( (ret = stat( name, &sb386 )) < 0 )
		return -1;

	copymem( &sb386, sbp286, 14 );           /* 7 shorts */
						 /* 386 pads to 8 shorts */
	copymem( (char *)&sb386 + 16, sbp286+14, 16 );   /* 4 longs */

	return ret;
}
Fstat(fd,sbp286)
	int fd;
	char * sbp286;
{

	int ret;

	if (sbp286 == BAD_ADDR) {
		errno = EFAULT;
		return -1;
	}

	if ( (ret = fstat( fd, &sb386 )) < 0 )
		return -1;

	copymem( &sb386, sbp286, 14 );           /* 7 shorts */
						 /* 386 pads to 8 shorts */
	copymem( (char *)&sb386 + 16, sbp286+14, 16 );   /* 4 longs */

	return ret;
}

Wait()
{
	int statbuf;
	int rv;

	rv = wait(&statbuf);

	/* wait routine expects status in %dx */
	if (rv != -1)
		rv |= (statbuf << 16);
	return rv;
}

Pipe()
{
	int fildes[2];
	int rv;

	rv = pipe(fildes);

	/* file descriptors returned in %ax:%dx */
	if (rv == 0)
		rv = (fildes[1] << 16) | fildes[0];
	return rv;
}

Fcntl(ap)
	short *ap;      /* pointer to 286 args */
{
	int arg;        /* 386 arg */
	struct flock flock;
	struct flock *fp;

	/* convert the arg as appropriate for the cmd */
	switch (ap[1]) {

	default:
		errno = EINVAL;
		return -1;

	/* no arg */
	case F_GETFD:
	case F_GETFL:
		arg = 0;
		break;

	/* int arg */
	case F_DUPFD:
	case F_SETFD:
	case F_SETFL:
		arg = ap[2];
		break;

	/* pointer arg, convert the structure */
	case F_GETLK:
	case F_SETLK:
	case F_SETLKW: {
		unsigned short T;
		int retval;

		fp = (struct flock *)cvtptr(*(int *)&ap[2]);
		if (fp == (struct flock *)BAD_ADDR) {
			errno = EFAULT;
			return -1;
		}
		copymem( fp, &flock, 16 );

		T = flock.l_pid;
		flock.l_pid = flock.l_sysid;	/* Xenix switches these fields*/
		flock.l_sysid = T;

		retval = fcntl( ap[0], ap[1], &flock );

		T = flock.l_pid;		/* Xenix switches these fields*/
		flock.l_pid = flock.l_sysid;
		flock.l_sysid = T;

		copymem( &flock, fp, 16 );
		return retval;
	}
	} /* esac */

	return fcntl(ap[0], ap[1], arg);
}

Stime(t)
	long t;
{
	return stime(&t);
}

Setpgrp(set)
	short set;
{
	if (set) {
		return setpgrp();
	} else {
		return getpgrp();
	}
}

StackGrow(stackFrame)
	char *stackFrame;
{
	int sp286 = (stackFrame - TheStack) + (CS << 1);

#ifdef DEBUG
	fprintf( dbgfd,"stackgrow:  grow sp to 0x%x\n",sp286);
#endif
	if (sp286 < (Stackbase - Stacksize)) {
		errno = EINVAL;
		return -1;
	} else if (sp286 > Stackbase) {
		errno =  ENOMEM;
		return -1;
	} else {
		return sp286;
	}
}

/*
 * dup and dup2 use the same system call.  Bit 0x40 on first file
 * descriptor tells if there is a second aregument or not.
 */
Dup( fd1, fd2 ) {
	if ( fd1 & 0x40 ) 	/* dup2 call */
		return dup2( fd1 & ~0x40, fd2 );
	else
		return dup( fd1 );
}

Execseg(addr, size)
	long addr;
	unsigned int size;
{
	int i, s;

	i = SELTOIDX(SEL(addr));
	if (i > Numdsegs || (Dsegs[i].type&0xffff) != DATA ||
			Dsegs[i].base == BAD_ADDR) {
		errno = EINVAL;
		return -1;
	} else if (OFF(addr) + size > NBPS || !(s = nextfreedseg(1,ANYWHERE))) {
		errno = ENOMEM;
		return -1;
	}
	s = IDXTOSEL(s);
	setsegdscr( s, Dsegs[i].base+OFF(addr), size,
			size, TEXT );
	return MAKEPTR(s, 0);
}

Unexecseg(addr)
	long addr;
{
	int i;

	if ( (i = SEL(addr)) <= Stacksel || (i = SELTOIDX(i)) > Numdsegs ||
			Dsegs[i].type != TEXT ||
			Dsegs[i].base == BAD_ADDR) {
		errno = EINVAL;
		return -1;
	}

	setsegdscr(IDXTOSEL(i), 0, 0, 0, 2);
	Dsegs[i].base = BAD_ADDR;
	return 0;
}
