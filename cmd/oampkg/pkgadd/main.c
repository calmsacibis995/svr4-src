/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:pkgadd/main.c	1.12.17.1"

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <pkgdev.h>
#include <pkginfo.h>
#include <pkglocs.h>
#include "install.h"
#include "msgdefs.h"

extern char	*optarg;
extern int	optind, errno, ckquit;
extern int	ds_totread;

extern void	trap(),
		progerr(),
		logerr(),
		quit(),
		exit(),
		ptext(),
		ds_order(),
		ds_putinfo(),
		setadmin(),
		echo();
extern int	devtype(),
		presvr4(),
		ckyorn(),
		pkgexecv(),
		ds_init(),
		ds_close(),
		ds_findpkg(),
		ds_readbuf(),
		isdir(),
		getvol(),
		_getvol(),
		getopt(),
		access(),
		mkdir(),
		chdir(),
		pkgmount(),
		pkgumount(),
		pkgtrans();
extern char	*pkgparam(), *devattr(), **gpkglist(), *getenv();

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
int	one_time = 0;	/* non-zero if one time installation only */
int	npkgs;		/* the number of packages yet to be installed */

char	*pkginst;	/* current package (source) instance to process */
char	*prog;		/* the basename of argv[0] */
char	*respfile;	/* response pathname (or NULL) */
char	*tmpdir;	/* location to place temporary files */
char	*ids_name;	/* name of data stream device */
void	(*func)();

static char	*device,
		*admnfile,	/* file to use for installation admin */
		*respdir,	/* used is respfile is a directory spec */
		*spoolto;	/* optarg specified with -s (or NULL) */
static int	interrupted,	/* if last pkg installation was quit */
		askflag;	/* non-zero if this is the "pkgask" process */

static int	givemsg,
		inprogress();

#define MAXARGS	20

static void	usage();
static int	pkginstall();
void	ckreturn();

main(argc,argv)
int	argc;
char	**argv;
{
	int	i, c, n,
		repeat;
	char	ans, 
		path[PATH_MAX],
		**pkglist;	/* points to array of packages */

	prog = strrchr(argv[0], '/');
	if(!prog++)
		prog = argv[0];
	askflag = !strcmp(prog, "pkgask");

	device = NULL;
	while ((c = getopt(argc,argv,"s:d:a:r:Onc?")) != EOF) {
		switch(c) {
		  case 's':
			spoolto = optarg;
			break;

		  case 'd':
			device = optarg;
			break;

		  case 'a':
			admnfile = optarg;
			break;

		  case 'r':
			respfile = optarg;
			if(isdir(respfile) == 0)
				respdir = respfile;
			break;

		  case 'n':
			nointeract++;
			break;

		  case 'O':
			one_time++;
			break;

		  default:
			usage();
		}
	}

	if(askflag && (spoolto || nointeract))
		usage();

	if(spoolto && (nointeract || admnfile || respfile))
		usage();

	func = signal(SIGINT, trap);
	if(func != SIG_DFL)
		(void) signal(SIGINT, func);
	(void) signal(SIGHUP, trap);

	/*
	 * initialize installation admin parameters 
	 */
	setadmin(admnfile);

	/*
	 * process response file argument
	 */
	if(respfile) {
		if(respfile[0] != '/') {
			progerr("response file <%s> must be full pathname",
				respfile);
			quit(1);
		}
		if(respdir == NULL) {
			if(askflag) {
				if(access(respfile, 0) == 0) {
					progerr("response file <%s> must not exist",
						respfile);
					quit(1);
				}
			} else if(access(respfile, 0)) {
				progerr("unable to access response file <%s>", 
					respfile);
				quit(1);
			}
		}
	} else if(askflag) {
		progerr("response file (to write) is required");
		usage();
		quit(1);
	}

	if(device == NULL) {
		device = devattr("spool", "pathname");
		if(device == NULL) {
			progerr("unable to determine device to install from");
			quit(1);
		}
	}

	if(spoolto)
		quit(pkgtrans(device, spoolto, &argv[optind], 0));

	tmpdir = getenv("TMPDIR");
	if(tmpdir == NULL)
		tmpdir = P_tmpdir;

recycle:
	if(devtype(device, &pkgdev)) {
		progerr("bad device <%s> specified", device);
		quit(1);
	}

again:
	givemsg = 1;

	ids_name = NULL;
	if(pkgdev.bdevice) {
		if (!one_time)
		   if(n = _getvol(pkgdev.bdevice, NULL, NULL, "Insert %v into %p.", pkgdev.norewind)) {
			if(n == 3)
				quit(3);
			if(n == 2)
				progerr("unknown device <%s>", pkgdev.name);
			else {
				progerr("unable to obtain package volume");
				logerr("getvol() returned <%d>", n);
			}
			quit(99);
		}
		inprogress(devattr(device,"volume"));
		if (isoam_ins(pkgdev.name, prog))
			quit(99);
		if(ds_readbuf(pkgdev.cdevice))
			ids_name = pkgdev.cdevice;
	}

	if(pkgdev.cdevice && !pkgdev.bdevice)
		ids_name = pkgdev.cdevice;
	else if(pkgdev.pathname)
		ids_name = pkgdev.pathname;

	if(ids_name) {
		/* initialize datastream */
		pkgdev.dirname = tempnam(tmpdir, "dstream");
		if(!pkgdev.dirname || mkdir(pkgdev.dirname, 0755)) {
			progerr(ERR_STREAMDIR);
			quit(99);
		}
	}

	repeat = ((optind >= argc) && (!one_time)) ;

	if(!ids_name && pkgdev.mount) {
		pkgdev.rdonly++;
		if(n = pkgmount(&pkgdev, NULL, 0, 0, 0))
		{
			if(ids_name) {
				(void)ds_close(1);
				rrmdir(pkgdev.dirname);
			}
			goto again;
		}
	}

	if(ids_name) {
		/* use character device to force rewind of datsream */
		if(pkgdev.cdevice && !pkgdev.bdevice && (!one_time) &&
		(n = _getvol(pkgdev.name, NULL, NULL, NULL, pkgdev.norewind))) {
			if(n == 3)
				quit(3);
			if(n == 2)
				progerr("unknown device <%s>", pkgdev.name);
			else {
				progerr("unable to obtain package volume");
				logerr("getvol() returned <%d>", n);
			}
			quit(99);
		}

		inprogress(devattr(device,"volume"));
		if(chdir(pkgdev.dirname)) {
			progerr(ERR_CHDIR, pkgdev.dirname);
			quit(99);
		}
		if(ds_init(ids_name, &argv[optind], pkgdev.norewind)) {
			progerr(ERR_DSINIT, ids_name);
			quit(99);
		}
	}

	pkglist = gpkglist(pkgdev.dirname, &argv[optind]);
	if(pkglist == NULL) {
		if(errno == ENOPKG) {
			/* check for existence of pre-SVR4 package */
			(void) sprintf(path, "%s/install/INSTALL", 
				pkgdev.dirname);
			if(access(path, 0) == 0) {
				pkginst = ((optind < argc) ? 
					argv[optind++] : NULL);
				ckreturn(presvr4(&pkginst));
				if(repeat || (optind < argc)) {
					if(ids_name) {
						(void)ds_close(1);
						rrmdir(pkgdev.dirname);
					}
					goto recycle;
				}
				quit(0);
			}
			progerr(ERR_NOPKGS, pkgdev.dirname);
			quit(1);
		} else {
			/* some internal error */
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
				break;
			}
		}
	}
	if(ids_name)
		ds_order(pkglist);

	for(npkgs=0; pkglist[npkgs]; )
		npkgs++;

	interrupted = 0;
	for(i=0; pkginst = pkglist[i]; i++) {
		if(ireboot && !askflag) {
			ptext(stderr, MSG_SUSPEND, pkginst);
			continue;
		}
		if(interrupted) {
			if(npkgs == 1) {
				echo(MSG_1MORETODO, 
					(askflag ? "processed" : "installed"));
			} else {
				echo(MSG_MORETODO, npkgs,
					(askflag ? "processed" : "installed"));
			}
			if(nointeract)
				quit(0);
			ckquit = 0;
			if(n = ckyorn(&ans, NULL, NULL, NULL, ASK_CONTINUE))
				quit(n);
			ckquit = 1;
			if(ans != 'y')
				quit(0);
		}
		if(askflag) {
			if(respdir) {
				(void) sprintf(path, "%s/%s", respdir, pkginst);
				respfile = path;
			} else if(npkgs > 1) {
				progerr("too many packages referenced!");
				quit(1);
			}
		} else {
			if(respdir){
				(void) sprintf(path, "%s/%s", respdir, pkginst);
				respfile = path;
				if(access(respfile, 0)) {
					progerr("unable to access response file <%s>", 
					respfile);
					quit(1);
				}
			}
		}
		interrupted = 0;
			
		echo("\nProcessing package instance <%s> from <%s>",
			pkginst, device);

		ckreturn(pkginstall());
		npkgs--;
		if((npkgs <= 0) && (pkgdev.mount || ids_name)) {
			(void) chdir("/");
			if(!ids_name)
				(void) pkgumount(&pkgdev);
		}
	}
	if(!ireboot && repeat && !pkgdev.pathname) {
		if(ids_name) {
			(void)ds_close(1);
			rrmdir(pkgdev.dirname);
		}
		goto recycle;
	}
	quit(0);
	/*NOTREACHED*/
}

static int
pkginstall()
{
	void	(*tmpfunc)();
	char	*arg[MAXARGS], path[PATH_MAX];
	char	buffer[256];
	int	n, nargs, dparts;

	(void) sprintf(path, "%s/pkginstall", PKGBIN);

	nargs = 0;
	arg[nargs++] = path;
	if(askflag)
		arg[nargs++] = "-i";
	else if(nointeract)
		arg[nargs++] = "-n";
	if(admnfile) {
		arg[nargs++] = "-a";
		arg[nargs++] = admnfile;
	}
	if(respfile) {
		arg[nargs++] = "-r";
		arg[nargs++] = respfile;
	}
	if(ids_name != NULL) {
		arg[nargs++] = "-d";
		arg[nargs++] = ids_name;
		dparts = ds_findpkg(ids_name, pkginst);
		if(dparts < 1) {
			progerr("unable to find archive for <%s> in datastream",
				pkginst);
			quit(99);
		}
		arg[nargs++] = "-p";
		ds_putinfo(buffer);
		arg[nargs++] = buffer;
	} else if(pkgdev.mount != NULL) {
		arg[nargs++] = "-d";
		arg[nargs++] = pkgdev.bdevice;
		arg[nargs++] = "-m";
		arg[nargs++] = pkgdev.mount;
		if(pkgdev.fstyp != NULL) {
			arg[nargs++] = "-f";
			arg[nargs++] = pkgdev.fstyp;
		}
	}
	arg[nargs++] = "-N";
	arg[nargs++] = prog;
	arg[nargs++] = pkgdev.dirname;
	arg[nargs++] = pkginst;
	arg[nargs++] = NULL;

	tmpfunc = signal(SIGINT, func);
	n = pkgexecv(NULL, NULL, arg);
	(void) signal(SIGINT, tmpfunc);
	if(ids_name != NULL)
		ds_totread += dparts; /* increment number of parts written */
	return(n);
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

static void
usage()
{
	if(askflag) {
		(void) fprintf(stderr, "usage: %s ", prog);
		(void) fprintf(stderr, "-r response [-d device] ");
		(void) fprintf(stderr, "[pkg [pkg ...]]\n");
	} else {
		(void) fprintf(stderr, "usage:\n\t%s ", prog);
		(void) fprintf(stderr, "[-n] [-d device] [-a admin] [-r response] ");
		(void) fprintf(stderr, "[pkg [pkg ...]]\n");
		(void) fprintf(stderr, "\t%s -s dir [-d device] [pkg [pkg ...]]\n",
			prog);
	}
	exit(1);
	/*NOTREACHED*/
}

isoam_ins(devname, cmd)
char	*devname;
char	*cmd;
{
	char	command[512];
	char	cmdbuf[512];
	FILE	*fp;

	if (strncmp(devname, "diskette", 8) == 0) {
		sprintf(command, "/usr/sadm/sysadm/bin/isoam.ins %s", devname);
		if ((fp = popen(command, "r")) != NULL) {
			while (fgets(cmdbuf, 512, fp) != NULL)
				;
			fflush(fp);
			(void) pclose(fp);
			if (strncmp(cmdbuf, "false", 5) == 0) {
				echo("\nYou are attempting to install a package that can not be installed");
				echo("using this command. If this is a Pre-SVR4 package, use the \"installpkg\"");
				echo("command to install this package.\n");
				return(-1);
			}
		}
		else
			fprintf(stderr, "WARNING: %s: popen() failed:\nCould not execute: %s\n",cmd,command);
	}
	return(0);
}

static int
inprogress(drive)
char *drive;
{
	if (givemsg == 1) {
	echo ("\n\nInstallation in progress.  Do not remove the %s.\n", drive);
	givemsg = 0;
	}
}
