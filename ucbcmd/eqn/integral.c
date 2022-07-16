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
  
#ident	"@(#)ucbeqn:integral.c	1.1.3.1"

# include "e.h"
# include "e.def"

integral(p, p1, p2) {
#ifndef	NEQN
	if (p1 != 0)
		printf(".ds %d \\h'-0.4m'\\v'0.4m'\\*(%d\\v'-0.4m'\n", p1, p1);
	if (p2 != 0)
		printf(".ds %d \\v'-0.3m'\\*(%d\\v'0.3m'\n", p2, p2);
#endif
	if (p1 != 0 && p2 != 0)
		shift2(p, p1, p2);
	else if (p1 != 0)
		bshiftb(p, SUB, p1);
	else if (p2 != 0)
		bshiftb(p, SUP, p2);
	if(dbg)printf(".\tintegral: S%d; h=%d b=%d\n", 
		p, eht[p], ebase[p]);
	lfont[p] = ROM;
}

setintegral() {
	char *f;

	yyval = oalloc();
	f = "\\(is";
#ifndef NEQN
	printf(".ds %d \\s%d\\v'.1m'\\s+4%s\\s-4\\v'-.1m'\\s%d\n", 
		yyval, ps, f, ps);
	eht[yyval] = VERT( (((ps+4)*12)/10)*6 );
	ebase[yyval] = VERT( (ps*6*3)/10 );
#else NEQN
	printf(".ds %d %s\n", yyval, f);
	eht[yyval] = VERT(2);
	ebase[yyval] = 0;
#endif NEQN
	lfont[yyval] = rfont[yyval] = ROM;
}
