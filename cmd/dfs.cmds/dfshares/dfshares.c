/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)dfs.cmds:dfshares/dfshares.c	1.3.3.1"

/*
	generic interface to dfshares, dfmounts.

	usage:	dfshares [-F fstype] [-o fs_options] [-h] [ args ]

	exec's /usr/lib/fs/<fstype>/<cmd>
	<cmd> is the basename of the command.

	if -F is missing, fstype is the first entry in /etc/dfs/fstypes
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>

#define DFSTYPES	"/etc/dfs/fstypes"		/* dfs list */
#define	FSCMD		"/usr/lib/fs/%s/%s"

#define	ARGVPAD		5	/* non-[arg...] elements in new argv list:
				   cmd name, , -h, -o, opts, (char *)0 terminator */

static char *getfs();
void perror();

main(argc, argv)
int argc;
char **argv;
{
	char *malloc();
	extern int getopt(), execvp();
	extern void exit();
	static int invalid();
	extern char *optarg;
	extern int optind;
	FILE *dfp;		/* fp for dfs list */
	int c, err = 0; 
	char subcmd[BUFSIZ];		/* fs specific command */
	char *cmd;			/* basename of this command */
	char *fsname = NULL;	/* file system name */
	char *opts = NULL;	/* -o options */
	char **nargv;		/* new argv list */
	int hflag = 0;
	int nargc = 0;		/* new argc */
	pid_t pid;		/* pid for fork */
	int retval;		/* exit status from exec'd commad */
	int showall = (argc <= 1);	/* show all resources */
	static char usage[] =
		"usage: %s [-F fstype] [-h] [-o fs_options ] [arg ...]\n";

	cmd = strrchr(argv[0], '/');	/* find the basename */
	if (cmd)
		++cmd;
	else
		cmd = argv[0];

	while ((c = getopt(argc, argv, "hF:o:")) != -1)
		switch (c) {
		case 'h':
			hflag = 1;	/* no header ... pass to subcommand */
			break;
		case 'F':
			err |= (fsname != NULL);	/* at most one -F */
			fsname = optarg;
			break;
		case 'o':		/* fs specific options */
			err |= (opts != NULL);	/* at most one -o */
			opts = optarg;
			break;
		case '?':
			err = 1;
			break;
		}
	if (err) {
		(void)fprintf(stderr, usage, cmd);
		exit(1);
	}

	if ((dfp = fopen(DFSTYPES, "r")) == NULL) {
		(void)fprintf(stderr, "%s: cannot open %s\n", cmd, DFSTYPES);
		(void)fprintf(stderr, "%s: possible cause: RFS/NFS not installed\n", cmd);
		exit(1);
	}

	/* allocate a block for the new argv list */
	if (!(nargv = (char **)malloc(sizeof(char *)*(argc-optind+ARGVPAD)))) {
		(void)fprintf(stderr, "%s: malloc failed.\n", cmd);
		exit(1);
	}
	nargv[nargc++] = cmd;
	if (hflag)
		nargv[nargc++] = "-h";
	if (opts) {
		nargv[nargc++] = "-o";
		nargv[nargc++] = opts;
	}
	for (; optind <= argc; ++optind)	/* this copies the last NULL */
		nargv[nargc++] = argv[optind];

	if (showall) {		/* command with no args -- show all dfs's */
		while (fsname = getfs(dfp)) {
			(void)sprintf(subcmd, FSCMD, fsname, cmd);
			switch (pid = fork()) {		/* do the subcommand */
			case 0:
				(void)execvp(subcmd, nargv);
				perror(subcmd);
				exit(1);
				/*NOTREACHED*/
			default:
				while (wait(&retval) != pid) ;
				/* take exit status into account */
				err |= (retval & 0xff00) >> 8;
				break;
			case -1:
				(void)fprintf(stderr, "%s: fork failed - try again later.\n",
					cmd);
				exit(1);
			}
		}
		(void)fclose(dfp);
		if (pid == 0) {		/* we never got into the loop! */
			(void)fprintf(stderr, "%s: no file systems in %s\n", cmd, DFSTYPES);
			(void)fprintf(stderr, usage, cmd);
			exit(1);
		}
		else
			exit(err);
	}

	if (fsname) {		/* generate fs specific command name */
		if (invalid(fsname, dfp)) {	/* valid ? */
			(void)fprintf(stderr, "%s: invalid file system name\n", cmd);
			(void)fprintf(stderr, usage, cmd);
			exit(1);
		}
		else
			(void)sprintf(subcmd, FSCMD, fsname, cmd);
	}
	else if (fsname = getfs(dfp))		/* use 1st line in dfstypes */
		(void)sprintf(subcmd, FSCMD, fsname, cmd);
	else {
		(void)fprintf(stderr, "%s: no file systems in %s\n", cmd, DFSTYPES);
		(void)fprintf(stderr, usage, cmd);
		exit(1);
	}

	(void)execvp(subcmd, nargv);
	perror(subcmd);				/* execvp failed */
	exit(1);
	/*NOTREACHED*/
}


/*
	invalid(name, f)  -  return non-zero if name is not in
			     the list of fs names in file f
*/

static int
invalid(name, f)
char *name;		/* file system name */
FILE *f;		/* file of list of systems */
{
	char *s;

	while (s = getfs(f))	/* while there's still hope ... */
		if (strcmp(s, name) == 0)
			return 0;	/* we got it! */
	return 1;
}


/*
   getfs(fp) - get the next file system name from fp
	       ignoring lines starting with a #.
	       All leading whitespace is discarded.
*/

static char buf[BUFSIZ];

static char *
getfs(fp)
FILE *fp;
{
	register char *s;

	while (s = fgets(buf, BUFSIZ, fp)) {
		while (isspace(*s))	/* leading whitespace doesn't count */
			++s;
		if (*s != '#') {	/* not a comment */
			char *t = s;

			while (!isspace(*t))	/* get the token */
				++t;
			*t = '\0';		/* ignore rest of line */
			return s;
		}
	}
	return NULL;	/* that's all, folks! */
}
