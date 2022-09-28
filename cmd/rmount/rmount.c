/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)rmount:rmount.c	1.1.8.1"


#include <sys/types.h>
#include <nserve.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mnttab.h>

#define RMNTTAB	"/etc/rfs/rmnttab"	/* rmount table */

char *cmd;

main(argc, argv)
int argc;
char **argv;
{
	extern char *optarg;
	extern int optind;
	int c, err = 0; 
	int dflg=0, rflg = 0;		/* command line flags */
	int Fflg=0, oflg = 0, cflg=0;
	char *fstype = NULL;
	char opt_buf[50];
	static char usage[] = "usage: %s [-F fstype] [-drc] [-o options] resource directory\n";

	cmd = argv[0];
	if (argc == 1) {		/* dump contents of rmnttab */
		return rshow();
	}
	else if (geteuid() != 0) {
		fprintf(stderr, "%s: must be super-user\n", cmd);
		return 1;
	}
	opt_buf[0] = NULL;
	while ((c = getopt(argc, argv, "F:drco:")) != -1)
		switch (c) {
		case 'F':
			if (Fflg) 
				err = 1;
			else {
				fstype = optarg;
				Fflg = 1;
				dflg = 1;
			}
			break;
		case 'd':
			if (Fflg|dflg)
				err = 1;
			else {
				fstype = "rfs";
				Fflg = 1;
				dflg = 1;
			}
			break;
		case 'c':
			if (cflg)
				err = 1;
			else {
				cflg = 1;
				if (opt_buf[0]) 
					strcat(opt_buf,",nocaching");
				else
					strcpy(opt_buf,"nocaching");
			}
			break;
		case 'r':
			if (rflg)
				err = 1;
			else {
				rflg = 1;
				if (opt_buf[0]) 
					strcat(opt_buf,",ro");
				else
					strcpy(opt_buf,"ro");
			}
			break;
		case 'o':
			if (oflg)
				err = 1;
			else {
				oflg = 1;
				if (opt_buf[0]) {
					strcat(opt_buf,",");
					strcat(opt_buf,optarg);
				}
				else
					strcpy(opt_buf,optarg);
			}
			break;
		case '?':
			err = 1;
			break;
		}

	if (err || optind+2 != argc) {
		fprintf(stderr, usage, cmd);
		return 1;
	}
	else if (!fstype) {
		fprintf(stderr, "%s: file system type not specified\n", cmd);
		fprintf(stderr, usage, cmd);
		return 1;
		}
		else  {
			if (!opt_buf[0])
				strcpy(opt_buf, "rw");
			return renter(argv[optind], argv[optind+1], opt_buf, fstype);
		}
}


/*
	renter - enter a request onto rmnttab
*/

extern char *cmd;
FILE *rp;
struct stat stbuf;

renter(resource, dir, opt, fs)
char *resource, *dir;
char *opt, *fs;
{
	char *fqn();	/* convert a name into a fully qualified domain.name */
	struct mnttab mtab;
	char fres[MAXDNAME], fdev[MAXDNAME];	/* fully qualified names */
	int err = 0;		/* return value */
	int ret;

	lock();

	fqn(resource, fres);	/* generate a fully qualified name */
	/* look for conflicts between rmnttab entries and the new one */

	ret=rd_rmnttab();
	switch(ret) {
		case 1:   /* rmnttab does not exist */
			if (!wr_rmnttab(fres, dir, opt, fs)) {
				unlock();
				return 1;
			}
			else {
				unlock();
				return 0;
			}
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

	while (getmntent(rp, &mtab) == 0) {
		if (strcmp(fqn(mtab.mnt_special, fdev), fres) == 0) {
			unlock();
			fprintf(stderr,"%s: %s already queued for mount\n",
				cmd, resource);
			fclose(rp);
			return 1;
		}
	}
	fclose(rp);

	/* write out the rmnttab file */
	if (!wr_rmnttab(fres, dir, opt, fs))
		err = 1;
	unlock();
	return err;
}


/*
	rshow - show pending mounts (dump rmnttab)
*/

rshow()
{
	struct mnttab mtab;
	char ptime[40];
	FILE *rfp;
	long ltime;
	int ret;

	if ((rfp=fopen (RMNTTAB, "r")) == NULL) 
		return 0;
	while ((ret=getmntent(rfp, &mtab)) == 0) {
		if (mtab.mnt_special && mtab.mnt_mountp && mtab.mnt_fstype && mtab.mnt_time) {
			ltime = atol(mtab.mnt_time);
			cftime(ptime, (char *)0, &ltime);
			printf("%s\t%s\t%s\t%s\t%s\n",
				mtab.mnt_special,
				mtab.mnt_mountp,
				mtab.mnt_fstype,
				mtab.mnt_mntopts,
				ptime);
		}
	}
	if (ret > 0) {
		fprintf (stderr, "%s: error in rmnttab\n", cmd);
		return 1;
	}
	fclose(rfp);
	return 0;
}
