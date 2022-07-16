/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:demo/pacman/pacdefs.h	1.1"
#include <curses.h>
/* dfp #define POS(row,col)    fputs(tgoto(vs_cm,(col),(row)),stdout)*/
/* #define POS(row,col)	tputs(tgoto(vs_cm,(col),(row)),1,putch) */
#define POS(row,col)	move(row, col)
/* dfp */
#define	PLOT(A,B,C)	POS(A,B);addch(C)
#define	SPLOT(A,B,S)	POS(A,B);addstr(S)
#define TMPF	"/usr/tmp/pacmanXXXXXX"
#define GAME1	'1'
#define GAME2	'2'
#define GAME3	'3'
#define MAXSCORE	"/usr/games/lib/paclog"
#define MSSAVE	5	/* maximum scores saved per game type */
#define MGTYPE	3	/* Maximum game types */
#define	MAXPAC	3	/* maximum number of pacmen to start */
#define	MAXMONSTER	4	/* max number of monsters */
#define EMPTY	0
#define FULL	1
#define LEFT	'h'
#define NLEFT	's'
#define RIGHT	'l'
#define NRIGHT	'f'
#define	NORTH	'k'	/* means UP, but UP defined in vsinit() */
#define NNORTH	'e'
#define	DOWN	'j'
#define NDOWN	'c'
#define HALT	' '
#define REDRAW	'\14'
#define DELETE	'\177'
#define ABORT	'\34'
#define QUIT	'q'
#define CNTLS	'\23'
#define BUF_SIZE	32
#define	UPINT	(-1)
#define	DOWNINT	1
#define	LEFTINT	(-2)
#define	RIGHTINT	2
#define	PACMAN	'@'
#define	MONSTER	'M'
#define	RUNNER	'S'
#define	TREASURE	'$'
#define	CHOICE		'*'
#define	GOLD		'.'
#define	POTION		'O'
#define	VACANT		' '	/* space */
#define	WALL		'#'
#define	GATE		'-'
#define	START	0
#define	RUN	1
#define	FAST	1
#define	SLOW	0
#define	PSTARTX	18
#define	PSTARTY	17
#define	MSTARTX	16	/* monster starting position */
#define	MSTARTY	10	/* monster starting position */
#define	MBEGINX	18	/* monster beginning position */
#define	MBEGINY	7	/* monster beginning position */
#define	TRYPOS	13
#define	TRXPOS	20
#define	GOTONE	1
#define	TURKEY	(-1)
#define	DUP	1
#define	DDOWN	4
#define	DRIGHT	3
#define	DLEFT	2
#define	DNULL	0
#define	BRDX	40
#define	BRDY	23
#define XWRAP	38

/* Scores */
#define	TREASVAL	50
#define	KILLSCORE	200
#define GOLDVAL		10

#define	MSTARTINTVL	10
#define	POTINTVL	25
#define BINTVL		10

#define GOLDCNT	185
#define CUP	'|'
#define CDOWN	'|'
#define CLEFT	'-'
#define CRIGHT	'-'
#define PUP	'v'
#define PDOWN	'^'
#define PLEFT	'>'
#define PRIGHT	'<'

struct	pac
{
	int	xpos;	/* real horizontal position */
	int	ypos;	/* real vertical   position */
	int	dirn;	/* direction of travel */
	int	speed;	/* FAST/SLOW	       */
	int	danger;	/* TRUE if can eat     */
	int	stat;	/* status		*/
	int	xdpos;	/* horizontal position currently displayed at */
	int	ydpos;	/* vertical   position currently displayed at */
};
