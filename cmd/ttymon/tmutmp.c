/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ttymon:tmutmp.c	1.10.3.1"

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

#include	<unistd.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<fcntl.h>
#include	<sys/types.h>
#include	<string.h>
#include	<memory.h>
#include	<utmp.h>
#include	"sac.h"

extern	char	Scratch[];
extern	void	log();
extern	time_t	time();
extern	char	*lastname();

/*
 * account - create a utmp record for service
 *
 */

int
account(line)
char	*line;
{
	struct utmp utmp;			/* prototype utmp entry */
	register struct utmp *up = &utmp;	/* and a pointer to it */
	extern	char *Tag;
	extern	struct	utmp *makeut();

	(void) memset(up, '\0', sizeof(utmp));
	up->ut_user[0] = '.';
	(void)strncpy(&up->ut_user[1], Tag, sizeof(up->ut_user)-1);
	(void)strncpy(up->ut_line, lastname(line), sizeof(up->ut_line));
	up->ut_pid = (o_pid_t)getpid();
	up->ut_type = USER_PROCESS;
	up->ut_id[0] = 't';
	up->ut_id[1] = 'm';
	up->ut_id[2] = SC_WILDC;
	up->ut_id[3] = SC_WILDC;
	up->ut_exit.e_termination = 0;
	up->ut_exit.e_exit = 0;
	(void)time(&up->ut_time);
	if (makeut(up) == NULL) {
		(void)sprintf(Scratch, "makeut for pid %d failed",up->ut_pid);
		log(Scratch);
		return(-1);
	}
	return(0);
}

void
cleanut(pid,status)
pid_t	pid;
int	status;
{
	register struct utmp *up;
	extern	struct utmp *modut();

	setutent();
	while (up = getutent()) {
		if (up->ut_pid != (o_pid_t)pid)
			continue;
		up->ut_type = DEAD_PROCESS;
		up->ut_exit.e_termination = (status & 0xff);
		up->ut_exit.e_exit = ((status >> 8) & 0xff);
		(void)time(&up->ut_time);
		if (modut(up) == NULL) {
			log("modify utmp failed\n");
		}
		endutent();
		return;
	}
	endutent();
	return;
}

/*
 * getty_account	- This is a copy of old getty account routine.
 *			- This is only called if ttymon is invoked as getty.
 *			- It tries to find its own INIT_PROCESS entry in utmp
 *			- and change it to LOGIN_PROCESS
 */
void
getty_account(line)
char *line;
{
	register o_pid_t ownpid;
	register struct utmp *u;

	ownpid = (o_pid_t)getpid();

	setutent();
	while ((u = getutent()) != NULL) {

		if (u->ut_type == INIT_PROCESS && u->ut_pid == ownpid) {
			(void)strncpy(u->ut_line,lastname(line),sizeof(u->ut_line));
			(void)strncpy(u->ut_user,"LOGIN",sizeof(u->ut_user));
			u->ut_type = LOGIN_PROCESS;

			/* Write out the updated entry. */
			(void)pututline(u);
			break;
		}
	}

	/* create wtmp entry also */
	if (u != NULL)
		updwtmp("/etc/wtmp", u);

	endutent();
}
