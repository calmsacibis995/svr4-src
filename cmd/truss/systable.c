/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)truss:systable.c	1.8.4.1"

#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/signal.h>
#include <sys/fault.h>
#include <sys/syscall.h>
#include "pcontrol.h"
#include "ramdata.h"
#include "systable.h"
#include "print.h"
#include "proto.h"

#ifdef i386	/* XENIX Support */
#include <sys/sysi86.h>
#include <sys/user.h>
struct user u;
#endif /* i386 */

/* tables of information about system calls - read-only data */

static	CONST	char * CONST	errcode[] = {	/* error code names */
	 NULL,		/*  0 */
	"EPERM",	/*  1 */
	"ENOENT",	/*  2 */
	"ESRCH",	/*  3 */
	"EINTR",	/*  4 */
	"EIO",		/*  5 */
	"ENXIO",	/*  6 */
	"E2BIG",	/*  7 */
	"ENOEXEC",	/*  8 */
	"EBADF",	/*  9 */
	"ECHILD",	/* 10 */
	"EAGAIN",	/* 11 */
	"ENOMEM",	/* 12 */
	"EACCES",	/* 13 */
	"EFAULT",	/* 14 */
	"ENOTBLK",	/* 15 */
	"EBUSY",	/* 16 */
	"EEXIST",	/* 17 */
	"EXDEV",	/* 18 */
	"ENODEV",	/* 19 */
	"ENOTDIR",	/* 20 */
	"EISDIR",	/* 21 */
	"EINVAL",	/* 22 */
	"ENFILE",	/* 23 */
	"EMFILE",	/* 24 */
	"ENOTTY",	/* 25 */
	"ETXTBSY",	/* 26 */
	"EFBIG",	/* 27 */
	"ENOSPC",	/* 28 */
	"ESPIPE",	/* 29 */
	"EROFS",	/* 30 */
	"EMLINK",	/* 31 */
	"EPIPE",	/* 32 */
	"EDOM",		/* 33 */
	"ERANGE",	/* 34 */
	"ENOMSG",	/* 35 */
	"EIDRM",	/* 36 */
	"ECHRNG",	/* 37 */
	"EL2NSYNC",	/* 38 */
	"EL3HLT",	/* 39 */
	"EL3RST",	/* 40 */
	"ELNRNG",	/* 41 */
	"EUNATCH",	/* 42 */
	"ENOCSI",	/* 43 */
	"EL2HLT",	/* 44 */
	"EDEADLK",	/* 45 */
	"ENOLCK",	/* 46 */
	 NULL,		/* 47 */
	 NULL,		/* 48 */
	 NULL,		/* 49 */
	"EBADE",	/* 50 */
	"EBADR",	/* 51 */
	"EXFULL",	/* 52 */
	"ENOANO",	/* 53 */
	"EBADRQC",	/* 54 */
	"EBADSLT",	/* 55 */
	"EDEADLOCK",	/* 56 */
	"EBFONT",	/* 57 */
	 NULL,		/* 58 */
	 NULL,		/* 59 */
	"ENOSTR",	/* 60 */
	"ENODATA",	/* 61 */
	"ETIME",	/* 62 */
	"ENOSR",	/* 63 */
	"ENONET",	/* 64 */
	"ENOPKG",	/* 65 */
	"EREMOTE",	/* 66 */
	"ENOLINK",	/* 67 */
	"EADV",		/* 68 */
	"ESRMNT",	/* 69 */
	"ECOMM",	/* 70 */
	"EPROTO",	/* 71 */
	 NULL,		/* 72 */
	 NULL,		/* 73 */
	"EMULTIHOP",	/* 74 */
	 NULL,		/* 75 */
	 NULL,		/* 76 */
	"EBADMSG",	/* 77 */
	"ENAMETOOLONG",	/* 78 */
	"EOVERFLOW",	/* 79 */
	"ENOTUNIQ",	/* 80 */
	"EBADFD",	/* 81 */
	"EREMCHG",	/* 82 */
	"ELIBACC",	/* 83 */
	"ELIBBAD",	/* 84 */
	"ELIBSCN",	/* 85 */
	"ELIBMAX",	/* 86 */
	"ELIBEXEC",	/* 87 */
	 NULL,		/* 88 */
	"ENOSYS",	/* 89 */
	"ELOOP",	/* 90 */
	"ERESTART",	/* 91 */
	"ESTRPIPE",	/* 92 */
	 NULL,		/* 93 */
	"EUSERS",	/* 94 */
	 NULL,		/* 95 */
	 NULL,		/* 96 */
	 NULL,		/* 97 */
	 NULL,		/* 98 */
	 NULL,		/* 99 */
	 NULL,		/* 100 */
	 NULL,		/* 101 */
	 NULL,		/* 102 */
	 NULL,		/* 103 */
	 NULL,		/* 104 */
	 NULL,		/* 105 */
	 NULL,		/* 106 */
	 NULL,		/* 107 */
	 NULL,		/* 108 */
	 NULL,		/* 109 */
	 NULL,		/* 110 */
	 NULL,		/* 111 */
	 NULL,		/* 112 */
	 NULL,		/* 113 */
	 NULL,		/* 114 */
	 NULL,		/* 115 */
	 NULL,		/* 116 */
	 NULL,		/* 117 */
	 NULL,		/* 118 */
	 NULL,		/* 119 */
	 NULL,		/* 120 */
	 NULL,		/* 121 */
	 NULL,		/* 122 */
	 NULL,		/* 123 */
	 NULL,		/* 124 */
	 NULL,		/* 125 */
	 NULL,		/* 126 */
	 NULL,		/* 127 */
	 NULL,		/* 128 */
	 NULL,		/* 129 */
	 NULL,		/* 130 */
	 NULL,		/* 131 */
	 NULL,		/* 132 */
	 NULL,		/* 133 */
	 NULL,		/* 134 */
	 NULL,		/* 135 */
	 NULL,		/* 136 */
	"ENOTNAM",	/* 137 */
	"ENAVAIL",	/* 138 */
	"EISNAM",	/* 139 */
};

#define	NERRCODE	(sizeof(errcode)/sizeof(char *))


#define	NIBASE	200
static	CONST	char * CONST	nicode[] = {	/* 3BNET error code names */
	"EBADADDR",		/* 200 */
	"EBADCONFIG",		/* 201 */
	"ENOTCONFIG",		/* 202 */
	"EOPENFAILED",		/* 203 */
	"ENOCIRCUIT",		/* 204 */
	"EHLPFAULT",		/* 205 */
	"EPORTNOTAVAIL",	/* 206 */
	"ENETNOTAVAIL",		/* 207 */
	"EDRIVERFAULT",		/* 208 */
	"EDEVICEFAULT",		/* 209 */
	"ENETFAULT",		/* 210 */
	"EBADPACKET",		/* 211 */
	"EDEVICERESET",		/* 212 */
};

#define	NNICODE		(sizeof(nicode)/sizeof(char *))


CONST char *
errname(err)		/* return the error code name (NULL if none) */
register int err;
{
	register CONST char * ename = NULL;

	if (err >= 0 && err < NERRCODE)
		ename = errcode[err];
	else if (err >= NIBASE && err < NIBASE+NNICODE)
		ename = nicode[err-NIBASE];

	return ename;
}


CONST	struct systable systable[] = {
 { NULL,	8, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX}, /*  0 */
 {"_exit",	1, DEC, NOV, DEC},				      /*  1 */
 {"fork",	0, DEC, NOV},					      /*  2 */
 {"read",	3, DEC, NOV, DEC, IOB, DEC},			      /*  3 */
 {"write",	3, DEC, NOV, DEC, IOB, DEC},			      /*  4 */
 {"open",	3, DEC, NOV, STG, OPN, OCT},			      /*  5 */
 {"close",	1, DEC, NOV, DEC},				      /*  6 */
 {"wait",	0, DEC, HHX},					      /*  7 */
 {"creat",	2, DEC, NOV, STG, OCT},				      /*  8 */
 {"link",	2, DEC, NOV, STG, STG},				      /*  9 */
 {"unlink",	1, DEC, NOV, STG},				      /* 10 */
 {"exec",	2, DEC, NOV, STG, DEC},				      /* 11 */
 {"chdir",	1, DEC, NOV, STG},				      /* 12 */
 {"time",	0, DEC, NOV},					      /* 13 */
 {"mknod",	3, DEC, NOV, STG, OCT, HEX},			      /* 14 */
 {"chmod",	2, DEC, NOV, STG, OCT},				      /* 15 */
 {"chown",	3, DEC, NOV, STG, DEC, DEC},			      /* 16 */
 {"brk",	1, DEC, NOV, HEX},				      /* 17 */
 {"stat",	2, DEC, NOV, STG, HEX},				      /* 18 */
 {"lseek",	3, DEC, NOV, DEC, DEX, DEC},			      /* 19 */
 {"getpid",	0, DEC, DEC},					      /* 20 */
 {"mount",	6, DEC, NOV, STG, STG, MTF, MFT, HEX, DEC},	      /* 21 */
 {"umount",	1, DEC, NOV, STG},				      /* 22 */
 {"setuid",	1, DEC, NOV, DEC},				      /* 23 */
 {"getuid",	0, DEC, DEC},					      /* 24 */
 {"stime",	1, DEC, NOV, DEC},				      /* 25 */
 {"ptrace",	4, HEX, NOV, DEC, DEC, HEX, HEX},		      /* 26 */
 {"alarm",	1, DEC, NOV, DEC},				      /* 27 */
 {"fstat",	2, DEC, NOV, DEC, HEX},				      /* 28 */
 {"pause",	0, DEC, NOV},					      /* 29 */
 {"utime",	2, DEC, NOV, STG, HEX},				      /* 30 */
 {"stty",	2, DEC, NOV, DEC, DEC},				      /* 31 */
 {"gtty",	2, DEC, NOV, DEC, DEC},				      /* 32 */
 {"access",	2, DEC, NOV, STG, DEC},				      /* 33 */
 {"nice",	1, DEC, NOV, DEC},				      /* 34 */
 {"statfs",	4, DEC, NOV, STG, HEX, DEC, DEC},		      /* 35 */
 {"sync",	0, DEC, NOV},					      /* 36 */
 {"kill",	2, DEC, NOV, DEC, SIG},				      /* 37 */
 {"fstatfs",	4, DEC, NOV, DEC, HEX, DEC, DEC},		      /* 38 */
 {"pgrpsys",	3, DEC, NOV, DEC, DEC, DEC},			      /* 39 */
 {"cxenix",	2, DEC, NOV, CXEN, HEX},			      /* 40 */
 {"dup",	1, DEC, NOV, DEC},				      /* 41 */
 {"pipe",	0, DEC, DEC},					      /* 42 */
 {"times",	1, DEC, NOV, HEX},				      /* 43 */
 {"profil",	4, DEC, NOV, HEX, DEC, HEX, OCT},		      /* 44 */
 {"plock",	1, DEC, NOV, PLK},				      /* 45 */
 {"setgid",	1, DEC, NOV, DEC},				      /* 46 */
 {"getgid",	0, DEC, DEC},					      /* 47 */
 {"signal",	2, HEX, NOV, SIG, ACT},				      /* 48 */
 {"msgsys",	6, DEC, NOV, DEC, DEC, DEC, DEC, DEC, DEC},	      /* 49 */

#ifdef i386
 {"sysi86",	4, HEX, NOV, SI86, HEX, HEX, HEX, DEC, DEC},	      /* 50 */
#else
 {"sys3b",	4, HHX, NOV, S3B, HEX, HEX, HEX},		      /* 50 */
#endif

 {"acct",	1, DEC, NOV, STG},				      /* 51 */
 {"shmsys",	4, DEC, NOV, DEC, HEX, HEX, HEX},		      /* 52 */
 {"semsys",	5, DEC, NOV, DEC, HEX, HEX, HEX, HEX},		      /* 53 */
 {"ioctl",	3, DEC, NOV, DEC, IOC, IOA},			      /* 54 */
 {"uadmin",	3, DEC, NOV, DEC, DEC, DEC},			      /* 55 */
 { NULL,	8, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX}, /* 56 */
 {"utssys",	4, DEC, NOV, HEX, DEC, UTS, HEX},		      /* 57 */
 {"fsync",	1, DEC, NOV, DEC},				      /* 58 */
 {"execve",	3, DEC, NOV, STG, HEX, HEX},			      /* 59 */
 {"umask",	1, OCT, NOV, OCT},				      /* 60 */
 {"chroot",	1, DEC, NOV, STG},				      /* 61 */
 {"fcntl",	3, DEC, NOV, DEC, FCN, HEX},			      /* 62 */
 {"ulimit",	2, DEX, NOV, ULM, DEC},				      /* 63 */

	/* The following 6 entries were reserved for Safari 4 */
 { NULL,	8, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX}, /* 64 */
 { NULL,	8, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX}, /* 65 */
 { NULL,	8, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX}, /* 66 */
 { NULL,	8, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX}, /* 67 */
 { NULL,	8, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX}, /* 68 */
 { NULL,	8, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX}, /* 69 */

	/* Obsolete RFS-specific entries */
 { NULL,	8, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX}, /* 70 */
 { NULL,	8, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX}, /* 71 */
 { NULL,	8, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX}, /* 72 */
 { NULL,	8, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX}, /* 73 */
 { NULL,	8, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX}, /* 74 */
 { NULL,	8, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX}, /* 75 */
 { NULL,	8, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX}, /* 76 */
 { NULL,	8, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX}, /* 77 */

 {"rfsys",	5, DEC, NOV, RFS, HEX, HEX, HEX, HEX},		      /* 78 */
 {"rmdir",	1, DEC, NOV, STG},				      /* 79 */
 {"mkdir",	2, DEC, NOV, STG, OCT},				      /* 80 */
 {"getdents",	3, DEC, NOV, DEC, HEX, DEC},			      /* 81 */
 { NULL,	8, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX}, /* 82 */
 { NULL,	8, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX}, /* 83 */
 {"sysfs",	3, DEC, NOV, SFS, DEX, DEX},			      /* 84 */
 {"getmsg",	4, DEC, NOV, DEC, HEX, HEX, HEX},		      /* 85 */
 {"putmsg",	4, DEC, NOV, DEC, HEX, HEX, SMF},		      /* 86 */
 {"poll",	3, DEC, NOV, HEX, DEC, DEC},			      /* 87 */
 {"lstat",	2, DEC, NOV, STG, HEX},				      /* 88 */
 {"symlink",	2, DEC, NOV, STG, STG},				      /* 89 */
 {"readlink",	3, DEC, NOV, STG, RLK, DEC},			      /* 90 */
 {"setgroups",	2, DEC, NOV, DEC, HEX},				      /* 91 */
 {"getgroups",	2, DEC, NOV, DEC, HEX},				      /* 92 */
 {"fchmod",	2, DEC, NOV, DEC, OCT},				      /* 93 */
 {"fchown",	3, DEC, NOV, DEC, DEC, DEC},			      /* 94 */
 {"sigprocmask",3, DEC, NOV, SPM, HEX, HEX},			      /* 95 */
 {"sigsuspend",	1, DEC, NOV, HEX},				      /* 96 */
 {"sigaltstack",2, DEC, NOV, HEX, HEX},				      /* 97 */
 {"sigaction",	3, DEC, NOV, SIG, HEX, HEX},			      /* 98 */
 {"sigpending",	2, DEC, NOV, DEC, HEX},				      /* 99 */
 {"context",	2, DEC, NOV, DEC, HEX},				      /* 100 */
 {"evsys",	3, DEC, NOV, DEC, DEC, HEX},			      /* 101 */
 {"evtrapret",	0, DEC, NOV},					      /* 102 */
 {"statvfs",	2, DEC, NOV, STG, HEX},				      /* 103 */
 {"fstatvfs",	2, DEC, NOV, DEC, HEX},				      /* 104 */
 { NULL,	8, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX}, /* 105 */
 {"nfssys",	2, DEC, NOV, DEC, HEX},				      /* 106 */
 {"waitsys",	4, DEC, NOV, HEX, DEC, HEX, WOP},		      /* 107 */
 {"sigsendsys",	2, DEC, NOV, HEX, SIG},				      /* 108 */
 {"hrtsys",	5, DEC, NOV, DEC, HEX, HEX, HEX, HEX},		      /* 109 */
 {"acancel",	3, DEC, NOV, DEC, HEX, DEC},			      /* 110 */
 {"async",	3, DEC, NOV, DEC, HEX, DEC},			      /* 111 */
 {"priocntlsys",4, DEC, NOV, DEC, HEX, DEC, HEX},		      /* 112 */
 {"pathconf",	2, DEC, NOV, STG, PTC},				      /* 113 */
 {"mincore",	3, DEC, NOV, HEX, DEC, HEX},			      /* 114 */
 {"mmap",	6, HEX, NOV, HEX, DEC, MPR, MTY, DEC, DEC},	      /* 115 */
 {"mprotect",	3, DEC, NOV, HEX, DEC, MPR},			      /* 116 */
 {"munmap",	2, DEC, NOV, HEX, DEC},				      /* 117 */
 {"fpathconf",	2, DEC, NOV, DEC, PTC},				      /* 118 */
 {"vfork",	0, DEC, NOV},					      /* 119 */
 {"fchdir",	1, DEC, NOV, DEC},				      /* 120 */
 {"readv",	3, DEC, NOV, DEC, HEX, DEC},			      /* 121 */
 {"writev",	3, DEC, NOV, DEC, HEX, DEC},			      /* 122 */
 {"xstat",	3, DEC, NOV, DEC, STG, HEX},			      /* 123 */
 {"lxstat",	3, DEC, NOV, DEC, STG, HEX},			      /* 124 */
 {"fxstat",	3, DEC, NOV, DEC, DEC, HEX},			      /* 125 */
 {"xmknod",	4, DEC, NOV, DEC, STG, OCT, HEX},		      /* 126 */
 {"clocal",	5, HEX, HEX, DEC, HEX, HEX, HEX, HEX},		      /* 127 */
 {"setrlimit",	2, DEC, NOV, RLM, HEX},				      /* 128 */
 {"getrlimit",	2, DEC, NOV, RLM, HEX},				      /* 129 */
 {"lchown",	3, DEC, NOV, STG, DEC, DEC},			      /* 130 */
 {"memcntl",	6, DEC, NOV, HEX, DEC, MCF, MC4, MC5, DEC},	      /* 131 */
 {"getpmsg",	5, DEC, NOV, DEC, HEX, HEX, HEX, HEX},		      /* 132 */
 {"putpmsg",	5, DEC, NOV, DEC, HEX, HEX, DEC, HHX},		      /* 133 */
 {"rename",	2, DEC, NOV, STG, STG},				      /* 134 */
 {"nuname",	1, DEC, NOV, HEX},				      /* 135 */
 {"setegid",	1, DEC, NOV, DEC},				      /* 136 */
 {"sysconfig",	1, DEC, NOV, CNF},				      /* 137 */
 {"adjtime",	2, DEC, NOV, HEX, HEX},				      /* 138 */
 {"systeminfo",	3, DEC, NOV, INF, RST, DEC},			      /* 139 */
 { NULL,	8, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX, HEX}, /* 140 */
 {"seteuid",	1, DEC, NOV, DEC},				      /* 141 */
 { NULL,	-1,DEC, NOV},					      /* end */
};

/* SYSEND == max syscall number + 1 */
#define	SYSEND	((sizeof(systable)/sizeof(struct systable))-1)


/* The following are for interpreting syscalls with sub-codes */

static	CONST	struct systable sigtable[] = {
 {"signal",	2, HEX, NOV, SIG, ACT},				      /* 0 */
 {"sigset",	2, HEX, NOV, SIX, ACT},				      /* 1 */
 {"sighold",	1, HEX, NOV, SIX},				      /* 2 */
 {"sigrelse",	1, HEX, NOV, SIX},				      /* 3 */
 {"sigignore",	1, HEX, NOV, SIX},				      /* 4 */
 {"sigpause",	1, HEX, NOV, SIX},				      /* 5 */
};
#define	NSIGCODE	(sizeof(sigtable)/sizeof(struct systable))

static	CONST	struct systable msgtable[] = {
 {"msgget",	3, DEC, NOV, HID, DEC, MSF},			      /* 0 */
 {"msgctl",	4, DEC, NOV, HID, DEC, MSC, HEX},		      /* 1 */
 {"msgrcv",	6, DEC, NOV, HID, DEC, HEX, DEC, DEC, MSF},	      /* 2 */
 {"msgsnd",	5, DEC, NOV, HID, DEC, HEX, DEC, MSF},		      /* 3 */
};
#define	NMSGCODE	(sizeof(msgtable)/sizeof(struct systable))

static	CONST	struct systable semtable[] = {
 {"semctl",	5, DEC, NOV, HID, DEC, DEC, SEC, DEX},		      /* 0 */
 {"semget",	4, DEC, NOV, HID, DEC, DEC, SEF},		      /* 1 */
 {"semop",	4, DEC, NOV, HID, DEC, HEX, DEC},		      /* 2  */
};
#define	NSEMCODE	(sizeof(semtable)/sizeof(struct systable))

static	CONST	struct systable shmtable[] = {
 {"shmat",	4, HEX, NOV, HID, DEC, DEX, SHF},		      /* 0 */
 {"shmctl",	4, DEC, NOV, HID, DEC, SHC, DEX},		      /* 1 */
 {"shmdt",	2, DEC, NOV, HID, HEX},				      /* 2 */
 {"shmget",	4, DEC, NOV, HID, DEC, DEC, SHF},		      /* 3 */
};
#define	NSHMCODE	(sizeof(shmtable)/sizeof(struct systable))

static	CONST	struct systable pidtable[] = {
 {"getpgrp",	1, DEC, NOV, HID},				      /* 0 */
 {"setpgrp",	1, DEC, NOV, HID},				      /* 1 */
 {"getsid",	2, DEC, NOV, HID, DEC},				      /* 2 */
 {"setsid",	1, DEC, NOV, HID},				      /* 3 */
 {"getpgid",	2, DEC, NOV, HID, DEC},				      /* 4 */
 {"setpgid",	3, DEC, NOV, HID, DEC, DEC},			      /* 5 */
};
#define	NPIDCODE	(sizeof(pidtable)/sizeof(struct systable))

static	CONST	struct systable sfstable[] = {
 {"sysfs",	3, DEC, NOV, SFS, DEX, DEX},			      /* 0 */
 {"sysfs",	2, DEC, NOV, SFS, STG},				      /* 1 */
 {"sysfs",	3, DEC, NOV, SFS, DEC, RST},			      /* 2 */
 {"sysfs",	1, DEC, NOV, SFS},				      /* 3 */
};
#define	NSFSCODE	(sizeof(sfstable)/sizeof(struct systable))

static	CONST	struct systable rfstable[] = {
 {"rfsys",	5, DEC, NOV, RFS, STG, DEC, DEC, DEC},		      /* 0 */
 {"rfsys",	2, NOV, NOV, RFS, STG},				      /* 1 */
 {"rfsys",	4, NOV, NOV, RFS, HEX, STG, DEC},		      /* 2 */
 {"rfsys",	3, DEC, NOV, RFS, RST, DEC},			      /* 3 */
 {"rfsys",	1, NOV, NOV, RFS},				      /* 4 */
 {"rfsys",	3, NOV, NOV, RFS, STG, DEC},			      /* 5 */
 {"rfsys",	3, NOV, NOV, RFS, RST, DEC},			      /* 6 */
 {"rfsys",	4, NOV, NOV, RFS, STG, DEC, HEX},		      /* 7 */
 {"rfsys",	4, NOV, NOV, RFS, DEC, HEX, HEX},		      /* 8 */
 {"rfsys",	2, DEC, NOV, RFS, RV1},				      /* 9 */
 {"rfsys",	4, DEC, NOV, RFS, RV2, HEX, HEX},		      /* 10 */
 {"rfsys",	1, DEC, NOV, RFS},				      /* 11 */
 {"rfsys",	2, DEC, NOV, RFS, RV3},				      /* 12 */
 {"rfsys",	3, DEC, NOV, RFS, STG, HEX},			      /* 13 */
 {"rfsys",	2, DEC, NOV, RFS, HEX},				      /* 14 */
 {"rfsys",	5, NOV, NOV, RFS, STG, STG, DEC, HEX, HEX},	      /* 15 */
 {"rfsys",	2, NOV, NOV, RFS, STG},				      /* 16 */
 {"rfsys",	1, NOV, NOV, RFS},				      /* 17 */
 {"rfsys",	1, NOV, NOV, RFS},				      /* 18 */
 {"rfsys",	2, DEC, NOV, RFS, HEX},				      /* 19 */
};
#define	NRFSCODE	(sizeof(rfstable)/sizeof(struct systable))

static	CONST	struct systable utstable[] = {
 {"utssys",	3, DEC, NOV, HEX, DEC, UTS},			      /* 0 */
 {"utssys",	4, DEC, NOV, HEX, HEX, HEX, HEX},		      /* err */
 {"utssys",	3, DEC, NOV, HEX, HHX, UTS},			      /* 2 */
 {"utssys",	4, DEC, NOV, STG, FUI, UTS, HEX}		      /* 3 */
};
#define	NUTSCODE	(sizeof(utstable)/sizeof(struct systable))

static	CONST	struct systable sgptable[] = {
 {"sigpending",	2, DEC, NOV, DEC, HEX},				      /* err */
 {"sigpending",	2, DEC, NOV, HID, HEX},				      /* 1 */
 {"sigfillset",	2, DEC, NOV, HID, HEX},				      /* 2 */
};
#define	NSGPCODE	(sizeof(sgptable)/sizeof(struct systable))

static	CONST	struct systable ctxtable[] = {
 {"getcontext",	2, DEC, NOV, HID, HEX},				      /* 0 */
 {"setcontext",	2, DEC, NOV, HID, HEX},				      /* 1 */
};
#define	NCTXCODE	(sizeof(ctxtable)/sizeof(struct systable))

static	CONST	struct systable hrttable[] = {
 {"hrtcntl",	5, DEC, NOV, HID, DEC, DEC, HEX, HEX},		      /* 0 */
 {"hrtalarm",	3, DEC, NOV, HID, HEX, DEC},			      /* 1 */
 {"hrtsleep",	2, DEC, NOV, HID, HEX},				      /* 2 */
 {"hrtcancel",	3, DEC, NOV, HID, HEX, DEC},			      /* 3 */
};
#define	NHRTCODE	(sizeof(hrttable)/sizeof(struct systable))

#ifdef i386 /* XENIX Support */
static	CONST	struct systable xentable[] = {
 {"obs_shutdown",	0, DEC, DEC}, 				/* 0 */
 {"locking",	3, DEC, DEC, NOV, DEC, DEC},			/* 1 */
 {"creatsem",	2, DEC, DEC, NOV, OCT},				/* 2 */
 {"opensem",	1, DEC, HID, STG},				/* 3 */
 {"sigsem",	1, DEC, HID, DEC},				/* 4 */
 {"waitsem",	1, DEC, HID, DEC},				/* 5 */
 {"nbwaitsem",	1, DEC, HID, DEC},				/* 6 */
 {"rdchk",	1, DEC, HID, DEC},				/* 7 */
 {"obs_stkgrow",	0, DEC, HEX},				/* 8 */
 {"obs_ptrace",	0, DEC, HEX},					/* 9 */
 {"chsize",	2, DEC, HID, DEC, DEC},				/* 10 */
 {"ftime",	1, DEC, HID, HEX},				/* 11 */
 {"nap",	1, DEC, HID, DEC},				/* 12 */
 {"sdget",	4, DEC, HID, STG, HHX, DEC, OCT},		/* 13 */
 {"xsdfree",	1, DEC, HID, HEX},				/* 14 */
 {"sdenter",	2, DEC, HID, HEX, HHX},				/* 15 */
 {"sdleave",	1, DEC, HID, HEX},				/* 16 */
 {"sdgetv",	1, DEC, HID, HEX},				/* 17 */
 {"sdwaitv",	2, DEC, HID, HEX, DEC},				/* 18 */
 {"obs_brkctl",	0, DEC, HEX},					/* 19 */
 {"reserved",	0, DEC, HEX},					/* 20 */
 {"obs_nfs_sys",	0, HEX, HEX},				/* 21 */
 {"obs_msgctl",	0, DEC, HEX},					/* 22 */
 {"obs_msgget",	0, DEC, HEX},					/* 23 */
 {"obs_msgsnd",	0, DEC, HEX},					/* 24 */
 {"obs_msgrcv",	0, DEC, HEX},					/* 25 */
 {"obs_semctl",	0, DEC, HEX},					/* 26 */
 {"obs_semget",	0, DEC, HEX},					/* 27 */
 {"obs_semop",	0, DEC, HEX},					/* 28 */
 {"obs_shmctl",	0, DEC, HEX},					/* 29 */
 {"obs_shmget",	0, DEC, HEX},					/* 30 */
 {"obs_shmat",	0, DEC, HEX},					/* 31 */
 {"proctl",	3, DEC, HID, DEC, DEC, HEX},			/* 32 */
 {"execseg",	0, DEC, HEX},					/* 33 */
 {"unexecseg",	0, DEC, HEX},					/* 34 */
 {"obs_swapadd",	0, HEX, HEX},				/* 35 */
};
#define	NXENCODE	(sizeof(xentable)/sizeof(struct systable))
#endif /* i386 */

CONST	struct sysalias sysalias[] = {
	{ "exit",	SYS_exit	},
	{ "sbrk",	SYS_brk		},
	{ "getppid",	SYS_getpid	},
	{ "geteuid",	SYS_getuid	},
	{ "getpgrp",	SYS_pgrpsys	},
	{ "setpgrp",	SYS_pgrpsys	},
	{ "getsid",	SYS_pgrpsys	},
	{ "setsid",	SYS_pgrpsys	},
	{ "getpgid",	SYS_pgrpsys	},
	{ "setpgid",	SYS_pgrpsys	},
	{ "getegid",	SYS_getgid	},
	{ "sigset",	SYS_signal	},
	{ "sighold",	SYS_signal	},
	{ "sigrelse",	SYS_signal	},
	{ "sigignore",	SYS_signal	},
	{ "sigpause",	SYS_signal	},
	{ "msgctl",	SYS_msgsys	},
	{ "msgget",	SYS_msgsys	},
	{ "msgsnd",	SYS_msgsys	},
	{ "msgrcv",	SYS_msgsys	},
	{ "msgop",	SYS_msgsys	},
	{ "shmctl",	SYS_shmsys	},
	{ "shmget",	SYS_shmsys	},
	{ "shmat",	SYS_shmsys	},
	{ "shmdt",	SYS_shmsys	},
	{ "shmop",	SYS_shmsys	},
	{ "semctl",	SYS_semsys	},
	{ "semget",	SYS_semsys	},
	{ "semop",	SYS_semsys	},
	{ "uname",	SYS_utssys	},
	{ "ustat",	SYS_utssys	},
	{ "fusers",	SYS_utssys	},
	{ "exec",	SYS_execve	},
	{ "execl",	SYS_execve	},
	{ "execv",	SYS_execve	},
	{ "execle",	SYS_execve	},
	{ "execlp",	SYS_execve	},
	{ "execvp",	SYS_execve	},
	{ "sigfillset",	SYS_sigpending	},
	{ "getcontext",	SYS_context	},
	{ "setcontext",	SYS_context	},
	{ "hrtcntl",	SYS_hrtsys	},
	{ "hrtalarm",	SYS_hrtsys	},
	{ "hrtsleep",	SYS_hrtsys	},
	{ "hrtcancel",	SYS_hrtsys	},
	{  NULL,	0	}	/* end-of-list */
};


/* return structure to interpret system call with sub-codes */
CONST struct systable *
subsys(syscall, subcode)
register int syscall;
register int subcode;
{
	register CONST struct systable *stp = NULL;

	if (subcode != -1) {
		switch (syscall) {
		case SYS_signal:	/* signal() + sigset() family */
			switch(subcode & ~SIGNO_MASK) {
			default:	subcode = 0;	break;
			case SIGDEFER:	subcode = 1;	break;
			case SIGHOLD:	subcode = 2;	break;
			case SIGRELSE:	subcode = 3;	break;
			case SIGIGNORE:	subcode = 4;	break;
			case SIGPAUSE:	subcode = 5;	break;
			}
			if ((unsigned)subcode < NSIGCODE)
				stp = &sigtable[subcode];
			break;
		case SYS_msgsys:	/* msgsys() */
			if ((unsigned)subcode < NMSGCODE)
				stp = &msgtable[subcode];
			break;
		case SYS_semsys:	/* semsys() */
			if ((unsigned)subcode < NSEMCODE)
				stp = &semtable[subcode];
			break;
		case SYS_shmsys:	/* shmsys() */
			if ((unsigned)subcode < NSHMCODE)
				stp = &shmtable[subcode];
			break;
		case SYS_pgrpsys:	/* pgrpsys() */
			if ((unsigned)subcode < NPIDCODE)
				stp = &pidtable[subcode];
			break;
		case SYS_utssys:	/* utssys() */
			if ((unsigned)subcode < NUTSCODE)
				stp = &utstable[subcode];
			break;
		case SYS_sysfs:		/* sysfs() */
			if ((unsigned)subcode < NSFSCODE)
				stp = &sfstable[subcode];
			break;
		case SYS_rfsys:		/* rfsys() */
			if ((unsigned)subcode < NRFSCODE)
				stp = &rfstable[subcode];
			break;
		case SYS_sigpending:	/* sigpending()/sigfillset() */
			if ((unsigned)subcode < NSGPCODE)
				stp = &sgptable[subcode];
			break;
		case SYS_context:	/* [get|set]context() */
			if ((unsigned)subcode < NCTXCODE)
				stp = &ctxtable[subcode];
			break;
		case SYS_hrtsys:	/* hrtsys() */
			if ((unsigned)subcode < NHRTCODE)
				stp = &hrttable[subcode];
			break;
		}
	}

	if (stp == NULL)
		stp = &systable[((unsigned)syscall < SYSEND)? syscall : 0];

	return stp;
}

#ifdef i386 /* XENIX Support */

extern pid_t g_upid;
CONST char *
cxenixname(code)
register int code;
{
	static char xstp[80];
	char x[10];
	int j, *ap;
	register CONST char *stp = NULL;
	if (sysi86(RDUBLK, g_upid, &u, sizeof(u)) >= 0) {
		code = (u.u_syscall >> 8) & 0xFF;
		if ((unsigned)code < NXENCODE) {
			ap = (int *)u.u_ar0[R_SP]; /* as per pcontrol.h: UESP */
			ap++;
			sprintf (xstp,"%s(", xentable[(unsigned)code].name);
			for (j = 0; j < xentable[(unsigned)code].nargs; j++) {
				sprintf (x," %x",ap++);
			    	strcat (xstp, x);
			}
			strcat(xstp,")");
			stp = xstp;
		}
	}
	return stp;
}
#endif /* i386 */

CONST char *
sysname(syscall, subcode)	/* return the name of the system call */
int syscall;
int subcode;
{
	register CONST struct systable * stp = subsys(syscall, subcode);
	register CONST char * name = stp->name;	/* may be NULL */

	if (name == NULL) {		/* manufacture a name */
		(void) sprintf(sys_name, "sys#%d", syscall);
		name = sys_name;
	}

	return name;
}

CONST char *
rawsigname(sig)		/* return the name of the signal */
int sig;		/* return NULL if unknown signal */
{
	register CONST char * name;

	switch (sig) {
	case SIGHUP:	name = "SIGHUP";	break;
	case SIGINT:	name = "SIGINT";	break;
	case SIGQUIT:	name = "SIGQUIT";	break;
	case SIGILL:	name = "SIGILL";	break;
	case SIGTRAP:	name = "SIGTRAP";	break;
	case SIGABRT:	name = "SIGABRT";	break;
	case SIGEMT:	name = "SIGEMT";	break;
	case SIGFPE:	name = "SIGFPE";	break;
	case SIGKILL:	name = "SIGKILL";	break;
	case SIGBUS:	name = "SIGBUS";	break;
	case SIGSEGV:	name = "SIGSEGV";	break;
	case SIGSYS:	name = "SIGSYS";	break;
	case SIGPIPE:	name = "SIGPIPE";	break;
	case SIGALRM:	name = "SIGALRM";	break;
	case SIGTERM:	name = "SIGTERM";	break;
	case SIGUSR1:	name = "SIGUSR1";	break;
	case SIGUSR2:	name = "SIGUSR2";	break;
	case SIGCLD:	name = "SIGCLD";	break;
	case SIGPWR:	name = "SIGPWR";	break;
	case SIGWINCH:	name = "SIGWINCH";	break;
	case SIGURG:	name = "SIGURG";	break;
	case SIGPOLL:	name = "SIGPOLL";	break;
	case SIGSTOP:	name = "SIGSTOP";	break;
	case SIGTSTP:	name = "SIGTSTP";	break;
	case SIGCONT:	name = "SIGCONT";	break;
	case SIGTTIN:	name = "SIGTTIN";	break;
	case SIGTTOU:	name = "SIGTTOU";	break;
	case SIGVTALRM:	name = "SIGVTALRM";	break;
	case SIGPROF:	name = "SIGPROF";	break;
	case SIGXCPU:	name = "SIGXCPU";	break;
	case SIGXFSZ:	name = "SIGXFSZ";	break;
	default:	name = NULL;		break;
	}

	return name;
}

CONST char *
signame(sig)		/* return the name of the signal */
int sig;		/* manufacture a name for unknown signal */
{
	register CONST char * name = rawsigname(sig);

	if (name == NULL) {			/* manufacture a name */
		(void) sprintf(sig_name, "SIG#%d", sig);
		name = sig_name;
	}

	return name;
}

CONST char *
rawfltname(flt)		/* return the name of the fault */
int flt;		/* return NULL if unknown fault */
{
	register CONST char * name;

	switch (flt) {
	case FLTILL:	name = "FLTILL";	break;
	case FLTPRIV:	name = "FLTPRIV";	break;
	case FLTBPT:	name = "FLTBPT";	break;
	case FLTTRACE:	name = "FLTTRACE";	break;
	case FLTACCESS:	name = "FLTACCESS";	break;
	case FLTBOUNDS:	name = "FLTBOUNDS";	break;
	case FLTIOVF:	name = "FLTIOVF";	break;
	case FLTIZDIV:	name = "FLTIZDIV";	break;
	case FLTFPE:	name = "FLTFPE";	break;
	case FLTSTACK:	name = "FLTSTACK";	break;
	case FLTPAGE:	name = "FLTPAGE";	break;
	default:	name = NULL;		break;
	}

	return name;
}

CONST char *
fltname(flt)		/* return the name of the fault */
int flt;		/* manufacture a name if fault unknown */
{
	register CONST char * name = rawfltname(flt);

	if (name == NULL) {			/* manufacture a name */
		(void) sprintf(flt_name, "FLT#%d", flt);
		name = flt_name;
	}

	return name;
}
