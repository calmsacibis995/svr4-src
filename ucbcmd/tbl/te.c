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
  
#ident	"@(#)ucbtbl:te.c	1.1.3.1"

 /* te.c: error message control, input line count */
# include "t..c"
error(s)
	char *s;
{
fprintf(stderr, "\n%s: line %d: %s\n", ifile, iline, s);
# ifdef unix
fprintf(stderr, "tbl quits\n");
exit(1);
# endif
# ifdef gcos
fprintf(stderr, "run terminated due to error condition detected by tbl preprocessor\n");
exit(0);
# endif
}
char *
errmsg(errnum)
	int errnum;
{
extern int sys_nerr;
extern char *sys_errlist[];
static char errmsgbuf[18];
if (errnum > sys_nerr)
	{
	sprintf(errmsgbuf, "Error %d", errnum);
	return (errmsgbuf);
	}
else
	return (sys_errlist[errnum]);
}
char *
gets1(s, len)
	char *s;
	int len;
{
char *p;
int nbl;
extern int errno;
while(len > 0)
	{
	iline++;
	while ((p = fgets(s,len,tabin))==0)
		{
		if (swapin()==0)
			return(0);
		}

	while (*s) s++;
	s--;
	if (*s == '\n') *s-- =0;
	else
		{
		if (!feof(tabin))
			{
			if (ferror(tabin))
				error(errmsg(errno));
			else
				error("Line too long");
			}
		}
	for(nbl=0; *s == '\\' && s>p; s--)
		nbl++;
	if (linstart && nbl % 2) /* fold escaped nl if in table */
		{
		s++;
		len -= s - p;
		continue;
		}
	break;
	}

return(p);
}
# define BACKMAX 500
char backup[BACKMAX];
char *backp = backup;
un1getc(c)
{
if (c=='\n')
	iline--;
*backp++ = c;
if (backp >= backup+BACKMAX)
	error("too much backup");
}
get1char()
{
int c;
if (backp>backup)
	c = *--backp;
else
	c=getc(tabin);
if (c== EOF) /* EOF */
	{
	if (swapin() ==0)
		error("unexpected EOF");
	c = getc(tabin);
	}
if (c== '\n')
	iline++;
return(c);
}
