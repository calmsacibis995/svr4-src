/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:help.h	1.1"
	/*
	 *	Common definitions for the help module
	 */

#define	Max_displays	100
#define	Max_lines	100
#define	Max_namelen	 16

#define	Min(x, y)	((x) < (y) ? (x) : (y))
#define	Max(x, y)	((x) > (y) ? (x) : (y))
#define	Absdif(x, y)	((x) > (y) ? (x) - (y) : (y) - (x))

#define	help_exit	-1		/* Special help input codes */
#define	help_intr	-2

	/*
	 *  Structure of help displays array
	 */

struct	s_help {
	char	llabel [16];		/* Long function key name */
	char	slabel [8];		/* Short function key name */
	long	disp;			/* Displacement in file of text */
	int	branch [7];		/* Branches to other labels */
	char	title [72];		/* Description for table of contents */
};

	/*
	 *  Global variables in the help process
	 */

extern	struct	s_help	*help_ptr;	/* Pointer to help array */
extern	int	last_help_ix;		/* Most recent index in help array */
extern	int	max_help_ix;		/* Largest valid index in help array */
extern	char	names [Max_displays] [Max_namelen];	/* Help display names */

extern	char	label [];		/* Window label */
extern	int	scrn_sz;		/* Height of screen in lines */
extern	int	scrn_wd;		/* Width of screen in columns */
extern	int	win_id;			/* Window ID */

extern	long	fil_disp;		/* Current displacement in help file */

extern	char	*calloc();
#define malloc(c) calloc((c),sizeof(char))
