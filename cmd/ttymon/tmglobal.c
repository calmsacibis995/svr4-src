/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ttymon:tmglobal.c	1.9.4.1"

#include <stdio.h>
#include <poll.h>
#include <signal.h>
#include <sys/resource.h>
#include <sac.h>
#include "tmstruct.h"
#include "ttymon.h"

/*
 *	global fd and fp
 */
FILE	*Logfp = NULL;		/* for log file			*/
int	Lckfd;			/* for pid file			*/
int	Sfd, Pfd;		/* for sacpipe and pmpipe 	*/
int	PCpipe[2];		/* pipe between Parent & Children */
#ifdef	DEBUG
FILE	*Debugfp = NULL;	/* for debug file		*/
#endif

char	State = PM_STARTING;	/* current state			*/
char	*Istate;		/* initial state			*/
char	*Tag;			/* port monitor tag			*/
int	Maxfiles;		/* Max number of open files		*/
int	Maxfds;			/* Max no of devices ttymon can monitor */

int	Reread_flag = FALSE;	/* reread pmtab flag			*/

int	Retry;			/* retry open_device flag		*/

struct  pmtab *PMtab = NULL;	/* head pointer to pmtab linked list 	*/
int	Nentries = 0;		/* # of entries in pmtab linked list	*/

struct  Gdef Gdef[MAXDEFS];	/* array to hold entries in /etc/ttydefs */
int	Ndefs = 0;		/* highest index to Gdef that was used   */
long	Mtime = 0;		/* last modification time of ttydefs	 */

struct pollfd *Pollp;		/* ptr to an array of poll struct 	 */
int	Npollfd;		/* size of the pollfd array		 */

char	Scratch[BUFSIZ];	/* general scratch buffer 	*/

struct Gdef DEFAULT = {		/* default terminal settings	*/
	"default",
	"9600",
	"9600 sane",
	0,
	/* 
	 * next label is set to 4800 so we can start searching ttydefs.
	 * if 4800 is not in ttydefs, we will loop back to use DEFAULT 
	 */
	"4800"
};

uid_t	Uucp_uid = 5;		/* owner's uid for bi-directional ports	*/
gid_t	Tty_gid = 7;		/* group id for all tty devices		*/

/*
 * Nlocked - 	number of ports that are either locked or have active
 *		sessions not under this ttymon.
 */
int	Nlocked = 0;

/* original rlimit value */
struct	rlimit	Rlimit;

/*
 * places to remember original signal dispositions and masks
 */

sigset_t	Origmask;		/* original signal mask */
struct	sigaction	Sigalrm;	/* SIGALRM */
struct	sigaction	Sigcld;		/* SIGCLD */
struct	sigaction	Sigint;		/* SIGINT */
struct	sigaction	Sigpoll;	/* SIGPOLL */
struct	sigaction	Sigterm;	/* SIGTERM */
#ifdef	DEBUG
struct	sigaction	Sigusr1;	/* SIGUSR1 */
struct	sigaction	Sigusr2;	/* SIGUSR2 */
#endif
