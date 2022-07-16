#ident	"@(#)ucbtroff:troff.d/devaps/daps.g	1.1.1.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 




	/********************************************************************
	 *																	*
	 *				POST-PROCESSOR GLOBAL VARIABLES						*
	 *																	*
	 ********************************************************************/



short 	debug[MAX_DEBUG];				/* array of debug flags */

short 	privelege = ON;					/* is user priveleged */

short	x_stat = 0;						/* program exits with this status */

short	ignore = NO;					/* don't ignore fatal errors */
short	report = NO;					/* just report results */
short	busyflag = OFF;					/* report typesetter availability */
short	waitflag = OFF;					/* wait for typesetter */

short	output = OFF;					/* ON if we do output for this page */

int		nolist = 0;						/* number of page pairs in olist[] */

int		olist[MAX_OUTLIST];				/* list of pages to process */

int		spage = 9999;					/* stop every spage pages */

int		scount = 0;						/* spage counter */

int		arg_index = 0;					/* offset from argv for next argument */

long	line_number = -1;				/* current line in input file */

int		lastw;							/* width of last printed character */

int		pageno = 0;						/* current page number */
long	paper = 0;						/* amount of paper used */

char	*banner = "\0";					/* pointer to banner string */
char	ban_buf[BAN_LENGTH];			/* buffer for banner read from file */
short	print_banner = NO;				/* print the job's banner? */
char	*cut_marks = "--";				/* string used for cut marks */
char	*ban_sep = "\\(en \\(en \\(en \\(en \\(en \\(en \\(en \\(en \\(en \\(en";

short	last_slant = 0;					/* last slant stored in the APS-5 */
short	last_req_slant = 0;				/* last requested slant from troff */
short	aps_slant = 0;					/* current slant angle for type */
int		aps_font = 0;					/* font that the APS is using */

short	range = 1;						/* current master range */


char	devname[NAME_LENGTH] = "aps";	/* name of phototypesetter */

char	*typesetter = "/dev/null";		/* should be /dev/aps */
char	*tracct = "";					/* accounting file pathname */
char	*fontdir = "/usr/ucblib/doctools/font";			/* font directory */

int		alt_tables = 0;					/* which font has alt tables loaded */
int		alt_font[FSIZE];				/* alternate font table */
char	alt_code[FSIZE];				/* alternate code table */

short	*pstab;							/* point size list */
short	*chtab;							/* char's index in chname array */
char	*chname;						/* special character strings */
char	*fitab[NFONT+1];				/* chars position on a font */
char	*widthtab[NFONT+1];				/* character's width on a font */
char	*codetab[NFONT+1];				/* APS-5 code for character */

int		nsizes;							/* number of sizes from DESC.out */
int		nfonts;							/* number of default fonts */
int		nchtab;							/* number of special chars in chtab */
int		smnt = 0;						/* index of first special font */
int		res = 723;						/* DESC.out res should equal this */

int		size = 1;						/* current internal point size */
int		font = 1;						/* current font number */

int		hpos;							/* troff calculated horiz position */
int		htrue = 0;						/* our calculated horiz position */
int		vpos;							/* current vertical position (down positive) */
int		hcutoff = HCUTOFF;				/* horizontal cutoff in device units */
double	cutoff = CUTOFF;				/* horizontal beam cutoff in inches */
long	cur_vpos = 0;					/* current distance from start of job */
long	max_vpos = 0;					/* maximum distance from start of job */


int		DX = 3;							/* step size in x for drawing */
int		DY = 3;							/* step size in y for drawing */
int		drawdot = '.';					/* draw with this character */
int		drawsize = 1;					/* shrink by this factor when drawing */


struct	{								/* font position information */
			char	*name;				/* external font name */
			int		number;				/* internal font name */
			int		nwfont;				/* width entries - load_alt uses it */
}	fontname[NFONT+1];

struct dev	dev;						/* store APS data here from DESC.out */

struct Font	*fontbase[NFONT+1];			/* point to font data from DESC.out */


FILE	*fp_debug = stderr;				/* debug file descriptor */
FILE	*fp_error = stderr;				/* error message file descriptor */
FILE	*fp_acct = stderr;				/* accounting file descriptor */
FILE	*tf = stdout;					/* typesetter output file */

int		horig;							/* not really used */
int		vorig;

int		vert_step = MAX_INT;			/* break up vert steps larger than this */



/*****************************************************************************/



#ifdef ADJUST							/* do the vertical adjustments */



	/********************************************************************
	 *																	*
	 *		Some of the characters on the current APS disk at Murray	*
	 *	Hill are quite bad. In particular lf, rf, lb, and rb are all	*
	 *	too low, so that braces built up from them don't join properly.	*
	 *	The data structures that follow are used to make the proper		*
	 *	vertical adjustment for problem characters such as these. They	*
	 *	are only used in routines put1s() and put1(). The code in these	*
	 *	two routines that was added to handle these special cases		*
	 *	should be removed or commented out when the characters are		*
	 *	finially fixed! In addition routine t_adjust() was written		*
	 *	to do the lookup in these tables and set the appropriate value	*
	 *	for v_adjust - it can be removed whenever you no longer need	*
	 *	these vertical adjustments.										*
	 *																	*
	 *		All of this vertical adjustment stuff is included in the	*
	 *	object code when the conditional compilation flag ADJUST is		*
	 *	defined. To eliminate the overhead of these adjustments just	*
	 *	remove the definition of ADJUST from daps.h.					*
	 *																	*
	 ********************************************************************/



int		v_adjust = 0;					/* vert adjustment - done in put1() */

char	*adj_tbl[] = {					/* adjust these characters */
			"lf",
			"rf",
			"lb",
			"rb",
			"lt",
			"rt",
			0,
};

int		vadjustment[] = {				/* corresponding vert adjustment */
			-1,
			-1,
			-3,
			-3,
			1,
			1,
			0,
};


#endif

