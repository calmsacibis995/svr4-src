/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)fs.cmds:mount.c	1.85"

#include	<stdio.h>
#include 	<limits.h>
#include 	<fcntl.h>
#include 	<unistd.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/statvfs.h>
#include	<sys/errno.h>
#include	<sys/mnttab.h>
#include	<sys/vfstab.h>
#include	<locale.h>

#define	VFS_PATH	"/usr/lib/fs"
#define ALT_PATH	"/etc/fs"
#define	REMOTE		"/etc/dfs/fstypes"
#define SEM_FILE	"/etc/.mnt.lock"

#define	ARGV_MAX	16
#define	TIME_MAX	50
#define	LINE_MAX	64
#define	FSTYPE_MAX	8
#define	REMOTE_MAX	64

#define	OLD	0
#define	NEW	1

#define	READONLY	0
#define	READWRITE	1
#define SUID 		2
#define NOSUID		3

#define	FORMAT	"%a %b %e %H:%M:%S %Y\n"	/* date time format */
				/* a - abbreviated weekday name */
				/* b - abbreviated month name */
				/* e - day of month */
				/* H - hour */
				/* M - minute */
				/* S - second */
				/* Y - Year */
				/* n - newline */

extern int	errno;
extern int	optind;
extern char	*optarg;

#ifdef __STDC__
extern void	usage(void);
extern char	*strrchr(char *, int);
extern char	*strcat(char *, char *);
extern int 	strlen(char *);
extern char	*flags(char *, int);
extern char	*remote(char *, FILE *);
#else
extern void	usage();
extern char	*strrchr();
extern char	*strcat();
extern int 	strlen();
extern char	*flags();
extern char	*remote();
#endif

char	*myopts[] = {
	"ro",
	"rw",
	"suid",
	"nosuid",
	NULL
};
char	*myname;		/* point to argv[0] */
char	mntflags[100];

/*
 * This is /usr/sbin/mount: the generic command that in turn
 * execs the appropriate /usr/lib/fs/{fstype}/mount.
 * The -F flag and argument are NOT passed.
 * If the usr file system is not mounted a duplicate copy
 * can be found in /sbin and this version execs the 
 * appropriate /etc/fs/{fstype}/mount
 *
 * If the -F fstype, special or directory are missing,
 * /etc/vfstab is searched to fill in the missing arguments.
 *
 * -V will print the built command on the stdout.
 * It isn't passed either.
 */
main(argc, argv)
	int	argc;
	char	*argv[];
{
	char	*special,		/* argument of special/resource */
		*mountp,		/* argument of mount directory */
		*fstype,		/* wherein the fstype name is filled */
		*newargv[ARGV_MAX],	/* arg list for specific command */
		*vfstab = VFSTAB,
		*farg = NULL, *Farg = NULL, *narg = NULL, *oarg = NULL;
	int	cflg, dflg, fflg, Fflg, nflg, oflg, pflg;
	int	rflg, vflg, Vflg, dashflg, questflg;
	int	ii, ij, ret, cc, err_flag = 0;
	struct stat	stbuf;
	struct vfstab	vget, vref;
	mode_t mode;
	FILE	*fd;

	(void)setlocale(LC_ALL, "");
	myname = strrchr(argv[0], '/');
	if (myname)
		myname++;
	else
		myname = argv[0];

	/* Process the args.  */
	cflg = dflg = fflg = Fflg = nflg = oflg =
	pflg = rflg = vflg = Vflg = dashflg = questflg = 0;
	while ((cc = getopt(argc, argv, "?cdf:F:n:o:prvV")) != -1)
		switch (cc) {
			case 'c':
				cflg++;
				break;
			case 'd':
				dflg++;
				break;
			case 'f':
				fflg++;
				farg = optarg;
				break;
			case 'F':
				Fflg++;
				Farg = optarg;
				break;
			case 'n':
				nflg++;
				narg = optarg;
				break; /* undocumented flag for rfs */
			case 'o':
				oflg++;
				oarg = optarg;
				break; /* fstype dependent options */
			case 'p':
				pflg++;
				break;
			case 'r':
				rflg++;
				break;
			case 'v':
				vflg++;
				break;
			case 'V':
				Vflg++;
				break;
			case '?':
				questflg++;
				break;
		}

	/* check for '-r' at end, for compatibility */
	if (strcmp(argv[argc-1], "-r") == 0) {
		rflg++;
		/* decrement so we don't consider as mount_point */
		argv[--argc] = NULL;
	}

	/* copy '--' to specific */
	if (strcmp(argv[optind-1], "--") == 0)
		dashflg++;

	/* option checking */
	/* more than two args not allowed */
	if (argc - optind > 2)
		usage();

	/* pv mututally exclusive */
	if (pflg && vflg) {
		fprintf(stderr, "-p and -v mutually exclusive\n", myname);
		usage();
	}

	/* dfF mututally exclusive */
	if (dflg + fflg + Fflg > 1) {
		fprintf(stderr, "%s: more than one FSType specified\n", myname);
		usage();
	}

	/* no arguments, only allow p,v,V or [F]? */
	if (optind == argc) {
		if (cflg || dflg || fflg || nflg || oflg || rflg)
			usage();

		if (Fflg && !questflg)
			usage();

		if (questflg) {
			if (Fflg) {
				newargv[2] = "-?";

#ifdef i386
				newargv[3] = NULL;
#endif
				doexec(Farg, newargv);
			}
			usage();
		}
	}

	if (questflg)
		usage();

	/* one or two args, allow any but p,v */
	if (optind != argc && (pflg || vflg)) {
		fprintf(stderr, "%s: cannot use -p and -v with arguments\n", myname);
		usage();
	}

	/* if only reporting mnttab, generic prints mnttab and exits */
	if (optind == argc) {
		if (Vflg) {
			printf("%s", myname);
			if (pflg)
				printf(" -p");
			if (vflg)
				printf(" -v");
			printf("\n");
			exit(0);
		}

		print_mnttab(vflg, pflg);
		exit(0);
	}

	/* Super users only beyond this point */
	/* but let ordinary users do -V */
	if (Vflg == 0 && getuid() != 0) {
		fprintf(stderr, "%s: permission denied\n", myname);
		exit(1);
	}

	/* get special and/or mount-point from arg(s) */
	special = argv[optind++];
	if (optind < argc)
		mountp = argv[optind++];
	else
		mountp = NULL;

	/* get fstype if one given */
	if (dflg)
		fstype = "rfs";
	else if (fflg) { 
		if ((strcmp(farg,"S51K")!=0) && (strcmp(farg, "S52K")!=0)) {
			fstype = farg;
		}
		else
			fstype = "s5";
		}
	else /* if (Fflg) */
		fstype = Farg;

	/* lookup only if we need to */
	if (fstype == NULL || oarg == NULL || special == NULL || mountp == NULL) {
		if ((fd = fopen(vfstab, "r")) == NULL) {
			if (fstype == NULL || special == NULL || mountp == NULL) {
				fprintf(stderr, "%s: cannot open vfstab\n", myname);
				exit(1);
			}
		}
		vfsnull(&vref);
		vref.vfs_special = special;
		vref.vfs_mountp = mountp;
		vref.vfs_fstype = fstype;

		/* get a vfstab entry matching mountp or special */
		ret = getvfsany(fd, &vget, &vref);

		/* if no entry and there was only one argument */
		/* then the argument could be the mount point */
		/* and not special as we thought earlier */
		if (ret == -1 && mountp == NULL) {
			rewind(fd);
			mountp = vref.vfs_mountp = special;
			special = vref.vfs_special = NULL;
			ret = getvfsany(fd, &vget, &vref);
		}

		fclose(fd);

		if (ret > 0)
			vfserror(ret);

		if (ret == 0) {
			if (fstype == NULL)
				fstype = vget.vfs_fstype;
			if (special == NULL)
				special = vget.vfs_special;
			if (mountp == NULL)
				mountp = vget.vfs_mountp;
			if (oflg == 0 && vget.vfs_mntopts) {
				oflg++;
				oarg = vget.vfs_mntopts;
			}
		} else if (special == NULL) {
			if ((ij =stat(mountp, &stbuf)) == -1) {
				fprintf(stderr, "%s: cannot stat %s\n", 
					myname, mountp);
				exit(2);
			}
			if (((mode = (stbuf.st_mode & S_IFMT)) == S_IFBLK)||
				(mode == S_IFCHR )) {
				fprintf(stderr, "%s: mount point cannot be determined\n", myname);
				exit(1);
			} else
				{
				fprintf(stderr, "%s: special cannot be determined\n", myname);
				exit(1);
			}

		} else if (fstype == NULL) {

#ifdef i386
			fstype = "s5";
		}
#else
			fprintf(stderr,
				"%s: FSType cannot be identified\n", myname);
			exit(1);
		}
#endif

	}

	if (strlen(fstype) > FSTYPE_MAX) {
		fprintf(stderr,
			"%s: FSType %s exceeds %d characters\n",
			myname, fstype, FSTYPE_MAX);
		exit(1);
	}
	if (*mountp == NULL) {
		fprintf(stderr, "%s: mount point cannot be determined\n", myname);
		exit(1);
	}
	if (*mountp != '/') {
		fprintf(stderr,
			"%s: mount-point %s is not an absolute pathname.\n",
			myname, mountp);
		exit(1);
	}
	if (stat(mountp, &stbuf) < 0) {
		if (errno == ENOENT || errno == ENOTDIR)
			fprintf(stderr,
				"%s: mount-point does not exist.\n",
				myname);
		else {
			fprintf(stderr,
				"%s: cannot stat mount-point.\n",
				myname);
			perror(myname);
		}
		exit(1);
	}

	/* create the new arg list, and end the list with a null pointer */
	ii = 2;
	if (cflg)
		newargv[ii++] = "-c";
	if (nflg) {
		newargv[ii++] = "-n";
		newargv[ii++] = narg;
	}
	if (oflg) {
		newargv[ii++] = "-o";
		newargv[ii++] = oarg;
	}
	if (rflg)
		newargv[ii++] = "-r";
	if (dashflg)
		newargv[ii++] = "--";
	newargv[ii++] = special;
	newargv[ii++] = mountp;
	newargv[ii] = NULL;

	if (Vflg) {
		printf("%s -F %s", myname, fstype);
		for (ii = 2; newargv[ii]; ii++)
			printf(" %s", newargv[ii]);
		printf("\n");
		exit(0);
	}

	doexec(fstype, newargv);
}

void
usage()
{
	fprintf(stderr,
		"Usage:\n%s [-v | -p]\n%s [-F FSType] [-V] [current_options] [-o specific_options] {special | mount_point}\n%s [-F FSType] [-V] [current_options] [-o specific_options] special mount_point\n",
		myname, myname, myname, myname);

	exit(1);
}

print_mnttab(vflg, pflg)
	int	vflg, pflg;
{
	FILE	*fd;
	FILE	*rfp;			/* this will be NULL if fopen fails */
	int	ret;
	char	time_buf[TIME_MAX];	/* array to hold date and time */
	char	*mnttab = MNTTAB;
	struct mnttab	mget;
	time_t	ltime;

	if ((fd = fopen(mnttab, "r")) == NULL) {
		fprintf(stderr, "%s: cannot open mnttab\n", myname);
		exit(1);
	}
	rfp = fopen(REMOTE, "r");
	while ((ret = getmntent(fd, &mget)) == 0)
		if (mget.mnt_special && mget.mnt_mountp && mget.mnt_fstype && mget.mnt_time) {
			ltime = atol(mget.mnt_time);
			cftime(time_buf, FORMAT, &ltime);
			if (pflg)
				printf("%s - %s %s - no %s\n",
					mget.mnt_special,
					mget.mnt_mountp,
					mget.mnt_fstype,
					mget.mnt_mntopts);
			else if (vflg) {
				char	line[LINE_MAX];

				printf("%s on %s type %s %s%s on %s",
					mget.mnt_special,
					mget.mnt_mountp,
					mget.mnt_fstype,
					flags(mget.mnt_mntopts, NEW),
					remote(mget.mnt_fstype, rfp),
					time_buf);
			} else
				printf("%s on %s %s%s on %s",
					mget.mnt_mountp,
					mget.mnt_special,
					flags(mget.mnt_mntopts, OLD),
					remote(mget.mnt_fstype, rfp),
					time_buf);
		}

	if (ret > 0)
		mnterror(ret);
}

char	*
flags(mntopts, flag)
	char	*mntopts;
	int	flag;
{
	char	*value;

	strcpy(mntflags, "");
	if (mntopts)
		while (*mntopts != '\0')  {
			switch (getsubopt(&mntopts, myopts, &value)) {
			case READONLY:
				if (flag == OLD)
					strcat(mntflags, "read only");
				else
					strcat(mntflags, "read-only");
				break;
			case READWRITE:
				strcat(mntflags, "read/write");
				break;
			case SUID:
				strcat(mntflags, "setuid");
				break;
			case NOSUID:
				strcat(mntflags, "nosuid");
				break;
			default:
				strcat(mntflags, value);
				break;
			}
	/* if mntopts still exist, then cat '/' separator to mntflags */
			if (*mntopts != '\0') {
				strcat(mntflags, "/");
			}
		}
	if (strlen(mntflags) > 0)
		return mntflags;
	else
		return	"read/write";
}

char	*
remote(fstype, rfp)
	char	*fstype;
	FILE	*rfp;
{
	char	buf[BUFSIZ];
	char	*fs;
	extern char *strtok();

	if (rfp == NULL || fstype == NULL || strlen(fstype) > FSTYPE_MAX)
		return	"";	/* not a remote */
	rewind(rfp);
	while (fgets(buf, sizeof(buf), rfp) != NULL) {
		fs = strtok(buf, " \t");
		if (strcmp (fstype,fs) == 0) 
			return	"/remote";	/* is a remote fs */
	}
	return	"";	/* not a remote */
}


vfserror(flag)
	int	flag;
{
	switch (flag) {
	case VFS_TOOLONG:
		fprintf(stderr, "%s: line in vfstab exceeds %d characters\n",
			myname, VFS_LINE_MAX-1);
		break;
	case VFS_TOOFEW:
		fprintf(stderr, "%s: line in vfstab has too few entries\n",
			myname);
		break;
	}
	exit(1);
}

mnterror(flag)
	int	flag;
{
	switch (flag) {
	case MNT_TOOLONG:
		fprintf(stderr, "%s: line in mnttab exceeds %d characters\n",
			myname, MNT_LINE_MAX-2);
		break;
	case MNT_TOOFEW:
		fprintf(stderr, "%s: line in mnttab has too few entries\n",
			myname);
		break;
	case MNT_TOOMANY:
		fprintf(stderr, "%s: line in mnttab has too many entries\n",
			myname);
		break;
	}
	exit(1);
}

doexec(fstype, newargv)
	char	*fstype, *newargv[];
{
	char	full_path[PATH_MAX];
	char	alter_path[PATH_MAX];
	char	*vfs_path = VFS_PATH;
	char	*alt_path = ALT_PATH;
	int	sem_file, nfile;

	/* build the full pathname of the fstype dependent command. */
	sprintf(full_path, "%s/%s/%s", vfs_path, fstype, myname);
	sprintf(alter_path, "%s/%s/%s", alt_path, fstype, myname);

	/* set the new argv[0] to the filename */
	newargv[1] = myname;

	if ((sem_file = creat(SEM_FILE,0600)) == -1 || lockf(sem_file,F_LOCK, 0L) <0 ) {
		fprintf(stderr,"mount: warning: cannot lock temp file <%s>\n",SEM_FILE);
	}

	/* Try to exec the fstype dependent portion of the mount. */
	/* See if the directory is there before trying to exec dependent */
	/* portion.  This is only useful for eliminating the '..mount: not found' */
	/* message when '/usr' is mounted */
	if (access(full_path,0)==0 ) {
		execv(full_path, &newargv[1]);
		if (errno == EACCES) {
			fprintf(stderr, "%s: cannot execute %s - permission denied\n", myname, full_path);
		}
		if (errno == ENOEXEC) {
			newargv[0] = "sh";
			newargv[1] = full_path;
			execv("/sbin/sh", &newargv[0]);
		}
	}
	execv(alter_path, &newargv[1]);
	if (errno == EACCES) {
		fprintf(stderr, "%s: cannot execute %s - permission denied\n", myname, alter_path);
		exit(1);
	}
	if (errno == ENOEXEC) {
		newargv[0] = "sh";
		newargv[1] = alter_path;
		execv("/sbin/sh", &newargv[0]);
	}
	fprintf(stderr, "%s: operation not applicable to FSType %s\n", myname, fstype);
	exit(1);
}
