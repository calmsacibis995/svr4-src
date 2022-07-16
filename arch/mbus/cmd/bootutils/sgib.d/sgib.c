/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988, 1989 Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:cmd/bootutils/sgib.d/sgib.c	1.3"

#ifndef lint 
static char sgib_copyright[] = "Copyright 1988, 1989 Intel Corp. 463021";
#endif 

/*
**	Install MSA  Bootstrap 
*/

#include <sys/types.h>
#include <stdio.h> 
#include <sys/fdisk.h>
#include <sys/ivlab.h>
#include <string.h>
#include "sgib.h"

extern int 	getopt();
extern char	*optarg;
extern int	optind;
extern void	exit();
extern long	atol();
extern void	mkboot();
extern void	mkrboot();
extern void	mkbtblk();
extern void	writebuff();

static void	giveusage();

struct  btblk btblk;			/* Definition of first block	*/
ushort	dev_gran = 	1024;		/* Default for Maxtor 140	*/
ushort	nfheads = 	15;		/* Default for Maxtor 140	*/
ushort	nrheads = 	0;		/* Default for Maxtor 140	*/
ushort	nsec = 		9;		/* Default for Maxtor 140	*/
ushort	ncyl = 		918;		/* Default for Maxtor 140	*/
ushort	ileave = 	1;		/* Default for Maxtor 140	*/
ushort	fsdelta =	270;		/* location of super block (max 140)*/
static char	*device = "/dev/tty";	/* Output device		*/
char	*boot = "/etc/dsboot" ;		/* Default MSA bootloader 	*/
char	*rboot = "/etc/boot" ;		/* Default Real Mode bootloader */
char	*msa_text_buf;			/* Buffer for MSA text (no data)*/
char	*real_buf_1;			/* First part of Real Mode text+data  */
char	*real_buf_2;			/* Rest of Real Mode text+data  */
uint	msa_text_loc;			/* MSA text location size 	*/
uint	msa_text_size;			/* MSA text size 		*/
uint	msa_data_size;			/* MSA data size 		*/
int	type = BDISK;			/* type of btblk, default is disk */
int	delta_flag = 0;			/* fsdelta on cmd line		*/
#ifdef MB1
int rboot_flag = 1;
#else
int rboot_flag = 0;
#endif
int	debug =	0;			/* Debug variable		*/

main(argc, argv)
int	argc;
char	**argv ;
{
	int c;


	while ((c = getopt(argc, argv, "BDFM:N:RTd:i:f:o:r:c:s:")) != EOF) {
		switch (c) {
		case 'B':
			type = BSERVER;		/* build a boot server imags */
			break ;
		case 'D':
			type = BDISK;		/* build a hard disk image */
			break ;
		case 'F':
			type = BFLOP;		/* build a floppy image */
			break ;
		case 'M':
			boot = optarg;	/* file name of MSA boot loader */
			break ;
		case 'N':
			rboot = optarg; /* file name of NON-MSA boot loader */
			break ;
		case 'R':
			rboot_flag = 1; /* real mode flag */
			break ;
		case 'T':		/* build a tape image -  both msa and */
			type = BTAPE;   /* non msa are made the same way */
			dev_gran = 	512;
			nfheads = 	0;
			nrheads = 	1;
			nsec	 = 	0;
			ncyl	 = 	0;
			ileave	 = 	0;
			break ;
		case 'd':		/* number of bytes per sector */
			dev_gran = (ushort)atol(optarg) ;
			break ;
		case 'i':		/* disk interleave */
			ileave = (ushort)atol(optarg) ;
			break ;
		case 'f':		/* number of fixed heads  */
			nfheads = (ushort)atol(optarg) ;
			break ;
		case 'o':		/* sector offset of root filesystem */
			delta_flag++;
			fsdelta = (ushort)atol(optarg) ;
			break ;
		case 'r':		/* number of removable heads */
			nrheads = (ushort)atol(optarg) ;
			break ;
		case 'c':		/* number of cylinders */
			ncyl = (ushort)atol(optarg) ;
			break ;
		case 's':		/* number of sec /track */
			nsec = (ushort)atol(optarg) ;
			break ;
		default:
			giveusage() ;
			break ;
		}
	}


	/* lets get the device names */

	if ( optind == (argc - 1))
		device = argv[optind++];
	else
		giveusage();

	if (optind != argc)
		giveusage() ;

	/* get the bootloaders */

	if (rboot_flag)
 		mkrboot ();
	mkboot  ();

	/* fill in the boot blk */

	mkbtblk ();  

	/* write it all out */

	writebuff(type, device);
	return(0);
}
void
giveusage()
{
	ERR(stderr, "Usage: sgib [- B | -D | -F | -T]\n");
	ERR(stderr, "\t[-M <MSA boot file]\n");
	ERR(stderr, "\t[-N <non msa boot file>]\n");
	ERR(stderr, "\t[-R <real mode boot flag>]\n");
	ERR(stderr, "\t[-o <fsdelta >]");
	ERR(stderr, "\t[-d <dev_gran]\n");
	ERR(stderr, "\t[-i <interleave>]\n");
	ERR(stderr, "\t[-c <# cylinders>]\n");
	ERR(stderr, "\t[-s <# sect/track>]\n");
	ERR(stderr, "\t[-f <# fixed heads>]\n");
	ERR(stderr, "\t[-r <# removable heads>]");
	ERR(stderr, "\toutput_device \n");
	exit(2);
}
