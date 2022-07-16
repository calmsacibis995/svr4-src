/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-os:mod.c	1.3"

#include <sys/types.h>
#include <sys/immu.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/fs/s5dir.h>
#include <sys/signal.h>
#include <sys/errno.h>
#include <sys/user.h>
#include <sys/cmn_err.h>

extern int eua_lim_ma;
static int cur_lim_user = 0;


int
enable_user_alloc(c)
register int c;
{
	switch (c) {
		case EUA_GET_LIM:
			return (eua_lim_ma);
		case EUA_GET_CUR:
			return (cur_lim_user);
		case EUA_REM_USER:	/* Remove a User */
			if (!(u.u_procp->p_user_license & PU_LIM_OK))
				return (0);
			if (!(u.u_procp->p_parent->p_user_license & PU_LIM_OK)) {
				if (cur_lim_user > 0) {
				    if (u.u_procp->p_user_license & PU_UUCP)
					u.u_procp->p_user_license &= ~PU_UUCP;
				    else
					cur_lim_user--;
				}
			}
			else if (u.u_procp->p_user_license & PU_LOGIN_PROC) {
				if (cur_lim_user > 0)
					cur_lim_user--;
			}
			u.u_procp->p_user_license = 0;
			return (0);
		case EUA_UUCP:		/* Enable uucp special */
			u.u_procp->p_user_license |=
					PU_LOGIN_PROC|PU_LIM_OK|PU_UUCP;
			return (0);
		case EUA_FORK:		/* Enable user from fork() */
			if (cur_lim_user >= eua_lim_ma)
				return (-1);
			cur_lim_user++;
			return (0);
		case EUA_ADD_USER:	/* Add a User */
			if (u.u_procp->p_user_license & PU_LIM_OK)
				return (0);
			u.u_procp->p_user_license |= PU_LOGIN_PROC|PU_LIM_OK;
			cur_lim_user++;
/*
 * EUA_ADD_USER comes from login(1M).  So if we're going in via login and
 * we're not the process group leader, then we're probably running under a
 * server.  With starlan it's ttysrv.  Therefore, mark the parent process.
 *
 * When this kid dies, exit won't call de-alloc because we're not the process group
 * leader.  ttysrv is running as root.  It has to be marked so that it's decremented
 * when it dies.
 */
			if (u.u_procp->p_pgrp == u.u_procp->p_parent->p_pgrp)  {
				u.u_procp->p_parent->p_user_license
						|= PU_LOGIN_PROC|PU_LIM_OK;
				u.u_procp->p_user_license &= ~PU_LOGIN_PROC;
			}
			return (0);
		}
	return (-1);
}
