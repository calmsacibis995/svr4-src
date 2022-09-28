/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:filter/hp2631a.c	1.1.3.1"

/*
	hp2631a - filter for hp2631a printer

	Usage: hp2631a [-c] [-e] [-n] [-bbaud]

	where
		-c means compressed mode
		-e means expanded mode
		-n means normal mode
		-bbaud selects the baud rate
		  (default baud rate is 2400)
*/


#include	<stdio.h>
#include	<signal.h>
#include	<sys/ioctl.h>
#include	<termio.h>

#define	ENQ	'\05'
#define	ACK	'\06'
#define	SO	'\016'
#define	SI	'\017'
#define	DC1	'\021'
#define	ESC	'\033'

#define	WBUFSIZ	80
#define	ALTIME	5

char reset[] = { ESC, 'E', '\0' };
char online[] = { ESC, 'n', '\0' };
char normal[] = { ESC, '&', 'k', '0', 'S', '\0' };
char compress[] = { ESC, '&', 'k', '2', 'S', '\0' };
char expand[] = { ESC, '&', 'k', '1', 'S', '\0' } ;
char clrtabs[] = { ESC, '3', '\0' };
char tabset[] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ESC, '1', '\0' };

struct termio save;

main(argc, argv)
int argc;
char *argv[];
{
	struct termio tbuf;
	char rbuf[BUFSIZ]; 	/* input buffer */
	char wbuf[WBUFSIZ]; 	/* output buffer */
	int nread;		/* # of characters read from stdin */
	int nwrite;		/* # of characters to be written to printer */
	int wptr;		/* # of characters in input buffer that
				   have been looked at */
	int nonprint;		/* set to 1 if last character read is a
				   "dangerous" control code, otherwise 0 */
	int escape;		/* set to 1 if last character read is ESC,
				   otherwise 0.  Certain escape sequences
				   will not be permitted to pass through
				   this filter */
	int i, baud = B2400, restore(), bval;
	char c, *name, *ttyname(), *arg;
	char *mode = normal;

	if((name = ttyname(1)) == NULL) {
		fprintf(stderr, "%s: standard output not a printer\n",
		   argv[0]);
		exit(1);
	}

	for(i = 1; i < argc; i++) {
		arg = argv[i];
		if(*(arg++) == '-') {
			switch(*arg++) {
			case 'n':
				mode = normal;
				break;
			case 'c':
				mode = compress;
				break;
			case 'e':
				mode = expand;
				break;
			case 'b':
				if((bval = atoi(arg)) > 0)
					switch(bval) {
					case 300:
						baud = B300;
						printf("300 baud\n");
						break;
					case 600:
						baud = B600;
						break;
					case 1200:
						baud = B1200;
						break;
					case 1800:
						baud = B1800;
						break;
					case 2400:
						baud = B2400;
						break;
					default:
						break;
					}
			default:
				break;
			}
		}
	}

	close(1);
	if(open(name, 2) != 1) {
		fprintf(stderr, "%s: can't open %s\n", argv[0], name);
		exit(2);
	}
	ioctl(1, TCGETA, &save);
	if(signal(SIGHUP, SIG_IGN) != SIG_IGN)
		signal(SIGHUP, restore);
	if(signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, restore);
	if(signal(SIGQUIT, SIG_IGN) != SIG_IGN)
		signal(SIGQUIT, restore);
	if(signal(SIGTERM, SIG_IGN) != SIG_IGN)
		signal(SIGTERM, restore);

	tbuf = save;
	tbuf.c_iflag = IGNPAR | ISTRIP;
	tbuf.c_oflag = 0;
	tbuf.c_cflag = baud | CS7 | CREAD | PARENB;
	tbuf.c_lflag = 0;
	tbuf.c_cc[VMIN] = 1;		/* So read returns after 1 char. */
	ioctl(1, TCSETAW, &tbuf);

	catch14();		/* catch alarm signal */

	ack();
	write(1, "\014", 1);
	sleep(5);
	write(1, "\033", 1);
	write(1, "E", 1);
	sleep(5);
	write(1, "\033", 1);
	write(1, "n", 1);
	ack();
	write(1, mode, 5);
	write(1, "\033", 1);
	write(1, "3", 1);
	for(i=1; i<=15; i++) {
		ack();
		write(1, " ", 1);
		write(1, " ", 1);
		write(1, " ", 1);
		write(1, " ", 1);
		write(1, " ", 1);
		write(1, " ", 1);
		write(1, " ", 1);
		write(1, " ", 1);
		write(1, "\033", 1);
		write(1, "1", 1);
	}
	write(1, "\015", 1);

	nonprint = escape = 0;
	while((nread = read(0, rbuf, BUFSIZ)) > 0) {
		wptr = 0;
		while(wptr < nread) {
			nwrite = 0;
			while(nwrite < WBUFSIZ - 2 && wptr < nread) {
				if(escape) { /* last char read was an ESC */
					escape = 0;

					/* eliminate escape codes which may
					   adversely affect subsequent print
					   jobs.		*/

					switch(c = rbuf[wptr++]) {
					case 'z':	/* self test */
					case 'E':	/* reset */
					case 'n':	/* on line */
					case 'o':	/* off line */
					case 'Y':	/* display functions
								mode	*/
					case 'Z':	/* turn off display
							   functions mode */
					case '(':	/* select primary
							   character set */
					case ')':	/* select secondary
							   character set */

						nonprint = 1;
						break;

					case ENQ:
					case SO:
					case SI:
					case DC1:
						nonprint = 1;
						break;

					case ESC:
						nonprint = 1;
						escape = 1;
						break;

					default:
						/* escape sequence is ok */

						wbuf[nwrite++] = ESC;
						break;
					}
				}
				else
					switch(c = rbuf[wptr++]) {
					case '\n':
						wbuf[nwrite++] = '\r';
						break;
	
					case ENQ:	/* ask for printer
							   acknowledgment */
					case SO:	/* select secondary
							   character set */
					case SI:	/* select primary
							   character set */
					case DC1:	/* status read back */
						nonprint = 1;
						break;
	
					case ESC:
						escape = 1;
						nonprint = 1;
						break;
	
					default:
						break;
					}

				if(nonprint)
					nonprint = 0;
				else
					wbuf[nwrite++] = c;
			}

			ack();
			write(1, wbuf, nwrite);
		}
	}
	restore();
}

catch14()
{
	signal(SIGALRM, catch14);
}

/* ack - wait until printer is ready for at least 80 characters */

ack()
{
	unsigned alarm();
	char ans = NULL;

	while(ans != ACK) {
		write(1, "\005", 1);
		alarm(ALTIME);
		read(1, &ans, 1);
		alarm(0);
	}
}

restore()
{
	ioctl(1, TCSETAW, &save);
	exit(0);
}
