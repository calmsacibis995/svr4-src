/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)priocntl:tspriocntl.c	1.6.6.1"
#include	<stdio.h>
#include	<unistd.h>
#include	<sys/types.h>
#include	<sys/procset.h>
#include	<sys/priocntl.h>
#include	<sys/tspriocntl.h>
#include	<errno.h>

#include	"priocntl.h"

/*
 * This file contains the class specific code implementing
 * the time-sharing priocntl sub-command.
 */

#define BASENMSZ	16

static void	print_tsinfo(), print_tsprocs(), set_tsprocs(), exec_tscmd();

static char usage[] =
"usage:	priocntl -l\n\
	priocntl -d [-d idtype] [idlist]\n\
	priocntl -s [-c TS] [-m tsuprilim] [-p tsupri] [-i idtype] [idlist]\n\
	priocntl -e [-c TS] [-m tsuprilim] [-p tsupri] command [argument(s)]\n";

static char	cmdpath[256];
static char	basenm[BASENMSZ];


main(argc, argv)
int	argc;
char	**argv;
{
	extern char	*optarg;
	extern int	optind;

	int		c;
	int		lflag, dflag, sflag, mflag, pflag, eflag, iflag;
	short		tsuprilim;
	short		tsupri;
	char		*idtypnm;
	idtype_t	idtype;
	int		idargc;

	strcpy(cmdpath, argv[0]);
	strcpy(basenm, basename(argv[0]));
	lflag = dflag = sflag = mflag = pflag = eflag = iflag = 0;
	while ((c = getopt(argc, argv, "ldsm:p:ec:i:")) != -1) {
		switch(c) {

		case 'l':
			lflag++;
			break;

		case 'd':
			dflag++;
			break;

		case 's':
			sflag++;
			break;

		case 'm':
			mflag++;
			tsuprilim = (short)atoi(optarg);
			break;

		case 'p':
			pflag++;
			tsupri = (short)atoi(optarg);
			break;

		case 'e':
			eflag++;
			break;

		case 'c':
			if (strcmp(optarg, "TS") != 0)
				fatalerr("error: %s executed for %s class, \
%s is actually sub-command for TS class\n", cmdpath, optarg, cmdpath);
			break;

		case 'i':
			iflag++;
			idtypnm = optarg;
			break;

		case '?':
			fatalerr(usage);

		default:
			break;
		}
	}

	if (lflag) {
		if (dflag || sflag || mflag || pflag || eflag || iflag)
			fatalerr(usage);

		print_tsinfo();
		exit(0);

	} else if (dflag) {
		if (lflag || sflag || mflag || pflag || eflag)
			fatalerr(usage);

		print_tsprocs();
		exit(0);

	} else if (sflag) {
		if (lflag || dflag || eflag)
			fatalerr(usage);
	
		if (iflag) {
			if (str2idtyp(idtypnm, &idtype) == -1)
				fatalerr("%s: Bad idtype %s\n", basenm,
				    idtypnm);
		} else
			idtype = P_PID;

		if (mflag == 0)
			tsuprilim = TS_NOCHANGE;

		if (pflag == 0)
			tsupri = TS_NOCHANGE;

		if (optind < argc)
			idargc = argc - optind;
		else
			idargc = 0;

		set_tsprocs(idtype, idargc, &argv[optind], tsuprilim, tsupri);
		exit(0);

	} else if (eflag) {
		if (lflag || dflag || sflag || iflag)
			fatalerr(usage);

		if (mflag == 0)
			tsuprilim = TS_NOCHANGE;

		if (pflag == 0)
			tsupri = TS_NOCHANGE;

		exec_tscmd(&argv[optind], tsuprilim, tsupri);

	} else {
		fatalerr(usage);
	}
}


/*
 * Print our class name and the configured user priority range.
 */
static void
print_tsinfo()
{
	pcinfo_t	pcinfo;

	strcpy(pcinfo.pc_clname, "TS");

	printf("TS (Time Sharing)\n");

	if (priocntl(0, 0, PC_GETCID, &pcinfo) == -1)
		fatalerr("\tCan't get configured TS user priority range\n");

	printf("\tConfigured TS User Priority Range: -%d through %d\n",
	    ((tsinfo_t *)pcinfo.pc_clinfo)->ts_maxupri,
	    ((tsinfo_t *)pcinfo.pc_clinfo)->ts_maxupri);
}


/*
 * Read a list of pids from stdin and print the user priority and user
 * priority limit for each of the corresponding processes.
 */
static void
print_tsprocs()
{
	pid_t		pidlist[NPIDS];
	int		numread;
	int		i;
	id_t		tscid;
	pcinfo_t	pcinfo;
	pcparms_t	pcparms;

	numread = fread(pidlist, sizeof(pid_t), NPIDS, stdin);

	printf("TIME SHARING PROCESSES:\n    PID    TSUPRILIM    TSUPRI\n");

	strcpy(pcinfo.pc_clname, "TS");

	if (priocntl(0, 0, PC_GETCID, &pcinfo) == -1)
		fatalerr("%s: Can't get TS class ID\n", basenm);

	tscid = pcinfo.pc_cid;

	if (numread <= 0)
		fatalerr("%s: No pids on input\n", basenm);


	pcparms.pc_cid = PC_CLNULL;
	for (i = 0; i < numread; i++) {
		printf("%7ld", pidlist[i]);
		if (priocntl(P_PID, pidlist[i], PC_GETPARMS, &pcparms) == -1) {
			printf("\tCan't get TS user priority\n");
			continue;
		}

		if (pcparms.pc_cid == tscid) {
			printf("    %5d       %5d\n",
			    ((tsparms_t *)pcparms.pc_clparms)->ts_uprilim,
			    ((tsparms_t *)pcparms.pc_clparms)->ts_upri);
		} else {

			/*
			 * Process from some class other than time sharing.
			 * It has probably changed class while priocntl
			 * command was executing (otherwise we wouldn't
			 * have been passed its pid).  Print the little
			 * we know about it.
			 */
			pcinfo.pc_cid = pcparms.pc_cid;
			if (priocntl(0, 0, PC_GETCLINFO, &pcinfo) != -1)
				printf("%ld\tChanged to class %s while priocntl \
command executing\n", pidlist[i], pcinfo.pc_clname);

		}
	}
}


/*
 * Set all processes in the set specified by idtype/idargv to time-sharing
 * (if they aren't already time-sharing) and set their user priority limit
 * and user priority to those specified by tsuprilim and tsupri.
 */
static void
set_tsprocs(idtype, idargc, idargv, tsuprilim, tsupri)
idtype_t	idtype;
int		idargc;
char		**idargv;
short		tsuprilim;
short		tsupri;
{
	pcinfo_t	pcinfo;
	pcparms_t	pcparms;
	short		maxupri;
	char		idtypnm[12];
	int		i;
	id_t		tscid;

	/*
	 * If both user priority and limit have been defaulted then they
	 * need to be changed to 0 for later priocntl system calls.
	 */
	if (tsuprilim == TS_NOCHANGE && tsupri == TS_NOCHANGE)
		tsuprilim = tsupri = 0;

	/*
	 * Get the time sharing class ID and max configured user priority.
	 */
	strcpy(pcinfo.pc_clname, "TS");
	if (priocntl(0, 0, PC_GETCID, &pcinfo) == -1)
		fatalerr("%s: Can't get TS class ID, priocntl system call \
failed with errno %d\n", basenm, errno);
	maxupri = ((tsinfo_t *)pcinfo.pc_clinfo)->ts_maxupri;

	/*
	 * Validate the tsuprilim and tsupri arguments.
	 */
	if ((tsuprilim > maxupri || tsuprilim < -maxupri) &&
	    tsuprilim != TS_NOCHANGE)
		fatalerr("%s: Specified user priority limit %d out of \
configured range\n", basenm, tsuprilim);

	if ((tsupri > maxupri || tsupri < -maxupri) &&
	    tsupri != TS_NOCHANGE)
		fatalerr("%s: Specified user priority %d out of \
configured range\n", basenm, tsupri);

	pcparms.pc_cid = pcinfo.pc_cid;
	((tsparms_t *)pcparms.pc_clparms)->ts_uprilim = tsuprilim;
	((tsparms_t *)pcparms.pc_clparms)->ts_upri = tsupri;

	if (idtype == P_ALL) {
		if (priocntl(P_ALL, 0, PC_SETPARMS, &pcparms) == -1) {
			if (errno == EPERM)
				fprintf(stderr, "Permissions error \
encountered on one or more processes.\n");
			else
				fatalerr("%s: Can't reset time sharing \
parameters\npriocntl system call failed with errno %d\n", basenm, errno);
		}
	} else if (idargc == 0) {
		if (priocntl(idtype, P_MYID, PC_SETPARMS, &pcparms) == -1) {
			if (errno == EPERM) {
				(void)idtyp2str(idtype, idtypnm);
				fprintf(stderr, "Permissions error \
encountered on current %s.\n", idtypnm);
			} else {
				fatalerr("%s: Can't reset time sharing \
parameters\npriocntl system call failed with errno %d\n", basenm, errno);
			}
		}
	} else {
		(void)idtyp2str(idtype, idtypnm);
		for (i = 0; i < idargc; i++) {
			if ( idtype == P_CID ) {
				tscid = clname2cid(idargv[i]);
				if (priocntl(idtype, tscid, PC_SETPARMS, &pcparms) == -1) {
					if (errno == EPERM)
						fprintf(stderr, "Permissions error \
encountered on %s %s.\n", idtypnm, idargv[i]);
					else
						fatalerr("%s: Can't reset \
time sharing parameters\npriocntl system call failed with errno %d\n", basenm, errno);
				}
			} else if (priocntl(idtype, (id_t)atol(idargv[i]),
			    PC_SETPARMS, &pcparms) == -1) {
				if (errno == EPERM)
					fprintf(stderr, "Permissions error \
encountered on %s %s.\n", idtypnm, idargv[i]);
				else
					fatalerr("%s: Can't reset time sharing \
parameters\npriocntl system call failed with errno %d\n", basenm, errno);
			}
		}
	}
		
}


/*
 * Execute the command pointed to by cmdargv as a time-sharing process
 * with the user priority limit given by tsuprilim and user priority tsupri.
 */
static void
exec_tscmd(cmdargv, tsuprilim, tsupri)
char	**cmdargv;
short	tsuprilim;
short	tsupri;
{
	pcinfo_t	pcinfo;
	pcparms_t	pcparms;
	short		maxupri;

	/*
	 * If both user priority and limit have been defaulted then they
	 * need to be changed to 0 for later priocntl system calls.
	 */
	if (tsuprilim == TS_NOCHANGE && tsupri == TS_NOCHANGE)
		tsuprilim = tsupri = 0;

	/*
	 * Get the time sharing class ID and max configured user priority.
	 */
	strcpy(pcinfo.pc_clname, "TS");
	if (priocntl(0, 0, PC_GETCID, &pcinfo) == -1)
		fatalerr("%s: Can't get TS class ID, priocntl system call \
failed with errno %d\n", basenm, errno);
	maxupri = ((tsinfo_t *)pcinfo.pc_clinfo)->ts_maxupri;

	if ((tsuprilim > maxupri || tsuprilim < -maxupri) &&
	    tsuprilim != TS_NOCHANGE)
		fatalerr("%s: Specified user priority limit %d out of \
configured range\n", basenm, tsuprilim);

	if ((tsupri > maxupri || tsupri < -maxupri) &&
	    tsupri != TS_NOCHANGE)
		fatalerr("%s: Specified user priority %d out of \
configured range\n", basenm, tsupri);

	pcparms.pc_cid = pcinfo.pc_cid;
	((tsparms_t *)pcparms.pc_clparms)->ts_uprilim = tsuprilim;
	((tsparms_t *)pcparms.pc_clparms)->ts_upri = tsupri;
	if (priocntl(P_PID, P_MYID, PC_SETPARMS, &pcparms) == -1)
		fatalerr("%s: Can't reset time sharing parameters\n\
priocntl system call failed with errno %d\n", basenm, errno);

	(void)execvp(cmdargv[0], cmdargv);
	fatalerr("%s: Can't execute %s, exec failed with errno %d\n",
	    basenm, cmdargv[0], errno);
}
