/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:cmd/mbusutils/misc.d/cpunum.c	1.3"

static char cpunum_copyright[] = "Copyright 1987 Intel Corp. 462800";

/*
 * ics_cpunum().c
 *	Print the number of a cpu, counting up from slot 0.
 */

#include <sys/types.h>
#include <sys/param.h>
#ifdef MB2
#include <sys/ics.h>
#endif
#include <stdio.h>
#include <errno.h>

extern long	atol();
extern int 	getopt();
extern void 	exit();

extern int	opterr;
extern char	*optarg;
extern int	optind;

extern	errno;

static char	*use0 =
"Usage: ics_cpunum() [-t] [-d dev]\n";

#ifdef MB2
int	slot = ICS_MY_SLOT_ID;
#endif

static int	verbose = 0;
static int terse = 0;

static void
giveusage()
{
	(void)fprintf(stderr, use0);
	exit(1);
}

void
main(argc, argv)
int	argc;
char	**argv;
{
	int devfd,mycpu;
	char *device = "/dev/ics";
	char	c;


	/*
	 * Decode/decipher the arguments.
	 */

	opterr = 0;
	while ((c = getopt(argc,argv,"tvd:")) != EOF)
		switch(c) {

		case 'v':
			verbose++;
			continue;
		case 't':
			terse++;
			continue;
		case 'd':
			device=optarg;
			continue;
		default:
			giveusage() ;
		}

	if (opterr)
		giveusage();
#if defined (MB1) || defined (AT386)
	mycpu=0;
	if(terse)
		printf("%d\n",mycpu);
	else
		printf("Executing on CPU number %d\n",mycpu);
	exit (0);
#endif
#ifdef MB2

/*	if (optind < argc)
		slot = atol(argv[optind++]);*/

	if(verbose) {
		(void)fprintf(stdout,"device is '%s'\n",device);
		fflush(stdout);
	}

	if (optind < argc)
		giveusage();

	/*
	 * Grab the device.
	 */
	if ((devfd = open(device, 2 )) == -1) {
		perror("ics_cpunum()");
		exit(2);
	}

	if((mycpu=ics_my_cpu(devfd)) == -1) {
		perror("ics_cpunum()");
		exit(2);
	}

	if(terse)
		printf("%d\n",mycpu);
	else
		printf("Executing on CPU number %d\n",mycpu);

	close(devfd);
#endif
	exit(0);
}
