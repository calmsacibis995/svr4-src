/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ident	"@(#)lp:cmd/lpsched/lpNet/bsdChild/printreq.c	1.4.3.1"

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "lp.h"			/* includes fcntl.h sys/types.h sys/stat.h */
#include "lpNet.h"
#include "requests.h"
#include "printers.h"
#include "secure.h"
#include "lpd.h"

#if defined (__STDC__)
static	char	* mkcfile(char *, REQUEST *);
static	char	* r_print_request(int, char *, int);
static	int	  card(char **, char, char *);
static	int	  parselpd(char *, char *, char **);
static	int	  sendfile(char, char *, char *, int);
#else
static	char	* mkcfile();
static	char	* r_print_request();
static	int	  card();
static	int	  parselpd();
static	int	  sendfile();
#endif

/*
 * Send print request to remote lpd
 * (S_PRINT_REQUEST processing)
 */
char *
#if defined (__STDC__)
s_print_request(char *msg)
#else
s_print_request(msg)
char	*msg;
#endif
{
	REQUEST	 *rp;
	SECURE	 *sp = NULL;
	PRINTER	 *pr = NULL;
	char	 *rqfile;
	char	 *cf = NULL;
	char	 *num;
	char	**lpnm, lpdnm[MAX_LPD_SPFN_SZ];
	char	 *jobid;
	short	  status;
	int	  n;

        (void)getmessage(msg, S_PRINT_REQUEST, &rqfile);
	logit(LOG_DEBUG, "S_PRINT_REQUEST(rqfile=\"%s\")", rqfile);
	if (!(rp = getrequest(rqfile)) || !(sp = getsecure(rqfile))) {
		status = MNOOPEN;
		goto out;
	}
	if (!(pr = getprinter(rp->destination)) || !pr->remote) {
		status = MNOMEM;
		goto out;
	}
	if (Printer = strchr(pr->remote, '!'))
		Printer++;
	else
		Printer = rp->destination;
	if (!(cf = mkcfile(sp->req_id, rp))) {
		status = MNOMEM;
		goto out;
	}
	if (!snd_lpd_msg(RECVJOB, Printer) || response() != 0) {	
		status = REPRINT;
		goto out;
	}
	jobid = rid2jid(sp->req_id);
	for (n=0, lpnm = rp->file_list; n<MAX_LPD_FILES && *lpnm; n++, lpnm++) {
		sprintf(lpdnm, "df%c%s%s", LPD_FILEID(n), jobid, Lhost);
		if ((status = sendfile(READDFILE, *lpnm, lpdnm, READ_FILE)) 
									!= MOK)
			goto out;
	}
	sprintf(lpdnm, "%s%s%s", CFPREFIX, jobid, Lhost);
	status = sendfile(READCFILE, cf, lpdnm, READ_BUF);
	closeRemote();
	if (openRemote())
		(void)snd_lpd_msg(PRINTJOB, Printer);
out:
	freerequest(rp);
	if (cf)
		free(cf);
	freeprinter(pr);
	if (status == REPRINT)
		cf = (char *)NULL;
	else
		cf = r_print_request(status, (sp ? sp->req_id : ""), 0);
	freesecure(sp);
	return(cf);
}

static 
struct fmt_map fmt_map[] =  {
	OTROFF,		FTROFF,
	TROFF,		FDITROFF,
	TEX,		FDVI,
	PLOT,		FGRAPH,
	RASTER,		FRASTER,
	CIF,		FCIF,
	FORTRAN,	FFORTRAN,
	SIMPLE,		FFRMT,
	POSTSCRIPT,	FFRMT,
	"",		FFRMT,
	NULL,		'\0'
};

static char *
#if defined (__STDC__)
mkcfile(char *reqid, REQUEST *rp)
#else
mkcfile(reqid, rp)
char	*reqid;
REQUEST	*rp;
#endif
{
	int	 	  n, nfiles;
	int		  i;
	char	 	  fmt = '\0';
	char		 *cp;
	char		 *cf = NULL;
	char		 *rval = NULL;
	char		 *flist;
	char		 *lpdargs;
	char		 *files[MAX_LPD_FILES];
	char		 *num;
	char		 *fonts[4];	/* troff fonts */
	char		**options, **modes;
	char		 *argv[sizeof(LPDOPTS)];
	struct fmt_map	 *fmap;

/* for readability...  (be careful nesting in if-else) */
#define CARD(k,s)	if (!card(&cf, k, s)) goto out

	options = dashos(rp->options);
	modes = dashos(rp->modes);
	CARD(HOST, Lhost);
	CARD(PERSON, rp->user);
	nfiles = lenlist(rp->file_list);
	nfiles = MIN(nfiles, MAX_LPD_FILES);
	if ((flist = find_listfld(FLIST, options)) &&
	    parseflist(flist+STRSIZE(FLIST), nfiles, files, NULL) != nfiles)
		goto out;
	cp = NULL;
	if (lpdargs = find_listfld(LPDFLD, options)) {
		parselpd(lpdargs+STRSIZE(LPDFLD), LPDOPTS, argv);
		if (argv[JOB_IDX])
			cp = argv[JOB_IDX];
	}
	if (!cp)
		if (!lpdargs && rp->title)	/* only use title if from lp */
			cp = rp->title;
		else if (flist)
			if (*files[0])
				cp = basename(files[0]);
			else
				cp = "stdin";
		else
			cp = reqid;
	CARD(JOBNAME, cp);
	if (lpdargs && argv[CLASS_IDX]) {
		CARD(CLASS, argv[CLASS_IDX]);
	} else
		CARD(CLASS, Lhost);
	if (!find_listfld(NOBANNER, options))
		CARD(LITERAL, rp->user);
	if (cp = find_listfld(IDENT, modes))
		CARD(INDENT, cp+STRSIZE(IDENT));
	if (rp->actions & ACT_MAIL)
		CARD(MAILUSER, rp->user);
	if (cp = find_listfld(WIDTHFLD, options))
		CARD(WIDTH, cp+STRSIZE(WIDTHFLD));
	if (cp = find_listfld(PRTITLE, modes)) {
		rmesc(cp += STRSIZE(PRTITLE));
		fmt = FPR;
	   	CARD(TITLE, cp);
	}
	for (fmap = fmt_map; !fmt && fmap->type; fmap++)
		if (STREQU(rp->input_type, fmap->type))
			fmt = fmap->keyc;
	if (fmt == FFRMT && 
	    find_listfld(CATVFILTER, modes) &&
	    find_listfld(NOFILEBREAK, options))
		fmt = FFRMTCC;
	else if (!fmt)
		goto out;
	if (lpdargs && (fmt == FTROFF || fmt == FDITROFF || fmt == FDVI)) {
		if (argv[FONT1_IDX] && *argv[FONT1_IDX])
			CARD(FONTR, argv[FONT1_IDX]);
		if (argv[FONT2_IDX] && *argv[FONT2_IDX])
			CARD(FONTI, argv[FONT2_IDX]);
		if (argv[FONT3_IDX] && *argv[FONT3_IDX])
			CARD(FONTB, argv[FONT3_IDX]);
		if (argv[FONT4_IDX] && *argv[FONT4_IDX])
			CARD(FONTS, argv[FONT4_IDX]);
	}
	sprintf(Buf, "%sA%s%s", DFPREFIX, rid2jid(reqid), Lhost);
	for (n = 0; n < nfiles; n++) {
		LPD_FILEX(Buf) = LPD_FILEID(n);
		for (i = rp->copies; i; i--)
			CARD(fmt, Buf);
		CARD(UNLINK, Buf);
		if (flist) {
			CARD(FILENAME, *files[n] ? files[n] :"standard input");
		} else
			CARD(FILENAME, NO_FILENAME);
	}
	rval = cf;
	cf = NULL;
out:
	if (options) freelist(options);
	if (modes) freelist(modes);
	if (cf) free(cf);
	return(rval);
}

static
#if defined (__STDC__)
sendfile(char type, char *buf, char *lpdnm, int flag)
#else
sendfile(type, buf, lpdnm, flag)
char	 type;
char	*buf;
char	*lpdnm;
int	 flag;		/* READ_FILE, READ_BUF */
#endif
{
	int		error;
	size_t		size;
	int		fd = -1;
	int		nrw;
	struct stat	stb;

	if (flag == READ_FILE) {	/* use file as input */
		if (stat(buf, &stb) < 0 || (fd = open(buf, O_RDONLY)) < 0) {
			logit(LOG_WARNING, "can't open spool file: %s", buf);
			return(MUNKNOWN);
		}
		size = stb.st_size;
		buf = Buf;
	} else				/* READ_BUF: use buffer as input */
		size = strlen(buf);
	if (!snd_lpd_msg(RECVJOB_2NDARY, type, size, lpdnm))
		return(REPRINT);
	if (response() != 0)		/* try harder ??? (sendfile) */
		return(REPRINT);
	error = 0;
	for (; size > 0; size -= nrw) {
		nrw = MIN(size, BUFSIZ);
		if (flag == READ_FILE) {
			if (!error && read(fd, buf, nrw) != nrw)
				error = errno ? errno : -1;
		}
		if (write(CIP->fd, buf, nrw) != nrw) {
			if (fd >= 0) 
				close(fd);	/* close spool file */
			logit(LOG_INFO, "lost connection");
			return(REPRINT);
		}
		if (flag == READ_BUF)
			buf += BUFSIZ;
	}
	if (fd >= 0) 
		close(fd);			/* close spool file */
	if (error) {
		NAK1();
		logit(LOG_WARNING, "spool file read error (%d)", error);
		return(REPRINT);
	}
	if (!ACK_SENT() || response())
		return(REPRINT);
	return(MOK);
}

/*
 * Return network response character
 */
#if defined (__STDC__)
response(void)
#else
response()
#endif
{
	char	resp;

	if (read(CIP->fd, &resp, 1) != 1) {
		logit(LOG_INFO, "%s: lost connnection", Printer);
		return(-1);
	} else
		if (resp)
			logit(LOG_INFO, "NAKed by remote lpd (%d)", resp);
		return(resp);
}

static
#if defined (__STDC__)
parselpd(char *argp, char *opts, char **argv)
#else
parselpd(argp, opts, argv)
char	 *argp;
char	 *opts;
char	**argv;
#endif
{
	char	*p;

	for (p = opts; *p; p++)
		argv[p-opts] = NULL;
	for (argp = getitem(argp, '-'); argp; argp = getitem(NULL, '-')) {
		for (p = opts; *p; p++)
			if (argp[0] == *p) {
				argv[p-opts] = &argp[1];
				break;
			}
	}
}

static
#if defined (__STDC__)
card(char **pbuf, char key, char *string)
#else
card(pbuf, key, string)
char	**pbuf;
char	  key;
char	 *string;
#endif
{
	int	 	 n;
	static char	*buf;		/* current buffer	  */
	static char	*pcur;		/* current buffer pointer */
	static int	 bufsize;	/* current buffer size	  */

	n = strlen(string) + 3;		/* key + string + \n + NULL */
	if (!*pbuf) {
		if (!(buf = (char *)malloc(bufsize = CFSIZE_INIT)))
			return(0); 
		pcur = *pbuf = buf;
	} else
		if (pcur + n > buf + bufsize) {
			if (!(buf = 
				(char *)realloc(buf, bufsize += CFSIZE_INC))) {
				free(*pbuf);		/* no looking back */
				*pbuf = NULL;
				return(0);
			}
			pcur = *pbuf = buf;
		}
	logit(LOG_DEBUG, "cf card: %c%s", key, string);
	sprintf(pcur, "%c%s\n", key, string);
	pcur += n-1;			/* position to overwrite NULL byte */
	return(1);
}

static char *
#if defined (__STDC__)
r_print_request(int status, char *reqid, int chkbits)
#else
r_print_request(status, reqid, chkbits)
int	 status;
char	*reqid;
int	 chkbits;
#endif
{
	logit(LOG_DEBUG, "R_PRINT_REQUEST(%d, \"%s\")", status, reqid);
	if (putmessage(Msg, R_PRINT_REQUEST, status, reqid, chkbits) < 0)
		return(NULL);
	else
		return(Msg);
}
