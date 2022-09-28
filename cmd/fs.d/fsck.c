/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)fs.cmds:fsck.c	1.18.4.1"

#include	<stdio.h>
#include	<errno.h>
#include	<limits.h>
#include	<sys/types.h>
#include	<sys/vfstab.h>

#define	ARGV_MAX	16
#define	FSTYPE_MAX	8
#define	VFS_PATH	"/usr/lib/fs"
#define	VFS_PATH2	"/etc/fs"

#define	CHECK(xx, yy)\
	if (xx == (yy)-1) {\
		fprintf(stderr, maxopterr, myname);\
		usage();\
	}
#define	OPTION(flag)\
		options++; \
		nargv[nargc++] = flag; \
		CHECK(nargc, ARGV_MAX);\
		break
#define	OPTARG(flag)\
		nargv[nargc++] = flag; \
		CHECK(nargc, ARGV_MAX);\
		if (optarg) {\
			nargv[nargc++] = optarg;\
			CHECK(nargc, ARGV_MAX);\
		}\
		break

extern char	*optarg;
extern int	optind;

extern char	*strrchr();

int	status;
int	nargc = 2;
int	options = 0;
char	*nargv[ARGV_MAX];
char	*myname, *fstype;
char	maxopterr[] = "%s: too many arguments\n";
char	vfstab[] = VFSTAB;

main(argc, argv)
	int	argc;
	char	*argv[];
{
	int	cc, ret;
	int	questflg = 0, Fflg = 0, Vflg = 0,sanity = 0;
	FILE	*fd;
	struct vfstab	vget, vref;

	myname = strrchr(argv[0], '/');
	if (myname)
		myname++;
	else
		myname = argv[0];

	while ((cc = getopt(argc, argv, "?bBDfF:lmnNo:pqs;S;t:T:VxyY")) != -1) {
		switch (cc) {
		case '?':
			questflg++;
			if (questflg > 1)
				usage();
			nargv[nargc++] = "-?";
			CHECK(nargc, ARGV_MAX);
			break;
		case 'b':
			OPTION("-b");
		case 'B':
			OPTION("-B");
		case 'D':
			OPTION("-D");
		case 'f':
			OPTION("-f");
		case 'F':
			Fflg++;
			/* check for more that one -F */
			if (Fflg > 1) {
				fprintf(stderr, "%s: more than one fstype specified\n", myname);
				usage();
			}
			fstype = optarg;
			if (strlen(fstype) > FSTYPE_MAX) {
				fprintf(stderr,
					"%s: Fstype %s exceeds %d characters\n",
					myname, fstype, FSTYPE_MAX);
						exit(1);
			}
			break;
		case 'l':
			OPTION("-l");
		case 'm':
			sanity++;
			OPTION("-m");
		case 'n':
			OPTION("-n");
		case 'N':
			OPTION("-N");
		case 'o':
			OPTARG("-o");
		case 'p':
			OPTION("-p");
		case 'q':
			OPTION("-q");
		case 's':
			OPTARG("-s");
		case 'S':
			OPTARG("-S");
		case 't':
			OPTARG("-t");
		case 'T':
			OPTARG("-T");
		case 'V':
			Vflg++;
			if (Vflg > 1)
				usage();
			break;
		case 'x':
			OPTION("-x");
		case 'y':
			OPTION("-y");
		case 'Y':
			OPTION("-Y");
		}
		optarg = NULL;
	}

	/* copy '--' to specific */
	if (strcmp(argv[optind-1], "--") == 0) {
		nargv[nargc++] = argv[optind-1];
		CHECK(nargc, ARGV_MAX);
	}

	if (questflg) {
		if (Fflg) {
			nargc = 2;
			nargv[nargc++] = "-?";
			nargv[nargc] = NULL;
			do_exec(fstype, nargv);
		}
		usage();
	}

	if ((sanity) && (options > 1)) {
		usage();
	}
	if (optind == argc) {
		if (fstype == NULL) {
			if ((argc > 2) && (sanity)) {
				usage();
			}
		}
		if ((fd = fopen(vfstab, "r")) == NULL) {
			fprintf(stderr, "%s: cannot open vfstab\n", myname);
			exit(1);
		}
		while ((ret = getvfsent(fd, &vget)) == 0)
			if (numbers(vget.vfs_fsckpass) &&
			    vget.vfs_fsckdev != NULL &&
			    vget.vfs_fstype != NULL &&
			   (fstype == NULL ||
			    strcmp(fstype, vget.vfs_fstype) == 0))
				execute(vget.vfs_fsckdev, vget.vfs_fstype, Vflg, fd);
		fclose(fd);
		if (ret > 0)
			vfserror(ret);
	} else {
		if (fstype == NULL && (fd = fopen(vfstab, "r")) == NULL) {
			fprintf(stderr, "%s: cannot open vfstab\n", myname);
			exit(1);
		}
		while (optind < argc) {
			if (fstype == NULL) {
				if ((argc > 3) && (sanity)) {
					usage();
				}
				/* must check for both special && raw devices */
				vfsnull(&vref);
				vref.vfs_fsckdev = argv[optind];
				if ((ret = getvfsany(fd, &vget, &vref)) == -1) {
					rewind(fd);
					vref.vfs_fsckdev = NULL;
					vref.vfs_special = argv[optind];
					ret = getvfsany(fd, &vget, &vref);
				}
				rewind(fd);
				if (ret == 0)
					execute(argv[optind], vget.vfs_fstype, Vflg, fd);
				else if (ret == -1) {

#ifdef i386
					execute(argv[optind], "s5", Vflg, fd);
				
#else
					fprintf(stderr, "%s: FSType cannot be identified\n", myname);
					exit(1);
#endif
				} else
					vfserror(ret);
			} else
				execute(argv[optind], fstype, Vflg, NULL);
			optind++;
		}
		if (fstype == NULL)
			fclose(fd);
	}
	exit(status);
}

/* see if all numbers */
numbers(yp)
	char	*yp;
{
	if (yp == NULL)
		return	0;
	while ('0' <= *yp && *yp <= '9')
		yp++;
	if (*yp)
		return	0;
	return	1;
}

execute(fsckdev, fstype, Vflg, fd)
	char	*fsckdev, *fstype;
	int	Vflg;
	FILE	*fd;
{
	int	st;
	pid_t	fk;
	char	full_path[PATH_MAX];
	char	*vfs_path = VFS_PATH;

	nargv[nargc] = fsckdev;

	if (Vflg) {
		prnt_cmd(stdout, fstype);
		return;
	}

	if ((fk = fork()) == (pid_t)-1) {
		fprintf(stderr, "%s: cannot fork.  Try again later\n", myname);
		perror(myname);
		exit(1);
	}

	if (fk == 0) {
		/* close the fd in the child only */
		if (fd)
			fclose(fd);

		/* Try to exec the fstype dependent portion of the mount. */
		do_exec(fstype, nargv);
	} else {
		/* parent waits for child */
		if (wait(&st) == (pid_t)-1) {
			fprintf(stderr, "%s: bad wait\n", myname);
			perror(myname);
			exit(1);
		}

		if ((st & 0xff) == 0x7f) {
			fprintf(stderr, "%s: warning: the following command (process %d) was stopped by signal %d\n", myname, fk, (st >> 8) & 0xff);
			prnt_cmd(stderr, fstype);
			status = ((st >> 8) & 0xff) | 0x80;
		} else if (st & 0xff) {
			fprintf(stderr, "%s: warning: the following command (process %d) was terminated by signal %d", myname, fk, st & 0x7f);
			if (st & 0x80)
				fprintf(stderr, " and dumped core");
			fprintf(stderr, "\n");
			prnt_cmd(stderr, fstype);
			status = ((st & 0xff) | 0x80);
		} else if (st & 0xff00)
			status = (st >> 8) & 0xff;
	}
}

do_exec(fstype, nargv)
	char	*fstype, *nargv[];
{
	char	full_path[PATH_MAX];
	char	*vfs_path = VFS_PATH;

	if (strlen(fstype) > FSTYPE_MAX) {
		fprintf(stderr,
			"%s: Fstype %s exceeds %d characters\n",
			myname, fstype, FSTYPE_MAX);
		exit(1);
	}

	/* build the full pathname of the fstype dependent command. */
	sprintf(full_path, "%s/%s/%s", vfs_path, fstype, myname);

	/* set the new argv[0] to the filename */
	nargv[1] = myname;

	/* Try to exec the fstype dependent portion of the mount. */
	execv(full_path, &nargv[1]);
	if (errno == EACCES) {
		fprintf(stderr, "%s: cannot execute %s - permission denied\n",myname,full_path);
	}
	if (errno == ENOEXEC) {
		nargv[0] = "sh";
		nargv[1] = full_path;
		execv("/sbin/sh", &nargv[0]);
	}
	/* second path to try */
	vfs_path = VFS_PATH2;
	/* build the full pathname of the fstype dependent command. */
	sprintf(full_path, "%s/%s/%s", vfs_path, fstype, myname);

	/* set the new argv[0] to the filename */
	nargv[1] = myname;
	/* Try to exec the second fstype dependent portion of the mount. */
	execv(full_path, &nargv[1]);
	if (errno == EACCES) {
		fprintf(stderr, "%s: cannot execute %s - permission denied\n",myname,full_path);
		exit(1);
	}
	if (errno == ENOEXEC) {
		nargv[0] = "sh";
		nargv[1] = full_path;
		execv("/sbin/sh", &nargv[0]);
	}
	fprintf(stderr, "%s: operation not applicable to FSType %s\n",myname,fstype);
	exit(1);
}

prnt_cmd(fd, fstype)
	FILE	*fd;
	char	*fstype;
{
	char	**argp;

	fprintf(fd, "%s -F %s", myname, fstype);
	for (argp = &nargv[2]; *argp; argp++)
		fprintf(fd, " %s", *argp);
	fprintf(fd, "\n");
}

vfserror(flag)
	int	flag;
{
	switch (flag) {
	case VFS_TOOLONG:
		fprintf(stderr, "%s: line in vfstab exceeds %d characters\n",
			myname, VFS_LINE_MAX-2);
		break;
	case VFS_TOOFEW:
		fprintf(stderr, "%s: line in vfstab has too few entries\n",
			myname);
		break;
	case VFS_TOOMANY:
		fprintf(stderr, "%s: line in vfstab has too many entries\n",
			myname);
		break;
	}
	exit(1);
}

extern int strcmp();
extern char *strchr();

int opterr = 1, optind = 1, optopt;
char *optarg;

getopt(argc, argv, opts)
int	argc;
char	**argv, *opts;
{
	static int sp = 1;
	register int c;
	register char *cp;

	if(sp == 1)
		if(optind >= argc ||
		   argv[optind][0] != '-' || argv[optind][1] == '\0')
			return(-1);
		else if(strcmp(argv[optind], "--") == 0) {
			optind++;
			return(-1);
		}
	optopt = c = argv[optind][sp];
	if(c == ':' || (cp=strchr(opts, c)) == 0) {
		if (opterr)
			fprintf(stderr, "%s: illegal option -- %c\n", *argv, c);
		if(argv[optind][++sp] == '\0') {
			optind++;
			sp = 1;
		}
		return('?');
	}
	if(*++cp == ':') {
		if(argv[optind][sp+1] != '\0')
			optarg = &argv[optind++][sp+1];
		else if(++optind >= argc) {
			if (opterr)
				fprintf(stderr, "%s: option requires an argument -- %c\n", *argv, c);
			sp = 1;
			return('?');
		} else
			optarg = argv[optind++];
		sp = 1;
	} else if (*cp == ';') {
		if (argv[optind][++sp] != '\0')
			if (isoptarg(c, &argv[optind][sp])) {
				optarg = &argv[optind++][sp];
				sp = 1;
			} else
				optarg = NULL;
		else {
			sp = 1;
			if (++optind >= argc || !isoptarg(c, &argv[optind][0]))
				optarg = NULL;
			else
				optarg = argv[optind++];
		}
	} else {
		if(argv[optind][++sp] == '\0') {
			sp = 1;
			optind++;
		}
		optarg = NULL;
	}
	return(c);
}

isoptarg(cc, arg)
	int	cc;
	char	*arg;
{
	if (cc == 's' || cc == 'S') {
		while(*arg >= '0' && *arg <= '9')
			arg++;
		if(*arg++ != ':')
			return	0;
		while(*arg >= '0' && *arg <= '9')
			arg++;
		if (*arg)
			return	0;
		return	1;
	}
	return	0;
}

usage()
{
	fprintf(stderr,"Usage:\n%s [-F FSType] [-V] [-m] [special ...]\n", myname);
	fprintf(stderr,"%s [-F FSType] [-V] [current_options] [-o specific_options] [special ...]\n", myname);

	exit(1);
}
