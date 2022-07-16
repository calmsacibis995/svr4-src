/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

#ident	"@(#)head.sys:sys/types.h	11.33.3.1"


/* POSIX Extensions */

typedef	unsigned char	uchar_t;
typedef	unsigned short	ushort_t;
typedef	unsigned int	uint_t;
typedef	unsigned long	ulong_t;


/* For BSD compatibility */
typedef char *		addr_t; /* ?<core address> type */

typedef	char *		caddr_t;	/* ?<core address> type */
typedef	long		daddr_t;	/* <disk address> type */
typedef	long		off_t;		/* ?<offset> type */
typedef	short		cnt_t;		/* ?<count> type */
typedef struct _label { int val[6]; } label_t;
typedef	ulong_t paddr_t;	/* <physical address> type */
typedef	uchar_t use_t;		/* use count for swap.  */
typedef	short		sysid_t;
typedef	short		index_t;
typedef	short		lock_t;		/* lock work for busy wait */
typedef enum boolean { B_FALSE, B_TRUE } boolean_t;
/*
 * New type from XENIX
 */
typedef char *      faddr_t;    /* same as caddr_t for 8086/386 */


typedef ulong_t k_sigset_t;	/* kernel signal set type */
typedef ulong_t k_fltset_t;	/* kernel fault set type */

/*
 * The following type is for various kinds of identifiers.  The
 * actual type must be the same for all since some system calls
 * (such as sigsend) take arguments that may be any of these
 * types.  The enumeration type idtype_t defined in sys/procset.h
 * is used to indicate what type of id is being specified.
 */

typedef long		id_t;		/* A process id,	*/
					/* process group id,	*/
					/* session id, 		*/
					/* scheduling class id,	*/
					/* user id, or group id.*/


/* Typedefs for dev_t components */

typedef ulong_t	major_t;	/* major part of device number */
typedef ulong_t	minor_t;	/* minor part of device number */


/*
 * For compatilbility reasons the following typedefs (prefixed o_) 
 * can't grow regardless of the EFT definition. Although,
 * applications should not explicitly use these typedefs
 * they may be included via a system header definition.
 * WARNING: These typedefs may be removed in a future
 * release. 
 *		ex. the definitions in s5inode.h remain small 
 *			to preserve compatibility in the S5
 *			file system type.
 */
typedef	ushort_t o_mode_t;		/* old file attribute type */
typedef short	o_dev_t;		/* old device type	*/
typedef	ushort_t o_uid_t;		/* old UID type		*/
typedef	o_uid_t	o_gid_t;		/* old GID type		*/
typedef	short	o_nlink_t;		/* old file link type	*/
typedef short	o_pid_t;		/* old process id type */
typedef ushort_t o_ino_t;		/* old inode type	*/


/* POSIX and XOPEN Declarations */

typedef	int		key_t;		/* IPC key type */
typedef	ulong_t	mode_t;			/* file attribute type	*/
typedef	long	uid_t;			/* UID type		*/
typedef	uid_t	gid_t;			/* GID type		*/
typedef	ulong_t nlink_t;		/* file link type	*/
typedef ulong_t	dev_t;		/* expanded device type */
typedef ulong_t	ino_t;		/* expanded inode type */
typedef long	pid_t;			/* process id type	*/

#ifndef _SIZE_T
#define _SIZE_T
typedef	uint_t	size_t;		/* len param for string funcs */
#endif
typedef ushort_t      sel_t;      /* Selector type */

#ifndef _TIME_T
#define _TIME_T
typedef	long		time_t;		/* time of day in seconds */
#endif	/* END _TIME_T */

#ifndef _CLOCK_T
#define _CLOCK_T
typedef	long		clock_t; /* relative time in a specified resolution */
#endif	/* ifndef _CLOCK_T */


#if (defined(_KERNEL) || !defined(_POSIX_SOURCE))

typedef	struct { int r[1]; } *	physadr;
typedef	unsigned char	unchar;
typedef	unsigned short	ushort;
typedef	unsigned int	uint;
typedef	unsigned long	ulong;




#if defined(_KERNEL)

#define SHRT_MIN        -32768          /* min value of a "short int" */
#define SHRT_MAX        32767           /* max value of a "short int" */
#define USHRT_MAX       65535		/* max value of an "unsigned short int" */
#define INT_MIN         (-2147483647-1)     /* min value of an "int" */
#define INT_MAX         2147483647      /* max value of an "int" */
#define UINT_MAX        4294967295	/* max value of an "unsigned int" */
#define LONG_MIN        (-2147483647-1)		/* min value of a "long int" */
#define LONG_MAX        2147483647      /* max value of a "long int" */
#define ULONG_MAX       4294967295 	/* max value of an "unsigned long int" */

#endif	/* defined(_KERNEL) */


#define	P_MYPID	((pid_t)0)

/*
 * The following is the value of type id_t to use to indicate the
 * caller's current id.  See procset.h for the type idtype_t
 * which defines which kind of id is being specified.
 */

#define	P_MYID	(-1)
#define NOPID (pid_t)(-1)

#ifndef NODEV
#define NODEV (dev_t)(-1)
#endif

/*
 * A host identifier is used to uniquely define a particular node
 * on an rfs network.  Its type is as follows.
 */

typedef	long	hostid_t;

/*
 * The following value of type hostid_t is used to indicate the
 * current host.  The actual hostid for each host is in the
 * kernel global variable rfs_hostid.
 */

#define	P_MYHOSTID	(-1)

typedef unsigned char	u_char;
typedef unsigned short	u_short;
typedef unsigned int	u_int;
typedef unsigned long	u_long;
typedef struct _quad { long val[2]; } quad;	/* used by UFS */


/*
 * Nested include for BSD/sockets source compatibility.
 * (The select macros used to be defined here).
 */
#include <sys/select.h>

#endif /* END (defined(_KERNEL) || !defined(_POSIX_SOURCE)) */

/*
 * These were added to allow non-ANSI compilers to compile the system.
 */

#ifdef __STDC__

#ifndef _VOID
#define _VOID	void
#endif

	/* End of ANSI section */

#else

#ifndef _VOID
#define _VOID	char
#endif

#ifndef const
#define const
#endif

#ifndef volatile
#define volatile
#endif

#endif /* of non-ANSI section */

#endif	/* _SYS_TYPES_H */
