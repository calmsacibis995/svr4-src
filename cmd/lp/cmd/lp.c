/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lp:cmd/lp.c	1.19.3.1"

/* lp -- print files on a line printer */

#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>
#include "requests.h"
#include "lp.h"
#include "msgs.h"
#include "printers.h"

#define WHO_AM_I	I_AM_LP
#include "oam.h"

#define TRUE 1
#define FALSE 0

char *dest = NULL;	/* destination class or printer */
struct stat stbuf;	/* Global stat buffer */
char *title = NULL;	/* User-supplied title for output */
int specialh = 0;	/* -H flag indicates special handling */
#define HOLD 1
#define RESUME 2
#define IMMEDIATE 3
char *formname = NULL;	/* form to use */
char *char_set = NULL;	/* print wheel or character set to use */
char *cont_type = NULL;	/* content type of input files */
short priority = -1;	/* priority of print request */
short copies = 0;	/* number of copies of output */
char **opts = NULL;	/* options for interface program */
char **yopts = NULL;	/* options for filter program */
char *pages = NULL;	/* pages to be printed */
short silent = FALSE;	/* don't run off at the mouth */
short mail = FALSE;	/* TRUE => user wants mail, FALSE ==> no mail */
short wrt = FALSE;	/* TRUE => user wants notification on tty via write
			   FALSE => don't write */
short raw = FALSE;	/* set option xx"stty=raw"xx and raw flag if true */
short copy = FALSE;	/* TRUE => copy files, FALSE ==> don't */
char *curdir;	/* working directory at time of request */
char *pre_rqid = NULL;	/* previos request id (-i option) */
char *reqid = NULL;	/* request id for this job */
char reqfile[20];	/* name of request file */

char **files = NULL;	/* list of file to be printed */
int nfiles = 0;		/* number of files on cmd line (excluding "-") */
int stdinp = 0;		/* indicates how many times to print std input
			   -1 ==> standard input empty		*/
char *stdinfile;
char *rfilebase;

extern char *strcpy(), *strdup(), *strchr(), *que_job(),
    *sprintlist(), *getspooldir();
extern int appendlist(), errno;

#define OPTSTRING "q:H:f:d:T:S:o:y:P:i:cmwn:st:r"

char *strcat(), *strncpy();

chk_cont_type(str)
char *str;
{
    if (STREQU(str, NAME_ANY) || STREQU(str, NAME_TERMINFO)) {
	LP_ERRMSG2(ERROR, E_LP_BADOARG, 'T', str);
	exit(1);
    }
}

main(argc, argv)
int argc;
char *argv[];
{
    int letter;
    char *p, **templist, **stemp;
    char *file;
    REQUEST *reqp, *makereq();
    int fileargs = 0;
    extern char *optarg;
    extern int optind, opterr, optopt;

    opterr = 0; /* disable printing of errors by getopt */
    while ((letter = getopt(argc, argv, OPTSTRING)) != -1)
	switch(letter) {
	case 'c':	/* copy files */
	    if (copy) LP_ERRMSG1(WARNING, E_LP_2MANY, 'c');
	    copy = TRUE;
	    break;
	case 'd':	/* destination */
	    if (dest) LP_ERRMSG1(WARNING, E_LP_2MANY, 'd');
	    dest = optarg;
	    if (!isprinter(dest) && !isclass(dest) && !STREQU(dest, NAME_ANY)) {
		LP_ERRMSG1(ERROR, E_LP_DSTUNK, dest);
		exit (1);
	    }
	    break;
	case 'f':
	    if (formname) LP_ERRMSG1(WARNING, E_LP_2MANY, 'f');
	    formname = optarg;
	    break;
	case 'H':
	    if (specialh) LP_ERRMSG1(WARNING, E_LP_2MANY, 'H');
	    if (STREQU(optarg, "hold")) specialh = HOLD;
	    else if (STREQU(optarg, "resume")) specialh = RESUME;
	    else if (STREQU(optarg, "immediate")) specialh = IMMEDIATE;
	    else {
		LP_ERRMSG2(ERROR, E_LP_BADOARG, 'H', optarg);
		exit(1);
	    }
	    break;
	case 'i':
	    if (pre_rqid) LP_ERRMSG1(WARNING, E_LP_2MANY, 'i');
	    pre_rqid = optarg;
	    break;
	case 'm':	/* mail */
	    if (mail) LP_ERRMSG1(WARNING, E_LP_2MANY, 'm');
	    mail = TRUE;
	    break;
	case 'n':	/* # of copies */
	    if (copies) LP_ERRMSG1(WARNING, E_LP_2MANY, 'n');
	    if (
		*optarg == 0
	     || (copies=(int)strtol(optarg, &p, 10)) <= 0
	     || copies > MOST_FILES
	     || *p
	    ) {
		LP_ERRMSG2(ERROR, E_LP_BADOARG, 'n', optarg);
		exit(1);
	    }
	    break;
	case 'o':	/* option for interface program */
	    stemp = templist = getlist(optarg, " \t", "");  /* MR bl88-13915 */
	    if (!stemp)
		break;			/* MR bl88-14720 */
	    while (*templist)
		appendlist(&opts, *templist++);
	    freelist(stemp);
	    break;
	case 'y':
	    stemp = templist = getlist(optarg, " \t", ",");
	    if (!stemp)
		break;			/* MR bl88-14720 */
	    while (*templist)
		appendlist(&yopts, *templist++);
	    freelist(stemp);
	    break;
	case 'P':
	    if (pages) LP_ERRMSG1(WARNING, E_LP_2MANY, 'P');
	    pages = optarg;
	    break;
	case 'q':
	    if (priority != -1) LP_ERRMSG1(WARNING, E_LP_2MANY, 'q');
	    priority = (int)strtol(optarg, &p, 10);
	    if (*p || priority<0 || priority>39) {
		LP_ERRMSG1(ERROR, E_LP_BADPRI, optarg);
	 	exit(1);
	    }
	    break;
	case 'r':
	    if (raw) LP_ERRMSG1(WARNING, E_LP_2MANY, 'r');
	    raw = TRUE;
	    break;
	case 's':	/* silent */
	    if (silent) LP_ERRMSG1(WARNING, E_LP_2MANY, 's');
	    silent = 1;
	    break;
	case 'S':
	    if (char_set) LP_ERRMSG1(WARNING, E_LP_2MANY, 'S');
	    char_set = optarg;
	    break;
	case 't':	/* title */
	    if (title) LP_ERRMSG1(WARNING, E_LP_2MANY, 't');
	    title = optarg;
	    break;
	case 'T':
	    if (cont_type) LP_ERRMSG1(WARNING, E_LP_2MANY, 'T');
	    chk_cont_type(optarg);
	    cont_type = optarg;
	    break;
	case 'w':	/* write */
	    if (wrt) LP_ERRMSG1(WARNING, E_LP_2MANY, 'w');
	    wrt = TRUE;
	    break;
	default:
	    if (optopt == '?') {
#define	P	(void) printf ("%s\n",
#define	X	);
P "usage:"								X
P ""									X
P "  (submit file(s) for printing)"					X
P "    lp [options] { file-name ... | - }"				X
P "	[-c]					(make copies first)"	X
P "	[-d destination]			(printer/class to use)"	X
P "	[-f form [-d any]]			(print on this form)"	X
P "	[-H hold]				(don't print yet)"	X
P "	[-H immediate]				(print first--reserved)"X
P "	[-m | -w]				(mail/write when done)"	X
P "	[-n copies]				(print this many copies)"X
P "	[-o nobanner]				(no banner page)"	X
P "	[-o nofilebreak]			(no inter-file formfeed)"X
P "	[-o length=scaled-number]		(page length)"		X
P "	[-o width=scaled-number]		(page width)"		X
P "	[-o lpi=scaled-number]			(line pitch)"		X
P "	[-o cpi=scaled-number]			(character pitch)"	X
P "	[-o stty='stty-options']		(port characteristics)"	X
P "	[-o other-local-options]		(as defined locally)"	X
P "	[-P page-list]				(locally defined)"	X
P "	[-q priority]				(priority level)"	X
P "	[-r]					(use no filter)"	X
P "	[-s]					(no request-id message)"X
P "	[-S char-set | print-wheel [-d any]]	(font to start with)"	X
P "	[-t title]				(title for banner page)"X
P "	[-T file-type]				(type of input files)"	X
P "	[-y local-modes]			(locally defined)"	X
P ""									X
P "  (change previous request)"						X
P "    lp -i request-id {options}"					X
P "	[-H resume]				(resume held request)"	X
P "	[other options listed above]"					X
		exit(0);
	    }
	    (p = "-X")[1] = optopt;
	    if (strchr(OPTSTRING, optopt))
		LP_ERRMSG1(ERROR, E_LP_OPTARG, p);
	    else
		LP_ERRMSG1(ERROR, E_LP_OPTION, p);
	    exit(1);
	}

    if (mail && wrt) LP_ERRMSG(WARNING, E_LPP_COMBMW);

    while (optind < argc) {
	fileargs++;
	file = argv[optind++];
	if(strcmp(file, "-") == 0) {
	    stdinp++;
	    appendlist(&files, file);
	} else {
	    if(Access(file, 4/*read*/) || Stat(file, &stbuf))
		LP_ERRMSG2(WARNING, E_LP_BADFILE, file, errno);
	    else if((stbuf.st_mode & S_IFMT) == S_IFDIR)
		LP_ERRMSG1(WARNING, E_LP_ISDIR, file);
	    else if(stbuf.st_size == 0)
		LP_ERRMSG1(WARNING, E_LP_EMPTY, file);
	    else {
		nfiles++;
		appendlist(&files, file);
		continue;
	    }
	}
    }

    if(fileargs == 0) {
	if (!pre_rqid) stdinp = 1;
    } else if (pre_rqid) {
	LP_ERRMSG(ERROR, E_LPP_ILLARG);
	exit(1);
    } else if(nfiles == 0 && stdinp == 0) {
	LP_ERRMSG(ERROR, E_LP_NOFILES);
	exit(1);
    }

/* args parsed, now let's do it */

    startup();	/* open message queue
		and catch interupts so it gets closed too */

    if (!(reqp = makereq())) {	/* establish defaults & sanity check args */
	LP_ERRMSG1(ERROR, E_LPP_FGETREQ, pre_rqid);
	err_exit();
    }

    /* allocate files for request, standard input and files if copy */
    if (pre_rqid) {
	if (putrequest(reqfile, reqp) == -1) {	/* write request file */
puterr:
	    switch(errno) {
	    default:
		LP_ERRMSG(ERROR, E_LPP_FPUTREQ);
		err_exit();
	    }
	}
	end_change(pre_rqid, reqp);
	reqid = pre_rqid;
    } else {
	allocfiles();
	if(stdinp > 0) {
	    savestd();	/* save standard input */
	}
	reqp->file_list = files;
	if (putrequest(reqfile, reqp) == -1) goto puterr;
	reqid = que_job(reqp);
    }

    signal(SIGHUP, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);

    clean_up();
    ack_job();		/* issue request id message */

    exit(0);
    return(0);
}
/* startup -- initialization routine */

startup()
{
#if	defined(__STDC__)
    void catch();
#else
    int catch();
#endif
int	try = 0;
    char *getcwd();

    for (;;)
	if (mopen() == 0) break;
	else {
	    if (errno == ENOSPC && try++ < 5) {
		sleep(3);
		continue;
	    }
	    LP_ERRMSG(ERROR, E_LP_MOPEN);
	    exit(1);
	}

    if(signal(SIGHUP, SIG_IGN) != SIG_IGN)
	signal(SIGHUP, catch);
    if(signal(SIGINT, SIG_IGN) != SIG_IGN)
	signal(SIGINT, catch);
    if(signal(SIGQUIT, SIG_IGN) != SIG_IGN)
	signal(SIGQUIT, catch);
    if(signal(SIGTERM, SIG_IGN) != SIG_IGN)
	signal(SIGTERM, catch);

    umask(0000);
    curdir = getcwd(NULL, 512);
}

/* catch -- catch signals */

#if	defined(__STDC__)
void
#endif
catch()
{
    signal(SIGHUP, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    err_exit(1);
}

/* clean_up -- called by catch() after interrupts
   or by err_exit() after errors */

clean_up()
{
    (void)mclose ();
}

err_exit()
{
    clean_up();
    exit(1);
}

/*
 * copyfile(stream, name) -- copy stream to file "name"
 */

copyfile(stream, name)
FILE *stream;
char *name;
{
    FILE *ostream;
    int i;
    char buf[BUFSIZ];

    if((ostream = fopen(name, "w")) == NULL) {
	LP_ERRMSG2(ERROR, E_LP_BADFILE, name, errno);
	return;
    }

    Chmod(name, 0600);
    while((i = fread(buf, sizeof(char), BUFSIZ, stream)) > 0) {
	fwrite(buf, sizeof(char), i, ostream);
	if(feof(stream)) break;
    }

    fclose(ostream);
}
/* makereq -- sanity check args, establish defaults */

REQUEST *
makereq()
{
    static REQUEST rq;
    REQUEST *oldrqp;
    char *getenv(), *preqfile;
    char **optp, *opt, buf[16], *pdest = dest, *start_ch();
    char *p;
    long errflg;

    if (!dest && !pre_rqid && !cont_type) {
	if ((dest = getenv("LPDEST")) && *dest)
	    ;
	else {
	    if (!(dest = getdefault())) {
		LP_ERRMSG(ERROR, E_LPP_NODEST);
		err_exit();
	    }
	}
    }
    if (!dest) dest = "any";

    if (!pre_rqid && !cont_type)
	cont_type = getenv("LPTYPE");
    if (!pre_rqid && !cont_type)
	cont_type = NAME_SIMPLE;

    if (formname && opts)
	for (optp = opts; *optp; optp++)
	    if (STRNEQU("lpi=", *optp, 4)
	     || STRNEQU("cpi=", *optp, 4)
	     || STRNEQU("length=", *optp, 7)
	     || STRNEQU("width=", *optp, 6)) {
		LP_ERRMSG(ERROR, E_LP_OPTCOMB);
		err_exit();
	    }

    if (raw && (yopts || pages)) {
	LP_ERRMSG(ERROR, E_LP_OPTCOMB);
	err_exit();
    }

    /* now to build the request */
    if (pre_rqid) {
	preqfile = start_ch(pre_rqid);
	strcpy(reqfile, preqfile);
	if (!(oldrqp = getrequest(preqfile))) return (NULL);
	rq.copies = (copies) ? copies : oldrqp->copies;
	rq.destination = (pdest) ? dest : oldrqp->destination;
	rq.file_list = oldrqp->file_list;
	rq.form = (formname) ? formname : oldrqp->form;
	rq.actions = (specialh) ? ((specialh == HOLD) ? ACT_HOLD :
	    ((specialh == RESUME) ? ACT_RESUME : ACT_IMMEDIATE)) :
	    oldrqp->actions;
	if (wrt) rq.actions |= ACT_WRITE;
	if (mail) rq.actions |= ACT_MAIL;
	if (raw) {
	    rq.actions |= ACT_RAW;
	    /*appendlist(&opts, "stty=raw");*/
	}
	rq.options = (opts) ? sprintlist(opts) : oldrqp->options;
	rq.priority = (priority == -1) ? oldrqp->priority : priority;
	rq.pages = (pages) ? pages : oldrqp->pages;
	rq.charset = (char_set) ? char_set : oldrqp->charset;
	rq.modes = (yopts) ? sprintlist(yopts) : oldrqp->modes;
	rq.title = (title) ? title : oldrqp->title;
	rq.input_type = (cont_type) ? cont_type : oldrqp->input_type;
	rq.user = oldrqp->user;
	rq.outcome = 0;
	return(&rq);
    }
    rq.copies = (copies) ? copies : 1;
    rq.destination = dest;
    rq.form = formname;
    rq.actions = (specialh) ? ((specialh == HOLD) ? ACT_HOLD : 
	((specialh == RESUME) ? ACT_RESUME : ACT_IMMEDIATE)) : 0;
    if (wrt) rq.actions |= ACT_WRITE;
    if (mail) rq.actions |= ACT_MAIL;
    if (raw) {
	rq.actions |= ACT_RAW;
	/*appendlist(&opts, "stty=raw");*/
    }
    rq.alert = NULL;
    rq.options = sprintlist(opts);
    rq.priority = priority;
    rq.pages = pages;
    rq.charset = char_set;
    rq.modes = sprintlist(yopts);
    rq.title = title;
    rq.input_type = cont_type;
    rq.file_list = 0;
    rq.user = getname();
    return(&rq);
}

/* files -- process command line file arguments */

allocfiles()
{
    char **reqfiles = 0, **filep, *p, *getfiles(), *prefix;
    FILE *f;
    int numfiles, filenum = 1;

    numfiles = 1 + ((stdinp > 0) ? 1 : 0) + ((copy) ? nfiles : 0);

    if ((prefix = getfiles(numfiles)) == NULL)
    {
	numfiles += nfiles;
	prefix = getfiles(numfiles);
	copy = 1;
    }
    
    strcpy(reqfile, prefix);
    strcat(reqfile, "-0000");
    rfilebase = makepath(Lp_Temp, reqfile, NULL);
    if (stdinp > 0) {
	stdinfile = strdup(rfilebase);
	p = strchr(stdinfile, 0) - 4;
	*p++ = '1';
	*p = 0;
	filenum++;
    }
    p = strchr(reqfile, 0) - 4; *p++ = '0'; *p = 0;
    p = strchr(rfilebase, 0) - 4;

    if (!files) appendlist(&files, "-");

    for (filep = files; *filep; filep++) {
	if(STREQU(*filep, "-")) {
	    if(stdinp > 0)
		appendlist(&reqfiles, stdinfile);
	} else {
	    if(copy) {
		if (f = fopen(*filep, "r")) {
		    sprintf(p, "%d", filenum++);
		    copyfile(f, rfilebase);
		    appendlist(&reqfiles, rfilebase);
		    fclose(f);
		} else
		    LP_ERRMSG2(WARNING, E_LP_BADFILE, *filep, errno);
	    } else {
		if (**filep == '/' || (curdir && *curdir))
		    appendlist(&reqfiles,
			(**filep == '/') ? *filep
				: makepath(curdir, *filep, (char *)0));
		else {
		    LP_ERRMSG (ERROR, E_LPP_CURDIR);
		    err_exit ();
		}
	    }
	}
    }
    freelist(files);
    files = reqfiles;
    return(1);
}

/* start_ch -- start change request */
char *
start_ch(rqid)
char *rqid;
{
    int size, type;
    short status;
    char message[100],
	 reply[100],
	 *rqfile;

    size = putmessage(message, S_START_CHANGE_REQUEST, rqid);
    assert(size != -1);
    if (msend(message)) {
	LP_ERRMSG(ERROR, E_LP_MSEND);
	err_exit();
    }
    if ((type = mrecv(reply, 100)) == -1) {
	LP_ERRMSG(ERROR, E_LP_MRECV);
	err_exit();
    }
    if (type != R_START_CHANGE_REQUEST
	   || getmessage(reply, type, &status, &rqfile) == -1) {
	LP_ERRMSG1(ERROR, E_LP_BADREPLY, type);
	err_exit();
    }

    switch (status) {
    case MOK:
	return(strdup(rqfile));
    case MNOPERM:
	LP_ERRMSG(ERROR, E_LP_NOTADM);
	break;
    case MUNKNOWN:
	LP_ERRMSG1(ERROR, E_LP_UNKREQID, rqid);
	break;
    case MBUSY:
	LP_ERRMSG1(ERROR, E_LP_BUSY, rqid);
	break;
    case M2LATE:
	LP_ERRMSG1(ERROR, E_LP_2LATE, rqid);
	break;
    case MGONEREMOTE:
	LP_ERRMSG1(ERROR, E_LP_GONEREMOTE, reqid);
	break;
    default:
	LP_ERRMSG1(ERROR, E_LP_BADSTATUS, status);
    }
    err_exit();
}

end_change(rqid, rqp)
char *rqid;
REQUEST *rqp;
{
    int size, type;
    long chkbits;
    short status;
    char message[100],
	 reply[100],
	 *chkp,
	 *rqfile;

    size = putmessage(message, S_END_CHANGE_REQUEST, rqid);
    assert(size != -1);
    if (msend(message)) {
	LP_ERRMSG(ERROR, E_LP_MSEND);
	err_exit();
    }
    if ((type = mrecv(reply, 100)) == -1) {
	LP_ERRMSG(ERROR, E_LP_MRECV);
	err_exit();
    }
    if (type != R_END_CHANGE_REQUEST
	   || getmessage(reply, type, &status, &chkbits) == -1) {
	LP_ERRMSG1(ERROR, E_LP_BADREPLY, type);
	err_exit();
    }

    switch (status) {
    case MOK:
	return(1);
    case MNOPERM:
	LP_ERRMSG(ERROR, E_LP_NOTADM);
	break;
    case MNOSTART:
	LP_ERRMSG(ERROR, E_LPP_NOSTART);
	break;
    case MNODEST:
	LP_ERRMSG1(ERROR, E_LP_DSTUNK, rqp->destination);
	break;
    case MDENYDEST:
	if (chkbits) {
	    chkp = message;
		/* PCK_TYPE indicates a Terminfo error, and should */
		/* be handled as a ``get help'' problem.	   */
	    if (chkbits & PCK_TYPE) chkp += sprintf(chkp, "");
	    if (chkbits & PCK_CHARSET) chkp += sprintf(chkp, "-S character-set, ");
	    if (chkbits & PCK_CPI) chkp += sprintf(chkp, "-o cpi=, ");
	    if (chkbits & PCK_LPI) chkp += sprintf(chkp, "-o lpi=, ");
	    if (chkbits & PCK_WIDTH) chkp += sprintf(chkp, "-o width=, ");
	    if (chkbits & PCK_LENGTH) chkp += sprintf(chkp, "-o length=, ");
	    if (chkbits & PCK_BANNER) chkp += sprintf(chkp, "-o nobanner, ");
	    chkp[-2] = 0;
	    LP_ERRMSG1(ERROR, E_LP_PTRCHK, message);
	}
	else LP_ERRMSG1(ERROR, E_LP_DENYDEST, rqp->destination);
	break;
    case MNOMEDIA:
	LP_ERRMSG(ERROR, E_LPP_NOMEDIA);
	break;
    case MDENYMEDIA:
	if (chkbits & PCK_CHARSET) LP_ERRMSG(ERROR, E_LPP_FORMCHARSET);
	else LP_ERRMSG1(ERROR, E_LPP_DENYMEDIA, rqp->form);
	break;
    case MNOMOUNT:
	LP_ERRMSG(ERROR, E_LPP_NOMOUNT);
	break;
    case MNOFILTER:
	LP_ERRMSG(ERROR, E_LP_NOFILTER);
	break;
    case MERRDEST:
	LP_ERRMSG1(ERROR, E_LP_REQDENY, rqp->destination);
	break;
    case MNOOPEN:
	LP_ERRMSG(ERROR, E_LPP_NOOPEN);
	break;
    default:
	LP_ERRMSG1(ERROR, E_LP_BADSTATUS, status);
    }
    err_exit();
}
/* getfile -- allocate the requested number of temp files */

char *
getfiles(number)
int number;
{
    int size, type;
    short status;
    char message[100],
	 reply[100],
	 *pfix;

    size = putmessage(message, S_ALLOC_FILES, number);
    assert(size != -1);
    if (msend(message)) {
	LP_ERRMSG(ERROR, E_LP_MSEND);
	err_exit();
    }
    if ((type = mrecv(reply, 100)) == -1) {
	LP_ERRMSG(ERROR, E_LP_MRECV);
	err_exit();
    }
    if (type != R_ALLOC_FILES
	   || getmessage(reply, type, &status, &pfix) == -1) {
	LP_ERRMSG1(ERROR, E_LP_BADREPLY, type);
	err_exit();
    }

    switch (status) {
    case MOK:
	return(strdup(pfix));
    case MOKREMOTE:
	clean_up();
	startup();
	return(NULL);
    case MNOMEM:
	LP_ERRMSG(ERROR, E_LP_NOSPACE);
	break;
    default:
	LP_ERRMSG1(ERROR, E_LP_BADSTATUS, status);
    }
    err_exit();
}

char *
que_job(rqp)
REQUEST *rqp;
{
    int size, type;
    long chkbits;
    short status;
    char message[100],
	 reply[100],
	 *chkp,
	 *req_id;

    size = putmessage(message, S_PRINT_REQUEST, reqfile);
    assert(size != -1);
    if (msend(message)) {
	LP_ERRMSG(ERROR, E_LP_MSEND);
	err_exit();
    }
    if ((type = mrecv(reply, 100)) == -1) {
	LP_ERRMSG(ERROR, E_LP_MRECV);
	err_exit();
    }
    if (type != R_PRINT_REQUEST
	   || getmessage(reply, type, &status, &req_id, &chkbits) == -1) {
	LP_ERRMSG1(ERROR, E_LP_BADREPLY, type);
	err_exit();
    }

    switch (status) {
    case MOK:
	return(strdup(req_id));
    case MNOPERM:
	LP_ERRMSG(ERROR, E_LP_NOTADM);
	break;
    case MNODEST:
	LP_ERRMSG1(ERROR, E_LP_DSTUNK, rqp->destination);
	break;
    case MDENYDEST:
	if (chkbits) {
	    chkp = message;
		/* PCK_TYPE indicates a Terminfo error, and should */
		/* be handled as a ``get help'' problem.	   */
	    if (chkbits & PCK_TYPE) chkp += sprintf(chkp, "");
	    if (chkbits & PCK_CHARSET) chkp += sprintf(chkp, "-S character-set, ");
	    if (chkbits & PCK_CPI) chkp += sprintf(chkp, "-o cpi=, ");
	    if (chkbits & PCK_LPI) chkp += sprintf(chkp, "-o lpi=, ");
	    if (chkbits & PCK_WIDTH) chkp += sprintf(chkp, "-o width=, ");
	    if (chkbits & PCK_LENGTH) chkp += sprintf(chkp, "-o length=, ");
	    if (chkbits & PCK_BANNER) chkp += sprintf(chkp, "-o nobanner, ");
	    chkp[-2] = 0;
	    LP_ERRMSG1(ERROR, E_LP_PTRCHK, message);
	}
	else LP_ERRMSG1(ERROR, E_LP_DENYDEST, rqp->destination);
	break;
    case MNOMEDIA:
	LP_ERRMSG(ERROR, E_LPP_NOMEDIA);
	break;
    case MDENYMEDIA:
	if (chkbits & PCK_CHARSET) LP_ERRMSG(ERROR, E_LPP_FORMCHARSET);
	else LP_ERRMSG1(ERROR, E_LPP_DENYMEDIA, rqp->form);
	break;
    case MNOMOUNT:
	LP_ERRMSG(ERROR, E_LPP_NOMOUNT);
	break;
    case MNOFILTER:
	LP_ERRMSG(ERROR, E_LP_NOFILTER);
	break;
    case MERRDEST:
	LP_ERRMSG1(ERROR, E_LP_REQDENY, rqp->destination);
	break;
    case MNOOPEN:
	LP_ERRMSG(ERROR, E_LPP_NOOPEN);
	break;
    case MUNKNOWN:
	LP_ERRMSG(ERROR, E_LPP_ODDFILE);
	break;
    default:
	LP_ERRMSG1(ERROR, E_LP_BADSTATUS, status);
    }
    err_exit();
}
/* ack_job -- issue request id message */

ack_job()
{
    if(silent || pre_rqid) return;
    printf("request id is %s (", reqid);
    if(nfiles > 0) {
	printf("%d file", nfiles);
	if(nfiles > 1)
	    printf("s");
    }
    if(stdinp > 0) {
	if(nfiles > 0)
	    printf(" and ");
	printf("standard input");
    }
    printf(")\n");
}
/* savestd -- save standard input */

savestd()
{
    copyfile(stdin, stdinfile);
    Stat(stdinfile, &stbuf);
    if(stbuf.st_size == 0) {
	if(nfiles == 0) {
	    LP_ERRMSG(ERROR, E_LP_NOFILES);
	    err_exit();
	} else	{/* inhibit printing of std input */
	    LP_ERRMSG1(WARNING, E_LP_EMPTY, "(standard input)");
	    stdinp = -1;
	}
    }
}
