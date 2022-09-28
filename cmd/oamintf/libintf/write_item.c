/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:libintf/write_item.c	1.1.2.2"

#include <stdio.h>
#include <ctype.h>
#include "intf.h"

int
write_item(menu_ptr, file_ptr)
struct menu_line *menu_ptr;		/* pointer to menu line */
FILE *file_ptr;				/* file to write to */

{
	struct menu_line *mnu_ptr;	/* pointer to menu line */

	if((menu_ptr == NULL) || ((!isprint(*(menu_ptr->line)))
		&& (*(menu_ptr->line) != '\n')))
		return(-1);
	mnu_ptr = menu_ptr;
	while(mnu_ptr != NULL) {
		(void) fputs(mnu_ptr->line, file_ptr);	
		mnu_ptr = mnu_ptr->next;
	}
	return(0);
	
}
