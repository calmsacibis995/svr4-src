/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:demo/form2.c	1.1"
/* ----------------------------------------------------------------------

	This program displays a form defined by a simple
	form description language descibed below.  Fields
	can be populated with data from an input file and
	the data from all modified fields will be saved
	in an output file.  The command line syntax is:

		mkform form_description data_input data_output

	where the files form_description, data_input, and
	data_output are described below.

	form_description contains one line describing each field
	on the form.  Each line should begin with a field
	type identification followed by several type dependent
	arguments.  The different field types and their arguments
	are given below:

	LABEL	label	frow	fcol
	STRING	fname	frow	fcol	cols	def
	ALPHA	fname	frow	fcol	cols	width	def
	ALNUM	fname	frow	fcol	cols	width	def
	INTEGER	fname	frow	fcol	cols	prec	vmin	vmax	def
	NUMERIC	fname	frow	fcol	cols	prec	vmin	vmax	def
	REGEXP	fname	frow	fcol	cols	rexp	def
	ENUM	fname	frow	fcol	cols	def	key1 key2 ...keyn
	BLOCK	fname	frow	fcol	rows	cols	nrow	def
	LINK	fname	frow	fcol
	PAGE

	where the first name in each line refers to the field type
	and additional names refer to a sequence of characters
	called a token.  A token must be quoted if it contains
	blanks.  The following definitions apply:

		label	- a label that will be displayed
		fname	- a name used to refer to the field
		frow	- row position with respect to the form
		fcol	- column position with respect to the form
		rows	- number of rows in the field
		cols	- number of columns in the field
		nrow	- number of offscreen rows
		def	- default value
		width	- minumum field width
		prec	- zero padding for INTEGER or
			  digits right of decimal point for NUMERIC
		vmin	- minimum acceptable value
		vmax	- maximum acceptable value
		rexp	- regular expression
		key?	- acceptable keywords

	An example of each of these fields is given below:

		LABEL "example label" 0 0
		STRING example_string 0 0 16 "a default string"
		ALPHA example_alpha 0 0 8 8 "abcdefgh"
		ALNUM example_alnum 0 0 8 8 "abcd1234"
		INTEGER example_integer 0 0 4 0 0 9999 "1234"
		NUMERIC example_numeric 0 0 8 2 0.00 9999.99 " 1000.00"
		REGEXP example_regexp 0 0 4 "^[a-f0-9]*$" "0000"
		ENUM example_enum 0 0 8 "red" "red" "green" "blue"
		BLOCK example_block 0 0 3 16 3 ""
		LINK example_string 0 0
		PAGE
	
	data_input contains fname/value pairs, one per line
	that will be used to populate the fields.  For example,
	given the above field definitions we can set several of
	the field values as follows:

		example_string "new string"
		example_integer "8888"
		example_enum "green"

	Each fname must be defined in the form_description file.

	data_output has the same format as data_input and is
	created as output based on data_input and user modified
	fields and can be used as input in a later session.
	That is, all fields that are different from their
	default value are written (in fname/value pairs)
	to the data_output file.

	The following key mapping is defined by get_request().
	Note that ^X represents the character control-X.

		^Q		- end form processing
		^J <LF>		- reset current field to default value

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

--------------------------- EXAMPLE --------------------------------

	what follows is an example "MEMO OF CALL" form_description file:

LABEL "MEMO OF CALL" 0 14
LABEL "To M" 2 0
LABEL "M" 4 0
LABEL "Phone: (" 6 0
LABEL ")" 6 11
LABEL "-" 6 16
LABEL "Extension:" 6 27
LABEL "Reason:" 8 0
LABEL "Message" 10 16
LABEL "Rec'd by:" 17 0
LABEL "Date:" 17 18
LABEL "/" 17 26
LABEL "Time:" 17 31
LABEL ":" 17 39
STRING to 2 4 38 ""
STRING from 4 1 41 ""
INTEGER areacode 6 8 3 3 0 0 "201"
INTEGER exchange 6 13 3 3 0 0 ""
INTEGER telno 6 17 4 4 0 0 ""
INTEGER extension 6 38 4 4 0 0 ""
ENUM reason 8 8 34 "Telephoned" "Telephoned" "Please call" "In response to your call" "Will call you later" "Called to see you" "Wishes to see you"
BLOCK message 11 0 5 42 5 ""
STRING by 17 10 6 ""
INTEGER month 17 24 2 2 1 12 ""
INTEGER day 17 27 2 2 1 31 ""
INTEGER hour 17 37 2 2 1 12 ""
INTEGER minute 17 40 2 2 0 60 ""

	an example data_input file used to initialize several of
	the form fields might contain the following:

by ross
month 12
day 25
hour 12
minute 00

	this would set the "by", "month", "day", "hour", and "minute"
	fields to the given values.  the following shell procedure
	might be used to populate the data_input file and invoke mkform.

date "+by $LOGNAME%nmonth %m%nday %d%nhour 2/2/90nminute %M" >/usr/tmp/data$$
mkform memo.desc /usr/tmp/data$$ msg$$
rm /usr/tmp/data$$

---------------------------------------------------------------------- */

#include <string.h>
#include <form.h>

extern int	atoi ();
extern long	atol ();
extern double	atof ();
extern char *	malloc ();

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

static void display_form (f)	/* create form windows and post form */
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

static void erase_form (f)	/* unpost form and delete form windows */
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
#define QUIT	(MAX_COMMAND + 1)	/* terminate form processing      */
#define RESET	(MAX_COMMAND + 2)	/* reset current field to default */

static int get_request (w)		/* virtual key mapping */
WINDOW * w;
{
	static int	mode	= REQ_INS_MODE;
	int		c	= wgetch (w);	/* read a character */

	switch (c)
	{
		case 0x11:	/* ^Q */	return	QUIT;
		case 0x0a:	/* ^J */	return	RESET;

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
	FIELD * f = current_field (form);

	switch (c)
	{
		case QUIT:
			if (form_driver (form, REQ_VALIDATION) == E_OK)
				return TRUE;
			break;
		case RESET:
			set_field_buffer (f, 0, field_buffer (f, 1));
			set_field_status (f, FALSE);
			return FALSE;
	}
	beep ();	/* signal error */
	return FALSE;
}

main (argc, argv)	/* mkform form_desc data_in data_out */
int argc;
char * argv[];
{
	WINDOW *	w;
	FILE *		form_desc;	/* form description file */
	FILE *		data_in;	/* data input file       */
	FILE *		data_out;	/* data output file      */
	FORM *		form;
	FIELD **	f;
	FIELD **	make_fields ();	/* make fields from form_desc	*/
	void		free_fields ();	/* free fields and asso info	*/
	void		fill_form ();	/* fill form from data_in	*/
	void		save_form ();	/* save form in data_out	*/
	int		c, done = FALSE;

	PGM = argv[0];

	if (argc != 4)
		error ("%s form_desc data_in data_out", PGM);
	else
	{
		if (! (form_desc = fopen (argv[1], "r")))
			error ("can't open %s", argv[1]);

		data_in  = fopen (argv[2], "r");
		data_out = fopen (argv[3], "w");
	}
	if (! (form = new_form (make_fields (form_desc))))
		error ("error return from new_form", NULL);

	start_curses ();
	fill_form (form, data_in);
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
	save_form (form, data_out);
	f = form_fields (form);
	free_form (form);
	free_fields (f);
	exit (0);
}
/*
	input/output support routines
*/
#define is_blank(x)	((x) == ' ' || (x) == '\t')
#define is_quote(x)	((x) == '"')
#define match(a,b)	(strcmp (a, b) == 0)

#define MAX_BUF		4096

static char	buffer [MAX_BUF + 1];	/* input/output buffer */
static char *	bp;
static char *	bmax = buffer + MAX_BUF;

static char * read_line (fp)	/* read a line into input buffer */
FILE * fp;
{
	char *	p;
	int	n;

	if ((p = fgets (bp = buffer, MAX_BUF, fp)))
		if (buffer[n = strlen (buffer)-1] == '\n')
			buffer[n] = '\0';
	return p;
}

static void write_line (fp)	/* write a line into output buffer */
FILE * fp;
{
	*bp = '\0';
	fputs (bp = buffer, fp);
	fputs ("\n", fp);
}

static char * get_tok ()	/* get next token from input buffer */
{
	char * v = (char *) 0;

	if (*bp)
	{
		while (*bp && is_blank (*bp))
			++bp;
		if (*bp)
		{
			if (is_quote (*bp))
				for (v = ++bp; *bp && ! is_quote (*bp); ++bp);
			else
				for (v = bp++; *bp && ! is_blank (*bp); ++bp);
			if (*bp)
				*bp++ = '\0';
		}
	}
	return v;
}

static void put_tok (s)	/* put a token into output buffer */
char * s;
{
	while (*s++)
		if (bp < bmax)
			*bp++ = *(s-1);
}

static char * copy_str (s)	/* alloc space for and copy string s */
char * s;
{
	char * v;

	if (s && (v = malloc (strlen (s) + 1)))
		return strcpy (v, s);
	return (char *) 0;
}
/*
	field support routines
*/
typedef FIELD *		(* PF_field ) ();
typedef void		(* PF_void ) ();

typedef struct
{
	char *		s;	/* field type identification string      */
	PF_field	make;	/* function to make a field of this type */
	PF_void		free;	/* function to free a field of this type */
}
	FIELD_ENTRY;

typedef struct
{
	FIELD_ENTRY *	type;	/* pointer into f_table (see below)   */
	char *		name;	/* field name used for identification */
	char *		args;	/* type specific information          */
}
	ID;	/* will be hooked onto field userptr */

#define set_field_default(f, v)	{					\
					set_field_buffer (f, 0, v);	\
					set_field_buffer (f, 1, v);	\
					set_field_status (f, FALSE);	\
				}
#define MAX_FIELD		512

static FIELD *			fields	[MAX_FIELD + 1]; /* field buffer */

static char * id (type, name, args)	/* alloc space for and initialize ID */
FIELD_ENTRY * type;
char * name;
char * args;
{
	ID * x = (ID *) malloc (sizeof (ID));

	if (x)
	{
		x -> type = type;
		x -> name = name;
		x -> args = args;
	}
	return (char *) x;
}

static FIELD * find_field (f, name)	/* find field in f with name */
FIELD ** f;
char * name;
{
	ID * x;

	while (*f)
	{
		x = (ID *) field_userptr (*f);

		if (x && x -> name && match (name, x -> name))
			break;
		++f;
	}
	return *f;
}
/*
	LABEL	label	frow	fcol
*/
static FIELD * make_label (type)
FIELD_ENTRY * type;
{
	char * label = get_tok ();
	int frow = atoi (get_tok ());
	int fcol = atoi (get_tok ());
	FIELD * f = new_field (1, strlen (label), frow, fcol, 0, 0);

	if (f)
	{
		set_field_buffer (f, 0, label);
		set_field_status (f, FALSE);
		set_field_back (f, A_NORMAL);
		set_field_userptr (f, id (type, NULL, NULL));
		field_opts_off (f, O_ACTIVE);
	}
	return f;
}
/*
	STRING	fname	frow	fcol	cols	def
*/
static FIELD * make_string (type)
FIELD_ENTRY * type;
{
	char * fname = copy_str (get_tok ());
	int frow = atoi (get_tok ());
	int fcol = atoi (get_tok ());
	int cols = atoi (get_tok ());
	char * def = get_tok ();
	FIELD * f = new_field (1, cols, frow, fcol, 0, 1);

	if (f)
	{
		set_field_userptr (f, id (type, fname, NULL));
		set_field_default (f, def);
	}
	return f;
}
/*
	ALPHA	fname	frow	fcol	cols	width	def
*/
static FIELD * make_alpha (type)
FIELD_ENTRY * type;
{
	char * fname = copy_str (get_tok ());
	int frow = atoi (get_tok ());
	int fcol = atoi (get_tok ());
	int cols = atoi (get_tok ());
	int width = atoi (get_tok ());
	char * def = get_tok ();
	FIELD * f = new_field (1, cols, frow, fcol, 0, 1);

	if (f)
	{
		set_field_userptr (f, id (type, fname, NULL));
		set_field_default (f, def);
		set_field_type (f, TYPE_ALPHA, width);
	}
	return f;
}
/*
	ALNUM	fname	frow	fcol	cols	width	def
*/
static FIELD * make_alnum (type)
FIELD_ENTRY * type;
{
	char * fname = copy_str (get_tok ());
	int frow = atoi (get_tok ());
	int fcol = atoi (get_tok ());
	int cols = atoi (get_tok ());
	int width = atoi (get_tok ());
	char * def = get_tok ();
	FIELD * f = new_field (1, cols, frow, fcol, 0, 1);

	if (f)
	{
		set_field_userptr (f, id (type, fname, NULL));
		set_field_default (f, def);
		set_field_type (f, TYPE_ALNUM, width);
	}
	return f;
}
/*
	INTEGER	fname	frow	fcol	cols	prec	vmin	vmax	def
*/
static FIELD * make_integer (type)
FIELD_ENTRY * type;
{
	char * fname = copy_str (get_tok ());
	int frow = atoi (get_tok ());
	int fcol = atoi (get_tok ());
	int cols = atoi (get_tok ());
	int prec = atoi (get_tok ());
	long vmin = atol (get_tok ());
	long vmax = atol (get_tok ());
	char * def = get_tok ();
	FIELD * f = new_field (1, cols, frow, fcol, 0, 1);

	if (f)
	{
		set_field_userptr (f, id (type, fname, NULL));
		set_field_default (f, def);
		set_field_just (f, JUSTIFY_RIGHT);
		set_field_type (f, TYPE_INTEGER, prec, vmin, vmax);
	}
	return f;
}
/*
	NUMERIC	fname	frow	fcol	cols	prec	vmin	vmax	def
*/
static FIELD * make_numeric (type)
FIELD_ENTRY * type;
{
	char * fname = copy_str (get_tok ());
	int frow = atoi (get_tok ());
	int fcol = atoi (get_tok ());
	int cols = atoi (get_tok ());
	int prec = atoi (get_tok ());
	double vmin = atof (get_tok ());
	double vmax = atof (get_tok ());
	char * def = get_tok ();
	FIELD * f = new_field (1, cols, frow, fcol, 0, 1);

	if (f)
	{
		set_field_userptr (f, id (type, fname, NULL));
		set_field_default (f, def);
		set_field_just (f, JUSTIFY_RIGHT);
		set_field_type (f, TYPE_NUMERIC, prec, vmin, vmax);
	}
	return f;
}
/*
	REGEXP	fname	frow	fcol	cols	rexp	def
*/
static FIELD * make_regexp (type)
FIELD_ENTRY * type;
{
	char * fname = copy_str (get_tok ());
	int frow = atoi (get_tok ());
	int fcol = atoi (get_tok ());
	int cols = atoi (get_tok ());
	char * rexp = get_tok ();
	char * def = get_tok ();
	FIELD * f = new_field (1, cols, frow, fcol, 0, 1);

	if (f)
	{
		set_field_userptr (f, id (type, fname, NULL));
		set_field_default (f, def);
		set_field_type (f, TYPE_REGEXP, rexp);
	}
	return f;
}
/*
	ENUM	fname	frow	fcol	cols	def	key1 key2 ...keyn
*/
#define MAX_KEYS	512
static char *		keys[MAX_KEYS];

static FIELD * make_enum (type)
FIELD_ENTRY * type;
{
	char * fname = copy_str (get_tok ());
	int frow = atoi (get_tok ());
	int fcol = atoi (get_tok ());
	int cols = atoi (get_tok ());
	char * def = get_tok ();
	FIELD * f = new_field (1, cols, frow, fcol, 0, 1);
	char ** p = keys;
	char ** x;
	char ** t;
	int n;

	while (*p = get_tok ())
		++p;

	n = p - keys;

	if (! f)
		return f;

	if (x = (char **) malloc ((n+1)*sizeof(char*)))
	{
		t = x;
		p = keys;

		while (*p)
			*x++ = copy_str (*p++);
		*x = (char *) 0;

		set_field_userptr (f, id (type, fname, t));
		set_field_default (f, def);
		set_field_type (f, TYPE_ENUM, t, FALSE, FALSE);
	}
	return f;
}

static void free_enum (f)
FIELD * f;
{
	ID * x = (ID *) field_userptr (f);

	if (x)
	{
		char ** p = (char **) x -> args;
		char ** t = p;

		if (x -> name)
			free (x -> name);
		if (t)
		{
			while (*t)
				free (*t++);
			free (p);
		}
		free (x);
	}
	free_field (f);
}
/*
	BLOCK	fname	frow	fcol	rows	cols	nrow	def
*/
static FIELD * make_block (type)
FIELD_ENTRY * type;
{
	char * fname = copy_str (get_tok ());
	int frow = atoi (get_tok ());
	int fcol = atoi (get_tok ());
	int rows = atoi (get_tok ());
	int cols = atoi (get_tok ());
	int ncol = atoi (get_tok ());
	char * def = get_tok ();
	FIELD * f = new_field (rows, cols, frow, fcol, ncol, 1);

	if (f)
	{
		set_field_userptr (f, id (type, fname, NULL));
		set_field_default (f, def);
	}
	return f;
}
/*
	LINK	fname	frow	fcol
*/
static FIELD * make_link (type)
FIELD_ENTRY * type;
{
	char * fname = copy_str (get_tok ());
	int frow = atoi (get_tok ());
	int fcol = atoi (get_tok ());
	FIELD * f = link_field (find_field (fields, fname), frow, fcol);

	if (f)
	{
		set_field_userptr (f, id (type, NULL, NULL));
		field_opts_off (f, O_ACTIVE);
	}
	return f;
}

static void free_basic (f)
FIELD * f;
{
	ID * x = (ID *) field_userptr (f);

	if (x)
	{
		if (x -> name)
			free (x -> name);
		if (x -> args)
			free (x -> args);
		free (x);
	}
	free_field (f);
}
/*
	define field types and support function
*/
static FIELD_ENTRY f_table [] =
{
	"LABEL",	make_label,	free_basic,
	"STRING",	make_string,	free_basic,
	"ALPHA",	make_alpha,	free_basic,
	"ALNUM",	make_alnum,	free_basic,
	"INTEGER",	make_integer,	free_basic,
	"NUMERIC",	make_numeric,	free_basic,
	"REGEXP",	make_regexp,	free_basic,
	"ENUM",		make_enum,	free_enum,
	"BLOCK",	make_block,	free_basic,
	"LINK",		make_link,	free_basic,
	(char *) 0,	(PF_field) 0,	(PF_void) 0,
};

static FIELD_ENTRY * get_type (s)
char * s; /* type identification string */
{
	int	i = 0;

	while (f_table[i].s)
	{
		if (match (f_table[i].s, s))
			return &f_table[i];
		++i;
	}
	return (FIELD_ENTRY *) 0;
}

static FIELD ** make_fields (fp) /* read form description and create fields */
FILE * fp;
{
	FIELD **	f = fields;
	FIELD **	fmax = fields + MAX_FIELD;
	FIELD_ENTRY *	type;
	char *		s;
	int		newPage = FALSE;

	set_field_back ((FIELD *) 0, A_UNDERLINE); /* change system default */

	while (f < fmax)
	{
		*f = (FIELD *) 0;

		if (! read_line (fp))
			break;

		s = get_tok ();

		if (match (s, "PAGE"))		/* look for page break */
			newPage = TRUE;

		else if (type = get_type (s))
		{
			*f = (* type -> make) (type);

			if (*f && newPage)
			{
				newPage = FALSE;
				set_new_page (*f, TRUE);
			}
			++f;
		}
	}
	fclose (fp);
	return fields;
}

static void free_fields (f)	/* free fields and related data */
FIELD ** f;
{
	ID * x;

	while (*f)
	{
		x = (ID *) field_userptr (*f);

		if (x)
			(* x -> type -> free) (*f);
		++f;
	}
}

static void fill_form (form, fp)	/* read in field data */
FORM * form;
FILE * fp;
{
	FIELD * f;
	char *  name;
	char *  data;

	if (fp)
	{
		while (read_line (fp))
		{
			name = get_tok ();
			data = get_tok ();

			if (f = find_field (form_fields (form), name))
				set_field_buffer (f, 0, data);
		}
		fclose (fp);
	}
}

static void save_form (form, fp)	/* write out field data */
FORM * form;
FILE * fp;
{
	FIELD ** f = form_fields (form);
	ID * x;

	if (fp)
	{
		while (*f)
		{
			x = (ID *) field_userptr (*f);

			if (x && x -> name && field_status (*f))
			{
				put_tok (x -> name);
				put_tok ("\t");
				put_tok ("\"");
				put_tok (field_buffer (*f, 0));
				put_tok ("\"");
				write_line (fp);
			}
			++f;
		}
		fclose (fp);
	}
}
