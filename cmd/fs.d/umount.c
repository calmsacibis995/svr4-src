/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)fs.cmds:umount.c	1.17.3.1"

#include	<stdio.h>
#include	<limits.h>
#include	<signal.h>
#include	<unistd.h>
#include	<sys/mnttab.h>
#include	<sys/errno.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#ifdef RFSUMOUNTHACK
#include	<sys/rf_sys.h>
#endif

#define	FS_PATH		"/usr/lib/fs"
#define ALT_PATH	"/etc/fs"
#define SEM_FILE	"/etc/.mnt.lock"
#define	FULLPATH_MAX	32
#define	FSTYPE_MAX	8
#define	ARGV_MAX	16

#define	questFLAG	0x01
#define	dFLAG		0x02
#define	oFLAG		0x08
#define	VFLAG		0x10
#define	dashFLAG	0x20

#ifdef __STDC__
extern char	*strrchr(char *, int);
#else
extern char	*strrchr();
#endif
extern void	rpterr(), usage(), mnterror();

extern int	errno;
extern	char	*optarg;	/* used by getopt */
extern	int	optind, opterr;

char	*myname;
char	fs_path[] = FS_PATH;
char	alt_path[] = ALT_PATH;
char	mnttab[] = MNTTAB;
char	temp[] = "/etc/mnttab.temp";

main(argc, argv)
	int	argc;
	char	**argv;
{
	FILE	*frp, *fwp;
	int 	ii, cc, ret, cmd_flags = 0, err_flag = 0, ein_flag = 0;
	int	no_mnttab = 0;
	int	sem_file = 0;
	char	*oarg = NULL;
	char	full_path[FULLPATH_MAX];
	char	alter_path[FULLPATH_MAX];
	char	mgetsave[MNT_LINE_MAX], *mp;
	char	*newargv[ARGV_MAX];
	struct mnttab	mget;
	struct mnttab	mref;
#ifdef RFSUMOUNTHACK
	int	cap;
	char	cappn[MAXPATHLEN];

	int	cap_unmount();
	void	cap_exit();
#endif

	myname = strrchr(argv[0], '/');
	if (myname)
		myname++;
	else
		myname = argv[0];

	/*
	 * Process the args.
	 * Usage:
	 *	umount [-V] [-d] [-o options] {special | mount_point}
	 *
	 * "-d" for compatibility
	 */
	while ((cc = getopt(argc, argv, "o:V?d")) != -1)
		switch (cc) {
		case '?':
			if (cmd_flags & questFLAG)
				err_flag++;
			else {
				cmd_flags |= questFLAG;
				err_flag++;
			}
			break;
		case 'd':
			if (cmd_flags & dFLAG)
				err_flag++;
			else
				cmd_flags |= dFLAG;
			break;
		case 'o':
			if (cmd_flags & (oFLAG))
				err_flag++;
			else {
				cmd_flags |= (oFLAG);
				oarg = optarg;
			}
			break;
		case 'V':
			if (cmd_flags & VFLAG)
				err_flag++;
			else
				cmd_flags |= VFLAG;
			break;
		default:
			err_flag++;
			break;
		}

	/* copy '--' to specific */
	if (strcmp(argv[optind-1], "--") == 0)
		cmd_flags |= dashFLAG;

	/* option checking */
		/* more than one arg not allowed */
	if (argc - optind > 1 ||
		/* no arguments, only allow ? */
	    (optind == argc && (cc = (cmd_flags & questFLAG)) != questFLAG ) ||
		/* one arg, allow d,o,V */
	    (optind != argc && (cmd_flags & ~(dFLAG|oFLAG|VFLAG))))
		err_flag++;

	if (err_flag) {
		usage();
		exit(2);
	}

	/* Super users only beyond this point */
	/* but let ordinary users do -V */
	if ((cmd_flags & VFLAG) == 0 && getuid() != 0) {
		fprintf(stderr, "%s: permission denied\n", myname);
		exit(1);
	}

	/* get mount-point */
	if ((frp = fopen(mnttab, "r")) == NULL) {
		fprintf(stderr, "%s: cannot open mnttab\n", myname);
		mget.mnt_special = argv[optind];
		no_mnttab++;
	}
else {
	mntnull(&mref);
	mref.mnt_mountp = argv[optind];
	if ((ret = getmntany(frp, &mget, &mref)) == -1) {
		mref.mnt_special = mref.mnt_mountp;
		mref.mnt_mountp = NULL;
		rewind(frp);
		if ((ret = getmntany(frp, &mget, &mref)) == -1) {
			fprintf(stderr, "%s: warning: %s not in mnttab\n", myname, mref.mnt_special);
			mntnull(&mget);
			mget.mnt_special = mget.mnt_mountp = mref.mnt_special;
		}
	}
	fclose(frp);

	if (ret > 0)
		mnterror(ret);

}
#ifdef RFSUMOUNTHACK
	/*
	 * There is a general problem with unmounting unreachable
	 * pathnames.  This hack is a restricted solution for
	 * RFS.  A later release will address the general problem.
	 */
	if ((cap = rfsys(RF_GETCAP, mget.mnt_mountp, cappn)) != -1){
		/* RFS knows how to deal with the pathname. */

		if (!(errno = cap_unmount(cap)))
			cap_exit(cappn, myname);	/* no return*/
		printf "rfs umounthack \n");
		rpterr(argv[optind]);
		exit(1);
	}
	
	/* We assume we were barking up the wrong tree calling rfsys. */
	errno = 0;
#endif

	/* try to exec the dependent portion */
	if ((mget.mnt_fstype != NULL) || (cmd_flags & VFLAG)) {
		if (strlen(mget.mnt_fstype) > FSTYPE_MAX) {
			fprintf(stderr, "%s: FSType %s exceeds %d characters\n", myname, mget.mnt_fstype, FSTYPE_MAX);
			exit(1);
		}

		/* build the full pathname of the fstype dependent command. */
		sprintf(full_path, "%s/%s/%s", fs_path, mget.mnt_fstype, myname);
		sprintf(alter_path, "%s/%s/%s", alt_path, mget.mnt_fstype, myname);

		/* create the new arg list, and end the list with a null pointer */
		ii = 2;
		if (cmd_flags & oFLAG) {
			newargv[ii++] = "-o";
			newargv[ii++] = oarg;
		}
		if (cmd_flags & dashFLAG) {
			newargv[ii++] = "--";
		}
		newargv[ii++] = argv[optind];
		newargv[ii] = NULL;

		/* set the new argv[0] to the filename */
		newargv[1] = myname;

		if (cmd_flags & VFLAG) {
			printf("%s", myname);
			for (ii = 2; newargv[ii]; ii++)
				printf(" %s", newargv[ii]);
			printf("\n");
			exit(0);
		}

		/* Try to exec the fstype dependent umount. */
		execv(full_path, &newargv[1]);
		if (errno == ENOEXEC) {
			newargv[0] = "sh";
			newargv[1] = full_path;
			execv("/sbin/sh", &newargv[0]);
		}
		newargv[1] = myname;
		execv(alter_path, &newargv[1]);
		if (errno == ENOEXEC) {
			newargv[0] = "sh";
			newargv[1] = alter_path;
			execv("/sbin/sh", &newargv[0]);
		}
		/* exec failed */
		if (errno != ENOENT) {
			fprintf(stderr, "umount: cannot execute %s\n", full_path);
			exit(1);
		}
	}

	if (cmd_flags & VFLAG) {
		printf("%s", myname);
		for (ii = 2; newargv[ii]; ii++)
			printf(" %s", newargv[ii]);
		printf("\n");
		exit(0);
	}

	/* don't use -o with generic */
	if (cmd_flags & oFLAG) {
		fprintf(stderr, "%s: %s specific umount does not exist\n", myname, mget.mnt_fstype );
		fprintf(stderr, "%s: -o suboptions will be ignored\n", myname);
	}

	/*
	 * Try to umount the mountpoint.
	 * If that fails, try the corresponding special.
	 * (This ordering is necessary for nfs umounts.)
	 * (for remote resources:  if the first umount returns EBUSY
	 * don't call umount again - umount() with a resource name
	 * will return a misleading error to the user
	 */
	if (((ret = umount(mget.mnt_mountp)) < 0) && (errno != EBUSY)) {
		ret = umount(mget.mnt_special);
	}

	if (ret < 0) {
		rpterr(argv[optind]);
		if (errno != EINVAL) {
			exit(1);
		}
		else {
			ein_flag = 1;
		}
	}
	/* if the mnttab doesn't exist exit after the umount */
		if (no_mnttab) {
			exit(1);
		}
	/* remove entry from mnttab */

	/* save the mget entry */
	mp = mgetsave;
	strcpy(mp, mget.mnt_special);
	mget.mnt_special = mp;

	if ((sem_file = creat(SEM_FILE,0600)) == -1 || lockf(sem_file,F_LOCK, 0L) <0 ) {
		fprintf(stderr,"umount: warning: cannot lock temp file <%s>\n",SEM_FILE);
	}

	/* remove entry from mnttab */
	if ((frp = fopen(mnttab, "r+")) == NULL) {
			fprintf(stderr, "%s: cannot open mnttab\n", myname);
			exit(1);
	}
	/*
	 * Lock the file to prevent many unmounts at once.
	 * This may sleep for the lock to be freed.
	 * This is done to ensure integrity of the mnttab.
	 */
	if (lockf(fileno(frp), F_LOCK, 0L) < 0) {
		fprintf(stderr, "%s: cannot lock mnttab\n", myname);
		perror(myname);
		exit (1);
	}

	/* check every entry for validity before we umount */
	while ((ret = getmntent(frp, &mref)) == 0)
		;
	if (ret > 0)
		mnterror(ret);
	rewind(frp);

	if ((fwp = fopen(temp, "w")) == NULL) {
		fprintf(stderr, "%s: cannot open %s for writing\n", myname, temp);
		exit(1);
	}
	signal(SIGHUP,  SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGINT,  SIG_IGN);

	/* for each entry ... */
	while ((ret = getmntent(frp, &mref)) != -1)  {
		/* if it's a valid entry and not the one we got above ... */
		if ( ret == 0 && (strcmp(mgetsave, mref.mnt_special) != 0))
			/* put it out */
			putmntent(fwp, &mref);
	}

	fclose(fwp);
	rename(temp, mnttab);
	fclose(frp);

/* if ein_flag is set, then this was not mounted - exit with nonzero ret code */
	if (ein_flag )
		exit(1);
	else  
		exit(0);
/*NOTREACHED*/
}

#ifdef RFSUMOUNTHACK

#define MAXSUBCAP 256

/* Try to unmount using the rfsys(RF_UNMOUNT, cap) kludge */
int
cap_unmount(cap)
	int	cap;	/* capability for a VFS */
{
	int subcap[MAXSUBCAP];
	int nsubcap;
	int snx;
	int error = 0;

	/* get capabilities for submounts */
	if ((nsubcap = rfsys(RF_SUBMNTS, cap, MAXSUBCAP, (caddr_t)subcap))
	  == -1) {
		return errno;
	}

	/* unmount submounts */
	for (snx = 0; snx < nsubcap; snx++) {
		if ((error = cap_unmount(subcap[snx])) == -1) {
			for (snx = 0; snx < nsubcap; snx++) {
				(void)rfsys(RF_PUTCAP, subcap[snx]);
			}
			return error;
		}
	}

	/* unmount this mount */
	if (rfsys(RF_UNMOUNT, cap) == -1) {
		error = errno;
	}
	return error;
}
#endif

void
rpterr(sp)
	char	*sp;
{
	switch (errno) {
	case EPERM:
		fprintf(stderr,"%s: permission denied\n", myname);
		break;
	case ENXIO:
		fprintf(stderr,"%s: %s no device\n", myname, sp);
		break;
	case ENOENT:
		fprintf(stderr,"%s: %s no such file or directory\n", myname, sp);
		break;
	case EINVAL:
		fprintf(stderr,"%s: %s not mounted\n", myname, sp);
		break;
	case EBUSY:
		fprintf(stderr,"%s: %s busy\n", myname, sp);
		break;
	case ENOTBLK:
		fprintf(stderr,"%s: %s block device required\n", myname, sp);
		break;
	case ECOMM:
		fprintf(stderr,"%s: warning: broken link detected\n", myname);
		break;
	default:
		perror(myname);
		fprintf(stderr,"%s: cannot unmount %s\n", myname, sp);
	}
}

void
usage()
{
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "%s [-V] ", myname);
	fprintf(stderr, "[-o specific_options] {special | mount-point}\n");
}

void
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
	default:
		break;
	}
	exit(1);
}

#ifdef RFSUMOUNTHACK
/*
 * Clean out all mnttab entries with prefix mp for the mountpoint.
 * No return.
 */
void
cap_exit(mp, myname)
	char *mp, *myname;
{
	struct mnttab mget;
	FILE *frp, *fwp;
	int ret;

	if ((frp = fopen(mnttab, "r+")) == NULL) {
		fprintf(stderr, "%s: cannot open mnttab\n", myname);
		exit(1);
	}
	/*
	 * Lock the file to prevent many unmounts at once.
	 * This may sleep for the lock to be freed.
	 * This is done to ensure integrity of the mnttab.
	 */
	if (lockf(fileno(frp), F_LOCK, 0L) < 0) {
		fprintf(stderr, "%s: cannot lock mnttab\n", myname);
		perror(myname);
		exit (1);
	}

	/* check every entry for validity before we umount */
	while ((ret = getmntent(frp, &mget)) == 0)
		;
	if (ret > 0)
		mnterror(ret);
	rewind(frp);

	if ((fwp = fopen(temp, "w")) == NULL) {
		fprintf(stderr, "%s: cannot open %s for writing\n",
		  myname, temp);
		exit(1);
	}

	signal(SIGHUP,  SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGINT,  SIG_IGN);

	/* for each entry ... */
	while ((ret = getmntent(frp, &mget)) != -1)
		if (ret == 0 && !sprefix(mp, mget.mnt_mountp))
			putmntent(fwp, &mget);

	fclose(fwp);
	rename(temp, mnttab);
	fclose(frp);

	exit(0);
}

/*
 * Returns 1 if s1 is a prefix of s2, 0 otherwise.
 */
int
sprefix(s1, s2)
	char	*s1, *s2;
{
	size_t	l1 = strlen(s1);
	size_t	l2 = strlen(s2);

	return l1 > l2 ? 0 : !strncmp(s1, s2, l1);
}
#endif
