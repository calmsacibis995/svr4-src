/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:rfs/du.h	1.3"

/*
 * Entry points for RFS into the SVR3.X system call protocol
 */

/* Entry points specific to the client */
extern int	du_lookup();
extern int	du_access();
extern int	du_create();
extern int	du_fcntl();
extern int	du_getattr();
extern int	du_mkdir();
extern int	du_open();
extern int	du_remove();
extern int	du_rename();
extern int	du_rmdir();
extern int	du_setattr();
extern int	du_fstatfs();

/* Entry points specific to the server */
extern int	dusr_saccess();
extern int	dusr_chdirec();
extern int	dusr_chmod();
extern int	dusr_iupdate();
extern int	dusr_chown();
extern int	dusr_coredump();
extern int	dusr_creat();
extern int	dusr_exec();
extern int	dusr_fcntl();
extern int	dusr_fstat();
extern int	dusr_fstatfs();
extern int	dusr_link();
extern int	dusr_link1();
extern int	dusr_mkdir();
extern int	dusr_mknod();
extern int	dusr_open();
extern int	dusr_rmdir();
extern int	dusr_rmount();
extern int	dusr_seek();
extern int	dusr_stat();
extern int	dusr_statfs();
extern int	dusr_unlink();
extern int	dusr_utime();
