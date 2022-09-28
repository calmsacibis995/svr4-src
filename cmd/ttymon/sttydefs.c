/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ttymon:sttydefs.c	1.11.6.1"

# include <stdio.h>
# include <unistd.h>
# include <stdlib.h>
# include <errno.h>
# include <sys/types.h>
# include <ctype.h>
# include <string.h>
# include <termio.h>
# include <sys/stat.h>
# include <signal.h>

# include "tmstruct.h"
# include "ttymon.h"

#ifdef MERGE386
long	sc_flag;
#endif

static	int  nflg = 0;		/* -n seen */
static	int  iflg = 0;		/* -i seen */
static	int  fflg = 0;		/* -f seen */
static	int  lflg = 0;		/* -l seen */

struct  Gdef Gdef[MAXDEFS];	/* array to hold entries in /etc/ttydefs */
int	Ndefs = 0;		/* highest index to Gdef that was used   */
char	Scratch[BUFSIZ];
static	struct Gdef DEFAULT = {		/* default terminal settings	*/
	"default",
	"9600",
	"9600 sane",
	0,
	/* 
	 * next label is set to 4800 so we can start searching ttydefs.
	 * if 4800 is not in ttydefs, we will loop back to use DEFAULT 
	 */
	"4800"
};


extern	void	log();

static	void	usage();
static	void	check_ref();
static	void	add_entry();
static	void	remove_entry();
static	int	copy_file();
static	int	verify();
static	FILE	*open_temp();
extern  void	read_ttydefs();
extern  int	check_version();
extern  int	find_label();

/*
 *	sttydefs - add, remove or check entries in /etc/ttydefs
 *
 *	Usage:	   sttydefs -a ttylabel [-n nextlabel] [-i initail-flags] 
 *			    [-f final-flags] [-b]
 *		   sttydefs -r ttylabel
 *		   sttydefs -l [ttylabel]
 *
 */

main(argc, argv)
int argc;
const	char *argv[];
{
	int c;			/* option letter */
	int errflg = 0;		/* error indicator */
	int  aflg = 0;		/* -a seen */
	int  bflg = 0;		/* -b seen */
	int	ret;
	const	char	*argtmp;
	char	*nextlabel;
	struct Gdef ttydef, *ptr;

	extern	char	*optarg;
	extern	int	optind;

	if (argc == 1)
		usage();
	ptr = &ttydef;
	ptr->g_autobaud = 0;
	while ((c = getopt(argc, argv, "a:n:i:f:br:l")) != -1) {
		switch (c) {
		case 'a':
			aflg = TRUE;
			ptr->g_id = optarg;
			break;
		case 'n':
			nflg = TRUE;
			ptr->g_nextid = optarg;
			break;
		case 'i':
			iflg = TRUE;
			ptr->g_iflags = optarg;
			break;
		case 'f':
			fflg = TRUE;
			ptr->g_fflags = optarg;
			break;
		case 'b':
			bflg = TRUE;
			ptr->g_autobaud |= A_FLAG;
			break;
		case 'r':
			if ((argc > 3) || (optind < argc))
				usage();
			remove_entry(optarg);
			break;
		case 'l':
			lflg = TRUE;
			if (argc > 3) 
				usage();
			if ((ret = check_version(TTYDEFS_VERS, TTYDEFS)) != 0) {
				if (ret != 2) {
					(void)fprintf(stderr, "%s version number is incorrect or missing.\n",TTYDEFS);
					exit(1);
				}
				(void)fprintf(stderr, "sttydefs: can't open %s.\n",TTYDEFS);
				exit(1);
			}
			if (argv[optind] == NULL) {
				read_ttydefs(NULL,TRUE);
				printf("\n");
				check_ref();
			}
			else {
				if (argc == 3) { /* -l ttylabel */
					if (verify(argv[optind],0) != 0) {
						errflg++;
						break;
					}
					argtmp = argv[optind];
				}
				else { /* -lttylabel */
					argtmp = argv[optind]+2;
				}
				read_ttydefs(argtmp,TRUE);
				if (Ndefs == 0) {
					(void)fprintf(stderr,
					"ttylabel <%s> not found.\n", argtmp);
					exit(1);
				}
				nextlabel = Gdef[--Ndefs].g_nextid;
				Ndefs = 0;
				read_ttydefs(nextlabel,FALSE);
				if (Ndefs == 0) {
					(void)printf("\nWarning -- nextlabel <%s> of <%s> does not reference any existing ttylabel.\n", 
					nextlabel, argtmp);
				}
			}
			exit(0);
			break;		/*NOTREACHED*/
		case '?':
			errflg++;
			break;
		} /* end switch */
		if (errflg) 
			usage();
	} /* end while */
	if (optind < argc) 
		usage();

	if (aflg) {
		add_entry(ptr); 	/* never return */
	}
	if ((iflg) || (fflg) || (bflg) || (nflg)) 
		usage();
	exit(0); 
	/* NOTREACHED */
}

/*
 *	verify	- to check if arg is valid
 *		- i.e. arg cannot start with '-' and
 *		  arg must not longer than maxarglen
 *		- return 0 if ok. Otherwise return -1
 */
static	int
verify(arg,maxarglen)
char	*arg;
int	maxarglen;
{
	if (*arg == '-') {
		(void)fprintf(stderr, "Invalid argument -- %s.\n", arg);
		return(-1);
	}
	if ((maxarglen) && ((int)strlen(arg) > maxarglen)) {
		arg[maxarglen] = '\0';
		(void)fprintf(stderr,"string too long, truncated to %s.\n",arg);
		return(-1);
	}
	return(0);
}

/*
 * usage - print out a usage message
 */

static	void
usage()
{
	(void)fprintf(stderr, "Usage:\tsttydefs -a ttylabel [-n nextlabel] [-i initial-flags]\n\t\t [-f final-flags] [-b]\n");
	(void)fprintf(stderr, "\tsttydefs -r ttylabel\n");
	(void)fprintf(stderr, "\tsttydefs -l [ttylabel]\n");
	exit(2);
}

/*
 *	add_entry - add an entry to /etc/ttydefs
 */

static	void
add_entry(ttydef)
struct Gdef *ttydef;
{
	FILE *fp;
	int	errflg = 0;
	char tbuf[BUFSIZ], *tp;
	int  add_version = FALSE;
	extern	int	check_flags();

	if (getuid()) {
		(void)fprintf(stderr, "User not privileged for operation.\n");
		exit(1);
	}
	tp = tbuf;
	*tp = '\0';
	if ((fp = fopen(TTYDEFS, "r")) != NULL) {
		if (check_version(TTYDEFS_VERS, TTYDEFS) != 0) {
			(void)fprintf(stderr, 
			"%s version number is incorrect or missing.\n",TTYDEFS);
			exit(1);
		}
		if (find_label(fp,ttydef->g_id)) {
			(void)fclose(fp);
			(void)fprintf(stderr,
			"Invalid request -- ttylabel <%s> already exists.\n",
			ttydef->g_id);
			exit(1);
		}	
		(void)fclose(fp);
	}
	else  {
		add_version = TRUE;
	}
	if ((fp = fopen(TTYDEFS, "a+")) == NULL) {
		(void)sprintf(Scratch, "Could not open \"%s\"", TTYDEFS);
		perror(Scratch);
		exit(1);
	}

	if (add_version) {
	   (void)fprintf(fp,"# VERSION=%d\n", TTYDEFS_VERS);
	}


	/* if optional fields are not provided, set to default */
	if (!iflg)
		ttydef->g_iflags = DEFAULT.g_iflags;
	else
		if (check_flags(ttydef->g_iflags) != 0 )
			errflg++;
	if (!fflg)
		ttydef->g_fflags = DEFAULT.g_fflags;
	else
		if (check_flags(ttydef->g_fflags) != 0 )
			errflg++;
	if (errflg)
		exit(1);

	if (!nflg)
		ttydef->g_nextid = ttydef->g_id;

	if (ttydef->g_autobaud & A_FLAG)  {
	   (void)fprintf(fp,"%s:%s:%s:A:%s\n", ttydef->g_id, ttydef->g_iflags,
		ttydef->g_fflags, ttydef->g_nextid);
	}
	else {
	   (void)fprintf(fp,"%s:%s:%s::%s\n", ttydef->g_id, ttydef->g_iflags,
		ttydef->g_fflags, ttydef->g_nextid);
	}
	(void)fclose(fp);
	exit(0);
}

static	void
remove_entry(ttylabel)
char	*ttylabel;
{
	FILE *tfp;		/* file pointer for temp file */
	int line;		/* line number entry is on */
	FILE *fp;		/* scratch file pointer */
	char *tname = "/etc/.ttydefs";

	if (getuid()) {
		(void)fprintf(stderr, "User not privileged for operation.\n");
		exit(1);
	}
	fp = fopen(TTYDEFS, "r");
	if (fp == NULL) {
		(void)sprintf(Scratch, "Could not open \"%s\"", TTYDEFS);
		perror(Scratch);
		exit(1);
	}
	if (check_version(TTYDEFS_VERS, TTYDEFS) != 0) {
		(void)fprintf(stderr, 
			"%s version number is incorrect or missing.\n",TTYDEFS);
		exit(1);
	}
	if ((line = find_label(fp, ttylabel)) == 0) {
		(void)fprintf(stderr, 
		"Invalid request, ttylabel <%s> does not exist.\n", ttylabel);
		exit(1);
	}
	tfp = open_temp(tname);
	if (line != 1) 
		if (copy_file(fp, tfp, 1, line - 1)) {
			(void)fprintf(stderr,"Error accessing temp file.\n");
			exit(1);
		}
	if (copy_file(fp, tfp, line + 1, -1)) {
		(void)fprintf(stderr,"Error accessing temp file.\n");
		exit(1);
	}
	(void)fclose(fp);
	if (fclose(tfp) == EOF) {
		(void)unlink(tname);
		(void)fprintf(stderr,"Error closing temp file.\n");
		exit(1);
	}
	(void)unlink(TTYDEFS);
	if (rename(tname, TTYDEFS) != 0 ) {
		perror("Rename failed");
		(void)unlink(tname);
		exit(1);
	}
	exit(0);
}

/*
 * open_temp - open up a temp file
 *
 *	args:	tname - temp file name
 */

static	FILE *
open_temp(tname)
char *tname;
{
	FILE *fp;			/* fp associated with tname */
	struct sigaction sigact;	/* for signal handling */

	sigact.sa_flags = 0;
	sigact.sa_handler = SIG_IGN;
	(void) sigemptyset(&sigact.sa_mask);
	(void) sigaddset(&sigact.sa_mask, SIGHUP);
	(void) sigaddset(&sigact.sa_mask, SIGINT);
	(void) sigaddset(&sigact.sa_mask, SIGQUIT);
	(void) sigaction(SIGHUP, &sigact, NULL);
	(void) sigaction(SIGINT, &sigact, NULL);
	(void) sigaction(SIGQUIT, &sigact, NULL);
	(void)umask(0333);
	if (access(tname, 0) != -1) {
		(void)fprintf(stderr,"tempfile busy; try again later.\n");
		exit(1);
	}
	fp = fopen(tname, "w");
	if (fp == NULL) {
		perror("Cannot create tempfile");
		exit(1);
	}
	return(fp);
}

/*
 * copy_file - copy information from one file to another, return 0 on
 *	success, -1 on failure
 *
 *	args:	fp - source file's file pointer
 *		tfp - destination file's file pointer
 *		start - starting line number
 *		finish - ending line number (-1 indicates entire file)
 */


static	int
copy_file(fp, tfp, start, finish)
FILE *fp;
FILE *tfp;
register int start;
register int finish;
{
	register int i;		/* loop variable */
	char dummy[BUFSIZ];	/* scratch buffer */

/*
 * always start from the beginning because line numbers are absolute
 */

	rewind(fp);

/*
 * get to the starting point of interest
 */

	if (start != 1) {
		for (i = 1; i < start; i++)
			if (!fgets(dummy, BUFSIZ, fp))
				return(-1);
	}

/*
 * copy as much as was requested
 */

	if (finish != -1) {
		for (i = start; i <= finish; i++) {
			if (!fgets(dummy, BUFSIZ, fp))
				return(-1);
			if (fputs(dummy, tfp) == EOF)
				return(-1);
		}
	}
	else {
		for (;;) {
			if (fgets(dummy, BUFSIZ, fp) == NULL) {
				if (feof(fp))
					break;
				else
					return(-1);
			}
			if (fputs(dummy, tfp) == EOF)
				return(-1);
		}
	}
	return(0);
}

/*
 *	check_ref	- to check if nextlabel are referencing
 *			  existing ttylabel
 */
static	void
check_ref()
{
	int	i;
	struct	Gdef	*np;
	extern	struct	Gdef	*find_def();
	np = &Gdef[0];
	for (i = 0; i < Ndefs; i++, np++) {
		if (find_def(np->g_nextid) == NULL) {
			(void)printf("Warning -- nextlabel <%s> of <%s> does not reference any existing ttylabel.\n", 
			np->g_nextid, np->g_id);
		}
	}
	return;
}

/*
 *	log	- print a message to stdout
 */

void
log(msg)
char	*msg;
{
	if (lflg)
		(void)printf("%s\n", msg);
	else
		(void)fprintf(stderr,"%s\n", msg);
}
