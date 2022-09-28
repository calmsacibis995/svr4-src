/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)sulogin:sulogin.c	1.9.4.1"
/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */


/*
 *	@(#) sulogin.c 1.2 88/05/05 sulogin:sulogin.c
 */
/*
 * sulogin - special login program exec'd from init to let user
 * come up single user, or go multi straight away.
 *
 *	Explain the scoop to the user, and prompt for
 *	root password or ^D. Good root password gets you
 *	single user, ^D exits sulogin, and init will
 *	go multi-user.
 *
 *	If /etc/passwd is missing, or there's no entry for root,
 *	go single user, no questions asked.
 *
 * Anthony Short, 11/29/82
 */

/*
 *	MODIFICATION HISTORY
 *
 *	M000	01 May 83	andyp	3.0 upgrade
 *	- index ==> strchr.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <termio.h>
#include <pwd.h>
#include <shadow.h>
#include <stdio.h>
#include <signal.h>
#include <utmpx.h>
#include <unistd.h>
#ifdef	M_V7
#define	strchr	index
#define	strrchr	rindex
#endif

char	minus[]	= "-";
char	shell[]	= "/sbin/su";

char	*crypt();
static	char	*getpass();
struct utmpx *getutxent(), *pututxline();
char	*strchr(), *strrchr();
static struct utmpx *u;
char	*ttyn = NULL;
#define SCPYN(a, b)	strncpy(a, b, sizeof(a))
extern char *findttyname();

main()
{
	struct stat info;			/* buffer for stat() call */
	register struct spwd *shpw;
	register struct passwd *pw;
	register char *pass;			/* password from user	  */
	register char *namep;

	if(stat("/etc/passwd",&info) != 0) {	/* if no pass file, single */
		printf("\n**** PASSWORD FILE MISSING! ****\n\n");
		single();			/* doesn't return	  */
	}

	setpwent();
	setspent();
	pw = getpwnam("root");		        /* find entry in passwd   */
	shpw = getspnam("root");
	endpwent();
	endspent();

	if(pw == (struct passwd *)0) {		/* if no root entry, single */
		printf("\n**** NO ENTRY FOR root IN PASSWORD FILE! ****\n\n");
		single();			/* doesn't return	  */
	}
	if (shpw == (struct spwd *) 0) {
		printf("\n**** NO ENTRY FOR root IN SHADOW FILE! ****\n\n");
		single();
		}
	while(1) {
		printf("\nType Ctrl-d to proceed with normal startup,\n");
		printf("(or give root password for system maintenance): ");

		if((pass = getpass()) == (char *)0)
			exit(0);	/* ^D, so straight to multi-user */

		if ( *shpw->sp_pwdp == '\0' ) 
			namep = pass;
		else
			namep = crypt(pass,shpw->sp_pwdp);

		if(strcmp(namep, shpw->sp_pwdp))
			printf("Login incorrect\n");
		else
			single();
	}
}



/*
 * single() - exec shell for single user mode
 */
single()
{
	/*
	 * update the utmpx file.
	 */
	ttyn = findttyname(0);
	if( ttyn==NULL )
        	ttyn = "/dev/???";
	while( (u = getutxent()) != NULL ) {
		if(!strcmp(u->ut_line,(ttyn+sizeof("/dev/")-1))) {
			time(&u->ut_tv.tv_sec);
			u->ut_type = INIT_PROCESS;
			if(strcmp(u->ut_user,"root")) {
				u->ut_pid = getpid();
                        	SCPYN(u->ut_user,"root");
			}
			break;
		}
	}
	if(u != NULL)
		pututxline(u);
	endutxent();		/* Close utmp file */
	printf("Entering System Maintenance Mode\n\n");
	execl(shell, shell, minus, (char *)0);
	exit(0);
}



/*
 * getpass() - hacked from the stdio library
 * version so we can distinguish newline and EOF.
 * Also don't need this routine to give a prompt.
 *
 * RETURN:	(char *)address of password string
 *			(could be null string)
 *
 *	   or	(char *)0 if user typed EOF
 */
static char *
getpass()
{
	struct termio ttyb;
	int flags;
	register char *p;
	register c;
	FILE *fi;
	static char pbuf[9];
	void (*signal())();
	void (*sig)();
	char *rval;		/* function return value */

	if ((fi = fopen("/dev/tty", "r")) == NULL)
		fi = stdin;
	else
		setbuf(fi, (char *)NULL);
	sig = signal(SIGINT, SIG_IGN);
	(void) ioctl(fileno(fi), TCGETA, &ttyb);
	flags = ttyb.c_lflag;
	ttyb.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
	(void) ioctl(fileno(fi), TCSETAF, &ttyb);
	p = pbuf;
	rval = pbuf;
	while((c = getc(fi)) != '\n') {
		if(c == EOF) {
			if(p == pbuf)		/* ^D, No password */
				rval = (char *)0;
			break;
		}
		if(p < &pbuf[8])
			*p++ = c;
	}
	*p = '\0';			/* terminate password string */
	fprintf(stderr, "\n");		/* echo a newline */
	ttyb.c_lflag = flags;
	(void) ioctl(fileno(fi), TCSETAW, &ttyb);
	signal(SIGINT, sig);
	if (fi != stdin)
		fclose(fi);
	return(rval);
}

char *
findttyname(fd)
int	fd;
{
 	extern char *ttyname();
	char *ttyn;

        ttyn = ttyname(fd);

/* do not use syscon or contty if console is present, assuming they are links */
	if (((strcmp(ttyn, "/dev/syscon") == 0) ||
             (strcmp(ttyn, "/dev/contty") == 0)) &&
            (access("/dev/console", F_OK)))
		ttyn = "/dev/console";

	return (ttyn);
}

