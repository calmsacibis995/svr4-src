/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:help_ds.c	1.1"
	/*
	 *	Display module for the help process
	 */

#include <stdio.h>
#include <tam.h>

#include "help.h"

#define	atr_ul	0x100		/* Underline attribute */
#define	atr_rv	0x200		/* Reverse video attribute */

#define	atr_mask	0xff00		/* Mask for attribute bits */
#define	chr_mask	0x00ff		/* Mask for character bits */

	/*
	 *  Global variables in this module
	 */

short	hlins [Max_lines] [80];	/* At most n lines of 80 chars per display */

short	catr;			/* Current attributes */
int	first_line;		/* First line of display which is on screen */
int	last_line;		/* Last line of display */

int	last_slk_ix;		/* Last key labeled */
int	slk_offset [8] =	/* Offset of keys in SLK line */
	{1, 10, 19, 31, 40, 52, 61, 70};
char	ll1 [80];		/* Long label line one */
char	ll2 [80];		/* Long label line two */
char	sl [80];		/* Short label line */

	/*
	 *  dsp_page  -  Display the specified help screen
	 */

dsp_page (ix)

	int	ix;		/* Index into help_page for this display */

	{
	register struct	s_help	*hptr;
	register short	*line_ptr;
	register char	*ptr;
	register	i;

	if (ix == 0)
		{		/* Index page display is special */
		dsp_index ();
		return;
		}
	hptr = help_ptr + ix;
	dsp_branch (hptr -> branch);	/* Display function key line */

	hlins [0] [0] = 0;		/* Blank top line */
	ptr = hptr -> title;
	line_ptr = hlins [1];		/* Next line contains title */
	i = 0;
	while (*ptr)
		{
		*line_ptr++ = *ptr++ | atr_ul;
		i++;
		if ((i >= scrn_wd) || (i >= sizeof (hptr -> title)))
			break;
		}
	*line_ptr = 0;
	lcenter (1, 0, i);			/* Center the title */
	hlins [2] [0] = 0;		/* Blank following line */

	hseek (hptr -> disp);
	fmt_text (3);			/* Now format the text */
	first_line = -1;
	dsp_screen (0);
	last_help_ix = ix;
	}

	/*
	 *  fmt_text  -  Read text from help file and output to 
	 *		the hlins array, formatted
	 */

fmt_text (cur_line)

	int	cur_line;

	{
	register short	*line_ptr;
	register short	atr;
	register	chr;
	register	col;

	char	buf [Max_namelen];
	char	center_flag;
	short	*sline_ptr;
	long	sdisp;

	int	l_mar;			/* Left margin */
	int	center_col;		/* Column to start centering at */

	line_ptr = hlins [cur_line];
	atr = 0;
	col = 0;
	l_mar = 2;
	center_flag = FALSE;
	sline_ptr = NULL;
	while ((chr = hgetc ()) != EOF)
		{
		while (col < l_mar)
			{
			*line_ptr++ = 0x20;
			col++;
			}
		if (chr == '\\')
			{
			get_code (buf);
			if (strcmp (buf, "EOT") == 0)
				{
				*line_ptr = 0;
				if (center_flag)
					lcenter (cur_line, center_col,
							col - center_col);
				cur_line++;
				break;
				}
			else
			if (strcmp (buf, "CEN") == 0)
				{
				center_flag = TRUE;
				center_col = col;
				}
			else
			if (strcmp (buf, "IND") == 0)
				{
				sline_ptr = line_ptr;
				sdisp = fil_disp;
				do
					{
					*line_ptr++ = 0x20 | atr;
					col++;
					}
				while ((col < scrn_wd) && ((col - 2) & 0x07));
				if (col < scrn_wd)
					l_mar = col;
				}
			else
			if (strcmp (buf, "UL") == 0)
				atr |= atr_ul;
			else
			if (strcmp (buf, "US") == 0)
				atr &= ~atr_ul;
			else
			if (strcmp (buf, "BL") == 0)
				atr |= atr_rv;
			else
			if (strcmp (buf, "BS") == 0)
				atr &= ~atr_rv;
			else
			if (*buf)
				{
				*line_ptr++ = *buf | atr;
				col++;
				}
			}
		else
		if (chr == 0x09)
			{
			sline_ptr = line_ptr;
			sdisp = fil_disp;
			do
				{
				*line_ptr++ = 0x20 | atr;
				col++;
				}
			while ((col < scrn_wd) && ((col - 2) & 0x07));
			}
		else
		if (chr == 0x0a)
			{		/* Forced line end */
			*line_ptr = 0;
			if (center_flag)
				lcenter (cur_line, center_col,
							col - center_col);
			if (++cur_line >= Max_lines)
				break;
			line_ptr = hlins [cur_line];
			col = 0;
			l_mar = 2;
			center_flag = FALSE;
			sline_ptr = NULL;
			}
		else
		if (chr == 0x20)
			{
			sline_ptr = line_ptr;
			sdisp = fil_disp;
			*line_ptr++ = chr | atr;
			col++;
			}
		else
			{
			*line_ptr++ = chr | atr;
			col++;
			}
		if (col >= scrn_wd)
			{		/* Time to wrap */
			if (sline_ptr)
				{		/* If wrap point found */
				line_ptr = sline_ptr;
				while (chr == 0x20)
					{
					sdisp = fil_disp;
					chr = hgetc ();
					}
				hseek (sdisp);
				}
			*line_ptr = 0;
			if (++cur_line >= Max_lines)
				break;
			line_ptr = hlins [cur_line];
			col = 0;
			center_flag = FALSE;
			sline_ptr = NULL;
			}
		}
	last_line = cur_line;
	}

	/*
	 *  lcenter  -  Center the specified line
	 */

lcenter (cur_line, col, len)

	int	cur_line;
	int	col;
	register	len;

	{
	register	adjust;
	register short	*line_ptr;

	adjust = (scrn_wd - len) >> 1;
	adjust -= col;
	if (adjust > 0)
		{
		line_ptr = hlins [cur_line] + col + len++;
		while (len-- > 0)
			{
			*(line_ptr + adjust) = *line_ptr;
			line_ptr--;
			}
		line_ptr = hlins [cur_line] + col;
		while (adjust-- > 0)
			*line_ptr++ = 0x20;
		}
	}

	/*
	 *  dsp_branch  -  Display function keys for specified page
	 */

dsp_branch (branch)

	int	*branch;		/* Branches to other labels */

	{
	register struct	s_help	*hptr;
	register	ix;

	last_slk_ix = 0;
	h_wslk (win_id, 1, "TABLE OFCONTENTS", "CONTENTS");
	for (ix = 2; ix <= 8; ix++)
		{
		if (*branch == 0)
			h_wslk (win_id, ix, "", "");
		else
			{
			hptr = help_ptr + *branch++;
			h_wslk (win_id, ix, hptr -> llabel, hptr -> slabel);
			}
		}
	wslk (win_id, 0, ll1, ll2, sl);		/* Actually write the SLK's */
	}

	/*
	 *  dsp_beg  -  Display first screenful of text
	 */

dsp_beg ()

	{
	dsp_screen (0);
	}

	/*
	 *  dsp_end  -  Display last screenful of text
	 */

dsp_end ()

	{
	dsp_screen (Max (0, last_line - (scrn_sz - 2)));
	}

	/*
	 *  dsp_next  -  Display next screenful of text
	 */

dsp_next ()

	{
	dsp_screen (Min (last_line - 2, first_line + (scrn_sz - 2)));
	}

	/*
	 *  dsp_prev  -  Display previous screenful of text
	 */

dsp_prev ()

	{
	dsp_screen (Max (0, first_line - (scrn_sz - 2)));
	}

	/*
	 *  dsp_sup  -  Scroll display up by one-quarter screen
	 */

dsp_sup ()

	{
	dsp_screen (Min (last_line - 2, first_line + (scrn_sz >> 2)));
	}

	/*
	 *  dsp_sdn  -  Scroll display down by 5 lines
	 */

dsp_sdn ()

	{
	dsp_screen (Max (0, first_line - (scrn_sz >> 2)));
	}

	/*
	 *  dsp_screen  -  Display screenful from given line
	 */

dsp_screen (cur_line)

	register	cur_line;	/* Line to put at top of screen */

	{
	register	scrn_ln;
	register	n;

	if ((first_line == -1) ||
	    (Absdif (cur_line, first_line) > (scrn_sz >> 1)))
		{			/* Initialize display */
		first_line = cur_line;
/***
 *** The following line:
 ***
 ***  -- Clears all current character attributes
 ***  -- I do not really know
 ***  -- Makes the cursor visible and blinking
 ***
 ***	wputs (win_id, "\033[0m\033[=0w\033[=1C");
 ***/
		catr = 0;
		for (scrn_ln = 0; scrn_ln < scrn_sz; scrn_ln++)
			{
			wgoto (win_id, scrn_ln, 0);
			if (cur_line >= last_line)
				dsp_text (0);
			else
				dsp_text (hlins [cur_line++]);
			}
		}
	else
	if (cur_line > first_line)
		{		/* Scroll up */
		wgoto (win_id, 0, 0);
		n = cur_line - first_line;
/***
 *** The following line deletes lines at the top of the screen
 ***
 ***		wprintf (win_id, "\033[%dM", n);
 ***/
		{
			int i;
			for(i=n; i--; ) TAMdeleteln( (short)win_id );
		}
					/* Delete lines at top of screen */
		first_line = cur_line;
		cur_line += (scrn_sz - n);
		for (scrn_ln = scrn_sz - n; scrn_ln < scrn_sz; scrn_ln++)
			{
			wgoto (win_id, scrn_ln, 0);
			if (cur_line >= last_line)
				dsp_text (0);
			else
				dsp_text (hlins [cur_line++]);
			}
		}
	else
	if (cur_line < first_line)
		{		/* Scroll down */
		wgoto (win_id, 0, 0);
		n = first_line - cur_line;
/***
 *** The following line inserts lines at the top of the screen
 ***
 ***		wprintf (win_id, "\033[%dL", n);
 ***/
		{
			int i;
			for(i=n;i--;) TAMinsertln( (short)win_id );
		}

					/* Insert lines at top of screen */
		first_line = cur_line;
		for (scrn_ln = 0; scrn_ln < n; scrn_ln++)
			{
			wgoto (win_id, scrn_ln, 0);
			if (cur_line >= last_line)
				dsp_text (0);
			else
				dsp_text (hlins [cur_line++]);
			}
		}
	if ((first_line + scrn_sz) < last_line)
		wprompt (win_id, "Press PAGE for more information");
	else
		wprompt (win_id, "");
	}

	/*
	 *  dsp_text  -  Display a line of text from the text buffer
	 */

dsp_text (line_ptr)

	register short	*line_ptr;

	{
	if (line_ptr)
		while (*line_ptr)
			{
			if (catr != (*line_ptr & atr_mask))
				{		/* Fix attributes for char */
/***
 *** Ignore attributes for now
				catr = *line_ptr & atr_mask;
				wputs (win_id, "\033[0");
				if (catr & atr_ul)
					wputs (win_id, ";4");
				if (catr & atr_rv)
					wputs (win_id, ";7");
				wputc (win_id, 'm');
***/
				}

			wputc (win_id, *line_ptr++ & chr_mask);
			}
	TAMclrtoeol( (short)win_id );
	}

	/*
	 *  h_wslk  -  Higher performance wslk routine (reduces system calls)
	 */

h_wslk (win_id, slk_ix, l_label, s_label)

	int	win_id;
	int	slk_ix;
	char	*l_label;
	char	*s_label;

	{
	register	char	*ptr;
	register	char	*sptr;
	register	int	ix;

	if (last_slk_ix == 0)
		{		/* Init label lines */
		for (ix = 0, ptr = ll1; ix < 80; *ptr++ = 0x20, ix++);
		for (ix = 0, ptr = ll2; ix < 80; *ptr++ = 0x20, ix++);
		for (ix = 0, ptr = sl; ix < 80; *ptr++ = 0x20, ix++);
		}
	last_slk_ix = slk_ix;

	ptr = &ll1 [slk_offset [slk_ix - 1]];
	sptr = l_label;
	for (ix = 0; ix < 8; ix++)
		if (*sptr)
			*ptr++ = *sptr++;
		else
			break;

	ptr = &ll2 [slk_offset [slk_ix - 1]];
	for (ix = 0; ix < 8; ix++)
		if (*sptr)
			*ptr++ = *sptr++;
		else
			break;

	ptr = &sl [slk_offset [slk_ix - 1]];
	sptr = s_label;
	for (ix = 0; ix < 8; ix++)
		if (*sptr)
			*ptr++ = *sptr++;
		else
			break;
	}

	/*
	 *  kb_beep  -  Beep the keyboard
	 */

kb_beep ()

	{
	beep();
	}
