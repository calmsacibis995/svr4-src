/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)csh:sh.exec.c	1.3.4.1"

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
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley Software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include "sh.h"
#include <sys/fs/ufs_fsdir.h>
#include "dir2.h"
/* #include <sys/dir.h> */
#include "sh.tconst.h"

#ifdef VPIX
static int dospath();
static void dosexec();
extern char *getenv();
#endif /* VPIX */

/*
 * C shell
 */

/*
 * System level search and execute of a command.
 * We look in each directory for the specified command name.
 * If the name contains a '/' then we execute only the full path name.
 * If there is no search path then we execute only full path names.
 */

/* 
 * As we search for the command we note the first non-trivial error
 * message for presentation to the user.  This allows us often
 * to show that a file has the wrong mode/no access when the file
 * is not in the last component of the search path, so we must
 * go on after first detecting the error.
 */
char *exerr;			/* Execution error message */
tchar *expath;			/* Path for exerr */

/*
 * Xhash is an array of HSHSIZ bits (HSHSIZ / 8 chars), which are used
 * to hash execs.  If it is allocated (havhash true), then to tell
 * whether ``name'' is (possibly) present in the i'th component
 * of the variable path, you look at the bit in xhash indexed by
 * hash(hashname("name"), i).  This is setup automatically
 * after .login is executed, and recomputed whenever ``path'' is
 * changed.
 * The two part hash function is designed to let texec() call the
 * more expensive hashname() only once and the simple hash() several
 * times (once for each path component checked).
 * Byte size is assumed to be 8.
 */
#define	HSHSIZ		8192			/* 1k bytes */
#define HSHMASK		(HSHSIZ - 1)
#define HSHMUL		243
char xhash[HSHSIZ / 8];
#define hash(a, b)	((a) * HSHMUL + (b) & HSHMASK)
#define bit(h, b)	((h)[(b) >> 3] & 1 << ((b) & 7))	/* bit test */
#define bis(h, b)	((h)[(b) >> 3] |= 1 << ((b) & 7))	/* bit set */
#ifdef VFORK
int	hits, misses;
#endif

extern DIR *opendir_();

/* Dummy search path for just absolute search when no path */
tchar *justabs[] =	{ S_ /* "" */, 0 };

doexec(t)
	register struct command *t;
{
	tchar *sav;
	register tchar *dp, **pv, **av;
	register struct varent *v;
	bool slash = any('/', t->t_dcom[0]);
	int hashval, i;
	tchar *blk[2];
#ifdef TRACE
	tprintf("TRACE- doexec()\n");
#endif

	/*
	 * Glob the command name.  If this does anything, then we
	 * will execute the command only relative to ".".  One special
	 * case: if there is no PATH, then we execute only commands
	 * which start with '/'.
	 */
	dp = globone(t->t_dcom[0]);
	sav = t->t_dcom[0];
	exerr = 0; expath = t->t_dcom[0] = dp;
	xfree(sav);
	v = adrof(S_path /*"path"*/);
	if (v == 0 && expath[0] != '/')
		pexerr();
	slash |= gflag;

	/*
	 * Glob the argument list, if necessary.
	 * Otherwise trim off the quote bits.
	 */
	gflag = 0; av = &t->t_dcom[1];
	tglob(av);
	if (gflag) {
		av = glob(av);
		if (av == 0)
			error("No match");
	}
	blk[0] = t->t_dcom[0];
	blk[1] = 0;
	av = blkspl(blk, av);
#ifdef VFORK
	Vav = av;
#endif
	trim(av);

	xechoit(av);		/* Echo command if -x */
	/*
	 * Since all internal file descriptors are set to close on exec,
	 * we don't need to close them explicitly here.  Just reorient
	 * ourselves for error messages.
	 */
	SHIN = 0; SHOUT = 1; SHDIAG = 2; OLDSTD = 0;

	/*
	 * We must do this AFTER any possible forking (like `foo`
	 * in glob) so that this shell can still do subprocesses.
	 */
	(void) sigsetmask(0);

	/*
	 * If no path, no words in path, or a / in the filename
	 * then restrict the command search.
	 */
	if (v == 0 || v->vec[0] == 0 || slash)
		pv = justabs;
	else
		pv = v->vec;
	sav = strspl(S_SLASH /* "/" */, *av);		/* / command name for postpending */
#ifdef VFORK
	Vsav = sav;
#endif
	if (havhash)
		hashval = hashname(*av);
	i = 0;
#ifdef VFORK
	hits++;
#endif
	do {
/*
 * The following hash does not seem to work
 * under SysV, commented out for the time
 * being
 *	if (!slash && pv[0][0] == '/' && havhash) {
 *		hashval1 = hash(hashval, i);
 *		if (!bit(xhash, hashval1))
 *			goto cont;
 *	}
 */

#ifdef VPIX
		if (dospath (tstostr(NULL,*pv))) {
		    char **dossfx;
		    tchar *dossimple, *dosfull, *dossav;
		    static char *suffixes[] =
			    { ".com", ".exe", ".bat", NULL };

		    for (dossfx = suffixes; *dossfx; dossfx++) {
			dossimple = strspl (*av, strtots(NOSTR,*dossfx));
			if (slash || pv[0][0] == 0 || eq (pv[0], "."))
				dosexec (dossimple, av);
			else if ((pv[0][0] != '/') || ((havhash))) {
					dossav = strspl (S_SLASH /* "/" */, dossimple);
					dosfull = strspl (*pv, dossav);
					dosexec (dosfull, av);
					xfree (dossav);
					xfree (dosfull);
				}
			xfree (dossimple);
		    }
		}
#endif /* VPIX */
		if (pv[0][0] == 0 || eq(pv[0], S_DOT/*"."*/)) {	/* don't make ./xxx */
			texec(*av, av);
		} else {
			dp = strspl(*pv, sav);
#ifdef VFORK
			Vdp = dp;
#endif
			texec(dp, av);
#ifdef VFORK
			Vdp = 0;
#endif
			xfree(dp);
		}
#ifdef VFORK
		misses++;
#endif
cont:
		pv++;
		i++;
	} while (*pv);
#ifdef VFORK
	hits--;
#endif
#ifdef VFORK
	Vsav = 0;
	Vav = 0;
#endif
	xfree(sav);
	xfree( (char *)av);
	pexerr();
}

#ifdef VPIX
/* see if path is in DOSPATH */
static int
dospath(path)
char *path;
{
	register char   *scanp, *scandp;
#define COLON   ':'

	if (path == 0)
		return 0;
	if ((scandp = getenv ("DOSPATH")) == NULL)
		return 0;
	for (;;) {
		scanp = path;
		for (;;) {
			if ((*scanp == 0 || *scanp == COLON) &&
					(*scandp == 0 || *scandp == COLON))
				return 1;
			if (*scanp != *scandp)
				break;
			scanp++;
			scandp++;
		}
		while (*scandp && *scandp != COLON)
			scandp++;
		if (*scandp == 0)
			return 0;
		scandp++;        /* scanp points after COLON */
	}
}

/* execute a DOS program by running vpix with adjusted args */
static void
dosexec(f, av)
tchar *f;  /* f = full or relative path of file */
tchar **av;  /* av = arg vector */
{
	register tchar **newav, **pv;
	register tchar *dp;
	char *real_name;
	tchar *sav;
	register struct varent *v;
	int i, hashval;
	static tchar *vpix;

	real_name = tstostr(NULL,f);
	vpix = strtots(NOSTR,"vpix");
	/* make sure cache hit was for this file and we can read it */
	if (access (real_name, 4) < 0) 
		return;

	if (chk_access (real_name, S_IEXEC) == 3) 
		error ("%s: cannot execute", real_name);

	/* set up the new argument list */
	for (i = 0; av[i]; i++)
		;        /* count args */
	newav = (tchar **)calloc (i + 3, sizeof (tchar *));
	newav[i + 2] = 0;
	while (--i > 0)
		newav[i + 2] = av[i];
	newav[2] = f;
	newav[1] = strtots(NOSTR, "-c");
	newav[0] = vpix;

	/* prepare to look for "vpix" program */
	v = adrof (S_path /* "path" */);
	if (v == 0 || v->vec[0] == 0)
		pv = justabs;
	else
		pv = v->vec;
	sav = strspl (S_SLASH /* "/" */, vpix);
	i = 0;
	if (havhash)
		hashval = xhash[hash (hashname(vpix), i)];

	/* look for "vpix" program */
	do {
		if (pv[0] == NULL || eq (pv[0], "."))
			texec (*newav, newav);
		else {
			dp = strspl (*pv, sav);
			texec (dp, newav);
			xfree (dp);
		}
  cont:
		pv++;
		i++;
	} while (*pv);
	xfree (sav);
	xfree (newav);

	expath = vpix;          /* found DOS program but not vpix */
}
#endif /* VPIX */

pexerr()
{

#ifdef TRACE
	tprintf("TRACE- pexerr()\n");
#endif
	/* Couldn't find the damn thing */
	setname(expath);
	/* xfree(expath); */
	if (exerr)
		bferr(exerr);
	bferr("Command not found");
}

/*
 * Execute command f, arg list t.
 * Record error message if not found.
 * Also do shell scripts here.
 */
texec(f, t)
	tchar *f;
	register tchar **t;
{
	register struct varent *v;
	register tchar **vp;
	extern char *sys_errlist[];
	tchar *lastsh[2];
	
	/* for execv interface */
	char *f_str;		/* for the first argument */
	tchar **ot;		/* save the original pointer */
	char **t_str, **ot_str;	/* for the second argunebt */
	int len;		/* length of the second argument */
	int i;			/* work */

#ifdef TRACE
	tprintf("TRACE- texec()\n");
#endif

	/* ============================ */
	/*  For tchar-> char interface */
	/* ============================ */
	/*execv(f, t);*/
	ot = t;				/* save the original pointer */
	f_str = tstostr(NULL, f);	/* allocate the first argument */
	len = blklen(t);		/* get the length */
	ot_str = t_str = (char **)calloc((unsigned int)(len+1), sizeof (char **));
	for (i = 0; i < len; i++) {
		*ot_str++ = tstostr(NULL, *ot++);
	}
	*ot_str = 0;
	execv(f_str, t_str);	/* exec the command */

	/* failed, free the area */
	xfree(f_str);			/* Free the first */
	ot_str = t_str;
	for (i = 0; i < len; i++) {	/* Free the second list */
		xfree(*ot_str++);
	}
	xfree((char *)t_str);			/* Free the holder */
	/* ============================ */
	/*  END For tchar-> char interface */
	/* ============================ */

	switch (errno) {

	case ENOEXEC:
		/* check that this is not a binary file */
		{       
			register int ff = open_(f, 0);
			tchar ch;

			if (ff != -1 && read_(ff, &ch, 1) == 1 && !isprint(ch)
			       && !isspace(ch)) {
				printf("Cannot execute binary file.\n");
				Perror(f);
				(void) close(ff);
				return;
			} 
			(void) close(ff);
		} 
		/*
		 * If there is an alias for shell, then
		 * put the words of the alias in front of the
		 * argument list replacing the command name.
		 * Note no interpretation of the words at this point.
		 */
		v = adrof1(S_shell /*"shell"*/, &aliases);
		if (v == 0) {
#ifdef OTHERSH
			register int ff = open_(f, 0);
			tchar ch;
#endif

			vp = lastsh;
			vp[0] = adrof(S_shell /*"shell"*/) ? value(S_shell /*"shell"*/) : S_SHELLPATH/*SHELLPATH*/;
			vp[1] =  (tchar *) NULL;
#ifdef OTHERSH
			if (ff != -1 && read_(ff, &ch, 1) == 1 && ch != '#')
				vp[0] = S_OTHERSH/*OTHERSH*/;
			(void) close(ff);
#endif
		} else
			vp = v->vec;
		t[0] = f;
		t = blkspl(vp, t);		/* Splice up the new arglst */
		f = *t;

		/* ============================ */
		/*  For tchar-> char interface */
		/* ============================ */
		ot = t;				/* save the original pointer */
		f_str = tstostr(NOSTR, f);	/* allocate the first argument */
		len = blklen(t);		/* get the length */
		ot_str = t_str = (char **)calloc((unsigned int)(len+1), sizeof (char **));
		for (i = 0; i < len; i++) {
			*ot_str++ = tstostr(NULL, *ot++);
		}
		*ot_str = 0;
		execv(f_str, t_str);	/* exec the command */

		/* failed, free the area */
		xfree(f_str);			/* Free the first */
		ot_str = t_str;
		for (i = 0; i < len; i++) {	/* Free the second list */
			xfree(*ot_str++);
		}
		xfree(t_str);			/* Free the holder */
		/* ============================ */
		/* execv(f, t); */
		/* ============================ */
		xfree( (char *)t);

		/* The sky is falling, the sky is falling! */

	case ENOMEM:
		Perror(f);

	case ENOENT:
		break;

	default:
		if (exerr == 0) {
			exerr = sys_errlist[errno];
			expath = savestr(f);
		}
	}
}

/*ARGSUSED*/
execash(t, kp)
	tchar **t;
	register struct command *kp;
{
#ifdef TRACE
	tprintf("TRACE- execash()\n");
#endif

	rechist();
	(void) signal(SIGINT, parintr);
	(void) signal(SIGQUIT, parintr);
	(void) signal(SIGTERM, parterm);	/* if doexec loses, screw */
	lshift(kp->t_dcom, 1);
	exiterr++;
	doexec(kp);
	/*NOTREACHED*/
}

xechoit(t)
	tchar **t;
{
#ifdef TRACE
	tprintf("TRACE- xechoit()\n");
#endif

	if (adrof(S_echo /*"echo"*/)) {
		flush();
		haderr = 1;
		blkpr(t), putchar('\n');
		haderr = 0;
	}
}

dohash()
{
	struct stat stb;
	DIR *dirp;
	register struct direct *dp;
	register int cnt;
	int i = 0;
	struct varent *v = adrof(S_path /*"path"*/);
	tchar **pv;
	int hashval;
	tchar curdir_[MAXNAMLEN+1];

#ifdef TRACE
	tprintf("TRACE- dohash()\n");
#endif
	havhash = 1;
	for (cnt = 0; cnt < sizeof(xhash)/sizeof(*xhash); cnt++)
		xhash[cnt] = 0;
	if (v == 0)
		return;
	for (pv = v->vec; *pv; pv++, i++) {
		if (pv[0][0] != '/')
			continue;
		dirp = opendir_(*pv);
		if (dirp == NULL)
			continue;
		if (fstat(dirp->dd_fd, &stb) < 0 || !isdir(stb)) {
			closedir(dirp);
			continue;
		}
		while ((dp = readdir(dirp)) != NULL) {
			if (dp->d_ino == 0)
				continue;
			if (dp->d_name[0] == '.' &&
			    (dp->d_name[1] == '\0' ||
			     dp->d_name[1] == '.' && dp->d_name[2] == '\0'))
				continue;
			hashval = hash(hashname(strtots(curdir_,dp->d_name)), i);
			bis(xhash, hashval);
		}
		closedir(dirp);
	}
}

dounhash()
{

#ifdef TRACE
	tprintf("TRACE- dounhash()\n");
#endif
	havhash = 0;
}

#ifdef VFORK
hashstat()
{
#ifdef TRACE
	tprintf("TRACE- hashstat_()\n");
#endif

	if (hits+misses)
		printf("%d hits, %d misses, %d%%\n",
			hits, misses, 100 * hits / (hits + misses));
}
#endif

/*
 * Hash a command name.
 */
hashname(cp)
	register tchar *cp;
{
	register long h = 0;

#ifdef TRACE
	tprintf("TRACE- hashname()\n");
#endif
	while (*cp)
		h = hash(h, *cp++);
	return ((int) h);
}



#ifdef VPIX
chk_access(name, mode)
register unsigned char	*name;
mode_t mode;
{	
	static int flag;
	static uid_t euid; 
	struct stat statb;
	mode_t ftype;
	
	if(flag == 0) {
		euid = geteuid();
		flag = 1;
	}
	ftype = statb.st_mode & S_IFMT;
	if (stat((char *)name, &statb) == 0) {
		ftype = statb.st_mode & S_IFMT;
		if(mode == S_IEXEC && 1 && ftype != S_IFREG)
			return(2);
		if(access((char *)name, 010|(mode>>6)) == 0) {
			if(euid == 0) {
				if (ftype != S_IFREG || mode != S_IEXEC)
					return(0);
		    		/* root can execute file as long as it has execute 
			   	permission for someone */
				if (statb.st_mode & (S_IEXEC|(S_IEXEC>>3)|(S_IEXEC>>6)))
					return(0);
				return(3);
			}
			return(0);
		}
	}
	return(errno == EACCES ? 3 : 1);
}
#endif
