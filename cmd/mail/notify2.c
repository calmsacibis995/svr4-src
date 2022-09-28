/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:notify2.c	1.5.3.1"
#include "mail.h"
#ifdef SVR4
# include <locale.h>
#endif

/*
 * notify2 - Do user-notification of new incoming mail message.
 *           Typically this program is used as the command in the
 *	     "Forward to |command" line. See also notify(1) which sets up
 *	     the user's mailfile to utilize this command.....
 */

extern char	*optarg;	/* for getopt */
extern char	*getenv();
extern void	notify();

static int dobackup();
static void securebuf();
static void notifyall();

main(argc, argv)
int	argc;
char	*argv[];
{
	char		*mailfile = (char *)NULL;
	char		*originator = (char *)NULL;
	char		*subject = (char *)NULL;
	register char	*logname;
	char		buf[2048];
	register int	c = 0;
	int		errflg = 0, n;
	FILE		*mfp, *tmpfp;

#ifdef SVR4
	(void)setlocale(LC_ALL, "");
#endif

	while ((c = getopt(argc, argv, "m:o:s:")) != EOF) {
		switch(c) {
		case 'm':
			/* place to put new messages */
			mailfile = optarg;
			break;
		case 'o':
			/* who sent this one */
			originator = optarg;
			break;
		case 's':
			/* value of Subject: line if present */
			subject = optarg;
			break;
		default:
			errflg++;
			break;
		}
	}

	if ((mailfile == (char *)NULL) ||
	    (originator == (char *)NULL) ||
	    (subject == (char *)NULL)) {
		errflg++;
	}

	if (errflg) {
		fprintf(stderr,
			"Usage: %s -m mailfile -o originator -s subject\n",
			argv[0]);
		exit(1);
	}

	umask (006);

	/* Open file user wants to use as the mailfile */
	if ((mfp = fopen(mailfile,"a")) == (FILE *)NULL) {
		if (!dobackup(mailfile, (FILE*)0)) {
			fprintf(stderr,"%s: cannot open '%s' for append\n",
				argv[0], mailfile);
			exit(2);
		}
	}

	/* Open backup temp file copy. */
	tmpfp = tmpfile();
	if (!tmpfp) {
		if (!dobackup(mailfile, (FILE*)0)) {
			fprintf(stderr, "%s: cannot create backup temp file\n", argv[0]);
			exit(2);
		}
	}

	/* Append the message to the mailfile */
	while ((n = fread(buf, 1, sizeof(buf), stdin)) > 0) {
		if ((fwrite(buf, 1, n, mfp) != n) ||
		    (fwrite(buf, 1, n, tmpfp) != n))
			if (!dobackup(mailfile, tmpfp)) {
				fprintf(stderr, "%s: cannot write file\n", argv[0]);
				exit(2);
			}
	}

	fclose (mfp);
	fclose(tmpfp);

	/*
	 * Who am I? Note that $LOGNAME is set by
	 * mail itself and is guaranteed to be valid.
	 */
	if ((logname = getenv("LOGNAME")) == (char *)NULL) {
		exit (0);
	}

	sprintf(buf, "\7\7\7New mail from '%s' appended to %s\r\n\7\7Subject: %s\r\n\n",
		originator, mailfile, subject);
	securebuf(buf);
	notifyall(logname, buf);
	exit (0);
	/* NOTREACHED */
}

/*
    Find out if there is a backup place to send the mail if the writes fail.
    The file /etc/mail/notify.fsys contains two columns. The first column
    contains a filesystem name. The second column contains a machine name to
    send the mail to, using $machine!$LOGNAME as the address.

    Return 0 if it cannot send the mail elsewhere. Otherwise do an exit(0);
*/
static int dobackup(mailfile, tmpfp)
    char *mailfile;
    FILE *tmpfp;
{
	char *lastsl;
	char buf[BUFSIZ];
	FILE *fsysfp = fopen("/etc/mail/notify.fsys", "r");
	if (!fsysfp)
		return 0;

	/* Strip the trailing filename from the mailfile path. */
	lastsl = strrchr(mailfile, '/');
	if (lastsl)
		*lastsl = '\0';
	else
		mailfile = "/";

	/* Look for the directory name */
	while (fgets(buf, sizeof(buf), fsysfp)) {
		char *fsys = strtok(buf, " \t\n");
		char *sysname = strtok((char*)0, " \t\n");

		/* Did we find the file system? */
		if (strcmp(fsys, mailfile) == 0) {
			/* Send the mail off to the remote system */
			char buf[BUFSIZ];
			char *logname = getenv("LOGNAME");
			FILE *mailcmd;
			if (!logname)
				return 0;

			sprintf(buf, "rmail %s!%s", sysname, logname);
			mailcmd = popen(buf, "w");
			if (!mailcmd)
				return 0;

			if (tmpfp) {
				rewind(tmpfp);
				if (!copystream(tmpfp, mailcmd)) {
					pclose(mailcmd);
					return 0;
				}
			}
			if (!copystream(stdin, mailcmd)) {
				pclose(mailcmd);
				return 0;
			}
			if (pclose(mailcmd) != 0)
				return 0;
			exit(0);
		}
	}
	return 0;
}

/*
    Make certain that non-printable characters
    within the mail message become printable.
*/
static void securebuf(buf)
    register char *buf;
{
    for ( ; *buf; buf++)
	if (!isprint(*buf) && !isspace(*buf) && (*buf != '\07'))
		*buf = '!';
}

/*
    Look up the list of systems in /etc/mail/notify.sys.
    The 1st column contains a system name and the 2nd
    column contains the path to the root for that system.
*/
static void notifyall(user, msg)
    char *user;
    char *msg;
{
    FILE *sysfp = fopen("/etc/mail/notify.sys", "r");

    /*
     * Look for the root paths. Skip the
     * line for the current system.
     */
    if (sysfp) {
	char buf[BUFSIZ];
	struct	utsname utsn;
	uname(&utsn);
	while (fgets(buf, sizeof(buf), sysfp)) {
	    char *sys = strtok(buf, " \t\n");
	    char *dir = strtok((char*)0, " \t\n");
	    if (strcmp(sys, utsn.sysname) != 0)
		notify(user, msg, 1, dir);
	}
	fclose(sysfp);
    }
    notify(user, msg, 1, "");
}
