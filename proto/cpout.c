/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)proto:cpout.c	1.3"

/*
** Cpio out only enough files to fill one floppy,
** or argv[1] blocks.
** Repeat until all of the files from standard input have been put onto a
** floppy.
*/

#define forward extern
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
extern char *malloc();
extern FILE *popen();
extern long lseek();
extern void exit();
extern void free();
extern void perror();
extern char *strcpy();
forward FILE *nextfloppy();

#define MAXLEN 2400*512 /* for quad-density floppies */
#define MAXFNSIZE 512
#define HEADERLEN 77 /* length of header on each file */
#define CRITSIZE 512 /* how much it is ok to waste without searching further */
#define TRAILERLEN 87 /* cpio trailer size */

short fcount=1;
long remains;
long targetsize = MAXLEN;

typedef struct QITEM {
	struct QITEM *qnext;
	long qsize;
	char *qname;
} QITEM;
QITEM *qhead=(QITEM *) NULL;
QITEM *qtail=(QITEM *) NULL;
QITEM **qpos;

main(argc,argv)
int argc;
char *argv[];
{
	long size;
	char name[MAXFNSIZE];
	short isdir;
	FILE *pipefd; /* initialize after checking args */
	FILE *ttyfd = fopen("/dev/tty","r");
	FILE *countfd = fopen("floppy.count","w");

	switch (argc) {
		case 1: break;
		case 2: if ( (targetsize = (atol(argv[1])*512)) > 0 ) {
			  break;
			  }
			  /* else fall through */
		default:
		       fprintf(stderr,"Usage: %s [nblks]\n",argv[0]);
		       exit(1);
	}
	pipefd = nextfloppy(ttyfd,(FILE *) NULL);
	if (ttyfd == (FILE *) NULL) {
		fprintf(stderr,"%s:No tty\n",argv[0]);
		exit(2);
	}
	if(!nextfile(name,&size,&isdir)) {
		fprintf(stderr,"%s:Can't get first filename\n",argv[0]);
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
					pipefd = nextfloppy(ttyfd,pipefd);
				}
			}
			else    break;
		}
		if (*qpos && (*qpos)->qsize <= remains) {
			QITEM *i = *qpos;
			if (pipefd) fprintf(pipefd,"%s\n",i->qname);
#if DEBUG
			fprintf(stderr,"DEQUE %s %ld of %ld\n",i->qname,i->qsize,remains);
#endif
			remains -= i->qsize;
			free(i->qname);
			*qpos = i->qnext;
			free(i);
			if (qtail == i)
				for(qtail=qhead; qtail && qtail->qnext ; qtail=qtail->qnext);
		}
		else if (size <= remains) {
			if (pipefd) fprintf(pipefd,"%s\n",name);
#if DEBUG
			fprintf(stderr,"INLIN %s %ld of %ld\n",name,size,remains);
#endif
			remains -= size;
			if(!nextfile(name,&size,&isdir)) {
				fprintf(stderr,"%s:",argv[0]);
				perror(name);
			}
		}
		else if (isdir || (remains < CRITSIZE) )
			pipefd = nextfloppy(ttyfd,pipefd);
		else if (size > targetsize ) {
			fprintf(stderr,"%s, %d bytes, is too big for the media\n",name,size);
			if(!nextfile(name,&size,&isdir)) {
				fprintf(stderr,"%s:",argv[0]);
				perror(name);
			}
		}
		else { /* enqueue file */
			char *s = malloc((unsigned)(strlen(name) + 1));
			QITEM *i = (QITEM *) malloc(sizeof(QITEM));
			if (s == NULL || i == (QITEM *) NULL ) {
				fprintf(stderr,"%s: Not enough memory\n",argv[0]);
				exit(2);
			}
			strcpy(s,name);
			i->qname = s;
			i->qsize = size;
			i->qnext = (QITEM *) NULL;
			if (qhead) {
				qtail->qnext = i;
				qtail = i;
			}
			else qtail = qhead = i;
#if DEBUG
			fprintf(stderr,"ENQUE %s %ld of %ld\n",name,size,remains);
#endif
			if(!nextfile(name,&size,&isdir)) {
				fprintf(stderr,"%s:",argv[0]);
			    /*    perror(name); */
			}
		}
	}
	pclose(pipefd);
	fprintf(stderr,"%ld bytes remain\n",remains);
	fprintf(countfd,"%d",--fcount);
	return 0;
}


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
	strcpy(newname,p);

	/* Determine if is a directory */
	if (stat(newname,&statbuf)) return 0;
	*newisdir = ((statbuf.st_mode&S_IFMT) == S_IFDIR);

	/* calculate size according to options */
	if (*newisdir) size = 0;
	else size = statbuf.st_size;
#ifdef DEBUG
	fprintf(stderr,"%s size is %ld\n",newname,size);
#endif
	size += HEADERLEN + strlen(newname);
	*newsize = size;
	return 1;
}


FILE *
nextfloppy(ttyfd,pipefd)
FILE *ttyfd, *pipefd;
{
	int c,lastchar = 0;
	if (pipefd) {
		pclose(pipefd);
		fprintf(stderr,"%ld bytes remain\n",remains);
	}
	lseek(1,0L,0);
	fprintf(stderr,"\07 Insert Volume #%d: <RETURN> to continue, s<RETURN> to skip:",
		fcount++);
	while((c=fgetc(ttyfd)) != '\n') lastchar=c;
	remains = targetsize - TRAILERLEN;
	qpos = &qhead;
	if (lastchar=='s') {
#ifdef DEBUG
	fprintf(stderr,"lastchar = s (%c) returning NULL!\n",(char)lastchar);
#endif
	return (FILE *) NULL;
	}
	return popen("cpio -ocB","w");
}
