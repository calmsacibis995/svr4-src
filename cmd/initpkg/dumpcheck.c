/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)initpkg:dumpcheck.c	1.1.3.1"

/*
 *	dumpcheck
 *
 *	Checks to see if there is a panic dump on the swap device
 *	and if there is, asks whether the dump should be saved.
 *
 *	If TIME is defined as 'n' in /etc/default/dump, dumpcheck will
 *	wait 'n' seconds before timing out and assuming a 'no' answer.
 *	If TIME is zero, the question will not be asked and no save
 *	will be done.  If TIME is negative or /etc/default/dump does
 *	not exist, dumpcheck will not time out.
 */

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <termio.h>
#include <deflt.h>

#ifdef MBUS
#define CMPSIZ	1024
#else
#define CMPSIZ	512
#endif

#define	DEFFILE	"/etc/default/dump"

main()
{
	extern char *defread();
	extern int defopen(), defcntl();
	void wakeup();
	int memfd, swapfd, flags, timeout = -1;
	char *ptr, membuf[CMPSIZ], swapbuf[CMPSIZ], ans[BUFSIZ];
	struct termio term_setting;

	/* open /dev/mem and /dev/rswap for reading */
	if ((memfd = open("/dev/mem", O_RDONLY)) == -1) {
		perror("dumpcheck: cannot open /dev/mem");
		exit(1);
	}
	if ((swapfd = open("/dev/rswap", O_RDONLY)) == -1) {
		perror("dumpcheck: cannot open /dev/rswap");
		exit(1);
	}

	/* read CMPSIZ bytes from /dev/mem & /dev/rswap */
	if (read(memfd, membuf, CMPSIZ) != CMPSIZ) {
		perror("dumpcheck: cannot read /dev/mem");
		exit(1);
	}
	if (read(swapfd, swapbuf, CMPSIZ) != CMPSIZ) {
		perror("dumpcheck: cannot read /dev/rswap");
		exit(1);
	}

	/* if they are not identical, there is no dump - exit */
	if (memcmp(membuf, swapbuf, CMPSIZ) != 0)
		exit(0);

	/* attempt to open default file to get TIME value */
	if (defopen(DEFFILE) == 0) {
		/* ignore case */
		flags = defcntl(DC_GETFLAGS, 0);
		TURNOFF(flags, DC_CASE);
		defcntl(DC_SETFLAGS, flags);

		if ((ptr = defread("TIME=")) != NULL)
			timeout = atoi(ptr);
		defopen(NULL);
	}

	if (timeout == 0)		/* TIME is 0 - exit immediately*/
		exit(0);
	else if (timeout > 0) {		/* set timeout for "save" question */
		signal(SIGALRM, wakeup);
		alarm(timeout);
	}

	/* ensure sane console port settings */
	ioctl(fileno(stdin), TCGETA, &term_setting);
	term_setting.c_iflag |= ICRNL;
	term_setting.c_oflag |= (OPOST | ONLCR);
	term_setting.c_lflag |= (ICANON | ECHO);
	ioctl(fileno(stdin), TCSETA, &term_setting);

	while (1) {
		printf("There may be a system dump memory image in the swap device.\n");
		printf("Do you want to save it? (y/n)> ");
		fflush(stdout);
		scanf("%s", ans);
		if (*ans == 'Y' || *ans == 'y') {
			/* save the dump */
			alarm(0);	/* cancel alarm */
			execl("/sbin/sh", "sh", "-c", "/sbin/dumpsave", NULL);
			perror("dumpcheck: cannot exec /sbin/sh");
			exit(1);
		}
		else if (*ans == 'N' || *ans == 'n')
			exit(0);
		else
			printf("???\n");
	}
}

/*
 *	This routine executes when the "save the dump" question has
 *	timed out.  Print "timeout." to the console and exit.
 */
void
wakeup()
{
	printf("timeout.\n\n");
	printf("No system dump image will be saved.\n");
	exit(2);
}
