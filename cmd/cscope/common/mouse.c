/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cscope:common/mouse.c	1.2"
/*	cscope - interactive C symbol cross-reference
 *
 *	mouse functions
 */

#include "global.h"

BOOL	mouse = NO;			/* mouse interface */

#ifdef UNIXPC	/* build command requires #ifdef instead of #if */
#include <sys/window.h>
BOOL	unixpcmouse = NO;	/* running with a mouse on the Unix PC? */
static int uw_hs, uw_vs;	/* character height and width */
#endif

typedef	struct {			/* menu */
	char	*text;
	char	*value;
} MENU;

static	MENU	mainmenu[] = {		/* main menu */
	"Send",		"##\033s##\r",
	"Repeat",	"\031",
	"Edit All",	"\05",
	"Rebuild",	"\022",
	"Shell",	"!",
	"Redraw",	"\f",
	"Help",		"?",
	"Exit",		"\04",
	NULL,		NULL
};

static	MENU	changemenu[] = {	/* change mode menu */
	"Mark Screen",	"*",
	"Mark All",	"a",
	"Change",	"\04",
	"No Change",	"\033",
	"Shell",	"!",
	"Redraw",	"\f",
	"Help",		"?",
	NULL,		NULL
};

static	MENU	*loaded;		/* menu loaded */
static	BOOL	emacsviterm = NO;	/* terminal type */

void	loadmenu();

/* see if there is a mouse interface */

void
mouseinit()
{
	char	*term;
	char	*mygetenv();

	/* see if this is emacsterm or viterm */
	term = mygetenv("TERM", "");
	if (strcmp(term, "emacsterm") == 0 || 
	    strcmp(term, "viterm") == 0) {
		emacsviterm = YES;
		mouse = YES;
	}
	/* the MOUSE enviroment variable is for 5620 terminal programs that have
	   mouse support but the TERM environment variable is the same as a
	   terminal without a mouse, such as myx */
	else if (strcmp(mygetenv("MOUSE", ""), "myx") == 0) {
		mouse = YES;
	}
#if UNIXPC
	else if (strcmp(term,"s4") == 0 || 
	         strcmp(term,"s120") == 0 ||
	         strcmp(term,"s90") == 0) {
		int retval;
		struct uwdata uwd;	/* Window data structure */
		struct umdata umd;	/* Mouse data structure */

		/* Ask for character size info */
			
		retval = ioctl(1,WIOCGETD,&uwd);
		if(retval || uwd.uw_hs <= 0 || uwd.uw_vs <= 0) {
			/**************************************************
			 * something wrong with the kernel, so fake it... 
			 **************************************************/
			if(!strcmp(term,"s4")) {
				uw_hs = 9;
				uw_vs = 12;
			}
			else {
				uw_hs = 6;
				uw_vs = 10;
			}
		}
		else {
			/* Kernel is working and knows about this font */
			uw_hs = uwd.uw_hs;
			uw_vs = uwd.uw_vs;
		}
		
		/**************************************************
		 * Now turn on mouse reporting so we can actually
		 * make use of all this stuff.
		 **************************************************/
		if((retval = ioctl(1,WIOCGETMOUSE,&umd)) != -1) {
			umd.um_flags= MSDOWN+MSUP;
			ioctl(1,WIOCSETMOUSE,&umd);
		}
		unixpcmouse = YES;
	}
#endif
	if (mouse == YES) {
		loadmenu(mainmenu);
	}
}

/* load the correct mouse menu */

void
mousemenu()
{
	if (mouse == YES) {
		if (changing == YES) {
			loadmenu(changemenu);
		}
		else {
			loadmenu(mainmenu);
		}
	}
}

/* download a menu */

void
loadmenu(menu)
MENU	*menu;
{
	register int	i;

	if (emacsviterm == YES) {
		mousereinit();
		(void) printf("\033V1");	/* display the scrollbar */
		(void) printf("\033M0@%s@%s@", menu[0].text, menu[0].value);
		for (i = 1; menu[i].text != NULL; ++i) {
			(void) printf("\033M@%s@%s@", menu[i].text, menu[i].value);
		}
	}
	else {	/* myx */
		int	len;
		
		mousecleanup();
		(void) printf("\033[6;1X\033[9;1X");
		for (i = 0; menu[i].text != NULL; ++i) {
			len = strlen(menu[i].text);
			(void) printf("\033[%d;%dx%s%s", len, len + strlen(menu[i].value), 
				menu[i].text, menu[i].value);
		}
		loaded = menu;
	}
	(void) fflush(stdout);
}

/* reinitialize the mouse in case curses changed the attributes */

void
mousereinit()
{
	if (emacsviterm == YES) {

		/* enable the mouse click and sweep coordinate control sequence */
		/* and switch to menu 2 */
		(void) printf("\033{2\033#2");
		(void) fflush(stdout);
	}
}

/* restore the mouse attributes */

void
mousecleanup()
{
	register int	i;

	if (loaded != NULL) {	/* only true for myx */
		
		/* remove the mouse menu */
		(void) printf("\033[6;0X\033[9;0X");
		for (i = 0; loaded[i].text != NULL; ++i) {
			(void) printf("\033[0;0x");
		}
		loaded = NULL;
	}
}

/* draw the scrollbar */

void
drawscrollbar(top, bot)
int top, bot;
{
	register p1, p2;

	if (emacsviterm == YES) {
		if (bot > top) {
			p1 = 16 + (top - 1) * 100 / totallines;
			p2 = 16 + (bot - 1) * 100 / totallines;
			if (p2 > 116) {
				p2 = 116;
			}
			if (p1 < 16) {
				p1 = 16;
			}
			/* don't send ^S or ^Q because it will hang a layer using cu(1) */
			if (p1 == ctrl('Q') || p1 == ctrl('S')) {
				++p1;
			}
			if (p2 == ctrl('Q') || p2 == ctrl('S')) {
				++p2;
			}
		}
		else {
			p1 = p2 = 16;
		}
		(void) printf("\033W%c%c", p1, p2);
	}
}

/* get the mouse information */

MOUSE *
getmouseaction(leading_char)
{
	static	MOUSE	m;

#if UNIXPC

	if(unixpcmouse == YES && leading_char == ESC) {

		/* Called if cscope received an ESC character.  See if it is
		 * a mouse report and if so, decipher it.  A mouse report
		 * looks like: "<ESC>[?xx;yy;b;rM"
		 */
		int x = 0, y = 0, button = 0, reason = 0;
		int i;
	
		/* Get a mouse report.  The form is: XX;YY;B;RM where
		 * XX is 1,2, or 3 decimal digits with the X pixel position.
		 * Similarly for YY.  B is a single decimal digit with the
		 * button number (4 for one, 2 for two, and 1 for three).
		 * R is the reason for the mouse report.
		 *
		 * In general, the input is read until the mouse report has
		 * been completely read in or we have discovered that this
		 * escape sequence is NOT a mouse report.  In the latter case
		 * return the last character read to the input stream with
		 * ungetch().
		 */
		
		/* Check for "[?" being next 2 chars */
		if(((i = mygetch()) != '[') || ((i = mygetch()) != '?')) {
			(void) ungetch(i);
			return(NULL);
		}
	
		/* Grab the X position (in pixels) */
		while(isdigit(i = mygetch())) {
			x = (x*10) + (i - '0');
		}
		if(i != ';') {
			(void) ungetch(i);
			return(NULL);	/* not a mouse report after all */
		}
	
		/* Grab the Y position (in pixels) */
		while(isdigit(i = mygetch())) {
			y = (y*10) + (i - '0');
		}
		if(i != ';') {
			(void) ungetch(i);
			return(NULL);
		}
	
		/* Get which button */
		if((button = mygetch()) > '4') {
			(void) ungetch(button);
			return(NULL);
		}
		if((i = mygetch()) != ';') {
			(void) ungetch(i);
			return(NULL);
		}
		
		/* Get the reason for this mouse report */
		if((reason = mygetch()) > '8') {
			(void) ungetch(reason);
			return(NULL);
		}
		
		/* sequence should terminate with an 'M' */
		if((i = mygetch()) != 'M') {
			(void) ungetch(i);
			return(NULL);
		}
	
	
		/* OK.  We get a mouse report whenever a button is depressed
		 * or released.  Let's ignore the report whenever the button
		 * is depressed until when I am ready to implement sweeping.
		 */
		if(reason != '2') {
			return(NULL);	/* '2' means button is released */
		}
	
		/************************************************************
		 * Always indicate button 1 irregardless of which button was
		 * really pushed.
		 ************************************************************/
		m.button = 1;
	
		/************************************************************
		 * Convert pixel coordinates to line and column coords.
		 * The height and width are obtained using an ioctl() call
		 * in mouseinit().  This assumes that variable width chars
		 * are not being used ('though it would probably work anyway).
		 ************************************************************/
		
		m.x1 = x/uw_hs;	/* pixel/horizontal_spacing */
		m.y1 = y/uw_vs;	/* pixel/vertical_spacing   */
	
		/* "null" out the other fields */
		m.percent = m.x2 = m.y2 = -1;
	}
	else
#endif	/* not UNIXPC */

	if (mouse == YES && leading_char == ctrl('X')) {
	
		switch (mygetch()) {
		case ctrl('_'):		/* click */
			if ((m.button = mygetch()) == '0') {	/* if scrollbar */
				m.percent = getpercent();
			}
			else {
				m.x1 = getcoordinate();
				m.y1 = getcoordinate();
				m.x2 = m.y2 = -1;
			}
			break;
	
		case ctrl(']'):		/* sweep */
			m.button = mygetch();
			m.x1 = getcoordinate();
			m.y1 = getcoordinate();
			m.x2 = getcoordinate();
			m.y2 = getcoordinate();
			break;
		default:
			return(NULL);
		}
	}
	else return(NULL);

	return(&m);
}

/* get a row or column coordinate from a mouse button click or sweep */

getcoordinate()
{
	register int  c, next;

	c = mygetch();
	next = 0;
	if (c == ctrl('A')) {
		next = 95;
		c = mygetch();
	}
	if (c < ' ') {
		return (0);
	}
	return (next + c - ' ');
}

/* get a percentage */

getpercent()
{
	register int c;

	c = mygetch();
	if (c < 16) {
		return(0);
	}
	if (c > 120) {
		return(100);
	}
	return(c - 16);
}
