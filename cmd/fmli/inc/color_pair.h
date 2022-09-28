/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)fmli:inc/color_pair.h	1.7"

/*
 * NOTE:
 *
 * If the terminal does not support more than 7 color pairs
 * then pairs 8 and greater will be ignored 
 * (The hp color terminal is one such terminal that only supports 7
 * color pairs)
 */ 

/* definable color pairs */
#define NUMDEFPAIRS		11	

#define WINDOW_PAIR		1
#define ACTIVE_TITLE_PAIR	2
#define INACTIVE_TITLE_PAIR	3
#define ACTIVE_BORD_PAIR	4
#define INACTIVE_BORD_PAIR	5
#define BANNER_PAIR		6
#define BAR_PAIR		7
#define SLK_PAIR		8
#define ACTIVE_SCROLL_PAIR	9
#define INACTIVE_SCROLL_PAIR	10	
#define FIELD_PAIR		11	

/* number of default colors and maximum total colors */
#define NUMDEFCOLORS	8
#define MAXCOLORS	64
#define MAXCOLPAIRS	64

extern int Color_terminal;		/* is the terminal a color terminal */
extern int Border_colors_differ;	/* do active/inactive border colors differ? */
extern int Pair_set[MAXCOLPAIRS];	/* is color pair set ? */

/*
 * If the color pair is greater than the number of COLOR_PAIRS ... 
 * or the color pair is not set by the application ...
 * then expand to JUST the video attribute ...
 * else expand to JUST the color attribute ...
 */
#define CHK_PAIR(vid, col) \
	((col > COLOR_PAIRS) || !Pair_set[col] ? vid : COLOR_PAIR(col))

/*
 * If the terminal is a color device ...
 * AND there are more color pairs then 7 ... 
 * then expand to CHK_PAIR(vid, col) ... 
 * else expand to vid 
 */
#define COL_ATTR(vid, col) \
	((Color_terminal == TRUE) && COLOR_PAIRS >= 7 ? CHK_PAIR(vid, col) :vid)
