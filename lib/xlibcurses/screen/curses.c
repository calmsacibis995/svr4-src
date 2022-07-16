/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/curses.c	1.13"

/* Define global variables */

#include	"curses_inc.h"

WINDOW	*stdscr, *curscr, *_virtscr;
int	LINES, COLS, TABSIZE, COLORS, COLOR_PAIRS;
short	curs_errno = -1;
int	(*_setidln)(), (*_useidln)(), (*_quick_ptr)();
int	(*_do_slk_ref)(), (*_do_slk_tch)(), (*_do_slk_noref)();
void	(*_rip_init)();		/* to initialize rip structures */
void	(*_slk_init)();		/* to initialize slk structures */
SCREEN	*SP;
MOUSE_STATUS Mouse_status = {-1, -1, {BUTTON_RELEASED, BUTTON_RELEASED, BUTTON_RELEASED}, 0};

#ifdef	_VR3_COMPAT_CODE
void	(*_y16update)();
chtype	*acs32map;

#undef	acs_map
_ochtype	*acs_map;
#else	/* _VR3_COMPAT_CODE */
chtype		*acs_map;
#endif	/* _VR3_COMPAT_CODE */

char	*curses_version = "SVR4", curs_parm_err[32];
bool	_use_env = TRUE;

#ifdef	DEBUG
FILE	*outf = stderr;		/* debug output file */
#endif	/* DEBUG */
