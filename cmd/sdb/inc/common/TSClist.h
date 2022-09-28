/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/TSClist.h	1.2"
#ifndef TSClist_h
#define TSClist_h

#include	"prioctl.h"

enum SC	{	indir_sc,
		exit_sc,
		fork_sc,
		read_sc,
		write_sc,
		open_sc,
		close_sc,
		wait_sc,
		creat_sc,
		link_sc,
		unlink_sc,
		exec_sc,
		chdir_sc,
		time_sc,
		mknod_sc,
		chmod_sc,
		chown_sc,
		brk_sc,
		stat_sc,
		lseek_sc,
		getpid_sc,
		mount_sc,
		umount_sc,
		setuid_sc,
		getuid_sc,
		stime_sc,
		ptrace_sc,
		alarm_sc,
		fstat_sc,
		pause_sc,
		utime_sc,
		stty_sc,
		gtty_sc,
		access_sc,
		nice_sc,
		statfs_sc,
		sync_sc,
		kill_sc,
		fstatfs_sc,
		setpgrp_sc,
		rename_sc,
		dup_sc,
		pipe_sc,
		times_sc,
		prof_sc,
		lock_sc,
		setgid_sc,
		getsid_sc,
		ssig_sc,
		msgsys_sc,
		sys3b_sc,
		sysacct_sc,
		shmsys_sc,
		semsys_sc,
		ioctl_sc,
		uadmin_sc,
		utssys_sc,
		fsync_sc,
		exece_sc,
		umask_sc,
		chroot_sc,
		fcntl_sc,
		ulimit_sc,
		advfs_sc,
		unadvfs_sc,
		rmount_sc,
		rumount_sc,
		rfstart_sc,
		rdebug_sc,
		rfstop_sc,
		rfsys_sc,
		rmdir_sc,
		mkdir_sc,
		getdents_sc,
		libattach_sc,
		libdetach_sc,
		sysfs_sc,
		getmsg_sc,
		putmsg_sc,
		poll_sc,
		lstat_sc,
		symlink_sc,
		readlink_sc,
		setgroups_sc,
		getgroups_sc,
		fchmod_sc,
		fchown_sc,
		sigprocmask_sc,
		sigsuspend_sc,
		sigaltstack_sc,
		sigaction_sc,
		sigpending_sc,
		sigret_sc,
		statvfs_sc,
		fstatvfs_sc,
		lastone
	};

class Assoccmds;

class TSClist {
	sysset_t	entrymask;
	sysset_t	exitmask;
	Assoccmds *	entryassoc[ lastone ];
	Assoccmds *	exitassoc[ lastone ];
public:
			TSClist();
			~TSClist();
	int		add( int, int = 0, Assoccmds * = 0 );
	int		remove( int, int = 0 );
	int		disable( int, int = 0 );
	int		enable( int, int = 0 );
	sysset_t *	tracemask( int = 0 );
	Assoccmds *	assoccmds( int, int = 0 );
};

#endif

// end of TSClist.h

