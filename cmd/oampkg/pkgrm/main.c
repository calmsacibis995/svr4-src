/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:pkgrm/main.c	1.11.7.1"

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <pkgstrct.h>
#include <pkgdev.h>
#include <pkginfo.h>
#include <pkglocs.h>
#include "install.h"
#include "msgdefs.h"

extern int	optind, errno;
extern char	*optarg, *pkgdir;

extern void	exit(),
		progerr(),
		ptext(),
		echo(),
		trap(),
		setadmin(),
		quit();
extern char	*pkgparam(), 
		*getenv(),
		**gpkglist();
extern int	getopt(),
		chdir(),
		ckyorn(),
		devtype(),
		pkgmount(),
		pkgumount(),
		pkgexecv(),
		rrmdir(),
		presvr4();

#define	ASK_CONFIRM \
"Do you want to remove this package"

struct admin
	adm;		/* holds info about installation admin */
struct pkgdev
	pkgdev;		/* holds info about the installation device */
int	reboot;		/* non-zero if reboot required after installation */
int	ireboot;	/* non-zero if immediate reboot required */
int	failflag;	/* non-zero if fatal error has occurred */
int	warnflag;	/* non-zero if non-fatal error has occurred */
int	intrflag;	/* non-zero if any pkg installation was interrupted */
int	admnflag;	/* non-zero if any pkg installation was interrupted */
int	nullflag;	/* non-zero if any pkg installation was interrupted */
int	nointeract;	/* non-zero if no interaction with user should occur */
int	npkgs;		/* the number of packages yet to be installed */
int	started;
char	*pkginst;	/* current package (source) instance to process */
char	*prog;		/* the basename of argv[0] */
char	*tmpdir;	/* location to place temporary files */

void	(*func)(), ckreturn();

static int	interrupted;
static char	*admnfile;	/* file to use for installation admin */

static int	
	doremove(), pkgremove();

static void
usage()
{
	(void) fprintf(stderr, "usage:\n");
	(void) fprintf(stderr, "\t%s [-a admin] [pkg [pkg ...]]\n", prog);
	(void) fprintf(stderr, "\t%s -n [-a admin] pkg [pkg ...]\n", prog);
	(void) fprintf(stderr, "\t%s -s spool [pkg [pkg ...]]\n", prog);
	exit(1);
}

main(argc,argv)
int	argc;
char	**argv;
{
	int	i, c, n;
	int	repeat;
	char	ans, 
		**pkglist,	/* points to array of packages */
		*device = 0;

	prog = strrchr(argv[0], '/');
	if(!prog++)
		prog = argv[0];

	while((c = getopt(argc, argv, "s:a:n?")) != EOF) {
		switch(c) {
		  case 's':
			device = optarg;
			break;

		  case 'a':
			admnfile = optarg;
			break;

		  case 'n':
			nointeract++;
			break;

		  default:
			usage();
		}
	}
	if(admnfile && device)
		usage();
	if(nointeract && (optind == argc))
		usage();

	func = signal(SIGINT, trap);
	if(func != SIG_DFL)
		(void) signal(SIGINT, func);
	(void) signal(SIGHUP, trap);

	tmpdir = getenv("TMPDIR");
	if(tmpdir == NULL)
		tmpdir = P_tmpdir;

	/*
	 * initialize installation admin parameters 
	 */
	if(device == NULL)
		setadmin(admnfile);

	if(devtype((device ? device : PKGLOC), &pkgdev)) {
		progerr("bad device <%s> specified", device);
		quit(1);
	}
	if(pkgdev.norewind) {
		progerr("datastream devices not supported");
		quit(1);
	}

	pkgdir = pkgdev.dirname;
	repeat = ((optind >= argc) && pkgdev.mount);

again:

	if(pkgdev.mount) {
		if(n = pkgmount(&pkgdev, NULL, 0, 0, 1))
			quit(n);
	}

	if(chdir(pkgdev.dirname)) {
		progerr(ERR_CHDIR, pkgdev.dirname);
		quit(99);
	}

	pkglist = gpkglist(pkgdev.dirname, &argv[optind]);
	if(pkglist == NULL) {
		if(errno == ENOPKG) {
			/* check for existence of pre-SVR4 package */
			progerr(ERR_NOPKGS, pkgdev.dirname);
			quit(1);
		} else {
			switch(errno) {
			  case ESRCH:
				quit(1);
				break;

			  case EINTR:
				quit(3);
				break;

			  default:
				progerr("internal error in gpkglist()");
				quit(99);
			}
		}
	}

	for(npkgs=0; pkglist[npkgs]; )
		npkgs++;

	interrupted = 0;
	for(i=0; pkginst = pkglist[i]; i++) {
		started = 0;
		if(ireboot) {
			ptext(stderr, MSG_SUSPEND, pkginst);
			continue;
		}
		if(interrupted) {
			if(npkgs == 1)
				echo(MSG_1MORETODO);
			else
				echo(MSG_MORETODO, npkgs-i);
			if(nointeract)
				quit(0);
			if(n = ckyorn(&ans, NULL, NULL, NULL, ASK_CONTINUE))
				quit(n);
			if(ans != 'y')
				quit(0);
		}
		interrupted = 0;
		ckreturn(doremove());
		if(pkgdev.mount) {
			(void) chdir("/");
			if(pkgumount(&pkgdev)) {
				progerr("unable to unmount <%s>", 
					pkgdev.bdevice);
				quit(99);
			}
		}
		npkgs--;
	}
	if(!ireboot && repeat)
		goto again;
	quit(0);
	/*NOTREACHED*/
}

static int
doremove()
{
	struct pkginfo info;
	char	ans;
	int	n;
	char inbu[80];

	info.pkginst = NULL;
	if(pkginfo(&info, pkginst, NULL, NULL)) {
		progerr("instance <%s> does not exist", pkginst);
		return(2);
	}

	if(!nointeract) {
		if(info.status == PI_SPOOLED)
			echo("\nThe following package is currently spooled:");
		else if(info.status != PI_PRESVR4)
			echo("\nThe following package is currently installed:");
		else
		{
			echo("\nThe following package is a pre-SVR4 package:");
			echo("   %-14.14s  %s", info.pkginst, info.name);
			echo("\nPlease use the \"removepkg\" command to remove this package.\n");
			echo("Press RETURN to continue:");
			gets(inbu);
			return(0);
		}
		echo("   %-14.14s  %s", info.pkginst, info.name);
		if(info.arch || info.version) 
			(void) fprintf(stderr, "   %14.14s  ", "");
		if(info.arch)
			(void) fprintf(stderr, "(%s) ", info.arch);
		if(info.version)
			(void) fprintf(stderr, "%s", info.version);
		if(info.arch || info.version)
			(void) fprintf(stderr, "\n");

		if(n = ckyorn(&ans, NULL, NULL, NULL, ASK_CONFIRM))
			quit(n);
		if(ans == 'n')
			return(0);
	}


	if(info.status == PI_SPOOLED) {
		/* removal from a directory */
		echo("\nRemoving spooled package instance <%s>", pkginst);
		return(rrmdir(pkginst));
	}
	return(pkgremove());
}

/*
 *  function which checks the indicated return value
 *  and indicates disposition of installation
 */
void
ckreturn(retcode) 
int	retcode;
{

	switch(retcode) {
	  case  0:
	  case 10:
	  case 20:
		break; /* empty case */

	  case  1:
	  case 11:
	  case 21:
		failflag++;
		interrupted++;
		break;

	  case  2:
	  case 12:
	  case 22:
		warnflag++;
		interrupted++;
		break;

	  case  3:
	  case 13:
	  case 23:
		intrflag++;
		interrupted++;
		break;

	  case  4:
	  case 14:
	  case 24:
		admnflag++;
		interrupted++;
		break;

	  case  5:
	  case 15:
	  case 25:
		nullflag++;
		interrupted++;
		break;

	  default:
		failflag++;
		interrupted++;
		return;
	}
	if(retcode >= 20)
		ireboot++;
	else if(retcode >= 10)
		reboot++;
}

#define MAXARGS 15

static int
pkgremove()
{
	void	(*tmpfunc)();
	char	*arg[MAXARGS], path[PATH_MAX];
	int	n, nargs;

	(void) sprintf(path, "%s/pkgremove", PKGBIN);

	nargs = 0;
	arg[nargs++] = path;
	if(nointeract)
		arg[nargs++] = "-n";
	if(admnfile) {
		arg[nargs++] = "-a";
		arg[nargs++] = admnfile;
	}
	arg[nargs++] = pkginst;
	arg[nargs++] = NULL;

	tmpfunc = signal(SIGINT, func);
	n = pkgexecv(NULL, NULL, arg);
	(void) signal(SIGINT, tmpfunc);

	(void) signal(SIGINT, func);
	return(n);
}
