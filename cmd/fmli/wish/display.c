/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)fmli:wish/display.c	1.6"

#include <stdio.h>
#include <sys/types.h>		/* EFT abs k16 */
#include "wish.h"
#include "typetab.h"
#include "sizes.h"

int
glob_display (path)
char	*path;
{
    char	*vpath;
    char	title[PATHSIZ];
    struct	ott_entry *path_to_ott();
    char	*path_to_vpath();
    char	*bsd_path_to_title();
    struct	ott_entry *ott, *path_to_ott();

    if ((vpath = path_to_vpath(path)) == NULL) {
	if ( access(path,00) )
	    mess_temp(nstrcat(bsd_path_to_title(path,MESS_COLS - 16)," does not exist.",NULL));
	else
	    mess_temp(nstrcat(bsd_path_to_title(path,MESS_COLS - 20)," is not displayable.",NULL));
	return(FAIL);
    }
    ott = path_to_ott(path);
    sprintf(title, "%s/%s", parent(path), ott->dname);
    return(objop("OPEN", "TEXT", "$VMSYS/OBJECTS/Text.disp", vpath,
	bsd_path_to_title(title,MAX_TITLE - 3 - strlen(ott->display)),
	ott->display, NULL));
}
