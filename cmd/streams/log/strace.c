/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-streams:log/strace.c	1.2.2.2"
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stropts.h>
#include <sys/strlog.h>

#define CTLSIZE sizeof(struct log_ctl)
#define DATSIZE LOGMSGSZ
#define TIMESIZE 26
#define LOGDEV "/dev/log"
#define MAXTID 50

static int errflg = 0;	/* set if error in argument parsing */
static int infile = 0;  /* set if using standard input for arguments */
static int log;


#define numeric(c) ((c <= '9') && (c >= '0'))
int
convarg(ap)
char *ap;
{
	short ids[2];

	if (!ap) return(-2);
	if (numeric(*ap)) return(atoi(ap));
	if (!strcmp(ap,"all")) return(-1);
	errflg = 1;
	return(-2);
}

char *
getarg()
{
	static char argbuf[40];
	static eofflg = 0;
	char *ap;
	int c;

	if (eofflg) {
		infile = 0;
		return(NULL);
	}

	ap = argbuf;

	/*
	 * Scan to first significant character in standard input.
	 * If EOF is encountered turn off standard input scanning and
	 * return NULL
	 */
	while ((c = getchar()) == ' ' || c == '\n' || c == '\t') ;
	if (c == EOF) {
		infile = 0;
		eofflg++;
		return(NULL);
	}
	/*
	 * collect token until whitespace is encountered.  Don't do anything
	 * with EOF here as it will be caught the next time around.
	 */
	while (1) {
		*ap++ = c;
		if ((c = getchar()) == ' ' || c == '\n' || c == '\t' || c == EOF) {
			if (c == EOF) eofflg++;
			*ap = '\0';
			return(argbuf);
		}
	}
}
			

getid(ac, av, tp)
int ac;
char **av;
struct trace_ids *tp;
{
	static index = 1;

	/*
	 * if inside of standard input scan take arguments from there.  
	 */
retry:
	if (infile) {
		tp->ti_mid = convarg(getarg());
		tp->ti_sid = convarg(getarg());
		tp->ti_level = convarg(getarg());
		if (errflg) return(0);
		/*
		 * if the previous operations encountered EOF, infile
		 * will be set to zero.  The trace_ids structure must
		 * then be loaded from the command line arguments.
		 * Otherwise, the structure is now valid and should
		 * be returned.
		 */
		if (infile) return(1);
	}
	/*
	 * if we get here we are either taking arguments from the 
	 * command line argument list or we hit end of file on standard
	 * input and should return to finish off the command line arguments
	 */
	if (index >= ac) return(0);

	/*
	 * if a '-' is present, start parsing from standard input
	 */
	if (!strcmp(av[index], "-")) {
		infile = 1;
		index++;
		goto retry;
	}

	/*
	 * Parsing from command line, make sure there are
	 * at least 3 arguments remaining.
	 */
	if ((index+2) >= ac) return(0);

	tp->ti_mid = convarg(av[index++]);
	tp->ti_sid = convarg(av[index++]);
	tp->ti_level = convarg(av[index++]);
	
	if (errflg) return(0);
	return(1);
}

main(ac, av)
int ac;
char **av;
{
	int  n;
	char cbuf[CTLSIZE];
	char dbuf[DATSIZE];
	struct strioctl istr;
	struct strbuf ctl, dat;
	struct log_ctl *lp = (struct log_ctl *)cbuf;
	struct trace_ids tid[MAXTID];
	struct trace_ids *tp;
	int ntid;
	int val;
	int flag;
	
	ctl.buf = cbuf;
	ctl.maxlen = CTLSIZE;
	dat.buf = dbuf;
	dat.len = dat.maxlen = DATSIZE;

	log = open(LOGDEV, O_RDWR);
	if (log < 0) {
		fprintf(stderr,"ERROR: unable to open %s\n", LOGDEV);
		exit(1);
	}

	tp = tid;
	ntid = 0;

	if (ac == 1) {
		ntid++;
		tid[0].ti_mid = -1;
		tid[0].ti_sid = -1;
		tid[0].ti_level = -1;
	} else 	while (getid(ac, av, tp)) {
		ntid++;
		tp++;
	}

	if (errflg) exit(errflg);

	istr.ic_cmd = I_TRCLOG;
	istr.ic_dp = (char *)tid;
	istr.ic_len = ntid * sizeof(struct trace_ids);
	istr.ic_timout = 0;
	if (ioctl(log, I_STR, &istr) < 0){
		fprintf(stderr,"ERROR: tracer already exists\n");
		exit(1);
	}

	setbuf(stdout, (char *)NULL);
	flag = 0;
	while (getmsg(log, &ctl, &dat, &flag) >= 0) {
		flag = 0;
		lp = (struct log_ctl *)cbuf;
		prlog(stdout, lp, dbuf);
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


prlog(log, lp, dp)
FILE *log;
struct log_ctl *lp;
char *dp;
{
	char *ts;
	int *args;
	char *ap;
	
	ts = ctime(&(lp->ttime));
	ts[19] = '\0';
	fprintf(log, "%06d %s %08x %2d %s%s%s %d %d ", 
			lp->seq_no, (ts+11), 
			lp->ltime, lp->level,
			((lp->flags & SL_FATAL) ? "F" : "."),
			((lp->flags & SL_NOTIFY) ? "N" : "."),
			((lp->flags & SL_ERROR) ? "E" : "."),
			lp->mid, lp->sid);

	args = logadjust((char *)dp);

	fprintf(log, dp, args[0], args[1], args[2]);
	putc('\n', log);
}

