/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-streams:kmacct/kmapr.c	1.1"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/kmacct.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#define	FALSE		0
#define	TRUE		1

#define	DBSIZE		4000		/* Size of the namelist database */
#define	STRSIZE		80000		/* Space for namelist names */
#define	MAXNAME		50		/* Largest name in database */
#define	KMADEV		"/dev/kmacct"	/* Default device name */
#define	UNIX		"/unix"		/* Default boot program */
#define	NLDB		"/tmp/kmanl.db"	/* Default database */

static	char	usage[] = "ld:k:n:s:u:";			
static	char	*heading1 =
	"    16 |   32 |   64 |  128 |  256 |  512 | 1024 | 2048 | 4096 | LARGE";
static	char	*heading2 =
	"+------+------+------+------+------+------+------+------+------+------+";

static	int	out[KMSIZES];		/* Count of unreturned buffers */
static	int	strsize	= STRSIZE;	/* Default space for function names */
static	int	dbsize	= DBSIZE;	/* Default space for namelist db */
static	int	dbp	= 0;		/* Index of last dbentry */
static	int	nkmasym;		/* Number of symbol table entries */
static	int	kmadepth;		/* Depth of stack trace */
static	struct	dbentry *db;		/* Array for namelist db */
static	kmasym_t	*kmasymtab;	/* Space for KMA symbol table */
static	caddr_t	*kmastack;		/* Array of calling sequences */
static	char	*names;			/* Array for function names */
static	printleak	= FALSE;	/* Print possible leaks only */

static	void	usagerr();

extern	void	exit(), perror();
extern	void	*malloc();
extern	int	getopt(), atoi(), ioctl(), read(), close();
extern	int	errno;

struct	dbentry	{
	caddr_t	addr;			/* Starting address of function */
	char	*name;			/* Pointer to name of function */
	};

 static int
checknldb(nfile, bfile)
	char	*nfile, *bfile;
{
	struct stat	sbuf1, sbuf2;
	char		sys[80];

	if (stat(nfile, &sbuf1) == -1)
		if (errno == ENOENT) {
			(void) sprintf(sys, "kmamkdb %s", bfile);
			return system(sys);
		} else {
			perror("Cannot stat namelist database");
			return -1;
		}

	if (stat(bfile, &sbuf2) == -1) {
		perror("Warning: cannot stat boot program");
		return 0;
	}

	if (sbuf2.st_mtime > sbuf1.st_mtime) 
		(void) fprintf(stderr, "Warning: %s newer than %s\n",
			bfile, nfile);

	return 0;
}

 static int
initialize(nfile, bfile)
	char	*nfile, *bfile;
{
	FILE		*fd;
	ulong		addr;
	char		name[MAXNAME];
	char		*nsp;
	

	if ((db = (struct dbentry *)
		malloc((unsigned) (dbsize + 2) * sizeof (struct dbentry)))
		== (struct dbentry *) NULL) {
		(void) fprintf(stderr, "Cannot allocate space for db\n");
		return -1;
	}
	
	if ((names = malloc((unsigned) (strsize + 50))) == (char *) NULL) {
		(void) fprintf(stderr, "Cannot allocate space for names\n");
		return -1;
	}

	if (checknldb(nfile, bfile))
		return -1;
	
	if ((fd = fopen(nfile, "r")) == NULL) {
		(void) fprintf(stderr, "Cannot open %s for reading\n", nfile);
		return -1;
	}

	nsp = names;

	db[dbp].addr = 0x00000000;
	(void) strcpy(names, "KERNSTART");
	nsp += (strlen("KERNSTART") + 1);

	while (fscanf(fd, "%i %s", &addr, name) == 2) {

		if (++dbp >= dbsize) {
			(void) fprintf(stderr, "Error: %s %d\n\t(%s)\n",
				"Too many entries in namelist--exceeded",
				dbsize,
				"use -d option to allocate more space");
			return -1;
		}

		if (db[dbp-1].addr > (caddr_t) addr) {
			(void) fprintf(stderr,
				"ERROR: %s is not sorted by address\n",
				nfile);
			exit(1);
		}

		db[dbp].addr = (caddr_t) addr;
		db[dbp].name = nsp;
		(void) strcpy(nsp, name);
		nsp += (strlen(name) + 1);
		
		if ((int) (nsp - names) >= strsize) {
			(void) fprintf(stderr, "Error: %s %d\n\t(%s)\n",
				"Too much string space used--exceeded",
				strsize,
				"use -s option to allocate more space");
			return -1;
		}
	}

	db[++dbp].addr = (caddr_t) 0xFFFFFFFF;
	(void) strcpy(nsp, "KERNEND");

	return 0;
	
}

 static char *
lookup(addr, off)
	caddr_t	addr;
	int	*off;
{
	int	left	= 0;
	int	right	= dbp;
	int	middle;
	caddr_t	dbs, dbe;

	for (;;) {

		middle = (right + left) / 2;
		dbs = db[middle].addr;
		dbe = db[middle+1].addr;

		if (addr >= dbs && addr < dbe) {
			*off = addr - db[middle].addr;
			return (db[middle].name);
		}

		if (left == right) {
			*off = 0;
			return ((char *) NULL);
		}

		if (addr < dbs)
			right = middle - 1;
		else
			left = middle + 1;
	}
	/*NOTREACHED*/
}

 static int
startup(argc, argv)
	int	argc;
	char	**argv;
{
	int	c, fd, size;
	char	*nfile	= NLDB;
	char	*bfile	= UNIX;
	char	*kdev	= KMADEV;
	char	*data;
	extern	char	*optarg;

	/*
	 *	Parse command line options.
	 */

	while((c = getopt(argc, argv, usage)) != EOF)
		switch(c) {
		case 'd':
			dbsize = atoi(optarg);
			break;
		case 'k':
			kdev = optarg;
			break;
		case 'l':
			printleak = TRUE;
			break;
		case 'n':
			nfile = optarg;
			break;
		case 's':
			strsize = atoi(optarg);
			break;
		case 'u':
			bfile = optarg;
			break;
		case '?':
		default:
			usagerr(argv[0]);
		}

	if (initialize(nfile, bfile) == -1) {
		(void) fprintf(stderr, "%s: cannot initialize namelist db\n",
			argv[0]);
		exit(2);
	}
	
	if ((fd = open(kdev, O_RDONLY)) == -1) {
		perror("Cannot open device to KMA Account driver");
		exit(errno);
	}

	if ((nkmasym = ioctl(fd, KMACCT_NDICT)) == -1) {
		perror("Cannot get number of KMA Account entries");
		exit(errno);
	}

	if ((kmadepth = ioctl(fd, KMACCT_DEPTH)) == -1) {
		perror("Cannot get depth of KMA Account trace");
		exit(errno);
	}

	if ((size = ioctl(fd, KMACCT_SIZE)) == -1) {
		perror("Cannot get size of KMA Account data");
		exit(errno);
	}

	if ((data = malloc((unsigned) size)) == NULL) {
		(void) fprintf(stderr, "%s: cannot allocate space for symbol table",
			argv[0]);
		exit(2);
	}

	if (read(fd, data, (unsigned) size) == -1) {
		perror("Cannot read KMA Account data");
		exit(errno);
	}

	(void) close(fd);

	kmasymtab = (kmasym_t *) data;
	data += nkmasym * sizeof(kmasym_t);
	kmastack = (caddr_t *) data;

	return 0;
}

 static void
printentry(which, kp)
	int		which;
	kmasym_t	*kp;
{
	int	i, index, off;
	char	*name;

	(void) printf("\t %s\n", heading1);

	(void) printf("\t %s\n%8s |", heading2, "ALLOCATE");
	for (i = 0; i < KMSIZES; i++) {
		(void) printf("%5lu |", kp->reqa[i]);
		out[i] += kp->reqa[i];
	}
	(void) putchar('\n');

	(void) printf("%8s |", "FREE");
	for (i = 0; i < KMSIZES; i++) {
		(void) printf("%5lu |", kp->reqf[i]);
		out[i] -= kp->reqf[i];
	}
	(void) putchar('\n');

	index = which * kmadepth;
	for(i = 0; i < kmadepth; i++) {
		name = lookup(kmastack[index+i], &off);
		if (name != (char *) NULL)
			(void) printf("\t\t%s + 0x%x\n", name, off);
		else
			break;
	}

	(void) putchar('\n');
}

 static int
leaks(kp)
	kmasym_t	*kp;
{
	int	i;

	for (i = 0; i < KMSIZES; i++) 
		if (kp->reqa[i] != kp->reqf[i])
			return -1;
	return 0;
}

main(argc, argv)
	int	argc;
	char	**argv;
{
	int		i;
	kmasym_t	*kp;

	(void) startup(argc, argv);

	/*
	 * Run through the symbol table, printing entry if
	 * it has been used.  If -l option was specified, only
	 * print entry if it shows a leak.  An entry is used
	 * iff there was a pc stack asssociated with it.
	 */

	for(i = 0, kp = kmasymtab; i < nkmasym; i++, kp++)
		if (kp->pc)
			if (printleak) {
				if (leaks(kp))
					printentry(i, kp);
			} else
				printentry(i, kp);

	(void) printf("\n\tBuffers still allocated:\n");
	(void) printf("\t%s\n", heading1);
	(void) printf("\t%s\n\t|", heading2);
	for (i = 0; i < KMSIZES; i++)
		(void) printf("%5d |", out[i]);
	(void) putchar('\n');

	exit(0);
	/*NOTREACHED*/
}


void
usagerr(prog)
char	*prog;
{
	(void) fprintf(stderr,
	    "USAGE: %s [-l] [-k kmacct_dev] [-u bootprogram] \\\n",
	    prog);
	(void) fprintf(stderr,
	    "\t[-n namelist] [-d dbsize] [-s strspace]\n\n");
	(void) fprintf(stderr,
	    "\t(no options)\tprint summary of memory usage\n");
	(void) fprintf(stderr,
	    "\t-l\t\tprint memory leaks (allocated but not freed)\n");
	(void) fprintf(stderr,
	    "\t-k kmacct_dev\tkma accounting device (default %s)\n",
	    KMADEV);
	(void) fprintf(stderr,
	    "\t-u bootprogram\tboot program (default %s)\n",
	    UNIX);
	(void) fprintf(stderr,
	    "\t-n namelist\tnamelist database (default %s)\n",
	    NLDB);
	(void) fprintf(stderr,
	    "\t-d dbsize\tsize of the namelist database (default %d)\n",
	    DBSIZE);
	(void) fprintf(stderr,
	    "\t-s strspace\tspace for namelist names (default %d)\n",
	    STRSIZE);

	exit(1);
}
