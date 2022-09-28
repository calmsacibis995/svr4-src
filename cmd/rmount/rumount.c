/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)rmount:rumount.c	1.1.6.1"


/*
	rumount -- dequeue a mount request

	usage: umount resource ...

	exit codes:
		0: 1 or more resources were removed from the queue
		1: no resources were removed from the queue
		2: syntax or functional error
*/

#include <sys/types.h>
#include <nserve.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mnttab.h>

#define MASK (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH)
#define TMPRMNT "/etc/rfs/tmprmnt"
#define RMNTTAB "/etc/rfs/rmnttab"

char *cmd;
FILE *rp;
struct stat stbuf;

main(argc, argv)
int argc;
char **argv;
{
	int chg = 0;			/* flag if rmnttab is altered */
	register int i;			/* argv index */
	char *fqn(), fdev[MAXDNAME], fres[MAXDNAME];	/* fully qualified names */
	struct mnttab mtab;
	struct stat stbuf;
	FILE *tfp;
	int ret;
	int tflag=1;			/* the flag to write on temp file */

	cmd = argv[0];			/* set command name for diagnostics */

	if (geteuid() != 0) {
		fprintf(stderr, "%s: must be super-user\n", cmd);
		return 2;
	}

	if (argc < 2) {
		fprintf(stderr, "usage: %s resource ...\n", cmd);
		return 2;
	}

	lock();
	ret = rd_rmnttab();
	switch(ret) {
		case 1:   /* rmnttab does not exist */
			unlock();
			fprintf(stderr, "%s: resources not queued:", cmd);
			for (i=1; i < argc; ++i)
				if (*argv[i])
					fprintf(stderr, " %s", argv[i]);
			fputc('\n', stderr);
			return 1;
			break;
		case 2:
			unlock();
			return 2;
			break;
		case 0:
			break;
		default:
			unlock();
			fprintf(stderr, "%s: error in reading rmnttab\n", cmd);
			return 0;
			break;
	}

	if ((tfp = fopen(TMPRMNT, "w")) == NULL) {
		unlock();
		fprintf(stderr, "%s: cannot creat temp rmnttab\n", cmd);
		return 2;
	}

	/* run through the table, looking for the entries */
	while (getmntent(rp, &mtab) == 0) {
		fqn(mtab.mnt_special, fdev);
		for (i = 1; i < argc; i++)
			if (*argv[i] && !strcmp(fdev, fqn(argv[i], fres))) {
				*argv[i] = '\0';
				chg++;
				tflag=0;
			}
		if (tflag) 
			fprintf(tfp, "%s\t%s\t%s\t%s\t%s\n",
				mtab.mnt_special, mtab.mnt_mountp,
				mtab.mnt_fstype, mtab.mnt_mntopts,
				mtab.mnt_time);
		tflag = 1;
	}


	/* some resources may be not queued. If so, list them out */
	if (chg != argc - 1) {
		fprintf(stderr, "%s: resources not queued:", cmd);
		for (i=1; i < argc; ++i)
			if (*argv[i])
				fprintf(stderr, " %s", argv[i]);
		fputc('\n', stderr);
	}
	fclose(tfp);
	fclose(rp);

	/* if all resources are not queued, don't rewrite the file */
	if (!chg) {
		unlink(TMPRMNT);
		unlock();
		return 1;
	}
		/* write rmnttab */
	else {
		unlink(RMNTTAB);
		link(TMPRMNT, RMNTTAB);
		chmod (RMNTTAB, MASK);
		chown (RMNTTAB, stbuf.st_uid, stbuf.st_gid);
		unlink (TMPRMNT);
		unlock();
		return 0;
	}
}
