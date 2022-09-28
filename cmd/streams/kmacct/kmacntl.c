/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-streams:kmacct/kmacntl.c	1.1"

#include <sys/types.h>
#include <sys/kmacct.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#define	FALSE	0
#define	TRUE 	1
#define	KMADEV		"/dev/kmacct"	/* Default device name */

static	char	usage[] = "k:nsyz";			
static void	usagerr();

extern	int	errno;
extern	int	getopt(), close(), ioctl();
extern	void	exit(), perror();

main(argc, argv)
	int	argc;
	char	**argv;
{
	int	retval, c, fd;
	char	*kdev	= KMADEV;
	extern	char	*optarg;
	int	type	= 0;
	int	error	= FALSE;

	/*
	 *	Parse and check command line options.
	 */

	while((c = getopt(argc, argv, usage)) != EOF)
		switch(c) {
		case 'k':
			kdev = optarg;
			break;

		case 's':
			if (type == 0)
				type = KMACCT_STATE;
			else 
				error = TRUE;
			break;

		case 'n':
			if (type == 0)
				type = KMACCT_OFF;
			else 
				error = TRUE;
			break;

		case 'y':
			if (type == 0)
				type = KMACCT_ON;
			else 
				error = TRUE;
			break;

		case 'z':
			if (type == 0)
				type = KMACCT_ZERO;
			else 
				error = TRUE;
			break;

		case '?':
		default:
			error = TRUE;
		}

	if (error)
		usagerr(argv[0]);

	if ((fd = open(kdev, O_RDONLY)) == -1) {
		perror("Cannot open device for KMEM Account driver");
		exit(errno);
	}

	if ((retval = ioctl(fd, type, 0)) == -1) {
		perror("Error in KMA Account driver ioctl");
		exit(errno);
	}

	if (type == KMACCT_STATE) 
		(void) printf("KMEM Accounting is %s\n", retval ? "ON" : "OFF");

	(void) close(fd);

	exit(0);
	/*NOTREACHED*/
}

void
usagerr(prog)
char	*prog;
{

	(void) fprintf(stderr,
	    "USAGE: %s [-k kmacct_dev] [-s | -n | -y | -z]\n\n",
	    prog);

	(void) fprintf(stderr,
	    "\t-k kmacct_dev\tkma accounting device (default %s)\n",
	    KMADEV);
	(void) fprintf(stderr,
	    "\t-s\t\treport kma accounting state (ON or OFF)\n");
	(void) fprintf(stderr,
	    "\t-n\t\tturn off kma accounting\n");
	(void) fprintf(stderr,
	    "\t-y\t\tturn on kma accounting\n");
	(void) fprintf(stderr,
	    "\t-z\t\tclear kma accounting data\n");

	exit(1);
}
