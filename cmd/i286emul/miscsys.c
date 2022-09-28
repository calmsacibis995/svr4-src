/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ident	"@(#)i286emu:miscsys.c	1.1"

#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include "vars.h"
#include <sys/ustat.h>

extern int errno;

/* this file contains miscellaneous special system call interface routines
 */

Fork()
{
	int rv;

	rv = fork();

	/* child must get non-zero %dx */
	if (rv == 0)
		rv = 0x10000;
	return rv;
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

#define makeptr(sel,off) (((sel)<<16)|off)

Exec( ap )
	unsigned short * ap;
{
	return exec_common( ap[0], ap[1], ap[2], ap[3], 0, 0 );
}

Exece( ap )
	unsigned short * ap;
{
	return exec_common( ap[0], ap[1], ap[2], ap[3], ap[4], ap[5] );
}

exec_common( offPath, selPath, offArgv, selArgv, offEnvv, selEnvv )
	unsigned offPath, selPath;      /* the program */
	unsigned offArgv, selArgv;      /* the args */
	unsigned offEnvv, selEnvv;      /* the environment */
{
	char **argv386;                 /* pointers to arg strings */
	char **envv386;                 /* pointers to env strings */
	int    argc;                    /* how many args */
	int    envc;                    /* how many envs */
	char  *prog;                    /* program to exec */
	unsigned short * Argv,          /* pointer to 286 arg pointers */
		       * Envv;          /* pointer to 286 env pointers */
	unsigned short * p;             /* temporary pointer */
	int i;                          /* for counting and indexing */
	int save_errno;                 /* holds errno after bad exec */

	prog = (char *)cvtchkptr(makeptr(selPath,offPath));
	Argv = (unsigned short *)cvtchkptr(makeptr(selArgv,offArgv));
	Envv = (unsigned short *)cvtchkptr(makeptr(selEnvv,offEnvv));

	/*
	 * count argument pointers
	 */
	argc = 0;
	if ( Argv ) {
		p = Argv;
		while ( 1 ) {
			if ( uu_model ) {
				if ( p[0] == 0 && p[1] == 0 )
					break;
				else
					p++;
			} else {
				if ( p[0] == 0 )
					break;
			}
			p++; argc++;
		}
	}

	/*
	 * count environment pointers
	 */
	envc = 0;
	if ( Envv ) {
		p = Envv;
		while ( 1 ) {
			if ( uu_model ) {
				if ( p[0] == 0 && p[1] == 0 )
					break;
				else
					p++;
			} else {
				if ( p[0] == 0 )
					break;
			}
			p++; envc++;
		}
	}

	/*
	 * allocate space for arg pointers and env pointers
	 */
	argv386 = (char **)getmem( argc * 4 + 4 );
	envv386 = (char **)getmem( envc * 4 + 4 );

	/*
	 * set up the arg pointers
	 */
	for ( i = 0; i < argc; i++ ) {
		int ptr286;

		if ( uu_model )
			ptr286 = makeptr(Argv[2*i+1],Argv[2*i]);
		else
			ptr286 = makeptr(firstds*8+7,Argv[i]);
		argv386[i] = cvtchkptr(ptr286);
	}
	argv386[argc] = 0;

	/*
	 * set up the env pointers
	 */
	for ( i = 0; i < envc; i++ ) {
		int ptr286;

		if ( uu_model )
			ptr286 = makeptr(Envv[2*i+1],Envv[2*i]);
		else
			ptr286 = makeptr(firstds*8+7,Envv[i]);
		envv386[i] = cvtchkptr(ptr286);
	}
	envv386[envc] = 0;

	execve( prog, argv386, envv386 );

	save_errno = errno;             /* free may change errno */
	free( argv386 );
	free( envv386 );
	errno = save_errno;
	return -1;
}

#include <sys/stat.h>

struct stat sb386;

Stat(ap)
	unsigned short *ap;        /* pointer to 286 args */
{

	char * name;
	char * sbp286;
	int ret;

	name = cvtptr( (ap[1] << 16) + ap[0] );
	sbp286 = cvtchkptr( (ap[3]<<16) + ap[2] );

	if ( (ret = stat( name, &sb386 )) < 0 ) {
		return -1;
	}

	copymem( &sb386, sbp286, 14 );           /* 7 shorts */
						 /* 386 pads to 8 shorts */
	copymem( (char *)&sb386 + 16, sbp286+14, 16 );   /* 4 longs */

	return ret;
}

Fstat(ap)
	unsigned short *ap;        /* pointer to 286 args */
{

	int fd;
	char * sbp286;
	int ret;

	fd = ap[0];
	sbp286 = cvtchkptr( (ap[2]<<16) + ap[1] );

	if ( (ret = fstat( fd, &sb386 )) < 0 )
		return -1;

	copymem( &sb386, sbp286, 14 );           /* 7 shorts */
						 /* 386 pads to 8 shorts */
	copymem( (char *)&sb386 + 16, sbp286+14, 16 );   /* 4 longs */

	return ret;
}

Stime(ap)
	long *ap;        /* pointer to 286 args */
{
	/* time passed as an arg, not a pointer */
	return stime(ap);
}

Ptrace()
{
	emprintf(
      "ptrace system call not supported in 286 compatibility mode\n");
	exit(1);
}

Setpgrp(ap)
	short *ap;        /* pointer to 286 args */
{
	/* arg specifies set or get pgrp */
	return *ap ? setpgrp() : getpgrp();
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

Utssys(ap)
	unsigned short *ap;        /* pointer to 286 args */
{
	struct ustat u;
	int rval;

	/* 4 th arg specifies uname or ustat */
	switch(ap[3]) {
	case 0: /* uname */
		return uname(cvtptr(*(int *)ap));
	case 2: /* ustat */
		rval = ustat( ap[2], &u );
		if ( rval < 0 )
			return rval;
		copymem( &u, cvtchkptr(*(int *)ap), 18 );
		return rval;
	default:
		errno = EFAULT;
		return -1;
	}
}

Sysi86(ap)
	short *ap;        /* pointer to 286 args */
{
	emprintf( "sysi86() not supported\n");
	exit(1);
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
		emprintf("unknown arg to fcntl(): %x\n", ap[1]);
		exit(1);

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
		copymem( fp, &flock, 16 );
		T = flock.l_pid;
		flock.l_pid = flock.l_sysid;
		flock.l_sysid = T;
		retval = fcntl( ap[0], ap[1], &flock );
		T = flock.l_pid;
		flock.l_pid = flock.l_sysid;
		flock.l_sysid = T;
		copymem( &flock, fp, 16 );
		return retval;
	} /* esac */
	}

	return fcntl(ap[0], ap[1], arg);
}

Sysfs(ap)
	short *ap;        /* pointer to 286 args */
{
	emprintf( "sysfs() not supported\n");
	exit(1);
}

Getuid() {
	return (geteuid() << 16) + getuid();
}

Getgid() {
	return (getegid() << 16) + getgid();
}

Getpid() {
	return (getppid() << 16) + getpid();
}
