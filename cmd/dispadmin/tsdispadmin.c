/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)dispadmin:tsdispadmin.c	1.5.1.1"
#include	<stdio.h>
#include	<string.h>
#include	<unistd.h>
#include	<sys/types.h>
#include	<sys/priocntl.h>
#include	<sys/tspriocntl.h>
#include	<sys/param.h>
#include	<sys/evecb.h>
#include	<sys/hrtcntl.h>
#include	<sys/ts.h>

/*
 * This file contains the class specific code implementing
 * the time-sharing dispadmin sub-command.
 */

#define	BASENMSZ	16

extern int	errno;
extern char	*basename();
extern void	fatalerr();
extern long	hrtconvert();

static void	get_tsdptbl(), set_tsdptbl();

static char usage[] =
"usage:	dispadmin -l\n\
	dispadmin -c TS -g [-r res]\n\
	dispadmin -c TS -s infile\n";

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
			if (strcmp(optarg, "TS") != 0)
				fatalerr("error: %s executed for %s class, \
%s is actually sub-command for TS class\n", cmdpath, optarg, cmdpath);
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

		printf("TS\t(Time Sharing)\n");
		exit(0);

	} else if (gflag) {
		if (lflag || sflag)
			fatalerr(usage);

		if (rflag == 0)
			res = 1000;

		get_tsdptbl(res);
		exit(0);

	} else if (sflag) {
		if (lflag || gflag || rflag)
			fatalerr(usage);

		set_tsdptbl(infile);
		exit(0);

	} else {
		fatalerr(usage);
	}
}


/*
 * Retrieve the current ts_dptbl from memory, convert the time quantum
 * values to the resolution specified by res and write the table to stdout.
 */
static void
get_tsdptbl(res)
ulong	res;
{
	int		i;
	int		tsdpsz;
	pcinfo_t	pcinfo;
	pcadmin_t	pcadmin;
	tsadmin_t	tsadmin;
	tsdpent_t	*ts_dptbl;
	hrtime_t	hrtime;

	strcpy(pcinfo.pc_clname, "TS");
	if (priocntl(0, 0, PC_GETCID, &pcinfo) == -1)
		fatalerr("%s: Can't get TS class ID, priocntl system \
call failed with errno %d\n", basenm, errno);

	pcadmin.pc_cid = pcinfo.pc_cid;
	pcadmin.pc_cladmin = (char *)&tsadmin;
	tsadmin.ts_cmd = TS_GETDPSIZE;

	if (priocntl(0, 0, PC_ADMIN, &pcadmin) == -1)
		fatalerr("%s: Can't get ts_dptbl size, priocntl system \
call failed with errno %d\n", basenm, errno);

	tsdpsz = tsadmin.ts_ndpents * sizeof(tsdpent_t);
	if ((ts_dptbl = (tsdpent_t *)malloc(tsdpsz)) == NULL)
		fatalerr("%s: Can't allocate memory for ts_dptbl\n", basenm);

	tsadmin.ts_dpents = ts_dptbl;

	tsadmin.ts_cmd = TS_GETDPTBL;
	if (priocntl(0, 0, PC_ADMIN, &pcadmin) == -1)
		fatalerr("%s: Can't get ts_dptbl, priocntl system call \
call failed with errno %d\n", basenm, errno);

	printf("# Time Sharing Dispatcher Configuration\n");
	printf("RES=%ld\n\n", res);
	printf("# ts_quantum  ts_tqexp  ts_slpret  ts_maxwait ts_lwait  \
PRIORITY LEVEL\n");

	for (i = 0; i < tsadmin.ts_ndpents; i++) {
		if (res != HZ) {
			hrtime.hrt_secs = 0;
			hrtime.hrt_rem = ts_dptbl[i].ts_quantum;
			hrtime.hrt_res = HZ;
			if (_hrtnewres(&hrtime, res, HRT_RNDUP) == -1)
				fatalerr("%s: Can't convert to requested \
resolution\n", basenm);
			if ((ts_dptbl[i].ts_quantum = hrtconvert(&hrtime))
			    == -1)
				fatalerr("%s: Can't express time quantum in \
requested resolution,\ntry coarser resolution\n", basenm);
		}
		printf("%10d%10d%10d%12d%10d        #   %3d\n",
		    ts_dptbl[i].ts_quantum, ts_dptbl[i].ts_tqexp,
		    ts_dptbl[i].ts_slpret, ts_dptbl[i].ts_maxwait,
		    ts_dptbl[i].ts_lwait, i);
	}
}


/*
 * Read the ts_dptbl values from infile, convert the time quantum values
 * to HZ resolution, do a little sanity checking and overwrite the table
 * in memory with the values from the file.
 */
static void
set_tsdptbl(infile)
char	*infile;
{
	int		i;
	int		ntsdpents;
	char		*tokp;
	pcinfo_t	pcinfo;
	pcadmin_t	pcadmin;
	tsadmin_t	tsadmin;
	tsdpent_t	*ts_dptbl;
	int		linenum;
	ulong		res;
	hrtime_t	hrtime;
	FILE		*fp;
	char		buf[512];
	int		wslength;

	strcpy(pcinfo.pc_clname, "TS");
	if (priocntl(0, 0, PC_GETCID, &pcinfo) == -1)
		fatalerr("%s: Can't get TS class ID, priocntl system \
call failed with errno %d\n", basenm, errno);

	pcadmin.pc_cid = pcinfo.pc_cid;
	pcadmin.pc_cladmin = (char *)&tsadmin;
	tsadmin.ts_cmd = TS_GETDPSIZE;

	if (priocntl(0, 0, PC_ADMIN, &pcadmin) == -1)
		fatalerr("%s: Can't get ts_dptbl size, priocntl system \
call failed with errno %d\n", basenm, errno);

	ntsdpents = tsadmin.ts_ndpents;
	if ((ts_dptbl =
	    (tsdpent_t *)malloc(ntsdpents * sizeof(tsdpent_t))) == NULL)
		fatalerr("%s: Can't allocate memory for ts_dptbl\n", basenm);

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
	 * non-blank, non-comment lines to fill the table (ts_ndpents lines).
	 * We assume that any non-blank, non-comment line is data for the
	 * table and fail if we find more or less than we need.
	 */
	for (i = 0; i < tsadmin.ts_ndpents; i++) {

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
resolution to ticks\n", basenm);
			if((ts_dptbl[i].ts_quantum = hrtconvert(&hrtime)) == -1)
				fatalerr("%s: ts_quantum value out of \
valid range; line %d of input,\ntable not overwritten\n", basenm, linenum);
		} else {
			ts_dptbl[i].ts_quantum = atol(tokp);
		}
		if (ts_dptbl[i].ts_quantum <= 0)
			fatalerr("%s: ts_quantum value out of valid range; \
line %d of input,\ntable not overwritten\n", basenm, linenum);

		if ((tokp = strtok(NULL, " \t")) == NULL || tokp[0] == '#')
			fatalerr("%s: Too few values, line %d of input file\n",
			    basenm, linenum);
		ts_dptbl[i].ts_tqexp = (short)atoi(tokp);
		if (ts_dptbl[i].ts_tqexp < 0 ||
		    ts_dptbl[i].ts_tqexp > tsadmin.ts_ndpents)
			fatalerr("%s: ts_tqexp value out of valid range; \
line %d of input,\ntable not overwritten\n", basenm, linenum);

		if ((tokp = strtok(NULL, " \t")) == NULL || tokp[0] == '#')
			fatalerr("%s: Too few values, line %d of input file\n",
			    basenm, linenum);
		ts_dptbl[i].ts_slpret = (short)atoi(tokp);
		if (ts_dptbl[i].ts_slpret < 0 ||
		    ts_dptbl[i].ts_slpret > tsadmin.ts_ndpents)
			fatalerr("%s: ts_slpret value out of valid range; \
line %d of input,\ntable not overwritten\n", basenm, linenum);

		if ((tokp = strtok(NULL, " \t")) == NULL || tokp[0] == '#')
			fatalerr("%s: Too few values, line %d of input file\n",
			    basenm, linenum);
		ts_dptbl[i].ts_maxwait = (short)atoi(tokp);
		if (ts_dptbl[i].ts_maxwait < 0)
			fatalerr("%s: ts_maxwait value out of valid range; \
line %d of input,\ntable not overwritten\n", basenm, linenum);

		if ((tokp = strtok(NULL, " \t")) == NULL || tokp[0] == '#')
			fatalerr("%s: Too few values, line %d of input file\n",
			    basenm, linenum);
		ts_dptbl[i].ts_lwait = (short)atoi(tokp);
		if (ts_dptbl[i].ts_lwait < 0 ||
		    ts_dptbl[i].ts_lwait > tsadmin.ts_ndpents)
			fatalerr("%s: ts_lwait value out of valid range; \
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

	tsadmin.ts_dpents = ts_dptbl;
	tsadmin.ts_cmd = TS_SETDPTBL;
	if (priocntl(0, 0, PC_ADMIN, &pcadmin) == -1)
		fatalerr("%s: Can't set ts_dptbl, priocntl system call \
failed with errno %d\n", basenm, errno);
}
