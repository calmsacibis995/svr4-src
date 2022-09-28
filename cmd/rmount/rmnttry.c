/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)rmount:rmnttry.c	1.1.11.1"


/*
	rmnttry - attempt to perform queued mounts
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <nserve.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mnttab.h>

#define	TIMEOUT	180	/* time limit (seconds) for mount to complete */
#define RMNTTAB "/etc/rfs/rmnttab"
#define TMP "/etc/rfs/tmp"

char *cmd;
FILE *mp;	/* mnttab file pointer */
FILE *rp;	/* rmnttab file pointer */
char *fqn();	/* fully qualified names */
struct stat stbuf;
struct mnttab mnt[30], mtab;
int mnt_count;	/* number of entries in mnttab */

main(argc, argv)
int argc;
char **argv;
{
	struct mnttab  rtab;	/* rmnttab structure */
	int fails = 0;		/* number of failed mounts */
	int chg = 0;		/* change flag for rmnttab */
	int i, j;		/* indices for command line names */
	char **ulist;		/* list of unused resources */
	FILE *tmp;		/* temp file to write unchanged entries
				   in rmnttab */
	int rfd;		/* file descriptor to create rmnttab */

	cmd = argv[0];
	if (geteuid() !=  0) {
		fprintf(stderr, "%s: must be super-user\n", cmd);
		exit(2);
	}

	lock();		/* lock file for entire session */

	/* if RFS is not running, then delete the rmnttab table */
	if (rfs_up() != 0) {
		if ((rfd=creat(RMNTTAB, (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH))) < 0) {
			fprintf(stderr, "%s: cannot create %s\n",cmd,RMNTTAB);
			fails = 2;	
		}
		close(rfd);
		unlock();
		return fails;
	}

	fails = rd_rmnttab();
	switch (fails) {
		case 1:		/* rmnttab does not exist, exit quietly */
			unlock();
			return 0;
			break;
		case 2:		/* open or stat fails */
			unlock();
			return 2;
			break;
		case 0:
			break;
		default:
			unlock();
			fprintf(stderr, "%s: error in reading rmnttab\n", cmd);
			return fails;
			break;
	}

	if (!rd_mnttab()) {		/* keep quiet in case of error */
		unlock();
		return 2;
	}

	if ((tmp=fopen(TMP, "w")) == NULL) {
		fprintf(stderr,"%s: cannot open temp file to write\n", cmd);
		return 2;
	}

	/* scan through rmnttab, trying either each entry or selected ones */

	while (getmntent(rp, &rtab) == 0) 
		if (argc == 1 || request(rtab.mnt_special, argv)) {

			/*  attempt to mount this resource but first see if it
			    or its mount point is already in /etc/mnttab  */

			for (i=0; i < mnt_count; i++) {
				if (strcmp(mnt[i].mnt_special,rtab.mnt_special) == 0) {
					printf("%s: warning: %s already mounted on %s\n",
						cmd, rtab.mnt_special, 
						mnt[i].mnt_mountp);
					chg++;
					break;
				}
			    	else if (strcmp(mnt[i].mnt_mountp, rtab.mnt_mountp) == 0) {
					printf("%s: warning: mount point %s used by %s\n",
						cmd, rtab.mnt_mountp, 
						mnt[i].mnt_special);
					chg++;
					break;
				}
			}  /* end of for */

			if (i == mnt_count)	/* it's not in mnttab */
				/* try the mount */
				if (!trymount(rtab.mnt_special, rtab.mnt_mountp, rtab.mnt_fstype, rtab.mnt_mntopts)) {
					chg++;
				}
				else {
				/* mark fail for proper return code */
					fails++;
					fprintf(tmp, "%s\t%s\t%s\t%s\t%s\n",
						rtab.mnt_special,
						rtab.mnt_mountp,
						rtab.mnt_fstype,
						rtab.mnt_mntopts,
						rtab.mnt_time);
				}
		}
		else {	/* resource in rmnttab but not requested to mount */
			fails++;
			fprintf(tmp, "%s\t%s\t%s\t%s\t%s\n",
				rtab.mnt_special,
				rtab.mnt_mountp,
				rtab.mnt_fstype,
				rtab.mnt_mntopts,
				rtab.mnt_time);
		}

	/* check for unused command line resources */
	for (ulist = &argv[1], i=1, j=0; i < argc; ++i)
		if (argv[i] && *argv[i])
			ulist[j++] = argv[i];
	if (j) {
		fprintf(stderr, "%s: resources not queued:", cmd);
		/* if none of the resources are queued, return 1 */
		if ( (i-1) == j ) 
			fails++;
		for (i=0; i < j; ++i)
			fprintf(stderr, " %s", ulist[i]);
		fputc('\n', stderr);
	}

	/* if rmnttab was not altered, don't write it out */
	if (!chg) {
		fclose(rp);
		fclose(tmp);
		unlink(TMP);
		unlock();
		return fails?1:0;
	}

	if (fails)
		fails = 1;	/* set proper return code */

	/* write rmnttab */

	fclose(rp);
	fclose(tmp);
	unlink(RMNTTAB);
	link(TMP, RMNTTAB);
	chmod(RMNTTAB, (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH));
	chown(RMNTTAB, stbuf.st_uid, stbuf.st_gid);
	unlink(TMP);
	unlock();
	return fails;
}


/*
	request - if resource is a command line argument, return 1
		and set the that command line argument to an  empty string.
		otherwise, return 0
*/

request(resource, av)
char *resource, **av;
{
	char buf[MAXDNAME];

	while (*++av) {
		fqn(*av, buf);
		if (strcmp(resource, buf) == 0) {
			**av = '\0';
			return 1;
		}
	}
	return 0;
}


/*
	rd_mnttab - read the mount table
	return:
		0 if error
		1 if correct
*/

#define MNTTAB	"/etc/mnttab"


rd_mnttab()
{
	struct stat stbuf;
	int i;

	if ((mp = fopen(MNTTAB, "r")) == NULL) {
		fprintf(stderr, "%s: cannot open %s\n", cmd, MNTTAB);
		return 0;
	}
	if (stat(MNTTAB, &stbuf) < 0) {
		fprintf(stderr, "%s: cannot stat %s\n", cmd, MNTTAB);
		return 0;
	}

	for (i=0; getmntent(mp, &mtab) == 0; i++) {
		if (!(mnt[i].mnt_special=(char *)malloc(strlen(mtab.mnt_special)+1))) {
			fprintf(stderr, "%s: cannot allocate space for mnt_special\n", cmd);
			fclose(mp);
			return 0;
	}
		strcpy(mnt[i].mnt_special, mtab.mnt_special);
		if (!(mnt[i].mnt_mountp=(char *)malloc(strlen(mtab.mnt_mountp)+1))) {
			fprintf(stderr, "%s: cannot allocate space for mnt_mountp\n", cmd);
			fclose(mp);
			return 0;
		}
		strcpy(mnt[i].mnt_mountp, mtab.mnt_mountp);
	}
	mnt_count = i;
	fclose(mp);
	return 1;
}
/*
	trymount - perform an /sbin/mount; return its exit status.

	TIMEOUT is the maximum time in seconds for /sbin/mount to complete
*/

pid_t pid;		/* PID of forked /sbin/mount. Kill it when the time's up */
char *spec, *dir, *fsys, *op;

trymount(special, mountp, fs, opts)
char *special, *mountp, *fs, *opts;
{
	int status;
	pid_t w;
	void (*istat)(), (*qstat)(), (*astat)();
	void killpid();

	if (spec = (char *)malloc(strlen(special)+1))
		strcpy (spec, special);
	else	fprintf(stderr, "%s: malloc for spec failed\n");
	if (dir = (char *)malloc(strlen(mountp)+1))
		strcpy (dir, mountp);
	else	fprintf(stderr, "%s: malloc for dir failed\n");
	if (fsys = (char *)malloc(strlen(fs)+1))
		strcpy (fsys, fs);
	else	fprintf(stderr, "%s: malloc for fsys failed\n");
	if (op = (char *)malloc(strlen(opts)+1))
		strcpy (op, opts);
	else	fprintf(stderr, "%s: malloc for op failed\n");
	astat = signal(SIGALRM, killpid);
	if ((pid = fork()) == (pid_t)0) {
		int fd;

		if ((fd = open("/dev/null", O_WRONLY)) < 0)
			fprintf(stderr, "%s: Can't open /dev/null\n", cmd);
		else {
			close(1);
			fcntl(fd, F_DUPFD, 1);		/* redirect stdout */
			close(2);
			fcntl(fd, F_DUPFD, 2);		/* redirect stderr */
			close(fd);
			/* perform the mount */
			(void) execl("/sbin/mount", "mount", "-F", fs, "-o", opts, special, mountp,(char *)0);
		}
		_exit(127);
	}
	else {
		alarm(TIMEOUT);		/* set mount time limit */
		(void) signal(SIGALRM, killpid);
	}

	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
	while((w = wait(&status)) != pid && w != (pid_t)-1)
		;
	alarm(0);
	(void) signal(SIGALRM, astat);
	(void) signal(SIGINT, istat);
	(void) signal(SIGQUIT, qstat);
	return((w == -1)? w: status);
}


/*
	killpid - kill the /sbin/mount command when it's been running too long
*/

void
killpid()
{
	kill(pid, SIGKILL);
	fprintf(stderr, "%s: \"/sbin/mount -F %s -o %s %s %s\" timed out.\n",
		cmd, fsys, op, spec, dir );
}
