/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/* 	Portions Copyright(c) 1988, Sun Microsystems, Inc.	*/
/*	All Rights Reserved.					*/

#ident	"@(#)cron:at.c	1.12.5.3"
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <locale.h>
#include "cron.h"

extern	int	getdate_err;

#define TMPFILE		"_at"	/* prefix for temporary files	*/
#define ATMODE		06444	/* Mode for creating files in ATDIR.
Setuid bit on so that if an owner of a file gives that file 
away to someone else, the setuid bit will no longer be set.  
If this happens, atrun will not execute the file	*/
#define ROOT		0	/* user-id of super-user */
#define BUFSIZE		512	/* for copying files */
#define LINSIZ		256	/* length of line buffer */
#define LINESIZE	130	/* for listing jobs */
#define	MAXTRYS		100	/* max trys to create at job file */

#define BADDATE		"bad date specification"
#define BADFIRST	"bad first argument"
#define BADHOURS	"hours field is too large"
#define BADMD		"bad month and day specification"
#define BADMINUTES	"minutes field is too large"
#define BADSHELL	"because your login shell isn't /usr/bin/sh, you can't use at"
#define WARNSHELL	"warning: commands will be executed using /usr/bin/sh\n"
#define CANTCD		"can't change directory to the at directory"
#define CANTCHOWN	"can't change the owner of your job to you"
#define CANTCREATE	"can't create a job for you"
#define INVALIDUSER	"you are not a valid user (no entry in /etc/passwd)"
#define	NONUMBER	"proper syntax is:\n\tat -ln\nwhere n is a number"
#define NOOPENDIR	"can't open the at directory"
#define NOTALLOWED	"you are not authorized to use at.  Sorry."
#define NOTHING		"nothing specified"
#define PAST		"it's past that time"

#define FORMAT		"%a %b %e %H:%M:%S %Y"

/*
	this data is used for parsing time
*/
#define	dysize(A)	(((A)%4) ? 365 : 366)
int	gmtflag = 0;
int	dflag = 0;
extern	time_t	timezone;
extern	char	*argp;
char	login[UNAMESIZE];
char	argpbuf[80];
char	pname[80];
char	pname1[80];
char	argbuf[80];
time_t	when, now, gtime();
struct	tm	*tp, at, rt, *localtime();
int	mday[12] =
{
	31,38,31,
	30,31,30,
	31,31,30,
	31,30,31,
};
int	mtab[12] =
{
	0,   31,  59,
	90,  120, 151,
	181, 212, 243,
	273, 304, 334,
};
int     dmsize[12] = {
	31,28,31,30,31,30,31,31,30,31,30,31};

struct	tm	*ct;
char	timebuf[80];
/*
 * Error in getdate(3G)
 */
static char 	*errlist[] = {
/* 	0	*/ 	"",
/*	1	*/	"getdate: The DATEMSK environment variable is not set",
/*	2	*/	"getdate: Error on \"open\" of the template file",
/*	3	*/	"getdate: Error on \"stat\" of the template file",
/*      4	*/	"getdate: The template file is not a regular file",
/*	5	*/	"getdate: An error is encountered while reading the template",
/*	6 	*/	"getdate: Malloc(3C) failed",
/*	7	*/	"getdate: There is no line in the template that matches the input",
/*	8	*/	"getdate: Invalid input specification"
};

short	jobtype = ATEVENT;		/* set to 1 if batch job */
char	*tfname;
extern char *malloc();
extern char *xmalloc();
extern int   per_errno;
extern void  exit();
time_t	num();
int	mailflag;

main(argc,argv)
char **argv;
{
	DIR *dir;
	FILE *fopen();
	struct dirent *dentry;
	struct passwd *pw;
	struct stat buf, st1, st2;
	uid_t user;
	int i,fd;
	void catch();
	unsigned int atdirlen;
	char *ptr,*job,pid[6];
	char *pp, *atdir, *patdir;
	char *fgets(),*strcat(),*strcpy(),*strrchr();
	char *mkjobname(),*getuser();
	char *jobfile = NULL;	      /* file containing job to be run */
	FILE *inputfile;
	time_t t,time();
	int  st = 1,try=0;
	char	*file;
	char	*getenv();
	struct  tm	*getdate();
	char  c;

	if(argc < 2){
		usage();
	}
	(void)setlocale(LC_ALL, "");
	pp = getuser((user=getuid()));
	if(pp == NULL) {
		if(per_errno == 2)
			atabort(BADSHELL);
		else
			atabort(INVALIDUSER);
	}
	strcpy(login,pp);
	if (!allowed(login,ATALLOW,ATDENY)) atabort(NOTALLOWED);

	if (strcmp(argv[1],"-r")==0) {
		/* remove jobs that are specified */
		if (chdir(ATDIR)==-1) atabort(CANTCD);
		for (i=2; i<argc; i++)
			if (stat(argv[i],&buf))
				fprintf(stderr,"at: %s does not exist\n",argv[i]);
			else if ((user!=buf.st_uid) && (user!=ROOT))
				fprintf(stderr,"at: you don't own %s\n",argv[i]);
			else {
				sendmsg(DELETE,login,argv[i],AT);
				unlink(argv[i]);
			}
		exit(0); 
	}

	if (strncmp(argv[1],"-l",2)==0) {
		/* list jobs for user */
		if (chdir(ATDIR)==-1) atabort(CANTCD);
		if (argc==2) {
			/* list all jobs for a user */
			atdirlen = strlen(ATDIR);
			if ((atdir = (char *)malloc(atdirlen + 1)) == NULL)
				atabort("Out of memory");
			strcpy(atdir, ATDIR);
			patdir= strrchr(atdir, '/');
			*patdir = '\0';
			if (stat(ATDIR,&st1) != 0 || stat(atdir,&st2) != 0)
				atabort("Can not get status of spooling directory for at");
			if ((dir=opendir(ATDIR)) == NULL) atabort(NOOPENDIR);
			for (;;) {
				if ((dentry = readdir(dir)) == NULL)
					break;
				if (dentry->d_ino==st1.st_ino || dentry->d_ino==st2.st_ino)
					continue;
				if (stat(dentry->d_name,&buf)) {
					unlink(dentry->d_name);
					continue; 
				}
				if ((user!=ROOT) && (buf.st_uid!=user))
					continue;
				ptr = dentry->d_name;
				if (((t=num(&ptr))==0) || (*ptr!='.'))
					continue;
				ascftime(timebuf, FORMAT, localtime(&t));
				if ((user==ROOT) && ((pw=getpwuid(buf.st_uid))!=NULL))
					printf("user = %s\t%s\t%s\n",pw->pw_name,dentry->d_name, timebuf);
				else	printf("%s\t%s\n",dentry->d_name, timebuf);
			}
			(void) closedir(dir);
		}
		else	/* list particular jobs for user */
			for (i=2; i<argc; i++) {
				ptr = argv[i];
				if (((t=num(&ptr))==0) || (*ptr!='.'))
					fprintf(stderr,"at: invalid job name %s\n",argv[i]);
				else if (stat(argv[i],&buf))
					fprintf(stderr,"at: %s does not exist\n",argv[i]);
				else if ((user!=buf.st_uid) && (user!=ROOT))
					fprintf(stderr,"at: you don't own %s\n",argv[i]);
				else 
					{
					ascftime(timebuf, FORMAT, localtime(&t));	
					printf("%s\t%s\n",argv[i],timebuf);
					}
			}
		exit(0);
	}

	for(i=1; i<argc && argv[i][0] == '-'; i++) {
		if(strncmp(argv[i], "-q",2) == 0) {
			if(argv[i][2] == NULL)
				atabort("no queue specified");
			jobtype = argv[i][2] - 'a';
		}
		else {
			pp = &argv[i][1];
			while ((c = *pp++) != '\0') {
				switch(c) {
					case 'm':
						mailflag++;		
						break;	
					case 'f':
						if (i+1 >= argc || argv[i+1][0] == '-')
							usage();
						else 
							jobfile = argv[++i];
							
						break;
					default: 
						usage();
				}
			}
		}
	}

	st = i;

	/* figure out what time to run the job */

	if(argc == 1 && jobtype != BATCHEVENT)
		atabort(NOTHING);
	time(&now);
	if(jobtype != BATCHEVENT) {	/* at job */
		argp = argpbuf;
		i = st;
		while(i < argc) {
			strcat(argp,argv[i]);
			strcat(argp, " ");
			i++;
		}
		if ((file = getenv("DATEMSK")) == 0 || file[0] == '\0')
		{
			tp = localtime(&now);
			mday[1] = 28 + leap(tp->tm_year);
			yyparse();
			atime(&at, &rt);
			when = gtime(&at);
			if(!gmtflag) {
				when += timezone;
				if(localtime(&when)->tm_isdst)
					when -= 60 * 60;
			}
		}
		else
		{	/*   DATEMSK is set  */
			if ((ct = getdate(argpbuf)) == NULL)		
				atabort(errlist[getdate_err]);
			else
				when = mktime(ct);
		}
	} else		/* batch job */
		when = now;

	if(when < now)	/* time has already past */
		atabort("too late");

	sprintf(pid,"%-5d",getpid());
	tfname=xmalloc(strlen(ATDIR)+strlen(TMPFILE)+7);
	strcat(strcat(strcat(strcpy(tfname,ATDIR),"/"),TMPFILE),pid);
	/* catch SIGINT, HUP, and QUIT signals */
	if (signal(SIGINT, catch) == SIG_IGN) signal(SIGINT,SIG_IGN);
	if (signal(SIGHUP, catch) == SIG_IGN) signal(SIGHUP,SIG_IGN);
	if (signal(SIGQUIT,catch) == SIG_IGN) signal(SIGQUIT,SIG_IGN);
	if (signal(SIGTERM,catch) == SIG_IGN) signal(SIGTERM,SIG_IGN);
	if((fd = open(tfname,O_CREAT|O_EXCL|O_WRONLY,ATMODE)) < 0)
		atabort(CANTCREATE);
	if (chown(tfname,user,getgid())==-1) {
		unlink(tfname);
		atabort(CANTCHOWN);
	}
	close(1);
	dup(fd);
	close(fd);
	sprintf(pname,"%s",PROTO);
	sprintf(pname1,"%s.%c",PROTO,'a'+jobtype);

	/*
	 * Open the input file with the user's permissions.
	 */
	if (jobfile != NULL) {
		if ((inputfile = fopen(jobfile, "r")) == NULL) {
			unlink(tfname);
			fprintf(stderr,"at: %s: %s\n", jobfile, errmsg(errno));
			exit(1);
		}
	} else 
		inputfile = stdin;



	copy(jobfile, inputfile);
	while (rename(tfname,job=mkjobname(when))==-1) {
		sleep(1);
		if(++try > MAXTRYS / 10){
			atabort(CANTCREATE);
		}
	}
	unlink(tfname);
	sendmsg(ADD,login,strrchr(job,'/')+1,AT);
	if(per_errno == 2)
		fprintf(stderr,WARNSHELL);
	cftime(timebuf, FORMAT, &when);
	fprintf(stderr,"job %s at %s\n",strrchr(job,'/')+1,timebuf);
	if (when-t-MINUTE < HOUR) fprintf(stderr,
	    "at: this job may not be executed at the proper time.\n");
	exit(0);
}


/***************/
find(elem,table,tabsize)
/***************/
char *elem,**table;
int tabsize;
{
	int i;

	for (i=0; i<(int)strlen(elem); i++)
		elem[i] = tolower(elem[i]);
	for (i=0; i<tabsize; i++)
		if (strcmp(elem,table[i])==0) return(i);
		else if (strncmp(elem,table[i],3)==0) return(i);
	return(-1);
}


/****************/
char
*mkjobname(t)
/****************/
time_t t;
{
	int i;
	char *name;
	struct  stat buf;
	name=xmalloc(200);
	for (i=0;i < MAXTRYS;i++) {
		sprintf(name,"%s/%ld.%c",ATDIR,t,'a'+jobtype);
		if (stat(name,&buf)) 
			return(name);
		t += 1;
	}
	atabort("queue full");
}


/****************/
void
catch()
/****************/
{
	unlink(tfname);
	exit(1);
}


/****************/
atabort(msg)
/****************/
char *msg;
{
	fprintf(stderr,"at: %s\n",msg);
	exit(1);
}

yywrap()
{
	return 1;
}

yyerror()
{
	atabort(BADDATE);
}

/*
 * add time structures logically
 */
atime(a, b)
register
struct tm *a, *b;
{
	if ((a->tm_sec += b->tm_sec) >= 60) {
		b->tm_min += a->tm_sec / 60;
		a->tm_sec %= 60;
	}
	if ((a->tm_min += b->tm_min) >= 60) {
		b->tm_hour += a->tm_min / 60;
		a->tm_min %= 60;
	}
	if ((a->tm_hour += b->tm_hour) >= 24) {
		b->tm_mday += a->tm_hour / 24;
		a->tm_hour %= 24;
	}
	a->tm_year += b->tm_year;
	if ((a->tm_mon += b->tm_mon) >= 12) {
		a->tm_year += a->tm_mon / 12;
		a->tm_mon %= 12;
	}
	a->tm_mday += b->tm_mday;
	while (a->tm_mday > mday[a->tm_mon]) {
		a->tm_mday -= mday[a->tm_mon++];
		if (a->tm_mon > 11) {
			a->tm_mon = 0;
			mday[1] = 28 + leap(++a->tm_year);
		}
	}
}

leap(year)
{
	return year % 4 == 0;
}

/*
 * return time from time structure
 */
time_t
gtime(tptr)
register
struct	tm *tptr;
{
	register i;
	long	tv;
	extern int dmsize[];

	tv = 0;
	for (i = 1970; i < tptr->tm_year+1900; i++)
		tv += dysize(i);
	if (dysize(tptr->tm_year) == 366 && tptr->tm_mon >= 2)
		++tv;
	for (i = 0; i < tptr->tm_mon; ++i)
		tv += dmsize[i];
	tv += tptr->tm_mday - 1;
	tv = 24 * tv + tptr->tm_hour;
	tv = 60 * tv + tptr->tm_min;
	tv = 60 * tv + tptr->tm_sec;
	return tv;
}

/*
 * make job file from proto + stdin
 */
copy(jobfile, inputfile)
char *jobfile;
FILE *inputfile;
{
	register c;
	register FILE *pfp;
	register FILE *xfp;
	char	dirbuf[512];
	char	line[BUFSIZ];
	register char **ep;
	char *strchr();
	mode_t um;
	char *val;
	extern char **environ;
	int pfd[2];
	pid_t pid;
	uid_t realusr;
	printf(": %s job\n",jobtype ? "batch" : "at");
	printf(": jobname: %.127s\n",(jobfile==NULL) ? "stdin" : jobfile);
	printf(": notify by mail: %s\n", (mailflag) ? "yes" : "no");
	for (ep=environ; *ep; ep++) {
		if ( strchr(*ep,'\'')!=NULL )
			continue;
		if ((val=strchr(*ep,'='))==NULL)
			continue;
		*val++ = '\0';
		printf("export %s; %s='%s'\n",*ep,*ep,val);
		*--val = '=';
	}
	if((pfp = fopen(pname1,"r")) == NULL && (pfp=fopen(pname,"r"))==NULL)
		atabort("no prototype");
	um = umask(0);
	while ((c = getc(pfp)) != EOF) {
		if (c != '$')
			putchar(c);
		else switch (c = getc(pfp)) {
		case EOF:
			goto out;
		case 'd':	/* fork off a child with submitter's permissions,  */
				/* otherwise, when IFS=/, /usr/bin/pwd would be parsed */
				/* by the shell as file "bin".  The shell would    */
				/* then search according to the submitter's PATH   */
				/* and run the file bin with root permission	   */

			(void) fflush(stdout);
			dirbuf[0] = NULL;
			if (pipe(pfd) != 0)
				atabort("pipe open failed");
			realusr=getuid(); /* get realusr before the fork */
			if ((pid = fork()) == (pid_t)-1)
				atabort("fork failed");
			if (pid == 0) {			/* child process */
				(void) close(pfd[0]);
			(void) setuid(realusr); /* remove setuid for pwd */
				if((xfp=popen("/usr/bin/pwd","r")) != NULL) {
					fscanf(xfp,"%s",dirbuf);
					(void) pclose(xfp);
					xfp=fdopen(pfd[1],"w");
					fprintf(xfp,"%s",dirbuf);
					(void) fclose(xfp);
				}
				exit(0);
			}
			(void) close(pfd[1]);		/* parent process */
			xfp = fdopen(pfd[0], "r");
			fscanf(xfp,"%s",dirbuf);
			printf("%s", dirbuf);
			(void) fclose(xfp);
			break;
		case 'l':
			printf("%ld",ulimit(1,-1L));
			break;
		case 'm':
			printf("%o", um);
			break;
		case '<':
			while (fgets(line, LINSIZ, inputfile) != NULL)
				puts(line);
			break;
		case 't':
			printf(":%lu", when);
			break;
		default:
			putchar(c);
		}
	}
out:
	fclose(pfp);
}


/*
 * Print usage info and exit.
 */
usage()
{
	fprintf(stderr,"usage: at [-m] [-f filename] time [date] [+ increment]\n");
	fprintf(stderr,"       at -l job\n");
	fprintf(stderr,"       at -r [jobs]\n");
	exit(1);
}
