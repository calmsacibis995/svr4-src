/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ttymon:stty.h	1.3.3.1"

#define ASYNC	1
#define FLOW	2
#define WINDOW	4
#define TERMIOS 8

struct	speeds {
	const char	*string;
	int	speed;
};

struct mds {
	const char	*string;
	long	set;
	long	reset;
};

