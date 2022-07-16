/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved. The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * Copyright (c) 1983, 1984 1985, 1986, 1987, 1988, Sun Microsystems, Inc.
 * All Rights Reserved.
 */

#ident	"@(#)ucbrefer:hunt8.c	1.1.3.1"

#include <stdio.h>
#include <assert.h>
#define unopen(fil) {if (fil!=NULL) {fclose(fil); fil=NULL;}}

extern long indexdate, gdate();
extern FILE *iopen();
runbib (s)
char *s;
{
	/* make a file suitable for fgrep */
	char tmp[200];
	sprintf(tmp, "/usr/ucblib/reftools/mkey %s >%s.ig", s,s);
	system(tmp);
}

makefgrep(indexname)
char *indexname;
{
	FILE *fa, *fb;
	if (ckexist(indexname, ".ig"))
	{
		/* existing gfrep -type index */
# if D1
		fprintf(stderr, "found fgrep\n");
# endif
		fa = iopen(indexname, ".ig");
		fb = iopen(indexname, "");
		if (gdate(fb)>gdate(fa))
		{
			if (fa!=NULL)
				fclose(fa);
			runbib(indexname);
			fa= iopen(indexname, ".ig");
		}
		indexdate = gdate(fa);
		unopen(fa); 
		unopen(fb);
	}
	else
		if (ckexist(indexname, ""))
		{
			/* make fgrep */
# if D1
			fprintf(stderr, "make fgrep\n");
# endif
			runbib(indexname);
			time(&indexdate);
		}
		else /* failure */
		return(0);
	return(1); /* success */
}

ckexist(s, t)
char *s, *t;
{
	char fnam[100];
	strcpy (fnam, s);
	strcat (fnam, t);
	return (access(fnam, 04) != -1);
}

FILE *
iopen(s, t)
char *s, *t;
{
	char fnam[100];
	FILE *f;
	strcpy (fnam, s);
	strcat (fnam, t);
	f = fopen (fnam, "r");
	if (f == NULL)
	{
		err("Missing expected file %s", fnam);
		exit(1);
	}
	return(f);
}
