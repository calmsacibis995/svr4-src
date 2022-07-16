/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_PRSYSTM_H
#define _SYS_PRSYSTM_H

#ident	"@(#)head.sys:sys/prsystm.h	1.4.3.1"

#include <sys/procfs.h>

#if defined(__STDC__)
extern void prawake(proc_t *);
extern void prinvalidate(struct user *);
extern void prgetpsinfo(proc_t *, struct prpsinfo *);
extern void prgetfpregs(proc_t *, fpregset_t *);
extern int prnsegs(proc_t *);
extern void prexit(proc_t *);
extern void prgetstatus(proc_t *, prstatus_t *);
#else
extern void prawake();
extern void prinvalidate();
extern void prgetpsinfo();
extern void prgetfpregs();
extern int prnsegs();
extern void prexit();
extern void prgetstatus();
#endif	/* __STDC__ */

#endif	/* _SYS_PRSYSTM_H */
