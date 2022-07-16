/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kdb:kdb/dbshow2.c	1.1.1.1"

/*
 * print various things
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/fs/ufs_inode.h"
#include "sys/sysmacros.h"
#include "sys/kdebugger.h"
#include "../db_as.h"

#define L(X) ((long)(X))

long findsymaddr();


db_puinode(_ip)
	as_addr_t	_ip;
{
	struct inode	inode_buf;
	register struct inode *ip = &inode_buf;
	static struct inode	*Ufs_inode;

	if (db_read(_ip, (char *)ip, sizeof(struct inode)) == -1)
		dberror("invalid address");

	if (Ufs_inode == NULL) {
		long	_Ufs_inode;

		if ((_Ufs_inode = findsymaddr("ufs_inode")) != 0L)
			Ufs_inode = *(struct inode **)_Ufs_inode;
	}
	if (Ufs_inode == NULL || _ip.a_as != AS_KVIRT)
		dbprintf( "ufs inode @ %x:\n", L(_ip.a_addr) );
	else
		dbprintf( "ufs inode %x @ %x:\n",
				(struct inode *)_ip.a_addr - Ufs_inode,
				L(_ip.a_addr) );

	dbprintf( "flag:    %8x   number:  %8x   dev: %d,%d   devvp: %x\n",
		L(ip->i_flag), L(ip->i_number),
		getemajor(ip->i_dev), geteminor(ip->i_dev),
		L(ip->i_devvp) );

	dbprintf(
	"links:   %8x   uid:     %8x   gid:     %8x   size:    %8x\n",
		L(ip->i_nlink), L(ip->i_uid), L(ip->i_gid), L(ip->i_size) );

	dbprintf( "mode:      %06o   gen:     %8x   blocks:  %8x\n",
		L(ip->i_mode), L(ip->i_gen), L(ip->i_blocks) );

	dbprintf(
	"fs:      %8x   dquot:   %8x   lock count:  %4x   owner:       %4x\n",
		L(ip->i_fs), L(ip->i_dquot), L(ip->i_count), L(ip->i_owner) );

	dbprintf(
	"map:     %8x   mapcnt:  %8x   nextr:   %8x   diroff:  %8x\n",
		L(ip->i_map), L(ip->i_mapcnt), L(ip->i_nextr), L(ip->i_diroff) );

	dbprintf( "db:      %8x   ib:      %8x\n",
		L(&((struct inode *)_ip.a_addr)->i_db[0]),
		L(&((struct inode *)_ip.a_addr)->i_ib[0]) );

	_ip.a_addr = (u_long)ITOV((struct inode *)_ip.a_addr);
	db_pvnode(_ip);
}
