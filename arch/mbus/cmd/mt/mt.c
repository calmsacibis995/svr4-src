/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1986, 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:cmd/mt/mt.c	1.3.1.1"

static char mt_copyright[] = "Copyright 1986, 1988 Intel Corp. 463028";

#include <sys/types.h>
#include <sys/param.h>
#include <sys/tape.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>

long	atol();
short	look_up_command();
char *prognam;

extern int	opterr;
extern char	*optarg;
extern int	optind;

extern errno;

#define	NOT_A_COMMAND			0
#define	WRITE_FILE_MARK			1
#define	SEEK_FORWARD_FILE		2
#define	SEEK_FORWARD_RECORD		3
#define	SEEK_BACKWARD_FILE		4
#define	SEEK_BACKWARD_RECORD	5
#define	REWIND_ONLY				6
#define	OFFLINE					7
#define	REWIND_AND_OFFLINE		8
#define	STATUS					9
#define	RETENSION				10
#define	ERASE					11

struct comtab {
	char	*string;
	short	action;
};

struct comtab comtab[] = {
	"eof", 			WRITE_FILE_MARK,
	"weof", 		WRITE_FILE_MARK,
	"fsf", 			SEEK_FORWARD_FILE,
	"fsr", 			SEEK_FORWARD_RECORD,
	"bsf", 			SEEK_BACKWARD_FILE,
	"bsr", 			SEEK_BACKWARD_RECORD,
	"rewind", 		REWIND_ONLY,
	"offline", 		OFFLINE,
	"rewoffl", 		REWIND_AND_OFFLINE,
	"status", 		STATUS,
	"retension", 	RETENSION,
	"erase", 		ERASE
};


char	*use0 = 
"Usage: mt [-f tapename] command [ count ]\n";

/* tapecntl error messages */
char ERAS[] ="Erase function failure.\n";
char WRPRT[] = "Write protected cartridge.\n";
char CHECK[] = "Please check that cartridge or device cables are inserted properly.\n";
char RET[] = "Retension function failure.\n";
char CHECK1[] = "Please check equipment thoroughly before retry.\n";
char REW[] = "Rewind function failure.\n";
char POS[] = "Positioning function failure.\n";
char BMEDIA[] ="No Data Found - End of written media or bad tape media suspected.\n";
char RESET[] = "Reset function failure.\n";
char OPN[] = "Device open failure.\n";
char USAGE[] = "Usage: tapecntl [ -etrw ] [ -p arg ] \n";

giveusage()
{
	if ((strcmp(prognam,"mt")) && (strcmp(prognam,"./mt"))){
		fprintf(stderr,"%s",USAGE);	
		exit(1);
	}else{
		fprintf(stderr, use0);
		exit(1);
	}
}


main(argc, argv)
int	argc;
char	**argv;
{
	char	command[80];			/* command string from user   */
	long	count = 1;			/* how many times to do it.   */
	int	op;				/* ioctl operation value      */
	int topts;				/* tapecntl type options used */
	char	*device = "/dev/rmt/c0s0n";	/* The name of the device.    */
	int	devfd;				/* File descriptor for above. */
	char	c;

	prognam = argv[0];
	opterr = 0;
	topts = 0;
	signal(SIGINT,SIG_DFL);

	while(( c = getopt(argc,argv,"etrmwp:f:")) != EOF){
		switch ( c ) {
		case 'e':
			topts++;
			(void)strcpy(command,"erase");
			break;
		case 't':
			topts++;
			(void)strcpy(command,"retension");
			break;
		case 'm':
			topts++;
			(void)strcpy(command,"eof");
			break;
		case 'r':
			topts++;
			(void)strcpy(command,"rewind");
			break;
		case 'w':
			topts++;
			(void)strcpy(command,"rewind");
			break;
		case 'p':
			topts++;
			(void)strcpy(command,"fsf");
			count = atoi(optarg);
			break;
		case 'f':	/* new device to open */
			device = optarg;
			continue;
		case '?':
			fprintf(stderr,"%s",USAGE);
			exit(1);
		}
	}

	if (opterr)
		giveusage();

	if (topts == 0){
		if (optind < argc) 
			strcpy(command,argv[optind++]);
		else
			giveusage();

		if (optind < argc)
			count = atol( argv[optind++] );
	
		if (count <= 0 ) {
			fprintf(stderr, "%s: '%d' not a sane count\n", prognam,count);
			exit(1);
		}
	}else {
		if (argc < 2) {
			fprintf(stderr,"%s",USAGE);
			exit(1);
		}
	}

	if (optind < argc)
		giveusage();

	/*
	 * Grab the device is available.
	 */
	if ((devfd = open(device, 2 )) == -1) {
		perror(prognam);
		exit(2);
	}

	switch (look_up_command(command)) {
	case WRITE_FILE_MARK:
		op = T_WRFILEM;
		break;
	case SEEK_FORWARD_FILE:
		op = T_SFF;
		break;
	case REWIND_ONLY:
		op = T_RWD;
		count = 1;
		break;
	case RETENSION:
		op = T_RETENSION;
		count = 1;
		break;
	case ERASE:
		op = T_ERASE;
		count = 1;
		break;

	case SEEK_FORWARD_RECORD:
	case SEEK_BACKWARD_FILE:
	case SEEK_BACKWARD_RECORD:
	case OFFLINE:
	case REWIND_AND_OFFLINE:
	case STATUS:
		fprintf(stderr, "%s: command '%s' not available\n", prognam,command);
		exit(1);

	case NOT_A_COMMAND:
	default:
		fprintf(stderr, "%s: command '%s' not recognized\n", prognam,command);
		exit(1);
	}

	while (count-- > 0) {
		switch (op) {
		case T_SFF:
			if (ioctl(devfd, op, 1) == -1) {
				perror(prognam);
				exit(1);
			}
		default:
			if (ioctl(devfd, op, (caddr_t)NULL) == -1) {
				perror(prognam);
				exit(1);
			}
		}
	}

	close(devfd);
	exit(0);
}


/**************************************************************************
 *		main ends here.  below are subroutines			  *
 **************************************************************************
 */

/*****************************************************************************
 *
 * TITLE:	look_up_command
 *
 * ABSTRACT:	Scan through the command table to match the users
 *				request with the things we can do.
 *
 *				Only enough of the command to be unique is required.
 *
 * CALLS:
 *
 * CALLING ROUTINES:	main
 *
 ****************************************************************************/
short
look_up_command(command)
char	*command;
{
	unsigned	i;
	short	ctbsize = (sizeof(comtab) / sizeof(struct comtab ));
	short	comidx = NOT_A_COMMAND;
	register char	*cpc, *cpt;

	for (i = 0; i < ctbsize; i++) {
		for (cpc = command, cpt = comtab[i].string; *cpc && (*cpc == *cpt); cpc++, cpt++)
			;
		if (!(*cpc)) {
			if (comidx == NOT_A_COMMAND)
				comidx = comtab[i].action;
			else {
				fprintf(stderr, "%s: command '%s' not unique\n", prognam,command);
				exit(1);
			}
		}
	}

	if (comidx == NOT_A_COMMAND) {
		fprintf(stderr, "%s: command '%s' not recognized\n", prognam,command);
		exit(1);
	}

	return(comidx);
}
