/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)autopush:autopush.c	1.8.1.1"

/*
 * autopush(1) is the command interface to the STREAMS
 * autopush mechanism.  The autopush command can be used
 * to configure autopush information about a STREAMS driver,
 * remove autopush information, and report on current configuration
 * information.  Its use is as follows:
 *
 *	autopush -f file
 *	autopush -r -M major -m minor
 *	autopush -g -M major -m minor
 *
 * The -f option allows autopush information to be set from a file.  The
 * format of the file is as follows:
 *
 * # Comment lines begin with a # in column one.
 * # The fields are separated by white space and are:
 * # major	minor	lastminor	module1 module2 ... module8
 *
 * "lastminor" is used to configure ranges of minor devices, from "minor"
 * to "lastminor" inclusive.  It should be set to zero when not in use.
 * The -r option allows autopush information to be removed for the given
 * major/minor pair.  The -g option allows the configuration information
 * to be printed.  The format of printing is the same as for the file.
 *
 */

#include "sys/types.h"
#include "sys/conf.h"
#include "sys/sad.h"
#include "stdio.h"
#include "fcntl.h"
#include "errno.h"
#include "ctype.h"
#include "memory.h"

#define OPTIONS	"M:f:gm:r"	/* command line options for getopt(3C) */
#define COMMENT	'#'
#define MINUS	'-'
#define SLASH	'/'

/*
 * Output format.
 */
#define OHEADER		"     Major      Minor  Lastminor\tModules\n"
#define OFORMAT1_ONE	"%10ld %10ld      -    \t"
#define OFORMAT1_RANGE	"%10ld %10ld %10ld\t"
#define OFORMAT1_ALL	"%10ld       ALL       -    \t"
#define OFORMAT2	"%s "
#define OFORMAT3	"%s\n"

static char *Openerr = "%s: ERROR: Could not open %s: ";
static char *Digiterr = "%s: ERROR: argument to %s option must be numeric\n";
static char *Badline = "%s: WARNING: File %s: bad input line %d ignored\n";
static char *Cmdp;			/* command name */

static void usage();
static int rem_info(), get_info(), set_info();
static int is_white_space(), parse_line();

extern int errno;
extern long atol();
extern void exit();
extern int getopt();
extern int ioctl();

/*
 * main():
 *	process command line arguments.
 */
int
main(argc, argv)
	int argc;
	char *argv[];
{
	int c;			/* character read by getopt(3C) */
	char *filenamep;	/* name of configuration file */
	major_t major;		/* major device number */
	minor_t minor;		/* minor device number */
	char *cp;
	int exitcode;
	ushort minflag = 0;	/* -m option used */
	ushort majflag = 0;	/* -M option used */
	ushort fflag = 0;	/* -f option used */
	ushort rflag = 0;	/* -r option used */
	ushort gflag = 0;	/* -g option used */
	ushort errflag = 0;	/* options usage error */
	extern char *optarg;	/* for getopts - points to argument of option */
	extern int optind;	/* for getopts - index into argument list */

	/*
	 * Get command name.
	 */
	Cmdp = argv[0];
	for (filenamep = argv[0]; *filenamep; filenamep++)
		if (*filenamep == SLASH)
			Cmdp = filenamep + 1;

	/*
	 * Get options.
	 */
	while (!errflag && ((c = getopt(argc, argv, OPTIONS)) != -1)) {
		switch (c) {
		case 'M':
			if (fflag|majflag)
				errflag++;
			else {
				majflag++;
				for (cp = optarg; *cp; cp++)
					if (!isdigit((int)*cp)) {
						(void) fprintf(stderr, Digiterr, Cmdp, "-M");
						exit(1);
					}
				major = (major_t)atol(optarg);
			}
			break;

		case 'm':
			if (fflag|minflag)
				errflag++;
			else {
				minflag++;
				for (cp = optarg; *cp; cp++)
					if (!isdigit((int)*cp)) {
						(void) fprintf(stderr, Digiterr, Cmdp, "-m");
						exit(1);
					}
				minor = (minor_t)atol(optarg);
			}
			break;

		case 'f':
			if (fflag|gflag|rflag|majflag|minflag)
				errflag++;
			else {
				fflag++;
				filenamep = optarg;
			}
			break;

		case 'r':
			if (fflag|gflag|rflag)
				errflag++;
			else
				rflag++;
			break;

		case 'g':
			if (fflag|gflag|rflag)
				errflag++;
			else
				gflag++;
			break;

		default:
			errflag++;
			break;
		} /* switch */
		if (errflag) {
			usage();
			exit(1);
		}
	} /* while */
	if (((gflag || rflag) && (!majflag || !minflag)) || (optind != argc)) {
		usage();
		exit(1);
	}
	if (fflag)
		exitcode = set_info(filenamep);
	else if (rflag)
		exitcode = rem_info(major, minor);
	else if (gflag)
		exitcode = get_info(major, minor);
	else {
		usage();
		exit(1);
	}
	exit(exitcode);
	/* NOTREACHED */
}

/*
 * usage():
 *	print out usage statement.
 */
static void
usage()
{
	(void) fprintf(stderr, "%s: USAGE:\n\t%s -f filename\n", Cmdp, Cmdp);
	(void) fprintf(stderr, "\t%s -r -M major -m minor\n", Cmdp);
	(void) fprintf(stderr, "\t%s -g -M major -m minor\n", Cmdp);
}

/*
 * set_info():
 *	set autopush configuration information.
 */
static int
set_info(namep)
	char *namep;		/* autopush configuration filename */
{
	int line;		/* line number of file */
	FILE *fp;		/* file pointer of config file */
	char buf[256];		/* input buffer */
	struct strapush push;	/* configuration information */
	int sadfd;		/* file descriptor to SAD driver */
	int retcode = 0;	/* return code */

	if ((sadfd = open(ADMINDEV, O_RDWR)) < 0) {
		(void) fprintf(stderr, Openerr, Cmdp, ADMINDEV);
		perror("");
		return (1);
	}
	if ((fp = fopen(namep, "r")) == NULL) {
		(void) fprintf(stderr, Openerr, Cmdp, namep);
		perror("");
		return (1);
	}
	line = 0;
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		line++;
		if ((buf[0] == COMMENT) || is_white_space(buf))
			continue;
		(void) memset((char *)&push, 0, sizeof(struct strapush));
		if (parse_line(buf, line, namep, &push))
			continue;
		if (push.sap_minor == (minor_t)-1)
			push.sap_cmd = SAP_ALL;
		else if (push.sap_lastminor == 0)
			push.sap_cmd = SAP_ONE;
		else
			push.sap_cmd = SAP_RANGE;
		errno = 0;
		if (ioctl(sadfd, SAD_SAP, &push) < 0) {
			retcode = 1;
			(void) fprintf(stderr, "%s: ERROR: File %s: could not configure autopush for line %d\n", Cmdp, namep, line);
			switch (errno) {
			case EPERM:
				(void) fprintf(stderr, "%s: ERROR: You don't have permission to set autopush information\n", Cmdp);
				break;

			case EINVAL:
				(void) fprintf(stderr, "%s: ERROR: Invalid major device number or\n\t\tinvalid module name or too many modules\n", Cmdp);
				break;

			case ENOSTR:
				(void) fprintf(stderr, "%s: ERROR: Major device is not a STREAMS driver\n", Cmdp);
				break;

			case EEXIST:
				(void) fprintf(stderr, "%s: ERROR: Major/minor already configured\n", Cmdp);
				break;

			case ENOSR:
				(void) fprintf(stderr, "%s: ERROR: Ran out of autopush structures\n", Cmdp);
				break;

			case ERANGE:
				(void) fprintf(stderr, "%s: ERROR: lastminor must be greater than minor\n", Cmdp);
				break;

			default:
				(void) fprintf(stderr, "%s: ERROR: ", Cmdp);
				perror("");
				break;
			} /* switch */
		} /* if */
	} /* while */
	return (retcode);
}

/*
 * rem_info():
 *	remove autopush configuration information.
 */
static int
rem_info(maj, min)
	major_t maj;
	minor_t min;
{
	struct strapush push;	/* configuration information */
	int sadfd;		/* file descriptor to SAD driver */
	int retcode = 0;	/* return code */

	if ((sadfd = open(ADMINDEV, O_RDWR)) < 0) {
		(void) fprintf(stderr, Openerr, Cmdp, ADMINDEV);
		perror("");
		return (1);
	}
	push.sap_cmd = SAP_CLEAR;
	push.sap_minor = min;
	push.sap_major = maj;
	errno = 0;
	if (ioctl(sadfd, SAD_SAP, &push) < 0) {
		retcode = 1;
		(void) fprintf(stderr, "%s: ERROR: Could not remove autopush information\n", Cmdp);
		switch (errno) {
		case EPERM:
			(void) fprintf(stderr, "%s: ERROR: You don't have permission to remove autopush information\n", Cmdp);
			break;

		case EINVAL:
			if ((min != 0) && (ioctl(sadfd, SAD_GAP, &push) == 0) &&
			    (push.sap_cmd == SAP_ALL))
				(void) fprintf(stderr, "%s: ERROR: When removing an entry for ALL minors, minor must be set to 0\n", Cmdp);
			else
				(void) fprintf(stderr, "%s: ERROR: Invalid major device number\n", Cmdp);
			break;

		case ENODEV:
			(void) fprintf(stderr, "%s: ERROR: Major/minor not configured for autopush\n", Cmdp);
			break;

		case ERANGE:
			(void) fprintf(stderr, "%s: ERROR: minor must be set to begining of range when clearing\n", Cmdp);
			break;

		default:
			(void) fprintf(stderr, "%s: ERROR: ", Cmdp);
			perror("");
			break;
		} /* switch */
	}
	return (retcode);
}

/*
 * get_info():
 *	get autopush configuration information.
 */
static int
get_info(maj, min)
	major_t maj;
	minor_t min;
{
	struct strapush push;	/* configuration information */
	int i;			/* counter */
	int sadfd;		/* file descriptor to SAD driver */

	if ((sadfd = open(USERDEV, O_RDWR)) < 0) {
		(void) fprintf(stderr, Openerr, Cmdp, USERDEV);
		perror("");
		return (1);
	}
	push.sap_major = maj;
	push.sap_minor = min;
	errno = 0;
	if (ioctl(sadfd, SAD_GAP, &push) < 0) {
		(void) fprintf(stderr, "%s: ERROR: Could not get autopush information\n", Cmdp);
		switch (errno) {
		case EINVAL:
			(void) fprintf(stderr, "%s: ERROR: Invalid major device number\n", Cmdp);
			break;

		case ENOSTR:
			(void) fprintf(stderr, "%s: ERROR: Major device is not a STREAMS driver\n", Cmdp);
			break;

		case ENODEV:
			(void) fprintf(stderr, "%s: ERROR: Major/minor not configured for autopush\n", Cmdp);
			break;

		default:
			(void) fprintf(stderr, "%s: ERROR: ", Cmdp);
			perror("");
			break;
		} /* switch */
		return (1);
	}
	(void) printf(OHEADER);
	switch (push.sap_cmd) {
	case SAP_ONE:
		(void) printf(OFORMAT1_ONE, push.sap_major, push.sap_minor);
		break;

	case SAP_RANGE:
		(void) printf(OFORMAT1_RANGE, push.sap_major, push.sap_minor, push.sap_lastminor);
		break;

	case SAP_ALL:
		(void) printf(OFORMAT1_ALL, push.sap_major);
		break;

	default:
		(void) fprintf(stderr, "%s: ERROR: Unknown configuration type\n", Cmdp);
		return (1);
	}
	if (push.sap_npush > 1)
		for (i = 0; i < (push.sap_npush - 1); i++)
			(void) printf(OFORMAT2, push.sap_list[i]);
	(void) printf(OFORMAT3, push.sap_list[(push.sap_npush - 1)]);
	return (0);
}

/*
 * is_white_space():
 *	Return 1 if buffer is all white space.
 *	Return 0 otherwise.
 */
static int
is_white_space(bufp)
	register char *bufp;	/* pointer to the buffer */
{

	while (*bufp) {
		if (!isspace((int)*bufp))
			return (0);
		bufp++;
	}
	return (1);
}

/*
 * parse_line():
 *	Parse input line from file and report any
 *	errors found.  Fill strapush structure along
 *	the way.  Returns 1 if the line has errors
 *	and 0 if the line is well-formed.
 *	Another hidden dependency on MAXAPUSH.
 */
static int
parse_line(linep, lineno, namep, pushp)
	char *linep;				/* the input buffer */
	int lineno;				/* the line number */
	char *namep;				/* the file name */
	register struct strapush *pushp;	/* for ioctl */
{
	register char *wp;			/* word pointer */
	register char *cp;			/* character pointer */
	register int midx;			/* module index */
	register int npush;			/* number of modules to push */

	/*
	 * Find the major device number.
	 */
	for (wp = linep; isspace((int)*wp); wp++)
		;
	for (cp = wp; isdigit((int)*cp); cp++)
		;
	if (!isspace((int)*cp)) {
		(void) fprintf(stderr, Badline, Cmdp, namep, lineno);
		return (1);
	}
	pushp->sap_major = (major_t)atol(wp);

	/*
	 * Find the minor device number.  Must handle negative values here.
	 */
	for (wp = cp; isspace((int)*wp); wp++)
		;
	for (cp = wp; (isdigit((int)*cp) || (*cp == MINUS)); cp++)
		;
	if (!isspace((int)*cp)) {
		(void) fprintf(stderr, Badline, Cmdp, namep, lineno);
		return (1);
	}
	pushp->sap_minor = (minor_t)atol(wp);

	/*
	 * Find the lastminor.
	 */
	for (wp = cp; isspace((int)*wp); wp++)
		;
	for (cp = wp; isdigit((int)*cp); cp++)
		;
	if (!isspace((int)*cp)) {
		(void) fprintf(stderr, Badline, Cmdp, namep, lineno);
		return (1);
	}
	pushp->sap_lastminor = (minor_t)atol(wp);

	/*
	 * Read the list of module names.
	 */
	npush = 0;
	while ((npush < MAXAPUSH) && (*cp)) {
		while (isspace((int)*cp))
			cp++;
		for (midx = 0; ((midx < FMNAMESZ) && (*cp)); midx++)
			if (!isspace((int)*cp))
				pushp->sap_list[npush][midx] = *cp++;
		if ((midx >= FMNAMESZ) && (*cp) && !isspace((int)*cp)) {
			(void) fprintf(stderr, "%s: ERROR: File %s: module name too long, line %d ignored\n", Cmdp, namep, lineno);
			return (1);
		}
		if (midx > 0) {
			pushp->sap_list[npush][midx] = (char)0;
			npush++;
		}
	}
	pushp->sap_npush = npush;

	/*
	 * We have everything we want from the line.
	 * Now make sure there is no extra garbage on the line.
	 */
	while ((*cp) && isspace((int)*cp))
		cp++;
	if (*cp) {
		(void) fprintf(stderr, "%s: ERROR: File %s: too many modules, line %d ignored\n", Cmdp, namep, lineno);
		return (1);
	}
	return (0);
}

