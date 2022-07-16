/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)master:rt/stubs.c	1.3"

#include <sys/errno.h>

rt_init() {}
rt_admin() {return (EFAULT);}
rt_enterclass() {return (EPERM);}
rt_exitclass() {}
rt_fork() {return (0);}
rt_getclinfo() {return (0);}
rt_getglobpri() {}
rt_nullclass() {}
rt_parmsget() {}
rt_parmsin() {return (EINVAL);}
rt_parmsout() {return (0);}
rt_parmsset() {return (EPERM);}
rt_proccmp() {return (0);}
rt_setdq() {}
rt_setrun() {}
rt_sleep() {}
rt_stop() {}
rt_swapin() {}
rt_swapout() {}
rt_tick() {}
rt_wakeup() {}
