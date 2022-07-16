/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:sys/mouse.h	1.1"
#ifndef _MOUSE
#define _MOUSE

/* Keyboard/Mouse Codes						*/
#define MSENABLE	0xD0	/* enable mouse			*/
#define MSDISABLE	0xD1	/* disable mouse		*/
#define BEGMOUSE	0xCE	/* mouse data follows		*/
#define BEGEMOUSE	0xCF	/* mouse data lost		*/
#define BEGKBD		0xDF	/* kbd data follows		*/

/* Bits in the mouse byte #1					*/
#define MBUTR	0x01			/* right button down	*/
#define MBUTM	0x02			/* middle button down	*/
#define MBUTL	0x04			/* left button down	*/
#define MSY	0x08			/* sign of Y		*/
#define MSX	0x10			/* sign of X		*/
#define MBUTALL	(MBUTL|MBUTM|MBUTR)	/* all the buttons	*/

/* Default scaling formulae					*/
#define MDPI	200			/* mouse dots/inch	*/
#define MVI	4			/* mouse vertical trav	*/
#define MHI	4			/* horizontal trav	*/
#define MXSCALE	((MHI*MDPI)/VIDWIDTH)	/* x scaling		*/
#define MYSCALE	((MVI*MDPI)/VIDHEIGHT)	/* y scaling		*/

/* Communication between interrupt and wproc level		*/
struct msinfo {
	long	physmx;
	long	physmy;
	char	mb;
};
#endif _MOUSE
