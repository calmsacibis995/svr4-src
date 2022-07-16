/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/ring.c	1.7"

#include	"curses_inc.h"

_ring(bf)
bool	bf;
{
#ifdef __STDC__
    extern	int	_outch(char);
#else
    extern	int	_outch();
#endif
    static	char	offsets[2] = {45 /* flash_screen */, 1 /* bell */ };
    char	**str_array = (char **) cur_strs;
#ifdef	DEBUG
    if (outf)
	fprintf(outf, "_ring().\n");
#endif	/* DEBUG */
    _PUTS(str_array[offsets[bf]] ? str_array[offsets[bf]] : str_array[offsets[1 - bf]], 0);
    (void) fflush(SP->term_file);
    if (_INPUTPENDING)
	(void) doupdate();
    return (OK);
}
