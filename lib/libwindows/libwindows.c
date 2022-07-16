/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)xt:hostagent.c	2.10.3.1"
/* All functions in this module will return a value of 0 for success and    */
/* -1 for failure unless otherwise mentioned.				    */

#include	<stdio.h>
#include	<fcntl.h>
#include	<sys/types.h>
#include	<sys/tty.h>
#include	<sys/jioctl.h>
#include	<sys/nxtproto.h>
#include	"windows.h"

#define		EOS	'\0'		/* End of string		     */

union		bbuf	{
		struct	agentrect	ar;
		char	buf[MAXPKTDSIZE];
};
union		bbuf ret;

/* This will open the control channel for  the tty on which the application  */
/* is running. Only the layers process can open this control channel in the  */
/* write mode.						     		     */

/* RETURNS :  A file descriptor that is to be used for all subsequent jagent */
/*calls. 							             */

int
openagent()
{
	char	*cp;
	int	cntlfd;
	char	*ttyname();
	char	*cntlf;

	if (ioctl(0, JMPX, 0) == -1)
		return(-1);
	if ((cntlf = ttyname(fileno (stdin))) == EOS)
		return(-1);
	cntlf[strlen(cntlf) - 1] = '0';
	return ( open (cntlf, O_RDONLY) );
}

/* This function will open the tty file for the indicated channel in the read
 * write mode.
 * RETURNS : the file descriptor for the opened channel.
 *
 * Note that the open is attempted several times on error for the following
 * reason. When a window is deleted, xt sends an M_HANGUP upsream. After the
 * stream head gets this M_HANGUP, it will reject all open attempts until
 * the last process which has that channel open closes the channel.
 * A couple of programs have been encountered which use libwindows to
 * Delete() one window and then very quickly do an openchan() either directly
 * or through Runlayers(). If the openchan is attempted before the
 * process running in the previous window dies, the openchan() will fail.
 *
 * No good solution for this problem is known, and that is why the open
 * is attempted several times. This should fix the problem unless the
 * process traps hangups and is very slow about exiting. Better to
 * fix it most of the time than none of the time.
 */
int
openchan(chan)
int chan;
{
	char	*cp;
	char	*ttyname();
	static	char	*chanf;
	int i;
	int retval;

	if (ioctl(0, JMPX, 0) == -1) 
		return(-1);
	if ((chanf = ttyname(fileno (stdin))) == EOS)
		return(-1);
	chanf[strlen(chanf) - 1] = chan + '0';

	for(i = 0 ; i < 5 ; ++i) {
		if( (retval = open (chanf, O_RDWR)) != -1 )
			break;
		sleep(2);
	}

	return ( retval );
}

/* This jagent command will open a new window with the supplied coordinates */
/* There will be no processes running on that window. An open on the        */
/* returned channel must be performed. This is automatically done by        */
/* Runlayers. Therefore, typically Newlayer should be followed by an open   */
/* or Runlayer call. Runlayer will result in a shell being automatically    */
/* forked by Layers.							    */

/* RETURNS : the channel no. of the new channel				    */

int
Newlayer(fd, x1, y1, x2, y2)
int	fd;
int	x1, y1, x2, y2;
{
	if( ioctlagent(fd, A_NEWLAYER, x1, y1, x2, y2, 0) )
		return(-1);
	return (ret.ar.chan);
}

/* This will make the indicated window current and on top.		    */
/* The processing for this command is local to the terminal.		     */ 

int
Current(fd, chan)
int	fd;
int	chan;
{
	return ( ioctlagent(fd, A_CURRENT, 0, 0, 0, 0, chan) );
}

/* This brings the indicated window on top. The window is not current.       */
/* The processing for this command is local to the terminal.		     */ 

int
Top(fd, chan)
int	fd;
int	chan;
{
	return ( ioctlagent(fd, A_TOP, 0, 0, 0, 0, chan) );
}

/* This function will put the indicated window at the bottom.                */
/* The processing for this command is local to the terminal.		     */ 

int
Bottom(fd, chan)
int	fd;
int	chan;
{
	return ( ioctlagent(fd, A_BOTTOM, 0, 0, 0, 0, chan) );
}

/* This function will move the indicated window to the supplied coordinates. */
/* The processing for this command is local to the terminal.		     */ 

int
Move(fd, chan, x1, y1)
int	fd;
int	chan;
int	x1, y1;
{
	return ( ioctlagent(fd, A_MOVE, x1, y1, 0, 0, chan) );
}


/* This jagent command will result in a C_NEW command from the terminal.    */
/* Layers will automatically open the file for the channel indicated by     */
/* C_NEW  and execute a shell in it.					    */ 
/* RETURNS : the channel no. of the new channel				    */

int
New(fd, x1, y1, x2, y2)
int	fd;
int	x1, y1, x2, y2;
{
	if( ioctlagent(fd, A_NEW, x1, y1, x2, y2, 0) )
		return(-1);
	return (ret.ar.chan);
}


/* This jagent command will result in a C_DELETE command from the terminal.  */
/* Layers will then kill all the processes running on that window.	     */

int
Delete(fd, chan)
int	fd;
int	chan;
{
	return ( ioctlagent(fd, A_DELETE, 0, 0, 0, 0, chan) );
}

int
Reshape(fd, chan, x1, y1, x2, y2)
int	fd;
int	chan;
int	x1, y1, x2, y2;
{
	return ( ioctlagent(fd, A_RESHAPE, x1, y1, x2, y2, chan) );
}

/* This jagent command will result in a C_EXIT command from the terminal.    */
/* Layers will exit after  killing all the processes running on all the      */
/* windows.	     							     */

int
Exit(fd)
int fd;
{
	return ( ioctlagent(fd, A_EXIT, 0, 0, 0, 0, 0) );
}


/* This jagent command will result in a C_RUN command from the XT driver.    */
/* Layers will then exec a shell on that window to handle the supplied	     */
/* command. Please note that there is no interaction with the terminal for   */
/* this command.							     */ 

int
Runlayer(chan, command)
int	chan;
char	*command;
{

	int fdc;
	char cb[TTYHOG+1];
	int rv;

	if ( (fdc = openchan(chan)) == -1 ) 
		return(-1);
	if ( strlen(command) > TTYHOG ) 
		return(-1);
	
	cb[0] = (char )chan;
	cb[1] = '\0';
	strcat(cb,command);
	rv = ioctl(fdc,JTRUN,cb);
	close(fdc);
	return(rv);
}


ioctlagent(fd, command, x1, y1, x2, y2, chan)
int	fd;
int	command;
int	x1, y1, x2, y2;
short	chan;
{
	union	bbuf	arbuf;
	int	size;
	static struct 		bagent cmd;

	arbuf.ar.command = command;
	arbuf.ar.chan = chan;
	arbuf.ar.r.origin.x = x1;
	arbuf.ar.r.origin.y = y1;
	arbuf.ar.r.corner.x = x2;
	arbuf.ar.r.corner.y = y2;

#if (vax || i386)
	swab(arbuf.buf, arbuf.buf, sizeof(struct agentrect));
#endif	

	cmd.size = sizeof(struct agentrect);
	cmd.src = arbuf.buf;
	cmd.dest = ret.buf;
	if( (size = ioctl(fd, JAGENT, &cmd) ) != sizeof(struct agentrect) )
		return(-1);

#if (vax || i386)
	swab(ret.buf, ret.buf, sizeof(struct agentrect));
#endif

	if(ret.ar.command == (-1))
		return(-1);
	return (0);
}

