/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:pkginstall/main.c	1.19.8.1"

#include <stdio.h>
#include <time.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <pkgstrct.h>
#include <pkginfo.h>
#include <pkgdev.h>
#include <pkglocs.h>
#include "install.h"

extern char	*optarg, **environ;
extern char	*pkgabrv, *pkgname,
		*pkgarch, *pkgvers,
		pkgwild[];
extern int	optind, errno, 
		Mntflg, installed;

extern long	ulimit();
extern void	*calloc(), 
		*realloc(),
		putparam(),
		ptext(),
		echo(),
		lockinst(),
		trap(),
		getbase(),
		setadmin(),
		merginfo(),
		ckrunlevel(),
		ckconflct(),
		cksetuid(),
		ckdepend(),
		ckpriv(),
		ckpartial(),
		ckspace(),
		ckdirs(),
		ckpkgdirs(),
		ckpkgfiles();
extern char	*qstrdup(),
		*mktemp(), 
		*pkgparam(), 
		*fpkgparam(), 
		*fpkginst(), 
		*getenv(),
		*getinst();
extern long	atol();
extern unsigned long
		umask();
extern int	getopt(),
		access(),
		unlink(),
		atoi(),
		mkdir(),
		chdir(),
		creat(),
		pkginfo(),
		ckvolseq(),
		ocfile(),
		pkgnmchk(),
		swapcfile(),
		pkgenv(),
		rrmdir(),
		pkgexecl(),
		sortmap(),
		reqexec(),
		setlist(),
		cftime(),
		ds_next(),
		ds_getinfo();
extern void	exit(),
		progerr(),
		logerr(),
		instvol(),
		predepend(),
		quit();

#define DEFPATH		"/sbin:/usr/sbin:/usr/bin"
#define MALSIZ	4	/* best guess at likely maximum value of MAXINST */ 
#define LSIZE	256	/* maximum line size supported in copyright file */

#define ERR_MEMORY	"memory allocation failure, errno=%d"
#define ERR_BADFORMAT	"<%s> is not properly formatted for installation"
#define ERR_INTONLY	"unable to install <%s> without user interaction"
#define ERR_NOREQUEST	"package does not contain an interactive request script"
#define ERR_LOCKFILE	"unable to create lockfile <%s>"
#define ERR_PKGINFO	"unable to open pkginfo file <%s>"
#define ERR_RESPONSE	"unable to open response file <%s>"
#define ERR_PKGMAP	"unable to open pkgmap file <%s>"
#define ERR_ULIMIT	"unable to set ulimit to <%ld>"
#define ERR_MKDIR	"unable to make temporary directory <%s>"
#define ERR_RMDIR	"unable to remove directory <%s> and its contents"
#define ERR_CHDIR	"unable to change directory to <%s>"
#define ERR_DSTREAM	"unable to unpack datastream"
#define ERR_DSTREAMSEQ	"datastream seqeunce corruption"
#define ERR_DSTREAMCNT	"datastream early termination problem"
#define ERR_RDONLY 	"read-only parameter <%s> cannot be assigned a value"
#define ERR_REQUEST	"request script did not complete successfully"
#define ERR_PREINSTALL	"preinstall script did not complete successfully"
#define ERR_POSTINSTALL	"postinstall script did not complete successfully"
#define ERR_OPRESVR4	"unable to unlink otpions file <%s>"
#define ERR_SYSINFO	\
	"unable to process installed package information, errno=%d"

#define MSG_INSTALLED	"   %d package %s already properly installed."

struct admin	adm;
struct pkgdev	pkgdev;

int	nocnflct, nosetuid;
int	dbchg;
int	rprcflag;
int	iflag;
int	dparts = 0;

int	reboot = 0;
int	ireboot = 0;
int	warnflag = 0;
int	failflag = 0;
int	started = 0;
int	update = 0;
int	opresvr4 = 0;
int	nointeract = 0;
int	maxinst = 1;

char	*prog,
	*pkginst,
	*msgtext,
	*respfile,
	**class,
	**pclass,
	instdir[PATH_MAX],
	pkgloc[PATH_MAX],
	pkgbin[PATH_MAX],
	pkgsav[PATH_MAX],
	ilockfile[PATH_MAX],
	rlockfile[PATH_MAX],
	savlog[PATH_MAX],
	tmpdir[PATH_MAX];
unsigned nclass;

void	ckreturn();

static char	*ro_params[] = {
	"PATH", "NAME", "PKG", "PKGINST",
	"VERSION", "ARCH", "BASEDIR", 
	"INSTDATE", "CATEGORY",
	NULL
};

static int	rdonly();
static void	copyright(),
		usage(),
		unpack();

main(argc, argv)
int	argc;
char	*argv[];
{
	struct pkginfo *prvinfo;
	FILE	*mapfp, *tmpfp;
	FILE	*fp;
	int	c, n;
	long	clock, limit;
	int	npkgs, part, nparts;
	char	*pt, 
		*value, 
		*srcinst, 
		*device,
		*admnfile = NULL;
	char	path[PATH_MAX],
		cmdbin[PATH_MAX],
		p_pkginfo[PATH_MAX],
		p_pkgmap[PATH_MAX],
		script[PATH_MAX],
		cbuf[64],
		param[64];
	void	(*func)();

	prog = strrchr(argv[0], '/');
	if(!prog++)
		prog = argv[0];

	(void) umask(0022);

	device = NULL;
	while((c = getopt(argc, argv, "N:Mf:p:d:m:b:r:ina:?")) != EOF) {
		switch(c) {

		  case 'N':
			prog = optarg;
			break;

		  case 'M':
			Mntflg++;
			break;

		  case 'p':
			dparts = ds_getinfo(optarg);
			break;

		  case 'i':
			iflag++;
			break;

		  case 'f':
			pkgdev.fstyp = optarg;
			break;

		  case 'b':
			(void) strcpy(cmdbin, optarg);
			break;

		  case 'd':
			device = optarg;
			break;

		  case 'm':
			pkgdev.mount = optarg;
			pkgdev.rdonly++;
			pkgdev.mntflg++;
			break;

		  case 'r':
			respfile = optarg;
			break;

		  case 'n':
			nointeract++;
			break;

		  case 'a':
			admnfile = optarg;
			break;

		  default:
			usage();
		}
	}
	if(iflag && (respfile == NULL))
		usage();

	if(device) {
		if(pkgdev.mount)
			pkgdev.bdevice = device;
		else 
			pkgdev.cdevice = device;
	}
	if(pkgdev.fstyp && !pkgdev.mount) {
		progerr("-f option requires -m option");
		usage();
	}

	n = strlen(prog) - sizeof("INSTALL") + 1; /* length of INSTALL */
	if((n >= 0) && !strcmp(&prog[n], "INSTALL")) {
		srcinst = COREPKG;
		pkgdev.bdevice = argv[optind++];
		pkgdev.dirname = pkgdev.mount = argv[optind];
		pkgdev.rdonly++;
		pkgdev.mntflg++;
		(void) sprintf(cmdbin, 
			"%s/%s/reloc.1/$UBIN:%s/%s/reloc.1/$SBIN",
			pkgdev.mount, COREPKG, pkgdev.mount, COREPKG);
	} else {
		pkgdev.dirname = argv[optind++];
		srcinst = argv[optind++];
		if(optind != argc)
			usage();
	}

	(void) pkgparam(NULL, NULL);
	if(!strcmp(srcinst, COREPKG)) {
		/* if this is the core package, turn off some of the normal
		 * functionality since we are using the package itself to
		 * do this installation
		 */
		adm.mail = NULL;
		adm.space = adm.idepend = adm.runlevel = adm.action =
			adm.partial = adm.conflict = adm.setuid = "nocheck";
		/* create initial system directories */
	} else {
		/*
		 * initialize installation admin parameters 
		 */
		setadmin(admnfile);
	}

	func = signal(SIGINT, trap);
	if(func != SIG_DFL)
		(void) signal(SIGINT, func);
	(void) signal(SIGHUP, trap);

	ckdirs();
	lockinst();
	tzset();

	(void) sprintf(instdir, "%s/%s", pkgdev.dirname, srcinst);

	if(pt = getenv("TMPDIR"))
		(void) sprintf(tmpdir, "%s/installXXXXXX", pt);
	else
		(void) strcpy(tmpdir, "/tmp/installXXXXXX");
	if((mktemp(tmpdir) == NULL) || mkdir(tmpdir, 0700)) {
		progerr(ERR_MKDIR, tmpdir);
		quit(99);
	}

	pt = getenv("TZ");
	environ = NULL;
	if(pt)
		putparam("TZ", pt);
	if(!cmdbin[0])
		(void) strcpy(cmdbin, PKGBIN);
	(void) sprintf(path, "%s:%s", DEFPATH, cmdbin);
	putparam("PATH", path);
	putparam("OAMBASE", "/usr/sadm/sysadm");

	(void) sprintf(p_pkginfo, "%s/%s", instdir, PKGINFO);
	(void) sprintf(p_pkgmap, "%s/%s", instdir, PKGMAP);

	/* verify existence of required information files */
	if(access(p_pkginfo, 0) || access(p_pkgmap, 0)) {
		progerr(ERR_BADFORMAT, instdir);
		quit(99);
	}
		
	if(pkgenv(instdir, srcinst))
		quit(1);

	if(pt = getenv("MAXINST"))
		maxinst = atol(pt);

	/*
	 *  verify that we are not trying to install an 
	 *  INTONLY package with no interaction
	 */
	if(pt = getenv("INTONLY")) {
		if(iflag || nointeract) {
			progerr(ERR_INTONLY, pkgabrv);
			quit(1);
		}
	}

	echo("\n%s", pkgname);
	echo("Version %s (%s)", pkgvers, pkgarch);
	copyright();

	/*
	 *  if this script was invoked by 'pkgask', just
	 *  execute request script and quit
	 */
	if(iflag) {
		if(pkgdev.cdevice)
			unpack(1, 0);
		(void) sprintf(path, "%s/install/request", instdir);
		if(access(path, 0)) {
			progerr(ERR_NOREQUEST);
			quit(1);
		}
		ckreturn(reqexec(path, respfile), ERR_REQUEST);

		if(warnflag || failflag) {
			(void) unlink(respfile);
			echo("\nResponse file <%s> was not created.", respfile);
		} else
			echo("\nResponse file <%s> was created.", respfile);
		quit(0);
	}

	/* 
	 * inspect the system to determine if any instances of the
	 * package being installed already exist on the system
	 */
	npkgs = 0;
	prvinfo = (struct pkginfo *) calloc(MALSIZ, sizeof(struct pkginfo));
	if(prvinfo == NULL) {
		progerr(ERR_MEMORY, errno);
		quit(99);
	}
	for(;;) {
		if(pkginfo(&prvinfo[npkgs], pkgwild, NULL, NULL)) {
			if((errno == ESRCH) || (errno == ENOENT)) 
				break;
			progerr(ERR_SYSINFO, errno);
			quit(99);
		}
		if((++npkgs % MALSIZ) == 0) {
			prvinfo = (struct pkginfo *) realloc(prvinfo, 
				(npkgs+MALSIZ) * sizeof(struct pkginfo));
			if(prvinfo == NULL) {
				progerr(ERR_MEMORY, errno);
				quit(99);
			}
		}
	}

	if(npkgs > 0) {
		/* 
		 * since an instance of this package already exists on
		 * the system, we must interact with the user to determine
		 * if we should overwrite an instance which is already
		 * installed, or possibly install a new instance of
		 * this package
		 */
		pkginst = getinst(prvinfo, npkgs);
	} else {
		/* 
		 * the first instance of a package on the system is 
		 * always identified by the package abbreviation
		 */
		pkginst = pkgabrv;
	}
	
	(void) sprintf(pkgloc, "%s/%s", PKGLOC, pkginst);
	(void) sprintf(pkgbin, "%s/install", pkgloc);
	(void) sprintf(pkgsav, "%s/save", pkgloc);
	(void) sprintf(ilockfile, "%s/!I-Lock!", pkgloc);
	(void) sprintf(rlockfile, "%s/!R-Lock!", pkgloc);
	(void) sprintf(savlog, "%s/logs/%s", PKGADM, pkginst);

	putparam("PKGINST", pkginst);
	putparam("PKGSAV", pkgsav);

	if(update) {
		/* read in parameter values from instance which is
		 * currently installed 
		 */
		if((fp = fopen(p_pkginfo, "r")) == NULL) {
			progerr(ERR_PKGINFO, p_pkginfo);
			quit(99);
		}
		param[0] = '\0';
		while(value = fpkgparam(fp, param)) {
			if(!strcmp(param, "CLASSES"))
				(void) setlist(&pclass, qstrdup(value));
			else if(!getenv(param) || !strcmp("BASEDIR", param))
				putparam(param, value);
			param[0] = '\0';
		}
		(void) fclose(fp);
		putparam("UPDATE", "yes");
	} else
		putparam("UPDATE", NULL);

	/*
	 *  determine if the pacakge has been partially
	 *  installed on or removed from this system
	 */
	ckpartial();

	/*
	 *  make sure current runlevel is appropriate
	 */
	ckrunlevel();

	/*
	 * determine package base directory (if this 
	 * package is relocatable)
	 */
	getbase();

	if(pkgdev.cdevice)
		/* get first volume which contains info files */
		unpack(1, 0);

	/*
	 * if no response file has been provided,
	 * initialize response file by executing any
	 * request script provided by this package
	 */
	if(respfile == NULL) {
		(void) sprintf(path, "%s/install/request", instdir);
		if(n = reqexec(path, NULL))
			ckreturn(n, ERR_REQUEST);
	}

	/* look for all parameters in response file which begin
	 * with a capital letter, and place them in the
	 * environment
	 */
	if(respfile) {
		if((fp = fopen(respfile, "r")) == NULL) {
			progerr(ERR_RESPONSE, respfile);
			quit(99);
		}
		param[0] = '\0';
		while(value = fpkgparam(fp, param)) {
			if(!isupper(param[0])) {
				param[0] = '\0';
				continue;
			}
			if(rdonly(param)) {
				progerr(ERR_RDONLY, param);
				param[0] = '\0';
				continue;
			}
			putparam(param, value);
			param[0] = '\0';
		}
		(void) fclose(fp);
	}
	nclass = setlist(&class, qstrdup(getenv("CLASSES")));

	/*
	 * the following two checks are done in the corresponding 
	 * ck() routine, but are repeated here to avoid re-processing
	 * the database if we are administered to not include these
	 * processes
	 */
	if(ADM(setuid, "nochange"))
		nosetuid++;
	if(ADM(conflict, "nochange"))
		nocnflct++;

	/*
	 * merg information in memory with the "contents" file; 
	 * this creates a temporary version of the "contents"
	 * file and modifies the entry in memory to reflect
	 * how the entry should look after the merg is complete
	 */

	if(ocfile(&mapfp, &tmpfp))
		quit(99);

	if((fp = fopen(p_pkgmap, "r")) == NULL) {
		progerr(ERR_PKGMAP, p_pkgmap);
		quit(99);
	}
	nparts = sortmap(fp, mapfp, tmpfp);
	if(installed > 0) {
		echo(MSG_INSTALLED, installed,
			(installed > 1) ? "pathnames are" : "pathname is");
	}

	/*
	 *  verify package information files are not corrupt
	 */
	ckpkgfiles();

	/*
	 *  verify package dependencies
	 */
	ckdepend();

	/*
	 *  check space requirements
	 */
	ckspace();

	/*
	 *  determine if any objects provided by this package
	 *  conflict with previously installed packages
	 */
	ckconflct();

	/*
	 *  determine if any objects provided by this package
	 *  will be installed with setuid or setgid enabled
	 */
	cksetuid();

	/*
	 * determine if any packaging scripts provided with
	 * this package will execute as a priviledged user
	 */
	ckpriv();

	/*
	 * if we have assumed that we were installing setuid or
	 * conflicting files, and the user chose to do otherwise,
	 * we need to read in the package map again and re-merg
	 * with the "contents" file
	 */
	if(rprcflag) {
		/*
		 * Work around for 'contents' file corruption problem
		 * here by using fopen()/fclose(). Function ocfile()
		 * takes care of the rest.
		*/

		fclose(tmpfp);
		fclose(mapfp);
		if(ocfile(&mapfp, &tmpfp))
			quit(99);
		nparts = sortmap(fp, mapfp, tmpfp);
	}
	(void) fclose(fp);

	echo("\nInstalling %s as <%s>\n", pkgname, pkginst);
	started++;

	/*
	 *  verify neccessary package installation directories exist
	 */
	ckpkgdirs();

	/*
	 *  create lockfile to indicate start of installation
	 */
	if(creat(ilockfile, 0644) < 0) {
		progerr(ERR_LOCKFILE, ilockfile);
		quit(99);
	}

	/* 
	 * replace contents file with recently created temp version
	 * which contains information about the objects being installed
	 */
	(void) fclose(mapfp);
	if(swapcfile(tmpfp, (dbchg ? pkginst : NULL)))
		quit(99);

	(void) time(&clock);
	(void) cftime(cbuf, "%b %d \045Y \045I:\045M", &clock);
	putparam("INSTDATE", qstrdup(cbuf));

	/*
	 *  check ulimit requirement (provided in pkginfo)
	 */
	if(pt = getenv("ULIMIT")) {
		limit = atol(pt);
		if((limit <= 0) || ulimit(2, limit)) {
			progerr(ERR_ULIMIT, limit);
			quit(99);
		}
	}

	/*
	 *  store information about package being installed;
	 *  modify installation parameters as neccessary and
	 *  copy contents of 'install' directory into $pkgloc
	 */
	merginfo();

	if(opresvr4) { 
		/* we are overwriting a pre-svr4 package,
		 * so remove the file in /usr/options now
		 */
		(void) sprintf(path, "%s/%s.name", PKGOLD, pkginst);
		if(unlink(path) && (errno != ENOENT)) {
			progerr(ERR_OPRESVR4, path);
			warnflag++;
		}
	}

	/*
	 *  execute preinstall script, if any
	 */
	(void) sprintf(script, "%s/install/preinstall", instdir);
	if(access(script, 0) == 0) {
		echo("## Executing preinstall script.");
		/* execute script residing in pkgbin instead of media */
		(void) sprintf(script, "%s/preinstall", pkgbin);
		if(n = pkgexecl(NULL, NULL, SHELL, script, NULL))
			ckreturn(n, ERR_PREINSTALL);
	}

	/* Check if postinstall script exists on package */
	(void) sprintf(script, "%s/install/postinstall", instdir);
	if(access(script, 0) != 0)
		script[0] = '\0';

	/*
	 *  install package one part (volume) at a time
	 */
	part = 1;
	while(part <= nparts) {
		if((part > 1) && pkgdev.cdevice) 
			unpack(part, nparts);
		instvol(srcinst, part, nparts);
		if(part++ >= nparts)
			break;
	}

	/*
	 *  execute postinstall script, if any
	 */
	if(script[0]) {
		(void) sprintf(script, "%s/postinstall", pkgbin);
		if(access(script, 0) == 0) {
			echo("## Executing postinstall script.");
			if(n = pkgexecl(NULL, NULL, SHELL, script, NULL))
				ckreturn(n, ERR_POSTINSTALL);
		}
	}

	if(!warnflag && !failflag) {
		if(pt = getenv("PREDEPEND"))
			predepend(pt);
		(void) unlink(rlockfile);
		(void) unlink(ilockfile);
		(void) unlink(savlog);
	}

	quit(0);
	/*NOTREACHED*/
}

void
ckreturn(retcode, msg)
int	retcode;
char	*msg;
{
	switch(retcode) {
	  case 2:
	  case 12:
	  case 22:
		warnflag++;
		if(msg)
			progerr(msg);
		/* fall through */
	  case 10:
	  case 20:
		if(retcode >= 10)
			reboot++;
		if(retcode >= 20)
			ireboot++;
		/* fall through */
	  case 0:
		break; /* okay */

	  case -1:
		retcode = 99;
	  case 99:
	  case 1:
	  case 11:
	  case 21:
	  case 4:
	  case 14:
	  case 24:
	  case 5:
	  case 15:
	  case 25:
		if(msg)
			progerr(msg);
		/* fall through */
	  case 3:
	  case 13:
	  case 23:
		quit(retcode);
		/* NOT REACHED */
	  default:
		if(msg)
			progerr(msg);
		quit(1);
	}
}

#define COPYRIGHT "install/copyright"

static void
copyright()
{
	FILE	*fp;
	char	line[LSIZE];
	char	cprtfile[PATH_MAX];

	sprintf(cprtfile, "%s/%s/", instdir, COPYRIGHT);
	if((fp = fopen(cprtfile, "r")) == NULL) {
		echo(getenv("VENDOR"));
	} else {
		while(fgets(line, LSIZE, fp))
			(void) fprintf(stderr, "%s", line);
		(void) fclose(fp);
	}
}


static int
rdonly(p)
char *p;
{
	int	i;

	for(i=0; ro_params[i]; i++) {
		if(!strcmp(p, ro_params[i]))
			return(1);
	}
	return(0);
}
	
static void
unpack(part, nparts)
int	part, nparts;
{
	/*
	 * read in next part from stream, even if we decide
	 * later that we don't need it
	 */
	if(dparts < 1) {
		progerr(ERR_DSTREAMCNT);
		quit(99);
	}
	if((access(instdir, 0) == 0) && rrmdir(instdir)) {
		progerr(ERR_RMDIR, instdir);
		quit(99);
	}
	if(mkdir(instdir, 0755)) {
		progerr(ERR_MKDIR, instdir);
		quit(99);
	}
	if(chdir(instdir)) {
		progerr(ERR_CHDIR, instdir);
		quit(99);
	}
	dparts--;
	if(ds_next(pkgdev.cdevice, instdir)) {
		progerr(ERR_DSTREAM);
		quit(99);
	}
	if(chdir(PKGADM)) {
		progerr(ERR_CHDIR, PKGADM);
		quit(99);
	}
}

static void
usage()
{
	(void) fprintf(stderr, "usage: %s [-d device] [-m mountpt [-f fstyp]] ",
		prog);
	(void) fprintf(stderr, "[-b bindir] [-a adminf] [-r respf] ");
	(void) fprintf(stderr, "directory pkginst\n");
	exit(1);
}
