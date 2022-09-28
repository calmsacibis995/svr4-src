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

#ident	"@(#)x286emul:sysent.c	1.1"

#include "vars.h"
#include "sysent.h"

/*
 * This table is the switch used to transfer
 * to the appropriate routine for processing a system call.
 */

#ifdef TRACE
int Close();
#else
int close();
#endif
int nosys(), exit(), Fork(), read(), write();
int open(), close(), Wait();
int creat(), link(), unlink(), chdir(), time();
int mknod(), chmod(), chown(), Break(), Stat(), lseek();
int mount(), umount(), setuid(), Getuid();
int alarm(), Fstat();
int pause(), utime(), stty(), gtty(), access(), nice();
int sync(), kill(), Clocal();
int Dup(), Pipe(), times(), profil(), plock(), setgid();
int Signal();
int acct(), Ioctl();
int Getpid();
int Exec(), Exece();
int umask(), Ulimit();
int Fcntl();
int chroot();
int Stime();
int Setpgrp(), Getgid();
int uadmin();


struct sysent Sysent[] =	/* system call table */
{
      { 0					}, /*  0 = indir */
      { exit,   INT				}, /*  1 = exit */
      { Fork					}, /*  2 = fork */
      { read,   INT, PTR, UINT			}, /*  3 = read */
      { write,  INT, PTR, UINT			}, /*  4 = write */
      { open,   PTR, INT, INT			}, /*  5 = open */
#ifdef TRACE
      { Close,  INT				}, /*  6 = close */
#else
      { close,  INT				}, /*  6 = close */
#endif
      { Wait					}, /*  7 = wait */
      { creat,  PTR, INT			}, /*  8 = creat */
      { link,   PTR, PTR			}, /*  9 = link */
      { unlink, PTR				}, /* 10 = unlink */
      { Exec, RDPTR, RDPTR		}, /* 11 = exec */
      { chdir,  PTR				}, /* 12 = chdir */
      { time,   ZERO				}, /* 13 = time */
      { mknod,  PTR, INT, INT			}, /* 14 = mknod */
      { chmod,  PTR, INT			}, /* 15 = chmod */
      { chown,  PTR, INT, INT			}, /* 16 = chown; now 3 args */
      { Break, RDPTR				}, /* 17 = break */
      { Stat,   PTR, PTR				}, /* 18 = stat */
      { lseek,  INT, LONG, INT			}, /* 19 = seek */
      { Getpid					}, /* 20 = getpid */
      { mount,  PTR, PTR, INT			}, /* 21 = mount */
      { umount, PTR				}, /* 22 = umount */
      { setuid, INT				}, /* 23 = setuid */
      { Getuid, 				}, /* 24 = Getuid */
      { Stime,  LONG				}, /* 25 = stime */
      { 0,					}, /* 26 = ptrace-no support */
      { alarm,  UINT				}, /* 27 = alarm */
      { Fstat,  INT, PTR				}, /* 28 = fstat */
      { pause					}, /* 29 = pause */
      { utime,  PTR, PTR			}, /* 30 = utime */
      { stty,   INT, PTR			}, /* 31 = stty */
      { gtty,   INT, PTR			}, /* 32 = gtty */
      { access, PTR, INT			}, /* 33 = access */
      { nice,   INT				}, /* 34 = nice */
      { 0	}, 						/* 35 */
      { sync					}, /* 36 = sync */
      { kill,   INT, INT			}, /* 37 = kill */
      { 0					}, /* 38 = fstatfs, clocal */
      { Setpgrp, INT				}, /* 39 = setpgrp */
      { 0		}, /* 40 = cxenix, handled by table */
      { Dup,    INT, INT			}, /* 41 = dup */
      { Pipe					}, /* 42 = pipe */
      { times,  PTR				}, /* 43 = times */
      { profil, PTR, INT, PTR, INT		}, /* 44 = prof */
      { plock,   INT				}, /* 45 = proc lock */
      { setgid, INT				}, /* 46 = setgid */
      { Getgid					}, /* 47 = Getgid */
      { Signal, INT, UINT, UINT				}, /* 48 = signal */
      { 0/*Msgsys*/, SPECIAL				}, /* 49 = IPC Messages */
      { 0/*Sysi86*/, SPECIAL				}, /* 50 = sysi86 sys calls */
      { acct,   PTR				}, /* 51 = turn acct off/on */
      { 0/*Shmsys*/, SPECIAL				}, /* 52 = IPC Shared Memory */
      { 0/*Semsys*/, SPECIAL				}, /* 53 = IPC Semaphores */
      { Ioctl,  SPECIAL				}, /* 54 = ioctl */
      { uadmin, INT, INT, INT			}, /* 55 = uadmin */
      { 0,					}, /* 56 = x */
      { 0/*Utssys*/, SPECIAL				}, /* 57 = utssys */
      { 0,					}, /* 58 = reserved for USG */
      { Exece, RDPTR, RDPTR, RDPTR		}, /* 59 = exece */
      { umask,  INT				}, /* 60 = umask */
      { chroot, PTR				}, /* 61 = chroot */
      { Fcntl, SPECIAL				}, /* 62 */
      { Ulimit, INT, LONG			}, /* 63 */
      { nosys,					}, /* 64 */
      { nosys,					}, /* 65 */
      { nosys,					}, /* 66 */
      { nosys,					}, /* 67 */
      { nosys,					}, /* 68 */
      { nosys,					}, /* 69 */
      { nosys,					}, /* 70 */
      { nosys,					}, /* 71 */
      { nosys,					}, /* 72 */
      { nosys,					}, /* 73 */
      { nosys,					}, /* 74 */
      { nosys,					}, /* 75 */
      { nosys,					}, /* 76 */
      { nosys,					}, /* 77 */
      { nosys,					}, /* 78 */
      { nosys,					}, /* 79 */
      { nosys,					}, /* 80 */
      { nosys,					}, /* 81 */
      { nosys,					}, /* 82 */
      { nosys,					}, /* 83 */
      { nosys,					}, /* 84 */
      { nosys,					}, /* 85 */
      { nosys,					}, /* 86 */
      { nosys,					}, /* 87 */
      { nosys,					}, /* 88 */
      { nosys,					}, /* 89 */
      { nosys,					}, /* 90 */
      { nosys,					}, /* 91 */
      { nosys,					}, /* 92 */
      { nosys,					}, /* 93 */
      { nosys,					}, /* 94 */
      { nosys,					}, /* 95 */
      { nosys,					}, /* 96 */
      { nosys,					}, /* 97 */
      { nosys,					}, /* 98 */
      { nosys,					}, /* 99 */
      { nosys,					}, /* 100 */
      { nosys,					}, /* 101 */
      { nosys,					}, /* 102 */
      { nosys,					}, /* 103 */
      { nosys,					}, /* 104 */
      { nosys,					}, /* 105 */
      { nosys,					}, /* 106 */
      { nosys,					}, /* 107 */
      { nosys,					}, /* 108 */
      { nosys,					}, /* 109 */
      { nosys,					}, /* 110 */
      { nosys,					}, /* 111 */
      { nosys,					}, /* 112 */
      { nosys,					}, /* 113 */
      { nosys,					}, /* 114 */
      { nosys,					}, /* 115 */
      { nosys,					}, /* 116 */
      { nosys,					}, /* 117 */
      { nosys,					}, /* 118 */
      { nosys,					}, /* 119 */
      { nosys,					}, /* 120 */
      { nosys,					}, /* 121 */
      { nosys,					}, /* 122 */
      { nosys,					}, /* 123 */
      { nosys,					}, /* 124 */
      { nosys,					}, /* 125 */
      { nosys,					}, /* 126 */
      { 0/*Clocal*/, STACKFRAME, SPECIAL,	}, /* 127 = clocal-only >Sys3 */
};

int nsyscalls = sizeof(Sysent)/sizeof(struct sysent);

/*
 * SetupSysent is a stub at this point
 */
void SetupSysent()
{
}

#if defined(TRACE) || defined(DEBUG)
char *SysCallNames[] = {
	"nosys",
	"exit,   INT",
	"Fork",
	"read,   INT, PTR, UINT",
	"write,  INT, PTR, UINT",
	"open,   PTR, INT, INT",
	"close,  INT",
	"Wait",
	"creat,  PTR, INT",
	"link,   PTR, PTR",
	"unlink, PTR",
	"Exec,   SPECIAL",
	"chdir,  PTR",
	"time,   ZERO",
	"mknod,  PTR, INT, INT",
	"chmod,  PTR, INT",
	"chown,  PTR, INT, INT",
	"Break, RDPTR",
	"Stat,   PTR, PTR",
	"lseek,  INT, LONG, INT",
	"Getpid",
	"mount,  PTR, PTR, INT",
	"umount, PTR",
	"setuid, INT",
	"getuid,",
	"nosys/*Stime*/,  SPECIAL",
	"nosys,",
	"alarm,  UINT",
	"Fstat,  UINT, PTR",
	"pause",
	"utime,  PTR, PTR",
	"stty,   INT, PTR",
	"gtty,   INT, PTR",
	"access, PTR, INT",
	"nice,   INT",
	"statfs, PTR, PTR, INT, INT",
	"sync",
	"kill,   INT, INT",
	"nosys",
	"Setpgrp, SPECIAL",
	"Cxenix, STACKFRAME, SPECIAL,",
	"Dup,    INT",
	"Pipe",
	"times,  PTR",
	"profil, PTR, INT, PTR, INT",
	"plock,   INT",
	"setgid, INT",
	"Getgid, SPECIAL",
	"Signal, INT, UINT, UINT",
	"Msgsys, SPECIAL",
	"Sysi86, SPECIAL",
	"acct,   PTR",
	"Shmsys, SPECIAL",
	"Semsys, SPECIAL",
	"Ioctl,  SPECIAL",
	"uadmin, INT, INT, INT",
	"nosys,",
	"Utssys, SPECIAL",
	"nosys,",
	"Exece,  SPECIAL",
	"umask,  INT",
	"chroot, PTR",
	"Fcntl,  SPECIAL",
	"Ulimit, INT, LONG",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"rmdir,  PTR",
	"mkdir,  PTR, INT",
	"getdents, INT, PTR, INT",
	"nosys,",
	"nosys,",
	"Sysfs,  SPECIAL",
	"getmsg, INT, PTR, PTR, PTR",
	"putmsg, INT, PTR, PTR, INT",
	"poll,   PTR, LONG, INT",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"nosys,",
	"Clocal, STACKFRAME, SPECIAL,",
};
#endif

extern int Locking(), rdchk(), chsize(), ftime(), nap();
extern int StackGrow(), Brkctl();
extern int creatsem(), opensem(), sigsem(), waitsem(), nbwaitsem();
extern int Msgctl(), msgget(), msgsnd(), msgrcv();
extern int semget(), semop(), Semctl();
extern int Execseg(), Unexecseg();
extern int SDget(), SDfree(), SDenter();
extern int SDleave(), SDgetv(), SDwaitv();
extern int Shmctl(), shmget(), Shmat();
struct sysent Xsysent[] =	/* xenix system call table */
{
      { 0 /*shutdown*/				}, /*  0 */
      { Locking, INT, INT, LONG			}, /*  1=locking */
      { creatsem, PTR, INT			}, /*  2=creatsem */
      { opensem, PTR				}, /*  3=opensem */
      { sigsem, INT				}, /*  4=sigsem */
      { waitsem, INT				}, /*  5=waitsem */
      { nbwaitsem, INT				}, /*  6=nbwaitsem */
      { rdchk, INT				}, /*  7=rdchk */
      { StackGrow, STACKFRAME			}, /*  8=stkgrow */
      { 0 /*xtrace*/				}, /*  9 */
      { chsize, INT, LONG			}, /*  10=chsize */
      { ftime, PTR				}, /*  11=ftime */
      { nap, LONG				}, /*  12=nap */
      { SDget, PTR, INT, INT, INT		}, /*  13=sdget */
      { SDfree, INT, INT			}, /*  14=sdfree */
      { SDenter, RDPTR, INT			}, /*  15=sdenter */
      { SDleave, RDPTR				}, /*  16=sdleave */
      { SDgetv, RDPTR				}, /*  17=sdgetv */
      { SDwaitv, RDPTR, INT			}, /*  18=sdwaitv */
      { Brkctl, INT, LONG, LONG			}, /* 19 = brkctl */
      { 0					}, /*  20 unused */
      { 0					}, /*  21 unused */
      { Msgctl, INT, INT, PTR			}, /*  22=msgctl */
      { msgget, LONG, INT			}, /*  23=msgget */
      { msgsnd, INT, PTR, INT, INT		}, /*  24=msgsnd */
      { msgrcv, INT, PTR, INT, LONG, INT	}, /*  25=msgrcv */
      { Semctl, INT, INT, INT, SPECIAL		}, /*  26=semctl */
      { semget, LONG, INT, INT			}, /*  27=semget */
      { semop, INT, PTR, INT			}, /*  28=semop */
      {	Shmctl, INT, INT, PTR			}, /*  29=shmctl */
      {	shmget, LONG, UINT, INT			}, /*  30=shmget */
      {	Shmat, INT, LONG, INT			}, /*  31=shmat */
      {	0					}, /*  32=proctl */
      {	Execseg, LONG, UINT			}, /*  33=execseg */
      {	Unexecseg, LONG				}, /*  34=uexecseg */
      {	0					}, /*  35=select */
};

int nxsyscalls = sizeof(Xsysent)/sizeof(struct sysent);

#if defined(TRACE) || defined(DEBUG)
char * XsysCallNames[] =	/* xenix system call names */
{
		"0 /*shutdown*/",
		"locking, INT, INT, LONG",
		"creatsem, PTR, INT",
		"opensem, PTR",
		"sigsem, INT",
		"waitsem, INT",
		"nbwaitsem, INT",
		"rdchk, INT",
		"StackGrow, STACKFRAME",
		"0 /*xtrace*/",
		"chsize, INT, LONG",
		"ftime, PTR",
		"nap, LONG",
		"sdget, PTR, INT, INT, INT",
		"sdfree, PTR",
		"sdenter, PTR, INT",
		"sdleave, PTR",
		"sdgetv, PTR",
		"sdwaitv, PTR, INT",
		"Brkctl, INT, LONG, LONG",
		"0",
		"0",
		"msgctl, INT, INT, PTR",
		"msgget, LONG, INT",
		"msgsnd, INT, PTR, INT, INT",
		"msgrcv, INT, PTR, INT, LONG, INT",
		"Semctl, INT, INT, INT, SPECIAL",
		"semget, LONG, INT, INT",
		"semop, INT, PTR, INT",
      		"Shmctl, INT, INT, PTR",
      		"shmget, LONG, INT, INT",
      		"Shmat, INT, LONG, INT",
		"0",
      		"Execseg, LONG, UINT",
      		"Unexecseg, LONG",
		"0",
};
#endif

#if 0
int uname(), Ustat();

struct sysent Psysent[] = {
	{ uname, PTR			}, /* 0 = uname */
	{ nosys				}, /* 1 */
	{ Ustat, INT, PTR		}, /* 2 = Ustat */
};
#endif

int Uname(), Ustat();

struct sysent Psysent[] = {
	{ Uname, PTR			}, /* 0 = uname */
	{ nosys				}, /* 1 */
	{ Ustat, INT, PTR		}, /* 2 = Ustat */
};

int npsyscalls = sizeof(Psysent)/sizeof(struct sysent);

#if defined(TRACE) || defined(DEBUG)
char * PsysCallNames[] =	/* pwb system call names */
{
	"Uname, PTR",
	"nosys",
	"Ustat, INT, PTR",
};
#endif

/*
 * errno_map maps System V.3 errno values into Xenix System V.0.
 */
unsigned char errno_map[] = {
	  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
	 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	 32, 33, 34, 43, 44, 37, 38, 39, 40, 41, 42, 43, 44, 36, 45, 47,
	 48, 49, 50, 51, 52, 53, 54, 55, 35, 57, 58, 59, 60, 61, 62, 63,
	 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
	 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
	 96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111,
	112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
	128,129,130,131,132,133,134, 35,136, 37, 38, 39, 40, 41, 42,143,
	144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
};

#ifdef TRACE
Close(i) {
		/* don't close debugging output descriptor while tracing */
	if (systrace && i == dbgdesc) {
		return 0;
	} else {
		return close(i);
	}
}
#endif
