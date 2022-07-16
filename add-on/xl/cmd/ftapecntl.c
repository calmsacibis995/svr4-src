/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)xl:cmd/ftapecntl.c	1.3"

/*	Copyright (c) 1989 Intel Corp.				*/
/*	  All Rights Reserved  	*/
/*
 *	INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *	This software is supplied under the terms of a license
 *	agreement or nondisclosure agreement with Intel Corpo-
 *	ration and may not be copied or disclosed except in
 *	accordance with the terms of that agreement.
 */

/************************************************************************/
/*	Copyright (c) 1988, 1989 ARCHIVE Corporation			*/
/*	This program is an unpublished work fully protected by the	*/
/*	United States Copyright laws and is considered a trade secret	*/
/*	belonging to Archive Corporation.				*/
/************************************************************************/
/*	file: ftapecntl.c						*/
/************************************************************************/
/* This program will be used by the tape utlities shell scripts to provide
*  basic control functions for tape status,retention,rewind,reset, and 
*  format. The device path can be passed as an argument otherwise it defaults
*  to /dev/rmt/f1q80c. The ftape function is defined by
*
*	Name: ftapecntl	- floppy tape control
*
*	Synopsis: ftapecntl	[ options ] [ arg ]
*
*	Options:  -s		status
*		  -t		retention
*		  -r		reset
*		  -w		rewind
*		  -f [n]	format n tracks
*/

#include <fcntl.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/dir.h>
#include <sys/signal.h>
#include <sys/param.h>
#include <stdio.h>
#include "../sys/ftape.h"

#define	NUMTRKS	28		/* Maximum number of track	*/

char WRPRT[] = "Write protected cartridge.\n";
char CHECK[] = "Please check that cartridge or device cables are inserted properly.\n";
char RET[] = "Retension function failure.\n";
char CHECK1[] = "Please check equipment thoroughly before retry.\n";
char REW[] = "Rewind function failure.\n";
char FMT[] = "Format function failure.\n";
char STS[] = "Status function failure.\n";
char RESET[] = "Reset function failure.\n";
char USAGE[] = "Usage: ftapecntl [ -strw ] [ -f arg ] [ device ]\n";

union{
	union	xl_status	s;
	int	ioarg;
} mt;

char	*stat_msgs[ 8 ] ={
    "ready",
    "error-detected",
    "cartridge-present",
    "write-protected",
    "new-cartridge",
    "cartridge-referenced",
    "beginning-of-tape"
};

void exit();

main(argc,argv)
int argc;
char **argv;
{
	register	int	c, tp, i, arg;
	extern		char	*optarg;
	extern		int	optind;
	extern		int	errno;
	unsigned	char	bits;
			int	linepos = 14;
			char	*device;
			char	*argptr = (char *)&mt;
			int	status = 0;
			int	retension = 0;
			int	reset = 0;
			int	rewind = 0;
			int	format = 0;
			int	tracks = 0;

	if ( argc < 2 ){
		fprintf(stderr,"%s",USAGE);
		usage();
		exit( 1 );
	}
	signal( SIGINT, SIG_DFL );

	device = "/dev/rmt/f1q80c";

	while( ( c = getopt( argc, argv, "strwf:" ) ) != EOF )
		switch( c ){
		case 's':
			status = 1;
			break;
		case 't':
			retension = 1;
			break;
		case 'r':
			reset = 1;
			break;
		case 'w':
			rewind = 1;
			break;
		case 'f':
			tracks = atoi( optarg );
			format = 1;
			break;
		case '?':
			fputs( USAGE, stderr );
			exit( 1 );
		}
	if ( optind + 1 == argc )
		device = argv[ optind ];
	else if ( optind + 1 < argc ){
		fputs( USAGE, stderr );
		exit( 1 );
	}
	if ( optind == 1 ){
		fprintf( stderr, "%s", USAGE );
		exit( 1 );
	}
	if ( format ){
		if ( ( tp = open( device, O_RDWR ) ) < 0 ){ 
			fprintf( stderr, "Device %s open failure.\n", device );
			fputs( CHECK, stderr );
			exit( 4 );
		}
		mt.ioarg = tracks;
		if ( mt.ioarg > NUMTRKS || mt.ioarg < 1 )
			mt.ioarg = NUMTRKS;
		if ( mt.ioarg%2 )	     /* always format tracks in pair */
			mt.ioarg++;
		printf( "Formatting %d tracks. This will take about %d minutes...\n", mt.ioarg, 5*(mt.ioarg/2)+6 );
		if ( ioctl( tp, XL_FORMAT, argptr ) < 0 ){
			close( tp );
			fputs( FMT, stderr );
			if ( errno == ENXIO ){
				fputs( CHECK, stderr );
				exit( 1 );
			}
			else{
				fputs( CHECK1, stderr );
				exit( 4 );
			}
		}
		close( tp );
		exit( 0 );
	}
	if ( ( tp = open( device, O_RDONLY ) ) < 0 ){ 
		fprintf( stderr, "Device %s open failure.\n", device );
		fputs( CHECK, stderr );
		exit( 4 );
	}
	if ( status ){
		if ( ioctl( tp, XL_STATUS, &mt ) < 0 ){
			close( tp );
			fputs( STS, stderr );
			if ( errno == ENXIO ){
				fputs( CHECK, stderr );
				exit( 1 );
			}
			else{
				fputs( CHECK1, stderr );
				exit( 2 );
			}
		}
		printf( "      status:" );
		bits = mt.s.stat[ 0 ];
		for ( i = 0; i < 8; ++i ){
			if ( bits & 1 ){
				linepos += strlen( stat_msgs[ i ] ) + 1;
				if ( linepos > 78 ){
					printf( "\n             " );
					linepos = 14;
				}
				printf( " %s", stat_msgs[ i ] );
			}
			bits >>= 1;
		}
		printf( "\n" );
		printf( "soft errors : %u\n", mt.s.status.sfterr );
		printf( "hard errors : %u\n", mt.s.status.hrderr );
		printf( "  underruns : %u\n", mt.s.status.undrun );
	}
	if ( retension && ioctl( tp, XL_RETEN, argptr ) < 0 ){
		close( tp );
		fputs( RET, stderr );
		if ( errno == ENXIO ){
			fputs( CHECK, stderr );
			exit( 1 );
		}
		else{
			fputs( CHECK1, stderr );
			exit( 2 );
		}
	}
	if ( reset && ioctl( tp, XL_RESET, argptr ) < 0 ){
		close( tp );
		fputs( RESET, stderr );
		fputs(CHECK,stderr);
		exit( 1 );
	}
	if ( rewind && ioctl( tp, XL_REWIND, argptr ) < 0 ){
		close( tp );
		fputs( REW, stderr );
		if ( errno == ENXIO ){
			fputs( CHECK, stderr );
			exit( 1 );
		}
		else{
			fputs( CHECK1, stderr );
			exit( 4 );
		}
	}
	close( tp );

	exit( 0 );
}

/*
 * help message
 */
usage (prog)
char *prog;
{
    fprintf (stderr,"Options:  -s          status\n");
    fprintf (stderr,"          -t          retention\n");
    fprintf (stderr,"          -r          reset fdc\n");
    fprintf (stderr,"          -w          rewind\n");
    fprintf (stderr,"          -f [n]      format n tracks, n = 1-28\n");
}

