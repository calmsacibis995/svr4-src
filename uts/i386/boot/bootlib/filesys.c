/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)boot:boot/bootlib/filesys.c	1.1.3.2"

#include "../sys/boot.h"
#include "../sys/libfm.h"
#include "../sys/inode.h"
#include "sys/ino.h"
#include "sys/alttbl.h"

/*
 * Generic boot filesystem routines.
 */

#include	"sys/types.h"
#include	"sys/elog.h"
#include	"sys/iobuf.h"
#include	"sys/vtoc.h"
#include	"sys/vnode.h"
#include	"sys/vfs.h"
#include	"sys/fs/bfs.h"

off_t 	fd;

extern	bfstyp_t	boot_fs_type;
extern  off_t 	bfsopen();
extern  int	bfsread();
/*
 * open a file 
 * input 
 *	fsdelta (file system offset S5 or BFS)
 *	pathname of the file
 *	adddress of boot attribute 
 * output
 * 	In BFS a fd is the byte offset of the file on the disk.
 *
 *	In S5 1K, global in inode struct is initialized; subsequent read
 *	uses in.
 * error
 *	file is not found:  fd = 0;
 *	file is too big:    fd = -1;  (S5 only)
 */


off_t
open(fname)
register char *fname;
{
	register int i;

	switch (boot_fs_type) {

	case s5: 
		i = biget (bnami(ROOTINO, fname));
		break;
	case BFS:
		if ((i = fd = bfsopen(fname)) > 0)
			debug(printf("BFS type open: %s\n", fname));
		else
			debug(printf("BFS type open failed: %s\n", fname));
		break;
	default:
		printf("open: Unknown Boot file system type\n");
		break;
	}
	return ((off_t) i);
}

/*
 *  read the open file returns number of bytes read
 *  input
 *	fd: for BFS, for S5 use in (global inode).
 *	foffset: file offset in file
 *	buf: buffer for reading
 *	numbytes:  number of bytes to read
 *	kbchk:	386 added, whether to abort by kbd interrupt during rd
 *  output
 *	If BFS: buffer is filled, from BFS
 * 	If S51K:  gbuf (global buffer) is used to fill
 *  error
 *	actual bytes transferred is returned, 0 if EOF.
 */


int
read(foffset,buffer,buffer_sel,nbytes)
register off_t	foffset;
register char	*buffer;
register ushort  buffer_sel;
register int	nbytes;
{

	switch (boot_fs_type) {
	case s5: 
		return((int)breadi(foffset, buffer, buffer_sel, nbytes));
		break;	/* NOT REACHED */

	case BFS:
		return (bfsread(foffset, buffer, buffer_sel, nbytes));
		break;	/* NOT REACHED */
	default:
		printf("read: Unknown Boot file system type\n");
		return -1;
		break;	/* NOT REACHED */
	}
}
