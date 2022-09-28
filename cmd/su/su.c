/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)su:su.c	1.9.9.2"
/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */
/*
 *	su [-] [name [arg ...]] change userid, `-' changes environment.
 *	If SULOG is defined, all attempts to su to another user are
 *	logged there.
 *	If CONSOLE is defined, all successful attempts to su to uid 0
 *	are also logged there.
 *
 *	If su cannot create, open, or write entries into SULOG,
 *	(or on the CONSOLE, if defined), the entry will not
 *	be logged -- thus losing a record of the su's attempted
 *	during this period.
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <crypt.h>
#include <pwd.h>
#include <shadow.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#define PATH	"/usr/bin"	/*path for users other than root*/
#define SUPATH	"/sbin:/usr/sbin:/usr/bin:/etc"	/*path for root*/
#define SUPRMT	"PS1=# "		/*primary prompt for root*/
#define ELIM 128
#define ROOT 0

#define	DEFFILE	"/etc/default/su"		/* default file M000 */
char	*Sulog, *Console;
char	*Path, *Supath;			/* M004 */
extern char *defread();
extern int defopen();

void to();
struct	passwd *pwd, *getpwnam();
char	*shell = "/sbin/sh";	/*default shell*/
char	su[16] = "su";		/*arg0 for exec of shprog*/
char	homedir[64] = "HOME=";
char	logname[20] = "LOGNAME=";
char	*suprmt = SUPRMT;
char	termtyp[20] = "TERM=";			/* M002 */
char	*term;
char	shelltyp[20] = "SHELL=";		/* M002 */
char	*hz, *tz;
char	tznam[15] = "TZ=";
char	hzname[10] = "HZ=";
char	path[1024] = "PATH=";			/* M004 */
char	supath[1024] = "PATH=";			/* M004 */
char	*envinit[ELIM];
extern	char **environ;
char *ttyn;

main(argc, argv)
int	argc;
char	**argv;
{
	struct spwd *sp;
	char *nptr, *password;
	char	*pshell = shell;
	int eflag = 0;
	int envidx = 0;
	uid_t uid;
	gid_t gid;
	char *dir, *shprog, *name;

	if (argc > 1 && *argv[1] == '-') {
		eflag++;	/*set eflag if `-' is specified*/
		argv++;
		argc--;
	}

	/* 
	 * determine specified userid, get their password file entry,
	 * and set variables to values in password file entry fields
	*/

	nptr = (argc > 1)? argv[1]: "root";
	if(((pwd = getpwnam(nptr)) == NULL) || ((sp = getspnam(nptr)) == NULL)) {
		fprintf(stderr,"su: Unknown id: %s\n",nptr);
		exit(1);
	}
	if (defopen(DEFFILE) == 0) {
		if (Sulog = defread("SULOG="))
			Sulog = strdup(Sulog);
		if (Console = defread("CONSOLE="))
			Console = strdup(Console);
		if (Path = defread("PATH="))
			Path = strdup(Path);
		if (Supath = defread("SUPATH="))
			Supath = strdup(Supath);
		defopen(NULL);
	}
	strcat(path, (Path) ? Path : PATH);
	strcat(supath, (Supath) ? Supath : SUPATH);
	uid = pwd->pw_uid;
	gid = pwd->pw_gid;
	dir = strcpy((char *)malloc(strlen(pwd->pw_dir)+1),pwd->pw_dir);
	shprog = strcpy((char *)malloc(strlen(pwd->pw_shell)+1),pwd->pw_shell);
	name = strcpy((char *)malloc(strlen(pwd->pw_name)+1),pwd->pw_name);
	if((ttyn=ttyname(0))==NULL)
		if((ttyn=ttyname(1))==NULL)
			if((ttyn=ttyname(2))==NULL)
				ttyn="/dev/???";

	/*if Sulog defined, create SULOG, if it does not exist, with
	  mode read/write user. Change owner and group to root
	*/
	if (Sulog != NULL)
	{
		close( open(Sulog, O_WRONLY | O_APPEND | O_CREAT, (S_IRUSR|S_IWUSR)) );
		chown(Sulog, (uid_t)ROOT, (gid_t)ROOT);
	}

	/*Prompt for password if invoking user is not root or
	  if specified(new) user requires a password
	*/
	if (sp->sp_pwdp[0] == '\0'  || getuid() == (uid_t)0)
		goto ok;
	password = getpass("Password:");

	if((strcmp(sp->sp_pwdp, crypt(password, sp->sp_pwdp)) != 0)) {
		if (Sulog != NULL)
			log(Sulog, nptr, 0);	/*log entry*/
		fprintf(stderr,"su: Sorry\n");
		exit(2);
	}
ok:
	endpwent();	/*close password file*/
	(void) endspent();	/*close shadow password file*/
	if (Sulog != NULL)
		log(Sulog, nptr, 1);	/*log entry*/

	/*set user and group ids to specified user*/

	/* Initialize the supplementary group access list */
	if (setgid(gid) != 0) {
		printf("su: Invalid ID\n");
		exit(2);
	}
	if  (initgroups(nptr, gid))
		exit(2);

	if (setuid(uid) != 0) {
		printf("su: Invalid ID\n");
		exit(2);
	}

	/*set environment variables for new user;
	  arg0 for exec of shprog must now contain `-'
	  so that environment of new user is given
	*/
	if (eflag) {
		strcat(homedir, dir);
		strcat(logname, name);			/* M003 */
		if (hz = getenv("HZ"))
			strcat(hzname, hz);
		if (tz = getenv("TZ"))
			strcat(tznam, tz);
		chdir(dir);
		envinit[envidx = 0] = homedir;
		envinit[++envidx] = ((uid == (uid_t)0) ? supath : path);
		envinit[++envidx] = logname;
		envinit[++envidx] = hzname;
		envinit[++envidx] = tznam;
		if ((term = getenv("TERM")) != NULL) {
			strcat(termtyp, term);
			envinit[++envidx] = termtyp;
		}
		envinit[++envidx] = NULL;
		environ = envinit;
		strcpy(su, "-su");
	}

	/*if new user is root:
		if CONSOLE defined, log entry there;
		if eflag not set, change environment to that of root.
	*/
	if (uid == (uid_t)0)
	{
		if (Console != NULL)
			if(strcmp(ttyn, Console) != 0) {
				signal(SIGALRM, to);
				alarm(30);
				log(Console, nptr, 1);
				alarm(0);
			}
		if (!eflag) envalt();
	}

	/*if new user's shell field is not NULL or equal to /sbin/sh,
	  set:
		pshell = their shell
		su = [-]last component of shell's pathname
	*/
	if (shprog[0] != '\0' && (strcmp(shell,shprog) != 0) ) {
		pshell = shprog;
		strcpy(su, eflag ? "-" : "" );
		strcat(su, strrchr(pshell,'/') + 1);
	}

	/*if additional arguments, exec shell program with array
	    of pointers to arguments:
		-> if shell = /sbin/sh, then su = [-]su
		-> if shell != /sbin/sh, then su = [-]last component of
						     shell's pathname

	  if no additional arguments, exec shell with arg0 of su
	    where:
		-> if shell = /sbin/sh, then su = [-]su
		-> if shell != /sbin/sh, then su = [-]last component of
						     shell's pathname
	*/
	if (argc > 2) {
		argv[1] = su;
		execv(pshell, &argv[1]);
	} else {
		execl(pshell, su, 0);
	}

	fprintf(stderr,"su: No shell\n");
	exit(3);
}

/*Environment altering routine -
	This routine is called when a user is su'ing to root
	without specifying the - flag.
	The user's PATH and PS1 variables are reset
	to the correct value for root.
	All of the user's other environment variables retain
	their current values after the su (if they are
	exported).
*/
envalt()
{

	/*If user has PATH variable in their environment, change its value
			to /bin:/etc:/usr/bin ;
	  if user does not have PATH variable, add it to the user's
			environment;
	  if either of the above fail, an error message is printed.
	*/
	if ( ( putenv(supath) ) != 0 ) {
		printf("su: unable to obtain memory to expand environment");
		exit(4);
	}

	/*If user has PROMPT variable in their environment, change its value
			to # ;
	  if user does not have PROMPT variable, add it to the user's
			environment;
	  if either of the above fail, an error message is printed.
	*/
	if ( ( putenv(suprmt) ) != 0 ) {
		printf("su: unable to obtain memory to expand environment");
		exit(4);
	}

	return;

}

/*Logging routine -
	where = SULOG or CONSOLE
	towho = specified user ( user being su'ed to )
	how = 0 if su attempt failed; 1 if su attempt succeeded
*/
log(where, towho, how)
char *where, *towho;
int how;
{
	FILE *logf;
	long now;
	struct tm *tmp;

	now = time(0);
	tmp = localtime(&now);

	/*open SULOG or CONSOLE -
		if open fails, return
	*/
	if ((logf=fopen(where,"a")) == NULL) return;

	/*write entry into SULOG or onto CONSOLE -
		 if write fails, return
	*/
	fprintf(logf,"SU %.2d/%.2d %.2d:%.2d %c %s %s-%s\n",
		tmp->tm_mon+1,tmp->tm_mday,tmp->tm_hour,tmp->tm_min,
		how?'+':'-',(ttyn + sizeof("/dev/") -1),
		cuserid((char *)0),towho); 

	fclose(logf);	/*close SULOG or CONSOLE*/

	return;
}

void to(){}
