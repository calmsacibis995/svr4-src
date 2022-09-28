/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nfs.cmds:nfs/share/issubdir.c	1.1.2.1"

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *	PROPRIETARY NOTICE (Combined)
 *
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 *
 *
 *
 *     Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 *
 *  (c) 1986,1987,1988.1989  Sun Microsystems, Inc
 *  (c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *            All rights reserved.
 *
 */
/*
 * Subdirectory detection: needed by exportfs and rpc.mountd.
 * The above programs call issubdir() frequently, so we make
 * it fast by caching the results of stat().
 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <string.h>

#define MAXSTATS 20	/* maximum number of stat()'s to save */

#define inoeq(ino1, ino2)	((ino1) == (ino2))
#define deveq(dev1, dev2)	((dev1) == (dev2))

/*
 * dir1 is a subdirectory of dir2 within the same filesystem if
 *     (a) dir1 is identical to dir2
 *     (b) dir1's parent is dir2
 */
issubdir(dir1, dir2)
	char *dir1;
	char *dir2;
{
	struct stat st;
	char *p;
	int index;
	dev_t parent_dev;
	ino_t parent_ino;

	static dev_t child_dev;
	static ino_t child_ino[MAXSTATS];
	static int valid;
	static char childdir[MAXPATHLEN];

	/*
	 * Get parent directory info
	 */
	if (stat(dir2, &st) < 0) {
		return(0);
	}
	parent_dev = st.st_dev;
	parent_ino = st.st_ino;

	if (strcmp(childdir, dir1) != 0) {
		/*
	 	 * Not in cache: get child directory info
		 */
		p = strcpy(childdir, dir1) + strlen(dir1);
		index = 0;
		valid = 0;
		for (;;) {
			if (stat(childdir, &st) < 0) {
				childdir[0] = 0;	/* invalidate cache */
				return (0);
			}
			if (index == 0) {
				child_dev = st.st_dev;
			}
			if (index > 0 && 
			    (child_dev != st.st_dev ||
			     inoeq(child_ino[index - 1], st.st_ino))) {
				/*
				 * Hit root: done
				 */
				break;
			}
			child_ino[index++] = st.st_ino;
			if (st.st_mode & S_IFDIR) {
				p = strcpy(p, "/..") + 3;
			} else {
				p = strrchr(childdir, '/');
				if (p == NULL) {
					p = strcpy(childdir, ".") + 1;
				} else {
					while (&p[1] >childdir && p[1] == '/') {
						p--;
					}
					*p = 0;
				}
			}
		}
		valid = index;
		(void) strcpy(childdir, dir1);
	}

	/*
	 * Perform the test
	 */
	if (!deveq(parent_dev, child_dev)) {
		return (0);
	}
	for (index = 0; index < valid; index++) {
		if (inoeq(child_ino[index], parent_ino)) {
			return (1);
		}
	}
	return (0);
}
