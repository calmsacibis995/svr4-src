/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */

#ident	"@(#)fmli:sys/test.c	1.3"

#include <sys/types.h>		/* EFT abs k16 */
#include	"test.h"
#undef BUFSIZ
#include	<stdio.h>
#include	"wish.h"
#include <sys/stat.h>

#define	prs(a)		putastr(a, out)

#define	failed(a, b)	mess_temp(b)

typedef struct io_struct {
	int	flags;
	union {
		FILE	*fp;
		struct {
			char	*val;
			int	count;
			int	pos;
		} str;
	} mu;
	struct io_struct	*next;
} IOSTRUCT;

int	ap, ac;
char	**av;

cmd_test(argn, com, in, out, err)
char	*com[];
int	argn;
IOSTRUCT	*in;
IOSTRUCT	*out;
IOSTRUCT	*err;
{
	ac = argn;
	av = com;
	ap = 1;

	if (eq(com[0],"["))
	{
		if (!eq(com[--ac], "]"))
			failed("test", "] missing");
	}

	com[ac] = 0;

	if (ac <= 1)
		return(FAIL);

	return(exp() ? SUCCESS : FAIL);
}

/*
 * CF is a "strcmp" function referenced in test.h .... 
 */ 
cf(s1, s2)
register char *s1, *s2;
{
	while (*s1++ == *s2)
		if (*s2++ == 0)
			return(0);
	return(*--s1 - *s2);
}
char *
nxtarg(mt)
{
	if (ap >= ac)
	{
		if (mt)
		{
			ap++;
			return(0);
		}
		failed("test", "argument expected");
	}
	return(av[ap++]);
}

exp()
{
	int	p1;
	char	*p2;

	p1 = e1();
	p2 = nxtarg(1);
	if (p2 != 0)
	{
		if (eq(p2, "-o"))
			return(p1 | exp());

		if (eq(p2, "]") && !eq(p2, ")"))
			failed("test", "test syntax error");
	}
	ap--;
	return(p1);
}

e1()
{
	int	p1;
	char	*p2;

	p1 = e2();
	p2 = nxtarg(1);

	if ((p2 != 0) && eq(p2, "-a"))
		return(p1 & e1());
	ap--;
	return(p1);
}

e2()
{
	if (eq(nxtarg(0), "!"))
		return(!e3());
	ap--;
	return(e3());
}

e3()
{
	int	p1;
	register char	*a;
	char	*p2;
	long	atol();
	long	int1, int2;

	a = nxtarg(0);
	if (eq(a, "("))
	{
		p1 = exp();
		if (!eq(nxtarg(0), ")"))
			failed("test",") expected");
		return(p1);
	}
	p2 = nxtarg(1);
	ap--;
	if ((p2 == 0) || (!eq(p2, "=") && !eq(p2, "!=")))
	{
		if (eq(a, "-r"))
			return(tio(nxtarg(0), 4));
		if (eq(a, "-w"))
			return(tio(nxtarg(0), 2));
		if (eq(a, "-x"))
			return(tio(nxtarg(0), 1));
		if (eq(a, "-d"))
			return(filtyp(nxtarg(0), S_IFDIR));
		if (eq(a, "-c"))
			return(filtyp(nxtarg(0), S_IFCHR));
		if (eq(a, "-b"))
			return(filtyp(nxtarg(0), S_IFBLK));
		if (eq(a, "-f"))
			return(filtyp(nxtarg(0), S_IFREG));
		if (eq(a, "-u"))
			return(ftype(nxtarg(0), S_ISUID));
		if (eq(a, "-g"))
			return(ftype(nxtarg(0), S_ISGID));
		if (eq(a, "-k"))
			return(ftype(nxtarg(0), S_ISVTX));
		if (eq(a, "-p"))
			return(filtyp(nxtarg(0),S_IFIFO));
   		if (eq(a, "-s"))
			return(fsizep(nxtarg(0)));
		if (eq(a, "-t"))
		{
			if (ap >= ac)		/* no args */
				return(isatty(1));
			else if (eq((a = nxtarg(0)), "-a") || eq(a, "-o"))
			{
				ap--;
				return(isatty(1));
			}
			else
				return(isatty(atoi(a)));
		}
		if (eq(a, "-n"))
			return(!eq(nxtarg(0), ""));
		if (eq(a, "-z"))
			return(eq(nxtarg(0), ""));
	}

	p2 = nxtarg(1);
	if (p2 == 0)
		return(!eq(a, ""));
	if (eq(p2, "-a") || eq(p2, "-o"))
	{
		ap--;
		return(!eq(a, ""));
	}
	if (eq(p2, "="))
		return(eq(nxtarg(0), a));
	if (eq(p2, "!="))
		return(!eq(nxtarg(0), a));
	int1 = atol(a);
	int2 = atol(nxtarg(0));
	if (eq(p2, "-eq"))
		return(int1 == int2);
	if (eq(p2, "-ne"))
		return(int1 != int2);
	if (eq(p2, "-gt"))
		return(int1 > int2);
	if (eq(p2, "-lt"))
		return(int1 < int2);
	if (eq(p2, "-ge"))
		return(int1 >= int2);
	if (eq(p2, "-le"))
		return(int1 <= int2);

	failed("test", "test - unknown operator");
/* NOTREACHED */
}

tio(a, f)
char	*a;
int	f;
{
	if (access(a, f) == 0)
		return(1);
	else
		return(0);
}

ftype(f, field)
char	*f;
int	field;
{
	struct stat statb;

	if (stat(f, &statb) < 0)
		return(0);
	if ((statb.st_mode & field) == field)
		return(1);
	return(0);
}

filtyp(f,field)
char	*f;
int field;
{
	struct stat statb;

	if (stat(f, &statb) < 0)
		return(0);
	if ((statb.st_mode & S_IFMT) == field)
		return(1);
	else
		return(0);
}



fsizep(f)
char	*f;
{
	struct stat statb;

	if (stat(f, &statb) < 0)
		return(0);
	return(statb.st_size > 0);
}

