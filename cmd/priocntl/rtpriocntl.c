/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)priocntl:rtpriocntl.c	1.7.5.2"
#include	<stdio.h>
#include	<string.h>
#include	<unistd.h>
#include	<sys/types.h>
#include	<sys/procset.h>
#include	<sys/priocntl.h>
#include	<sys/rtpriocntl.h>
#include	<sys/param.h>
#include	<sys/evecb.h>
#include	<sys/hrtcntl.h>
#include	<limits.h>
#include	<errno.h>

#include	"priocntl.h"

/*
 * This file contains the class specific code implementing
 * the real-time priocntl sub-command.
 */

#define	BASENMSZ	16

static void	print_rtinfo(), print_rtprocs(), set_rtprocs(), exec_rtcmd();

static char usage[] =
"usage:	priocntl -l\n\
	priocntl -d [-i idtype] [idlist]\n\
	priocntl -s [-c RT] [-p rtpri] [-t tqntm [-r res]] [-i idtype] [idlist]\n\
	priocntl -e [-c RT] [-p rtpri] [-t tqntm [-r res]] command [argument(s)]\n";

static char	cmdpath[256];
static char	basenm[BASENMSZ];


main(argc, argv)
int	argc;
char	**argv;
{
	extern char	*optarg;
	extern int	optind;

	int		c;
	int		lflag, dflag, sflag, pflag, tflag, rflag, eflag, iflag;
	short		rtpri;
	long		tqntm;
	long		res;
	char		*idtypnm;
	idtype_t	idtype;
	int		idargc;

	strcpy(cmdpath, argv[0]);
	strcpy(basenm, basename(argv[0]));
	lflag = dflag = sflag = pflag = tflag = rflag = eflag = iflag = 0;
	while ((c = getopt(argc, argv, "ldsp:t:r:ec:i:")) != -1) {
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

		case 'p':
			pflag++;
			rtpri = (short)atoi(optarg);
			break;

		case 't':
			tflag++;
			tqntm = atol(optarg);
			break;

		case 'r':
			rflag++;
			res = atol(optarg);
			break;

		case 'e':
			eflag++;
			break;

		case 'c':
			if (strcmp(optarg, "RT") != 0)
				fatalerr("error: %s executed for %s class, \
%s is actually sub-command for RT class\n", cmdpath, optarg, cmdpath);
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
		if (dflag || sflag || pflag || tflag || rflag || eflag || iflag)
			fatalerr(usage);

		print_rtinfo();
		exit(0);

	} else if (dflag) {
		if (lflag || sflag || pflag || tflag || rflag || eflag)
			fatalerr(usage);

		print_rtprocs();
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

		if (pflag == 0)
			rtpri = RT_NOCHANGE;

		if (tflag == 0)
			tqntm = RT_NOCHANGE;
		else if (tqntm < 1)
			fatalerr("%s: Invalid time quantum specified; time \
quantum must be positive\n", basenm);

		if (rflag == 0)
			res = 1000;

		if (optind < argc)
			idargc = argc - optind;
		else
			idargc = 0;

		set_rtprocs(idtype, idargc, &argv[optind], rtpri, tqntm, res);
		exit(0);

	} else if (eflag) {
		if (lflag || dflag || sflag || iflag)
			fatalerr(usage);

		if (pflag == 0)
			rtpri = RT_NOCHANGE;

		if (tflag == 0)
			tqntm = RT_NOCHANGE;
		else if (tqntm < 1)
			fatalerr("%s: Invalid time quantum specified; time \
quantum must be positive\n", basenm);

		if (rflag == 0)
			res = 1000;

		exec_rtcmd(&argv[optind], rtpri, tqntm, res);

	} else {
		fatalerr(usage);
	}
}


/*
 * Print our class name and the maximum configured real-time priority.
 */
static void
print_rtinfo()
{
	pcinfo_t	pcinfo;

	strcpy(pcinfo.pc_clname, "RT");

	printf("RT (Real Time)\n");

	if (priocntl(0, 0, PC_GETCID, &pcinfo) == -1)
		fatalerr("\tCan't get maximum configured RT priority\n");

	printf("\tMaximum Configured RT Priority: %d\n",
	    ((rtinfo_t *)pcinfo.pc_clinfo)->rt_maxpri);
}


/*
 * Read a list of pids from stdin and print the real-time priority and time
 * quantum (in millisecond resolution) for each of the corresponding processes.
 */
static void
print_rtprocs()
{
	pid_t		pidlist[NPIDS];
	int		numread;
	int		i;
	id_t		rtcid;
	pcinfo_t	pcinfo;
	pcparms_t	pcparms;
	ulong		tqsecs;
	long		tqnsecs;

	numread = fread(pidlist, sizeof(pid_t), NPIDS, stdin);

	printf("REAL TIME PROCESSES:\n    PID    RTPRI       TQNTM\n");

	strcpy(pcinfo.pc_clname, "RT");

	if (priocntl(0, 0, PC_GETCID, &pcinfo) == -1)
		fatalerr("%s: Can't get RT class ID\n", basenm);
	rtcid = pcinfo.pc_cid;

	if (numread <= 0)
		fatalerr("%s: No pids on input\n", basenm);

	pcparms.pc_cid = PC_CLNULL;
	for (i = 0; i < numread; i++) {
		printf("%7ld", pidlist[i]);
		if (priocntl(P_PID, pidlist[i], PC_GETPARMS, &pcparms) == -1) {
			printf("\tCan't get real time parameters\n");
			continue;
		}

		if (pcparms.pc_cid == rtcid) {
			printf("   %5d",
			    ((rtparms_t *)pcparms.pc_clparms)->rt_pri);
			tqsecs = ((rtparms_t *)pcparms.pc_clparms)->rt_tqsecs;
			tqnsecs = ((rtparms_t *)pcparms.pc_clparms)->rt_tqnsecs;
			if (tqsecs > LONG_MAX / 1000 - 1)
				printf("    Time quantum too large to express in \
millisecond resolution.\n");
			else
				printf(" %11ld\n", tqsecs * 1000 + tqnsecs / 1000000);
		} else {

			/*
			 * Process from some class other than real time.
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
 * Set all processes in the set specified by idtype/idargv to real time
 * (if they aren't already real time) and set their real-time priority
 * and quantum to those specified by rtpri and tqntm/res.
 */
static void
set_rtprocs(idtype, idargc, idargv, rtpri, tqntm, res)
idtype_t	idtype;
int		idargc;
char		**idargv;
short		rtpri;
long		tqntm;
long		res;
{
	pcinfo_t	pcinfo;
	pcparms_t	pcparms;
	short		maxrtpri;
	hrtime_t	hrtime;
	char		idtypnm[12];
	int		i;
	id_t		rtcid;


	/*
	 * Get the real time class ID and max configured RT priority.
	 */
	strcpy(pcinfo.pc_clname, "RT");
	if (priocntl(0, 0, PC_GETCID, &pcinfo) == -1)
		fatalerr("%s: Can't get RT class ID, priocntl system call \
failed with errno %d\n", basenm, errno);
	maxrtpri = ((rtinfo_t *)pcinfo.pc_clinfo)->rt_maxpri;

	/*
	 * Validate the rtpri and res arguments.
	 */
	if ((rtpri > maxrtpri || rtpri < 0) &&
	    rtpri != RT_NOCHANGE)
		fatalerr("%s: Specified real time priority %d out of \
configured range\n", basenm, rtpri);

	if (res > 1000000000 || res < 1)
		fatalerr("%s: Invalid resolution specified; resolution must be \
between 1 and 1,000,000,000\n, basenm");

	pcparms.pc_cid = pcinfo.pc_cid;
	((rtparms_t *)pcparms.pc_clparms)->rt_pri = rtpri;
	if (tqntm == RT_NOCHANGE) {
		((rtparms_t *)pcparms.pc_clparms)->rt_tqnsecs = RT_NOCHANGE;
	} else {
		hrtime.hrt_secs = 0;
		hrtime.hrt_rem = tqntm;
		hrtime.hrt_res = res;
		if (_hrtnewres(&hrtime, NANOSEC, HRT_RNDUP) == -1)
			fatalerr("%s: Can't convert resolution.\n", basenm);
		((rtparms_t *)pcparms.pc_clparms)->rt_tqsecs = hrtime.hrt_secs;
		((rtparms_t *)pcparms.pc_clparms)->rt_tqnsecs = hrtime.hrt_rem;
	}

	if (idtype == P_ALL) {
		if (priocntl(P_ALL, 0, PC_SETPARMS, &pcparms) == -1) {
			if (errno == EPERM)
				fprintf(stderr, "Permissions error \
encountered on one or more processes.\n");
			else
				fatalerr("%s: Can't reset real time parameters\n\
priocntl system call failed with errno %d\n", basenm, errno);
		}
	} else if (idargc == 0) {
		if (priocntl(idtype, P_MYID, PC_SETPARMS, &pcparms) == -1) {
			if (errno == EPERM) {
				(void)idtyp2str(idtype, idtypnm);
				fprintf(stderr, "Permissions error \
encountered on current %s.\n", idtypnm);
			} else {
				fatalerr("%s: Can't reset real time parameters\n\
priocntl system call failed with errno %d\n", basenm, errno);
			}
		}
	} else {
		(void)idtyp2str(idtype, idtypnm);
		for (i = 0; i < idargc; i++) {
			if ( idtype == P_CID ) {
				rtcid = clname2cid(idargv[i]);
				if (priocntl(idtype, rtcid, PC_SETPARMS, &pcparms) == -1) {
					if (errno == EPERM)
						fprintf(stderr, "Permissions error \
encountered on %s %s.\n", idtypnm, idargv[i]);
					else
						fatalerr("%s: Can't reset \
real time parameters\npriocntl system call failed with errno %d\n", basenm, errno);
				}
			} else if (priocntl(idtype, (id_t)atol(idargv[i]),
			    PC_SETPARMS, &pcparms) == -1) {
				if (errno == EPERM)
					fprintf(stderr, "Permissions error \
encountered on %s %s.\n", idtypnm, idargv[i]);
				else
					fatalerr("%s: Can't reset real time \
parameters\npriocntl system call failed with errno %d\n", basenm, errno);
			}
		}
	}
		
}


/*
 * Execute the command pointed to by cmdargv as a real-time process
 * with real time priority rtpri and quantum tqntm/res.
 */
static void
exec_rtcmd(cmdargv, rtpri, tqntm, res)
char	**cmdargv;
short	rtpri;
long	tqntm;
long	res;
{
	pcinfo_t	pcinfo;
	pcparms_t	pcparms;
	short		maxrtpri;
	hrtime_t	hrtime;

	/*
	 * Get the real time class ID and max configured RT priority.
	 */
	strcpy(pcinfo.pc_clname, "RT");
	if (priocntl(0, 0, PC_GETCID, &pcinfo) == -1)
		fatalerr("%s: Can't get RT class ID, priocntl system call \
failed with errno %d\n", basenm, errno);
	maxrtpri = ((rtinfo_t *)pcinfo.pc_clinfo)->rt_maxpri;

	if ((rtpri > maxrtpri || rtpri < 0) &&
	    rtpri != RT_NOCHANGE)
		fatalerr("%s: Specified real time priority %d out of \
configured range\n", basenm, rtpri);

	if (res > 1000000000 || res < 1)
		fatalerr("%s: Invalid resolution specified; resolution must be \
between 1 and 1,000,000,000\n", basenm);

	pcparms.pc_cid = pcinfo.pc_cid;
	((rtparms_t *)pcparms.pc_clparms)->rt_pri = rtpri;
	if (tqntm == RT_NOCHANGE) {
		((rtparms_t *)pcparms.pc_clparms)->rt_tqnsecs = RT_NOCHANGE;
	} else {
		hrtime.hrt_secs = 0;
		hrtime.hrt_rem = tqntm;
		hrtime.hrt_res = res;
		if (_hrtnewres(&hrtime, NANOSEC, HRT_RNDUP) == -1)
			fatalerr("%s: Can't convert resolution.\n", basenm);
		((rtparms_t *)pcparms.pc_clparms)->rt_tqsecs = hrtime.hrt_secs;
		((rtparms_t *)pcparms.pc_clparms)->rt_tqnsecs = hrtime.hrt_rem;
	}

	if (priocntl(P_PID, P_MYID, PC_SETPARMS, &pcparms) == -1) {
		fatalerr("%s: Can't reset real time parameters\n\
priocntl system call failed with errno %d\n", basenm, errno);
	}
	(void)execvp(cmdargv[0], cmdargv);
	fatalerr("%s: Can't execute %s, exec failed with errno %d\n",
	    basenm, cmdargv[0], errno);
}
