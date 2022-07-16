/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _GRP_H
#define _GRP_H 

#ident	"@(#)head:grp.h	1.3.3.1"

#include <sys/types.h>

struct	group {	/* see getgrent(3) */
	char	*gr_name;
	char	*gr_passwd;
	gid_t	gr_gid;
	char	**gr_mem;
};

#include <stdio.h>

#if defined(__STDC__)

#if !defined(_POSIX_SOURCE) 
extern void endgrent(void);
extern struct group *fgetgrent(FILE *);
extern struct group *getgrent(void);
#endif /* !defined(_POSIX_SOURCE) */ 
extern struct group *getgrgid(gid_t);
extern struct group *getgrnam(const char *);
#if !defined(_POSIX_SOURCE) 
extern void setgrent(void);
extern int initgroups(const char *, gid_t);
#endif /* !defined(_POSIX_SOURCE) */ 

#else

#if !defined(_POSIX_SOURCE) 
extern void endgrent();
extern struct group *fgetgrent();
extern struct group *getgrent();
#endif /* !defined(_POSIX_SOURCE) */ 
extern struct group *getgrgid();
extern struct group *getgrnam();
#if !defined(_POSIX_SOURCE) 
extern void setgrent();
extern int initgroups();
#endif /* !defined(_POSIX_SOURCE) */ 

#endif	/* __STDC__ */

#endif 	/* _GRP_H */
