/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbplot:libplot/t450/open.c	1.1.3.1"

/*
 * Copyright (c) 1983, 1984 1985, 1986, 1987, 1988, Sun Microsystems, Inc.
 * All Rights Reserved.
 */

#
#include <sgtty.h>
/* gsi plotting output routines */
# define DOWN 012
# define UP 013
# define LEFT 010
# define RIGHT 040
# define BEL 007
# define ACK 006
#define PLOTIN 063
#define PLOTOUT 064
# define CR 015
# define FF 014
# define VERTRESP 48
# define HORZRESP 60.
# define HORZRES 6.
# define VERTRES 8.
/* down is line feed, up is reverse oyne feed,
   left is bwckspace, right is space.  48 points per inch
   vertically, 60 horizontally */

int xnow, ynow;
int OUTF;
struct sgttyb ITTY, PTTY;
float HEIGHT = 6.0, WIDTH = 6.0, OFFSET = 0.0;
int xscale, xoffset, yscale;
float botx = 0., boty = 0., obotx = 0., oboty = 0.;
float scalex = 1., scaley = 1.;

openpl ()
{
	int reset();
		xnow = ynow = 0;
		OUTF = 1;
		printf("\r");
		gtty(OUTF, &ITTY);
		signal (2, reset);
		PTTY = ITTY;
		PTTY.sg_flags &= ~CRMOD;	/* don't map lf */
		stty(OUTF,&PTTY);
		/* initialize constants */
		xscale  = 4096./(HORZRESP * WIDTH);
		yscale = 4096 /(VERTRESP * HEIGHT);
		xoffset = OFFSET * HORZRESP;
		return;
}

openvt(){
openpl();
}
