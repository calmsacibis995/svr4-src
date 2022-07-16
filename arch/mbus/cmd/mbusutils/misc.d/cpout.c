/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:cmd/mbusutils/misc.d/cpout.c	1.3"

static char cpout_copyright[] = "Copyright 1988 Intel Corp. 463024";

/*
** Cpio out only enough files to fill the specified number of blocks.
** Repeat until all of the files from the standard input have been placed
** onto the output media.
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>

extern int getopt();
extern unsigned int strlen();
extern void *malloc();
extern FILE *popen();
extern void exit();
extern void free();
extern void perror();
extern char *strcpy();
extern long atol();

#define MAXFNSIZE 512
#define HEADERLEN 77 /* length of header on each file */
#define TRAILERLEN 87 /* cpio trailer size */
#define DEFBLKSIZE 512    

static short medcount=1;
static long remains;
static long targetsize;
static long critsize = DEFBLKSIZE;

typedef struct QITEM {
	struct QITEM *qnext;
	long qsize;
	char *qname;
} QITEM;
static QITEM *qhead=(QITEM *) NULL;
static QITEM *qtail=(QITEM *) NULL;
static QITEM **qpos;

#define LENCMD	256						
#define LENOPTIONS	10
static char command[LENCMD];
static char *bufsize = (char *) NULL;		
static char *msg = (char *) NULL;		


main(argc,argv)
int argc;
char *argv[];
{
	long size;
	char name[MAXFNSIZE];
	short isdir;
	FILE *pipefd; /* initialize after checking args */
	FILE *ttyfd = fopen("/dev/tty","r");

	extern char *optarg;		
	extern int optind;		
	static int nextfile();
	static FILE *nextmedium();
	int c;								
	char options[LENOPTIONS];				
	short int aflag,cflag,Bflag,vflag,Vflag,errflag;	
	long blksize = DEFBLKSIZE ;  /* is 1024 if k flag is set    */
	struct stat statbuf;

	aflag=cflag=Bflag=vflag=Vflag=errflag=0;
	while ( (c = getopt(argc, argv, "acBvVkC:O:M:")) != -1)
		switch (c) {
		case 'a':
			aflag++;
			break;
		
		case 'c':
			cflag++;
			break;

		case 'B':
			Bflag++;
			break;

		case 'v':
			vflag++;
			break;

		case 'V':
			Vflag++;
			break;
		
		case 'k':
			blksize = critsize = 1024;
			break;
		
		case 'C':
			bufsize = optarg;
			break;
		
		case 'O':
			break;
		
		case 'M':
			msg = optarg;
			break;
		
		case '?':
			errflag++;
			break;
		}
	
	/* Prepare an option string to be appended to the cpio command */

	options[0] = '-';
	options[1] = 'o';
	c=2;
	if (aflag)
		options[c++] = 'a';
	if (cflag)
		options[c++] = 'c';
	if (Bflag)
		options[c++] = 'B';
	if (vflag)
		options[c++] = 'v';
	if (Vflag)
		options[c++] = 'V';
	options[c] = '\0';

	(void) sprintf(command,"cpio %s",options);

	if (bufsize) 
		(void) sprintf(command, "%s -C %s",command,bufsize);
	if (msg) 
		(void) sprintf(command, "%s -M \"%s\"",command,msg);

	if (optind == (argc-2)) {
		if ( (targetsize = (atol(argv[optind++])*blksize)) <= 0 )	
			errflag++;
		else {
			(void) sprintf(command, "%s -O %s",command,argv[optind]);
			if (stat(argv[optind++],&statbuf) < 0) {
				if (errno == ENOENT) {
					(void) fprintf(stderr,"%s: %s must be a special file\n",argv[0], argv[optind-1]);
				}
				else {
					perror(argv[optind-1]);
					(void) fprintf(stderr,"%s: Can't stat %s\n",argv[0],argv[optind-1]);
				}
				exit(1);
			}
			else {
				switch (statbuf.st_mode & S_IFMT) {	/* type of file */
					case S_IFCHR: /* character special */
					case S_IFBLK: /* block special */
					case S_IFIFO: /* fifo */
					case S_IFNAM: /* special named file */
						break;
					default:
						(void) fprintf(stderr,"%s: %s must be a special file\n",argv[0], argv[optind-1]);
						exit(1);
						break;
				}
			}
		}
	}
	else
		errflag++;

#if DEBUG
	(void) fprintf(stderr,"COMMAND is <%s>\n",command);
#endif
		
	if (errflag) {
		(void) fprintf(stderr,"Usage: %s [-acBvVk] [-C bufsize] [-M message] nblks specfile\n",argv[0]);
		exit(1);
	}

	pipefd = nextmedium(ttyfd,(FILE *) NULL);
	if (ttyfd == (FILE *) NULL) {
		(void) fprintf(stderr,"%s:No tty\n",argv[0]);
		exit(2);
	}
	if(!nextfile(name,&size,&isdir)) {
		(void) fprintf(stderr,"%s:Can't get first filename\n",argv[0]);
		exit(3);
	}

	for(;;) {
		if (feof(stdin)) { /* do we need to deque? */
			if ( *qpos ) {
				if ( (*qpos)->qsize > remains ) {
					/* no room here, get another        */
					/* there is potential wastage here  */
					/* because there might be something */
					/* in the queue that would fit, but */
					/* since we need another volume     */
					/* anyway, we won't worry about it  */
					pipefd = nextmedium(ttyfd,pipefd);
				}
			}
			else    break;
		}
		if (*qpos && (*qpos)->qsize <= remains) {
			QITEM *i = *qpos;
			if (pipefd) 
				(void) fprintf(pipefd,"%s\n",i->qname);
#if DEBUG
			(void) fprintf(stderr,"DEQUE %s %ld of %ld\n",i->qname,i->qsize,remains);
#endif
			remains -= i->qsize;
			free(i->qname);
			*qpos = i->qnext;
			free(i);
			if (qtail == i)
				for(qtail=qhead; qtail && qtail->qnext ; qtail=qtail->qnext);
		}
		else if (size <= remains) {
			if (pipefd) 
				(void) fprintf(pipefd,"%s\n",name);
#if DEBUG
			(void) fprintf(stderr,"INLIN %s %ld of %ld\n",name,size,remains);
#endif
			remains -= size;
			if(!nextfile(name,&size,&isdir)) {
				perror(name);
				(void) fprintf(stderr,"%s:",argv[0]);
			}
		}
		else if (isdir || (remains < critsize) )
			pipefd = nextmedium(ttyfd,pipefd);
		else if (size > targetsize ) {
			(void) fprintf(stderr,"%s, %d bytes, is too big for the media\n",name,size);
			if(!nextfile(name,&size,&isdir)) {
				perror(name);
				(void) fprintf(stderr,"%s:",argv[0]);
			}
		}
		else { /* enqueue file */
			char *s = malloc((unsigned)(strlen(name) + 1));
			QITEM *i = (QITEM *) malloc(sizeof(QITEM));
			if (s == NULL || i == (QITEM *) NULL ) {
				(void) fprintf(stderr,"%s: Not enough memory\n",argv[0]);
				exit(2);
			}
			(void) strcpy(s,name);
			i->qname = s;
			i->qsize = size;
			i->qnext = (QITEM *) NULL;
			if (qhead) {
				qtail->qnext = i;
				qtail = i;
			}
			else qtail = qhead = i;
#if DEBUG
			(void) fprintf(stderr,"ENQUE %s %ld of %ld\n",name,size,remains);
#endif
			if(!nextfile(name,&size,&isdir)) {
			    perror(name);
				(void) fprintf(stderr,"%s:",argv[0]);
			}
		}
	}
	(void) pclose(pipefd);
	(void) printf("%ld bytes remain\n",remains);
	(void) fprintf(stderr,"Volumes written: %d\n",--medcount);
	return 0;
}

static int
nextfile(newname,newsize,newisdir)
char *newname;
long *newsize;
short *newisdir;
{
	char buffer[MAXFNSIZE];
	char *p;
	struct stat statbuf;
	long size;

	if (fgets(buffer,sizeof buffer, stdin) == NULL) {
		if (ferror(stdin)) return 0;
		else return 1;
	}
  	/* Remove all unnecessary current directory prefixes */
	p = buffer;
	while (p[0]=='.' && p[1]=='/' && p[2]!= '\0') p+=2;

	/* Take off trailing newline */
  	p[strlen(p)-1] = '\0';
	(void) strcpy(newname,p);

	/* Determine if is a directory */
	if (stat(newname,&statbuf)) return 0;
	*newisdir = ((statbuf.st_mode&S_IFMT) == S_IFDIR);

	/* calculate size according to options */
	if (*newisdir) size = 0;
	else size = statbuf.st_size;
#ifdef DEBUG
	(void) fprintf(stderr,"%s size is %ld\n",newname,size);
#endif
	size += HEADERLEN + strlen(newname);
	*newsize = size;
	return 1;
}

static FILE *
nextmedium(ttyfd,pipefd)
FILE *ttyfd, *pipefd;
{
	int c,lastchar = 0;
	if (pipefd) {
		(void) pclose(pipefd);
		(void) printf("%ld bytes remain\n",remains);
	}
	(void) printf("\07 Insert Volume #%d: <RETURN> to continue, s<RETURN> to skip:",
		medcount++);
	while((c=fgetc(ttyfd)) != '\n') lastchar=c;
	remains = targetsize - TRAILERLEN;
	qpos = &qhead;
	if (lastchar=='s') {
#ifdef DEBUG
	(void) fprintf(stderr,"lastchar = s (%c) returning NULL!\n",(char)lastchar);
#endif
	return (FILE *) NULL;
	}
	return popen(command,"w");
}
