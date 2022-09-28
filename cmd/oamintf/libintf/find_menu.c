/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:libintf/find_menu.c	1.3.1.2"

/*LINTLIBRARY*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "intf.h"
#include "menu_io.h"

extern char	*getenv(),
		*read_item();
extern struct menu_file
		*input_menu();
extern void	free_menu();


char curdir[PATH_MAX];	/* current directory REFERS TO OAMBASE */
char cur_pdir[PATH_MAX];	/* current complete directory path */
char curmenu[PATH_MAX];	/* current menu */
char tmpmenu[PATH_MAX];	/* temp menu */
char m_name[20];	/* current menu name */
char p_name[PATH_MAX];	/* path to parent menu */
char left[PATH_MAX];	/* remaining path to return */
char pkginst[LNSZ];	/* pkginst identifiers */


int	ismenu(), isfml();

struct menu_item m_item;	/* menu item to return */
struct menu_line m_line;	/* menu line structure */
struct menu_line *ptr_menu;	/* pointer to menu line structure */

static struct item_def *check_menu();

/*
 * find_menu(path)
 *
 * This function translates the logical path described in "path" into a 
 * physical path to the menu file that will contain the last item in the path.  
 *
 * A pointer to a structure of the following definition is returned:
 *
 * struct menu_item {	- struct of menu item info - returned from find_menu *
 * 	char *path;	- physical path to menu containing item *
 * 	char *menu_name;- name of menu containing item *
 *	int exists	- flag if item already exists *
 * 	char *item;	- menu item *
 * 	char *par_menu;	- parent menu - NULL if not placeholder *
 * 	char *par_item;	- menu item within parent - NULL if not placeholder *
 *	char *leftover;	- left over items not matched from input path *
 *	char *pkginsts;	- any package instances found on the menu line *
 * };
 *
 * The physical path has as its first component the variable: OAMBASE.  
 *
 * Examples:
 *  path arg             path 	             *menu_name    *item
 * --------           -----------------      ------------  --------
 * main:devices       OAMBASE/menu           main.menu     devices
 * main:devices:copy  OAMBASE/menu/devices   devices.menu  copy
 * main:users:add     OAMBASE/menu/usermgmt  users.menu    add
 * 
 * This function is provided for the convenience of edsysadm and the
 * Interface installation/removal tools.
 *
 * Calling sequence:
 * 
 * item_ptr = find_menu(path);
 * 
 * where path is defined as:
 *    char *path;
 *
 * and where the return value (item_ptr) is:
 *    struct menu_item *item_ptr;
 *
 * Within the menu_item structure, par_menu and par_item are provided for
 * the installation (mod_menus.c) and removal (remove.c) tools because if it's
 * necessary to modify an empty placeholder menu, the name of the parent menu 
 * file and parent menu item must be known in order to change the placeholder
 * menu into a displayed menu.  Therefore par_menu will be the parent menu file
 * name and par_item will be the parent menu item if the item indicated in the 
 * path goes under a placeholder menu.  Otherwise, these two items will contain 
 * a NULL.  This arrangement works for only one level of placeholder.  If a 
 * placeholder menu ever needs to be defined under a placeholder menu and both 
 * intervening placeholder menus must be activated along the way during 
 * installation, this scheme will need to change.
 */
 

struct menu_item *
find_menu(prog, path)
char *prog;	/* name of calling program */
char *path;	/* logical path name to look for */
{
	struct menu_file *file_str;	/* ptr to menu file structure */
	struct item_def *itemptr;	/* pointer to menu item structure */
	char *oambase;		/* oambase base directory */
	char *menu;		/* ptr to current menu item */
	char *next;		/* ptr to next menu item */
	char *lookahead;	/* ptr used to look ahead for next item */
	char *leftptr;		/* pointer into 'left' tokens */
	char *pkgpos;		/* pointer to pkginst identifier if present */
	char *pos;		/* pointer to position in maction */
	int done;		/* done flag */
	int first;		/* set if main is first in input path */
	int last;		/* flag when last item retrieved from path */
	int isfmlobj;		/* flag if is an fml object */

	ptr_menu = &m_line;

	/* init flags */
	first = 1;	/* assume main is first */
	last = 0;
	done = 0;

	/* save left so can return remaining tokens not matched */
	strcpy(left, path);
	leftptr = left;
	
	/* 
	 * init par_item & par_menu to NULL.  They will be reset if
	 * the parent item is not a placeholder.
	 */
	m_item.par_item = NULL;
	m_item.par_menu = NULL;

	/* get OAMBASE from the environment and set curdir & curmenu */
	oambase = getenv("OAMBASE");

	(void) strcpy(curdir, "OAMBASE/menu"); /* path from OAMBASE */
	(void) strcpy(curmenu, oambase);  /* full path - OAMBASE resolved */
	(void) strcat(curmenu, "/menu/");   /* don't forget dir. delimiter */
	(void) strcpy(cur_pdir, curmenu);  /* also complete current path */
	*(strrchr(cur_pdir, '/')) = NULL;

	/* pick off first name in input path - check to see if it's "main" */
	menu = strtok(path, PATH_DELIMIT);
	if(strncmp(menu, MAIN, sizeof(MAIN)-1) != 0) {
		first = 0;
		next = menu;
	}
	leftptr += (strlen(menu) + 1);
	/* set m_name and curmenu */
	(void) strcpy(m_name, MAIN_NAME);
	(void) strcat(curmenu, MAIN_NAME);  /* add main menu name to curmenu */

	/* pick off next name in path */
	if(first) {
		next = strtok(NULL, PATH_DELIMIT);
		leftptr += (strlen(next) + 1);
	}

	while(!done) {
		
		/*
		 * Use lookahead to see if the last item has been read.
		 * Save it and process the previous item found on the 
		 * last pass through the loop.  The last item may or 
		 * may not already be in a menu file, depending on whether
		 * or not it already exists.  If it already exists, then
		 * a flag will be set in the menu_item structure.
		 */

		if((lookahead = strtok(NULL, PATH_DELIMIT)) == NULL) last = 1;


		/* 
		 * check to see if a temp. file is present in addition
		 * the regular menu file.  If it is, it means we
		 * are looking for a menu via interface installation
		 * and there have been modifications made to the regular
		 * menu file.  Use the temp file with changes made.
		 */
	
		(void) sprintf(tmpmenu, "%s/%s", cur_pdir, TMP_NAME);

		if((file_str = input_menu(prog, curmenu, tmpmenu)) == NULL)
			return(NULL);

		itemptr = file_str->entries;

		while(itemptr != NULL) {
			if((strncmp(itemptr->mname, next, MAXITEMLEN) > 0) && 
			   (!last)) {
				m_item.exists = 0;
				break;
			}
			else if(strncmp(itemptr->mname, next, MAXITEMLEN) == 0){

				isfmlobj = isfml(itemptr->maction);

				if(last) {
					m_item.exists = 1;
					if((strlen(itemptr->pkginsts) > 2)
					   /* pkginst present */
					   && isfmlobj) {
						(void) strcpy(pkginst, 
							(itemptr->pkginsts+1));
						pkgpos = strrchr(pkginst, '#');
						*pkgpos = NULL;
					      }
					break;
				}
				else { 	/* not last - make sure it's a menu */
					itemptr = check_menu(itemptr, isfmlobj,
						next);
					if(itemptr == NULL) { /* not a menu */
						/* can't continue */
						break;
					}
				}

				/* check for placeholder */
				if(itemptr->pholder != P_NONE) {

					/* have a placeholder */
					/* save item name */
					m_item.par_item = next;

					/* save parent menu path */
					(void) sprintf(p_name, "%s/%s", curdir,
						m_name);
					m_item.par_menu = p_name;

				}

				/* 
				 * reset cur_pdir, curdir, and curmenu based on
				 * any directory info listed in the action field
				 */

				(void) strcat(cur_pdir, DIR_DELIMIT);
				(void) strcat(curdir, DIR_DELIMIT);
				(void) strcpy(curmenu, cur_pdir);

				(void) strcat(curdir, itemptr->maction);
				(void) strcat(cur_pdir, itemptr->maction);
				pos = strrchr(curdir, (int) '/');
				*pos = NULL;
				pos = strrchr(cur_pdir, (int) '/');
				*pos = NULL;

				/* copy menu file name to curmenu */
				(void) strcat(curmenu, itemptr->maction);

				/* reset menu */
				menu=next;
				next=lookahead;
				leftptr += (strlen(next) + 1);
				pos = strrchr(itemptr->maction, (int) '/');
				(void) strcpy(m_name, (pos+1));
				break;
					
			}
			else if((strncmp(itemptr->mname, next, MAXITEMLEN) > 0)
				&& (last)) {
				m_item.exists = 0;
				break;
			}
			itemptr = itemptr->next;
		}
		if(last) break;
	}
	
	m_item.menu_name = m_name;
	m_item.item = next;
	m_item.path = curdir;
	m_item.leftover = lookahead;

	m_item.leftover = leftptr;
	m_item.pkginsts = pkginst;
	m_item.m_desc = itemptr->mdescr;

	return(&m_item);
}

int
isfml(action)
char *action;		/* action to check */
{
	char *pos;		/* pointer to position in action */

	pos = strrchr(action, (int) '/');
	if(pos == NULL) pos = action;
	else pos++;	/* pos is at 1st char of action file */

	if((strncmp(pos,FORM_PFX,strlen(FORM_PFX)) == 0)
		||(strncmp(pos,TEXT_PFX,strlen(TEXT_PFX)) == 0)
		||(strncmp(pos,MENU_PFX,strlen(MENU_PFX))== 0))	
			return(TRUE);
	else return(FALSE);
}

int
ismenu(action)
char *action;		/* action to check */
{
	char *pos;		/* pointer to position in action */

	/* see if it's a menu */
	pos = strrchr(action, (int) '.');
	if(pos != NULL) {
		if(strncmp(pos, MENU_SFX, strlen(MENU_SFX)) == 0)
			return(TRUE);
	}
	return(FALSE);
}

static struct item_def *
check_menu(itemptr, isfmlobj, name)
struct item_def *itemptr;	/* start with this item */
int isfmlobj;			/* flag if incoming obj is fml obj */
char *name;			/* name to check for */
{
	int fmlflag;		/* flag if fml object */
	int mnuflag;		/* flag if menu object */

	fmlflag = isfmlobj;
	mnuflag = ismenu(itemptr->maction);

	for(;;) {
		if((!fmlflag) && mnuflag) return(itemptr);

		if(fmlflag || (!mnuflag)) itemptr = itemptr->next;

		if(itemptr == NULL) return(NULL);
		if(strncmp(itemptr->orig_name, name, MAXITEMLEN) != 0)
			/* gone past last possible item by that name */
			return(NULL);
		fmlflag = isfml(itemptr->maction);
		mnuflag = ismenu(itemptr->maction);
	}
}
