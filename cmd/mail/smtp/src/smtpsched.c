/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:smtp/src/smtpsched.c	1.4.3.1"
#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <sys/stat.h>
#include <time.h>
#include "s_string.h"
#include "smtp.h"
#include "xmail.h"

/*
 *  arghhh! I wish I knew of a better way to do this
 */
#if defined(SVR3) || defined(SVR4)
#define	SYS5	1
#endif
#ifdef SYS5

#include <dirent.h>
typedef struct dirent Direct;

#else

#ifdef BSD

#include <sys/dir.h>
#else
#include <ndir.h>
#endif
typedef struct direct Direct;

#endif

/*
 *  names of the limit files
 */
#define CON ".smtpscheds"
#define NCON ".nsmtpscheds"

#define	OLD	1*3600L		/* old C file -> try less often */
#define	VOLD	6*3600L		/* very old C file -> try much less often */
#define	OLDW	1*3600L		/* wait time between tries at old files */
#define	VOLDW	4*3600L		/* wait time between tries at very old files */

extern char *UPASROOT;
int warn = -1;
#ifdef SVR4
#define	remove	int_remove	/* remove() is def'd in stdio.h in SVR4 */
#endif
int remove = -1;
int cleanup;
int verbose;
int testmode;
int Xonly;
int Conly;
string *replyaddr;
string *dest;
int mypid;
char **getcmd();
extern char *fileoftype();

#define	MAXDEST 50		/* # destinations remembered */
#define	MAXTPERD 5*60		/* total time allowed before skipping dests */
struct {
	string	*dest;
	time_t	time;
} destlist[MAXDEST];		/* time consumed by unsuccessful tries at dest */
int	ndest = 0;

int	debug;


/*
 *  actions to take when locking
 */
#define BLOCK 0		/* wait for 5 minutes if the directory is locked */
#define SKIP 1		/* skip the directory if it is locked */
#define IGNORE 2	/* don't lock the directory of care if it is */

/*
 *  If called with arguments, those arguments are spool directories.  Descend
 *  each one processing the control files in it (C.* and X.*).
 *
 *  If called without arguments, descend all spool directories.
 *
 *  -s #scheds  specifies a maximum number of concurrent smtpscheds.
 *  -w #days	causes users to be warned if their mail files are older
 *		than #days days
 *  -r #days	causes mail older than #days days to be returned to sender
 *  -c		cleanup empty directories
 */
usage()
{
	fprintf(stderr, "smtpsched [-cvt] [-w #days] [-r #days] [-s #scheds] [dir]\n");
	exit(1);
}

main(ac, av)
	int ac;
	char *av[];
{
	DIR *dirp;
	Direct *dp;
	int max=0;
	int c;
	extern int optind;
	extern char *optarg;

	mypid = getpid();

	/*
	 *  avoid annoying distractions
	 */
	signal(SIGPIPE, SIG_IGN);
	signal(SIGHUP, SIG_IGN);

	while ((c = getopt(ac, av, "XCcvtr:w:s:d")) != EOF)
		switch (c) {
		case 'X':
			Xonly = 1;
			break;
		case 'C':
			Conly = 1;
			break;
		case 't':
			testmode = 1;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'c':
			cleanup = 1;
			break;
		case 'r':
			remove = atoi(optarg);
			break;
		case 's':
			max = atoi(optarg);
			break;
		case 'w':
			warn = atoi(optarg);
			break;
		case 'd':
			debug = 1;
			break;
		default:
			usage();
		}
	/*
	 *  go to top spool directory
	 */
	if(chdir(SMTPQROOT)<0){
		log("can't chdir to %s\n", SMTPQROOT);
		exit(1);
	}

	/*
	 *  if there are too many running exit
	 */
	if(max && toomany(max)<0)
		exit(0);

	/*
	 *  if specific directories, do just them.  Keep running the directory until there
	 *  is no change.
	 */
	if(optind!=ac){
		for(; optind<ac; optind++)
			while (dodir(av[optind], SKIP) && !warn && !remove)
				;
		return 0;
	}

	/*
	 *  walk through all directories in top directory.  the lock is
	 *  non-blocking (if neither r nor w options specified) to let
	 *  different instances of smtpsched to skip
	 *  over each other.
	 */
	dirp = opendir(".");
#ifdef SVR4
	if (dirp == (DIR *)0) {
#else
	if(dirp<0){
#endif
		log("couldn't read %s\n", SMTPQROOT);
		exit(1);
	}
	while(dp = readdir(dirp)){
		if(strcmp(dp->d_name, ".")!=0 && strcmp(dp->d_name, "..")!=0)
			dodir(dp->d_name, (warn>=0 || remove>=0) ? IGNORE : SKIP);
	}
	closedir(dirp);

	/* Rename Log file nightly */
	if (cleanup) {
		register struct tm *tp;
		char nm[256];
		time_t now;

		(void) time(&now);
		tp = localtime(&now);
		(void) sprintf(nm, "LOG.%d", tp->tm_wday);
		(void) unlink(nm);
		if (link("LOG", nm) == 0)
			(void) unlink("LOG");
	}

	return 0;
}

/*
 *  walk through all entries in this directory.  process any
 *  not starting with '.'.  lock the directory before proceeding.
 */
dodir(dname, action)
	char *dname;
{
	DIR *dirp;
	Direct *dp;
	int i;
	int changed=0;
	int ents=0;
	static string *ds;
	extern int errno;

	ds = s_reset(ds);
	s_append(ds, dname);

	/*
	 *  lock the directory.  the lock is in the parent directory.
	 */
	switch(action){
	case BLOCK:
		for(i=0; i<60; i++){
			if(lock(s_to_c(ds))==0)
				break;
			sleep(5);
		}
		if(i==60)
			return changed;
		break;
	case SKIP:
		if(lock(s_to_c(ds))<0){
			log("couldn't lock %s\n", dname);
			return changed;
		}
		break;
	case IGNORE:
		break;
	}

	/*
	 *  descend into the directory.  if it isn't a directory,
	 *  this will fail.
	 */
	if(chdir(s_to_c(ds))<0){
		unlock(s_to_c(ds));
		return changed;
	}

	/*
	 *  walk through the entries
	 */
	dirp = opendir(".");
#ifdef SVR4
	if (dirp == (DIR *)0) {
#else
	if(dirp<0){
#endif
		log("couldn't read directory %s\n", dname);
		if(chdir("..")<0){
			log("can't chdir ..\n");
			fflush(stdout);
			exit(1);
		}
		unlock(dname);
		return changed;
	}
	while(dp = readdir(dirp)){
		if(strcmp(dp->d_name, ".")!=0 && strcmp(dp->d_name, "..")!=0)
			switch (dofile(dname, dp->d_name)) {
			case 0:
				/* file removed */
				changed = 1;
				break;
			case 1:
				ents++;
				break;
			}
	}
	closedir(dirp);

	/*
	 *  go back up.  symbolic links could be painful!!!!
	 */
	if(chdir("..")<0){
		log("can't chdir ..\n");
		fflush(stdout);
		exit(1);
	}

	/*
	 *  cleanup empty directories
	 */
	if(cleanup && ents==0){
		log("%s empty\n", s_to_c(ds));
		if(rmdir(s_to_c(ds))<0)
			log("can't unlink: %d\n", errno);
	}

	if(action != IGNORE)
		unlock(s_to_c(ds));

	return changed;
}

/*
 *  process a spool control file.  control file names start with C. or
 *  X.  all error goes into an error file.
 *
 *  return 0 if file removed, 1 otherwise.
 */
dofile(dname, name)
	char *dname;
	char *name;
{
	int rv;
	int fd, ofd;
	char *ef;
	struct stat sb;
	time_t now, Edate, Cdate;

	rv = -1;

	/*
	 *  if the file is inconsistent, remove it
	 */
	if(cleanup && inconsistent(name)){
		log("%s/%s inconsistent\n", dname, name);
		unlink(name);
		return 0;
	}

	/*
	 *  if this is not a control file, ignore it
	 */
	if(name[1]!='.' || (name[0]!='C' && name[0]!='X'))
		return 1;

	/*
	 *  if file is too old, warn user and remove it.  if checking age,
	 *  don't run the control file.
	 */
	if(warn>=0 || remove>=0) {
		if(checkage(name)==0) {
			log("%s/%s too old\n", dname, name);
			doremove(name);
			return 0;
		}
		return 1;
	}

	/*
	 *  don't run control file when cleaning up
	 */
	if(cleanup)
		return 1;

	/*
	 * Backoff scheme: don't try old C files very often
	 */
	ef = fileoftype('E', name);
	if (name[0]=='C') {
		now = time((time_t *)0);
		Cdate = now;
		Edate = now-VOLDW-1;
		if (stat(name, &sb)==0)
			Cdate = sb.st_ctime;
		if (stat(ef, &sb)==0)
			Edate = sb.st_mtime;
		if (now-Cdate>VOLD && now-Edate<VOLDW
		 || now-Cdate>OLD  && now-Edate<OLDW) {
			if (verbose)
				log("ignore %s/%s: not time yet\n", dname, name);
			if (debug==0)
				return 1;
		}
	}

	/*
	 * in test mode, just return
	 */
	if (testmode) {
		log("would process %s/%s\n", dname, name);
		return 1;
	}
	/*
	 *  redirect output to an error file
	 */
	ofd = dup(2);
	close(2);
	fd = open(ef, 1);
	if(fd<0)
		fd = creat(ef, 0666);
	if(fd>=0){
		lseek(fd, 0l, 2);
	
		/*
		 *  process the file
		 */
		if(name[0]=='C') {
			rv = dosmtp(dname, name);
		} else if(name[0]=='X') {
			rv = dormail(dname, name);
		}

		/*
		 *  get old error file back
		 */
		close(2);
		(void) dup(ofd);
		close(ofd);
	}

	/*
	 *  if processing was successful, remove the spool files
	 */
	if(rv==0) {
		doremove(name);
		return 0;
	}
	return 1;
}

/*
 *  remove the control file, data file, and error file
 */
doremove(ctl)
	char *ctl;
{
	fflush(stdout);
	unlink(fileoftype('E', ctl));
	unlink(ctl);
	unlink(fileoftype('D', ctl));
}

/*
 *  run rmail.  rmail takes care of its own errors, so if rmail fails,
 *  just don't remove the files.
 */
dormail(dname, ctl)
	char *dname;
	char *ctl;
{
	char **av;

	log("dormail %s/%s\n", dname, ctl);

	/*
	 *  fork off the command
	 */
#ifdef SVR4
	av = getcmd(ctl, "/usr/bin/rmail");
#else
	av = getcmd(ctl, "/bin/rmail");
#endif
	if(av && docmd(ctl, av)==0){
		log("success\n");
		return 0;
	} else {
		log("failure\n");
		return -1;
	}
}

/*
 *  run smtp.  if an error occurs, determine its importance and send
 *  a error mail message if it is fatal.
 */
dosmtp(dname, ctl)
	char *dname;
	char *ctl;
{
	static string *cmd;
	int status, i;
	char **av;
	time_t t0, t1;

	log("dosmtp %s/%s\n", dname, ctl);

	/*
	 *  fork off the command
	 */
	cmd = s_reset(cmd);
	s_append(cmd, UPASROOT);
	s_append(cmd, SMTP);
	av = getcmd(ctl, s_to_c(cmd));
	if (av==NULL) {
		log("cmdfail\n", 0);
		return -1;
	}
	/*
	 * Check whether unsuccessful attempts at this destination
	 * have taken too much time.  If so, pass over the file.
	 */
	for (i=0; i<ndest; i++) {
		if (strcmp(s_to_c(dest), s_to_c(destlist[i].dest))==0) {
			if (destlist[i].time > MAXTPERD) {
				log("passed %s (%d sec)\n", s_to_c(dest), destlist[i].time);
				return -1;
			}
			break;
		}
	}
	if (i==ndest) {
		if (ndest<MAXDEST)
			ndest++;
		else
			i = 0;		/* loses storage */
		destlist[i].dest = s_copy(s_to_c(dest));
		destlist[i].time = 0;
	}
	if (debug)
		log("time %d for %s\n", destlist[i].time, s_to_c(dest));
	time(&t0);
	switch(status=docmd(ctl, av)){
	case 0:		/* it worked */
		log("success\n");
		destlist[i].time = 0;
		return 0;

	case 67:	/* rejected by the other side */
	case 68:
		log("fail %d\n", status);
		returnmail(ctl, 1);
		destlist[i].time = 0;
		return 0;

	default:	/* possibly a temporary problem */
		log("fail %d\n", status);
		time(&t1);
		destlist[i].time += t1-t0;
		return -1;
	}
}

/*
 *  open a control file and parse the first line.  It contains
 *  the reply address and the destination (for returning the mail).
 *
 *  It leaves the control file open and returns the fp.
 */
FILE *
parseline1(ctl)
	char *ctl;
{
	FILE *fp;
	static string *line;

	fp = fopen(ctl, "r");
	if(fp==NULL)
		return NULL;

	/*
	 *  get reply address and destination
	 */
	line = s_reset(line);
	if(s_read_line(fp, line)==NULL){
		fprintf(stderr, "smtpsched: error reading ctl file %s\n", ctl);
		fclose(fp);
		return NULL;
	}
	replyaddr = s_parse(s_restart(line), s_reset(replyaddr));
	dest = s_parse(line, s_reset(dest));
	return fp;
}

/*
 * Read control file to get arguments for command.  Leave dest and replyaddr
 * available.
 * the control file
 * has two lines.  the first is reply address and recipients.  the second is
 * the arguments for the command.
 */
char **
getcmd(ctl, cmd)
	char *ctl;
	char *cmd;
{
	static string *args;
	FILE *fp;
	static char *av[1024];
	int ac=0;
	char *cp;

	fp = parseline1(ctl);
	if (fp==NULL)
		return NULL;

	/*
	 *  make command line
	 */
	av[ac++] = cmd;
	args = s_reset(args);
	if(s_read_line(fp, args)==NULL){
		fprintf(stderr, "smtpsched: error reading ctl file %s\n", ctl);
		fclose(fp);
		return NULL;
	}
	fclose(fp);
	for(cp = s_to_c(args); *cp && ac<1023;){
		av[ac++] = cp++;
		while(*cp && !isspace(*cp))
			cp++;
		while(isspace(*cp))
			*cp++ = 0;
	}
	av[ac] = 0;
	return av;
}

/*
 *  execute a command, put standard error in the error file.
 */
docmd(ctl, av)
	char *ctl;
	char **av;
{
	int fd;
	int pid, status;
	int n;

	/*
	 *  fork off the command
	 */
	switch(pid = fork()){
	case -1:
		return 1;
	case 0:
		/*
		 *  make data file standard input
		 */
		close(0);
		fd = open(fileoftype('D', ctl), 0);
		if(fd<0){
			perror("smtpsched: error reading data file:\n");
			exit(1);
		}
	
		/*
		 *  make error file standard output
		 */
		close(1);
		fd = dup(2);
	
		/*
		 *  start the command
		 */
		execvp(av[0], av);
		exit(1);
	default:
		/*
		 *  wait for the command to terminate
		 */
		while((n = wait(&status))>=0)
			if(n == pid)
				break;
		if(status&0xff)
			return -1;
		else
			return (status>>8)&0xff;
	}

}

/*
 *  see if the number of consumers has been exceeded.  if not, add this process
 *  to the list.
 *
 *  returns 0 if there were the number was not exceeded, -1 otherwise
 */
toomany(max)
	int max;
{
	FILE *ifp=NULL;
	FILE *ofp=NULL;
	int cur=0;
	int pid;

	/*
	 *  lock consumers file
	 */
	if(lock(CON)<0)
		return -1;

	/*
	 *  open old and new consumer files
	 */
	ofp = fopen(NCON, "w");
	if(ofp==NULL){
		fprintf(stderr, "can't open %s\n", NCON);
		goto error;
	}
	ifp = fopen(CON, "r");
	if(ifp!=NULL){
		/*
		 *  see how many consumers are still around
		 */
		while(fscanf(ifp, "%d", &pid)==1){
			if(kill(pid, 0) == 0){
				cur++;
				if(fprintf(ofp, "%d\n", pid)<0){
					fprintf(stderr, "error writing %s\n", NCON);
					goto error;
				}
			}
		}
		if(cur >= max)
			goto error;
	}

	/*
	 *  add us to the group of consumers
	 */
	if(fprintf(ofp, "%d\n", getpid())<0){
		fprintf(stderr, "error writing %s\n", NCON);
		goto error;
	}
	if(ifp!=NULL)
		fclose(ifp);
	if(fclose(ofp)==EOF)
		goto error;
	unlink(CON);
	if(link(NCON, CON)<0)
		fprintf(stderr, "can't link %s to %s file\n", CON, NCON);
	unlink(NCON);
	unlock(CON);
	return 0;
error:
	/*
	 *  too many consumers or we can't make a new consumer file
	 */
	if(ifp!=NULL)
		fclose(ifp);
	if(ofp!=NULL)
		fclose(ofp);
	unlink(NCON);
	unlock(CON);
	return -1;
}

/*
 *  return true if the file is inconsistent.  The following are inconsistent:
 *	- a control file without a datafile
 *	- an error file without a datafile
 *	- a day old data file without a control file
 *	- a limit file of any kind
 */
inconsistent(file)
	char *file;
{
	struct stat s;
	int days;

	/*
	 *  switch on file type
	 */
	switch(file[0]){
	case 'C':
	case 'X':
		/*
		 *  if no data file, control file is inconsistent
		 */
		if(stat(fileoftype('D', file), &s)<0)
			return 1;
		break;
	case 'E':
		/*
		 *  if no control file, error file is inconsistent
		 */
		if(stat(fileoftype('X', file), &s)<0
		&& stat(fileoftype('C', file), &s)<0)
			return 1;

		/*
		 *  if no data file, error file is inconsistent
		 */
		if(stat(fileoftype('D', file), &s)<0)
			return 1;
		break;
	case 'D':
		/*
		 *  look for a control file
		 */
		if(stat(fileoftype('X', file), &s)==0
		|| stat(fileoftype('C', file), &s)==0)
			break;

		/*
		 *  no control file, data file inconsistent if >=1 day old
		 */
		if(stat(file, &s)<0)
			return 0;
		days = (time((long *)0) - s.st_ctime)/(24*60*60);
		if(days>0)
			return 1;
		break;
	default:
		break;
	}
	return 0;
}

/*
 *  check the age of a file.  if it is greater than warn or remove, tell the
 *  sender.  return 0 if the file is to be removed, -1 otherwise.
 */
checkage(ctl)
	char *ctl;
{
	struct stat s;
	int days;
	FILE *fp;

	/*
	 *  get the file's age
	 */
	if(stat(ctl, &s)<0)
		return -1;
	days = (time((long *)0) - s.st_ctime)/(24*60*60);

	/*
	 *  check for removal
	 */
	if(remove>=0 && days>=remove){
		fp = parseline1(ctl);
		if(fp==NULL)
			return -1;
		fclose(fp);

		log("r %d %d\n", days, remove);
		return returnmail(ctl, 1);
	}

	/*
	 *  check for warning
	 */
	if(warn>=0 && days>=warn){
		fp = parseline1(ctl);
		if(fp==NULL)
			return -1;
		fclose(fp);

		log("w %d %d\n", days, warn);
		returnmail(ctl, 0);
	}

	return -1;
}

/*
 *  return a piece of mail with a reason for the return
 */
returnmail(ctl, warn)
	char *ctl;
{
	int pid, status;
	int pfd[2];
	char buf[132];
	int n, reads;
	FILE *fp;
	FILE *ifp;
	long now;
	char asct[27];

	log("returnmail %s to %s about %s\n", ctl, s_to_c(replyaddr),
		s_to_c(dest));

	if(pipe(pfd)<0)
		return -1;

	switch(pid=fork()){
	case -1:
		close(pfd[0]);
		close(pfd[1]);
		return -1;
	case 0:
		/*
		 *  start up the mailer to take the refusal message
		 */
		close(0);
		dup(pfd[0]);
		close(pfd[1]);
#ifdef SVR4
		execl("/usr/bin/rmail", "/usr/bin/rmail",  s_to_c(replyaddr), (char *)0);
#else
		execl("/bin/rmail", "/bin/rmail",  s_to_c(replyaddr), (char *)0);
#endif
		exit(1);
	default:
		/*
		 *  pipe the refusal message to the mailer
		 */
		close(pfd[0]);
		fp = fdopen(pfd[1], "w");
		if(fp==NULL) {
			close(pfd[1]);
			break;
		}

		/*
		 *  the From line
		 */
		now = time((long *)0);
		strcpy(asct, ctime(&now));
		asct[24] = 0;
		fprintf(fp, "From postmaster %s\n", asct);

		/*
		 *  the refusal message
		 */
		if(warn) {
			fprintf(fp, "Subject: smtp mail failed\n\n");
			fprintf(fp, "Your mail to %s is undeliverable.\n",
				s_to_c(dest));
		} else {
			fprintf(fp, "Subject: smtp mail warning\n\n");
			fprintf(fp, "Your mail to %s is not yet delivered.\n",
				s_to_c(dest));
			fprintf(fp, "Delivery attempts continue.\n");
		}

		/*
		 *  then diagnosis of error
		 */
		fprintf(fp, "---------- diagnosis ----------\n");
		ifp = fopen(fileoftype('E', ctl), "r");
		if(ifp!=NULL){
			for(reads=0; reads<20; reads++) {
				if(fgets(buf, sizeof(buf), ifp)==NULL)
					break;
				fputs(buf, fp);
			}
			fclose(ifp);
		}

		/*
		 *  finally the message itself
		 */
		fprintf(fp, "---------- unsent mail ----------\n");
		ifp = fopen(fileoftype('D', ctl), "r");
		if(ifp!=NULL){
			for(reads=0; reads<50; reads++) {
				if(fgets(buf, sizeof(buf), ifp)==NULL)
					break;
				fputs(buf, fp);
			}
			fclose(ifp);
		}
		fclose(fp);

		/*
		 *  wait for the warning to finish
		 */
		while((n = wait(&status))>=0)
			if(n == pid)
				break;
		return status ? -1 : 0;
	}
	close(pfd[1]);
	return -1;
}

char progname[] = "smtpsched";

log(f, a, b, c)
char *f;
{
	char msg[256];
	char *p;

	sprintf(msg, f, a, b, c);
	/* Strip any trailing newline.  This is done here so that the
	   log() messages can remain the same from the research version */
	p = &msg[strlen(msg)-1];
	if (*p == '\n')
		*p = '\0';
	smtplog(msg);
}
