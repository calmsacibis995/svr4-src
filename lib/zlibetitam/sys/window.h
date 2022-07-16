/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:sys/window.h	1.1"
/*
	Unix Window System
	User-Level Window Defs

 */

#ifndef WINDOW_H
#define WINDOW_H

/* ioctls	*/
#define WIOC		('W'<<8)
#define WIOCGETD	(WIOC|1)	/* get window data		*/
#define WIOCSETD	(WIOC|2)	/* set window data		*/
#define WIOCLFONT	(WIOC|3)	/* load window font		*/
#define WIOCUFONT	(WIOC|4)	/* unload window font		*/
#define WIOCSELECT	(WIOC|5)	/* set keyboard window		*/
#define WIOCREAD	(WIOC|6)	/* read whole screen (32k)	*/
#define WIOCGETTEXT	(WIOC|7)	/* get window text info		*/
#define WIOCSETTEXT	(WIOC|8)	/* set window text info		*/
#define WIOCPGRP	(WIOC|9)	/* set window pgrp		*/
#define WIOCSYS		(WIOC|10)	/* set system window slot	*/
#define WIOCGETMOUSE	(WIOC|11)	/* get mouse info		*/
#define WIOCSETMOUSE	(WIOC|12)	/* set mouse info		*/
#define WIOCRASTOP	(WIOC|13)	/* user-level rastop		*/
#define WIOCGSYS	(WIOC|14)	/* get system window pgrp	*/
#define WIOCGCURR	(WIOC|15)	/* get current window number	*/
#define WIOCGPREV	(WIOC|16)	/* get previous window number	*/

/* files, devices							*/
#define WDEV		"/dev/w"	/* individual windows		*/
#define WFONT		"/etc/system.ft"/* system font			*/

/* WIOCGET/SET TEXT params						*/
#define WTXTPROMPT	0		/* prompt line			*/
#define WTXTCMD		1		/* command line			*/
#define WTXTLABEL	2		/* window label			*/
#define WTXTUSER	3		/* user-specific text		*/
#define WTXTSLK1	4		/* first of 2 SLK lines		*/
#define WTXTSLK2	5		/* second SLK line		*/

#define WTXTNUM		6		/* there are 6 text slots/win	*/
#define WTXTLEN		81		/* each can be 80 chars + null	*/

/* character attribute masks		*/
#define A_UNDERLINE	000000400000L
#define A_REVERSE	000001000000L
#define A_BOLD		000010000000L
#define A_STRIKE	A_BOLD
#define A_DIM		000004000000L

/* slots in syswin for WIOCGET/SET SYS					*/
#define SYSWIN		3	/* number of sys wins	*/
#define SYSWMGR		0	/* window manager	*/
#define SYSPMGR		1	/* telephony manager	*/
#define SYSSMGR		2	/* status mgr		*/

/* rastop source operators	*/
#define SRCSRC		0	/* source		*/
#define SRCPAT		1	/* pattern		*/
#define SRCAND		2	/* source & pattern	*/
#define SRCOR		3	/* source | pattern	*/
#define SRCXOR		4	/* source ^ pattern	*/
#define SRCMAX		4

/* rastop destination operators	*/
#define DSTSRC		0	/* srcop(src)		*/
#define DSTAND		1	/* srcop(src) & dst	*/
#define DSTOR		2	/* srcop(src) | dst	*/
#define DSTXOR		3	/* srcop(src) ^ dst	*/
#define DSTCAM		4	/* ~srcop(src) & dst	*/
#define DSTMAX		4

/* w_uflags	*/
#define NBORDER		0x1	/* borderless		*/
#define VCWIDTH		0x2	/* variable chr spacing	*/
#define BORDHSCROLL	0x4	/* border hscroll icons	*/
#define BORDVSCROLL	0x8	/* border vscroll icons	*/
#define BORDHELP	0x10	/* border help patch	*/
#define BORDCANCEL	0x20	/* border cancel patch	*/
#define BORDRESIZE	0x40	/* border re-size patch	*/
#define NBORDMOVE	0x80	/* no border move patch	*/
#define UNCOVERED	0x100	/* uncovered (RO)	*/
#define KBDWIN		0x200	/* keyboard (RO)	*/
#define NOCLEAR		0x400	/* don't clear on create*/
#define NOSETUFLAGS	(UNCOVERED|KBDWIN)

/* basic params	*/
#define WTXTVS		12		/* text line height (sys font)	*/
#define WLINE(n)	((n-1)*WTXTVS)
#define YTXTPROMPT	WLINE(26)	/* prompts			*/
#define YTXTCMD		WLINE(27)	/* command/echo			*/
#define YTXTSLK1	WLINE(28)	/* screen labeled keys #1	*/
#define YTXTSLK2	WLINE(29)	/* screen labeled keys #2	*/

#define WINWIDTH	720		/* logical area (for windows)	*/
#define WINHEIGHT	(348-4*WTXTVS)

struct uwdata				/* user window information	*/
{
	unsigned short	uw_x;		/* upper-left-corner x (pixels)	*/
	unsigned short	uw_y;		/* upper-left-corner y (pixels)	*/
	unsigned short	uw_width;	/* width (pixels)		*/
	unsigned short	uw_height;	/* height (pixels)		*/
	unsigned short	uw_uflags;	/* various flags (see above)	*/
	unsigned char	uw_hs;		/* horizontal size (RO)		*/
	unsigned char	uw_vs;		/* vertical size (RO)		*/
	unsigned char	uw_baseline;	/* baseline (RO)		*/
	unsigned short	uw_cx;		/* current x position (RO)	*/
	unsigned short	uw_cy;		/* current y position (RO)	*/
};

struct utdata				/* user text data		*/
{
	short		ut_num;		/* number (see above)		*/
	char		ut_text[WTXTLEN]; /* text			*/
};

#define	MSDOWN		0x1		/* when buttons go down		*/
#define MSUP		0x2		/* when buttons go up		*/
#define MSIN		0x4		/* when mouse is in rectangle	*/
#define MSOUT		0x8		/* when mouse is outside rect	*/
#define MSICON		0x10		/* load new mouse icon		*/

struct umdata				/* user mouse data		*/
{
	char		um_flags;	/* wakeup flags			*/
	short		um_x;		/* motion rectnalge		*/
	short		um_y;
	short		um_w;
	short		um_h;
	struct icon	*um_icon;	/* ptr to icon if MSICON=1	*/
};

struct urdata				/* user rastop data		*/
{
	unsigned short	*ur_srcbase;	/* ptr to source data		*/
	unsigned short	ur_srcwidth;	/* number bytes/row		*/
	unsigned short	*ur_dstbase;	/* ptr to dest data		*/
	unsigned short	ur_dstwidth;	/* number bytes/row		*/
	unsigned short	ur_srcx;	/* source x 			*/
	unsigned short	ur_srcy;	/* source y			*/
	unsigned short	ur_dstx;	/* destination x		*/
	unsigned short	ur_dsty;	/* destination y		*/
	unsigned short	ur_width;	/* width			*/
	unsigned short	ur_height;	/* height			*/
	unsigned char	ur_srcop;	/* source operation		*/
	unsigned char	ur_dstop;	/* destination operation	*/
	unsigned short	*ur_pattern;	/* pattern pointer		*/
};

#endif /*WINDOW_H*/
