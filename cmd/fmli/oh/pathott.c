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
#ident	"@(#)fmli:oh/pathott.c	1.8"

#include <stdio.h>
#include <sys/types.h>		/* EFT abs k16 */
#include "typetab.h"
#include "sizes.h"

extern int Vflag;

struct ott_entry *
path_to_ott(path)
char	*path;
{
    register char	*name;
    register struct ott_entry	*entry;
    struct ott_entry	*name_to_ott();
    struct ott_entry	*dname_to_ott();
    char	*filename();
    char	*parent();
    char	*nstrcat();
    char	*path_to_title();

    if (make_current(parent(path)) == O_FAIL) {
	if (Vflag)
	    mess_temp(nstrcat("Could not open folder ",
			      path_to_title(parent(path), NULL, MESS_COLS-22), NULL));
	else
	    mess_temp("Command unknown, please try again");
	return(NULL);
    }
    if ((entry = name_to_ott(name = filename(path))) == NULL &&
	(entry = dname_to_ott(name)) == NULL) {
 /*
  * Backedup the changes to test the valid fmli name
  */
  /*
	if ( strncmp("Text", name, 4) == 0 ||
	     strncmp("Menu", name, 4) == 0 ||
	     strncmp("Form", name, 4) == 0 )    */
  /* Changed the message. Removed the word object  */
	    mess_temp(nstrcat("Could not access ", name, NULL));
  /*
	else
	    mess_temp("Command unknown, please try again");   */
	return(NULL);
    }
    return(entry);
}
