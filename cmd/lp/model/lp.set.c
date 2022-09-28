/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:model/lp.set.c	1.4.2.1"

#include "stdio.h"

#include "lp.h"
#include "lp.set.h"

extern char		*getenv();

/**
 ** main()
 **/

int			main (argc, argv)
	int			argc;
	char			*argv[];
{
	static char		not_set[10]	= "H V W L S";

	int			exit_code;

	char			*TERM		= getenv("TERM");


	if (!TERM || !*TERM || tidbit(TERM, (char *)0) == -1)
		exit (1);

	/*
	 * Very simple calling sequence:
	 *
	 *	lpset horz-pitch vert-pitch width length char-set
	 *
	 * The first four can be scaled with 'i' (inches) or
	 * 'c' (centimeters). A pitch scaled with 'i' is same
	 * as an unscaled pitch.
	 * Blank arguments will skip the corresponding setting.
	 */
	if (argc != 6)
		exit (1);

	exit_code = 0;

	if (argv[1][0]) {
		switch (set_pitch(argv[1], 'H', 1)) {
		case E_SUCCESS:
			not_set[0] = ' ';
			break;
		case E_FAILURE:
			break;
		default:
			exit_code = 1;
			break;
		}
	} else
		not_set[0] = ' ';

	if (argv[2][0]) {
		switch (set_pitch(argv[2], 'V', 1)) {
		case E_SUCCESS:
			not_set[2] = ' ';
			break;
		case E_FAILURE:
			break;
		default:
			exit_code = 1;
			break;
		}
	} else
		not_set[2] = ' ';

	if (argv[3][0]) {
		switch (set_size(argv[3], 'W', 1)) {
		case E_SUCCESS:
			not_set[4] = ' ';
			break;
		case E_FAILURE:
			break;
		default:
			exit_code = 1;
			break;
		}
	} else
		not_set[4] = ' ';

	if (argv[4][0]) {
		switch (set_size(argv[4], 'L', 1)) {
		case E_SUCCESS:
			not_set[6] = ' ';
			break;
		case E_FAILURE:
			break;
		default:
			exit_code = 1;
			break;
		}
	} else
		not_set[6] = ' ';

	if (argv[5][0]) {
		switch (set_charset(argv[5], 1, TERM)) {
		case E_SUCCESS:
			not_set[8] = ' ';
			break;
		case E_FAILURE:
			break;
		default:
			exit_code = 1;
			break;
		}
	} else
		not_set[8] = ' ';

	fprintf (stderr, "%s\n", not_set);

	exit (exit_code);
	/*NOTREACHED*/
}
