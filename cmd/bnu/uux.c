/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)bnu:uux.c	2.13.5.1"

#include "uucp.h"

#define SHORTBUF 64

#define NOSYSPART 0
#define HASSYSPART 1

#define GENSEND(f, a, b, c) {\
ASSERT(fprintf(f, "S %s %s %s -%s %s 0666 %s %s\n", a, b, User, _Statop?"o":"", c, User, _Sfile) >= 0, Ct_WRITE, "", errno);\
}
#define GENRCV(f, a, b) {\
char tbuf[SHORTBUF]; \
gename (DATAPRE, xsys, 'Z', tbuf); \
ASSERT(fprintf(f, "R %s %s %s - %s 0666 %s %s\n", a, b, User, _Sfile, User, tbuf) \
 >= 0, Ct_WRITE, "", errno);\
}

#define APPCMD(p)	{(void) strcat(cmd, p); (void) strcat(cmd, " ");}

static char	_Sfile[MAXFULLNAME];
static int	_Statop;
char Sgrade[NAMESIZE];
void cleanup();
static void usage();

/*
 *	uux
 */
main(argc, argv, envp)
char *argv[];
char *envp[];
{
	char *jid();
	FILE *fprx = NULL, *fpc = NULL, *fpd = NULL, *fp = NULL;
	static void onintr();
	int cfileUsed = 0;	/*  >0 if commands put in C. file flag  */
	int rflag = 0;		/*  C. files for receiving flag  */
	int cflag = 0;		/* if > 0 make local copy of files to be sent */
	int nflag = 0;		/* if != 0, do not request error notification */
	int zflag = 0;		/* if != 0, request success notification */
	int pipein = 0;
	int startjob = 1;
	short jflag = 0;	/* -j flag  output Jobid */
	int bringback = 0;	/* return stdin to invoker on error */
	int ret, i;
	char *getprm();
	char redir = '\0';	/* X_STDIN, X_STDOUT, X_STDERR as approprite */
	char command = TRUE;
	char cfile[NAMESIZE];	/* send commands for files from here */
	char dfile[NAMESIZE];	/* used for all data files from here */
	char rxfile[NAMESIZE];	/* file for X_ commands */
	char tfile[NAMESIZE];	/* temporary file name */
	char t2file[NAMESIZE];	/* temporary file name */
	char buf[BUFSIZ];
	char inargs[BUFSIZ];
	char cmd[BUFSIZ];
	char *ap;
	char prm[BUFSIZ];
	char syspart[NAMEBUF], rest[BUFSIZ];
	char xsys[NAMEBUF];
	char	*fopt = NULL;
	char	*retaddr = NULL;
	static char emsg1[] = "No administrator defined service grades available on this machine.";
	static char emsg2[] = "UUCP service grades range from [A-Z][a-z] only.";

	struct stat stbuf;

 /* we want this to run as uucp, even if the kernel doesn't */
	Uid = getuid();
	Euid = geteuid();	/* this should be UUCPUID */
	if (Uid == 0)
	    setuid(UUCPUID);

/* init environment for fork-exec */
	Env = envp;

	/* choose LOGFILE */
	(void) strcpy(Logfile, LOGUUX);

	/*
	 * determine local system name
	 */
	(void) strcpy(Progname, "uux");
	Pchar = 'X';
	(void) signal(SIGILL, onintr);
	(void) signal(SIGTRAP, onintr);
	(void) signal(SIGIOT, onintr);
	(void) signal(SIGEMT, onintr);
	(void) signal(SIGFPE, onintr);
	(void) signal(SIGBUS, onintr);
	(void) signal(SIGSEGV, onintr);
	(void) signal(SIGSYS, onintr);
	(void) signal(SIGTERM, SIG_IGN);
	uucpname(Myname);
	Ofn = 1;
	Ifn = 0;
	*_Sfile = '\0';

	/*
	 * determine id of user starting remote 
	 * execution
	 */
	guinfo(Uid, User);
	(void) strcpy(Loginuser,User);

	*Sgrade = NULLCHAR;

	/*
	 * this is a check to see if we are using the administrator
	 * defined service grade. The GRADES file will determine if 
	 * we are. If so then setup the default grade variables.
	 */

	if (eaccess(GRADES, 04) != -1) {
		Grade = 'A';
		Sgrades = TRUE;
		(void) strncpy(Sgrade, "default", NAMESIZE-1);
		Sgrade[NAMESIZE-1] = NULLCHAR;
	}

	/*
	 * create/append command log
	 */
	commandlog(argc,argv);

	/*
	 * since getopt() can't handle the pipe input option '-';
	 * change it to "-p"
	 */
	for (i=1; i<argc  &&  *argv[i] == '-'; i++)
	    if (EQUALS(argv[i], "-"))
		argv[i] = "-p";

	while ((i = getopt(argc, argv, "a:bcCjg:nprs:x:z")) != EOF) {
		switch(i){

		/*
		 * use this name in the U line
		 */
		case 'a':
			retaddr = optarg;
			break;

		/*
		 * if return code non-zero, return command's input
		 */
		case 'b':
			bringback = 1;
			break;

		/* do not make local copies of files to be sent (default) */
		case 'c':
			cflag = 0;
			break;

		/* make local copies of files to be sent */
		case 'C':
			cflag = 1;
			break;
		/*
		 * set priority of request
		 */
		case 'g':
			if (!Sgrades) {
				if (strlen(optarg) < (size_t) 2 && isalnum(*optarg))
					Grade = *optarg;
				else {
					(void) fprintf(stderr, "%s\n%s\n",
						emsg1, emsg2);
					cleanup(-1);
				}
			}
			else {
				(void) strncpy(Sgrade, optarg, NAMESIZE-1);
				Sgrade[NAMESIZE-1] = NULLCHAR;
				if (vergrd(Sgrade) != SUCCESS)
					cleanup(FAIL);
			}
			break;


		case 'j':	/* job id */
			jflag = 1;
			break;


		/*
		 * do not send failure notification to user
		 */
		case 'n':
			nflag++;
			break;

		/*
		 * send success notification to user
		 */
		case 'z':
			zflag++;
			break;

		/*
		 * -p or - option specifies input from pipe
		 */
		case 'p':
			pipein = 1;
			break;

		/*
		 * do not start transfer
		 */
		case 'r':
			startjob = 0;
			break;

		case 's':
			fopt = optarg;
			_Statop++;
			break;

		/*
		 * debugging level
		 */
		case 'x':
			Debug = atoi(optarg);
			if (Debug <= 0)
				Debug = 1;
			break;

		default:
			usage();
		}
	}

	DEBUG(4, "\n\n** %s **\n", "START");

	if( optind >= argc )
		usage();

	/*
	 * copy arguments into a buffer for later
	 * processing
	 */
	inargs[0] = '\0';
	for (; optind < argc; optind++) {
		DEBUG(4, "arg - %s:", argv[optind]);
		(void) strcat(inargs, " ");
		(void) strcat(inargs, argv[optind]);
	}

	/*
	 * get working directory and change
	 * to spool directory
	 */
	DEBUG(4, "arg - %s\n", inargs);
	gwd(Wrkdir);
	if(fopt){
		if(*fopt != '/')
			(void) sprintf(_Sfile, "%s/%s", Wrkdir, fopt);
		else
			(void) sprintf(_Sfile, "%s", fopt);

	}
	else
		strcpy(_Sfile, "dummy");

	if (chdir(WORKSPACE) != 0) {
	    (void) fprintf(stderr, "No spool directory - %s - get help\n", WORKSPACE);
	    cleanup(12);
	}
	/*
	 * find remote system name
	 * remote name is first to know that 
	 * is not > or <
	 */
	ap = inargs;
	xsys[0] = '\0';
	while ((ap = getprm(ap, NULL, prm)) != NULL) {
		if (prm[0] == '>' || prm[0] == '<') {
			ap = getprm(ap, NULL, prm);
			continue;
		}

		/*
		 * split name into system name
		 * and command name
		 */
		(void) split(prm, xsys, CNULL, rest);
		break;
	}
	if (xsys[0] == '\0')
		(void) strcpy(xsys, Myname);
	strncpy(Rmtname, xsys, MAXBASENAME);
	Rmtname[MAXBASENAME] = '\0';
	DEBUG(4, "xsys %s\n", xsys);

	/*
	 * check to see if system name is valid
	 */
	if (versys(xsys) != 0) {
		/*
		 * bad system name
		 */
		fprintf(stderr, "bad system name: %s\n", xsys);
		if (fprx != NULL)
			(void) fclose(fprx);
		if (fpc != NULL)
			(void) fclose(fpc);
		cleanup(11);
	}

	DEBUG(6, "User %s\n", User);
	if (retaddr == NULL)
		retaddr = User;

	/*
	 * initialize command buffer
	 */
	*cmd = '\0';

	/*
	 * generate JCL files to work from
	 */

	/*
	 * fpc is the C. file for the local site.
	 * collect commands into cfile.
	 * commit if not empty (at end).
	 * 
	 * the appropriate C. file.
	 */
	gename(CMDPRE, xsys, Grade, cfile);
	DEBUG(9, "cfile = %s\n", cfile);
	ASSERT(access(cfile, 0) != 0, Fl_EXISTS, cfile, errno);
	fpc = fdopen(ret = creat(cfile, CFILEMODE), "w");
	ASSERT(ret >= 0 && fpc != NULL, Ct_OPEN, cfile, errno);
	setbuf(fpc, CNULL);

	/*  set Jobid -- C.jobid */
	(void) strncpy(Jobid, BASENAME(cfile, '.'), NAMESIZE);
	Jobid[NAMESIZE-1] = '\0';

	/*
	 * rxfile is the X. file for the job, fprx is its stream ptr.
	 * if the command is to be executed locally, rxfile becomes
	 * a local X. file, otherwise we send it as a D. file to the
	 * remote site.
	 */
	
	gename(DATAPRE, xsys, 'X', rxfile);
	DEBUG(9, "rxfile = %s\n", rxfile);
	ASSERT(access(rxfile, 0) != 0, Fl_EXISTS, rxfile, errno);
	fprx = fdopen(ret = creat(rxfile, DFILEMODE), "w");
	ASSERT(ret >= 0 && fprx != NULL, Ct_WRITE, rxfile, errno);
	setbuf(fprx, CNULL);
	clearerr(fprx);

	(void) fprintf(fprx,"%c %s %s\n", X_USER, User, Myname);
	if (zflag) {
		(void) fprintf(fprx, "%c return status on success\n",
			X_COMMENT);
		(void) fprintf(fprx,"%c\n", X_SENDZERO);
	}

	if (nflag) {
		(void) fprintf(fprx, "%c don't return status on failure\n",
			X_COMMENT);
		(void) fprintf(fprx,"%c\n", X_SENDNOTHING);
	} else {
		(void) fprintf(fprx, "%c return status on failure\n",
			X_COMMENT);
		fprintf(fprx,"%c\n", X_NONZERO);
	}

	if (bringback) {
		(void) fprintf(fprx, "%c return input on abnormal exit\n",
			X_COMMENT);
		(void) fprintf(fprx,"%c\n", X_BRINGBACK);
	}
	if (_Statop)
		(void) fprintf(fprx,"%c %s\n", X_MAILF, _Sfile);

	if (retaddr != NULL) {
		(void) fprintf(fprx, "%c return address for status or input return\n",
			X_COMMENT);
		(void) fprintf(fprx,"%c %s\n", X_RETADDR, retaddr);
	}

	/*
	 * create a JCL file to spool pipe input into
	 */
	if (pipein) {
		/*
		 * fpd is the D. file into which we now read
		 * input from stdin
		 */
		
		gename(DATAPRE, Myname, 'B', dfile);
		
		ASSERT(access(dfile, 0) != 0, Fl_EXISTS, dfile, errno);
		fpd = fdopen(ret = creat(dfile, DFILEMODE), "w");
		ASSERT(ret >= 0 && fpd != NULL, Ct_OPEN, dfile, errno);

		/*
		 * read pipe to EOF
		 */
		while (!feof(stdin)) {
			ret = fread(buf, 1, BUFSIZ, stdin);
			ASSERT(fwrite(buf, 1, ret, fpd) == ret, Ct_WRITE,
				dfile, errno);
		}
		ASSERT(ferror(fpd) == 0, Ct_WRITE, dfile, errno);
		(void) fclose(fpd);

		/*
		 * if command is to be executed on remote
		 * create extra JCL
		 */
		if (!EQUALSN(Myname, xsys, MAXBASENAME)) {
			GENSEND(fpc, dfile, dfile, dfile);
		}

		/*
		 * create file for X_ commands
		 */
		(void) fprintf(fprx, "%c %s\n", X_RQDFILE, dfile);
		(void) fprintf(fprx, "%c %s\n", X_STDIN, dfile);

		if (EQUALS(Myname, xsys))
			wfcommit(dfile, dfile, xsys);

	}
	/*
	 * parse command
	 */
	ap = inargs;
	while ((ap = getprm(ap, NULL, prm)) != NULL) {
		DEBUG(4, "prm - %s\n", prm);

		/*
		 * redirection of I/O
		 */
		if ( prm[0] == '<' ) {
		    redir = X_STDIN;
		    continue;
		}
		if ( prm[0] == '>' ) {
		    redir = X_STDOUT;
		    continue;
		}
		if ( EQUALS(prm, "2>") ) {
		    redir = X_STDERR;
		    continue;
		}

		/*
		 * some terminator
		 */
		if ( prm[0] == '|' || prm[0] == '^'
		  || prm[0] == '&' || prm[0] == ';') {
			if (*cmd != '\0')	/* not 1st thing on line */
				APPCMD(prm);
			command = TRUE;
			continue;
		}

		/*
		 * process command or file or option
		 * break out system and file name and
		 * use default if necessary
		 */
		ret = split(prm, syspart, CNULL, rest);
		DEBUG(4, "syspart -> %s, ", syspart);
		DEBUG(4, "rest -> %s, ", rest);
		DEBUG(4, "ret -> %d\n", ret);

		if (command  && redir == '\0') {
			/*
			 * command
			 */
			APPCMD(rest);
			command = FALSE;
			continue;
		}

		if (syspart[0] == '\0') {
			(void) strcpy(syspart, Myname);
			DEBUG(6, "syspart -> %s\n", syspart);
		} else if (versys(syspart) != 0) {
			/*
			 * bad system name
			 */
			fprintf(stderr, "bad system name: %s\n", syspart);
			if (fprx != NULL)
				(void) fclose(fprx);
			if (fpc != NULL)
				(void) fclose(fpc);
			cleanup(11);
		}

		/*
		 * process file or option
		 */

		/*
		 * process file argument
		 * expand filename and create JCL card for
		 * redirected output
		 * e.g., X file sys
		 */
		if ((redir == X_STDOUT) || (redir == X_STDERR)) {
			if (rest[0] != '~')
				if (ckexpf(rest))
					cleanup(6);
			ASSERT(fprintf(fprx, "%c %s %s\n", redir, rest,
				syspart) >= 0, Ct_WRITE, rxfile, errno);
			redir = '\0';
			continue;
		}

		/*
		 * if no system specified, then being
		 * processed locally
		 */
		if (ret == NOSYSPART && redir == '\0') {

			/*
			 * option
			 */
			APPCMD(rest);
			continue;
		}


		/* local xeqn + local file  (!x !f) */
		if ((EQUALSN(xsys, Myname, MAXBASENAME))
		 && (EQUALSN(xsys, syspart, MAXBASENAME))) {
			/*
			 * create JCL card
			 */
			if (ckexpf(rest))
				cleanup(7);
			/*
			 * JCL card for local input
			 * e.g., I file
			 */
			if (redir == X_STDIN) {
				(void) fprintf(fprx, "%c %s\n", X_STDIN, rest);
			} else
				APPCMD(rest);
			ASSERT(fprx != NULL, Ct_WRITE, rxfile, errno);
			redir = '\0';
			continue;
		}

		/* remote xeqn + local file (sys!x !f) */
		if (EQUALSN(syspart, Myname, MAXBASENAME)) {
			/*
			 * check access to local file
			 * if cflag is set, copy to spool directory
			 * otherwise, just mention it in the X. file
			 */
			if (ckexpf(rest))
				cleanup(6);
			DEBUG(4, "rest %s\n", rest);

			/* see if I can read this file as read uid, gid */
			if (uidstat(rest, &stbuf) != 0) {
			    (void) fprintf(stderr,
			      "can't get file status %s\n", rest);
			    cleanup(8);
			}
			if ( !(stbuf.st_mode & ANYREAD)
		  	  && !(stbuf.st_uid == Uid && stbuf.st_mode & 0400)
		  	  && !(stbuf.st_gid ==getgid() && stbuf.st_mode & 0040)
		  	  ) {
				fprintf(stderr,"permission denied %s\n", rest);
				cleanup(1);
			}

			/* D. file for sending local file */
			gename(DATAPRE, xsys, 'A', dfile);

			if (cflag || !(stbuf.st_mode & ANYREAD)) {
				/* make local copy */
				if (uidxcp(rest, dfile) != 0) {
				    fprintf(stderr,"can't copy %s\n", rest);
				    cleanup(5);
				}
				(void) chmod(dfile, DFILEMODE);
				/* generate 'send' entry in command file */
				GENSEND(fpc, rest, dfile, dfile);
			} else		/* don't make local copy */
				GENSEND(fpc, rest, dfile, dfile);

			/*
			 * JCL cards for redirected input in X. file,
			 * e.g.
			 * I D.xxx
			 * F D.xxx
			 */
			if (redir == X_STDIN) {
				/*
				 * don't bother making a X_RQDFILE line that
				 * renames stdin on the remote side, since the
				 * remote command can't know its name anyway
				 */
				(void) fprintf(fprx, "%c %s\n", X_STDIN, dfile);
				(void) fprintf(fprx, "%c %s\n", X_RQDFILE, dfile);
			} else {
				APPCMD(BASENAME(rest, '/'));;
				/*
				 * generate X. JCL card that specifies
				 * F file 
				 */
				(void) fprintf(fprx, "%c %s %s\n", X_RQDFILE,
				 dfile, BASENAME(rest, '/'));
			}
			redir = '\0';

			continue;
		}

		/* local xeqn + remote file (!x sys!f ) */
		if (EQUALS(Myname, xsys)) {
			/*
			 * expand receive file name
			 */
			if (ckexpf(rest))
				cleanup(6);
			/*
			 * tfile is command file for receive from remote.
			 * we defer commiting until later so 
			 * that only one C. file is created per site.
			 *
			 * dfile is name of data file to receive into;
			 * we don't use it, just name it.
			 *
			 * the name of the remote is appended to the
			 * X_RQDFILE line to help uuxqt in finding the
			 * D. file when it arrives.
			 */
			if (gtcfile(tfile, syspart) != SUCCESS) {
				gename(CMDPRE, syspart, 'R', tfile);
				
				ASSERT(access(tfile, 0) != 0,
				    Fl_EXISTS, tfile, errno);
				svcfile(tfile, syspart, Sgrade);
				(void) close(creat(tfile, CFILEMODE));
			}
			fp = fopen(tfile, "a");
			ASSERT(fp != NULL, Ct_OPEN, tfile, errno);
			setbuf(fp, CNULL);
			gename(DATAPRE, syspart, 'R', dfile);

			/* prepare JCL card to receive file */
			GENRCV(fp, rest, dfile);
			ASSERT(ferror(fp) == 0, Ct_WRITE, dfile, errno);
			(void) fclose(fp);
			rflag++;
			if (rest[0] != '~')
				if (ckexpf(rest))
					cleanup(7);

			/*
			 * generate receive entries
			 */
			if (redir == X_STDIN) {
				(void) fprintf(fprx,
					"%c %s/%s/%s\n", X_RQDFILE, Spool,
					syspart, dfile);
				(void) fprintf(fprx, "%c %s\n", X_STDIN, dfile);
			} else {
				(void) fprintf(fprx, "%c %s/%s/%s %s\n",
				X_RQDFILE, Spool, syspart, dfile,
				BASENAME(rest, '/'));
				APPCMD(BASENAME(rest, '/'));
			}

			redir = '\0';
			continue;
		}

		/* remote xeqn/file, different remotes (xsys!cmd syspart!rest) */
		if (!EQUALS(syspart, xsys)) {
			/*
			 * strategy:
			 * request rest from syspart.
			 *
			 * set up a local X. file that will send rest to xsys,
			 * once it arrives from syspart.
			 *
			 * arrange so that the xsys D. file (fated to become
			 * an X. file on xsys), rest is required and named.
			 *
			 * pictorially:
			 *
			 * ===== syspart/C.syspartR.... =====	(tfile)
			 * R rest D.syspart...			(dfile)
			 *
			 *
			 * ===== local/X.local... =====		(t2file)
			 * F Spool/syspart/D.syspart... rest	(dfile)
			 * C uucp -C rest D.syspart...		(dfile)
			 *
			 * ===== xsys/D.xsysG....		(fprx)
			 * F D.syspart... rest			(dfile)
			 * or, in the case of redir == '<'
			 * F D.syspart...			(dfile)
			 * I D.syspart...			(dfile)
			 *
			 * while we do push rest around a bunch,
			 * we use the protection scheme to good effect.
			 *
			 * we must rely on uucp's treatment of requests
			 * form XQTDIR to get the data file to the right
			 * place ultimately.
			 */

			/* build (or append to) C.syspartR... */
			if (gtcfile(tfile, syspart) != SUCCESS) {
				gename(CMDPRE, syspart, 'R', tfile);
				
				ASSERT(access(tfile, 0) != 0,
				    Fl_EXISTS, tfile, errno);
				svcfile(tfile, syspart, Sgrade);
				(void) close(creat(tfile, CFILEMODE));
			}
			fp = fopen(tfile, "a");
			ASSERT(fp != NULL, Ct_OPEN, tfile, errno);
			setbuf(fp, CNULL);
			gename(DATAPRE, syspart, 'R', dfile);
			GENRCV(fp, rest, dfile);
			ASSERT(ferror(fp) == 0, Ct_WRITE, dfile, errno);
			(void) fclose(fp);

			/* build local/X.localG... */
			gename(XQTPRE, Myname, Grade, t2file);
			ASSERT(access(t2file, 0)!=0, Fl_EXISTS, t2file, errno);
			(void) close(creat(t2file, CFILEMODE));
			fp = fopen(t2file, "w");
			ASSERT(fp != NULL, Ct_OPEN, t2file, errno);
			setbuf(fp, CNULL);
			(void) fprintf(fp, "%c %s/%s/%s %s\n", X_RQDFILE,
				Spool, syspart, dfile, BASENAME(rest, '/'));
			(void) fprintf(fp, "%c uucp -C %s %s!%s\n",
				X_CMD, BASENAME(rest, '/'), xsys, dfile);
			ASSERT(ferror(fp) == 0, Ct_WRITE, t2file, errno);
			(void) fclose(fp);

			/* put t2file where uuxqt can get at it */
			wfcommit(t2file, t2file, Myname);

			/* generate xsys/X.sysG... cards */
			if (redir == X_STDIN) {
				(void) fprintf(fprx, "%c %s\n",
					X_RQDFILE, dfile);
				(void) fprintf(fprx, "%c %s\n", X_STDIN, dfile);
			} else {
				(void) fprintf(fprx, "%c %s %s\n", X_RQDFILE,
				 dfile, BASENAME(rest, '/'));
				APPCMD(BASENAME(rest, '/'));
			}
			redir = '\0';
			continue;
		}

		/* remote xeqn + remote file, same remote (sys!x sys!f) */
		if (rest[0] != '~')	/* expand '~' on remote */
			if (ckexpf(rest))
				cleanup(7);
		if (redir == X_STDIN) {
			(void) fprintf(fprx, "%c %s\n", X_STDIN, rest);
		}
		else
			APPCMD(rest);
		redir = '\0';
		continue;

	}

	/*
	 * place command to be executed in JCL file
	 */
	(void) fprintf(fprx, "%c %s\n", X_CMD, cmd);
	ASSERT(ferror(fprx) == 0, Ct_WRITE, rxfile, errno);
	(void) fclose(fprx);		/* rxfile is ready for commit */
	logent(cmd, "QUEUED");

	/* the X. name must have the X.Jobid so status reporting is by jobid */
	(void) sprintf(tfile, "X.%s", Jobid);
	if (EQUALS(xsys, Myname)) {
		/* local xeqn -- use X_ file here */
		wfcommit(rxfile, tfile, xsys);
		
		/*
		 * see if -r option requested JCL to be queued only
		 */
		if (startjob)
			xuuxqt(Myname);
	} else {
		/* remote xeqn -- send rxfile to remote */
		/* put it in a place where cico can get at it */

		GENSEND(fpc, rxfile, tfile, rxfile);
	}

	cfileUsed = (ftell(fpc) != 0L);	/* was cfile used? */
	ASSERT(ferror(fpc) == 0, Ct_WRITE, cfile, errno);
	(void) fclose(fpc);

	/* commit C. files for remote receive */

	commitall();

	/*
	 * has any command been placed in command JCL file
	 */
	if (cfileUsed) {

		svcfile(cfile, xsys, Sgrade);
		commitall();

		/*
		 * see if -r option requested JCL to be queued only
		 */
		if (startjob)
			xuucico(xsys);
	} else
		unlink(cfile);

	if (jflag) {	/* print Jobid */
		(void) strncpy(Jobid, jid(), NAMESIZE);
		printf("%s\n", Jobid);
	}

	cleanup(0);
	/* NOTREACHED */
}


/*
 * cleanup and unlink if error
 *	code	-> exit code
 * return:
 *	none
 */
void
cleanup(code)
register int code;
{
	rmlock(CNULL);
	if (code) {
		wfabort();
		fprintf(stderr, "uux failed ( %d )\n", code);
	}
	DEBUG(1, "exit code %d\n", code);
	if (code < 0)
		exit(-code);
	else
		exit(code);
}

/*
 * catch signal then cleanup and exit
 */
static void
onintr(inter)
register int inter;
{
	char str[30];
	(void) signal(inter, SIG_IGN);
	(void) sprintf(str, "XSIGNAL %d", inter);
	logent(str, "XCAUGHT");
	cleanup(-inter);
}


static void
usage()
{

	(void) fprintf(stderr,
"Usage:  %s [-aNAME] [-b] [-c] [-C] [-j] [-gGRADE] [-n] [-p] \\\n\
	[-r] [-sFILE] [-xNUM] [-z] command-string\n", Progname);
	exit(2);
}
