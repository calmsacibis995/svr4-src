/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:libintf/read_item.c	1.3.2.2"

/*LINTLIBRARY*/
#include <stdio.h>
#include <ctype.h>
#include "intf.h"

extern void	*malloc();
extern void	free();

char newline[LNSZ];		/* used to get newline if don't malloc */

char *
read_item(menu_ptr, fileptr, full_line)
struct menu_line *menu_ptr;	/* menu line */
FILE *fileptr;			/* file to read from */
int full_line;			/* flag to read & store full line or not*/
{
	/*
	 * read_item(menu_ptr, fileptr, full_line) - routine to read the
	 * next menu item from a menu file.  This routine takes care of
	 * reading multiple line menu items - it looks for the continuation
	 * char ('\') at the end of the line  - if it's there and if
	 * full_line is set, it malloc's space for another menu_line
	 * structure and reads the next line into it.  If full_line is not
	 * set the routine simply reads the next lines until it reaches the
	 * last line of the item.  If full_line is not set, no malloc's
	 * occur.  This was added as a performance improvement - since this
	 * routine is used by installation (mod_menus) as well as the 
	 * object generator in the interface - it would not be a good idea
	 * to be doing malloc's from within the object generator if they're
	 * not necessary.  This takes advantage of the fact that menu 
	 * information will ALWAYS be contained on the first line of a menu
	 * item.  Other lines will contain only package instance identifiers.
	 * So from within the object generator, the other lines are simply
	 * read and thrown away by this routine.
	 * Return code:  a pointer to the first line of the menu item if
	 * one was read and NULL if no menu item was available (no lines read).
	 */

	char *lineptr;			/* pointer to line structure */
	struct menu_line *curptr;	/* current menu ptr */
	int done;			/* done flag */

	/* first free any already allocated space */
	/* take advantage of undisturbed contents after freeing space */
	if(full_line) {
		curptr = menu_ptr;
		while(curptr->next != NULL) {
			curptr = curptr->next;
			free(curptr);
		}
	}

	curptr = menu_ptr;
	done = 0;
	menu_ptr->next = NULL;
	lineptr = menu_ptr->line;

	if(fgets(menu_ptr->line, sizeof(menu_ptr->line), fileptr) == NULL)
		return(NULL);
	else {
		while(!done) {
			/* see if continuation char '\' is at end of line */
			/* find end of line */
			while(*lineptr++);

			/* lineptr now points to two past last char in line */
			lineptr-=2;
			if ( lineptr < menu_ptr->line ) lineptr = menu_ptr->line;
			while(isspace(*lineptr--));
			lineptr++;
			if ( lineptr < menu_ptr->line ) lineptr = menu_ptr->line;
	
			if(*lineptr == '\\') { /* continuation: get next line */
				if(full_line) {
					/* malloc space */
					curptr->next = (struct menu_line *) 
					       malloc(sizeof(struct menu_line));
					curptr = curptr->next;
					lineptr = curptr->line;
				}
				else lineptr = newline;
				if(fgets(lineptr, sizeof(curptr->line), 
					fileptr) == NULL) done = 1;
			}
			else done = 1;
			if(full_line) curptr->next = NULL;
		}
		return(menu_ptr->line);
	}
}
