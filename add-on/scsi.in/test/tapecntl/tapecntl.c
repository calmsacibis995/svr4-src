#ident	"@(#)tapecntl.c	1.2	92/12/06	JPB"

/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)scsi.in:test/tapecntl/tapecntl.c	1.3.1.1"

/* This program will be used by the tape utilities shell scripts to provide
*  basic control functions for tape erase, retension, rewind, reset, and
*  tape positioning via file-mark count. The tapecntl function is defined by
*
*	Name: tapecntl	- tape control
*
*	Synopsis: tapecntl	[ options ] [ arg ]
*
*	Options:  -b		read block length limits
*		  -l		load
*		  -u		unload
*		  -e		erase
*		  -t		retension
*		  -r		reset
*		  -v		set variable length block mode
*		  -f (n)	set fixed block length to (n) bytes
*		  -w		rewind
*		  -p (n)	position - position to filemark (n)
*/

#include <fcntl.h>
#include "sys/types.h"
#include "sys/errno.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/param.h"
#include "sys/buf.h"
#include "sys/scsi.h"
#include "sys/sdi.h"
#include "sys/st01.h"
#include "sys/tape.h"
#include <stdio.h>

static char INVLENMSG[] ="Invalid block size. Must be in multiple of 512 bytes.\n";
static char ERAS[] ="Erase function failure.\n";
static char WRPRT[] = "Write protected cartridge.\n";
static char CHECK[] = "Please check tape or cables are installed properly.\n";
static char RET[] = "Retension function failure.\n";
static char CHECK1[] = "Please check equipment thoroughly before retry.\n";
static char LOADMSG[] ="Load function failure.\n";
static char UNLOADMSG[] ="Unload function failure.\n";
static char BLKLENMSG[] ="Read block length limits function failure.\n";
static char BLKSETMSG[] ="Set block length function failure.\n";
static char REW[] = "Rewind function failure.\n";
static char POS[] = "Positioning function failure.\n";
static char BMEDIA[] ="No Data Found - End of written media or bad tape media suspected.\n";
static char RESET[] = "Reset function failure.\n";
static char OPN[] = "Device open failure.\n";
static char USAGE[] = "Usage: tapecntl [ -belrtuvw ] [ -f arg ] [ -p arg ] [ device ] \n";
static char USAGE2[] = "\n\
tapecntl: -b		read block length limits\n\
	  -e		erase\n\
	  -l		load\n\
	  -r		reset \n\
	  -t		retension\n\
	  -u		unload\n\
	  -v		set variable length block mode\n\
	  -w		rewind\n\
	  -f (n)	set fixed block length to (n) bytes\n\
	  -p (n)	position - space (n) filemarks forward\n";


void exit();

main(argc,argv)
int argc;
char **argv;
{
	int	c,tp,arg,length;
	extern char *optarg;
	extern int optind;
	extern int errno;

	char	*device;
	struct  tape tpp;
	struct	blklen *blp;

	char	stat_buf[6];
	int	blen = 0;
	int	load = 0;
	int	unload = 0;
	int	erase = 0;
	int	retension = 0;
	int	reset = 0;
	int	variable = 0;
	int	fixed = 0;
	int	rewind = 0;
	int	position = arg = 0;

	blp = &tpp.t_blklen;

	if (argc < 2)
	{
		fprintf(stderr,"%s",USAGE2);
		exit(1);
	}
	signal(SIGINT,SIG_DFL);

	device = "/dev/rst/c0s0n";

	while(( c = getopt(argc,argv,"bluetrvf:wp:?")) != EOF)
		switch ( c ) {

		case 'b':
			blen = 1;
			break;

		case 'l':
			load = 1;
			break;

		case 'u':
			unload = 1;
			break;

		case 'e':
			erase = 1;
			break;

		case 't':
			retension = 1;
			break;

		case 'r':
			reset = 1;
			break;

		case 'v':
			variable = 1;
			break;

		case 'f':
			fixed = 1;
			length = atoi(optarg);
			break;

		case 'w':
			rewind = 1;
			break;

		case 'p':
			position = 1;
			arg = atoi(optarg);
			break;

		case '?':
			fprintf(stderr,"%s",USAGE);
			exit(1);
		}

	if ( optind + 1 == argc )
		device = argv[optind];
	else if ( optind + 1 < argc ) {
		fputs(USAGE, stderr);
		exit(1);
	}
	if (optind == 1) {
		fprintf(stderr,"%s",USAGE);
		exit(1);
	}

	if ( blen ) {
		if((tp = open(device, O_RDONLY))<0) { 
			fputs(OPN,stderr);
			fputs(CHECK,stderr);
			exit(4);
  		}
		if(ioctl(tp,T_RDBLKLEN,blp,sizeof(struct blklen))<0) {
		   	close (tp);
		   	fputs(BLKLENMSG, stderr);
			fputs(CHECK,stderr);
			exit(1);
		}
		printf("Block length limits:\n");
		printf("\tMaximum block length = %d\n", blp->max_blen);
		printf("\tMinimum block length = %d\n", blp->min_blen);
		close (tp);
		exit(0);
	}

	if ( variable ) {
		if((tp = open(device, O_RDONLY))<0) { 
			fputs(OPN,stderr);
			fputs(CHECK,stderr);
			exit(4);
  		}
		blp->max_blen = 0;
		blp->min_blen = 0;
		if(ioctl(tp,T_WRBLKLEN,blp,sizeof(struct blklen))<0) {
			perror("T_WRBLKLEN failed");
		   	if (errno == ENXIO)
				fputs(BLKSETMSG, stderr);
			fputs(CHECK,stderr);
		   	close (tp);
			exit(1);
		}
		close (tp);
		exit(0);
	}


	if ( fixed ) {
		if (length%512) {
			fputs(INVLENMSG, stderr);
			exit(4);
		}
		if((tp = open(device, O_RDONLY))<0) { 
			fputs(OPN,stderr);
			fputs(CHECK,stderr);
			exit(4);
  		}
		blp->max_blen = length;
		blp->min_blen = length;
		if(ioctl(tp,T_WRBLKLEN,blp,sizeof(struct blklen))<0) {
			perror("T_WRBLKLEN failed");
		   	if (errno == ENXIO)
				fputs(BLKSETMSG, stderr);
			fputs(CHECK,stderr);
		   	close (tp);
			exit(1);
		}
		close (tp);
		exit(0);
	}

	if ( load ) {
		if((tp = open(device, O_RDONLY))<0) { 
			fputs(OPN,stderr);
			fputs(CHECK,stderr);
			exit(4);
  		}
		if(ioctl(tp,T_LOAD,stat_buf,6)<0) {
		   	close (tp);
		   	fputs(LOADMSG, stderr);
			fputs(CHECK,stderr);
			exit(1);
		}
		close (tp);
		exit(0);
	}
	if ( unload ) {
		if((tp = open(device, O_RDONLY))<0) { 
			fputs(OPN,stderr);
			fputs(CHECK,stderr);
			exit(4);
  		}
		if(ioctl(tp,T_UNLOAD,stat_buf,6)<0) {
		   	close (tp);
		   	fputs(UNLOADMSG, stderr);
			fputs(CHECK,stderr);
			exit(1);
		}
		close (tp);
		exit(0);
	}
	if ( erase ) {
		if((tp = open(device, O_RDONLY))<0) { 
			fputs(OPN,stderr);
			fputs(CHECK,stderr);
			exit(4);
  		}
		if(ioctl(tp,T_ERASE,0,0)<0) {
		   	close (tp);
		   	fputs(ERAS, stderr);
		   	if(errno == ENXIO) {
				fputs(CHECK,stderr);
				exit(1);
		      	} else
		    	if(errno == EROFS) {
				fputs(WRPRT,stderr);
				exit(3);
		      	} else {
				fputs(CHECK1,stderr);
				exit(2);
		      	}
		}
		close (tp);
		exit(0);
	}

	if ( retension ) {
		if((tp = open(device, O_RDONLY))<0) { 
			fputs(OPN,stderr);
			fputs(CHECK,stderr);
			exit(4);
  		}
		if(ioctl(tp,T_RETENSION,0,0)<0)  {
			close (tp);
			fputs (RET, stderr);
			if(errno == ENXIO) {
				fputs(CHECK,stderr);
				exit(1);
			} else {
				fputs(CHECK1,stderr);
				exit(2);
			}
		}
		close (tp);
		exit(0);
	}
	if ( reset ) {
		if((tp = open(device, O_RDONLY))<0) { 
			fputs(OPN,stderr);
			fputs(CHECK,stderr);
			exit(4);
  		}
		if (ioctl(tp,T_RST,0,0)<0) {
			close (tp);
			fputs(RESET,stderr);
			fputs(CHECK,stderr);
			exit(1);
		}
		close (tp);
		exit(0);
	}
	if ( rewind ) {
		if((tp = open(device, O_RDONLY))<0) { 
			fputs(OPN,stderr);
			fputs(CHECK,stderr);
			exit(4);
  		}
		if(ioctl(tp,T_RWD,0,0)<0) {
			close (tp);
		 	fputs (REW, stderr);
	         	if(errno == ENXIO) {
				fputs(CHECK,stderr);
				exit(1);
		   	} else {
				fputs(CHECK1,stderr);
				exit(2);
		   	}
		}
		close (tp);
		exit(0);
	}
	if ( position ) {
		if((tp = open(device, O_RDONLY))<0) { 
			fputs(OPN,stderr);
			fputs(CHECK,stderr);
			exit(4);
  		}
		if(ioctl(tp,T_SFF,arg,0)<0) {
			close (tp);
			fputs (POS, stderr);
			if(errno == ENXIO) {
				fputs(CHECK,stderr);
				exit(1);
			} else
		  	if(errno == EIO) {
				fputs(BMEDIA,stderr);
				exit(2);
		  	}
		}
		close (tp);
		exit(0);
	}
	return(0);
}
