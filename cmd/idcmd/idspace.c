/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)idcmd:idspace.c	1.3.1.1"
/*
 * Idspace.c investigate free space in /, /usr, and /tmp
 *
 * Usage: idspace checks /, /usr (if it is a separate filesystem), and
 *        /tmp (if it is a separate filesystem). It uses the required
 *        space for a kernel reconfiguration as default as follows:
 *
 *	"idspace" - Check for / or /stand= (sizeof)unix + 400 blk & 100 inodes, and
 *                         /usr = 400 blocks, 100 inodes (if /usr is a filesys)
 *                         /tmp = 400 blocks, 100 inodes (if /tmp is a filesys)
 *
 *      "idspace -r 3000" - Check for / = 3000 blocks and 100 inodes.
 *      "idspace -u 4000 -i 300" - Check for /usr = 4000 blocks and 300 inodes.
 *
 * exit 0 - success
 *	1 - command syntax error, or needed file does not exist.
 *	2 - file system has insufficient space or inodes.
 *	3 - requested file system does not exist.
 *
 */

#include <stdio.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <sys/mnttab.h>
#include <ustat.h>

/* error messages */
#define USAGE		"Usage:  idspace [-u blocks | -t blocks | -r blocks] [-i inodes]"

#define NEEDBLK	400
#define NEEDINO	100
#define REMOTE_FST_1	"rfs"
#define REMOTE_FST_2	"nfs"

char fbuf[512];
char errbuf[200];
int debug;		/* debug flag */

char iflag=0;		/* over ride default count */
char rflag=0;		/* check root file system only */
char uflag=0;		/* check /usr file system only */
char tflag=0;		/* check /tmp file system only */
char onlyflag=0;	/* check one system only; u, t, r used */

long rootblk;
long usrblk = NEEDBLK;
long tmpblk = NEEDBLK;
long inodecnt = NEEDINO;
int usrfound=0;
int tmpfound=0;
int rootfound=0;

struct stat pstat;
extern void exit();
extern char *optarg;
extern int optind;
extern int errno;

main(argc, argv)
int argc;
char *argv[];
{
	register int ret_i, ret_j, m;
	FILE *fi;
	struct statvfs statbuf;
	struct mnttab mget;
	char	 dev[40];

	while ((m = getopt(argc, argv, "?#r:u:t:i:")) != EOF)
		switch (m) {
		case 'i':
			iflag++; 
			inodecnt=atol(optarg);
			break;
		case 'r':
			rflag++; 
			onlyflag++;
			rootblk=atol(optarg);
			break;
		case 'u':
			uflag++; 
			onlyflag++;
			usrblk=atol(optarg);
			break;
		case 't':
			tflag++; 
			onlyflag++;
			tmpblk=atol(optarg);
			break;
		case '#':
			debug++;
			break;
		case '?':
			fprintf(stderr, "%s\n", USAGE);
			exit(1);
		}

	if (onlyflag > 1){
		fprintf(stderr,"only one of -r, -u, -t options at a time.\n"); 
		error(USAGE, 1);
	}

	if (!onlyflag){
		if(stat("/stand/unix", &pstat))
			error("Can't obtain size of /stand/unix.", 1);
		rootblk=pstat.st_size/512 + NEEDBLK;	/* free space needed to hold new unix */
		if (debug)
			fprintf (stderr,"root free space needed (/stand/unix + %d) = %ld blocks\n", NEEDBLK, rootblk);

	}
	if ((fi = fopen(MNTTAB, "r")) == NULL) {
		sprintf(errbuf,"cannot open %s",MNTTAB);
		fclose(fi);
		error(errbuf,1);
	}

	while ((ret_i = getmntent(fi, &mget)) == 0) {
		if ((strcmp(mget.mnt_fstype, REMOTE_FST_1) == 0) ||
		   (strcmp(mget.mnt_fstype, REMOTE_FST_2) == 0))
			continue;
		if ((ret_j = statvfs(mget.mnt_mountp, &statbuf)) != 0) {
			sprintf(errbuf,"cannot statvfs %s\n", mget.mnt_mountp);
			error(errbuf,0);
			continue;
		}
		checkit(&mget, &statbuf);
	}

	if (rflag)	/* This case can't happen */
		if (!rootfound)
			error("Can not find / file system",3);
	if (uflag)
		if (!usrfound)
			error("Can not find /usr file system",3);
	if (tflag)
		if (!tmpfound)
			error("Can not find /tmp file system",3);
	exit(0);
}

checkit(mountb, sbuf)
struct mnttab *mountb;
struct statvfs *sbuf;
{
	int physblks;

	physblks = sbuf->f_frsize/512;
	if (debug)
		fprintf(stderr, "block size factor is %d\n",physblks);

	if (!onlyflag || rflag){
		if (!strcmp(mountb->mnt_mountp, "/")){
			rootfound++;
			if (debug)
				fprintf (stderr,"checking / (root) free space.\n");
			if(((sbuf->f_bfree * physblks) < rootblk) ||
				sbuf->f_ffree < inodecnt)
				nospace("/ (root)", rootblk, inodecnt);
		}
	}
	if (!onlyflag || uflag){
		if (!strcmp(mountb->mnt_mountp, "/usr")){
			usrfound++;
			if (debug)
				fprintf (stderr,"checking /usr free space.\n");
			if(((sbuf->f_bfree * physblks) < usrblk) ||
				sbuf->f_ffree < inodecnt)
				nospace("/usr", usrblk, inodecnt);
		}
	}
	if (!onlyflag || tflag){
		if (!strcmp(mountb->mnt_mountp, "/tmp")){
			tmpfound++;
			if (debug)
				fprintf (stderr,"checking /tmp free space.\n");
			if(((sbuf->f_bfree * physblks) < tmpblk) ||
				sbuf->f_ffree < inodecnt)
				nospace("/tmp", tmpblk, inodecnt);
		}
	}
}

nospace(bdev, blocks, inodes)
char *bdev;
long blocks;
long inodes;
{
	sprintf(errbuf, "Not enough free space or i-nodes in %s file system.\n         %ld blocks, %ld inodes are needed.", bdev, blocks, inodes);
	error(errbuf,2);
}

/* print error message */

error(msg,err)
char *msg;
int err;
{
	fprintf(stderr, "idspace: %s\n", msg);
	if (err)
		exit(err);
}
