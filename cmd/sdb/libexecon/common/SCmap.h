/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:libexecon/common/SCmap.h	1.1"
#ifndef SCmap_h
#define SCmap_h

#define bit(x)	(1<<((x)-1))

#define SCMAX	3

struct SCmap {
	int	word;
	int	shift;
};

static SCmap sclist[] =	{	{ 0, 31 },	// indir
				{ 0, 30 },	// exit
				{ 0, 29 },	// exit
				{ 0, 28 },	// exit
				{ 0, 27 },	// exit
				{ 0, 26 },	// exit
				{ 0, 25 },	// exit
				{ 0, 24 },	// exit
				{ 0, 23 },	// exit
				{ 0, 22 },	// exit
				{ 0, 21 },	// exit
				{ 0, 20 },	// exit
				{ 0, 19 },	// exit
				{ 0, 18 },	// exit
				{ 0, 17 },	// exit
				{ 0, 16 },	// exit
				{ 0, 15 },	// exit
				{ 0, 14 },	// exit
				{ 0, 13 },	// exit
				{ 0, 12 },	// exit
				{ 0, 11 },	// exit
				{ 0, 10 },	// exit
				{ 0,  9 },	// exit
				{ 0,  8 },	// exit
				{ 0,  7 },	// exit
				{ 0,  6 },	// exit
				{ 0,  5 },	// exit
				{ 0,  4 },	// exit
				{ 0,  3 },	// exit
				{ 0,  2 },	// exit
				{ 0,  1 },	// exit
				{ 0,  0 },	// exit
				{ 1, 31 },	// indir
				{ 1, 30 },	// exit
				{ 1, 29 },	// exit
				{ 1, 28 },	// exit
				{ 1, 27 },	// exit
				{ 1, 26 },	// exit
				{ 1, 25 },	// exit
				{ 1, 24 },	// exit
				{ 1, 23 },	// exit
				{ 1, 22 },	// exit
				{ 1, 21 },	// exit
				{ 1, 20 },	// exit
				{ 1, 19 },	// exit
				{ 1, 18 },	// exit
				{ 1, 17 },	// exit
				{ 1, 16 },	// exit
				{ 1, 15 },	// exit
				{ 1, 14 },	// exit
				{ 1, 13 },	// exit
				{ 1, 12 },	// exit
				{ 1, 11 },	// exit
				{ 1, 10 },	// exit
				{ 1,  9 },	// exit
				{ 1,  8 },	// exit
				{ 1,  7 },	// exit
				{ 1,  6 },	// exit
				{ 1,  5 },	// exit
				{ 1,  4 },	// exit
				{ 1,  3 },	// exit
				{ 1,  2 },	// exit
				{ 1,  1 },	// exit
				{ 1,  0 },	// exit
				{ 2, 31 },	// indir
				{ 2, 30 },	// exit
				{ 2, 29 },	// exit
				{ 2, 28 },	// exit
				{ 2, 27 },	// exit
				{ 2, 26 },	// exit
				{ 2, 25 },	// exit
				{ 2, 24 },	// exit
				{ 2, 23 },	// exit
				{ 2, 22 },	// exit
				{ 2, 21 },	// exit
				{ 2, 20 },	// exit
				{ 2, 19 },	// exit
				{ 2, 18 },	// exit
				{ 2, 17 },	// exit
				{ 2, 16 },	// exit
				{ 2, 15 },	// exit
				{ 2, 14 },	// exit
				{ 2, 13 },	// exit
				{ 2, 12 },	// exit
				{ 2, 11 },	// exit
				{ 2, 10 },	// exit
				{ 2,  9 },	// exit
				{ 2,  8 },	// exit
				{ 2,  7 },	// exit
				{ 2,  6 },	// exit
				{ 2,  5 },	// exit
				{ 2,  4 },	// exit
				{ 2,  3 },	// exit
				{ 2,  2 },	// exit
				{ 2,  1 },	// exit
				{ 2,  0 },	// exit
				{ -1, -1 }	// no more
			};

#endif

// end of SCmap.h

