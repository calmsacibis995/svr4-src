/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:smtp/src/qlib.c	1.4.3.1"
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <pwd.h>
#include <grp.h>
#include "s_string.h"
#include "smtp.h"
#include "xmail.h"

char spoolsubdir[16];

extern char *UPASROOT;

/*
 *  copy the directory component of `file' into lf.  return
 *  a pointer to the base name of `file'
 */
static char *
copydir(lf, file)
	string *lf;
	char *file;
{
	char *base;

	base = strrchr(file, '/');
	if (base){
		*base = 0;
		s_append(lf, file);
		s_append(lf, "/");
		*base++ = '/';
	} else
		base = file;
	return base;
}

/*
 *  convert the control file name into a file name of the
 *  type specified.
 */
char *
fileoftype(type, ctl)
	char type;
	char *ctl;
{
	static string *x;
	char *cp;

	x = s_reset(x);
	s_append(x, ctl);
	cp = strrchr(s_to_c(x), '/');
	if(cp)
		cp++;
	else
		cp = s_to_c(x);
	*cp = type;
	return s_to_c(x);
}

/*
 *  creates a file with a unique name
 *  based on `template'.  Any trailing string of x's in the
 *  template are converted to pppppsssssvv, where ppppp is the
 *  process id, sssss is the last 16 bits of the time, vv is enough
 *  to make the name unique.  If there aren't enough x's to fit all
 *  of pppppsssssvv, only as much as will fit (starting right to left)
 *  will be substituted for the x's.
 *
 *  returns an open fd or -1 if the file couldn't be created.
 *  the new name is put into template.
 */
mkdatafile(template)
	char *template;
{
	struct stat s;
	int i, len, pid, fd;
	long seed;
	char hash[14];
	char *xp;

	/*
	 *  find the number of x's
	 */
	if((len = strlen(template))==0)
		return -1;
	for(xp = template+len-1; xp>=template; xp--)
		if(*xp!='x')
			break;
	xp++;
	len = len-(xp-template);

	/*
	 *  make sure it's <= 12
	 */
	if(len>12)
		len = 12;
	pid = getpid();
	seed = (time((long *)0)%100000)*100;

	/*
	 *  try 100 different file names
	 */
	for(i=0; i<100; i++){
		sprintf(hash, "%05d%07ld", pid, seed+i);
		strncpy(xp, hash+(12-len), len);
		if(stat(template, &s)>=0)
			continue;
		if((fd = creat(template, 0660))<0)
			return -1;
		return fd;
	}
	return -1;
}

/*
 *  creates a control file to go with the data file.
 *  the control file name is the same as the data file
 *  with the first character replaced by 'C'.
 */
int
mkctlfile(letter, dataname, contents)
	char letter;
	char *dataname;
	char *contents;
{
	int fd;
	static string *cf;
	static string *tf;

	/*
 	 *  make the file names
	 */
	cf = s_reset(cf);
	s_append(cf, fileoftype(letter, dataname));
	tf = s_reset(tf);
	s_append(tf, fileoftype('T', dataname));

	/*
	 *  create the control file with a temporary (non-control) name
	 */
	fd = creat(s_to_c(tf), 0660);
	if (fd<0)
		return -1;
	if(write(fd, contents, strlen(contents))!=strlen(contents)){
		close(fd);
		unlink(s_to_c(tf));
		return -1;
	}
	if(close(fd)<0){
		unlink(s_to_c(tf));
		return -1;
	}

	/*
	 *  change it's name so that it looks like a control file
	 */
	if(link(s_to_c(tf), s_to_c(cf))<0){
		unlink(s_to_c(tf));
		return -1;
	}
	unlink(s_to_c(tf));
	return 0;
}

/*
 *  Fill name with the lock name for file.  The lockname is
 *  dir/L.xxx where dir is the directory containing the file
 *  being locked and xxx is the first 12 characters of that file's
 *  name.
 */
static
setlname(lf, file)
	string *lf;
	char *file;
{
	char *base;

	/*
	 *  copy over directory portion
	 */
	base = copydir(lf, file);

	/*
	 *  copy in the rest
	 */
	s_append(lf, "L.");
	s_append(lf, base);

	/*
	 *  make sure we didn't get too long
	 */
	base = strrchr(s_to_c(lf), '/');
	if(base)
		base++;
	else
		base = s_to_c(lf);
	if((int)strlen(base)>14)
		base[14] = 0;
	return 0;
}

/*
 *  Return true if file has been locked by us or another program using the same
 *  lock name scheme.
 *
 *  Remove the lock file if the locking process has gone away.
 */
int
islocked(file)
	char *file;
{
	struct stat stbuf;
	static string *ln;
	int pid;
	FILE *fp;

	ln = s_reset(ln);
	if(setlname(ln, file)<0)
		return 1;
	if(stat(s_to_c(ln), &stbuf)==0) {
		fp = fopen(s_to_c(ln), "r");
		if (fp == 0 || fscanf(fp, "%d", &pid)!=1) {
			/*
			 *  either we made the lock wrong
			 *  or it just went away (race)
			 */
			fprintf(stderr,	"can't read pid: breaking lock %s\n",
				s_to_c(ln));
			if(fp)
				fclose(fp);
			unlink(s_to_c(ln));
			return 0;
		}
		if(fp)
			fclose(fp);
		if (kill(pid, 0) == 0)
			return 1;

		/*
		 *  locker has gone away
		 */
		fprintf(stderr, "breaking stale lock %s\n", s_to_c(ln));
		unlink(s_to_c(ln));
	}
	return 0;
}

/*
 *  lock a file being processed.  see setlname (above) for the name of the lock.
 *  the lock file is in the same directory.
 *
 *  returns 0 if the lock was granted, -1 otherwise.  this is a none
 *  blocking routine.
 */
lock(file)
	char *file;
{
	int fd;
	char pidbuf[20];
	static string *tn;
	static string *ln;

	/*
	 *  create a temporary file (in same directory)
	 */
	tn = s_reset(tn);
	copydir(tn, file);
	s_append(tn, "T.xxxxxxxxxxxx");
	fd = mkdatafile(s_to_c(tn));
	if(fd<0)
		return -1;
	sprintf(pidbuf, "%d lock", getpid());
	write(fd, pidbuf, strlen(pidbuf));
	close(fd);

	/*
	 *  Make a link to it with the lock file name.  This will fail only
	 *  if it already exists.
	 */
	ln = s_reset(ln);
	setlname(ln, file);
	while(link(s_to_c(tn), s_to_c(ln)) < 0) {
		/*
		 *  might be a stale lock
		 */
		if(islocked(file)){
			unlink(s_to_c(tn));
			return -1;
		}
	}
	unlink(s_to_c(tn));
	return 0;
}

/*
 *  unlock a file
 */
unlock(file)
	char *file;
{
	static string *ln;

	ln = s_reset(ln);
	setlname(ln, file);
	unlink(s_to_c(ln));
}

/*
 *  make a spool directory and cd into it.  the spool directory is in
 *  /usr/spool/smtpq and it's name is the 2 most significant elements of
 *  the domain name, `target'.
 */
gotodir(target)
	char *target;
{
	register char *bp, *lp;
	register struct passwd *pw = NULL;
	register struct group *gr = NULL;
#ifdef SVR3
	struct passwd *getpwnam();
	struct group *getgrnam();
#endif
	int elems;

	if (chdir(SMTPQROOT) < 0) {
		mkdir(SMTPQROOT, 0775);
		if (chdir(SMTPQROOT) < 0)
			return -1;
		if ((pw = getpwnam("uucp")) == NULL)
			return -1;
		if ((gr = getgrnam("mail")) == NULL)
			return -1;
		(void) chmod(".", 0775);
		(void) chown(".", pw->pw_uid, gr->gr_gid);
	}

	if(strchr(target, '/')==0){
		lp = bp = target+strlen(target)-1;
		for(elems=0; bp>target && lp-bp<14; bp--){
			if(*bp=='.')
				if(++elems==2){
					bp++;
					break;
				}
		}
		strcpy(spoolsubdir, bp);
	} else {
		bp = target;
		lp = spoolsubdir;
		for(elems=0; bp-target<14; bp++){
			if(*bp=='/') {
				if(++elems==2)
					break;
				*lp++ = '.';
			} else
				*lp++ = *bp;
		}
		*lp = 0;
	}

	if (chdir(spoolsubdir) < 0) {
		mkdir(spoolsubdir, 0775);
		if (chdir(spoolsubdir) < 0)
			return -1;
		if (!pw && ((pw = getpwnam("uucp")) == NULL))
			return -1;
		if (!gr && ((gr = getgrnam("mail")) == NULL))
			return -1;
		(void) chmod(".", 0775);
		(void) chown(".", pw->pw_uid, gr->gr_gid);
	}

	return 0;
}

/*
 *  start the scheduler
 */
smtpsched(av0, target)
	char *av0;
	char *target;
{
	int status;
	static string *cmd;

	switch(fork()){
	case -1:
		break;
	case 0:
		/*
		 *  exec the sched process
		 */
		cmd = s_reset(cmd);
		s_append(cmd, UPASROOT);
		s_append(cmd, "/smtpsched");
		execl(s_to_c(cmd), av0, target, (char *)0);
		exit(1);
	default:
#ifndef SVR3
#ifndef SVR4
		/*
		 *  wait for any sub processes to finish
		 */
		while(wait(&status)>=0)
			;
#endif
#endif
		break;
	}
}
