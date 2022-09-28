/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */

#ident	"@(#)fmli:inc/mio.h	1.2"

#ifndef	CONT
/* compatibility w/ getok */
union tok
{
	char	*s;
	int	b;
};
#endif


#define	CONT	-1
#define	BACK	-2
#define	BPAINT	-3

#define	CBUT	0	/* for wcntrl only */
#define	STR	1
#define	ABUT	2	/* button reports */
#define	VBUT	4
#define	SBUT	6
#define CMD_KEY	8	/* only for objhandler */
#define	SCREPAINT	15

/* flags (to be or'ed with window) */
#define	CCP	01000 /* current cursor position */
#define	INV	02000 /* inverse video */

/* windows for wprintf */
#define	FBUT	0
#define	LBUT	13
#define	TTL	15
/* NOTE: MAIL and NEWS are BUTTON numbers, MAIL_WIN and NEWS_WIN are WINDOW
	numbers - if they are ever changed to not coincide, some poor soul had
	better go through all the code and change all the MAIL and NEWS's to
	MAIL_WIN and NEWS_WIN as appropriate */
#define	MAIL_WIN	17
#define	MAIL	17
#define	NEWS_WIN	16
#define	NEWS	16
#define	DWH	18
#define	MWH	19
#define	CLBUT	20
#define	CRBUT	21
#define	DWC	(DWH | CCP)
#define	MWC	(MWH | CCP)

#define	BBUT0	16
#define	BBUT1	17
/* modes for enhancement of display */
#define	BONW	7
#define	WONB	0
#define MESS_PGLAB	0
#define MESS_LAB	1
#define MESS_WAIT	2
