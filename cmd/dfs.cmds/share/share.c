/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)dfs.cmds:share/share.c	1.10.3.1"

/*
	generic interface to share

	usage:	share [-F fstype] [-o fs_options] [-d desc] [ args ]

	exec's /usr/lib/fs/<fstype>/<cmd>
	<cmd> is the basename of the command.

	if -F is missing, fstype is the first entry in /etc/dfs/fstypes
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <nserve.h>

#define DFSTYPES	"/etc/dfs/fstypes"		/* dfs list */
#define	FSCMD		"/usr/lib/fs/%s/%s"
#define	SHAREFILE	"/etc/dfs/sharetab"
#define	MAXFIELD	5
#define	BUFSIZE		512

#define	ARGVPAD		6	/* non-[arg...] elements in new argv list:
				   cmd name, -o, opts, -d, desc, (char *)0
				   terminator */

static char *fieldv[MAXFIELD];
static struct	stat	stbuf;
static char *cmd;		/* basename of this command */
static char *getfs();
void perror();

main(argc, argv)
int argc;
char **argv;
{
	extern int getopt();
	char *malloc();
	static int invalid();
	extern char *optarg;
	extern int optind;
	FILE *dfp;		/* fp for dfs list */
	int c, err = 0; 
	char subcmd[BUFSIZ];	/* fs specific command */
	char *fsname = NULL;	/* file system name */
	char *desc = NULL;	/* for -d */
	char *opts = NULL;	/* -o options */
	char **nargv;		/* new argv list */
	int nargc = 0;		/* new argc */
	int optnum;
	pid_t pid;		/* pid for fork */
	int retval;		/* exit status from exec'd commad */
	int showall = (argc <= 1);	/* show all resources */
	static char usage[] =
		"usage: %s [-F fstype] [-o fs_options ] [-d description] [pathname [resourcename]]\n";

	cmd = strrchr(argv[0], '/');	/* find the basename */
	if (cmd)
		++cmd;
	else
		cmd = argv[0];

	while ((c = getopt(argc, argv, "F:d:o:")) != -1)
		switch (c) {
		case 'F':
			err |= (fsname != NULL);	/* at most one -F */
			fsname = optarg;
			break;
		case 'd':			/* description */
			err |= (desc != NULL);		/* at most one -d */
			desc = optarg;
			break;
		case 'o':			/* fs specific options */
			err |= (opts != NULL);		/* at most one -o */
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
	optnum = optind;
	nargv[nargc++] = cmd;
	if (opts) {
		nargv[nargc++] = "-o";
		nargv[nargc++] = opts;
	}
	if (desc) {
		nargv[nargc++] = "-d";
		nargv[nargc++] = desc;
	}
	for (; optind <= argc; ++optind)	/* this copies the last NULL */
		nargv[nargc++] = argv[optind];

	if (showall) {		/* share with no args -- show all dfs's */
		while (fsname = getfs(dfp)) {
				list_res(fsname);
			}
		(void)fclose(dfp);
		exit(0);
	}

	if (fsname) {		/* generate fs specific command name */
		if (invalid(fsname, dfp)) {	/* valid ? */
			(void)fprintf(stderr, "%s: invalid file system name\n", cmd);
			(void)fprintf(stderr, usage, cmd);
			exit(1);
		}
		if (argc <= 3 && argc == optnum) {	/* list shared resources for
					   share -F fstype */
			list_res(fsname);
			exit(0);
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
	perror(subcmd);
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

/* list data in /etc/dfs/sharetab in adv(1) format */

static
list_res(fsname)
char *fsname;
{
	char	res[SZ_RES + 1];
	char	advbuf[BUFSIZE];
	char	*clnt, *s;
	FILE	*fp;


	if (stat(SHAREFILE,&stbuf) != -1) {
		if ((fp = fopen(SHAREFILE, "r")) == NULL) {
			fprintf(stderr,"%s: cannot open <%s>\n",cmd,SHAREFILE);
			exit(1);
		}
		while (fgets(advbuf,BUFSIZE,fp)) {
			get_data(advbuf);
			if (strcmp(fieldv[2], fsname) == 0) {
				printf("%-14.14s", fieldv[1]);
				printf("  %s  ", fieldv[0]);
				printf(" %s ", fieldv[3]);
				if (*fieldv[4])		/* description */
					printf("  \"%s\" ", fieldv[4]);
				else	printf("  \"\"  ");
				printf("\n");
			}
		}
		fclose(fp);
	}
}

char empty[] = "";

static
get_data(s)
char	*s;
{
	register int fieldc = 0;

	/*
 	 *	This function parses an advertise entry from 
 	 *	/etc/dfs/sharetab and sets the pointers appropriately.
	 *	fieldv[0] :  pathname
	 *	fieldv[1] :  resource
	 *	fieldv[2] :  fstype
	 *	fieldv[3] :  options
	 *	fieldv[4] :  description
 	 */

	while ((*s != '\n') && (*s != '\0') && (fieldc < 5)) {
		while (isspace(*s))
			s++;
		fieldv[fieldc++] = s;

		if ( fieldc == 5) {	/* get the description field */
			if (fieldv[4][strlen(fieldv[4])-1] == '\n')
				fieldv[4][strlen(fieldv[4])-1] = '\0';
			break;
		}
		while (*s && !isspace(*s)) ++s;
		if (*s)
			*s++ = '\0';
	}
	while (fieldc <5)
		fieldv[fieldc++] = empty;
}
