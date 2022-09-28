/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)disksetup:prtvtoc.c	1.3.1.1"

/* The prtvtoc command has three functions, the primary function is to   */
/* display the contents of the pdinfo, the vtoc and the alternates table */
/* as a source of system information. The second purpose for the command */
/* is to output the contents of the vtoc to in a format for later use    */
/* the edvtoc command. The third purpose is output the contents of the   */
/* contents of the pdinfo, vtoc and alternates table into the            */
/* /etc/partitions file for use with the mkpart command. The format of   */
/* be the same format as used in the 3.2 releases			 */

#include <stdio.h>
#include <fcntl.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/vtoc.h>
#include <sys/stat.h>
#include <sys/alttbl.h>
#include <sys/mkdev.h>
#include <sys/hd.h>

char    *devname;		/* name of device */
int	devfd;			/* device file descriptor */
FILE	*vtocfile;
struct  disk_parms   dp;        /* device parameters */
struct  pdinfo	pdinfo;		/* physical device info area */
struct  vtoc	vtoc;		/* table of contents */
struct  alt_info alttbl;	/* alternate sector and track tables */
char    *buf;			/* buffer used to read in disk structs. */
char    errstring[] = "PRTVTOC error";

void
main(argc, argv)
int	argc;
char	*argv[];
{
	static char     options[] = "aef:p";
	extern int	optind;
	extern char	*optarg;
	int	create_partsfile = 0;	/* flag to create /etc/partition */
	int	create_vtocfile = 0;	/* flag to create file for edvtoc */
	int 	prt_alts = 0;		/* flag to print alternates info */
	int 	prt_pdinfo = 0;		/* flag to print pdinfo */
	char	wini_str[] = "DPT_WINI";
	char	scsihd_str[] = "DPT_SCSI_HD";
	char	scsiod_str[] = "DPT_SCSI_OD";
	char 	*typestr;
	minor_t minor_val;
	struct stat statbuf;
	int c, i;

	while ( (c=getopt(argc, argv, options)) != EOF ) {
		switch (c) {
		case 'a':
			prt_alts++;
			break;
		case 'e':
			create_partsfile++;
			break;
		case 'f':
			if ((vtocfile = fopen(optarg, "w")) == NULL) {
				fprintf(stderr,"prtvtoc: unable to open/create %s file\n",optarg);
				exit(1);
			}
			create_vtocfile++;
			break;
		case 'p':
			prt_pdinfo++;
			break;
		default:
			fprintf(stderr,"Invalid option '%s'\n",argv[optind]);
			giveusage();
			exit(1);
		}
	}

		/* get the last argument -- device stanza */
	if (argc != optind+1) {
		fprintf(stderr,"Missing disk device name\n");
		giveusage();
		exit(1);
	}
	devname = argv[optind];
	if (stat(devname, &statbuf)) {
		fprintf(stderr,"prtvtoc invalid device %s, stat failed\n",devname);
		giveusage();
		exit(1);
	}
	if ((statbuf.st_mode & S_IFMT) != S_IFCHR) {
		fprintf(stderr,"prtvtoc: device %s is not character special\n",devname);
		giveusage();
		exit(1);
	}
	minor_val = minor(statbuf.st_rdev);
	if ((minor_val % V_NUMPAR) != 0) {
		fprintf(stderr,"prtvtoc: device %s is not a slice 0 device\n",devname);
		giveusage();
		exit(1);
	}
	if ((devfd=open(devname, O_RDONLY)) == -1) {
		fprintf(stderr,"prtvtoc: open of %s failed\n", devname);
		perror(errstring);
		exit(2);
	}

	if (ioctl(devfd, V_GETPARMS, &dp) == -1) {
		fprintf(stderr,"prtvtoc: GETPARMS on %s failed\n", devname);
		perror(errstring);
		exit(2);
	}

	if ((buf=(char *)malloc(sizeof(alttbl))) == NULL) {
		fprintf(stderr,"prtvtoc: malloc of buffer failed\n");
		perror(errstring);
		exit(3);
	}

	if ( (lseek(devfd, dp.dp_secsiz*VTOC_SEC, 0) == -1) ||
    	   (read(devfd, buf, dp.dp_secsiz) == -1)) {
		fprintf(stderr,"prtvtoc: unable to read pdinfo structure.\n");
		perror(errstring);
		exit(4);
	}
	memcpy((char *)&pdinfo, buf, sizeof(pdinfo));
	if ((pdinfo.sanity != VALID_PD) || (pdinfo.version != V_VERSION)) {
		fprintf(stderr,"prtvtoc: invalid pdinfo block found.\n");
		giveusage();
		exit(5);
	}
	memcpy((char *)&vtoc, &buf[pdinfo.vtoc_ptr%dp.dp_secsiz], sizeof(vtoc));
	if ((vtoc.v_sanity != VTOC_SANE) || (vtoc.v_version != V_VERSION)) {
		fprintf(stderr,"prtvtoc: invalid VTOC found.\n");
		giveusage();
		exit(6);
	}

	if (dp.dp_type == DPT_WINI) {
		if ((lseek(devfd,(pdinfo.alt_ptr/dp.dp_secsiz)*dp.dp_secsiz,0) == -1) ||
	  	   (read(devfd, buf, sizeof(alttbl)) == -1)) {
			fprintf(stderr,"prtvtoc: seeking/reading alternates table failed\n");
			giveusage();
			exit(7);
		}
		memcpy((char *)&alttbl, buf, sizeof(alttbl));
		if ((alttbl.alt_sanity != ALT_SANITY) || 
	   	   (alttbl.alt_version != ALT_VERSION)) {
			fprintf(stderr,"Warning: invalid alternates table found.\n");
			giveusage();
			exit(8);
		}
	}
	/* Either write vtoc file or write vtoc info to screen */
	if (!create_vtocfile && !prt_pdinfo && !prt_alts && !create_partsfile)
		for (i=0; i < V_NUMPAR; i++) 
			if (vtoc.v_part[i].p_flag & V_VALID) {
				printf("slice %d:\t",i);
				printslice(&vtoc.v_part[i]);
			}
	
	if (create_vtocfile) {
		fprintf(vtocfile,"#SLICE	TAG 	FLAGS	START	SIZE\n");
		for (i=0; i < V_NUMPAR; i++) 
			writeslice(i);
		fclose(vtocfile);
	}

	if (prt_pdinfo) {
		printf("\tDevice %s\n",devname);
		if (dp.dp_type == DPT_WINI)
			typestr = wini_str;
		else
			if (dp.dp_type == DPT_SCSI_HD)
				typestr = scsihd_str;
			else
				typestr = scsiod_str;
		printf("device type:\t\t%d (%s)\n",dp.dp_type,typestr);
		printf("cylinders:\t\t%ld\t\theads:\t\t%ld\n",pdinfo.cyls,pdinfo.tracks);
		printf("sectors/track:\t\t%ld\t\tbytes/sector:\t%ld\n",pdinfo.sectors,pdinfo.bytes);
		printf("number of partitions:\t%d",vtoc.v_nparts);
		printf("\t\tsize of alts table:\t%d\n", pdinfo.alt_len);
	}

	if (prt_alts) 
		if (dp.dp_type == DPT_WINI) {
			printalts(&alttbl.alt_sec, 1);
			printalts(&alttbl.alt_trk, 0);
		}
		else 
			printf("The -a option is not supported for this drive\n");

	if (create_partsfile) {
		write_partsfile();
	}
	close(devfd);
	exit(0);
}

writeslice(slice)
int slice;
{
	fprintf(vtocfile,"%2d	0x%x	0x%x	%ld	%ld\n",slice,
		vtoc.v_part[slice].p_tag, vtoc.v_part[slice].p_flag, 
		vtoc.v_part[slice].p_start,vtoc.v_part[slice].p_size);
}

printalts(altptr, sectors)
struct alt_table	*altptr;
int			sectors;
{
	int i, j;


	printf("\nALTERNATE %s TABLE: %d alternates available, %d used\n",
			sectors? "SECTOR" : "TRACK",
			altptr->alt_reserved, altptr->alt_used);

	if (altptr->alt_used > 0) {
		printf("\nAlternates are assigned for the following bad %ss:\n",
				sectors? "sector" : "track");
		for (i = j = 0; i < (int)altptr->alt_used; ++i) {
			if (altptr->alt_bad[i] == -1)
				continue;
			printf("\t%ld -> %ld", altptr->alt_bad[i],
				sectors ? altptr->alt_base + i
					: altptr->alt_base / (daddr_t)dp.dp_sectors + i);
			if ((++j % 3) == 0) printf("\n");
		}
		printf("\n");
	}
	if (altptr->alt_used != altptr->alt_reserved) {
		printf("\nThe following %ss are available as alternates:\n",
				sectors? "sector" : "track");
		for (i = altptr->alt_used, j = 0; i < (int)altptr->alt_reserved; ++i) {
			if (altptr->alt_bad[i] == -1)
				continue;
			printf("\t\t%ld",
				sectors ? altptr->alt_base + i
					: altptr->alt_base / (daddr_t)dp.dp_sectors + i);
			if ((++j % 4) == 0) printf("\n");
		}
		printf("\n");
	}
}


/*  Printslice prints a single slice entry.  */
printslice(v_p)
struct partition *v_p;
{
	daddr_t cylstart, cylsecs;
	double cyllength;

	switch(v_p->p_tag) {
	case V_BOOT:	printf("BOOT\t\t");			break;
	case V_ROOT:	printf("ROOT\t\t");			break;
	case V_SWAP:	printf("SWAP\t\t");			break;
	case V_USR:	printf("USER\t\t");			break;
	case V_BACKUP:	printf("DISK\t\t");			break;
	case V_STAND:	printf("STAND\t\t");			break;
	case V_HOME:	printf("HOME\t\t");			break;
	case V_DUMP:	printf("DUMP\t\t");			break;
	case V_VAR:	printf("VAR\t\t");			break;
	case V_ALTS:	printf("ALTERNATES\t");			break;
	case V_ALTTRK:	printf("ALT TRACKS\t");			break;
	case V_OTHER:	printf("NONUNIX\t\t");			break;
	default:	printf("unknown 0x%x\t",v_p->p_tag);	break;
	}

	printf("permissions:\t");
	if (v_p->p_flag & V_VALID)	printf("VALID ");
	if (v_p->p_flag & V_UNMNT)	printf("UNMOUNTABLE ");
	if (v_p->p_flag & V_RONLY)	printf("READ ONLY ");
	if (v_p->p_flag & V_OPEN)	printf("(driver open) ");
	if (v_p->p_flag & ~(V_VALID|V_OPEN|V_RONLY|V_UNMNT))
		printf("other stuff: 0x%x",v_p->p_flag);
	cylsecs = dp.dp_heads*dp.dp_sectors;
	cylstart = v_p->p_start / cylsecs;
	cyllength = (float)v_p->p_size / (float)cylsecs;
	printf("\n\tstarting sector:\t%ld (cyl %ld)\t\tlength:\t%ld (%.2f cyls)\n",
		v_p->p_start, cylstart, v_p->p_size, cyllength);
}

/*
 * Giveusage ()
 * Give a (not so) concise message on how to use mkpart.
 */
giveusage()
{
	fprintf(stderr,"prtvtoc [-a] [-e] [-p] [-f filename] raw-device\n");
	if (devfd)
		close(devfd);
}


write_partsfile()
{
#define PARTFILE   "/etc/partitions"	/* partitions file for mkpart */
	FILE	*partfile;		/* /etc/partition file */
	int 	partfd;			/* descriptor to creat/open partfile */
	char diskname[7];
	char ptag[10], pname[10];
	char *str1, *str2, *str3;
	int i, ctlnum, drivnum, targnum;

	if (((partfd=open(PARTFILE,O_CREAT|O_WRONLY|O_APPEND,0644)) == NULL) ||
	   ((partfile = fdopen(partfd,"w")) == NULL)) {
		sprintf(buf,"%s opening /etc/partitions",errstring);
		perror(buf);
		exit(9);
	}
	if (devname[10] == 'c')
		if (sscanf(devname,"/dev/rdsk/c%1dt%1dd%1d",&ctlnum,&targnum,&drivnum) != 3) {
			fprintf(stderr,"prtvtoc: can't parse dev to create /etc/partitions file\n");
			exit(10);
		}
		else
			sprintf(diskname,"disk%d%d%d",ctlnum,targnum,drivnum);
	else
		if (devname[10] == '0')
			sprintf(diskname,"disk0");
		else
			sprintf(diskname,"disk01");
			

	/* Now we write out the drive definition to the partition file. */
	fprintf(partfile,
	    "%s:\n    heads = %d, cyls = %d, sectors = %d, bpsec = %d,\n",
 	    diskname,(int)dp.dp_heads,dp.dp_cyls,dp.dp_sectors,dp.dp_secsiz);
	fprintf(partfile,
		"    vtocsec = %d, altsec = %ld, boot = \"/etc/boot\", device = \"%s\"",
		VTOC_SEC, pdinfo.alt_ptr/(daddr_t)dp.dp_secsiz, devname);

	/* Write out any bad sectors */
	if (alttbl.alt_sec.alt_used != 0) {
		fprintf(partfile,",\n    badsec = ( %ld",alttbl.alt_sec.alt_bad[0]);
		for (i=1; i < (int)alttbl.alt_sec.alt_used; i++)
			fprintf(partfile,",\n               %ld",alttbl.alt_sec.alt_bad[i]);
		fprintf(partfile,")");
	}

	/* Write out any bad tracks */
	if (alttbl.alt_trk.alt_used != 0) {
		fprintf(partfile,",\n    badtrk = ( %ld",alttbl.alt_trk.alt_bad[0]);
		for (i=1; i < (int)alttbl.alt_trk.alt_used; i++)
			fprintf(partfile,",\n               %ld",alttbl.alt_trk.alt_bad[i]);
		fprintf(partfile,")");
	}
	fprintf(partfile,"\n\n");

	for (i=1; i < V_NUMPAR; i++) {
		if (vtoc.v_part[i].p_flag & V_VALID) {
			switch(vtoc.v_part[i].p_tag) {
			   case V_BOOT: sprintf(ptag,"BOOT");
					sprintf(pname,"reserved");
					break;
			   case V_ROOT: sprintf(ptag,"ROOT");
					sprintf(pname,"root");
				        break;
			   case V_SWAP:	sprintf(ptag,"SWAP");
					sprintf(pname,"swap");
					break;
			   case V_USR:	sprintf(ptag,"USR");
					if (i == 3)
						sprintf(pname,"usr");
					else
						if (i == 4)
							sprintf(pname,"home");
						else
							sprintf(pname,"home%d",i);
					break;
			   case V_BACKUP: sprintf(ptag,"DISK");
					sprintf(pname,"disk");
					break;
			   case V_STAND: sprintf(ptag,"STAND");
					sprintf(pname,"stand");
					break;
			   case V_HOME:	sprintf(ptag,"HOME");
					sprintf(pname,"home%d",i);
					break;
			   case V_DUMP:	sprintf(ptag,"DUMP");
					sprintf(pname,"dump");
					break;
			   case V_VAR:	sprintf(ptag,"VAR");
					sprintf(pname,"var");
					break;
			   case V_ALTS:	sprintf(ptag,"ALTS");
					sprintf(pname,"alts");
					break;
			   case V_ALTTRK: sprintf(ptag,"ALTTRK");
					sprintf(pname,"trkalt");
					break;
			   case V_OTHER: sprintf(ptag,"OTHER");
					sprintf(pname,"dos");
					break;
			   default:	printf(ptag,"UNKNOWN");	
					sprintf(pname,"unknown");
					break;
			}
			fprintf(partfile,
			"%s:\n\tpartition = %d, start = %ld, size = %ld,\n",
			pname,i,vtoc.v_part[i].p_start, vtoc.v_part[i].p_size);
			fprintf(partfile, "\ttag = %s,", ptag);
			if (vtoc.v_part[i].p_flag & V_UNMNT)
				fprintf(partfile,
					" perm = NOMOUNT, perm = VALID\n\n");
			else
				if (vtoc.v_part[i].p_flag & V_RONLY)
					fprintf(partfile," perm = RO, perm = VALID\n\n");
				else
					fprintf(partfile," perm = VALID\n\n");
		}
	}
	fclose(partfile);
	close(partfd);
	printf("Successfully created/updated /etc/partitions file.\n");
}

