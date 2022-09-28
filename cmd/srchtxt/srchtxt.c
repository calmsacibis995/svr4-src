/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)srchtxt:srchtxt.c	1.4.1.1"

#include	<stdio.h>
#include	<dirent.h>
#include	<regexpr.h>
#include	<string.h>
#include	<fcntl.h>
#include	<locale.h>
#include	<sys/types.h>
#include	<sys/file.h>
#include	<sys/mman.h>
#include	<sys/stat.h>

#define	P_locale	"/usr/lib/locale/"
#define	L_locale	(sizeof(P_locale))
#define	MESSAGES	"/LC_MESSAGES/"
#define	ESIZE		BUFSIZ

/* External functions */

extern	int	getopt();
extern	void	exit();
extern	char	*strecpy();
extern	char	*strrchr();
extern	char	*strchr();
extern	caddr_t mmap();
extern  void	close();
extern	void	munmap();


/* External variables */

extern	char	*optarg;
extern	int	optind;
extern	int	errno;
extern	int	sys_nerr;
extern	char	*sys_errlist[];

/* Internal functions */

static	void	usage();
static	void	prnt_str();
static	int	attach();
static	void	find_msgs();
static	char	*syserr();

/* Internal variables */

static	char	*cmdname; 	/* points to the name of the command */
static	int	lflg;		/* set if locale is specified on command line */
static	int	mflg;		/* set if message file is specified on command line */
static	char	locale[15];	/* save name of current locale */
static  char	*msgfile;       /* points to the argument immediately following the m option */
static	char	*text;		/* pointer to search pattern */
static	int	textflg;	/* set if text pattern is specified on command line */
static	int	sflg;		/* set if the s option is specified */
static	char	*fname;		/* points to message file name */
static	int	msgnum;		/* message number */


main(argc, argv)
int	argc;
char	*argv[];
{
	int	ch;
	char	*end;
	int	addr;
	int	len;
	int	len1;
	int	fd;
	size_t	size;
	char	pathname[128];
	char	*cp;
	char	ebuf[ESIZE];
	DIR	*dirp;
	struct	dirent	*dp;

	/* find last level of path in command name */
	if (cmdname = strrchr(*argv, '/'))
		++cmdname;
	else
		cmdname = *argv;

	/* parse command line */
	while ((ch = getopt(argc, argv, "sl:m:")) != -1)
		switch(ch) {
			case	'l':
				lflg++;
				(void)strcpy(locale, optarg);
				continue;
			case	'm':
				mflg++;
				msgfile = optarg;
				continue;
			case	's':
				sflg++;
				continue;
			default:
				usage();
			}
	if (mflg && optind < argc) {
		text = argv[optind++];
		textflg++;
	}
	if (optind != argc )
		usage();

	/* create full path name to message files */
	if (!lflg)
		(void)strcpy(locale, setlocale(LC_MESSAGES, ""));
	(void)strcpy(pathname, P_locale);
	(void)strcpy(&pathname[L_locale - 1], locale);
	(void)strcat(pathname, MESSAGES);
	len = strlen(pathname);

	if (textflg) {
			/* compile regular expression */
		if (compile(text, &ebuf[0], &ebuf[ESIZE]) == (char *)NULL) {
			(void)fprintf(stderr, "%s: ERROR: regular expression compile failed\n", cmdname);
			exit(1);
		}
	}

	/* access message files */ 
	if (mflg) {
		end = msgfile + strlen(msgfile) + 1;
		if (*msgfile == ',' || *(end - 2) == ',')
			usage();
		while ((fname = strtok(msgfile, ",\0")) != NULL) {
			if (strchr(fname, '/') != (char *)NULL) {
				cp = fname;
				len1 = 0;
			}
			else {
				cp = pathname;
				len1 = len;
			}	
			msgfile = msgfile + strlen(fname) + 1;
			if ((addr = attach(cp, len1, &fd, &size)) == -1) {
				(void)fprintf(stderr, "%s: ERROR: failed to access message file '%s'\n",cmdname, cp);
				if (end != msgfile)
					continue;
				else
					break;
			}
			find_msgs(addr, ebuf);
			munmap((caddr_t)addr, size);
			close(fd);
			if  (end == msgfile)
				break;
		}
	} /* end if (mflg) */
	else { 
		if((dirp = opendir(pathname)) == (DIR *) NULL) {
			(void)fprintf(stderr, "%s: ERROR: %s %s\n", cmdname, pathname, syserr());
			exit(1);
		}
		while( (dp = readdir(dirp)) != (struct dirent *)NULL ) {
			if (dp->d_name[0] == '.')
				continue;
			fname = dp->d_name;
			if ((addr = attach(pathname, len, &fd, &size)) == -1) {
				(void)fprintf(stderr, "%s: ERROR: failed to access message file '%s'\n",cmdname, pathname);
				continue;
			}
			find_msgs(addr, ebuf);
			munmap((caddr_t)addr, size);
			close(fd);
		}
		(void)closedir(dirp);
	}
	exit(0);
}


/* print usage message */
static void
usage()
{
	(void)fprintf(stderr, "usage: srchtxt [-s]\n       srchtxt [-s] -l locale\n       srchtxt [-s] [-l locale] [[-m msgfile,...] [text]]\n");
	exit(1);
}

/* print string - non-graphic characters are printed as alphabetic
		  escape sequences
*/
static	void
prnt_str(instring)
char	*instring;
{
	char	outstring[1024];

	(void)strecpy(outstring, instring, (char *)NULL);
	if (sflg)
		(void)fprintf(stdout, "%s\n", outstring);
	else
		(void)fprintf(stdout, "<%s:%d>%s\n", fname, msgnum, outstring);
}

/* mmap a message file to the address space */
static int
attach(path, len, fdescr, size)
char	*path;
int	len;
int	*fdescr;
size_t	*size;
{
	int	fd = -1;
	caddr_t	addr;
	struct	stat	sb;

	(void)strcpy(&path[len], fname);
	if ( (fd = open(path, O_RDONLY)) != -1 &&
		fstat(fd, &sb) != -1 && 
			(addr = mmap(0, sb.st_size,
				PROT_READ, MAP_SHARED,
					fd, 0)) != (caddr_t)-1 ) {
		*fdescr = fd; 
		*size = sb.st_size;
		return( (int) addr);
	}
	else {
		if (fd == -1)
			close(fd);
		return(-1);
	}
}


/* find messages in message files */
static void
find_msgs(addr, regexpr)
int	addr;
char	*regexpr;
{
	int	num_msgs;
	char	*msg;

	num_msgs = *(int *)addr; 
	for (msgnum = 1; msgnum<=num_msgs; msgnum++) {
		msg = (char *)(*(int *)(addr + sizeof(int) * msgnum) + addr);
		if (textflg)
			if (step(msg, regexpr)) {
				prnt_str(msg);
					continue;
				}
				else
					continue;
			prnt_str(msg);
	}
}

/* print description of error */ 
static char *
syserr()
{
	return (errno <= 0 ? "No error (?)"
	   : errno < sys_nerr ? sys_errlist[errno]
	   : "Unknown error (!)");
}
