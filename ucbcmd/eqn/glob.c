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
  
#ident	"@(#)ucbeqn:glob.c	1.1.3.1"

#include "e.h"

int	dbg;	/* debugging print if non-zero */
int	lp[80];	/* stack for things like piles and matrices */
int	ct;	/* pointer to lp */
int	used[100];	/* available registers */
int	ps;	/* default init point size */
int	deltaps	= 3;	/* default change in ps */
int	gsize	= 10;	/* default initial point size */
int	gfont	= ITAL;	/* italic */
int	ft;	/* default font */
FILE	*curfile;	/* current input file */
int	ifile;
int	linect;	/* line number in file */
int	eqline;	/* line where eqn started */
int	svargc;
char	**svargv;
int	eht[100];
int	ebase[100];
int	lfont[100];
int	rfont[100];
int	eqnreg;	/* register where final string appears */
int	eqnht;	/* inal height of equation */
int	lefteq	= '\0';	/* left in-line delimiter */
int	righteq	= '\0';	/* right in-line delimiter */
int	lastchar;	/* last character read by lex */
int	markline	= 0;	/* 1 if this EQ/EN contains mark or lineup */
