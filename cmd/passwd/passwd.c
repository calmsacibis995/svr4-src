/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)passwd:passwd.c	1.4.7.4"

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */
/*
 * passwd is a program whose sole purpose is to manage 
 * the password file. It allows system administrator
 * to add, change and display password attributes.
 * Non privileged user can change password or display 
 * password attributes which corresponds to their login name.
 */

#include <stdio.h>
#include <signal.h>
#include <pwd.h>
#include <shadow.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>         /* isalpha(c), isdigit(c), islower(c), toupper(c) */
#include <errno.h>
#include <crypt.h>
#include <deflt.h>
#include <unistd.h>
#include <stdlib.h>


#define PWADMIN	"/etc/default/passwd"
#define MINWEEKS	-1	/* minimum weeks before next password change */
#define MAXWEEKS	-1	/* maximum weeks before password change */
#define WARNWEEKS	-1	/* number weeks before password expires 
                                   to warn the user */
#define MINLENGTH	6	/* minimum length for passwords */
/* flags  indicate password attributes to be modified */

#define PFLAG 0x080		/* change user's password */  
#define LFLAG 0x001		/* lock user's password  */
#define DFLAG 0x002		/* delete user's  password */
#define MFLAG 0x004		/* set max field -- # of days passwd is valid */		 
#define NFLAG 0x008		/* set min field -- # of days between password changes */
#define SFLAG 0x010		/* display password attributes */
#define FFLAG 0x020		/* expire  user's password */
#define AFLAG 0x040		/* display password attributes for all users*/
#define SAFLAG (SFLAG|AFLAG)	/* display password attributes for all users*/
#define WFLAG 0x100		/* warn user to change passwd */

/* exit  code */

#define SUCCESS	0	/* succeeded */
#define NOPERM	1	/* No permission */
#define BADSYN	2	/* Incorrect syntax */
#define FMERR	3	/* File manipulation error */
#define FATAL	4	/* Old file can not be recover */
#define FBUSY	5	/* Lock file busy */
#define BADOPT	6	/* Invalid argument to option */
#define BADAGE	6	/* Aging is disabled  */
 
/* define error messages */
#define MSG_NP	"Permission denied"
#define MSG_NV  "Invalid argument to option"
#define MSG_BS	"Invalid combination of options"
#define MSG_FE	"Unexpected failure. Password file unchanged."
#define MSG_FF	"Unexpected failure. Password file missing."
#define MSG_FB	"Password file(s) busy. Try again later."
#define MSG_AD	"Password aging is disabled"

/* return code from ckarg() routine */
#define FAIL 		-1

#define NUMCP	13	/* number of characters for valid password */
#define MINLENGTH 6  	/* for passwords */

/* print password status */

#define PRT_PWD(pwdp)	{\
	if (*pwdp == NULL) \
		fprintf(stdout, "NP  ");\
	else if (strlen(pwdp) < NUMCP) \
		(void) fprintf(stdout, "LK  ");\
	else\
		(void) fprintf(stdout, "PS  ");\
}

#define PRT_AGE()	{\
	if (sp->sp_max != -1) { \
		if (sp->sp_lstchg) {\
			lstchg = sp->sp_lstchg * DAY;\
			tmp = gmtime(&lstchg);\
			(void) fprintf(stdout,"%.2d/%.2d/%.2d  ",(tmp->tm_mon + 1),tmp->tm_mday,tmp->tm_year);\
		} else\
			(void) fprintf(stdout,"00/00/00  ");\
		if ((sp->sp_min >= 0) && (sp->sp_warn > 0))\
			(void) fprintf(stdout, "%d  %d  %d ", sp->sp_min, sp->sp_max, sp->sp_warn);\
		else if (sp->sp_min >= 0) \
			(void) fprintf(stdout, "%d  %d  ", sp->sp_min, sp->sp_max);\
		else if (sp->sp_warn > 0) \
			(void) fprintf(stdout, "    %d  %d ",  sp->sp_max, sp->sp_warn);\
		else \
			(void) fprintf(stdout, "    %d  ", sp->sp_max);\
	}\
}

extern int optind;
struct passwd *pwd;
struct spwd *sp;
extern  int 	errno;
char 	lkstring[] = "*LK*";		/*lock string  to lock user's password*/
char	nullstr[] = "";
/* usage message of non privileged user */
char 	usage[]  = "	Usage:\n\tpasswd [-s] [name]\n";
/* usage message of privileged user */
char 	sausage[]  = "	Usage:\n\tpasswd [name]\n\tpasswd  [-l|-d]  [-n min] [-f] [-x max] [-w warn] name\n\tpasswd -s [-a]\n\tpasswd -s [name]\n";
char	opwbuf[10];
char	*pw, *uname, *prognamep, *pswd;
int 	retval,opwlen;
uid_t	uid;
int 	mindate, maxdate, warndate;		/* password aging information */
int 	minlength;
int 	end_of_file = 0;
int 	pwd_error = 0;
int 	sp_error = 0;

main (argc, argv)
int argc;
char *argv[];
{

	/* passwd calls getpass() to get the password. The getpass() routine
	   returns at most 8 charaters. Therefore, array of 10 chars is
	   sufficient.
	*/
	char	pwbuf[10];			
	char	buf[10];
	char 	*p, *o;
	char 	saltc[2];	 /* crypt() takes 2 char string as a salt */
	 int	count; 		 /* count verifications */
	time_t 	salt;
	int 	insist;
	int 	tmpflag, flags;
	int	c;
	int 	i, j, k, flag;	/* for triviality checks */
	int 	pwlen;  	/* length of old passwords */

	flag = 0;
	insist = 0;		/* # of times the program  prompts 
				 * for valid password */
	count = 0;
	prognamep = argv[0];	
	uid = getuid();		/* get the user id */
	
	/* 
	 * ckarg() parses the arguments. In case of an error, 
	 * it sets the retval and returns FAIL (-1). It return
	 * the value which indicate which password attributes
	 * to modified 
	*/

	switch (flag = ckarg( argc, argv)) { 
	case FAIL:		/* failed */
		exit(retval); 
	case SAFLAG:		/* display password attributes */
		exit(display((struct spwd *) NULL));
	default:	
		break;
	}

	argc -= optind;

	if (argc < 1 ) {
		if ((uname = getlogin()) == NULL) { 
			(void) fprintf(stderr, "%s", uid > 0 ? usage :sausage);
			exit(NOPERM);
		} else if (!flag)	/* if flag set, must be displaying or */
					/* modifying password aging attributes */
			(void) fprintf(stderr,"%s:  Changing password for %s\n",
				       prognamep,  uname);
	} else
		uname = argv[optind];

	if (((pwd = getpwnam(uname)) == NULL) ||  
            ((sp = getspnam(uname)) == NULL)) { 
		(void) fprintf(stderr, "%s:  %s does not exist\n", 
   			       prognamep,  uname);
		exit(NOPERM);
	}

	if (uid != 0 && uid != pwd->pw_uid) { 
		(void) fprintf(stderr, "%s: %s\n", prognamep, MSG_NP);
		exit(NOPERM);
	}

	/* If the flag is set display/update password attributes.*/

	switch (flag) {
	case SFLAG:		/* display password attributes */
		exit(display(sp));
	case 0:			/* changing user password */
		break;
	default:		/* changing user password attributes */
		/* lock the password file */
		if (lckpwdf() != 0) {
			(void) fprintf(stderr, "%s: %s\n", prognamep, MSG_FB);
			exit(FBUSY);
		}
		retval = update(flag);
		(void) ulckpwdf();
		exit(retval);

	}

	if ((retval = ck_passwd(&flag)) != SUCCESS)
		exit(retval);
tryagn:
	if( insist >= 3) {       /* three chances to meet triviality standard */
		(void) fprintf(stderr, "Too many failures - try later\n", prognamep);
		exit(NOPERM);
	}
	if ((pswd = getpass("New password:")) == NULL) {
		(void) fprintf(stderr, "Sorry.\n");
		return(FMERR);
	}
	else {
		(void) strcpy (pwbuf, pswd);
		pwlen = strlen (pwbuf);
	}

	/* Make sure new password is long enough */

	if (uid != 0 && (pwlen < MINLENGTH ) ) { 
		(void) fprintf(stderr, "Password is too short - must be at least 6 characters\n");
		insist++;
		goto tryagn;
	}

	/* Check the circular shift of the logonid */
	 
	if( uid != 0 && circ(uname, pwbuf) ) {
		(void) fprintf(stderr, "Password cannot be circular shift of logonid\n");
		insist++;
		goto tryagn;
	}

	/* Insure passwords contain at least two alpha characters */
	/* and one numeric or special character */               

	flags = 0;
	tmpflag = 0;
	p = pwbuf;
	if (uid != 0) {
		while (c = *p++) {
			if (isalpha(c) && tmpflag )
				 flags |= 1;
			else if (isalpha(c) && !tmpflag ) {
				flags |= 2;
				tmpflag = 1;
			} else if (isdigit(c) ) 
				flags |= 4;
			else 
				flags |= 8;
		}
		 
		/*		7 = lca,lca,num
		 *		7 = lca,uca,num
		 *		7 = uca,uca,num
		 *		11 = lca,lca,spec
		 *		11 = lca,uca,spec
		 *		11 = uca,uca,spec
		 *		15 = spec,num,alpha,alpha
		 */
		 
		if ( flags != 7 && flags != 11 && flags != 15  ) {
			(void) fprintf(stderr,"Password must contain at least two alphabetic characters and\n");
			(void) fprintf(stderr,"at least one numeric or special character.\n");
			insist++;
			goto tryagn;
		}
	}
	if ( uid != 0 ) {
		p = pwbuf;
		o = opwbuf;
		if ( pwlen >= opwlen) {
			i = pwlen;
			k = pwlen - opwlen;
		} else {
			i = opwlen;
			k = opwlen - pwlen;
		}
		for ( j = 1; j  <= i; j++ ) 
			if ( *p++ != *o++ ) 
				k++;
		if ( k  <  3 ) {
			(void) fprintf(stderr, "Passwords must differ by at least 3 positions\n");
			insist++;
			goto tryagn;
		}
	}

	/* Ensure password was typed correctly, user gets three chances */

	if ((pswd = getpass("Re-enter new password:")) == NULL) {
		(void) fprintf(stderr, "Sorry.\n");
		return(FMERR);
	}
	else 
	(void) strcpy (buf, pswd);
	if (strcmp (buf, pwbuf)) {
		if (++count > 2) { 
			(void) fprintf(stderr, "%s: Too many tries; try again later\n", prognamep);
			exit(NOPERM);
		} else
			(void) fprintf(stderr, "They don't match; try again.\n");
		goto tryagn;
	}

	/* Construct salt, then encrypt the new password */

	(void) time((time_t *)&salt);
	salt += (long)getpid();

	saltc[0] = salt & 077;
	saltc[1] = (salt >> 6) & 077;
	for (i=0; i<2; i++) {
		c = saltc[i] + '.';
		if (c>'9') c += 7;
		if (c>'Z') c += 6;
		saltc[i] = c;
	}
	pw = crypt (pwbuf, saltc);

	/* lock the password file */
	if (lckpwdf() != 0) {
		(void) fprintf(stderr, "%s: %s\n", prognamep, MSG_FB);
		return (FBUSY);
	}
	flag |= PFLAG;
	retval = update(flag);
	(void) ulckpwdf();
	exit(retval);
}


/* 
 * ck_passwd():  Verify user old password. It also check 
 * password aging information to varify that user is authorized
 * to change password.
 */ 

int
ck_passwd(flagp)
int *flagp;
{
	register int now;
	if (sp->sp_pwdp[0] && uid != 0) {
		if ((pswd = getpass("Old password:")) == NULL) {
		(void) fprintf(stderr, "Sorry.\n");
		return(FMERR);
		}
		else {
		(void) strcpy (opwbuf, pswd);
		opwlen = strlen(opwbuf);       /* get length of old password */
		pw = crypt (opwbuf, sp->sp_pwdp);
		}
		if (strcmp (pw, sp->sp_pwdp) != 0) {
			(void) fprintf(stderr, "Sorry.\n");
			return (NOPERM);
		}
	} else
		opwbuf[0] = '\0';
	/* password age checking applies */
	if (sp->sp_max != -1 && sp->sp_lstchg != 0) {
		now  =  DAY_NOW;
		if (sp->sp_lstchg <= now) {
			if (uid != 0 && ( now < sp->sp_lstchg  + sp->sp_min)) { 
				(void) fprintf(stderr, "%s:  Sorry: < %ld days since the last change\n", prognamep, sp->sp_min);
				return (NOPERM);
			}
			if (sp->sp_min > sp->sp_max && uid != 0) { 
				(void) fprintf(stderr, "%s: You may not change this password\n", prognamep);
				return (NOPERM);
			}
		}
	/* aging is turned on */
	} else if(sp->sp_lstchg == 0 && sp->sp_max > 0 || sp->sp_min > 0) {
			return (SUCCESS);
	} else {
		/* aging not turned on */
		/* so turn on passwd for user with default values */
		*flagp |= MFLAG;
		*flagp |= NFLAG;
		*flagp |= WFLAG;
	}
	return (SUCCESS);
}

/*
 * update(): updates the password file.	    
 * It takes "flag" as an argument to determine which 
 * password attributes to modify. It returns 0 for success 
 * and  > 0 for failure.
 * Side effect: Variable sp points to NULL.
 */

int 
update(flag)
int flag;
{
	register int i;
	struct stat buf;
	register found = 0;
	FILE *tsfp;


	/* ignore all the signals */

	for (i=1; i < NSIG; i++)
		(void) sigset(i, SIG_IGN);
	

 	/* Clear the errno. It is set because SIGKILL can not be ignored. */

	errno = 0;

	/* Mode  of the shadow file should be 400 or 000 */

	if (stat(SHADOW, &buf) < 0) {
		(void) fprintf(stderr, "%s: %s\n", prognamep, MSG_FE);
		return (FMERR);
	}

	(void) umask(S_IAMB & ~(buf.st_mode & S_IRUSR));
	if ((tsfp = fopen(SHADTEMP, "w")) == NULL) {
		(void) fprintf(stderr, "%s: %s\n", prognamep, MSG_FE);
		return (FMERR);
	}

	/*
	 *	copy passwd  files to temps, replacing matching lines
	 *	with new password attributes.
	 */

	end_of_file = 0;
	errno = 0;
	sp_error = 0;

	while (!end_of_file) {
	if ((sp = getspent()) != NULL) {
		if (strcmp(sp->sp_namp, uname) == 0) { 
			found = 1;
			/* LFLAG and DFLAG should be checked before FFLAG.
			   FFLAG clears the sp_lstchg field. We do not
			   want sp_lstchg field to be set if one execute
			   passwd -d -f name or passwd -l -f name.
			*/

			if (flag & LFLAG) {	 /* lock password */
				sp->sp_pwdp = pw;
				sp->sp_lstchg = DAY_NOW;
			} 
			if (flag & DFLAG) {	 /* delete password */
				sp->sp_pwdp = nullstr;
				sp->sp_lstchg = DAY_NOW;
			} 
			if (flag & FFLAG)	 /* expire password */
				sp->sp_lstchg = (long) 0;
			if (flag & MFLAG)  { 	/* set max field */
				if (!(flag & NFLAG) && sp->sp_min == -1)
					sp->sp_min = 0;
				if (maxdate == -1) {	/* trun off aging */
					sp->sp_min = -1;
					sp->sp_warn = -1;
				}
				else if (sp->sp_max == -1)
					sp->sp_lstchg = 0;
				sp->sp_max = maxdate;
			}
			if (flag & NFLAG) {   /* set min field */
				if (sp->sp_max == -1 && mindate != -1) {
					(void) fprintf(stderr,"%s\n %s", MSG_AD, sausage); 
					(void) unlink(SHADTEMP);
					return (BADAGE);
				}
				sp->sp_min = mindate;
			}
			if (flag & WFLAG) {   /* set warn field */
				if (sp->sp_max == -1 && warndate != -1) {
					(void) fprintf(stderr,"%s\n %s", MSG_AD, sausage); 
					(void) unlink(SHADTEMP);
					return (BADAGE);
				}
				sp->sp_warn = warndate;
			}
			if (flag & PFLAG)  {	/* change password */
				sp->sp_pwdp = pw;
				/* update the last change field */
				sp->sp_lstchg = DAY_NOW;
				if (sp->sp_max == 0) {   /* turn off aging */
					sp->sp_max = -1;
					sp->sp_min = -1;
				}
			}
		}
		if (putspent (sp, tsfp) != 0) { 
			(void) unlink(SHADTEMP);
			(void) fprintf(stderr, "%s: %s\n", prognamep, MSG_FE);
			return (FMERR);
		}

	} else {
 		if (errno == 0) 
			/* end of file */
			end_of_file = 1;	
		else if (errno == EINVAL) {
			/*bad entry found in /etc/shadow, skip it */
			errno = 0;
			sp_error++;
		}  else
			/* unexpected error found */
			end_of_file = 1;
	}
	} /*end of while*/

	if (sp_error >= 1)
		fprintf(stderr, "%s: Bad entry found in the password file.\n",
				prognamep);

	if (fclose (tsfp)) {
		(void) unlink(SHADTEMP);
		(void) fprintf(stderr, "%s: %s\n", prognamep, MSG_FE);
		return (FMERR);
	}


	/* Check if user name exists */

	if (found == 0) {
		(void) unlink(SHADTEMP);
		(void) fprintf(stderr, "%s: %s\n", prognamep, MSG_FE);
		return (FMERR);
	}

	/*
	 *	Rename temp file back to  appropriate passwd file.
	 */

	/* remove old passwd file */
	if (unlink(OSHADOW) && access(OSHADOW, 0) == 0) {
		(void) unlink(SHADTEMP);
		(void) fprintf(stderr, "%s: %s\n", prognamep, MSG_FE);
		return (FMERR);
	}

	/* rename password file to old password file */
	if (rename(SHADOW, OSHADOW) == -1) {
		(void) unlink(SHADTEMP);
		(void) fprintf(stderr, "%s: %s\n", prognamep, MSG_FE);
		return (FMERR);
	}

	/* rename temparory password file to password file */
	if (rename(SHADTEMP, SHADOW) == -1) {
		(void) unlink(SHADOW);
		if (link (OSHADOW, SHADOW)) { 
			(void) fprintf(stderr, "%s: %s\n", prognamep, MSG_FF);
			return (FATAL);
		}
		(void) fprintf(stderr, "%s: %s\n", prognamep, MSG_FE);
		return (FMERR);
	}

	return(SUCCESS);
}

circ( s, t )
char *s, *t;
{
	char c, *p, *o, *r, buff[25], ubuff[25], pubuff[25];
	int i, j, k, l, m;
	 
	m = 2;
	i = strlen(s);
	o = &ubuff[0];
	for ( p = s; c = *p++; *o++ = c ) 
		if ( islower(c) )
			 c = toupper(c);
	*o = '\0';
	o = &pubuff[0];
	for ( p = t; c = *p++; *o++ = c ) 
		if ( islower(c) ) 
			c = toupper(c);

	*o = '\0';
	 
	p = &ubuff[0];
	while ( m-- ) {
		for ( k = 0; k  <=  i; k++) {
			c = *p++;
			o = p;
			l = i;
			r = &buff[0];
			while (--l) 
				*r++ = *o++;
			*r++ = c;
			*r = '\0';
			p = &buff[0];
			if ( strcmp( p, pubuff ) == 0 ) 
				return (1);
		}
		p = p + i;
		r = &ubuff[0];;
		j = i;
		while ( j-- ) 
			*--p = *r++;
	}
	return (SUCCESS);
}

/*
 * ckarg(): This function parses and verifies the 	
 * arguments.  It takes two parameters:			
 * argc => # of arguments				
 * argv => pointer to an argument			
 * In case of an error it prints the appropriate error 	
 * message, sets the retval and returns FAIL(-1).	 		
 */

int
ckarg(argc, argv)
int argc;
char **argv;
{
	extern char *optarg;
	char *char_p;
	register int c, flag = 0;

        /*
         * if password administration file can't be opened
         * use built in defaults.
         */
        if( (defopen(PWADMIN)) != 0) { /* M005  start */
		mindate = MINWEEKS * 7;
		maxdate = MAXWEEKS * 7;
		warndate = WARNWEEKS * 7;
                minlength = MINLENGTH;
        }
        else {
                if( (char_p=defread("PASSLENGTH=")) == NULL )
                        minlength = MINLENGTH;
                else {
                        minlength = atoi(char_p);
                        if( minlength < 6 || minlength > 8 )
                                minlength = MINLENGTH;
                }
		if( (char_p=defread("MINWEEKS=")) == NULL )
			mindate = 7 * MINWEEKS;
		else {
			mindate = 7 * atoi(char_p);
			if (mindate < 0)
				mindate = 7 * MINWEEKS;
		}
		if( (char_p=defread("WARNWEEKS=")) == NULL )
			warndate = 7 * WARNWEEKS;
		else {
			warndate = 7 * atoi(char_p);
			if (warndate < 0)
				warndate = 7 * WARNWEEKS;
		}
		if( (char_p=defread("MAXWEEKS=")) == NULL )
			maxdate = 7 * MAXWEEKS;
		else if ((maxdate = atoi(char_p)) == -1) {
			mindate = -1;
			warndate = -1;
		}
		else if (maxdate < -1)
			maxdate = 7 * MAXWEEKS;
		else
			maxdate *= 7;
                defopen(NULL);                  /* close defaults file */
        }
#ifdef DEBUG
	printf("ckarg: maxdate == %d, mindate == %d\n", maxdate, mindate);
#endif
	while ((c = getopt(argc, argv, "aldfsx:n:w:")) != EOF) {
		switch (c) {
		case 'd':		/* delet the password */
			/* Only privileged process can execute this */
			if (ckuid() != 0)
				return (FAIL);
			if (flag & (LFLAG|SAFLAG|DFLAG)) {
				(void) fprintf(stderr,"%s\n %s", MSG_BS, sausage); 
				retval = BADSYN;
				return (FAIL);
			}
			flag |= DFLAG;
			pw = nullstr;
			break;
		case 'l':		/* lock the password */
			/* Only privileged process can execute this */
			if (ckuid() != 0)
				return (FAIL);
			if (flag & (DFLAG|SAFLAG|LFLAG))  {
				(void) fprintf(stderr,"%s\n %s", MSG_BS,sausage);
				retval = BADSYN;
				return (FAIL);
			}
			flag |= LFLAG;
			pw = &lkstring[0];
			break;
		case 'x':		/* set the max date */
			/* Only privileged process can execute this */
			if (ckuid() != 0)
				return (FAIL);
			if (flag & (SAFLAG|MFLAG)) {
				(void) fprintf(stderr,"%s\n %s", MSG_BS, sausage);
				retval = BADSYN;
				return (FAIL);
			}
			flag |= MFLAG;
			if ((maxdate = (int) strtol(optarg, &char_p, 10)) < -1
			    || *char_p != '\0' || strlen(optarg)  <= 0) {
				(void) fprintf(stderr, "%s: %s -x\n", prognamep, MSG_NV);
				retval = BADOPT;
				return (FAIL);
			}
			break;
		case 'n':		/* set the min date */
			/* Only privileged process can execute this */
			if (ckuid() != 0)
				return (FAIL);
			if (flag & (SAFLAG|NFLAG)) { 
				(void) fprintf(stderr,"%s\n %s", MSG_BS, sausage);
				retval = BADSYN;
				return (FAIL);
			}
			flag |= NFLAG;
			if (((mindate = (int) strtol(optarg, &char_p,10)) < 0 
			    || *char_p != '\0') || strlen(optarg)  <= 0) {
				(void) fprintf(stderr, "%s: %s -n\n", prognamep, MSG_NV);
				retval = BADOPT;
				return (FAIL);
			} 
			break;
		case 'w':		/* set the warning field */
			/* Only privileged process can execute this */
			if (ckuid() != 0)
				return (FAIL);
			if (flag & (SAFLAG|WFLAG)) { 
				(void) fprintf(stderr,"%s\n %s", MSG_BS, sausage);
				retval = BADSYN;
				return (FAIL);
			}
			flag |= WFLAG;
			if (((warndate = (int) strtol(optarg, &char_p,10)) < 0 
			    || *char_p != '\0') || strlen(optarg)  <= 0) {
				(void) fprintf(stderr, "%s: %s -w\n", prognamep, MSG_NV);
				retval = BADOPT;
				return (FAIL);
			} 
			break;
		case 's':		/* display password attributes */
			if (flag && (flag != AFLAG)) { 
				(void) fprintf(stderr,"%s\n %s", MSG_BS, sausage);
				retval = BADSYN;
				return (FAIL);
			} 
			flag |= SFLAG;
			break;
		case 'a':		/* display password attributes */
			/* Only privileged process can execute this */
			if (ckuid() != 0)
				return (FAIL);
			if (flag && (flag != SFLAG)) { 
				(void) fprintf(stderr,"%s\n %s", MSG_BS,sausage);
				retval = BADSYN;
				return (FAIL);
			} 
			flag |= AFLAG;
			break;
		case 'f':		/* expire password attributes */
			/* Only privileged process can execute this */
			if (ckuid() != 0)
				return (FAIL);
			if (flag & (SAFLAG|FFLAG)) { 
				(void) fprintf(stderr,"%s\n %s", MSG_BS, uid > 0 ? usage : sausage);
				retval = BADSYN;
				return (FAIL);
			} 
			flag |= FFLAG;
			break;
		case '?':
			(void) fprintf(stderr,"%s", uid > 0 ? usage : sausage);
			retval = BADSYN;
			return (FAIL);
		}
	}

	argc -=  optind;
	if (argc > 1) {
		fprintf(stderr,"%s", uid > 0 ? usage : sausage);
		retval = BADSYN;
		return (FAIL);
	}

	/* If no options are specified or only the show option */
	/* is specified, return because no option error checking */
	/* is needed */
	if (!flag || (flag == SFLAG)) 
		return (flag);

	if (flag == AFLAG) {
		(void) fprintf(stderr,"%s", sausage);
		retval = BADSYN;
		return (FAIL);
	}
	if (flag != SAFLAG && argc < 1) {
		(void) fprintf(stderr,"%s", sausage);
		retval = BADSYN;
		return (FAIL);
	}
	if (flag == SAFLAG && argc >= 1) {
		(void) fprintf(stderr,"%s", sausage);
		retval = BADSYN;
		return (FAIL);
	}
	if ((maxdate == -1) &&  (flag & NFLAG)) {
		(void) fprintf(stderr, "%s: %s -x\n", prognamep, MSG_NV);
		retval = BADOPT;
		return (FAIL);
	}
	return (flag);
}

/*
 *  display():  displays password attributes.       
 *  It takes user name as a parameter. If the user    
 *  name is NULL then it displays password attributes 
 *  for all entries on the file. It returns 0 for     
 *  success and positive  number for failure.	      
 */

int
display(sp)
struct spwd *sp;
{
	struct tm *tmp;
	register int found = 0;
	long lstchg;

	if (sp != NULL) {
		(void) fprintf(stdout,"%s  ", sp->sp_namp);
		PRT_PWD(sp->sp_pwdp);
		PRT_AGE();
		(void) fprintf(stdout,"\n");
		return (SUCCESS);
	} 
	end_of_file =0;
	pwd_error = 0;
	errno = 0;
	sp_error = 0;

	while (!end_of_file) {
	if ((pwd = getpwent()) != NULL) {
		if ((sp = getspnam(pwd->pw_name)) != NULL) {
			found++;
			(void) fprintf(stdout,"%s  ", pwd->pw_name);
			PRT_PWD(sp->sp_pwdp);
			PRT_AGE();
			(void) fprintf(stdout, "\n");
		}
	} else {
		if (errno == 0) 
			end_of_file = 1;
		else if (errno == EINVAL) {
			/* Bad entry found in /etc/passwd, skip it */	
			pwd_error++;
			errno = 0;
		} else 
			/* unexpected error found */
			end_of_file = 1;
	}
	} /*end of while*/

	if (pwd_error >=1)
		fprintf(stderr,"%s: Bad entry found in the password file.\n",
			prognamep);
		 	
	/* If password files do not have any entries or files are missing, 
	   return fatal error.
	*/
	if (found == 0) {
		(void) fprintf(stderr, "%s: %s\n", prognamep, MSG_FF);
		return (FATAL);
	}
	return (SUCCESS);
}

int
ckuid()
{
	if(uid != 0) {
		(void) fprintf(stderr,"%s: %s\n", prognamep, MSG_NP);
		return (retval = NOPERM); 
	} 
	return (SUCCESS);
}

