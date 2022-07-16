/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_IPC_H
#define _SYS_IPC_H

#ident	"@(#)head.sys:sys/ipc.h	11.10.5.1"
/* Common IPC Access Structure */

/* The kernel supports both the SVR3 ipc_perm and expanded ipc_perm
** structures simultaneously.
*/

/* Applications that read /dev/mem must be built like the kernel. A new
** symbol "_KMEMUSER" is defined for this purpose.
*/

#if defined(_KERNEL) || defined(_KMEMUSER)
/* SVR3 ipc_perm structure */
struct o_ipc_perm {
	o_uid_t	uid;	/* owner's user id */
	o_gid_t	gid;	/* owner's group id */
	o_uid_t	cuid;	/* creator's user id */
	o_gid_t	cgid;	/* creator's group id */
	o_mode_t mode;	/* access modes */
	ushort	seq;	/* slot usage sequence number */
	key_t	key;	/* key */
};
/* expanded ipc_perm structure */
struct ipc_perm {
	uid_t	uid;	/* owner's user id */
	gid_t	gid;	/* owner's group id */
	uid_t	cuid;	/* creator's user id */
	gid_t	cgid;	/* creator's group id */
	mode_t	mode;	/* access modes */
	ulong	seq;	/* slot usage sequence number */
	key_t	key;	/* key */
	long	pad[4]; /* reserve area */
};
#else	/* user definition */

struct ipc_perm {
#if !defined(_STYPES)
/* maps to kernel ipc_perm structure */
	uid_t	uid;	/* owner's user id */
	gid_t	gid;	/* owner's group id */
	uid_t	cuid;	/* creator's user id */
	gid_t	cgid;	/* creator's group id */
	mode_t	mode;	/* access modes */
	ulong	seq;	/* slot usage sequence number */
	key_t	key;	/* key */
	long	pad[4];	/* reserve area */
#else
	o_uid_t	uid;	/* owner's user id */
	o_gid_t	gid;	/* owner's group id */
	o_uid_t	cuid;	/* creator's user id */
	o_gid_t	cgid;	/* creator's group id */
	o_mode_t mode;	/* access modes */
	ushort	seq;	/* slot usage sequence number */
	key_t	key;	/* key */
#endif	/* !defined(_STYPES) */
};
#endif	/* defined(_KERNEL) */

/* Common IPC Definitions. */
/* Mode bits. */
#define	IPC_ALLOC	0100000		/* entry currently allocated */
#define	IPC_CREAT	0001000		/* create entry if key doesn't exist */
#define	IPC_EXCL	0002000		/* fail if key exists */
#define	IPC_NOWAIT	0004000		/* error if request must wait */

/* Keys. */
#define	IPC_PRIVATE	(key_t)0	/* private key */

/* Control Commands. */

#if defined(_KERNEL) || defined(_KMEMUSER)
/* Command values for EFT definition */
#define IPC_RMID	10	/* remove identifier */
#define	IPC_SET		11	/* set options */
#define	IPC_STAT	12	/* get options */

	/* For compatibility */
#define	IPC_O_RMID	0	/* remove identifier */
#define	IPC_O_SET	1	/* set options */
#define	IPC_O_STAT	2	/* get options */

#else	/* user definition */

#if !defined(_STYPES)

	/* EFT definition */
#define	IPC_RMID	10	/* remove identifier */
#define	IPC_SET		11	/* set options */
#define	IPC_STAT	12	/* get options */
#else	/* compatibility mode - NON EFT  definition */
#define	IPC_RMID	0	/* remove identifier */
#define	IPC_SET		1	/* set options */
#define	IPC_STAT	2	/* get options */

#endif	/* !defined(_STYPES) */
#endif	/* defined(_KERNEL) */

#if defined(__STDC__) && !defined(_KERNEL)
key_t ftok(const char *, int);
#endif

#endif	/* _SYS_IPC_H */
