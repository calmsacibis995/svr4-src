/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)shl:defs.h	1.8.6.1"
#include	<sys/types.h>
#include	<sys/stream.h>
#include	<sys/errno.h>
#include	<signal.h>
#include	<sys/termio.h>
#include	<sys/strtty.h>
#include	<sys/nsxt.h>
#include	<fcntl.h>
#include	<stdio.h>

#define		SYSERROR	-1
#define		MAX_LAYERS	MAXPCHAN
#define		NAMSIZ		8

#ifndef	ROOT_UID
#define	ROOT_UID	0
#endif

#ifndef	SYS_GID
#define	SYS_GID		3
#endif

#ifdef	u3b15
#define		set_dev(x)	if (cntlf[5] == 's') \
							sprintf(&cntlf[9], "%03d", conv(x)); \
						else \
							sprintf(&cntlf[10], "%03d", conv(x));
#else
#define		set_dev(x)	sprintf(&cntlf[9], "%03d", conv(x))
#endif

#define		prompt()	printf(">>> ");

struct layer
{
	char name[NAMSIZ + 1];
	pid_t p_grp;
};

extern struct layer 	*layers[];
extern int		 		chan;
extern int 				max_index;
extern int				fildes[2];

extern char 			*cntlf;
extern char				cntl_fl[];
extern char				ocntl_fl[];
extern char				vcntl_fl[];

extern int  			cntl_chan_fd;
extern int			real_tty_fd;

extern int				errno;
extern int				sys_nerr;
extern char				*sys_errlist[];

extern struct utmp		*u_entry;
extern struct termio	ttysave;

extern uid_t				uid;
extern gid_t				gid;

char 					*strcpy();
void						hangup();
void						child_death();


