/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/scr_all.c	1.3"
#include	"curses_inc.h"

/* Set <screen> idea of the screen image to that stored in "file". */

_scr_all(file,which)
char	*file;
int	which;
{
    int		rv;
    FILE	*filep;

    if ((filep = fopen(file,"r")) == NULL)
	return (ERR);
    rv = scr_reset(filep,which);
    fclose(filep);
    return (rv);
}
