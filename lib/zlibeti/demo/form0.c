/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:demo/form0.c	1.1"
/*
	display sample form and exit
*/
#include <string.h>
#include <form.h>

FIELD * make_label (frow, fcol, label)
int frow;	/* first row		*/
int fcol;	/* first column		*/
char * label;	/* label		*/
{
	FIELD * f = new_field (1, strlen (label), frow, fcol, 0, 0);

	if (f)
	{
		set_field_buffer (f, 0, label);
		field_opts_off (f, O_ACTIVE);
	}
	return f;
}

FIELD * make_field (frow, fcol, cols)
int frow;	/* first row		*/
int fcol;	/* first column		*/
int cols;	/* number of columns	*/
{
	FIELD * f = new_field (1, cols, frow, fcol, 0, 0);

	if (f)
		set_field_back (f, A_UNDERLINE);
	return f;
}

main ()
{
	FORM *		form;
	FIELD *		f[6];
	int		i = 0;
/*
	curses initialization
*/
	initscr ();
	nonl ();
	raw ();
	noecho ();
	wclear (stdscr);
/*
	create fields
*/
	f[0] = make_label (0, 7, "Sample Form");
	f[1] = make_label (2, 0, "Field 1:");
	f[2] = make_field (2, 9, 16);
	f[3] = make_label (3, 0, "Field 2:");
	f[4] = make_field (3, 9, 16);
	f[5] = (FIELD *) 0;
/*
	create and display form
*/
	form = new_form (f);
	post_form (form);
	wrefresh (stdscr);
	sleep (5);
/*
	erase form and free both form and fields
*/
	unpost_form (form);
	wrefresh (stdscr);
	free_form (form);

	while (f[i])
		free_field (f[i++]);
/*
	curses termination
*/
	endwin ();
	exit (0);
}
