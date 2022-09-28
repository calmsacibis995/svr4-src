/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:filter/prx.c	1.1.3.1"

/* prx -- filter for PRINTRONIX P-300 line printer.
	Lines beginning with backspace are passed through unchanged,
	since this is the way PRINTRONIX knows to print in double
	character size.  All other backspaces are filtered out.
	Bold is simulated by underlining -- PRINTRONIX cannot backspace,
	therefore underlining is done by printing text from one buffer,
	then doing a '\r' (but no \n), and printing underlines at
	proper character positions.
*/

#include	<stdio.h>
#include	<signal.h>
#include	<sys/ioctl.h>
#include	<termio.h>

#define	LINEMAX	132


char buf1[LINEMAX], buf2[LINEMAX];
int n1 = 0, n2 = -1;
struct termio save;

main(argc, argv)
int argc;
char *argv[];
{
	int i, bs, spaces, baud, restore();
	char c, *p, line[BUFSIZ], *fgets();
	struct termio tbuf;

	ioctl(1, TCGETA, &save);

	if(signal(SIGHUP, SIG_IGN) != SIG_IGN)
		signal(SIGHUP, restore);
	if(signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, restore);
	if(signal(SIGQUIT, SIG_IGN) != SIG_IGN)
		signal(SIGQUIT, restore);
	if(signal(SIGTERM, SIG_IGN) != SIG_IGN)
		signal(SIGTERM, restore);
	if(argc < 2 || (baud = atoi(argv[1])) <= 0)
		baud = B1200;
	else
		switch(baud) {
		case 300:
			baud = B300;
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
		case 4800:
			baud = B4800;
			break;
		case 9600:
			baud = B9600;
			break;
		default:
			baud = B1200;
			break;
		}

	tbuf = save;
	tbuf.c_iflag = IGNPAR | ISTRIP | IXON | BRKINT;
	tbuf.c_oflag = OPOST | ONLCR | TAB3 | FF1;
	tbuf.c_cflag = baud | CS7 | CREAD | PARENB;
	tbuf.c_lflag = ISIG;
	ioctl(1, TCSETA, &tbuf);
/* Simulate XOFF with ioctl.	If printer is alive, it will send
an XON within 30 seconds. */

	for(i = 0; i < LINEMAX; i++) {
		buf2[i] = ' ';
		buf1[i] = '\0';
	}

	while(fgets(line,BUFSIZ,stdin) != NULL) {
/* Original program had a gets() instead of a fgets().  Original dumped
core when a non-ASCII file was received.  Gets() changes a '\n' at end
of line to a '\0'.  The following 3 lines restore the gets() appearance. */
		p = line;
		while(*p != '\0') p++;
		if(*(--p) == '\n') *p = '\0';

		p = line;
		bs = 0;
		if(*p == '\b')
			puts(line);	/* For double ht char. */
		else {
			while((c = *(p++)) != '\0') {
				switch(c) {
				case '\b':
					if(n1) {
						bs++;
						n1--;
					}
					break;
				case '\t':
					spaces = 8 - (n1 % 8);
					for(i = 1; i <= spaces; i++)
						buf1[n1++] = ' ';
					break;
				default:
					if(bs) {
						if(c == '_' ||
						   c == buf1[n1]) {
							n2 = n1++;
							buf2[n2] = '_';
						}
						else if(buf1[n1] == '_') {
							n2 = n1;
							buf2[n2] = '_';
							buf1[n1++] = c;
						}
						else {
							buf1[n1++] = c;
						}
						bs--;
					}
					else
						buf1[n1++] = c;
					if(n1 >= LINEMAX)
						n1--;
				}
			}
			flushbuf();
		}
	}
	restore();
}

flushbuf()
{
	int i;
	if(n1) {
		if(n2 >= 0) {
			buf1[n1++] = '\r';
			buf2[++n2] = '\0';
		}
		else
			buf1[n1++] = '\n';
		buf1[n1] = '\0';
		printf("%s", buf1);
		for(i = 0; i < n1; i++)
			buf1[i] = '\0';
		n1 = 0;
		if(n2 >= 0) {
			printf("%s\n", buf2);
			for(i = 0; i <= n2; i++)
				buf2[i] = ' ';
			n2 = -1;
		}
	}
	else
		printf("\n");
}

restore()
{
	ioctl(1, TCSETAW, &save);
	exit(0);
}
