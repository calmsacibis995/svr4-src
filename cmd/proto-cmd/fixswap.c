/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)proto-cmd:fixswap.c	1.3"

#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/sysi86.h>
#include <sys/swap.h>
#include <sys/vtoc.h>
#include <sys/stat.h>

#define SWPLO	0

/* Temporary band-aid for Load B5 - just to let things compile */
#ifndef MSFILES
#define	MSFILES	16		/* The maximum number of swap	*/
				/* files which can be allocated.*/
				/* It is limited by the size of	*/
				/* the dbd_swpi field in the	*/
				/* dbd_t structure.		*/
#endif

long	pdloc;
struct disk_parms dp;
struct pdinfo	pdinfo;
struct vtoc vtoc;
int swplo = SWPLO;
int blkcnt;
char *rdevfile = "/dev/rdsk/0s0";
char *devfile = "/dev/dsk/0s0";
int rdevfd;
int fd;
char *swapfile = "/dev/swap";
struct stat sbuf;

swpi_t fixit;

main(argc, argv)
 char *argv[];
{
 char *cp;
 extern int errno;
 int slice;
 int dflag;
 int i;

	dflag = 0;
	if (argc > 1) {
		for (i = 1; i < argc; i++) {
			if (argv[i][0] == '-' && argv[i][1] == 'd')
				dflag = 1;
			else {
				printf("usage: fixswap [-d]\n");
				exit(1);
			}
		}
	}
	if (stat(swapfile, &sbuf)) {
		printf("fixswap: unable to stat %s, errno=%d\n", swapfile, errno);
		exit(1);
	}
	if ((sbuf.st_mode & S_IFMT) != S_IFBLK) {
		printf("fixswap: %s not a block special file\n", swapfile);
		exit(1);
	}
	if (dflag) {
		delswap();
		exit(0);
	}
	slice = minor(sbuf.st_rdev);
	if ((rdevfd = open(rdevfile, 0)) < 0) {
		printf("fixswap: unable to open %s, errno=%d\n", rdevfile, errno);
		exit(1);
	}
	ioctl(rdevfd, V_GETPARMS, (char *)&dp);
	ioctl(rdevfd, V_PDLOC, (char *)&pdloc);
	if ((fd = open(devfile, 0)) < 0) {
		printf("fixswap: unable to open %s, errno=%d\n", devfile, errno);
		exit(1);
	}
	lseek(fd, pdloc * dp.dp_secsiz, 0);
	if (read(fd, &pdinfo, sizeof(pdinfo)) != sizeof(pdinfo)) {
		printf("fixswap: read of pdinfo failed\n");
		exit(1);
	}
	lseek(fd, pdinfo.vtoc_ptr, 0);
	if (read(fd, &vtoc, sizeof(vtoc)) != sizeof(vtoc)) {
		printf("fixswap: read of VTOC failed\n");
		exit(1);
	}
	close(rdevfd);
	close(fd);
	if (vtoc.v_sanity != VTOC_SANE) {
		printf("fixswap: bad VTOC sanity field\n");
		exit(1);
	}
	if (vtoc.v_version != V_VERSION) {
		printf("fixswap: VTOC version not %d\n", V_VERSION);
		exit(1);
	}
	if (vtoc.v_nparts <= slice) {
		printf("fixswap: minor dev of %s too big\n", swapfile);
		exit(1);
	}
	if (vtoc.v_part[slice].p_tag != V_SWAP) {
		printf("fixswap: swap partition (%d) not marked as V_SWAP in VTOC\n",
			slice);
		exit(1);
	}
	blkcnt = vtoc.v_part[slice].p_size;
	if (blkcnt <= 0) {
		printf("fixswap: swap partition (%d) has bad size (%d)\n",
			slice, blkcnt);
		exit(1);
	}
	fixit.si_cmd = SI_ADD;
	fixit.si_buf = swapfile;
	fixit.si_swplo = swplo;
	fixit.si_nblks = blkcnt;
	if (sysi86(SI86SWPI, &fixit) < 0) {
		printf("fixswap: swapadd call failed: errno = %d\n", errno);
		exit(1);
	} else printf("%s added with swplo=%d, nblks=%d\n", swapfile, swplo, blkcnt);
	exit(0);
}

swpt_t swpbuf[MSFILES];

delswap()
{
 int sndx;
 int i;
 swpt_t *sp;
 int otherswap;

	fixit.si_cmd = SI_LIST;
	fixit.si_buf = (char *) swpbuf;
	if (sysi86(SI86SWPI, &fixit) < 0) {
		printf("fixswap: swaplist call failed: errno = %d\n", errno);
		exit(1);
	}
	sndx = -1;
	otherswap = -1;
	for (sp = swpbuf, i = 0; i < MSFILES; sp++, i++) {
		if (sp->st_ucnt == 0)
			continue;
		if (sp->st_flags & ST_INDEL) {
			if (sp->st_dev != sbuf.st_rdev || sp->st_swplo != swplo) {
				otherswap = i;
				continue;
			}
			sndx = i;
			continue;
		}
		if (sp->st_dev != sbuf.st_rdev || sp->st_swplo != swplo) {
			printf("fixswap: unknown swap device in use\n");
			exit(1);
		}
		sndx = i;
		fixit.si_cmd = SI_DEL;
		fixit.si_buf = swapfile;
		fixit.si_swplo = swplo;
		if (sysi86(SI86SWPI, &fixit) < 0) {
			printf("fixswap: swapdelete call failed: errno = %d\n", errno);
			exit(1);
		}
		continue;
	}
	if (sndx < 0) exit(0);
	sp = &swpbuf[sndx];
	do {
		sleep(1);
		fixit.si_cmd = SI_LIST;
		fixit.si_buf = (char *) swpbuf;
		if (sysi86(SI86SWPI, &fixit) < 0) {
			printf("fixswap: swaplist call failed: errno = %d\n", errno);
			exit(1);
		}
	} while (sp->st_ucnt);
	printf("%s successfully deleted as active swap device\n", swapfile);
}
