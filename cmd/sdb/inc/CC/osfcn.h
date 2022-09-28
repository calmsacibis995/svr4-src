/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _UNISTD_H
#define _UNISTD_H

#ident	"@(#)sdb:inc/CC/osfcn.h	1.4"
  
/* Symbolic constants for the "access" routine: */
#define	R_OK	4	/* Test for Read permission */
#define	W_OK	2	/* Test for Write permission */
#define	X_OK	1	/* Test for eXecute permission */
#define	F_OK	0	/* Test for existence of File */

#define F_ULOCK	0	/* Unlock a previously locked region */
#define F_LOCK	1	/* Lock a region for exclusive use */
#define F_TLOCK	2	/* Test and lock a region for exclusive use */
#define F_TEST	3	/* Test a region for other processes locks */


/* Symbolic constants for the "lseek" routine: */
#define	SEEK_SET	0	/* Set file pointer to "offset" */
#define	SEEK_CUR	1	/* Set file pointer to current plus "offset" */
#define	SEEK_END	2	/* Set file pointer to EOF plus "offset" */

/* Path names: */
#define	GF_PATH	"/etc/group"	/* Path name of the "group" file */
#define	PF_PATH	"/etc/passwd"	/* Path name of the "passwd" file */


/* command names for POSIX sysconf */
#define _SC_ARG_MAX	1
#define _SC_CHILD_MAX	2
#define _SC_CLK_TCK	3
#define _SC_NGROUPS_MAX 4
#define _SC_OPEN_MAX	5
#define _SC_JOB_CONTROL 6
#define _SC_SAVED_IDS	7
#define _SC_VERSION	8
#define _SC_PASS_MAX	9
#define _SC_LOGNAME_MAX	10

/* command names for POSIX pathconf */

#define _PC_LINK_MAX	1
#define _PC_MAX_CANON	2
#define _PC_MAX_INPUT	3
#define _PC_NAME_MAX	4
#define _PC_PATH_MAX	5
#define _PC_PIPE_BUF	6
#define _PC_NO_TRUNC	7
#define _PC_VDISABLE	8
#define _PC_CHOWN_RESTRICTED	9

/* compile-time symbolic constants,
** Support does not mean the feature is enabled.
** Use pathconf/sysconf to obtain actual configuration value.
** 
*/

#define _POSIX_JOB_CONTROL	1
#define _POSIX_SAVED_IDS	1

#ifndef _POSIX_VDISABLE
#define _POSIX_VDISABLE		0
#endif

/* Current version of POSIX */
#define _POSIX_VERSION		198808L


#if defined(__STDC__)

#include <sys/types.h>

extern int access(const char *, int);
extern int acct(const char *);
extern int atoi(const char *);
extern unsigned alarm(unsigned);
extern int brk(void *);
extern int chdir(const char *);
extern int chmod(const char *, mode_t);
extern int chown(const char *, uid_t, gid_t);
extern int chroot(const char *);
extern int close(int);
extern int dup(int);
extern int execl(const char *, const char *, ...);
extern int execle(const char *, const char *, ...);
extern int execlp(const char *, const char *, ...);
extern int execv(const char *, const char **);
extern int execve(const char *, const char **, const char**);
extern int execvp(const char *, const char **);
extern void exit(int);
extern void _exit(int);
extern pid_t fork(void);
extern gid_t getegid(void);
extern uid_t geteuid(void);
extern gid_t getgid(void);
extern pid_t getpid(void);
extern pid_t getppid(void);
extern pid_t getpgrp(void);
extern uid_t getuid(void);
extern int ioctl(int, int, ...);
extern int isatty(int);
extern int kill(int, int);
extern int link(const char *, const char *);
extern int lockf(int, int, long);
extern long lseek(int, long, int);
extern int mkdir(const char *, int);
extern int nice(int);
extern int open(const char *, int, ...);
extern int pathconf(char *, int);
extern int fpathconf(int, int);
extern int pause(void);
extern int pipe(int *);
extern void profil(char *, int, int, int);
extern int ptrace(int, pid_t, int, int);
extern int read(int, void *, unsigned);
extern int rmdir(const char *);
extern void *sbrk(int);
extern int setgid(gid_t);
extern pid_t setpgrp(void);
extern int setuid(uid_t);
extern unsigned sleep(unsigned);
extern int stime(const time_t *);
extern void sync(void);
extern int sysconf(int);
extern long ulimit(int, long);
extern mode_t umask(mode_t);
extern int unlink(const char *);
extern int wait(int *);
extern int write(int, const void *, unsigned);

#else
extern unsigned alarm();
extern void exit();
extern void _exit();
extern unsigned short getegid();
extern unsigned short geteuid();
extern unsigned short getgid();
extern unsigned short getuid();
extern long lseek();
extern void profil();
extern char *sbrk();
extern unsigned sleep();
extern void sync();
extern long ulimit();

#endif

#endif /* _UNISTD_H */
