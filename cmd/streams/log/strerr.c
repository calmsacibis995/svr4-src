/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-streams:log/strerr.c	1.2.2.2"
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stropts.h>
#include <sys/strlog.h>

#define CTLSIZE sizeof(struct log_ctl)
#define DATSIZE LOGMSGSZ
#define TIMESIZE 26
#define ADMSTR "root"
#define LOGDEV "/dev/log"
#define LOGNAME "STR.LOGGER"
#define ERRFILE "/var/adm/streams/error.xxxxx"
#define NSECDAY 86400

makefile(name, time)
char *name;
time_t time;
{
	char *r;
	struct tm *tp;
	
	tp = localtime(&time);
	r = &(name[strlen(name) - 5]);
	sprintf(r, "%02d-%02d", (tp->tm_mon+1), tp->tm_mday);
}

FILE *
logfile(log, lp)
struct log_ctl *lp;
FILE *log;
{
	static time_t lasttime = 0;
	char *errfile = ERRFILE;
	time_t newtime;

	newtime = lp->ttime - timezone;

	/*
	 * If it is a new day make a new log file
	 */
	if (((newtime/NSECDAY) != (lasttime/NSECDAY)) || !log) {
		if (log) fclose(log);
		lasttime = newtime;
		makefile(errfile, lp->ttime);
		return(fopen(errfile, "a+"));
	}
	lasttime = newtime;
	return(log);
		
}


main(ac, av)
int ac;
char **av;
{
	int fd, n;
	char cbuf[CTLSIZE];
	char dbuf[DATSIZE];	/* must start on word boundary */
	char mailcmd[40];
	int flag;
	struct strbuf ctl;
	struct strbuf dat;
	struct strioctl istr;
	struct log_ctl *lp;
	FILE *pfile, *popen();
	FILE *log;

	ctl.buf = cbuf;
	ctl.maxlen = CTLSIZE;
	dat.buf = dbuf;
	dat.maxlen = dat.len = DATSIZE;
	fd = open(LOGDEV, O_RDWR);
	if (fd < 0) {
		fprintf(stderr,"ERROR: unable to open %s\n", LOGDEV);
		exit(1);
	}

	istr.ic_cmd = I_ERRLOG;
	istr.ic_timout = istr.ic_len = 0;
	istr.ic_dp = NULL;
	if (ioctl(fd, I_STR, &istr) < 0) {
		fprintf(stderr,"ERROR: error logger already exists\n");
		exit(1);
	}

	log = NULL;
	flag = 0;
	while (getmsg(fd, &ctl, &dat, &flag) >= 0) {
		flag = 0;
		lp = (struct log_ctl *)cbuf;
		log = logfile(log, lp);
		prlog(log, lp, dbuf, 1);
		fflush(log);

		if (!(lp->flags & SL_NOTIFY)) continue;
		sprintf(mailcmd, "mail %s", ADMSTR);
		if((pfile = popen(mailcmd, "w")) != NULL) {
			fprintf(pfile, "Streams Error Logger message notification:\n\n");
			prlog(pfile, lp, dbuf, 0);
			pclose(pfile);
		}
	}
}


/*
 * calculate the address of the log printf arguments.  The pointer lp MUST
 * start on a word boundary.  This also assumes that struct log_msg is
 * an integral number of words long.  If either of these is violated,
 * an alignment fault will result.  
 */
int *
logadjust(dp)
char *dp;
{
	while (*dp++ != 0);
	dp = (char *)(((unsigned long)dp + sizeof(int) - 1) & ~(sizeof(int) - 1));
	return( (int *)dp);	
}


prlog(log, lp, dp, flag)
FILE *log;
struct log_ctl *lp;
char *dp;
{
	char *ts;
	int *args;
	char *ap;
	
	ts = ctime(&(lp->ttime));
	ts[19] = '\0';
	if (flag) {
		fprintf(log, "%06d %s %08x %s%s%s ", lp->seq_no, (ts+11), lp->ltime,
			((lp->flags & SL_FATAL) ? "F" : "."),
			((lp->flags & SL_NOTIFY) ? "N" : "."),
			((lp->flags & SL_TRACE) ? "T" : "."));
		fprintf(log, "%d %d ",
			lp->mid, lp->sid);
	} else 	fprintf(log, "%06d ", lp->seq_no);	
	args = logadjust((char *)dp);

	fprintf(log, dp, args[0], args[1], args[2]);
	putc('\n', log);
}



