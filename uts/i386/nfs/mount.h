/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _NFS_MOUNT_H
#define _NFS_MOUNT_H

#ident	"@(#)kern-nfs:mount.h	1.1"

/*
 *  		PROPRIETARY NOTICE (Combined)
 *  
 *  This source code is unpublished proprietary information
 *  constituting, or derived under license from AT&T's Unix(r) System V.
 *  
 *  
 *  
 *  		Copyright Notice 
 *  
 *  Notice of copyright on this source code product does not indicate 
 *  publication.
 *  
 *  	(c) 1986,1987,1988,1989  Sun Microsystems, Inc.
 *  	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *  	          All rights reserved.
 */

#ifdef NFSCLIENT

struct nfs_args {
	struct netbuf		*addr;		/* file server address */
	struct netbuf		*syncaddr;	/* secure NFS time sync address */
	struct knetconfig	*knconf;	/* transport knetconfig struct */
	char			*hostname;	/* server's hostname */
	char			*netname;	/* server's netname */
	caddr_t			fh;		/* File handle to be mounted */
	int			flags;		/* flags */
	int			wsize;		/* write size in bytes */
	int			rsize;		/* read size in bytes */
	int			timeo;		/* initial timeout in .1 secs */
	int			retrans;	/* times to retry send */
	int			acregmin;	/* attr cache file min secs */
	int			acregmax;	/* attr cache file max secs */
	int			acdirmin;	/* attr cache dir min secs */
	int			acdirmax;	/* attr cache dir max secs */
};

/*
 * NFS mount option flags
 */
#define	NFSMNT_SOFT		0x001	/* soft mount (hard is default) */
#define	NFSMNT_WSIZE		0x002	/* set write size */
#define	NFSMNT_RSIZE		0x004	/* set read size */
#define	NFSMNT_TIMEO		0x008	/* set initial timeout */
#define	NFSMNT_RETRANS		0x010	/* set number of request retrys */
#define	NFSMNT_HOSTNAME		0x020	/* set hostname for error printf */
#define	NFSMNT_INT		0x040	/* allow interrupts on hard mount */
#define	NFSMNT_NOAC		0x080	/* don't cache attributes */
#define	NFSMNT_ACREGMIN		0x0100	/* set min secs for file attr cache */
#define	NFSMNT_ACREGMAX		0x0200	/* set max secs for file attr cache */
#define	NFSMNT_ACDIRMIN		0x0400	/* set min secs for dir attr cache */
#define	NFSMNT_ACDIRMAX		0x0800	/* set max secs for dir attr cache */
#define	NFSMNT_SECURE		0x1000	/* secure mount */
#define	NFSMNT_NOCTO		0x2000	/* no close-to-open consistency */
#define	NFSMNT_KNCONF		0x4000	/* transport's knetconfig structure */
#define	NFSMNT_GRPID		0x8000	/* System V-style gid inheritance */
#define	NFSMNT_RPCTIMESYNC	0x10000	/* use RPC to do secure NFS time sync */
#endif	/* NFSCLIENT */

#endif	/* _NFS_MOUNT_H */
