/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:demo/form1.c	1.1"
/*
	This program displays a sweepstakes entry form.

	The following key mapping is defined by get_request().
	Note that ^X represents the character control-X.

		^Q		- end form processing

		^F		- move to next page
		^B		- move to previous page
		^N		- move to next field
		^P		- move to previous field
		home key	- move to first field
		home down	- move to last field
		^L		- move left to field
		^R		- move right to field
		^U		- move up to field
		^D		- move down to field

		^W		- move to next word
		^T		- move to previous word
		^S		- move to beginning of field data
		^E		- move to end of field data
		left arrow	- move left in field
		right arrow	- move right in field
		down arrow	- move down in field
		up arrow	- move up in field

		^M <CR>		- enter new line
		^I		- insert blank character
		^O		- insert blank line
		^V		- delete character
		^H <BS>		- delete previous character
		^Y		- delete line
		^G		- delete word
		^C		- clear to end of line
		^K		- clear to end of field
		^X		- clear entire field
		^A		- request next field choice
		^Z		- request previous field choice
		ESC		- toggle between insert and overlay mode
*/

#include <string.h>
#include <form.h>

static char *	PGM	= (char *) 0;	/* program name			*/
static int	CURSES	= FALSE;	/* is curses initialized ?	*/

static void start_curses ()	/* curses initialization */
{
	CURSES = TRUE;
	initscr ();
	nonl ();
	raw ();
	noecho ();
	wclear (stdscr);
}

static void end_curses ()	/* curses termination */
{
	if (CURSES)
	{
		CURSES = FALSE;
		endwin ();
	}
}

static void error (f, s)	/* fatal error handler */
char * f;
char * s;
{
	end_curses ();
	printf ("%s: ", PGM);
	printf (f, s);
	printf ("\n");
	exit (1);
}

static void display_form (f)	/* create form windows and post */
FORM * f;
{
	WINDOW *	w;
	int		rows;
	int		cols;

	scale_form (f, &rows, &cols);	/* get dimensions of form */
/*
	create window 4 characters larger than form dimensions
	with top left corner at (0, 0).  sub-window is positioned
	at (2, 2) relative to form window origin with dimensions
	equal to the form dimensions.
*/
	if (w = newwin (rows+4, cols+4, 0, 0))
	{
		set_form_win (f, w);
		set_form_sub (f, derwin (w, rows, cols, 2, 2));
		box (w, 0, 0);
		keypad (w, 1);
	}
	else
		error ("error return from newwin", NULL);

	if (post_form (f) != E_OK)
		error ("error return from post_form", NULL);
}

static void erase_form (f)	/* unpost and delete form windows */
FORM * f;
{
	WINDOW * w = form_win (f);
	WINDOW * s = form_sub (f);

	unpost_form (f);
	werase (w);
	wrefresh (w);
	delwin (s);
	delwin (w);
}

/*
	define application commands
*/
#define QUIT		(MAX_COMMAND + 1)

static int get_request (w)		/* virtual key mapping */
WINDOW * w;
{
	static int	mode	= REQ_INS_MODE;
	int		c	= wgetch (w);	/* read a character */

	switch (c)
	{
		case 0x11:	/* ^Q */	return	QUIT;

		case 0x06:	/* ^F */	return	REQ_NEXT_PAGE;
		case 0x02:	/* ^B */	return	REQ_PREV_PAGE;
		case 0x0e:	/* ^N */	return	REQ_NEXT_FIELD;
		case 0x10:	/* ^P */	return	REQ_PREV_FIELD;
		case KEY_HOME:			return	REQ_FIRST_FIELD;
		case KEY_LL:			return	REQ_LAST_FIELD;
		case 0x0c:	/* ^L */	return	REQ_LEFT_FIELD;
		case 0x12:	/* ^R */	return	REQ_RIGHT_FIELD;
		case 0x15:	/* ^U */	return	REQ_UP_FIELD;
		case 0x04:	/* ^D */	return	REQ_DOWN_FIELD;
		case 0x17:	/* ^W */	return	REQ_NEXT_WORD;
		case 0x14:	/* ^T */	return	REQ_PREV_WORD;
		case 0x13:	/* ^S */	return	REQ_BEG_FIELD;
		case 0x05:	/* ^E */	return	REQ_END_FIELD;
		case KEY_LEFT:			return	REQ_LEFT_CHAR;
		case KEY_RIGHT:			return	REQ_RIGHT_CHAR;
		case KEY_DOWN:			return	REQ_DOWN_CHAR;
		case KEY_UP:			return	REQ_UP_CHAR;
		case 0x0d:	/* ^M */	return	REQ_NEW_LINE;
		case 0x09:	/* ^I */	return	REQ_INS_CHAR;
		case 0x0f:	/* ^O */	return	REQ_INS_LINE;
		case 0x16:	/* ^V */	return	REQ_DEL_CHAR;
		case 0x08:	/* ^H */	return	REQ_DEL_PREV;
		case 0x19:	/* ^Y */	return	REQ_DEL_LINE;
		case 0x07:	/* ^G */	return	REQ_DEL_WORD;
		case 0x03:	/* ^C */	return	REQ_CLR_EOL;
		case 0x0b:	/* ^K */	return	REQ_CLR_EOF;
		case 0x18:	/* ^X */	return	REQ_CLR_FIELD;
		case 0x01:	/* ^A */	return	REQ_NEXT_CHOICE;
		case 0x1a:	/* ^Z */	return	REQ_PREV_CHOICE;
		case 0x1b:	/* ESC */
						if (mode == REQ_INS_MODE)
							return mode = REQ_OVL_MODE;
						else
							return mode = REQ_INS_MODE;
	}
	return c;
}

static int my_driver (form, c)	/* handle application commands */
FORM * form;
int c;
{
	switch (c)
	{
		case QUIT:
			if (form_driver (form, REQ_VALIDATION) == E_OK)
				return TRUE;
			break;
	}
	beep ();	/* signal error */
	return FALSE;
}

main (argc, argv)
int argc;
char * argv[];
{
	WINDOW *	w;
	FORM *		form;
	FIELD **	f;
	FIELD **	make_fields ();
	void		free_fields ();
	int		c, done = FALSE;

	PGM = argv[0];

	if (! (form = new_form (make_fields ())))
		error ("error return from new_form", NULL);

	start_curses ();
	display_form (form);
/*
	interact with user
*/
	w = form_win (form);

	while (! done)
	{
		switch (form_driver (form, c = get_request (w)))
		{
			case E_OK:
				break;
			case E_UNKNOWN_COMMAND:
				done = my_driver (form, c);
				break;
			default:
				beep ();	/* signal error */
				break;
		}
	}
	erase_form (form);
	end_curses ();
	f = form_fields (form);
	free_form (form);
	free_fields (f);
	exit (0);
}

typedef FIELD *		(* PF_field ) ();

typedef struct
{
	PF_field	type;	/* field constructor	*/
	int		rows;	/* number of rows	*/
	int		cols;	/* number of columns	*/
	int		frow;	/* first row		*/
	int		fcol;	/* first column		*/
	char *		v;	/* field value		*/
}
	FIELD_RECORD;

static FIELD * LABEL (x)	/* create a LABEL field */
FIELD_RECORD * x;
{
	FIELD * f = new_field (1, strlen (x->v), x->frow, x->fcol, 0, 0);

	if (f)
	{
		set_field_buffer (f, 0, x->v);
		field_opts_off (f, O_ACTIVE);
	}
	return f;
}

static FIELD * STRING (x)	/* create a STRING field */
FIELD_RECORD * x;
{
	FIELD * f = new_field (x->rows, x->cols, x->frow, x->fcol, 0, 0);

	if (f)
		set_field_back (f, A_UNDERLINE);
	return f;
}
/*
	field definitions
*/
static FIELD_RECORD F [] =
{
	LABEL,		0,	0,	0,	11,	"Sweepstakes Entry Form",
	LABEL,		0,	0,	2,	0,	"Last Name",
	LABEL,		0,	0,	2,	20,	"First",
	LABEL,		0,	0,	2,	34,	"Middle",
	LABEL,		0,	0,	5,	0,	"Comments",
	STRING,		1,	18,	3,	0,	(char *) 0,
	STRING,		1,	12,	3,	20,	(char *) 0,
	STRING,		1,	12,	3,	34,	(char *) 0,
	STRING,		4,	46,	6,	0,	(char *) 0,
	(PF_field) 0,	0,	0,	0,	0,	(char *) 0,
};

#define MAX_FIELD	512

static FIELD *		fields [MAX_FIELD + 1];	/* field buffer */

static FIELD ** make_fields ()	/* create the fields */
{
	FIELD ** f = fields;
	int i;

	for (i = 0; i < MAX_FIELD && F[i].type; ++i, ++f)
		*f = (* F[i].type) (& F[i]);

	*f = (FIELD *) 0;
	return fields;
}

static void free_fields (f)	/* free the fields */
FIELD ** f;
{
	while (*f)
		free_field (*f++);
}

