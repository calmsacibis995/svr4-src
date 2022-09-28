/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lp:cmd/bsd/lpr/lpr.c	1.4.2.1"

/* lpr -- print files on a line printer */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <filehdr.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>
#include <pwd.h>
#include "requests.h"
#include "lp.h"
#include "msgs.h"
#include "printers.h"
#define WHO_AM_I	I_AM_OZ
#include "oam_def.h"
#include "lpd.h"

#define	OMAGIC  0407            /* old impure format */
#define NMAGIC  0410            /* read-only text */
#define ZMAGIC  0413            /* demand load format */
#define ARMAG  0177545		/* pdp11/pre System V ar archive */

#define TRUE	1
#define FALSE	0

#define OPT_SIZE	2		/* Size of options is 2 (ex: -P )*/
#define MAX_FILE_SIZE	20	


struct stat	  stbuf;	/* Global stat buffer */
char		 *cont_type;  	/* file content type */
short		  copies;	/* number of copies of output */
char		**opts;		/* options for interface program */
char		**yopts;	/* options for filter program */
short		  mail = FALSE;	/* TRUE => user wants mail, FALSE ==> no mail */
short		  copy = TRUE;	/* TRUE => copy files, FALSE ==> don't */
char		 *curdir;	/* working directory at time of request */
char		 *reqid;	/* request id for this job */
char		  reqfile[20];	/* name of request file */
char		**files;	/* list of files to be printed */
int		  nfiles;	/* number of files on cmd line */
int		  Nps;		/* number of input files that are postscript */
int		  stdinp;	/* flag that indicates to print stdin */
char		 *stdinfile;
char		 *rfilebase;

char		 *Title;	/* pr title */
char		 *Class;	/* Replaces system name */
char		 *Jobname;	/* Replaces filename for LPD Compatibility */

char		 *firstfile;  	/* first file name for banner */

char		**lpdlist;	/* The lpd="" list */

char		  Msg[MSGMAX];	/* buffer used for lpsched communication */

int		  cflag,   jflag;
int		  prflag,  tflag;

char		 *Person;	/* user name */

#if defined(__STDC__)
static	REQUEST	* makereq(void);
static	char	* getfiles(int);
static	char	* que_job(REQUEST *);
static	int	  allocfiles(void);
static	int 	  test(char *);
static	size_t	  savestdin(void);
static	void 	  ack_job(void);
static	void 	  appendlpd(char ***, char *, char *);
static	void 	  copyfile(FILE *, char *);
static	void 	  escape(char *, char **, char *);
static	void 	  startup(void);
	void	  done(int);
#else
static	REQUEST	* makereq();
static	char	* getfiles();
static	char	* que_job();
static	int	  allocfiles();
static	int 	  test();
static	size_t	  savestdin();
static	void 	  ack_job();
static	void 	  appendlpd();
static	void 	  copyfile();
static	void 	  escape();
static	void 	  startup();
	void	  done();
#endif

main(argc, argv)
int	  argc;
char	**argv;
{
	
    	int		 i;
    	int		 fileargs = 0;
	int		 insize;
    	char		*file;
	char		*cptr;
	char		*newstr;
	register char	*arg;
	register char	*argval;
    	REQUEST		*reqp;
	struct passwd	*p;
	char		*basename();

	Name = argv[0];
	if (!(Person = getlogin()) ||
	    !(p = getpwnam(Person))) {
		if (!(p = getpwuid(getuid())))
			fatal("Who are you?");
		Person = p->pw_name;
	}
	setuid(p->pw_uid);	/* so lpsched knows who this really is */
	while(argc > 1 && argv[1][0] == '-') {
		argc--;
		arg = *++argv;
		argval = NULL;
		
		switch( arg[1] ) {

		case 'P':	/* destination */
			if (arg[2])
				argval = Printer = &arg[2];
			else if (argc > 1) {
				argc--; 
				argval = Printer = *++argv;
			}
	    		break;

		case '#':	/* # of copies */
			if (isdigit(arg[2])) {
				argval = &arg[2];
				i = atoi( &arg[2] );
				if (i>0)
					copies = i;
				
	    		}
	    		break;

		case 'C':	/* option for interface program */
			cflag = 1;
			if (arg[2])
				argval = Class = &arg[2];	
			else if (argc > 1) {
				argc--;
				argval = Class = *++argv;
			}
			if (strpbrk(Class, TITLE_ESCHARS)) {
				escape(Class, &newstr, TITLE_ESCHARS);
				Class = newstr;
			}
	    		break;

		case 'J':
			jflag = 1;
			if (arg[2])
				argval = Jobname = &arg[2];	
			else if (argc > 1) {
				argc--;
				argval = Jobname = *++argv;
			}
			if (strpbrk(Jobname, TITLE_ESCHARS)) {
				escape(Jobname, &newstr, TITLE_ESCHARS);
				Jobname = newstr;
			}
	    		break;

		case 'T':	/* title */
			/*
			** Title is used only if "pr" filter is specified.
		 	** Otherwise it is ignored.
			*/
			tflag = 1;
			if (arg[2] )
		    		argval = Title = &arg[2];
			else if (argc > 1) {
				argc--;
				argval = Title = *++argv;
			}
			if (strpbrk(Title, PRTITLE_ESCHARS)) {
				escape(Title, &newstr, PRTITLE_ESCHARS);
				Title = newstr;
			}
	    		break;

		case 'w':
			cptr = malloc(sizeof(WIDTHFLD) + strlen(arg+2));
			strcpy(cptr, WIDTHFLD);
			strcat(cptr, arg+2);
			appendlist(&opts, cptr);
			free(cptr);
			argval = &arg[2];
	    		break;

		case 'm':	/* mail */
	    		mail = TRUE;
	    		break;

		case 'h':
			/* Case of no banner */
	    		appendlist(&opts, NOBANNER );
	    		break;

		case 's':	/* Use link , do not copy files */
	    		copy = FALSE;
	    		break;

		case 'p':
			prflag = 1;
			cont_type = SIMPLE;
			break;

		case 'l':
			appendlist(&yopts, CATVFILTER);
			appendlist(&opts, NOFILEBREAK);
			cont_type = SIMPLE;
			break;

		case 'i':
			cptr = malloc(sizeof(IDENT)+ strlen(arg+2) + 1 );
			strcpy(cptr, IDENT);
			if (arg[2]) {
				strcat(cptr, arg+2);
				argval = &arg[2];
			} else
				strcat(cptr, "8");
			appendlist(&yopts, cptr);
			free(cptr);
			break;

		case 't':
			cont_type = OTROFF;
			break;

		case 'd':
			cont_type = TEX;
			break;

		case 'g':
			cont_type = PLOT;
			break;

		case 'v':
			cont_type = RASTER;
			break;

		case 'c':
			cont_type = CIF;
			break;

		case 'f':
			cont_type = FORTRAN;
			break;

		case 'n':
			cont_type = TROFF;
			break;

		case 'r':		/* remove option not supported */
			break;

		case '1':		/* not supported except by LPD */
		case '2':
		case '3':
		case '4':
			if (arg[2])
				argval = &arg[2];
			else if (argc > 1) {
				argc--; 
				argval = *++argv;
			}
	    		break;

		default:
			printf("usage: lpr [-Pprinter] [-#num] [-Cclass] [-Jjob] [-Ttitle] [-i [indent]] [-1234 font] [-wcols] [-m] [-h] [-s] [-pltndgvcf]");
			exit(0);
		}
		appendlpd(&lpdlist, arg, argval);
	}

	/*
	** Copy the lpdlist into the options list.
	** Note that the lpdlist is appended to the options list.
	*/

	cptr = malloc(sizeof(LPDFLD) + 1);
	(void)sprintf(cptr, "%s'", LPDFLD);
	appendlist(&opts, cptr);
	free(cptr);
	if (lpdlist) {
		appendlist(&opts, argval = sprintlist(lpdlist));
		free(argval);
		freelist(lpdlist);
	}
	appendlist(&opts, "'");

	/* 
	** 	If a printer is not specified via -P, take the printer 
	** 	from the PRINTER environment variable, if set. Otherwise, 
	**	the default printer is: lp
	*/
	if (!Printer && !(Printer = (char *)getenv("PRINTER")))
		Printer = DEFLP;

	if (!isprinter(Printer)) {
		fatal("unknown printer");
		exit(0);
	}

	/*
	** argc is the number of files to print. 
	*/
    	while (--argc) {
		fileargs++;
		file = *++argv;
		if (fileargs == 1)
			firstfile = basename(file);
		if (test(file) < 0)
			continue;	/* file unreasonable */
		if (strpbrk(file, FLIST_ESCHARS))
			escape(file, &newstr, FLIST_ESCHARS);
		else
			newstr =  file;

		if (nfiles == 0) { /* first time only */
			cptr = malloc(sizeof(FLIST) + 
				      strlen(newstr) +
				      MAX_FILE_SIZE + 3);

			sprintf(cptr, "%s'%s", FLIST, newstr);
		} else {
			cptr = malloc(strlen(newstr) + MAX_FILE_SIZE + 2);
			strcpy(cptr, newstr);
		}
		sprintf(strchr(cptr, NULL), ":%d", stbuf.st_size); 
		appendlist(&opts, cptr);
		free(cptr);
		nfiles++;
		appendlist(&files, file);
		if (newstr != file)
			free(newstr);
	}

	if (fileargs == 0)
		stdinp = 1;
	else if (nfiles == 0)
                exit(1);
        else {
		appendlist(&opts, "'");
		if (!cont_type && nfiles == Nps)
			cont_type = POSTSCRIPT;
	}

	/* 
	** open message queue and catch interupts so it gets closed too 
	**/
    	startup();	

    	reqp = makereq();
	
    	/* 
	** allocate files for request, standard input and files if copy 
	*/

	allocfiles();
	reqp->file_list = files;
	if (stdinp > 0) {
		if (insize = savestdin()) {
			cptr = malloc(sizeof(FLIST) + MAX_FILE_SIZE + 3);
			sprintf(cptr, "%s':%d'", FLIST, insize);
			appendlist(&opts, cptr);
			free(cptr);
			reqp->options = sprintlist(opts);
			reqp->input_type = cont_type;	/* may have changed */
		} else {
			printf("%s: standard input: empty input file\n", Name);
			done(1);
			/*NOTREACHED*/
		}
	}
	if (putrequest(reqfile, reqp) == -1) {
		lp_fatal(E_LPP_FPUTREQ);
		/*NOTREACHED*/
	}

	reqid = que_job(reqp);

#define ACKJOB
#ifdef ACKJOB
    	ack_job();		/* issue request id message */
#endif
    	done(0);
	/*NOTREACHED*/
}

/* catch -- catch signals */
/*ARGSUSED*/
static void
#if defined(__STDC__)
catch(int s)
#else
catch(s)
int	s;
#endif
{
	done(1);
	/*NOTREACHED*/
}

/* startup -- initialization routine */

static void
#if defined(__STDC__)
startup(void)
#else
startup()
#endif
{
	register int	 try = 0;

    	if (sigset(SIGHUP, SIG_IGN) != SIG_IGN)
		sigset(SIGHUP, catch);
    	if (sigset(SIGINT, SIG_IGN) != SIG_IGN)
		sigset(SIGINT, catch);
    	if (sigset(SIGQUIT, SIG_IGN) != SIG_IGN)
		sigset(SIGQUIT, catch);
    	if (sigset(SIGTERM, SIG_IGN) != SIG_IGN)
		sigset(SIGTERM, catch);

    	for (;;) {
		if (mopen() == 0) 
			break;
		else if (errno == ENOSPC && try++ < 5) {
			sleep(3);
			continue;
		} else {
	    		lp_fatal(E_LP_MOPEN);
			/*NOTREACHED*/
		}
	}

    	umask(0000);
    	curdir = getcwd(NULL, 512);
}

void
#if defined(__STDC__)
done(int i)
#else
done(i)
int	i;
#endif
{
	(void)mclose();
	exit(i);
}

/*
 * copyfile(stream, name) -- copy stream to file "name"
 */
static void
#if defined(__STDC__)
copyfile(FILE *stream, char *fname)
#else
copyfile(stream, fname)
FILE *stream;
char *fname;
#endif
{
	FILE *ostream;
    	int i;
    	char buf[BUFSIZ];

    	if ((ostream = fopen(fname, "w")) == NULL) {
		printf("%s: cannot create %s\n", Name, fname) ;
		done(1);
		/*NOTREACHED*/
    	}

    	chmod(fname, 0600);
    	while((i = fread(buf, sizeof(char), BUFSIZ, stream)) > 0) {
		fwrite(buf, sizeof(char), i, ostream);
		if (feof(stream)) break;
    	}

    	fclose(ostream);
}

/* makereq -- sanity check args, establish defaults */

static REQUEST *
#if defined(__STDC__)
makereq(void)
#else
makereq()
#endif
{
    	static REQUEST	 rq;
	int 	 mem_count = 0;
	char	*cptr;
	char	*title;


    	/* now to build the request */
    	rq.copies = (copies) ? copies : 1;
    	rq.destination = Printer;
    	rq.form = NULL;
    	if (mail) rq.actions = ACT_MAIL;
    	rq.alert = NULL;
    	rq.options = sprintlist(opts);
    	rq.priority = -1;
    	rq.pages = NULL;
    	rq.charset = NULL;
	/*
	** Filter options to be placed now:
	** Special case of "lpr -p -T"
	** lpr -p -T and lpr -p will have prtilte appended to yopts
	** lpr -T and lpr   will not have any title appended to yopts.
	*/

	/*
	** If prflag is not set, ignore title.
	*/
	if (prflag) {
		if (tflag) {
			cptr = malloc(sizeof(PRTITLE) + strlen(Title) + 2);
			(void)sprintf(cptr, "%s'%s'", PRTITLE, Title);
		} else {
			if (stdinp) {
				cptr = malloc(sizeof(PRTITLE) +
					      sizeof("standard input") + 1);
				(void)sprintf(cptr, "%s'standard input'", 
								PRTITLE);
			} else {
				cptr = malloc(sizeof(PRTITLE) +
					      strlen(firstfile) + 2);
				(void)sprintf(cptr, "%s'%s'", PRTITLE, firstfile);
			}

		}
		appendlist(&yopts, cptr);
		free(cptr);
	}
    	rq.modes = sprintlist(yopts);
	/*
	** Copy the -C and -J options if present. ELse use defaults
	*/
	if (jflag)
		mem_count = strlen(Jobname);
	else
		if (stdinp)
			mem_count = sizeof("stdin") - 1;	
		else
			mem_count = strlen(firstfile);	
	
	mem_count += sizeof(JCSEP);
	if (cflag)	
		mem_count += strlen(Class);
	else {
		Lhost = gethostname();
		mem_count += strlen(Lhost);
	}
	title = malloc(mem_count + 1);

	if (jflag)
		strcpy(title, Jobname);
	else
		if (stdinp)
			strcpy(title, "stdin");
		else
			strcpy(title, firstfile);
	strcat(title, JCSEP);
	if (cflag)
		strcat(title, Class);
	else
		strcat(title, Lhost);

    	rq.title = (char *)title;
    	rq.input_type = cont_type;
    	rq.file_list = 0;
    	rq.user = Person;
    	return(&rq);
}

/*
 * Process command line file arguments
 *	Among its many jobs:
 *		1) Do S_ALLOC_FILES
 *		2) Fill in reqfile[] with request file name
 *		3) Initialize stdinfile to spool file name for stdin
 *		4) Copy files to spool directory (if needed)
 *		5) Allocate new file list
 */
static
#if defined(__STDC__)
allocfiles(void)
#else
allocfiles()
#endif
{
	char **reqfiles = 0, **filep, *p, *prefix;
	FILE *f;
	int numfiles, filenum = 0;
	
	numfiles = 1 + ((stdinp > 0) ? 1 : (copy ? nfiles : 0));

	prefix = getfiles(numfiles);
	strcpy(reqfile, prefix);
	strcat(reqfile, "-000000");
    	rfilebase = makepath(Lp_Temp, reqfile, NULL);
    	if (stdinp > 0) {
		stdinfile = strdup(rfilebase);
		p = strchr(stdinfile, 0) - 6;
		*p++ = '1';
		*p = 0;
		filenum++;
		appendlist(&reqfiles, stdinfile);
    	}
    	p = strchr(reqfile, 0) - 6; *p++ = '0'; *p = 0;
    	p = strchr(rfilebase, 0) - 6;

    	for (filep = files; filep && *filep; filep++) {
	    	if (copy) {
			if (f = fopen(*filep, "r")) {
		    		sprintf(p, "%d", ++filenum);
		    		copyfile(f, rfilebase);
		    		appendlist(&reqfiles, rfilebase);
		    		fclose(f);
			} else
				printf("%s: cannot access %s\n", Name, *filep);
	    	} else {
			if (**filep == '/')
		    		appendlist(&reqfiles, *filep);
			 else if (curdir && *curdir)
		    		appendlist(&reqfiles, makepath(curdir, 
					    		       *filep, 
					    		       NULL));
			 else {
				fatal("can't get current directory\n");
				/*NOTREACHED*/
			}
	    	}
	}
    	freelist(files);
    	files = reqfiles;
    	return(1);
}

/* getfile -- allocate the requested number of temp files */

static char *
#if defined(__STDC__)
getfiles(int number)
#else
getfiles(number)
int	number;
#endif
{
	int	 size, type;
	short	 status;
	char	*pfix;

	snd_msg(S_ALLOC_FILES, number);
	rcv_msg(R_ALLOC_FILES, &status, &pfix);

	switch (status) {
		case MOK:
			return(strdup(pfix));
		case MNOMEM:
			printf("too many files - break up the job\n");
			done(1);
			/*NOTREACHED*/
			break;
		default:
			lp_fatal(E_LP_BADSTATUS, status); 
			/*NOTREACHED*/
			break;
	}
}

static char *
#if defined(__STDC__)
que_job(REQUEST *rqp)
#else
que_job(rqp)
REQUEST	*rqp;
#endif
{
	int 	 size, type;
	long	 chkbits;
	short	 status;
	char	*chkp, *req_id;

	snd_msg(S_PRINT_REQUEST, reqfile);
	rcv_msg(R_PRINT_REQUEST, &status, &req_id, &chkbits);

	switch (status) {
	case MOK:
		return(strdup(req_id));
	case MNOMEM:
		lp_fatal(E_LP_MNOMEM);
		/*NOTREACHED*/
		break;
    	case MNODEST:
                fatal("unknown printer");
		/*NOTREACHED*/
		break;
    	case MDENYDEST:
		if (chkbits) {
			if (chkbits & PCK_TYPE)		/* No terminfo */
                		fatal("unknown printer");
			else {
	    			char buf[20];

				buf[0] = NULL;
	    			if (chkbits & PCK_WIDTH) 
					strcat(buf, "-w width ");
	    			if (chkbits & PCK_BANNER) 
					strcat(buf, "-h ");
	    			lp_fatal(E_LP_PTRCHK, buf);
			}
		} else 
			lp_fatal(E_LP_DENYDEST, Printer);
		/*NOTREACHED*/
		break;
    	case MNOFILTER:
		lp_fatal(E_LP_NOFILTER);
		/*NOTREACHED*/
		break;
    	case MERRDEST:
		fatal("Printer queue is disabled");
		/*NOTREACHED*/
		break;
   	case MNOOPEN:
		lp_fatal(E_LPP_NOOPEN);
		/*NOTREACHED*/
		break;
	default:
 		lp_fatal(E_LP_BADSTATUS, status);
		/*NOTREACHED*/
		break;
	}
}

/* ack_job -- issue request id message */

static void
#if defined(__STDC__)
ack_job(void)
#else
ack_job()
#endif
{
	printf("request id is %s (", reqid);
	if (nfiles > 0) {
		printf("%d file", nfiles);
		if (nfiles > 1)
			printf("s");
	}
	if (stdinp > 0) {
		if (nfiles > 0)
			printf(" and ");
		printf("standard input");
    	}                                              

	printf(")\n");
}

/* savestdin -- save standard input */

static size_t
#if defined(__STDC__)
savestdin(void)
#else
savestdin()
#endif
{
	copyfile(stdin, stdinfile);
	stat(stdinfile, &stbuf);
	if (!cont_type && psfile(stdinfile))
		cont_type = POSTSCRIPT;
	return(stbuf.st_size);
}

static void
#if defined(__STDC__)
appendlpd(char ***list, char *opt, char *val)
#else
appendlpd(list, opt, val)
char	***list;
char	  *opt, *val;
#endif
{
	char *buf;
	char *newstr = "";

	if (val)
		escape(val, &newstr, LPD_ESCHARS);
	buf = malloc(OPT_SIZE + strlen(newstr) + 1);
	strncpy(buf, opt, OPT_SIZE);
	buf[OPT_SIZE] = NULL;
	strcat(buf, newstr);
	appendlist(list, buf);
	free(buf);
	if (val)
		free(newstr);
}
	

/*
** Test if the file is printable. Return -1 if unprintable.
*/
static
#if defined(__STDC__)
test(char *file)
#else
test(file)
char *file;
#endif
{

	register int	 fd, magic;
	register struct filehdr *fh;
	char		 buf[FILHSZ]; 		/* FILHDR size */

	if (access(file, 4/*read*/)) {
		printf("%s: cannot access %s\n", Name, file);
		return(-1);	
	}
	if (stat(file, &stbuf)) {
		printf("%s: cannot stat %s\n", Name, file);
		return(-1);
	}
	if ((stbuf.st_mode & S_IFMT) == S_IFDIR) {
		printf("%s: %s is a directory.\n", Name, file);
		return(-1);
	}
	if (stbuf.st_size == 0) {
		printf("%s: %s is an empty file\n", Name, file);
		return(-1);
	}
	if ((fd = open(file, O_RDONLY)) < 0) {
                printf("%s: cannot open %s\n", Name, file);
		return(-1);
	}
	magic = 0;
	if (read(fd, buf, FILHSZ) == FILHSZ) {
		fh = (FILHDR *)buf;
		magic = fh->f_magic;
		/*
		** These are chosen from filehdr.h.
		*/
		switch (magic) {
		case OMAGIC:
		case NMAGIC:
		case ZMAGIC:
                        printf("%s: %s is an executable program", Name, file);
			goto error1;
		case ARMAG:
                        printf("%s: %s is an archive file" , Name, file);
			goto error1;
                default:
			/*
		 	 ** The magic number for an executable file can vary 
			 ** from 0400 to 0600 depending on the machine (3b2,
			 ** 386 etc). 
		 	 */
			if ( (magic > 0400 ) && (magic <0600)) {
       				printf(
				"%s: %s is an archive or an executable program",
								Name, file);
				goto error1;
			}
			if (buf[0] == '\100' && 
			    buf[1] == '\357' && 
			   (strcmp(cont_type, OTROFF) != 0)) {
				printf("%s: %s is a troff file, assuming '-t'\n",
				Name, file);			
				cont_type = OTROFF;
			}
			break;
		}
	}
	if (!cont_type && psfile(file))
		Nps++;		/* count number of PostScript files */
	(void)close(fd);	/* File is printable */
	return(1);

error1:
	printf(" and is unprintable\n");
 	(void)close(fd);                
 	return(-1);       
}

static void
#if defined(__STDC__)
escape(char *old, char **new, char *esc)
#else
escape(old, new, esc)
char *old, **new, *esc;
#endif
{
	char	*p;

	p = malloc(2*strlen(old) + 1);	/* maximum sized string */
	canonize(p, old, esc);
	*new = p;
}
