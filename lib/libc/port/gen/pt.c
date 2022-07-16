/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/pt.c	1.5"

#ifdef __STDC__
	#pragma weak ptsname = _ptsname
	#pragma weak grantpt = _grantpt
	#pragma weak unlockpt = _unlockpt
#endif

#include "synonyms.h"
#include "sys/types.h"
#ifdef i386
#include <sys/tss.h>
#include <sys/immu.h>
#include <sys/user.h>
#else
#include <sys/psw.h>
#include <sys/pcb.h>
#endif
#include "sys/param.h"
#include "sys/mkdev.h"
#include "sys/fs/s5dir.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/fcntl.h"
#include "sys/stat.h"
#include "sys/ptms.h"
#include <string.h>
#include <unistd.h>

#define PTSNAME "/dev/pts/"		/* slave name */
#define PTLEN   13			/* slave name length */
#define PTPATH  "/usr/lib/pt_chmod"    	/* setuid root program */
#define PTPGM   "pt_chmod"		/* setuid root program */

static char sname[PTLEN];

static void itoa();

/*
 *  Check that fd argument is a file descriptor of an opened master.
 *  Do this by sending an ISPTM ioctl message down stream. Ioctl()
 *  will fail if: (1) fd is not a valid file descriptor. (2) the file 
 *  represented by fd does not understand ISPTM (not a master device). 
 *  If we have a valid master, get its minor number via fstat(). 
 *  Concatenate it to PTSNAME and return it as the name of the slave
 *  device.
 */
char *
ptsname(fd)
int fd;
{
	register dev_t dev;
	struct stat status;
	struct strioctl istr;

	istr.ic_cmd = ISPTM;
	istr.ic_len = 0;
	istr.ic_timout = 0;
	istr.ic_dp = NULL;

	if ( ioctl( fd, I_STR, &istr) < 0)
		return( NULL);

	if ( fstat( fd, &status) < 0 )
		return( NULL);

	dev = minor( status.st_rdev);

	strcpy(sname,PTSNAME);
	itoa(dev, sname + strlen(PTSNAME));

	if ( access( sname, 00) < 0)
		return( NULL);

	return( sname);
}


/*
 * Send an ioctl down to the master device requesting the
 * master/slave pair be unlocked.
 */
unlockpt(fd)
int fd;
{
	struct strioctl istr;

	istr.ic_cmd = UNLKPT;
	istr.ic_len = 0;
	istr.ic_timout = 0;
	istr.ic_dp = NULL;


	if ( ioctl( fd, I_STR, &istr) < 0)
		return( -1);

	return( 0);
}


/*
 * Execute a setuid root program to change the mode, ownership and
 * group of the slave device. The parent forks a child process that
 * executes the setuid program. It then waits for the child to return.
 */
grantpt(fd)
int fd;
{
	int	st_loc;
	pid_t	pid;
	int	w;

	char	fds[4];


	if ( !( pid = fork())) {
		itoa(fd, fds);
		execl( PTPATH, PTPGM, fds, (char *)0);
		/*
		 * the process should not return, unless exec() failed.
		 */
		return( -1);
	}

	/*
	 * wait() will return the process id for the child process
	 * or -1 (on failure).
	 */
	w = waitpid(pid, &st_loc, 0);

	/*
	 * if w == -1, the child process did not fork properly.
	 * errno is set to ECHILD.
	 */
	if ( w == -1)
		return( -1);

	/*
	 * If child terminated due to exit()...
	 *         if high order bits are zero
	 *                   was an exit(0). 
	 *         else it was an exit(-1);
	 * Else it was struck by a signal.
	 */
	if (( st_loc & 0377) == 0)
		return((( st_loc & 0177400) == 0) ? 0 : -1);
	else
		return( -1);
}

static void
itoa(i, ptr)
register int i;
register char *ptr;
{
	register int dig;
	
	if (i < 10)
		dig = 1;
	else if (i < 100)
		dig = 2;
	else
		dig = 3;
	ptr += dig;
	*ptr = '\0';
	while (--dig >= 0) {
		*(--ptr) = i % 10 + '0';
		i /= 10;
	}
}
