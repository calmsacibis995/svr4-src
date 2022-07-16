/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:nfs/nfs_common.c	1.3"

/*      @(#)nfs_common.c 1.4 88/09/19 SMI      */

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

#include <sys/errno.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/vfs.h>
#include <sys/vnode.h>
#include <rpc/types.h>
#include <nfs/nfs.h>
#include <sys/mode.h>


#ifdef NFSDEBUG
int nfsdebug = 2;
#endif


/*
 * General utilities
 */

/*
 * Returns the prefered transfer size in bytes based on
 * what network interfaces are available.
 */
nfstsize()
{
#ifdef	SYSV
	/*
	 *	Unfortunately, the networking architecture in System V doesn't
	 *	allow us to ask this question of the network interfaces.
	 */
	return (NFS_MAXDATA);
#else
	register struct ifnet *ifp;

	for (ifp = ifnet; ifp; ifp = ifp->if_next) {
		if (ifp->if_name[0] == 'e' && ifp->if_name[1] == 'c') {
#ifdef NFSDEBUG
			printf("nfstsize: %d\n", ECTSIZE);
#endif
			return (ECTSIZE);
		}
	}
#ifdef NFSDEBUG
	printf("nfstsize: %d\n", IETSIZE);
#endif
	return (IETSIZE);
#endif
}

/*
 * Utilities used by both client and server.
 */

void
vattr_to_nattr(vap, na)
	register struct vattr *vap;
	register struct nfsfattr *na;
{

	na->na_type = (enum nfsftype)vap->va_type;

	if (vap->va_mode == (unsigned short) -1)
                na->na_mode = (unsigned long) -1;
        else
                na->na_mode = VTTOIF(vap->va_type) | vap->va_mode;

        if (vap->va_uid == (unsigned short) -1)
                na->na_uid = (unsigned long) -1;
        else
                na->na_uid = vap->va_uid;
 
        if (vap->va_gid == (unsigned short) -1)
                na->na_gid = (unsigned long) -1;
        else
                na->na_gid = vap->va_gid;

	na->na_fsid = vap->va_fsid;
	na->na_nodeid = vap->va_nodeid;
	na->na_nlink = vap->va_nlink;
	na->na_size = vap->va_size;
	na->na_atime.tv_sec  = vap->va_atime.tv_sec;
	na->na_atime.tv_usec = vap->va_atime.tv_nsec/1000;
	na->na_mtime.tv_sec  = vap->va_mtime.tv_sec;
	na->na_mtime.tv_usec = vap->va_mtime.tv_nsec/1000;
	na->na_ctime.tv_sec  = vap->va_ctime.tv_sec;
	na->na_ctime.tv_usec = vap->va_ctime.tv_nsec/1000;
	na->na_rdev = vap->va_rdev;
	na->na_blocks = vap->va_nblocks;
	na->na_blocksize = vap->va_blksize;

	/*
	 * This bit of ugliness is a *TEMPORARY* hack to preserve the
	 * over-the-wire protocols for named-pipe vnodes.  It remaps the
	 * VFIFO type to the special over-the-wire type. (see note in nfs.h)
	 *
	 * BUYER BEWARE:
	 *  If you are porting the NFS to a non-Sun server, you probably
	 *  don't want to include the following block of code.  The
	 *  over-the-wire special file types will be changing with the
	 *  NFS Protocol Revision.
	 */
	if (vap->va_type == VFIFO)
		NA_SETFIFO(na);
}

#ifdef NFSDEBUG
/*
 * Utilities used by both client and server
 * Standard levels:
 * 0) no debugging
 * 1) hard failures
 * 2) soft failures
 * 3) current test software
 * 4) main procedure entry points
 * 5) main procedure exit points
 * 6) utility procedure entry points
 * 7) utility procedure exit points
 * 8) obscure procedure entry points
 * 9) obscure procedure exit points
 * 10) random stuff
 * 11) all <= 1
 * 12) all <= 2
 * 13) all <= 3
 * ...
 */

/*VARARGS2*/
dprint(var, level, str, a1, a2, a3, a4, a5, a6, a7, a8, a9)
	int var;
	int level;
	char *str;
	int a1, a2, a3, a4, a5, a6, a7, a8, a9;
{
	if (var == level || (var > 10 && (var - 10) >= level))
		printf(str, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}
#endif
