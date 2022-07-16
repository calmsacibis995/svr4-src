/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/newscreen.c	1.22"
#include	"curses_inc.h"

#ifdef	_VR2_COMPAT_CODE
extern	char	_endwin;
#endif	/* _VR2_COMPAT_CODE */

/* 1200 is put at the 0th location since 0 is probably a mistake. */
static long    baud_convert[] =
		{
		    1200, 50, 75, 110, 135, 150, 200, 300, 600, 1200,
		    1800, 2400, 4800, 9600, 19200, 38400
		};

static	char	isfilter = 0;
static	int	_chk_trm();
static	void	_forget();

/*
 * newscreen sets up a terminal and returns a pointer to the terminal
 * structure or NULL in case of an error.  The parameters are:
 *	type: terminal type
 *	lsize, csize, tabsize: physical sizes
 *	infptr, outfptr: input and output stdio stream file pointers
 */

SCREEN	*
newscreen(type, lsize, csize, tabsize, outfptr, infptr)
char	*type;
int	lsize, csize, tabsize;
FILE	*outfptr, *infptr;
{
    int		old_lines = LINES, old_cols = COLS, retcode;
#ifndef	_IOFBF
    char	*sobuf;
#endif	/* _IOBUF */
    WINDOW	*old_curscr = curscr;
    SCREEN	*old = SP;
    TERMINAL	*old_term = cur_term;

#ifdef	DEBUG
    if (outf == NULL)
    {
	outf = fopen("trace", "w");
	if (outf == NULL)
	{
	    perror("trace");
	    exit(-1);
	}
	setbuf(outf, (char *) NULL);
    }

    if (outf)
	fprintf(outf,
	    "NEWTERM(type=%s, outfptr=%x %d, infptr=%x %d) isatty(2) %d, getenv %s\n",
	    type, outfptr, fileno(outfptr), infptr, fileno(infptr), isatty(2), getenv("TERM"));
#endif	/* DEBUG */


    /* read in terminfo file */

    if (setupterm(type, fileno(outfptr), &retcode) != 0)
	goto err2;

    if ((curs_errno = _chk_trm()) != -1)
    {
	(void) strcpy(curs_parm_err, cur_term->_termname);
	goto err2;
    }

    /* use calloc because almost everything needs to be zero */
    if ((SP = (SCREEN *) calloc(1, sizeof (SCREEN))) == NULL)
	goto err1;

    SP->term_file = outfptr;
    SP->input_file = infptr;

    /*
     * The default is echo, for upward compatibility, but we do
     * all echoing in curses to avoid problems with the tty driver
     * echoing things during critical sections.
     */

    SP->fl_echoit = 1;

    /* set some fields for cur_term structure */

    (void) typeahead(fileno(infptr));
    (void) tinputfd(fileno(infptr));

    /*
     * We use LINES instead of the SP variable and a local variable because
     * slk_init and rip_init update the LINES value and application code
     * may look at the value of LINES in the function called by rip_init.
     */

    LINES = SP->lsize = lsize > 0 ? lsize : lines;

    /* force the output to be buffered */
#ifdef	_IOFBF
    (void) setvbuf(outfptr, (char *) NULL, _IOFBF, 0);
#else	/* _IOFBF */
    if ((sobuf = malloc(BUFSIZ)) == NULL)
    {
	curs_errno = CURS_BAD_MALLOC;
#ifdef	DEBUG
	strcpy(curs_parm_err, "newscreen");
#endif	/* DEBUG */
    }
    setbuf(outfptr, sobuf);
#endif	/* _IOFBF */

    SP->baud = baud_convert[_BR(PROGTTY)];

    /* figure out how much each terminal capability costs */
    _init_costs();

    /* initialize the array of alternate characters */
    (void) init_acs();

    SP->tcap = cur_term;

    /* set tty settings to something reasonable for us */
#ifdef	SYSV
    PROGTTY.c_lflag &= ~ECHO;
    PROGTTY.c_lflag |= ISIG;
    PROGTTY.c_oflag &= ~(OCRNL|ONLCR); /* why would anyone set OCRNL? */
#else	/* SYSV */
    PROGTTY.sg_flags &= ~(RAW|ECHO|CRMOD);
#endif	/* SYSV */

    cbreak();

    COLS = SP->csize = csize > 0 ? csize : columns;
    if (tabsize == 0)
	tabsize = (init_tabs == -1) ? 8 : init_tabs;
    SP->tsize = tabsize;
#ifdef	DEBUG
    if (outf)
	fprintf(outf, "LINES = %d, COLS = %d\n", LINES, COLS);
#endif	/* DEBUG */

    if ((curscr = SP->cur_scr = newwin(LINES, COLS, 0, 0)) == NULL)
	goto err;

    SP->fl_endwin = 2;
#ifdef	_VR2_COMPAT_CODE
    _endwin = FALSE;
#endif	/* _VR2_COMPAT_CODE */
    curscr->_sync = TRUE;

    /*
     * This will tell _quick_echo (if it's ever called), whether
     * _quick_echo should let wrefresh handle everything.
     */

    if (ceol_standout_glitch || (magic_cookie_glitch >= 0) ||
	tilde_glitch || (transparent_underline && erase_overstrike))
    {
	curscr->_flags |= _CANT_BE_IMMED;
    }
    if (!(SP->virt_scr = newwin(LINES, COLS, 0, 0)))
	goto err;
    _virtscr = SP->virt_scr;
    
    SP->virt_scr->_clear = FALSE;

    /* video mark map for cookie terminals */

    if (ceol_standout_glitch || (magic_cookie_glitch >= 0))
    {
	register	int	i, nc;
	register	char	**marks;

	if ((marks = (char **) calloc((unsigned) LINES, sizeof(char *))) == NULL)
	    goto err;
	SP->_mks = marks;
	nc = (COLS / BITSPERBYTE) + (COLS % BITSPERBYTE ? 1 : 0);
	if ((*marks = (char *) calloc((unsigned) nc * LINES, sizeof(char))) == NULL)
	    goto err;
	for (i = LINES - 1; i-- > 0; ++marks)
	    *(marks + 1) = *marks + nc;
    }

    /* hash tables for lines */
    if ((SP->cur_hash = (int *) calloc((unsigned) 2 * LINES, sizeof(int))) == NULL)
	goto err;
    SP->virt_hash = SP->cur_hash + LINES;

    /* adjust the screen size if soft labels and/or ripoffline are used */
    if (_slk_init)
	(*_slk_init)();
    if (_rip_init)
	(*_rip_init)();

    if ((SP->std_scr = newwin(LINES, COLS, 0, 0)) == NULL)
    {
	/* free all the storage allocated above and return NULL */
err:
	delscreen(SP);
	COLS = old_cols;
	curscr = old_curscr;
	LINES = old_lines;
err1:
	SP = old;

	curs_errno = CURS_BAD_MALLOC;
#ifdef	DEBUG
	strcpy(curs_parm_err, "newscreen");
#endif	/* DEBUG */

err2:
	cur_term = old_term;
	return (NULL);
    }
#ifdef	DEBUG
    if (outf)
	fprintf(outf, "SP %x, stdscr %x, curscr %x\n", SP, SP->std_scr, curscr);
#endif	/* DEBUG */

    if ((SP->imode = (enter_insert_mode && exit_insert_mode)) &&
	(SP->dmode = (enter_delete_mode && exit_delete_mode)))
    {
	if (strcmp(enter_insert_mode,enter_delete_mode) == 0)
	    SP->sid_equal = TRUE;
	if (strcmp(exit_insert_mode,exit_delete_mode) == 0)
	    SP->eid_equal = TRUE;
    }
    SP->ichok = (SP->imode || insert_character || parm_ich);
    SP->dchok = (delete_character || parm_dch);

    stdscr = SP->std_scr;
    TABSIZE = SP->tsize;

    return (SP);
}

/*
 * check if terminal have capabilities to do basic cursor movements and
 * screen clearing
 */
static
_chk_trm()
{
    short	error_num = -1;
#ifdef	DEBUG
    if (outf)
	fprintf(outf, "chk_trm().\n");
#endif	/* DEBUG */

    if (generic_type)
	error_num = CURS_UNKNOWN;
    else
    {
	if (isfilter)
	{
	    _forget();
	    /* Only need to move left or right on current line */
	    if (!(cursor_left || carriage_return || column_address || parm_left_cursor))
	    {
		goto out_stupid;
	    }
	}
	else
	{
	    if ((hard_copy || over_strike) ||
			/* some way to move up, down, left */
		(!(cursor_address) &&
		(!((cursor_up || cursor_home) && cursor_down &&
		(cursor_left || carriage_return)))) || (!clear_screen))
	    {
out_stupid:
		error_num = CURS_STUPID;
	    }
	}
    }
    return (error_num);
}

filter()
{
    isfilter = 1;
}

/*
 * if (for some reason) user assumes that terminal has only one line,
 * disable all capabilities that deal with non-horizontal cursor movement
 */
static void
_forget()
{
    row_address = cursor_address = clear_screen = parm_down_cursor =
	cursor_up = cursor_down = NULL;
    cursor_home = carriage_return;
    lines = 1;
}
