/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:tamwin.h	1.1"
# ifndef WINDOW

#include	"tam.h"
#include	<stdio.h>

#define	WNAME	"/dev/window"

#define HS_DEFAULT 9
#define VS_DEFAULT 12
#define BASE_DEFAULT 9

#define	reg	register

typedef char bool;

#define CHAR(a)		(((achr_t)a) & 0x7F)

# define	TRUE	(1)
# define	FALSE	(0)
# define	ERR	(0)
# define	OK	(1)

#ifdef SYS5
#include	<termio.h>
typedef struct termio	TTY;
#else
#include	<sgtty.h>
typedef	struct sgttyb	TTY;
#endif /*SYS5*/

/*
 * Capabilities from termcap
 */

extern bool     AM, BS, CA, NC, slkshort;
extern bool	MS;
extern char     *BC, *BE, *BO, *CD, *CE, *CI, *CL, *CM,
		*CV, *EE, *FE, *FL, *HO, *KM, *LL, *ND,
		*SE, *SO, *TI, *UE, *UP, *US, *VE, *VS,
		*XE, *XS, PC;

extern short	SG;
/*
 * From the tty modes...
 */
extern char	ttytype[];
/*
 * window definitions
 */
typedef int (*Fint)();
typedef unsigned short	achr_t;

/* ansi attributes					*/
#define ATTRUNDER	(A_UNDERLINE << 8)	/* underlined	*/
#define ATTRREV		(A_REVERSE << 8)	/* inverse video*/
#define ATTRBOLD	(A_BOLD << 8)		/* bold		*/
#define ATTRSTRIKE	(A_STRIKE << 8)		/* strike-out	*/
#define ATTRALL		(ATTRREV|ATTRUNDER|ATTRSTRIKE|ATTRBOLD)
#define ATTRFONT	0xE000	/* high 3 bits are font		*/

/* ansi interpreter states				*/
#define	NORM	0	/* normal characters		*/
#define ESC	1	/* received ESC (0x1b)		*/
#define CSI	2	/* received CSI (ESC [)		*/
#define PARAM	3	/* parsing parm string		*/

/* other ansi parameters				*/
#define NAPARAM	16	/* max number of parameters	*/
#define NAROW	27	/* max number of char rows/w	*/
#define ERRCHR	'?'	/* error character		*/
#define ERRATTR	ATTRREV


/* Generic Rectangle							*/
struct recdef
{
	unsigned short	rec_ulx;	/* upper-left x			*/
 	unsigned short	rec_uly;	/* upper-left y			*/
	unsigned short	rec_lrx;	/* lower-right x (exclusive)	*/
	unsigned short	rec_lry;	/* lower-right y (exclusive)	*/
};

/* Window Structure							*/

struct	dirt	{ short firstc, lastc; };
typedef struct dirt DIRT;

struct window {
	struct	recdef	w_rec;		/* window dimensions		*/
	struct	recdef	w_inrec;	/* dimensions within borders  	*/
	struct	oldef	*w_ol;		/* obscured list		*/
	struct	window	*w_back;	/* window behind this one	*/
	struct	window	*w_front;	/* window in front of this one	*/
	unsigned short	w_uflags;	/* user specified flags		*/
	short	w_astate;		/* ansi parser state		*/
	short	w_iparam;		/* parameter index		*/
	short	w_nparam;		/* parameter count		*/
	short	w_aparam[NAPARAM];	/* parameters			*/
	Fint	*w_adisp;		/* ptr to dispatch table	*/
	unsigned char	w_sflags;	/* window state flags		*/
	short	w_cury, w_curx;
	achr_t	w_attr;
	achr_t	**w_rptr;
	DIRT	*w_dirt;
	char	w_nflags;		/* noise line dirty flags	*/
	char	w_noise[WTXTNUM][WTXTLEN];/* noise line storage		*/
	FILE	*w_outf;		/* output stdio file descriptor	*/
};

typedef struct window WINDOW;

/* Obscured Rectangle							*/
struct oldef
{
	struct	recdef	ol_rec;		/* dimensions of obs. on screen	*/
	WINDOW		*ol_lobs;	/* ptr to frontmost wp		*/
	struct	oldef	*ol_next;	/* next oldef in a chain	*/
	struct	oldef	*ol_last;	/* prev oldef in a chain	*/
};

typedef union { WINDOW *w; FILE *f; int i; } WF;
WF wnmap();			/* map window/file # to WINDOW/FILE pointer */

/* window state flags (w_sflags) */
#define	_DIRTY		1	/* window contents modified	*/
#define _SS2		2	/* pending single-shift 2 	*/
#define _CUROFF		4	/* cursor turned off		*/
#define _BELL		8	/* bell char seen		*/
#define _MAPNL		0x10	/* map nl to cr-nl on output	*/
#define _NSCROLL	0x20	/* window doesn't scroll	*/
/* user specified window flags (w_uflags) */
/* #define NBORDER	1	/* window has borders (window.h)*/

#define	_NOCHANGE	-1

extern WF	window[];
extern WINDOW	physcr;

extern achr_t	aspace;
extern bool	tbtflg;
extern bool	twbflg;

#endif /*WINDOW*/
