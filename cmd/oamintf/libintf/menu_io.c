/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:libintf/menu_io.c	1.2.2.2"

/*LINTLIBRARY*/
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "intf.h"
#include "menu_io.h"
#include "inst_err.h"

extern char	*read_item();
extern void	*malloc(),
		*calloc(),
		free(),
		inst_err();
extern int	unlink(),
		write_item();
extern time_t	time();

struct menu_line m_line;	/* menu line structure */
struct menu_line *ptr_menu = &m_line;	/* pointer to menu line structure */

static time_t dnclock = 0;
static struct menu_file
		*read_menu();
static int	write_menu(),
		parse_item();
static char	*nxt_fld();

static int
parse_item(prog, mitem, ptr_menu)
char *prog;		/* calling program name */
struct item_def *mitem;	/* menu item structure */
struct menu_line *ptr_menu;	/* menu line to parse */
{
	char *mname;	/* menu item name */
	char *mdescr;	/* menu item description */
	char *maction;	/* menu item action */
	char pline[BUFSIZ];	/* buffer to hold complete line */
	char *plineptr;	/* pointer to pline */
	char *pos;	/* position in pline */
	char *nxt;	/* next field from nextfld() */
	char *orig;	/* name of orig name */
	char *end;	/* end of orig name */

	plineptr = pline;

	strcpy(plineptr, ptr_menu->line);
	while(ptr_menu->next != NULL) {
		if((pos = strrchr(plineptr, (int) '\\')) != NULL) {
			*pos = NULL;	 /* remove continuation char */
			*(pos + 1) = NULL;	/* remove newline */
		}
		ptr_menu = ptr_menu->next;
		strcat(plineptr, ptr_menu->line);
	}
	/* remove newline */
	if((pos = strrchr(plineptr, (int) '\n')) != NULL) *pos = NULL;

	mitem->pholder = P_NONE;
	mname = plineptr;
	if(((mdescr = nxt_fld(mname)) == NULL) 
		|| ((maction = nxt_fld(mdescr)) == NULL)) {
			inst_err(prog, ERR, INV_FILE, ptr_menu->line);
			return(-1);
		}

	nxt = nxt_fld(maction);
		
	/* check for inactive placeholder */
	if(strncmp(maction, PHOLDER, sizeof(PHOLDER)-1) == 0) {
		mitem->pholder = P_INACTIVE;
		if(nxt != NULL) {
			maction = nxt;
			nxt = nxt_fld(nxt);
		} else {
			inst_err(prog, ERR, INV_FILE, ptr_menu->line);
			return(-1);
		}
	}

	(void) strcpy(mitem->mname, mname);
	(void) strcpy(mitem->mdescr, mdescr);
	(void) strcpy(mitem->maction, maction);

	/*
	 * check for active placeholder, evidence of menu item rename, or
	 * pkginst identifiers.
	 */

	*(mitem->pkginsts) = NULL;
	*(mitem->orig_name) = NULL;
	if(nxt != NULL) {
		/* there is a next field */
		if(strncmp(nxt, PHOLDER, sizeof(PHOLDER)-1) == 0) {
			mitem->pholder = P_ACTIVE;
			nxt = nxt_fld(nxt);
		}
		/* see if menu item rename field is present */
		if((nxt != NULL) && (*nxt == '[')) {	/* menu item rename */
			orig = ++nxt;
			nxt = nxt_fld(nxt);
			end = strchr(orig, (int) ']');
			if(end == NULL) {
				inst_err(prog, ERR, INV_FILE, ptr_menu->line);
				return(-1);
			}
			else {
				*end = NULL;
				(void) strcpy(mitem->orig_name, orig);
			}
		}
		if(nxt != NULL) { /* pkginst identifiers */
			(void) strcpy(mitem->pkginsts, nxt);

		}
	}
	return(0);
}

static struct menu_file *
read_menu(prog, file)
char *prog;		/* calling program name - for error msgs */
FILE *file;		/* file to read header from */
{

	struct menu_line *hdr_ptr;	/* menu header line */
	struct menu_line *hdr_last;	/* previous header line */
	struct item_def *mitem;	/* menu item */
	struct item_def *item_last;	/* previous menu item line */
	struct menu_file *menu;
	char *menustr;			/* return from read_item() */
	int first_hdr;			/* flags first header line */
	int first_item;			/* flags first item line */

	first_item = 1;
	first_hdr = 1;
	hdr_ptr = NULL;
	hdr_last = NULL;
	item_last = NULL;
	
	if((menu = (struct menu_file *) calloc(1, sizeof(struct menu_file)))
		== NULL) return(NULL);

	/* get menu file info */

	while(menustr = read_item(ptr_menu, file, FULL_LINE)) {
		if((*menustr == '#') || (strlen(menustr) < 5)) {
			/* malloc space for header line */
			if((hdr_ptr = (struct menu_line *)
				calloc(1, sizeof(struct menu_line))) == NULL)
					return(NULL);
			if(first_hdr) {
				menu->head = hdr_ptr;
				first_hdr = 0;
			}
			else {
				hdr_last->next = hdr_ptr;
			}

			strcpy(hdr_ptr->line, menustr);
			hdr_last = hdr_ptr;
			continue;
		} else {
			/* malloc space for menu item */
			if((mitem = (struct item_def *) 
				calloc(1, sizeof(struct item_def))) == NULL)
					return(NULL);
			if(first_item) {
				menu->entries = mitem;
				first_item = 0;
			}
			else item_last->next = mitem;
	
			if(parse_item(prog, mitem, ptr_menu))
				return(NULL);
			item_last = mitem;
			continue;
		}
	}
	if(first_hdr && first_item)
		return(NULL);
	hdr_last->next = NULL;

	/* make sure there is a blank line at end of header line list */
	if(*(hdr_last->line) != '\n') {
		/* malloc space for one more header line */
		if((hdr_ptr = (struct menu_line *)
			calloc(1, sizeof(struct menu_line))) == NULL)
				return(NULL);
		*(hdr_ptr->line) = '\n';
		*(hdr_ptr->line + 1) = NULL;
		hdr_ptr->next = NULL;
		hdr_last->next = hdr_ptr;
	}
	return(menu);
}

static int
write_menu(menu, file)
struct menu_file *menu;		/* menu structure to write */
FILE *file;			/* file to write to */
{
	
	struct menu_line *hdr_ptr;	/* menu header line */
	struct item_def *item_ptr;	/* menu item */
	char outbuf[BUFSIZ];		/* output buffer */

	hdr_ptr = menu->head;
	item_ptr = menu->entries;

	/* write header */

	if(write_item(hdr_ptr, file) != 0) return(-1);

	/* write menu items */

	while(item_ptr != NULL) {
		(void) sprintf(outbuf, "%s%c%s%c", item_ptr->mname, TAB_CHAR,
				item_ptr->mdescr, TAB_CHAR);
		if(item_ptr->pholder == P_INACTIVE) {
			(void) strcat(outbuf, PHOLDER);
			(void) strcat(outbuf, TAB_DELIMIT);
		}
		(void) strcat(outbuf, item_ptr->maction);
		if(item_ptr->pholder == P_ACTIVE) {
			(void) strcat(outbuf, TAB_DELIMIT);
			(void) strcat(outbuf, PHOLDER);
		}
		if(*(item_ptr->orig_name) != NULL) {
			(void) strcat(outbuf, TAB_DELIMIT);
			(void) strcat(outbuf, "[");
			(void) strcat(outbuf, item_ptr->orig_name);
			(void) strcat(outbuf, "]");
		}
		if(*(item_ptr->pkginsts) != NULL) {
			(void) strcat(outbuf, TAB_DELIMIT);
			(void) strcat(outbuf, item_ptr->pkginsts);
		}
		(void) strcat(outbuf, "\n");
		if(fputs(outbuf, file) == EOF) return(-1);
		item_ptr = item_ptr->next;
	}

	return(0);
}

void
free_menu(menu)
struct menu_file *menu;		/* menu structure to free */
{
	struct menu_line *hdrptr;	/* pointer to menu header lines */
	struct menu_line *nexthdr;	/* pointer to next menu header line */
	struct item_def *itemptr;	/* pointer to menu item lines */
	struct item_def *nextitem;	/* pointer to next menu item line */

	hdrptr = menu->head;
	itemptr = menu->entries;

	while(hdrptr != NULL) {
		nexthdr = hdrptr->next;
		free(hdrptr);
		hdrptr = nexthdr;
	}
	while(itemptr != NULL) {
		nextitem = itemptr->next;
		free(itemptr);
		itemptr = nextitem;
	}
	free(menu);
}

static char *
nxt_fld(pos)
char *pos;		/* position to parse from */
{
	char *next;	/* pointer to next field - return value */

	next = strchr(pos, TAB_CHAR);
	if(next != NULL) { 
		*next = NULL;
		next++;
		return(next);
	}
	else return(NULL);

}

struct menu_file *
input_menu(prog, menu_nam, tmp_nam)
char *prog;			/* calling program name - for error msgs */
char *menu_nam;			/* menu name */
char *tmp_nam;			/* temp menu name */
{
	struct menu_file *file_str;	/* menu file structure to return */
	struct stat statbuf;		/* used for stat() */
	char *menuname;			/* save name for print message if err */
	FILE *menufile;			/* menu file pointer */

	menufile = (FILE *)NULL;
	if(stat(tmp_nam, &statbuf) == 0) {
		/* temp menu file exists, but now we need to make
		 * sure it was created by this invocation of mod_menus!
		 */
		if(dnclock && (statbuf.st_mtime >= dnclock)) {
			/* already a temp file - read from it */
			if((menufile = fopen(tmp_nam, "r")) == NULL) {
				/* failed to open */	
				inst_err(prog, ERR, FILE_OPN, tmp_nam);
				return(NULL);
			}
			menuname = tmp_nam;
		} else {
			/* unlink the old one */
			(void) unlink(tmp_nam);
		}
	}
	if(menufile == NULL) {
		/* open regular menu file */
		if((menufile = fopen(menu_nam, "r")) == NULL) {
			/* failed to open */
			inst_err(prog, ERR, FILE_OPN, menu_nam);
			return(NULL);
		}
		menuname = menu_nam;
	}

	/* read in the menu */
	if((file_str = read_menu(prog, menufile)) == NULL) {
		inst_err(prog, ERR, FILE_RD, menuname);
		return(NULL);
	}
	(void) fclose(menufile);
	return(file_str);
}

int
output_menu(prog, file_str, tmp)
char *prog;				/* calling program name */
struct menu_file *file_str;		/* menu to output */
char *tmp;				/* name of temp file to output to */
{
	FILE *menufile;			/* menu file pointer */

	if(!dnclock) {
		/* we set clock when we first attempt to create ANY
		 * temporary menu file so when we encounter temp
		 * files, we know that we created them
		 */
		(void) time(&dnclock); /* current timestamp */
	}
	if((menufile = fopen(tmp, "w")) == NULL) {
		/* failed to open */	
		inst_err(prog, ERR, FILE_OPN, tmp);
		return(-1);
	}
	if(write_menu(file_str, menufile) < 0) {
		inst_err(prog, ERR, FILE_WR, tmp);
		return(-1);
	}
	(void) fclose(menufile);
	return(0);
}
