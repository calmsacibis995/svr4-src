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

#ident	"@(#)ucbrefer:inv6.c	1.1.3.1"

#include <stdio.h>
#include <assert.h>

whash(ft, fa, fb, nhash, iflong, ptotct, phused)
FILE *fa, *fb, *ft;
int nhash, *phused;
long *ptotct;
{
	char line[100];
	int hash = 0, hused = 0;
	long totct = 0L;
	int ct = 0;
	long point;
	long opoint = -1;
	int m;
	int k; 
	long lp;
	long *hpt;
	int *hfreq = NULL;

	hpt = (long *) calloc (nhash+1, sizeof(*hpt));
	assert (hpt != NULL);
	hfreq = (int *) calloc (nhash, sizeof(*hfreq));
	assert (hfreq != NULL);
	hpt[0] = 0;
	lp= 0;
	while (fgets(line, 100, ft))
	{
		totct++;
		sscanf(line, "%d %ld", &k, &point);
		if (hash < k)
		{
			hused++;
			if (iflong) putl(-1L, fb); 
			else putw(-1, fb);
			hfreq[hash]=ct;
			while (hash<k)
			{
				hpt[++hash] = lp;
				hfreq[hash] = 0;
			}
			hpt[hash] = lp += iflong? sizeof(long) : sizeof(int);
			opoint= -1;
			ct=0;
		}
		if (point!=opoint)
		{
			if (iflong)
				putl(opoint=point, fb);
			else
				putw( (int)(opoint=point), fb);
			lp += iflong? sizeof(long) : sizeof(int);
			ct++;
		}
	}
	if (iflong) putl(-1L, fb); 
	else putw(-1,fb);
	while (hash<nhash)
		hpt[++hash]=lp;
	fwrite(&nhash, sizeof(nhash), 1, fa);
	fwrite(&iflong, sizeof(iflong), 1, fa);
	fwrite(hpt, sizeof(*hpt), nhash, fa);
	fwrite (hfreq, sizeof(*hfreq), nhash, fa);
	*ptotct = totct;
	*phused = hused;
}

putl(ll, f)
long ll;
FILE *f;
{
	putw(ll, f);
}

long
getl(f)
FILE *f;
{
	return(getw(f));
}
