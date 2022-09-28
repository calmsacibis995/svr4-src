/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)bnu:bnuconvert.c	1.3.5.1"

#include	"uucp.h"

main(argc, argv)
int argc;
char **argv;
{
	char fdgrade();
	DIR *machdir, *spooldir;
	char machname[MAXFULLNAME];
	char file1[NAMESIZE+1], file2[NAMESIZE+1];
	struct cs_struct svdcfile;
	int c;

	(void) strcpy(Progname, "bnuconvert");

	Uid = getuid();
	Euid = geteuid();
	if (Uid == 0)
		(void) setuid(UUCPUID);

	while ((c = getopt(argc, argv, "x:")) != EOF)
		switch(c) {
		case 'x':
			Debug = atoi(optarg);
			if (Debug < 0)
				Debug = 1;
			break;
		default:
			(void) fprintf(stderr, "usage: bnuconvert [-xLEVEL]\n");
			exit(-1);
		}

	DEBUG(5, "Progname (%s): STARTED\n", Progname);

	/* find the default directory to queue to */

	if (eaccess(GRADES, 04) != -1) 
		svdcfile.grade = fdgrade();
	else 
		svdcfile.grade = D_QUEUE;

	DEBUG(5, "All jobs will be placed in directory (%c) ", svdcfile.grade);
	DEBUG(5, "under each remote name in the spool area.%c\n", NULLCHAR);

	if ((spooldir = opendir(SPOOL)) == NULL) {
		(void) fprintf(stderr, "CAN'T OPEN (%s): errno (%d)\n",
			SPOOL, errno);
		exit(1);
	}

	while (gdirf(spooldir, file1, SPOOL)) {

		(void) sprintf(Rmtname, "%s", file1);
		(void) sprintf(machname, "%s/%s", SPOOL, file1);
		DEBUG(9, "File1 is (%s)\n", file1);
		DEBUG(9, "Rmtname is (%s)\n", Rmtname);
		DEBUG(9, "Machname is (%s)\n", machname);

		if (chdir(machname) != 0) {
			(void) fprintf(stderr, "CAN'T CHDIR (%s): errno (%d)\n",
				machname, errno);
			exit(1);
		}

		if ((machdir = opendir(machname)) == NULL) {
			(void) fprintf(stderr, "CAN'T OPEN (%s): errno (%d)\n",
				machname, errno);
				continue;
		}

		DEBUG(7, "Directory: %s\n", machname);

		while (gnamef(machdir, file2) == TRUE) {

			DEBUG(9, "File read from (%s) ", machname);
			DEBUG(9, "is (%s)\n", file2);

			if (!EQUALSN(file2, "C.",2))
				continue;

			/* build a saved C. file structure */

			(void) strncpy(svdcfile.file, file2, NAMESIZE-1);
			(void) sprintf(svdcfile.sys, "%s/%c", Rmtname, svdcfile.grade);

			DEBUG(9, "Rmtname is (%s)\n", Rmtname);
			DEBUG(9, "Default directory to queue to is (%c)\n", svdcfile.grade);
			DEBUG(7, "Directory to queue to is (%s)\n", svdcfile.sys);

			/* place any and all D. files related to the
			** C. file in the proper spool area.
			*/

			putdfiles(svdcfile);

			/* Now queue the C. file */

			wfcommit(svdcfile.file, svdcfile.file, svdcfile.sys);
		}
		closedir(machdir);
	}
	closedir(spooldir);
	exit(0);
}
/* a dummy cleanup function to satisfy a .o file */
void cleanup() {}
