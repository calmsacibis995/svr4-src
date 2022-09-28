/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:maillock.h	1.6.3.1"

#ifdef SVR3
#     define	MAILDIR		"/usr/mail/"
#     define	SAVEDIR		"/usr/mail/:saved/"
#else
#     define	MAILDIR		"/var/mail/"
#     define	SAVEDIR		"/var/mail/:saved/"
#endif

#define	L_SUCCESS	0
#define	L_NAMELEN	1	/* recipient name > 13 chars */
#define	L_TMPLOCK	2	/* problem creating temp lockfile */
#define L_TMPWRITE	3	/* problem writing pid into temp lockfile */
#define	L_MAXTRYS	4	/* cannot link to lockfile after N tries */
#define	L_ERROR		5	/* Something other than EEXIST happened */
#define	L_MANLOCK	6	/* cannot set mandatory lock on temp lockfile */

#if defined(__STDC__) || defined(__cplusplus)
extern int maillock(char *user, int retrycnt);
extern int mailunlock(void);
#else
extern int maillock();
extern int mailunlock();
#endif
