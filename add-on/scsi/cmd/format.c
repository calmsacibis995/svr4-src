/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)scsi:cmd/format.c	1.3"

#include 	"stdio.h"
#include	"utmp.h"

#ifndef i386
#include	"sys/pdi.h"
#include	"sys/extbus.h"
#endif /* ix86 */

#include	"sys/stat.h"
#include	"scl.h"
#include	"tokens.h"
#include	"sys/vtoc.h"		/* Included just to satisfy scsicomm.h */
#include	"scsicomm.h"

#define NORMEXIT	0
#define USAGE		2
#define TRUE		1
#define FALSE		0
#if u3b2
#define SCSI_DIR	"/usr/lib/scsi/"
#define SCSI_DIR2 	"/usr/lib/scsi/format.d/"
#define	SUBUTILDIR	"/usr/lib/scsi/format.d/"
#elif i386
#define SCSI_DIR	"/etc/scsi/"
#define SCSI_DIR2 	"/etc/scsi/format.d/"
#define	SUBUTILDIR	"/etc/scsi/format.d/"
#else
#define SCSI_DIR	"/etc/scsi.d/"
#define SCSI_DIR2	"/etc/scsi.d/format.d/"
#define	SUBUTILDIR	"/etc/scsi.d/format.d/"
#endif /* u3b2 || ix86 */
#define	HOSTFILE	"HAXXXXXX"
#define	INDEXFILE	"/etc/scsi/tc.index"

#define	BUFSIZE		512

extern FILE	*scriptfile_open();
extern int	errno;
void		error();
void		usage();

char	Devfile[128];	/* Device File name */
int	Show = FALSE;

main(argc, argv)
int 	argc;
char 	*argv[];
{
	char		subutil[128];	/* Sub-utility file name */
	char		subutilfile[256];
	char		*scriptfile;	/* Script file name */
	FILE		*scriptfp;	/* Script file file pointer */
	struct stat	buf;
	int		argc1;		/* To copy subutility name with "-h" option */
	char		*argv1[64];	/* To copy subutility name with "-h" option: copy argument list to local array "argv1[]" */
	char		hopt[3];	/* To copy subutility name with "-h" option: array to hold "-h\NULL" */
	char		*name, *strrchr();	/* To strip pathname preceding Cmdname */

	/* Strip pathname preceding Cmdname */
	if ((name = strrchr(argv[0], '/')) == 0)
		name = argv[0];
	else
		name++;

	strcpy(Cmdname, name);

       /* Don't check arguments so future subutilities can now have options */


	/* The last argument must be the device file */
	if (argc <= 1)
		/* Not enough arguments */
		usage();

	(void) strcpy(Devfile, argv[argc-1]);
	/* Check for superuser priviledge */ 
	if (geteuid() != 0)
	    error("Not super user\n");

	/* Clear Hostfile variable */
	Hostfile[0] = '\0';

	/* Open the SCSI special device files */
	if (scsi_open(Devfile, HOSTFILE))
		error("SCSI special device file open failed\n");

	/* Open the script file */
	if ((scriptfp = scriptfile_open(INDEXFILE)) == NULL)
		/* Script file cannot be opened so the device cannot be formatted */
		error("Script file open failed\n");

	/* Get the subutility name from the scriptfile */
	if (get_string(scriptfp,subutil) < 0)
		/* no subutility string on first line of script file */
		error("Could not find sub-utility name in script file.\n");

	/* Check to see if the subutility exists in the current directory */
	(void) strcpy(subutilfile, subutil);
	if (stat(subutilfile, &buf) < 0) {
		/* The subutility file doesn't exist in the current
		 * directory, so next check the subutility directory.
		 */
		errno = 0;
		(void) strcpy(subutilfile, SUBUTILDIR);
		(void) strcat(subutilfile, subutil);
		if (stat(subutilfile, &buf) < 0) {
			   /* could not find subutilfile */
			   error("Could not find sub-utility.\n");
		}
	} 
	/* Close Host Adapter special device file */
	close(Hostfdes);

	/* Unlink the Host Adapter special device file */
	unlink(Hostfile);

	/* Exec the subutility with the same options */
	argv[0] = subutilfile;

	/* Add a "-h" flag to the list of command line arguments if "scsihdefix" */
	if (strcmp(Cmdname, "scsihdefix") == 0) {

		argv1[0] = subutilfile;
		argv1[1] = "-h";

		for ( argc1 = 1; argc1 < argc; argc1++ ) {
			argv1[argc1 +1] = argv[argc1];
		}

		++argc1;
		argv1[argc1] = NULL;

		execv(argv1[0], argv1);
	} else
		execv(argv[0], argv);

	error("%s exec failed\n", argv[0]);

  /* NOTREACHED */
}


void
usage()
{
	if (strcmp(Cmdname, "scsiformat") == 0)
		(void) fprintf(stderr, "Usage: %s [ -[n] v [u] | -[n] V [u] ] /dev/rdsk/c?t?d?s0\n", Cmdname);
	else
		(void) fprintf(stderr, "Usage: %s [ -p ] [ -b 0x(blockno) ] [ -r | -s file ] /dev/rdsk/c?t?d?s0\n", Cmdname);

	exit(USAGE);
}	/* usage() */
