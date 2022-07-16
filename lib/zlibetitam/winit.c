/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:winit.c	1.5"
#include "cvttam.h"

extern char *tigetstr ();		/* This is not done in curses.h */

int		_Firsttime=1;		/* Indicates winit not yet called */
NOISEWINDOW	CmdWindow;		/* Where cmd line gets displayed */
NOISEWINDOW	PromptWindow;		/* Where prompt line gets displayed */
NOISEWINDOW	SlkWindow[NFKEYS];	/* Soft label keys */
char		*Reverse;		/* Indicates terminal has reverse */
int		Keypad;			/* Indicates state of keypad */
int		NumSlkLines;		/* Lines occupied by slks */
short		wncur;			/* Current window */
TAMWIN		TamWinPool [NWINDOW];	/* A Collection of TAM windows */
WSTAT		WStatPool[NWINDOW];	/* A collection of WSTATs */
TAMWINLIST	FreeWin,		/* List of available TAMWINs */
		UsedWin;		/* List of active TAMWINs */

static void
SlkInit ()
{
  register i;
  register start;		/* Start of labels window */
  register ils;			/* Interlabel spacing assumming a grouping */
				/* of 3-2-3 */
  register lnkeys;		/* Size of labels */

  NumSlkLines = (LINES < 24) ? 1 : 2;

  /*** Calculate interlabelspacing as follows: 
      1.  2 * ils + NFKEYS * LNKEYS + NFKEYS - 1 = COLS
      2.  ils = (COLS - NFKEYS * LNKEYS - NFKEYS + 1) / 2
      3.  ils = (COLS - NFKEYS * LNKEYS - NFKEYS + 1 - 1) / 2
    In line 1 (NFKEYS - 1) is the number of single spaces between
    labels (ils is the spacing between these groups.  Line 2 is obvious.
    The addition of -1 in line 3 is for rounding down after the division by 2.
  ***/

  lnkeys = LNKEYS+1;
  if ((ils = (COLS - NFKEYS * LNKEYS - NFKEYS + 1 - 1) / 2) <= 0) {
    /* Set label spacing to 0 and recalculate size of each label */
    /* such that all labels fit on screen. */
    ils = 0;
    lnkeys = (COLS-NFKEYS+1)/NFKEYS+1;
  }
  start = 0;
  for (i=0; i<NFKEYS; i++) {
    if ((SlkWin(i) = newwin (NumSlkLines, lnkeys, LINES-NumSlkLines, start))
	 == (WINDOW *)0) {
      break;
    }
    leaveok (SlkWin(i), TRUE);
    scrollok (SlkWin(i), FALSE);
    BlankSlk(i) = TRUE;
    start += lnkeys;
    if (i==2 || i==4) {
      start += ils;
    }
  }
  /* Note: if the above test fails all SlkWindows will be null */
}

void
TAMwinit ()
{
  register i;

  if (_Firsttime) {
    _envinit();
    Keypad = FALSE;
    wncur = -1;
    _Firsttime = 0;
    (void)initscr ();
    /* Minimum window size is 4 lines by 15 cols. */
    /* Lines = 1 cmd line + 1 prompt line + 1 slk line + 1 for user window. */
    /* Cols = 1 char for each slk + 1 space between each slk. */
    if (LINES < 4 || COLS < 15) {
      (void)fprintf (stderr, "winit: Your terminal is too small (%d x %d)\n",
		       LINES, COLS);
      (void)fprintf (stderr, "winit: Minimum size terminal is 4 x 15)\n");
      endwin ();
      exit (ERR);
    }
    noecho ();
    cbreak ();
    nonl ();

    /* Find out if terminal has reverse mode. */

    Reverse = tigetstr ("rev");

    /* Create cmd, prompt and slk windows */

    SlkInit ();
    CmdWindow.blank = TRUE;
    CmdWindow.window = newwin (1, COLS, LINES-(NumSlkLines+2), 0);
    leaveok (CmdWindow.window, TRUE);
    PromptWindow.blank = TRUE;
    PromptWindow.window = newwin (1, COLS, LINES-(NumSlkLines+1), 0);
    leaveok (PromptWindow.window, TRUE);
    LINES -= (NumSlkLines+2);

    /* Set up double link list among all TAMWINs */

    Head(&FreeWin) = (TAMWIN *)0;
    Tail(&FreeWin) = (TAMWIN *)0;
    for (i=NWINDOW; i--;) {
      TamWinPool[i].id = i;
      Cmd(int2TamWin(i)) = (char *)0;
      Prompt(int2TamWin(i)) = (char *)0;
      User(int2TamWin(i)) = (char *)0;
      Label(int2TamWin(i)) = (char *)0;
      Wstat(int2TamWin(i)) = &WStatPool[i];
      _listadd (&FreeWin, &TamWinPool[i]);
    }

    /* Set up free and used list pointers */

    Head(&UsedWin) = (TAMWIN *)0;
    Tail(&UsedWin) = (TAMWIN *)0;
  }
}
