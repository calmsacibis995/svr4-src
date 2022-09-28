/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* Copyright (c) 1981 Regents of the University of California */
#ident	"@(#)vi:port/expreserve.c	1.22"

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include "ex_tune.h"
#define TMP	"/tmp"

#define FTYPE(A)	(A.st_mode)
#define FMODE(A)	(A.st_mode)
#define	IDENTICAL(A,B)	(A.st_dev==B.st_dev && A.st_ino==B.st_ino)
#define ISBLK(A)	((A.st_mode & S_IFMT) == S_IFBLK)
#define ISCHR(A)	((A.st_mode & S_IFMT) == S_IFCHR)
#define ISDIR(A)	((A.st_mode & S_IFMT) == S_IFDIR)
#define ISFIFO(A)	((A.st_mode & S_IFMT) == S_IFIFO)
#define ISREG(A)	((A.st_mode & S_IFMT) == S_IFREG)

#ifdef VMUNIX
#define	HBLKS	2
#else
#define HBLKS	1
#endif

/*
 * Expreserve - preserve a file in usrpath(preserve)
 *
 * This routine is very naive - it doesn't remove anything from
 * usrpath(preserve)... this may mean that we  * stuff there... 
 * the danger in doing anything with usrpath(preserve)
 * is that the clock may be messed up and we may get confused.
 *
 * We are called in two ways - first from the editor with no arguments
 * and the standard input open on the temp file. Second with an argument
 * to preserve the entire contents of /tmp (root only).
 *
 * BUG: should do something about preserving Rx... (register contents)
 *      temporaries.
 */

#ifndef VMUNIX
#define	LBLKS	125
#else
#define	LBLKS	900
#endif

struct 	header {
	time_t	Time;			/* Time temp file last updated */
	int	Uid;			/* This user's identity */
#ifndef VMUNIX
	short	Flines;			/* Number of lines in file */
#else
	int	Flines;
#endif
	unsigned char	Savedfile[FNSIZE];	/* The current file name */
	short	Blocks[LBLKS];		/* Blocks where line pointers stashed */
	short	encrypted;		/* Encrypted temp file flag */
} H;

struct	passwd *getpwuid();
off_t	lseek();
FILE	*popen();
void exit(), perror();
#define eq(a, b) strcmp(a, b) == 0

main(argc)
	int argc;
{
	register DIR *tf;
	struct dirent *direntry;
	unsigned char *filname;
	struct stat stbuf;

	/*
	 * If only one argument, then preserve the standard input.
	 */
	if (argc == 1) {
		if (copyout((char *) 0))
			exit(1);
		exit(0);
	}

	/*
	 * If not super user, then can only preserve standard input.
	 */
	if (getuid()) {
		fprintf(stderr, "NOT super user\n");
		exit(1);
	}

	/*
	 * ... else preserve all the stuff in /tmp, removing
	 * it as we go.
	 */
	if (chdir(TMP) < 0) {
		perror(TMP);
		exit(1);
	}

	if ((tf = opendir(".")) == NULL)
	{
		perror(TMP);
		exit(1);
	}
	while ((direntry = readdir(tf)) != NULL) 
	{
		filname = (unsigned char *)direntry->d_name;
		/*
		 * Ex temporaries must begin with Ex;
		 * we check that the 10th character of the name is null
		 * so we won't have to worry about non-null terminated names
		 * later on.
		 */
		if (filname[0] != 'E' || filname[1] != 'x' || filname[10])
			continue;
		if (stat((char *)filname, &stbuf))
			continue;
		if (!ISREG(stbuf))
			continue;
		/*
		 * Save the file.
		 */
		(void) copyout(filname);
	}
	closedir(tf);
	exit(0);
}

unsigned char	mydir[] =	USRPRESERVE;
unsigned char	pattern[] =	"/Exaa`XXXXXXXXXX";

/*
 * Copy file name into usrpath(preserve)/...
 * If name is (char *) 0, then do the standard input.
 * We make some checks on the input to make sure it is
 * really an editor temporary, generate a name for the
 * file (this is the slowest thing since we must stat
 * to find a unique name), and finally copy the file.
 */
copyout(name)
	unsigned char *name;
{
	int i;
	static int reenter;
	unsigned char buf[BUFSIZ];
	unsigned char	savdir[256];
	unsigned char	savfil[256];
	unsigned char	mkbuf[256];
	struct passwd *pp;
	struct stat	stbuf;

	/*
	 * The first time we put in the digits of our
	 * process number at the end of the pattern.
	 */
	if (reenter == 0) {
		mkdigits(pattern);
		reenter++;
	}

	/*
	 * If a file name was given, make it the standard
	 * input if possible.
	 */
	if (name != 0) {
		(void) close(0);
		/*
		 * Need read/write access for arcane reasons
		 * (see below).
		 */
		if (open(name, 2) < 0)
			return (-1);
	}

	/*
	 * Get the header block.
	 */
	(void) lseek(0, 0l, 0);
	if (read(0, (char *) &H, sizeof H) != sizeof H) {
format:
		if (name == 0)
			fprintf(stderr, "Buffer format error\t");
		return (-1);
	}

	/*
	 * Consistency checks so we don't copy out garbage.
	 */
	if (H.Flines < 0) {
#ifdef DEBUG
		fprintf(stderr, "Negative number of lines\n");
#endif
		goto format;
	}
	if (H.Blocks[0] != HBLKS || H.Blocks[1] != HBLKS+1) {
#ifdef DEBUG
		fprintf(stderr, "Blocks %d %d\n", H.Blocks[0], H.Blocks[1]);
#endif
		goto format;
	}
	if (name == 0 && H.Uid != getuid()) {
#ifdef DEBUG
		fprintf(stderr, "Wrong user-id\n");
#endif
		goto format;
	}
	if (lseek(0, 0l, 0)) {
#ifdef DEBUG
		fprintf(stderr, "Negative number of lines\n");
#endif
		goto format;
	}

	/*
	 * If no name was assigned to the file, then give it the name
	 * LOST, by putting this in the header.
	 */
	if (H.Savedfile[0] == 0) {
		(void) strcpy(H.Savedfile, "LOST");
		(void) write(0, (char *) &H, sizeof H);
		H.Savedfile[0] = 0;
		(void) lseek(0, 0l, 0);
	}

	/*
	 * File is good.  Get a name and create a file for the copy.
	 */
	mknext(pattern);
	(void) close(1);
	/*
	 * See if preservation directory for user exists.
	 */
	
	strcpy(savdir, mydir);
	pp = getpwuid(H.Uid);
	if (pp)
		strcat(savdir,pp->pw_name);
	else {
		fprintf(stderr, "Unable to get uid for user.\n");
		return(-1);
	}
	if (stat((char *)savdir, &stbuf) < 0) {
		sprintf((char *)mkbuf, "mkdir %s 2> /dev/null \n", savdir);	
		if (system((char *)mkbuf) != 0) {
			fprintf(stderr, "Unable to create directory \"%s\"\n", savdir);
			perror("");
			return(-1);
		}
		(void) chmod (savdir, 0700);
		(void) chown (savdir, H.Uid, 2);
	}
	strcpy (savfil, savdir);
	strcat(savfil, pattern);
	if (creat(savfil, 0600) < 0) {
		if (name == 0)
			perror((char *)savfil);
		return(1);
	}
	/*
	 * Make target owned by user.
	 */

	(void) chown (savfil, H.Uid, 2);

	/*
	 * Copy the file.
	 */
	for (;;) {
		i = read(0, buf, BUFSIZ);
		if (i < 0) {
			if (name)
				perror("Buffer read error");
			(void) unlink(savfil);
			return (-1);
		}
		if (i == 0) {
			if (name)
				(void) unlink(name);
			notify(H.Uid, H.Savedfile, (int) name, H.encrypted);
			return (0);
		}
		if (write(1, buf, i) != i) {
			if (name == 0)
				perror((char *)savfil);
			(void) unlink(savfil);
			return (-1);
		}
	}
}

/*
 * Blast the last 5 characters of cp to be the process number.
 */
mkdigits(cp)
	unsigned char *cp;
{
	register pid_t i;
	register int j;

	for (i = getpid(), j = 10, cp += strlen(cp); j > 0; i /= 10, j--)
		*--cp = i % 10 | '0';
}

/*
 * Make the name in cp be unique by clobbering up to
 * three alphabetic characters into a sequence of the form 'aab', 'aac', etc.
 * Mktemp gets weird names too quickly to be useful here.
 */
mknext(cp)
	unsigned char *cp;
{
	unsigned char *dcp;
	struct stat stb;

	dcp = cp + strlen(cp) - 1;
	while (isdigit(*dcp))
		dcp--;
whoops:
	if (dcp[0] == 'z') {
		dcp[0] = 'a';
		if (dcp[-1] == 'z') {
			dcp[-1] = 'a';
			if (dcp[-2] == 'z')
				fprintf(stderr, "Can't find a name\t");
			dcp[-2]++;
		} else
			dcp[-1]++;
	} else
		dcp[0]++;
	if (stat((char *)cp, &stb) == 0)
		goto whoops;
}

/*
 * Notify user uid that his file fname has been saved.
 */
notify(uid, fname, flag, cryflag)
	int uid;
	unsigned char *fname;
{
	struct passwd *pp = getpwuid(uid);
	register FILE *mf;
	unsigned char cmd[BUFSIZ];
	
	if (pp == NULL)
		return;
	sprintf((char *)cmd, "/usr/bin/mail %s", pp->pw_name);
	mf = popen((char *)cmd, "w");
	if (mf == NULL)
		return;
	setbuf(mf, (char *)cmd);
	if (fname[0] == 0) {
		fprintf(mf,
"A copy of an editor buffer of yours was saved when %s.\n",
		flag ? "the system went down" : "the editor was killed");
		fprintf(mf,
"No name was associated with this buffer so it has been named \"LOST\".\n");
	} else
		fprintf(mf,
"A copy of an editor buffer of your file \"%s\"%s\nwas saved when %s.\n", fname,(cryflag) ? "[ENCRYPTED]" : "",
		/*
		 * "the editor was killed" is perhaps still not an ideal
		 * error message.  Usually, either it was forceably terminated
		 * or the phone was hung up, but we don't know which.
		 */
		flag ? "the system went down" : "the editor was killed");
	fprintf(mf,
"This buffer can be retrieved using the \"recover\" command of the editor.\n");
	fprintf(mf,
"An easy way to do this is to give the command \"vi -r %s\".\n",fname);
	fprintf(mf,
"This works for \"edit\" and \"ex\" also.\n");
	(void) pclose(mf);
}
