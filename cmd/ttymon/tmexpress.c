/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ttymon:tmexpress.c	1.19.5.1"

#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<errno.h>
#include	<ctype.h>
#include	<string.h>
#include	<signal.h>
#include	<sys/stat.h>
#include	<utmp.h>
#include	"ttymon.h"
#include	"tmextern.h"
#include	"tmstruct.h"

static	char	devbuf[BUFSIZ];
static	char	*devname;

static	int	parse_args();
static	void	ttymon_options();
static	void	getty_options();
static	void	usage();
static	char	*find_ttyname();

extern	char	Scratch[];
extern	void	log();
extern	void	tmchild();
extern	int	check_identity();
extern	int	turnon_canon();
extern	int	vml();
extern	pid_t	getsid(), tcgetsid();

/*
 * ttymon_express - This is call when ttymon is invoked with args
 *		    or invoked as getty
 *		  - This special version of ttymon will monitor
 *		    one port only
 *		  - It is intended to be used when some process
 *		    wants to have a login session on the fly
 */
void
ttymon_express(argc,argv)
int	argc;
const	char	*argv[];
{
	struct	pmtab	*pmtab;
	struct	sigaction	sigact;
	extern	int	Retry;
	extern	void	open_device();
	extern	void	read_ttydefs();
#ifdef	DEBUG
	extern	FILE	*Debugfp;
	extern	void	opendebug();
#endif

#ifdef	DEBUG
	opendebug(TRUE);
#endif

	sigact.sa_flags = 0;
	sigact.sa_handler = SIG_IGN;
	(void)sigemptyset(&sigact.sa_mask);
	(void)sigaction(SIGINT, &sigact, NULL);

	if ((pmtab = ALLOC_PMTAB) == PNULL) {
		log("ttymon_express: ALLOC_PMTAB failed");
		exit(1);
	}

	if (parse_args(argc, argv, pmtab) != 0) {
		log("ttymon_express: parse_args failed");
		exit(1);
	}

	read_ttydefs(NULL,FALSE);

	if ((pmtab->p_device == NULL) || (*(pmtab->p_device) == '\0')) {
/*
		if (tcgetsid(0) != getsid(getpid()) ) {
*/
			devname = find_ttyname(0);
			if ((devname == NULL) || (*devname == '\0')) {
				log("ttyname cannot find the device on fd 0");
				exit(1);
			}
			pmtab->p_device = devname;
#ifdef	DEBUG
			(void)sprintf(Scratch,"ttymon_express: devname = %s",
					 devname);
			debug(Scratch);
#endif
			/*
			 * become session leader 
		 	 * fd 0 is closed and reopened just to make sure
		 	 * controlling tty is set up right
		 	 */
			(void)setsid();
			(void)close(0);
			if (open(pmtab->p_device, O_RDWR) < 0) {
				(void)sprintf(Scratch,
				"open %s failed, errno = %d",
				pmtab->p_device,errno);
				log(Scratch);
				exit(1);
			}
/*
		}
#ifdef	DEBUG
		else {
			(void)sprintf(Scratch,
			"ttymon_express: controlling tty already setup.");
			debug(Scratch);
			(void)sprintf(Scratch,"tcgetsid = %d, getsid =%d",
					 tcgetsid(0),getsid(getpid()));
			debug(Scratch);
		}
#endif
*/
		if ((pmtab->p_modules != NULL) &&
		    (*(pmtab->p_modules) != '\0')) {
		   if (push_linedisc(0,pmtab->p_modules,pmtab->p_device) == -1)
			exit(1);
		}
		if (initial_termio(0, pmtab) == -1) 
			exit(1);
	}
	else {
		(void)setsid();
		(void)close(0);
		Retry = FALSE;
		open_device(pmtab);
		if (Retry)		/* open failed */
			exit(1);
	}
	tmchild(pmtab);
	exit(1);	/*NOTREACHED*/
}

/*
 * parse_arg	- parse cmd line arguments
 */
static	int
parse_args(argc, argv, pmtab)
int	argc;
const	char	*argv[];
struct	pmtab	*pmtab;
{
	extern	char	*lastname();
	extern	void	getty_account();

	/* initialize fields to some default first */
	pmtab->p_tag = "";
	pmtab->p_flags = 0;
	pmtab->p_identity = "root";
	pmtab->p_res1 = "reserved";
	pmtab->p_res2 = "reserved";
	pmtab->p_res3 = "reserved";
	if (check_identity(pmtab) != 0) {
		exit(1);
	}
	pmtab->p_ttyflags = 0;
	pmtab->p_count = 0;
	pmtab->p_server = "/usr/bin/login";
	pmtab->p_timeout = 0;
	pmtab->p_modules = "";
	pmtab->p_prompt = "login: ";
	pmtab->p_dmsg = "";
	pmtab->p_device = "";
	pmtab->p_status = GETTY;
	if (strcmp(lastname(argv[0]), "getty") == 0) {
		pmtab->p_ttylabel = "300";
		getty_options(argc,argv,pmtab);
	}
	else {
		pmtab->p_ttylabel = "9600";
		ttymon_options(argc,argv,pmtab);
	}
	if ((pmtab->p_device != NULL) && (*(pmtab->p_device) != '\0'))
		getty_account(pmtab->p_device); /* utmp accounting */
	return(0);
}


/*
 * 	ttymon_options - scan and check args for ttymon express 
 */

static	void
ttymon_options(argc, argv,pmtab)
int argc;
const	char *argv[];
struct pmtab	*pmtab;
{
	int 	c;			/* option letter */
	char 	*timeout;
	int  	gflag = 0;		/* -g seen */
	int	size = 0;
	char	tbuf[BUFSIZ];

	extern	char	*optarg;
	extern	int	optind;
	extern	void	copystr();
	extern	char	*strsave();
	extern	char	*getword();

	while ((c = getopt(argc, argv, "gd:ht:p:m:l:")) != -1) {
		switch (c) {
		case 'g':
			gflag = 1;
			break;
		case 'd':
			pmtab->p_device = optarg;
			break;
		case 'h':
			pmtab->p_ttyflags &= ~H_FLAG;
			break;
/*
		case 'b':
			pmtab->p_ttyflags |= B_FLAG;
			pmtab->p_ttyflags |= R_FLAG;
			break;
*/
		case 't':
			timeout = optarg;
			while (*optarg) {
				if (!isdigit(*optarg++)) {
					log("Invalid argument for \"-t\" -- number expected.");
					usage();
				}
			}
			pmtab->p_timeout = atoi(timeout);
			break;
		case 'p':
			copystr(tbuf, optarg);
			pmtab->p_prompt = strsave(getword(tbuf,&size,TRUE));
			break;
		case 'm':
			pmtab->p_modules = optarg;
			if (vml(pmtab->p_modules) != 0) 
				usage();
			break;
		case 'l':
			pmtab->p_ttylabel = optarg;
			break;
		case '?':
			usage();
			break;	/*NOTREACHED*/
		}
	}
	if (optind < argc)
		usage();

	if (!gflag) 
		usage();
}

/*
 * usage - print out a usage message
 */

static 	void
usage()
{
	int	fd;
	char	*umsg = "Usage: ttymon\n  ttymon -g [-h] [-d device] [-l ttylabel] [-t timeout] [-p prompt] [-m modules]\n";

	if (isatty(2))
		(void)fprintf(stderr,"%s", umsg);
	else
		if ((fd = open(CONSOLE, O_WRONLY|O_NOCTTY)) != -1)
			(void)write(fd,umsg,strlen(umsg));
	exit(1);
}

/*
 *	getty_options	- this is cut from getty.c
 *			- it scan getty cmd args 
 *			- modification is made to stuff args in pmtab
 */
static	void
getty_options(argc,argv,pmtab)
int argc;
char **argv;
struct	pmtab	*pmtab;
{
	char	*ptr;

	/* 
	 * the pre-4.0 getty's hang_up_line() is a no-op.
	 * For compatibility, H_FLAG cannot be set for this "getty".
	 */
	pmtab->p_ttyflags &= ~(H_FLAG);

	while(--argc && **++argv == '-') {
		for(ptr = *argv + 1; *ptr;ptr++) 
		switch(*ptr) {
		case 'h':
			break;
		case 't':
			if(isdigit(*++ptr)) {
				(void)sscanf(ptr,"%d",&(pmtab->p_timeout));
				while(isdigit(*++ptr));
				ptr--;
			} else if(--argc) {
				if(isdigit(*(ptr = *++argv)))
					(void)sscanf(ptr,"%d",&(pmtab->p_timeout));
				else {
					(void) sprintf(Scratch,
					"getty: timeout argument <%s> invalid",
						 *argv);
					log(Scratch);
					exit(1);
				}
			}
			break;

		case 'c':
			log("Use \"sttydefs -l\" to check /etc/ttydefs.");
			exit(0);
		default:
			break;
		}
	}

	if(argc < 1) {
		log("getty: no terminal line specified.");
		exit(1);
	} 
	else {
		(void)strcat(devbuf,"/dev/");
		(void)strcat(devbuf,*argv);
		pmtab->p_device = devbuf;
	}

	if(--argc > 0 ) {
		pmtab->p_ttylabel = *++argv;
	} 

	/*
	 * every thing after this will be ignored
	 * i.e. termtype and linedisc are ignored
	 */
}

/*
 * find_ttyname(fd) 	- find the name of device associated with fd.
 *			- it first tries utmp to see if an entry exists
 *			- with my pid and ut_line is defined. If ut_line
 *			- is defined, it will see if the major and minor
 *			- number of fd and devname from utmp match.
 *			- If utmp search fails, ttyname(fd) will be called.
 */
static	char	*
find_ttyname(fd)
int	fd;
{
	register o_pid_t ownpid;
	register struct utmp *u;
	static	struct	stat	statf, statu;
	static	char	buf[BUFSIZ];

	ownpid = (o_pid_t)getpid();
	setutent();
	while ((u = getutent()) != NULL) {
		if (u->ut_pid == ownpid) {
			if (strlen(u->ut_line) != 0) {
				if (*(u->ut_line) != '/') {
					(void)strcpy(buf, "/dev/");
					(void)strncat(buf, u->ut_line, 
						sizeof(u->ut_line));
				}
				else {
					(void)strncat(buf, u->ut_line, 
						sizeof(u->ut_line));
				}
			}
			else
				u = NULL;
			break;
		}
	}
	endutent();
	if (	(u != NULL) &&
		(fstat(fd, &statf) == 0) && 
		(stat(buf, &statu) == 0) &&
		(statf.st_dev == statu.st_dev) &&
		(statf.st_rdev == statu.st_rdev)    ) {
#ifdef	DEBUG
			(void)sprintf(Scratch,
			"ttymon_express: find device name from utmp.");
			debug(Scratch);
#endif
			return(buf);
	}
	else {
#ifdef	DEBUG
		(void)sprintf(Scratch,
		"ttymon_express: calling ttyname to find device name.");
		debug(Scratch);
#endif
		return(ttyname(fd));
	}
}
