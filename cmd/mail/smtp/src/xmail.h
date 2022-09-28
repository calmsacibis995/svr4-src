/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:smtp/src/xmail.h	1.2.3.1"
extern char *MAILROOT;	/* root of mail system */
extern char *UPASROOT;	/* root of upas system */
extern char *SMTPQROOT; /* root of smtpq directory */
extern char *SYSALIAS;	/* file system alias files are listed in */
extern char *USERALIAS;	/* file system alias files are listed in */
extern int MBOXMODE;	/* default mailbox protection mode */

/* format of REMOTE FROM lines */
extern char *REMFROMRE;
extern int REMSENDERMATCH;
extern int REMDATEMATCH;
extern int REMSYSMATCH;

/* format of mailbox FROM lines */
#define IS_HEADER(p) ((p)[0]=='F'&&(p)[1]=='r'&&(p)[2]=='o'&&(p)[3]=='m'&&(p)[4]==' ')
extern char *FROMRE;
extern int SENDERMATCH;
extern int DATEMATCH;

extern void print_header();
extern void print_remote_header();

#ifndef NULL
#define NULL 0
#endif

#if defined(SYS5) || defined(SVR3) || defined(SVR4)

#define SIGRETURN void
typedef void (*SIG_TYP)();

#else

#ifdef BSD

#define SIGRETURN int
typedef int (*SIG_TYP)();

#else

#define SIGRETURN int

#endif

#endif
