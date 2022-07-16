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

#ident	"@(#)ucbrefer:hunt5.c	1.1.3.1"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

extern char *soutput, *tagout, usedir[];
union ptr {
	unsigned *a;
	long *b;
};


result(master, nf, fc)
union ptr master;
FILE *fc;
{
	int i, c;
	char *s;
	long lp;
	extern int iflong;
	char res[100];

	for(i=0; i<nf; i++)
	{
		lp = iflong ? master.b[i] : master.a[i];
		fseek(fc,lp, 0);
		fgets(res, 100, fc);
		for(s=res; c = *s; s++)
			if (c== ';')
			{
				*s=0;
				break;
			}
		if (tagout !=0)
		{
			if (res[0]=='/' || usedir[0]==0)
				sprintf(tagout, "%s", res);
			else
				sprintf(tagout, "%s/%s", usedir, res);
			while (*tagout) tagout++;
		}
		else
		{
			if (res[0]!='/' || usedir[0]==0)
				printf("%s/", usedir);
			printf("%s\n", res);
		}
	}
}

long
gdate(f)
FILE *f;
{
	struct stat sb;
	fstat (f->_file, &sb);
	return  (sb . st_mtime);
}
