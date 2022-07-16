/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/scr_ll_dump.c	1.5"
#include	"curses_inc.h"
#include	<sys/types.h>
#include	<sys/stat.h>

extern	char	*ttyname();

scr_ll_dump(filep)
register	FILE	*filep;
{
    short	magic = SVR3_DUMP_MAGIC_NUMBER, rv = ERR;
    char	*thistty;
    SLK_MAP	*slk = SP->slk;
    struct	stat	statbuf;

    if (fwrite((char *) &magic, sizeof(short), 1, filep) != 1)
	goto err;

    /* write term name and modification time */
    if ((thistty = ttyname(cur_term->Filedes)) == NULL)
	statbuf.st_mtime = 0;
    else
	(void) stat(thistty, &statbuf);

    if (fwrite((char *) &(statbuf.st_mtime), sizeof(time_t), 1, filep) != 1)
	goto err;

    /* write curscr */
    if (_INPUTPENDING)
	(void) force_doupdate();
    if (putwin(curscr, filep) == ERR)
	goto err;

    /* next output: 0 no slk, 1 hardware slk, 2 simulated slk */

    magic = (!slk) ? 0 : (slk->_win) ? 2 : 1;
    if (fwrite((char *) &magic, sizeof(int), 1, filep) != 1)
	goto err;
    if (magic)
    {
	short	i, labmax = slk->_num, lablen = slk->_len + 1;

	/* output the soft labels themselves */
	if ((fwrite((char *) &labmax, sizeof(short), 1, filep) != 1) ||
	    (fwrite((char *) &lablen, sizeof(short), 1, filep) != 1))
	{
	    goto err;
	}
	for (i = 0; i < labmax; i++)
	    if ((fwrite(slk->_ldis[i], sizeof(char), lablen, filep) != lablen) ||
		(fwrite(slk->_lval[i], sizeof(char), lablen, filep) != lablen))
	    {
		goto err;
	    }
    }

    /* now write information about colors.  Use the following format.	*/
    /* Line 1 is mandatory, the remaining lines are required only if 	*/
    /* line one is 1.							*/
    /* line 1: 0 (no colors) or 1 (colors)				*/
    /* line 2: number of colors, number of color pairs, can_change	*/
    /* X lines: Contents of colors (r, g, b)				*/
    /* Y lines: Contents of color-pairs					*/

    magic = ((cur_term->_pairs_tbl) ? 1 : 0);
    if (fwrite((char *) &magic, sizeof(int), 1, filep) != 1)
	goto err;
    if (magic)
    {
	/* number of colors and color_pairs	*/
	if ((fwrite((char *) &COLORS, sizeof(int), 1, filep) != 1) ||
	    (fwrite((char *) &COLOR_PAIRS, sizeof(int), 1, filep) != 1) ||
	    (fwrite((char *) &can_change, sizeof(char), 1, filep) != 1))
	    goto err;

	/* contents of color_table		*/

	if (can_change)
	{
	    if (fwrite((char *) &(cur_term->_color_tbl->r),
				sizeof(_Color), COLORS, filep) != COLORS)
		goto err;
	}

	/* contents of pairs_table		*/

	if (fwrite((char *) &(cur_term->_pairs_tbl->foreground),
		sizeof(_Color_pair), COLOR_PAIRS, filep) != COLOR_PAIRS)
	    goto err;
    }

    /* success */
    rv = OK;
err :
    return (rv);
}
