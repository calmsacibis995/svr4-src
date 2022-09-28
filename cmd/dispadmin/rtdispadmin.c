/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)dispadmin:rtdispadmin.c	1.5.1.1"
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>
#include	<sys/types.h>
#include	<sys/priocntl.h>
#include	<sys/rtpriocntl.h>
#include	<sys/param.h>
#include	<sys/evecb.h>
#include	<sys/hrtcntl.h>
#include	<sys/rt.h>

/*
 * This file contains the class specific code implementing
 * the real-time dispadmin sub-command.
 */

#define BASENMSZ	16

extern int	errno;
extern char	*basename();
extern void	fatalerr();
extern long	hrtconvert();

static void	get_rtdptbl(), set_rtdptbl();

static char usage[] =
"usage:	dispadmin -l\n\
	dispadmin -c RT -g [-r res]\n\
	dispadmin -c RT -s infile\n";

static char	basenm[BASENMSZ];
static char	cmdpath[256];


main(argc, argv)
int	argc;
char	**argv;
{
	extern char	*optarg;

	int		c;
	int		lflag, gflag, rflag, sflag;
	ulong		res;
	char		*infile;

	strcpy(cmdpath, argv[0]);
	strcpy(basenm, basename(argv[0]));
	lflag = gflag = rflag = sflag = 0;
	while ((c = getopt(argc, argv, "lc:gr:s:")) != -1) {
		switch (c) {

		case 'l':
			lflag++;
			break;

		case 'c':
			if (strcmp(optarg, "RT") != 0)
				fatalerr("error: %s executed for %s class, \
%s is actually sub-command for RT class\n", cmdpath, optarg, cmdpath);
			break;

		case 'g':
			gflag++;
			break;

		case 'r':
			rflag++;
			res = strtoul(optarg, (char **)NULL, 10);
			break;

		case 's':
			sflag++;
			infile = optarg;
			break;

		case '?':
			fatalerr(usage);

		default:
			break;
		}
	}

	if (lflag) {
		if (gflag || rflag || sflag)
			fatalerr(usage);

		printf("RT\t(Real Time)\n");
		exit(0);

	} else if (gflag) {
		if (lflag || sflag)
			fatalerr(usage);

		if (rflag == 0)
			res = 1000;

		get_rtdptbl(res);
		exit(0);

	} else if (sflag) {
		if (lflag || gflag || rflag)
			fatalerr(usage);

		set_rtdptbl(infile);
		exit(0);

	} else {
		fatalerr(usage);
	}
}


/*
 * Retrieve the current rt_dptbl from memory, convert the time quantum
 * values to the resolution specified by res and write the table to stdout.
 */
static void
get_rtdptbl(res)
ulong	res;
{
	int		i;
	int		rtdpsz;
	pcinfo_t	pcinfo;
	pcadmin_t	pcadmin;
	rtadmin_t	rtadmin;
	rtdpent_t	*rt_dptbl;
	hrtime_t	hrtime;

	strcpy(pcinfo.pc_clname, "RT");
	if (priocntl(0, 0, PC_GETCID, &pcinfo) == -1)
		fatalerr("%s: Can't get RT class ID, priocntl system \
call failed with errno %d\n", basenm, errno);

	pcadmin.pc_cid = pcinfo.pc_cid;
	pcadmin.pc_cladmin = (char *)&rtadmin;
	rtadmin.rt_cmd = RT_GETDPSIZE;

	if (priocntl(0, 0, PC_ADMIN, &pcadmin) == -1)
		fatalerr("%s: Can't get rt_dptbl size, priocntl system \
call failed with errno %d\n", basenm, errno);

	rtdpsz = rtadmin.rt_ndpents * sizeof(rtdpent_t);
	if ((rt_dptbl = (rtdpent_t *)malloc(rtdpsz)) == NULL)
		fatalerr("%s: Can't allocate memory for rt_dptbl\n", basenm);

	rtadmin.rt_dpents = rt_dptbl;

	rtadmin.rt_cmd = RT_GETDPTBL;
	if (priocntl(0, 0, PC_ADMIN, &pcadmin) == -1)
		fatalerr("%s: Can't get rt_dptbl, priocntl system call \
failed with errno %d\n", basenm, errno);

	printf("# Real Time Dispatcher Configuration\n");
	printf("RES=%ld\n\n", res);
	printf("# TIME QUANTUM                    PRIORITY\n");
	printf("# (rt_quantum)                      LEVEL\n");

	for (i = 0; i < rtadmin.rt_ndpents; i++) {
		if (res != HZ) {
			hrtime.hrt_secs = 0;
			hrtime.hrt_rem = rt_dptbl[i].rt_quantum;
			hrtime.hrt_res = HZ;
			if (_hrtnewres(&hrtime, res, HRT_RNDUP) == -1)
				fatalerr("%s: Can't convert to requested \
resolution\n", basenm);
			if((rt_dptbl[i].rt_quantum = hrtconvert(&hrtime)) == -1)
				fatalerr("%s: Can't express time quantum in \
requested resolution,\ntry coarser resolution\n", basenm);
		}
		printf("%10ld                    #      %3d\n",
		    rt_dptbl[i].rt_quantum, i);
	}
}


/*
 * Read the rt_dptbl values from infile, convert the time quantum values
 * to HZ resolution, do a little sanity checking and overwrite the table
 * in memory with the values from the file.
 */
static void
set_rtdptbl(infile)
char	*infile;
{
	int		i;
	int		nrtdpents;
	char		*tokp;
	pcinfo_t	pcinfo;
	pcadmin_t	pcadmin;
	rtadmin_t	rtadmin;
	rtdpent_t	*rt_dptbl;
	int		linenum;
	ulong		res;
	hrtime_t	hrtime;
	FILE		*fp;
	char		buf[512];
	int		wslength;

	strcpy(pcinfo.pc_clname, "RT");
	if (priocntl(0, 0, PC_GETCID, &pcinfo) == -1)
		fatalerr("%s: Can't get RT class ID, priocntl system \
call failed with errno %d\n", basenm, errno);

	pcadmin.pc_cid = pcinfo.pc_cid;
	pcadmin.pc_cladmin = (char *)&rtadmin;
	rtadmin.rt_cmd = RT_GETDPSIZE;

	if (priocntl(0, 0, PC_ADMIN, &pcadmin) == -1)
		fatalerr("%s: Can't get rt_dptbl size, priocntl system \
call failed with errno %d\n", basenm, errno);

	nrtdpents = rtadmin.rt_ndpents;
	if ((rt_dptbl =
	    (rtdpent_t *)malloc(nrtdpents * sizeof(rtdpent_t))) == NULL)
		fatalerr("%s: Can't allocate memory for rt_dptbl\n", basenm);

	if ((fp = fopen(infile, "r")) == NULL)
		fatalerr("%s: Can't open %s for input\n", basenm, infile);

	linenum = 0;

	/*
	 * Find the first non-blank, non-comment line.  A comment line
	 * is any line with '#' as the first non-white-space character.
	 */
	do {
		if (fgets(buf, sizeof(buf), fp) == NULL)
			fatalerr("%s: Too few lines in input table\n",basenm);
		linenum++;
	} while (buf[0] == '#' || buf[0] == '\0' ||
	    (wslength = strspn(buf, " \t\n")) == strlen(buf) ||
	    strchr(buf, '#') == buf + wslength);

	if ((tokp = strtok(buf, " \t")) == NULL)
		fatalerr("%s: Bad RES specification, line %d of input file\n",
		    basenm, linenum);
	if (strlen(tokp) > 4) {
		if (strncmp(tokp, "RES=", 4) != 0)
			fatalerr("%s: Bad RES specification, \
line %d of input file\n", basenm, linenum);
		if (tokp[4] == '-')
			fatalerr("%s: Bad RES specification, \
line %d of input file\n", basenm, linenum);
		res = strtoul(&tokp[4], (char **)NULL, 10);
	} else if (strlen(tokp) == 4) {
		if (strcmp(tokp, "RES=") != 0)
			fatalerr("%s: Bad RES specification, \
line %d of input file\n", basenm, linenum);
		if ((tokp = strtok(NULL, " \t")) == NULL)
			fatalerr("%s: Bad RES specification, \
line %d of input file\n", basenm, linenum);
		if (tokp[0] == '-')
			fatalerr("%s: Bad RES specification, \
line %d of input file\n", basenm, linenum);
		res = strtoul(tokp, (char **)NULL, 10);
	} else if (strlen(tokp) == 3) {
		if (strcmp(tokp, "RES") != 0)
			fatalerr("%s: Bad RES specification, \
line %d of input file\n", basenm, linenum);
		if ((tokp = strtok(NULL, " \t")) == NULL)
			fatalerr("%s: Bad RES specification, \
line %d of input file\n", basenm, linenum);
		if (strlen(tokp) > 1) {
			if (strncmp(tokp, "=", 1) != 0)
				fatalerr("%s: Bad RES specification, \
line %d of input file\n", basenm, linenum);
			if (tokp[1] == '-')
				fatalerr("%s: Bad RES specification, \
line %d of input file\n", basenm, linenum);
			res = strtoul(&tokp[1], (char **)NULL, 10);
		} else if (strlen(tokp) == 1) {
			if ((tokp = strtok(NULL, " \t")) == NULL)
				fatalerr("%s: Bad RES specification, \
line %d of input file\n", basenm, linenum);
			if (tokp[0] == '-')
				fatalerr("%s: Bad RES specification, \
line %d of input file\n", basenm, linenum);
			res = strtoul(tokp, (char **)NULL, 10);
		}
	} else {
		fatalerr("%s: Bad RES specification, line %d of input file\n",
		    basenm, linenum);
	}

	/*
	 * The remainder of the input file should contain exactly enough
	 * non-blank, non-comment lines to fill the table (rt_ndpents lines).
	 * We assume that any non-blank, non-comment line is data for the
	 * table and fail if we find more or less than we need.
	 */
	for (i = 0; i < rtadmin.rt_ndpents; i++) {

		/*
		 * Get the next non-blank, non-comment line.
		 */
		do {
			if (fgets(buf, sizeof(buf), fp) == NULL)
				fatalerr("%s: Too few lines in input table\n",
				    basenm);
			linenum++;
		} while (buf[0] == '#' || buf[0] == '\0' ||
		    (wslength = strspn(buf, " \t\n")) == strlen(buf) ||
		    strchr(buf, '#') == buf + wslength);

		if ((tokp = strtok(buf, " \t")) == NULL)
			fatalerr("%s: Too few values, line %d of input file\n",
			    basenm, linenum);

		if (res != HZ) {
			hrtime.hrt_secs = 0;
			hrtime.hrt_rem = atol(tokp);
			hrtime.hrt_res = res;
			if (_hrtnewres(&hrtime, HZ, HRT_RNDUP) == -1)
				fatalerr("%s: Can't convert specified \
resolution to ticks\n, basenm");
			if((rt_dptbl[i].rt_quantum = hrtconvert(&hrtime)) == -1)
				fatalerr("%s: rt_quantum value out of \
valid range; line %d of input,\ntable not overwritten\n", basenm, linenum);
		} else {
			rt_dptbl[i].rt_quantum = atol(tokp);
		}
		if (rt_dptbl[i].rt_quantum <= 0)
			fatalerr("%s: rt_quantum value out of valid range; \
line %d of input,\ntable not overwritten\n", basenm, linenum);

		if ((tokp = strtok(NULL, " \t")) != NULL && tokp[0] != '#')
			fatalerr("%s: Too many values, line %d of input file\n",
			    basenm, linenum);
	}

	/*
	 * We've read enough lines to fill the table.  We fail
	 * if the input file contains any more.
	 */
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		if (buf[0] != '#' && buf[0] != '\0' &&
		    (wslength = strspn(buf, " \t\n")) != strlen(buf) &&
		    strchr(buf, '#') != buf + wslength)
			fatalerr("%s: Too many lines in input table\n",
				basenm);
	}

	rtadmin.rt_dpents = rt_dptbl;
	rtadmin.rt_cmd = RT_SETDPTBL;
	if (priocntl(0, 0, PC_ADMIN, &pcadmin) == -1)
		fatalerr("%s: Can't set rt_dptbl, priocntl system call \
failed with errno %d\n", basenm, errno);
}
