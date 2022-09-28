/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)login:login.c	1.43.9.6"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

/*
 * Usage: login [ -d device ] [ name ] [ environment args ]
 *
 *	Conditional assemblies:
 *
 *	NO_MAIL	causes the MAIL environment variable not to be set
 *		specified by CONSOLE.  CONSOLE MUST NOT be defined as
 *		either "/dev/syscon" or "/dev/systty"!!
 *	MAXTRYS is the number of attempts permitted.  0 is "no limit".
 */

#include <sys/types.h>
#include <utmpx.h>
#include <lastlog.h>
#include <signal.h>
#include <pwd.h>
#include <stdio.h>
#ifdef	SECURITY
#include <fcntl.h>

#ifdef i386
#include <unistd.h>
#endif

#endif	/* SECURITY */

#ifndef i386
#include <unistd.h>	/* For logfile locking */
#endif

#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/utsname.h>
#include <utime.h> 
#include <termio.h>
#include <sys/stropts.h>
#include <shadow.h>	/* shadow password header file */
#include <time.h>
#include <sys/param.h> 
#include <sys/fcntl.h> 
#include <deflt.h>
#include <grp.h>
#ifdef LIMITED		/* per-user licensing */
#include <errno.h>
#include <sys/sysi86.h>
#endif
#ifndef	MAXTRYS
#	define	MAXTRYS	5	/* default */
#endif

#ifndef i386
#ifndef	MAXTIME
#	define	MAXTIME	60	/* default */
#endif
#endif

#ifdef	SECURITY
#ifndef	NFAILURES
#	define	NFAILURES 5	/* default */
#endif
#define	LOGINLOG	"/var/adm/loginlog"	/* login log file */
#define LNAME_SIZE	20	/* size of logged logname */
#define TTYN_SIZE	15	/* size of logged tty name */
#define TIME_SIZE	30	/* size of logged time string */
#define ENT_SIZE	68	/* last three numbers + 3 */
#define L_WAITTIME	5	/* waittime for log file to unlock */
#endif	/* SECURITY */

#ifndef	SLEEPTIME
#	define	SLEEPTIME 4	/* sleeptime before login incorrect msg */
#endif
#ifndef	DISABLETIME
#	define	DISABLETIME	20	/* seconds login disabled after
					   NFAILURES or MAXTRYS unsuccessful 
					   attempts */
#endif

#define SCPYN(a, b)	strncpy(a, b, sizeof(a))
#define EQN(a, b)	(!strncmp(a, b, sizeof(a)-1))
#define ENVSTRNCAT(to, from) {int deflen; deflen = strlen(to);\
		strncpy((to) + deflen, (from), sizeof(to) - (1 + deflen));}
#define DIAL_FILE	"/etc/dialups"
#define DPASS_FILE	"/etc/d_passwd"
#define SHELL		"/usr/bin/sh"
#define SHELL2		"/sbin/sh"
#define SUBLOGIN	"<!sublogin>"
#define LASTLOG		"/var/adm/lastlog"

#define	ROOTUID	 0
#define PBUFSIZE 8	/* max significant characters in a password */

#define	MAXARGS 63
#define	MAXENV 1024
#define MAXLINE 256

#define NMAX	sizeof(utmp.ut_name)

/*	Illegal passwd entries.
*/
static struct passwd nouser = { "", "no:password", ~ROOTUID };
static struct spwd noupass = { "", "no:password" };

static struct utsname un;
static struct utmpx utmp;
static char u_name[64];
static char minusnam[16] = "-";
static char shell[256] = { "SHELL=" };
static char home[256] = { "HOME=" };
static char term[64] = { "TERM=" };
static char logname[30] = { "LOGNAME=" };
static char timez[100] = { "TZ=" };
static char hertz[10] = { "HZ=" };
static char path[64] = { "PATH=" };

static char loginmsg[] = "login: ";
static char passwdmsg[] = "Password:";
static char incorrectmsg[] = "Login incorrect\n";

#ifndef	NO_MAIL
static char mail[30] = { "MAIL=/var/mail/" };
#endif

static char *envinit[10+MAXARGS] = 
	{home, path, logname, hertz, timez, term, 0, 0};
static int basicenv;
static int intrupt;
static char envblk[MAXENV];
static struct passwd *pwd;
static struct spwd *shpwd ;	/* Shadow password structure */

struct passwd *getpwnam();
struct spwd *getspnam() ;
char *crypt();
char *getpass(), *fgetpass();
char *strrchr(),*strchr(),*strcat();
extern char **environ;

static time_t now;
extern long atol();

extern void setbuf();

#ifdef i386
extern void getterm();
#endif

extern int findiop();

#ifdef	SECURITY
static char *log_entry[NFAILURES] ;
static int writelog=0 ;
#endif

char	*Pndefault	= "/etc/default/login";
char	*Altshell	= NULL;
char	*Console	= NULL;
int	Idleweeks	= -1;
char	*Passreq	= NULL;
#define	DEFUMASK	022
mode_t	Umask		= DEFUMASK;

char 	*Def_tz		= NULL;
#define	DEF_TZ		"EST5EDT"

char 	*Def_hertz	= NULL;

#define SET_FSIZ	2			/* ulimit() command arg */
long	Def_ulimit	= 0;

#define MAX_TIMEOUT	(15 * 60)
#define DEF_TIMEOUT	60
unsigned Def_timeout	= DEF_TIMEOUT;

char	*Def_path	= NULL;
char	*Def_supath	= NULL;
#define DEF_PATH	"/usr/bin" 	/* same as PATH */
#define DEF_SUPATH	"/sbin:/usr/sbin:/usr/bin:/etc" /* same as ROOTPATH */

extern	long	atol();
extern	char	*defread(), *basename(), *strdup();
extern	int	defopen();

/*
 * ttyprompt will point to the environment variable TTYPROMPT.
 * TTYPROMPT is set by ttymon if ttymon already wrote out the prompt.
 */
char	*ttyprompt = NULL;

char 	*ttyn = NULL;
static	struct	group	*grpstr;
struct	group	*getgrnam();
char	*ttygrp = "tty";
int	hflag, rflag;
int	valid_luser = 0;	/* for remote, is this user valid? */
void	uppercaseterm();
extern	int	optind;
extern	char	*optarg;
static	int	get_options();
static	void	usage();
static	int doremotelogin(), doremoteterm();
int	lastlogok = 0;
struct lastlog ll, newll;
int	usererr = -1;
char	rusername[NMAX+1], lusername[NMAX+1];
char	rpassword[NMAX+1];
char	terminal[64];
char	*zero = (char *)0;

main(argc, argv ,renvp)
char **argv,**renvp;
{

#ifndef i386
	register char *namep;
#endif

	int j,k,l_index,length;
	char *ttyntail, *envtz, *getenv();
	int nopassword = 1;
	register int i;
	register struct utmpx *u;
	struct utmp ut;
	struct utmpx *getutxent(), *pututxline();
	int	fd, fdl;
	char **envp,*ptr,*endptr;
	int sublogin;
	extern char **getargs();
	extern char *findttyname();
	char inputline[MAXLINE];
	int n;

#ifdef	SECURITY
	long timenow ;
	struct stat dbuf ;
#endif

#if	MAXTRYS > 0
	int trys = 0;
#endif

	defaults();

	/*Set flag to disable the pid check if you find that you are	*/
	/*a subsystem login.						*/

	sublogin = 0;
	if( *renvp && strcmp(*renvp,SUBLOGIN) == 0 )
		sublogin = 1;

	if ( Umask < 0 || ((mode_t) 0777) < Umask )
		Umask = DEFUMASK;
	umask(Umask);
	if (Def_timeout > MAX_TIMEOUT)
		Def_timeout = MAX_TIMEOUT;
	alarm(Def_timeout);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	nice(0);
	if (get_options(argc, argv) == -1) {
		usage();
		exit(1);
	}
	/* if devicename is not passed as argument, call ttyname(0) */
	if( ttyn==NULL ) {
		ttyn = findttyname(0);
		if( ttyn==NULL )
			ttyn = "/dev/???";
	}

/* skip over "/dev/" */

	ttyntail = basename(ttyn);

#ifdef	SECURITY
	/* if the logfile exist, turn on attempt logging and
	   initialize the string storage area */
	if ( stat (LOGINLOG, &dbuf) == 0 )
		{
		writelog = 1 ;
		for ( i = 0 ; i < NFAILURES ; i++ )
			{
			if ( !(log_entry[i] = (char *) malloc ((unsigned) ENT_SIZE) ) )
				{
				writelog = 0 ;
				break ;
				}
			* log_entry[i] = '\0' ;
			}
		}
#endif

	if (rflag) {
		usererr = doremotelogin(argv[optind]);
		SCPYN(utmp.ut_host, argv[optind++]);
		doremoteterm(terminal);
	}
	else if (hflag) {
		SCPYN(utmp.ut_host, argv[optind++]);
		if (argv[optind][0] != '-')
			SCPYN(terminal, argv[optind]);
		optind++;
	}
	/*
	 * get the prompt set by ttymon
	 */
	ttyprompt = getenv("TTYPROMPT");
	if ((ttyprompt != NULL) && (*ttyprompt != '\0')) {
		/*
		 * if ttyprompt is set, there should be data on
		 * the stream already. 
		 */
		if( (envp = getargs(inputline)) != (char**)NULL )
		{
			uppercaseterm(*envp);
			SCPYN(utmp.ut_user,*envp);
			SCPYN(u_name, *envp++);
			goto first;
		}
	}
	else if ( optind < argc ) {
		SCPYN(utmp.ut_user, argv[optind]);
		SCPYN(u_name, argv[optind]);
		strcpy(inputline, u_name);
		strcat(inputline, "   \n");
		envp = &argv[optind+1];
		goto first;
	}

loop:
#ifdef	SECURITY
	/* If logging is turned on and there is an unsuccessful
	   login attempt, put it in the string storage area */
	if ( (trys > 0) && (writelog == 1) )
		{
		time (&timenow) ;
		(void) strncat ( log_entry[trys-1], u_name, LNAME_SIZE ) ;
		(void) strncat ( log_entry[trys-1], ":", (size_t) 1 ) ;
		(void) strncat ( log_entry[trys-1], ttyn, TTYN_SIZE ) ;
		(void) strncat ( log_entry[trys-1], ":", (size_t) 1 ) ;
		(void) strncat ( log_entry[trys-1], ctime(&timenow), TIME_SIZE ) ;
		}
	if ( trys >= NFAILURES )
		{
		/* If logging is turned on, output the string storage
		   area to the log file, and sleep for DISABLETIME 
		   seconds before exiting. */
		if ( writelog )
			badlogin () ;
		sleep (DISABLETIME) ;
		exit (1) ;
		}
#endif	/* SECURITY */
	u_name[0] = utmp.ut_user[0] = '\0';
first:
	fflush(stdout);
#if	MAXTRYS > 0
	if( ++trys > MAXTRYS )
	{
		sleep (DISABLETIME);
		exit(1);
	}
#endif
	while( utmp.ut_user[0] == '\0' )
	{
		if (rflag && valid_luser) {
			SCPYN(utmp.ut_user, lusername);
			SCPYN(u_name, lusername);
			envp = &zero;
		}
		else {
			/*
			 * if TTYPROMPT is not set, print out our own prompt
			 * otherwise, print out ttyprompt
			 */
			if ((ttyprompt == NULL) || (*ttyprompt == '\0'))
				fputs(loginmsg, stdout);
			else
				fputs(ttyprompt, stdout);
			fflush(stdout);
			if( (envp = getargs(inputline)) != (char**)NULL )
			{
				SCPYN(utmp.ut_user,*envp);
				SCPYN(u_name, *envp++);
			}
		}
	}

	/* If any of the common login messages was the input, we must be
	 * looking at a tty that is running login.  We exit because
	 * they will chat at each other until one times out otherwise.
	 * In time, init(1M) sees this and decides something is amiss.
	 */
	if( EQN(loginmsg, inputline)  ||  EQN(passwdmsg, inputline)  ||
	    EQN(incorrectmsg, inputline) )
	{
		printf("Looking at a login line.\n");
		exit(8);
	}
	  
	setpwent();
	(void) setspent () ;	/* Setting the shadow password file */

	if ( (pwd = getpwnam (u_name) ) == NULL ||
	     ( shpwd = getspnam (u_name) ) == NULL) 
	{
		pwd = &nouser ;
		shpwd = &noupass ;
	}

	(void) endspent () ;	/* Closing the shadow password file */
	endpwent();

	if (pwd->pw_uid == 0) {
		if ((Console != NULL) && (strcmp(ttyn, Console) != 0)) {
			printf("Not on system console\n");
			exit(10);
		}
		if (Def_supath != NULL)
			Def_path = Def_supath;
		else
			Def_path = DEF_SUPATH;
	}

	if ( usererr == -1 && *shpwd->sp_pwdp != '\0' ) 
	{
		if( gpass(passwdmsg, shpwd->sp_pwdp) ) {
			dialpass(ttyn);
			sleep (SLEEPTIME) ;
			printf( incorrectmsg );
			goto loop;
		}
		nopassword = 0;
	}

	/*
	 * get dialup password, if necessary
	 */
	if( dialpass(ttyn) ) {
		sleep (SLEEPTIME) ;
		printf( incorrectmsg );
		goto loop;
	}

	/*
	 * optionally adjust nice(2)
	 */
	if( strncmp("pri=", pwd->pw_gecos, 4) == 0 )
	{
		int mflg, pri;

		pri = 0;
		mflg = 0;
		i = 4;
		if( pwd->pw_gecos[i] == '-' )
		{
			mflg++;
			i++;
		}
		while( pwd->pw_gecos[i] >= '0' && pwd->pw_gecos[i] <= '9' )
			pri = (pri * 10) + pwd->pw_gecos[i++] - '0';
		if( mflg )
			pri = -pri;
		nice(pri);
	}

	if( chdir(pwd->pw_dir) < 0 )
	{
		printf("Unable to change directory to \"%s\"\n",pwd->pw_dir);
		goto loop;
	}
	time(&utmp.ut_tv.tv_sec);
	utmp.ut_pid = getpid();

	/*Find the entry for this pid (or line if we are a sublogin) in	*/
	/*the utmp file.						*/


	while( (u = getutxent()) != NULL )
	{
		if( (u->ut_type == INIT_PROCESS ||
			u->ut_type == LOGIN_PROCESS ||
			u->ut_type == USER_PROCESS) &&
			( (sublogin && strncmp(u->ut_line,ttyntail,
			sizeof(u->ut_line)) == 0) || u->ut_pid == utmp.ut_pid) )
		{

	/* Copy in the name of the tty minus the "/dev/", the id, and set */
	/* the type of entry to USER_PROCESS.				  */

			SCPYN(utmp.ut_line,(ttyn+sizeof("/dev/")-1));
			utmp.ut_id[0] = u->ut_id[0];
			utmp.ut_id[1] = u->ut_id[1];
			utmp.ut_id[2] = u->ut_id[2];
			utmp.ut_id[3] = u->ut_id[3];
			utmp.ut_type = USER_PROCESS;

	/* Return the new updated utmp file entry. */

			pututxline(&utmp);

			break;
		}
	}
	endutxent();		/* Close utmp file */

	if( u == (struct utmpx *)NULL )
	{
		printf("No utmpx entry.  You must exec \"login\" from\
 the lowest level \"sh\".\n");
		exit(1);
	} else {
		/* Now attempt to write out this entry to the wtmp file if */
		/* we were successful in getting it from the utmp file and */
		/* the wtmp file exists. 			           */

		updwtmpx(WTMPX_FILE, &utmp);
	}
#ifdef LIMITED
/* 
 * If you have a limited binary license, then we only allow a LIMITED
 * number of concurrent users.
 */
	{ /* Start LIMITED Block */
	int login_limit;
	struct utmpx *tu;
	int uucpid = 0;
	if ((login_limit = sysi86(SI86LIMUSER, 0)) > 0) {
	  int users = 0;
	  int rootin = 0;
	  while ((tu = getutxent()) != NULL)
		if (tu->ut_type == USER_PROCESS) {
			if (strcmp(tu->ut_user, "root") == 0)
				rootin++;
		}
	  endutxent();
/*
 * If I'm root login'ing in, utmp has root written in (on line 386).
 * Therefore, subtract 1 from rootin if I'm root.
 */
	  if (pwd->pw_uid == 0)
		rootin--;
	  users = sysi86(SI86LIMUSER, 1);
/*
 * Only root can log into the system one time, once login_limit has been reached
 * The uucico shell has un-unlimited login power
 */
	  if (strcmp (pwd->pw_shell, "/usr/lib/uucp/uucico") == 0)
		uucpid++;
	  if ((users >= login_limit) && !uucpid) {
		if ((pwd->pw_uid != 0) ||
		   ((pwd->pw_uid == 0) && rootin)) {
			printf("Sorry, your login was unsuccessful.\n\n\
 This system has been configured to support %d users,\n\
 there are currently this many users logged in.\n\nPlease try again later.\n", login_limit);
			exit(1);
		}
	  }
	}
	errno = 0;
	if (((login_limit = sysi86(SI86LIMUSER, (4 + uucpid))) != 0) && (errno == EINVAL))
		; /* Old kernel, new login OK */
	else switch (errno) {
		case 0: /* successful enable */
			break;
		default: printf ("Cannot enable user; errno = %d\n", errno);
			exit (1);
	}
	} /* End LIMITED Block */
#endif /* LIMITED */


	/* Check for login expiration */

	if ( shpwd->sp_expire > 0 ) {
		if ( shpwd->sp_expire < DAY_NOW ) {
			printf( incorrectmsg );
			exit(1);
		}
	}

	lastlogok = 0;
	if ((fdl = open(LASTLOG, O_RDWR|O_CREAT, 0444)) >= 0) {
		lseek(fdl, (long)pwd->pw_uid * sizeof(struct lastlog), 0);
		if (read(fdl, (char *)&ll, sizeof(ll)) == sizeof(ll) &&
			ll.ll_time != 0)
			lastlogok = 1;
		lseek(fdl, (long)pwd->pw_uid * sizeof(struct lastlog), 0);
		time(&newll.ll_time);
		SCPYN(newll.ll_line, (ttyn+sizeof("/dev/")-1));
		SCPYN(newll.ll_host, utmp.ut_host);

		/* Check for login inactivity */

		if (( shpwd->sp_inact > 0) && ll.ll_time )
			if((( ll.ll_time / DAY ) + shpwd->sp_inact) < DAY_NOW ) {
				printf(incorrectmsg);
				write(fdl, (char * )&ll, sizeof(ll));
				close(fdl);
				exit(1);
			}

		write(fdl, (char * )&newll, sizeof(newll));
		close(fdl);
	} 

	/* Set mode to r/w user & w group, owner to user and group to tty */
	chmod(ttyn,S_IRUSR|S_IWUSR|S_IWGRP);
	if ((grpstr = getgrnam(ttygrp)) == NULL) 
		chown(ttyn, pwd->pw_uid, pwd->pw_gid);
	else
		chown(ttyn, pwd->pw_uid, grpstr->gr_gid);

	/* If the shell field starts with a '*', do a chroot to the home */
	/* directory and perform a new login.			 */

	if( *pwd->pw_shell == '*' )
	{
		if( chroot(pwd->pw_dir) < 0 )
		{
			printf("No Root Directory\n");
			goto loop;
		}

	/* Set the environment flag <!sublogin> so that the next login	*/
	/* knows that it is a sublogin.					*/

		envinit[0] = SUBLOGIN;
		envinit[1] = (char*)NULL;
		printf("Subsystem root: %s\n",pwd->pw_dir);
		execle("/usr/bin/login", "login", (char*)0, &envinit[0]);
		execle("/etc/login", "login", (char*)0, &envinit[0]);
		printf("No /usr/bin/login or /etc/login on root\n");
		goto loop;
	}
	if (Def_ulimit > 0L && ulimit(SET_FSIZ, Def_ulimit) < 0L)
		printf("Could not set ULIMIT to %ld\n", Def_ulimit);	

	if( setgid(pwd->pw_gid) == -1 )
	{
		printf("Bad group id.\n");
		exit(1);
	}
	/* Initialize the supplementary group access list. */
	if (initgroups(u_name, pwd->pw_gid) == -1) {
		printf("Could not initialize groups.\n");
		exit(1);
	}
	if( setuid(pwd->pw_uid) == -1 )
	{
		printf("Bad user id.\n");
		exit(1);
	}


	alarm(0);	/*give user time to come up with new password if needed*/

	now  = DAY_NOW ;
	/* We want to make sure that we only kick off /usr/bin/passwd if
	   passwords are required for the system, the user does not
   	   have a password, AND the user's NULL password can be changed
	   according to its password aging information */

	if((rflag) && (usererr == -1) || !(rflag))
	 {
		if( nopassword && (pwd->pw_uid != 0) && ( Passreq != NULL ) && 
 	    	!strcmp("YES", Passreq)  &&
	    	( (shpwd->sp_max == -1) || (shpwd->sp_lstchg > now) ||
	      	( (now >= shpwd->sp_lstchg + shpwd->sp_min) &&
 			(shpwd->sp_max >= shpwd->sp_min) 	    ) ) )
		{
			printf("You don't have a password.  Choose one.\n");
			printf("passwd %s\n", u_name);
			fflush(stdout);
			n = exec_pass(u_name);
			if( n > 0 )
				exit(9);
			if( n < 0 )
			{
				printf("Cannot execute /usr/bin/passwd\n");
				exit(9); 
			}
			shpwd->sp_lstchg = now;
		}
	}

	/* Check for login expiration */

	if ( shpwd->sp_expire > 0 ) {
		if ( shpwd->sp_expire < DAY_NOW ) {
			printf( incorrectmsg );
			exit(1);
		}
	}

	/* Is the age of the password to be checked? */

	if( ( shpwd->sp_lstchg == 0 )					||
	    ( shpwd->sp_lstchg > now )					||
	    ( (shpwd->sp_max >= 0 )				&&
	      ( now > (shpwd->sp_lstchg + shpwd->sp_max) )	&&
	      ( shpwd->sp_max >= shpwd->sp_min ) ) )
		{
		if ((Idleweeks == 0)
		    || ((Idleweeks > 0) &&
			(now > (shpwd->sp_lstchg + (7 * Idleweeks)))))
			{
			printf("Your password has been expired for too long;");
			printf("please contact the system administrator\n");
			exit(1);
			}
		else
			{
			printf("Your password has expired. Choose a new one\n");
			n = exec_pass(u_name);
			if( n > 0 )
				exit(9);
			if( n < 0 )
				{
				printf("Cannot execute /usr/bin/passwd\n");
				exit(9);
				}
			shpwd->sp_lstchg = now;
			}
		}


	/* Warn user that password will expire in n days */

	if (( shpwd->sp_warn > 0 ) && ( shpwd->sp_max > 0 ) &&
            ( now + shpwd->sp_warn) >= ( shpwd->sp_lstchg + shpwd->sp_max))

		printf("Your password will expire in %d days\n",
		((shpwd->sp_lstchg + shpwd->sp_max) - now));

	/* Set up the basic environment for the exec.  This includes	*/
	/* HOME, PATH, LOGNAME, SHELL, TERM, HZ, TZ, and MAIL.		*/

#ifdef i386
	if (!rflag)
	   getterm(ttyntail, term + strlen(term), "/etc/ttytype", "unknown");
#endif
	
	if (rflag) {
		ENVSTRNCAT(term, terminal);
	} else if (hflag)
		strcpy(term, terminal);
	ENVSTRNCAT(logname, pwd->pw_name);
	if ((Def_tz == NULL ) && ((Def_tz = getenv("TZ")) == NULL))
		strcat(timez, DEF_TZ);
	else
		ENVSTRNCAT(timez, Def_tz);
	if ( Def_hertz == NULL )
		sprintf(hertz + strlen(hertz), "%u", HZ);
	else			
		ENVSTRNCAT(hertz, Def_hertz);
	if (Def_path == NULL)
		strcat(path, DEF_PATH);
	else
		ENVSTRNCAT(path, Def_path);
	ENVSTRNCAT(home, pwd->pw_dir);
	
	/* Find the end of the basic environment */
	for( basicenv=0; envinit[basicenv] != NULL; basicenv++ );

	if (*pwd->pw_shell == '\0') {
		/*
		 * If possible, use the primary default shell,
		 * otherwise, use the secondary one.
		 */
		if (access(SHELL, X_OK) == 0)
			pwd->pw_shell = SHELL;
		else
			pwd->pw_shell = SHELL2;
	} else
		if (Altshell != NULL && strcmp(Altshell, "YES") == 0) {
			envinit[basicenv++] = shell;
			ENVSTRNCAT(shell, pwd->pw_shell);
		}


#ifndef	NO_MAIL
	envinit[basicenv++] = mail;
	strcat(mail,pwd->pw_name);
#endif

	/* Add in all the environment variables picked up from the */
	/* argument list to "login" or from the user response to the */
	/* "login" request. */

	for( j=0,k=0,l_index=0,ptr= &envblk[0]; *envp && j<(MAXARGS-1);
		j++,envp++ )
	{

	/* Scan each string provided.  If it doesn't have the format */
	/* xxx=yyy, then add the string "Ln=" to the beginning. */

		if( (endptr = strchr(*envp,'=')) == (char*)NULL )
		{
			envinit[basicenv+k] = ptr;
			sprintf(ptr,"L%d=%s",l_index,*envp);

	/* Advance "ptr" to the beginning of the next argument.	*/

			while( *ptr++ );
			k++;
			l_index++;
		}

	/* Is this an environmental variable we permit?	*/

		else if( !legalenvvar(*envp) )
			continue;


	/* Check to see whether this string replaces any previously- */
	/* defined string. */

		else
		{
			for( i=0,length=endptr+1-*envp; i<basicenv+k; i++ )
			{
				if( strncmp(*envp,envinit[i],length) == 0 )
				{
					envinit[i] = *envp;
					break;
				}
			}

	/* If it doesn't, place it at the end of environment array. */

			if( i == basicenv+k )
			{
				envinit[basicenv+k] = *envp;
				k++;
			}
		}
	}

	/* Switch to the new environment. */

	environ = envinit;
	alarm(0);

	signal(SIGQUIT, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	uname(&un);

#if i386
	printf("UNIX System V/386 Release %s Version %s\n%s\n\
Copyright (C) 1984, 1986, 1987, 1988, 1989, 1990 AT&T\n\
Copyright (C) 1990 UNIX System Laboratories, Inc.\n\
Copyright (C) 1987, 1988 Microsoft Corp.\nAll Rights Reserved\n",
		un.release, un.version, un.nodename);
#else
#if defined (MB1) || defined (MB2) || defined  (MB2AT)
	printf("UNIX System V/386 Release %s Version %s\n%s\n\
Copyright (C) 1984, 1986, 1987, 1988, 1989, 1990 AT&T\n\
Copyright (C) 1990 UNIX System Laboratories, Inc.\n\
Copyright (C) 1986, 1987, 1988, 1989, 1990 Intel Corp.\n\
Copyright (C) 1987, 1988 Microsoft Corp.\nAll Rights Reserved\n",
		un.release, un.version, un.nodename);
#else
#if sparc
	printf("UNIX System V/SPARC Release %s Version %s\n%s\n\
Copyright (C) 1984, 1986, 1987, 1988, 1989, 1990 AT&T\n\
Copyright (C) 1990 UNIX System Laboratories, Inc.\n\
Copyright (C) 1987, 1988 Microsoft Corp.\nAll Rights Reserved\n",
		un.release, un.version, un.nodename);
#else
	printf("UNIX System V Release %s AT&T %s\n%s\n\
Copyright (C) 1984, 1986, 1987, 1988, 1989, 1990 AT&T\n\
Copyright (C) 1990 UNIX System Laboratories, Inc.\n",
		un.release, un.machine, un.nodename);
#endif /*sparc*/
#endif /*MB1-MB2*/
#endif /*i386*/

#ifdef	SECURITY
	/*	
	 *	Advise the user the time and date that this login-id
	 *	was last used. 
	 */

	if (lastlogok) {	
		printf("Last login: %.*s ", 24-5, ctime(&ll.ll_time));
		if (*ll.ll_host != '\0')
			printf("from %.*s\n", sizeof(ll.ll_host), ll.ll_host);
		else 
			printf("on %.*s\n", sizeof(ll.ll_line), ll.ll_line);
	}

#endif	/* SECURITY */



	strcat(minusnam, basename(pwd->pw_shell));
	execl(pwd->pw_shell, minusnam, (char*)0);
	
	/* pwd->pw_shell was not an executable object file, maybe it
	 * is a shell proceedure or a command line with arguments.
	 * If so, turn off the SHELL= environment variable.
	 */
	for (i = 0; envinit[i] != NULL; ++i)
		{
		if ((envinit[i] == shell) &&
		    ((endptr = strchr(shell, '=')) != NULL))
			(*++endptr) = '\0';
		}
	if( access( pwd->pw_shell, R_OK|X_OK ) == 0 )
		execl(SHELL, "sh", pwd->pw_shell, (char*)0);
	printf("No shell\n");
	exit(1);
	/* NOTREACHED */
}

static
dialpass(ttyn)
char *ttyn;
{
	register FILE *fp;
	char defpass[30];
	char line[80];
	register char *p1, *p2;

	if( (fp=fopen(DIAL_FILE, "r")) == NULL )
		return(0);
	while( (p1 = fgets(line, sizeof(line), fp)) != NULL )
	{
		while( *p1 != '\n' && *p1 != ' ' && *p1 != '\t' )
			p1++;
		*p1 = '\0';
		if( strcmp(line, ttyn) == 0 )
			break;
	}
	fclose(fp);
	if( p1 == NULL || (fp = fopen(DPASS_FILE, "r")) == NULL )
		return(0);
	defpass[0] = '\0';
	p2 = 0;
	while( (p1 = fgets(line, sizeof(line)-1, fp)) != NULL )
	{
		while( *p1 && *p1 != ':' )
			p1++;
		*p1++ = '\0';
		p2 = p1;
		while( *p1 && *p1 != ':' )
			p1++;
		*p1 = '\0';
		if( strcmp(pwd->pw_shell, line) == 0 )
			break;

		if( strcmp(SHELL, line) == 0 )
			SCPYN(defpass, p2);
		p2 = 0;
	}
	fclose(fp);
	if( !p2 )
		p2 = defpass;
	if( *p2 != '\0' )
		return(gpass("Dialup Password:", p2));
	return(0);
}

static
gpass(prmt, pswd)
char *prmt, *pswd;
{
	register char *p1;
	time_t time() ;

	/* getpass() fails if it cannot open /dev/tty.
	 * If this happens, and the real UID is root,
	 * then use the current stdin and stderr.
	 * This allows login to work with network connections
	 * and other non-ttys.
	 */
	if( ((p1 = getpass(prmt)) == (char *)0) && (getuid() == ROOTUID) ) {
		p1 = fgetpass(stdin, stderr, prmt);
	}
	if( !p1 || strcmp(crypt(p1, pswd), pswd) )
	{
	/* A sleep was done with the following code:
	#if  MAXTRYS > 0  &&  MAXTIME > 0  &&  (MAXTIME - 2*MAXTRYS) > 0
		sleep( (MAXTIME - 2*MAXTRYS)/MAXTRYS );
	#endif
	It was changed to sleep(SLEEPTIME) because the above algorithm
	will cause login to sleep for a long time if MAXTRYS were made
	small in relation to MAXTIME.
	*/
		sleep (SLEEPTIME) ;
		return(1);
	}
	return(0);
}

#define	WHITESPACE	0
#define	ARGUMENT	1

static char **
getargs(inline)
char *inline;
{
	static char envbuf[MAXLINE];
	static char *args[MAXARGS];
	register char *ptr,**answer;
	register int c;
	int state;
	extern int quotec();

	for( ptr= envbuf; ptr < &envbuf[sizeof(envbuf)]; )
		*ptr++ = '\0';

	for( answer= args; answer < &args[MAXARGS]; )
		*answer++ = (char *)NULL;

	for( ptr= envbuf,answer= &args[0],state = WHITESPACE;
	     (c = getc(stdin)) != EOF; )
	{

		*(inline++) = c;
		switch( c ) {
	
		case '\n':
			if( ptr == &envbuf[0] ) return((char **)NULL);
			else return(&args[0]);
		case ' ':
		case '\t':
			if( state == ARGUMENT )
			{
				*ptr++ = '\0';
				state = WHITESPACE;
			}
			break;
		case '\\':
			c = quotec();
		default:
			if( state == WHITESPACE )
			{
				*answer++ = ptr;
				state = ARGUMENT;
			}
			*ptr++ = c;
		}

	/* If the buffer is full, force the next character to be read to */
	/* be a <newline>.						 */

		if( ptr == &envbuf[sizeof(envbuf)-1] )
		{
			ungetc('\n',stdin);
			putc('\n',stdout);
		}
	}

	/* If we left loop because an EOF was received, exit immediately. */

	exit(0);
	/* NOTREACHED */
}

static int
quotec()
{
	register int c, i, num;

	switch( c = getc(stdin) )
	{
	case 'n':
		c = '\n';
		break;
	case 'r':
		c = '\r';
		break;
	case 'v':
		c = '\013';
		break;
	case 'b':
		c = '\b';
		break;
	case 't':
		c = '\t';
		break;
	case 'f':
		c = '\f';
		break;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
		for( num=0,i=0; i<3; i++ )
		{
			num = num * 8 + (c - '0');
			if( (c = getc(stdin)) < '0' || c > '7' )
				break;
		}
		ungetc(c,stdin);
		c = num & 0377;
		break;
	default:
		break;
	}
	return(c);
}


static char *illegal[] = {
		"SHELL=",
		"HOME=",
		"LOGNAME=",
#ifndef	NO_MAIL
		"MAIL=",
#endif
		"CDPATH=",
		"IFS=",
		"PATH=",

#ifdef i386
		"TZ=",
		"HZ=",
#endif

		0
		};

/* Is it legal to insert this environmental variable? */
static int
legalenvvar(s)
char *s;
{
	register char **p;

	for( p=illegal; *p; p++ )
		if( !strncmp(s, *p, strlen(*p)) )
			return(0);
	return(1);
}

#ifdef	SECURITY


static int
doremotelogin(host)
	char *host;
{
	getstr(rusername, sizeof (rusername), "remuser");
	getstr(lusername, sizeof (lusername), "locuser");
	getstr(terminal, sizeof(terminal), "Terminal type");
	if (getuid()) {
		pwd = &nouser;
		return(-1);
	}
	pwd = getpwnam(lusername);
	if (pwd == NULL) {
		pwd = &nouser;
		return(-1);
	}
	valid_luser = 1;	/* have entry in passwd file for this user */
	return(ruserok(host, (pwd->pw_uid == 0), rusername, lusername));
}

getstr(buf, cnt, err)
	char *buf;
	int cnt;
	char *err;
{
	char c;

	do {
		if (read(0, &c, 1) != 1)
			exit(1);
		*buf++ = c;
	} while (cnt > 1 && c != 0);
	*buf = 0;
#ifdef DEBUG
	if (i == cnt) {
		printf("%s too long\r\n", err);
	}
#endif
}

char	*speeds[] =
    { "0", "50", "75", "110", "134", "150", "200", "300",
      "600", "1200", "1800", "2400", "4800", "9600", "19200", "38400" };
#define	NSPEEDS	(sizeof (speeds) / sizeof (speeds[0]))

static int
doremoteterm(term)
	char *term;
{

	struct termios tp;
	register char *cp = strchr(term, '/'), **cpp;
	char *speed;

	ioctl(0, TCGETS, &tp);

	if (cp) {
		*cp++ = '\0';
		speed = cp;
		cp = strchr(speed, '/');
		if (cp)
			*cp++ = '\0';
		for (cpp = speeds; cpp < &speeds[NSPEEDS]; cpp++)
			if (strcmp(*cpp, speed) == 0) {
				tp.c_cflag = ( (tp.c_cflag & ~CBAUD) |
					       ((cpp-speeds) & CBAUD) );
				break;
			}
	}
	tp.c_lflag |= ECHO|ICANON;
	tp.c_oflag |= XTABS;
	tp.c_iflag |= IGNPAR|ICRNL;

	ioctl(0, TCSETS, &tp);

}

/*
 * badlogin() - log to the log file after
 *     NFAILURES unsuccessful attempts
 */
badlogin ()
{
	int retval, count1, fildes;
	void 	donothing();

	/* Tries to open the log file. If succeed, lock it and write
	   in the failed attempts */
	if ( (fildes = open (LOGINLOG, O_APPEND|O_WRONLY)) == -1 )
		return (0) ;
	else	{
		(void) sigset ( SIGALRM, donothing ) ;
		(void) alarm ( L_WAITTIME ) ;
		retval = lockf ( fildes, F_LOCK, 0L ) ;
		(void) alarm ( 0 ) ;
		(void) sigset ( SIGALRM, SIG_DFL ) ;
		if ( retval == 0 )
			{
			for ( count1 = 0 ; count1 < NFAILURES ; count1++ )
			   write (fildes, log_entry[count1],
				  (unsigned) strlen (log_entry[count1])) ;
			(void) lockf (fildes, F_ULOCK, 0L) ;
			(void) close (fildes) ;
			}
		return (0) ;
		}
}

void
donothing()
{}

#endif	/* SECURITY */


char *
getpass(prompt)
char *prompt;
{
	char *p;
	FILE *fi;

	if( (fi = fopen("/dev/tty", "r")) == NULL ) {
			return((char*)NULL);
	}
	setbuf(fi, (char*)NULL);
	p = fgetpass(fi, stderr, prompt);
	if( fi != stdin )
		(void)fclose(fi);
	return(p);
}


char *
fgetpass(fi, fo, prompt)
FILE *fi, *fo;
char *prompt;
{
	struct termio ttyb;
	unsigned short flags;
	register char *p;
	register int c;
	static char pbuf[PBUFSIZE + 1];
	void (*sig)(), catch();

	sig = signal(SIGINT, catch);
	intrupt = 0;
	(void)ioctl(fileno(fi), TCGETA, &ttyb);
	flags = ttyb.c_lflag;
	ttyb.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
	(void)ioctl(fileno(fi), TCSETAF, &ttyb);
	(void)fputs(prompt, fo);
	for( p=pbuf; !intrupt && (c = getc(fi)) != '\n' && c != EOF; )
	{
		if( p < &pbuf[PBUFSIZE] )
			*p++ = c;
	}
	*p = '\0';
	(void)putc('\n', fo);
	ttyb.c_lflag = flags;
	(void)ioctl(fileno(fi), TCSETAW, &ttyb);
	(void)signal(SIGINT, sig);
	if( intrupt )
		(void)kill(getpid(), SIGINT);

	
	return(pbuf);
}


static void
catch()
{
	++intrupt;
}


/*
 * uppercaseterm - if all input char are upper case
 *		   set the corresponding termio
 */
void
uppercaseterm(strp)
char	*strp;
{
	int 	upper = 0;
	int 	lower = 0;
	char	*sp;
	struct	termio	termio;

	for (sp = strp; *sp; sp++) {
		if (islower(*sp)) 
			lower++;
		else if (isupper(*sp))
			upper++;
	}

	if (upper > 0 && lower == 0) {
		ioctl(0,TCGETA,&termio);
		termio.c_iflag |= IUCLC;
		termio.c_oflag |= OLCUC;
		termio.c_lflag |= XCASE;
		ioctl(0,TCSETAW,&termio);
		for (sp = strp; *sp; sp++) 
			if (*sp >= 'A' && *sp <= 'Z' ) *sp += ('a' - 'A');
	}
}

/*
 * finttyname - call ttyname(), but do not return syscon or contty
 *
 */

char *
findttyname(fd)
int	fd;
{
	extern char *ttyname();
	char *ttyn;

	ttyn = ttyname(fd);

/* do not use syscon, systty, or contty if console is present, 
		assuming they are links */

	if (((strcmp(ttyn, "/dev/syscon") == 0) ||
		(strcmp(ttyn, "/dev/systty") == 0) ) &&
		(access("/dev/console", F_OK) == 0))
		ttyn = "/dev/console";

	return (ttyn);
}

/***	defaults -- read defaults
 *
 */

defaults()
{
	extern char *defread();
	extern int defopen(), defcntl();
	register int  flags;
	register char *ptr;

	if (defopen(Pndefault) == 0) {
		/* ignore case */
		flags = defcntl(DC_GETFLAGS, 0);
		TURNOFF(flags, DC_CASE);
		defcntl(DC_SETFLAGS, flags);

		if ((Console = defread("CONSOLE=")) != NULL)
			Console = strdup(Console);
		if ((Altshell = defread("ALTSHELL=")) != NULL)
			Altshell = strdup(Altshell);
		if ((Passreq = defread("PASSREQ=")) != NULL)
			Passreq = strdup(Passreq);
		if ((Def_tz = defread("TIMEZONE=")) != NULL)
			Def_tz = strdup(Def_tz);
		if ((Def_hertz = defread("HZ=")) != NULL)
			Def_hertz = strdup(Def_hertz);
		if ((Def_path   = defread("PATH=")) != NULL)
			Def_path = strdup(Def_path);
		if ((Def_supath = defread("SUPATH=")) != NULL)
			Def_supath = strdup(Def_supath);
		if ((ptr = defread("ULIMIT=")) != NULL)
		    Def_ulimit = atol(ptr);
		if ((ptr = defread("TIMEOUT=")) != NULL)
		    Def_timeout = (unsigned) atoi(ptr);
		if ((ptr = defread("UMASK=")) != NULL)
			if (sscanf(ptr, "%lo", &Umask) != 1)
				Umask = DEFUMASK;
		if ((ptr = defread("IDLEWEEKS=")) != NULL)
			Idleweeks = atoi(ptr);
		(void) defopen(NULL);
	}
	return;
}

/*
 * exec_pass() - exec passwd 
 *
 */
static	int
exec_pass(usernam)

	char *usernam;
{
	int	status, w;
	pid_t	pid;

	if((pid = fork()) == 0) {

		execl("/usr/bin/passwd", "/usr/bin/passwd", usernam, NULL);
		exit(127);
	}

	while((w = (int)wait(&status)) != pid && w != -1)
		;
	return((w == -1)? w: status);
}

/*
 * get_options(argc, argv) - parse the cmd line.
 *			   - return 0 if successful, -1 if failed.
 */
static	int
get_options(argc, argv)
int	argc;
char	*argv[];
{
	int	c;
	int	errflg = 0;
	extern	char	*ttyn;

	while ((c = getopt(argc, argv, "d:hr")) != -1) {
		switch (c) {
		case 'd':
			ttyn = optarg;
			break;
		case 'h':
			if (hflag || rflag) {
				fprintf(stderr,"Only one of -r and -h allowed\n");
				exit(1);
			}
			hflag++;
			break;
		case 'r':
			if (hflag || rflag) {
				fprintf(stderr,"Only one of -r and -h allowed\n");
				exit(1);
			}
			rflag++;
			break;
		default:
			errflg++;
			break;
		} /* end switch */
	} /* end while */
	if (errflg)
		return(-1);
	return(0);
}

static	void
usage()
{
	fprintf(stderr, 
		"Usage:\tlogin [-h|-r] [ -d device ] [ name [ env-var ... ]]\n");
}
