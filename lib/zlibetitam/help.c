/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:help.c	1.1"
	/*
	 *	Initialization and main loop for the help process
	 */

#include <stdio.h>
#include <sys/signal.h>
#include <tam.h>
#include <message.h>

#include "help.h"

extern	char	*optarg;

	/*
	 *  Tokens for keywords in help file
	 */

#define  tk_label       0
#define  tk_contents    1
#define  tk_name        2
#define  tk_llabel      3
#define  tk_slabel      4
#define  tk_branch      5
#define  tk_title       6
#define  tk_text        7
#define  tk_err         -1

	/*  Filler     -  Returns true if character is just taking up space */
	/*  Separator  -  Returns true if character is a name separator */
	/*  Valid      -  Returns true if character is valid in a keyword */

#define  Filler(x) (((x) == 0x20) || ((x) == ',') || ((x) == 0x09) || ((x) == 0x0a))

#define  Separator(x) (((x) == ',') || ((x) == 0x0a))

#define  Valid(x) ((((x) >= 'a') && ((x) <= 'z')) || (((x) >= 'A') && ((x) <= 'Z')))

#define	 Quote(x) ((x) == '\"')

	/*
	 *  Global variables in the help process
	 */

FILE	*fil_ptr;		/* File stream pointer */
long	fil_disp;		/* Current file displacement */
char	fil_buf [81];		/* File buffer */
char	*fil_bptr;		/* File buffer pointer */
int	last_char_read;

FILE	*dbg_file;		/* Debug file stream pointer */

int	help_ix;		/* Current index in help page structure */
int	last_help_ix;		/* Most recent index in help page structure */
int	max_help_ix;		/* Largest valid index in help page structure */
struct	s_help	*help_ptr;	/* Pointer to help array */
char	names [Max_displays] [Max_namelen];	/* Help display names */

char	label [80] = {"Help"};		/* Window label */

int	scrn_sz;		/* Height of screen in lines */
int	scrn_wd;		/* Width of screen in columns */
int	win_id;			/* Window ID */

	/*
	 *  Keyword table
	 */

struct	s_kwtbl
	{
	char	*keyword;
	int	token;
	}

	kw_tbl [] =
	{
	"Wlabel",	tk_label,
	"Contents",	tk_contents,
	"Name",		tk_name,
	"Llabel",	tk_llabel,
	"Slabel",	tk_slabel,
	"Branch",	tk_branch,
	"Title",	tk_title,
	"Text",		tk_text,
	0,		tk_err		/* Keyword not found */
	};

	/*
	 *  main  -  Start execution of process
	 */

main (argc, argv)

	int	argc;
	char	*argv [];

	{
	WSTAT	win_stat;
	int	ch;
	char	*title;
	char	*filname;
	int	ix;

	int	sc ();

	winit ();
	keypad (0, 1);
	win_id = wcreate (5, 10, LINES - 10, COLS - 20, BORDVSCROLL | BORDHELP |
					BORDCANCEL | BORDRESIZE);
	wgetstat (win_id, &win_stat);
	scrn_sz = win_stat.height;
	scrn_wd = win_stat.width;

	dbg_file = NULL;
	filname = NULL;
	title = NULL;
	while ((ch = getopt (argc, argv, "d:h:t:")) != EOF)
		switch (ch)
			{
			case 'd':
				dbg_file = fopen (optarg, "w");
				break;

			case 'h':
				filname = optarg;
				fil_ptr = fopen (optarg, "r");
				break;

			case 't':
				title = optarg;
				break;

			default:
				message (MT_ERROR, (char *)0, (char *)0,
			"Usage: uahelp -h helpfile [-t title] [-d debugfile]");
				wexit (1);		/* Argument error */
			}

	if (filname == NULL)
		{
		message (MT_ERROR, (char *)0, (char *)0,
			"Usage: uahelp -h helpfile [-t title] [-d debugfile]");
		wexit (1);		/* Argument error */
		}
	if (fil_ptr == NULL)
		{
		message (MT_ERROR, (char*)0, (char *)0, "Can't open help file %s", filname);
		wexit (1);		/* Argument error */
		}

	if (!prc_fil ())			/* Process input file */
		{
		message (MT_ERROR, (char*)0, (char *)0, "Syntax error in help file");
		wexit (2);		/* Bad help file */
		}

	wlabel (win_id, label);
	ix = 0;
	if (title != NULL)		/* Get index of initial display */
		ix = fnd_name (title);
	if (ix == 0)
		ix = 1;
	last_help_ix = ix;

/* Main loop  -  Display help screen and wait for user input */

	while (TRUE)
		{
		dsp_page (ix);
		ch = prc_kbd (ix);
		if (ch == help_intr)
			continue;
		else
		if (ch == help_exit)
			break;
		else
			ix = ch;
		}
	wexit (0);
	}

	/*
	 *  prc_fil  -  Process the specified input file, setting up the
	 *		help_page structure for the display routines
	 */

prc_fil ()

	{
	int	token;
	char	buf [Max_namelen];
	register struct	s_help	*hptr;

	hseek (0);			/* Point to beginning of file */
	help_ix = 0;
	max_help_ix = 0;
	strncpy (names [0], "Table of contents", Max_namelen);
	while ((last_char_read = hgetc ()) != EOF)
		{
		token = get_tok ();	/* Read key word and tokenize */
		if (token == tk_err)
			if (last_char_read == EOF)
				break;
		if (skp_eql () == tk_err)	/* Skip over equals sign */
			token = tk_err;
		switch (token)
			{
			case tk_label:
				if (get_string (label, sizeof(label)) == tk_err)
					{
					wrt_dbg ("***	Error - Label not properly quoted\n");
					return (FALSE);
					}
				break;

			case tk_contents:
				if (max_help_ix > 0)
					{
					wrt_dbg ("***	Error - More than one Contents statement\n");
					return (FALSE);
					}
				help_ix = 1;
				while (get_name (names [help_ix]))
					if (++help_ix >= Max_displays)
						{
						wrt_dbg ("***	Error - Maximum number of displays exceeded\n");
						return (FALSE);
						}
				max_help_ix = help_ix;
				help_ix = 0;
				help_ptr = (struct s_help *)malloc
				  ((max_help_ix + 1) * sizeof (struct s_help));
				if (help_ptr == NULL)
					{
					wrt_dbg ("***	Error - malloc failed \n");
					return (FALSE);
					}
				break;

			case tk_name:
				if (max_help_ix == 0)
					{
					wrt_dbg ("***	Error - Contents not yet defined\n");
					return (FALSE);
					}
				if (help_ix != 0)
					{
					wrt_dbg ("***	Error - Previous display definition not yet complete\n");
					return (FALSE);
					}
				if (get_name (buf))
					{
					wrt_dbg ("***	Error - Only one name can be defined in the Name statment\n");
					return (FALSE);
					}
				if ((help_ix = fnd_name (buf)) == 0)
					{
					wrt_dbg ("***	Error - Name not already defined\n");
					return (FALSE);
					}
				init_page ();		/* Init help struct */
				break;
				
			case tk_llabel:
				if (help_ix == 0)
					{
					wrt_dbg ("***	Error - Llabel definition must be preceeded by Name definition\n");
					return (FALSE);
					}
				hptr = help_ptr + help_ix;
				if (get_string (hptr -> llabel,
				     sizeof (hptr -> llabel)) == tk_err)
					{
					wrt_dbg ("***	Error - Label not properly quoted\n");
					return (FALSE);
					}
				break;

			case tk_slabel:
				if (help_ix == 0)
					{
					wrt_dbg ("***	Error - Slabel definition must be preceeded by Name definition\n");
					return (FALSE);
					}
				hptr = help_ptr + help_ix;
				if (get_string (hptr -> slabel,
				     sizeof (hptr -> slabel)) == tk_err)
					{
					wrt_dbg ("***	Error - Label not properly quoted\n");
					return (FALSE);
					}
				break;

			case tk_branch:
				if (help_ix == 0)
					{
					wrt_dbg ("***	Error - Branch definition must be preceeded by Name definition\n");
					return (FALSE);
					}
				if (get_branch () == tk_err)
					return (FALSE);
				break;

			case tk_title:
				if (help_ix == 0)
					{
					wrt_dbg ("***	Error - Title definition must be preceeded by Name definition\n");
					return (FALSE);
					}
				hptr = help_ptr + help_ix;
				if (get_string (hptr -> title,
				     sizeof (hptr -> title)) == tk_err)
					{
					wrt_dbg ("***	Error - Label not properly quoted\n");
					return (FALSE);
					}
				break;

			case tk_text:
				if (help_ix == 0)
					{
					wrt_dbg ("***	Error - Text definition must be preceeded by Name definition\n");
					return (FALSE);
					}
				get_text ();
				help_ix = 0;
				break;

			case tk_err:
				wrt_dbg ("***	Error - Invalid token\n");
				return (FALSE);
				break;
			}
		}
	dbg_file = NULL;		/* Syntax OK, turn off reporting */
	return (TRUE);
	}

	/*
	 *  init_page  -  Initialize the help_page structure for a new
	 *		screenful.
	 */

init_page ()

	{
	register	i;
	register struct	s_help	*hptr;

	hptr = help_ptr + help_ix;
	hptr -> llabel [0] = 0;			/* Init to null strings */
	hptr -> slabel [0] = 0;
	hptr -> title [0] = 0;
	hptr -> disp = 0;			/* Set no associated text */
	for (i = 0; i < 7; i++)
		hptr -> branch [i] = 0;		/* No branches */
	}

	/*
	 *  get_tok  -  Read a word form the input file and convert to case
	 */

get_tok ()

	{
	char	keyword [Max_namelen];
	int	i = 0;

	skp_fil ();			/* Skip up to keyword */
	while (Valid (last_char_read))
		{			/* Read in keyword */
		keyword [i++] = last_char_read;
		last_char_read = hgetc ();
		if (i >= Max_namelen) break;
		}

	if (i == 0) return (tk_err);	/* Invalid token */
	if (i < Max_namelen) keyword [i] = 0;	/* Terminate string */

	i = 0;
	while (kw_tbl [i] .keyword)
		{
		if (strncmp (kw_tbl [i] .keyword, keyword, Max_namelen) == 0)
			break;		/* Found the keyword */
		i++;
		}
	return (kw_tbl [i] .token);
	}

	/*
	 *  skp_eql  -  Skip over filler and equals sign
	 *
	 *	Returns tk_err if no equals sign found
	 */

skp_eql ()

	{
	skp_fil ();
	if (last_char_read != '=') return (tk_err);

	last_char_read = hgetc ();
	return (0);
	}

	/*
	 *  get_branch  -  Parse a branch statement
	 */

get_branch ()

	{
	register	i;
	register struct	s_help	*hptr;
	char	buf [Max_namelen];
	int	ix;
	char	err_buf [256];

	hptr = help_ptr + help_ix;
	i = 0;
	while (get_name (buf))
		{
		if ((ix = fnd_name (buf)) == 0)
			{
			sprintf (err_buf, "***	Error - The name %s in Branch not already defined\n", buf);
			wrt_dbg (err_buf);
			return (tk_err);
			}
		hptr -> branch [i] = ix;
		if (++i >= 7)
			{
			wrt_dbg ("***	Error - More than seven branches defined\n");
			return (tk_err);
			}
		}
	if ((ix = fnd_name (buf)) == 0)
		{
		sprintf (err_buf, "***	Error - The name %s in Branch not already defined\n", buf);
		wrt_dbg (err_buf);
		return (tk_err);
		}
	hptr -> branch [i++] = ix;
	if (i < 7)
		hptr -> branch [i] = 0;
	}

	/*
	 *  get_name  -  Get name of help display and store in specified buffer
	 *
	 *	Returns TRUE if more names follow
	 */

get_name (ptr)

	char	*ptr;

	{
	int	i = Max_namelen;

	skp_fil ();
	while (i-- > 0)
		{
		*ptr++ = last_char_read;
		last_char_read = hgetc ();
		if (Separator (last_char_read)) break;
		}
	if (i > 0) *ptr++ = 0;
	if (!Separator (last_char_read))
		last_char_read = hgetc ();
	return (last_char_read == ',');
	}

	/*
	 *  fnd_name  -  Look up help display by name, return its index,
	 *		or 0 if not found.
	 */

fnd_name (h_name)

	register char	*h_name;

	{
	register	ix;

	for (ix = 1; ix <= max_help_ix; ix++)
		{
		if (strncmp (h_name, names [ix], Max_namelen) == 0)
			return (ix);
		}
	return (0);
	}

	/*
	 *  get_string  -  Get single line text string surrounded by quotes
	 */

get_string (ptr, max_len)

	char	*ptr;
	int	max_len;

	{
	skp_fil ();
	if (!Quote (last_char_read))
		return (tk_err);
	last_char_read = hgetc ();
	while (max_len-- > 0)
		{
		*ptr++ = last_char_read;
		last_char_read = hgetc ();
		if (Quote (last_char_read)) break;
		if (last_char_read == 0x0a) return (tk_err);
		}
	if (max_len > 0) *ptr++ = 0;
	last_char_read = hgetc ();
	return (0);
	}

	/*
	 *  get_text  -  Set displacement of text area and skip to end
	 */

get_text ()

	{
	register struct	s_help	*hptr;
	char	buf [Max_namelen];

	hptr = help_ptr + help_ix;
	skp_fil ();
	hptr -> disp = fil_disp - 1;
	while ((last_char_read = hgetc ()) != EOF)
		{
		if (last_char_read == '\\')
			{
			get_code (buf);
			if (strcmp (buf, "EOT") == 0)
				break;		/* End of text */
			}
		}
	return (0);
	}

	/*
	 *  skp_fil  -  Skip over inconsequential characters
	 */

skp_fil ()

	{
	while (Filler (last_char_read))
		last_char_read = hgetc ();
	}

	/*
	 *  get_code  -  Get embedded text code from help file
	 */

get_code (ptr)

	register char	*ptr;			/* Points to buffer for code */

	{
	register	i;

	if (*fil_bptr == 0x0a)
		hgetc ();			/* Skip over escaped new line */
	else
	if ((*fil_bptr >= 'A') && (*fil_bptr <= 'Z'))
		{
		i = 0;
		while ((*fil_bptr >= 'A') && (*fil_bptr <= 'Z'))
			{
			if (i++ >= Max_namelen) break;
			fil_disp++;
			*ptr++ = *fil_bptr++;
			}
		if (*fil_bptr == '\\')
			hgetc ();
		}
	else
	if (*fil_bptr == '\\')
		{
		*ptr++ = '\\';
		hgetc ();
		}
	*ptr = 0;
	}

	/*
	 *  hgetc  -  Get next character from help file
	 */

hgetc ()

	{
	if (*fil_bptr == 0)
		{
		fil_bptr = fgets (fil_buf, sizeof (fil_buf), fil_ptr);
		if (fil_bptr == NULL)
			return (EOF);
		wrt_dbg (fil_buf);
		}
	fil_disp++;
	return (*fil_bptr++);
	}

	/*
	 *  wrt_dbg  -  Write string to debug file, if it exists
	 */

wrt_dbg (ptr)

	register char	*ptr;

	{
	if (dbg_file != NULL)
		fputs (ptr, dbg_file);
	}

	/*
	 *  hseek  -  Seek to specified displacement in help file
	 */

hseek (disp)

	long	disp;

	{
	fseek (fil_ptr, disp, 0);
	fil_disp = disp;
	fil_bptr = fil_buf;
	*fil_bptr = 0;
	}

	/*
	 *  sc  -  Signal catcher
	 */

sc (sig)

	int	sig;		/* Signal that was caught */

	{
	signal (sig, sc);
	}
