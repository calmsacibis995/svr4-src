/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:cmd/lpsched/lpNet/bsdChild/recvjob.c	1.5.2.1"

#include <sys/types.h>
#include <sys/resource.h>
#include <sys/statfs.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <signal.h>
#if	defined(__STDC__)
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include "lp.h"			/* includes sys/stat.h fcntl.h */
#define  WHO_AM_I	I_AM_OZ
#include "oam_def.h"
#include "requests.h"
#include "secure.h"
#include "printers.h"
#include "lpNet.h"
#include "lpd.h"

static char 	 Spoolfn[MAX_SV_SPFN_SZ]; /* name of last spool file created */
static char	*Jobname;
static int	 Jobsize;

#if defined (__STDC__)
static	REQUEST	* mkrequest(char *, size_t);
static	char	* cvt_spoolfn(char *, int);
static	int	  chksize(char *, size_t);
static	int	  frecverr(char *, ...);
static	int	  noresponse(void);
static	int	  print_request(REQUEST *, char *);
static	int	  qdisabled(void);
static	int	  read2file(char *, size_t);
static	int	  read2mem(char *, size_t);
static	int	  smail(char *, long, ...);
static	void	  catch(int);
static	void	  rcleanup(void);
static	void	  trashrequest(REQUEST *);
#else
static	REQUEST	* mkrequest();
static	char	* cvt_spoolfn();
static	int	  chksize();
static	int	  frecverr();
static	int	  noresponse();
static	int	  print_request();
static	int	  qdisabled();
static	int	  read2file();
static	int	  read2mem();
static	int	  smail();
static	void	  catch();
static	void	  rcleanup();
static	void	  trashrequest();
#endif

static	void 	(*alrmhdlr)();

/*
 * Recieve print job from remote lpd
 */
void
#if defined (__STDC__)
recvjob(void)
#else
recvjob()
#endif
{
	register char	*cp;
	char		*blank;
	REQUEST		*rqp;
	size_t		 size;
	char		*cfile;
	struct stat	 stb;

	(void)sigset(SIGTERM, catch);
	(void)sigset(SIGPIPE, catch);
	alrmhdlr = sigset(SIGALRM, catch);

	if (!isprinter(Printer)) {
		frecverr("unknown printer %s", Printer);
		/*NOTREACHED*/
	}
	if (qdisabled()) {
		NAK1();
		logit(LOG_INFO, "request for disabled printer: %s", Printer);
		done(1);
		/*NOTREACHED*/
	}
	if (chdir(Lp_Requests) < 0 ||
	    stat(Rhost, &stb) < 0 && 
		(mkdir_lpdir(Rhost, MODE_DIR) < 0 || stat(Rhost, &stb) < 0) ||
	   (stb.st_mode & S_IFMT) != S_IFDIR) {
		frecverr("%s: %s/%s", Printer, Lp_Requests, Rhost);
		/*NOTREACHED*/
	}
	if (chdir(Lp_Tmp) < 0 ||
	    stat(Rhost, &stb) < 0 && 
		(mkdir_lpdir(Rhost, MODE_DIR) < 0 || stat(Rhost, &stb) < 0) ||
	   (stb.st_mode & S_IFMT) != S_IFDIR) {
		frecverr("%s: %s/%s", Printer, Lp_Tmp, Rhost);
		/*NOTREACHED*/
	}
	ACK();
	for (;;) {
		/*
		 * Read a command to tell us what to do
		 */
		cp = Msg;	/* borrow Msg buffer to read 2ndary request */
		do {
			int	nr;

			if ((nr = read(CIP->fd, cp, 1)) != 1) {
				if (nr < 0) {
					frecverr("%s: Lost connection", 
								Printer);
					/*NOTREACHED*/
				}
				return;
			}
		} while (*cp++ != '\n');
		*--cp = '\0';
		cp = Msg;
		logit(LOG_DEBUG, "received lpd message: %d%s", *cp, cp+1);
		switch (*cp++) {

		case CLEANUP:	/* cleanup because data sent was bad */
			rcleanup();
			continue;

		case READCFILE:	/* read cf file */ 
			size = strtol(cp, &blank, 10);
			if (cp = blank, *cp++ != ' ')
				break;
			if (!(cfile = (char *)malloc(size))) {
				NAK1();	     /* force remote to close & retry */
				rcleanup();
				continue;
			}
			if (!read2mem(cfile, size)) {
				free(cfile);
				rcleanup();  /* remote will close & retry */
				continue;
			}
			if (!(rqp = mkrequest(cfile, size))) {
				frecverr(
				"request creation failed (cf: %lu bytes)", size);
				/*NOTREACHED*/
			}
			cp = cvt_spoolfn(cp, CFILE);
			print_request(rqp, cp);	   /* does putrequest(), etc. */
			trashrequest(rqp);
			free(cfile);
			continue;

		case READDFILE:	/* read df file */
			size = strtol(cp, &blank, 10);
			if (cp = blank, *cp++ != ' ')
				break;
			if (!chksize(Rhost, size)) {
				NAK2();	   /* remote will wait and retry */
				continue;
			}
			cp = cvt_spoolfn(cp, DFILE);
			strcpy(Spoolfn, cp);
			(void) read2file(cp, size);
			continue;

		default:
			break;
		}
		errno = EINVAL;
		frecverr("protocol screwup");
		/*NOTREACHED*/
	}
}

/*
 * Convert LPD-style spool file name to SVR4 spool file name
 */
static char *
#if defined (__STDC__)
cvt_spoolfn(char *cp, int offset)
#else
cvt_spoolfn(cp, offset)
char	*cp;
int	 offset;
#endif
{
	char	 	c;
	static char	buf[MAX_SV_SPFN_SZ];

	c = *LPD_HOSTNAME(cp);
	*LPD_HOSTNAME(cp) = NULL;	/* terminate jobid string */
	(void)sprintf(buf, "%s/%d-%d", Rhost, 
				       atoi(LPD_JOBID(cp)), 
				       LPD_FILENO(cp)+offset);
	*LPD_HOSTNAME(cp) = c;
	return(buf);
} 

/*VARARGS1*/
static
#if defined (__STDC__)
frecverr(char *msg, ...)
#else
frecverr(msg, va_alist)
char	*msg;
va_dcl
#endif
{
	char	*fmt;
	char	*errstr = NULL;
	va_list  argp;

	if (errno > 0 && errno < sys_nerr) 
		errstr = PERROR; 
#if defined (__STDC__)
	va_start(argp, msg);
#else
	va_start(argp);
#endif
	(void)vsprintf(Buf, msg, argp);
	va_end(argp);
	if (errstr) {
		strcat(Buf, ": ");
		strcat(Buf, errstr);
	}
	WriteLogMsg(Buf);

	rcleanup();
	NAK1();
	done(1);
	/*NOTREACHED*/
}

static void
#if defined (__STDC__)
catch(int s)
#else
catch(s)
int	s;
#endif
{
	rcleanup();
	if (s == SIGALRM && alrmhdlr != SIG_DFL && alrmhdlr != SIG_IGN)
		(*alrmhdlr)(s);
	done(1);
	/*NOTREACHED*/
}

/*
 * Clean-up after partially submitted job
 */
static void
#if defined (__STDC__)
rcleanup(void)
#else
rcleanup()
#endif
{
	char	*cp;
	int	 n;

#ifndef DEBUG
	if (Spoolfn[0]) {
		cp = strrchr(Spoolfn, '-');
		n = atoi(++cp);
		do {
			(void)sprintf(cp, "%d", n);
			(void)unlink(Spoolfn);
		} while (n--);
	}
	cp = makepath(Lp_Requests, Spoolfn, NULL);
	(void)unlink(cp);		/* secure file also */
	free(cp);
#endif
	Spoolfn[0] = NULL;
}

/*
 * See if spool filesystem has a chance of holding incoming file
 */
static
#if defined (__STDC__)
chksize(char *dir, size_t size)
#else
chksize(dir, size)
char	*dir;
size_t	 size;
#endif
{
	register size_t	bsize;
	struct statfs	stfb;

	if (statfs(dir, &stfb, sizeof(struct statfs), 0) < 0)
		return(1);	/* ??? */
	bsize = stfb.f_bsize;
	size = (size + bsize - 1) / bsize;
	if (size > stfb.f_bfree)
		return(0);
	return(1);
}

/*
 * Convert lpd cf file to request structure
 * (sets Jobname and Jobsize as a side-effect)
 */
static REQUEST *
#if defined (__STDC__)
mkrequest(char *pcf, size_t n)
#else
mkrequest(pcf, n)
char	*pcf;
size_t	 n;
#endif
{
	register int	  i;
	register char	 *cp;
	static REQUEST	  rq;
	REQUEST          *ret = NULL;
	int	  	  copies = 0;
	int	  	  nfiles = 0;
	int		  title_set = 0;
	int	  	  reqno;
	int		  nps = 0;
	int		  banner = 0;
	char 	 	 *pend;
	char		 *class = NULL;
	char		**opts = NULL,
			**modes = NULL,
			**flist = NULL;
	struct stat	  stb;

	Jobname = NULL;				/* global */
	Jobsize = 0;				/* global */
	pend = pcf + n;
	for (cp = pcf, i=0; n--; cp++)		/* convert to strings */
		if (*cp == '\n')
			*cp = NULL;
	while(pcf < pend) {
		switch (*pcf++) {

		case JOBNAME:
			Jobname = pcf;
			break;

		case CLASS:
			class = pcf;
			break;

		case LITERAL:
			banner = 1;
			break;

		case TITLE:
			if (title_set)
				break;
			(void)sprintf(Buf, "%s'", PRTITLE);
			canonize(strchr(Buf, NULL), pcf, PRTITLE_ESCHARS);
			strcat(Buf, "'");
			appendlist(&modes, Buf);
			title_set = 1;
			break;

		case HOST:
			/* use local concept of who remote is: Rhost */
			break;

		case PERSON:
			rq.user = makestr(Rhost, "!", pcf, NULL);
			break;

		case MAILUSER:
			rq.actions |= ACT_MAIL;
			break;

		case WIDTH:
			(void)sprintf(Buf, "%s%s", WIDTHFLD, pcf);
			appendlist(&opts, Buf);
			break;

		case INDENT:
			(void)sprintf(Buf, "%s%s", IDENT, pcf);
			appendlist(&opts, Buf);
			break;

		case FILENAME:
			if (!flist) {
				(void)sprintf(Buf, "%s'", FLIST);
				appendlist(&flist, Buf);
			}
			cp = cvt_spoolfn(cp, DFILE);
			if (stat(cp, &stb) < 0)
				goto out;
			if (*pcf)
				canonize(Buf, pcf, FLIST_ESCHARS);
			else
				strcpy(Buf, "-");
			(void)sprintf(strchr(Buf, NULL), ":%lu", stb.st_size);
			Jobsize += stb.st_size;
			appendlist(&flist, Buf);
			nfiles++;
			break;

		default:
			/* Look for format lines */
			if (!copies) {
				i = *LPD_HOSTNAME(pcf);
				*LPD_HOSTNAME(pcf) = NULL;
				reqno = atoi(LPD_JOBID(pcf));
				*LPD_HOSTNAME(pcf) = i;

				switch (*(pcf-1)) {

				case FFRMTCC:
					appendlist(&modes, CATVFILTER);
					appendlist(&opts, NOFILEBREAK);
					rq.input_type = SIMPLE;
					break;

				case FPR:
					if (!title_set) { /* shouldn't happen */
						(void)sprintf(Buf, "%s''",  
								PRTITLE);
						appendlist(&modes, Buf);
						title_set = 1;
					}
					rq.input_type = SIMPLE;
					break;

				case FTROFF:
					rq.input_type = OTROFF;
					break;

				case FDITROFF:
					rq.input_type = TROFF;
					break;

				case FDVI:
					rq.input_type = TEX;
					break;

				case FGRAPH:
					rq.input_type = PLOT;
					break;

				case FCIF:
					rq.input_type = CIF;
					break;

				case FRASTER:
					rq.input_type = RASTER;
					break;

				case FFORTRAN:
					rq.input_type = FORTRAN;
					break;

				case FFRMT:
				default:
					break;
				}
			}
			if (FORMAT_LINE(*(pcf-1))) {
				if (!flist)	/* count first group only */
					copies++;
				cp = pcf;	/* remember df name */
			}
			break;
		}
		pcf += strlen(pcf) + 1;
	}
	appendlist(&flist, "'");
	if (banner) {
		if (!(cp = (char *)malloc(strlen(NB(Jobname)) + 
				  		strlen(NB(class)) + 
							sizeof(JCSEP))))
			goto out;
		(void)sprintf(cp, "%s%s%s", NB(Jobname), JCSEP, NB(class));
		rq.title = cp;          /* jobname class */
	} else
		appendlist(&opts, NOBANNER);
	mergelist(&opts, flist);
	rq.options = sprintlist(opts);
	rq.copies = copies;
	rq.destination = Printer;
	rq.modes = sprintlist(modes);
	rq.priority = -1;
	for (i=1; i<=nfiles; i++) {
		(void)sprintf(Buf, "%s/%s/%d-%d", Lp_Tmp, Rhost, reqno, i);
		appendlist(&rq.file_list, Buf);
		if (!rq.input_type && psfile(Buf))	/* PostScript file? */
				nps++;
	}
	if (!rq.input_type)
		if (nps == nfiles)	/* if one is text, they all are */
			rq.input_type = POSTSCRIPT;
		else
			rq.input_type = SIMPLE;
	ret = &rq;
out:
	if (!ret)
		trashrequest(&rq);
	if (opts)
		freelist(opts);
	if (modes)
		freelist(modes);
	if (flist)
		freelist(flist);
	return(ret);
}

/*
 * Free request structure
 */
static void
#if defined (__STDC__)
trashrequest(register REQUEST *rqp)
#else
trashrequest(rqp)
register REQUEST	*rqp;
#endif
{
	rqp->input_type = NULL;		/* since we didn't strdup() */
	rqp->destination = NULL;
	freerequest(rqp);
	memset((char *)rqp, 0, sizeof(REQUEST));
}

/*
 * Read incoming file into spool directory
 */
static
#if defined (__STDC__)
read2file(char *file, size_t size)
#else
read2file(file, size)
char	*file;
size_t	 size;
#endif
{
	int	 nr;
	int	 nrw;
	int	 fd;
	char	*cp;

	fd = open(file, O_WRONLY|O_CREAT|O_TRUNC, 0600);
	if (fd < 0) {
		frecverr(file);
		/*NOTREACHED*/
	}
	ACK();
	for (; size > 0; size -= nrw) {
		nrw = MIN(BUFSIZ, size);
		cp = Buf;
		do {
			nr = read(CIP->fd, cp, nrw);
			if (nr <= 0) {
				frecverr("Lost connection");
				/*NOTREACHED*/
			}
			nrw -= nr;
			cp += nr;
		} while (nrw > 0);
		nrw = MIN(BUFSIZ, size);
		if (write(fd, Buf, nrw) != nrw) {
			frecverr("%s: write error", file);
			/*NOTREACHED*/
		}
	}
	(void) close(fd);		/* close spool file */
	if (noresponse()) {		/* file sent had bad data in it */
		(void) unlink(file);
		return(0);
	}
	ACK();
	return(1);
}

/*
 * Read incoming file into memory buffer
 */
static
#if defined (__STDC__)
read2mem(char *pmem, size_t size)
#else
read2mem(pmem, size)
char	*pmem;
size_t	 size;
#endif
{
	int	 nr;

	ACK();
	while (size) {
		nr = read(CIP->fd, pmem, size);
		if (nr <= 0) {
			frecverr("Lost connection");
			/*NOTREACHED*/
		}
		size -= MIN(nr, size);
		pmem += nr;
	}
	if (noresponse()) 	/* file sent had bad data in it */
		return(0);
	ACK();
	return(1);
}

static
#if defined (__STDC__)
noresponse(void)
#else
noresponse()
#endif
{
	char resp;

	if (read(CIP->fd, &resp, 1) != 1) {
		frecverr("Lost connection");
		/*NOTREACHED*/
	}
	if (resp == ACKBYTE)
		return(0);
	return(1);
}

static
#if defined (__STDC__)
print_request(REQUEST *rqp, char *file)
#else
print_request(rqp, file)
REQUEST	*rqp;
char	*file;
#endif
{
	short	 status;
	long	 chkbits;
	char	*reqid;
	char	*user = rqp->user;
	SECURE	 secure;


	secure.size = Jobsize;		/* calculated by mkrequest() */
	secure.date = time(0);
	secure.system = Rhost;
	secure.user = strchr(user, '!') + 1;
	reqid = strrchr(file, '-');	/* construct req-id from file name */
	*reqid = NULL;
	secure.req_id = mkreqid(Printer, basename(file));
	*reqid = '-';			/* restore file name */
	secure.uid = secure.gid = UID_MAX + 1;
	if (putrequest(file, rqp) < 0 || putsecure(file, &secure) < 0) {
		char	*error = PERROR;

		smail(user, E_LP_PUTREQUEST, error);
		frecverr("can't putrequest: %s", error);
		/*NOTREACHED*/
	}
	free(secure.req_id);
	putmessage(Msg, S_PRINT_REQUEST, file);
	if (msend(Msg) == -1 || mrecv(Msg, MSGMAX) != R_PRINT_REQUEST) {
		smail(user, E_LP_NEEDSCHED); 
		frecverr("%s link to lpsched down", Name);
		/*NOTREACHED*/
	}
	getmessage(Msg, R_PRINT_REQUEST, &status, &reqid, &chkbits);
	if (status != MOK)
		logit(LOG_DEBUG, "%s print request failed, status = %d", 
							Printer, status);
	switch (status) {
	case MOK:
		break;
	case MNOMEM:
		smail(user, E_LP_MNOMEM);
		break;
    	case MDENYDEST:
		if (chkbits) {
			if (chkbits & PCK_TYPE)
                		smail(user, E_LP_PGONE, rqp->destination);
			else {
	    			char buf[20];

				buf[0] = NULL;
	    			if (chkbits & PCK_WIDTH) 
					strcat(buf, "-w width ");
	    			if (chkbits & PCK_BANNER) 
					strcat(buf, "-h ");
	    			smail(user, E_LP_PTRCHK, buf);
			}
		} else 
			smail(user, E_LP_DENYDEST, rqp->destination);
		break;
    	case MNOFILTER:
		smail(user, E_LP_NOFILTER);
		break;
    	case MERRDEST:
		smail(user, E_LP_REQDENY, rqp->destination);
		break;
   	case MNOOPEN:
		smail(user, E_LPP_NOOPEN);
		frecverr("lpsched can't open %s", file);	/* cleans-up */
		/*NOTREACHED*/
	default:
 		smail(user, E_LP_BADSTATUS, status);
		break;
	}
}

/*
 * Send message to remote user
 */
/*VARARGS2*/
static
#if defined (__STDC__)
smail(char *user, long msgid, ...)
#else
smail(user, msgid, va_alist)
char	*user;
long	 msgid;
va_dcl
#endif
{
	int		 p[2];
	int		 i;
	va_list		 argp;
	struct rlimit	 rl;

	if (pipe(p) < 0) {
		logit(LOG_WARNING, "%s can't create mail pipe: %s", Name, PERROR);
		return;
	}
	switch(fork()) {
	case -1:
		logit(LOG_WARNING, "%s can't fork: %s", Name, PERROR);
		return;

	case 0:
		(void)fclose(stdout);
		(void)fclose(stderr);
		(void)dup2(p[0], 0);			/*stdin*/
		(void)open("/dev/null", O_RDWR);	/*stdout*/
		(void)open("/dev/null", O_RDWR);	/*stderr*/
		if (getrlimit(RLIMIT_NOFILE, &rl) == 0)
			for (i=3; i < rl.rlim_cur; i++)
				(void)close(i);
		putenv("LOGNAME=lp");
		execl(BINMAIL, basename(BINMAIL), user, NULL);
		logit(LOG_WARNING, 
			"%s can't execl %s: %s", Name, BINMAIL, PERROR);
		exit(1);
		/*NOTREACHED*/

	default:
		fflush(stderr);
		(void)dup2(p[1], fileno(stderr)); /* _lp_msg() needs stderr */
		(void)close(p[0]);
		(void)close(p[1]);
		fprintf(stderr, "Subject: printer job\n\n");
		fprintf(stderr, "Your printer job ");
		if (Jobname && *Jobname)
			fprintf(stderr, "(%s)", Jobname);
		fprintf(stderr, "\ncould not be printed.\n");
		fprintf(stderr, "\nReason for failure:\n\n");
#if defined (__STDC__)
		va_start(argp, msgid);
#else
		va_start(argp);
#endif
		_lp_msg(msgid, argp);
		va_end(argp);
		fclose(stderr);			/* close pipe to mail */
		wait(0);
	}
}

/*
 * Check to see if printer queue is rejecting
 */
static
#if defined (__STDC__)
qdisabled(void)
#else
qdisabled()
#endif
{
	char	*pjunk;
	short	 status, pstatus;
	long	 ljunk;

	putmessage(Msg, S_INQUIRE_PRINTER_STATUS, Printer);
	if (msend(Msg) == -1 || 
	    mrecv(Msg, MSGMAX) != R_INQUIRE_PRINTER_STATUS) {
		logit(LOG_WARNING, "%s link to lpsched down", Name);
		return(1);
	}
	getmessage(Msg, R_INQUIRE_PRINTER_STATUS, &status,
						  &pjunk,
						  &pjunk,
						  &pjunk,
						  &pjunk,
						  &pjunk,
						  &pstatus,
						  &pjunk,
						  &ljunk,
						  &ljunk);
	if (status != MOK || pstatus & PS_REJECTED)
		return(1);
	return(0);
}
