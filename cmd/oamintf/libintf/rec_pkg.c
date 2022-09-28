/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:libintf/rec_pkg.c	1.2.2.2"

/*LINTLIBRARY*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "intf.h"

extern void	*malloc();
extern int	search_pkg();

int
record_pkg(ptr_menu, pkginst)
struct menu_line *ptr_menu;		/* ptr to menu item */
char *pkginst;				/* package instance */
{
	struct menu_line *mnu_ptr;	/* volatile pointer to menu item */
	char *line;			/* pointer to actual line */
	int i;				/* misc counter */
	int len;			/* total line length */
	int done = 0;			/* done flag */

	if(search_pkg(ptr_menu, pkginst, MENU) == FOUND) return(0);
	mnu_ptr = ptr_menu;

	while(!done) {	/* find *end* of current line */
		line = mnu_ptr->line;
		i = 0;
		while(*(line+i++));	/* two past last char in line */
		i -= 2;
		while(isspace(*(line+(i--))));
		i++;
		if(*(line+i) == '\\')	/* go to next */
			mnu_ptr = mnu_ptr->next;
		else done = 1;
	}

	len = strlen(pkginst) + 5;	/* 2 "#"'s, "^", a "\n" 
					   and cont. ('\') if needed */
	if((i + len) > LNSZ) {		/* add a new line */
		mnu_ptr->next = (struct menu_line *) 
					malloc(sizeof(struct menu_line));
		*(line+i+1) = '\\';	/* add continuation char */
		*(line+i+2) = '\n';
		line = mnu_ptr->next->line;
		i = 0;
	}
	else *(line+i+1) = NULL;

	/* NOW add pkginst to end of line */
	(void) strcat(line, TAB_DELIMIT);
	(void) strcat(line, "#");
	(void) strcat(line, pkginst);
	(void) strcat(line, "#");
	(void) strcat(line, "\n");
	return(1);
}

int
search_pkg(ptr_menu, pkginst, type)
struct menu_line *ptr_menu;		/* pointer to line structure */
char *pkginst;				/* pkginst to search for */
int type;				/* MENU or EXPRESS */
{
	char *line;			/* pointer to actual line */
	struct menu_line *mnu_ptr;	/* pointer to menu line structure */
	int notuniq;			/* notuniq EXPRESS line */

	/*
	 * search_pkg(ptr_menu, pkginst) searches the menu line defined
	 * by ptr_menu for the pkginst identifier in pkginst.
	 * Returns FOUND if pkginst associated with menu line, otherwise
	 * returns NOTFOUND.
	 */

	mnu_ptr = ptr_menu;
	line = ptr_menu->line;

	/*
	 * skip over name^descr^action - action may be placeholder^action
	 * or action^placeholder - so also skip over either
	 * name^descr^[placeholder^]action   OR
	 * name^descr^action[^placeholder]
	 */

	while(*line) {
		if(*line++ == TAB_CHAR)
			break;	/* skip to descr beginning */
	}
	if(type == EXPRESS) {
		/* check and flag if it's nonunique */
		if(strncmp(line, NONUNIQUE, sizeof(NONUNIQUE)-1) == 0)
			notuniq = 1;
		else if(strncmp(line, PHOLDER, strlen(PHOLDER)-1) == 0) {
			return(0);	/* 0 pkginst's on placeholder line */
		}
	}
	while(*line) {
		if(*line++ == TAB_CHAR)
			break;	/* skip to action */
	}

	if(strncmp(line, PHOLDER, sizeof(PHOLDER)-1) == 0) {
		/* skip to real action */
		while(*line) {
			if(*line++ == TAB_CHAR)
				break;	/* skip to action */
		}
	}
	/* skip to what's beyond action */
	if(!notuniq) {
		while(*line && (*line != TAB_CHAR)
		&& (*line != '\\') && (*line != '\n')) 
			line++;
	}
	if((type == MENU) && (*line == TAB_CHAR)) { 
		/* could be placeholder or could be pkginst info */
		/* check if placeholder, then skip over that */
		line++;
		if((strncmp(line, PHOLDER, sizeof(PHOLDER)-1) == 0) 
			|| (*line == '[')) {
			/* *line == '[' when rename field present */
			/* skip to what's beyond */
			while((*line != TAB_CHAR) && (*line != NULL) 
			&& (*line != '\\') && (*line != '\n')) 
				line++;		
		}
	}
	for(;;) {
		if((*line == '\n') || (*line == NULL)) return(NOTFOUND);
		if(*line == '\\') {
			mnu_ptr = mnu_ptr->next;
			if(mnu_ptr == NULL) return(NOTFOUND);
			line = mnu_ptr->line;
		}
		if(*line == TAB_CHAR) line++;
		if(*line == '#') {
			line++;
			if(strncmp(line, pkginst, strlen(pkginst)) == 0)
				return(FOUND);
		}

		/* skip to next */
		while((*line != TAB_CHAR) && (*line != NULL) 
			&& (*line != '\\') && (*line != '\n')) line++;		
	
	}
}
