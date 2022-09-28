/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/oslevel.h	1.9"
#ifndef oslevel_h
#define oslevel_h

#include	"Itype.h"
#include	"prioctl.h"

struct Key {
	int	pid;
	int	fd;
};

// all /proc vs ptrace routines declared here ...
extern int		get_key( int, Key & );
extern Key		dispose_key( Key );
extern void		stop_self_on_exec();
extern int		stop_proc( Key, prstatus * );
extern int		kill_proc( Key );
extern int		wait_for( Key, prstatus & );
extern int		get_status( Key, prstatus & );
extern int		send_sig( Key, int );
extern int		clear_pend_sig( Key );
extern int		run_proc( Key, prrun &, int );
extern int		step_proc( Key, prrun &, int );
extern unsigned long	get_pc( Key, prstatus & );
extern unsigned long	get_ap( Key );
extern int		getfpset( Key, fpregset_t & );
extern int		catch_sigs( Key, sigset_t, prrun & );
extern int		get_bytes( Key, unsigned long, void *, int );
extern int		put_bytes( Key, unsigned long, void *, int );
extern int		live_proc( int );
extern int		open_object( Key, unsigned long );
extern Iaddr		get_sigreturn( Key );
extern char*            psargs( Key );
extern int              update_stack( Key, Iaddr &, Iaddr & );

#endif

// end of oslevel.h
