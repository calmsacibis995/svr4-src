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

#ident	"@(#)mbus:cmd/bootutils/hdformat.d/mdl.c	1.3"

static char prog_copyright[] = "Copyright 1989 Intel Corp. 463990-010";

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <memory.h>
#include <sys/types.h>
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
#ifdef __STDC__
#include <sys/types.h> 
#endif
#include <sys/stat.h> 
#include "../sgib.d/sgib.h"

#define MAXHEAD   99
#define MINHEAD   0
#define MAXCYL	  9999
#define MINCYL	  1
#define MAXDISK	  999
#define MINDISK	  0
	
#define WHOLEPART	0		/* the partition # after V_CONFIG */
#define mdlintlv	1
extern int errno;
#ifdef __STDC__
extern int	close();
static void giveusage();
static void	fill_mdl();
static void	print_mdl();
static void	write_mdl();
static int	get_mdl();
static void	format_mdl();
static void	b_print_mdl();
static void	b_fill_mdl();
static int	verify_mdl();
extern int	getopt();
extern int	atoi();
extern void exit();
#endif


static int		debug =  0;	/* 1=turn on a lot of print statements */
static int		debug0 = 0;	/* 1=print what function is executing */
static int		fd;		/* file descriptor for the disk */

struct disk_parms   dp;         /* device parameters */
struct vtoc		vtoc;
static union esdi506mdl mdl;
struct absio 		absio;
struct pdinfo		pdinfo;
struct ivlab		ivlab;
struct drtab		drtab;
struct alt_info		alt_info;
static struct stat	my_stat;
static union io_arg		ia;

static int	write_flag	=	0;
static int	new_mdl		=	0;
static char 	*buf; /* buffer for absio */

static int	heads;		/* heads on the drive */
static int	head_flag; 
static int	bpsec;		/* bytes per sector */
static int	bpsec_flag; 
static int 	sectors;	/* sectors per track */
static int	sectors_flag; 
static int	cyls;		/* cyls on the drive */
static int	cyl_flag;
static int	start;		/* start of the drive */
static int	batch; 
static char	devicenode[80];

static void	initialize_mdl();

main(argc, argv)
int	argc;
char	**argv;
{
	static char     options[] = "HANDbs:f:c:d:";
	extern char	*optarg;
	extern int	optind;
	int		c;

	heads = cyls = bpsec = sectors = 0;

	while ( (c=getopt(argc, argv, options)) != EOF ) {
		switch (c) {
		case 'c': /* cyls on the drive */
			{
			cyls = atoi(optarg);
			cyl_flag++;
			}
			break;
		case 'd': /* bytes per sector */
			{
			bpsec = atoi(optarg);
			bpsec_flag++;
			}
			break;
		case 's': /* sectors/trk on the drive */
			{
			sectors = atoi(optarg);
			sectors_flag++;
			}
			break;
		case 'f': /* heads on the drive */
			{
			heads = atoi(optarg);
			head_flag++;
			}
			break;
		case 'b': /* do this in batch mode */
			batch++;
			break;
		case 'D':
			debug++;
			debug0++;
			break ;
		case 'N': /* write an new mdl */
			new_mdl++;
			write_flag++;
			break ;
		case 'A': /* add to an existing mdl */
			write_flag++;
			break ;
		case 'H': /* help */
			giveusage();
			exit(2);
			break ;
		default:
			fprintf(stderr,"Invalid option '%s'\n",argv[optind]);
			giveusage();
			exit(3);
		}
	}

		/* get the last argument -- device */
	if (argc != optind+1) {
		fprintf(stderr,"Missing or bad device name ie. /dev/rdsk/1s0\n");
		giveusage();
		exit(4);
	}


	if((sectors_flag != 1) || (head_flag != 1) || (bpsec_flag != 1)
	   || (cyl_flag != 1)){
		giveusage();
		exit(4);
	}
	/*
	 * need to sanity check cyls here 'cause now i know the value for heads
	 */
	if(cyls < (4 + NALTS(heads,cyls))){ 
		if(fprintf(stderr,"mdl: not enough cylinders specified\n") == -1)
			perror("mdl: "); 
		giveusage();
		exit(4);
	}
	
	strcpy(devicenode,argv[optind]);
					/* need to open the disk Xs0 */
	if ((fd = open(argv[optind], O_RDWR)) == -1) {
		perror("mdl: opening disk ");
		exit(5);
	}
	if(stat(argv[optind], &my_stat) == -1)
		perror("mdl: stat");
	if((my_stat.st_mode & S_IFMT) != S_IFCHR){
		if(fprintf(stderr,"mdl: must be character device\n") == -1)
			perror("mdl: ");
		giveusage();
		exit(4);
	}
	if(debug)printf("node %s fd=%d\n",argv[optind],fd);	

	ia.ia_cd.ncyl = (unsigned short)cyls;
	ia.ia_cd.nhead = (unsigned char)heads;
	ia.ia_cd.nsec = (unsigned char)sectors;
	ia.ia_cd.secsiz = (unsigned short)bpsec;
	if ((int)ioctl(fd, V_CONFIG, &ia) == -1) {
		perror("V_CONFIG");
		exit(6);
	}

	drtab.dr_ncyl	= (ushort)cyls;
	drtab.dr_nfhead	= (char)heads;
	drtab.dr_nrhead	= 0;
	drtab.dr_nsec	= (char)sectors;
	drtab.dr_lsecsiz	= lobyte(bpsec);
	drtab.dr_hsecsiz	= hibyte(bpsec);
	drtab.dr_nalt	= NALTS(heads, cyls);

	if(memcpy((char *)ivlab.v_dspecial,(char *)&drtab,sizeof(struct drtab))== NULL)
		perror("mdl: memcpy");
	if(ioctl( fd, V_L_VLAB,&ivlab) == -1 ){
		fprintf(stderr,"V_L_VLAB failed errno = %d\n",errno);
		exit(9);
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
		perror("V_L_PDIN");
		exit(10);
	}

	start = 0;
	vtoc.v_sanity = VTOC_SANE;
	vtoc.v_nparts = 1;
	vtoc.v_part[WHOLEPART].p_flag = V_UNMNT | V_VALID;
	vtoc.v_part[WHOLEPART].p_tag  = V_BACKUP;
	vtoc.v_part[WHOLEPART].p_start= start;
	vtoc.v_part[WHOLEPART].p_size = heads * cyls * sectors;

	if(ioctl( fd, V_L_VTOC,&vtoc) == -1 ){
		perror("V_L_VTOC failed");
		exit(12);
	}

	alt_info.alt_version = ALT_VERSION;
	alt_info.alt_sanity = ALT_SANITY;
	if(ioctl( fd, V_L_SWALT, &alt_info) == -1 ){
		perror("V_L_SWALT failed");
		exit(14);
	}

	/*
	 * if the command line wants to lie we'll go along 
	 */
	if (get_mdl() == 1){		/* get the MDL from the disk */
		format_mdl();		/* no valid mdl so make new one */
		write_flag++;		/* if=0, mdl is on the disk */
		new_mdl++;
	}				/* the driver doesnt format the mdl */
					/* YET so format_mdl() */
	if(batch){
		if(new_mdl){
			initialize_mdl();
			b_fill_mdl();
			write_mdl();
		}else /* not write so must be read */
			b_print_mdl();
	}else{ /* it is interactive */
		if (write_flag){		/* put the new mdl together */
			if(new_mdl){	
				do{
					initialize_mdl();
					fill_mdl(new_mdl);
					write_mdl();
				}while(verify_mdl() != 0);
			}else{ /* it is append mode */
				while(verify_mdl() != 0){
					fill_mdl(new_mdl);
					write_mdl();
				}
			}
		}else /* just show the mdl */
			print_mdl();
	}	
	close(fd);
	return(0);
} /* end of prog */

/*
 * verify that the list in the mdl is correct
 */
verify_mdl()
{
	char 		resp[10];

	printf("\n");
	print_mdl();
	printf("\n\nIs this list correct? ");
	fflush(stdin);
	gets(resp);
	if ((resp[0] == 'N') || (resp[0] == 'n'))
		return(1);
	else
		return(0);

}

/*
 * a batch mode fill operation
 */
#ifdef __STDC__
void
#endif
b_fill_mdl()
{
	int i, j, sane, cnt, heads;
	int	cyl, offset, length	;

	if(debug0)printf("b_fill_mdl\n");

	if (dp.dp_type==DPT_ESDI_HD) {
		scanf("%d\n",&heads);
		i=0	;
		while (i < heads) {
			if ( scanf("%d\n", &sane) != 1) {
				fprintf (stderr, "b_fill_mdl : Incomplete mdl \n");
				exit (1)		;
			}
			if(sane == BBHESDIMDLVALID) {
				mdl.esdimdl[i].header.magic = sane;
				mdl.esdimdl[i].header.head  = i;
				j=0	;
				while (1) {
					if (scanf("%d %d %d\n", &cyl, &offset, &length) != 3) {
						fprintf (stderr, "b_fill_mdl : Incomplete mdl \n");
						exit (1)		;
					}
					if (cyl == 0xFFFF)  /* End of defect for this surface. */
						break	;
					mdl.esdimdl[i].defects[j].hi_cyl = (cyl >> 8) & 0xff;
					mdl.esdimdl[i].defects[j].lo_cyl = cyl & 0xff;
					mdl.esdimdl[i].defects[j].hi_offset = (offset >> 8) & 0xff;
					mdl.esdimdl[i].defects[j].lo_offset = offset & 0xff;
					mdl.esdimdl[i].defects[j].length = length ;
					++j	;
				}
			}
			else {
				fprintf (stderr, "b_fill_mdl : Invalid sanity %d for MDL ...\n",
								sane);
				exit (1)	;
			}

			++i	;
		}
	}
	else {
		scanf("%x %d",&sane,&cnt);
		if(sane == BBH506MDLVALID){
			mdl.st506mdl.header.bb_valid = sane;
			mdl.st506mdl.header.bb_num = cnt;
			for (i=0;i < cnt;i++){
				scanf("%d %d",&mdl.st506mdl.defects[i].be_cyl,
									&mdl.st506mdl.defects[i].be_surface);
				mdl.st506mdl.defects[i].be_reserved = 48;
			}
		}
		else {
			fprintf (stderr, "b_fill_mdl : Invalid sanity %d for MDL ...\n",
								sane);
			exit (1)	;
		}
	}
	write_mdl();
}

/*
 * batch mode expects the input EXACTLY this form
 */
#ifdef __STDC__
void
#endif
b_print_mdl()
{
	unsigned short	i;
	int j, surface;
	int	cylnum 	;
	
	if (dp.dp_type==DPT_ESDI_HD) {
		int	surface	;

		surface = 0	;

		fprintf (stdout, "%d\n", heads)	;
		for (surface=0; surface < heads; surface++ ) {
			fprintf (stdout, "%d\n", mdl.esdimdl[surface].header.magic);
			for (j=0; j<BBHESDIMAXDFCTS; j++) {
				fprintf (stdout, "%d %d %d\n", 
						   (mdl.esdimdl[surface].defects[j].lo_cyl |
						   ((mdl.esdimdl[surface].defects[j].hi_cyl << 8) 
												& 0xff00)), 
							(mdl.esdimdl[surface].defects[j].lo_offset | 
						   ((mdl.esdimdl[surface].defects[j].hi_offset << 8)
												& 0xff00)),
							 mdl.esdimdl[surface].defects[j].length) ;
				cylnum = (mdl.esdimdl[surface].defects[j].lo_cyl |
				   ((mdl.esdimdl[surface].defects[j].hi_cyl << 8) & 0xff00)); 
				if (cylnum == 0xFFFF) /* End of defects for this surface. */
					break	;
			}
		}	
	}
	else {
		fprintf(stdout,"%x         %d         \n",
						mdl.st506mdl.header.bb_valid,mdl.st506mdl.header.bb_num);
		for (i=0;i < mdl.st506mdl.header.bb_num;i++){
			fprintf(stdout,"%d         %d         \n", 
				mdl.st506mdl.defects[i].be_cyl,mdl.st506mdl.defects[i].be_surface);
		}
	}

}

/*
 * Formats the tracks used to store the Manufacturer's Defect List (MDL).
 */
#ifdef __STDC__
void
#endif
format_mdl()
{
	int		rtrack;		/* Track # (partition rel) of MDL */
	int		ntrack;		/* Number of tracks to be formatted. */
	int		retrycnt;	/* Count the # of format retries */
	int		i;			/* throwaway var */
	union io_arg    ia;	/* Parameter block for ioctl */
	int  	mfgtrk; 	/* Track number of MDL partition.*/

	if(debug0)printf("format_mdl()\n");

	/* 
	 * Issue V_GETPARMS to see if we have ESDI or ST506 drive.
	 */
	if (ioctl(fd, V_GETPARMS, &dp) == -1) {
		fprintf( stderr, " GETPARMS on ");
		perror("devicenode");
		exit(67);
	}

	/*
	 * Calc the track # (partition relative) that contains the MDL.
	 */
		
	mfgtrk = pdinfo.mfgst / sectors	;

	if (dp.dp_type == DPT_ESDI_HD) { 
		/* 
		 * Esdi MDL is written on all the tracks of that cylinder.
		 */
		rtrack = mfgtrk;
		ntrack = dp.dp_heads;
	}
	else 	 {
		switch (dp.dp_secsiz) {
			case 1024:
				rtrack = mfgtrk + (2 * dp.dp_heads - 4);
				break;
			case 512:
				rtrack = mfgtrk + (2 * dp.dp_heads - 3);
				break;
			case 256:
				rtrack = mfgtrk + (2 * dp.dp_heads - 2);
				break;
			case 128:
				rtrack = mfgtrk + (2 * dp.dp_heads - 1);
				break;
		}

		ntrack = 1	;
	}

	/*
	 * Set up the parameter block to format the MDL track.
	 */
	while (ntrack > 0) {
		ia.ia_fmt.num_trks = 1;
		ia.ia_fmt.intlv = mdlintlv;
		ia.ia_fmt.start_trk = (ushort) (rtrack + ntrack - 1);
	
		/*
		 * Try formatting the track several times before giving up.
		 */
		for (retrycnt=0; retrycnt < 5; retrycnt++) {
			if (ioctl(fd, V_FORMAT, &ia) == 0)
				break;
		}

		/*
		 * If the track could not be formatted then give up.
		 */
		if (retrycnt == 5) {
			fprintf(stderr,
				"\nFAILED format manufacture partition 5 times at track %ld.\n",
				ia.ia_fmt.start_trk) ;
			exit (1)	;
		
		}

		--ntrack;
	}
	
}

/*
 * load the MDL from the disk to struct mdl
 */
get_mdl()
{
	unsigned int i;

	i = I_ESDI;
	if (ioctl(fd, V_R_MDL, &i) != -1) {
		if ( (ioctl (fd, V_U_MDL, &mdl) != -1) && 
			 (mdl.esdimdl[0].header.magic == BBHESDIMDLVALID) )
				return (0);
	}
	else {
		i = I_ST506;
		if (ioctl(fd, V_R_MDL, &i) != -1) {
			if ( (ioctl (fd, V_U_MDL, &mdl) != -1) && 
				 ((mdl.st506mdl.header.bb_valid == BBH506MDLVALID) && 
			    (mdl.st506mdl.header.bb_num <= BBH506MAXDFCTS)) )
				return (0);
		}
	}

	/* I cannot read a valid mdl off the disk. */
	return (1);

}

/*
 * write struct mdl to the disk 
 */
#ifdef __STDC__
void
#endif
write_mdl()
{

	if (ioctl (fd, V_L_MDL, &mdl) != -1) {
		if (ioctl (fd, V_W_MDL, NULL) != -1) 
			return ;
		else {
			perror ("write_mdl : V_W_MDL")	;
			exit (1)						;
		}
	}
	else {
		perror ("write_mdl : V_L_MDL")	;
		exit(1);
	}
}

/*
 * print the mdl struct
 */
#ifdef __STDC__
void
#endif
print_mdl()
{
unsigned short i;
int surface, cnt, printcnt,newline;
int	cylnum;

  if((mdl.st506mdl.header.bb_valid == BBH506MDLVALID) && 
						(mdl.st506mdl.header.bb_num < 256)){
	printf("Number of Defects %d\n",mdl.st506mdl.header.bb_num);
	if (mdl.st506mdl.header.bb_num == 0) {
		/*
		 * Nothing to do.
		 */
		return	;
	}
	printf("Head-Cyl        Head-Cyl        Head-Cyl        Head-Cyl\n");
	printf("--------------------------------------------------------");
	for (i=0;i < mdl.st506mdl.header.bb_num;i++){
		if( (i % 4) == 0)
			printf("\n");
		printf("%.2d-",mdl.st506mdl.defects[i].be_surface);
		printf("%.4d         ",   mdl.st506mdl.defects[i].be_cyl);
	}
	printf("\n");

  }else if (mdl.esdimdl[0].header.magic == BBHESDIMDLVALID) {

	printf("Head-Cyl-Offset-Length        Head-Cyl-Offset-Length\n");
	printf("----------------------------------------------------\n");
	for (printcnt=0,newline=0,surface=0; surface < heads; surface++ )
		for (cnt=0; cnt<BBHESDIMAXDFCTS; cnt++){
			if (newline)  {
				printf("\n");
				newline = 0	;
			}
			
			cylnum = (mdl.esdimdl[surface].defects[cnt].lo_cyl |
				   ((mdl.esdimdl[surface].defects[cnt].hi_cyl << 8) & 0xff00)); 
			if (cylnum != 0xFFFF) {
				if ((++printcnt % 2) == 0) 
					newline = 1;
				printf("%.2d-%.4d-%.5d-%.4d            ", 
				surface,
				(mdl.esdimdl[surface].defects[cnt].lo_cyl |
				   ((mdl.esdimdl[surface].defects[cnt].hi_cyl << 8) & 0xff00)), 
				(mdl.esdimdl[surface].defects[cnt].lo_offset |
				   ((mdl.esdimdl[surface].defects[cnt].hi_offset<<8)& 0xff00)), 
				mdl.esdimdl[surface].defects[cnt].length);
			}
			else 
				break	; /* End of defects for this surface. */

		}
		printf("\n");

  } else {
	printf("There is no valid MDL currently on the disk\n");
	mdl.st506mdl.header.bb_num=0;
	return;
  }
}

/*
 * fill in the mdl struct with the bad blocks
 */
#ifdef __STDC__
void
#endif
fill_esdimdl()
{
	int i;
	int cyl,head,offset, length;
	char ans[20];
	char ans1[20];
	char ans2[20];
	char ans3[20];
	char ans4[20];
	char response[20];
	int	cylnum	;

	if(debug0)printf("fill_esdimdl()\n");
	while (1) {
		printf ("Do you want to continue ? [<y>/n] : ")	;
        fflush(stdout);
		if ( (gets(response) != NULL) && (response [0] == 'n') ) {
            printf("Are you sure you've entered all defects (y/<n>)? ");
        	fflush(stdout);
            if ( (gets(response) != NULL) && (response[0] == 'y') ) 
				break	;
		}

		while (1){
			printf("\nHead: ");
			(void)gets(ans1);
			head = atoi(ans1);
			if (head < MINHEAD || head >MAXHEAD) {
				printf("Illegal number of heads(%d)\n",head);
				continue;
			}
			break;
		}
		while (1){
			printf("Cyl: ");
			(void)gets(ans2);
			cyl = atoi(ans2);
			if (cyl < MINCYL || cyl >MAXCYL) {
				printf("Illegal number of cylinders (%d)\n",cyl);
				continue;
			}
			break;
		}
		while (1){
			printf("Offset (in bytes): ");
			(void)gets(ans3);
			offset = atoi(ans3);
			break;
		}
		while (1){
			printf("Length of defect (in bits): ");
			(void)gets(ans4);
			length = atoi(ans4);
			break;
		}
		printf("\nHead: %.2d    ",head);
		printf("Cylinder: %.4d ",cyl);
		printf("Offset: %.4d    ",offset);
		printf("Length: %.4d\n",length);
		printf("OK <y|n> ? ");
		(void)gets(ans);
		if(ans[0] == 'y') {
			/* Find the next available entry for this surface. */
			for (i=0; i<BBHESDIMAXDFCTS; i++) {
				cylnum = (mdl.esdimdl[head].defects[i].lo_cyl |
				   ((mdl.esdimdl[head].defects[i].hi_cyl << 8) & 0xff00)); 
				if (cylnum == 0xFFFF) 
					break	;
			}

			mdl.esdimdl[head].defects[i].lo_cyl = cyl & 0xff	;
			mdl.esdimdl[head].defects[i].hi_cyl = (cyl >> 8) & 0xff	;
			mdl.esdimdl[head].defects[i].lo_offset = offset & 0xff	;
			mdl.esdimdl[head].defects[i].hi_offset = (offset >> 8) & 0xff	;
			mdl.esdimdl[head].defects[i].length = length	;
		} 
		else {
			printf("Entry ignored");
		}
	}
	printf("\n\n\n");
}

#ifdef __STDC__
void
#endif
fill_st506mdl()
{
	int i;
	int cyl,head;
	char ans[20];
	char ans1[20];
	char ans2[20];
	char response[20];

	if(debug0)printf("fill_st506mdl()\n");

	printf("Current number of defects %d\n", mdl.st506mdl.header.bb_num);

	for (i=mdl.st506mdl.header.bb_num;i < BBH506MAXDFCTS;i++){
		if(debug){
			printf("%.2d-",mdl.st506mdl.defects[i].be_surface);
			printf("%.4d                ",   mdl.st506mdl.defects[i].be_cyl);
		}
	
		printf ("Do you want to continue ? [<y>/n] : ")	;
        fflush(stdout);
		if ( (gets(response) != NULL) && (response [0] == 'n') ) {
            printf("Are you sure you've entered all defects (y/<n>)? ");
        	fflush(stdout);
            if ( (gets(response) != NULL) && (response[0] == 'y') ) 
				break	;
		}

		while (1){
			printf("\nHead: ");
			(void)gets(ans1);
			head = atoi(ans1);
			if (head < MINHEAD || head >MAXHEAD) {
				printf("Illegal number of heads(%d)\n",head);
				continue;
			}
			break;
		}
		while (1){
			printf("Cyl: ");
			(void)gets(ans2);
			cyl = atoi(ans2);
			if (cyl < MINCYL || cyl >MAXCYL) {
				printf("Illegal number of cylinders (%d)\n",cyl);
				continue;
			}
			break;
		}
		printf("\nHead: %.2d    ",head);
		printf("Cylinder: %.4d\n",cyl);
		printf("OK <y|n> ? ");
		(void)gets(ans);
		if (ans[0] == 'y') {
			mdl.st506mdl.defects[i].be_surface = head;
			mdl.st506mdl.defects[i].be_cyl = cyl;
			mdl.st506mdl.defects[i].be_reserved = 48;
			if(debug){
				printf("%.2d-",mdl.st506mdl.defects[i].be_surface);
				printf("%.4d                ",  mdl.st506mdl.defects[i].be_cyl);
			}
			mdl.st506mdl.header.bb_num++;
			if(ans1[0] == 'q' || ans2[0] == 'q') {
				mdl.st506mdl.header.bb_num--;
			}	
		} 
		else {
			printf("Entry ignored");
		}
	}
	printf("\n\n\n");
}

/*
 * fill in the mdl struct with the bad blocks
 */
#ifdef __STDC__
void
#endif
fill_mdl(iflag)
unsigned short iflag;
{
	if (iflag) { /* I initialized the mdl */
		if (dp.dp_type == DPT_ESDI_HD) 
			fill_esdimdl()	;
		else 
			fill_st506mdl()	;
	}
	else {
		/*
		 * I did not initialize the mdl. So look at the existing mdl to 
		 * determine the kind of mdl to be filled.
		 */
		if (mdl.st506mdl.header.bb_valid == BBH506MDLVALID) 
			fill_st506mdl()	;
		else 
			fill_esdimdl()	;
		
	}
}

#ifdef __STDC__
void
#endif
initialize_mdl()
{
	int	surface, i	;

	if (dp.dp_type == DPT_ESDI_HD) {
		for (surface=0; surface < BBHESDIMAXHEADS; surface++) {
			mdl.esdimdl[surface].header.magic = BBHESDIMDLVALID;
			mdl.esdimdl[surface].header.version = 0x3130		;
			for (i=0; i<BBHESDIMAXDFCTS; i++) {
				mdl.esdimdl[surface].defects[i].hi_cyl = 0xFF	;
				mdl.esdimdl[surface].defects[i].lo_cyl = 0xFF	;
			}
		}
	}
	else {
		mdl.st506mdl.header.bb_valid = BBH506MDLVALID	;	
		mdl.st506mdl.header.bb_num = 0	;	
	}
}

/*
 * Giveusage ()
 * Give a concise message on how to use this prog. 
 */
#ifdef __STDC__
void
#endif
giveusage()
{
	printf("mdl [[-A | -N ] & [ -c -d -s -f /dev/rdsk/Xs0 ] \n\n");
	printf("        -> -A:	write additional defect info to disk.\n");
	printf("        -> -N:	write a new defect list to disk.\n");
	printf("        -> -c: 	cyls on the drive \n");
	printf("        -> -d: 	bytes per sector \n");
	printf("        -> -s: 	sectors/trk on the drive \n");
	printf("        -> -f: 	fixed heads on the drive \n");
	printf("        -> -b: 	do this in batch mode (uses stdin/stdout)\n");
}
