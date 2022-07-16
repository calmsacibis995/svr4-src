/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head:shadow.h	1.2"

#define PASSWD 		"/etc/passwd"
#define SHADOW		"/etc/shadow"
#define OPASSWD		"/etc/opasswd"
#define OSHADOW 	"/etc/oshadow"
#define PASSTEMP	"/etc/ptmp"
#define SHADTEMP	"/etc/stmp"

#define	DAY		(24L * 60 * 60) /* 1 day in seconds */
#define DAY_NOW		(long)time((long *)0) / DAY
			/* The above timezone variable is set by a call to
			   any ctime(3c) routine.  Programs using the DAY_NOW
			   macro must call one of the ctime routines, 
			   e.g. tzset(), BEFORE referencing DAY_NOW */

/* The spwd structure is used in the retreval of information from
   /etc/shadow.  It is used by routines in the libos library */

struct spwd {
	char *sp_namp ; /* user name */
	char *sp_pwdp ; /* user password */
	long sp_lstchg ; /* password lastchanged date */
	long sp_min ; /* minimum number of days between password changes */
	long sp_max ; /* number of days password is valid */
	long sp_warn ; /* number of days to warn user to change passwd */
	long sp_inact ; /* number of days the login may be inactive */
	long sp_expire ; /* date when the login is no longer valid */
	unsigned long  sp_flag; /* currently not being used */
} ;

#if defined(__STDC__)

#ifndef _STDIO_H
#include <stdio.h>
#endif

/* Declare all shadow password functions */

extern void	setspent(void), endspent(void);
extern	struct	spwd	*getspent(void), *fgetspent(FILE *), *getspnam(const char *);
extern	int	putspent(const struct spwd *, FILE *), lckpwdf(void), ulckpwdf(void);

#else

/* Declare all shadow password functions */

void 		setspent(), endspent() ;
struct spwd 	*getspent(), *fgetspent(), *getspnam() ;
int 		putspent(), lckpwdf(), ulckpwdf() ;

#endif
