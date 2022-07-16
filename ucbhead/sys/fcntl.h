/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbhead:sys/fcntl.h	1.7.3.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

#ifndef _SYS_FCNTL_H
#define _SYS_FCNTL_H

#ifndef _SYS_TYPES_H
#include <sys/types.h>
#endif

/* Flag values accessible to open(2) and fcntl(2) */
/*  (The first three can only be set by open) */
#define	O_RDONLY 0
#define	O_WRONLY 1
#define	O_RDWR	 2
#define	O_NDELAY 04	/* Non-blocking I/O */
#define	O_APPEND 010	/* append (writes guaranteed at the end) */
#define O_SYNC	 020	/* synchronous write option */
#define O_NONBLOCK 0200 /* Non-blocking I/O (POSIX) */
#define O_PRIV 010000   /* Private access to file */

/* Flag values accessible only to open(2) */
#define	O_CREAT	00400	/* open with file create (uses third open arg)*/
#define	O_TRUNC	01000	/* open with truncation */
#define	O_EXCL	02000	/* exclusive open */
#define	O_NOCTTY 04000	/* don't allocate controlling tty (POSIX) */

/* fcntl(2) requests */
#define	F_DUPFD		0	/* Duplicate fildes */
#define	F_GETFD		1	/* Get fildes flags */
#define	F_SETFD		2	/* Set fildes flags */
#define	F_GETFL		3	/* Get file flags */
#define	F_SETFL		4	/* Set file flags */
#define	F_SETLK		6	/* Set file lock */
#define	F_SETLKW	7	/* Set file lock and wait */

/* Applications that read /dev/mem must be built like the kernel. A new
** symbol "_KMEMUSER" is defined for this purpose. Applications that read /dev/mem
** will migrate with the kernel to an "_LTYPES" definition.
*/

#if defined(_KERNEL) || defined(_KMEMUSER)
#define	F_GETLK		14	/* Get file lock */
#define	F_O_GETLK	5	/* SVR3 Get file lock */

#else	/* user definition */

#if defined(_LTYPES)	/* EFT definition */
#define	F_GETLK		14	/* Get file lock */
#else
#define	F_GETLK		5	/* Get file lock */
#endif	/* defined(_LTYPES) */

#endif	/* defined(_KERNEL) */

#define	F_SETLK		6	/* Set file lock */
#define	F_SETLKW	7	/* Set file lock and wait */


#define	F_CHKFL		8	/* Reserved */
#define	F_ALLOCSP	10	/* Reserved */
#define	F_FREESP	11	/* Free file space */
#define	F_ISSTREAM	13	/* Is the file desc. a stream ? */
#define	F_PRIV		15	/* Turn on private access to file */
#define	F_NPRIV		16	/* Turn off private access to file */
#define F_QUOTACTL	17	/* UFS quota call */
#define F_BLOCKS	18	/* Get number of BLKSIZE blocks allocated */
#define F_BLKSIZE	19	/* Get optimal I/O block size */ 

#define F_RSETLK	20	/* Remote SETLK for NFS */
#define F_RGETLK	21	/* Remote GETLK for NFS */
#define F_RSETLKW	22	/* Remote SETLKW for NFS */

#define	F_GETOWN	23	/* Get owner */
#define	F_SETOWN	24	/* Set owner */

/* flags for F_GETFL, F_SETFL-- copied from <sys/file.h> */
#ifndef FOPEN
#define FOPEN           0xFFFFFFFF
#define FREAD           0x01
#define FWRITE          0x02
#define FNDELAY         0x04
#define FAPPEND         0x08
#define FSYNC           0x10
#define FNONBLOCK       0x80

#define FMASK           0xFF    /* should be disjoint from FASYNC */
 
/* open-only modes */
 
#define FCREAT          0x0100
#define FTRUNC          0x0200
#define FEXCL           0x0400
#define FNOCTTY         0x0800
#define FASYNC          0x1000
 
/* file descriptor flags */
#define FCLOSEXEC       001     /* close on exec */
#endif
 
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
        o-pid_t	l_pid;
} flock_t;

#else

typedef struct flock {
	short	l_type;
	short	l_whence;
	off_t	l_start;
	off_t	l_len;		/* len == 0 means until end of file */
	long	l_sysid;
        pid_t	l_pid;
	long 	pad[4];		/* reserve area */
} flock_t;

#endif	/* define(_STYPES) */

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
 
#define O_ACCMODE       3       /* Mask for file access modes */
#define FD_CLOEXEC      1       /* close on exec flag */
 

#endif	/* _SYS_FCNTL_H */
