/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)sa:timex.c	1.22"
/*	timex.c 1.19 of 7/21/89 	*/
#include <sys/types.h>
#include <sys/times.h>
#include <sys/param.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <pwd.h>

time_t	times();
long	atol();
extern	time_t time();
int	isatty();
char	*ttyname();
char	*strcat();
char	*strncpy();
struct	passwd *getpwuid();

extern int errno;
extern char *sys_errlist[];
char	fname[20];


main(argc, argv)
int argc;
char **argv;
{
	struct	tms buffer, obuffer;
	int	status;
	register pid_t	p;
	int	c;
	time_t	before, after;
	char	stime[9], etime[9];
	char	cmd[80];
	extern	char	*optarg;
	extern	int	optind;
	int	pflg = 0, sflg = 0, oflg = 0;
	char	aopt[25];
	FILE	*popen(), *pipin;
	char	ttyid[12], line[150];
	char	eol;
	char	fld[20][12];
	int	iline = 0, i, nfld;
	int	ichar, iblok;
	long	chars = 0, bloks = 0;

	/* check options; */
	while((c = getopt(argc, argv, "sopfhkmrt")) != EOF)
		switch(c)  {
		case 's':  sflg++;  break;
		case 'o':  oflg++;  break;
		case 'p':  pflg++;  break;

		case 'f':  strcat(aopt, "-f ");  break;
		case 'h':  strcat(aopt, "-h ");  break;
		case 'k':  strcat(aopt, "-k ");  break;
		case 'm':  strcat(aopt, "-m ");  break;
		case 'r':  strcat(aopt, "-r ");  break;
		case 't':  strcat(aopt, "-t ");  break;

		case '?':  diag("Usage: timex [-s][-o][-p[-fhkmrt]] cmd");
				break;
		}
	if(optind >= argc)	diag("Missing command");

#if u3b2 || u3b15
	/* check to see if accounting is installed and print a somewhat
	   meaninful message if not  */
	if (((oflg+pflg) != 0) && (access("/usr/bin/acctcom", 01) == -1) ) {
		oflg = 0;
		pflg = 0;
		fprintf(stderr, "Information from -p and -o options not available\n");
		fprintf(stderr, " because process accounting is not operational.\n");
	}
#endif

	if (sflg) {
		sprintf(fname,"/tmp/tmx%ld",getpid());
		sprintf(cmd,"/usr/lib/sa/sadc 1 1 %s",fname);
		system(cmd);
	}
	if (pflg + oflg) hmstime(stime);
	before = times(&obuffer);
	if ((p = fork()) == (pid_t)-1) diag("Try again.\n");
	if(p == 0) {
		setgid(getgid());
		execvp(*(argv+optind),(argv+optind));
		fprintf(stderr, "%s: %s\n", *(argv+optind), sys_errlist[errno]);
		exit(1);
	}
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	while(wait(&status) != p);
	if((status&0377) != 0)
		fprintf(stderr,"Command terminated abnormally.\n");
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
	after = times(&buffer);
	if (pflg + oflg) hmstime(etime);
	if (sflg) system(cmd);

	fprintf(stderr,"\n");
	printt("real", (after-before));
	printt("user", buffer.tms_cutime - obuffer.tms_cutime);
	printt("sys ", buffer.tms_cstime - obuffer.tms_cstime);
	fprintf(stderr,"\n");

	if (oflg+pflg) {
		if(isatty(0))
			sprintf(ttyid, "-l %s", ttyname(0)+5);
		sprintf(cmd, "acctcom -S %s -E %s -u %s %s -i %s",
			stime, etime, getpwuid(getuid())->pw_name, ttyid, aopt);
		pipin = popen(cmd, "r");
		while(fscanf(pipin, "%[^\n]%1c", line, &eol) > 1) {
			if(pflg)
				fprintf(stderr, "%s\n", line);
			if(oflg)  {
				nfld=sscanf(line,
				"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
				fld[0], fld[1], fld[2], fld[3], fld[4],
				fld[5], fld[6], fld[7], fld[8], fld[9],
				fld[10], fld[11], fld[12], fld[13], fld[14],
				fld[15], fld[16], fld[17], fld[18], fld[19]);
				if(++iline == 3)
					for(i=0; i<nfld; i++)  {
						if(strcmp(fld[i], "CHARS") == 0)
							ichar = i+2;
						if(strcmp(fld[i],"BLOCKS") == 0)
							iblok = i+2;
					}
				if (iline > 4)  {
					chars += atol(fld[ichar]);
					bloks += atol(fld[iblok]);
				}
			}
		}
		pclose(pipin);

		if(oflg)
			if(iline > 4)
				fprintf(stderr,
				"\nCHARS TRNSFD = %ld\nBLOCKS READ  = %ld\n",
				chars, bloks);
			else
				fprintf(stderr, "\nNo process records found!\n");
	}

	if (sflg)  {

#ifdef i386
		sprintf(cmd,"/usr/bin/sar -ubdycwaqvmpgrkxDS -f %s 1>&2",fname);
#else
		sprintf(cmd,"/usr/bin/sar -ubdycwaqvmpgrkxCDS -f %s 1>&2",fname);
#endif

		system(cmd);
		unlink(fname);
	}
	exit(status>>8);
}

char quant[] = { HZ/10, 10, 10, 6, 10, 6, 10, 10, 10 };
char *pad  = "000      ";
char *sep  = "\0\0.\0:\0:\0\0";
char *nsep = "\0\0.\0 \0 \0\0";

printt(s, a)
char	*s;
time_t	a;
{
	char	digit[9];
	register	i;
	char	c;
	int	nonzero;

	for(i=0; i<9; i++) {
		digit[i] = a % quant[i];
		a /= quant[i];
	}
	fprintf(stderr,s);
	nonzero = 0;
	while(--i>0) {
		c = digit[i]!=0 ? digit[i]+'0':
		    nonzero ? '0':
		    pad[i];
		if (c != '\0') putc(c,stderr);
		nonzero |= digit[i];
		c = nonzero?sep[i]:nsep[i];
		if (c != '\0') putc(c,stderr);
	}
	fprintf(stderr,"%c",digit[0]*100/HZ+'0');
	fprintf(stderr,"\n");
}

/*
** hmstime() sets current time in hh:mm:ss string format in stime;
*/

hmstime(stime)
char	stime[];
{
	char	*ltime, *ctime();
	long tme;

	tme = time((long *)0);
	ltime = ctime(&tme);
	strncpy(stime, ltime+11, 8);
}
 
diag(s)
char *s;
{
	fprintf(stderr,"%s\n",s);
	unlink(fname);
	exit(1);
}
