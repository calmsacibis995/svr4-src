/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)sa:sar.c	1.67"

/*
 *	sar.c - It generates a report either from an input data file or by
 *		invoking sadc to read system activity counters at the
 *		specified intervals.
 */

#include <stdio.h>
#include <sys/param.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <sys/fcntl.h>
#include <sys/flock.h>
#include <sys/fs/rf_acct.h>

#include "sa.h"

struct 	sa 	nx,ox,ax,bx;
struct 	tm 	*localtime(), *curt,args,arge;
struct 	utsname name;
int	sflg, eflg, iflg, oflg, fflg;
int	dflg=0;
float	Isyscall, Isysread, Isyswrite, Isysexec, Ireadch, Iwritech;
float	Osyscall, Osysread, Osyswrite, Osysexec, Oreadch, Owritech;
float 	Lsyscall, Lsysread, Lsyswrite, Lsysexec, Lreadch, Lwritech;

/* 
 *	The following are to keep averages for sar -x since both sar -x and
 * 	sar -c keep averages on open, lookup, create, and readdir operations 
 */
ulong   a_rfs_in_open, a_rfs_in_lookup, a_rfs_in_create, a_rfs_in_readdir;
ulong   a_rfs_out_open, a_rfs_out_lookup, a_rfs_out_create, a_rfs_out_readdir;

int	realtime;
int	passno;
int	t=0;
int	n=0;
int	lines=0;

#if	u3b2 || u3b5 || i386
int	hz;
double 	magic = 4.294967296e9;
#endif

#ifndef u370
int	recsz, tblmap[SINFO];
#endif

#ifdef u370
/* tblmap is a kludge for the 370 - not really used */
int 	recsz, tblmap[10];	
#endif

int	i,j;	
int	tabflg;
char	options[29],fopt[29];
char	cc;
float	tdiff;
float	sec_diff, totsec_diff=0.0;
time_t	ts, te;

float	stime, etime, isec;
int	fin, fout;
pid_t	childid;
int	pipedes[2];
char	arg1[10], arg2[10];
int	strlen(), strdmp();
char	*strcpy(), *strncat(), *strncpy(), *strchr();
extern  int optind;
extern  char *optarg;

main (argc,argv)
char	**argv;
int	argc;
{
	char    flnm[50], ofile[50];
	char	ccc;
	long    temp;
	int	jj=0;
	float	convtm();
	long	lseek();
	extern 	time_t time();

	/*    process options with arguments and pack options without arguments  */
	while ((i= getopt(argc,argv,"uybdvcwaqmpgrkxACDSo:s:e:i:f:")) != EOF)
		switch(ccc = i) {
		case 'D':
			dflg++;
			break;
		case 'o':
			oflg++;
			sprintf(ofile,"%s",optarg);
			break;
		case 's':
			if (sscanf(optarg,"%d:%d:%d",
		    	    &args.tm_hour, &args.tm_min, &args.tm_sec) < 1)
				pillarg();
			else {
				sflg++,
				stime = args.tm_hour*3600.0 +
					args.tm_min*60.0 +
					args.tm_sec;
			}
			break;
		case 'e':
			if (sscanf(optarg,"%d:%d:%d",
			    &arge.tm_hour, &arge.tm_min, &arge.tm_sec) < 1)
				pillarg();
			else {
				eflg++;
				etime = arge.tm_hour*3600.0 +
					arge.tm_min*60.0 +
					arge.tm_sec;
			}
			break;
		case 'i':
			if (sscanf(optarg,"%f",&isec) < 1)
				pillarg();
			else {
				if (isec > 0.0)
					iflg++;
			}
			break;
		case 'f':
			fflg++;
			sprintf(flnm,"%s",optarg);
			break;
		case '?':
			fprintf(stderr,"usage: sar [-ubdycwaqvmpgrkxACDS][-o file] t [n]\n");
			fprintf(stderr,"       sar [-ubdycwaqvmpgrkxACDS][-s hh:mm][-e hh:mm][-i ss][-f file]\n");
			exit(2);
			break;
		default:
			strncat(options,&ccc,1);
			break;
		}

	/*   Are starting and ending times consistent?  */
	if ((sflg) && (eflg) && (etime <= stime))
		pmsgexit("etime <= stime");

	/*   
	 *   Determine if t and n arguments are given, and whether to run in 
	 *   real time or from a file.
         */
	switch(argc - optind) {
	case 0:		/*   Get input data from file   */
		if (fflg == 0) {
			temp = time((long *) 0);
			curt = localtime(&temp);
			sprintf(flnm,"/var/adm/sa/sa%.2d", curt->tm_mday);
		}
		if ((fin = open(flnm, 0)) == -1) {
			fprintf(stderr, "sar:Can't open %s\n", flnm);
			exit(1);
		}
		break;
	case 1:		/*   Real time data; one cycle   */
		realtime++;
		t = atoi(argv[optind]);
		n = 2;
		break;
	case 2:		/*   Real time data; specified cycles   */
	default:
		realtime++;
		t = atoi(argv[optind]);
		n = 1 + atoi(argv[optind+1]);
		break;
	}

	/*  "u" is default option to display cpu utilization   */
	if (strlen(options) == 0)
		strcpy(options, "u");
	/*  'A' means all data options   */
	if (strchr(options, 'A') != NULL) {
		strcpy(options, "udqbwcayvmpgrkxCS");
		dflg++;
	}
	else if ((dflg)
	     && (strchr(options,'b') == NULL)
	     && (strchr(options,'u') == NULL)
	     && (strchr(options,'c') == NULL)) strcat(options,"u");

	if (realtime) {
	/*  Get input data from sadc via pipe   */
		if ((t <= 0) || (n < 2))
			pmsgexit("args t & n <= 0");
		sprintf(arg1,"%d", t);
		sprintf(arg2,"%d", n);
		if (pipe(pipedes) == -1)
			perrexit();
		if ((childid = fork()) == 0){	
			close(1);       /*  shift pipedes[write] to stdout  */
			dup(pipedes[1]);
			if (execlp ("/usr/lib/sa/sadc","/usr/lib/sa/sadc",arg1,arg2,0) == -1)
				perrexit();
		} else if (childid == -1) {
			pmsgexit("Could not fork to exec sadc");
		}		/*   parent:   */
		fin = pipedes[0];
		close(pipedes[1]);	/*   Close unused output   */
	}

	if(oflg) {
		if(strcmp(ofile, flnm) == 0)
			pmsgexit("ofile same as ffile");
		fout = creat(ofile, 00644);
	}

	/*    read the header record and compute record size */
	if (read(fin, tblmap, sizeof tblmap) < 0)
		perrexit ();

#ifndef u370
	for (i=0; i<SINFO; i++)
		recsz += tblmap[i];
	recsz = sizeof (struct sa) - sizeof nx.devio + recsz * sizeof nx.devio[0];
#endif

#ifdef u370
		recsz = sizeof (struct sa);
#endif

	if (oflg) write(fout, tblmap, sizeof(tblmap));

	/*   Make single pass, processing all options   */
	if (realtime) {
		strcpy(fopt, options);
		passno++;
		prpass();
		kill(childid, 2);
		wait((int *) 0);
	}
	else {
		/*   Make multiple passes, one for each option   */
		while(strlen(strncpy(fopt, &options[jj++], 1)) > 0) {
			lseek(fin, (long)(sizeof tblmap), 0);
			passno++;
			prpass();
		}
	}
	exit(0);
}


/*****************************************************/
/*	Read records from input, classify, and 	     */
/*	decide on printing                           */
/*****************************************************/
prpass()
{
	int 	recno=0;
	float 	tnext=0;
	float 	trec;

	if (sflg) 
		tnext = stime;
	while (read(fin, &nx, (unsigned)recsz) > 0) {
		curt = localtime(&nx.ts);
		trec = curt->tm_hour * 3600.0 
			+ curt->tm_min * 60.0
			+ curt->tm_sec;
		if ((recno == 0) && (trec < stime))
			continue;
		if ((eflg) && (trec > etime))
			break;
		if ((oflg) && (passno == 1))
			write(fout, &nx, (unsigned)recsz);
		if (recno == 0) {
			if (passno == 1) {
				uname(&name);
				printf("\n%s %s %s %s %s    %.2d/%.2d/%.2d\n",
					name.sysname,
					name.nodename,
					name.release,
					name.version,
					name.machine,
					curt->tm_mon + 1,
					curt->tm_mday,
					curt->tm_year);
			}
			prthdg();
			recno = 1;
			if ((iflg) && (tnext == 0))
				tnext = trec;
		}
		if ((nx.si.cpu[0] + nx.si.cpu[1] + nx.si.cpu[2] + nx.si.cpu[3]) < 0) {
		/*  
		 *    This dummy record signifies system restart.  New initial 
		 *    values of counters follow in next record prevent printing 
		 *    f this is for option 'U' and no other processor exists 
		 */

#ifdef i386
		   if (!((fopt[0] == 'U') && (!realtime))) {
#else
		   if (!((fopt[0] == 'U') && (!nx.bpb_utilize) && (!realtime))) {
#endif

			prttim();
			printf("\tunix restarts\n");
			recno = 1;
			continue;
		   }
		}
		if ((iflg) && (trec < tnext))
			continue;
		if (recno++ > 1) {

#ifdef u370
			tdiff = nx.elpstm - ox.elpstm;
			sec_diff = (float) (nx.elpstm/HZ - ox.elpstm/HZ);
#else
			ts = ox.si.cpu[0] + ox.si.cpu[1] + ox.si.cpu[2] + ox.si.cpu[3];
			te = nx.si.cpu[0] + nx.si.cpu[1] + nx.si.cpu[2] + nx.si.cpu[3];
			tdiff = (float) (te - ts);
			sec_diff = (float) (te/HZ - ts/HZ);
			if (nx.apstate) {
				tdiff = tdiff/2;
				sec_diff = sec_diff/2;
			}
#endif
			if (tdiff <= 0) 
				continue;
			prtopt();	/*  Print a line of data  */
			if (passno == 1) {
				totsec_diff += sec_diff;
				lines++;
			}
		}
		ox = nx;		/*  Age the data	*/
		if (isec > 0)
			while (tnext <= trec)
				tnext += isec;
	}
	if (lines > 1)
		prtavg();
	ax = bx;		/*  Zero out the accumulators   */
}


/************************************************************/
/*      print time label routine	                    */
/************************************************************/
prttim()
{
	curt = localtime(&nx.ts);
	printf("%.2d:%.2d:%.2d",
		curt->tm_hour,
		curt->tm_min,
		curt->tm_sec);
	tabflg = 1;
}


/***********************************************************/
/*      test if 8-spaces to be added routine               */
/***********************************************************/
tsttab()
{
	if (tabflg == 0) 
		printf("        ");
	else
		tabflg = 0;
}


/************************************************************/
/*      print report heading routine                        */
/************************************************************/
prthdg()
{
	int	jj=0;
	char	ccc;

	printf("\n");
	prttim();
	while((ccc = fopt[jj++]) != NULL)
	switch(ccc){
	case 'u':
		tsttab();

#ifndef u370
		if (dflg) {
			printf(" %7s %7s %7s %7s %7s\n",
				"%usr",
				"%sys",
				"%sys",
				"%wio",
				"%idle");
			tsttab();
			printf(" %7s %7s %7s\n",
				"",
				"local",
				"remote");
			break;
		}
		printf(" %7s %7s %7s %7s\n",
			"%usr",
			"%sys",
			"%wio",
			"%idle");
#endif

#ifdef u370
		printf(" %7s %7s %7s %7s\n",
			"%usr",
			"%usup",
			"%tss",
			"%idle");
#endif

		break;
	case 'y':
		tsttab();

#ifndef u370
		printf(" %7s %7s %7s %7s %7s %7s\n",
			"rawch/s",
			"canch/s",
			"outch/s",
			"rcvin/s",
			"xmtin/s",
			"mdmin/s");
#endif

#ifdef u370
		printf(" %7s %7s %7s %7s\n", 
			"rch/s", 
			"s1rch/s", 
			"wch/s", 
			"s1wch/s"); 
#endif 

		break;
	case 'b':
		tsttab();
		printf(" %7s %7s %7s %7s %7s %7s %7s %7s\n",
			"bread/s",
			"lread/s",
			"%rcache",
			"bwrit/s",
			"lwrit/s",
			"%wcache",
			"pread/s",
			"pwrit/s");
		break;
	case 'd':
		tsttab();

#ifndef u370
		printf(" %7s %7s %7s %7s %7s %7s %7s\n",
			"device",
			"%busy",
			"avque",
			"r+w/s",
			"blks/s",
			"avwait",
			"avserv");
#endif

#ifdef u370
		printf(" %7s %7s %7s %7s %7s %7s\n", 
			"device", 
			"sread/s", 
			"pread/s", 
			"swrit/s", 
			"pwrit/s",
			"total/s");
#endif

		break;
	case 'v':
		tsttab();
		printf(" %s %s %s %s\n",
			"proc-sz ov",
			"inod-sz ov",
			"file-sz ov",
			"lock-sz");
		break;
	case 'c':
		tsttab();
		printf(" %7s %7s %7s %7s %7s %7s %7s\n",
			"scall/s",
			"sread/s",
			"swrit/s",
			"fork/s",
			"exec/s",
			"rchar/s",
			"wchar/s");
		break;
	case 'w':
		tsttab();

#ifndef u370
		printf(" %7s %7s %7s %7s %7s\n",
			"swpin/s",
			"pswin/s",
			"swpot/s",
			"pswot/s",
			"pswch/s");
#endif

#ifdef u370
		printf(" %7s %7s %7s\n",
			"sched/s",
			"tsend/s",
			"disp/s");
#endif

		break;
	case 'a':
		tsttab();
		printf(" %7s %7s %7s\n",
			"iget/s",
			"namei/s",
			"dirbk/s");
		break;
	case 'q':
		tsttab();

#ifndef u370
		printf(" %7s %7s %7s %7s\n",
			"runq-sz",
			"%runocc",
			"swpq-sz",
			"%swpocc");
#endif

#ifdef u370
		printf(" %7s %7s %7s %7s %7s\n",
			"run-sz",
			"wtsm-sz",
			"semtm/p",
			"wtio-sz",
			"tmio/p");
#endif 

		break;
	case 'm':
		tsttab();
		printf(" %7s %7s\n",
			"msg/s",
			"sema/s");
		break;

#if     vax || u3b || u3b5 || u3b2 || i386
	case 'r':
		tsttab();
		printf(" %7s %7s\n",
			"freemem",
			"freeswp");
		break;
#endif

#if	u3b2 || u3b15 || i386
	case 'k':
		tsttab();
		printf(" %7s %7s %5s %7s %7s %5s %11s %5s\n",
			"sml_mem",
			"alloc",
			"fail",
			"lg_mem",
			"alloc",
			"fail",
			"ovsz_alloc",
			"fail");
		break;
	case 'x':
		tsttab();
		printf(" %6s %8s %8s %9s %9s %9s %7s\n",
			"open/s",
			"create/s",
			"lookup/s",
			"readdir/s",
			"getpage/s",
			"putpage/s",
			"other/s");
		break;
#endif

#if     u3b || u3b5 || u3b2 || i386
	case 'S':
		tsttab();
		printf("%13s %9s %9s %9s %9s\n",
			"serv/lo - hi",
			"request",
			"request",
			"server",
			"server");
		tsttab();
		printf("%8d -%3d %8s %10s %9s %10s\n",
			nx.minserve,
			nx.maxserve,
			"%busy",
			"avg lgth",
			"%avail",
			"avg avail");
		break;
	case 'C':
		tsttab();
		printf(" %11s %11s %11s %11s %11s %11s\n",
			"snd-inv/s",
			"snd-msg/s",
			"rcv-inv/s",
			"rcv-msg/s",
			"dis-bread/s",
			"blk-inv/s");
		break;
#endif

#if     vax || u3b || u3b5 || u3b2 || i386
	case 'p':
		tsttab();
		printf(" %7s %7s %7s %7s %7s %7s\n",
			"atch/s",
			"pgin/s",
			"ppgin/s",
			"pflt/s",
			"vflt/s",
			"slock/s");
		break;
	case 'g':
		tsttab();
		printf(" %8s %8s %8s %8s %6s\n",
			"pgout/s",
			"ppgout/s",
			"pgfree/s",
			"pgscan/s",
			"%s5ipf");
		break;
#endif

	}
	if (jj > 2) printf("\n");
}


/**********************************************************/
/*      print options routine                             */
/**********************************************************/
prtopt()
{
	register int ii,kk,mm;
	int 	jj=0;
	char	ccc;

	if (strcmp(fopt, "d") == 0)   
		printf("\n");
	prttim();

#ifndef u370
	for(ii=0; ii<4; ii++)
		ax.si.cpu[ii] += nx.si.cpu[ii] - ox.si.cpu[ii];
	if (dflg) ax.rf_srv.rfsi_serve += nx.rf_srv.rfsi_serve - ox.rf_srv.rfsi_serve;
#endif

#ifdef u370
	ax.elpstm += nx.elpstm - ox.elpstm;

	/*    get elapsed time from tss table - u370 */
	ax.tmelps += nx.ccv - ox.ccv;	
	ax.nap = nx.nap;
#endif 

	while ((ccc = fopt[jj++]) != NULL)
	switch(ccc){
	case 'u':
		tsttab();

#ifndef u370
	if (dflg) {
		if (nx.apstate) {
			printf(" %7.0f %7.0f %7.0f %7.0f %7.0f\n",
				(float)(nx.si.cpu[1] - ox.si.cpu[1])/(2*tdiff) * 100.0,

				(float)(nx.si.cpu[2] - ox.si.cpu[2] -
				(nx.rf_srv.rfsi_serve - ox.rf_srv.rfsi_serve) )/(2*tdiff) * 100.0,
				(float)(nx.rf_srv.rfsi_serve - ox.rf_srv.rfsi_serve)/(2*tdiff) * 100.0,
				(float)(nx.si.cpu[3] - ox.si.cpu[3])/(2*tdiff) * 100.0,
				(float)(nx.si.cpu[0] - ox.si.cpu[0])/(2*tdiff) * 100.0);
		} else {
			printf(" %7.0f %7.0f %7.0f %7.0f %7.0f\n",
				(float)(nx.si.cpu[1] - ox.si.cpu[1])/tdiff * 100.0,
				(float)(nx.si.cpu[2] - ox.si.cpu[2] -
			       	(nx.rf_srv.rfsi_serve - ox.rf_srv.rfsi_serve) )/tdiff * 100.0,
				(float)(nx.rf_srv.rfsi_serve - ox.rf_srv.rfsi_serve)/tdiff * 100.0,
				(float)(nx.si.cpu[3] - ox.si.cpu[3])/tdiff * 100.0,
				(float)(nx.si.cpu[0] - ox.si.cpu[0])/tdiff * 100.0);
		}
		break;
	}
	if (nx.apstate)
		printf(" %7.0f %7.0f %7.0f %7.0f\n",
		    	(float)(nx.si.cpu[1] - ox.si.cpu[1])/(2*tdiff) * 100.0,
		    	(float)(nx.si.cpu[2] - ox.si.cpu[2])/(2*tdiff) * 100.0,
		    	(float)(nx.si.cpu[3] - ox.si.cpu[3])/(2*tdiff) * 100.0,
		    	(float)(nx.si.cpu[0] - ox.si.cpu[0])/(2*tdiff) * 100.0);
	else
		printf(" %7.0f %7.0f %7.0f %7.0f\n",
			(float)(nx.si.cpu[1] - ox.si.cpu[1])/tdiff * 100.0,
		    	(float)(nx.si.cpu[2] - ox.si.cpu[2])/tdiff * 100.0,
		    	(float)(nx.si.cpu[3] - ox.si.cpu[3])/tdiff * 100.0,
		    	(float)(nx.si.cpu[0] - ox.si.cpu[0])/tdiff * 100.0);
#endif

#ifdef u370
	printf(" %7.0f %7.0f %7.0f %7.0f\n",
		(float)(nx.usrtm - ox.usrtm)/ 
		((nx.ccv - ox.ccv)*(double)nx.nap)*100.0, 
		(float)(nx.usuptm - ox.usuptm)/ 
		((nx.ccv - ox.ccv)*(double)nx.nap)*100.0, 
		(float)((nx.ccv - ox.ccv)*(double)nx.nap - 
			(nx.idletm - ox.idletm) - 
			(nx.vmtm - ox.vmtm))/ 
			((nx.ccv - ox.ccv)*(double)nx.nap)*100.0, 
		(float)(nx.idletm - ox.idletm)/ 
			((nx.ccv - ox.ccv)*(double)nx.nap)*100.0); 

		ax.idletm += nx.idletm - ox.idletm;
		ax.usrtm += nx.usrtm - ox.usrtm;
		ax.usuptm  += nx.usuptm - ox.usuptm;
		ax.vmtm += nx.vmtm - ox.vmtm;
#endif

		break;
	case 'y':
		tsttab();

#ifndef u370
		printf(" %7.0f %7.0f %7.0f %7.0f %7.0f %7.0f\n",
			(float)(nx.si.rawch - ox.si.rawch)/tdiff * HZ,
			(float)(nx.si.canch - ox.si.canch)/tdiff * HZ,
			(float)(nx.si.outch - ox.si.outch)/tdiff * HZ,
			(float)(nx.si.rcvint - ox.si.rcvint)/tdiff * HZ,
			(float)(nx.si.xmtint - ox.si.xmtint)/tdiff * HZ,
			(float)(nx.si.mdmint - ox.si.mdmint)/tdiff * HZ);

		ax.si.rawch += nx.si.rawch - ox.si.rawch;
		ax.si.canch += nx.si.canch - ox.si.canch;
		ax.si.outch += nx.si.outch - ox.si.outch;
		ax.si.rcvint += nx.si.rcvint - ox.si.rcvint;
		ax.si.xmtint += nx.si.xmtint - ox.si.xmtint;
		ax.si.mdmint += nx.si.mdmint - ox.si.mdmint;
#endif

#ifdef u370
		printf(" %7.0f %7.0f %7.0f %7.0f\n", 
			(float)(nx.si.termin - ox.si.termin)/tdiff * HZ, 
			(float)(nx.si.s1in - ox.si.s1in)/tdiff * HZ, 
			(float)(nx.si.termout - ox.si.termout)/tdiff * HZ, 
			(float)(nx.si.s1out - ox.si.s1out)/tdiff * HZ); 

		ax.si.termin += nx.si.termin - ox.si.termin; 
		ax.si.s1in += nx.si.s1in - ox.si.s1in; 
		ax.si.termout += nx.si.termout - ox.si.termout; 
		ax.si.s1out += nx.si.s1out - ox.si.s1out; 
#endif

		break;
	case 'b':
		tsttab();
		if (dflg) {
			printf("\n   local  %4.0f %7.0f %7.0f %7.0f %7.0f %7.0f %7.0f %7.0f\n",
				(float)(nx.si.bread - ox.si.bread)/tdiff * HZ,
				(float)(nx.si.lread - ox.si.lread)/tdiff * HZ,
                                ((nx.si.lread - ox.si.lread) <= 0) ? 100 :
				(((float)(nx.si.lread - ox.si.lread) -
				  (float)(nx.si.bread - ox.si.bread))/
				  (float)(nx.si.lread - ox.si.lread) * 100.0),
				(float)(nx.si.bwrite - ox.si.bwrite)/tdiff * HZ,
				(float)(nx.si.lwrite - ox.si.lwrite)/tdiff * HZ,
				((nx.si.lwrite - ox.si.lwrite) <= 0) ? 100 :
				(((float)(nx.si.lwrite - ox.si.lwrite) -
				  (float)(nx.si.bwrite - ox.si.bwrite))/
				  (float)(nx.si.lwrite - ox.si.lwrite) * 100.0),
				(float)(nx.si.phread - ox.si.phread)/tdiff * HZ,
				(float)(nx.si.phwrite - ox.si.phwrite)/tdiff * HZ);

			ax.si.bread += nx.si.bread - ox.si.bread;
			ax.si.bwrite += nx.si.bwrite - ox.si.bwrite;
			ax.si.lread += nx.si.lread - ox.si.lread;
			ax.si.lwrite += nx.si.lwrite - ox.si.lwrite;
			ax.si.phread += nx.si.phread - ox.si.phread;
			ax.si.phwrite += nx.si.phwrite - ox.si.phwrite;

			printf("   remote %4.0f %7.0f %7.0f %7.0f %7.0f %7.0f \n",
				(float)((nx.rfc.rfci_pmread - ox.rfc.rfci_pmread)
					/tdiff * HZ) * BLKPERPG,
				(float)((nx.rfc.rfci_ptread - ox.rfc.rfci_ptread)
					/tdiff * HZ) * BLKPERPG,
				(((float)(nx.rfc.rfci_ptread - ox.rfc.rfci_ptread) -
	         		(float)(nx.rfc.rfci_pmread - ox.rfc.rfci_pmread))/
				(float)((nx.rfc.rfci_ptread - ox.rfc.rfci_ptread)?
				(nx.rfc.rfci_ptread - ox.rfc.rfci_ptread):1) * 100.0),
				(float)((nx.rfc.rfci_pmwrite - ox.rfc.rfci_pmwrite)
					/tdiff * HZ) * BLKPERPG,
				(float)((nx.rfc.rfci_ptwrite - ox.rfc.rfci_ptwrite)
					/tdiff * HZ) * BLKPERPG,
				(((float)(nx.rfc.rfci_ptwrite - ox.rfc.rfci_ptwrite) -
				(float)(nx.rfc.rfci_pmwrite - ox.rfc.rfci_pmwrite))/
				(float)((nx.rfc.rfci_ptwrite - ox.rfc.rfci_ptwrite)?
				(nx.rfc.rfci_ptwrite - ox.rfc.rfci_ptwrite):1) * 100.0));

			ax.rfc.rfci_pmread += nx.rfc.rfci_pmread - ox.rfc.rfci_pmread;
			ax.rfc.rfci_ptread += nx.rfc.rfci_ptread - ox.rfc.rfci_ptread;
			ax.rfc.rfci_pmwrite += nx.rfc.rfci_pmwrite - ox.rfc.rfci_pmwrite;
			ax.rfc.rfci_ptwrite += nx.rfc.rfci_ptwrite - ox.rfc.rfci_ptwrite;
			break;
		}

		printf(" %7.0f %7.0f %7.0f %7.0f %7.0f %7.0f %7.0f %7.0f\n",
			(float)(nx.si.bread - ox.si.bread)/tdiff * HZ,
			(float)(nx.si.lread - ox.si.lread)/tdiff * HZ,
			((nx.si.lread - ox.si.lread) <= 0) ? 100 :
			(((float)(nx.si.lread - ox.si.lread) -
			  (float)(nx.si.bread - ox.si.bread))/
			  (float)(nx.si.lread - ox.si.lread) * 100.0),
			(float)(nx.si.bwrite - ox.si.bwrite)/tdiff * HZ,
			(float)(nx.si.lwrite - ox.si.lwrite)/tdiff * HZ,
			((nx.si.lwrite - ox.si.lwrite) <= 0) ? 100 :
			(((float)(nx.si.lwrite - ox.si.lwrite) -
			  (float)(nx.si.bwrite - ox.si.bwrite))/
			  (float)(nx.si.lwrite - ox.si.lwrite) * 100.0),
			(float)(nx.si.phread - ox.si.phread)/tdiff * HZ,
			(float)(nx.si.phwrite - ox.si.phwrite)/tdiff * HZ);

		ax.si.bread += nx.si.bread - ox.si.bread;
		ax.si.bwrite += nx.si.bwrite - ox.si.bwrite;
		ax.si.lread += nx.si.lread - ox.si.lread;
		ax.si.lwrite += nx.si.lwrite - ox.si.lwrite;
		ax.si.phread += nx.si.phread - ox.si.phread;
		ax.si.phwrite += nx.si.phwrite - ox.si.phwrite;
		break;
	case 'd':

#ifndef u370
		ii = 0;
		for (j=0; j<SINFO; j++){
                        hz = HZ;
#ifdef i386
            		if ( j == SD01 )
                                hz = 1000;
#endif
			for (kk=0; kk<tblmap[j]; kk++){

				if (((nx.devio[ii][0] - ox.devio[ii][0]) > 0) 
				&& ((nx.devio[ii][2] - ox.devio[ii][2]) > 0)){

					tsttab();
#ifdef u3b
		if (j == DSKINFO)  
#endif

#ifdef u3b5
		if (j == DFDFC) 
#endif

#ifdef u3b2
		if (j == ID || j == IF)
#endif

#ifdef i286
		if (j == WNS)
#endif

#ifdef i386
		if ((j == ID) || (j == SD01))
#endif

#if (!defined(u3b) && !defined(u3b5) && !defined(u3b2) && !defined(i286) && !defined(i386))
		if (j == GDS) 
#endif

			printf(" %4s%-3d", devnm[j], kk);
		else
			printf(" %5s%-2d", devnm[j], kk);

/* Check if denaminator is 0 blank out the field */

		if ( (tdiff != 0) 
			&& ((nx.devio[ii][2] - ox.devio[ii][2]) != 0)
			&& ((nx.devio[ii][0] - ox.devio[ii][0]) != 0)) {
			printf(" %7.0f %7.1f %7.0f %7.0f %7.1f %7.1f\n",
			(float)(nx.devio[ii][2] - ox.devio[ii][2])/(tdiff*hz/HZ) *100.0,
			(float)(nx.devio[ii][3] - ox.devio[ii][3])/
				(float)(nx.devio[ii][2] - ox.devio[ii][2]),
			(float)(nx.devio[ii][0] - ox.devio[ii][0])/tdiff * HZ,
			(float)(nx.devio[ii][1] - ox.devio[ii][1])/tdiff * HZ,
			((float)(nx.devio[ii][3] - ox.devio[ii][3]) -
				(float)(nx.devio[ii][2] - ox.devio[ii][2]))/
				(float)(nx.devio[ii][0] - ox.devio[ii][0])/
				hz * 1000.,
			(float)(nx.devio[ii][2] - ox.devio[ii][2]) /
				(float)(nx.devio[ii][0] - ox.devio[ii][0] )/hz * 1000.);
			}else  
				if ( (tdiff != 0) 
					&& ((nx.devio[ii][2] - ox.devio[ii][2]) != 0)) {
				printf(" %7.0f %7.1f %7.0f %7.0f %7.1f %7.1f\n",
				(float)(nx.devio[ii][2] - ox.devio[ii][2])/(tdiff*hz/HZ) *100.0,
				(float)(nx.devio[ii][3] - ox.devio[ii][3])/
					(float)(nx.devio[ii][2] - ox.devio[ii][2]),
				(float)(nx.devio[ii][0] - ox.devio[ii][0])/tdiff * HZ,
				(float)(nx.devio[ii][1] - ox.devio[ii][1])/tdiff * HZ,"       ","       ");
			}
			else 	
				if ( (tdiff != 0)  ) {
				printf(" %7.0f %7.1f %7.0f %7.0f %7.1f %7.1f\n",
				(float)(nx.devio[ii][2] - ox.devio[ii][2])/(tdiff*hz/HZ) *100.0,"       ",
				(float)(nx.devio[ii][0] - ox.devio[ii][0])/tdiff * HZ,
				(float)(nx.devio[ii][1] - ox.devio[ii][1])/tdiff * HZ,"       ", "       ");
				}
			else {

				printf(" %7.0f %7.1f %7.0f %7.0f %7.1f %7.1f\n",
			"       ", "       ", "       ", "       ", 
			"       ", "       ");	
			}
				for(mm=0; mm<4; mm++)
					ax.devio[ii][mm] +=
					nx.devio[ii][mm] - ox.devio[ii][mm];
				}
				ii++;
			}
		}
#endif

#ifdef u370
		for (ii=0; ii<NDEV; ii++){ 
			if ((nx.io[ii].io_total - ox.io[ii].io_total)/HZ == 0)
				continue;
			tsttab(); 
			if (ii < NDRUM) 
				printf(" drm%-4d", ii+1); 
			else 
				printf(" dsk%-4d", ii - NDRUM + 1); 
			printf(" %7.1f %7.1f %7.1f %7.1f %7.1f\n", 
				(float)(nx.io[ii].io_sread - ox.io[ii].io_sread)/tdiff * HZ, 
				(float)(nx.io[ii].io_pread - ox.io[ii].io_pread)/tdiff * HZ, 
				(float)(nx.io[ii].io_swrite - ox.io[ii].io_swrite)/tdiff * HZ, 
				(float)(nx.io[ii].io_pwrite - ox.io[ii].io_pwrite)/tdiff * HZ, 
				(float)(nx.io[ii].io_total - ox.io[ii].io_total)/tdiff * HZ); 

			ax.io[ii].io_sread += nx.io[ii].io_sread - ox.io[ii].io_sread; 
			ax.io[ii].io_pread += nx.io[ii].io_pread - ox.io[ii].io_pread; 
			ax.io[ii].io_swrite += nx.io[ii].io_swrite - ox.io[ii].io_swrite; 
			ax.io[ii].io_pwrite += nx.io[ii].io_pwrite - ox.io[ii].io_pwrite; 
			ax.io[ii].io_total += nx.io[ii].io_total - ox.io[ii].io_total;
	} 
#endif 

		break;
	case 'v':
		tsttab();
		printf(" %3d/%3d%3ld %3d/%3d%3ld %3d/%3d%3ld %3d/%3d\n",
			nx.szproc, nx.mszproc, (nx.procovf - ox.procovf),
			nx.szinode, nx.mszinode, (nx.inodeovf - ox.inodeovf),
			nx.szfile, nx.mszfile, (nx.fileovf - ox.fileovf),
			nx.szlckr, nx.mszlckr);
		break;
	case 'c':
		tsttab();
	if (dflg) {
		Isyscall = ((float)(nx.rfs_in.fsivop_open - ox.rfs_in.fsivop_open)
		 	+ (float)(nx.rfs_in.fsivop_close - ox.rfs_in.fsivop_close)
		 	+ (float)(nx.rfs_in.fsivop_read - ox.rfs_in.fsivop_read)
		 	+ (float)(nx.rfs_in.fsivop_write - ox.rfs_in.fsivop_write)
		 	+ (float)(nx.rfs_in.fsivop_lookup - ox.rfs_in.fsivop_lookup)
		 	+ (float)(nx.rfs_in.fsivop_create - ox.rfs_in.fsivop_create)
		 	+ (float)(nx.rfs_in.fsivop_readdir - ox.rfs_in.fsivop_readdir))
					/tdiff * HZ;

		Osyscall = ((float)(nx.rfs_out.fsivop_open - ox.rfs_out.fsivop_open)
			+ (float)(nx.rfs_out.fsivop_close - ox.rfs_out.fsivop_close)
			+ (float)(nx.rfs_out.fsivop_read - ox.rfs_out.fsivop_read)
			+ (float)(nx.rfs_out.fsivop_write - ox.rfs_out.fsivop_write)
		 	+ (float)(nx.rfs_out.fsivop_lookup - ox.rfs_out.fsivop_lookup)
		 	+ (float)(nx.rfs_out.fsivop_create - ox.rfs_out.fsivop_create)
		 	+ (float)(nx.rfs_out.fsivop_readdir - ox.rfs_out.fsivop_readdir))
					/tdiff * HZ;

		Lsyscall = (float)(nx.si.syscall - ox.si.syscall )/tdiff * HZ -
				(Osyscall);

		Isysread = (float)(nx.rfs_in.fsivop_read - ox.rfs_in.fsivop_read)
					/tdiff * HZ;
		Osysread = (float)(nx.rfs_out.fsivop_read - ox.rfs_out.fsivop_read)
					/tdiff * HZ;
		Lsysread = (float)(nx.si.sysread - ox.si.sysread )/tdiff * HZ -
				(Osysread);

		Isyswrite = (float)(nx.rfs_in.fsivop_write - ox.rfs_in.fsivop_write)
					/tdiff * HZ;
		Osyswrite = (float)(nx.rfs_out.fsivop_write - ox.rfs_out.fsivop_write)
					/tdiff * HZ;
		Lsyswrite = (float)(nx.si.syswrite - ox.si.syswrite )/tdiff * HZ -
				(Osyswrite);

		Isysexec = 0;
		Osysexec = 0;
		Lsysexec = (float)(nx.si.sysexec - ox.si.sysexec )/tdiff * HZ;

		Ireadch = (float)(nx.rfs_in.fsireadch - ox.rfs_in.fsireadch)
					/tdiff * HZ;
		Oreadch = (float)(nx.rfs_out.fsireadch - ox.rfs_out.fsireadch)
					/tdiff * HZ;
		Lreadch = (float)(nx.si.readch - ox.si.readch )/tdiff * HZ -
				(Oreadch);

		Iwritech = (float)(nx.rfs_in.fsiwritech - ox.rfs_in.fsiwritech)
					/tdiff * HZ;
		Owritech = (float)(nx.rfs_out.fsiwritech - ox.rfs_out.fsiwritech)
					/tdiff * HZ;
		Lwritech = (float)(nx.si.writech - ox.si.writech )/tdiff * HZ -
				(Owritech);

		printf("\n%-8s %7.0f %7.0f %7.0f %7s %7.2f %7.0f %7.0f\n",
			"   in", Isyscall,Isysread,Isyswrite,"",Isysexec,
			Ireadch,Iwritech);
		printf("%-8s %7.0f %7.0f %7.0f %7s %7.2f %7.0f %7.0f\n",
			"   out",Osyscall,Osysread,Osyswrite,"",Osysexec,
			Oreadch,Owritech);
		printf("%-8s %7.0f %7.0f %7.0f %7.2f %7.2f %7.0f %7.0f\n",
			"   local",
			Lsyscall >= 0.0 ? Lsyscall : 0.0,
			Lsysread >= 0.0 ? Lsysread : 0.0,
			Lsyswrite >= 0.0 ? Lsyswrite : 0.0,
			(float)(nx.si.sysfork - ox.si.sysfork)/tdiff * HZ,
			Lsysexec,
			Lreadch >= 0.0 ? Lreadch : 0.0,
			Lwritech >= 0.0 ? Lwritech : 0.0);
 
		ax.rfs_in.fsivop_open += nx.rfs_in.fsivop_open 
					- ox.rfs_in.fsivop_open;
		ax.rfs_in.fsivop_close += nx.rfs_in.fsivop_close 
					- ox.rfs_in.fsivop_close;
		ax.rfs_in.fsivop_lookup += nx.rfs_in.fsivop_lookup 
					- ox.rfs_in.fsivop_lookup;
		ax.rfs_in.fsivop_create += nx.rfs_in.fsivop_create 
					- ox.rfs_in.fsivop_create;
		ax.rfs_in.fsivop_readdir += nx.rfs_in.fsivop_readdir 
					- ox.rfs_in.fsivop_readdir;

		ax.rfs_out.fsivop_open += nx.rfs_out.fsivop_open 
					- ox.rfs_out.fsivop_open;
		ax.rfs_out.fsivop_close += nx.rfs_out.fsivop_close 
					- ox.rfs_out.fsivop_close;
		ax.rfs_out.fsivop_lookup += nx.rfs_out.fsivop_lookup 
					- ox.rfs_out.fsivop_lookup;
		ax.rfs_out.fsivop_create += nx.rfs_out.fsivop_create 
					- ox.rfs_out.fsivop_create;
		ax.rfs_out.fsivop_readdir += nx.rfs_out.fsivop_readdir 
					- ox.rfs_out.fsivop_readdir;
		ax.si.syscall += nx.si.syscall - ox.si.syscall;
 
		ax.rfs_in.fsivop_read += nx.rfs_in.fsivop_read 
					- ox.rfs_in.fsivop_read;
		ax.rfs_out.fsivop_read += nx.rfs_out.fsivop_read 
					- ox.rfs_out.fsivop_read;
		ax.si.sysread += nx.si.sysread - ox.si.sysread;
 
		ax.rfs_in.fsivop_write += nx.rfs_in.fsivop_write 
					- ox.rfs_in.fsivop_write;
		ax.rfs_out.fsivop_write += nx.rfs_out.fsivop_write 
					- ox.rfs_out.fsivop_write;
		ax.si.syswrite += nx.si.syswrite - ox.si.syswrite;
 
		ax.si.sysexec += nx.si.sysexec - ox.si.sysexec;
 
		ax.rfs_in.fsireadch += nx.rfs_in.fsireadch - ox.rfs_in.fsireadch;
		ax.rfs_out.fsireadch += nx.rfs_out.fsireadch - ox.rfs_out.fsireadch;
		ax.si.readch += nx.si.readch - ox.si.readch;
	 
		ax.rfs_in.fsiwritech += nx.rfs_in.fsiwritech - ox.rfs_in.fsiwritech;
		ax.rfs_out.fsiwritech += nx.rfs_out.fsiwritech - ox.rfs_out.fsiwritech;
		ax.si.writech += nx.si.writech - ox.si.writech;

		ax.si.sysfork += nx.si.sysfork - ox.si.sysfork;

		break;
	}
		printf(" %7.0f %7.0f %7.0f %7.2f %7.2f %7.0f %7.0f\n",
			(float)(nx.si.syscall - ox.si.syscall)/tdiff *HZ,
			(float)(nx.si.sysread - ox.si.sysread)/tdiff *HZ,
			(float)(nx.si.syswrite - ox.si.syswrite)/tdiff *HZ,
			(float)(nx.si.sysfork - ox.si.sysfork)/tdiff *HZ,
			(float)(nx.si.sysexec - ox.si.sysexec)/tdiff *HZ,
			(float)(nx.si.readch - ox.si.readch)/tdiff * HZ,
			(float)(nx.si.writech - ox.si.writech)/tdiff * HZ);

		ax.si.syscall += nx.si.syscall - ox.si.syscall;
		ax.si.sysread += nx.si.sysread - ox.si.sysread;
		ax.si.syswrite += nx.si.syswrite - ox.si.syswrite;
		ax.si.sysfork += nx.si.sysfork - ox.si.sysfork;
		ax.si.sysexec += nx.si.sysexec - ox.si.sysexec;
		ax.si.readch += nx.si.readch - ox.si.readch;
		ax.si.writech += nx.si.writech - ox.si.writech;
		break;
	case 'w':
		tsttab();

#ifndef u370
		printf(" %7.2f %7.1f %7.2f %7.1f %7.0f\n",
			(float)(nx.vmi.v_swpin - ox.vmi.v_swpin)/tdiff * HZ,
			(float)(nx.vmi.v_pswpin - ox.vmi.v_pswpin)/tdiff * HZ,
			(float)(nx.vmi.v_swpout - ox.vmi.v_swpout)/tdiff * HZ,
			(float)(nx.vmi.v_pswpout - ox.vmi.v_pswpout)/tdiff * HZ,
			(float)(nx.si.pswitch - ox.si.pswitch)/tdiff * HZ);

		ax.vmi.v_swpin += nx.vmi.v_swpin - ox.vmi.v_swpin;
		ax.vmi.v_pswpin += nx.vmi.v_pswpin - ox.vmi.v_pswpin;
		ax.vmi.v_swpout += nx.vmi.v_swpout - ox.vmi.v_swpout;
		ax.vmi.v_pswpout += nx.vmi.v_pswpout - ox.vmi.v_pswpout;
		ax.si.pswitch += nx.si.pswitch - ox.si.pswitch;
#endif

#ifdef u370
		printf(" %7.0f %7.0f %7.0f\n",
			(float)(nx.intsched - ox.intsched)/(nx.ccv - ox.ccv)*1e6,
			(float)(nx.tsend - ox.tsend)/(nx.ccv - ox.ccv)*1e6,
			(float)(nx.mkdisp - ox.mkdisp)/(nx.ccv - ox.ccv)*1e6);
		ax.intsched += nx.intsched - ox.intsched;
		ax.tsend += nx.tsend - ox.tsend;
		ax.mkdisp += nx.mkdisp - ox.mkdisp;
#endif

		break;
	case 'a':
		tsttab();
		printf(" %7.0f %7.0f %7.0f\n",
			(float)(nx.si.iget - ox.si.iget)/tdiff * HZ,
			(float)(nx.si.namei - ox.si.namei)/tdiff * HZ,
			(float)(nx.si.dirblk - ox.si.dirblk)/tdiff * HZ);

		ax.si.iget += nx.si.iget - ox.si.iget;
		ax.si.namei += nx.si.namei - ox.si.namei;
		ax.si.dirblk += nx.si.dirblk - ox.si.dirblk;
		break;

#if u3b || u3b15 || u3b2 || i386
	case 'S':
		tsttab();
		printf("%10.0f %12.1f %9.0f %9.1f %10.0f\n",
			(float)(nx.rf_srv.rfsi_nservers - ox.rf_srv.rfsi_nservers)
				/sec_diff,

			(float)((nx.rf_srv.rfsi_rcv_occ - ox.rf_srv.rfsi_rcv_occ)
					/sec_diff * 100.0) <= 100.0 ?
				(nx.rf_srv.rfsi_rcv_occ - ox.rf_srv.rfsi_rcv_occ)
					/sec_diff *100.0 :
				100.0,

			((nx.rf_srv.rfsi_rcv_occ - ox.rf_srv.rfsi_rcv_occ <= 0) ? 0.0 :
				(float)(nx.rf_srv.rfsi_rcv_que - ox.rf_srv.rfsi_rcv_que)/
					(float)(nx.rf_srv.rfsi_rcv_occ - ox.rf_srv.rfsi_rcv_occ)),

			(float)((nx.rf_srv.rfsi_srv_occ - ox.rf_srv.rfsi_srv_occ)
					/sec_diff * 100.0) <= 100.0 ?
				(nx.rf_srv.rfsi_srv_occ - ox.rf_srv.rfsi_srv_occ)
					/sec_diff * 100.0 :
				100.0,

			(nx.rf_srv.rfsi_srv_occ - ox.rf_srv.rfsi_srv_occ == 0) ? 0.0 :
				(float)(nx.rf_srv.rfsi_srv_que - ox.rf_srv.rfsi_srv_que)/
					(float)(nx.rf_srv.rfsi_srv_occ - ox.rf_srv.rfsi_srv_occ));
 


		ax.rf_srv.rfsi_nservers += nx.rf_srv.rfsi_nservers - ox.rf_srv.rfsi_nservers;
		ax.rf_srv.rfsi_rcv_occ += nx.rf_srv.rfsi_rcv_occ - ox.rf_srv.rfsi_rcv_occ;
		ax.rf_srv.rfsi_rcv_que += nx.rf_srv.rfsi_rcv_que - ox.rf_srv.rfsi_rcv_que;
		ax.rf_srv.rfsi_srv_occ += nx.rf_srv.rfsi_srv_occ - ox.rf_srv.rfsi_srv_occ;
		ax.rf_srv.rfsi_srv_que += nx.rf_srv.rfsi_srv_que - ox.rf_srv.rfsi_srv_que;
		break;
	case 'C':
		tsttab();
		printf("%11.1f %11.1f %11.1f %11.1f %11.1f %11.1f\n",
			(float)(nx.rfc.rfci_snd_dis - ox.rfc.rfci_snd_dis)/tdiff * HZ,
			(float)(nx.rfc.rfci_snd_msg - ox.rfc.rfci_snd_msg)/tdiff * HZ,
			(float)(nx.rfc.rfci_rcv_dis - ox.rfc.rfci_rcv_dis)/tdiff * HZ,
			(float)(nx.rfc.rfci_rcv_msg - ox.rfc.rfci_rcv_msg)/tdiff * HZ,
			(float)(nx.rfc.rfci_dis_data - ox.rfc.rfci_dis_data)/tdiff * HZ,
			(float)(nx.rfc.rfci_pabort - ox.rfc.rfci_pabort)/tdiff * HZ);

		ax.rfc.rfci_snd_dis += nx.rfc.rfci_snd_dis - ox.rfc.rfci_snd_dis;
		ax.rfc.rfci_snd_msg += nx.rfc.rfci_snd_msg - ox.rfc.rfci_snd_msg;
		ax.rfc.rfci_rcv_dis += nx.rfc.rfci_rcv_dis - ox.rfc.rfci_rcv_dis;
		ax.rfc.rfci_rcv_msg += nx.rfc.rfci_rcv_msg - ox.rfc.rfci_rcv_msg;
		ax.rfc.rfci_dis_data += nx.rfc.rfci_dis_data - ox.rfc.rfci_dis_data;
		ax.rfc.rfci_pabort += nx.rfc.rfci_pabort - ox.rfc.rfci_pabort;
		break;
#endif
	
	case 'q':

#ifndef u370
		tsttab();
		if ((nx.si.runocc - ox.si.runocc) == 0)
			printf(" %7s %7s", "  ", "  ");
		else {
			printf(" %7.1f %7.0f",
				(float)(nx.si.runque -ox.si.runque)/
				(float)(nx.si.runocc - ox.si.runocc),
				(float)(nx.si.runocc -ox.si.runocc)/sec_diff * 100.0);
			ax.si.runque += nx.si.runque - ox.si.runque;
			ax.si.runocc += nx.si.runocc - ox.si.runocc;
		}
		if ((nx.si.swpocc - ox.si.swpocc) == 0)
			printf(" %7s %7s\n","  ","  ");
		else {
			printf(" %7.1f %7.0f\n",
				(float)(nx.si.swpque -ox.si.swpque)/
				(float)(nx.si.swpocc - ox.si.swpocc),
				(float)(nx.si.swpocc -ox.si.swpocc)/sec_diff *100.0);
			ax.si.swpque += nx.si.swpque - ox.si.swpque;
			ax.si.swpocc += nx.si.swpocc - ox.si.swpocc;

		}
#endif

#ifdef u370
		printf(" %7.0f %7.0f %7.0f %7.0f %7.0f\n",
			(float)nx.pi.run,
			(float)nx.pi.wtsem,
			(float)(nx.pi.wtsemtm/((nx.pi.wtsem)*HZ)),
			(float)nx.pi.wtio,
			(float)(nx.pi.wtiotm/((nx.pi.wtio)*HZ)));

		ax.pi.run += nx.pi.run;
		ax.pi.wtsem += nx.pi.wtsem;
		ax.pi.wtsemtm += nx.pi.wtsemtm;
		ax.pi.wtio += nx.pi.wtio;
		ax.pi.wtiotm += nx.pi.wtiotm;
#endif

		break;
	case 'm':
		tsttab();
		printf(" %7.2f %7.2f\n",
			(float)(nx.si.msg - ox.si.msg)/tdiff * HZ,
			(float)(nx.si.sema - ox.si.sema)/tdiff * HZ);

		ax.si.msg += nx.si.msg - ox.si.msg;
		ax.si.sema += nx.si.sema - ox.si.sema;
		break;

#if	vax || u3b
	case 'r':
		tsttab();
		printf(" %7.0f %7.0f\n",
			(float)nx.mi.freemem,
			(float)nx.mi.freeswap);
		ax.mi.freemem += nx.mi.freemem;
		ax.mi.freeswap += nx.mi.freeswap;
		break;
#endif

#if	u3b2 || u3b15 || i386

	case 'k':
		tsttab();
		printf(" %7.0f %7.0f %5.0f %7.0f %7.0f %5.0f %11.0f %5.0f\n",
			(float)nx.km.km_mem[KMEM_SMALL],
			(float)nx.km.km_alloc[KMEM_SMALL],
			(float)nx.km.km_fail[KMEM_SMALL],
			(float)nx.km.km_mem[KMEM_LARGE],
			(float)nx.km.km_alloc[KMEM_LARGE],
			(float)nx.km.km_fail[KMEM_LARGE],
			(float)nx.km.km_alloc[KMEM_OSIZE],
			(float)nx.km.km_fail[KMEM_OSIZE]);

		ax.km.km_mem[KMEM_SMALL] += nx.km.km_mem[KMEM_SMALL];
		ax.km.km_alloc[KMEM_SMALL] += nx.km.km_alloc[KMEM_SMALL];
		ax.km.km_fail[KMEM_SMALL] += nx.km.km_fail[KMEM_SMALL];
		ax.km.km_mem[KMEM_LARGE] += nx.km.km_mem[KMEM_LARGE];
		ax.km.km_alloc[KMEM_LARGE] += nx.km.km_alloc[KMEM_LARGE];
		ax.km.km_fail[KMEM_LARGE] += nx.km.km_fail[KMEM_LARGE];
		ax.km.km_alloc[KMEM_OSIZE] += nx.km.km_alloc[KMEM_OSIZE];
		ax.km.km_fail[KMEM_OSIZE] += nx.km.km_fail[KMEM_OSIZE];
		break;
	case 'x':
		tsttab();
		printf("\n%-8s %6.2f %8.2f %8.2f %9.2f %9.2f %9.2f %7.2f\n",
			"  in",
			(float)(nx.rfs_in.fsivop_open - ox.rfs_in.fsivop_open)
					/tdiff * HZ,
			(float)(nx.rfs_in.fsivop_create - ox.rfs_in.fsivop_create)
					/tdiff * HZ,
			(float)(nx.rfs_in.fsivop_lookup - ox.rfs_in.fsivop_lookup)
					/tdiff * HZ,
			(float)(nx.rfs_in.fsivop_readdir - ox.rfs_in.fsivop_readdir)
					/tdiff * HZ,
			(float)(nx.rfs_in.fsivop_getpage - ox.rfs_in.fsivop_getpage)
					/tdiff * HZ,
			(float)(nx.rfs_in.fsivop_putpage - ox.rfs_in.fsivop_putpage)
					/tdiff * HZ,
			(float)(nx.rfs_in.fsivop_other - ox.rfs_in.fsivop_other)
					/tdiff * HZ);
		
		printf("%-8s %6.2f %8.2f %8.2f %9.2f %9.2f %9.2f %7.2f\n",
			"  out",
			(float)(nx.rfs_out.fsivop_open - ox.rfs_out.fsivop_open)
					/tdiff * HZ,
			(float)(nx.rfs_out.fsivop_create - ox.rfs_out.fsivop_create)
					/tdiff * HZ,
			(float)(nx.rfs_out.fsivop_lookup - ox.rfs_out.fsivop_lookup)
					/tdiff * HZ,
			(float)(nx.rfs_out.fsivop_readdir - ox.rfs_out.fsivop_readdir)
					/tdiff * HZ,
			(float)(nx.rfs_out.fsivop_getpage - ox.rfs_out.fsivop_getpage)
					/tdiff * HZ,
			(float)(nx.rfs_out.fsivop_putpage - ox.rfs_out.fsivop_putpage)
					/tdiff * HZ,
			(float)(nx.rfs_out.fsivop_other - ox.rfs_out.fsivop_other)
					/tdiff * HZ);

		ax.rfs_in.fsivop_getpage += nx.rfs_in.fsivop_getpage 
					  - ox.rfs_in.fsivop_getpage;
		ax.rfs_out.fsivop_getpage += nx.rfs_out.fsivop_getpage 
					  - ox.rfs_out.fsivop_getpage;
		ax.rfs_in.fsivop_putpage += nx.rfs_in.fsivop_putpage 
					  - ox.rfs_in.fsivop_putpage;
		ax.rfs_out.fsivop_putpage += nx.rfs_out.fsivop_putpage 
					  - ox.rfs_out.fsivop_putpage;
		ax.rfs_in.fsivop_other += nx.rfs_in.fsivop_other 
					  - ox.rfs_in.fsivop_other;
		ax.rfs_out.fsivop_other += nx.rfs_out.fsivop_other 
					  - ox.rfs_out.fsivop_other;
		a_rfs_in_open += nx.rfs_in.fsivop_open 
				- ox.rfs_in.fsivop_open;
		a_rfs_in_lookup += nx.rfs_in.fsivop_lookup 
				- ox.rfs_in.fsivop_lookup;
		a_rfs_in_create += nx.rfs_in.fsivop_create 
				- ox.rfs_in.fsivop_create;
		a_rfs_in_readdir += nx.rfs_in.fsivop_readdir 
				- ox.rfs_in.fsivop_readdir;
	
		a_rfs_out_open += nx.rfs_out.fsivop_open 
				- ox.rfs_out.fsivop_open;
		a_rfs_out_lookup += nx.rfs_out.fsivop_lookup 
				- ox.rfs_out.fsivop_lookup;
		a_rfs_out_create += nx.rfs_out.fsivop_create 
				- ox.rfs_out.fsivop_create;
		a_rfs_out_readdir += nx.rfs_out.fsivop_readdir 
				- ox.rfs_out.fsivop_readdir;
	break;

#endif
/* new freemem for 3b2 */


#if     u3b2 || u3b5 || i386

 	{
	unsigned long k0, k1, x;
	case 'r':
		tsttab();
		k1 = (nx.mi.freemem[1] - ox.mi.freemem[1]);
		if (nx.mi.freemem[0] >= ox.mi.freemem[0]) {
			k0 = nx.mi.freemem[0] - ox.mi.freemem[0]; 
		}
		else
		{ 	k0 = 1 + (~(ox.mi.freemem[0] - nx.mi.freemem[0]));
			k1--; 
		}
		printf(" %7.0f %7.0f\n",
			(double)(k0 + magic * (double)k1)/tdiff,
			(float)nx.mi.freeswap);

			x = ax.mi.freemem[0];
			ax.mi.freemem[0] += k0;
			ax.mi.freemem[1] += k1;
			if ( x > ax.mi.freemem[0])
				ax.mi.freemem[1]++;
			ax.mi.freeswap += nx.mi.freeswap;
		break;
 	}
#endif


	 
#if     vax || u3b || u3b5 || u3b2 || i386
	case 'p':
		tsttab();
		printf(" %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f\n",
			(float)((nx.vmi.v_xsfrec - ox.vmi.v_xsfrec) 
				+ (nx.vmi.v_xifrec - ox.vmi.v_xifrec))
				/ tdiff * HZ,

			(float)((nx.vmi.v_pgin - ox.vmi.v_pgin) / tdiff * HZ),

			(float)((nx.vmi.v_pgpgin - ox.vmi.v_pgpgin) 
				/ tdiff * HZ),
	
			(float)((nx.vmi.v_pfault - ox.vmi.v_pfault) 
				/ tdiff * HZ),

			(float)((nx.vmi.v_vfault - ox.vmi.v_vfault) 
				/ tdiff * HZ),

			(float)((nx.vmi.v_sftlock - ox.vmi.v_sftlock) 
				/ tdiff * HZ));
		
			ax.vmi.v_xsfrec += nx.vmi.v_xsfrec - ox.vmi.v_xsfrec;
			ax.vmi.v_xifrec += nx.vmi.v_xifrec - ox.vmi.v_xifrec;
			ax.vmi.v_pgin += nx.vmi.v_pgin - ox.vmi.v_pgin;
			ax.vmi.v_pgpgin += nx.vmi.v_pgpgin - ox.vmi.v_pgpgin;
			ax.vmi.v_pfault += nx.vmi.v_pfault - ox.vmi.v_pfault;
			ax.vmi.v_vfault += nx.vmi.v_vfault - ox.vmi.v_vfault;
			ax.vmi.v_sftlock += nx.vmi.v_sftlock - ox.vmi.v_sftlock;
		break;

	case 'g':
		{
		long ipagediff, inopagediff;
		tsttab();
		ipagediff = nx.si.s5ipage - ox.si.s5ipage;
		inopagediff = nx.si.s5inopage - ox.si.s5inopage;
		printf(" %8.2f %8.2f %8.2f %8.2f %6.2f\n",
			(float)((nx.vmi.v_pgout - ox.vmi.v_pgout) / tdiff * HZ),

			(float)((nx.vmi.v_pgpgout - ox.vmi.v_pgpgout) 
				/ tdiff * HZ),
			(float)((nx.vmi.v_dfree - ox.vmi.v_dfree) 
				/ tdiff * HZ),
			(float)((nx.vmi.v_scan - ox.vmi.v_scan) 
				/ tdiff * HZ),
			((ipagediff + inopagediff) <= 0) ? 0 :
			    ((double)ipagediff/
				(double)(ipagediff + inopagediff)) * 100);
	
			ax.vmi.v_pgout += nx.vmi.v_pgout - ox.vmi.v_pgout;
			ax.vmi.v_pgpgout += nx.vmi.v_pgpgout - ox.vmi.v_pgpgout;
			ax.vmi.v_dfree += nx.vmi.v_dfree - ox.vmi.v_dfree;
			ax.vmi.v_scan += nx.vmi.v_scan - ox.vmi.v_scan;
			ax.si.s5ipage += ipagediff;
			ax.si.s5inopage += inopagediff;
		}
		break;

#endif
	}
	if (jj > 2) printf("\n");
}


/**********************************************************/
/*      print average routine   			  */
/**********************************************************/
prtavg()
{
	register int ii,kk;
	int	jj=0;
	char	ccc;

#ifndef u370
	tdiff = ax.si.cpu[0] + ax.si.cpu[1] + ax.si.cpu[2] + ax.si.cpu[3];
	if (nx.apstate)
		tdiff = tdiff/2;
#endif

#ifdef u370
	tdiff = ax.elpstm;
#endif

	if (tdiff <= 0.0)
		return;
	printf("\n");

	while((ccc = fopt[jj++]) != NULL)
	switch(ccc){
	case 'u':
#ifndef u370
		if (dflg) {
			if (nx.apstate) {
				if ( tdiff != 0) { 
					printf("Average  %7.0f %7.0f %7.0f %7.0f %7.0f\n",
					(float)ax.si.cpu[1]/(2*tdiff) * 100.0,
					(float)(ax.si.cpu[2] - ax.rf_srv.rfsi_serve)/(2*tdiff) * 100.0,
					(float)(ax.rf_srv.rfsi_serve)/(2*tdiff) * 100.0,
					(float)ax.si.cpu[3]/(2*tdiff) * 100.0,
					(float)ax.si.cpu[0]/(2*tdiff) * 100.0);
				}
				else
					printf("Average %7.0f %7.0f %7.0f %7.0f %7.0f\n","       ","       ","       ","       ","       ");
			}else{
				if ( tdiff != 0) { 
				printf("Average  %7.0f %7.0f %7.0f %7.0f %7.0f\n",
				(float)ax.si.cpu[1]/tdiff * 100.0,
				(float)(ax.si.cpu[2] - ax.rf_srv.rfsi_serve)/tdiff * 100.0,
				(float)(ax.rf_srv.rfsi_serve)/tdiff * 100.0,
				(float)ax.si.cpu[3]/tdiff * 100.0,
				(float)ax.si.cpu[0]/tdiff * 100.0);
				}
				else
					printf("Average %7.0f %7.0f %7.0f %7.0f %7.0f\n","       ","       ","       ","       ","       ");
			}
		break;
		}
		if (nx.apstate) {
			if ( tdiff != 0) { 
			printf("Average  %7.0f %7.0f %7.0f %7.0f\n",
			(float)ax.si.cpu[1]/(2*tdiff) * 100.0,
			(float)ax.si.cpu[2]/(2*tdiff) * 100.0,
			(float)ax.si.cpu[3]/(2*tdiff) * 100.0,
			(float)ax.si.cpu[0]/(2*tdiff) * 100.0);
			}
			else
				printf("Average %7.0f %7.0f %7.0f %7.0f %7.0f\n","       ","       ","       ","       ","       ");
		}else {
			if ( tdiff != 0) { 
			printf("Average  %7.0f %7.0f %7.0f %7.0f\n",
			(float)ax.si.cpu[1]/tdiff * 100.0,
			(float)ax.si.cpu[2]/tdiff * 100.0,
			(float)ax.si.cpu[3]/tdiff * 100.0,
			(float)ax.si.cpu[0]/tdiff * 100.0);
			}
			else
				printf("Average %7.0f %7.0f %7.0f %7.0f %7.0f\n","       ","       ","       ","       ","       ");
		}
#endif

#ifdef u370
		if (ax.tmelps == 0 || nx.nap == 0) 
			printf("Average  %7.0f %7.0f %7.0f %7.0f\n", 
			"       ", "      ", "       ","       ");
		else {
			printf("Average  %7.0f %7.0f %7.0f %7.0f\n",
			(float) (ax.usrtm)/(ax.tmelps * (double)nx.nap) * 100.0,
			(float) (ax.usuptm)/(ax.tmelps * (double)ax.nap) * 100.0,
			(float) ((ax.tmelps)  * (double) ax.nap - ax.idletm - ax.vmtm )/
				((ax.tmelps)  * (double) ax.nap) * 100.0,
			(float) (ax.idletm)/(ax.tmelps * (double)nx.nap) * 100.0);
			}
#endif
		break;
	case 'y':

#ifndef u370
		if ( tdiff != 0) { 
		printf("Average  %7.0f %7.0f %7.0f %7.0f %7.0f %7.0f\n",
			(float)ax.si.rawch/tdiff *HZ,
			(float)ax.si.canch/tdiff *HZ,
			(float)ax.si.outch/tdiff *HZ,
			(float)ax.si.rcvint/tdiff *HZ,
			(float)ax.si.xmtint/tdiff *HZ,
			(float)ax.si.mdmint/tdiff *HZ);
		}else
			printf("Average  %7.0f %7.0f %7.0f %7.0f %7.0f %7.0f\n","       ","       ","       ","       ","       ","       ");
#endif

#ifdef u370
		if ( tdiff != 0) { 
		printf("Average  %7.0f %7.0f %7.0f %7.0f\n\n",
			(float)ax.si.termin/tdiff * HZ,
			(float)ax.si.s1in/tdiff * HZ,
			(float)ax.si.termout/tdiff * HZ,
			(float)ax.si.s1out/tdiff * HZ);
		}else
			printf("Average  %7.0f %7.0f %7.0f %7.0f %7.0f %7.0f\n","       ","       ","       ","       ","       ","       ");
#endif

		break;
	case 'b':
		if (dflg) {
			printf("Average\n   local  %4.0f %7.0f %7.0f %7.0f %7.0f %7.0f %7.0f %7.0f\n",
				(float)ax.si.bread/tdiff *HZ,
				(float)ax.si.lread/tdiff *HZ,
				((ax.si.lread == 0) ? 100 :
					((float)(ax.si.lread - ax.si.bread)/
					(float)(ax.si.lread) * 100.0)),
				(float)ax.si.bwrite/tdiff *HZ,
				(float)ax.si.lwrite/tdiff *HZ,
				((ax.si.lwrite == 0) ? 100.0 :
					((float)(ax.si.lwrite - ax.si.bwrite)/
					(float)(ax.si.lwrite) * 100.0)),
				(float)ax.si.phread/tdiff *HZ,
				(float)ax.si.phwrite/tdiff *HZ);
			printf("   remote %4.0f %7.0f %7.0f %7.0f %7.0f %7.0f\n",
				(float)(ax.rfc.rfci_pmread/tdiff *HZ) * BLKPERPG,
				(float)(ax.rfc.rfci_ptread/tdiff *HZ) * BLKPERPG,
				(float)(ax.rfc.rfci_ptread - ax.rfc.rfci_pmread)/
				(float)(ax.rfc.rfci_ptread?(ax.rfc.rfci_ptread):1) * 100.0,
				(float)(ax.rfc.rfci_pmwrite/tdiff *HZ) * BLKPERPG,
				(float)(ax.rfc.rfci_ptwrite/tdiff *HZ) * BLKPERPG,
				(float)(ax.rfc.rfci_ptwrite - ax.rfc.rfci_pmwrite)/
					(float)(ax.rfc.rfci_ptwrite?(ax.rfc.rfci_ptwrite):
					1) * 100.0);
				break;
		}
		printf("Average  %7.0f %7.0f %7.0f %7.0f %7.0f %7.0f %7.0f %7.0f\n",
			(float)ax.si.bread/tdiff *HZ,
			(float)ax.si.lread/tdiff *HZ,
			((ax.si.lread == 0) ? 100.0 :
				((float)(ax.si.lread - ax.si.bread)/
				(float)(ax.si.lread) * 100.0)),
			(float)ax.si.bwrite/tdiff *HZ,
			(float)ax.si.lwrite/tdiff *HZ,
			((ax.si.lwrite == 0) ? 100.0 :
				((float)(ax.si.lwrite - ax.si.bwrite)/
				(float)(ax.si.lwrite) * 100.0)),
			(float)ax.si.phread/tdiff *HZ,
			(float)ax.si.phwrite/tdiff *HZ);
		break;
	case 'd':
		ii = 0;
		printf("Average ");
		tabflg = 1;

#ifndef u370
		for (j=0; j<SINFO; j++) {
                        hz = HZ;
#ifdef i386
            		if ( j == SD01 )
                                hz = 1000;
#endif
		    	for (kk=0; kk<tblmap[j]; kk++){

#ifdef i386
				{ 
#else
				if ((ax.devio[ii][0] > 0) && (ax.devio[ii][2] > 0)){
#endif

				tsttab();

#if u3b
				if (j == DSKINFO)
#endif

#if u3b5
				if (j == DFDFC)
#endif

#if u3b2
				if (j == ID || j == IF)
#endif

#if i386
				if ((j == ID) || (j == SD01)) 
#endif

#ifdef i286
				if (j == WNS)
#endif

					printf(" %4s%-3d",devnm[j],kk);
				else
					printf(" %5s%-2d",devnm[j],kk);
					if (tdiff != 0 && ax.devio[ii][2] != 0) {
					printf(" %7.0f %7.1f %7.0f %7.0f %7.1f %7.1f\n",
					(float)ax.devio[ii][2]/(tdiff*hz/HZ) *  100.0,
					(float)ax.devio[ii][3]/(float)ax.devio[ii][2],
					(float)ax.devio[ii][0]/tdiff *HZ,
					(float)ax.devio[ii][1]/tdiff *HZ,
					(float)(ax.devio[ii][3] - ax.devio[ii][2]) /
						(float)ax.devio[ii][0] /hz * 1000.,
					(float)ax.devio[ii][2] /(float)ax.devio[ii][0] /hz *1000.);
					}else 
					printf(" %7.0f %7.1f %7.0f %7.0f %7.1f %7.1f\n",
						"       ","       ","       ","       ","       ");
			}
			ii++;
		    }
		}
#endif

#ifdef u370
		for (i=0; i<NDEV; i++){ 
			if (ax.io[i].io_total/HZ == 0)
				continue;
		tsttab();
			if (i < NDRUM){ 
				printf(" drm%-4d", i + 1);
			} 
			else { 
				printf(" dsk%-4d", i - NDRUM + 1);
			} 
			if ( tdiff != 0) {
			printf(" %7.1f %7.1f %7.1f %7.1f %7.1f\n", 
				(float)ax.io[i].io_sread/tdiff * HZ, 
				(float)ax.io[i].io_pread/tdiff * HZ, 
				(float)ax.io[i].io_swrite/tdiff * HZ, 
				(float)ax.io[i].io_pwrite/tdiff * HZ, 
				(float)ax.io[i].io_total/tdiff * HZ); 
			} else
			printf(" %7.1f %7.1f %7.1f %7.1f %7.1f\n", 
			"       ","       ","       ","       ","       ");
		} 
#endif 

		break;
	case 'v':
		break;
	case 'c':
		if (dflg) {
			Isyscall = (float)((ax.rfs_in.fsivop_open
					+ ax.rfs_in.fsivop_close 	
					+ ax.rfs_in.fsivop_read 	
					+ ax.rfs_in.fsivop_write 	
					+ ax.rfs_in.fsivop_lookup 	
					+ ax.rfs_in.fsivop_create 	
					+ ax.rfs_in.fsivop_readdir) 	
					/tdiff * HZ);
			Osyscall = (float)((ax.rfs_out.fsivop_open
					+ ax.rfs_out.fsivop_close 	
					+ ax.rfs_out.fsivop_read 	
					+ ax.rfs_out.fsivop_write 	
					+ ax.rfs_out.fsivop_lookup 	
					+ ax.rfs_out.fsivop_create 	
					+ ax.rfs_out.fsivop_readdir) 	
					/tdiff * HZ);
			Lsyscall = (float)(ax.si.syscall/tdiff * HZ) -
					(Osyscall);

			Isysread = (float)(ax.rfs_in.fsivop_read/tdiff * HZ);
			Osysread = (float)(ax.rfs_out.fsivop_read/tdiff * HZ);
			Lsysread = (float)(ax.si.sysread/tdiff * HZ) -
					(Osysread);

			Isyswrite = (float)(ax.rfs_in.fsivop_write/tdiff * HZ);
			Osyswrite = (float)(ax.rfs_in.fsivop_write/tdiff * HZ);
			Lsyswrite = (float)(ax.si.syswrite/tdiff * HZ) -
					(Osyswrite);

			Isysexec = 0;
			Osysexec = 0;
			Lsysexec = (float)(ax.si.sysexec/tdiff * HZ);

			Ireadch = (float)(ax.rfs_in.fsireadch/tdiff * HZ);
			Oreadch = (float)(ax.rfs_out.fsireadch/tdiff * HZ);
			Lreadch = (float)(ax.si.readch/tdiff * HZ) -
					(Oreadch);

			Iwritech = (float)(ax.rfs_in.fsiwritech/tdiff * HZ);
			Owritech = (float)(ax.rfs_out.fsiwritech/tdiff * HZ);
			Lwritech = (float)(ax.si.writech/tdiff * HZ) -
					(Owritech);

			printf("Average\n%-8s %7.0f %7.0f %7.0f %7s %7.2f %7.0f %7.0f\n",
				"   in", Isyscall,Isysread,Isyswrite,"",Isysexec,
				Ireadch,Iwritech);
			printf("%-8s %7.0f %7.0f %7.0f %7s %7.2f %7.0f %7.0f\n",
				"   out",Osyscall,Osysread,Osyswrite,"",Osysexec,
				Oreadch,Owritech);
			printf("%-8s %7.0f %7.0f %7.0f %7.2f %7.2f %7.0f %7.0f\n",
				"   local",
				Lsyscall >= 0.0 ? Lsyscall : 0.0,
				Lsysread >= 0.0 ? Lsysread : 0.0,
				Lsyswrite >= 0.0 ? Lsyswrite : 0.0,
				(float)ax.si.sysfork/tdiff *HZ,
				Lsysexec,
				Lreadch >= 0.0 ? Lreadch : 0.0,
				Lwritech >= 0.0 ? Lwritech : 0.0);
			break;
		}

		printf("Average  %7.0f %7.0f %7.0f %7.2f %7.2f %7.0f %7.0f\n",
			(float)ax.si.syscall/tdiff *HZ,
			(float)ax.si.sysread/tdiff *HZ,
			(float)ax.si.syswrite/tdiff *HZ,
			(float)ax.si.sysfork/tdiff *HZ,
			(float)ax.si.sysexec/tdiff *HZ,
			(float)ax.si.readch/tdiff * HZ,
			(float)ax.si.writech/tdiff * HZ);
		break;
	case 'w':
#ifndef u370
		printf("Average  %7.2f %7.1f %7.2f %7.1f %7.0f\n",
			(float)ax.vmi.v_swpin/tdiff * HZ,
			(float)ax.vmi.v_pswpin /tdiff * HZ,
			(float)ax.vmi.v_swpout/tdiff * HZ,
			(float)ax.vmi.v_pswpout/tdiff * HZ,
			(float)ax.si.pswitch/tdiff * HZ);
#endif
#ifdef u370
		printf("Average  %7.1f %7.1f %7.1f\n",
		(float)ax.intsched/ax.tmelps*1e6,
		(float)ax.tsend/ax.tmelps*1e6,
		(float)ax.mkdisp/ax.tmelps*1e6);
#endif 
		break;
	case 'a':
		printf("Average  %7.0f %7.0f %7.0f\n",
			(float)ax.si.iget/tdiff * HZ,
			(float)ax.si.namei/tdiff * HZ,
			(float)ax.si.dirblk/tdiff * HZ);
		break;

#if u3b2 || u3b15 || u3b2 || i386
	case 'S':
		printf("Average %10.0f %12.1f %9.0f %9.1f %10.0f\n",
			(float)ax.rf_srv.rfsi_nservers / totsec_diff,

			(float)(ax.rf_srv.rfsi_rcv_occ / totsec_diff * 100.0)
				<= 100.0 ?
				ax.rf_srv.rfsi_rcv_occ / totsec_diff * 100.0 :
				100.0,

			(ax.rf_srv.rfsi_rcv_occ == 0) ? 0.0 :
				(float)ax.rf_srv.rfsi_rcv_que / (float)ax.rf_srv.rfsi_rcv_occ,
			(float)(ax.rf_srv.rfsi_srv_occ / totsec_diff * 100.0)
				<= 100.0 ?
				ax.rf_srv.rfsi_srv_occ / totsec_diff * 100.0 :
				100.0,

			(ax.rf_srv.rfsi_srv_occ == 0 ) ? 0.0 :
				(float)ax.rf_srv.rfsi_srv_que / (float)ax.rf_srv.rfsi_srv_occ );
		break;
	case 'C':
		printf("Average %11.1f %11.1f %11.1f %11.1f %11.1f %11.1f\n",
			(float)(ax.rfc.rfci_snd_dis)/tdiff * HZ,
			(float)(ax.rfc.rfci_snd_msg)/tdiff * HZ,
			(float)(ax.rfc.rfci_rcv_dis)/tdiff * HZ,
			(float)(ax.rfc.rfci_rcv_msg)/tdiff * HZ,
			(float)(ax.rfc.rfci_dis_data)/tdiff * HZ,
			(float)(ax.rfc.rfci_pabort)/tdiff * HZ);
		break;
#endif

	case 'q':
#ifndef u370
		if (ax.si.runocc == 0)
			printf("Average  %7s %7s ","  ","  ");
		else {
			printf("Average  %7.1f %7.0f ",
			(float)ax.si.runque /
				(float)ax.si.runocc,
			(float)ax.si.runocc /tdiff *HZ *100.0);
		}
		if (ax.si.swpocc == 0)
			printf("%7s %7s\n","  ","  ");
		else {
			printf("%7.1f %7.0f\n",
			(float)ax.si.swpque/
				(float)ax.si.swpocc,
			(float)ax.si.swpocc/tdiff *HZ *100.0);

		}
#endif
#ifdef u370
		printf("Average  %7.0f %7.0f %7.0f %7.0f %7.0f\n",
				(float)(ax.pi.run/lines),
				(float)(ax.pi.wtsem/lines),
				(float)(ax.pi.wtsemtm/((ax.pi.wtsem)*HZ)),
				(float)(ax.pi.wtio/lines),
				(float)(ax.pi.wtiotm/((ax.pi.wtio)*HZ)));

#endif

		break;
	case 'm':
		printf("Average  %7.2f %7.2f\n",
			(float)ax.si.msg/tdiff * HZ,
			(float)ax.si.sema/tdiff * HZ);
		break;

#if	vax || u3b  
	case 'r':
		printf("Average  %7.0f %7.0f\n",
			(float)(ax.mi.freemem / lines),
			(float)(ax.mi.freeswap) / lines);
		break;
#endif

#if  u3b2 || u3b5 || i386
	case 'r':
		printf("Average  %7.0f",
			(double)(ax.mi.freemem[0] + magic * (double)ax.mi.freemem[1])/tdiff);
		printf("%7.0f\n",(float)(ax.mi.freeswap) / lines);
		break;
#endif

#if     vax || u3b || u3b5 || u3b2 || i386
	case 'k':
		printf("Average  %7.0f %7.0f %5.0f %7.0f %7.0f %5.0f %11.0f %5.0f\n",
			(float)(ax.km.km_mem[KMEM_SMALL] / lines),
			(float)(ax.km.km_alloc[KMEM_SMALL] / lines),
			(float)(ax.km.km_fail[KMEM_SMALL] / lines),
			(float)(ax.km.km_mem[KMEM_LARGE] / lines),
			(float)(ax.km.km_alloc[KMEM_LARGE] / lines),
			(float)(ax.km.km_fail[KMEM_LARGE] / lines),
			(float)(ax.km.km_alloc[KMEM_OSIZE] / lines),
			(float)(ax.km.km_fail[KMEM_OSIZE] / lines));
		break;

	case 'x':
		printf("Average\n%-8s %6.2f %8.2f %8.2f %9.2f %9.2f %9.2f %7.2f\n",
			"  in",
			(float)(a_rfs_in_open / tdiff * HZ),
			(float)(a_rfs_in_create / tdiff * HZ),
			(float)(a_rfs_in_lookup / tdiff * HZ),
			(float)(a_rfs_in_readdir / tdiff * HZ),
			(float)(ax.rfs_in.fsivop_getpage / tdiff * HZ),
			(float)(ax.rfs_in.fsivop_putpage / tdiff * HZ),
			(float)(ax.rfs_in.fsivop_other / tdiff * HZ));

		printf("%-8s %6.2f %8.2f %8.2f %9.2f %9.2f %9.2f %7.2f\n",
			"  out",
			(float)(a_rfs_out_open / tdiff * HZ),
			(float)(a_rfs_out_create / tdiff * HZ),
			(float)(a_rfs_out_lookup / tdiff * HZ),
			(float)(a_rfs_out_readdir / tdiff * HZ),
			(float)(ax.rfs_out.fsivop_getpage / tdiff * HZ),
			(float)(ax.rfs_out.fsivop_putpage / tdiff * HZ),
			(float)(ax.rfs_out.fsivop_other / tdiff * HZ));
		break;
#endif
			
#if	vax || u3b || u3b15 || u3b2 || i386
	case 'p':
		printf("Average  %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f\n",
			(float)((ax.vmi.v_xsfrec + ax.vmi.v_xifrec)
				/ tdiff * HZ),
			(float)(ax.vmi.v_pgin / tdiff * HZ), 
			(float)(ax.vmi.v_pgpgin / tdiff * HZ), 
			(float)(ax.vmi.v_pfault / tdiff * HZ), 
			(float)(ax.vmi.v_vfault / tdiff * HZ), 
			(float)(ax.vmi.v_sftlock / tdiff * HZ));
		break;

	case 'g':
		printf("Average %8.2f %8.2f %8.2f %8.2f %6.2f\n",
			(float)(ax.vmi.v_pgout / tdiff * HZ), 
			(float)(ax.vmi.v_pgpgout / tdiff * HZ), 
			(float)(ax.vmi.v_dfree / tdiff * HZ), 
			(float)(ax.vmi.v_scan / tdiff * HZ), 
			(ax.si.s5ipage + ax.si.s5inopage <= 0) ? 0 :
			    ((double)ax.si.s5ipage /
				(double)(ax.si.s5ipage + ax.si.s5inopage)) * 100);
		break;

#endif
	}
}


/**********************************************************/
/*      error exit routines  				  */
/**********************************************************/
pillarg()
{
	fprintf(stderr,"%s -- illegal argument for option  %c\n",
		optarg,cc);
	exit(1);
}


perrexit()
{
	perror("sar");
	exit(1);
}


pmsgexit(s)
char	*s;
{
	fprintf(stderr, "%s\n", s);
	exit(1);
}
