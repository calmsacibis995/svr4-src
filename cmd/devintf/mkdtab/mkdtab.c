/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)devintf:mkdtab/mkdtab.c	1.1.8.1"

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<fcntl.h>
#include	<unistd.h>
#include	<devmgmt.h>
#include	<sys/mkdev.h>
#include	<sys/cram.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/vtoc.h>
#include	<sys/vfstab.h>
#include	<sys/fs/s5filsys.h>
#include	"mkdtab.h"

/*
 * Update device.tab and dgroup.tab to reflect current configuration.
 * Designed so it can be run either once at installation time or after
 * every reboot.  The alias naming scheme used is non-intuitive but
 * is consistent with existing conventions and documentation and with
 * the device numbering scheme used by the disks command.
 * Code borrowed liberally from prtconf, disks and prtvtoc commands.
 */

static struct dpart {
	char	alias[ALIASMAX];
	char	cdevice[MAXPATHLEN];
	char	bdevice[MAXPATHLEN];
	long	capacity;
} dparttab[16];

static int		vfsnum;
static char		putdevcmd[512];
static char		cmd[MAXPATHLEN];
static struct vfstab	*vfstab;
unsigned char		crambuf[2];

static void		ctape(), fdisk(), hdisk(), initialize(), mkdgroups();
static char		*memstr();
static boolean_t	s5part();

main(argc, argv)
int	argc;
char	**argv;
{
	int			i, drivenum, fd;
	struct stat		sb;
	major_t			ctcmajor;

	strcpy(cmd, argv[0]);

	if (geteuid()) {
		fprintf(stderr, "%s: not super user.\n", cmd);
		exit(255);
	}

	/* initialize vfstab in memory */

	if (argc >=2)	/* since only one argument, no need for checks/cases */
		initialize(1);
	else
		initialize(0);

	/*
	 * For AT386, we can only use CRAM to determine how many floppy and
	 * hard disks are configured on the system and their types. The CRAM
	 * configuration information may only be altered via the "set-up"
	 * floppy.
	 *
	 * Using CRAM and the vfstab information, construct device.tab
	 * information for the floppy and hard disks.
	 *
	 */

	if ((fd = open("/dev/cram", O_RDONLY)) < 0) {
		fprintf (stderr, "%s: Can't open /dev/cram\n", cmd);
		exit(1);
	}

/* floppy disk */
	crambuf[0] = DDTB;
	ioctl(fd, CMOSREAD, crambuf);

	/* only two integral devices possible */
#ifdef MBUS
#ifdef MB1
	fdisk(1,0,0);	/* Drive 0 alias diskette1 */
#else
	fdisk(2,0,0);	/* Drive 0 alias diskette1 */
#endif /* MB1 */
#else /* not MBUS */
	fdisk(((crambuf[1] >> 4) & 0x0F),0,0);	/* Drive 0 alias diskette1 */
	fdisk((crambuf[1]        & 0x0F),1,0);	/* Drive 1 alias diskette2 */
#endif

/* hard disk */
	crambuf[0] = FDTB;
	ioctl(fd, CMOSREAD, crambuf);

	/* only two integral devices possible */
	if (((crambuf[1] >> 4) & 0x0F) != NULL)	/* Drive 0 alias disk1 */
		hdisk(0);
	if ((crambuf[1]        & 0x0F) != NULL)	/* Drive 1 alias disk2 */
		hdisk(1);

	close(fd);

	/*
	 * Other devices such as cartridge tape have no CRAM entries.
	 * We can only check for the existence of the special device
	 * directory and create the device.tab entry based soley on this.
	 *
	 */

/* CTC device (WANGTEK only) */
	if (stat("/dev/rmt/c0s0", &sb) == 0)
		ctape();
	/*
	 * Update the dgroup.tab file.
	 */
	mkdgroups();
}


/*
 * Add device table entry for the floppy drive.
 */

static void
fdisk(drvtyp, num, mflag)			/* at386 specific */
int	drvtyp;
int	num;
int	mflag;
{
	char	desc[DESCMAX];		/* "desc=" table keyword value	*/
	char	mdenslist[DESCMAX];	/* floppy density keyword value	*/
	char	tbuf[DESCMAX];		/* scratch buffer		*/
	char	mdensdefault[ALIASMAX];	/* floppy density default value	*/
	char	mediatype[ALIASMAX];	/* "type=" keyword value	*/
	char	display[ALIASMAX];	/* "display=" keyword value	*/
	char	fmtcdevice[MAXPATHLEN];	/* need for spcl case 2 and 4	*/
	int	blocks, inodes, cyl;	/* more keyword values		*/
	int	multiflag = mflag;	/* is set for flops that work	*/
					/* at multiple densities	*/
	switch (drvtyp) {
		case 0:
			return;

		case 1:
			/* 5.25 360 KB Floppy Disk */
			sprintf(dparttab[0].bdevice, FBDEV1, num);
			sprintf(dparttab[0].cdevice, FCDEV1, num);
			strcpy(fmtcdevice, dparttab[0].cdevice);
			blocks=FBLK1;
			inodes=FINO1;
			cyl=FBPC1;

			if (!multiflag) {
				sprintf(desc,  "Floppy Drive %d", num+1);
				strcpy(mediatype, "diskette");
				strcpy(display, "true");
				sprintf(dparttab[0].alias, "diskette%d", num+1);
			}
			else {
	/* if special mdens case */
				multiflag=0;
				strcpy(desc, FDESC1);
				strcpy(mediatype, "mdens");
				strcpy(display, "false");
				sprintf(dparttab[0].alias, FDENS1, num+1);
			}
			break;

		case 2:
			/* 5.25 1.2 MB Floppy Disk */
			blocks=FBLK2;
			inodes=FINO2;
			cyl=FBPC2;

			if (!multiflag) {
				sprintf(dparttab[0].bdevice, FBDEV, num);
				sprintf(dparttab[0].cdevice, FCDEV, num);
				sprintf(fmtcdevice, FCDEV2, num);
				sprintf(desc,  "Floppy Drive %d", num+1);
				strcpy(mediatype, "diskette");
				strcpy(display, "true");
				sprintf(dparttab[0].alias, "diskette%d", num+1);
	/*
	 * Case 2 hardware is special for the AT&T at386
	 * It can behave like types 1, 2 or 5. Type 2 is default.
	 */
				multiflag=2;
				sprintf(tbuf,"\"mdenslist=%s,%s,%s\"", FDENS2, FDENS5, FDENS1);
				sprintf(mdenslist, tbuf, num+1, num+1, num+1);
				sprintf(tbuf,"\"mdensdefault=%s\"", FDENS2);
				sprintf(mdensdefault, tbuf, num+1);
			}

			else {
	/* if special mdens case */
				sprintf(dparttab[0].bdevice, FBDEV2, num);
				sprintf(dparttab[0].cdevice, FCDEV2, num);
				sprintf(fmtcdevice, FCDEV2, num);
				multiflag=0;
				strcpy(desc, FDESC2);
				strcpy(mediatype, "mdens");
				strcpy(display, "false");
				sprintf(dparttab[0].alias, FDENS2, num+1);
			}
			break;

		case 3:
			/* 3.5 720 KB Floppy Disk */
			sprintf(dparttab[0].bdevice, FBDEV3, num);
			sprintf(dparttab[0].cdevice, FCDEV3, num);
			strcpy(fmtcdevice, dparttab[0].cdevice);
			blocks=FBLK3;
			inodes=FINO3;
			cyl=FBPC3;

			if (!multiflag) {
				sprintf(desc,  "Floppy Drive %d", num+1);
				strcpy(mediatype, "diskette");
				strcpy(display, "true");
				sprintf(dparttab[0].alias, "diskette%d", num+1);
			}
			else {
		/* if special mdens case */
				multiflag=0;
				strcpy(desc, FDESC3);
				strcpy(mediatype, "mdens");
				strcpy(display, "false");
				sprintf(dparttab[0].alias, FDENS3, num+1);
			}
			break;

		case 4:
			/* 3.5 1.44 MB Floppy Disk */
			blocks=FBLK4;
			inodes=FINO4;
			cyl=FBPC4;

			if (!multiflag) {
				sprintf(dparttab[0].bdevice, FBDEV, num);
				sprintf(dparttab[0].cdevice, FCDEV, num);
				sprintf(fmtcdevice, FCDEV4, num);
				sprintf(desc,  "Floppy Drive %d", num+1);
				strcpy(mediatype, "diskette");
				strcpy(display, "true");
				sprintf(dparttab[0].alias, "diskette%d", num+1);
	/*
	 * Case 4 hardware is special for the AT&T at386
	 * It can behave like types 3 or 4. Type 4 is default.
	 */
				sprintf(dparttab[0].bdevice, FBDEV, num);
				sprintf(dparttab[0].cdevice, FCDEV, num);
				multiflag=4;
				sprintf(tbuf,"\"mdenslist=%s,%s\"", FDENS4, FDENS3);
				sprintf(mdenslist, tbuf, num+1, num+1, num+1);
				sprintf(tbuf,"\"mdensdefault=%s\"", FDENS4);
				sprintf(mdensdefault, tbuf, num+1);
			}

			else {
	/* if special mdens case */
				sprintf(dparttab[0].bdevice, FBDEV4, num);
				sprintf(dparttab[0].cdevice, FCDEV4, num);
				sprintf(fmtcdevice, FCDEV4, num);
				multiflag=0;
				strcpy(desc, FDESC4);
				strcpy(mediatype, "mdens");
				strcpy(display, "false");
				sprintf(dparttab[0].alias, FDENS4, num+1);
			}
			break;

		case 5:
			/* 5.25 720 KB Floppy Disk */
			sprintf(dparttab[0].bdevice, FBDEV5, num);
			sprintf(dparttab[0].cdevice, FCDEV5, num);
			strcpy(fmtcdevice, dparttab[0].cdevice);
			blocks=FBLK5;
			inodes=FINO5;
			cyl=FBPC5;

			if (!multiflag) {
				sprintf(desc,  "Floppy Drive %d", num+1);
				strcpy(mediatype, "diskette");
				strcpy(display, "true");
				sprintf(dparttab[0].alias, "diskette%d", num+1);
			}
			else {
		/* if special mdens case */
				multiflag=0;
				strcpy(desc, FDESC5);
				strcpy(mediatype, "mdens");
				strcpy(display, "false");
				sprintf(dparttab[0].alias, FDENS5, num+1);
			}
			break;

		default:
			fprintf(stderr, "%s: Warning: type %d type from CRAM for floppy drive %d is unknown.\n", cmd, drvtyp, num);
			return;
			break;
	}

	sprintf(putdevcmd, "/usr/bin/putdev -a %s cdevice=%s bdevice=%s \
desc=\"%s\" mountpt=/install volume=diskette type=%s display=%s removable=true \
capacity=%d fmtcmd=\"/usr/sbin/format -v %s\" \
erasecmd=\"/usr/sadm/sysadm/bin/floperase %s\" copy=true mkdtab=true \
mkfscmd=\"/sbin/mkfs -F s5 %s %d:%d 2 %d\" %s %s",
dparttab[0].alias, dparttab[0].cdevice, dparttab[0].bdevice, desc, mediatype,
display, blocks, fmtcdevice, dparttab[0].cdevice, dparttab[0].bdevice, blocks,
inodes, cyl, mdenslist, mdensdefault);

	(void)system(putdevcmd);

/*
 * If the floppy device is a multi-density device, we must now
 * add the type=mdens entries to device.tab.  We null the values
 * of mdenslist and mdensdefault as they are not appropriate as
 * subdevice values. Fdisk() is recursive (for now).
 */

	if (multiflag) {
		strcpy(mdenslist,"");
		strcpy(mdensdefault,"");

		switch(multiflag) {
			case 2:
				fdisk(2, num, multiflag);	/* HIGH	*/
				fdisk(5, num, multiflag);	/* MED	*/
				fdisk(1, num, multiflag);	/* LOW	*/
				break;

			case 4:
				fdisk(4, num, multiflag);	/* HIGH	*/
				fdisk(3, num, multiflag);	/* LOW	*/
				break;
		}
	}

	return;
}

/*
 * hdisk() gets information about the specified hard drive from the vtoc
 * and vfstab and adds the disk and partition entries to device.tab. If
 * we can't access the raw disk we simply assume it isn't properly configured
 * and we add no entries to device.tab.
 *
 * The scsimkdev command takes care of SCSI devices, not this program
 */

static void
hdisk(drive)				/* at386 specific */
int		drive;
{
	char		cdskpath[MAXPATHLEN];
	char		bdskpath[MAXPATHLEN];
	char		addcmd[BUFSIZ];
	char		displaycmd[BUFSIZ];
	char		*mountpoint;
	int		i, j, dpartcnt, fd;
	struct disk_parms	dp;
	struct pdinfo	pdinfo;
	struct vtoc	vtoc;
	char		*pdbuf;

	sprintf(cdskpath, "/dev/rdsk/%ds0", drive);
	if ((fd = open(cdskpath, O_RDONLY)) == -1) {
		fprintf(stderr, "%s: cannot open %s; entry skipped.\n", cmd, cdskpath);
		return;
	}

	if (drive != 0)
		sprintf(addcmd,"addcmd=\"/sbin/diskadd %d\"", drive);
	else
		strcpy(addcmd,"");

	/*
	 * Begin building the putdev command string that will be
	 * used to make the entry for this disk.
	 * This putdev command will enter all the basic information
	 * for the disk.  More information will be added if the
	 * pdinfo and vtoc can be read.
	 */
	sprintf(bdskpath, "/dev/dsk/%ds0", drive);
	sprintf(displaycmd, "dispdisk %s", cdskpath);
	sprintf(putdevcmd, "/usr/bin/putdev -a disk%d cdevice=%s \
bdevice=%s desc=\"Disk Drive\" type=disk display=true displaycmd=\"%s\" \
remove=true part=true mkdtab=true removable=false %s",\
drive + 1, cdskpath, bdskpath, displaycmd, addcmd);
	(void)system(putdevcmd);

	if (ioctl(fd, V_GETPARMS, &dp) == -1) {
		fprintf(stderr, "%s: ioctl V_GETPARMS failed on drive %d.\n", cmd, drive + 1);
		fprintf(stderr, "Partition information not added to the device table for drive %d.\n", drive + 1);
		return;
	}

	/*
	 * Read pdinfo.
	 */

	if ((pdbuf=(char *)malloc(dp.dp_secsiz)) == NULL) {
		fprintf(stderr, "%s: malloc of pdinfo buffer failed while attempting to identify drive %d.\n", cmd, drive + 1);
		fprintf(stderr, "Partition information not added to the device table for drive %d.\n", drive + 1);
		return;
	}

	if ( ((lseek(fd, dp.dp_secsiz*VTOC_SEC, 0)) == -1) ||
	     ((read(fd, pdbuf, dp.dp_secsiz)) == -1) ) {
		fprintf(stderr, "%s: failed lseek/read of pdinfo for drive %d.\n", cmd, drive + 1);
		fprintf(stderr, "Partition information not added to the device table for drive %d.\n", drive + 1);
		return;
	}

	memcpy((char *)&pdinfo, pdbuf, sizeof(pdinfo));
	if (!(pdinfo.sanity == VALID_PD)) {
		fprintf(stderr, "%s: invalid pdinfo detected for drive %d.\n", cmd, drive + 1);
		fprintf(stderr, "Partition information not added to the device table for drive %d.\n", drive + 1);
		return;
	}

	memcpy((char *)&vtoc, &pdbuf[pdinfo.vtoc_ptr%dp.dp_secsiz], sizeof(vtoc));
	if (!(vtoc.v_sanity == VTOC_SANE)) {
		fprintf(stderr, "%s: insane vtoc detected on drive %d.\n", cmd, drive + 1);
		fprintf(stderr, "Partition information not added to the device table for drive %d.\n", drive + 1);
		return;
	}

	/*
	 * This putdev command will add the additional information
	 */
	sprintf(putdevcmd, "/usr/bin/putdev -m disk%d capacity=%ld dpartlist=",drive + 1, vtoc.v_part[0].p_size);

	/*
	 * Build a table of disk partitions we are interested in and finish
	 * the putdev command string for the disk by adding the dpartlist.
	 */
	dpartcnt = 0;
	for (i = 0; i < (int)vtoc.v_nparts; ++i) {
		if (vtoc.v_part[i].p_size == 0 || vtoc.v_part[i].p_flag !=  (ushort) 512)
			continue;
		sprintf(dparttab[dpartcnt].alias, "dpart%d%02d", drive + 1, i);
		sprintf(dparttab[dpartcnt].cdevice, "/dev/rdsk/%ds%d",
		    drive, i);
		sprintf(dparttab[dpartcnt].bdevice, "/dev/dsk/%ds%d",
		    drive, i);
		dparttab[dpartcnt].capacity = vtoc.v_part[i].p_size;

		if (dpartcnt != 0)
			strcat(putdevcmd, ",");
		strcat(putdevcmd, dparttab[dpartcnt].alias);
		dpartcnt++;
	}
	(void)system(putdevcmd);

	/*
	 * We assemble the rest of the information about the partitions by
	 * looking in the vfstab and at the disk itself.  If vfstab says the
	 * partition contains a non-s5 file system we believe it, otherwise
	 * we call s5part() which will check for an s5 super block on the disk.
	 */
	for (i = 0; i < dpartcnt; i++) {
		for (j = 0; j < vfsnum; j++) {
			if(strcmp(dparttab[i].bdevice,vfstab[j].vfs_special)==0)
				break;
		}
		if (j < vfsnum) {
			/*
			 * Partition found in vfstab.
			 */
			if (strncmp(vfstab[j].vfs_fstype,"s5",2) == 0) {
				/*
				 * Call s5part() but ignore return value. If
				 * s5part() finds a file system it will create
				 * the device.tab entry.  If not, we have a
				 * conflict with what vfstab says so we leave
				 * this partition out of device.tab.
				 */
				(void)s5part(i, vfstab[j].vfs_mountp);
			} else {
				if (strcmp(vfstab[j].vfs_mountp, "-") == 0)
					mountpoint="/mnt";
				else
					mountpoint=vfstab[j].vfs_mountp;
				sprintf(putdevcmd, "/usr/bin/putdev \
-a %s cdevice=%s bdevice=%s desc=\"Disk Partition\" type=dpart removable=false \
capacity=%ld dparttype=fs fstype=%s mountpt=%s mkdtab=true",
dparttab[i].alias, dparttab[i].cdevice, dparttab[i].bdevice,
dparttab[i].capacity, vfstab[j].vfs_fstype, mountpoint);
				(void)system(putdevcmd);
			}
		} else {
			/*
			 * Partition not in vfstab.  See if it's an s5
			 * file system; if not, call it a data partition.
			 */
			if (s5part(i, NULL) == B_FALSE) {
				sprintf(putdevcmd, "/usr/bin/putdev \
-a %s cdevice=%s bdevice=%s desc=\"Disk Partition\" type=dpart removable=false \
capacity=%ld dparttype=dp mkdtab=true", dparttab[i].alias,
dparttab[i].cdevice, dparttab[i].bdevice, dparttab[i].capacity);
				(void)system(putdevcmd);
			}
		}
	}
}


/*
 * Add device table entry for the cartridge tape drive.
 */
static void
ctape()
{
	(void)system("/usr/bin/putdev -a ctape1 cdevice=/dev/rmt/c0s0 \
desc=\"Cartridge Tape Drive\" volume=\"cartridge tape\" \
type=ctape removable=true mkdtab=true capacity=120000 bufsize=20480 \
norewind=\"/dev/rmt/c0s0n\" \
erasecmd=\"/usr/lib/tape/tapecntl -e\" display=true");
}

static void
initialize(flag)
int	flag;			/* if true, re-initialize the table */
{
	FILE		*fp;
	int		i;
	struct vfstab	vfsent;
	char		**olddevlist;

	char	*criteria[] = {
			"type=disk",
			"type=dpart",
			"type=ctape",
			"type=diskette",
			"type=mdens",
			(char *)NULL
		};
	char	*altcriteria[] = {
			"mkdtab=true",
			(char *)NULL
		};

	/*
	 * Build a copy of vfstab in memory for later use.
	 */
	if ((fp = fopen("/etc/vfstab", "r")) == NULL) {
		fprintf(stderr,
		    "%s: can't update device tables:Can't open /etc/vfstab\n",
		     cmd);
		exit(1);
	}

	/*
	 * Go through the vfstab file once to get the number of entries so
	 * we can allocate the right amount of contiguous memory.
	 */
	vfsnum = 0;
	while (getvfsent(fp, &vfsent) == 0)
		vfsnum++;
	rewind(fp);

	if ((vfstab = (struct vfstab *)malloc(vfsnum * sizeof(struct vfstab)))
	    == NULL) {
		fprintf(stderr,"%s: can't update device tables:Out of memory\n",
		    cmd);
		exit(1);
	}

	/*
	 * Go through the vfstab file one more time to populate our copy in
	 * memory.  We only populate the fields we'll need.
	 */
	i = 0;
	while (getvfsent(fp, &vfsent) == 0 && i < vfsnum) {
		if (vfsent.vfs_special == NULL)
			vfstab[i].vfs_special = NULL;
		else
			vfstab[i].vfs_special = memstr(vfsent.vfs_special);
		if (vfsent.vfs_mountp == NULL)
			vfstab[i].vfs_mountp = NULL;
		else
			vfstab[i].vfs_mountp = memstr(vfsent.vfs_mountp);
		if (vfsent.vfs_fstype == NULL)
			vfstab[i].vfs_fstype = NULL;
		else
			vfstab[i].vfs_fstype = memstr(vfsent.vfs_fstype);
		i++;
	}
	(void)fclose(fp);

	/*
	 * If the "-f" flag is passed to mkdtab, remove all current entries
	 * of type disk, dpart, ctape, diskette, and mdens from the device
	 * and device group tables. Otherwise, we will only remove the
	 * entries populated by the mkdtab command. This will preserve
	 * entries created by the user or add-ons like SCSI on each boot.
	 *
	 * Any changes made manually since the last time this command
	 * was run will be lost.  Note that after this we are committed
	 * to try our best to rebuild the tables (i.e. the command
	 * should try not to fail completely after this point).
	 */

	if (flag)
		olddevlist = getdev((char **)NULL, criteria, 0);
	else
		olddevlist = getdev((char **)NULL, altcriteria, 0);

	_enddevtab();	/* getdev() should do this but doesn't */

	for (i = 0; olddevlist[i] != (char *)NULL; i++) {
		(void)sprintf(putdevcmd,"/usr/bin/putdev -d %s 2>/dev/null",
			olddevlist[i]);
		(void)system(putdevcmd);
	}

	for (i = 0; olddevlist[i] != (char *)NULL; i++) {
		(void)sprintf(putdevcmd,"/usr/bin/putdgrp -d %s 2>/dev/null",
			olddevlist[i]);
		(void)system(putdevcmd);
	}
}


/*
 * s5part() reads the raw partition looking for an s5 file system.  If
 * it finds one it adds a partition entry to device.tab using the
 * information passed as arguments and additional info read from the
 * super-block.  Returns B_TRUE if an s5 file system is found, otherwise
 * returns B_FALSE.
 */
static boolean_t
s5part(dpindex, mountpt)
int	dpindex;
char	*mountpt;
{
	int		fd;
	long		lbsize, ninodes;
	struct filsys	s5super;
	char		*mountpoint;
	struct stat	psb, rsb;

	if ((fd = open(dparttab[dpindex].cdevice, O_RDONLY)) == -1)
		return(B_FALSE);

	if (lseek(fd, SUPERBOFF, SEEK_SET) == -1) {
		(void)close(fd);
		return(B_FALSE);
	}

	if (read(fd, &s5super, sizeof(struct filsys)) < sizeof(struct filsys)) {
		(void)close(fd);
		return(B_FALSE);
	}

	(void)close(fd);

	if (s5super.s_magic != FsMAGIC)
		return(B_FALSE);

	switch(s5super.s_type) {

	case Fs1b:
		lbsize = 512;
		ninodes = (s5super.s_isize - 2) * 8;
		break;

	case Fs2b:
		lbsize = 1024;	/* may be wrong for 3b15 */
		ninodes = (s5super.s_isize - 2) * 16; /* may be wrong for 3b15*/
		break;

	case Fs4b:
		lbsize = 2048;
		ninodes = (s5super.s_isize - 2) * 32;
		break;

	default:
		return(B_FALSE);
	}

	if (mountpt != NULL) {
		mountpoint = mountpt;	/* Use mount point passed as arg */
	} else {
		if (strcmp(s5super.s_fname, "root") == 0 &&
		    stat(dparttab[dpindex].bdevice, &psb) == 0 &&
		    stat("/dev/root", &rsb) == 0 &&
		    psb.st_rdev ==  rsb.st_rdev)
			mountpoint = "/";
		else
			mountpoint = "/mnt";
	}
	sprintf(putdevcmd,"/usr/bin/putdev -a %s cdevice=%s bdevice=%s \
desc=\"Disk Partition\" type=dpart mkdtab=true removable=false \
capacity=%ld dparttype=fs fstype=s5 mountpt=%s fsname=%s volname=%s \
lbsize=%ld nlblocks=%ld ninodes=%ld", dparttab[dpindex].alias,
dparttab[dpindex].cdevice, dparttab[dpindex].bdevice,
dparttab[dpindex].capacity, mountpoint, s5super.s_fname, s5super.s_fpack,
lbsize, s5super.s_fsize, ninodes);

	(void)system(putdevcmd);
	return(B_TRUE);
}


/*
 * Update the dgroup.tab file with information from the updated device.tab.
 */
static void
mkdgroups()
{
	int	i;
	char	*criteria[2];
	char	**devlist;

	criteria[1] = (char *)NULL;

	criteria[0] = "type=disk";
	devlist = getdev((char **)NULL, criteria, DTAB_ANDCRITERIA);
	sprintf(putdevcmd, "/usr/bin/putdgrp disk");
	for (i = 0; devlist[i] != (char *)NULL; i++) {
		strcat(putdevcmd, " ");
		strcat(putdevcmd, devlist[i]);
	}
	if (i != 0)
		(void)system(putdevcmd);

	criteria[0] = "type=dpart";
	devlist = getdev((char **)NULL, criteria, DTAB_ANDCRITERIA);
	sprintf(putdevcmd, "/usr/bin/putdgrp dpart");
	for (i = 0; devlist[i] != (char *)NULL; i++) {
		strcat(putdevcmd, " ");
		strcat(putdevcmd, devlist[i]);
	}
	if (i != 0)
		(void)system(putdevcmd);

	criteria[0] = "type=ctape";
	devlist = getdev((char **)NULL, criteria, DTAB_ANDCRITERIA);
	sprintf(putdevcmd, "/usr/bin/putdgrp ctape");
	for (i = 0; devlist[i] != (char *)NULL; i++) {
		strcat(putdevcmd, " ");
		strcat(putdevcmd, devlist[i]);
	}
	if (i != 0)
		(void)system(putdevcmd);

	criteria[0] = "type=diskette";
	devlist = getdev((char **)NULL, criteria, DTAB_ANDCRITERIA);
	sprintf(putdevcmd, "/usr/bin/putdgrp diskette");
	for (i = 0; devlist[i] != (char *)NULL; i++) {
		strcat(putdevcmd, " ");
		strcat(putdevcmd, devlist[i]);
	}
	if (i != 0)
		(void)system(putdevcmd);

	criteria[0] = "type=mdens";
	devlist = getdev((char **)NULL, criteria, DTAB_ANDCRITERIA);
	sprintf(putdevcmd, "/usr/bin/putdgrp mdens");
	for (i = 0; devlist[i] != (char *)NULL; i++) {
		strcat(putdevcmd, " ");
		strcat(putdevcmd, devlist[i]);
	}
	if (i != 0)
		(void)system(putdevcmd);
}


static char *
memstr(str)
register char	*str;
{
	register char	*mem;

	if ((mem = (char *)malloc((uint_t)strlen(str) + 1)) == NULL) {
		fprintf(stderr,"%s: can't update device tables:Out of memory\n",
		    cmd);
		exit(1);
	}
	return(strcpy(mem, str));
}
