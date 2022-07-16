/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)qt:cmd/tapecntl.c	1.3"
/* This program will be used by the tape utlities shell scripts to provide
*  basic control functions for tape erase,retention,rewind,reset, and 
*  tape positioning via file-mark count. The tapecntl function is defined by
*
*	Name: tapecntl	- tape control
*
*	Synopsis: tapecntl	[ options ] [ arg ]
*
*	Options:  -e		erase
*		  -t		retention
*		  -r		reset
*		  -w		rewind
*		  -p [n]	position - position to filemark (n)
*/

#include <fcntl.h>
#include "sys/types.h"
#include "sys/errno.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/param.h"
#include <sys/tape.h>
#include <stdio.h>


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


void exit();

main(argc,argv)
int argc;
char **argv;
{
	register int c,tp,arg;
	extern char *optarg;
	extern int optind;
	extern int errno;

	if (argc < 2)
	{
		fprintf(stderr,"%s",USAGE);
		exit(1);
	}
	signal(SIGINT,SIG_DFL);

	while(( c = getopt(argc,argv,"etrwp:")) != EOF)
		switch ( c ) {

		case 'e':
			if((tp = open("/dev/rmt/c0s0",O_RDONLY))<0) { 
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
			break;

		case 't':
			if((tp = open("/dev/rmt/c0s0",O_RDONLY))<0) { 
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
			break;

		case 'r':
			if((tp = open("/dev/rmt/c0s0",O_RDONLY))<0) { 
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
			break;

		case 'w':
			if((tp = open("/dev/rmt/c0s0",O_RDONLY))<0) { 
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
			break;

		case 'p':
			if((tp = open("/dev/rmt/c0s0n",O_RDONLY))<0) { 
				fputs(OPN,stderr);
				fputs(CHECK,stderr);
				exit(4);
	  		}
			arg = atoi(optarg);
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
			break;

		case '?':
			fprintf(stderr,"%s",USAGE);
			exit(1);
		}

	if (optind == 1)
	{
		fprintf(stderr,"%s",USAGE);
		exit(1);
	}
	exit(0);
}
