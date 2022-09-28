/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/printers/chkprinter.c	1.8.2.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "sys/types.h"

#include "lp.h"
#include "lp.set.h"
#include "printers.h"

extern short		output_res_char,
			output_res_line,
			output_res_horz_inch,
			output_res_vert_inch;

/**
 ** chkprinter() - CHECK VALIDITY OF PITCH/SIZE/CHARSET FOR TERMINFO TYPE
 **/

unsigned long
#if	defined(__STDC__)
chkprinter (
	char *			type,
	char *			cpi,
	char *			lpi,
	char *			len,
	char *			wid,
	char *			cs
)
#else
chkprinter (type, cpi, lpi, len, wid, cs)
	char			*type,
				*cpi,
				*lpi,
				*len,
				*wid,
				*cs;
#endif
{
	register unsigned long	retflags	= 0;


	if (tidbit(type, (char *)0) == -1)
		return (retflags | PCK_TYPE);

	output_res_char = -1;
	output_res_line = -1;
	output_res_horz_inch = -1;
	output_res_vert_inch = -1;

	if (cpi && *cpi && set_pitch(cpi, 'H', 0) != E_SUCCESS)
		retflags |= PCK_CPI;

	if (lpi && *lpi && set_pitch(lpi, 'V', 0) != E_SUCCESS)
		retflags |= PCK_LPI;

	if (len && *len && set_size(len, 'L', 0) != E_SUCCESS)
		retflags |= PCK_LENGTH;

	if (wid && *wid && set_size(wid, 'W', 0) != E_SUCCESS)
		retflags |= PCK_WIDTH;

	if (cs && *cs && set_charset(cs, 0, type) != E_SUCCESS)
		retflags |= PCK_CHARSET;

	return (retflags);
}
