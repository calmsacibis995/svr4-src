/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ls:ls.c	1.38.9.3"

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */
/*
 *	Copyright (c) 1986, 1987, 1988, Sun Microsystems, Inc.
 *	All Rights Reserved.
 */
/*
* 	list file or directory;
* 	define DOTSUP to suppress listing of files beginning with dot
*/
#include	<sys/param.h>
#include	<sys/types.h>
#include	<sys/mkdev.h>
#include	<sys/stat.h>
#include	<stdio.h>
#include	<ctype.h>
#include	<dirent.h>
#include	<string.h>
#include	<locale.h>
#include	<curses.h>
#include	<termios.h>

#ifndef STANDALONE
#define TERMINFO
#endif

/* -DNOTERMINFO can be defined on the cc command line to prevent
 * the use of terminfo.  This should be done on systems not having
 * the terminfo feature (pre 6.0 sytems ?).
 * As a result, columnar listings assume 80 columns for output,
 * unless told otherwise via the COLUMNS environment variable.
 */
#ifdef NOTERMINFO
#undef TERMINFO
#endif

#include	<term.h>

#define	BFSIZE	16
#define	DOTSUP	1
#define ISARG   0100000 /* this bit equals 1 in lflags of structure lbuf 
                        *  if *namep is to be used;
                        */

/*	Date and time	formats   */

#define FORMAT1	 " %b %e  %Y "
#define FORMAT2  " %b %e %H:%M "        /*
					 * b --- abbreviated month name
					   e --- day number
					   Y --- year in the form ccyy
					   H --- hour (24 hour version)
					   M --- minute   */

struct	lbuf	{
	union	{
		char	lname[MAXNAMLEN];   /* used for filename in a directory */
		char	*namep;          /* for name in ls-command; */
	} ln;
	char	ltype;  	/* filetype */
	ino_t	lnum;		/* inode number of file */
	mode_t	lflags; 	/* 0777 bits used as r,w,x permissions */
	nlink_t	lnl;    	/* number of links to file */
	uid_t	luid;
	gid_t	lgid;
	off_t	lsize;  	/* filesize or major/minor dev numbers */
	u_long	lblocks;	/* number of file blocks */
	time_t	lmtime;
	char	*flinkto;	/* symbolic link contents */
};

struct dchain {
	char *dc_name;		/* path name */
	struct dchain *dc_next;	/* next directory in the chain */
};

struct dchain *dfirst;	/* start of the dir chain */
struct dchain *cdfirst;	/* start of the durrent dir chain */
struct dchain *dtemp;	/* temporary - used for linking */
char *curdir;		/* the current directory */

int	nfiles = 0;	/* number of flist entries in current use */
int	nargs = 0;	/* number of flist entries used for arguments */
int	maxfils = 0;	/* number of flist/lbuf entries allocated */
int	maxn = 0;	/* number of flist entries with lbufs assigned */
int	quantn = 64;	/* allocation growth quantum */

struct	lbuf	*nxtlbf;	/* pointer to next lbuf to be assigned */
struct	lbuf	**flist;	/* pointer to list of lbuf pointers */
struct	lbuf	*gstat();

FILE	*pwdfu, *pwdfg;

int	aflg, bflg, cflg, dflg, fflg, gflg, iflg, lflg, mflg;
int	nflg, oflg, pflg, qflg, sflg, tflg, uflg, xflg;
int	Cflg, Fflg, Rflg, Lflg;
int	rflg = 1;   /* initialized to 1 for special use in compar() */
mode_t	flags;
int	err = 0;	/* Contains return code */

uid_t	lastuid	= (uid_t)-1;
gid_t	lastgid = (gid_t)-1;
int	statreq;    /* is > 0 if any of sflg, (n)lflg, tflg are on */

char	*dotp = ".";
char	*makename();
char	tbufu[BFSIZE], tbufg[BFSIZE];   /* assumed 15 = max. length of user/group name */

unsigned long	tblocks;  /* total number of blocks of files in a directory */
time_t	year, now;

int	num_cols = 80;
int	colwidth;
int	filewidth;
int	fixedwidth;
int	nomocore;
int	curcol;
int	compar();
void	qsort();

struct	winsize	win;

extern	int stat(), lstat();
static char	time_buf[50];	/* array to hold day and time */

main(argc, argv)
int argc;
char *argv[];
{
	extern char	*optarg;
	extern int	optind;
	int	amino, opterr=0;
	int	c;
	register struct lbuf *ep;
	struct	lbuf	lb;
	int	i, width;
	long	time();
	char *malloc();
	void exit();

	(void)setlocale(LC_ALL, "");
#ifdef STANDALONE
	if (argv[0][0] == '\0')
		argc = getargv("ls", &argv, 0);
#endif

	lb.lmtime = time((long *) NULL);
	year = lb.lmtime - 6L*30L*24L*60L*60L; /* 6 months ago */
	now = lb.lmtime + 60;
	if ((!strcmp((char *)basename(argv[0]), "lc")) || (isatty(1)))
		{
		Cflg = 1;
		mflg = 0;
		}

	while ((c=getopt(argc, argv,
			"RadC1xmnlogrtucpFbqisfL")) != EOF) switch(c) {
		case 'R':
			Rflg++;
			statreq++;
			continue;
		case 'a':
			aflg++;
			continue;
		case 'd':
			dflg++;
			continue;
		case 'C':
			Cflg = 1;
			mflg = 0;
			continue;
		case '1':
			Cflg = 0;
			continue;
		case 'x':
			xflg = 1;
			Cflg = 1;
			mflg = 0;
			continue;
		case 'm':
			Cflg = 0;
			mflg = 1;
			continue;
		case 'n':
			nflg++;
		case 'l':
			lflg++;
			statreq++;
			continue;
		case 'o':
			oflg++;
			lflg++;
			statreq++;
			continue;
		case 'g':
			gflg++;
			lflg++;
			statreq++;
			continue;
		case 'r':
			rflg = -1;
			continue;
		case 't':
			tflg++;
			statreq++;
			continue;
		case 'u':
			uflg++;
			continue;
		case 'c':
			cflg++;
			continue;
		case 'p':
			pflg++;
			statreq++;
			continue;
		case 'F':
			Fflg++;
			statreq++;
			continue;
		case 'b':
			bflg = 1;
			qflg = 0;
			continue;
		case 'q':
			qflg = 1;
			bflg = 0;
			continue;
		case 'i':
			iflg++;
			continue;
		case 's':
			sflg++;
			statreq++;
			continue;
		case 'f':
			fflg++;
			continue;
		case 'L':
			Lflg++;
			continue;
		case '?':
			opterr++;
			continue;
		}
	if(opterr) {
		if (!strcmp((char *)basename(argv[0]), "lc")) {
			fprintf(stderr,"usage: lc -1RadCxmnlogrtucpFbqisfL [files]\n");
			exit(2);
		} else {
			fprintf(stderr,"usage: ls -1RadCxmnlogrtucpFbqisfL [files]\n");
			exit(2);
		}
	}

	if (fflg) {
		aflg++;
		lflg = 0;
		sflg = 0;
		tflg = 0;
		statreq = 0;
	}

	fixedwidth = 2;
	if (pflg || Fflg)
		fixedwidth++;
	if (iflg)
		fixedwidth += 6;
	if (sflg)
		fixedwidth += 5;

	if (lflg) {
		if (!gflg && !oflg)
			gflg = oflg = 1;
		else
		if (gflg && oflg)
			gflg = oflg = 0;
		Cflg = mflg = 0;
		if (oflg > 0)
			if ((pwdfu = fopen("/etc/passwd", "r")) == NULL) {
				fprintf(stderr,"%s file cannot be opened for reading\n","/etc/passwd");
				exit(2);
			}
		if (gflg > 0)
			if ((pwdfg = fopen("/etc/group", "r")) == NULL) {
				fprintf(stderr,"%s file cannot be opened for reading\n","/etc/group");
				exit(2);
			}
	}

	if (Cflg || mflg) {
		char *getenv();
		char *clptr;
		if ((clptr = getenv("COLUMNS")) != NULL)
			num_cols = atoi(clptr);
#ifdef TERMINFO
		else {
			if (ioctl(1, TIOCGWINSZ, &win) != -1)
				num_cols = (win.ws_col == 0 ? 80 : win.ws_col);	
		}
#endif
		if (num_cols < 20 || num_cols > 160)
			/* assume it is an error */
			num_cols = 80;
	}

	/* allocate space for flist and the associated	*/
	/* data structures (lbufs)			*/
	maxfils = quantn;
	if((flist=(struct lbuf **)malloc((unsigned)(maxfils * sizeof(struct lbuf *)))) == NULL
	|| (nxtlbf = (struct lbuf *)malloc((unsigned)(quantn * sizeof(struct lbuf)))) == NULL) {
		fprintf(stderr, "ls: out of memory\n");
		exit(2);
	}
	if ((amino=(argc-optind))==0) { /* case when no names are given
					* in ls-command and current 
					* directory is to be used 
 					*/
		argv[optind] = dotp;
	}

	for (i=0; i < (amino ? amino : 1); i++) {
		if (Cflg || mflg) {
			width = strlen(argv[optind]);
			if (width > filewidth)
				filewidth = width;
		}
		if ((ep = gstat((*argv[optind] ? argv[optind] : dotp), 1))==NULL)
		{
			if (nomocore)
				exit(2);
			err = 2;
			optind++;
			continue;
		}
		ep->ln.namep = (*argv[optind] ? argv[optind] : dotp);
		ep->lflags |= ISARG;
		optind++;
		nargs++;	/* count good arguments stored in flist */
	}
	colwidth = fixedwidth + filewidth;
	qsort(flist, (unsigned)nargs, sizeof(struct lbuf *), compar);
	for (i=0; i<nargs; i++) {
		if (flist[i]->ltype=='d' && dflg==0 || fflg)
			break;
	}
	pem(&flist[0],&flist[i], 0);
	for (; i<nargs; i++) {
		pdirectory(flist[i]->ln.namep, (amino>1), nargs);
		if (nomocore)
			exit(2);
		/* -R: print subdirectories found */
		while (dfirst || cdfirst) {
			/* Place direct subdirs on front in right order */
			while (cdfirst) {
				/* reverse cdfirst onto front of dfirst */
				dtemp = cdfirst;
				cdfirst = cdfirst -> dc_next;
				dtemp -> dc_next = dfirst;
				dfirst = dtemp;
			}
			/* take off first dir on dfirst & print it */
			dtemp = dfirst;
			dfirst = dfirst->dc_next;
			pdirectory (dtemp->dc_name, 1, nargs);
			if (nomocore)
				exit(2);
			free (dtemp->dc_name);
			free ((char *)dtemp);
		}
	}
	exit(err);
	/*NOTREACHED*/
}

/*
 * pdirectory: print the directory name, labelling it if title is
 * nonzero, using lp as the place to start reading in the dir.
 */
pdirectory (name, title, lp)
char *name;
int title;
int lp;
{
	register struct dchain *dp;
	register struct lbuf *ap;
	register char *pname;
	register int j;

	filewidth = 0;
	curdir = name;
	if (title) {
		putc('\n', stdout);
		pprintf(name, ":");
		new_line();
	}
	nfiles = lp;
	rddir(name);
	if (nomocore)
		return;
	if (fflg==0)
		qsort(&flist[lp],(unsigned)(nfiles - lp),sizeof(struct lbuf *),compar);
	if (Rflg) for (j = nfiles - 1; j >= lp; j--) {
		ap = flist[j];
		if (ap->ltype == 'd' && strcmp(ap->ln.lname, ".") &&
				strcmp(ap->ln.lname, "..")) {
			dp = (struct dchain *)calloc(1,sizeof(struct dchain));
			if (dp == NULL)
				fprintf(stderr,"ls: out of memory\n");
			pname = makename(curdir, ap->ln.lname);
			dp->dc_name = (char *)calloc(1,strlen(pname)+1);
			if (dp->dc_name == NULL) {
				fprintf(stderr,"ls: out of memory\n");
				free(dp);
			}
			else {
				strcpy(dp->dc_name, pname);
				dp->dc_next = dfirst;
				dfirst = dp;
			}
		}
	}
	if (lflg || sflg)
		curcol += printf("total %u", tblocks);
	pem(&flist[lp],&flist[nfiles],lflg||sflg);
}

/*
 * pem: print 'em.  Print a list of files (e.g. a directory) bounded
 * by slp and lp.
 */
pem(slp, lp, tot_flag)
	register struct lbuf **slp, **lp;
	int tot_flag;
{
	int  ncols, nrows, row, col;
	register struct lbuf **ep;

	if (Cflg || mflg)
		if (colwidth > num_cols) {
			ncols=1;
		}
		else {
			ncols = num_cols / colwidth;
		}

	if (ncols == 1 || mflg || xflg || !Cflg) {
		for (ep = slp; ep < lp; ep++)
			pentry(*ep);
		new_line();
		return;
	}
	/* otherwise print -C columns */
	if (tot_flag)
		slp--;
	nrows = (lp - slp - 1) / ncols + 1;
	for (row = 0; row < nrows; row++) {
		col = (row == 0 && tot_flag);
		for (; col < ncols; col++) {
			ep = slp + (nrows * col) + row;
			if (ep < lp)
				pentry(*ep);
		}
		new_line();
	}
}

pentry(ap)  /* print one output entry;
            *  if uid/gid is not found in the appropriate
            *  file (passwd/group), then print uid/gid instead of 
            *  user/group name;
            */
struct lbuf *ap;
{
	register struct lbuf *p;
	char buf[BUFSIZ];
	char *dmark = "";	/* Used if -p or -F option active */

	p = ap;
	column();
	if (iflg)
		if (mflg && !lflg)
			curcol += printf("%u ", p->lnum);
		else
			curcol += printf("%5u ", p->lnum);
	if (sflg)
		curcol += printf( (mflg && !lflg) ? "%ld " : "%4ld " ,
			(p->ltype != 'b' && p->ltype != 'c') ?
				p->lblocks : 0L );
	if (lflg) {
		putchar(p->ltype);
		curcol++;
		pmode(p->lflags);
		curcol += printf("%4d ", p->lnl);
		if (oflg)
			if(!nflg && getname(p->luid, tbufu, 0)==0)
				curcol += printf("%-9.9s", tbufu);
			else
				curcol += printf("%-9u", p->luid);
		if (gflg)
			if(!nflg && getname(p->lgid, tbufg, 1)==0)
				curcol += printf("%-9.9s", tbufg);
			else
				curcol += printf("%-9u", p->lgid);
		if (p->ltype=='b' || p->ltype=='c')
			curcol += printf("%3d,%3d", major((dev_t)p->lsize), minor((dev_t)p->lsize));
		else
			curcol += printf("%7ld", p->lsize);
		if((p->lmtime < year) || (p->lmtime > now))
			{
			cftime(time_buf, FORMAT1, &p->lmtime);
			curcol += printf("%s", time_buf);
			}
		else
			{
			cftime(time_buf, FORMAT2, &p->lmtime);
			curcol += printf("%s", time_buf);
			}
	}

	/*
	 * prevent both "->" and trailing marks
	 * from appearing
	 */

	if (pflg && p->ltype == 'd')
		dmark = "/";

	if (Fflg && !(lflg && p->flinkto)) {
		if (p->ltype == 'd')
			dmark = "/";
		else if (p->ltype == 'l')
			dmark = "@";
		else if (p->lflags & (S_IXUSR|S_IXGRP|S_IXOTH))
			dmark = "*";
		else
			dmark = "";
	}

	if (lflg && p->flinkto) {
		strncpy(buf, " -> ", 4);
		strcpy(buf + 4, p->flinkto);
		dmark = buf;
	}

	if (p->lflags & ISARG) {
		if (qflg || bflg)
			pprintf(p->ln.namep,dmark);
		else
			curcol += printf("%s%s",p->ln.namep,dmark);
	} else {
		if (qflg || bflg)
			pprintf(p->ln.lname,dmark);
		else
			curcol += printf("%s%s",p->ln.lname,dmark);
	}
}

/* print various r,w,x permissions 
 */
pmode(aflag)
mode_t aflag;
{
        /* these arrays are declared static to allow initializations */
	static int	m0[] = { 1, S_IRUSR, 'r', '-' };
	static int	m1[] = { 1, S_IWUSR, 'w', '-' };
	static int	m2[] = { 3, S_ISUID|S_IXUSR, 's', S_IXUSR, 'x', S_ISUID, 'S', '-' };
	static int	m3[] = { 1, S_IRGRP, 'r', '-' };
	static int	m4[] = { 1, S_IWGRP, 'w', '-' };
	static int	m5[] = { 3, S_ISGID|S_IXGRP, 's', S_IXGRP, 'x', S_ISGID, 'l', '-'};
	static int	m6[] = { 1, S_IROTH, 'r', '-' };
	static int	m7[] = { 1, S_IWOTH, 'w', '-' };
	static int	m8[] = { 3, S_ISVTX|S_IXOTH, 't', S_IXOTH, 'x', S_ISVTX, 'T', '-'};

        static int  *m[] = { m0, m1, m2, m3, m4, m5, m6, m7, m8};

	register int **mp;

	flags = aflag;
	for (mp = &m[0]; mp < &m[sizeof(m)/sizeof(m[0])];)
		select(*mp++);
}

select(pairp)
register int *pairp;
{
	register int n;

	n = *pairp++;
	while (n-->0) {
		if((flags & *pairp) == *pairp) {
			pairp++;
			break;
		}else {
			pairp += 2;
		}
	}
	putchar(*pairp);
	curcol++;
}

/*
 * column: get to the beginning of the next column.
 */
column()
{

	if (curcol == 0)
		return;
	if (mflg) {
		putc(',', stdout);
		curcol++;
		if (curcol + colwidth + 2 > num_cols) {
			putc('\n', stdout);
			curcol = 0;
			return;
		}
		putc(' ', stdout);
		curcol++;
		return;
	}
	if (Cflg == 0) {
		putc('\n', stdout);
		curcol = 0;
		return;
	}
	if ((curcol / colwidth + 2) * colwidth > num_cols) {
		putc('\n', stdout);
		curcol = 0;
		return;
	}
	do {
		putc(' ', stdout);
		curcol++;
	} while (curcol % colwidth);
}

new_line()
{
	if (curcol) {
		putc('\n',stdout);
		curcol = 0;
	}
}

/* read each filename in directory dir and store its
 *  status in flist[nfiles] 
 *  use makename() to form pathname dir/filename;
 */
rddir(dir)
char *dir;
{
	struct dirent *dentry;
	DIR *dirf;
	register int j;
	register struct lbuf *ep;
	register int width;

	if ((dirf = opendir(dir)) == NULL) {
		fflush(stdout);
		fprintf(stderr, "can not access directory %s\n", dir);
		err = 2;
		return;
	} else {
          	tblocks = 0;
		while (dentry = readdir(dirf)) {
          		if (aflg==0 && dentry->d_name[0]=='.' 
# ifndef DOTSUP
          			&& (dentry->d_name[1]=='\0' || dentry->d_name[1]=='.'
          			&& dentry->d_name[2]=='\0')
# endif
          			)  /* check for directory items '.', '..', 
                                   *  and items without valid inode-number;
                                   */
          			continue;
			if (Cflg || mflg) {
				width = strlen(dentry->d_name);
				if (width > filewidth)
					filewidth = width;
			}
          		ep = gstat(makename(dir, dentry->d_name), 0);
          		if (ep==NULL) {
				if (nomocore)
					return;
          			continue;
			}
                        else {
          		     ep->lnum = dentry->d_ino;
			     for (j=0; dentry->d_name[j] != '\0'; j++)
          		         ep->ln.lname[j] = dentry->d_name[j];
			     ep->ln.lname[j] = '\0';
                        }
          	}
          	(void) closedir(dirf);
		colwidth = fixedwidth + filewidth;
	}
}

/* get status of file and recomputes tblocks;
 * argfl = 1 if file is a name in ls-command and  = 0
 * for filename in a directory whose name is an
 * argument in the command;
 * stores a pointer in flist[nfiles] and
 * returns that pointer;
 * returns NULL if failed;
 */
struct lbuf *
gstat(file, argfl)
char *file;
{
	struct stat statb, statb1;
	register struct lbuf *rep;
	char *malloc(), *realloc();
	char buf[BUFSIZ];
	int cc;
	int (*statf)() = Lflg ? stat : lstat;

	if (nomocore)
		return(NULL);
	else if (nfiles >= maxfils) { 
/* all flist/lbuf pair assigned files time to get some more space */
		maxfils += quantn;
		if((flist=(struct lbuf **)realloc((char *)flist, (unsigned)(maxfils * sizeof(struct lbuf *)))) == NULL
		|| (nxtlbf = (struct lbuf *)malloc((unsigned)(quantn * sizeof(struct lbuf)))) == NULL) {
			fprintf(stderr, "ls: out of memory\n");
			nomocore = 1;
			return(NULL);
		}
	}

/* nfiles is reset to nargs for each directory
 * that is given as an argument maxn is checked
 * to prevent the assignment of an lbuf to a flist entry
 * that already has one assigned.
 */
	if(nfiles >= maxn) {
		rep = nxtlbf++;
		flist[nfiles++] = rep;
		maxn = nfiles;
	} else {
		rep = flist[nfiles++];
	}
	rep->lflags = (mode_t)0;
	rep->flinkto = NULL;
	if (argfl || statreq) {
		if ((*statf)(file, &statb) < 0) {
			perror(file);
			nfiles--;
			return(NULL);
		}
		rep->lnum = statb.st_ino;
		rep->lsize = statb.st_size;
		rep->lblocks = statb.st_blocks;
		switch(statb.st_mode & S_IFMT) {
		case S_IFDIR:
			rep->ltype = 'd';
			break;

		case S_IFBLK:
			rep->ltype = 'b';
			rep->lsize = (off_t)statb.st_rdev;
			break;
		case S_IFCHR:
			rep->ltype = 'c';
			rep->lsize = (off_t)statb.st_rdev;
			break;
		case S_IFIFO:
			rep->ltype = 'p';
			break;
		case S_IFNAM:
                        /* 'st_rdev' field is really the subtype */
                        switch (statb.st_rdev) {
                        case S_INSEM:
                               rep->ltype = 's';
                               break;
                        case S_INSHD:
                               rep->ltype = 'm';
                               break;
			default:
			       rep->ltype = '-';
			       break;
			}
			break;
		case S_IFLNK:
			rep->ltype = 'l';
			if (lflg) {
				cc = readlink(file,buf,BUFSIZ);
				if (cc >= 0) {

					/*
					 * follow the symbolic link
					 * to generate the appropriate
					 * Fflg marker for the object
					 * eg, /bin -> /sym/bin/
					 */
					if ((Fflg || pflg) && (stat(file, &statb1) >= 0)) {
						switch(statb1.st_mode & S_IFMT) {
						case S_IFDIR:
							buf[cc++] = '/';
							break;
						default:
							if ((statb1.st_mode & ~S_IFMT) &
								(S_IXUSR|S_IXGRP|S_IXOTH))
								buf[cc++] = '*';
							break;
						}
					}
					buf[cc] = '\0';
					rep->flinkto = strdup(buf);
				}
				break;
			}

			/*
			 * ls /sym behaves differently from ls /sym/
			 * when /sym is a symbolic link. This is fixed
			 * when explicit arguments are specified.
			 */

			if (!argfl || stat(file, &statb1) < 0)
				break;
			if ((statb1.st_mode & S_IFMT) == S_IFDIR) {
				statb = statb1;
				rep->ltype = 'd';
				rep->lsize = statb1.st_size;
			}
			break;
		default:
			rep->ltype = '-';
		}
		rep->lflags = statb.st_mode & ~S_IFMT;

		/* mask ISARG and other file-type bits */
	
		rep->luid = statb.st_uid;
		rep->lgid = statb.st_gid;
		rep->lnl = statb.st_nlink;
		if (uflg)
			rep->lmtime = statb.st_atime;
		else if (cflg)
			rep->lmtime = statb.st_ctime;
		else
			rep->lmtime = statb.st_mtime;
		if (rep->ltype != 'b' && rep->ltype != 'c')
			tblocks += rep->lblocks;
	}
        return(rep);
}

/* returns pathname of the form dir/file;
 *  dir is a null-terminated string;
 */
char *
makename(dir, file) 
char *dir, *file;
{
	static char dfile[MAXNAMLEN];  /*  Maximum length of a
                                        *  file/dir name in ls-command;
                                        *  dfile is static as this is returned
                                        *  by makename();
                                        */
	register char *dp, *fp;

	dp = dfile;
	fp = dir;
	while (*fp)
		*dp++ = *fp++;
	if (dp > dfile && *(dp - 1) != '/')
		*dp++ = '/';
	fp = file;
	while (*fp)
		*dp++ = *fp++;
	*dp = '\0';
	return(dfile);
}

/* get name from passwd/group file for a given uid/gid
 *  and store it in buf; lastuid is set to uid;
 *  returns -1 if uid is not in file
 */
getname(uid, buf, type)
uid_t uid;
int type;
char buf[];
{
        int c;
        register i, j, n;

	if (uid==(type ? lastgid : lastuid))
		return(0);
	rewind(type ? pwdfg : pwdfu);
	if(type)
		lastgid = (gid_t)-1;
	else	lastuid = (uid_t)-1;
	do {
		i = 0;
		j = 0;
		n = 0;
                while((c=fgetc(type ? pwdfg : pwdfu)) != '\n') {  /* '\n' indicates end of 
                                                  *  a per user/group record
                                                  *  in passwd/group file;
                                                  */
                     if (c==EOF)
                        return(-1);  
                     else if (c==':') j++;
                          else if (j==0 && i < (BFSIZE - 1)) buf[i++] = c;
                               else if (j==2)
                                       n = n*10 + (c-'0');
		}
	} while ((uid_t)n != uid);
	buf[i] = '\0';
	if (type)
		lastgid = uid;
	else	lastuid = uid;
	return(0);
}

compar(pp1, pp2)  /* return >0 if item pointed by pp2 should appear first */
struct lbuf **pp1, **pp2;
{
	register struct lbuf *p1, *p2;

	p1 = *pp1;
	p2 = *pp2;
	if (dflg==0) {
/* compare two names in ls-command one of which is file
 *  and the other is a directory;
 *  this portion is not used for comparing files within
 *  a directory name of ls-command;
 */
		if (p1->lflags&ISARG && p1->ltype=='d') {
			if (!(p2->lflags&ISARG && p2->ltype=='d'))
				return(1);
                }
                else {
			if (p2->lflags&ISARG && p2->ltype=='d')
				return(-1);
		}
	}
	if (tflg) {
		if(p2->lmtime == p1->lmtime)
			return(0);
		else if(p2->lmtime > p1->lmtime)
			     return(rflg);
		else return(-rflg);
	}
        else
             return(rflg * strcmp(p1->lflags&ISARG? p1->ln.namep: p1->ln.lname,
				p2->lflags&ISARG? p2->ln.namep: p2->ln.lname));
}

pprintf(s1,s2)
	char *s1, *s2;
{
	register char *s;
	register int   c;
	register int  cc;
	int i;

	for (s = s1, i = 0; i < 2; i++, s = s2)
		while(c = (unsigned char) *s++) {
			if ( ! isprint(c) ) {
				if (qflg)
					c = '?';
				else if (bflg) {
					curcol += 3;
					putc ('\\', stdout);
					cc = '0' + (c>>6 & 07);
					putc (cc, stdout);
					cc = '0' + (c>>3 & 07);
					putc (cc, stdout);
					c = '0' + (c & 07);
				}
			}
			curcol++;
			putc(c, stdout);
		}
}
