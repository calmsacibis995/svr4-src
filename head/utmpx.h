/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head:utmpx.h	1.4"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

#ifndef _UTMPX_H
#define _UTMPX_H

#include <sys/types.h>
#include <sys/time.h>
#include <utmp.h>

#define	UTMPX_FILE	"/var/adm/utmpx"
#define	WTMPX_FILE	"/var/adm/wtmpx"

#define	ut_name	ut_user
#define ut_xtime ut_tv.tv_sec

struct utmpx
  {
	char	ut_user[32];		/* user login name */
	char	ut_id[4]; 		/* inittab id */
	char	ut_line[32];		/* device name (console, lnxx) */
	pid_t	ut_pid;			/* process id */
	short	ut_type; 		/* type of entry */
	struct exit_status ut_exit;     /* process termination/exit status */
	struct timeval ut_tv;		/* time entry was made */
	long	ut_session;		/* session ID, used for windowing */
	long	pad[5];			/* reserved for future use */
	short	ut_syslen;		/* significant length of ut_host */
					/*   including terminating null */
	char	ut_host[257];		/* remote host name */
  } ;

/*	Definitions for ut_type						*/

#define	EMPTY		0
#define	RUN_LVL		1
#define	BOOT_TIME	2
#define	OLD_TIME	3
#define	NEW_TIME	4
#define	INIT_PROCESS	5	/* Process spawned by "init" */
#define	LOGIN_PROCESS	6	/* A "getty" process waiting for login */
#define	USER_PROCESS	7	/* A user process */
#define	DEAD_PROCESS	8
#define	ACCOUNTING	9

#define	UTMAXTYPE	ACCOUNTING	/* Largest legal value of ut_type */

/*	Special strings or formats used in the "ut_line" field when	*/
/*	accounting for something other than a process.			*/
/*	No string for the ut_line field can be more than 11 chars +	*/
/*	a NULL in length.						*/

#define	RUNLVL_MSG	"run-level %c"
#define	BOOT_MSG	"system boot"
#define	OTIME_MSG	"old time"
#define	NTIME_MSG	"new time"
#define MOD_WIN		10

#if defined(__STDC__)
extern void endutxent(void);
extern struct utmpx *getutxent(void);
extern struct utmpx *getutxid(const struct utmpx *);
extern struct utmpx *getutxline(const struct utmpx *);
extern struct utmpx *pututxline(const struct utmpx *); 
extern void setutxent(void);
extern int utmpxname(const char *);
extern struct utmpx *makeutx(const struct utmpx *);
extern struct utmpx *modutx(const struct utmpx *);
extern void getutmp(const struct utmpx *, struct utmp *);
extern void getutmpx(const struct utmp *, struct utmpx *);
extern void updwtmp(const char *, struct utmp *);
extern void updwtmpx(const char *, struct utmpx *);
#else
extern void endutxent();
extern struct utmpx *getutxent();
extern struct utmpx *getutxid();
extern struct utmpx *getutxline();
extern struct utmpx *pututxline(); 
extern void setutxent();
extern int utmpxname();
extern struct utmpx *makeutx();
extern struct utmpx *modutx();
extern void getutmp();
extern void getutmpx();
extern void updwtmp();
extern void updwtmpx();
#endif

#endif /* _UTMPX_H */
