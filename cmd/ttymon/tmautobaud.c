/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ttymon:tmautobaud.c	1.6.3.1"

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <termio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stropts.h>

#define	NTRY	5

/*
 * At this time, we only recognize certain speeds.
 * This table can be expanded if new patterns are found
 */
static struct	autobaud {
	char	*a_speed;
	char	*a_pattern;	/* first byte is length */
} autob2400[] = {
	"110",		"\3\000\000\000",
	"1200",		"\2\346\200",
	"2400",		"\1\15",
	"4800",		"\1\371",
	"4800",		"\1\362",
	"9600",		"\1\377",
	0,		0
};

extern	char 	Scratch[];
extern	void	log();
extern	void	exit();
extern	unsigned alarm();
extern	int	read();
extern	int	ioctl();

/*
 *	auto_termio - set termio to allow autobaud
 *		    - the line is set to raw mode, with VMIN = 5, VTIME = 1
 *		    - baud rate is set to 2400
 */
auto_termio(fd)
int	fd;
{
	struct termio termio;

	if (ioctl(fd, TCGETA, &termio) == -1) {
		(void)sprintf(Scratch,
			"auto_termio: ioctl TCGETA failed, fd = %d, errno = %d",
			fd, errno);
		log(Scratch);
		return(-1);
	}
	termio.c_iflag = 0;
	termio.c_cflag &= ~(CBAUD|CSIZE|PARENB); 
	termio.c_cflag |= CREAD|HUPCL|(CS8&CSIZE)|(B2400&CBAUD);
	termio.c_lflag &= ~(ISIG|ICANON|ECHO|ECHOE|ECHOK);
	termio.c_oflag = 0;
	
	termio.c_cc[VMIN] = 5;
	termio.c_cc[VTIME] = 1;

	if (ioctl(fd, TCSETAF, &termio) == -1) {
		(void)sprintf(Scratch,
			"auto_termio: ioctl TCSETAF failed, fd = %d, errno = %d",
			fd, errno);
		log(Scratch);
		return(-1);
	}
	return(0);
}

/*
 *	autobaud - determine the baudrate by reading data at 2400 baud rate
 *		 - the program is anticipating <CR> 
 *		 - the bit pattern is matched again an autobaud table
 *		 - if a match is found, the matched speed is returned
 *		 - otherwise, NULL is returned 
 */

char *
autobaud(fd,timeout)
int	fd;
int	timeout;
{
	int i, k, count;
	static char	buf[5];
	register char *cp = buf;
	struct	autobaud *tp;
	struct	sigaction sigact;
	extern	void	timedout();
	extern	void	flush_input();

#ifdef	DEBUG
	debug("in autobaud");
#endif
	sigact.sa_flags = 0;
	sigact.sa_handler = SIG_IGN;
	(void)sigemptyset(&sigact.sa_mask);
	(void)sigaction(SIGINT, &sigact, NULL);
	count = NTRY;
	while (count--) {
		if (timeout) {
			sigact.sa_flags = SA_RESETHAND | SA_NODEFER;
			sigact.sa_handler = timedout;
			(void)sigemptyset(&sigact.sa_mask);
			(void)sigaction(SIGALRM, &sigact, NULL);
			(void)alarm((unsigned)timeout);
		}
		cp = &buf[1];
		if ((k=read(fd, cp, 5)) < 0) {
			(void)sprintf(Scratch, "autobaud: read failed, errno = %d",
				errno);
			log(Scratch);
			exit(1);
		}
		if (timeout)
			(void)alarm((unsigned)0);
		buf[0] = (char)k;
		for (tp = autob2400; tp->a_speed; tp++) {
			for (i = 0;; i++) {
				if (buf[i] != tp->a_pattern[i])
					break;
				if (i == buf[0]) {
					return(tp->a_speed);
				}
			}
		}
		flush_input(fd);
	} /* end while */
	return(NULL);		/* autobaud failed */
}
