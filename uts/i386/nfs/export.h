/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _NFS_EXPORT_H
#define _NFS_EXPORT_H

#ident	"@(#)kern-nfs:export.h	1.1"

/*      @(#)export.h 1.7 88/08/19 SMI      */

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

/*
 * exported vfs flags.
 */

#define	EX_RDONLY	0x01	/* exported read only */
#define	EX_RDMOSTLY	0x02	/* exported read mostly */
#define	EX_RDWR		0x04	/* exported read-write */
#define	EX_EXCEPTIONS	0x08	/* exported with ``exceptions'' lists */
#define	EX_ALL		(EX_RDONLY | EX_RDMOSTLY | EX_RDWR | EX_EXCEPTIONS)

#define	EXMAXADDRS	256	/* max number in address list */
struct exaddrlist {
	unsigned naddrs;		/* number of addresses */
	struct netbuf *addrvec;		/* pointer to array of addresses */
	struct netbuf *addrmask;	/* mask of comparable bits of addrvec */
};

/*
 * Associated with AUTH_UNIX is an array of internet addresses
 * to check root permission.
 */
#define	EXMAXROOTADDRS	256		/* should be config option */
struct unixexport {
	struct exaddrlist rootaddrs;
};

/*
 * Associated with AUTH_DES is a list of network names to check
 * root permission, plus a time window to check for expired
 * credentials.
 */
#define	EXMAXROOTNAMES	256		/* should be config option */
struct desexport {
	unsigned nnames;
	char **rootnames;
	int window;
};


/*
 * The export information passed to exportfs()
 */
struct export {
	int		ex_flags;	/* flags */
	unsigned	ex_anon;	/* uid for unauthenticated requests */
	int		ex_auth;	/* switch */
	union {
		struct unixexport	exunix;		/* case AUTH_UNIX */
		struct desexport	exdes;		/* case AUTH_DES */
	} ex_u;
	struct exaddrlist ex_roaddrs;
	struct exaddrlist ex_rwaddrs;
};
#define	ex_des	ex_u.exdes
#define	ex_unix	ex_u.exunix

#ifdef	_KERNEL
/*
 * A node associated with an export entry on the list of exported
 * filesystems.
 */
struct exportinfo {
	struct export		exi_export;
	fsid_t			exi_fsid;
	struct fid		*exi_fid;
	struct exportinfo	*exi_next;
};
extern struct exportinfo *findexport();
#endif

#endif	/* _NFS_EXPORT_H */
