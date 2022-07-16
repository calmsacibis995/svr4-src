/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:cvttam.h	1.2"
/************************************************************************
 *                            cvttam.h                                  *
 ************************************************************************
 *                                                                      *
 *  Structure definitions and extern declarations for TAM using Curses  *
 *  conversion package.                                                 *
 *                                                                      *
 ************************************************************************/

#include "curses.h"

/***
 *** Because of naming conflicts we cannot include any of the real TAM
 *** header files.  Thus declare here the parts of real TAM that we need.
 ***/

#define NBORDER		0x1	/* borderless */
#define NWINDOW		20	/* Maximum number of TAM windows */
#define NFKEYS		8	/* Number of function keys */
#define LNKEYS		8	/* Length of each function key */

/* Value of state flags given in TAMWIN */

#define NODELAY		0x1	/* NODELAY if 1 */

typedef struct 
{
	short	begy,
		begx,
		height,
		width;
	unsigned short uflags;
} WSTAT;


/***
 *** Structures used to map TAM window ids to real curses WINDOWs
 ***/

typedef struct TAMWIN
{
	int		state;
	int		id;		/* TAM window Id                  */
	WSTAT		*wstat;
	struct TAMWIN	*next,		/* Next TAM window in Free list or */
			*last;		/* window above me on the screen. */

	WINDOW		*wborder,	/* Curses window containing border */
			*wscroll;	/* Scrolling region of TAM window */

	char	slk0[NFKEYS][LNKEYS+1],	/* Top line screen labeled keys */
		slk1[NFKEYS][LNKEYS+1];	/* Bottom line screen labeled keys */

	char		*prompt,	/* Prompt line */
			*command,	/* Command line */
			*label,		/* Label line (in top border) */
			*user;		/* User line (used by window mgr) */

} TAMWIN;

typedef struct
{
	TAMWIN		*head,
			*tail;
} TAMWINLIST;

typedef struct {
  WINDOW	*window;
  int		blank;		/* Indicates this slk is blank */
} NOISEWINDOW;

/***
 *** Global variables used to manage TAM windows
 ***/

extern char		*malloc ();
extern int		NumSlkLines;	/* Number of lines occupied by slks */
extern short		wncur;		/* Current window */
extern TAMWINLIST	FreeWin,	/* List of available TAMwin's */
			UsedWin;	/* List of active TAMwin's    */

extern TAMWIN		TamWinPool[];	/* A collection of TAM windows */

extern NOISEWINDOW	PromptWindow,	/* Global prompt window       (20) */
			CmdWindow,	/* Global command window      (21) */
			SlkWindow[];	/* Screen Labeled key windows (22-23)  */


/***
 *** Useful #define's for translating between the TAM world and the TAM conversion
 *** library world
 ***/

#define Tail(list)		((list)->tail)
#define Head(list)		((list)->head)
#define Next(lp)		((lp)->next)
#define Last(lp)		((lp)->last)
#define CurrentWin		(Tail(&UsedWin))
#define LastWin			(Head(&UsedWin))
#define int2TamWin(i)		(&TamWinPool[i])
#define TamWin2int(w)		w->id
#define State(tw)		((tw)->state)
#define Border(tw)		((tw)->wborder)
#define Scroll(tw)		((tw)->wscroll)
#define Cmd(tw)			((tw)->command)
#define BlankCmd		CmdWindow.blank
#define Prompt(tw)		((tw)->prompt)
#define BlankPrompt		PromptWindow.blank
#define Label(tw)		((tw)->label)
#define User(tw)		((tw)->user)
#define Id(tw)			((tw)->id)
#define BlankSlk(i)		SlkWindow[i].blank
#define SlkWin(i)		SlkWindow[i].window
#define Slk0(tw, kn)		((tw)->slk0[kn])
#define Slk1(tw, kn)		((tw)->slk1[kn])
#define Slk0Char(tw,kn,i)	((tw)->slk0[kn][i])
#define Slk1Char(tw,kn,i)	((tw)->slk1[kn][i])
#define InitSlk0(tw,kn)		((tw)->slk0[kn][0] = '\0')
#define InitSlk1(tw,kn)		((tw)->slk1[kn][0] = '\0')

/* These macros reference the WSTAT stucture within each TAMWIN */

#define Wstat(tw)	((tw)->wstat)
#define Begy(ws)	((ws)->begy)
#define Begx(ws)	((ws)->begx)
#define Height(ws)	((ws)->height)
#define Width(ws)	((ws)->width)
#define Uflags(ws)	((ws)->uflags)

/* Define external definition for global keypad state */

extern int	 Keypad;
extern char	 *Reverse;

/* Define values of external functions */

extern TAMWIN	*_listdel ();
extern void	 _listadd ();
extern TAMWIN	*_validwindow ();
extern void	 _undowindow ();
extern void	 _doborder ();
extern void	 _post ();
extern int	 _winsize ();
extern void	 _noncurrent ();
extern void	 _current ();
extern void	 _envinit ();
extern void	 _envinit ();
extern int	 _winsize ();
extern void	 exit ();
extern void	 free ();
extern int	 TAMwgetstat ();
