/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:fs_subr.h	1.2"
#ifndef	_FS_FS_SUBR_H
#define _FS_FS_SUBR_H


/*
 * Utilities shared among file system implementations.
 */
extern int	fs_nosys();
extern int	fs_sync();
extern void	fs_rwlock();
extern void	fs_rwunlock();
extern int	fs_cmp();
extern int	fs_frlock();
extern int	fs_setfl();
extern int	fs_poll();
extern int	fs_vcode();
extern int	fs_pathconf();

#endif /* _FS_FS_SUBR_H */
