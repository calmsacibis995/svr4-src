/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mkpart:mkpart.c	1.1.1.1"

/*
 *	Mkpart - includes support for the SCSI Support package
 *
 *	Mkpart is a maintenance program for UNIX System V.3 disks, disk vtocs,
 *	and partitions.  The program allows the user to list and update the
 *	volume table of contents (vtoc), partitions, alternate blocks
 *	table, write the boot block, OR format a given disk.
 *
 *	The command line specifies which disk device the program operates on,
 *	and what functions are to be performed.  The description of the device
 *	characteristics, the boot program (if any), and the partitions and their
 *	characteristics are obtained from the partition description file.  For
 * 	a description of the command line parameters, read giveusage().
 *
 *      The partition file is a parameterized english-like description of
 *	characteristics.  They are grouped together in named stanzas, and can
 *	reference other stanzas.  Their syntax is described by the YACC grammar
 */


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fs/s5param.h>
#include <a.out.h>
#include <fcntl.h>
#include "parse.h"
#include "mkpart.h"
#include <sys/vtoc.h>
#include <sys/alttbl.h>
#include <sys/hd.h>
#include <sys/fdisk.h>
#include <libelf.h>
#include <sys/sdi_edt.h>

extern int errno;

char		*partfile = PARTFILE;	/* default partition file */
char		*bootfile;		/* bootstrap file */
char		*devicestanza;		/* physical device stanza name */
char		bootonly;		/* -[bB] flag */
char		format;			/* -F format device flag */
char            intlv;			/* interleave factor from -F arg */
char            initialize;             /* -i flag */
char            verify;                 /* -v flag */
char            *wpfile;                /* file to write for -x flag */
#define VER_WRITE  1
#define VER_READ   2
char		targ;			/* -t subflags */
#define TF_ALTS         0x01                    /* list alternates table */
#define TF_PARTS	0x04			/* list partition info */
#define TF_VTOC		0x08			/* list vtoc info */
#define TF_WPART        0x10                    /* generate a partitions file */

FILE		*input;			/* partition file input stream */

struct intlv_entry	{
	unsigned char flag;
	unsigned char secno;
} intlvtab[64];	/* buffer for interleave table - max 64 sec/track */

int     	MaxUseDepth;		/* Limit for mutual USE checking */

devstanza 	*mydev;		/* Device stanza that we work on */
int		devfd;		/* ...and its file descriptor */
int		bootfd;		/* boot file descriptor */

struct disk_parms   dp;         /* device parameters */
struct pdinfo	pdinfo;		/* physical device info area */
struct vtoc	vtoc;		/* table of contents */
struct alt_info alttbl;		/* alternate sector and track tables */
daddr_t         verbads[MAX_ALTENTS]; /* bad sectors found in verify */
FILHDR		filehdr;	/* see filehdr.h */
AOUTHDR		aouthdr;	/* see aouthdr.h */
SCNHDR		scnhdr;		/* see scnhdr.h */

/* structures to be used for the loading of an ELF bootable */
Elf		*elfd;		/* ELF file descriptor */
Elf32_Ehdr	*ehdr;		/* ELF executable header structure */
Elf32_Phdr	*ephdr, *phdr;	/* ELF program header structure */
int		elfboot = 0;    /* flag to designate either ELF or COFF boot */

node		*addparts,	/* List of partitions to add */
		*subparts,      /* List of partitions to remove */
		*badsecs;       /* List of bad sectors to add */
daddr_t         pd_sector;      /* sector from which pdinfo is read/written */

struct absio	absbuf;		/* for RDABS and WRABS ioctl calls */
struct mboot    mboot;          /* fdisk partition table */
struct ipart    fd_parts[FD_NUMPART],
		*unix_part;

/* First_alt - Flag.
 *	If value is 1, we are establishing the alternates table
 *	for the first time.  in that case, DON'T use the V_ADDBAD
 *	ioctl, but DO update table on disk, directly, and use ioctl
 *	with V_REMOUNT to inform the driver to pick up the new table.
*/
short int	First_alt=0;

/** The opening of the device was moved from getdevice() to a new **/
/** subroutine called mkpartopen(). When the dev is open it sets  **/
/** this flag (dev_fd_open) to 1. Additionally, mkpartclose will set the **/
/** the flag to 0 to indicate the device is closed.		  **/

int dev_fd_open = 0;

#define SCSIFORMAT "scsiformat"
#define SCSIVERIFY "scsiformat"
#define SCSIALTS   "scsihdfix"
#define SCSICMDS   "/etc/scsi"

/** "is_scsi_dev" is set to 1 if the device is a scsi **/
/** This is set in the getdevice() routine **/

int is_scsi_dev = 0;

/** the variable not_for_scsi is set whenever the command line **/
/** contains an option illegal for the scsi implementation **/

int not_for_scsi = 0;

main(argc, argv)
int	argc;
char	**argv;
{
static char     options[] = "p:P:bB:f:t:F:A:ivVx:";
extern char	*optarg;
extern int	optind;
int		c;
symbol		*dev;
int		hasvtoc = 0;
int		hasboot = 0;

	initnodes();	/* init first allocation of parse nodes */

	/*
	 * Mkpart recognizes the following command line arguments:
	 * mkpart [-f file] {-p part}* {-P part}* [-b] [-B file] [-F interleave]
	 *      {-A sector}* [-i] [-v] [-t [vpab]] dev
	 *
	 * Run with only dev specified, mkpart initializes the device with
	 * the boot program (or none) given in dev, and a vtoc with one
	 * partition that spans the disk.  By specifying one or more -P's,
	 * more partitions can be added.
	 *
	 * dev is a device stanza name in the partition file (see -f below).
	 *	It specifies (and/or refers to other device stanzas that
	 *	specify) all of the device characteristics.
	 * -f specifies the partition and device specification stanza file.
	 *	If not present, /etc/partitions is assumed.
	 * -p removes a partition from the vtoc on the specified device.  It
	 *	is removed by it's partition number; no comparisons are made
	 *	by attribute.  Part is a stanza name that contains or refers
	 *	to a stanza that contains a partno parameter.
	 * -P adds a partition to the vtoc on the specified device.  Part is
	 *	a stanza which contains and/or refers to other stanzas that
	 *	contain all of the necessary parameters for a vtoc partition.
	 * -b causes only the boot program to be updated, unless other options
	 *	are specified.
	 * -B specifies a different boot program than the one given by the
	 *	device stanza.
	 * -F causes the device to be formatted with specified interleave
	 *      factor. This flag cannot be used in conjuction with others.
	 *	Care is taken to preserve bad sector/track information.
	 * -t creates a listing of the current vtoc.  The sub parameters
	 *	specify pieces to be printed: a - alternate sectors, b - bad
	 *	sectors, p - partitions, and v - vtoc and related structures.
	 *	The absence of sub parameters implies them all.  Sub params
	 *	a and b are NOT CURRENTLY SUPPORTED.
	 * -A adds the specified absolute sector to the alternates table
	 *      if there is a spare alternate sector to use for it, both
	 *      on the disk and in core (with ioctl).
	 * -i causes the device to be completely initialized.  This will
	 *      ignore any existing vtoc, partitions, or alternates and
	 *      will start from scratch.
	 * -v causes a verification pass to be run on the entire drive
	 *      looking for bad sectors.  If not specified, only bad
	 *      sectors from the device stanza or -A arguments will be
	 *      added (if necessary).  If specified, it will add any bad
	 *      sectors found to those already known.  THIS OPTION WILL
	 *      ADD CONSIDERABLE TIME TO THE RUNNING OF MKPART!
	 */
	while ( (c=getopt(argc, argv, options)) != EOF ) {
		switch (c) {
		case 'B':
			bootfile = optarg;
			break;

		case 'b':
			bootonly++;
			break;

		case 'v':
			not_for_scsi = 1;
			verify |= VER_READ;
			break;
		case 'V':
			not_for_scsi = 1;
			verify |= (VER_READ | VER_WRITE);
			break;
		case 'i':
			initialize++;
			break;
		/*
		 * For adding partitions, we build a parse list of the
		 * specified partition stanza names under addparts.
		 */
		case 'P':
			if (addparts) {
				node *newpart = newnode(LIST);

				newpart->ListRef = (void *)optarg;
				newpart->ListNext = addparts;
				addparts = newpart;
			} else {
				addparts = newnode(LIST);
				addparts->ListRef = (void *)optarg;
			}
			break;

		/*
		 * For removing partitions, we build a parse list of the
		 * specified partition stanza names under subparts.
		 */
		case 'p':
			if (subparts) {
				node *newpart = newnode(LIST);

				newpart->ListRef = (void *)optarg;
				newpart->ListNext = subparts;
				subparts = newpart;
			} else {
				subparts = newnode(LIST);
				subparts->ListRef = (void *)optarg;
			}
			break;

		case 'A':
			{
			node * newalt = newnode(LIST);
			node * newnum = newnode(NUMBER);

			newnum->Number = atol(optarg);
			newalt->ListRef = (void *)newnum;
			newalt->ListNext = badsecs;
			badsecs = newalt;

			not_for_scsi = 1;

			break;
			}

		case 'F':
			{
			long i = atol(optarg);
			struct stat	statbuf;

			not_for_scsi = 1;

			format++;
			if ((i < 0) || (i > 16)) {
			    fprintf(stderr,"Illegal interleave specified\n");
			    exit(65);
			}
			intlv = (char) i;
			if (stat(argv[optind],&statbuf) == -1) {
				perror(argv[optind]); giveusage(1);
				exit(1);
			} else if ( !(statbuf.st_mode & S_IFCHR) ) {
				fprintf(stderr,"%s: not character special device\n",
						argv[optind]);
				giveusage(1);
				exit(1);
			}
			break;
			}

		case 'f':
			partfile = optarg;
			break;

		case 'x':
			wpfile = optarg;
			targ |= TF_WPART;
			break;
		case 't':
			for( ; *optarg; optarg++ ) {
				switch( *optarg ) {
				case 'a':
					not_for_scsi = 1;
					targ |= TF_ALTS;	break;
				case 'p':
					targ |= TF_PARTS;	break;
				case 'v':
					targ |= TF_VTOC;	break;
				case 'd':
					not_for_scsi = 2;
					targ = TF_ALTS|TF_PARTS|TF_VTOC;
					optarg--;
					optind--;
					break;
				default:
					goto badopt;
				}
				if (targ == TF_ALTS|TF_PARTS|TF_VTOC)
					break;
			}
			if (!targ) {
				not_for_scsi = 2;
				targ = TF_ALTS|TF_PARTS|TF_VTOC;
			}
			break;

		default:
	badopt:
			fprintf(stderr,"Invalid option '%s'\n",argv[optind]);
			giveusage(0);
			exit(1);
		}
	}

		/* get the last argument -- device stanza */
	if (argc != optind+1) {
		fprintf(stderr,"Missing or bad device stanza name\n");
		giveusage(0);
		exit(2);
	}

	devicestanza = argv[optind];

	/* Special case for low-level format.
	 * The 'device' is an actual UNIX System device name
	 * (e.g. /dev/rdsk/0s0) instead of a device stanza reference.
	 *
	 * Formatting will erase the fdisk table in abs sector 0.
	 * It is, therefore, inappropriate to do anything else
	 * after this operation, before another fdisk table
	 * is established with the 'fdisk' command.
	*/
	if (format) {
		(void)getdevice(devicestanza);
		formatdevice();
		exit(0);
	}


		/* allow some crazy to type his own stanza file at us */
	if (partfile[0] == '-' && !partfile[1]) {
		input = stdin;
	} else {
		if ( !(input = fopen(partfile, "r")) ) {
			fprintf(stderr,"mkpart: ");
			perror(partfile);
			exit(3);
		}
	}

		/* Parse the entire stanza file.  See partitions.y */
	if (yyparse()) {
		fprintf(stderr,"Exiting due to stanza file errors\n");
		exit(4);
	}

	/*
	 * Now we should have a definition of the device stanza.  Look it
	 * up, collect all info from refered-to stanzas into global mydev.
	 */
	if ( !( (dev = lookup(devicestanza))->flags & SY_DEFN ) ) {
		fprintf(stderr,"Device stanza '%s' not defined in file %s\n",
			devicestanza, partfile);
		exit(5);
	} else if ( ((stanza *)(dev->ref))->dev.ds_type != S_DEVICE) {
		fprintf(stderr, "Stanza '%s' is not a device stanza in file %s.\n",
				devicestanza, partfile);
		exit(5);
	} else {
		mydev = (devstanza *)newdevstanza();
		buildstanza(mydev,dev);
	}

	/*
	 * Add any bad sectors specified as 'A' options to those found
	 * in the device stanza.
	 */
	if (mydev->ds_badsec)
		{
		node *np = mydev->ds_badsec;

		while (np->ListNext)
			np = np->ListNext;
		np->ListNext = badsecs;
		}
	else mydev->ds_badsec = badsecs;

	/*
	 * Open boot file and read in filehdr and aouthdr.  returns true if
	 * there is a valid boot program.  Filehdr.f_nscns and
	 * aouthdr.[tdb]size fields are set correctly in any event.
	 */
	hasboot = getboot();

	/*
	 * Read data structures off device to get current values for dp,
	 * [ivlab,] pdinfo, vtoc, and alttbl.  hasvtoc is true if we could get
	 * them, otherwise we assume an uninitialized device and fill in some
	 * reasonable values.
	 */
	hasvtoc = getdevice(mydev->ds_device);

	if (verify) {
		verifydevice();
	}

	if (targ) {
		printdevinfo(targ);
	}

		/* don't do anything else if only asked for a listing */
	if (!targ || addparts || subparts) {

		/* fill in drtab and boot related structs */
		builddevice(mydev);

		updateparts();  /* update vtoc partitions */
		/* only update alts if installing initial system.
		 * Otherwise, let the dynamic bad block handler
		 * mind the store.
		*/
		if (First_alt && !is_scsi_dev)
			updatealts();   /* update alternates table */
		updatedevice(); /* update pdinfo, vtoc base */

		if (hasboot) {
			writeboot();
		} else if (bootonly) {
			fprintf(stderr,"Warning: null bootstrap specified\n");
		}

		if (!bootonly || addparts || subparts) {
			writevtoc();
		}
		return(0);
	}
}

/*
 * Getdevice( device name )
 * returns boolean	true -> could get valid disk structs off device
 *			false-> un-vtoc-ed device
 * Verifies that the device specified is a character special.  Then it
 * gets device info from V_GETPARMS ioctl into global dp.
 * Read data into global structs pdinfo, vtoc,
 * and alttbl.  If the device isn't char special or the V_GETPARMS fails,
 * give a message and quit.  Otherwise, if a seek or read fails, fill in
 * all of the structs with default values built (or calculated) from
 * dp.
 */
int
getdevice(name)
char *name;
{
struct stat	statbuf;
unsigned long	l;
int	i;
char            *buf;

	if(!dev_fd_open)
		if(mkpartopen(name))
			exit(1);


	if (ioctl(devfd, V_GETPARMS, &dp) == -1) {
		fprintf(stderr,"GETPARMS on ");
		perror(name);
		exit(1);
	}

	/*
	 * if the parameters returned from the controller don't agree with
	 * the values in the device stanza, attempt to V_CONFIG the device
	 * and re-get parameters.
	 * NOTE: If we're going to create a partitions-type file (-x flag)
	 * don't do any of this, 'cuz user may not be sure what things
	 * really look like!
	 */

	if (!format && !(targ & TF_WPART) &&
	    ((mydev->ds_heads != dp.dp_heads) ||
	     (mydev->ds_cyls != dp.dp_cyls) ||
	     (mydev->ds_sectors != dp.dp_sectors) ||
	     (mydev->ds_bpsec != dp.dp_secsiz)))
		{
		union io_arg ia;

		fprintf(stderr,
			"WARNING: device stanza parameters do not match driver values!\n");
		fprintf(stderr,"Attempting to reconfigure drive.\n");
		ia.ia_cd.ncyl = mydev->ds_cyls;
		ia.ia_cd.nhead = mydev->ds_heads;
		ia.ia_cd.nsec = mydev->ds_sectors;
		ia.ia_cd.secsiz = mydev->ds_bpsec;
		if (ioctl(devfd, V_CONFIG, &ia) == -1)
			{
			fprintf(stderr,
				"Configure Drive request FAILED!\n");
			exit(66);
			}
		if (ioctl(devfd, V_GETPARMS, &dp) == -1)
			{
			fprintf(stderr,"second GETPARMS on ");
			perror(name);
			exit(67);
			}
		}

	/*
	 * If we're supposed to format the disk, we have all the info
	 * we need, (V_PARM), so just return.
	*/
	if (format) return 0;

	if ((buf=malloc(dp.dp_secsiz)) == NULL)
		{
		fprintf(stderr,"mkpart: malloc of buffer failed\n");
		exit(69);
		}

	pd_sector = mydev->ds_vtocsec;
	absbuf.abs_sec = 0;
	absbuf.abs_buf = (char *)&mboot;
	if( ((i=ioctl(devfd, V_RDABS, &absbuf)) == -1) ||
			(mboot.signature != MBB_MAGIC) )
	{
		if (i >= 0)
		   fprintf(stderr,"Error: invalid fdisk partition table found\n");
		else {
		   fprintf(stderr,"Reading fdisk partition table: ");
		   perror(name);
		}
		exit(1);
	}
	/* copy the partition stuff into fd_parts */
	COPY(fd_parts[0],mboot.parts[0],sizeof(struct ipart)*FD_NUMPART);

	/* find an active UNIX System partition */
	unix_part = NULL;
	for (i=0; i<FD_NUMPART; i++) {
		if (fd_parts[i].systid == UNIXOS) {
			if (fd_parts[i].bootid == ACTIVE || unix_part == NULL)
				unix_part = (struct ipart *)&fd_parts[i];
		}
	}
	if (unix_part == NULL) {
		fprintf(stderr, "No UNIX System partition in fdisk table!\n");
		exit(1);
	}

	if (initialize) {	/* Make sure we don't have an old pdinfo */
		absbuf.abs_sec = unix_part->relsect + pd_sector;
		((struct pdinfo *)absbuf.abs_buf)->sanity = 0;
		ioctl(devfd, V_WRABS, &absbuf);
		remount();	/* make sure driver knows about it */
	}

	/*
	 * If we're supposed to initialize drive, don't even bother trying
	 * to read structures from the disk.
	 */

	if (initialize) goto makedflt;

	if ( (lseek(devfd, dp.dp_secsiz*pd_sector, 0) == -1) ||
			(read(devfd, buf, dp.dp_secsiz) == -1)	) {
		fprintf(stderr,"Warning: seeking/reading pdinfo: ");
		perror(name);
		goto makedflt;
	}
	COPY(pdinfo, buf[0], sizeof(pdinfo));
	if(pdinfo.sanity != VALID_PD)
		{
		fprintf(stderr,"Warning: invalid pdinfo block found -- initializing.\n");
		goto makedflt;
		}
	if ( (lseek(devfd, (pdinfo.vtoc_ptr/dp.dp_secsiz)*dp.dp_secsiz, 0) == -1) ||
			(read(devfd, buf, dp.dp_secsiz) == -1)	) {
		fprintf(stderr,"Warning: seeking/reading VTOC: ");
		perror(name);
		goto makedflt;
	}
	COPY(vtoc, buf[pdinfo.vtoc_ptr%dp.dp_secsiz], sizeof(vtoc));
	if(vtoc.v_sanity != VTOC_SANE)
		{
		fprintf(stderr,"Warning: invalid VTOC found -- initializing.\n");
		goto makedflt;
		}
	/* The following is okay for both scsi and esdi */
	/* enlarge 'buf' to hold largest possible alternates table */
	if ((buf=realloc(buf, sizeof(alttbl))) == NULL)
		{
		fprintf(stderr,"mkpart: realloc of buffer failed\n");
		exit(69);
		}

	/** Do not attempt any alt handling for Scsi. Instead, NULL the **/
	/** ptr and set the altlen to 0.			  	**/

	if(!is_scsi_dev)
	{
		if ( (lseek(devfd, (pdinfo.alt_ptr/dp.dp_secsiz)*dp.dp_secsiz, 0) == -1) ||
				(read(devfd, buf, sizeof(alttbl)) == -1)	) {
			fprintf(stderr,"Warning: seeking/reading alternates table: ");
			perror(name);
			goto makedflt;
		}
		COPY(alttbl, buf[pdinfo.alt_ptr%dp.dp_secsiz], sizeof(alttbl));
		if (alttbl.alt_sanity != ALT_SANITY ||
			alttbl.alt_version != ALT_VERSION) {
			fprintf(stderr,"Warning: invalid alternates table found -- initializing.\n");
			goto makedflt;
		}
	}
	else
	{
		/** SCSI **/
		pdinfo.alt_ptr = (long)0;
		pdinfo.alt_len = 0;
	}
	return 1;	/* Successfully read all structures */

makedflt:
	/*
	 * Fill in all of the data structures as best as can be done.  Since
	 * the device has no info out there, we assume that the user will
	 * be specifying this stuff later on from device and partition stanzas.
	 */

	pd_sector = mydev->ds_vtocsec;
	/* Initialize pdinfo structure */
	pdinfo.driveid = 0;		/* reasonable default value	*/
	pdinfo.sanity = VALID_PD;
	pdinfo.version = V_VERSION;
	strncpy(pdinfo.serial, "            ", sizeof(pdinfo.serial));
	pdinfo.cyls = dp.dp_cyls;
	pdinfo.tracks = dp.dp_heads;
	pdinfo.sectors = dp.dp_sectors;
	pdinfo.bytes = dp.dp_secsiz;
	pdinfo.logicalst = dp.dp_pstartsec;
	pdinfo.vtoc_ptr = dp.dp_secsiz * pd_sector + sizeof(pdinfo);
	pdinfo.vtoc_len =  sizeof(vtoc);


	/** Zero the alt_ptr and alt_len for scsi **/
	if(!is_scsi_dev)
	{
		pdinfo.alt_ptr = dp.dp_secsiz * (pd_sector + 1);
		pdinfo.alt_len = sizeof(alttbl);
	}
	else
	{
		pdinfo.alt_ptr = (long)0;
		pdinfo.alt_len = 0;
	}
	/* Initialize vtoc */
	vtoc.v_sanity = VTOC_SANE;
	vtoc.v_version = V_VERSION;
	strncpy(vtoc.v_volume, mydev->ds_device, sizeof(vtoc.v_volume));
	vtoc.v_nparts = 1;
	vtoc.v_part[0].p_tag = V_BACKUP;
	vtoc.v_part[0].p_flag = V_UNMNT | V_VALID;
	vtoc.v_part[0].p_start = unix_part->relsect;
	vtoc.v_part[0].p_size = unix_part->numsect;

	/** Zero the new SCSI timstamp element **/
	/** Do it for both scsi and esdi in    **/
	/** case esdi wants to do something    **/
	/** with it later.		       **/

	vtoc.timestamp[0] = (long)0;

	/* Build empty alternates table for ESDI ONLY */

	if(!is_scsi_dev)
	{
		memset((char *)&alttbl, 0, sizeof(alttbl));
		alttbl.alt_sanity = ALT_SANITY;
		alttbl.alt_version = ALT_VERSION;
	}
	return 0;	/* couldn't get real data, built defaults */
}

/*
 * Getboot ()
 * returns boolean	true -> found a valid bootstrap a.out and read its
 *				filehdr and aouthdr.
 *			false-> initialized filehdr and aouthdr to 0 sections
 *				and 0 sized text, data, bss.
 */
int
getboot()
{
	int len;

	if (bootfile) { /* if -B, overwrite boot filename gotten from stanza */
		mydev->ds_boot = bootfile;
	}

	if (!mydev->ds_boot || !mydev->ds_boot[0]) { /* check for null name */
		goto noboot;
	}
	if ((bootfd = open(mydev->ds_boot, O_RDONLY)) == -1 ||
			(len = read(bootfd, (char *)&filehdr, FILHSZ)) != 0 &&
			len != FILHSZ) {
		fprintf(stderr,"Opening/reading boot file ");
		perror(mydev->ds_boot);
		exit(30);
	} else if (!len) {	/* boot file was empty! */
noboot:
		filehdr.f_nscns = 0;
		aouthdr.tsize = aouthdr.dsize = aouthdr.bsize = 0;
		return 0;	/* no bootstrap found */
	}

	if (!ISCOFF(filehdr.f_magic)) {
		/* Not COFF boot, check if ELF Format */
		lseek(bootfd, 0, 0);
		if (elf_version (EV_CURRENT) == EV_NONE) {
			fprintf (stderr, "ELF access library out of date\n");
			exit (31);
		}
		if ((elfd = elf_begin (bootfd, ELF_C_READ, NULL)) == NULL) {
			fprintf (stderr, "can't elf_begin: %s\n", elf_errmsg (0));
			exit (31);
		}
		if ((ehdr = elf32_getehdr (elfd)) == NULL) {
			elf_end (elfd);
			fprintf(stderr,"Invalid Boot file, not ELF or COFF executable \n");
			exit(31);
		}
		else {
			elfboot = 1;
			return 1; /* ELF bootstrap ok */
		}
	}

	if (filehdr.f_opthdr > 0) {
		if(read(bootfd,(char *)&aouthdr,filehdr.f_opthdr) !=
							filehdr.f_opthdr) {
			fprintf(stderr,"Reading optional header from boot file ");
			perror(mydev->ds_name->name);
			exit(30);
		}
	}

	return 1;	/* bootstrap looks ok */
}

/*
 * Formatdevice()
 */
void
formatdevice()
{
long    numtrks = dp.dp_heads * dp.dp_cyls;
long    curtrk;
daddr_t cursec;
union io_arg    ia;
int     dotpos = 0;
int     dotcnt = 0;
int     retrycnt,i;
char    resp[10];
char    *rptr;

short	knt_bad = 0,
	knt_marked=0;

struct	{
	unsigned	track : 1;	/* bad track flag */
	unsigned	sector : 1;	/* bad sector flag */
} bad;

extern struct intlv_entry intlvtab[];	/* buffer for interleave table */
extern char intlv;	/* interleave factor from cmd line */

char *interleave();
int sec_verify();

printf("\nABOUT TO FORMAT ENTIRE DRIVE.  THIS WILL DESTROY ANY DATA\n");
printf("on %s.  Continue (y/n)? ", devicestanza);
rptr = gets(resp);
if (!rptr || !((resp[0] == 'Y') || (resp[0] == 'y'))) return;

fprintf(stderr,"\nAttempting to format %ld tracks.\n\nFormatting ", numtrks);

ia.ia_fmt.num_trks  = 1;	/* # sectors to format */
ia.ia_fmt.intlv = intlv;		/* set interleave */

for (curtrk=0; curtrk<numtrks; curtrk++)
	{
	ia.ia_fmt.start_trk = (ushort) curtrk;

	cursec = curtrk * dp.dp_sectors;

	bad.track = 0;

	/* check for sectors that are marked bad */
	if (sec_verify(cursec, dp.dp_sectors, 1) != 0) {

		/* check for a marked bad block */
		for (bad.track = 1, bad.sector=0, i=0; i < dp.dp_sectors; i++) {
			if (sec_verify(cursec+i, 1, 1) != BAD_BLK)
				bad.track = 0;
			else
				bad.sector = 1;

			if ( !bad.track && bad.sector)
				break;
		}
		if(bad.track) {
			++knt_bad;
			fprintf(stderr, "\rTrack %6ld bad        \nFormatting ",
					curtrk);
		}
		else if(bad.sector) {
			++knt_marked;
			fprintf(stderr, "\rMarking track %6ld bad\nFormatting ",
					curtrk);
		}

		if (!bad.track && bad.sector && ioctl(devfd, FMTBAD, &ia) != -1) {
			fprintf(stderr,"\nFormat bad FAILED at track %ld.\n",curtrk);
			bad.track = 1;
		}
	}
	/* else - no bad sectors on track */

	for (retrycnt=0; !bad.track && retrycnt<5; retrycnt++)
		{
		if (ioctl(devfd, V_FORMAT, &ia) != -1)
			break;
		}
	if (retrycnt == 5)
		{
		fprintf(stderr,
			"\rFormat request FAILED 5 times at track %ld.\n",curtrk);
		}
	if(retrycnt != 0)
		fprintf(stderr,"Continuing... ");
	fprintf(stderr,"%6ld\b\b\b\b\b\b",curtrk);
	}

if(knt_bad || knt_marked) {
	fprintf(stderr,"\n\nFound %d complete and %d partial bad tracks. ",
					knt_bad, knt_marked);
	fprintf(stderr,"Total bad tracks = %d\n", knt_bad + knt_marked);
}

fprintf(stderr,"\nFormat complete.\n");
}


/*
 * Build up interleave table (intlvtab) where index to table is physical
 * sector number and table value is logical sector number.
 */
char *
interleave(factor)
int	factor;
{
	int     i,secno;

	for (i = 0; i < dp.dp_sectors; i++)
		intlvtab[i].secno =  0xFF;

	i = secno = 0;
	do {
		if (intlvtab[i].secno == 0xFF) {
			intlvtab[i].secno = (unsigned char)secno++;
			i = (i + factor) % dp.dp_sectors;
		} else
			i++;
	} while (secno < dp.dp_sectors);

	return( (char *)intlvtab );
}

/* verify n_sec sectors using V_VERIFY ioctl
 *	returns 0 for success, else
 *	return the error-code from the driver.
*/
sec_verify(start_sec, n_sec, n_tries)
	daddr_t		start_sec;	/* first sector to verify */
	int		n_sec;		/* number of sectors to verify */
	int		n_tries;	/* number of times (if all succeed) */
{
	union vfy_io	vfy;

	while (n_tries-- > 0) {
		vfy.vfy_in.abs_sec = start_sec;
		vfy.vfy_in.num_sec = n_sec;
		vfy.vfy_in.time_flg = 0;
		if (ioctl(devfd, V_VERIFY, &vfy) != 0) {
			fprintf(stderr, "\n");
			perror("Verify operation failed");
			exit(81);
		}
		if (vfy.vfy_out.err_code)
			return vfy.vfy_out.err_code;
	}
	return 0;
}


/*
 * Verifydevice
 * Attempt to read every sector of the drive and add bad sectors found to
 * list in mydev->ds_badsec.
 */
verifydevice()
{
int     trksiz = dp.dp_secsiz*dp.dp_sectors;
char    *verbuf = malloc(dp.dp_secsiz*dp.dp_sectors);
daddr_t cursec;
daddr_t lastsec = (long)dp.dp_cyls * dp.dp_sectors * dp.dp_heads;
daddr_t	lastusec = unix_part->numsect;	/* last UNIX System sector */
int     cylsiz = dp.dp_sectors * dp.dp_heads;
int     dotpos = 0;
int     badcnt = 0;
int     i;
char    *bptr, *rptr;
char    resp[10];

bptr = verbuf;
for (i=0; i<trksiz; i++) *bptr++ = 0xe5;

if ((verify & VER_WRITE) == 0) goto do_readonly;

printf("\nCAUTION: ABOUT TO DO DESTRUCTIVE WRITE ON ENTIRE UNIX SYSTEM PARTITION.\n");
printf("THIS WILL DESTROY ANY DATA ON %s.  Continue (y/n)? ",mydev->ds_device);
rptr = gets(resp);
if(!rptr ||  !((resp[0] == 'Y') || (resp[0] == 'y'))) goto do_readonly;

/* start at 0, relative to begining of UNIX System Partition */
for (cursec=0; cursec<lastusec; cursec+=dp.dp_sectors)
	{
	if (lseek(devfd,cursec*dp.dp_secsiz,0) == -1)
		{
		fprintf(stderr,"Error seeking sector %ld!\n",cursec);
		exit(71);
		}
	if (write(devfd,verbuf,trksiz) != trksiz)
		{
		daddr_t tmpsec;
		daddr_t tmpend=cursec+dp.dp_sectors;
		for (tmpsec=cursec; tmpsec<tmpend; tmpsec++)
		    {
		    int tmptry;
		    for (tmptry=0; tmptry<5; tmptry++)
			{
			if (lseek(devfd,tmpsec*dp.dp_secsiz,0) == -1)
				{
				fprintf(stderr,"Error seeking sector %ld!\n",
					tmpsec);
				exit(71);
				}
			if (write(devfd,verbuf,dp.dp_secsiz) != dp.dp_secsiz)
				{
				addbad(&badcnt, unix_part->relsect+tmpsec);
				break;
				}
			}
		    }
		}
	fprintf(stderr,"%6ld\b\b\b\b\b\b", cursec);
	}
fprintf(stderr,"\n");
dotpos = 0;

do_readonly:
for (cursec=0; cursec<lastusec; cursec+=dp.dp_sectors)
	{
	if (lseek(devfd,cursec*dp.dp_secsiz,0) == -1)
		{
		fprintf(stderr,"Error seeking sector %ld!\n",cursec);
		exit(71);
		}
	if (read(devfd,verbuf,trksiz) != trksiz)
		{
		daddr_t tmpsec;
		daddr_t tmpend=cursec+dp.dp_sectors;
		for (tmpsec=cursec; tmpsec<tmpend; tmpsec++)
		    {
		    int tmptry;
		    for (tmptry=0; tmptry<5; tmptry++)
			{
			if (lseek(devfd,tmpsec*dp.dp_secsiz,0) == -1)
				{
				fprintf(stderr,"Error seeking sector %ld!\n",
					tmpsec);
				exit(71);
				}
			if (read(devfd,verbuf,dp.dp_secsiz) != dp.dp_secsiz)
				{
				addbad(&badcnt, unix_part->relsect+tmpsec);
				break;
				}
			}
		    }
		}
	fprintf(stderr,"%6ld\b\b\b\b\b\b",cursec);
	}
fprintf(stderr,"\n");
if(badcnt > 0)
	{
	printf("Bad sectors found during verify pass are:\n");
	for (i=0; i<badcnt; )
		{
		printf("\t%ld",verbads[i]);
		if (((i++ % 4) == 0) && (i != 1))
			printf("\n");
		}
	printf("\nVerify complete.\n");
	}
}

/*
 * addbad(addr-of-counter, sector)
 * adds sector to verbads and list from mydev->ds_badsec IFF it currently
 * isn't in verbads.
 */

addbad(cntptr, secno)
int *cntptr;
daddr_t secno;

{
node    *lp, *np;
int i;

for (i=0; i<*cntptr; i++)
	{
	if (verbads[i] == secno)
		return;
	}
if (*cntptr > MAX_ALTENTS)
	{
	fprintf(stderr,"TOO MANY BAD SECTORS ON DRIVE!\n");
	exit(72);
	}
verbads[*cntptr++] = secno;
lp = newnode(LIST);
np = newnode(NUMBER);
lp->ListRef = (void *)np;
np->Number = secno;
lp->ListNext = mydev->ds_badsec;
mydev->ds_badsec = lp;
}



/*
 * Builddevice( device stanza )
 * Pull all of the values gotten from the user's device stanza(s) into the
 * various disk structures.
 * CURRENTLY WE DO VERY LITTLE VERIFICATION.  Builddevice should do lots of
 * validation to keep the user from shooting him/herself in the foot.
 */
void
builddevice(dev)
devstanza *dev;
{
	if (dev->ds_dserial) {
		strncpy(pdinfo.serial, dev->ds_dserial, sizeof(pdinfo.serial));
	}
	pdinfo.tracks = dev->ds_heads;
	dp.dp_cyls = pdinfo.cyls = dev->ds_cyls;
	dp.dp_secsiz = pdinfo.bytes = dev->ds_bpsec;

	pdinfo.vtoc_ptr = dev->ds_vtocsec * dp.dp_secsiz + sizeof(pdinfo);
	pdinfo.vtoc_len = sizeof(vtoc);

	/** Zero the alt variables for SCSI **/

	if(!is_scsi_dev)
	{
		pdinfo.alt_ptr = dev->ds_altsec * dp.dp_secsiz;
		pdinfo.alt_len = sizeof(alttbl);
	}
	else
	{
		pdinfo.alt_ptr = (long)0;
		pdinfo.alt_len = 0;
	}
}

/*
 * READ_ELF_BOOT ()
 * reads in the elf bootable into a buffer which is returned. Routine primarily
 * uses libelf calls to do elf specific actions.
 */
char *
read_elf_boot()
{
	char *buf;
	int  i;
	Elf_Scn *scn;
	Elf32_Shdr *eshdr;

	if ((ephdr = elf32_getphdr (elfd)) == NULL) {
		fprintf (stderr, "can't get ELF program header: %s\n", elf_errmsg (0));
		exit(31);
	}

	for (i = 0; i < (int)ehdr->e_phnum; i++)
		if (ephdr[i].p_type == PT_LOAD && ephdr[i].p_filesz > 0)
			break;
	if (i >= (int)ehdr->e_phnum) {
		fprintf (stderr, "can't find loadable ELF segment\n");
		exit(31);
	}
	if ((buf = malloc(((ephdr->p_filesz+dp.dp_secsiz-1)/dp.dp_secsiz)
		*dp.dp_secsiz)) == NULL) {
		fprintf (stderr, "can't all allocate boot buffer\n");
		exit(31);
	}
	phdr = &ephdr[i];
	for (scn = NULL; scn = elf_nextscn (elfd, scn); ) {
		if ((eshdr = elf32_getshdr (scn)) == NULL) {
			free (buf);
			printf ("Invalid boot, empty segment\n");
			exit(31);
		}
		if (eshdr->sh_addr >= phdr->p_vaddr && 
		    eshdr->sh_addr < phdr->p_vaddr + phdr->p_filesz &&
		    eshdr->sh_type == SHT_PROGBITS &&
		    eshdr->sh_flags & SHF_ALLOC) {
			int nbytes;
			Elf_Data *data;

			if ((data = elf_getdata (scn, NULL)) == NULL || 
			     data->d_buf == NULL) {
                                free (buf);
				fprintf (stderr, "Invalid boot, empty segment\n");
				exit(31);
			}
			nbytes = eshdr->sh_size;
			if (eshdr->sh_addr + eshdr->sh_size > phdr->p_vaddr + 
			    phdr->p_filesz)
                                nbytes -= eshdr->sh_addr + eshdr->sh_size - 
					  phdr->p_vaddr - phdr->p_filesz;
			memcpy (&buf[eshdr->sh_addr - phdr->p_vaddr], 
				(char *) data->d_buf, nbytes);
		}
	}
	return (buf);
}



/*
 * Writeboot ()
 * Writes the bottstrap code and the current volume label out to the disk
 * (the modified volume label is updated later in writevtoc(); this separation
 * supports -b).  The volume label appears in the middle of the bootstrap code;
 * it appears at sector VLAB_SECT, offset by VLAB_START.  We merely open a hole
 * in the bootstrap for the label, but the bootstrap must take this hole into
 * account and do something like jump around it (this must be a pretty clever
 * trick for the bootstrap coder, but Intel likes it this way )-: ).  We
 * guarantee that bss is initialized to 0, but Intel's old bootstrap doesn't
 * assume that.
 */
void
writeboot()
{
char		*buf;
char		*p;
daddr_t         isecp, boot_offset;
long		len;
int             secno = 0;
long            blockno;
int             i;
struct partition *pp;
int             rootfound = 0;

	if (elfboot)
		buf = read_elf_boot();
	else { /* COFF bootable will be read into buf */
		/* get a buffer for the whole bootstrap and label */
		/* rif -- the bootstrap can be no bigger than HDPDLOC sectors */

		if ((buf = malloc(HDPDLOC*dp.dp_secsiz)) == NULL) {
			fprintf(stderr,"Cannot malloc boot buffer\n");
			exit(45);
		}
		p = buf;	/* p will walk thru buf, where data is read */
		/* isecp will point at scnhdr structs in a.out */
		isecp = FILHSZ + filehdr.f_opthdr;

		/*
	 	* Loop for each section in the a.out.  Lseek and read the boot
	 	* section header.  Subloop to read all of section into buf.
	 	*/
		for (; secno < filehdr.f_nscns; (isecp += SCNHSZ), secno++) {
				/* seek and read section header */
			if ((lseek(bootfd,isecp,0) == -1) ||
					(read(bootfd,&scnhdr,SCNHSZ) != SCNHSZ)) {
				fprintf(stderr,"Seeking/reading section header %d ",secno);
				perror(mydev->ds_boot);
				exit(40);
			}
				/* seek start of section */
			if (lseek(bootfd,scnhdr.s_scnptr,0) == -1) {
				fprintf(stderr,"Seeking section %d ",secno);
				perror(mydev->ds_boot);
				exit(41);
			}
				/* read section */
			for (blockno=0; len=readbootblock(blockno,p); blockno++) {
				p += len;	/* advance buffer pointer */
				/*
			 	* The reading loop terminates if we tried to read a
			 	* block and it had zero length, or if the current
			 	* block was short.
			 	*/
				if (len != dp.dp_secsiz) break;
			}
		}
	}
	
	/*
	 * Write out the boot.
	 */

	/*
	 * Our boot track goes where the fdisk partition table says it
	 * should go.
	 */

	/* If we don't have a root partition, don't write the boot. */

	for (pp = vtoc.v_part; pp < &vtoc.v_part[vtoc.v_nparts]; pp++) {
		if (pp->p_tag == V_ROOT && (pp->p_flag & V_VALID)) {
			rootfound = 1;
			break;
		}
	}

	if (!rootfound) 
		return; 

	/* set location of boot to beginning of UNIX System partition.
	*/
	boot_offset = (daddr_t)0L;

	/* round length of boot to a sector boundary */
	if (elfboot)
		len = (((phdr->p_filesz)+(dp.dp_secsiz-1))/dp.dp_secsiz)*dp.dp_secsiz;
	else
		len = (((p-buf)+(dp.dp_secsiz-1))/dp.dp_secsiz)*dp.dp_secsiz;

	for(i=0; i < len/512; i++){
	    absbuf.abs_sec = unix_part->relsect + i;
	    absbuf.abs_buf = (buf + (i * 512));
	    if(ioctl(devfd, V_WRABS, &absbuf) != 0){
		perror("WRITE of boot");
		exit(43);
	    }

	}
	return;

noboot:
	fprintf(stderr, "Run fdisk to create a UNIX System partition in the ");
	fprintf(stderr,"proper cylinders and\nRe-run mkpart with the -b flag.\n");
	return;
}

/*
 * Readbootblock ( block # in current section, buffer pointer )
 * returns length read for this block.
 */
int
readbootblock(blockno,buf)
long    blockno;
char    *buf;
{
int	len;

		/* calculate length to read */
	if (blockno < scnhdr.s_size/dp.dp_secsiz) {
		len = dp.dp_secsiz;
	} else {
		len = scnhdr.s_size%dp.dp_secsiz;
	}

	/*
	 * If the section type is text or data, read real data from the file.
	 * If the section is bss, just return len.  If the section
	 * is some other kind, don't read anything and report 0 length;  this
	 * will advance us to the next section.
	 */
	if (scnhdr.s_flags & (STYP_TEXT|STYP_DATA)) {
		if (read(bootfd,buf,len) != len) {
			fprintf(stderr,"Reading section, block number %ld ",blockno);
			perror(mydev->ds_boot);
			exit(42);
		}
	} else len = 0;
	return len;
}

/*
 * Updatedevice ()
 * Generally, any info that needs to be "bubbled up" from the partitions or
 * alternates table should be handled here.  Currently, we don't do anything.
 */
void
updatedevice()
{
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
void
writevtoc()
{
  char	*buf;

		/* We allocate a buffer large enough to accomodate
			the largest object, the alternates list.
		*/
	if ((buf=malloc(sizeof(alttbl))) == NULL)
		{
		fprintf(stderr,"writevtoc -- can't allocate buffer\n");
		exit(69);
		}
	/*
	 * If we're doing a floppy, we're all done.  Otherwise, put out the
	 * other structs:  pdinfo, vtoc, and alttbl.
	 */
	if ((dp.dp_type == DPT_WINI) || (dp.dp_type == DPT_SCSI_HD) ||
			(dp.dp_type == DPT_SCSI_OD)) {
			/* put pdinfo & vtoc into the same sector */
		*((struct pdinfo *)buf) = pdinfo;
		*((struct vtoc *)&buf[pdinfo.vtoc_ptr%dp.dp_secsiz]) = vtoc;
		absbuf.abs_sec = unix_part->relsect + pd_sector;
		absbuf.abs_buf=buf;
		if (ioctl(devfd, V_WRABS, &absbuf) != 0) {
			perror("Writing new pdinfo");
			exit(51);
		}
	/*					*/
	/*	now do the alternate table	*/
	/*					*/
	/* With Maria's implementation of automatic BBH, it becomes imperative
	 * that we use the V_ADDBAD ioctl, for non-initialization situations,
	 * instead of the write(), as the write() may cause the disk
	 * and kernel versions of the alttbl to be out of sync.
	 */

	/* only write table to disk on initialization */
	/* DON'T DO THIS FOR SCSI DISKS **/
	if ((First_alt || initialize) && !is_scsi_dev)
	  {		/* - otherwise, dynamic bbh takes care of things */
		if ((lseek(devfd,pdinfo.alt_ptr,0) == -1) ||
		    (write(devfd,(char *)&alttbl,sizeof(alttbl)) != sizeof(alttbl))) {
			perror("Writing new alternate block table");
			exit(53);
		}
	  }
	}

	/* make sure things are clean! */
	remount();
	sync();
}

/* Remount ()
 * Make sure changes to special sectors are recognized by the disk driver.
 */
remount()
{
	/*
	 * Attempt to remount the device.  Generally, one can expect this to
	 * fail if somebody has one of the partitions open.  This means that
	 * the user won't be able to use the new structures until reboot time.
	 */
	if (ioctl(devfd, V_REMOUNT, NULL) == -1) {
		fprintf(stderr,"Warning:  V_REMOUNT io control failed.  This may result from\n");
		fprintf(stderr,"one or more of the partitions on the disk being open.\n");
		fprintf(stderr,"Disk must be manually re-mounted or system re-booted for\n");
		fprintf(stderr,"any changes in partitions or alternate sectors to be useable.\n");
		exit(54);
	}
	mkpartclose();
	if (mkpartopen(mydev->ds_device)) {
		perror("Can't re-open disk device");
		exit(54);
	}
}

/*
 * Updateparts ()
 * Walk through all of the partitions to be removed and remove them.  Alter
 * the partition count to reflect those removed at the end.  Walk through
 * partitions to be added and fill them in -- regardless of the info being
 * overlaid!
 */
void
updateparts()
{
	struct partition *p;
	partstanza *mypart;
	node *n;
	symbol *part;

	long *pt;	/** *pt is used like *p for the timestamp array **/

	for (n=subparts; n; n = n->ListNext) {	/* For each removed partition */
			/* look up the name; make sure it was defined */
		if ( !((part = lookup(n->ListRef))->flags & SY_DEFN) ) {
			fprintf(stderr,"Partition stanza '%s' not defined in file %s\n",
				(char *)n->ListRef, partfile);
			exit(6);
		}

		/*
		 * Build a stanza combining any other referenced stanzas
		 * into one.  Replace the name on the partition list with its
		 * stanza (this last operation is not needed, but is
		 * conceivably useful in some future scenarios).
		 */
		mypart = (partstanza *)newpartstanza();
		buildstanza(mypart, part);
		n->ListRef = (void *)mypart;

			/* Validate partition number from stanza */
		if (mypart->ps_partno == UNDEFINED_NUMBER) {
			fprintf(stderr,"Stanza %s has no partition number\n", mypart->ps_name->name);
			exit(8);
		}
			/* Clear partition entry */
		p = &vtoc.v_part[mypart->ps_partno];
		if (p->p_tag == V_ALTS)
			{
			fprintf(stderr,"Removal of ALTS partition not supported\n");
			exit(64);
			}
		p->p_tag = p->p_flag = p->p_start = p->p_size = 0;

		/** Clear the timestamp **/
		vtoc.timestamp[mypart->ps_partno] = (long)0;
	}

	/*
	 * Reset vtoc.v_nparts.  After deleting partitions, walk back from the
	 * original last partition until we find one that is still valid and
	 * call it the last partition.
	 */
	for (p = &vtoc.v_part[vtoc.v_nparts-1], pt = &vtoc.timestamp[vtoc.v_nparts-1];
			p >= vtoc.v_part && !(p->p_flag & V_VALID); p--,pt--) {
		/* null */ ;
	}
	vtoc.v_nparts = p - vtoc.v_part + 1;


	/*
	 * Add new partitions.  Loop around for each one, build it up from
	 * any other referenced stanzas, stuff the data into the partition,
	 * fix up the number of partitions if this is beyond the previous
	 * last partition.  CURRENTLY, PARTITION VALUES ARE NOT SANITY CHECKED.
	 * We ought to at least ensure that the numbers are reasonable.
	 */
	for (n=addparts; n; n = n->ListNext) {
		if ( !((part = lookup(n->ListRef))->flags & SY_DEFN) ) {
			fprintf(stderr,"Partition stanza '%s' not defined in file %s\n",
				(char *)n->ListRef, partfile);
			exit(7);
		}
		mypart = (partstanza *)newpartstanza();
		buildstanza(mypart, part);
		n->ListRef = (void *)mypart;
		p = &vtoc.v_part[mypart->ps_partno];
		p->p_tag = mypart->ps_ptag;
		p->p_flag = mypart->ps_perm | V_VALID; /* by definition */
		p->p_start = mypart->ps_start;
		p->p_size = mypart->ps_size;
		if (mypart->ps_partno >= vtoc.v_nparts) {
			vtoc.v_nparts = mypart->ps_partno + 1;
		}
		/** Initialize the new timestamp variable **/

		vtoc.timestamp[mypart->ps_partno] = (long)0;

		/** ONLY DO ALT HANDLING FOR ESDI **/
		if(!is_scsi_dev)
		{

			if (p->p_tag == V_ALTS && alttbl.alt_sec.alt_reserved == 0) {
				alttbl.alt_sec.alt_reserved = p->p_size;
				alttbl.alt_sec.alt_base = p->p_start;

				/* Set flag - Adding an ALTS slice for the very
				 *	first time. With dynamic BBH, it is no longer
				 * appropriate to add additional ALTS areas. One must
				 * allocate all ALTS at installation time.
				*/
				First_alt++;
			}
			if (p->p_tag == V_ALTTRK && alttbl.alt_trk.alt_reserved == 0) {
				alttbl.alt_trk.alt_reserved = p->p_size / dp.dp_sectors;
				alttbl.alt_trk.alt_base = p->p_start;

				/* Adding an ALTTRK slice for the first time. */
				First_alt++;
			}
		}
	}
}

/*
 * UpdateAlts
 * Update the Alternate Sector Table (alttbl).  At this point, mydev->ds_badsec
 * points to a list of bad sectors.  These come from 3 places:
 *   - from the device stanza,
 *   - from -A arguments, and
 *   - from any sectors found defective during the 'format' surface analysis.
 * Alttbl has any entries which were in the alternates table as read from
 * the disk (if any) plus sectors from any alternates partitions which were
 * added during this run.  These new ones (at least) have not been verified
 * as being useable.  We first look through the bad sector list to see if
 * any show up as being alternates.  If we find one and it isn't assigned
 * yet, just remove it from the table (this may result in an incompletely
 * filled alternates table, but rarely -- big deal).  If we find one which
 * is already assigned, we complain (the only solution to this is to reformat
 * and re-build everything).  After getting a good list of alternates, we
 * look through the bad sector list again.  If we've already assigned an
 * alternate for a bad one, all's ok.  If not, assign the next available
 * alternate (if none left, complain).  Then try the V_ADDBAD call to
 * add the baddy to the incore alternates table.  This may not work if
 * we've just added the first alternates partition or a previous one was
 * full.  In this case, warn the user that he should reboot if the V_REMOUNT
 * call fails.
 * We also initialize the alternate track table from mydev->ds_badtrk;
 * this list was read in from the device stanza.
 */

updatealts()
{
node *badptr;
daddr_t curbad;
int i,j;

/** NOTE: NEVER CALLED FOR SCSI! **/
long maxsec = (long)dp.dp_heads * dp.dp_cyls * dp.dp_sectors;

if((alttbl.alt_sec.alt_reserved == 0) && mydev->ds_badsec) {
	fprintf(stderr,
		"Warning: No alternates partition in VTOC for device %s\n",
		mydev->ds_device);
	fprintf(stderr,"         Bad sectors will not be marked!\n");
	goto do_tracks;
}
for (badptr=mydev->ds_badsec; badptr; badptr=badptr->ListNext) {
	if (badptr->ListElem->token == RANGE) {
		fprintf(stderr,
			"RANGE illegal for bad sector specification\n");
		exit(61);
	}
	curbad = badptr->ListElem->Number;
	if (!First_alt) {
		union io_arg	new_badblk,
				*newbad = &new_badblk;

		newbad->ia_abs.bad_sector = curbad;
		if(ioctl(devfd, V_ADDBAD, newbad) == -1) {
			fprintf(stderr,
				"Failed V_ADDBAD on bad block %#lx\n",
					curbad);
			perror("Unable to assign alternate");
			exit(54);
		}
#ifdef DEBUG
		printf("Bad sector: %#lx assigned to alternate: %#lx.\n",
				curbad, newbad->ia_abs.new_sector);
#endif
		break;	/* driver will do all the checking */
	}
	if (curbad >= maxsec) {
		fprintf(stderr,
			"Bad sector %ld is past the end of the drive.\n",
			curbad);
		exit(69);
	}
	/* if unused alternate is bad, then excise it from the list. */
	for (i=alttbl.alt_sec.alt_used; i<alttbl.alt_sec.alt_reserved; i++) {
		if (alttbl.alt_sec.alt_base + i == curbad) {
			alttbl.alt_sec.alt_bad[i] = -1;
			if (i == alttbl.alt_sec.alt_used) {
				while (++alttbl.alt_sec.alt_used < alttbl.alt_sec.alt_reserved)
					if (alttbl.alt_sec.alt_bad[alttbl.alt_sec.alt_used] != -1)
						break;
			}
			break;
		}
	}
	/* if used alternate is bad, give up */
	for (i=0; i<alttbl.alt_sec.alt_used; i++) {
		if (alttbl.alt_sec.alt_base + i == curbad) {
			fprintf(stderr,
				"Bad sector %ld is an assigned alternate!\n",
				curbad);
			exit(62);
		}
	}
}

if (!First_alt)	return;	/* bad blocks already submitted via ioctl() */

for (badptr=mydev->ds_badsec; badptr; badptr=badptr->ListNext) {
	curbad = badptr->ListElem->Number;
	/* don't map alternates */
	if (curbad >= alttbl.alt_sec.alt_base && curbad <
			alttbl.alt_sec.alt_base + alttbl.alt_sec.alt_reserved)
		continue;
	/* check if bad block already mapped (already in list) */
	for (i=0; i<alttbl.alt_sec.alt_used; i++)
		if (alttbl.alt_sec.alt_bad[i] == curbad) break;
	if (i == alttbl.alt_sec.alt_used) {	/* this is a new bad block */
		if (alttbl.alt_sec.alt_used >= alttbl.alt_sec.alt_reserved) {
			fprintf(stderr,
				"Insufficient alternates available for bad sector %ld!\n",
				 curbad);
			exit(63);
		}
		alttbl.alt_sec.alt_bad[alttbl.alt_sec.alt_used] = curbad;
		while (++alttbl.alt_sec.alt_used < alttbl.alt_sec.alt_reserved)
			if (alttbl.alt_sec.alt_bad[alttbl.alt_sec.alt_used] != -1)
				break;
		/* TRY THE V_ADDBAD HERE */
	}
}

do_tracks:
	/* Put bad tracks into bad track table */
if ((alttbl.alt_trk.alt_reserved == 0) && mydev->ds_badtrk) {
	fprintf(stderr,
		"Warning: No alternate track partition in VTOC for device %s\n",
		mydev->ds_device);
	fprintf(stderr,"         Bad tracks will not be marked!\n");
	return;
}
for (badptr=mydev->ds_badtrk; badptr; badptr=badptr->ListNext) {
	if (badptr->ListElem->token == RANGE) {
		fprintf(stderr,
			"RANGE illegal for bad track specification\n");
		exit(61);
	}
	curbad = badptr->ListElem->Number;
	if (curbad >= maxsec / dp.dp_sectors) {
		fprintf(stderr,
			"Bad track %ld is past the end of the drive.\n",
			curbad);
		exit(69);
	}
	/* if unused alternate is bad, then excise it from the list. */
	for (i=alttbl.alt_trk.alt_used; i<alttbl.alt_trk.alt_reserved; i++) {
		if (alttbl.alt_trk.alt_base / dp.dp_sectors + i == curbad) {
			alttbl.alt_trk.alt_bad[i] = -1;
			if (i == alttbl.alt_trk.alt_used) {
				while (++alttbl.alt_trk.alt_used < alttbl.alt_trk.alt_reserved)
					if (alttbl.alt_trk.alt_bad[alttbl.alt_trk.alt_used] != -1)
						break;
			}
			break;
		}
	}
	/* if used alternate is bad, give up */
	for (i=0; i<alttbl.alt_trk.alt_used; i++) {
		if (alttbl.alt_trk.alt_base / dp.dp_sectors + i == curbad) {
			fprintf(stderr,
				"Bad track %ld is an assigned alternate!\n",
				curbad);
			exit(62);
		}
	}
}

for (badptr=mydev->ds_badtrk; badptr; badptr=badptr->ListNext) {
	curbad = badptr->ListElem->Number;
	/* don't map alternates */
	if (curbad >= alttbl.alt_trk.alt_base / dp.dp_sectors &&
			curbad < alttbl.alt_trk.alt_base / dp.dp_sectors
					+ alttbl.alt_trk.alt_reserved)
		continue;
	/* check if bad track already mapped (already in list) */
	for (i=0; i<alttbl.alt_trk.alt_used; i++)
		if (alttbl.alt_trk.alt_bad[i] == curbad) break;
	if (i == alttbl.alt_trk.alt_used) {	/* this is a new bad track */
		if (alttbl.alt_trk.alt_used >= alttbl.alt_trk.alt_reserved) {
			fprintf(stderr,
				"Insufficient alternates available for bad track %ld!\n",
				 curbad);
			exit(63);
		}
		alttbl.alt_trk.alt_bad[alttbl.alt_trk.alt_used] = curbad;
		while (++alttbl.alt_trk.alt_used < alttbl.alt_trk.alt_reserved)
			if (alttbl.alt_trk.alt_bad[alttbl.alt_trk.alt_used] != -1)
				break;
	}
}
}

/*
 * Printdevinfo ( print option flags )
 * Print info for user.
 */
void
printdevinfo(flags)
int flags;
{
static struct Px_data {
		int     ptx_tagv;
		char    *ptx_name;
		char    *ptx_tags;
		char    *ptx_cnt;
		} pxdata [] =  {{V_BOOT, "bootx", "BOOT", 0},
				{V_ROOT, "rootx", "ROOT", 0},
				{V_DUMP, "dumpx", "DUMP", 0},
				{V_HOME, "homex", "HOME", 0},
				{V_SWAP, "swapx", "SWAP", 0},
				{V_STAND, "standx", "STAND", 0},
				{V_USR,  "usrx",  "USR",  0},
				{V_VAR,  "varx",  "VAR",  0},
				{V_ALTS, "altsx", "ALTS", 0},
				{V_ALTTRK,"trkaltx","ALTTRK",0},
				{V_OTHER,"otherx","OTHER",0}};

	if (flags & TF_WPART) {
		FILE *pfile;
		int     i;
		char    pxnam [8];

		if ((pfile=fopen(wpfile,"w")) == NULL)
			{
			perror("mkpart opening file for -x");
			exit(101);
			}
		fprintf(pfile,
			"diskx:\nheads = %d, cyls = %d, sectors = %d, bpsec = %d,\n",
			pdinfo.tracks, pdinfo.cyls, pdinfo.sectors, pdinfo.bytes);
		fprintf(pfile,
			"vtocsec = %ld, altsec = %ld, boot = \"%s\", device = \"%s\"\n\n",
			mydev->ds_vtocsec, mydev->ds_altsec,
			(mydev->ds_boot ? mydev->ds_boot : "/etc/boot"),
			mydev->ds_device);
		for (i=0; i<vtoc.v_nparts; i++)
			{
			struct partition *pp = &vtoc.v_part[i];

			if (pp->p_flag & V_VALID)
				{
				int j;
				int K = sizeof(pxdata) / sizeof(struct Px_data);
				int goodp=0;

				for (j=0; j < K; j++)
					{
					if (pp->p_tag == pxdata[j].ptx_tagv)
						{
						goodp = 1;
						break;
						}
					}
				if (goodp)
					{
					if (pxdata[j].ptx_cnt++)
						sprintf(pxnam,"%s%d",
							pxdata[j].ptx_name,
							pxdata[j].ptx_cnt);
					else    sprintf(pxnam,"%s",
							pxdata[j].ptx_name);
					fprintf(pfile,"%s:\n",pxnam);
					fprintf(pfile,
						"partition = %d, start = %ld, size = %ld,\n",
						i, pp->p_start, pp->p_size);
					fprintf(pfile,"tag = %s",pxdata[j].ptx_tags);
					if (pp->p_flag & V_VALID)
						fprintf(pfile,", perm = VALID");
					if (pp->p_flag & V_UNMNT)
						fprintf(pfile,", perm = NOMOUNT");
					if (pp->p_flag & V_RONLY)
						fprintf(pfile,", perm = RO");
					fprintf(pfile,"\n\n");
					}
				}
			}
		if (ferror(pfile))
			{
			perror("mkpart writing to file for -x");
			exit(101);
			}
		if (fclose(pfile) != 0)
			{
			perror("closing file for -x");
			exit(101);
			}
	}

	if (flags & TF_VTOC) {
		printf("\tDevice %s\n",mydev->ds_device);
		printf("device type:\t\t%ld\n",pdinfo.driveid);
		printf("serial number:\t\t%.12s\n",pdinfo.serial);
		printf("cylinders:\t\t%ld\t\theads:\t\t%ld\n",pdinfo.cyls,pdinfo.tracks);
		printf("sectors/track:\t\t%ld\t\tbytes/sector:\t%ld\n",pdinfo.sectors,pdinfo.bytes);
		printf("number of partitions:\t%d",vtoc.v_nparts);
		printf("\t\tsize of alts table:\t%d\n", pdinfo.alt_len);
	}
	if (flags & TF_PARTS) {
		int i;

		for (i = 0; i < vtoc.v_nparts; i++) {
			printf("partition %d:\t",i);
			printpart(&vtoc.v_part[i]);
		}
	}
	if (flags & TF_ALTS) {
		/** Never true for SCSI since this bit is masked off **/
		printalts(&alttbl.alt_sec, 1);
		printalts(&alttbl.alt_trk, 0);
	}
}


void
printalts(altptr, sectors)
struct alt_table	*altptr;
int			sectors;
{
	int i, j;

	/** Never true for SCSI since this bit is masked off **/

	printf("\nALTERNATE %s TABLE: %d alternates available, %d used\n",
			sectors? "SECTOR" : "TRACK",
			altptr->alt_reserved, altptr->alt_used);

	if (altptr->alt_used > 0) {
		printf("\nAlternates are assigned for the following bad %ss:\n",
				sectors? "sector" : "track");
		for (i = j = 0; i < altptr->alt_used; ++i) {
			if (altptr->alt_bad[i] == -1)
				continue;
			printf("\t%ld -> %ld", altptr->alt_bad[i],
				sectors ? altptr->alt_base + i
					: altptr->alt_base / dp.dp_sectors + i);
			if ((++j % 3) == 0) printf("\n");
		}
		printf("\n");
	}
	if (altptr->alt_used != altptr->alt_reserved) {
		printf("\nThe following %ss are available as alternates:\n",
				sectors? "sector" : "track");
		for (i = altptr->alt_used, j = 0; i < altptr->alt_reserved; ++i) {
			if (altptr->alt_bad[i] == -1)
				continue;
			printf("\t\t%ld",
				sectors ? altptr->alt_base + i
					: altptr->alt_base / dp.dp_sectors + i);
			if ((++j % 4) == 0) printf("\n");
		}
		printf("\n");
	}
}


/*
 * Printpart ( partition entry pointer )
 * Support for printdevinfo().  Print out a formatted partition report.
 */
void
printpart(v_p)
struct partition *v_p;
{
	switch(v_p->p_tag) {
	case V_BOOT:	printf("BOOT\t\t");			break;
	case V_DUMP:	printf("DUMP\t\t");			break;
	case V_ROOT:	printf("ROOT\t\t");			break;
	case V_HOME:	printf("HOME\t\t");			break;
	case V_SWAP:	printf("SWAP\t\t");			break;
	case V_USR:	printf("USER\t\t");			break;
	case V_VAR:	printf("VAR\t\t");			break;
	case V_STAND:	printf("STAND\t\t");			break;
	case V_BACKUP:	printf("DISK\t\t");			break;
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
	printf("\n\tstarting sector:\t%ld (0x%lx)\t\tlength:\t%ld (0x%lx)\n",
		v_p->p_start, v_p->p_start, v_p->p_size, v_p->p_size);
}

/*
 * Giveusage ()
 * Give a (not so) concise message on how to use mkpart.
 */
void
giveusage(extent)
int extent;	/* extent == 1 -> give shortform */
{
    if(extent) {
	fprintf(stderr,"\nmkpart [options] device\nor,\n");
	fprintf(stderr,"mkpart -F interleave_factor raw_UNIX_System_device\n\n");
	return;
    }
fprintf(stderr,"mkpart [-P add_partition_name] [-p remove_partition_name]\n");
fprintf(stderr,"   [-b] [-B boot_code_file] [-f partition_file]\n");
fprintf(stderr,"   [-A absolute_sector_number] [-F interleave_factor]\n");
fprintf(stderr,"   [-t apv] [-x filename] [-v] [-V] [-i] device\n");
fprintf(stderr,"NOTE: multiple P, p, and A flags may be specified.\n");
fprintf(stderr,"-b just rewrites the boot code, as determined from the partition file.\n");
fprintf(stderr,"-B specifies a different boot file.\n");
fprintf(stderr,"-F formats the entire device with specified interleave.\n");
fprintf(stderr,"\tThe last arg is a raw UNIX System device (e.g. /dev/rdsk/1s0)\n");
fprintf(stderr,"-A marks the sector bad and assigns it an alternate.\n");
fprintf(stderr,"-f specifies the partition file; its absence implies %s.\n",
	PARTFILE);
fprintf(stderr,"-t asks for a listing of:\n");
fprintf(stderr,"   a - alternate sectors table,\n");
fprintf(stderr,"   p - partitions,\n");
fprintf(stderr,"   v - vtoc and physical drive characteristics.\n");
fprintf(stderr,"-x writes a complete device and partition stanza list for the\n");
fprintf(stderr,"   specified device to file 'filename' (useful for recovery).\n");
fprintf(stderr,"-v attempts to read every sector on the device and adds any\n");
fprintf(stderr,"   bad ones to the alternates table.  THIS TAKES A WHILE...\n");
fprintf(stderr,"-V does -v also, but first WRITES every sector of the drive.\n");
fprintf(stderr,"-i initializes the device, ignoring any existing VTOC data.\n");
fprintf(stderr,"   MUST BE USED if the device has never been formatted;\n");
fprintf(stderr,"   may be used to re-initialize a drive.\n");
}


#define MAXUSEDEPTH     100	/* Deepest 'use' or 'usepart' nesting */

/*
 * Buildstanza ( fresh stanza pointer, symbol table entry pointer )
 * Build a new stanza from the one that the symbol table entry points at,
 * including all of the data from any other stanzas that are 'use'ed or
 * 'usepart'ed.  In particular, used stanzas are included deepest first.
 */
void
buildstanza(r,n)
stanza *r;
symbol *n;
{
stanza		*s = (stanza *)n->ref;		/* use'ed stanza, if any */
int     	usesdepth = -1;			/* current top of stanza stack*/
int     	stanzatype = s->dev.ds_type;	/* either dev or part stanza */
stanza		*uses[MAXUSEDEPTH];		/* use stanza stack */

	/*
	 * Walk through all use'ed stanzas and collect them onto the use'ed
	 * stanza stack.  Watch for stack overflows and mutual recursion.
	 */
		/* push current stanza, check for another use */
	while ( (uses[++usesdepth] = s)->dev.ds_use ) {
		s = (stanza *)s->dev.ds_use->ref; /* get next stanza */
		if( !s ) {
			fprintf(stderr,"Referenced stanza '%s' not defined\n",
				uses[usesdepth]->dev.ds_use->name);
			exit(20);
		}
		/*
		 * Check for mutual recursion.  MaxUseDepth is set during
		 * the parse of the stanza file to the number of stanzas
		 * encountered.  If we have gone through all of them, we
		 * must be reusing one now... and forever.
		 */
		if (usesdepth > MaxUseDepth) {
			fprintf(stderr,"From stanza '%s' circular USE\n",
				n->name);
			exit(20);
		} else if (usesdepth >= MAXUSEDEPTH) {
			myerror("Stanzas nested too deeply",0);
		} else if (s->dev.ds_type != stanzatype) {
			fprintf(stderr,"From stanza '%s' incompatible stanza USEd\n",
				n->name);
			exit(20);
		}
	}

/*
 * CHECKSTANZAELEM is a macro that saves a lot of paper (and keying time).
 * It checks stanza field 'f' for some condition, COND(f); if true for both
 * the current source (s) stanza and for the resultant (r) stanza that we are
 * building up, it prints the message 'm' (which should be a "ed string).
 * The implication is that we have two stanzas that specify the same value.
 * In any event, it then performs operation OP(r->f,s->f).
 */

#define CHECKSTANZAELEM(f,COND,OP,m)                                         \
	if (COND(s->f)) {                                                    \
		if (COND(r->f)) {                                            \
			fprintf(stderr,"stanza %s, warning: %s\n",n->name,m);\
		}                                                            \
		OP(r->f,s->f);                                               \
	}

/*
 * The following definitions are useful CONDs for CHECKSTANZA.  Note that these
 * are all named for the opposite condition that is actually checked (it seemed
 * like a good idea at the time... :-).
 */
#define ZERO(a)         (a)
#define UNDEFSECTOR(a)  ((a)!=UNDEFINED_SECTOR)
#define UNDEFNUMBER(a)  ((a)!=UNDEFINED_NUMBER)

/*
 * The following are OPs for CHECKSTANZA.
 */
#define ASSIGN(a,b)     ((a)=(b))

	/*
	 * For each used stanza, in stack order, pick up the stanza and
	 * check and then include its data into the result stanza.
	 */
	for (s = uses[usesdepth]; usesdepth-- >= 0; s = uses[usesdepth]) {
			/* Device stanzas processed here... */
		if (stanzatype == S_DEVICE) {
			CHECKSTANZAELEM(dev.ds_boot,ZERO,ASSIGN,"boot file redefined")
			CHECKSTANZAELEM(dev.ds_device,ZERO,ASSIGN,"device name redefined")
			CHECKSTANZAELEM(dev.ds_dserial,ZERO,ASSIGN,"serial number redefined")
			CHECKSTANZAELEM(dev.ds_heads,ZERO,ASSIGN,"number of heads redefined")
			CHECKSTANZAELEM(dev.ds_cyls,ZERO,ASSIGN,"number of cylinders redefined")
			CHECKSTANZAELEM(dev.ds_sectors,ZERO,ASSIGN,"number of sectors/track redefined")
			CHECKSTANZAELEM(dev.ds_bpsec,ZERO,ASSIGN,"number of bytes/sector redefined")
			CHECKSTANZAELEM(dev.ds_vtocsec,UNDEFSECTOR,ASSIGN,"vtoc sector redefined")
			CHECKSTANZAELEM(dev.ds_altsec,UNDEFSECTOR,ASSIGN,"alternate track table redefined")
			/*
			 * For bad sector lists, if there isn't a list in the
			 * result stanza yet, put the current one there, else
			 * merge them together.  Mergeranges checks for
			 * collisions.
			 */
			if (r->dev.ds_badsec) {
				r->dev.ds_badsec =
					mergeranges(r->dev.ds_badsec,s->dev.ds_badsec);
			} else if (s->dev.ds_badsec) {
				r->dev.ds_badsec = s->dev.ds_badsec;
			}
			if (r->dev.ds_badtrk) {
				r->dev.ds_badtrk =
					mergeranges(r->dev.ds_badtrk,s->dev.ds_badtrk);
			} else if (s->dev.ds_badtrk) {
				r->dev.ds_badtrk = s->dev.ds_badtrk;
			}
		} else {
				/* Partition stanzas processed here... */
			CHECKSTANZAELEM(part.ps_partno,UNDEFNUMBER,ASSIGN,"partition number redefined")
			CHECKSTANZAELEM(part.ps_ptag,ZERO,ASSIGN,"partition tag redefined")
			CHECKSTANZAELEM(part.ps_start,UNDEFSECTOR,ASSIGN,"starting sector redefined")
			CHECKSTANZAELEM(part.ps_size,UNDEFSECTOR,ASSIGN,"partition size redefined")
			r->part.ps_perm |= s->part.ps_perm;
		}

		/* make the result point at the last stanza processed */
		r->dev.ds_name = uses[0]->dev.ds_name;
	}
}

/*
 * Printstanza ( stanza pointer )
 * A debugging utility that prints a formatted stanza.
 */
void
printstanza(s)
stanza *s;
{
	if (s->dev.ds_type == S_DEVICE) {
		printf("\tDevice Stanza '%s'\n",s->dev.ds_name->name);
		printf("boot code file:\t\t'%s'\n",s->dev.ds_boot?s->dev.ds_boot:"none");
		printf("device name:\t\t'%s'\n",s->dev.ds_device?s->dev.ds_device:"none");
		printf("serial #:\t\t'%s'\n",s->dev.ds_dserial?s->dev.ds_dserial:"none");
		printf("# of heads:\t\t%d\n",s->dev.ds_heads);
		printf("# of cyls:\t\t%d\n",s->dev.ds_cyls);
		printf("# of sectors:\t\t%d\n",s->dev.ds_sectors);
		printf("# of bytes/sector:\t%d\n",s->dev.ds_bpsec);
		printf("vtoc sector:\t\t0x%x\n",s->dev.ds_vtocsec);
		printf("alt table sector:\t0x%x\n",s->dev.ds_altsec);
		{
		node *n = s->dev.ds_badsec;

		printf("bad sectors:\t\t");
		while (n) {
			if (n->ListElem->token == RANGE) {
				printf("0x%x - 0x%x, ", n->ListElem->RangeLo,n->ListElem->RangeHi);
			} else {
				printf("0x%x, ", n->ListElem->Number);
			}
			n = n->ListNext;
		}
		printf("\n");
		}
		{
		node *n = s->dev.ds_badtrk;

		printf("bad tracks:\t\t");
		while (n) {
			if (n->ListElem->token == RANGE) {
				printf("0x%x - 0x%x, ", n->ListElem->RangeLo,n->ListElem->RangeHi);
			} else {
				printf("0x%x, ", n->ListElem->Number);
			}
			n = n->ListNext;
		}
		printf("\n");
		}
	} else if (s->part.ps_type == S_PART) {
		printf("\tPartition Stanza '%s'\n",s->part.ps_name->name);
		printf("partition #:\t\t%d\n",s->part.ps_partno);
		printf("starting sector #:\t%ld\n",s->part.ps_start);
		printf("partition size:\t\t%ld\n",s->part.ps_size);
		printf("partition tag:\t\t");
		switch(s->part.ps_ptag) {
		case V_BOOT:    printf("BOOT\n");       break;
		case V_ROOT:    printf("ROOT\n");       break;
		case V_DUMP:    printf("DUMP\n");       break;
		case V_HOME:    printf("HOME\n");       break;
		case V_SWAP:    printf("SWAP\n");       break;
		case V_STAND:   printf("STAND\n");      break;
		case V_USR:     printf("USR\n");        break;
		case V_VAR:     printf("VAR\n");        break;
		case V_BACKUP:  printf("BACKUP\n");     break;
		case V_ALTS:    printf("ALTS\n");       break;
		case V_ALTTRK:	printf("ALT TRACKS\t");	break;
		case V_OTHER:   printf("OTHER\n");      break;
		defualt:        printf("Unknown type %d\n",s->part.ps_ptag);
		}
		printf("permissions:\t\t");
		{
		int p = s->part.ps_perm;

		if (p) {
			if (p&V_UNMNT) { printf("UNMOUNT ");  p&=~V_UNMNT; }
			if (p&V_RONLY) { printf("RONLY ");    p&=~V_RONLY; }
			if (p&V_OPEN)  { printf("OPEN ");     p&=~V_OPEN;  }
			if (p&V_VALID) { printf("VALID ");    p&=~V_VALID; }
			if (p) { printf("unknown bits 0x%x",p); }
		} else {
			printf("No permission bits set!");
		}
		printf("\n");
		}
	} else {
		printf("Unknown stanza type %d for stanza '%s'\n",s->part.ps_type,s->part.ps_name->name);
	}
}

/*
 * mkpartopen() will open the specified device and set the dev_fd_open
 * flag to 1. This open was pulled out of getdevice() so it may be
 * called separately for determining if this is a scsi device.
 * getdevice() in turn has been changed to use this routine for opening
 * the device.
 */

int
mkpartopen(name)
char *name;
{
	struct stat statbuf;
	/** used for determining if this is a scsi disk **/
	struct bus_type	bus_type;

	if(dev_fd_open)
		return(0);

	if (stat(name,&statbuf) == -1) {
		perror(name);
		return(1);
	} else if ( !(statbuf.st_mode & S_IFCHR) ) {
		fprintf(stderr,"Device %s is not character special\n",name);
		return(1);
	}

	if ((devfd = open(name, 2 )) == -1) {
		fprintf(stderr,"Opening device ");
		perror(name);
		return(1);
	}
	/** If the B_GETTYPE ioctl succeeds and the bus_name is "scsi" **/
	/** Then it's time to see if the options given on the command  **/
	/** line are valid for scsi disks. If they're not valid, give  **/
	/** advice and handle the alts flag, otherwise continue on.    **/

	if(ioctl(devfd, B_GETTYPE, &bus_type) >= 0 &&
		!strncmp(bus_type.bus_name, "scsi", 4))
	{
		is_scsi_dev = 1;
		if(not_for_scsi == 1)
		{
fprintf(stderr,"mkpart [-P add_partition_name] [-p remove_partition_name]\n");
fprintf(stderr,"   [-b] [-B boot_code_file] [-f partition_file]\n");
fprintf(stderr,"   [-t vp] [-x filename] [-i] device\n");
fprintf(stderr,"\n");
fprintf(stderr,"NOTE: multiple P and p flags may be specified.\n");
fprintf(stderr,"-b just rewrites the boot code, as determined from the partition file.\n");
fprintf(stderr,"-B specifies a different boot file.\n");
fprintf(stderr,"\tThe last arg is a raw UNIX System device (e.g. /dev/rdsk/1s0)\n");
fprintf(stderr,"-f specifies the partition file; its absence implies %s.\n",
	PARTFILE);
fprintf(stderr,"-t asks for a listing of:\n");
fprintf(stderr,"   p - partitions,\n");
fprintf(stderr,"   v - vtoc and physical drive characteristics.\n");
fprintf(stderr,"-x writes a complete device and partition stanza list for the\n");
fprintf(stderr,"   specified device to file 'filename' (useful for recovery).\n");
fprintf(stderr,"-i initializes the device, ignoring any existing VTOC data.\n");
fprintf(stderr,"   MUST BE USED if the device has never been formatted;\n");
fprintf(stderr,"   may be used to re-initialize a drive.\n");
fprintf(stderr,"\n");
fprintf(stderr,"NOTE: The options; -A, -F, -ta, -V, and -v are not valid for SCSI disks.\n");
fprintf(stderr,"      For the following mkpart options, use the designated SCSI command:\n\n");
fprintf(stderr,"      -A:	%s/%s\n",SCSICMDS,SCSIALTS);
fprintf(stderr,"      -F:	%s/%s\n",SCSICMDS,SCSIFORMAT);
fprintf(stderr,"      -ta:	%s/%s\n",SCSICMDS,SCSIALTS);
fprintf(stderr,"      -V:	%s/%s\n",SCSICMDS,SCSIVERIFY);
fprintf(stderr,"      -v:	%s/%s\n",SCSICMDS,SCSIVERIFY);
			
			mkpartclose();
			exit(0);
		}
		else if(not_for_scsi == 2)
		{
			/** Remove the TF_ALTS flag from targ **/
			/** This way the default for scsi   **/
			/** disks will be TF_PARTS and TF_VTOC **/

			targ &= ~TF_ALTS;
		}
	}

	/** indicate dev is open **/
	dev_fd_open = 1;
	return(0);
}

/*
 * mkpartclose() is the counterpart of mkpartopen() in that it
 * closes the device and sets the dev_fd_open flag to 0 to
 * indicate that the device is not open.
 */

int
mkpartclose()
{
	close(devfd);
	dev_fd_open=0;
	return(0);
}

