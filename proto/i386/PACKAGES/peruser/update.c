/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)proto:i386/PACKAGES/peruser/update.c	1.3.1.1"

#include <stdio.h>
#ifndef BOOTFLOP
#include <termio.h>
#include <values.h>
#include <string.h>
#endif

#include <fcntl.h>
#ifdef i386
#include <sys/sysi86.h>
#endif

#ifndef BOOTFLOP
struct termio newtty, oldtty;
int whoami = 0;
#endif

#ifndef MAX_LIM
#define MAX_LIM  2
#endif

#define BIG_VAL 100
#ifndef DFLT_LEVEL
#define DFLT_LEVEL 16
#endif

#define REMOVE 1
#define INSTALL 2
#define INITIAL 3

main(argc, argv)
int argc;
char *argv[];
{
#ifdef BOOTFLOP
	exit (update(DFLT_LEVEL));
#else
	int a, b, looping;
	char *prog;

	if ((argc > 4) || (argc == 3)) {
		printf ("Usage: %s args ...\n", argv[0]);
		exit (1);
	}
	
	if ((prog = strrchr(argv[0],'/')) == NULL)
		prog = argv[0];
	else
		prog++;
	if (strcmp("initial", prog) == 0)
		whoami = INITIAL;
	else if (strcmp("update", prog) == 0)
		whoami = INSTALL;
	else
		whoami = REMOVE;
#ifdef i386
	if ((a = sysi86(SI86LIMUSER,0)) < 0) {
		printf ("This is not a UNIX System V/386 Release 4.0 Version 1 or \
higher system.\n");
		exit (1);
	}
#else
	a = DFLT_LEVEL;
#endif
	if (whoami != REMOVE)
	   if ((b = MAX_LIM) == BIG_VAL)
		b = MAXINT;
	if ((strcmp(argv[1], "1") == 0) && (strcmp(argv[2],"2") == 0) &&
	    (strcmp(argv[3],"3") == 0)) {
	    if (a == b)
		exit (0);
	    else
		if (a == MAXINT)
			exit (BIG_VAL);
		exit (a);
	}
	if (whoami == REMOVE) {
		b = a;
		a = DFLT_LEVEL;
	}
	if (whoami == INITIAL) {
		exit (update(DFLT_LEVEL));
	}
	if ((whoami != REMOVE) && (argc == 2)) {
		exit (update(atoi(DFLT_LEVEL)));
	}

     if (a != b) {
	if ((whoami != REMOVE) && (a != DFLT_LEVEL)) { /* Installing a unlimited user on top of a 32 user */
		if (a < DFLT_LEVEL)
			exit (wrong_version(a, b));
		printf ("\nConfirm\n\nThe UNIX System V/386 Release 4.0 ");
		if (a == MAXINT)
			printf ("Unlimited");
		else
			printf ("%d", a);
		printf (" User License\nPackage is already installed on the system.\n\n");
		printf ("This installation will remove this package and replace it \
with\na User License permitting ");
		if (b == MAXINT)
			printf ("Unlimited");
		else
			printf ("%d", b);
		printf (" concurrent users.\n");
	} else {
	printf ("\nConfirm\n\nYou are about to %s the UNIX System V/386 Release 4.0\n", ((whoami == REMOVE) ? "remove":"install"));
	if (b == MAXINT)
		printf ("Unlimited");
	else
		printf ("%d", b);
	printf (" User License Package.");
     if (whoami == REMOVE) {
	printf ("\n\nThis will reduce the number of concurrent users that your\n\
system can support to %d.\n", a);
     } else {
	printf ("\n\nYour system can currently support ");
	if (a == MAXINT)
		printf ("Unlimited");
	else
		printf ("%d", a);
	printf (" users.\n\nThis update will enable your system to support ");
	if (b == MAXINT)
		printf ("Unlimited");
	else
		printf ("%d", b);
	printf (" concurrent users.\n");
     }
     }
	printf ("\nStrike ENTER when ready\nor ESC to stop.");

	ioctl (0, TCGETA, &oldtty);
	newtty = oldtty;
	newtty.c_lflag &= ~(ICANON|ECHO|ISIG);
	newtty.c_iflag &= ~ICRNL;
	newtty.c_cc[VMIN] = 1;
	newtty.c_cc[VTIME] = 1;
	ioctl (0, TCSETAW, &newtty);
	looping = 1;
	a = 0;
	while (looping) {
		fflush (stdout);
		if (read (0, &a, 1) < 0)
			continue;
		switch (a) {
			case 012:
			case 015:
				a = '\n';
			case 033:
				looping = 0;
				break;
			default:
				printf ("\007");
				break;
		}
	}
	ioctl (0, TCSETAW, &oldtty);
	printf ("\n");
	if (a == '\033')
		exit (1);
    } /* End a != b */
	/*
	** Update
	*/
	printf ("\n\n");
	fflush (stdout);
     	if (whoami == REMOVE)
		b = DFLT_LEVEL;
	else
		system ("echo Installing the `echo ${NAME}`");
	exit (update (b));
#endif /* BOOTFLOP */
}

update(val)
int val;
{
	FILE *fp, *fp1;
	char buf[BUFSIZ], *ptr;
	int found = 0;
	extern char *fgets();
	if (val < 2)
		return (1);
	chdir ("/etc"); chdir ("conf"); chdir ("pack.d");
	chdir ("../../usr/bin"); chdir ("../../../usr");
	chdir ("../etc/conf/pack.d"); chdir ("RFS"); chdir ("../kernel");
	if ((fp = fopen ("space.c","r")) == NULL)
		return (1);
	if ((fp1 = fopen ("space.1","w")) == NULL) {
		fclose (fp);
		return (1);
	}
	while (fgets(buf, BUFSIZ-1, fp)) {
		if (!found) {
			ptr = buf;
			if ((strncmp (buf,"int",3) == 0) &&
			    ((*(ptr+3) == ' ') || (*(ptr+3) == '\t')) &&
			    (strncmp (&buf[4],"eua_lim_ma", 10) == 0)) {
				sprintf (buf, "int	eua_lim_ma = %d;\n", val);
				found = 1;
			}
		}
		fputs (buf, fp1);
	}
	fclose (fp); fclose (fp1);
	if (!found) {
		printf ("This system will not allow per user licensing.\n");
		unlink("space.1");
		return (1);
	}
	unlink ("space.c");
	if (link ("space.1", "space.c") < 0)
		return (1);
	unlink ("space.1");
#ifndef BOOTFLOP
	if (whoami == REMOVE)
		return (idbuild(val));
	else
#endif
		return (0);
}

#ifndef BOOTFLOP
idbuild(val)
int val;
{
	int a;
	a = system ("/etc/conf/bin/idbuild 2>/tmp/idb_b");

	if (a == 0) {
		printf ("\nYour system has now been configured to support\n\
%d users, once the UNIX System has been re-booted.\n\n\
To allow more than this number to login at the same time,\n\
it will be necessary to install one of the User License\n\
Packages available for this system.\n\nStrike ENTER when ready.", val);
	} else {
		printf ("\nAn error occurred while removing this package.\n\
Please look at the log file /tmp/idb_b to correct the problem.\n\n\
After the problem is corrected, please run removepkg again.");
#ifdef i386
		whoami = INSTALL;
		update (sysi86(SI86LIMUSER,0));
#endif /* i386 */
	}
	getchar();
	return (a);
}

wrong_version(a, b)
int a, b;
{
	printf ("\n\nThis package upgrades you from a %d User System to ", DFLT_LEVEL);
	if (b == MAXINT)
		printf ("Unlimited");
	else
		printf ("%d", b);
	printf (" User License. Your\n");

	printf ("system is currently a %d User System and therefore this upgrade cannot be used.\n\n", a);
	return (1);
}
#endif /* BOOTFLOPY */
