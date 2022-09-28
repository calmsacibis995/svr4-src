/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)chmod:chmod.c	1.10"

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

/*
 * chmod option mode files
 * where 
 *	mode is [ugoa][+-=][rwxlstugo] or an octal number
 *	option is -R
 */

/*
 *  Note that many convolutions are necessary
 *  due to the re-use of bits between locking
 *  and setgid
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/dir.h>

#define	USER	05700	/* user's bits */
#define	GROUP	02070	/* group's bits */
#define	OTHER	00007	/* other's bits */
#define	ALL	07777	/* all */

#define	READ	00444	/* read permit */
#define	WRITE	00222	/* write permit */
#define	EXEC	00111	/* exec permit */
#define	SETID	06000	/* set[ug]id */
#define	LOCK	02000	/* lock permit */
#define	STICKY	01000	/* sticky bit */

char	*ms;
char *msp;
int rflag;
mode_t newmode(), abs(), who();
void errmsg();

extern int	optind;
extern int	errno;
extern char	*sys_errlist[];

main(argc, argv)
int argc;
char **argv;
{
	register int i;
	register char *p;
	int status = 0;

	argv++, argc--;
	while (argc > 0 && argv[0][0] == '-') {
		for (p = &argv[0][1]; *p; p++)  {
			if (*p == 'R') 
				rflag++;
			else {
				goto done;
			}
		}
		argc--, argv++;
	}

	/* 
	 * Check for sufficient arguments 
	 * or a usage error. 
	 */
done:

	if(argc < 2) 
		errmsg(2, 255, "Usage: chmod [-R] [ugoa][+-=][rwxlstugo] file ...\n", (char *)0);
	

	ms = argv[0];

	for (i = 1; i < argc; i++)
		status += dochmod(argv[i]);

	exit(status);

}

int
dochmod(name)
	char *name;
{
	static struct stat st;

	if (lstat(name, &st) < 0) {
		errmsg(2, 0, "can't access %s\n", name);
		return 1;
	}

	if ((st.st_mode & S_IFMT) == S_IFLNK) {
		if (stat(name, &st) < 0) {
			errmsg(2, 0, "can't access %s\n", name);
			return 1;
		}
	}

	if (rflag && (st.st_mode&S_IFMT) == S_IFDIR)
		return chmodr(name, st.st_mode);

	if (chmod(name, newmode(st.st_mode, name)) == -1) {
		errmsg(2, 0, "can't change %s\n", name);
		return 1;
	}

	return 0;
}

chmodr(dir, mode)
	char *dir;
	mode_t  mode;
{
	register DIR *dirp;
	register struct dirent *dp;
	struct stat st;
	char savedir[1024];
	int ecode;

	if (getcwd(savedir, 1024) == 0) 
		errmsg(2, 255, "chmod: could not getcwd %s\n", savedir);

	/*
	 * Change what we are given before doing it's contents
	 */
	if (chmod(dir, newmode(mode, dir)) < 0) {
		errmsg(2, 0, "can't change %s\n", dir);
		return (1);
	}
	if (chdir(dir) < 0) {
		errmsg(2, 0, "%s\n", sys_errlist[errno]);
		return (1);
	}
	if ((dirp = opendir(".")) == NULL) {
		errmsg(2, 0, "%s\n", sys_errlist[errno]);
		return (1);
	}
	dp = readdir(dirp);
	dp = readdir(dirp); /* read "." and ".." */
	ecode = 0;
	for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
		dochmod(dp->d_name);
	(void) closedir(dirp);
	if (chdir(savedir) < 0) {
		errmsg(2, 255, "can't change back to %s\n", savedir);
	}
	return (ecode);
}


static mode_t
newmode(nm, file)
mode_t  nm;
char  *file;
{
	/* m contains USER|GROUP|OTHER information
	   o contains +|-|= information
	   b contains rwx(slt) information  */
	mode_t m, b;
	register int o;
	register int lcheck, scheck, xcheck, goon;
	mode_t om = nm;	/* save mode for file mode incompatibility return */

	msp = ms;
	if (*msp >= '0' && *msp <= '9')
		return(abs(om));
	do {
		m = who();
		while (o = what()) {
/*
	this section processes permissions
*/
			b = 0;
			goon = 0;
			lcheck = scheck = xcheck = 0;
			switch (*msp) {
			case 'u':
				b = (nm & USER) >> 6;
				goto dup;
			case 'g':
				b = (nm & GROUP) >> 3;
				goto dup;
			case 'o':
				b = (nm & OTHER);
		    dup:
				b &= (READ|WRITE|EXEC);
				b |= (b << 3) | (b << 6);
				msp++;
				goon = 1;
			}
			while (goon == 0) switch (*msp++) {
			case 'r':
				b |= READ;
				continue;
			case 'w':
				b |= WRITE;
				continue;
			case 'x':
				b |= EXEC;
				xcheck = 1;
				continue;
			case 'l':
				b |= LOCK;
				m |= LOCK;
				lcheck = 1;
				continue;
			case 's':
				b |= SETID;
				scheck = 1;
				continue;
			case 't':
				b |= STICKY;
				continue;
			default:
				msp--;
				goon = 1;
			}

			b &= m;

			switch (o) {
			case '+':

				/* is group execution requested? */
				if ( xcheck == 1 && (b & GROUP & EXEC) == (GROUP & EXEC)) {

					/* not locking, too! */
					if ( lcheck == 1 ) {
						errmsg(1, 3, "Group execution and locking not permitted together\n", (char *)0);
					}

					/* not if the file is already lockable */
					if ( (nm & GROUP & (LOCK | EXEC)) == LOCK ) {
						errmsg(2, 0, "Group execution not permitted on %s, a lockable file\n", file);
						return(om);
					}
				}

				/* is setgid on execution requested? */
				if ( scheck == 1 && (b & GROUP & SETID) == (GROUP & SETID) ) {

					/* not locking, too! */
					if ( lcheck == 1 && (b & GROUP & EXEC) == (GROUP & EXEC) ) {
						errmsg(1, 4, "Set-group-ID and locking not permitted together\n", (char *)0);
					}

					/* not if the file is already lockable */
					if ( (nm & GROUP & (LOCK | EXEC)) == LOCK ) {
						errmsg(2, 0, "Set-group-ID not permitted on %s, a lockable file\n", file);
						return(om);
					}
				}

				/* is setid on execution requested? */
				if ( scheck == 1 ) {

					/* the corresponding execution must be requested or already set */
					if ( ((nm | b) & m & EXEC & (USER | GROUP)) != (m & EXEC & (USER | GROUP)) ) {
						errmsg(2, 0, "Execute permission required for set-ID on execution for %s\n", file);
						return(om);
					}
				}

				/* is locking requested? */
				if ( lcheck == 1 ) {

					/* not if the file has group execution set */
					/* NOTE: this also covers files with setgid */
					if ( (nm & GROUP & EXEC) == (GROUP & EXEC) ) {
						errmsg(2, 0, "Locking not permitted on %s, a group executable file\n", file);
						return(om);
					}
				}

				/* create new mode */
				nm |= b;
				break;
			case '-':

				/* don't turn off locking, unless it's on */
				if ( lcheck == 1 && scheck == 0 
				     && (nm & GROUP & (LOCK | EXEC)) != LOCK )
					b &= ~LOCK;

				/* don't turn off setgid, unless it's on */
				if ( scheck == 1 && lcheck == 0
				     && (nm & GROUP & (LOCK | EXEC)) == LOCK )
					b &= ~(GROUP & SETID);

				/* if execution is being turned off and the corresponding
				   setid is not, turn setid off, too & warn the user */
				if ( xcheck == 1 && scheck == 0
				     && ((m & GROUP) == GROUP || (m & USER) == USER)
				     && (nm & m & (SETID | EXEC)) == (m & (SETID | EXEC)) ) {
					errmsg(2, 0, "Corresponding set-ID also disabled on %s since set-ID requires execute permission\n", file);
					if ( (b & USER & SETID) != (USER & SETID)
					     && (nm & USER & (SETID | EXEC)) == (m & USER & (SETID | EXEC)) )
						b |= USER & SETID;
					if ( (b & GROUP & SETID) != (GROUP & SETID)
					     && (nm & GROUP & (SETID | EXEC)) == (m & GROUP & (SETID | EXEC)) )
						b |= GROUP & SETID;
				}

				/* create new mode */
				nm &= ~b;
				break;
			case '=':

				/* is locking requested? */
				if ( lcheck == 1 ) {

					/* not group execution, too! */
					if ( (b & GROUP & EXEC) == (GROUP & EXEC) ) {
						errmsg(1, 3, "Group execution and locking not permitted together\n", (char *)0);
					}

					/* if the file has group execution set, turn it off! */
					if ( (m & GROUP) != GROUP ) {
						nm &= ~(GROUP & EXEC);
					}
				}

/* is setid on execution requested?
   the corresponding execution must be requested, too! */
				if (scheck == 1 
				  && (b & EXEC & (USER | GROUP))  
				    != (m & EXEC & (USER | GROUP)) ) {
					errmsg(1, 2, "Execute permission required for set-ID on execution\n", (char *)0);
				}

/* 
 * The ISGID bit on directories will not be changed when 
 * the mode argument is a string with "=". 
 */

				if ((om & S_IFMT) == S_IFDIR)
					b = (b & ~S_ISGID) | (om & S_ISGID);

				/* create new mode */
				nm &= ~m;
				nm |= b;
				break;
			}
		}
	} while (*msp++ == ',');
	if (*--msp) {
		errmsg(1, 5, "invalid mode\n", (char *)0);
	}
	return(nm);
}

static mode_t
abs(mode)
	mode_t mode;
{
	register c;
	mode_t i;

	for ( i = 0; (c = *msp) >= '0' && c <= '7'; msp++)
		i = (mode_t)((i << 3) + (c - '0'));
	if (*msp)
		errmsg(1, 6, "invalid mode\n", (char *)0);

/* The ISGID bit on directories will not be changed when the mode argument is
 * octal numeric. Only "g+s" and "g-s" arguments can change ISGID bit when
 * applied to directories.
 */
	if ((mode & S_IFMT) == S_IFDIR)
		return((i & ~S_ISGID) | (mode & S_ISGID));
	return(i);
}

static mode_t
who()
{
	register mode_t m;

	m = 0;
	for (;; msp++) switch (*msp) {
	case 'u':
		m |= USER;
		continue;
	case 'g':
		m |= GROUP;
		continue;
	case 'o':
		m |= OTHER;
		continue;
	case 'a':
		m |= ALL;
		continue;
	default:
		if (m == 0)
			m = ALL;
		return m;
	}
}

static int
what()
{
	switch (*msp) {
	case '+':
	case '-':
	case '=':
		return *msp++;
	}
	return(0);
}

static void
errmsg(severity, code, format, file)
int severity, code;
char *format;
char *file;
{
	static char *msg[] = {
	"",
	"ERROR",
	"WARNING",
	""
	};

	(void) fprintf(stderr, "chmod: %s: ", msg[severity]);
	(void) fprintf(stderr, format, file);
	if (code != 0)
		exit(code);
	return;
}
