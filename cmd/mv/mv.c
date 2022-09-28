/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

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

#ident	"@(#)mv:mv.c	1.30.6.3"

/*
 * Combined mv/cp/ln command:
 *	mv file1 file2
 *	mv dir1 dir2
 *	mv file1 ... filen dir1
 */



#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<utime.h>
#include	<signal.h>
#include	<errno.h>
#include	<sys/param.h>
#include	<dirent.h>
#include	<sys/dir.h>
#include	<stdlib.h>

#define FTYPE(A)	(A.st_mode)
#define FMODE(A)	(A.st_mode)
#define	IDENTICAL(A,B)	(A.st_dev==B.st_dev && A.st_ino==B.st_ino)
#define ISBLK(A)	((A.st_mode & S_IFMT) == S_IFBLK)
#define ISCHR(A)	((A.st_mode & S_IFMT) == S_IFCHR)
#define ISDIR(A)	((A.st_mode & S_IFMT) == S_IFDIR)
#define ISFIFO(A)	((A.st_mode & S_IFMT) == S_IFIFO)
#define ISLNK(A)	((A.st_mode & S_IFMT) == S_IFLNK)
#define ISREG(A)	((A.st_mode & S_IFMT) == S_IFREG)
#define ISDEV(A)	((A.st_mode & S_IFMT) == S_IFCHR || \
                         (A.st_mode & S_IFMT) == S_IFBLK || \
                         (A.st_mode & S_IFMT) == S_IFIFO)

#define BLKSIZE	4096
#define PATHSIZE 1024
#define	DOT	"."
#define	DOTDOT	".."
#define	DELIM	'/'
#define EQ(x,y)	!strcmp(x,y)
#define	FALSE	0
#define MODEBITS (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)
#define TRUE 1

extern int errno;

char	*dname();
char	*strrchr();
extern	int errno;
extern  char *optarg;
extern	int optind, opterr;
struct stat s1, s2;
int cpy = FALSE;	
int mve = FALSE;	
int lnk = FALSE;	
char	*cmd;
int	silent = 0;
int	iflg = 0;
int	nflg = 0;
int	pflg = 0;
int	rflg = 0;
int	sflg = 0;
mode_t	fixmode = (mode_t)0;		/* cleanup mode after copy */
int	targetexists = 0;

main(argc, argv)
register char *argv[];
{
	register int c, i, r, errflg = 0;
	
	/*
	 * Determine command invoked (mv, cp, or ln) 
	 */
	
	if (cmd = strrchr(argv[0], '/'))
		++cmd;
	else
		cmd = argv[0];
	
	/*
	 * Set flags based on command.
	 */
	 
	  if (EQ(cmd, "mv"))
	  	mve = TRUE;
	  else if (EQ(cmd, "ln")) 
		   lnk = TRUE;
	  	else
		   cpy = TRUE;   /* default */
	
	/*
	 * Check for options:
	 * 	cp [-ipr] file1 [file2 ...] target
	 *	ln [-f] [-n] [-s] file1 [file2 ...] target
 	 *	ln [-f] [-n] [-s] file1 [file2 ...] target
	 *	mv [-f|i] file1 [file2 ...] target
	 *	mv [-f|i] dir1 target
	 */
	 
	if (cpy) {
		while ((c = getopt(argc, argv,"ipr")) != EOF)
			switch (c) {
                                case 'i':
                                        iflg++;
                                        break;
                                case 'p':
                                        pflg++;
                                        break;
                                case 'r':
                                        rflg++;
                                        break;
                                default:
                                        errflg++;
			}
	}
	else if (mve) {
		while ((c = getopt(argc, argv,"fis")) != EOF)
			switch(c) {
                                case 'f':
                                        silent++;
                                        break;
                                case 'i':
                                        iflg++;
                                        break;
                                default:
                                        errflg++;
			}
	}
	else {
		while ((c = getopt(argc, argv,"fns")) != EOF)
			switch(c) {
                                case 'f':
                                        silent++;
                                        break;
				case 'n': nflg++;
					break;
                                case 's':
                                        sflg++;
                                        break;
			default:
                                errflg++;
			}
	}

	/* For BSD compatibility allow - to delimit the end of
	 * options for mv.
	 */
	if (mve && optind < argc && !strcmp(argv[optind], "-"))
		optind++;
	
	/*
	 * Check for sufficient arguments 
	 * or a usage error.
	 */

	argc -= optind;	 
	argv  = &argv[optind];
	
	if (argc < 2) {
		fprintf(stderr,"%s: Insufficient arguments (%d)\n",cmd,argc);
		usage();
	}
	
	if (errflg != 0)
		usage();
	
	/*
	 * If there is more than a source and target,
	 * the last argument (the target) must be a directory
	 * which really exists.
	 */
	 
	if (argc > 2) {
		if (stat(argv[argc-1], &s2) < 0) {
			fprintf(stderr, "%s: %s not found\n", cmd, argv[argc-1]);
			exit(2);
		}
		
		if (!ISDIR(s2)) {
			fprintf(stderr,"%s: Target must be directory\n",cmd);
			usage();
		}
	}	
	
	/* 
	 * While target has trailing 
	 * DELIM (/), remove them (unless only "/")
	 */
	while (((c = strlen(argv[argc-1])) > 1)
	    &&  (argv[argc-1][c-1] == DELIM))
		 argv[argc-1][c-1]=NULL;

	/*
	 * Perform a multiple argument mv|cp|ln by
	 * multiple invocations of move().
	 */
	 
	r = 0;
	for (i=0; i<argc-1; i++)
		r += move(argv[i], argv[argc-1]);
	
	/* 
	 * Show errors by nonzero exit code.
	 */
	 
	exit(r?2:0);
}

move(source, target)
char *source, *target;
{
	register last;

	/* 
	 * While source has trailing 
	 * DELIM (/), remove them (unless only "/")
	 */

	while (((last = strlen(source)) > 1)
	    &&  (source[last-1] == DELIM))
		 source[last-1]=NULL;
	
	if (lnk)
		return(lnkfil(source, target));
	else
		return(cpymve(source, target));
}


lnkfil(source, target)
char *source, *target;
{
	char	*buf = (char *)NULL;

 	if (sflg) {

		/*
		 * If target is a directory make complete
		 * name of the new symbolic link within that
		 * directory.
		 */

 		if ((stat(target, &s2) >= 0) && ISDIR(s2)) {
			if ((buf = (char *)malloc(strlen(target) + strlen(dname(source)) + 4)) == NULL) {
				fprintf(stderr,"%s: Insufficient memory to %s %s\n ",cmd,cmd,source);
				exit(3);
			}
			sprintf(buf, "%s/%s", target, dname(source));
			target = buf;
 		}

		/*
		 * Create a symbolic link to the source.
		 */

 		if (symlink(source, target) < 0) {
			fprintf(stderr, "%s: cannot create %s\n", cmd, target);
 			perror(cmd);
			if (buf != NULL)
				free(buf);
 			return(1);
 		}
		if (buf != NULL)
			free(buf);
 		return(0);
 	}

        if (chkfiles(source, &target))
		return(1);


        /*
         * Make sure source file is not a directory,
	 * we can't link directories...
	 */

	if (ISDIR(s1)) {
		fprintf(stderr,"%s: <%s> directory\n", cmd, source);
		return(1);
	}

        /*
	 * hard link, call link() and return.
	 */
	
        if(link(source, target) < 0) {
                if(errno == EXDEV)
                	fprintf(stderr, "%s: different file system\n", cmd);
                else
                     	fprintf(stderr, "%s: errno: %d no permission for %s\n", cmd, errno, target);
                if(buf != NULL)
                        free(buf);
                return(1);
	}
        else{
                if(buf != NULL)
                        free(buf);
                 return(0);
	}
}


cpymve(source, target)
char *source, *target;
{
 	int n;
	char buf[8192];
	int fi, fo;
	mode_t savemode;

        if (n = chkfiles(source, &target))
		return(n);

        /*
         * If it's a recursive copy and source
	 * is a directory, then call rcopy.
	 */
	if (cpy) {
                if (ISDIR(s1)) {
                        if (!rflg) {
                                 fprintf(stderr,
                                 "cp: <%s> directory\n",
                                            source);
                                 return (1);
			}

			if (stat(target, &s2) < 0) {
                                if (mkdir(target,
                                    (s1.st_mode & MODEBITS) | S_IRWXU) < 0) {
                                        fprintf(stderr,"%s: ", cmd);
                                        perror(target);
                                        return (1);
                                }
                                fixmode = s1.st_mode;
			} else if (!(ISDIR(s2))) {
                                  fprintf(stderr, "cp: %s: Not a directory.\n",
                                            target);
                                  return (1);
			} else if (pflg)
                                fixmode = s1.st_mode;
			n = rcopy(source, target);
			if (fixmode != (mode_t)0)
                                (void) chmod(target, fixmode & MODEBITS);
			if (pflg)
				setimes(target);
			return (n);
		}
		else
			goto copy;
	}

	if (mve) {
		if (rename(source, target) >= 0)
			return (0);
		if (errno != EXDEV) {
			if (ISDIR(s1)) {
				fprintf(stderr, "%s: <%s> directory\n", cmd, source);
				return(1);
			}
			fprintf(stderr,"%s: cannot rename %s\n", cmd, source); 
			perror(cmd);
			return(1);
		}
		if (ISDIR(s1)) {
			fprintf(stderr, "mv: can't mv directories across file systems\n");
			return (1);
		}

		/*
		 * File can't be renamed, try to recreate the symbolic
		 * link or special device, or copy the file wholesale
		 * between file systems.
		 */
		if (ISLNK(s1)) {
			register m;
			register mode_t md;
			char symln[MAXPATHLEN + 1];

			if (targetexists && unlink(target) < 0) {
				fprintf(stderr, "%s: cannot unlink %s\n", cmd, target);
				perror(cmd);
				return (1);
			}

			m = readlink(source, symln, sizeof (symln) - 1);
			if (m < 0) {
				Perror(source);
				return (1);
			}
			symln[m] = '\0';

			md = umask(~(s1.st_mode & MODEBITS));
			if (symlink(symln, target) < 0) {
				Perror(target);
				return (1);
			}
			(void) umask(md);
			goto cleanup;
		}
		if (ISDEV(s1)) {

			if (targetexists && unlink(target) < 0) {
				fprintf(stderr, "%s: cannot unlink %s\n", cmd, target);
				perror(cmd);
				return (1);
			}

			if (mknod(target, s1.st_mode, s1.st_rdev) < 0) {
				Perror(target);
				return (1);
			}

			setimes(target);
			goto cleanup;
		}

		if (ISREG(s1))
		{
			struct stat spd;        /* stat of source's parent */
			int uid;                /* real uid */

			if (accs_parent(source, 2, &spd) == -1)
				goto unlink_fail;

			/*
			* If sticky bit set on source's parent, then move
			* only when : superuser, source's owner, parent's
			* owner, or source is writable. Otherwise, we
			* won't be able to unlink source later and will be
			* left with two links and an error exit from mv.
			*/
			if (spd.st_mode&S_ISVTX && (uid=getuid()) != 0 &&
			  s1.st_uid != uid && spd.st_uid != uid &&
			  access(source, 2)<0) {
				fprintf(stderr,"%s: cannot unlink %s\n",
							 cmd, source);
				perror(cmd);
				return (1);
			}
			if (targetexists && unlink(target) < 0) {
				fprintf(stderr, "%s: cannot unlink %s\n", cmd, target);
				perror(cmd);
				return (1);
			}


copy:
			fi = open(source, O_RDONLY);
			if (fi < 0) {
				fprintf(stderr,"%s: cannot open %s\n", cmd, source);
				perror(source);
				return (1);
			}

			fo = creat(target, s1.st_mode & MODEBITS);
			if (fo < 0) {
				fprintf(stderr,"%s: cannot create %s\n", cmd,target); 
				perror(cmd);
				close(fi);
				return (1);
			}

			/*
			 * If we created a target, set its permissions
			 * to the source before any copying so that any
		 	 * partially copied file will have the source's 
			 * permissions (at most) or umask permissions
			 * whichever is the most restrictive.
			 */

			if (!targetexists || pflg)
				chmod(target, FMODE(s1));

			for (;;) {
				n = read(fi, buf, sizeof buf);
				if (n == 0) {
					break;
				} else if (n < 0) {
					Perror2(source, "read");
					close(fi);
					close(fo);
					if (ISREG(s2))
						unlink(target);
					return (1);
				} else if (write(fo, buf, n) != n) {
					Perror2(target, "write");
					close(fi);
					close(fo);
					if (ISREG(s2))
						unlink(target);
					return (1);
				}
			}

			close(fi);
			if (close(fo) < 0) {
				Perror2(target, "write");
				return (1);
			}
			if (mve || (cpy && pflg)) 
				setimes(target);
			if (cpy)
				return(0);
			goto cleanup;
		}
		fprintf(stderr,"%s: unknown file type %o", source, s1.st_mode);
		return (1);

cleanup:
		if (unlink(source) < 0) {
			(void) unlink(target);
unlink_fail:
			fprintf(stderr, "%s: cannot unlink %s\n", cmd, source);
			perror(cmd);
			return (1);
		}
		return (0);
	}
}

chkfiles(source, to)
char *source, **to;
{
	char	*buf = (char *)NULL;
	int	(*statf)() = cpy ? stat : lstat;
	int	n;
	char    *target = *to;

        /*
         * Make sure source file exists.
	 */

        if ((*statf)(source, &s1) < 0) {
		fprintf(stderr, "%s: cannot access %s\n", cmd, source);
		return(1);
	}
	
	/*
	 * If stat fails, then the target doesn't exist,
	 * we will create a new target with default file type of regular.
 	 */	
	
	FTYPE(s2) = S_IFREG;
	targetexists = 0;
	if ((*statf)(target, &s2) >= 0) {
		if(ISLNK(s2))
			stat(target, &s2);
		/*
		 * If target is a directory,
		 * make complete name of new file
		 * within that directory.
		 */
		if (ISDIR(s2)) {
			if ((buf = (char *)malloc(strlen(target) + strlen(dname(source)) + 4)) == NULL) {
				fprintf(stderr,"%s: Insufficient memory to %s %s\n ",cmd,cmd,source);
				exit(3);
			}
			sprintf(buf, "%s/%s", target, dname(source));
			*to = target = buf;
		}
		
		/*
		 * If filenames for the source and target are
		 * the same and the inodes are the same, it is
		 * an error.
		 */
	
		if ((*statf)(target, &s2) >= 0) {
			targetexists++;
			if (IDENTICAL(s1,s2)) {
				fprintf(stderr, "%s: %s and %s are identical\n",
				                        cmd, source, target);
				if (buf != NULL)
		                        free(buf);
				return(1);
			}
			if (lnk && nflg && !silent) {
				fprintf(stderr,"%s: %s: File exists\n", cmd, target);
				return(1);
			}
			
			/*
			 * If the user does not have access to
			 * the target, ask ----if it is not
			 * silent and user invoked command
			 * interactively.
			 */
	

			if (iflg && !silent && isatty(fileno(stdin))) {
				fprintf(stderr,"%s: overwrite %s? ", cmd, target);
				if (getresp()) {
					if (buf != NULL)
						free(buf);
					return(1);
				}
			}

			if (!cpy && (access(target, 2) < 0) &&
				!silent && isatty(fileno(stdin)) &&
				!ISLNK(s2)) {

				fprintf(stderr,"%s: %s: %o mode? ", cmd, target,
						FMODE(s2) & MODEBITS);
				if (getresp()) {
					if (buf != NULL)
						free(buf);
					return(1);
				}
			}

			if(lnk && unlink(target) < 0) {
				fprintf(stderr, "%s: cannot unlink %s\n", cmd,
								 target);
				perror(cmd);
				return(1);
			}
		}
	}
	return(0);
}

rcopy(from, to)
char *from, *to;
{
	DIR *fold = opendir(from);
	struct dirent *dp;
	struct stat statb;
	int errs = 0;
	char fromname[MAXPATHLEN + 1];

	if (fold == 0 || (pflg && fstat(fold->dd_fd, &statb) < 0)) {
		Perror(from);
		return (1);
	}
	for (;;) {
		dp = readdir(fold);
		if (dp == 0) {
			(void) closedir(fold);
			if (pflg)
                                return (setimes(to) + errs);
			return (errs);
		}
		if (dp->d_ino == 0)
			continue;
		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;
		if (strlen(from)+1+strlen(dp->d_name) >= sizeof fromname - 1) {
			(void) fprintf(stderr, "cp: %s/%s: Name too long\n",
                            from, dp->d_name);
			errs++;
			continue;
		}
		(void) sprintf(fromname, "%s/%s", from, dp->d_name);
		errs += move(fromname, to);
	}
}

/* In addition to checking write access on parent dir, do a stat on it */
accs_parent(name, amode, sdirp)
register char *name;
register int amode;
register struct stat *sdirp;
{
	register c;
	register char *p, *q;
	char *buf;

	/*
	 * Allocate a buffer for parent.
	 */
	
	if ((buf = malloc(strlen(name) + 2)) == NULL) {
		fprintf(stderr,"%s: Insufficient memory space.\n",cmd);
		exit(3);
	}
	p = q = buf;
	
	/* 
	 * Copy name into buf and set 'q' to point to the last
	 * delimiter within name.
	 */
	 
	while (c = *p++ = *name++)
		if (c == DELIM)
			q = p-1;
	
	/*
	 * If the name had no '\' or was "\" then leave it alone,
	 * otherwise null the name at the last delimiter.
	 */
	 
	if (q == buf && *q == DELIM)
		q++;
	*q = NULL;
	
	/*
	 * Find the access of the parent.
	 * If no parent specified, use dot.
	 */
	 
	if ((c = access(buf[0] ? buf : DOT,amode)) == -1) {
	/* No write access to directory : no need to check sticky bit */
		free(buf);
		return(c);
	}

	/*
	 * Stat the parent : needed for sticky bit check.
	 * If stat fails, move() should fail the access, 
	 * since we cannot proceed anyway.
	 */
	c = stat(buf[0] ? buf : DOT, sdirp);
	free(buf);
	return(c);
}

char *
dname(name)
register char *name;
{
	register char *p;

	/* 
	 * Return just the file name given the complete path.
	 * Like basename(1).
	 */
	 
	p = name;
	
	/*
	 * While there are characters left,
	 * set name to start after last
	 * delimiter.
	 */
	 
	while (*p)
		if (*p++ == DELIM && *p)
			name = p;
	return(name);
}

getresp()
{
	register  c, i;

        /* Get response from user. Based on
         * first character, make decision.
         * Discard rest of line.
         */ 
        i = c = getchar();
        while (c != '\n' && c != EOF)
                c = getchar();
     	if (i != 'y')
		return(1);
	return(0); 
}                                        

usage()
{
	register char *opt;
	
	/*
	 * Display usage message.
	 */
	 
 	opt = cpy ? " [-i] [-p]" : lnk ? " [-f] [-n] [-s]" : " [-f] [-i]";
	fprintf(stderr, "Usage: %s%s f1 f2\n       %s%s f1 ... fn d1\n", 
		cmd, opt, cmd, opt);
	if(mve)
		fprintf(stderr, "       mv [-f] [-i] d1 d2\n");
	else if (lnk)
		fprintf(stderr, "       ln [-f] [-n] [-s] d1 d2\n");
	else if (cpy)
		fprintf(stderr, "       cp [-i] [-p] [-r] d1 d2\n");
	exit(2);
}

setimes(to)
char *to;
{
 	struct utimbuf *times;

        times = (struct utimbuf *)malloc((unsigned)sizeof(struct utimbuf));

        times->actime = s1.st_atime;
	times->modtime = s1.st_mtime;
	utime(to, times);

		
}



Perror(s)
char *s;
{
	char buf[MAXPATHLEN + 10];
	
	sprintf(buf, "%s: %s", cmd, s);
	perror(buf);
}

Perror2(s1, s2)
char *s1, *s2;
{
	char buf[MAXPATHLEN + 20];

	sprintf(buf, "%s: %s: %s", cmd, s1, s2);
	perror(buf);
}
