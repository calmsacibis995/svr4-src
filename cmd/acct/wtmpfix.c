/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)acct:wtmpfix.c	1.12"
/*
 * wtmpfix - adjust wtmp file and remove date changes.
 *	wtmpfix <wtmp1 >wtmp2
 *
 *	code are added to really fix wtmp if it is corrupted .. 
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#include "acctdef.h"
#include <utmp.h>
#include <signal.h>
#include <time.h>
#include <ctype.h>
#include <locale.h>

#define 	MAXRUNTIME	3600	/* time out after 1 hour */
#define		DAYEPOCH	(60*60*24)
#define		wout(f,w)	 fwrite(w,sizeof(struct utmp),1,f);

FILE	*Wtmp, *Opw;
FILE	*fp;
char	*Ofile	={ "/tmp/wXXXXXX" };
static char time_buf[50];

struct	dtab
{
	long	d_off1;		/* file offset start */
	long	d_off2;		/* file offset stop */
	long	d_adj;		/* time adjustment */
	struct dtab *d_ndp;	/* next record */
};

struct	dtab	*Fdp;		/* list header */
struct	dtab	*Ldp;		/* list trailer */

long	ftell();
long 	lastmonth, nextmonth;
long	recno;

struct	utmp	Ut, Ut2;

int	year, month;
int 	ch;
int	n;
int	multimode;		/* multi user mode	 WHCC */
void 	abort(), intr();


main(argc, argv)
char	**argv;
{
	long	time(), tloc, ftell();
	struct tm	*localtime(), *tmp;

	(void)setlocale(LC_ALL, "");
	setbuf(stdout, NULL);
	alarm(MAXRUNTIME);

	if ((int)signal(SIGALRM, abort) == -1)
		perror("signal"), exit(1); 
	if((int)signal(SIGINT,intr) == -1) 
		perror("signal"), exit(1); 

	time(&tloc);
	tmp = localtime(&tloc);
	year = tmp->tm_year;
	month = tmp->tm_mon + 1;
	lastmonth = ((year + 1900 - 1970) * 365 + (month - 1) * 30) * DAYEPOCH; 
	nextmonth = ((year + 1900 - 1970) * 365 + (month + 1) * 30) * DAYEPOCH; 

	if(argc < 2){
		argv[argc] = "-";
		argc++;
	}

	mktemp(Ofile);
	if((Opw=fopen(Ofile,"w"))==NULL)
		err("cannot make temporary: %s", Ofile);

	while(--argc > 0){
		argv++;
		if(strcmp(*argv,"-")==0)
			Wtmp = stdin;
		else if((Wtmp = fopen(*argv,"r"))==NULL)
			err("Cannot open: %s", *argv);

		scanfile();

		if(Wtmp!=stdin)
			fclose(Wtmp);
	}
	fclose(Opw);

	if((Opw=fopen(Ofile,"r"))==NULL)
		err("Cannot read from temp: %s", Ofile);
	recno = 0;
	while(winp(Opw,&Ut)){
		adjust(recno,&Ut);
		recno += sizeof(struct utmp);
		wout(stdout,&Ut);
	}
	fclose(Opw);
	unlink(Ofile);
	exit(0);
}

err(f,m1)
char *f;
long m1;
{
	fprintf(stderr,f,m1);
	fprintf(stderr,"\n");
	intr();
}

winp(f,w)
FILE *f;
struct utmp *w;
{
	if(fread(w,sizeof(struct utmp),1,f)!=1)
		return 0;
	if((w->ut_type >= EMPTY) && (w->ut_type <= UTMAXTYPE))
		return ((unsigned)w);
	else {
		fprintf(stderr,"Bad file at offset %ld\n",
			ftell(f)-sizeof(struct utmp));
		cftime(time_buf, DATE_FMT, &w->ut_time);
		fprintf(stderr,"%-12s %-8s %lu %s",
			w->ut_line,w->ut_user,w->ut_time,time_buf);
		intr();
	}
}

mkdtab(p)
long	p;
{

	register struct dtab *dp;

	dp = Ldp;
	if(dp == NULL){
		dp = (struct dtab *)calloc(sizeof(struct dtab),1);
		if(dp == NULL)
			err("out of core");
		Fdp = Ldp = dp;
	}
	dp->d_off1 = p;
}

setdtab(p,w1,w2)
long	p;
register struct utmp *w1, *w2;
{
	register struct dtab *dp;

	if((dp=Ldp)==NULL)
		err("no dtab");
	dp->d_off2 = p;
	dp->d_adj = w2->ut_time - w1->ut_time;
	if((Ldp=(struct dtab *)calloc(sizeof(struct dtab),1))==NULL)
		err("out of core");
	Ldp->d_off1 = dp->d_off1;
	dp->d_ndp = Ldp;
}

adjust(p,w)
long	p;
register struct utmp *w;
{

	long pp;
	register struct dtab *dp;

	pp = p;

	for(dp=Fdp;dp!=NULL;dp=dp->d_ndp){
		if(dp->d_adj==0)
			continue;
		if(pp>=dp->d_off1 && pp < dp->d_off2)
			w->ut_time += dp->d_adj;
	}
}

/*
 *	invalid() determines whether the name field adheres to
 *	the criteria set forth in acctcon1.  If the name violates
 *	conventions, it returns a truth value meaning the name is
 *	invalid; if the name is okay, it returns false indicating
 *	the name is not invalid.
 */

invalid(name)
char	*name;
{
	register int	i;

	for(i=0; i<NSZ; i++) {
		if(name[i] == '\0')
			return(VALID);
		if( ! (isalnum(name[i]) || (name[i] == '$')
			|| (name[i] == ' ') || (name[i] == '.') )) {
			return(INVALID);
		}
	}
	return(VALID);
}

static void
intr()
{
	signal(SIGINT,SIG_IGN);
	unlink(Ofile);
	exit(1);
}

/* scanfile:
 * 1)  	reads the file, to see if the record is within reasonable
 * 	range; if not, then it will scan the file, delete foreign stuff.
 * 2)   enter setdtab if in multiuser mode
 * 3)   change bad login names to INVALID
 */

scanfile()
{
	while ( (n = fread(&Ut, sizeof Ut, 1, Wtmp)) > 0) { 
		if (n == 0)
			exit(0);
		if( ! inrange() ) {
			for (;;) {
				if (fseek(Wtmp, -(long) sizeof Ut, 1) != 0) 
					perror("seek error\n"), exit(1);
				if ( (ch = getc(Wtmp)) == EOF )
					perror("read\n"), exit(1);
				fprintf(stderr, "checking offset %lo\n", ftell(Wtmp));
				if(fread(&Ut, sizeof(Ut), 1, Wtmp) == 0) {
					exit (1);
				}
				if ( inrange() )
					break;
			}
		}
		
		/* Now we have a good utmp record, do more processing */

#define UTYPE	Ut.ut_type    
#define ULINE	Ut.ut_line    

			if(recno == 0 || UTYPE==BOOT_TIME)
				mkdtab(recno);
			if (UTYPE==RUN_LVL) {	
				if (strncmp(ULINE, "run-level S", 11) == 0)
					multimode = 0;
				if (strncmp(ULINE, "run-level 2", 11) == 0)
					multimode++;
			}
			if(invalid(Ut.ut_name)) {
				fprintf(stderr,
					"wtmpfix: logname \"%8.8s\" changed to \"INVALID\"\n",
					Ut.ut_name);
				strncpy(Ut.ut_name, "INVALID", NSZ);
			}
			if(UTYPE==OLD_TIME){
				if(!winp(Wtmp,&Ut2))
					err("Input truncated at offset %ld",recno);
				if(Ut2.ut_type!=NEW_TIME)
					err("New date expected at offset %ld",recno);
				if (multimode)  /* multiuser */
					setdtab(recno,&Ut,&Ut2);
				recno += (2 * sizeof(struct utmp));
				wout(Opw,&Ut);
				wout(Opw,&Ut2);
				continue;
			}
			wout(Opw,&Ut);
			recno += sizeof(struct utmp);
	}
}


inrange()
{
	if ( (strcmp(Ut.ut_line, RUNLVL_MSG) == 0) ||
		  (strcmp(Ut.ut_line, BOOT_MSG) == 0) ||
		  (strcmp(Ut.ut_line, "acctg on") == 0) ||
		  (strcmp(Ut.ut_line, OTIME_MSG) == 0) ||
		  (strcmp(Ut.ut_line, NTIME_MSG) == 0) )
			return 1;

	if ( /* 	Ut.ut_line[0] != '\0' && */
		Ut.ut_id != 0 &&
		Ut.ut_time > 0 &&
		Ut.ut_time > lastmonth &&
		Ut.ut_time < nextmonth &&
		Ut.ut_type >= EMPTY &&
		Ut.ut_type <= UTMAXTYPE && 
		Ut.ut_pid >= 0 && 
		Ut.ut_pid <= MAXPID ) 
			return 1;
	return 0;
}

static void abort()
{
	fprintf(stderr, "give up\n");
	exit(1);
}
