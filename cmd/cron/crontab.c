/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/* 	Portions Copyright(c) 1988, Sun Microsystems, Inc.	*/
/*	All Rights Reserved.					*/

#ident	"@(#)cron:crontab.c	1.8.5.3"
/**************************************************************************
 ***			C r o n t a b . c				***
 **************************************************************************

	date:	7/2/82
	description:	This program implements crontab (see cron(1)).
			This program should be set-uid to root.
	files:
		/usr/sbin/cron drwxr-xr-x root sys
		/etc/cron.d/cron.allow -rw-r--r-- root sys
		/etc/cron.d/cron.deny -rw-r--r-- root sys

 **************************************************************************/


#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <locale.h>
#include "cron.h"

#define TMPFILE		"_cron"		/* prefix for tmp file */
#define CRMODE		0444	/* mode for creating crontabs */

#define BADCREATE	"can't create your crontab file in the crontab directory."
#define BADOPEN		"can't open your crontab file."
#define BADSHELL	"because your login shell isn't /usr/bin/sh, you can't use cron."
#define WARNSHELL	"warning: commands will be executed using /usr/bin/sh\n"
#define BADUSAGE	"proper usage is: \n	crontab [file | -e | -l | -r ] [user]"
#define INVALIDUSER	"you are not a valid user (no entry in /etc/passwd)."
#define NOTALLOWED	"you are not authorized to use cron.  Sorry."
#define NOTROOT		"you must be super-user to access another user's crontab file"
#define EOLN		"unexpected end of line."
#define UNEXPECT	"unexpected character found in line."
#define OUTOFBOUND	"number out of bounds."
#define	ERRSFND		"errors detected in input, no crontab file generated."
#define BADREAD		"error reading your crontab file"

extern int opterr, optind, per_errno;
extern char *optarg, *xmalloc();
int err,cursor;
void catch();
char *cf,*tnam,line[CTLINESIZE];
char edtemp[5+13+1];

main(argc,argv)
char **argv;
{
	int c, rflag, lflag, eflag, errflg;
	char login[UNAMESIZE],*getuser(),*strcat(),*strcpy();
	char *pp;
	FILE *fp, *tmpfp;
	struct stat stbuf;
	time_t omodtime;
	char *editor, *getenv();
	char buf[BUFSIZ];
	uid_t ruid;
	pid_t pid;
	int stat_loc;

	rflag = 0;
	lflag = 0;
	eflag = 0;
	errflg = 0;
	(void)setlocale(LC_ALL, "");
	while ((c=getopt(argc, argv, "elr")) != EOF)
		switch (c) {
			case 'e':
				if (lflag || rflag)
					errflg++;
				else
					eflag++;
				break;
			case 'l':
				if (eflag || rflag)
					errflg++;
				else
					lflag++;
				break;
			case 'r':
				if (eflag || lflag)
					errflg++;
				else
					rflag++;
				break;
			case '?':
				errflg++;
				break;
		}
	argc -= optind;
	argv += optind;
	if (errflg || argc > 1)
		crabort(BADUSAGE);
	ruid = getuid();
	if ((eflag || lflag || rflag) && argc == 1) {
		if (ruid != 0)
			crabort(NOTROOT);
		pp = *argv++;
		if (getpwnam(pp) == NULL)
			crabort(INVALIDUSER);
	} else {
		pp = getuser(ruid);
		if(pp == NULL) {
			if (per_errno==2)
				crabort(BADSHELL);
			else
				crabort(INVALIDUSER); 
		}
	}
		
	strcpy(login,pp);
	if (!allowed(login,CRONALLOW,CRONDENY)) crabort(NOTALLOWED);

	cf = xmalloc(strlen(CRONDIR)+strlen(login)+2);
	strcat(strcat(strcpy(cf,CRONDIR),"/"),login);
	if (rflag) {
		unlink(cf);
		sendmsg(DELETE,login,login,CRON);
		exit(0); 
	}
	if (lflag) {
		if((fp = fopen(cf,"r")) == NULL)
			crabort(BADOPEN);
		while(fgets(line,CTLINESIZE,fp) != NULL)
			fputs(line,stdout);
		fclose(fp);
		exit(0);
	}
	if (eflag) {
		if((fp = fopen(cf,"r")) == NULL) {
			if(errno != ENOENT)
				crabort(BADOPEN);
		}
		(void)strcpy(edtemp, "/tmp/crontabXXXXXX");
		(void)mktemp(edtemp);
		/* 
		 * Fork off a child with user's permissions, 
		 * to edit the crontab file
		 */
		if ((pid = fork()) == (pid_t)-1)
			crabort("fork failed");
		if (pid == 0) {		/* child process */
			/* give up super-user privileges. */
			setuid(ruid);
			if((tmpfp = fopen(edtemp,"w")) == NULL)
				crabort("can't create temporary file");
			if(fp != NULL) {
				/*
				 * Copy user's crontab file to temporary file.
				 */
				while(fgets(line,CTLINESIZE,fp) != NULL) {
					fputs(line,tmpfp);
					if(ferror(tmpfp)) {
						fclose(fp);
						fclose(tmpfp);
						crabort("write error on temporary file");
					}
				}
				if (ferror(fp)) {
					fclose(fp);
					fclose(tmpfp);
					crabort(BADREAD);
				}
				fclose(fp);
			}
			if(fclose(tmpfp) == EOF)
				crabort("write error on temporary file");
			if(stat(edtemp, &stbuf) < 0)
				crabort("can't stat temporary file");
			omodtime = stbuf.st_mtime;
			editor = getenv("VISUAL");
			if (editor == NULL)
				editor = getenv("EDITOR");
			if (editor == NULL)
				editor = "ed";
			(void)sprintf(buf, "%s %s", editor, edtemp);
			if (system(buf) == 0) {
				/* sanity checks */
				if((tmpfp = fopen(edtemp, "r")) == NULL)
					crabort("can't open temporary file");
				if(fstat(fileno(tmpfp), &stbuf) < 0)
					crabort("can't stat temporary file");
				if(stbuf.st_size == 0)
					crabort("temporary file empty");
				if(omodtime == stbuf.st_mtime) {
					(void)unlink(edtemp);
					fprintf(stderr,
					    "The crontab file was not changed.\n");
					exit(1);
				}
				exit(0);
			} else {
				/*
				 * Couldn't run editor.
				 */
				(void)unlink(edtemp);
				exit(1);
			}
		}
		wait(&stat_loc);
		if ((stat_loc & 0xFF00) != 0)
			exit(1);
		if ((tmpfp = fopen(edtemp,"r")) == NULL) 
			crabort("can't open temporary file\n");
		copycron(tmpfp);
		(void)unlink(edtemp);
	} else {
		if (argc==0) 
			copycron(stdin);
		else if (access(argv[0],04) || (fp=fopen(argv[0],"r"))==NULL)
			 crabort(BADOPEN);
		else copycron(fp);
	}
	sendmsg(ADD,login,login,CRON);
	if(per_errno == 2)
		fprintf(stderr,WARNSHELL);
	exit(0);
}


/******************/
copycron(fp)
/******************/
FILE *fp;
{
	FILE *tfp,*fdopen();
	char pid[6],*strcat(),*strcpy();
	int t;

	sprintf(pid,"%-5d",getpid());
	tnam=xmalloc(strlen(CRONDIR)+strlen(TMPFILE)+7);
	strcat(strcat(strcat(strcpy(tnam,CRONDIR),"/"),TMPFILE),pid);
	/* catch SIGINT, SIGHUP, SIGQUIT signals */
	if (signal(SIGINT,catch) == SIG_IGN) signal(SIGINT,SIG_IGN);
	if (signal(SIGHUP,catch) == SIG_IGN) signal(SIGHUP,SIG_IGN);
	if (signal(SIGQUIT,catch) == SIG_IGN) signal(SIGQUIT,SIG_IGN);
	if (signal(SIGTERM,catch) == SIG_IGN) signal(SIGTERM,SIG_IGN);
	if ((t=creat(tnam,CRMODE))==-1) crabort(BADCREATE);
	if ((tfp=fdopen(t,"w"))==NULL) {
		unlink(tnam);
		crabort(BADCREATE); 
	}
	err=0;	/* if errors found, err set to 1 */
	while (fgets(line,CTLINESIZE,fp) != NULL) {
		cursor=0;
		while(line[cursor] == ' ' || line[cursor] == '\t')
			cursor++;
		if(line[cursor] == '#')
			goto cont;
		if (next_field(0,59)) continue;
		if (next_field(0,23)) continue;
		if (next_field(1,31)) continue;
		if (next_field(1,12)) continue;
		if (next_field(0,06)) continue;
		if (line[++cursor] == '\0') {
			cerror(EOLN);
			continue; 
		}
cont:
		if (fputs(line,tfp) == EOF) {
			unlink(tnam);
			crabort(BADCREATE); 
		}
	}
	fclose(fp);
	fclose(tfp);
	if (!err) {
		/* make file tfp the new crontab */
		unlink(cf);
		if (link(tnam,cf)==-1) {
			unlink(tnam);
			crabort(BADCREATE); 
		} 
	} else {
		fprintf(stderr,"crontab: %s\n",ERRSFND);
	}
	unlink(tnam);
}


/*****************/
next_field(lower,upper)
/*****************/
int lower,upper;
{
	int num,num2;

	while ((line[cursor]==' ') || (line[cursor]=='\t')) cursor++;
	if (line[cursor] == '\0') {
		cerror(EOLN);
		return(1); 
	}
	if (line[cursor] == '*') {
		cursor++;
		if ((line[cursor]!=' ') && (line[cursor]!='\t')) {
			cerror(UNEXPECT);
			return(1); 
		}
		return(0); 
	}
	while (TRUE) {
		if (!isdigit(line[cursor])) {
			cerror(UNEXPECT);
			return(1); 
		}
		num = 0;
		do { 
			num = num*10 + (line[cursor]-'0'); 
		}			while (isdigit(line[++cursor]));
		if ((num<lower) || (num>upper)) {
			cerror(OUTOFBOUND);
			return(1); 
		}
		if (line[cursor]=='-') {
			if (!isdigit(line[++cursor])) {
				cerror(UNEXPECT);
				return(1); 
			}
			num2 = 0;
			do { 
				num2 = num2*10 + (line[cursor]-'0'); 
			}				while (isdigit(line[++cursor]));
			if ((num2<lower) || (num2>upper)) {
				cerror(OUTOFBOUND);
				return(1); 
			}
		}
		if ((line[cursor]==' ') || (line[cursor]=='\t')) break;
		if (line[cursor]=='\0') {
			cerror(EOLN);
			return(1); 
		}
		if (line[cursor++]!=',') {
			cerror(UNEXPECT);
			return(1); 
		}
	}
	return(0);
}


/**********/
cerror(msg)
/**********/
char *msg;
{
	fprintf(stderr,"%scrontab: error on previous line; %s\n",line,msg);
	err=1;
}


/**********/
void
catch()
/**********/
{
	unlink(tnam);
	exit(1);
}


/**********/
crabort(msg)
/**********/
char *msg;
{
	int sverrno;

	if (strcmp(edtemp, "") != 0) {
		sverrno = errno;
		(void)unlink(edtemp);
		errno = sverrno;
	}
	if (tnam != NULL) {
		sverrno = errno;
		(void)unlink(tnam);
		errno = sverrno;
	}
	fprintf(stderr,"crontab: %s\n",msg);
	exit(1);
}
