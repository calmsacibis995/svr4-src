/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pkging:flop_disk.c	1.3.1.1"

/*
** Present the user with the:
**	Strike "C" to install from Cartridge Tape
**	or "F" to install from Floppy Diskette.
**
**	Strike "EST" to stop.
**
**	Exit 0 for floppy,
**	Exit 1 for Tape
**	Exit 100 for ESC
*/


#include <stdio.h>
#include <termio.h>
#include <fcntl.h>
#include <signal.h>

int RC = (-1);

void timer()
{
	exit (RC);
}

struct termio newtty, oldtty;

main()
{
	int b, looping;
	char buf[BUFSIZ+1];

	ioctl (0, TCGETA, &oldtty);
	newtty = oldtty;
	newtty.c_lflag &= ~(ICANON|ECHO|ISIG);
	newtty.c_iflag &= ~ICRNL;
	newtty.c_cc[VMIN] = 1;
	newtty.c_cc[VTIME] = 1;
	ioctl (0, TCSETAW, &newtty);
	looping = 1;
	printf ("Confirm\n\nPlease indicate the installation medium you intend to use.\n\n");
	printf ("Strike \"C\" to install from CARTRIDGE TAPE\nor \"F\" to install from FLOPPY DISKETTE.\n");
	printf ("\nStrike ESC to stop.");
	b = 0;
	while (looping) {
		fflush (stdout);
		if (read (0, &b, 1) < 0)
			continue;
		switch (b) {
			case 'C':
			case 'c':
			case 'F':
			case 'f':
			case 033:
				looping = 0;
				break;
			default:
				printf ("\007");
				break;
		}
	}
	ioctl (0, TCSETAW, &oldtty);
	signal (SIGALRM, timer);
	printf ("\n");
	if (b == '\033') {
		RC=100;
	} else if ((b == 'C') || (b == 'c')) {
		RC=1;
	} else
		RC=0;
	alarm (2);
	fgets(buf, BUFSIZ, stdin);   /* Flush newline some might have entered */
	exit (RC);
}
