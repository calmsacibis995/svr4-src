/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)pwconv:pwconv.c	1.11.3.1"

/*  pwconv.c  */
/*  Conversion aid to copy appropriate fields from the	*/
/*  password file to the shadow file.			*/

#include <pwd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>	
#include <shadow.h>
#include <grp.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#define PRIVILEGED	0 			/* priviledged id */	

/* exit  code */
#define SUCCESS	0	/* succeeded */
#define NOPERM	1	/* No permission */
#define BADSYN	2	/* Incorrect syntax */
#define FMERR	3	/* File manipulation error */
#define FATAL	4	/* Old file can not be recover */
#define FBUSY	5	/* Lock file busy */
#define BADSHW	6	/* Bad entry in shadow file  */

#define	DELPTMP()	(void) unlink(PASSTEMP)
#define	DELSHWTMP()	(void) unlink(SHADTEMP)

char pwdflr[]	= "x";				/* password filler */
char *prognamp;
void f_err(), f_miss(), f_bdshw();

main(argc,argv)
int argc;
char **argv;
{
	extern	int	errno;
	void  no_recover(), no_convert();
	struct  passwd  *pwdp;
	struct	spwd	*sp, sp_pwd;		/* default entry */
	struct stat buf;
	FILE	*tp_fp, *tsp_fp;
	register time_t	when, minweeks, maxweeks;
	int file_exist = 1;
	int end_of_file = 0;
	mode_t mode;
	int pwerr = 0;
	ushort i;
	gid_t pwd_gid, sp_gid;
	uid_t pwd_uid, sp_uid;
 

	prognamp = argv[0];
        /* only PRIVILEGED can execute this command */
	if (getuid() != PRIVILEGED) {
		fprintf(stderr, "%s: Permission denied.\n", prognamp);
		exit(NOPERM);
	}

	/* No argument can be passed to the command*/
	if (argc > 1) {
		fprintf(stderr, "%s: Invalid command syntax.\n", prognamp);
		fprintf(stderr, "Usage: pwconv\n");
		exit(BADSYN);
	}

	
	/* lock file so that only one process can read or write at a time */
	if (lckpwdf() < 0) { 
		(void)fprintf(stderr, "%s: Password file(s) busy.  Try again later.\n", prognamp);
		exit(FBUSY);
	}
 

      	/* All signals will be ignored during the execution of pwconv */
	for (i=1; i < NSIG; i++) 
		sigset(i, SIG_IGN);
 
	/* reset errno to avoid side effects of a failed */
	/* sigset (e.g., SIGKILL) */
	errno = 0;

	/* check the file status of the password file */
	/* get the gid of the password file */
	if (stat(PASSWD, &buf) < 0) {
		(void) f_err();
		exit(FMERR);
	} 
	pwd_gid = buf.st_gid;
	pwd_uid = buf.st_uid;
 
	/* mode for the password file should be read-only or less */
	umask(S_IAMB & ~(buf.st_mode & (S_IRUSR|S_IRGRP|S_IROTH)));

	/* open temporary password file */
	if ((tp_fp = fopen(PASSTEMP,"w")) == NULL) {
		(void) f_err();
		exit(FMERR);
	}

        if (chown(PASSTEMP, pwd_uid, pwd_gid) < 0) {
		DELPTMP();
		(void) f_err();
		exit(FMERR);
	}
	/* default mode mask of the shadow file */
	mode = S_IAMB & ~(S_IRUSR);

	/* check the existence of  shadow file */
	/* if the shadow file exists, get mode mask and group id of the file */
	/* if file does not exist, the default group name will be the group  */
	/* name of the password file.  */

	if( access(SHADOW,0) == 0) {
		if (stat(SHADOW, &buf) == 0) {
			mode  = S_IAMB & ~(buf.st_mode & S_IRUSR);
			sp_gid = buf.st_gid;
			sp_uid = buf.st_uid;
		} else {
			DELPTMP();
			(void) f_err();
			exit(FMERR);
		}
	} else  { 
		sp_gid = pwd_gid;
		sp_uid = pwd_uid;
		file_exist = 0;
	}


	/* get the mode of shadow password file  -- mode of the file should
	   be read-only for user or less. */
	umask(mode);

	/* open temporary shadow file */
	if ((tsp_fp = fopen(SHADTEMP, "w")) == NULL) {
		DELPTMP();
		(void) f_err();
		exit(FMERR);
	}

	/* change the group of the temporary shadow password file */
        if (chown(SHADTEMP, sp_uid, sp_gid) < 0) {
		(void) no_convert();
		exit(FMERR);
	}
 
	/* Reads the password file.  				*/
	/* If the shadow password file not exists, or		*/
	/* if an entry doesn't have a corresponding entry in    */
	/* the shadow file, entries/entry will be created.      */
 
    while (!end_of_file) {
	if ((pwdp = getpwent()) != NULL) {
		if (!file_exist || (sp = getspnam(pwdp->pw_name)) == NULL ) {
			if (errno == EINVAL) { /* Bad entry in shadow exit */
				DELSHWTMP();
				DELPTMP();
				(void) f_bdshw();
				exit(BADSHW);
			}
			sp = &sp_pwd;
			sp->sp_namp = pwdp->pw_name;
 			if (*pwdp->pw_passwd == NULL) 
				(void)fprintf(stderr,"%s: WARNING user %s has no password\n", prognamp, sp->sp_namp); 

			/* Copy the password field in the password file to */
			/* shadow password file.  Replace the password */
			/* field with an 'x'.			        */

			sp->sp_pwdp=pwdp->pw_passwd;
			pwdp->pw_passwd = pwdflr;

			/* If login has aging info, split the aging info */
			/* into age, max and min.                        */
			/* Convert aging info from weeks to days */

		   	if (*pwdp->pw_age != NULL) {
				when = (long) a64l (pwdp->pw_age);
				maxweeks = when & 077;
				minweeks = (when >> 6) & 077;
				when >>= 12;
				sp->sp_lstchg = when * 7;
				sp->sp_min = minweeks * 7;
				sp->sp_max = maxweeks * 7;
				sp->sp_warn = -1;
				sp->sp_inact = -1;
				sp->sp_expire = -1;
				sp->sp_flag = 0;
				pwdp->pw_age = "";
			} else {
 
			/* if no aging, last_changed will be the day the 
			conversion is done, min and max fields will be null */
			/* use timezone to get local time */
 
				sp->sp_lstchg = DAY_NOW;
				sp->sp_min =  -1;
				sp->sp_max =  -1;
				sp->sp_warn = -1;
				sp->sp_inact = -1;
				sp->sp_expire = -1;
				sp->sp_flag = 0;
		   	}
		} else {
		/* If an entry in the password file has a string,    	 */
		/* other than 'x', in the password field, that password	 */
		/* entry will be entered in the corresponding entry	 */
		/* in the shadow file.  The password field will be replaced   */
		/* with an 'x'.                                          */
		/* If aging is not specified, last_changed day will be   */
		/* updated and max and min will be cleared.              */
 			if (*pwdp->pw_passwd == NULL) 
				(void)fprintf(stderr,"%s: WARNING user %s has no password\n", prognamp, sp->sp_namp); 
			if (strcmp(pwdp->pw_passwd, pwdflr)) {
				sp->sp_pwdp=pwdp->pw_passwd;
				pwdp->pw_passwd = pwdflr;
		   		if (*pwdp->pw_age == NULL) {
					sp->sp_lstchg = DAY_NOW;
					sp->sp_min =  -1;
					sp->sp_max =  -1;
					sp->sp_warn = -1;
					sp->sp_inact = -1;
					sp->sp_expire = -1;
					sp->sp_flag = 0;
				}
			}
		/* If aging info is added, split aging info into age,    */
		/* max and min.  Convert aging info from weeks to days.  */
		   	if (*pwdp->pw_age != NULL) {
				when = (long) a64l (pwdp->pw_age);
				maxweeks = when & 077;
				minweeks = (when >> 6) & 077;
				when >>= 12;
				sp->sp_lstchg = when * 7;
				sp->sp_min = minweeks * 7;
				sp->sp_max = maxweeks * 7;
				sp->sp_warn = -1;
				sp->sp_inact = -1;
				sp->sp_expire = -1;
				sp->sp_flag = 0;
 			 	pwdp->pw_age = "";
		   	}
		}

		/* write an entry to temporary password file */
		if ((putpwent(pwdp,tp_fp)) != 0 ) {
			(void) no_convert();
			exit(FMERR);
		}

		/* write an entry to temporary shadow password file */
		if (putspent(sp,tsp_fp) != 0) {
			(void) no_convert();
			exit(FMERR);
		}
	} else {
		if (errno == 0)
			end_of_file = 1;
		else if (errno == EINVAL) {
			errno = 0;
			pwerr = 1;
		}
		else
			end_of_file = 1;
	}

	}
 	
	(void)fclose(tsp_fp);
	(void)fclose(tp_fp);
	if (pwerr)
		(void)fprintf(stderr,"%s: WARNING bad entry in /etc/passwd\n",
			 prognamp); 


	/* delete old password file if it exists*/
	if (unlink (OPASSWD) && (access (OPASSWD, 0) == 0)) {
		(void) no_convert();
		exit(FMERR);
	}

	/* rename the password file to old password file  */
	if (rename(PASSWD, OPASSWD) == -1) {
		(void) no_convert();
		exit(FMERR);
	}

	/* rename temporary password file to password file */
	if (rename(PASSTEMP, PASSWD) == -1) {
		/* link old password file to password file */
		if (link(OPASSWD, PASSWD) < 0) {
			(void) no_recover();
			exit(FATAL);
 		}
		(void) no_convert();
		exit(FMERR);
	}
 
	/* delete old shadow password file if it exists */
	if ( unlink (OSHADOW) && (access (OSHADOW, 0) == 0)) {
		/* link old password file to password file */
		if (unlink(PASSWD) || link (OPASSWD, PASSWD)) {
			(void) no_recover();
			exit(FATAL);
 		}
		(void) no_convert();
		exit(FMERR);
	}

	/* link shadow password file to old shadow password file */
	if (file_exist && rename(SHADOW, OSHADOW)) {
		/* link old password file to password file */
		if (unlink(PASSWD) || link (OPASSWD, PASSWD)) {
			(void) no_recover();
			exit(FATAL);
 		}
		(void) no_convert();
		exit(FMERR);
	}


	/* link temporary shadow password file to shadow password file */ 
	if (rename(SHADTEMP, SHADOW) == -1) {
		/* link old shadow password file to shadow password file */
		if (file_exist && (link(OSHADOW, SHADOW))) {
			(void) no_recover();
			exit(FATAL);
		} 
		if (unlink(PASSWD) || link (OPASSWD, PASSWD)) {
			(void) no_recover();
			exit(FATAL);
		}
		(void) no_convert();
		exit(FMERR);
	}

	/* Change old password file to read only by owner   */
	/* If chmod fails, delete the old password file so that */
	/* the password fields can not be read by others */
	if (chmod(OPASSWD, S_IRUSR) < 0) 
		(void) unlink(OPASSWD);
 
	(void) ulckpwdf();
	exit(0);
}
 
void
no_recover()
{
	DELPTMP();
	DELSHWTMP();
	(void) f_miss();
}

void
no_convert()
{
	DELPTMP();
	DELSHWTMP();
	(void) f_err();
}
 
void
f_err()
{
	fprintf(stderr,"%s: Unexpected failure. Conversion not done.\n",prognamp);
  	(void) ulckpwdf();
}

void
f_miss()
{
	fprintf(stderr,"%s: Unexpected failure. Password file(s) missing.\n",prognamp);
  	(void) ulckpwdf();
}

void
f_bdshw()
{
	fprintf(stderr,"%s: Bad entry in /etc/shadow. Conversion not done.\n",prognamp);
  	(void) ulckpwdf();
}
