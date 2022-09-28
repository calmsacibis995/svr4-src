/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.bin/ftp/getpass.c	1.4.3.1"

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */


#include <stdio.h>
#include <signal.h>
#include <termio.h>
#include <sys/ttold.h>
#include <sys/types.h>
#include <sys/stropts.h>

static	struct sgttyb ttyb;
static	int flags;
static	FILE *fi;
static	int ttcompat;

#ifdef SYSV
#define signal(s,f)	sigset(s,f)
#endif /* SYSV */

static void
intfix()
{
	ttyb.sg_flags = flags;
	if (fi != NULL)
		(void) stty(fileno(fi), &ttyb);
	if (ttcompat)
		(void) ioctl(fileno(fi), I_POP, "ttcompat");
	exit(SIGINT);
}

char *
mygetpass(prompt)
char *prompt;
{
	register char *p;
	register int c;
	static char pbuf[50+1];
	void (*sig)();

	if ((fi = fopen("/dev/tty", "r")) == NULL)
		fi = stdin;
	else
		setbuf(fi, (char *)NULL);

	sig = signal(SIGINT, (void (*)())intfix);
	c = ioctl(fileno(fi), I_FIND, "ttcompat");
	if (c == 0) {
		if (ioctl (fileno(fi), I_PUSH, "ttcompat") < 0) {
			perror ("ftp: ioctl I_PUSH ttcompat");
		}
		ttcompat = 1;
	} else if (c < 0)
		perror("ftp: ioctl I_FIND ttcompat");

	if ( gtty(fileno(fi), &ttyb) < 0 )
		perror("ftp: gtty");	/* go ahead, anyway */
	flags = ttyb.sg_flags;
	ttyb.sg_flags &= ~ECHO;
	(void) stty(fileno(fi), &ttyb);
	fprintf(stderr, "%s", prompt); (void) fflush(stderr);
	for (p=pbuf; (c = getc(fi))!='\n' && c!=EOF;) {
		if (p < &pbuf[sizeof(pbuf)-1])
			*p++ = c;
	}
	*p = '\0';
	fprintf(stderr, "\n"); (void) fflush(stderr);
	ttyb.sg_flags = flags;
	(void) stty(fileno(fi), &ttyb);
	(void) signal(SIGINT, sig);
	if (ttcompat)
		(void) ioctl(fileno(fi), I_POP, "ttcompat");
	if (fi != stdin)
		(void) fclose(fi);
	return(pbuf);
}
