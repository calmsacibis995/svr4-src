/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)copy:copy.c	1.1.2.1"

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */


/***	copy
 *
 *	copy [-a] [-ad] [-l] [-n] [-o] [-m] [-r] [-v] sources destination
 *
 *	MODIFICATION HISTORY
 *	M001	10-OCT-82
 *	-Installed ability to copy IFNAM (semaphore) files
 *	 Let this Moo cover previous attempt too.
 *	 Big kludge here -- had to use close on semaphore (ugh!)
 *	-Fixed the copying of groups of device special files
 *	 eliminating false error messages when umask is not zero
 *	-Fixed the processing of command-line arguments so that
 *	 `copy -m -n -o -r...' is now the same as `copy -mnor...'
 *	M002	18 Apr 83	3.0 upgrade
 *	- Added recognition of S_IFIFO files, conditional on S_IFIFO.
 *	  Added recognition of name space files, conditional on S_IFNAM.
 *	  Effectively removed recognition of S_MP* files, conditional on
 *	  S_MP* (which should not exist).
 *	- Changed logic in sf().
 *	M003	25 Apr 84	
 *	- If a regular file is specified on the command line and the -a
 *	  option is specified, then the user will now be prompted to copy
 *	  the file.
 *	M004	02 Sep 85	
 *	- Changed buffer size from 512 to BUFSIZ.  Added meaningful return
 *	  values.  Now returns !0 if any of the file copies failed. Unnoted.
 *	M005	23 May 85	
 *	- Complain about illegal options on command line.  (They used to
 *	  be ignored.)  Also made exit status return non-zero.
 */

#include	<stdio.h>
#include	<errno.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/fcntl.h>
#include 	<utime.h>
#include	<unistd.h>
#define	DIRSIZ		30	/* in case somebody likes to type long names */
#include	<dirent.h>
#define	LSYSDIRENT	25
#define	LSYSDIRNAM	14
#define	ERROR		1
#define	OK		0

extern	int	errno;


struct	stat	stat1, stat2;

mode_t	type1, type2;

#define	NAMSZ1	256
#define	NAMSZ2	NAMSZ1 + 15
char	name1[ NAMSZ1 ],name2[ NAMSZ2];	/* buffer -> for source and destination names */
char	*name1l, *name2l;	/* points to the last char '\0' in name1 and name2 */
char	cmdline[ NAMSZ2+6 ];
char *usage = "Usage: copy [-n] [-l] [-a[d]] [-m] [-o] [-r] [-v] src ... [dst]\n";

struct	dirent	*buffer; 

int	askflag;	/* ask before doing anything */
int	askdirflag;	/* ask before doing anything to directories */
int	linkflag;	/* links are used where possible */
int	newonlyflag;	/* override will not be done */
int	ownerflag;	/* if super user, will change owner on created files */
int	utimeflag;	/* will change times on created files */
int	recursiveflag;	/* will branch down directory structure */
int	verboseflag;	/* prints lots of information */
uid_t	suflag;		/* 0 if super user otherwise 1 */
mode_t	Vumask;		/* The Value of umask, which is unchanged 	*/

main(argc, argv)
int	argc;
char	**argv;
{
	register int i;
	register char *s, *t;
	char *name2ptr;	/* remembers loc of last arg */
	int k = 0;
	int retval = OK;

	suflag = getuid();	/* set suflag id */
	
	Vumask = umask(0);		/* so flags won't be disturbed */
	{mode_t sink;		/* Keep lint happy without wasting space */
	sink = umask (Vumask);	/* Restore the umask to value set by parent */
	}
	Vumask = ~Vumask;	/* Compute the complement just once	*/
	
	buffer = (struct dirent *)malloc((unsigned)sizeof(struct dirent));

	for(i=1; i<argc; i++) {
	    if (argv[i][0] == '-')  {
		register char	*flagc;
		for (flagc=argv[i] + 1; *flagc != '\0'; flagc++) {
					    /* M001 each char in flag*/
		    switch (*flagc) {
		    case 'a':
			if (argv[i][2] == 'd') {
			    flagc++;	/* M001	*/
			    askdirflag = 1;
			    recursiveflag = 1;
			} else
			    askflag = askdirflag = 1;
			break;
		    case 'l':
			linkflag = 1;
			break;
		    case 'n':
			newonlyflag = 1;
			break;
		    case 'o':
			ownerflag = 1;
/*
/*			if (suflag == 0) ownerflag = 1;
/*			else
/*			fprintf(stderr, "copy: only super user may set -o flag\n");
/* */
			break;
		    case 'm':
			utimeflag = 1;
			break;
		    case 'r':
			recursiveflag = 1;
			break;
		    case 'v':
			verboseflag = 1;
			break;
		    default:		/* M004 begin */
			fprintf(stderr, "Bad option - %c\n", *flagc);
			fprintf(stderr, "%s", usage);
			exit(1);	/* M004 end */
		    }	/* end switch on flag character			*/
		}	/* end for each character in flag		*/
	    }	else	/* Arg does not begin with a hyphen		*/
		{
		    name2ptr = argv[k++] = argv[i];/* copy the arg	*/
		}
	}     /* end for each argument					*/

	if (--k < 1)	/* k now is the number of sources */ {
	    if (k) {
		fprintf(stderr, "%s", usage);
		exit(1);	/* M004 */
	    }
	    k++;	/* if only one source, use working directory */
	    name2ptr = ".";
	}

	for(i=0; i<k; i++)	/* this code solves the multiple sources problem */ {
		/* get a source */
		s = name1;
		t = argv[i];
		while (*s++ = *t++);
		name1l = --s;	/* point to the '\0' in char string */
		while (*--s == '/' && s > name1) {
			*s = 0;
			name1l = s;
		}	/* extra / */

		/* get the destination */
		s = name2;
		t = name2ptr;	/* always the same file */
		while (*s++ = *t++);
		name2l = --s;
		while (*--s == '/' && s > name2) {
			*s = 0;
			name2l = s;
		}	/* extra / */

		getstat();
		if (k > 1 && type2 == S_IFREG) {
			fprintf(stderr, "copy: destination must be directory\n");
			exit(1);
		}
		if (type1 == S_IFDIR && type2 != S_IFREG) {
			if (cd() == ERROR)	/* do directory trans */
				retval = ERROR;
		}
		else {
			s = buffer -> d_name;
			t = name1;
			while (*s = *t++) if (*s++ == '/') s = buffer -> d_name;
			if (ask(askflag,"copy file",name1)) {
				if (cp() == ERROR)
					retval = ERROR;
			}
		}
	}
	exit(retval);
}

getstat()	/* gets the status on the two files */
{
	if (stat(name1,&stat1) == -1) {
		type1 = (mode_t)-1;
	}
	else {
		type1 = stat1.st_mode & S_IFMT;
	}

	if (stat(name2,&stat2) == -1) {
		type2 = (mode_t)-1;
	}
	else {
		type2 = stat2.st_mode & S_IFMT;
	}
}

setimes(to)
char *to;
{
	struct	utimbuf	*orgdate;

	orgdate = (struct utimbuf *)malloc((unsigned)sizeof(struct utimbuf));

	orgdate -> actime = stat1.st_atime;
	orgdate -> modtime = stat1.st_mtime;
	utime(to, orgdate);
}

cd()	/* copy a directory */
{
	DIR	*srcdir;	/* source-directory pointer */
	mode_t	mode, modeflags;
	uid_t	own_uid;
	gid_t	own_gid;
	char	*name1s, *name2s;
	/* time_t	orgdate[3]; */
	int	retval = OK;

	mode = type2;
	modeflags = stat1.st_mode & ~S_IFMT;	/* save for later use */
	own_uid = stat1.st_uid;
	own_gid = stat1.st_gid;
	if (type2 == (mode_t)-1) /* does not exist so create the directory */ {
		strcpy(cmdline, "mkdir ");
		strcat(cmdline, name2);
		if (system(cmdline) != 0) {
			fprintf(stderr, "copy: could not %s\n", cmdline);
			exit(1);
		}
		getstat();
	}
	if (type2 != S_IFDIR) {
		if (type2 == (mode_t)-1)
			fprintf(stderr, "Copy: could not mkdir %s\n",name2);
		else
			fprintf(stderr, "copy: %s is not a directory\n",name2);
		return(ERROR);
	}
	if ((srcdir = opendir(name1)) == NULL) {
		fprintf(stderr, "copy: could not open directory %s\n",name1);
		return(ERROR);
	}
	name1s = name1l;
	name2s = name2l;	/* save pointers */
	while (getentry(srcdir))	/* get an entry */ {
		if (strcmp(".", buffer -> d_name) == 0 ||
		    strcmp("..", buffer -> d_name) == 0)
			continue;
		push_name();	/* and push it onto the name1 & name2 */
		getstat();
		switch (type1) {
		case S_IFREG:
			if (ask(askflag,"copy file",name1)) {
				if (cp() == ERROR)
					retval = ERROR;
			}
			break;
		case S_IFCHR:
		case S_IFBLK:
#ifdef	S_IFMPC
		case S_IFMPC:
		case S_IFMPB:
#endif
#ifdef	S_IFNAM
		case S_IFNAM:	/* M001 */
#endif
#ifdef	S_IFIFO			/* M002 begin */
		case S_IFIFO:
#endif				/* M002 end */
			if (ask(askdirflag,"copy special file",name1)) {
				if (sf() == ERROR)
					retval = ERROR;
			}
			break;
		case S_IFDIR:
			if (recursiveflag) {	/* ignore if not */
				if (ask(askdirflag,"examine directory",name1))
					if (cd() == ERROR) /* copy directory */
						retval = ERROR;
			}
			break;
		case (mode_t)-1:
			fprintf(stderr, "copy: %s does not exist\n",name1);
			return(ERROR);
		default:
			fprintf(stderr, "copy: %s: funny file type %06o\n",name1, type1);
			return(ERROR);
		}
		name1l = name1s;
		*name1l = 0;	/* pop name1 */
		name2l = name2s;
		*name2l = 0;	/* pop name2 */
	}
	closedir(srcdir);	/* close the source-directory */
	if (mode == (mode_t)-1)	/* if directory was created then change */ {
		if (chmod(name2,modeflags) == -1) {
		    fprintf (stderr, "copy: Unable to chmod %s\n", name2);
		    retval = ERROR;
		}
		if (ownerflag) {
		    if (chown (name2, own_uid, own_gid) == -1) {
			fprintf (stderr, "copy: Unable to chown %s\n", name2);
		    	retval = ERROR;
		    }
		}
		if (utimeflag) setimes(name2);
	}
	return(retval);
}


sf()	/* copy a special file */
{
	int	retval = OK;

	if (mknod(name2, stat1.st_mode, stat1.st_rdev) == -1) {
		if (suflag && errno == EPERM)
			fprintf(stderr, "copy: Only super user can copy special files.\n");
		else
			fprintf(stderr, "copy: Unable to create %s\n", name2);
		return(ERROR);
	}
	if (ownerflag) {
		if (chown(name2, stat1.st_uid, stat1.st_gid) == -1) {
			fprintf (stderr, "Unable to chown %s\n", name2);
			retval = ERROR;
		}
	}
	if (utimeflag)
	/*	utime(name2, &stat1.st_atime);	 */
		setimes(name2);
	getstat();
	if (type1 != type2 || (stat1.st_mode & Vumask) != stat2.st_mode) {
		fprintf(stderr, "copy: could not create special or name file %s\n",name2);
		unlink(name2);
		return(ERROR);
		}
	return(retval);
}

getentry(dirp)
DIR *dirp;	/* the directory file pointer */
{
	if ((buffer = readdir(dirp)) == NULL) {
#ifdef	DEBUG
		printf("End of directory\n");
#endif
		return(0);
	}
#ifdef	DEBUG
	printf(
		"read dir entry %u %.*s\n",
		buffer -> d_ino, LSYSDIRNAM, buffer -> d_name
	);
#endif
	return(1);
}

push_name()
{
	register char *s, *t;

	/* add entry onto name1 */
	if (name1l+17 > &name1[256]) {
		fprintf(stderr, "copy: file name too long %s\n",name1);
		exit(1);
	}
	s = name1l;
	t = buffer -> d_name;
	*s++ = '/';
	while (*s++ = *t++);
	name1l = --s;

	/* add entry onto name2 */
	if (name2l+17 > &name2[256]) {
		fprintf(stderr, "Copy: file name too long %s\n",name2);
		exit(1);
	}
	s = name2l;
	t = buffer -> d_name;
	*s++ = '/';
	while (*s++ = *t++);
	name2l = --s;
}


cp()	/* works just like the cp command */
{
	static char buf[BUFSIZ];	/* M??? */
	int fold, fnew;
	register int n;
	register char *s, *t;
	int	retval = OK;

	/* is target a directory? */
	if (type2 == S_IFDIR)	/* if so then pad name2 with buffer */ {
	    s = name2l;	/* set to last char in destination name */
	    *s++ = '/';	/* push a '/' */
	    t = buffer -> d_name;	/* points to begin of source string */
	    while (*s++ = *t++);/* add last part of name2 */
	    getstat();
	}
	if (type2 != (mode_t)-1) {
	    if (newonlyflag)    /* cannot process if flag is on and old */ {
		fprintf(stderr, "copy: cannot overwrite %s\n",name2);
		return(ERROR);
	    }
	    if (stat1.st_dev==stat2.st_dev) {
		if (stat1.st_ino==stat2.st_ino)
		    return(OK);
		if (linkflag && unlink(name2) == -1) {
		    fprintf(stderr, "copy: cannot remove %s\n",name2);
		    return(ERROR);
		}
	    }
	}

	switch ( type1 ) {
	case S_IFCHR:
	case S_IFBLK:
#ifdef	S_IFMPC
	case S_IFMPC:
	case S_IFMPB:
#endif
#ifdef	S_IFNAM
	case S_IFNAM:	/* M001 */
#endif
#ifdef	S_IFIFO			/* M002 begin */
	case S_IFIFO:
#endif				/* M002 end */
		if (linkflag == 0 || link(name1, name2) == -1) {
			if (sf() == ERROR)
				retval = ERROR;
		}
		break;

	default:
		if (linkflag == 0 || link(name1,name2) == -1) {
			if ((fold = open(name1, O_RDONLY)) == -1) {
			    fprintf(stderr, "copy: cannot open %s\n",name1);
			    return(ERROR);
			}
			if ((fnew = creat(name2,stat1.st_mode)) < 0) {
			    fprintf(stderr, "copy: cannot create %s\n",name2);
			    close(fold);
			    return(ERROR);
			}
			if (ownerflag && type2 == (mode_t)-1) {
			    if (chown(name2, stat1.st_uid, stat1.st_gid) == -1) {
				fprintf (stderr, "Unable to chown %s\n", name2);
				retval = ERROR;
			    }
			}
			while (n = read(fold,buf,BUFSIZ)) {	/* M??? */
			    if (n < 0) {
				fprintf(stderr, "copy: read error on %s\n",name1);
				unlink(name2);
				retval = ERROR;
				break;
			    }
			    else if (write(fnew, buf, n) != n) {
				fprintf(stderr, "copy: write error on %s\n",name2);
				unlink(name2);
				retval = ERROR;
				break;
			    }
			}
			close(fold);
			close(fnew);
		}
	}

	if (utimeflag)
/*		utime(name2, &stat1.st_atime);	 */
		setimes(name2);

	return(retval);
}

ask(f,s,t)
int	f;
char	*s, *t;
{
	char ch;
	register flg;
	if (f)	/* if flag true then ask question */ {
		fprintf(stderr, "%s %s? ",s,t);
		if (read(0,&ch,1) != 1) return(0);
		flg = ch == 'y';
		while (ch != '\n' && read(0,&ch,1) == 1) ;
		return(flg);
	}
	else if (verboseflag) printf("%s %s\n",s,t);
	return(1);
}

