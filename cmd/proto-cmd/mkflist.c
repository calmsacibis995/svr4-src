/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)proto-cmd:mkflist.c	1.3"

/*
 *********************************************************************
 *                                                                   *
 * NAME:                                                             *
 *	main()                                                       *
 *                                                                   *
 * DESCRIPTION:                                                      *
 *	The pupose of this program is to create files containing a   *
 *	list of files to be cpio(ed) to a floppy.                    *
 *	Therefore the size of all the files in each list could       *
 *	never be greater than the size of the floppy.                *
 *	This calculation is provided here.                           *
 *                                                                   *
 *	It takes as arguments:                                       *
 *		1.- Pathname of file to read, the "Plist"            *
 *		2.- The core pathname of files to write,             *
 *			    the "cpiolist.n" files.                  *
 *		3.- The pathname for the warning messages,           *
 *		    	the "errmsg" file.                           *
 *		4.- The maximum number of blocks.                    *
 *		5.- The initial sequence number for the              *
 *			cpiolist.n files.                            *
 *                                                                   *
 *	It reads the records of "Plist" one a time, until EOF.       *
 *                                                                   *
 *	All the pathnames will get recorded in one of the n          *
 *	"cpiolist.n" files, execpt the ones that were found to be    *
 *	non exixtent.                                                *
 *	For each nonexisting pathname, an error message gets logged  *
 *              in the errmsg file.                                  *
 *                                                                   *
 *	Before the pathnames get recorded, it is determined if the   *
 *	size of the file will cause an overflow. If that's the case, *
 *	the file is queued until the floppy is exhausted. Then the   *
 *	"cpiolist.n" file is closed, and another one is opened.      *
 *	Any queued files are written out first, then new files.      *
 *                                                                   *
 *********************************************************************
 */
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <string.h>

#define  LINE_SZ	BUFSIZ
#define  PATH_SZ	130
#define  BYTES		512
#define  LOW_WATER	BUFSIZ

/*
 * The format of cpio archive is: HeaderFileHeaderFile ...TRAILER!!!
 * The size of the Header (ASCSZ or 76 in 3.2) + number of character for the filename +1
 * The size of the trailer (size of Header) + number of characters in "TRAILER!!!\0"
 * See section 4 in the Programmer Users Manual.
 */
#ifdef __STDC__
#include <archives.h>
#define  CPIOHEADER	(ASCSZ + 4)
#define  CPIOTRAILER	(ASCSZ + 15)
#else	/* 3.2 CPIO Values */
#define  CPIOHEADER	76
#define  CPIOTRAILER	87
#endif 


void	exit(), free(), perror();

FILE	*fopen(), *freopen();

FILE	*newfloppy();

long int	 atol();
#ifdef TSIZE
long int	nblock();
#endif

char	*fgets(), *strcpy(), *strncat(), *malloc();

long int  MAXBLOCKS, availspace;
struct stat buf, *stbuf;
FILE *fr, *fw, *fe;
char file2write[PATH_SZ], file4errs[PATH_SZ], filecore[PATH_SZ];
char name[LINE_SZ];

typedef struct QITEM {
	struct QITEM *qnext;
	long qsize;
	char *qname;
} QITEM;
QITEM *qhead=(QITEM *) NULL;
QITEM *qtail=(QITEM *) NULL;
QITEM **qpos;


main(argc, argv)
int argc;
char *argv[];
{
	extern struct stat buf, *stbuf;
	extern char name[LINE_SZ];
	extern FILE *fr, *fw, *fe;
	extern char file2write[], file4errs[], filecore[];
	int  n;
	long int medium_size;
	extern long int  MAXBLOCKS;
	extern long int availspace;
	long int size4cpio;
#ifdef TSIZE
	long int totsize, remd;
	long int tfilesize;
	long int filesize;
#endif

	if ( argc != 6 )
	{
		printf("\n*****USAGE: %s arg1 arg2 arg arg4 arg5",argv[0]);
		printf("\narg1 is pathname of file2read");
		printf("\narg2 is the core pathname for files2write");
		printf("\narg3 is the pathname of file for warning msgs.");
		printf("\narg4 is the maximum number of blocks.");
		printf("\narg5 is the number of 1st cpiolist (1 or 2)\n");
		exit(1);
	}

	/* Initialize variables */
	stbuf = &buf;
	n = atoi( argv[5] );
	strcpy(filecore, argv[2]);
	sprintf(file2write, "%s%.2d", filecore, n);
	strcpy(file4errs, argv[3]);
	MAXBLOCKS = atol( argv[4] );
	/*
	 * The maxblocks number must be multiple of 10
	 */
	MAXBLOCKS = ( (MAXBLOCKS /10 ) * 10);
	medium_size = MAXBLOCKS * BYTES;

	availspace = ( MAXBLOCKS * BYTES ) - CPIOTRAILER;

	/* open the "Plist" file which was passed as 1st argument */
	if (*argv[1] == '-') {
		fr = stdin;
	}
	else
	{	if ( (fr = fopen( argv[1], "r" )) == NULL ) {
			perror(*argv);
			exit(1);
		}
	}

	/* open the first "cpiolist.1" to write */
	if ( (fw = fopen( file2write, "w" )) == NULL )
	{
		perror(file2write);
		exit(1);
	}

	/* open to write warning error msgs. as they are found */
	if ( (fe = fopen( file4errs, "w" )) == NULL )
	{
		perror(file4errs);
		exit(1);
	}

	if (!getnextf()) {
		perror("First file");
		exit(2);
	}
	for(qpos = &qhead;;) {

#ifdef TSIZE
	    /* determine size of file, including indirect blocks */
	    filesize = nblock(stbuf->st_size);
	    tfilesize += filesize;
#endif

		size4cpio = ( CPIOHEADER + strlen(name) + 1 );

		/* see if this is a directory - then size=headersize */
		if ( (stbuf->st_mode&S_IFMT) != S_IFDIR )
			size4cpio += stbuf->st_size;

		if (feof(stdin)) { /* do we need to deque? */
			if ( *qpos ) {
				if ( (*qpos)->qsize > availspace ) {
					/* There may be an item in the queue 
					 * that fits, but, since we need 
					 * another list anyway ...
					*/
#ifdef DEBUG
	fprintf(stderr,"End of input: Floppy %d FULL.\n", n);
#endif
					fw=newfloppy(++n);
				}
			}
			else    break;
		}
		if (*qpos && (*qpos)->qsize <= availspace) {
			QITEM *i = *qpos;
			fprintf(fw,"%s\n",i->qname);
#if DEBUG
		fprintf(stderr,"DEQUE %s %ld of %ld\n",i->qname,i->qsize,availspace);
#endif
			availspace -= i->qsize;
			free(i->qname);
			*qpos = i->qnext;
			free(i);
			if (qtail == i)
				for(qtail=qhead; qtail && qtail->qnext ; qtail=qtail->qnext);
		}
		else if (size4cpio <= availspace) {
			fprintf(fw,"%s\n",name);
#if DEBUG
			fprintf(stderr,"OUTPUT %s %ld of %ld\n",
					name,size4cpio,availspace);
#endif
#ifdef TSIZE
			totsize += size4cpio;
#endif
			availspace -= size4cpio;
			(void)getnextf();
		}
		else if ( (stbuf->st_mode&S_IFMT) == S_IFDIR
					|| (availspace < LOW_WATER) ) {
#ifdef DEBUG
	fprintf(stderr,"Floppy %d FULL (%ld left).\n", n, availspace);
#endif
			fw=newfloppy(++n);
		}
		else if (size4cpio > medium_size ) {
			fprintf(fe,"%s: %d bytes - too big for medium\n",
						name, size4cpio);
			fprintf(stderr,"%s: %d bytes - too big for medium\n",
						name, size4cpio);
			(void)getnextf();
		}
		else { /* enqueue file */
			char *s = malloc((unsigned)(strlen(name) + 1));
			QITEM *i = (QITEM *) malloc(sizeof(QITEM));
			if (s == NULL || i == (QITEM *) NULL ) {
				fprintf(stderr,"%s: Not enough memory\n",*argv);
				exit(2);
			}
			strcpy(s,name);
			i->qname = s;
			i->qsize = size4cpio;
			i->qnext = (QITEM *) NULL;
			if (qhead) {
				qtail->qnext = i;
				qtail = i;
			}
			else qtail = qhead = i;
#if DEBUG
			fprintf(stderr,"ENQUE %s %ld of %ld\n",
					name,size4cpio,availspace);
#endif
			(void)getnextf();
		}
	}


#ifdef DEBUG
	fprintf(stderr, "DONE - %ld left on last floppy.\n", availspace);
#endif
	fclose(fw);
	fclose(fe);
	fclose(fr);
	return(0);
}

getnextf()
{
	extern char name[];
	extern FILE *fr;
	char *pt;

	if ( fgets(name, LINE_SZ, fr) == NULL ) {
#ifdef DEBUG
		if (!feof(fr)) perror("fgets");
#endif
		return(0);
	}

	/* remove trailing '\n', if any */
	if ( (pt=strchr(name, '\n')) != NULL )
			*pt = '\0';
#ifdef DEBUG
	fprintf(stderr, "\nFilename: %s\n", name);
#endif

	if ( stat(name, stbuf) == -1 ) {
		/* log in err file */
		fprintf(fe,"***stat() - errno=%d on \"%s\"\n", errno, name);
		perror(name);			/* write to stderr */
		return(getnextf());
	}

	return(1);
}

FILE *
newfloppy(n)
int n;
{
	extern char file2write[];
	extern FILE *fw;

	/* open new list file */
	sprintf(file2write, "%s%.2d", filecore, n);
	if ((fw=freopen(file2write,"w",fw)) == NULL)
	{	perror(file2write);
		exit(1);
	}
	qpos = &qhead;
	availspace = ( MAXBLOCKS * BYTES ) - CPIOTRAILER;

	return(fw);
}

#ifdef TSIZE
/*
 *****************************************************************************
 *                                                                           *
 *                                                                           *
 *	NAME:                                                                *
 *		nblock()
 *                                                                           *
 *	DESCRIPTION:                                                         *
 *		It takes as argument the size of a file in bytes and returns *
 *	the actual size in blocks (512 bytes blks) that the file will take   *
 *	in the file system.                                                  *
 *                                                                           *
 *                                                                           *
 *****************************************************************************
 */

/* #include <sys/param.h> - included above, already */

#define DIRECT	10	/* Number of direct blocks */

#ifdef u370
	/* Number of pointers in an indirect block */
#define INDIR	(BSIZE/sizeof(daddr_t))
	/* Number of right shifts to divide by INDIR */
#define INSHFT	10
#else
	/* Number of pointers in an indirect block */
#define INDIR	128
	/* Number of right shifts to divide by INDIR */
#define INSHFT	7
#endif

long int nblock(size)
long int size;
{
	long blocks, tot;

	blocks = tot = (size + BSIZE - 1) >> BSHIFT;
	if(blocks > DIRECT)
		tot += ((blocks - DIRECT - 1) >> INSHFT) + 1;
	if(blocks > DIRECT + INDIR)
		tot += ((blocks - DIRECT - INDIR - 1) >> (INSHFT * 2)) + 1;
	if(blocks > DIRECT + INDIR + INDIR*INDIR)
		tot++;
	return(tot);
}
#endif
