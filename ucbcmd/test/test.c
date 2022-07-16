/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/* Copyright (c) 1980 Regents of the University of California.
 * All rights reserved. The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
    
/*
 * Copyright (c) 1983, 1984 1985, 1986, 1987, 1988, Sun Microsystems, Inc.
 * All Rights Reserved.
 */

#ident	"@(#)ucbtest:test.c	1.1.1.1"

/*
 * test expression
 * [ expression ]
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#define EQ(a,b)	((strcmp(a,b)==0))

int	ap;
int	ac;
char	**av;

char	*nxtarg();

main(argc, argv)
char *argv[];
{
	int status;

	ac = argc; av = argv; ap = 1;
	if(EQ(argv[0],"[")) {
		if(!EQ(argv[--ac],"]"))
			synbad("] missing","");
	}
	argv[ac] = 0;
	if (ac<=1) exit(1);
	status = (exp()?0:1);
	if (nxtarg(1)!=0)
		synbad("too many arguments","");
	exit(status);
}

char *nxtarg(mt) {

	if (ap>=ac) {
		if(mt) {
			ap++;
			return(0);
		}
		synbad("argument expected","");
	}
	return(av[ap++]);
}

exp() {
	int p1;
	char *p2;

	p1 = e1();
	p2 = nxtarg(1);
	if (p2 != 0) {
		if (EQ(p2, "-o"))
			return(p1 | exp());
		if (EQ(p2, "]"))
			synbad("syntax error","");
	}
	ap--;
	return(p1);
}

e1() {
	int p1;
	char *p2;

	p1 = e2();
	p2 = nxtarg(1);
	if ((p2 != 0) && EQ(p2, "-a"))
		return(p1 & e1());
	ap--;
	return(p1);
}

e2() {
	if (EQ(nxtarg(0), "!"))
		return(!e3());
	ap--;
	return(e3());
}

e3() {
	int p1;
	register char *a;
	char *p2;
	int int1, int2;

	a=nxtarg(0);
	if(EQ(a, "(")) {
		p1 = exp();
		if(!EQ(nxtarg(0), ")")) synbad(") expected","");
		return(p1);
	}
	p2 = nxtarg(1);
	ap--;
	if ((p2 == 0) || (!EQ(p2, "=") && !EQ(p2, "!="))) {
		if(EQ(a, "-r"))
			return(tio(nxtarg(0), 4));

		if(EQ(a, "-w"))
			return(tio(nxtarg(0), 2));

		if(EQ(a, "-x"))
			return(tio(nxtarg(0), 1));

		if(EQ(a, "-d"))
			return(filtyp(nxtarg(0), S_IFDIR));

		if(EQ(a, "-c"))
			return(filtyp(nxtarg(0), S_IFCHR));

		if(EQ(a, "-b"))
			return(filtyp(nxtarg(0), S_IFBLK));

		if(EQ(a, "-f")) {
			struct stat statb;

			return(stat(nxtarg(0), &statb) >= 0 &&
			    (statb.st_mode & S_IFMT) != S_IFDIR);
		}

		if(EQ(a, "-h"))
			return(filtyp(nxtarg(0), S_IFLNK));

		if(EQ(a, "-u"))
			return(ftype(nxtarg(0), S_ISUID));

		if(EQ(a, "-g"))
			return(ftype(nxtarg(0), S_ISGID));

		if(EQ(a, "-k"))
			return(ftype(nxtarg(0), S_ISVTX));

		if(EQ(a, "-p"))
#ifdef S_IFIFO
			return(filtyp(nxtarg(0), S_IFIFO));
#else
			return(nxtarg(0), 0);
#endif

		if(EQ(a, "-s"))
			return(fsizep(nxtarg(0)));

		if(EQ(a, "-t"))
			if(ap>=ac)
				return(isatty(1));
			else if (EQ((a = nxtarg(0)), "-a") || EQ(a, "-o")) {
				ap--;
				return(isatty(1));
			} else
				return(isatty(atoi(a)));

		if(EQ(a, "-n"))
			return(!EQ(nxtarg(0), ""));
		if(EQ(a, "-z"))
			return(EQ(nxtarg(0), ""));
	}

	p2 = nxtarg(1);
	if (p2==0)
		return(!EQ(a,""));
	if (EQ(p2, "-a") || EQ(p2, "-o")) {
		ap--;
		return(!EQ(a,""));
	}
	if(EQ(p2, "="))
		return(EQ(nxtarg(0), a));

	if(EQ(p2, "!="))
		return(!EQ(nxtarg(0), a));

	int1 = atoi(a);
	int2 = atoi(nxtarg(0));
	if(EQ(p2, "-eq"))
		return(int1==int2);
	if(EQ(p2, "-ne"))
		return(int1!=int2);
	if(EQ(p2, "-gt"))
		return(int1>int2);
	if(EQ(p2, "-lt"))
		return(int1<int2);
	if(EQ(p2, "-ge"))
		return(int1>=int2);
	if(EQ(p2, "-le"))
		return(int1<=int2);

	synbad("unknown operator ",p2);
	/* NOTREACHED */
}

tio(a, f)
char *a;
int f;
{

	if (access(a, f) == 0)
		return(1);
	else
		return(0);
}

ftype(f, field)
char *f;
int field;
{
	struct stat statb;

	if(stat(f,&statb)<0)
		return(0);
	if((statb.st_mode&field)==field)
		return(1);
	return(0);
}

filtyp(f, field)
char *f;
int field;
{
	struct stat statb;

	if (field == S_IFLNK) {
		if(lstat(f,&statb)<0)
			return(0);
	} else {
		if(stat(f,&statb)<0)
			return(0);
	}
	if((statb.st_mode&S_IFMT)==field)
		return(1);
	else
		return(0);
}

fsizep(f)
char *f;
{
	struct stat statb;

	if(stat(f,&statb)<0)
		return(0);
	return(statb.st_size>0);
}

synbad(s1,s2)
char *s1, *s2;
{
	(void) write(2, "test: ", 6);
	(void) write(2, s1, strlen(s1));
	(void) write(2, s2, strlen(s2));
	(void) write(2, "\n", 1);
	exit(255);
}

length(s)
	char *s;
{
	char *es=s;
	while(*es++);
	return(es-s-1);
}
