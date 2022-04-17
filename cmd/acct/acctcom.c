/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)acct:acctcom.c	1.37.2.2"

#include <time.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#include "acctdef.h"
#include <grp.h>
#include <sys/acct.h>
#include <pwd.h>
#include <sys/stat.h>
#include <locale.h>

struct	acct ab;
char	command_name[16];
char	obuf[BUFSIZ];
static char	time_buf[50];

double	cpucut,
	syscut,
	hogcut,
	iocut,
	realtot,
	cputot,
	usertot,
	systot,
	kcoretot,
	iotot,
	rwtot;
extern long	timezone;
extern int	daylight;	/* daylight savings time if set */
long	daydiff,
	offset = -2,
	elapsed,
	sys,
	user,
	cpu,
	mem,
	io,
	rw,
	cmdcount;
time_t	tstrt_b,
	tstrt_a,
	tend_b,
	tend_a,
	etime;
int	backward,
	flag_field,
	average,
	quiet,
	option,
	verbose = 1,
	uidflag,
	gidflag,
	unkid,	/*user doesn't have login on this machine*/
	errflg,
	su_user,
	fileout = 0,
	stdinflg,
	nfiles;
#ifdef uts
dev_t   linedev = 0xffff;  /* changed from -1, as dev_t is now ushort */
#else
dev_t	linedev = (dev_t)-1;
#endif
uid_t	uidval;
gid_t	gidval;
char	*cname = NULL; /* command name pattern to match*/

struct passwd *getpwnam(), *getpwuid(), *pw;
struct group *getgrnam(),*grp;
long	ftell(),
	convtime(),
	time();

#ifdef uts
float   expand();
#else
long	expand();
#endif

char	*ofile,
	*devtolin(),
	*uidtonam();
dev_t	lintodev();
FILE	*ostrm;

main(argc, argv)
char **argv;
{
	register int	c;
	extern int	optind;
	extern char	*optarg;

	(void)setlocale(LC_ALL, "");
	setbuf(stdout,obuf);
	while((c = getopt(argc, argv,
		"C:E:H:I:O:S:abe:fg:hikl:mn:o:qrs:tu:v")) != EOF) {
		switch(c) {
		case 'C':
			sscanf(optarg,"%lf",&cpucut);
			continue;
		case 'O':
			sscanf(optarg,"%lf",&syscut);
			continue;
		case 'H':
			sscanf(optarg,"%lf",&hogcut);
			continue;
		case 'I':
			sscanf(optarg,"%lf",&iocut);
			continue;
		case 'a':
			average++;
			continue;
		case 'b':
			backward++;
			continue;
		case 'g':
			if(sscanf(optarg,"%ld",&gidval) == 1)
				gidflag++;
			else if((grp=getgrnam(optarg)) == NULL)
				fatal("Unknown group", optarg);
			else {
				gidval=grp->gr_gid;
				gidflag++;
			}
			continue;
		case 'h':
			option |= HOGFACTOR;
			continue;
		case 'i':
			option |= IORW;
			continue;
		case 'k':
			option |= KCOREMIN;
			continue;
		case 'm':
			option |= MEANSIZE;
			continue;
		case 'n':
			cname=(char *)cmset(optarg);
			continue;
		case 't':
			option |= SEPTIME;
			continue;
		case 'r':
			option |= CPUFACTOR;
			continue;
		case 'v':
			verbose=0;
			continue;
		case 'l':
			linedev = lintodev(optarg);
			continue;
		case 'u':
			if(*optarg == '?')
				unkid++;
			if(*optarg == '#')
				su_user++;
			if((pw = getpwnam(optarg)) == NULL) {
				uidval = (uid_t)atoi(optarg);
				/* atoi will return 0 in abnormal situation */
				if (uidval == 0 && strcmp(optarg, "0") != 0) {
					fprintf(stderr, "%s: Unknown user %s\n", argv[0], optarg);
					exit(1);
				}
 				if ((pw = getpwuid(uidval)) == NULL) {
					fprintf(stderr, "%s: Unknown user %s\n", argv[0], optarg);
					exit(1);
				}
				uidflag++;
			} else {
				uidval = pw->pw_uid;
				uidflag++;
			}
			continue;
		case 'q':
			quiet++;
			verbose=0;
			average++;
			continue;
		case 's':
			tend_a = convtime(optarg);
			continue;
		case 'S':
			tstrt_a = convtime(optarg);
			continue;
		case 'f':
			flag_field++;
			continue;
		case 'e':
			tstrt_b = convtime(optarg);
			continue;
		case 'E':
			tend_b = convtime(optarg);
			continue;
		case 'o':
			ofile = optarg;
			fileout++;
			if((ostrm = fopen(ofile, "w")) == NULL) {
				perror("open error on output file");
				errflg++;
			}
			continue;
		case '?':
			errflg++;
			continue;
		}
	}

	if(errflg) {
		usage();
		exit(1);
	}


	argv = &argv[optind];
	while(optind++ < argc) {
		dofile(*argv++);    /* change from *argv */
		nfiles++;
	}

	if(nfiles==0) {
		if(isatty(0) || isdevnull())
			dofile(PACCT);
		else {
			stdinflg = 1;
			backward = offset = 0;
			dofile(NULL);
		}
	}
	doexit(0);
}

dofile(fname)
char *fname;
{
	register struct acct *a = &ab;
	struct tm *t;
	long curtime;
	time_t	ts_a = 0,
		ts_b = 0,
		te_a = 0,
		te_b = 0;
	long	daystart;
	long	nsize;
	int	ver;	/* version of acct structure */

	if(fname != NULL)
		if(freopen(fname, "r", stdin) == NULL) {
			fprintf(stderr, "acctcom: cannot open %s\n", fname);
			return;
		}

	if (fread((char *)&ab, sizeof(struct acct), 1, stdin) != 1)
		return;
	else if (ab.ac_flag & AEXPND)
		ver = 2;	/* 4.0 acct structure */
	else 
		ver = 1;	/* 3.x acct structure */

	rewind(stdin);
		

	if(backward) {
		if (ver == 2)
			nsize = sizeof(struct acct);	/* make sure offset is signed */
		else
			nsize = sizeof(struct o_acct);	/* make sure offset is signed */
		fseek(stdin, (long)(-nsize), 2);
	}
	tzset();
	daydiff = a->ac_btime - (a->ac_btime % SECSINDAY);
	daystart = (a->ac_btime-timezone) - ((a->ac_btime-timezone) % SECSINDAY);
	time(&curtime);
	t=localtime(&curtime);
	if(daydiff < (curtime - (curtime % SECSINDAY))) {
		/*
		 * it is older than today
		 */
		cftime(time_buf, DATE_FMT, &a->ac_btime);
		fprintf(stdout, "\nACCOUNTING RECORDS FROM:  %s", time_buf);
	}

	if(tstrt_a) {
		ts_a = tstrt_a + daystart;
		if(daylight && t->tm_isdst != 0)
			ts_a -= 3600;
		cftime(time_buf, DATE_FMT, &ts_a);
		fprintf(stdout, "START AFT: %s", time_buf);
	}
	if(tstrt_b) {
		ts_b = tstrt_b + daystart;
		if(daylight && t->tm_isdst != 0)
			ts_b -= 3600;
		cftime(time_buf, DATE_FMT, &ts_b);
		fprintf(stdout, "START BEF: %s", time_buf);
	}
	if(tend_a) {
		te_a = tend_a + daystart;
		if(daylight && t->tm_isdst != 0)
			te_a -= 3600;
		cftime(time_buf, DATE_FMT, &te_a);
		fprintf(stdout, "END AFTER: %s", time_buf);
	}
	if(tend_b) {
		te_b = tend_b + daystart;
		if(daylight && t->tm_isdst != 0)
			te_b -= 3600;
		cftime(time_buf, DATE_FMT, &te_b);
		fprintf(stdout, "END BEFOR: %s", time_buf);
	}
	if(ts_a) {
		if (te_b && ts_a > te_b) te_b += SECSINDAY;
	}

	while(aread(ver) != 0) {
		elapsed = expand(a->ac_etime);
		etime = a->ac_btime + (long)SECS(elapsed);
		if(ts_a || ts_b || te_a || te_b) {

			if(te_a && (etime < te_a)) {
				if(backward) return;
				else continue;
			}
			if(te_b && (etime > te_b)) {
				if(backward) continue;
				else return;
			}
			if(ts_a && (a->ac_btime < ts_a))
				continue;
			if(ts_b && (a->ac_btime > ts_b))
				continue;
		}
		if(!MYKIND(a->ac_flag))
			continue;
		if(su_user && !SU(a->ac_flag))
			continue;
		sys = expand(a->ac_stime);
		user = expand(a->ac_utime);
		cpu = sys + user;
		if(cpu == 0)
			cpu = 1;
		mem = expand(a->ac_mem);
		strncpy(command_name, a->ac_comm, 8);
		io=expand(a->ac_io);
		rw=expand(a->ac_rw);
		if(cpucut && cpucut >= SECS(cpu))
			continue;
		if(syscut && syscut >= SECS(sys))
			continue;
#ifdef uts
		if(linedev != 0xffff && a->ac_tty != linedev)
			continue;
#else
		if(linedev != (dev_t)-1 && a->ac_tty != linedev)
			continue;
#endif
		if(uidflag && a->ac_uid != uidval)
			continue;
		if(gidflag && a->ac_gid != gidval)
			continue;
		if(cname && !cmatch(a->ac_comm,cname))
			continue;
		if(iocut && iocut > io)
			continue;
		if(unkid && uidtonam(a->ac_uid)[0] != '?')
			continue;
		if(verbose && (fileout == 0)) {
			printhd();
			verbose = 0;
		}
		if(elapsed == 0)
			elapsed++;
		if(hogcut && hogcut >= (double)cpu/(double)elapsed)
			continue;
		if(fileout)
			fwrite(&ab, sizeof(ab), 1, ostrm);
		else
			println();
		if(average) {
			cmdcount++;
			realtot += (double)elapsed;
			usertot += (double)user;
			systot += (double)sys;
			kcoretot += (double)mem;
			iotot += (double)io;
			rwtot += (double)rw;
		};
	}
}

aread(ver)
int ver;
{
	static	 ok = 1;
	struct o_acct oab;
	int ret;

	if (ver != 2) {
		if ((ret = fread((char *)&oab, sizeof(struct o_acct), 1, stdin)) == 1){
			/* copy SVR3 acct struct to SVR4 acct struct */
			ab.ac_flag = oab.ac_flag | AEXPND;
			ab.ac_stat = oab.ac_stat;
			ab.ac_uid = (uid_t) oab.ac_uid;
			ab.ac_gid = (gid_t) oab.ac_gid;
			ab.ac_tty = (dev_t) oab.ac_tty;
			ab.ac_btime = oab.ac_btime;
			ab.ac_utime = oab.ac_utime;
			ab.ac_stime = oab.ac_stime;
			ab.ac_mem = oab.ac_mem;
			ab.ac_io = oab.ac_io;
			ab.ac_rw = oab.ac_rw;
			strcpy(ab.ac_comm, oab.ac_comm);
		}
	} else
		ret = fread((char *)&ab, sizeof(struct acct), 1, stdin);
	

	if(backward) {
		if(ok) {
			if(fseek(stdin,
				(long)(offset*(ver == 2 ? sizeof(struct acct) :
					sizeof(struct o_acct))), 1) != 0) {

					rewind(stdin);	/* get 1st record */
					ok = 0;
			}
		} else
			ret = 0;
	}
	return(ret != 1 ? 0 : 1);
}

printhd()
{
	fprintf(stdout, "COMMAND                          START    END          REAL");
	ps("CPU");
	if(option & SEPTIME)
		ps("(SECS)");
	if(option & IORW){
		ps("CHARS");
		ps("BLOCKS");
	}
	if(option & CPUFACTOR)
		ps("CPU");
	if(option & HOGFACTOR)
		ps("HOG");
	if(!option || (option & MEANSIZE))
		ps("MEAN");
	if(option & KCOREMIN)
		ps("KCORE");
	fprintf(stdout, "\n");
	fprintf(stdout, "NAME       USER     TTYNAME      TIME     TIME       (SECS)");
	if(option & SEPTIME) {
		ps("SYS");
		ps("USER");
	} else
		ps("(SECS)");
	if(option & IORW) {
		ps("TRNSFD");
		ps("READ");
	}
	if(option & CPUFACTOR)
		ps("FACTOR");
	if(option & HOGFACTOR)
		ps("FACTOR");
	if(!option || (option & MEANSIZE))
		ps("SIZE(K)");
	if(option & KCOREMIN)
		ps("MIN");
	if(flag_field)
		fprintf(stdout, "  F STAT");
	fprintf(stdout, "\n");
	fflush(stdout);
}

println()
{

	char name[32];
	register struct acct *a = &ab;

	if(quiet)
		return;
	if(!SU(a->ac_flag))
		strcpy(name,command_name);
	else {
		strcpy(name,"#");
		strcat(name,command_name);
	}
	fprintf(stdout, "%-9.9s", name);
	strcpy(name,uidtonam(a->ac_uid));
	if(*name != '?')
		fprintf(stdout, "  %-9.9s", name);
	else
		fprintf(stdout, "  %-9d",a->ac_uid);
#ifdef uts
	fprintf(stdout, " %-12.12s",a->ac_tty != 0xffff? devtolin(a->ac_tty):"?");
#else
	fprintf(stdout, " %-12.12s",a->ac_tty != (dev_t)-1? devtolin(a->ac_tty):"?");
#endif
	cftime(time_buf, DATE_FMT1, &a->ac_btime);
	fprintf(stdout, "%.9s", time_buf);
	cftime(time_buf, DATE_FMT1, &etime);
	fprintf(stdout, "%.9s ", time_buf);
	pf((double)SECS(elapsed));
	if(option & SEPTIME) {
		pf((double)sys / HZ);
		pf((double)user / HZ);
	} else
		pf((double)cpu / HZ);
	if(option & IORW)
		fprintf(stdout, "%8ld%8ld",io,rw);
	if(option & CPUFACTOR)
		pf((double)user / cpu);
	if(option & HOGFACTOR)
		pf((double)cpu / elapsed);
	if(!option || (option & MEANSIZE))
		pf(KCORE(mem / cpu));
	if(option & KCOREMIN)
		pf(MINT(KCORE(mem)));
	if(flag_field)
		fprintf(stdout, "  %1o %3o", a->ac_flag, a->ac_stat);
	fprintf(stdout, "\n");
}

/*
 * convtime converts time arg to internal value
 * arg has form hr:min:sec, min or sec are assumed to be 0 if omitted
 */
long
convtime(str)
char *str;
{
	long	hr, min, sec;

	min = sec = 0;

	if(sscanf(str, "%ld:%ld:%ld", &hr, &min, &sec) < 1) {
		fatal("acctcom: bad time:", str);
	}
	tzset();
	sec += (min*60);
	sec += (hr*3600);
	return(sec + timezone);
}

cmatch(comm, cstr)
register char	*comm, *cstr;
{

	char	xcomm[9];
	register i;

	for(i=0;i<8;i++){
		if(comm[i]==' '||comm[i]=='\0')
			break;
		xcomm[i] = comm[i];
	}
	xcomm[i] = '\0';

	return(regex(cstr,xcomm));
}

cmset(pattern)
register char	*pattern;
{

	if((pattern=(char *)regcmp(pattern,(char *)0))==NULL){
		fatal("pattern syntax", NULL);
	}

	return((unsigned)pattern);
}

doexit(status)
{
	if(!average)
		exit(status);
	if(cmdcount) {
		fprintf(stdout, "cmds=%ld ",cmdcount);
		fprintf(stdout, "Real=%-6.2f ",SECS(realtot)/cmdcount);
		cputot = systot + usertot;
		fprintf(stdout, "CPU=%-6.2f ",SECS(cputot)/cmdcount);
		fprintf(stdout, "USER=%-6.2f ",SECS(usertot)/cmdcount);
		fprintf(stdout, "SYS=%-6.2f ",SECS(systot)/cmdcount);
		fprintf(stdout, "CHAR=%-8.2f ",iotot/cmdcount);
		fprintf(stdout, "BLK=%-8.2f ",rwtot/cmdcount);
		fprintf(stdout, "USR/TOT=%-4.2f ",usertot/cputot);
		fprintf(stdout, "HOG=%-4.2f ",cputot/realtot);
		fprintf(stdout, "\n");
	}
	else
		fprintf(stdout, "\nNo commands matched\n");
	exit(status);
}
isdevnull()
{
	struct stat	filearg;
	struct stat	devnull;

	if(fstat(0,&filearg) == -1) {
		fprintf(stderr,"acctcom: cannot stat stdin\n");
		return(NULL);
	}
	if(stat("/dev/null",&devnull) == -1) {
		fprintf(stderr,"acctcom: cannot stat /dev/null\n");
		return(NULL);
	}

	if(filearg.st_rdev == devnull.st_rdev) return(1);
	else return(NULL);
}

fatal(s1, s2)
char *s1, *s2;
{
	fprintf(stderr,"acctcom: %s %s\n", s1, s2);
	exit(1);
}

usage()
{
	fprintf(stderr, "Usage: acctcom [options] [files]\n");
	fprintf(stderr, "\nWhere options can be:\n");
	diag("-b	read backwards through file");
	diag("-f	print the fork/exec flag and exit status");
	diag("-h	print hog factor (total-CPU-time/elapsed-time)");
	diag("-i	print I/O counts");
	diag("-k	show total Kcore minutes instead of memory size");
	diag("-m	show mean memory size");
	diag("-r	show CPU factor (user-time/(sys-time + user-time))");
	diag("-t	show separate system and user CPU times");
	diag("-v	don't print column headings");
	diag("-a	print average statistics of selected commands");
	diag("-q	print average statistics only");
	diag("-l line	\tshow processes belonging to terminal /dev/line");
	diag("-u user	\tshow processes belonging to user name or user ID");
	diag("-u #	\tshow processes executed by super-user");
	diag("-u ?	\tshow processes executed by unknown UID's");
	diag("-g group	show processes belonging to group name of group ID");
	diag("-s time	\tshow processes ending after time (hh[:mm[:ss]])");
	diag("-e time	\tshow processes starting before time");
	diag("-S time	\tshow processes starting after time");
	diag("-E time	\tshow processes ending before time");
	diag("-n regex	select commands matching the ed(1) regular expression");
	diag("-o file	\tdo not print, put selected pacct records into file");
	diag("-H factor	show processes that exceed hog factor");
	diag("-O sec	\tshow processes that exceed CPU system time sec");
	diag("-C sec	\tshow processes that exceed total CPU time sec");
	diag("-I chars	show processes that transfer more than char chars");
}
