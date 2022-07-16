/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:cmd/bootutils/hdformat.d/hdformat.c	1.3"

static char prog_copyright[] = "Copyright 1989 Intel Corp. 464464";

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <sys/param.h>
#include <fcntl.h>
#include <sys/vtoc.h>
#include <sys/hd.h>
#include <errno.h>
#include <sys/bbh.h> 
#include <sys/fdisk.h> 
#include <sys/alttbl.h>
#include <sys/ivlab.h>
#include <ctype.h> 
#include <memory.h> 
#include <sys/types.h> 
#include <sys/stat.h> 

#include "../sgib.d/sgib.h"  

#define mdlintlv	1		/* interleave for the mdl track */
#define WHOLEPART	0		/* the partition # after V_CONFIG */

#ifdef __STDC__
extern int	getopt();
extern int	atoi();
extern int	ioctl();
extern int	close();
extern int 	errno;
extern void 	exit();
#endif
static	giveusage();
static  write_structs();
static	int formatdevice();

static int		fd;		/* file descriptor for the disk */

struct vtoc		vtoc;
struct absio 		absio;
struct pdinfo		pdinfo;
struct drtab		drtab;
struct ivlab		ivlab;	
struct alt_info		alt_info;
static struct stat	my_stat;
static struct btblk	btblk;
static union io_arg	ia;

static char	intlv;		/* interleave factor from -F arg */

static int	heads;		/* heads on the drive */
static int	head_flag; 
static int	bpsec;		/* bytes per sector */
static int	bpsec_flag; 
static int 	sectors;	/* sectors per track */
static int	sectors_flag; 
static int	cyls;		/* cyls on the drive */
static int	cyl_flag;
static int	start;		/* start of the drive */
static int 	fmt_flag;
static char	devicenode[80];

main(argc, argv)
int	argc;
char	**argv;
{
	static char     options[] = "s:f:c:d:i:";
	extern char	*optarg;
	extern int	optind;
	int		c, i, err;
	int		mdltype;

	heads = cyls = bpsec = sectors = 0;

	while ( (c=getopt(argc, argv, options)) != EOF ) {
		switch (c) {
		case 'i': /* do a V_FMTPART with interleave */
			{
				fmt_flag++;
				i = atoi(optarg);
				if ((i < 0) || (i > 16)) {
					if(fprintf(stderr,"hdformat: Illegal interleave (0-16)\n")==-1)
						perror("hdformat: ");
					exit(1);
				}
				intlv = (char) i;
			}
			break;
		case 'c': /* cyls on the drive */
			{
			cyls = atoi(optarg);
			cyl_flag++;
			}
			break;
		case 'd': /* bytes per sector */
			{
			bpsec = atoi(optarg);
			if((bpsec!=256) && (bpsec!=512) && (bpsec!=1024) && (bpsec!=2048)){
				if(fprintf(stderr,"hdformat: bad value for density %d\n",bpsec)==-1)
						perror("hdformat: ");
				(void)giveusage();
				exit(1);
			}
			bpsec_flag++;
			}
			break;
		case 's': /* sectors/trk on the drive */
			{
			sectors = atoi(optarg);
			if(sectors < 1){
				if(fprintf(stderr,"hdformat: bad value for sectors ( > 0 )\n")==-1)
						perror("hdformat: ");
				(void)giveusage();
				exit(1);
			}
			sectors_flag++;
			}
			break;
		case 'f': /* heads on the drive */
			{
			heads = atoi(optarg);
			if(heads < 1){
				if(fprintf(stderr,"hdformat: bad value for heads ( > 0 )\n")==-1)
						perror("hdformat: ");
				(void)giveusage();
				exit(1);
			}
			head_flag++;
			}
			break;
		default:
			if(fprintf(stderr,"Invalid option '%s'\n",argv[optind])==-1)
						perror("hdformat: ");
			(void)giveusage();
			exit(1);
		}
	}

		/* get the last argument -- device */
	if (argc != optind+1) {
		if(fprintf(stderr,"hdformat: Missing or bad device name ie. /dev/rdsk/1s0\n")==-1)
						perror("hdformat: ");
		(void)giveusage();
		exit(1);
	}
	if(strcpy(devicenode,argv[optind]) == NULL)
		perror("hdformat: ");

	if((sectors_flag != 1) || (head_flag != 1) || (bpsec_flag != 1)
	   || (cyl_flag != 1) || (fmt_flag != 1)){
		(void)giveusage();
		exit(2);
	}
	/*
	 * need to sanity check cyls here 'cause now i know the value for heads
	 */
	if(cyls < (4 + NALTS(heads,cyls))){ 
		if(fprintf(stderr,"hdformat: not enough cylinders specified\n") == -1)
			perror("hdformat: "); 
		(void)giveusage();
		exit(3);
	}
	
	/* 
	 * need to open the disk Xs0 
	 */
	if ((fd = open(argv[optind], O_RDWR)) == -1 ) {
		perror("hdformat: opening disk ");
		exit(4);
	}
	
	/*
	 * make sure they use the char. device
	 */
	if(stat(argv[optind], &my_stat) == -1)
		perror("hdformat: stat");
	if((my_stat.st_mode & S_IFMT) != S_IFCHR){
		if(fprintf(stderr,"hdformat: must be character device\n") == -1)
			perror("hdformat: ");
		(void)giveusage();
		exit(5);
	}
	ia.ia_cd.ncyl = (unsigned short)cyls;
	ia.ia_cd.nhead = (unsigned char)heads;
	ia.ia_cd.nsec = (unsigned char)sectors;
	ia.ia_cd.secsiz = (unsigned short)bpsec;
	if ((int)ioctl(fd, V_CONFIG, &ia) == -1) {
		perror("hdformat: V_CONFIG");
		exit(6);
	}
	/*
	 * put down the btblk.signature
	 */
	absio.abs_buf = (char *)&btblk;
	absio.abs_sec = BTBLK_LOC;

	btblk.signature = BTBLK_MAGIC;
	drtab.dr_ncyl	= (ushort)cyls;
	drtab.dr_nfhead	= (char)heads;
	drtab.dr_nrhead	= 0;
	drtab.dr_nsec	= (char)sectors;
	drtab.dr_lsecsiz	= lobyte(bpsec);
	drtab.dr_hsecsiz	= hibyte(bpsec);
	drtab.dr_nalt	= NALTS(heads, cyls);

	if(memcpy((char *)btblk.ivlab.v_dspecial,(char *)&drtab,sizeof(struct drtab)) == NULL)
		perror("hdformat: ");

	if(ioctl(fd, V_WRABS, &absio) == -1){
		perror("hdformat: V_WRABS failed ");
		exit(7);
	}

	pdinfo.sanity = (unsigned long)VALID_PD;
	pdinfo.vtoc_ptr = (VTOC_SEC * bpsec) + sizeof(struct pdinfo);
	pdinfo.vtoc_len = sizeof(struct vtoc);
	pdinfo.alt_ptr = (VTOC_SEC + 1) * bpsec;
	pdinfo.alt_len = sizeof(struct alt_info);
	pdinfo.cyls = cyls;
	pdinfo.tracks = heads;
	pdinfo.sectors = sectors;
	pdinfo.bytes = bpsec;

	pdinfo.mfgst = (cyls - 3) * heads * sectors;
	pdinfo.mfgsz = heads * sectors * 2;

	pdinfo.relst = START_ALT(heads,cyls,sectors)*sectors;
	pdinfo.relnext = pdinfo.relst;
	pdinfo.relsz = pdinfo.mfgst - pdinfo.relst;
	
	if(ioctl( fd, V_L_PDIN, &pdinfo) == -1 ){
		perror("hdformat: V_L_PDIN");
		exit(8);
	}

	start = 0;
	vtoc.v_sanity = VTOC_SANE;
	vtoc.v_nparts = 1;
	vtoc.v_part[WHOLEPART].p_flag = V_UNMNT | V_VALID;
	vtoc.v_part[WHOLEPART].p_tag  = V_BACKUP;
	vtoc.v_part[WHOLEPART].p_start= start;
	vtoc.v_part[WHOLEPART].p_size = pdinfo.relst;

	if(ioctl( fd, V_L_VTOC,&vtoc) == -1 ){
		perror("hdformat: V_L_VTOC failed");
		exit(9);
	}

	mdltype = I_ST506;
	if(ioctl( fd, V_R_MDL, &mdltype) == -1 ){
		mdltype = I_ESDI;
		if(ioctl( fd, V_R_MDL, &mdltype) == -1 ){
			perror("hdformat: The manufacturer's defect list must be written prior to format\n");
			exit(10);
		}
	}
	alt_info.alt_sanity = ALT_VERSION;
	alt_info.alt_sanity = ALT_SANITY;
	if(ioctl( fd, V_L_SWALT, &alt_info) == -1 ){
		perror("hdformat: V_L_SWALT failed");
		exit(11);
	}

	if((err = formatdevice()) != 0){
		if(close(fd) == -1)
			perror("hdformat: ");
		return(err);
	}

	/*
	 * put down the btblk.sstructure
	 */
	if(ioctl(fd, V_WRABS, &absio) == -1){
		perror("hdformat: V_WRABS failed ");
		exit(12);
	}
	/*
	 * write out the rest of the structs pdinfo, vtoc, altinfo
	 */
	(void)write_structs();

	if(close(fd) == -1)
		perror("hdformat: ");
	return(0);
} /* end of prog */

/*
 * Formatdevice()
 */
static
formatdevice()
{
	struct fmtpart fmtpart;
	int    rep;
	
	if(printf("\nABOUT TO FORMAT ENTIRE DRIVE.\nTHIS WILL ") == -1)
		perror("hdformat: ");
	if(printf("DESTROY ANY DATA ON %s.  Continue (y/n)? ",devicenode)== -1)
		perror("hdformat: ");
	do{
		rep = getc(stdin);
	}while ( rep != 'n' && rep != 'N' && rep != 'Y' && rep != 'y' );

	if( rep == 'N' || rep == 'n')
		return(0); /* we are done */

	if(printf("FORMATTING  %s\n",devicenode)== -1)
		perror("hdformat: ");
	fmtpart.partnum = WHOLEPART; 	/* partition 0 */
	fmtpart.method = FMTANY;	/* set method  */
	fmtpart.intlv = intlv;		/* set interleave */

	if (ioctl(fd, V_FMTPART, &fmtpart) == -1){
		perror("hdformat: V_FMTPART failed");
		return(errno);
	}

	if(fprintf(stderr,"\nFormat complete.\n")== -1)
		perror("hdformat: ");
	return(0);
}

/*
 * write out the pdinfo vtoc and swalt structures
 */
write_structs()
{
	if(ioctl( fd, V_W_PDIN, NULL) == -1 ){
		perror("hdformat: V_W_PDIN");
		exit(13);
	}
	if(ioctl( fd, V_W_VTOC,NULL) == -1 ){
		perror("hdformat: V_W_VTOC failed");
		exit(14);
	}
	if(ioctl( fd, V_W_SWALT,NULL) == -1 ){
		perror("hdformat: V_W_SWALT failed");
		exit(15);
	}
	return(0);
}
/*
 * Giveusage ()
 * Give a concise message on how to use this prog. 
 */
giveusage()
{
	if(printf("hdformat -i -c -d -s -h /dev/rdsk/Xs0  \n\n") == -1)
		perror("hdformat: ");
	if(printf("        -> -i:	the interleave of the disk\n") == -1)
		perror("hdformat: ");
	if(printf("        -> -c: 	cyls on the drive \n") == -1)
		perror("hdformat: ");
	if(printf("        -> -d: 	bytes per sector \n") == -1)
		perror("hdformat: ");
	if(printf("        -> -s: 	sectors/trk on the drive \n") == -1)
		perror("hdformat: ");
	if(printf("        -> -f: 	fixed heads on the drive \n") == -1)
		perror("hdformat: ");
	return(0);
}
