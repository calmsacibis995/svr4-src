/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:xx/FACE3.2/admbin/chkuser.c	1.1"
/*                       chkuser.c
 *
 * chkuser -u 
 *
 *	Is user a privilege user? 
 *
 *	passes by returning "0" to stdout if:
 *		Real user or effective user is root.
 *		Real user or effective user is privileged user (in /etc/.useradm)
 *
 *	fails by returning a "1" to stdout  
 *
 * chkuser -c "command :arg1: :arg2: ...:argn:"
 *
 *  	Is the command a valid command?
 *
 *	passes by running the command if:
 *		Real user or effective user is root.
 *		Real user or effective user is privileged user (in /etc/.useradm)
 *		The command resides in /usr/vmsys/admsets/base-adm
 *
 * chkuser -u -c "command :arg1: :arg2: ...:argn:"
 *
 *      runs command if user tests pass but does not return a value
 *	to stdout.
 *	fails by returning a "1" to stdout  
 *
 */
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <pwd.h>
#define CMDLENGTH	BUFSIZ /* max command line length */
#define SETSLNGTH	BUFSIZ /* max command sets length */
#define USERLNGTH	8      /* max user name length    */

#ifdef DEBUG
#define ROOT		(getuid())		/* root uid */
#define USERADM		"etc/useradm"
#define ADMSETS		"usr/admsets/"
#else
#define ROOT		0			/* root uid */
#define USERADM		"/etc/.useradm"
#define ADMSETS		"/usr/sadm/sysadm/admsets/"
#endif /* DEBUG */

char str[L_cuserid + 10];
struct admset {
	char *name;
	struct admset *next;
} *admsets;

char *malloc();

main(argc, argv)
int argc;
char **argv;
{
	register optchar;
	extern char *optarg; 		
	char usercmd[CMDLENGTH]; 	/* command line from the user */
	int uflg=0, cflg=0, opterr=0;
	int putenv();
	char *ptr;
	int retcode;

	admsets = (struct admset *) malloc(sizeof(struct admset));
	admsets->next = NULL;
	admsets->name = NULL;

	while ((optchar = getopt(argc, argv, "uc:" )) != EOF)
		switch(optchar)
		{
		case 'u':
			/* check for multiple uses of -u. Only one is permissible */
			if(uflg) {  
			  opterr++; 
			  break;
			}
			uflg++;
			continue;
		case 'c':
			/* check for multiple uses of -c. Only one is permissible */
			if(cflg) {  
			  opterr++; 
			  break;
			}
			cflg++;
			/* change : to " */
			while((ptr=strchr(optarg,':')) != NULL ) *ptr = '"';
			strcpy(usercmd,optarg);
			continue;
		case '?':
			opterr++;
			break;
		}

	if (!(uflg || cflg )) opterr++; /* check that chkuser has an option */

	if(opterr) 
	{
		fprintf(stderr, "Usage: chkuser [-u] [-c \"command :arg1: :arg2: ...:argn:\"]\n");
		exit(1);
	}

	retcode = chkuser();
#ifdef DEBUG
	{
	struct admset *p;
		p = admsets->next;
		while (p != NULL) {
			printf("DBG addr %d name %s\n", p, p->name);
			p = p->next;
		}
	}
#endif /* DEBUG */

	if(uflg) 
	{
		/* check the validity of the user 

		 * case 1: chkuser -u -c "command"
		 * return a "1" to stdout if chkuser fails
		 * return nothing to stdout if chkuser passes */

		if(cflg) {
		    if(retcode != 0) fprintf(stdout,"1\n");
		    }

	        /* case 2: chkuser -u 
		 * return a "1" to stdout if chkuser fails
		 * return a "0" to stdout if chkuser passes */

		else fprintf(stdout, "%d\n", retcode);
	}

	if(cflg) 
	{
		/* check the validity of the user */
		if(retcode != 0) exit(1);
		else 		  
		{
			/* check the validity of the command */
			if(chkcommand(usercmd) != 0) exit(1);
			else 
			{
				sprintf(str,"REAL_LOGIN=%s",cuserid(NULL));
				if (putenv(str) != 0) exit (1);
				if (setuid(ROOT) != 0) exit(1);
				if (putenv("PATH=/bin:/usr/bin:/etc") != 0) exit(1);
				exit(system(usercmd));
			}
		}
	}
	/*
	 * exit with the 0 or 1 from chkuser()
	 */
	exit(retcode);
}


/* This function searches through /etc/.useradm for valid users */

chkuser()
{
	char *user;
	struct passwd *pw, *getpwuid();
	FILE *fp;
	char s[256];
	char *p;
	struct admset *obj;


	/*  	
         *	Get login name from uid.  getpwuid was used because
	 *	getlogin() fails when running layers.
	 */

	if ((pw = getpwuid(getuid())) == NULL) {
		fprintf(stderr,"Can't read user name on system\n");
		return(1);
	}

	user = strcpy(malloc(strlen(pw->pw_name)+1),pw->pw_name);

	/*  	
         *	If /etc/.useradm does not exist, exit.
	 */

	if((fp = fopen(USERADM, "r")) == NULL)
	{       
		fprintf(stderr, "Can't open %s file for reading.\n", USERADM);
		endpwent();
		return(1);
	}


	/*  	Search through /etc/.useradm. 
         *	If the user is valid, return a "0." 
	 */

	while(fgets(s, sizeof(s), fp) != NULL)
	{
		p = strtok(s, " \n");
		if (p == NULL || *p == '#')		/* blank or comment */
			continue;
		if (strcmp(p, user) == 0)
		{
			p = strtok(NULL, " \n");
			while (p != NULL) {
				obj = (struct admset *) malloc(sizeof(struct admset));
				obj->next = admsets->next;
				admsets->next = obj;
				if (*p == '/')		/* abs. path */
					obj->name = strcpy(malloc(strlen(p)+1), p);
				else {			/* relative to ADMSETS */
					obj->name = malloc(strlen(ADMSETS)+strlen(p)+1);
					strcpy(obj->name, ADMSETS);
					strcat(obj->name, p);
				}
				p = strtok(NULL, " \n");
			}
			fclose(fp);
			endpwent();
			return(0);
		}
	}
	endpwent();
	fclose(fp);
	return(1);
}


/* This function searches through the USERADM file for the data contained in it */

chkcommand(usercmd)
char *usercmd;
{
	char fromadmin[CMDLENGTH];
	FILE *fp;
	struct admset *aset;

	/*	stop if invalid characters are found in the command line */

	if((strpbrk(usercmd, ";|^`'?{}()$#\\")) != NULL ) {
		fprintf(stderr, "Illegal character in command argument.\n");
		return(1);
	}


	aset = admsets->next;
	while (aset != NULL) {

		if((fp = fopen(aset->name, "r")) == NULL) {       
			fprintf(stderr, "Can't open %s file for reading.\n",
					aset->name);
			return(1);
		}

		/*   	Search through ADMSETS ( /usr/vmsys/admsets/base-adm).
		 *	If the user's command is valid return a 0.
		 *
		 *	There are 2 possible cases to check for commands:
		 *
		 *	1. The command has no arguments:
		 *
		 *	   In this case an exact string compare is done between
		 *	   the command sent into chkuser by the user (fromuser)
		 *	   and the commands residing in /usr/vmsys/admsets/base-adm.
		 *
		 *	   NOTE: for release 1 base-adm is the only command set we are
		 *	   allowing commands to reside in. 
		 *
		 *
		 *	2. The command has arguments:
		 *
		 *	   In this case a string compare is done by appending a space
		 *	   to the end of the each command in /usr/vmsys/admsets/base-adm
		 *	   and comparing this to the first "strlen" characters in the
		 *	   command sent into chkuser by the user.
		 *
		 *
		 */

		while(fscanf(fp, "%s", fromadmin) != EOF) {
			if ( ( strchr(usercmd,' ') ) == NULL ) {
				if (strcmp(usercmd,fromadmin) == 0) {
					fclose(fp);
					return(0);
				}
			}
			else {
				strcat(fromadmin," ");
				if (strncmp(usercmd,fromadmin,strlen(fromadmin)) == 0) {
					fclose(fp);
					return(0);
				}
			}
		}
		fclose(fp);
		aset = aset->next;
	}

	/*  If the program got this far without returning, the command line 
	    was invalid.   At this point return a "1" for invalid command line. */

	return(1);
}
