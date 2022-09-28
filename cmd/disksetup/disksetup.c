/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)disksetup:disksetup.c	1.4.3.6"

/* The file disksetup.c contains the architecture independent routines used   */
/* to install a hard disk as the boot disk or an additional disk. The tasks   */
/* it will handle are: retrieving the location of the UNIX partition, surface */
/* analysis, setting up the pdinfo, VTOC and alternates table and writing     */
/* them to the disk, loading the hard disk boot routine, issuing mkfs, labelit*/
/* and mount requests for slices which will be filesystems, and updating the  */
/* the vfstab file appropriately.					      */

#include <sys/types.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/fsid.h>
#include <sys/fstyp.h>
#include <sys/hd.h>
#include <malloc.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/swap.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/alttbl.h>
#include <sys/param.h>
#include <sys/vtoc.h>

#ifdef MBUS
#include <sys/bbh.h>
#endif /* MBUS */



/*
 * The following structure is used to contain information about partitions
 */
#define TRUE		1
#define FALSE		0
#define RESERVED        34	/* reserved sectors at start of drive */
#define ROOTSLICE	1
#define SWAPSLICE	2
#define USRSLICE	3
#define HOMESLICE	4
#define DOSSLICE	5
#define DUMPSLICE	6
#define BOOTSLICE	7
#define ALTSSLICE	8
#define TALTSLICE	9
#define STANDSLICE	10
#define VARSLICE	11
#define HOME2SLICE	12
#define GOOD_BLK	0
#define SNAMSZ		33
#define S51KMKFS        1
#define S52KMKFS        2
#define UFSMKFS         3
#define BFSMKFS         4
#define LINESIZE	512
#define ONEMB		1048576L
#define FOURMB		4194394L
#define FRAGSIZE	1024

int     diskfd;         	/* file descriptor for raw wini disk */
int     vfstabfd = 0;         	/* file descriptor for /etc/vfstab */
int     defaultsfd;         	/* file descriptor for default setup file */
short	defaultsflag = FALSE;	/* Flag to designate valid def. file found */
FILE	*defaultsfile;		/* Flag to designate valid def. file found */
short	defaults_rejected = TRUE; /* Flag to designate if defaults choose */
int	bootfd;			/* boot file descriptor */
int	bootdisk = 0;		/* flag signifying if device is boot disk */
int	installflg = 0;		/* flag signifying installing disk */
struct  disk_parms      dp;     /* Disk parameters returned from driver */
struct	vtoc		vtoc;	/* struct containing slice info */
struct  pdinfo		pdinfo; /* struct containing disk param info */
struct	alt_info 	alttbls;/* struct contains bad sec & track info */
#define sec 		alttbls.alt_sec
#define trk 		alttbls.alt_trk
char    replybuf[160];           /* used for user replies to questions */
char    *devname;		/* pointer to device name */
char    *bootname;		/* pointer to boot file name */
char    mkfsname[20];		/* pointer to device name to issue mkfs calls */
int     cylsecs;                /* number of sectors per cylinder */
long    cylbytes;               /* number of bytes per cylinder */
daddr_t	unix_base;		/* first sector of UNIX System partition */
daddr_t	unix_size;		/* # sectors in UNIX System partition */
daddr_t pstart;			/* next slice start location */
int	load_boot = FALSE;      /* flag for load boot option */
int	scsi_flag = FALSE;	/* flag indicating a scsi drive */
int	instsysflag = FALSE;    /* indicates 2nd disk of dual disk install*/
struct absio	absio;		/* buf used for RDABS & WRABS ioctls */
char	errstring[12] = "Disksetup";
/* querylist is used to request slices in the right order for a boot */
/* disk, i.e., stand, dump, swap, root, usr, home, var, tmp, etc.    */
/* the order creates precedence and physical location on the disk    */
#ifdef MBUS
	int querylist[V_NUMPAR] = {0, 1, 10, 2, 3, 4, 11, 12, 13, 14, 15, 6, 0, 0, 0, 0};
#else
     	int querylist[V_NUMPAR] = {0, 10, 6, 2, 1, 3, 4, 11, 12, 13, 14, 15, 0,
0, 0, 0};
#endif


/* sliceinfo has two purposes, first contain setup info for the first disk, */
/* second is to contain info the user chooses for setup of the disk. The */
/* sname field will contain the name of the slice/filesystem. The size field */
/* represents the minimum size slice can be for the system to install. The */
/* createflag designates if the slice is to be created. The mkfsflag  */
/* designates the need to issue a mkfs on the slice. */
struct {char sname[SNAMSZ];	/* slice name */
	int  size;		/* recommended size if created */
	short createflag;	/* Turned on when user specified */
	short mkfsflag;		/* Used to indicate if and which mkfs to run */
	short reqflag;		/* Used to indicate required slice (eg. / ) */
	int  minsz;		/* minimum recommended size */
	} sliceinfo[V_NUMPAR];

#ifdef MBUS
	char options_string[7] = "VIBb:d:"; 
	int verify_flg =	FALSE;	/* -V option */
#else 	/* AT386 */
	int verify_flg = 	TRUE;	/* -V option default */
#ifdef NLTDISK	/* Non_Logical-Translation disk */
#define MAXCYLS	2048
#define MINCYLS 306
#define PARAM_CHANGED 11
	int param_override = 0;
	int partend = 0;
	char options_string[7] = "IBOb:d:";
#else
	char options_string[6] = "IBb:d:"; 
#endif 	/* NLTDISK */
#endif 	/* AT386 */

void
main(argc,argv)
int argc;
char *argv[];
{
	extern char	*optarg;
	extern int	optind;
	int	c, errflg = 0;
	struct stat statbuf;

	while ((c = getopt(argc, argv, options_string)) != -1) {
		switch (c) {
		case 'b': 
			if ((bootfd = open(optarg, O_RDONLY)) == -1) {
				fprintf(stderr, "Unable to open specified boot routine.\n");
				exit(40);
			}
			bootname = optarg;
			load_boot = TRUE;  
			break;
		case 'B':
			bootdisk = TRUE;  
			break;
		case 'I':
			installflg = TRUE;  
			break;
		case 'd' :
			if (((defaultsfd = open(optarg, O_RDONLY)) == -1) ||
			   ((defaultsfile = fdopen(defaultsfd, "r"))  == NULL))
				fprintf(stderr,"Unable to open defaults file.\n");
			else
				defaultsflag = TRUE;
			break;
#ifdef NLTDISK
		case 'O' :
			param_override = TRUE;
			break;
#endif
#ifdef MBUS
		case 'V':
			verify_flg = TRUE;  
			break;
#endif
		case '?':
			++errflg;
		}
	}

	if (argc - optind < 1)
		++errflg;
	if (errflg) {
		giveusage();
		exit(40);
	}
	devname = argv[optind];
#ifdef MBUS
	sprintf(mkfsname,"/dev/dsk/%cs",devname[10]);
#else
	strncpy(mkfsname, devname, (strlen(devname) - 1)); 
#endif
	if (stat(devname, &statbuf)) {
		fprintf(stderr,"disksetup stat of %s failed\n",devname);
		perror(errstring);
		giveusage();
		exit(1);
	}
	if ((statbuf.st_mode & S_IFMT) != S_IFCHR) {
		fprintf(stderr,"disksetup: device %s is not character special\n",devname);
		giveusage();
		exit(1);
	}
	if ((diskfd=open(devname,O_RDWR)) == -1) {
		fprintf(stderr,"Disksetup unable to open %s",devname);
		exit(50);
	}
	if (ioctl(diskfd,V_GETPARMS,&dp) == -1) {
		fprintf(stderr,"Disksetup V_GETPARMS failed on %s",devname);
		exit(51);
	}

        if ((dp.dp_type == DPT_SCSI_HD) || (dp.dp_type == DPT_SCSI_OD))
		scsi_flag = TRUE;
	memset((char *)&sliceinfo, 0, sizeof(sliceinfo));
#ifdef NLTDISK
        if (param_override) 
		mod_param();
	if (param_override == PARAM_CHANGED) {
		get_unix_partition();
		init_structs();
		writevtoc();
		exit(PARAM_CHANGED);
	}
#endif /* NLTDISK */

	get_unix_partition(); /*retrieve part. table from disk */

	if (load_boot)
		loadboot();  /* writes boot to track 0 of Unix part. */

	if (installflg) {  /* installing boot disk or additional disks */
		init_structs(); /* initialize pdinfo and vtoc */
		if (verify_flg)
			do_surface_analysis(); /* search for media defects */
		if (defaultsflag == TRUE)
			offer_defaults();
		if (defaults_rejected == TRUE)
			setup_vtoc(); /* query user for slices and sizes */
		writevtoc();  /*writes pdinfo, vtoc and alternates table */
		create_fs();  /* Issues mkfs calls, mounts and updates vfstab */
	}
	exit(0);
}

giveusage()
{
#ifdef MBUS
	fprintf(stderr, "Usage: disksetup [-BIOV] [-b bootfile] [-d configfile] device\n");
#else
#ifdef NLTDISK
	fprintf(stderr, "Usage: disksetup [-BIO] [-b bootfile] [-d configfile] device\n");
#else
	fprintf(stderr, "Usage: disksetup -BI -b bootfile [-d configfile] raw-device (install boot disk)\n");
	fprintf(stderr, "Usage: disksetup -I [-d configfile] raw-device (install additional disk(s))\n");
	fprintf(stderr, "Usage: disksetup -b bootfile raw-device (write boot code to disk)\n");
#endif /* NLTDISK */
#endif /* MBUS    */
}

cannot_install()
{
	fprintf(stderr,
"The UNIX System cannot be installed on the partition as it is currently defined.\n");
	fprintf(stderr,
"Try the installation again but do not have UNIX System partition begin \n");
	fprintf(stderr,
"on cylinder %d.\n\n",
		unix_base / cylsecs);
	exit(99);
}

fs_error(fsname)
char *fsname;
{
	fprintf(stderr,"Cannot create/mount the %s filesystem.  Please contact\n",fsname);
	fprintf(stderr,"your service representative for further assistance.\n");
	exit(1);
}

/* do_surface_analysis verifies all sectors in the Unix partition. It looks */
/* for bad tracks (3 or more bad sectors in the track) and bad sectors. All */
/* defects are then kept in the appropriate table (ie bad tracks in the bad */
/* track table). Alternates are then reserved for the found defects and for */
/* future defects. Number of alt sectors to be reserved should be the number*/
/* of bad sectors found + 1 sector/MB of space in UNIX partion (minimum 32) */
do_surface_analysis()
{
	daddr_t	lastusec = unix_base + unix_size;
	daddr_t cursec, badtrk_loc;
	daddr_t	badtrks[MAX_ALTENTS], badsects[MAX_ALTENTS];
	int     extra_alt, j, i; 
	int 	altsec_size, altsec_start, alttrk_size, alttrk_start;

	if (scsi_flag == TRUE) {
		printf("Surface analysis of your disk is recommended\n");
		printf("but not required.\n\n");
		printf("Do you wish to skip surface analysis? (y/n) ");
		if (!yes_response()) {
			printf("\nChecking for bad sectors in the UNIX System partition...\n\n");
			sprintf(replybuf,"/etc/scsi/scsiformat -i -nv -u %s",devname);
			if (system(replybuf)) {
				fprintf(stderr,"Surface analysis of your SCSI drive failed. The drive may not be connected\n");
 				fprintf(stderr,"correctly or, may not be formatted.\n");
				perror(errstring);
				cannot_install();
			}
		}
		return;
	}
	altsec_start = 0;
	printf("\nChecking for bad sectors in the UNIX System partition...\n\n");
	/* Check BOOT track(s) for defects, if any found can't install */
	if (verify(unix_base, RESERVED, 3)) {
		fprintf(stderr,"Bad sector detected on boot track.\n");
		cannot_install();
	}
	cursec = unix_base;
	if ((cursec % (daddr_t)dp.dp_sectors) != 0) {
		i = (daddr_t)dp.dp_sectors - (cursec % (daddr_t)dp.dp_sectors);
		sector_scan(cursec, i, &badsects);
		cursec += i;
	}
	for (; cursec < lastusec; cursec += dp.dp_sectors) {
		if (cursec % cylsecs == 0) {
			printf("\rChecking cylinder: %d", cursec / cylsecs);
			fflush(stdout);
		}
		/* check track, if any defects than check 1 sector at a time */
		if (verify(cursec, dp.dp_sectors, 1) &&
		    sector_scan(cursec, dp.dp_sectors, &badsects)) {
			/* Entire track was marked bad */
			if (trk.alt_used < MAX_ALTENTS) 
				badtrks[trk.alt_used++] = cursec / (daddr_t)dp.dp_sectors;
			else {
				fprintf(stderr,"Too many bad tracks found (%d).", trk.alt_used);
				cannot_install();
			}
		}
	}
	printf("\r                       \r");

	/* Make sure there are no bad tracks in the alternate sector area */
	if (trk.alt_used > 0) {
		alttrk_start = pstart;
		alttrk_size = trk.alt_used * dp.dp_sectors;
		altsec_start = alttrk_start + alttrk_size;
		altsec_size = cylsecs - (altsec_start % cylsecs);
 		if ((extra_alt=(unix_size * (daddr_t)dp.dp_secsiz)/ONEMB) < 32)
			extra_alt = 32;
 		while (altsec_size < (int)sec.alt_used + extra_alt)
			altsec_size += dp.dp_sectors;
		if (altsec_size > MAX_ALTENTS)
			altsec_size = MAX_ALTENTS;
		i = 0;
		badtrk_loc = badtrks[i] * (daddr_t)dp.dp_sectors;  
		while ((i < (int)trk.alt_used) && 
		      (badtrk_loc < (daddr_t)(altsec_start + altsec_size))) {
			if (badtrk_loc < (daddr_t)altsec_start) {
				j = badtrks[i] - (alttrk_start/(daddr_t)dp.dp_sectors);
				trk.alt_bad[j] = -1;
				badtrks[i] = -1;
				badtrk_loc=badtrks[++i]*(daddr_t)dp.dp_sectors; 
			}
			else {
				/* Take bad track out of the bad track list */
				if (--trk.alt_used > 0) 
					for (j=i; j < (int)trk.alt_used; j++)
						badtrks[j] = badtrks[j+1];
				for (j=0; j < (int)dp.dp_sectors; j++)
					if (sec.alt_used < MAX_ALTENTS)
						badsects[sec.alt_used++] = badtrk_loc+j;
					else {
						fprintf(stderr,"Too many bad sectors found (%d).", sec.alt_used);
						cannot_install();
					}
				/* Adjust the start of the alt sector area */
				alttrk_size -= dp.dp_sectors;
				altsec_start -= dp.dp_sectors;
				altsec_size += dp.dp_sectors;
				badtrk_loc=badtrks[++i]*(daddr_t)dp.dp_sectors; 
		 	}
		}
		/* Reserve space for track alternates */
		if (trk.alt_used > 0) {
			vtoc.v_part[TALTSLICE].p_start = alttrk_start;
			vtoc.v_part[TALTSLICE].p_size = alttrk_size;
			vtoc.v_part[TALTSLICE].p_flag = V_VALID | V_UNMNT;
			vtoc.v_part[TALTSLICE].p_tag = V_ALTTRK;
			pstart += alttrk_size;
		}
		/* If any bad tracks remain put them in available slots of  */
		/* trk.alt_bad array (ie skip slots of alttrks that are bad) */
		for (i=0,j=0; i < (int)trk.alt_used; ) {
			if (trk.alt_bad[i] == -1)
				i++;
			else
				if (badtrks[j] == -1)
					j++;
			else 
				trk.alt_bad[i++] = badtrks[j++];
		}
	}
	/* setup altsec start and size if not set up above during badtrk part*/
	if (altsec_start == 0) {
		altsec_start = pstart;
		altsec_size = cylsecs - (altsec_start % cylsecs);
 		if ((extra_alt=(unix_size * (daddr_t)dp.dp_secsiz)/ONEMB) < 32)
			extra_alt = 32;
 		while (altsec_size < (int)sec.alt_used + extra_alt)
			altsec_size += dp.dp_sectors;
	}
	if (altsec_size > MAX_ALTENTS)
		altsec_size = MAX_ALTENTS;
	/* Now handle bad sectors checking for bad sectors in the alt sector */
 	/* area mark as -1 in sec.alt_bad and then in badsects array 	     */
	for (i=0; i < (int)sec.alt_used; i++) {
		if ((badsects[i] >= altsec_start) && 
		   (badsects[i] < altsec_start+altsec_size)) { 
			sec.alt_bad[badsects[i]-altsec_start] = -1;
			badsects[i] = -1;
		}
	}
 	/* Now go thru bad sectors again copying remaining sectors to into */
	/* available entries in sec.alt_bad				   */
	for (i=0,j=0; i < (int)sec.alt_used; ) {
		if (sec.alt_bad[i] == -1)
			i++;
		else
			if (badsects[j] == -1)
				j++;
			else 
				sec.alt_bad[i++] = badsects[j++];
	}
	/* reserve space in chunks of cylinders to correspond with the	*/
	/*	calculated need for alternates.				*/
	vtoc.v_part[ALTSSLICE].p_start = altsec_start;
	vtoc.v_part[ALTSSLICE].p_flag = V_VALID | V_UNMNT;
	vtoc.v_part[ALTSSLICE].p_tag = V_ALTS;
	vtoc.v_part[ALTSSLICE].p_size = altsec_size;
	trk.alt_reserved = trk.alt_used;
	sec.alt_reserved = altsec_size;
	trk.alt_base = vtoc.v_part[TALTSLICE].p_start;
	sec.alt_base = vtoc.v_part[ALTSSLICE].p_start;
	pstart += altsec_size;
	if (pstart % (daddr_t)dp.dp_sectors) /*keep pstart track aligned */
		pstart = (pstart / (daddr_t)dp.dp_sectors + 1) * dp.dp_sectors;

	if (trk.alt_used > 0) {
		printf("%d bad track%s found during the verify pass.\n\n",
		trk.alt_used, trk.alt_used == 1 ? " was" : "s were");
	}
	if (sec.alt_used > 0) {
		printf("%d bad sector%s found during the verify pass.\n\n",
		sec.alt_used, sec.alt_used == 1 ? " was" : "s were");
	}
}


verify(start_sec, n_sec, n_tries)
daddr_t	start_sec;	/* first sector to verify */
int	n_sec;		/* number of sectors to verify */
int	n_tries;	/* number of times (if all succeed) */
{
	union vfy_io	vfy;

	while (n_tries-- > 0) {
		vfy.vfy_in.abs_sec = start_sec;
		vfy.vfy_in.num_sec = n_sec;
		vfy.vfy_in.time_flg = 0;
		if (ioctl(diskfd, V_VERIFY, &vfy) != 0) {
			fprintf(stderr, "\n");
			perror("Verify operation failed");
			exit(81);
		}
		if (vfy.vfy_out.err_code)
			return vfy.vfy_out.err_code;
	}
	return 0;
}


sector_scan(start_sec, n_sec, badsects)
daddr_t	start_sec;	/* first sector to verify */
int	n_sec;		/* number of sectors to verify */
daddr_t *badsects;
{
	ushort	statbuf[64];		/* error status of sectors in a track */
	int	bad_blk_cnt, i;

	bad_blk_cnt = 0;
	for (i = 0; i < n_sec; i++) {
		if ((statbuf[i] = verify(start_sec+i, 1, 2)) != GOOD_BLK)
			bad_blk_cnt++;
	}
	/* If more than 3 sectors are bad consider the track bad. Others */
	/* will go bad or allow translation having only a portion marked bad */ 
	if (bad_blk_cnt > 3)
		return TRUE;
	
	for (i = 0; i < n_sec; i++) {
		if (statbuf[i] != GOOD_BLK)
			if (sec.alt_used < MAX_ALTENTS)
				badsects[sec.alt_used++] = start_sec + i;
			else {
				fprintf(stderr,"Too many bad sectors found.\n");
				cannot_install();
			}
	}
	return FALSE;
}

/* convert string of /dev/rdsk/xxxxx to /dev/dsk/xxxxx */
char *
rawtoblk(rawdev)
char *rawdev;
{
 	char *tmppt;
	static char tmpdev[20];
	int found = 0;

        sprintf(tmpdev, "%s", rawdev);
	tmppt = tmpdev;
	while (*tmppt != NULL) {
		if (*tmppt == 'r' || found) {
                        *tmppt = *(tmppt+1);
                        found = TRUE;
		}
		tmppt++;
	}
	return(tmpdev);
}

/*
 * Writevtoc ()
 * Write out the updated volume label, pdinfo, vtoc, and alternate table.  We
 * assume that the pdinfo and vtoc, together, will fit into a single BSIZE'ed
 * block.  (This is currently true on even 512 byte/block systems;  this code
 * may need fixing if a data structure grows).
 * We are careful to read the block that the volume label resides in, and
 * overwrite the label at its offset;  writeboot() should have taken care of
 * leaving this hole.
 */
writevtoc()
{
  	char	*buf;
  	int len, i;

	for (i=1; i < V_NUMPAR; i++) 
		if (vtoc.v_part[i].p_size == 0)  {
				vtoc.v_part[i].p_tag = 0;
				vtoc.v_part[i].p_flag = 0;
				vtoc.v_part[i].p_start = 0;
		}

/*############################################################*/
#ifdef MBUS
		/* Write the PDINFO to disk.  */
		if ((ioctl( diskfd, V_L_PDIN, &pdinfo) == -1) ||
		    (ioctl( diskfd, V_W_PDIN, NULL) == -1)) {
			perror( "Writing new PDINFO");
			exit(51);
		}

		/* Write the VTOC to disk.  */
		if ((ioctl( diskfd, V_L_VTOC, &vtoc) == -1) ||
		    (ioctl( diskfd, V_W_VTOC, NULL) == -1)) {
			perror( "Writing new VTOC");
			exit(52);
		}

		/* Write the SW Alt Info to disk.  */
		if ((ioctl( diskfd, V_L_SWALT, &alttbls) == -1) ||
		    (ioctl( diskfd, V_W_SWALT, NULL) == -1)) {
			perror( "Writing new SW Alternate Info");
			exit(53);
		}
#else
/*############################################################*/

	/* allocate a buffer large enough to accomodate the alternates list. */
	len = ((sizeof(alttbls) + 511) / dp.dp_secsiz) * dp.dp_secsiz;
	if ((buf=malloc(len)) == NULL) {
		fprintf(stderr,"writevtoc -- can't allocate buffer\n");
		exit(69);
	}
	/* put pdinfo & vtoc into the same sector */
	*((struct pdinfo *)buf) = pdinfo;
	*((struct vtoc *)&buf[pdinfo.vtoc_ptr%dp.dp_secsiz]) = vtoc;
	absio.abs_sec = unix_base + VTOC_SEC;
	absio.abs_buf = buf;
	if (ioctl(diskfd, V_WRABS, &absio) == -1) {
		fprintf(stderr,"Error Writing pdinfo and VTOC.\n");
		exit(51);
	}
	/*	now do the alternate table	*/
	if (scsi_flag == FALSE) {
		memcpy(buf, ((char *) &alttbls), sizeof(alttbls)); 
		for (i=0; i < len/512; i++) { 
			absio.abs_sec = unix_base + VTOC_SEC + 1 + i; 
			absio.abs_buf = (buf + (i * 512)); 
			if (ioctl(diskfd, V_WRABS, &absio) != 0) {
				fprintf(stderr,"Error writing alternates table to the disk!\n"); 
				exit(43); 
	    		} 
		} 
	}
	free(buf);
#endif /* MBUS */
	sync();
	ioctl(diskfd, V_REMOUNT, NULL);
	close(diskfd);
	if (bootdisk) {
		char *blkdev;
		register swapres_t	*si;
		register struct swaptable	*st;
		register struct swapent *swapent;
		swapres_t		swpi;
		int num = 0;

		blkdev = rawtoblk(mkfsname);
		strncat(blkdev,"2",1);
		if ((num = swapctl(SC_GETNSWP, NULL)) == -1)
			fprintf(stderr,"Disksetup: can't retrieve swap devs\n");
        	if ((st = (swaptbl_t *) malloc(num * sizeof(swapent_t) + sizeof(int))) == NULL) 
                	fprintf(stderr,"Disksetup: Malloc for swap setup failed\n");
		else {
			swapent=st->swt_ent;
			for (i=0; i<num; i++)
				swapent->ste_path = (char *) malloc(MAXPATHLEN);
			st->swt_n = num;
			if ((num = swapctl(SC_LIST, st)) == -1)
				fprintf(stderr,"Disksetup: can't retrieve swap devs\n");
			else {
/* check if hard disk swap devive added previously, can't reset */
				if (strcmp(st->swt_ent->ste_path,blkdev) == 0)  
					return;
			}
		}
/* Add hardware swap device to system swap devs */
		si = &swpi;
		si->sr_name = blkdev;
		si->sr_start = 0;
		si->sr_length = vtoc.v_part[SWAPSLICE].p_size;
		if (swapctl(SC_ADD, si) < 0) {
			fprintf(stderr,"Disksetup: add of disk swap failed\n");
			perror(errstring);
		}
/* Now remove ramd swap dev initially set up by boot process */
		si = &swpi;
		si->sr_name = st->swt_ent->ste_path;
		si->sr_start = 0;
		if (swapctl(SC_REMOVE, si) < 0) {
			fprintf(stderr,"Disksetup: remove of swap failed\n");
			perror(errstring);
		}
	}
}

int 
yes_response()
{
	for (;;) {
		gets(replybuf);
		if (replybuf[0] == 'y' || replybuf[0] == 'Y') 
			return 1;
		if (replybuf[0] == 'n' || replybuf[0] == 'N') 
			return 0;
		printf("\nInvalid response - please answer with y or n.");
	}
}

fill_vtoc()
{
	int i, j, lastslice = 0;

	for (i=1; i < V_NUMPAR; i++) {
		if (bootdisk)
			j = querylist[i];
		else
			j = i;
		if (sliceinfo[j].size > 0) {
			lastslice = j;
			vtoc.v_part[j].p_start = pstart;
			vtoc.v_part[j].p_size = sliceinfo[j].size;
			pstart += sliceinfo[j].size;
			vtoc.v_part[j].p_flag = V_VALID;
			switch (j) {
			case ROOTSLICE  : if (bootdisk == TRUE)
						vtoc.v_part[j].p_tag = V_ROOT;
					  else
						vtoc.v_part[j].p_tag = V_USR;
				  	  break;
			case SWAPSLICE  : if (strcmp(sliceinfo[j].sname,"/dev/swap")== 0)
						vtoc.v_part[j].p_tag = V_SWAP;
					  else
						vtoc.v_part[j].p_tag = V_USR;
				  	  break;
			case STANDSLICE : if (strcmp(sliceinfo[j].sname,"/stand") == 0) 
						vtoc.v_part[j].p_tag = V_STAND;
					  else
						vtoc.v_part[j].p_tag = V_USR;
				  	  break;
			case VARSLICE : if (strcmp(sliceinfo[j].sname,"/var") == 0) 
						vtoc.v_part[j].p_tag = V_VAR;
					  else
						vtoc.v_part[j].p_tag = V_USR;
				  	  break;
			case HOMESLICE  : 
			case HOME2SLICE : if (strncmp(sliceinfo[j].sname,"/home",5) == 0)  
					  	vtoc.v_part[j].p_tag = V_HOME;
					  else
						vtoc.v_part[j].p_tag = V_USR;
				  	  break;
			case USRSLICE   : vtoc.v_part[j].p_tag = V_USR;
				  	  break;
			case DUMPSLICE  : if (strcmp(sliceinfo[j].sname,"/dev/dump") == 0)
					 	vtoc.v_part[j].p_tag = V_DUMP;
					  else
						vtoc.v_part[j].p_tag = V_USR;
				  	  break;
			default	 	: vtoc.v_part[j].p_tag = V_USR;
			}
			if (sliceinfo[j].mkfsflag == 0 && vtoc.v_part[j].p_tag != V_USR)
				vtoc.v_part[j].p_flag |=V_UNMNT;
		}
	}
	for (i=1; i < V_NUMPAR; i++) 
		if (vtoc.v_part[i].p_size > 0)
			vtoc.v_nparts = i+1;
	/* any remaining cyls are tacked onto last slice set up */
	if (vtoc.v_part[lastslice].p_start + vtoc.v_part[lastslice].p_size < 
	    unix_base + unix_size)
		vtoc.v_part[lastslice].p_size = unix_base + unix_size -
			vtoc.v_part[lastslice].p_start;
		
}

daddr_t
check_swapsz(numsects, memsize)
daddr_t numsects;
daddr_t memsize;

{
	daddr_t newsects;
	if ((memsize * (daddr_t)dp.dp_secsiz) > ONEMB * 24)
		newsects = ((numsects/cylsecs) / 2) * cylsecs;
	else
		if ((memsize * (daddr_t)dp.dp_secsiz) > ONEMB * 12)
			newsects = ((numsects/cylsecs) * 0.75) * cylsecs;
		else
			newsects = numsects;
	return newsects;
}

/* offer_defaults will read in defaults file, display the defaults, query the */
/* user if they want the defaults. If the user chooses the defaults, the vtoc */
/* and sliceinfo will be setup accordingly.				      */
offer_defaults()
{
	int i, n, slicenum, wflag = FALSE;
	int cnt, totalpcnt = 0;
	int prtflag = TRUE;
	daddr_t init_pstart, memsize, dfltsz, minsz;
	daddr_t oneMBsects = ONEMB/(daddr_t)dp.dp_secsiz; /* 1MB in sectors */
	daddr_t numsects, availsects, availcyls, reqcyls, totalcyls;
	FILE *pipe;
	short reqslice_err = FALSE;
	char slicename[32], fstyp[32], sizetype, mintype, reqflg, line[LINESIZE];

	if (((pipe = popen("memsize", "r")) == NULL) ||
	   (fscanf(pipe, "%ld",&memsize) != 1)) {
		fprintf(stderr,"Cannot retrieve size of memory, 4MB will be assumed\n");
		memsize = FOURMB;
	}
	if (pipe != NULL)
		(void)pclose(pipe);
	else
		memsize = FOURMB;
	if (memsize % ONEMB != 0)
		memsize = ((memsize / ONEMB) + 1) * ONEMB;
	memsize /= (daddr_t)dp.dp_secsiz; /* convert memsize to sectors */
	availsects = (unix_base + unix_size) - pstart;
 
	for (i=1; (fgets(line,LINESIZE,defaultsfile) != NULL); i++) {
		n = sscanf(line,"%d %32s %32s %d%c %d%c%c", &slicenum,
		    slicename, fstyp, &dfltsz, &sizetype, &minsz, &mintype, &reqflg); 
		if ((n < 7) ||
		   ((slicenum < 1) || (slicenum >= V_NUMPAR) || (slicenum == BOOTSLICE)) ||
		   ((scsi_flag == FALSE) &&
		   ((slicenum == ALTSSLICE) || (slicenum == TALTSLICE)))) {
			fprintf(stderr,"Disksetup: defaults file line %d is invalid and will be skipped.\n",i);
			continue;
		}
		if (reqflg == '\n') {	/* Old format, mintype is meg */
			reqflg = mintype;
			mintype = 'M';
		}
		strcpy(sliceinfo[slicenum].sname, slicename);
		sliceinfo[slicenum].createflag = TRUE;

		switch (mintype) {
			case 'm': if (((numsects = minsz * memsize) % cylsecs) != 0)
					numsects = (numsects/cylsecs + 1) * cylsecs;
				  minsz = numsects;
				  break;
			case 'M':
			default:
				  minsz *= oneMBsects;
				  if ((minsz % cylsecs) != 0)
					minsz = (minsz / cylsecs + 1) * cylsecs;
				  break;
		}
		sliceinfo[slicenum].minsz = minsz;
		if (reqflg == 'R')
			sliceinfo[slicenum].reqflag = TRUE;
		else
			sliceinfo[slicenum].reqflag = FALSE;
		if (strcmp(fstyp, "bfs") == 0)
			sliceinfo[slicenum].mkfsflag = BFSMKFS;
		else
			if (strcmp(fstyp, "ufs") == 0)
				sliceinfo[slicenum].mkfsflag = UFSMKFS;
			else
				if (strcmp(fstyp, "s5") == 0)
					sliceinfo[slicenum].mkfsflag = S51KMKFS;
				else
					if (strcmp(fstyp, "s52k") == 0)
						sliceinfo[slicenum].mkfsflag = S52KMKFS;
		switch (sizetype) {
		/* set size as neg. to flag for calc. after M and m entries */
		case 'W': sliceinfo[slicenum].size = -(dfltsz);
			  totalpcnt += dfltsz;
			  wflag = TRUE;
			  break;
		case 'm': if (((numsects = dfltsz * memsize) % cylsecs) != 0)
				numsects = (numsects/cylsecs + 1) * cylsecs;
			  if ((strcmp(slicename, "/dev/swap") == 0) &&
			     (dfltsz == 2))
				numsects = check_swapsz(numsects, memsize);
			  if ((numsects <= availsects) && 
			     (numsects >= minsz))
				sliceinfo[slicenum].size = numsects;
			  else
				if (availsects < minsz)
					sliceinfo[slicenum].size = 0;
				else 
					sliceinfo[slicenum].size = availsects;
			  availsects -= sliceinfo[slicenum].size;
			  break;
		case 'M': if (((numsects = dfltsz * oneMBsects) % cylsecs) != 0)
				numsects = (numsects/cylsecs + 1) * cylsecs;
			  if (numsects <= availsects)
				sliceinfo[slicenum].size += numsects;
			  else
				if (availsects < minsz)
					sliceinfo[slicenum].size = 0;
				else
					sliceinfo[slicenum].size = availsects;
			  availsects -= sliceinfo[slicenum].size;
			  break;
		default:  break;
		}
	}
	if (wflag == TRUE) {
		if (availsects > 0) {
			availcyls = availsects / cylsecs;
			totalcyls = availcyls;
			for (i=1; i < V_NUMPAR; i++)
				if (sliceinfo[i].size < 0) {
					n = -(sliceinfo[i].size)*100/totalpcnt;
					reqcyls = (n * totalcyls) / 100;
					if ((reqcyls <= availcyls) &&
					   (reqcyls * cylsecs > sliceinfo[i].minsz))
						sliceinfo[i].size = reqcyls * cylsecs;
					else
						if (availcyls * cylsecs >= sliceinfo[i].minsz)
							sliceinfo[i].size =(sliceinfo[i].minsz/cylsecs + 1) * cylsecs;
						else
							sliceinfo[i].size = 0;
					availcyls -= sliceinfo[i].size/cylsecs;
					availsects -= sliceinfo[i].size;
				}
		}
		else /* W requests made but no sects left, set W slices to 0 */
			for (i=1; i < V_NUMPAR; i++)
				if (sliceinfo[i].size < 0) 
					sliceinfo[i].size = 0;
	}
	/* left over cylinders are added to the root slice */
	if ((availsects > 0) && (availsects > cylsecs)) {
		availcyls = availsects / cylsecs;
		sliceinfo[ROOTSLICE].size += availcyls * cylsecs;
	}
	fclose(defaultsfile);
	close(defaultsfd);
	printf("The following slice sizes are the recommended configuration for your disk.\n");
	for (i=1; i < V_NUMPAR; i++)
		if (sliceinfo[i].size > 0)
			if (sliceinfo[i].mkfsflag > 0)
				printf("A %s filesystem of %ld cylinders (%.1f MB)\n",
				  sliceinfo[i].sname, sliceinfo[i].size/cylsecs,
				  (float)sliceinfo[i].size*(float)dp.dp_secsiz/ONEMB);
			else
				printf("A %s slice of %ld cylinders (%.1f MB)\n",
				  sliceinfo[i].sname, sliceinfo[i].size/cylsecs,
				  (float)sliceinfo[i].size*(float)dp.dp_secsiz/ONEMB);
	for (i=1; i < V_NUMPAR; i++) 
		if ((sliceinfo[i].createflag == TRUE) && 
		   (sliceinfo[i].size == 0)) {
			if (prtflag == TRUE) {
				printf("\nBased on the default size recommendations, disk space was not available\n");
				printf("for the following slices:\n");
				prtflag = FALSE;
			}
			if (sliceinfo[i].mkfsflag > 0) 
				if (sliceinfo[i].reqflag == TRUE) {
					reqslice_err = TRUE;
					printf("The Required %s filesystem was not allocated space.\n",sliceinfo[i].sname);
					printf("This slice is required for successful installation.\n\n"); 
				}
				else
					printf("The %s filesystem.\n",sliceinfo[i].sname);
			else
				if (sliceinfo[i].reqflag == TRUE) {
					reqslice_err = TRUE;
					printf("The Required %s slice was not allocated space.\n",sliceinfo[i].sname);
					printf("This slice is required for successful installation.\n\n"); 
				}
				else
					printf("The %s slice.\n",sliceinfo[i].sname);
		}
	init_pstart = pstart;
	fill_vtoc();
	if (reqslice_err == TRUE) {
		printf("\nThe default layout will not allow all required slices to be created.\n");
		printf("You will be required to designate the sizes of slices to create a\n");
		printf("valid layout for the slices you requested.\n\n");
	}
	else {
		printf("\nIs this configuration acceptable? (y/n) ");
		if (yes_response()) 
			defaults_rejected = FALSE;
	}
	if ((reqslice_err == TRUE) || (defaults_rejected == TRUE)) {
		pstart = init_pstart;
		for (i=1; i < V_NUMPAR; i++) 
			if (sliceinfo[i].createflag && vtoc.v_part[i].p_size) {
				vtoc.v_part[i].p_size = 0;
				vtoc.v_part[i].p_start = 0;
				vtoc.v_part[i].p_flag = 0;
			}
	}
}

/* setup_vtoc will make the calls to first obtain the slice configuration   */
/* info and then obtain the sizes for the slices the user choose.           */
setup_vtoc()
{
	daddr_t init_pstart = pstart;
	int i, define_slices = TRUE;
	short reqslice_err;

	if (defaultsflag == TRUE)
		define_slices = FALSE;
	for (;;) {
		reqslice_err = FALSE;
		if (define_slices == TRUE)
			if (bootdisk)
				get_bootdsk_slices();
			else
				get_slices();	
		get_slice_sizes();
		printf("\nYou have specified the following disk configuration:\n");
		for (i=1; i < V_NUMPAR; i++) {
			if (sliceinfo[i].createflag)
				if (sliceinfo[i].mkfsflag > 0)
					printf("A %s filesystem with %d cylinders (%.1f MB)\n",
					  sliceinfo[i].sname,vtoc.v_part[i].p_size/cylsecs,
					  (float)vtoc.v_part[i].p_size*(float)dp.dp_secsiz/ONEMB);
				else
					printf("A %s slice with %d cylinders (%.1f MB)\n",
					  sliceinfo[i].sname,vtoc.v_part[i].p_size/cylsecs,
					  (float)vtoc.v_part[i].p_size*(float)dp.dp_secsiz/ONEMB);
		/* Go through to set v_nparts to be the total number of */
		/* slices which includes slice 0 */
			if (vtoc.v_part[i].p_size > 0)
				vtoc.v_nparts = i + 1;
			if ((sliceinfo[i].reqflag == TRUE) &&
			   (vtoc.v_part[i].p_size == 0)) {
				printf("Required slice %s was not allocated space.\n", sliceinfo[i].sname);
				reqslice_err = TRUE;
			}
		}
		if (reqslice_err == TRUE) {
			printf("A required slice was not allocated space. You must reallocate the disk space\n");
			printf("such that all required slices are created.\n\n");
		}
		else {
			printf("\nIs this allocation acceptable to you (y/n)? ");
			if (yes_response())
				break;
		}
		if (defaultsflag == FALSE) {
			printf("\nYou have rejected the disk configuration. Do you want\n");
			printf("to redefine the slices to be created? (y/n)? ");
			if (yes_response())
				define_slices = TRUE;
			else
				define_slices = FALSE;
		}
		pstart = init_pstart;
		for (i=1; i < V_NUMPAR; i++) {
			if (sliceinfo[i].createflag && vtoc.v_part[i].p_size) {
				vtoc.v_part[i].p_size = 0;
				vtoc.v_part[i].p_start = 0;
				vtoc.v_part[i].p_flag = 0;
				if (define_slices == TRUE) {
					sliceinfo[i].createflag = 0;
					vtoc.v_part[i].p_tag = 0;
				}
			}
		}
	}
}
int
get_fs_type(req_flag)
int req_flag; /* does the slice require a mkfs type */
{
	if (req_flag == TRUE) 
		for (;;) {
			printf("Which type of filesystem do you want created on this slice? (s5 or ufs) ");
			gets(replybuf);
			if ((strncmp(replybuf,"s52k",4) == 0) ||
		    	    (strncmp(replybuf,"S52K",4) == 0))
				return(S52KMKFS);
			if ((strncmp(replybuf,"s5",2) == 0) ||
		    	    (strncmp(replybuf,"S5",2) == 0))
				return(S51KMKFS);
			if ((strncmp(replybuf,"ufs",3) == 0) ||
		    	    (strncmp(replybuf,"UFS",3) == 0))
				return(UFSMKFS);
			printf("Invalid response - please answer with s5, s52k, or ufs.\n");
		}
	else
		for (;;) {
			printf("Which type of filesystem do you wish to create on this\n");
			printf("slice? (s5, s52k, ufs, or na if no filesystem should be created) ");
			gets(replybuf);
			if ((strncmp(replybuf,"s52k",4) == 0) ||
		    	    (strncmp(replybuf,"S52K",4) == 0))
				return(S52KMKFS);
			if ((strncmp(replybuf,"s5",2) == 0) ||
		    	    (strncmp(replybuf,"S5",2) == 0))
				return(S51KMKFS);
			if ((strncmp(replybuf,"ufs",3) == 0) ||
		    	    (strncmp(replybuf,"UFS",3) == 0))
				return(UFSMKFS);
			if ((strncmp(replybuf,"na",2) == 0) ||
		    	    (strncmp(replybuf,"NA",2) == 0))
				return(0);
			printf("Invalid response - please answer with s5, s52k, ufs, or na.\n");
		}
}

/* get_bootdsk_slices queries the user on their preferences for the setup  */
/* of a boot disk. This allows for setup of root, swap, usr, usr2, dump,   */
/* stand, and home slices.						   */
get_bootdsk_slices()
{
	printf("You will now be queried on the setup of your disk. After you\n");
	printf("have determined which slices will be created, you will be \n");
	printf("queried to designate the sizes of the various slices.\n\n");
	sliceinfo[ROOTSLICE].createflag = TRUE;
	vtoc.v_part[ROOTSLICE].p_tag = V_ROOT;
	sprintf(sliceinfo[ROOTSLICE].sname,"/");
	printf("A root filesystem is required and will be created.\n");
	sliceinfo[ROOTSLICE].mkfsflag = get_fs_type(TRUE);
	sliceinfo[SWAPSLICE].createflag = TRUE;
	vtoc.v_part[SWAPSLICE].p_tag = V_SWAP;
	sprintf(sliceinfo[SWAPSLICE].sname,"/dev/swap");
	sliceinfo[STANDSLICE].createflag = TRUE;
	sprintf(sliceinfo[STANDSLICE].sname,"/stand");
	sliceinfo[STANDSLICE].mkfsflag = BFSMKFS;
	vtoc.v_part[STANDSLICE].p_tag = V_STAND;
	printf("\nDo you wish to have separate root and usr filesystems (y/n)? ");
	if (yes_response()) {
		sliceinfo[USRSLICE].createflag = TRUE;
		vtoc.v_part[USRSLICE].p_tag = V_USR;
		sliceinfo[USRSLICE].mkfsflag = get_fs_type(TRUE);
		sprintf(sliceinfo[USRSLICE].sname,"/usr");
	}
	printf("\nDo you want to allocate a crash/dump area on your disk (y/n)? ");
	if (yes_response()) {
		sliceinfo[DUMPSLICE].createflag = TRUE;
		vtoc.v_part[DUMPSLICE].p_tag = V_DUMP;
		sprintf(sliceinfo[DUMPSLICE].sname,"/dev/dump");
	}
	printf("\nDo you want to create a home filesystem (y/n)? "); 
	if (yes_response()) { 
		sliceinfo[HOMESLICE].createflag = TRUE; 
		vtoc.v_part[HOMESLICE].p_tag = V_HOME; 
		sliceinfo[HOMESLICE].mkfsflag = get_fs_type(TRUE);
		sprintf(sliceinfo[HOMESLICE].sname,"/home");
	} 
	printf("\nDo you want to create a var filesystem (y/n)? "); 
	if (yes_response()) {
		sliceinfo[VARSLICE].createflag = TRUE;
		vtoc.v_part[VARSLICE].p_tag = V_VAR;
		sliceinfo[VARSLICE].mkfsflag = get_fs_type(TRUE);
		sprintf(sliceinfo[VARSLICE].sname,"/var");
	}
	printf("\nDo you want to create a home2 filesystem (y/n)? "); 
	if (yes_response()) { 
		sliceinfo[HOME2SLICE].createflag = TRUE; 
		vtoc.v_part[HOME2SLICE].p_tag = V_HOME; 
		sliceinfo[HOME2SLICE].mkfsflag = get_fs_type(TRUE);
		sprintf(sliceinfo[HOME2SLICE].sname,"/home2");
	} 
}

/* chkname verifies that the name for the slice is a valid mount point  */
/* and is unique.							*/
int
chkname(slicename,cur_index)
char *slicename;
int cur_index;
{
	int i;
	struct stat statbuf;

	if (stat(slicename, &statbuf) == 0)  
		if ((statbuf.st_mode & S_IFMT) != S_IFDIR) {
			fprintf(stderr,"%s is an existing file or device. Please enter a unique pathname.\n",slicename);
			return(1);
		}
	for (i=0; i<cur_index; i++) {
		if (strcmp(sliceinfo[i].sname,slicename) == 0) {
			fprintf(stderr,"%s has already been used. Please enter a unique pathname.\n",slicename);
			return(1);
		}
	}
	return(0);
}

/* get_slices will query the user to collect slices for configuring   */
/* additional disks added to the system. The user will be allowed to  */
/* choose the number of slices desired and the names of them.	      */
get_slices()
{
	long count, len, i, slices;
	long availslices=0;
	char *endptr;

	for (i=1; i < V_NUMPAR; i++)
		if (vtoc.v_part[i].p_size == 0)
			availslices++;
	printf("You will now be queried on the setup of your disk. After you\n");
	printf("have determined which slices will be created, you will be \n");
	printf("queried to designate the sizes of the various slices.\n");
	for (;;) {
		printf("\nHow many slices/filesystems do you want created on the disk (1 - %d)? ", availslices);
		gets(replybuf);
		slices = strtol(replybuf, &endptr, 10);
		if ((replybuf != endptr) &&
		   ((slices > 0) && (slices <= availslices)))
			break;
		printf("Illegal value: %d; try again. \n", slices);
	}
	for (i = 1, count = 1; count <= slices && i < V_NUMPAR; i++) 
		if (vtoc.v_part[i].p_size == 0) {
			printf("\nPlease enter the absolute pathname (e.g., /usr3) for \n");
			printf("slice/filesystem %d (1 - 32 chars)? ",count);
			for (;;) {
				gets(replybuf);
				if (((len = strlen(replybuf) + 1) > SNAMSZ) ||
				   (replybuf[0] != '/')) {
					printf("Illegal value: %s \n",replybuf);
					printf("Value must begin with '/' and contain 32 characters or less.\n");
				}
				else
					if (chkname(replybuf,i) == 0) 	
						break;
			}
			sprintf(sliceinfo[i].sname, "          ");
			strncpy(sliceinfo[i].sname, replybuf, len);
			sliceinfo[i].createflag = TRUE;
			sliceinfo[i].size = 0;
			sliceinfo[i].mkfsflag = get_fs_type(FALSE); 
			vtoc.v_part[i].p_tag = V_USR;
			count++;
		}
}

/* get_slice_sizes will go through the sliceinfo structure to query the */
/* user on the desired slice size. The slices to be queried on will have */
/* the createflag set. Slices which have predetermined sizes (boot and alts */
/* will have been setup in other routines.				*/
get_slice_sizes()
{
	long cyls, i, j;
	long remcyls;
	/* calc cylsper_1mb by rounding up # of cyls >= 1 MB */
	int cylsper_1mb = (ONEMB + cylbytes/2) / cylbytes;
	char *endptr;

	remcyls = ((unix_base + unix_size) - pstart) / cylsecs;
	printf("\nYou will now specify the size in cylinders of each slice.\n");
	printf("(One megabyte of disk space is approximately %d cylinders.)\n", cylsper_1mb);
	for (i=1; i < V_NUMPAR; i++) {
		if (bootdisk)
			j = querylist[i];
		else
			j = i;
		if ((sliceinfo[j].createflag) && (remcyls > 0)) 
			for (;;) {
				if (sliceinfo[j].minsz > 0)
					printf("\nThe recommended minimum size for the %s slice is %d cylinders (%d MB).\n",
					sliceinfo[j].sname,(sliceinfo[j].minsz+cylsecs/2)/cylsecs,
					(sliceinfo[j].minsz*(int)dp.dp_secsiz+ONEMB/2)/ONEMB);
				printf("How many of the remaining cylinders would you like for %s (0 - %d)? ",sliceinfo[j].sname,remcyls);
				gets(replybuf);
				cyls = strtol(replybuf, &endptr, 10);
				if ((replybuf != endptr) &&
				   ((cyls >= 0) && (cyls <= remcyls))) {
					vtoc.v_part[j].p_start = pstart;
					vtoc.v_part[j].p_size = cyls * cylsecs;
					vtoc.v_part[j].p_flag = V_VALID;
					pstart += cyls * cylsecs;
					if (sliceinfo[j].mkfsflag == 0 && vtoc.v_part[j].p_tag != V_USR)
						vtoc.v_part[j].p_flag |=V_UNMNT;
					remcyls -= cyls;
					break;
				}
				printf("Illegal value: %d; try again. \n",cyls);
			}
	}
}

/* issue_mkfs will handle the details of the mkfs exec. The items to be dealt */
/* with include which mkfs, and where mkfs is 				      */
issue_mkfs(slice, rawdev, size, secspercyl)
int slice;
char *rawdev, *size, *secspercyl;
{
	if (sliceinfo[slice].mkfsflag == BFSMKFS)  
		sprintf(replybuf,"/etc/fs/bfs/mkfs %s %s >/dev/null 2>&1",rawdev,size);
	else
		if ((sliceinfo[slice].mkfsflag == S51KMKFS) || 
 	   	   (sliceinfo[slice].mkfsflag == S52KMKFS)) 
			sprintf(replybuf,"/etc/fs/s5/mkfs -b %d %s %s %d %s >/dev/null 2>&1",
				((sliceinfo[slice].mkfsflag == S52KMKFS)? 2048 : 1024), rawdev, size, fsgap(), secspercyl);
		else
			if (scsi_flag == TRUE)
				sprintf(replybuf,"/etc/fs/ufs/mkfs  -C  %s  %s  %d  %d  4096 %d 12 10 60 2048 t 0 4 >/dev/null 2>&1",
				rawdev,size,(int)dp.dp_sectors,(int)dp.dp_heads,
				FRAGSIZE);
			else
				sprintf(replybuf,"/etc/fs/ufs/mkfs  -C  %s  %s  %d  %d  4096 %d 16 10 60 2048 t 0 4 >/dev/null 2>&1",
				rawdev,size,(int)dp.dp_sectors,(int)dp.dp_heads,
				FRAGSIZE);
	if (system(replybuf)) {
		fprintf(stderr,"Disksetup: unable to create filesystem on %s.\n",rawdev);
		perror(errstring);
		fs_error(sliceinfo[slice].sname);
	}
}

create_fs()
{
	int i;
	char secspercyl[5];
	int blkspersec;

	blkspersec = dp.dp_secsiz / 512;
	sprintf(secspercyl, "%d", cylsecs);
	printf("\nFilesystems will now be created on the needed slices\n");
	for (i=1; i < V_NUMPAR; i++) {
		if ((vtoc.v_part[i].p_size > 0) && (sliceinfo[i].mkfsflag >0)) {
			char rawdev[20], size[12];

			if (scsi_flag == TRUE)
				sprintf(rawdev, "%s%x", mkfsname, i);
			else
				sprintf(rawdev, "%s%d", mkfsname, i);
			printf("Creating the %s filesystem on %s \n",sliceinfo[i].sname,rawdev);
			sprintf(size, "%ld", vtoc.v_part[i].p_size*blkspersec);
			issue_mkfs(i, rawdev, size, secspercyl);
			if (sliceinfo[i].mkfsflag != BFSMKFS) 
				label_fs(i, rawdev);
			write_vfstab(i, rawdev);
		}
	}
	close(vfstabfd);
}

label_fs(slice, dev)
int slice;
char *dev;
{
	char disk[7], fsnam[7];

	sprintf(disk,"slic%d",slice);
	if ((int)strlen(sliceinfo[slice].sname) > 6) {
		strncpy(fsnam,sliceinfo[slice].sname,6);
		fsnam[6] = '\0';
	}
	else
		strcpy(fsnam,sliceinfo[slice].sname);
	if ((sliceinfo[slice].mkfsflag == S51KMKFS) || 
  	   (sliceinfo[slice].mkfsflag == S52KMKFS)) 
		sprintf(replybuf,"/etc/fs/s5/labelit %s %s %s >/dev/null 2>&1", dev, sliceinfo[slice].sname, disk);
	else
		sprintf(replybuf,"/etc/fs/ufs/labelit %s %s %s >/dev/null 2>&1", dev, fsnam, disk);
	if (system(replybuf)) {
		fprintf(stderr,"Disksetup: warning unable to label slice %s %s %s\n",dev, fsnam, disk);
		perror(errstring);
	}
}

write_vfstab(slice, dev)
int slice;
char *dev;
{
	char blkdev[20], buf[80], *tmppt;
	int perms, len;
	int found = 0;
	struct stat statbuf;

	sprintf(blkdev, "%s", dev);
	tmppt = blkdev;
	while (*tmppt != NULL) {
		if (*tmppt == 'r' || found) {
			*tmppt = *(tmppt+1);
			found = TRUE;
		}
		tmppt++;
	}
	if (bootdisk && slice == 1) {
		if (sliceinfo[1].mkfsflag == UFSMKFS)
			sprintf(replybuf,"/etc/fs/ufs/mount %s /mnt >/dev/null 2>&1",blkdev);
		else
			sprintf(replybuf,"/etc/fs/s5/mount %s /mnt >/dev/null 2>&1",blkdev);
		if (system(replybuf)) {
			fprintf(stderr,"Disksetup: cannot mount root\n");
			perror(errstring);
			fs_error(sliceinfo[slice].sname);
		}
		if (mkdir("/mnt/etc", 0775) == -1) {
			fprintf(stderr,"Cannot create /mnt/etc.\n");
			perror(errstring);
			fs_error(sliceinfo[slice].sname);
		}
		if ((vfstabfd=open("/mnt/etc/vfstab",O_CREAT|O_WRONLY,0644)) == -1) {
			fprintf(stderr,"Cannot create /etc/vfstab.\n");
			perror(errstring);
			fs_error(sliceinfo[slice].sname);
		}
		else {
			if (sliceinfo[1].mkfsflag == UFSMKFS)
				len = sprintf(buf,
	"/dev/root	/dev/rroot	/	ufs	1	yes	-\n");
			else
				len = sprintf(buf,
	"/dev/root	/dev/rroot	/	s5	1	yes	-\n");
			if (write(vfstabfd, buf, len) != len) {
				fprintf(stderr,"Disksetup: cannot write /etc/vfstab entry.\n");
				perror(errstring);
				fs_error(sliceinfo[slice].sname);
			}
		}
	}
	else {
		if (vfstabfd == 0)
			if ((vfstabfd=open("/etc/vfstab",O_WRONLY|O_APPEND)) == -1) 
				if ((vfstabfd = open("mnt/etc/vfstab", O_WRONLY|O_APPEND)) == -1) {
					fprintf(stderr,"Disksetup: cannot open /etc/vfstab.\n");
					perror(errstring);
					fs_error(sliceinfo[slice].sname);
				}
				else 
					instsysflag = TRUE;
		if (sliceinfo[slice].mkfsflag == BFSMKFS) 
			len = sprintf(buf, "%s	%s	%s	bfs	1	yes	-\n", blkdev, dev, sliceinfo[slice].sname);
		else
			if (sliceinfo[slice].mkfsflag == UFSMKFS) 
				len = sprintf(buf, "%s	%s	%s	ufs	1	yes	-\n", blkdev, dev, sliceinfo[slice].sname);
			else
				len = sprintf(buf, "%s	%s	%s	s5	1	yes	-\n", blkdev, dev, sliceinfo[slice].sname);
		if (write(vfstabfd, buf, len) != len) {
			fprintf(stderr,"Disksetup: cannot write /etc/vfstab entry.\n");
			perror(errstring);
			fs_error(sliceinfo[slice].sname);
		}
		if ((bootdisk == TRUE) || (instsysflag == TRUE)) {
			sprintf(buf, "/mnt%s", sliceinfo[slice].sname);
			if (strncmp(sliceinfo[slice].sname,"/tmp",4) == 0)   
				perms = 01777;
			else 
				perms = 0755;
			if (mkdir(buf, perms) != 0) {
				fprintf(stderr,"Disksetup: could not create %s mount point.\n",buf);
				perror(errstring);
				fs_error(buf);
			}
		}
		else {
			strcpy(buf, sliceinfo[slice].sname);
			sprintf(replybuf,"mkdir -m 0755 -p %s",buf);
			if (stat(buf, &statbuf) == -1)  {
				if (system(replybuf) != 0) {
					fprintf(stderr,"Disksetup: could not create %s mount point.\n",buf);
					perror(errstring);
				}
			}
			else {
				if (!(statbuf.st_mode & S_IFDIR)) {
					fprintf(stderr,"Disksetup: %s is not a valid mount point\n",buf);
					fs_error(buf);
				}
			}
		}
		if (sliceinfo[slice].mkfsflag == BFSMKFS) 
			sprintf(replybuf,"/etc/fs/bfs/mount %s %s >/dev/null 2>&1",blkdev,buf);
		else 
			if ((sliceinfo[slice].mkfsflag == S51KMKFS) || 
			   (sliceinfo[slice].mkfsflag == S52KMKFS)) 
				sprintf(replybuf,"/etc/fs/s5/mount %s %s >/dev/null 2>&1",blkdev,buf);
			else
				sprintf(replybuf,"/etc/fs/ufs/mount %s %s ",blkdev,buf);
		if (system(replybuf)) {
			fprintf(stderr,"Disksetup: unable to mount %s.\n",buf);
			perror(errstring);
			fs_error(sliceinfo[slice].sname);
		}
	}
}

/* Utility routine for turning signals on and off */
set_sig(n)
void (*n)();
{
	signal(SIGINT, n);
	signal(SIGQUIT, n);
	signal(SIGUSR1, n);
	signal(SIGUSR2, n);
}

set_sig_on()
{
	set_sig(SIG_DFL);
}

set_sig_off()
{
	set_sig(SIG_IGN);
}

#ifdef NLTDISK
mod_param()
{
        for (;;) {
                union io_arg ia;
		int i;

                printf("\nDisk parameters currently configured are as follows:\n");
                printf("Number of cylinders: %d\nNumber of heads: %d\nSectors/track: %d\n",          
                       dp.dp_cyls, dp.dp_heads, dp.dp_sectors);
                printf("Is this correct (y/n)? ");
                fflush(stdout);
                gets(replybuf);
                if(replybuf[0] == 'y') break;

                printf("\nYou may only change the number of cylinders on the disk with this program.\n");
                printf("Run the Setup program if the number of heads or sectors/track are incorrect.\n");

                for (;;)
                {
                        printf("Enter number of cylinders on the drive: ");
                        fflush(stdout);
                        gets(replybuf);
                        i = atoi(replybuf);
                        if ((i >= MINCYLS) && (i <= MAXCYLS))
                        {
                                if (i != dp.dp_cyls)
                                        param_override = PARAM_CHANGED;
                                dp.dp_cyls = i;
                     		break;
                        }
                        printf("Illegal value %d entered\n",i);
                }
                ia.ia_cd.ncyl = dp.dp_cyls;
                ia.ia_cd.nhead = dp.dp_heads;
                ia.ia_cd.nsec = dp.dp_sectors;
                ia.ia_cd.secsiz = dp.dp_secsiz;
                if (ioctl(diskfd, V_CONFIG, &ia) == -1)
                {
                        sprintf(replybuf,"Disksetup V_CONFIG call failed on %s",devname);                      
                        perror(replybuf);
                        exit(66);
                }
                if (ioctl(diskfd, V_GETPARMS, &dp) == -1)
                {
                        sprintf(replybuf,"disksetup second V_GETPARMS call failed on %s",devname);             
                        perror(replybuf);
                        exit(67);
                }
        }
}
#endif /* NLTDISK */
