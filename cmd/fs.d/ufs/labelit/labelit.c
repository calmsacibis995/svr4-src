/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */

#ident	"@(#)ufs.cmds:ufs/labelit/labelit.c	1.6.3.2"

/*
 * Label a file system volume.
 */

#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/mntent.h>
#include <sys/vnode.h>
#include <fcntl.h>
#include <sys/fs/ufs_inode.h>
#include <sys/sysmacros.h>
#include <sys/fs/ufs_fs.h>

union sbtag {
	char		dummy[SBSIZE];
	struct fs	sblk;
} sb_un, altsb_un;

#define sblock sb_un.sblk
#define altsblock altsb_un.sblk

#ifdef RT
#define IFTAPE(s) (equal(s, "/dev/mt", 7) || equal(s, "mt", 2))
#define TAPENAMES "'/dev/mt'"
#else
#define IFTAPE(s)	(equal(s, "/dev/rmt", 8) || equal(s, "rmt", 3) || \
			 equal(s, "/dev/rtp", 8) || equal(s, "rtp", 3))
#define TAPENAMES "'/dev/rmt' or '/dev/rtp'"
#endif

#ifdef i386
struct {
	char	t_magic[8];
	char	t_volume[6];
	char	t_reels,
		t_reel;
	long	t_time,
		t_length,
		t_dens;
	char	t_fill[472];
} Tape_hdr;
#else
struct {
	char	t_magic[8];
	char	t_volume[6];
	char	t_reels,
		t_reel;
	long	t_time,
		t_length,
		t_dens,
		t_reelblks,
		t_blksize,
		t_nblocks;
	char	t_fill[472];
} Tape_hdr;
#endif

extern int	optind;
extern char	*optarg;

int	status;

main(argc, argv)
	int	argc;
	char	*argv[];
{
	int	opt;
	char	*special = NULL;
	char	*fsname = NULL;
	char	*volume = NULL;

	while ((opt = getopt (argc, argv, "?o:")) != EOF) {
		switch (opt) {

		case 'o':		/* specific options (none defined yet) */
			break;

#if 0
		case 'V':		/* Print command line */
			{
				char		*opt_text;
				int		opt_count;

				(void) fprintf (stdout, "labelit -F ufs ");
				for (opt_count = 1; opt_count < argc ; opt_count++) {
					opt_text = argv[opt_count];
					if (opt_text)
						(void) fprintf (stdout, " %s ", opt_text);
				}
				(void) fprintf (stdout, "\n");
			}
			break;
#endif
		case '?':
			usage();
		}
	}
	if (optind > (argc - 1)) {
		usage ();
	}
	argc -= optind;
	argv = &argv[optind];
	special = argv[0];
	if (argc > 0) {
		fsname = argv[1];
		if (strlen(fsname) > 6) usage();
	}
	if (argc > 1) {
		volume = argv[2];
		if (strlen(volume) > 6) usage();
	}
	if (IFTAPE(special)) {
		int	fso;

		if ((fso = open(special, 1)) < 0) {
			(void) fprintf (stderr, "labelit: ");
			perror ("open");
			exit (31+1);
		}
		printf("Skipping label check!\n");
		printf("NEW fsname = %.6s, NEW volname = %.6s -- DEL if wrong!!\n",
			fsname,volume);
		sleep(10);
		sprintf(sblock.fs_fsmnt, "%.6s", fsname);
		sprintf((char *)&sblock.fs_rotbl[0], "%.6s", volume);

		strcpy(Tape_hdr.t_magic, "Volcopy");
		sprintf(Tape_hdr.t_volume, "%.6s", volume);
		if (write(fso, &Tape_hdr, sizeof(Tape_hdr)) < 0 ||
		    write(fso, &sblock, sizeof(sblock)) < 0) {
			fprintf(stderr, "labelit: canot write label\n");
			exit(31+1);
		}
		close(fso);
	} else
		label(special, fsname, volume);
	exit(0);
}

usage ()
{

	(void) fprintf (stderr, "ufs usage: labelit [-F ufs] [generic options] special [fsname volume]\n");
	exit (31+1);
}

label (special, fsname, volume)
	char		*special;
	char		*fsname;
	char		*volume;
{
	int	f;
	int	blk;
	int	i;
	int	cylno;
	int	rpos;
	char	*p;
	int	offset;

	if (fsname == NULL) {
		f = open(special, O_RDONLY);
	} else {
		f = open(special, O_RDWR);
	}
	if (f < 0) {
		(void) fprintf (stderr, "labelit: ");
		perror ("open");
		exit (31+1);
	}
	if (lseek(f, SBLOCK * DEV_BSIZE, 0) < 0) {
		(void) fprintf (stderr, "labelit: ");
		perror ("lseek");
		exit (31+1);
	}
	if (read(f, &sblock, SBSIZE) != SBSIZE) {
		(void) fprintf (stderr, "labelit: ");
		perror ("read");
		exit (31+1);
	}
	if (sblock.fs_magic != FS_MAGIC) {
		(void) fprintf (stderr, "labelit: ");
		(void) fprintf (stderr, "bad super block magic number\n");
		exit (31+1);
	}

	/*
	 * calculate the available blocks for each rotational position
	 */
	blk = sblock.fs_spc * sblock.fs_cpc / NSPF(&sblock);
	for (i = 0; i < blk; i += sblock.fs_frag)
		/* void */;
	i -= sblock.fs_frag;
	cylno = cbtocylno(&sblock, i);
	rpos = cbtorpos(&sblock, i);
	blk = i / sblock.fs_frag;
	p = (char *)&(sblock.fs_rotbl[blk]);
	if (fsname != NULL) {
		for (i = 0; i < 14; i++)
			p[i] = '\0';		
		for (i = 0; (i < 6) && (fsname[i]); i++, p++)
			*p = fsname[i];
		p++;	
	}
	if (volume != NULL) {
		for (i = 0; (i < 6) && (volume[i]); i++, p++)
			*p = volume[i];
	}
	if (fsname != NULL) {
		if (lseek(f, SBLOCK * DEV_BSIZE, 0) < 0) {
			(void) fprintf (stderr, "labelit: ");
			perror ("lseek");
			exit (31+1);
		}
		if (write(f, &sblock, SBSIZE) != SBSIZE) {
			(void) fprintf (stderr, "labelit: ");
			perror ("write");
			exit (31+1);
		}
		for (i = 0; i < sblock.fs_ncg; i++) {
		offset = cgsblock(&sblock, i) * sblock.fs_fsize;
		if (lseek(f, offset, 0) < 0) {
			(void) fprintf (stderr, "labelit: ");
			perror ("lseek");
			exit (31+1);
		}
		if (read(f, &altsblock, SBSIZE) != SBSIZE) {
			(void) fprintf (stderr, "labelit: ");
			perror ("read");
			exit (31+1);
		}
		if (altsblock.fs_magic != FS_MAGIC) {
			(void) fprintf (stderr, "labelit: ");
			(void) fprintf (stderr, "bad alternate super block(%i) magic number\n", i);
			exit (31+1);
		}
		bcopy((char *)&(sblock.fs_rotbl[blk]),
			(char *)&(altsblock.fs_rotbl[blk]), 14);
		
		if (lseek(f, offset, 0) < 0) {
			(void) fprintf (stderr, "labelit: ");
			perror ("lseek");
			exit (31+1);
		}
		if (write(f, &altsblock, SBSIZE) != SBSIZE) {
			(void) fprintf (stderr, "labelit: ");
			perror ("write");
			exit (31+1);
		}
		}
	}
	p = (char *)&(sblock.fs_rotbl[blk]);
	fprintf (stderr, "fsname: ");
	for (i = 0; (i < 6) && (*p); i++, p++) {
		fprintf (stderr, "%c", *p);
	}
	fprintf (stderr, "\n");
	fprintf (stderr, "volume: ");
	p++;
	for (i = 0; (i < 6); i++, p++) {
		fprintf (stderr, "%c", *p);
	}
	fprintf (stderr, "\n");
}

equal(s1,s2,ct)
	char *s1, *s2;
	int ct;
{
	return	!strncmp(s1, s2, ct);
}
