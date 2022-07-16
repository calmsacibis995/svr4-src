/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_SAD_H
#define _SYS_SAD_H

#ident	"@(#)head.sys:sys/sad.h	1.5.5.1"

/*
 * Streams Administrative Driver
 */

/*
 *  ioctl defines
 */
#define	SADIOC		('D'<<8)
#define SAD_SAP		(SADIOC|01)	/* set autopush */
#define SAD_GAP		(SADIOC|02)	/* get autopush */
#define SAD_VML		(SADIOC|03)	/* validate module list */

/*
 * Device naming and numbering conventions.
 */
#define USERDEV "/dev/sad/user"
#define ADMINDEV "/dev/sad/admin"

#define USRMIN 0
#define ADMMIN 1

/*
 * The maximum modules you can push on a stream using
 * the autopush feature.  This should be less than NSTRPUSH.
 */
#define MAXAPUSH	8

/*
 * autopush info common to user and kernel
 */
struct apcommon {
	uint	apc_cmd;		/* command (see below) */
	long	apc_major;		/* major # of device */
	long	apc_minor;		/* minor # of device */
	long	apc_lastminor;		/* last minor for range */
	uint	apc_npush;		/* number of modules to push */
};

/*
 * ap_cmd: various flavors of autopush
 */
#define SAP_CLEAR	0		/* remove configuration list */
#define SAP_ONE		1		/* configure one minor device */
#define SAP_RANGE	2		/* configure range of minor devices */
#define SAP_ALL		3		/* configure all minor devices */

/*
 * format for autopush ioctls
 */
struct strapush {
	struct apcommon sap_common;				/* see above */
	char		sap_list[MAXAPUSH][FMNAMESZ + 1];	/* module list */
};

#define sap_cmd		sap_common.apc_cmd
#define sap_major	sap_common.apc_major
#define sap_minor	sap_common.apc_minor
#define sap_lastminor	sap_common.apc_lastminor
#define sap_npush	sap_common.apc_npush

#ifdef _KERNEL

/*
 * state values for ioctls
 */
#define GETSTRUCT	1
#define GETRESULT	2
#define GETLIST		3

struct saddev {
	queue_t	*sa_qp;		/* pointer to read queue */
	caddr_t	 sa_addr;	/* saved address for copyout */
	int	 sa_flags;	/* see below */
};

/*
 * values for saddev flags field.
 */
#define SADPRIV		0x01

/*
 * Module Autopush Cache
 */
struct autopush {
	struct autopush	*ap_nextp;		/* next on list */
	int		 ap_flags;		/* see below */
	struct apcommon  ap_common;		/* see above */
	ushort		 ap_list[MAXAPUSH];	/* list of modules to push */
						/* (indices into fmodsw array) */
};

/*
 * The command issued by the user ultimately becomes
 * the type of the autopush entry.  Therefore, occurrences of
 * "type" in the code refer to an existing autopush entry.
 * Occurrences of "cmd" in the code refer to the command the
 * user is currently trying to complete.  types and cmds take
 * on the same values.
 */
#define ap_type		ap_common.apc_cmd
#define ap_major	ap_common.apc_major
#define ap_minor	ap_common.apc_minor
#define ap_lastminor	ap_common.apc_lastminor
#define ap_npush	ap_common.apc_npush

/*
 * autopush flag values
 */
#define APFREE	0x00	/* free */
#define APUSED	0x01	/* used */
#define APHASH	0x02	/* on hash list */

/*
 * hash function for cache
 */
#define strphash(maj)	strpcache[(((int)maj)&strpmask)]

extern struct saddev saddev[];		/* sad device array */
extern int sadcnt;			/* number of sad devices */
extern struct autopush autopush[];	/* autopush data array */
extern int nautopush;			/* maximum number of autopushable devices */
extern struct autopush *strpfreep;	/* autopush freelist */
extern struct autopush *strpcache[];	/* autopush hash list */
extern int nstrphash;			/* size of hash list */
extern int strpmask;			/* used in hash function */
extern queue_t *sadminqp;		/* queue for STREAMS admin daemon */

#endif /* _KERNEL */

#endif	/* _SYS_SAD_H */
