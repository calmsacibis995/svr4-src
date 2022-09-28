/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ttymon:tmlock.c	1.5.3.1"
#include	<stdio.h>
#include	<unistd.h>
#include	<string.h>
#include	<sys/termios.h>
/*--------------------------------------------------------- */
/* the follwing are here so we can use routines in ulockf.c */
int	Debug = 0;
char	*Bnptr;
/* dummies for using uucp .o routines */
/*VARARGS*/
/*ARGSUSED*/
void
assert(s1,s2,i1,s3,i2)
char	*s1, *s2, *s3;
int	i1, i2;
{}

void
cleanup(){}

/*ARGSUSED*/
void
logent(s1, s2)
char	*s1, *s2;
{}		/* so we can load ulockf() */
/*---------------------------------------------------------- */
extern	int	lockf();

/*
 *	lastname	- If the path name starts with "/dev/",
 *			  return the rest of the string.
 *			- Otherwise, return the last token of the path name
 */
char	*
lastname(name)
char	*name;
{
	char	*sp, *p;
	sp = name;
	if (strncmp(sp, "/dev/", 5) == 0)
		sp += 5;
	else
		while ((p = (char *)strchr(sp,'/')) != (char *)NULL) {
			sp = ++p;
		}
	return(sp);
}

/*
 *	tm_lock(fd)	- set advisory lock on the device
 */

tm_lock(fd)
int	fd;
{
	extern	int	fd_mklock();
	return(fd_mklock(fd));
}

/*
 *	tm_checklock	- check if advisory lock is on 
 */

tm_checklock(fd)
int	fd;
{
	extern	int	fd_cklock();
	return(fd_cklock(fd));
}

/*
 * check_session(fd) - check if a session established on fd
 *		       return 1 if session exists, otherwise, return 0.
 *
 */
int
check_session(fd)
int	fd;
{
	pid_t	sid;

	if (ioctl(fd, TIOCGSID, &sid) == -1)
		return(0);
	else if (sid == 0)
		return(0);
	else
		return(1);
}
