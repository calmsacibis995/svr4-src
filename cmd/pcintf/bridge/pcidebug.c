/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/pcidebug.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)pcidebug.c	3.7	LCC);	/* Modified: 15:06:51 3/23/88 */

/*****************************************************************************

	Copyright (c) 1985 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#include	<stdio.h>
#include	<fcntl.h>
#include	<errno.h>
#include	<string.h>
#include	<ctype.h>
#include	<signal.h>
#include	"log.h"

static char scratch[] = "\n# Merge 286/386 copyright (c) 1985, 1987 by Locus Computing Corporation.\n# All Rights Reserved.\n";

extern int
	errno;

#define	strequ(s1, s2)	(strcmp((s1), (s2)) == 0)

#ifdef	SYS5
#define	SIG_DBG1	SIGUSR1
#endif	/* SYS5 */

#ifndef	SIG_DBG1
#include	"Unknown Unix version"
#endif	/* SIG_DBG1 */

long
	getBitList();			/* Decode bit number lists */

char
	*usageMesg = "pcidebug: Usage %s <svrPid> <[=+-~]chanList|on|off|close|child> [...]\n\t\tchanList = chanNum1[,chanNum2[...]]\n";

main(argc, argv)
int
	argc;
char
	*argv[];
{
register int
	argN;				/* Current argument number */
register char
	*arg;				/* Current argument */
long
	setChans = 0,			/* Use this channel set */
	onChans = 0,			/* Turn these channels on */
	offChans = 0,			/* Turn these channels off */
	flipChans = 0,			/* Invert these channels */
	changeTypes = 0;		/* Types of changes requested */
int
	chanDesc,			/* Descriptor of debug channel file */
	svrPid;				/* Process id of server */
char
	chanName[32];			/* Name of debug channel file */

	/* Validate usage */
	if (argc < 3) {
		fprintf(stderr, usageMesg, argv[0]);
		exit(1);
	}

	/* Check to see that first arg is a number */
	if (!isdigit(*argv[1])) {
		fprintf(stderr, usageMesg, argv[0]);
		exit(1);
	}

	/* Check to see that the number is the pid of an existing process */
	svrPid = atoi(argv[1]);
	if (kill(svrPid, 0) < 0) {
		if (errno == ESRCH)
			fprintf(stderr, "pcidebug: Process %d doesn't exist\n",
				svrPid);
		else if (errno == EPERM)
			fprintf(stderr, "pcidebug: No permission to signal process %d\n", svrPid);

		else
			fprintf(stderr, "pcidebug: Can't signal process %d\n",
				svrPid);

		exit(1);
	}

	/* Scan rest of argument and get new debug channel assignments */
	for (argN = 2; argN < argc; argN++) {
		arg = argv[argN];

		if (strequ(arg, "child"))		/* Change child */
			changeTypes |= CHG_CHILD;
		else if (strequ(arg, "off")) {
			setChans = 0;
			changeTypes |= CHG_SET;
		}
		else if (strequ(arg, "on")) {		/* Turn everything on */
			setChans = 0xffffL;
			changeTypes |= CHG_SET;
		}
		else if (strequ(arg, "close"))		/* Close log file */
			changeTypes |= CHG_CLOSE;
		else if (*arg == '=') {			/* Set abs. channels */
			setChans = getBitList(&arg[1]);
			changeTypes |= CHG_SET;
		}
		else if (*arg == '+') {			/* Add channels */
			onChans = getBitList(&arg[1]);
			changeTypes |= CHG_ON;
		}
		else if (*arg == '-') {			/* Remove channels */
			offChans = getBitList(&arg[1]);
			changeTypes |= CHG_OFF;
		}
		else if (*arg == '~') {			/* Toggle channels */
			flipChans = getBitList(&arg[1]);
			changeTypes |= CHG_INV;
		}
		else {					/* else Bad argument */
			fprintf(stderr, "pcidebug: Invalid argument: \"%s\"\n",
				arg);
			exit(1);
		}
	}

	/* Invent file name and create it */
	sprintf(chanName, chanPat, svrPid);
	if ((chanDesc = open(chanName, O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0)
	{
		fprintf(stderr, "Cannot create channel file %s (%d)\n",
			chanName, errno);
		exit(1);
	}

	/* Write the debug bits to channel file */
	write(chanDesc, &changeTypes, sizeof changeTypes);
	write(chanDesc, &setChans, sizeof setChans);
	write(chanDesc, &onChans, sizeof onChans);
	write(chanDesc, &offChans, sizeof offChans);
	write(chanDesc, &flipChans, sizeof flipChans);
	close(chanDesc);

	/* Signal the server to read the channel file */
	if (kill(svrPid, SIG_DBG1) < 0) {
		if (errno == ESRCH)
			fprintf(stderr, "pcidebug: Process %d disappeared\n",
				svrPid);
		else if (errno == EPERM)
			fprintf(stderr, "pcidebug: Lost permission to signal process %d\n",
				svrPid);

		else
			fprintf(stderr, "pcidebug: Can't signal process %d (%d)\n",
				svrPid, errno);

		exit(1);
	}
}


/*
   getBitList: Decode a list of bit numbers
		return a long with the indicated bits set
*/

long
getBitList(bitString)
register char
	*bitString;
{
register long
	theBits = 0;			/* Decoded bits */
register int
	bitNum;				/* A bit number */
char
	*bitEnd;			/* Char that ended number conversion */

	/* Scan list of numbers and accumulate bits */
	for (;;) {
		bitNum = strtol(bitString, &bitEnd, 0x0);
		/* If a number was converted, add it's bit to mask */
		if (bitEnd != bitString)
			if (bitNum <= 0 || bitNum > 32)
				fprintf(stderr, "pcidebug: Bit %d is out of range\n", bitNum);
			else
				theBits |= 1L << bitNum - 1;

		bitString = bitEnd;

		while (!isdigit(*bitString))
			if (*bitString++ == '\0')
				return theBits;
	}
}
