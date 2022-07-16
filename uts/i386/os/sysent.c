/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-os:sysent.c	1.3.1.2"

#include "sys/param.h"
#include "sys/types.h"
#include "sys/systm.h"

/*
 * This table is the switch used to transfer to the appropriate
 * routine for processing a system call.  Each row contains the
 * number of arguments expected, a switch that tells systrap()
 * in trap.c whether a setjmp() is not necessary, and a pointer
 * to the routine.
 */

int	access(), alarm(), brk(), chdir(), chmod(), chown(), chroot();
int	close(), creat(), dup(), exec(), exece(), fcntl(), fork(), fstat();
int	fsync(), getgid(), getpid(), getuid(), gtime(), gtty(), ioctl();
int	kill(), link(), lock_mem(), lseek(), mknod(), msgsys(), mknod();
int	mount(), msgsys(), nice(), nosys(), open();
int	pipe(), profil(), ptrace(), read(), rename();
int	semsys(), setgid(), setpgrp(), setuid(), shmsys();
int	ssig(), sigprocmask(), sigsuspend(), sigaltstack();
int	sigaction(), sigpending(), setcontext();
int	stat(), stime(), stty(), syssync(), sysacct(), times(), ulimit();
int	getrlimit(), setrlimit();
int	umask(), umount(), unlink(), utime(), utssys(), wait(), write();
int	readv(), writev();

void	pause(), rexit();
int	rfsys();

int	rmdir(), mkdir(), getdents(), statfs(), fstatfs();
int	sysfs(), getmsg(), poll(), putmsg(), sysi86(), uadmin();
int	lstat(), symlink(), readlink();
int	setgroups(), getgroups(), fchdir(), fchown(), fchmod();
int	statvfs(), fstatvfs();

int	async(), async_cancel();
int	ev_evsys(), ev_evtrapret();
int	hrtsys();
int	priocntlsys();
int	waitsys();
int	sigsendsys();
int	mincore(), mmap(), mprotect(), munmap(), vfork();
int	xstat(), lxstat(), fxstat();
int	xmknod();
int	nuname(), lchown();
int	getpmsg(), putpmsg();
int	memcntl();
int	cxenix(), clocal();
int	sysconfig();
int	adjtime();
int	systeminfo();
int	setegid(), seteuid();
int	nfssys();
int	pathconf(), fpathconf();

#ifdef MEGA
int	uexch();
#endif /* MEGA */

struct sysent sysent[] = {
	0, 0, nosys,			/*  0 = indir */
	1, 0, (int(*)())rexit,		/*  1 = exit */
	0, 0, fork,			/*  2 = fork */
	3, SETJUMP|ASYNC|IOSYS, read,	/*  3 = read */
	3, SETJUMP|ASYNC|IOSYS, write,	/*  4 = write */
	3, SETJUMP, open,		/*  5 = open */
	1, SETJUMP, close,		/*  6 = close */
	0, SETJUMP, wait,		/*  7 = wait */
	2, SETJUMP, creat,		/*  8 = creat */
	2, 0, link,			/*  9 = link */
	1, 0, unlink,			/* 10 = unlink */
	2, 0, exec,			/* 11 = exec */
	1, 0, chdir,			/* 12 = chdir */
	0, 0, gtime,			/* 13 = time */
	3, 0, mknod,			/* 14 = mknod */
	2, 0, chmod,			/* 15 = chmod */
	3, 0, chown,			/* 16 = chown */
	1, 0, brk,			/* 17 = brk */
	2, 0, stat,			/* 18 = stat */
	3, 0, lseek,			/* 19 = lseek */
	0, 0, getpid,			/* 20 = getpid */
	6, 0, mount,			/* 21 = mount */
	1, 0, umount,			/* 22 = umount */
	1, 0, setuid,			/* 23 = setuid */
	0, 0, getuid,			/* 24 = getuid */
	1, 0, stime,			/* 25 = stime */
	4, SETJUMP, ptrace,		/* 26 = ptrace */
	1, 0, alarm,			/* 27 = alarm */
	2, 0, fstat,			/* 28 = fstat */
	0, SETJUMP, (int(*)())pause,	/* 29 = pause */
	2, 0, utime,			/* 30 = utime */
	2, SETJUMP, stty,		/* 31 = stty */
	2, SETJUMP, gtty,		/* 32 = gtty */
	2, 0, access,			/* 33 = access */
	1, 0, nice,			/* 34 = nice */
	4, 0, statfs,			/* 35 = statfs */
	0, SETJUMP, syssync,		/* 36 = sync */
	2, 0, kill,			/* 37 = kill */
	4, 0, fstatfs,			/* 38 = fstatfs */
	3, 0, setpgrp,			/* 39 = setpgrp */
	0, SETJUMP, cxenix,		/* 40 = cxenix */
	1, 0, dup,			/* 41 = dup */
	0, SETJUMP, pipe,		/* 42 = pipe */
	1, 0, times,			/* 43 = times */
	4, 0, profil,			/* 44 = prof */
	1, 0, lock_mem,			/* 45 = proc lock */
	1, 0, setgid,			/* 46 = setgid */
	0, 0, getgid,			/* 47 = getgid */
	2, SETJUMP, ssig,		/* 48 = sig */
	6, SETJUMP, msgsys,		/* 49 = IPC message */
	4, 0, sysi86,			/* 50 = i386-specific system call */
	1, 0, sysacct,			/* 51 = turn acct off/on */
	4, SETJUMP, shmsys,            	/* 52 = shared memory */
	5, SETJUMP, semsys,		/* 53 = IPC semaphores */
	3, SETJUMP, ioctl,		/* 54 = ioctl */
	3, SETJUMP, uadmin,		/* 55 = uadmin */
#ifdef MEGA
	3, 0, uexch,			/* 56 = uexch */
#else
	0, 0, nosys,			/* 56 = reserved for exch */
#endif /* MEGA */
	4, 0, utssys,			/* 57 = utssys */
	1, 0, fsync,			/* 58 = fsync */
	3, 0, exece,			/* 59 = exece */
	1, 0, umask,			/* 60 = umask */
	1, 0, chroot,			/* 61 = chroot */
	3, SETJUMP, fcntl,		/* 62 = fcntl */
	2, 0, ulimit,			/* 63 = ulimit */
	/*
	 * The following 6 entries were reserved for the UNIX PC.
	 */
	0, 0, nosys,			/* 64 = unused */
	0, 0, nosys,			/* 65 = unused */
	0, 0, nosys,			/* 66 = unused */
	0, 0, nosys,			/* 67 = file locking call */
	0, 0, nosys,			/* 68 = local system calls */
	0, 0, nosys,			/* 69 = inode open */

	0, 0, nosys,			/* 70 = was advfs */
	0, 0, nosys,			/* 71 = was unadvfs */
	0, 0, nosys,			/* 72 = unused */
	0, 0, nosys,			/* 73 = unused */
	0, 0, nosys,			/* 74 = was rfstart */
	0, 0, nosys,			/* 75 = unused */
	0, 0, nosys,			/* 76 = was rdebug */
	0, 0, nosys,			/* 77 = was rfstop */
	6, SETJUMP, rfsys,		/* 78 = rfsys */
	1, 0, rmdir,			/* 79 = rmdir */
	2, 0, mkdir,			/* 80 = mkdir */
	3, 0, getdents,			/* 81 = getdents */
	0, 0, nosys,			/* 82 = was libattach */
	0, 0, nosys,			/* 83 = was libdetach */
	3, 0, sysfs,			/* 84 = sysfs */
	4, SETJUMP, getmsg,		/* 85 = getmsg */
	4, SETJUMP, putmsg,		/* 86 = putmsg */
	3, SETJUMP, poll,		/* 87 = poll */
	2, 0, lstat,			/* 88 = lstat */
	2, 0, symlink,			/* 89 = symlink */
	3, 0, readlink,			/* 90 = readlink */
	2, 0, setgroups,		/* 91 = setgroups */
	2, 0, getgroups,		/* 92 = getgroups */
	2, 0, fchmod,			/* 93 = fchmod */
	3, 0, fchown,			/* 94 = fchown */
	3, 0, sigprocmask,		/* 95 = sigprocmask */
	1, SETJUMP, sigsuspend,		/* 96 = sigsuspend */
	2, 0, sigaltstack,		/* 97 = sigaltstack  */
	3, 0, sigaction,		/* 98 = sigaction */
	2, 0, sigpending,		/* 99 = sigpending */
	2, 0, setcontext,		/* 100 = setcontext */
	3, 0, ev_evsys,			/* 101 = evsys */
	0, 0, ev_evtrapret,		/* 102 = evtrapret */
	2, 0, statvfs,			/* 103 = statvfs */
	2, 0, fstatvfs,			/* 104 = fstatvfs */
	0, 0, nosys,			/* 105 = reserved */
	2, 0, nfssys,		   	/* 106 = nfssys */
	4, SETJUMP, waitsys,		/* 107 = waitset */
	2, 0, sigsendsys,		/* 108 = sigsendset */
	5, SETJUMP, hrtsys,		/* 109 = hrtsys */
	3, 0, async_cancel,		/* 110 = acancel */
	3, 0, async,			/* 111 = async */
	4, 0, priocntlsys,		/* 112 = priocntlsys */
	2, 0, pathconf,			/* 113 = pathconf */
	3, 0, mincore,			/* 114 = mincore */
	6, 0, mmap,			/* 115 = mmap */
	3, 0, mprotect,			/* 116 = mprotect */
	2, 0, munmap,			/* 117 = munmap */
	2, 0, fpathconf,		/* 118 = fpathconf */
	0, 0, vfork,			/* 119 = vfork */
	1, 0, fchdir,			/* 120 = fchdir */
	3, 0, readv,			/* 121 = readv */
	3, 0, writev,			/* 122 = writev */
	3, 0, xstat,			/* 123 = xstat */
	3, 0, lxstat,			/* 124 = lxstat */
	3, 0, fxstat,			/* 125 = fxstat */
	4, 0, xmknod,			/* 126 = xmknod */
	5, SETJUMP, clocal,		/* 127 = clocal */
	2, 0, setrlimit,		/* 128 = setrlimit */
	2, 0, getrlimit,		/* 129 = getrlimit */
	3, 0, lchown,			/* 130 = lchown */
	6, 0, memcntl,			/* 131 = memcntl */
	5, SETJUMP, getpmsg,		/* 132 = getpmsg */
	5, SETJUMP, putpmsg,		/* 133 = putpmsg */
	2, 0, rename,			/* 134 = rename */
	1, 0, nuname,			/* 135 = nuname */
	1, 0, setegid,			/* 136 = setegid */
	1, 0, sysconfig,		/* 137 = sysconfig */
	2, 0, adjtime,			/* 138 = adjtime */
	3, 0, systeminfo,		/* 139 = systeminfo */
	0, 0, nosys,			/* 140 = reserved */
	1, 0, seteuid,			/* 141 = seteuid */
};

unsigned sysentsize = sizeof(sysent)/sizeof(struct sysent);
