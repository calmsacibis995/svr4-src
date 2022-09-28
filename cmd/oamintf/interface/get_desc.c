/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:interface/get_desc.c	1.3.1.2"
#include <stdio.h>
#include <string.h>
#include "local.h"

extern char	*menutok();

static char Menumark[] = "#menu#";	/* menu line prefix */
static char Helpmark[] = "#help#";	/* help line prefix */
static char Lifemark[] = "#life#";	/* lifetime line prefix - not used? */
static char Identmark[] = "#ident";	/* star ident marker */
/*
 * This is added so now you can specify the size of a menu by adding a
 * entry in such a way "#rows#15" in the .menu file. The object_gen will
 * insert a line "rows=15" to the Menu. file.
 */
static char Rowsmark[] = "#rows#";	/* rows line prefix */

int identfound = 0;			/* flag if ident line found */
int menufound = 0;			/* flag if menu line found */
int helpfound = 0;			/* flag if help line found */
int rowsfound = 0;			/* flag if rows line found */

int
get_desc(inbuf, desc, title)
char *inbuf;	/* char array containing line to identify */
char **desc;	/* pointer to char pointer - set to beginning of description */
char **title;	/* " - set to beginning of help title, if its a help line */
{
	/* 
	 * get_desc(inbuf, desc, title) - get_desc parses the line
	 * in the array pointed to by "inbuf".  If its a menu identification
	 * line (indicated by the prefix "#menu#"), get_desc sets *desc
	 * to point to the first character the description (the char 
	 * immediately following the prefix).  If its a help ident line
	 * (indicated by the prefix "#help#"), get_desc sets *desc
	 * to point to the first character of the help file name and
	 * *title points to the first character of the help title.
	 * Life ident lines indicate the lifetime of the menu object.
	 * (lifetime is an FMLI identifier).  If a life ident line
	 * (indicated by the prefix "#life#") is present, *desc
	 * is set to point to the first character immediately following
	 * the prefix.  No checks are made on the validity of the life
	 * descriptor.  That is passed directly to FMLI where the check
	 * is made.
	 */

	char *p;		/* pointer to position in string */
	char *ret;		/* return - pointer to description */
	char *tmpdesc;		/* tmp desc pointer for use with menutok */
	char *tmptitle;		/* tmp title pointer for use with menutok */

	/* check for star ident line */
	/* set identfound past ident line so don't do strncmp for every line */
	if((!identfound) &&
		(strncmp(inbuf, Identmark, sizeof(Identmark)-1) == 0)) {
		identfound = 1;
		return(STAR);
	}

	/* check for menu ident line */
	if((!menufound) && (strncmp(inbuf, Menumark,sizeof(Menumark)-1) == 0)) {
		menufound = 1;
		ret = inbuf + sizeof(Menumark) - 1;

		/* legal chars for a menu title? */
		for (p = &ret[0]; *p; p++) {
			if ((*p >= 'a' && *p <= 'z') ||
				(*p >= 'A' && *p <= 'Z') || *p == '\n' ||
				*p == ',' || *p == '-' || *p == ':' || *p == ';')
				;	/* do nothing */
			else
				*p = ' ';
		}
		*desc = ret;
		return(MENU);
	}

	/* check for a help ident line */
	else if((!helpfound) &&
		(strncmp(inbuf, Helpmark, sizeof(Helpmark)-1) == 0)) {
		helpfound = 1;
		ret = inbuf + sizeof(Helpmark) - 1;
		tmpdesc = menutok(ret);
		tmptitle = menutok(NULL);
		*desc = tmpdesc;
		*title = tmptitle;
		/* get rid of nl at end of title */
		while(*tmptitle++);
		*(tmptitle -2) = '\0';
		return(HELP);
	}

	/* check for a rows ident line */
	else if((!rowsfound) &&
		(strncmp(inbuf, Rowsmark, sizeof(Rowsmark)-1) == 0)) {
		rowsfound = 1;
		ret = inbuf + sizeof(Rowsmark) - 1;
		*desc = ret;
		return(ROWS);
	}
	/* check for a life ident line */
	else if (strncmp(inbuf, Lifemark, sizeof(Lifemark)-1) == 0) {
		*desc = inbuf + sizeof(Lifemark) -1;
		return(LIFE);
	}
	else return(ERR_RET);
}
