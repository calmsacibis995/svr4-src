/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:nfs/nfs_aux.c	1.3"
#include "sys/types.h"
#include "sys/vfs.h"
#include "sys/kmem.h"
#include "sys/tiuser.h"

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
 * The policies and routines below guarantee that a unique device number
 * (as returned by the stat() system call) is associated with each mounted
 * filesystem and they must be adhered to by all filesystem types.
 * 	Local filesystems (i.e., those with associated Unix devices)
 * do not use the routines below.  Their device number is the device number
 * of the Unix device associated with the filesystem. The range
 * 0x0000 - 0x7fff is reserved for filesystems of this type.
 * 	Non-local filesystems use the range 0x8000-0xffff. For the major
 * device number, filesystem types which only require one major device number
 * for all mounts use their reserved number which is 0x80 + the index of
 * their filesystem type in the filesystem type table (vfs_conf.c). This
 * number may be obtained by calling the routine vfs_fixedmajor(). Filesystem
 * types requiring more than one major device number should obtain these
 * numbers via calls to vfs_getmajor() and release them via calls to
 * vfs_putmajor(). Minor device numbers are under the control of
 * individual filesystem types. Any filesystem types that wishes may
 * allocate and de-allocate minor device numbers using the routines
 * vfs_getnum() and vfs_putnum() and its own private minor device number map.
 */

#define	NBBY		8
#define	LOGBBY		3
#define	MAJOR_MIN	128
#define	vfsNVFS		(&vfssw[nfstype])

/*
 * Return the reserved major device number for this filesystem type
 * defined as its position in the filesystem type table.
 */
int
vfs_fixedmajor(vfsp)
	struct vfs* vfsp;
{
	register struct vfssw *vs;

	for (vs = vfssw; vs < vfsNVFS; vs++) {
		if (vs->vsw_vfsops == vfsp->vfs_op)
			break;
	}
	return ((vs - vfssw) + MAJOR_MIN);
}

/*
 * Set and return the first free position from the bitmap "map".
 * Return -1 if no position found.
 */
int
vfs_getnum(map, mapsize)
	register char *map;
	int mapsize;
{
	register int i;
	register char *mp;

	for (mp = map; mp < &map[mapsize]; mp++) {
		if (*mp != (char)0xff) {
			for (i=0; i < NBBY; i++) {
				if (!((*mp >> i) & 0x1)) {
					*mp |= (1 << i);
					return ((mp - map) * NBBY  + i);
				}
			}
		}
	}
	return (-1);
}

/*
 * Clear the designated position "n" in bitmap "map".
 */
void
vfs_putnum(map, n)
	register char *map;
	int n;
{

	if (n >= 0)
		map[n >> LOGBBY] &= ~(1 << (n - ((n >> LOGBBY) << LOGBBY)));
}

struct netbuf *
nfs_copyin_netbuf(from)
	struct netbuf *from;
{
	struct netbuf *addr;
	char *userbufptr;

	addr = kmem_alloc(sizeof (struct netbuf), KM_SLEEP);

	if (copyin((caddr_t) from, (caddr_t) addr, sizeof (struct netbuf))) {
		kmem_free((caddr_t) addr, sizeof (struct netbuf));
		return ((struct netbuf *) 0);
	}

	userbufptr = addr->buf;
	addr->buf = kmem_alloc(addr->len, KM_SLEEP);
	addr->maxlen = addr->len;
	if (copyin(userbufptr, addr->buf, addr->len)) {
		kmem_free((caddr_t) addr->buf, addr->len);
		kmem_free((caddr_t) addr, sizeof (struct netbuf));
		return ((struct netbuf *) 0);
	}

	return (addr);
}
