/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_FCNTL_H
#define _SYS_FCNTL_H

#ident	"@(#)head.sys:sys/fcntl.h	11.40.4.1"
#ifndef _SYS_TYPES_H
#include <sys/types.h>
#endif

/*
 * Flag values accessible to open(2) and fcntl(2)
 * (the first three can only be set by open).
 */
#define	O_RDONLY	0
#define	O_WRONLY	1
#define	O_RDWR		2
#if !defined(_POSIX_SOURCE)
#define	O_NDELAY	0x04	/* non-blocking I/O */
#endif /* !defined(_POSIX_SOURCE) */
#define	O_APPEND	0x08	/* append (writes guaranteed at the end) */
#if !defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE)
#define	O_SYNC		0x10	/* synchronous write option */
#endif /* !defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE */

/*
 * The following flag is added for asynchronous raw disk io feature 
 */
#define O_RAIOSIG	0x20	/* cause a signal for a completed RAIO request */ 

#define	O_NONBLOCK	0x80	/* non-blocking I/O (POSIX) */

/*
 * Flag values accessible only to open(2).
 */
#define	O_CREAT		0x100	/* open with file create (uses third open arg) */
#define	O_TRUNC		0x200	/* open with truncation */
#define	O_EXCL		0x400	/* exclusive open */
#define	O_NOCTTY	0x800	/* don't allocate controlling tty (POSIX) */

/* fcntl(2) requests */
#define	F_DUPFD		0	/* Duplicate fildes */
#define	F_GETFD		1	/* Get fildes flags */
#define	F_SETFD		2	/* Set fildes flags */
#define	F_GETFL		3	/* Get file flags */
#define	F_SETFL		4	/* Set file flags */

/*
 * Applications that read /dev/mem must be built like the kernel.  A
 * new symbol "_KMEMUSER" is defined for this purpose.
 */
#if defined(_KERNEL) || defined(_KMEMUSER)
#define	F_GETLK		14	/* Get file lock */
#define	F_O_GETLK	5	/* SVR3 Get file lock */

#else	/* user definition */

#if defined(_STYPES)	/* SVR3 definition */
#define	F_GETLK		5	/* Get file lock */
#else
#define	F_GETLK		14	/* Get file lock */
#endif	/* defined(_STYPES) */

#endif	/* defined(_KERNEL) */

#define	F_SETLK		6	/* Set file lock */
#define	F_SETLKW	7	/* Set file lock and wait */

#if !defined(_POSIX_SOURCE)
#define	F_CHKFL		8	/* Unused */

#define	F_ALLOCSP	10	/* Reserved */
#define	F_FREESP	11	/* Free file space */

#define F_RSETLK	20	/* Remote SETLK for NFS */
#define F_RGETLK	21	/* Remote GETLK for NFS */
#define F_RSETLKW	22	/* Remote SETLKW for NFS */

#define	F_GETOWN	23	/* Get owner (socket emulation) */
#define	F_SETOWN	24	/* Set owner (socket emulation) */
#endif /* !defined(_POSIX_SOURCE) */


#define F_CHSIZE    0x6000  /* XENIX chsize() system call */
#define F_RDCHK     0x6001  /* XENIX rdchk() system call */

/*
 * Fcntl(2) requests made from the XENIX locking(S) system call.  These fcntl()
 * requests are made only from the kernel.
 *
 * N.B.  The high nibble of the high byte is F_SETLK or F_SETLKW, and the low
 *       nibble of the high byte is F_UNLCK, F_WRLCK, or F_RDLCK.  However, *   no code actually relies on this.
 */

#define F_LK_UNLCK  0x6300  /* locking() LK_UNLCK request */
#define F_LK_LOCK   0x7200  /* locking() LK_LOCK request */
#define F_LK_NBLCK  0x6200  /* locking() LK_NBLCK request */
#define F_LK_RLCK   0x7100  /* locking() LK_RLCK request */
#define F_LK_NBRLCK 0x6100  /* locking() LK_NBRLCK request */

#define LK_CMDTYPE(x)   ((x >> 12) & 0x7) /* get high nibble of high byte */
#define LK_LCKTYPE(x)   ((x >> 8) & 0x7)  /* get low nibble of high byte */


/*
 * File segment locking set data type - information passed to system by user.
 */
#if defined(_KERNEL) || defined(_KMEMUSER)
	/* EFT definition */
typedef struct flock {
	short	l_type;
	short	l_whence;
	off_t	l_start;
	off_t	l_len;		/* len == 0 means until end of file */
        long	l_sysid;
        pid_t	l_pid;
	long	pad[4];		/* reserve area */
} flock_t;

typedef struct o_flock {
	short	l_type;
	short	l_whence;
	long	l_start;
	long	l_len;		/* len == 0 means until end of file */
        short   l_sysid;
        o_pid_t l_pid;
} o_flock_t;

#else		/* user level definition */

#if defined(_STYPES)
	/* SVR3 definition */
typedef struct flock {
	short	l_type;
	short	l_whence;
	off_t	l_start;
	off_t	l_len;		/* len == 0 means until end of file */
	short	l_sysid;
        o_pid_t	l_pid;
} flock_t;


#else

typedef struct flock {
	short	l_type;
	short	l_whence;
	off_t	l_start;
	off_t	l_len;		/* len == 0 means until end of file */
	long	l_sysid;
        pid_t	l_pid;
	long	pad[4];		/* reserve area */
} flock_t;

#endif	/* defined(_STYPES) */

#endif	/* defined(_KERNEL) */

/*
 * File segment locking types.
 */
#define	F_RDLCK	01	/* Read lock */
#define	F_WRLCK	02	/* Write lock */
#define	F_UNLCK	03	/* Remove lock(s) */

/*
 * POSIX constants 
 */

#define	O_ACCMODE	3	/* Mask for file access modes */
#define	FD_CLOEXEC	1	/* close on exec flag */

#endif	/* _SYS_FCNTL_H */
