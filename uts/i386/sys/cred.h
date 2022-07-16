/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_CRED_H
#define _SYS_CRED_H

#ident	"@(#)head.sys:sys/cred.h	1.8.3.1"
/*
 * User credentials.  The size of the cr_groups[] array is configurable
 * but is the same (ngroups_max) for all cred structures; cr_ngroups
 * records the number of elements currently in use, not the array size.
 */

typedef struct cred {
	ushort	cr_ref;			/* reference count */
	ushort	cr_ngroups;		/* number of groups in cr_groups */
	uid_t	cr_uid;			/* effective user id */
	gid_t	cr_gid;			/* effective group id */
	uid_t	cr_ruid;		/* real user id */
	gid_t	cr_rgid;		/* real group id */
	uid_t	cr_suid;		/* "saved" user id (from exec) */
	gid_t	cr_sgid;		/* "saved" group id (from exec) */
	gid_t	cr_groups[1];		/* supplementary group list */
} cred_t;

#ifdef _KERNEL

#define	crhold(cr)	(cr)->cr_ref++

extern int ngroups_max;

#if defined(__STDC__)

extern void cred_init(void);
extern void crfree(cred_t *);
extern cred_t *crget(void);
extern cred_t *crcopy(cred_t *);
extern cred_t *crdup(cred_t *);
extern cred_t *crgetcred(void);
extern int suser(cred_t *);
extern int groupmember(gid_t, cred_t *);
extern int hasprocperm(cred_t *, cred_t *);

#else

extern void cred_init();
extern void crfree();
extern cred_t *crget();
extern cred_t *crcopy();
extern cred_t *crdup();
extern cred_t *crgetcred();
extern int suser();
extern int groupmember();
extern int hasprocperm();


#endif	/* __STDC */


#endif

#endif	/* _SYS_CRED_H */
