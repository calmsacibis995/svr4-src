/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)idcmd:idcheck.c	1.3"

/*
 * This command is used by Installable Drivers to return selected
 * information about the system configuration. The various forms are:
 *    "idcheck -p device-name [-i dir]"
 *    "idcheck -y device-name [-i dir]"
 *    "idcheck -v vector [-i dir]"
 *    "idcheck -d dma-channel [-i dir]"
 *    "idcheck -a -l lower_address -u upper_address [-i dir]"
 *    "idcheck -c -l lower_address -u upper_address [-i dir]"
 * This command scans sdevice and mdevice and returns:
 *    100 if an error occurs.
 *    0	  if no conflict exists.
 * Options:
 * '-r' report name of conflicting device on stdout.
 * '-p' returns a value from 1 to 31 if the package exists.
 *	The exit code is calculated from the following.
 * 	add 1 if the directory '/etc/conf/pack.d/DSP' exists.
 *	add 2 if an entry can be found in mdevice.
 *	add 4 if the System Module is in '/etc/conf/sdevice.d'.
 *	add 8 if an entry can be found in sdevice.
 *	add 16 if a Driver.o file can be found for the device.
 * '-y' returns a 1 if the specified device exists in sdevice, and the
 *      "Configure" field contains a 'Y'. If the device "Configure" field
 *      is 'N' a 0 is returned. (change made 1/27/88).
 * '-v' returns 'type' field of device that is using the vector specified.
 *	(i.e. Non-zero means another DSP is already using the vector.)
 * '-d' returns 1 if the dma channel specified is being used.
 * '-a' returns 1 if the IOA region bounded by (lower, upper) conflicts
 *	with another device.
 *	The exit code is calculated from the following.
 *	add 1 if a device using this IOA exists and does not
 *		contain the 'O' option in field 3 of mdevice.
 *	add 2 if a device using this IOA exists and does
 *		contain the 'O' option in field 3 of mdevice.
 * '-c' returns 1 if the CMA region bounded by (lower, upper) conflicts
 *	with another device.
 * '-l' address - lower bound of adress range specified in hex.
 * '-u' address - upper bound of address range specified in hex.
 *	The leading 0x is unnecessary for the '-l' and '-u' options.
 * '-i' dir - specifies the directory in which the sdevice file resides.
 *
 * Note: If the device's sdevice entry contains an 'N' (device not
 *       configured), the entire line is ignored when doing the -v, -d
 *       -a, and -c checks. (change made 1/27/88).
 */

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "inst.h"
#include "defines.h"

/* check if two line segments, (a,b) and (x,y), overlap. */
#define OVERLAP(a,b,x,y)	(!(b < x || a > y))

/* diagnostics */
#define OPT	0
#define SYS	1
#define MAST	2

/* messages */
#define	IONEOF	1
#define IUSAGE	2

/* fields or elements being searched */
#define VECTOR	1	/* search for identical interrupt vector	*/
#define	IOA	2	/* serach for overlapping I/O addresses		*/
#define SPY	3	/* check if device's type field has IOASPY	*/
#define CMA	4	/* seacrh for overlapping I/O addresses		*/
#define DMA	5	/* search for identical DMA controller		*/
#define DSP	6	/* check if DSP exists				*/
#define YN	7	/* check if DSP configured in (sdev field 2=Y/N) */

/* "master" and "system" return codes */
#define	IERROR		-1
#define IEOF		-2
#define IFOUND		-3
#define INOTEXIST	-4

/* exit codes */
#define EERROR		100
#define ENOCONFLICT	0
#define	EEXIST		1

/* store input parameters */
struct INPUT {
	char device[15];	/* name of device */
	int vector;		/* interrupt vector number */
	int dma;		/* dma channel */
	long lower;		/* lower IOA or CMS address */
	long upper;		/* upper IOA or CMA address */
	int flag;		/* field or element being searched */
	int nflag;		/* number of flags specified */
} input, temp;

int c, rc, s, m, p, f, d_o;
int verbose;
int uflag, lflag, iflag, debug;
char buf[100];
struct sdev sys;
struct mdev mas;
struct stat st_stat;
extern char *optarg;
extern char pathinst[];

main(argc, argv)
int argc;
char *argv[];
{
	while ((c = getopt(argc, argv, "#?rp:y:v:i:acl:u:d:")) != EOF)
		switch(c) {
		case 'p':
			input.nflag++;
			input.flag = DSP;
			sscanf(optarg, "%s", input.device);
			break;
		case 'v':
			input.nflag++;
			input.flag = VECTOR;
			sscanf(optarg, "%d", &input.vector);
			break;
		case 'd':
			input.nflag++;
			input.flag = DMA;
			sscanf(optarg, "%d", &input.dma);
			break;
		case 'y':
			input.nflag++;
			input.flag = YN;
			sscanf(optarg, "%s", input.device);
			break;
		case 'r':
			verbose++;
			break;
		case 'a':
			input.nflag++;
			input.flag = IOA;
			break;
		case 'c':
			input.nflag++;
			input.flag = CMA;
			break;
		case 'l':
			lflag++;
			sscanf(optarg, "%lx", &input.lower);
			break;
		case 'u':
			uflag++;
			sscanf(optarg, "%lx", &input.upper);
			break;
		case 'i':
			iflag++;
			sprintf(pathinst, "%s", optarg);
			break;
		case '?':
			message(IUSAGE);
		case '#':
			debug++;
			break;
		}

		if (input.nflag == 0 || input.nflag > 1)
			message(IUSAGE);

		if ((input.flag == IOA || input.flag == CMA) && (!lflag || !uflag))
			message(IUSAGE);

		if (debug)
			diag(OPT, 0);


	switch (input.flag) {
	case VECTOR:
	case CMA:
	case IOA:
		s = System(&input);
		if (s == IEOF)
			exit( ENOCONFLICT );
		if (s == IERROR)
			exit( EERROR );
		if (s == IFOUND){
			prdev(sys.device);
			exit( EEXIST );
		}
		prdev(sys.device);
		exit( s );

	case DMA:
		m = master(&input);
		if (m == IEOF)
			exit( ENOCONFLICT );
		if (m == IERROR)
			exit( EERROR );
		if (m == IFOUND){
			prdev(mas.device);
			exit( EEXIST );
		}

	case YN:
		s = System(&input);
		if (s == IEOF){
			fprintf(stderr, "idcheck: Cannot find package '%s' for Y/N test.\n", input.device);
			exit( EERROR );
		}
		if (s == IERROR)
			exit( EERROR );
		if (s == IFOUND){
			if (sys.conf == 'Y')
				exit( EEXIST );
			else
				exit( ENOCONFLICT );
		}

	case DSP:
		s = System(&input);
		m = master(&input);
		if (s == IERROR || m == IERROR)
			exit( EERROR );

		/* check if DSP exists under /etc/conf/pack.d */
		sprintf(buf, "%s/pack.d/%s", ROOT, input.device);
		p = (stat(buf, &st_stat) == 0);

		/* check if System Module exists under /etc/conf/sdevice.d */
		sprintf(buf, "%s/sdevice.d/%s", ROOT, input.device);
		f = (stat(buf, &st_stat) == 0);

		/* check if Driver.o Module exists under /etc/conf/pack.d */
		sprintf(buf, "%s/pack.d/%s/Driver.o", ROOT, input.device);
		d_o = (stat(buf, &st_stat) == 0);

		rc = p ? 1 : 0;
		rc = (m == IFOUND) ? rc + 2 : rc;
		rc = f ? rc + 4 : rc;
		rc = (s == IFOUND) ? rc + 8 : rc;
		rc = d_o ? rc + 16 : rc;

		/* In case 'rc' not equal to ENOCONFLICT */
		if (rc == 0)
			exit( ENOCONFLICT );
		exit(rc);
	}
}



/* search system file */

System(in)
struct INPUT *in;
{
	register int rc = 0, mrc, r, found = 0;

	if (getinst(SDEV, RESET, &sys) == -1)
		return(IERROR);

	while (1) {

		r = getinst(SDEV, NEXT, &sys);
		if (debug)
			diag(in, SYS, r);
		switch (r) {

		case -1:		/* getinst error */
			return(IERROR);

		case 0:			/* getinst EOF */
			if (found)
				return(rc);
			return(IEOF);

		default:
			switch (in->flag) {

			case DSP:
			if (!(strcmp(in->device, sys.device)))
				return(IFOUND);
			break;

			case YN:
			if (!(strcmp(in->device, sys.device)))
				return(IFOUND);
			break;

			case VECTOR:
			if (sys.conf == 'N')
				break;
			if (in->vector == sys.vector && sys.type > 0)
				return(sys.type);
			break;

			case IOA:
			if (sys.conf == 'N')
				break;
			if (OVERLAP(in->lower, in->upper, sys.sioa, sys.eioa)) {
				found = 1;
				temp.flag = SPY;
				strcpy(temp.device, sys.device);

				/* check if mdevice entry has the 'O' option
				 * that allows overlapping IOA regions */
				mrc = master(&temp);
				if (mrc == IEOF || mrc == IERROR)
					return(IERROR);
				if (mrc == IFOUND)
					rc |= 2;
				else if (mrc == INOTEXIST)
					rc |= 1;
				return(rc);
			}
			break;

			case CMA:
			if (sys.conf == 'N')
				break;
			if (OVERLAP(in->lower, in->upper, sys.scma, sys.ecma))
				return(IFOUND);
			break;
			}	/* end of inner switch */

			break;	/* end default case of outer switch */
		}		/* end of outer switch */
	}			/* end of while */
}



/* search master file */

master(in)
struct INPUT *in;
{
	register int r;

	if (getinst(MDEV, RESET, &mas) == -1)
		return(IERROR);

	while(1) {

		r = getinst(MDEV, NEXT, &mas);

		if (debug)
			diag(in, MAST, r);

		switch (r) {

		case -1:		/* getinst error */
			return(IERROR);

		case 0:			/* getinst EOF */
			return(IEOF);

		default:
			switch (in->flag) {

			case DSP:
			if (!(strcmp(in->device, mas.device)))
				return(IFOUND);
			break;

			case DMA:
			if (in->dma == mas.chan){
				if (getinst(SDEV, mas.device, &sys) != 1){
					fprintf(stderr, "idcheck: Cannot find '%s' sdevice entry for DMA channel test.\n", mas.device);
				exit( EERROR );
				}
				if (sys.conf == 'N')
					break;
				return(IFOUND);
			}
			break;

			case SPY:
			if (!(strcmp(in->device, mas.device)))
				if (strchr(mas.type, IOASPY))
					return(IFOUND);
				else
					return(INOTEXIST);
			break;
			}	/* end of inner switch */

			break;	/* end default case of outer switch */
		}		/* end of outer switch */
	}			/* end of while */
}



/* print error messages */

message(n)
int n;
{
	fprintf(stderr, "idcheck: ");

	switch(n) {
	case IONEOF:
		fprintf(stderr,
			"Only one of '-p', '-v', '-d', '-a', '-c' can be specified\n");
		break;
	case IUSAGE:
		fprintf(stderr, "Usage: idcheck [-r] [-i dir]\n");
		fprintf(stderr, "\t\t\t[-p package]\n");
		fprintf(stderr, "\t\t\t[-y package]\n");
		fprintf(stderr, "\t\t\t[-v vector]\n");
		fprintf(stderr, "\t\t\t[-d dma-channel]\n");
		fprintf(stderr, "\t\t\t[-a -l lower-address -u upper-address]\n");
		fprintf(stderr, "\t\t\t[-c -l lower-address -u upper-address]\n");
		break;
	}

	exit(100);
}



/* print diagnostics */

diag(in, type, rc)
struct INPUT *in;
int type, rc;
{
	switch (type) {
	case SYS:
		if (rc <= 0) {
			fprintf(stderr, "System getinst rc=%d\n", rc);
			return;
		}
		fprintf(stderr, "System getinst: flag=%d rc=%d\n",
			in->flag, rc);
		fprintf(stderr, "\tdevice=%s vec=%hd type=%hd\n",
			sys.device, sys.vector, sys.type);
		fprintf(stderr, "\tsioa=0x%lx eioa=0x%lx scma=0x%lx ecma=0x%lx\n",
			sys.sioa, sys.eioa, sys.scma, sys.ecma);
		break;

	case MAST:
		if (rc <= 0) {
			fprintf(stderr, "Master getinst rc=%d\n", rc);
			return;
		}
		fprintf(stderr, "Master getinst: flag=%d rc=%d\n",
			in->flag, rc);
		fprintf(stderr, "\tdevice=%s chan=%hd type=%s\n",
			mas.device, mas.chan, mas.type);
		break;

	case OPT:
		if (iflag)
			fprintf(stderr,"input=%s\t", pathinst);
		if (input.flag == DSP)
			fprintf(stderr, "package=%s\n", input.device);
		else if (input.flag == VECTOR)
			fprintf(stderr, "vector=%d\n", input.vector);
		else if (input.flag == DMA)
			fprintf(stderr, "dma-channel=%d\n", input.dma);
		else if (input.flag == IOA)
			fprintf(stderr, "IOA: lbound=0x%lx ubound=0x%lx\n",
				input.lower, input.upper);
		else if (input.flag == CMA)
			fprintf(stderr, "CMA: lbound=0x%lx ubound=0x%lx\n",
				input.lower, input.upper);
		break;
	}
}
prdev(dev)
char *dev;
{
	if (verbose)
		fprintf(stdout, "%s\n", dev);
}
