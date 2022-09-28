/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)bnu:uucp.c	2.16.4.1"

#include "uucp.h"

/*
 * uucp
 * user id 
 * make a copy in spool directory
 */
int Copy = 0;
static int _Transfer = 0;
char Nuser[32];
char *Ropt = " ";
char Optns[10];
char Uopts[BUFSIZ];
char Xopts[BUFSIZ];
char Sgrade[NAMESIZE];
int Mail = 0;
int Notify = 0;

void cleanup(), ruux(), usage();
int eaccess(), guinfo(), vergrd(), gwd(), ckexpf(), uidstat(), uidxcp(),
	copy(), gtcfile();
void commitall(), wfabort(), mailst(), gename(), svcfile();

char	Sfile[MAXFULLNAME];

main(argc, argv, envp)
char *argv[];
char	**envp;
{
	char *jid();
	int	ret;
	int	errors = 0;
	char	*fopt;
	char	sys1[MAXFULLNAME], sys2[MAXFULLNAME];
	char	fwd1[MAXFULLNAME], fwd2[MAXFULLNAME];
	char	file1[MAXFULLNAME], file2[MAXFULLNAME];
	short	jflag = 0;	/* -j flag  Jobid printout */
	static char emsg1[] = "No administrator defined service grades available on this machine.";
	static char emsg2[] = "UUCP service grades range from [A-Z][a-z] only.";
	extern int	split();


	/* this fails in some versions, but it doesn't hurt */
	Uid = getuid();
	Euid = geteuid();
	if (Uid == 0)
		(void) setuid(UUCPUID);

	/* choose LOGFILE */
	(void) strcpy(Logfile, LOGUUCP);

	Env = envp;
	fopt = NULL;
	(void) strcpy(Progname, "uucp");
	Pchar = 'U';
	*Uopts = NULLCHAR;
	*Xopts = NULLCHAR;
	*Sgrade = NULLCHAR;

	if (eaccess(GRADES, 0) != -1) {
		Grade = 'A';
		Sgrades = TRUE;
		sprintf(Sgrade, "%s", "default");
	}

	/*
	 * find name of local system
	 */
	uucpname(Myname);
	Optns[0] = '-';
	Optns[1] = 'd';
	Optns[2] = 'c';
	Optns[3] = Nuser[0] = Sfile[0] = NULLCHAR;

	/*
	 * find id of user who spawned command to 
	 * determine
	 */
	(void) guinfo(Uid, User);

	/*
	 * create/append command log
	 */
	commandlog(argc,argv);

	while ((ret = getopt(argc, argv, "Ccdfg:jmn:rs:x:")) != EOF) {
		switch (ret) {

		/*
		 * make a copy of the file in the spool
		 * directory.
		 */
		case 'C':
			Copy = 1;
			Optns[2] = 'C';
			break;

		/*
		 * not used (default)
		 */
		case 'c':
			break;

		/*
		 * not used (default)
		 */
		case 'd':
			break;
		case 'f':
			Optns[1] = 'f';
			break;

		/*
		 * set service grade
		 */
		case 'g':
			sprintf(Xopts+strlen(Xopts), "-g%s", optarg);
			if (!Sgrades) {
				if (strlen(optarg) < (size_t)2 && isalnum(*optarg)) 
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
		 * send notification to local user
		 */
		case 'm':
			Mail = 1;
			(void) strcat(Optns, "m");
			break;

		/*
		 * send notification to user on remote
		 * if no user specified do not send notification
		 */
		case 'n':
			Notify = 1;
			(void) strcat(Optns, "n");
			(void) sprintf(Nuser, "%.8s", optarg);
			(void) sprintf(Uopts+strlen(Uopts), "-n%s ", Nuser);
			break;

		/*
		 * create JCL files but do not start uucico
		 */
		case 'r':
			Ropt = "-r";
			break;

		/*
		 * return status file
		 */
		case 's':
			fopt = optarg;
			/* "m" needed for compatability */
			(void) strcat(Optns, "mo");
			break;

		/*
		 * turn on debugging
		 */
		case 'x':
			Debug = atoi(optarg);
			if (Debug <= 0)
				Debug = 1;
#ifdef SMALL
			fprintf(stderr,
			"WARNING: uucp built with SMALL flag defined -- no debug info available\n");
#endif /* SMALL */
			break;

		default:
			usage();
			break;
		}
	}
	DEBUG(4, "\n\n** %s **\n", "START");
	gwd(Wrkdir);
	if (fopt) {
		if (*fopt != '/')
			(void) sprintf(Sfile, "%s/%s", Wrkdir, fopt);
		else
			(void) sprintf(Sfile, "%s", fopt);

	}
	else
		strcpy (Sfile, "dummy");

	/*
	 * work in WORKSPACE directory
	 */
	ret = chdir(WORKSPACE);
	if (ret != 0) {
		(void) fprintf(stderr, "No work directory - %s - get help\n",
		    WORKSPACE);
		cleanup(-12);
	}

	if (Nuser[0] == NULLCHAR)
		(void) strcpy(Nuser, User);
	(void) strcpy(Loginuser, User);
	DEBUG(4, "UID %ld, ", (long) Uid);
	DEBUG(4, "User %s\n", User);
	if (argc - optind < 2) {
		usage();
	}

	/*
	 * set up "to" system and file names
	 */

	(void) split(argv[argc - 1], sys2, fwd2, file2);
	if (*sys2 != NULLCHAR) {
		if (versys(sys2) != 0) {
			(void) fprintf(stderr, "bad system: %s\n", sys2);
			cleanup(-11);
		}
	}
	else
		(void) strcpy(sys2, Myname);

	(void) strncpy(Rmtname, sys2, MAXBASENAME);
	Rmtname[MAXBASENAME] = NULLCHAR;

	DEBUG(9, "sys2: %s, ", sys2);
	DEBUG(9, "fwd2: %s, ", fwd2);
	DEBUG(9, "file2: %s\n", file2);

	/*
	 * if there are more than 2 argsc, file2 is a directory
	 */
	if (argc - optind > 2)
		(void) strcat(file2, "/");

	/*
	 * do each from argument
	 */

	for ( ; optind < argc - 1; optind++) {
	    (void) split(argv[optind], sys1, fwd1, file1);
	    if (*sys1 != NULLCHAR) {
		if (versys(sys1) != 0) {
			(void) fprintf(stderr, "bad system: %s\n", sys1);
			cleanup(-11);
		}
	    }

	    /*  source files can have at most one ! */
	    if (*fwd1 != NULLCHAR) {
		/* syntax error */
	        (void) fprintf(stderr, "illegal  syntax %s\n", argv[optind]);
	        exit(2);
	    }

	    /*
	     * check for required remote expansion of file names -- generate
	     *	and execute a uux command
	     * e.g.
	     *		uucp   owl!~/dan/*   ~/dan/
	     *
	     * NOTE: The source file part must be full path name.
	     *  If ~ it will be expanded locally - it assumes the remote
	     *  names are the same.
	     */

	    if (*sys1 != NULLCHAR)
		if ((strchr(file1, '*') != NULL
		      || strchr(file1, '?') != NULL
		      || strchr(file1, '[') != NULL)) {
		        /* do a uux command */
		        if (ckexpf(file1) == FAIL)
			    exit(6);
		        ruux(sys1, sys1, file1, sys2, fwd2, file2);
		        continue;
		}

	    /*
	     * check for forwarding -- generate and execute a uux command
	     * e.g.
	     *		uucp uucp.c raven!owl!~/dan/
	     */

	    if (*fwd2 != NULLCHAR) {
	        ruux(sys2, sys1, file1, "", fwd2, file2);
	        continue;
	    }

	    /*
	     * check for both source and destination on other systems --
	     *  generate and execute a uux command
	     */

	    if (*sys1 != NULLCHAR )
		if ( (!EQUALS(Myname, sys1))
	    	  && *sys2 != NULLCHAR
	    	  && (!EQUALS(sys2, Myname)) ) {
		    ruux(sys2, sys1, file1, "", fwd2, file2);
	            continue;
	        }


	    if (*sys1 == NULLCHAR)
		(void) strcpy(sys1, Myname);
	    else {
		(void) strncpy(Rmtname, sys1, MAXBASENAME);
		Rmtname[MAXBASENAME] = NULLCHAR;
	    }

	    DEBUG(4, "sys1 - %s, ", sys1);
	    DEBUG(4, "file1 - %s, ", file1);
	    DEBUG(4, "Rmtname - %s\n", Rmtname);
	    if (copy(sys1, file1, sys2, file2))
	    	errors++;
	}

	/* move the work files to their proper places */
	commitall();

	/*
	 * do not spawn daemon if -r option specified
	 */
	if (*Ropt != '-') {
#ifndef	V7
		long	limit;
		char	msg[100];
		limit = ulimit(1, (long) 0);
		if (limit < MINULIMIT)  {
			(void) sprintf(msg,
			    "ULIMIT (%ld) < MINULIMIT (%ld)", limit, MINULIMIT);
			logent(msg, "Low-ULIMIT");
		}
		else
#endif
			xuucico(Rmtname);
	}
	if (jflag) {
		(void) strncpy(Jobid, jid(), NAMESIZE);
		printf("%s\n", Jobid);
	}
	cleanup(errors);
	/*NOTREACHED*/
}

/*
 * cleanup lock files before exiting
 */
void
cleanup(code)
register int	code;
{
	rmlock(CNULL);
	if (code != 0)
		wfabort();	/* this may be extreme -- abort all work */
	if (code < 0) {
	       (void) fprintf(stderr, "uucp failed completely (%d)\n", (-code));
		exit(-code);
	}
	else if (code > 0) {
		(void) fprintf(stderr, 
		"uucp failed partially: %d file(s) sent; %d error(s)\n ",
		 _Transfer, code);
		exit(code);
	}
	exit(code);
}

/*
 * generate copy files for s1!f1 -> s2!f2
 *	Note: only one remote machine, other situations
 *	have been taken care of in main.
 * return:
 *	0	-> success
 * Non-zero     -> failure
 */
int
copy(s1, f1, s2, f2)
char *s1, *f1, *s2, *f2;
{
	FILE *cfp;
	static FILE *syscfile();
	struct stat stbuf, stbuf1;
	int type, statret;
	char dfile[NAMESIZE];
	char cfile[NAMESIZE];
	char file1[MAXFULLNAME], file2[MAXFULLNAME];
	char msg[BUFSIZ];

	type = 0;
	(void) strcpy(file1, f1);
	(void) strcpy(file2, f2);
	if (!EQUALS(s1, Myname))
		type = 1;
	if (!EQUALS(s2, Myname))
		type = 2;

	switch (type) {
	case 0:

		/*
		 * all work here
		 */
		DEBUG(4, "all work here %d\n", type);

		/*
		 * check access control permissions
		 */
		if (ckexpf(file1))
			 return(-6);
		if (ckexpf(file2))
			 return(-7);
		if (uidstat(file1, &stbuf) != 0) {
			(void) fprintf(stderr,
			    "can't get file status %s\n copy failed\n", file1);
			return(8);
		}
		statret = uidstat(file2, &stbuf1);
		if (statret == 0
		  && stbuf.st_ino == stbuf1.st_ino
		  && stbuf.st_dev == stbuf1.st_dev) {
			(void) fprintf(stderr,
			    "%s %s - same file; can't copy\n", file1, file2);
			return(5);
		}
		if (chkperm(file1, file2, strchr(Optns, 'd')) ) {
			(void) fprintf(stderr, "permission denied\n");
			cleanup(1);
		}
		if ((stbuf.st_mode & S_IFMT) == S_IFDIR) {
			(void) fprintf(stderr, "directory name illegal - %s\n",
			  file1);
			return(9);
		}
		/* see if I can read this file as read uid, gid */
		if ( !(stbuf.st_mode & ANYREAD)
		  && !(stbuf.st_uid == Uid && stbuf.st_mode & 0400)
		  && !(stbuf.st_gid == getgid() && stbuf.st_mode & 0040)
		  ) {
			(void) fprintf(stderr,
			    "uucp can't read (%s) mode (%lo)\n",
			    file1, (long) stbuf.st_mode);
			(void) fprintf(stderr, "use cp command\n");
			return(3);
		}
		if (statret == 0 && (stbuf1.st_mode & ANYWRITE) == 0) {
			(void) fprintf(stderr,
			    "can't write file (%s) mode (%lo)\n",
			    file2, (long) stbuf1.st_mode);
			return(4);
		}

		/*
		 * copy file locally
		 */
		DEBUG(2, "local copy:  uidxcp(%s, ", file1);
		DEBUG(2, "%s\n", file2);

		/* do copy as Uid, but the file will be owned by uucp */
		if (uidxcp(file1, file2) == FAIL) {
			(void) fprintf(stderr,
			    "can't copy file (%s) errno %d\n", file2, errno);
			return(5);
		}
		/*
		 * doing ((mode & 0777) | 0666) so that file mode will be
		 * 666, but setuid bit will NOT be preserved. if do only
		 * (mode | 0666), then a local uucp of a setuid file will 
		 * create a file owned by uucp with the setuid bit on
		 * (lesson 1: how to give away your Systems file . . .).
		 */
		(void) chmod(file2, (int) ((stbuf.st_mode & LEGALMODE) | PUB_FILEMODE));
		/*
		 * if user specified -m, notify "local" user
		 */
		 if ( Mail ) {
		 	sprintf(msg,
		 	"REQUEST: %s!%s --> %s!%s (%s)\n(SYSTEM %s) copy succeeded\n",
		 	s1, file1, s2, file2, User, s2 );
		 	mailst(User, msg, "", "");
		}
		/*
		 * if user specified -n, notify "remote" user
		 */
		if ( Notify ) {
			sprintf(msg, "%s from %s!%s arrived\n",
				file2, s1, User );
			mailst(Nuser, msg, "", "");
		}
		return(0);
	case 1:

		/*
		 * receive file
		 */
		DEBUG(4, "receive file - %d\n", type);

		/*
		 * expand source and destination file names
		 * and check access permissions
		 */
		if (file1[0] != '~')
			if (ckexpf(file1))
				 return(6);
		if (ckexpf(file2))
			 return(7);


		gename(DATAPRE, s2, Grade, dfile);

		/*
		 * insert JCL card in file
		 */
		cfp = syscfile(cfile, s1);
		(void) fprintf(cfp, 
	       	"R %s %s %s %s %s %o %s %s\n", file1, file2,
			User, Optns,
			*Sfile ? Sfile : "dummy",
			0777, Nuser, dfile);
		(void) fclose(cfp);
		(void) sprintf(msg, "%s!%s --> %s!%s", Rmtname, file1,
		    Myname, file2);
		logent(msg, "QUEUED");
		break;
	case 2:

		/*
		 * send file
		 */
		if (ckexpf(file1))
			 return(6);
		/* XQTDIR hook enables 3rd party uux requests (cough) */
		if (file2[0] != '~' && !EQUALS(Wrkdir, XQTDIR))
			if (ckexpf(file2))
				 return(7);
		DEBUG(4, "send file - %d\n", type);

		if (uidstat(file1, &stbuf) != 0) {
			(void) fprintf(stderr,
			    "can't get status for file %s\n", file1);
			return(8);
		}
		if ((stbuf.st_mode & S_IFMT) == S_IFDIR) {
			(void) fprintf(stderr,
			    "directory name illegal - %s\n", file1);
			return(9);
		}
		/* see if I can read this file as read uid, gid */
		if ( !(stbuf.st_mode & ANYREAD)
		  && !(stbuf.st_uid == Uid && stbuf.st_mode & 0400)
		  && !(stbuf.st_gid == getgid() && stbuf.st_mode & 0040)
		  ) {
			(void) fprintf(stderr,
			    "uucp can't read (%s) mode (%o)\n",
			    file1, stbuf.st_mode);
			return(3);
		}

		/*
		 * make a copy of file in spool directory
		 */

		gename(DATAPRE, s2, Grade, dfile);

		if (Copy || !READANY(file1) ) {

			if (uidxcp(file1, dfile))
			    return(5);

			(void) chmod(dfile, DFILEMODE);
		}

		cfp = syscfile(cfile, s2);
		(void) fprintf(cfp, "S  %s %s %s %s %s %lo %s %s\n",
		    file1, file2, User, Optns, dfile,
		    (long) stbuf.st_mode & LEGALMODE, Nuser, Sfile);
		(void) fclose(cfp);
		(void) sprintf(msg, "%s!%s --> %s!%s", Myname, file1,
		    Rmtname, file2);
		logent(msg, "QUEUED");
		break;
	}
	_Transfer++;
	return(0);
}


/*
 *	syscfile(file, sys)
 *	char	*file, *sys;
 *
 *	get the cfile for system sys (creat if need be)
 *	return stream pointer
 *
 *	returns
 *		stream pointer to open cfile
 *		
 */

static FILE	*
syscfile(file, sys)
char	*file, *sys;
{
	FILE	*cfp;

	if (gtcfile(file, sys) == FAIL) {
		gename(CMDPRE, sys, Grade, file);
		ASSERT(access(file, 0) != 0, Fl_EXISTS, file, errno);
		cfp = fdopen(creat(file, CFILEMODE), "w");
		svcfile(file, sys, Sgrade);
	} else
		cfp = fopen(file, "a");
	ASSERT(cfp != NULL, Ct_OPEN, file, errno);
	return(cfp);
}


/*
 * generate and execute a uux command
 */

void
ruux(rmt, sys1, file1, sys2, fwd2, file2)
char *rmt, *sys1, *file1, *sys2, *fwd2, *file2;
{
    char cmd[BUFSIZ];
    char xcmd[BUFSIZ];
    char * xarg[6];
    int narg = 0;
    int i;

    xarg[narg++] = UUX;
    xarg[narg++] = "-C";
    if (*Xopts != NULLCHAR)
	xarg[narg++] = Xopts;
    if (*Ropt  != ' ')
	xarg[narg++] = Ropt;

    (void) sprintf(cmd, "%s!uucp -C", rmt);

    if (*Uopts != NULLCHAR)
	(void) sprintf(cmd+strlen(cmd), " (%s) ", Uopts);

    if (*sys1 == NULLCHAR || EQUALS(sys1, Myname)) {
        if (ckexpf(file1))
  	    exit(6);
	(void) sprintf(cmd+strlen(cmd), " %s!%s ", sys1, file1);
    }
    else
	if (!EQUALS(rmt, sys1))
	    (void) sprintf(cmd+strlen(cmd), " (%s!%s) ", sys1, file1);
	else
	    (void) sprintf(cmd+strlen(cmd), " (%s) ", file1);

    if (*fwd2 != NULLCHAR) {
	if (*sys2 != NULLCHAR)
	    (void) sprintf(cmd+strlen(cmd),
		" (%s!%s!%s) ", sys2, fwd2, file2);
	else
	    (void) sprintf(cmd+strlen(cmd), " (%s!%s) ", fwd2, file2);
    }
    else {
	if (*sys2 == NULLCHAR || EQUALS(sys2, Myname))
	    if (ckexpf(file2))
		exit(7);
	(void) sprintf(cmd+strlen(cmd), " (%s!%s) ", sys2, file2);
    }

    xarg[narg++] = cmd;
    xarg[narg++] = (char *) 0;

    xcmd[0] = NULLCHAR;
    for (i=0; i < narg; i++) {
	strcat(xcmd, xarg[i]);
	strcat(xcmd, " ");
    }
    DEBUG(2, "cmd: %s\n", xcmd);
    logent(xcmd, "QUEUED");

    if (fork() == 0) {
	ASSERT(setuid(getuid()) == 0, "setuid", "failed", 99);
	execv(UUX, xarg);
	exit(0);
    }
    return;
}

void
usage()
{

	(void) fprintf(stderr,
"Usage:  %s [-c|-C] [-d|-f] [-gGRADE] [-j] [-m] [-nUSER]\\\n\
	[-r] [-sFILE] [-xDEBUG_LEVEL] source-files destination-file\n",
	Progname);
	cleanup(-2);
}
