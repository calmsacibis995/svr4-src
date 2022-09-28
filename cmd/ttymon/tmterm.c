/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ttymon:tmterm.c	1.11.7.1"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <termio.h>
#include <sys/stermio.h>
#include <sys/termiox.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include "sys/stropts.h"
#include "sys/signal.h"
#include "ttymon.h" 
#include "tmstruct.h" 

extern	char	Scratch[];
extern	void	log();
extern	void	mkargv();

/*
 *	set_termio	- set termio on device 
 *		fd	- fd for the device
 *		options - stty termio options 
 *		aspeed  - autobaud speed 
 *		clear	- if TRUE, current flags will be set to some defaults
 *			  before applying the options 
 *		    	- if FALSE, current flags will not be cleared
 *		mode	- terminal mode, CANON, RAW
 */
int
set_termio(fd,options,aspeed,clear,mode)
int	fd;
char	*options;
char	*aspeed;
int	clear;
long	mode;
{
	struct 	 termio termio;
	struct 	 termios termios;
	struct 	 stio stermio;
	struct 	 termiox termiox;
	struct 	 winsize winsize;
	struct 	 winsize owinsize;
	int	 term;
	int	 cnt = 1;
	char	 *uarg;	
	char	 *argvp[MAXARGS];	/* stty args */
	static   char	 *binstty = "/usr/bin/stty";
	static	 char	buf[BUFSIZ];
	extern 	 int get_ttymode(), set_ttymode();
	extern	 char	*sttyparse();

#ifdef	DEBUG
	debug("in set_termio");
#endif

	if ((term = get_ttymode(fd, &termio, &termios, &stermio, 
				&termiox, &winsize)) < 0) {
		(void)sprintf(Scratch,
			"set_termio: get_ttymode failed, errno = %d", errno);
		log(Scratch);
		return(-1);
	}
	owinsize = winsize;
	if (clear) {
		termios.c_iflag = 0;
		termios.c_cflag = 0;
		termios.c_lflag = 0;
		termios.c_oflag = 0;

		termios.c_iflag |= (IGNPAR|ISTRIP|ICRNL|IXON); 
		termios.c_cflag |= CS7|CREAD|PARENB|(B9600&CBAUD);
		if (mode & CANON) {
			termios.c_lflag |= (ISIG|ICANON|ECHO|ECHOE|ECHOK); 
			termios.c_cc[VEOF] = CEOF;
			termios.c_cc[VEOL] = CNUL;
		}
		else  {
			termios.c_lflag &= ECHO;
			termios.c_cc[VMIN] = 1;
			termios.c_cc[VTIME] = 0;
		}
		termios.c_oflag |= OPOST|ONLCR;

	}

	if (options != NULL && *options != '\0') {
		/* just a place holder to make it look like invoking stty */
		argvp[0] = binstty;
		(void)strcpy(buf,options);
		mkargv(buf,&argvp[1],&cnt,MAXARGS-1);
		if ((aspeed != NULL) && (*aspeed != '\0')) {
			argvp[cnt++] = aspeed;
		}
		argvp[cnt] = (char *)0;
		if ((uarg = sttyparse(cnt, argvp, term, &termio, &termios, 
				&termiox, &winsize)) != NULL) {
			(void)sprintf(Scratch, "sttyparse unknown mode: %s",
				uarg);
			log(Scratch);
			return(-1);
		}
	}
	if (set_ttymode(fd, term, &termio, &termios, &stermio, 
			&termiox, &winsize, &owinsize) != 0) {
		(void)sprintf(Scratch,
			"set_termio: set_ttymode failed, errno = %d", errno);
		log(Scratch);
		return(-1);
	}
	return(0);
}

/*
 *	turnon_canon	- turn on canonical processing
 *			- return 0 if succeeds, -1 if fails
 */
turnon_canon(fd)
int	fd;
{
	struct termio termio;

#ifdef	DEBUG
	debug("in turnon_canon");
#endif
	if (ioctl(fd, TCGETA, &termio) != 0) {
		(void)sprintf(Scratch,
		"turnon_canon: TCGETA failed, fd = %d, errno = %d", fd, errno);
		log(Scratch);
		return(-1);
	}
	termio.c_lflag |= (ISIG|ICANON|ECHO|ECHOE|ECHOK); 
	termio.c_cc[VEOF] = CEOF;
	termio.c_cc[VEOL] = CNUL;
	if (ioctl(fd, TCSETA, &termio) != 0) {
		(void)sprintf(Scratch,
		"turnon_canon: TCSETA failed, fd = %d, errno = %d", fd, errno);
		log(Scratch);
		return(-1);
	}
	return(0);
}

/*
 *	flush_input	- flush the input queue
 */
void
flush_input(fd)
int	fd;
{
	if (ioctl(fd, I_FLUSH, FLUSHR) == -1) {
		(void)sprintf(Scratch,"flush_input failed, fd = %d, errno = %d",
			fd, errno);
		log(Scratch);
	}
	return;
}

/*
 * push_linedisc	- if modules is not NULL, pop everything
 *			- then push modules specified by "modules"
 */

push_linedisc(fd,modules,device)
int	fd;		/* fd to push modules on			 */
char	*modules;	/* ptr to a list of comma separated module names */
char	*device;	/* device name for printing msg			 */
{
	char	*p, *tp;
	char	buf[BUFSIZ];

#ifdef	DEBUG
	debug("in push_linedisc");
#endif
	/*
	 * copy modules into buf so we won't mess up the original buffer
	 * because strtok will chop the string
	 */
	p = strcpy(buf,modules);

	while(ioctl(fd, I_POP) >= 0)  /* pop everything */ 
		;
	for (p=(char *)strtok(p,","); p!=(char *)NULL; 
		p=(char *)strtok(NULL,",")) {
		for (tp = p + strlen(p) - 1; tp >= p && isspace(*tp); --tp)
			*tp = '\0';
		if (ioctl(fd, I_PUSH, p) == -1) {
			(void)sprintf(Scratch,
			"push (%s) on %s failed, errno = %d",
			p, device, errno);
			log(Scratch);
			return(-1);
		}  
	}
	return(0);
}

/*
 *	hang_up_line	- set speed to B0. This will drop DTR
 */
hang_up_line(fd)
int	fd;
{
	struct termio termio;

#ifdef	DEBUG
	debug("in hang_up_line");
#endif
	if (ioctl(fd,TCGETA,&termio) < 0) {
		(void)sprintf(Scratch,
			"hang_up_line: TCGETA failed, errno = %d", errno);
		log(Scratch);
		return(-1);
	}
	termio.c_cflag &= ~CBAUD;
	termio.c_cflag |= B0;

	if (ioctl(fd,TCSETA,&termio) < 0) {
		(void)sprintf(Scratch,
			"hang_up_line: TCSETA failed, errno = %d", errno);
		log(Scratch);
		return(-1);
	}
	return(0);
}

/*
 * initial_termio	- set initial termios
 *			- return 0 if successful, -1 if failed.
 */
int
initial_termio(fd,pmptr)
int	fd;
struct	pmtab	*pmptr;
{
	int	ret;
	struct	Gdef *speedef;
	struct	Gdef *get_speed();
	extern	int  auto_termio();

	speedef = get_speed(pmptr->p_ttylabel);
	if (speedef->g_autobaud & A_FLAG) {
		pmptr->p_ttyflags |= A_FLAG;
		if (auto_termio(fd) == -1) {
			(void)close(fd);
			return(-1);
		}
	}
	else {
		if (pmptr->p_ttyflags & R_FLAG)
			ret = set_termio(fd,speedef->g_iflags,
				(char *)NULL, TRUE, (long)RAW);
		else 
			ret = set_termio(fd,speedef->g_iflags,
				(char *)NULL, TRUE, (long)CANON);
		if (ret == -1) {
			(void)sprintf(Scratch,"initial termio on (%s) failed",
				pmptr->p_device);
			log(Scratch);
			(void)close(fd);
			return(-1);
		}
	}
	return(0);
}
