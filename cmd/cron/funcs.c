/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/* 	Portions Copyright(c) 1988, Sun Microsystems, Inc.	*/
/*	All Rights Reserved.					*/

#ident	"@(#)cron:funcs.c	1.4.1.3"

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <dirent.h>
#include <sys/dir.h>
#include <sys/fcntl.h>
#include <errno.h>
#include "cron.h"

#define CANTCD          "can't change directory to the at directory"
#define NOREADDIR       "can't read the at directory"


/*****************/
time_t
num(ptr)
/*****************/
char **ptr;
{
	time_t n=0;
	while (isdigit(**ptr)) {
		n = n*10 + (**ptr - '0');
		*ptr += 1; }
	return(n);
}


int dom[12]={31,28,31,30,31,30,31,31,30,31,30,31};

/*****************/
days_btwn(m1,d1,y1,m2,d2,y2)
/*****************/
int m1,d1,y1,m2,d2,y2;
{
	/* calculate the number of "full" days in between m1/d1/y1 and m2/d2/y2.
	   NOTE: there should not be more than a year separation in the dates.
		 also, m should be in 0 to 11, and d should be in 1 to 31.    */
	
	int days,m;

	if ((m1==m2) && (d1==d2) && (y1==y2)) return(0);
	if ((m1==m2) && (d1<d2)) return(d2-d1-1);
	/* the remaining dates are on different months */
	days = (days_in_mon(m1,y1)-d1) + (d2-1);
	m = (m1 + 1) % 12;
	while (m != m2) {
		if (m==0) y1++;
		days += days_in_mon(m,y1);
		m = (m + 1) % 12; }
	return(days);
}


/*****************/
days_in_mon(m,y)
/*****************/
int m,y;
{
	/* returns the number of days in month m of year y
	   NOTE: m should be in the range 0 to 11	*/

	return( dom[m] + (((m==1)&&((y%4)==0)) ? 1:0 ));
}

/*****************/
char
*xmalloc(size)
/*****************/
int size;
{
	char *p;
	char *malloc();

	if((p=malloc(size)) == NULL) {
		printf("cannot allocate %d bype of space\n",size);
		fflush(stdout);
		exit(55);
	}
	return p;
}


/********************************/
sendmsg(action,login,fname, etype)
/********************************/
        char action;
	char *login;
        char *fname;
	char etype;
{
        static int msgfd = -2;
        struct message *pmsg;
        register int i;

        pmsg = &msgbuf;
        if (msgfd == -2) {
                if ((msgfd = open(FIFO,O_WRONLY|O_NDELAY)) < 0) {
                        if (errno == ENXIO || errno == ENOENT)
                                fprintf(stderr,"cron may not be running - call your system administrator\n");
                        else
                                fprintf(stderr,"error in message queue open");
                        return;
                }
        }
        pmsg->etype = etype;
        pmsg->action = action;
        (void) strncpy(pmsg->fname, fname, FLEN);
        (void) strncpy(pmsg->logname, login, LLEN);
        if ((i = write(msgfd, pmsg, sizeof(struct message))) < 0)
                fprintf(stderr,"error in message send");
        else if (i != sizeof(struct message))
                fprintf(stderr,"error in message send: Premature EOF\n");
}

/*****************/
char
*errmsg(errnum)
/*****************/
int errnum;
{
        extern int sys_nerr;
        extern char *sys_errlist[];
        static char msgbuf[6+10+1];

        if (errnum < 0 || errnum > sys_nerr) {
                (void) sprintf(msgbuf, "Error %d", errnum);
                return (msgbuf);
        } else
                return (sys_errlist[errnum]);
}



/***********************/
int
filewanted(direntry)
/***********************/
        struct dirent *direntry;
{
        char *p;
        char *q;
        register char c;
        extern time_t num();

        p = direntry->d_name;
        (void) num(&p);
        if (p == direntry->d_name)
                return (0);     /* didn't start with a number */
        if (*p++ != '.')
                return (0);     /* followed by a period */
        c = *p++;
        if (c < 'a' || c > 'z') 
                return (0);     /* followed by a queue name */
        return (1);
}

/*
 * Scan the directory dirname calling select to make a list of selected
 * directory entries then sort using qsort and compare routine dcomp.
 * Returns the number of entries and a pointer to a list of pointers to
 * struct direct (through namelist). Returns -1 if there were any errors.
 */


#ifdef DIRSIZ
#undef DIRSIZ

#endif
#define DIRSIZ(dp) \
	(dp)->d_reclen

ascandir(dirname, namelist, select, dcomp)
	char *dirname;
	struct dirent *(*namelist[]);
	int (*select)(), (*dcomp)();
{
	register struct dirent *d, *p, **names;
	register int nitems;
	register char *cp1, *cp2;
	struct stat stb;
	long arraysz;
	DIR *dirp;
	char *malloc();

	if ((dirp = opendir(dirname)) == NULL)
		return(-1);
	if (fstat(dirp->dd_fd, &stb) < 0)
		return(-1);

	/*
	 * estimate the array size by taking the size of the directory file
	 * and dividing it by a multiple of the minimum size entry. 
	 */
	arraysz = (stb.st_size / 24);
	names = (struct dirent **)malloc(arraysz * sizeof(struct dirent *));
	if (names == NULL)
		return(-1);

	nitems = 0;
	while ((d = readdir(dirp)) != NULL) {
		if (select != NULL && !(*select)(d)) 
			continue;	/* just selected names */

		/*
		 * Make a minimum size copy of the data
		 */
		p = (struct dirent *)malloc(DIRSIZ(d));
		if (p == NULL) 
			return(-1); 
		p->d_ino = d->d_ino;
		p->d_reclen = d->d_reclen;
		/*p->d_namlen = d->d_namlen;*/
		for (cp1 = p->d_name, cp2 = d->d_name; *cp1++ = *cp2++; );
		/*
		 * Check to make sure the array has space left and
		 * realloc the maximum size.
		 */
		if (++nitems >= arraysz) {
			if (fstat(dirp->dd_fd, &stb) < 0) 
				return(-1);	/* just might have grown */
			arraysz = stb.st_size / 12;
			names = (struct dirent **)realloc((char *)names,
				arraysz * sizeof(struct dirent *));
			if (names == NULL)
				return(-1); 
		}
		names[nitems-1] = p;
	}
	closedir(dirp);
	if (nitems && dcomp != NULL)
		qsort(names, nitems, sizeof(struct dirent *), dcomp);
	*namelist = names;
	return(nitems);
}
