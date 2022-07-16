/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _LIMITS_H
#define _LIMITS_H

#ident	"@(#)head:limits.h	1.34"

/* Sizes of integral types */
#define CHAR_BIT        8       	/* max # of bits in a "char" */
#define SCHAR_MIN	(-128)		/* min value of a "signed char" */
#define SCHAR_MAX	127		/* max value of a "signed char" */
#define UCHAR_MAX	255		/* max value of an "unsigned char" */

#define MB_LEN_MAX	5

#if defined(__STDC__)
#if #machine(i386) || #machine(sparc)
#define CHAR_MIN        SCHAR_MIN	/* min value of a "char" */
#define CHAR_MAX        SCHAR_MAX	/* max value of a "char" */
#else
#define CHAR_MIN        0               /* min value of a "char" */
#define CHAR_MAX        255             /* max value of a "char" */
#endif	/* i386 || sparc */
#else
#if i386 || sparc
#define CHAR_MIN        SCHAR_MIN	/* min value of a "char" */
#define CHAR_MAX        SCHAR_MAX	/* max value of a "char" */
#else
#define CHAR_MIN        0               /* min value of a "char" */
#define CHAR_MAX        255             /* max value of a "char" */
#endif	/* i386 || sparc */
#endif	/* __STDC__ */

#define SHRT_MIN        (-32768)        /* min value of a "short int" */
#define SHRT_MAX        32767           /* max value of a "short int" */
#define USHRT_MAX       65535		/* max value of an "unsigned short int" */
#define INT_MIN         (-2147483647-1)     /* min value of an "int" */
#define INT_MAX         2147483647      /* max value of an "int" */
#define UINT_MAX        4294967295	/* max value of an "unsigned int" */
#define LONG_MIN        (-2147483647-1)		/* min value of a "long int" */
#define LONG_MAX        2147483647      /* max value of a "long int" */
#define ULONG_MAX       4294967295 	/* max value of an "unsigned long int" */

#if __STDC__ - 0 == 0 || defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE)

#define	ARG_MAX		5120		/* max length of arguments to exec */
#define	LINK_MAX	1000		/* max # of links to a single file */

#ifndef MAX_CANON
#define MAX_CANON	256		/* max bytes in a line for canoical proccessing */
#endif

#ifndef MAX_INPUT
#define	MAX_INPUT	512		/* max size of a char input buffer */
#endif

#define NGROUPS_MAX	16		/* max number of groups for a user */


#ifndef PATH_MAX
#define	PATH_MAX	1024		/* max # of characters in a path name */
#endif

#if defined(__STDC__)
#if #machine(u3b15)
#define	PIPE_BUF	4096		/* max # bytes atomic in write to a pipe */
#else
#define	PIPE_BUF	5120		/* max # bytes atomic in write to a pipe */
#endif 	/* u3b15 */
#else	/* not __STDC__ */
#if u3b15
#define	PIPE_BUF	4096		/* max # bytes atomic in write to a pipe */
#else
#define	PIPE_BUF	5120		/* max # bytes atomic in write to a pipe */
#endif 	/* u3b15 */
#endif	/* __STDC__ */

#ifndef TMP_MAX
#define TMP_MAX		17576	/* 26 * 26 * 26 */
#endif

/* POSIX conformant definitions - An implementation may define
** other symbols which reflect the actual implementation. Alternate
** definitions may not be as restrictive as the POSIX definitions.
*/

#define _POSIX_ARG_MAX		4096
#define _POSIX_CHILD_MAX	   6
#define _POSIX_LINK_MAX		   8
#define _POSIX_MAX_CANON	 255
#define _POSIX_MAX_INPUT	 255
#define _POSIX_NAME_MAX		  14
#define _POSIX_NGROUPS_MAX	   0
#define _POSIX_OPEN_MAX		  16
#define _POSIX_PATH_MAX		 255
#define _POSIX_PIPE_BUF		 512

#endif

#if (__STDC__ - 0 == 0 && !defined(_POSIX_SOURCE)) || defined(_XOPEN_SOURCE)

#define	PASS_MAX	8		/* max # of characters in a password */

#define	NL_ARGMAX	9		/* max value of "digit" in calls to the
					 * NLS printf() and scanf() */
#define	NL_LANGMAX	14		/* max # of bytes in a LANG name */
#define	NL_MSGMAX	32767		/* max message number */
#define	NL_NMAX		1		/* max # of bytes in N-to-1 mapping characters */
#define	NL_SETMAX	255		/* max set number */
#define	NL_TEXTMAX	255		/* max set number */
#define	NZERO		20		/* default process priority */

#define	WORD_BIT	32		/* # of bits in a "word" or "int" */
#define	LONG_BIT	32		/* # of bits in a "long" */

#define	DBL_DIG		15		/* digits of precision of a "double" */
#define	DBL_MAX		1.7976931348623157E+308  /* max decimal value of a "double"*/
#define	DBL_MIN		2.2250738585072014E-308  /* min decimal value of a "double"*/
#define	FLT_DIG		6		/* digits of precision of a "float" */
#define	FLT_MAX		3.40282347E+38F /* max decimal value of a "float" */
#define	FLT_MIN		1.17549435E-38F /* min decimal value of a "float" */

#endif

#if __STDC__ - 0 == 0 && !defined(_POSIX_SOURCE) && !defined(_XOPEN_SOURCE)

#define	FCHR_MAX	1048576		/* max size of a file in bytes */
#define	PID_MAX		30000		/* max value for a process ID */
#define	CHILD_MAX	25		/* max # of processes per user id */
#define	NAME_MAX	14		/* max # of characters in a file name */

#ifndef OPEN_MAX
#if i386
#define	OPEN_MAX	60		/* max # of files a process can have open */
#else
#define	OPEN_MAX	20		/* max # of files a process can have open */
#endif
#endif

#if defined(__STDC__)
#if #machine(u3b15)
#define	PIPE_MAX	4096		/* max # bytes written to a pipe in a write */
#else
#define	PIPE_MAX	5120		/* max # bytes written to a pipe in a write */
#endif 	/* u3b15 */
#else	/* not __STDC__ */
#if u3b15
#define	PIPE_MAX	4096		/* max # bytes written to a pipe in a write */
#else
#define	PIPE_MAX	5120		/* max # bytes written to a pipe in a write */
#endif 	/* u3b15 */
#endif	/* __STDC__ */

#define	STD_BLK		1024		/* # bytes in a physical I/O block */
#define	UID_MAX		60002		/* max value for a user or group ID */
#define	USI_MAX		4294967295	/* max decimal value of an "unsigned" */
#define	SYSPID_MAX	1		/* max pid of system processes */

#if !defined(_STYPES)
#define SYS_NMLN	257	/* 4.0 size of utsname elements */
				/* also defined in sys/utsname.h */
#else
#define SYS_NMLN	9	/* old size of utsname elements */
#endif	/* _STYPES */

#ifndef CLK_TCK
#define CLK_TCK	_sysconf(3)	/* 3B2 clock ticks per second */
				/* 3 is _SC_CLK_TCK */
#endif

#define LOGNAME_MAX	8		/* max # of characters in a login name */

#endif

#endif	/* _LIMITS_H */
