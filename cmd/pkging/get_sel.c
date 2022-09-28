/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pkging:get_sel.c	1.3"

/*
**	Read from stdin until ESC or top to highest numbers are selected.
**	If ESC is pressed, return all numbers inputed upto the max.
**	Beep if anything else -OR- number out of range.
*/

#include <stdio.h>
#include <termio.h>
#include <fcntl.h>

struct termio newtty, oldtty;

main(argc, argv)
int argc;
char *argv[];
{
	int max, b, looping;
	int i, j = 0;
	char buf[BUFSIZ];

	if (argc != 2) {
		printf ("Usage %s number\n", argv[0]);
		exit (100);
	}
	max = atoi(argv[1]);
	if (max < 3) {
		printf ("Number must be >= 3\nUsage %s number\n", argv[0]);
		exit (100);
	}
	chg_stty(0);
	looping = 1;
	while (looping) {
		printf ("Enter Package Number: ");
		if (jgets (buf, BUFSIZ-1, stdin) < 0)
			strcpy(buf,"0");
		printf ("\n\033[A\r");
		if (buf[0] == '\033') {
			end(0);
		}
		b = atoi (buf);
		if ((b <= 0) || (b > max)) {
			printf ("                         \033[AIllegal number\r");
			continue;
		} else {
			printf ("                       \033[A                                              \n");
		}
		if (b == max) { /* CANCEL */
			end (1);
		}
		if (b == (max - 1)) { /* ALL */
		   for (b = 1; b < max - 1; b++)
			fprintf (stderr, "%d\n", b);
		   end (0);
		}
		fprintf (stderr,"%d\n", b);
		j++;
		for (i = 0; i < j; i++)
			printf ("\033[C\033[C  ");
		printf ("\033[B%d", b);
		printf ("\033[A\r");
		printf ("\033[A\r");
	}
	end (0);
}

end(i)
int i;
{
	chg_stty (1);
	printf ("                                                                              \n");
	printf ("                                                                              \r");
	fflush (stdout);
	exit (i);
}

chg_stty(flg)
{
	if (flg == 0) {
		ioctl (0, TCGETA, &oldtty);
		newtty = oldtty;
		newtty.c_lflag &= ~(ICANON|ECHO|ISIG);
		newtty.c_iflag &= ~ICRNL;
		newtty.c_cc[VMIN] = 1;
		newtty.c_cc[VTIME] = 1;
		ioctl (0, TCSETAW, &newtty);
	} else {
		ioctl (0, TCSETAW, &oldtty);
	}
}

jgets (bufp, length, fp)
char *bufp;
int length;
FILE *fp;
{
	int looping = 1;
	int index = 0;
	fflush (stdout);
	while (looping) {
		bufp[index] = 0;
		if (read (0, &bufp[index], 1) < 0)
			continue;
		switch (bufp[index]) {
			case 033:
			case '\r':
			case '\n': looping = 0;
				   write (1, "\n", 1);
				   break;
			case '\b':
				   write (1, &bufp[index], 1);
				   break;
			default:
				   if ((bufp[index] < ' ') || (bufp[index] > '~')) {
					   write (1, "\n", 1);
					   return (-1);
				   }
				   write (1, &bufp[index], 1);
				   index++;
				   break;
		}
	}
	return (1);
}
