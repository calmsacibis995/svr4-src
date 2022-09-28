/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:intf_reloc/reloc.c	1.4.1.2"

#include <stdio.h>
#include <string.h>
#include <pkginfo.h>
#include "intf.h"
#include "oldmenu.h"
#include "menu_io.h"
#include "inst_err.h"

char t_name[] = TMP_NAME;	/* name of temp menu created to save changes */

char main_menu[] = "main:(MENU)";
char old_name[] = OLD_SYS;

struct item_def old_entry = {
	OLD_SYS,		/* menu item name */
	"Pre-SVR4.0 System Administration",	/* menu item description */
	"o_sysadm.help",	/* menu item help file */
	P_NONE,			/* placeholder status */
	OLD_SYSADM,		/* menu item action */
	"#_PRE4.0#",		/* pkg instance identifiers */
	NULL,			/* original name if rename */
	NULL			/* next menu item */
};

char cur_menu[LNSZ];		/* current menu file */
char tmp_menu[LNSZ];		/* temp menu file */
char outstr[LNSZ];		/* output line for new entries */

FILE *exprfile;			/* file ptr for express mode log */
char *oambase;			/* oambase environment */

struct reloc_tab {
	char from_dir[LNSZ];
	char to_loc[LNSZ/2];
};

extern char *getenv();
extern struct menu_item *find_menu();
extern struct old_item *input_oldmenu();
extern void inst_err();
extern struct menu_file *input_menu();
extern int write_menu();
extern void free_menu();
extern int mk_dir();

struct item_def *add_first();
struct item_def *find_old();
struct item_def *add_old();
int is_oldpkg();
int search_old();
void new_name();
int proc_ph();
int log_expr();
void add_pkg();
int srch_pkg();

char *prog;		/* program name */

main(argc, argv)
int argc;
char **argv;
{

	/* 
	 * the following table gives the "from" and "to" locations
	 * of all pre4.0 menus that must be relocated.
	 * The initial version of this software only has one relocation:
	 * from packagemgmt to applications.  The pre4.0 location should
	 * be specified in the form of a physical pathname.  The 4.0
	 * location should be specified as the logical location.
	 * The only difference is that "(MENU)" should be appended to the
	 * end of the logical location so find_menu() will locate that
	 * menu and not its parent.
	 */

	static struct reloc_tab table[] = {
	"/usr/admin/menu/packagemgmt",		"main:applications:(MENU)",
	NULL,					NULL
	};


	char *cur_dir;		/* pointer to position in current dir. */
	char *pos;		/* position within directory name buffer */
	char *exp_file;		/* express file log */
	char *from_loc;		/* 3.0 directory to move from */
	char *to_loc;		/* 4.0 menu to move to */
	int i;			/* cntr */
	int errors;		/* count of errors */
	int was_empty;		/* set if 4.0 menu was empty when read */
	int turn_pholder;	/* ON, OFF, or NEUTRAL */
	int ret;		/* ret from strcmp */
	int filechanged;	/* set if parent pholder file changed */
	int done;		/* flag to skip out of loop */
	struct menu_item *m_item;	/* return from find_menu() */
	struct menu_file *file_str;	/* return from input_menu() */
	struct item_def *newptr;	/* ptr within list of 4.0 menu items */
	struct item_def *prev;		/* prev ptr in 4.0 menu item list */
	struct item_def *itemptr;	/* ptr within pholder 4.0 menu list */
	struct old_item *menuptr;	/* old menu ptr -from input_oldmenu() */
	struct old_item *oldptr;	/* pointer to old menu item */

	i = 0;
	prog = argv[0];
	oambase = getenv(OAMBASE);

	if(argc != 2) {
		printf("usage error \n");
		return(ERR_RET);
	}
	exp_file = argv[1];

	if((exprfile = fopen(exp_file, "a")) == NULL) {
		inst_err(prog, ERR, FILE_OPN, exp_file);
		return(ERR_RET);
	}
	

	while(*(table[i].to_loc) != NULL) {
		from_loc = table[i].from_dir;
		to_loc = table[i].to_loc;
		i++;

		/* neutralize 'turn_pholder' flag */
		turn_pholder = NEUTRAL;

		/*
		 * find and translate logical 4.0 location to actual
		 * pathname.  find_menu() does this and returns all sorts
		 * of useful info in a structure.  Info also includes
		 * information about parent placeholder, if applicable.
		 */
		m_item = find_menu(prog, to_loc);
		if(m_item == NULL) {
			inst_err(prog, ERR, INV_PATH, to_loc);
			return(ERR_RET);
		}


		cur_dir = m_item->path;

		/* build path name to temp file in cur_dir */
		if(strncmp(cur_dir, OAMBASE, sizeof(OAMBASE)-1) == 0) {
			/* skip "OAMBASE" in cur_dir string */
			cur_dir += sizeof(OAMBASE);
		}

		/* build temporary menu path */
		(void) sprintf(tmp_menu,"%s/%s/%s", oambase, cur_dir, t_name);

		/* path to menu file containing the item */
		(void) sprintf(cur_menu, "%s/%s/%s", oambase, cur_dir, 
			m_item->menu_name);

		/* use input_menu to input 4.0 menu - it will pick up
		 * temp file if one is there */
		if((file_str = input_menu(prog, cur_menu, tmp_menu)) == NULL)
			return(ERR_RET);

		/* use input_oldmenu to input pre-4.0 menu */
		menuptr = input_oldmenu(from_loc);


		/* now compare the two menus */
		if(file_str->entries == NULL) was_empty = 1;

		if(menuptr != NULL) { /* old entries present */
			oldptr = menuptr;
			while(oldptr != NULL) {
				newptr = file_str->entries;
				if(newptr == NULL) {
					if((newptr = add_first(oldptr,file_str,
						cur_dir)) == NULL)
						return(ERR_RET);
					oldptr = oldptr->o_next;
					/* go on to next old entry */
					continue;
				}
				prev = NULL;
				for(;;) {
					/* compare new & old names */
					ret = strcmp(newptr->mname,
						oldptr->o_name);

					/* check comparison & don't forget
					 * to compare with orig_name */

					/* matches either name or orig_name */
					if(ret == 0) { /* found */
						if((newptr = find_old(newptr,
							oldptr->o_name)) 
							== NULL) 
							/*
							 * slightly obtuse....
							 * if NULL, then found
							 * entry. no need to
							 * add it again, so
							 * break.
							 */
							break;
						else {
							/* 
							 * if not NULL,
							 * then add entry
							 * following returned
							 * newptr, which is
							 * last with that 
							 * base name.
							 */
							newptr = add_old(newptr,
							  oldptr, cur_dir);
							break;
						}
					}
					/* else new is alphabetically less than
					 * old menu name - keep going */
					else if(ret < 0) {
						prev = newptr;
						newptr = newptr->next;
						if(newptr == NULL) {
							/* end of list */
							add_old(prev, oldptr,
							  cur_dir);
							break;
						}
					}
					/* new is alphabetically greater than
					 * old menu name - add old name */
					else {
						if(prev == NULL) {
							
						  if((newptr = add_first(oldptr,
							file_str, cur_dir)) 
								== NULL)
								return(ERR_RET);
						  else break;
						}
						add_old(prev, oldptr, cur_dir);
						break;
					}
				}
				oldptr = oldptr->o_next;
			}
		}
		if(!was_empty) {	/* 4.0 menu was NOT empty */
			turn_pholder = ON;
			/* now check to make sure all entries should be there */
			prev = NULL;
			newptr = file_str->entries;
			while(newptr != NULL) {
				if(is_oldpkg(newptr)) {
					if(search_old(newptr, menuptr) 
					  == NOTFOUND)  {
						if(prev != NULL)
							prev->next=newptr->next;
						else file_str->entries = 
							newptr->next;
						log_expr(newptr, cur_dir, 
							NONPHOLDER, DELETE);
						/* FREE newptr */
						free(newptr);
					}
				}
				prev = newptr;
				newptr = newptr->next;
			}
			if(file_str->entries == NULL) {
				/* turn any parent placeholder OFF */
				/* if applicable */
				turn_pholder = OFF;
			}
		}
		else if(file_str->entries != NULL)
			turn_pholder = ON;
		if(output_menu(prog, file_str, tmp_menu) < 0) return(ERR_RET);
		if(move_tmp(tmp_menu, cur_menu) < 0)  return(ERR_RET);
		free_menu(file_str);

		if((turn_pholder != NEUTRAL) && (m_item->par_menu != NULL)) {
			/* turn parent placeholder ON or OFF */
			filechanged = 0;

			/* build path name to temp file in cur_dir */
			if(strncmp(m_item->par_menu, OAMBASE, sizeof(OAMBASE)-1)
				== 0) {
				/* skip "OAMBASE" in cur_dir string */
				m_item->par_menu += sizeof(OAMBASE);
				(void) sprintf(cur_menu, "%s/%s",oambase,
					m_item->par_menu);
			}
			else (void) strcpy(cur_menu, m_item->par_menu);

			(void) strcpy(tmp_menu, cur_menu);
			if((pos = strrchr(tmp_menu, (int) '/')) != NULL)
				*(pos + 1) = NULL;
			(void) strcat(tmp_menu, t_name);
			if((file_str = input_menu(prog, cur_menu, tmp_menu)) 
				== NULL)
				return(-1);

			itemptr = file_str->entries;
			done = 0;
			do {	/* while (!done) */

				if(strncmp(itemptr->mname, m_item->par_item, 
					strlen(m_item->par_item)) == 0) {

					/* found it */
					filechanged = proc_ph(itemptr, 
						turn_pholder, ISPHOLDER, 
						OLD_PKG, cur_dir);
					break;
				}
				if(itemptr->next == NULL) done = 1;
				else itemptr = itemptr->next;
			} while (!done);
			if(filechanged) {
				if(output_menu(prog, file_str, tmp_menu) < 0) 
					return(-1);

				if(move_tmp(tmp_menu, cur_menu) < 0) return(-1);
				free_menu(file_str);
			}
		}
	}
	return(update_main());
}

struct item_def *
translate(oldptr)
struct old_item *oldptr;	/* pointer to 3.0 formatted menu item */
{
	/*
	 * translate the 3.0 formatted menu item, 'oldptr', into
	 * a 4.0 formatted menu item.
	 */

	struct item_def *newitem;	/* new menu item */

	if((newitem = (struct item_def *) malloc(sizeof(struct item_def)))
		== NULL) return(NULL);

	/* now fill in info */
	strncpy(newitem->mname, oldptr->o_name, (NAMELEN+1));
	strncpy(newitem->mdescr, oldptr->o_desc, (DESCLEN+1));

	strncpy(newitem->help, oldptr->o_name, (MAXDIR - strlen(HELP_SFX)));
	strcat(newitem->help, HELP_SFX);
	newitem->pholder = P_NONE;

	sprintf(newitem->maction, "%s %s", OLD_SYSADM, newitem->mname);

	sprintf(newitem->pkginsts, "#%s#", OLD_PKG);
	*(newitem->orig_name) = NULL;

	newitem->next = NULL;
	return(newitem);
}

struct item_def *
add_first(oldptr, file_str, cur_dir)
struct old_item *oldptr;	/* pointer to old menu item */
struct menu_file *file_str;	/* return from input_menu() */
char *cur_dir;			/* current directory */
{
	/*
	 * add an old menu item as the first menu item in menu 
	 * represented by "file_str".
	 */

	struct item_def *newptr;	/* ptr to new 4.0 entry */
	
	newptr = translate(oldptr);
	if(newptr == NULL) return(NULL);
	else {
		newptr->next = file_str->entries;
		file_str->entries = newptr;
	}
	log_expr(newptr, cur_dir, NONPHOLDER, NO_DEL);
	return(newptr);
	
}

struct item_def *
find_old(newptr, name)
struct item_def *newptr;	/* pointer to a 4.0 menu entry */
char *name;			/* name to look for */
{
	/*
	 * starting with 4.0 entry pointed to by 'newptr', locate
	 * entry that represents a relocated 3.0 entry with the
	 * name of 'name'.
	 * Don't forget that the relocated entry may have been renamed
	 * due to collision.
	 * Slightly obtuse...., return NULL if entry found.  Otherwise
	 * return a pointer to the last entry with that orig_name.
	 */

	struct item_def *ptr;	/* intermediate pointer used */
	struct item_def *prev;	/* previous pointer */
	int ret;		/* ret from name comparison */
	int pkgret;		/* ret from pkginst comparision */
	int origret;		/* ret from orig_name comparison */

	ptr = newptr;

	
	for(;;) {
		ret = strcmp(ptr->mname, name);
		pkgret = strncmp((ptr->pkginsts + 1), OLD_PKG, strlen(OLD_PKG));
		origret = strcmp(ptr->orig_name, name);

		if((ret == 0) && (pkgret == 0)) /* * found it */
			return(NULL);
		else if((origret == 0) && (pkgret == 0)) /* found it */
			return(NULL);

		else if((ret != 0) && (origret != 0)) 
			/* past possible matches */
			return(prev);
		
		prev = ptr;
		ptr = ptr->next;
	}
		

}

struct item_def *
add_old(newptr, oldptr, cur_dir)
struct item_def *newptr;	/* pointer to a 4.0 menu entry */
struct old_item *oldptr;	/* pointer to a 3.0 menu entry */
char *cur_dir;			/* current directory */
{
	/*
	 * add a 4.0 - formatted entry to represent 'old_item'
	 * behind 'newptr' in the menu linked list.
	 */

	/*
	 * don't forget to check - if 'newptr' has same name as 
	 * oldptr->o_name, then need to rename for collision.
	 */

	struct item_def *ptr;	/* ptr to new 4.0 entry */
	
	ptr = translate(oldptr);
	if(ptr == NULL) return(NULL);
	else {
		ptr->next = newptr->next;
		newptr->next = ptr;
	}
	if((strcmp(newptr->mname, ptr->mname) == 0) ||
		(strcmp(newptr->orig_name, ptr->mname) == 0)) {
		/* collision */
		/* rename the 3.0 entry */
		strcpy(ptr->orig_name, ptr->mname);
		new_name(ptr->mname, newptr->mname, ptr->orig_name);
	}
	log_expr(ptr, cur_dir, NONPHOLDER, NO_DEL);
	return(ptr);
}


int
is_oldpkg(newptr)
struct item_def *newptr;	/* pointer to a 4.0 menu entry */
{
	char *ptr;	/* pointer in pkginst list */
	/*
	 * check to see if the entry pointed to by 'newptr'
	 * represents a relocated 3.0 entry.
	 */

	if(*(newptr->pkginsts) == '#') ptr = newptr->pkginsts + 1;
	else ptr = newptr->pkginsts;

	if(strncmp(ptr, OLD_PKG, strlen(OLD_PKG)) == 0) {
		return(TRUE);
	}
	else {
		return(FALSE);
	}
}

int
search_old(newptr, menuptr)
struct item_def *newptr;	/* pointer to a 4.0 menu entry */
struct old_item *menuptr;	/* pointer to the 3.0 menu linked list */
{
	/*
	 * search the 3.0 linked menu list for an entry that corresponds
	 * to 'newptr' (newptr is a 4.0 formatted entry that represents
	 * a relocated 3.0 entry).  If no such entry exists, then
	 * it means the original 3.0 entry has probably been removed
	 * during package removal.  It should no longer appear in
	 * the 4.0 menu in this case, so return NOTFOUND (0).  Otherwise
	 * return FOUND (1).
	 */

	struct old_item *optr;
	
	optr = menuptr;
	while(optr != NULL) {
		if((strcmp(newptr->mname, optr->o_name) == 0) ||
			(strcmp(newptr->orig_name, optr->o_name) == 0))
				return(FOUND);
		optr = optr->o_next;
	}

	/* if reached this point, then it's not there, so return NOTFOUND */
	return(NOTFOUND);

}

void
new_name(to, from, orig)
char *to;		/* place to copy new name to */
char *from;		/* derive the new name from this name */
char *orig;		/* original name */

{
	char *pos;	/* character position */
	char *svpos;	/* save character position */
	char buildbuf[20];	/* buffer to build new name into */
	
	int cnt;	/* count of digits */
	int num;	/* rename number */
	int len;	/* length of 'from' string */
	
	cnt = 0;
	pos = from;

	while(isalpha(*pos)) pos++;
	/* pos is now at first non alpha char */

	if((isspace(*pos)) || (*pos == NULL)) {	/* first rename of orig */
		num = 2;
		cnt = 1;
	}

	else {	/* multiple rename */
		svpos = pos;
		while(isdigit(*svpos)) {	/* count how many digits */
			cnt += 1;
			svpos++;
		}
		len = strlen(from);	/* get length of from string */

		num = atoi(pos);
		if(num == 9) cnt += 1;	/* add 1 digit if next is 10 */
		num++;
	}
	
	strncpy(buildbuf, orig, (MAXITEMLEN - cnt));
	*(buildbuf + MAXITEMLEN - cnt) = NULL;
	(void) sprintf(to, "%s%d", buildbuf, num);
	inst_err(prog, WARN, RENAME, orig, to);

}

int
proc_ph(itemptr, turn_pholder, pflag, pkginst, cur_dir)
struct item_def *itemptr;	/* pointer to menu item */
int turn_pholder;		/* ON or OFF */
int pflag;			/* indicates parent placeholder or not */
char *pkginst;			/* package instance */
char *cur_dir;			/* current directory */
{
	int changed;		/* indicates if item was changed or not */

	changed = 0;
	if((itemptr->pholder == P_INACTIVE) && (turn_pholder == ON)) {
		itemptr->pholder = P_ACTIVE;
		log_expr(itemptr, cur_dir, pflag, NO_DEL);
		changed = 1;
	}

	if((itemptr->pholder == P_ACTIVE) && (turn_pholder == OFF)) {
		itemptr->pholder = P_INACTIVE;
		*(itemptr->pkginsts) = NULL;
		changed = 1;
		log_expr(itemptr, cur_dir, pflag, DELETE);
	}

	if((itemptr->pholder == P_ACTIVE) && (!srch_pkg(itemptr, pkginst)))  {
		add_pkg(itemptr, pkginst);
		changed = 1;
	}
	
	return(changed);
}

int
log_expr(itemptr, cur_dir, ispholder, delflag)
struct item_def *itemptr;	/* ptr to item to log in expr file */
char *cur_dir;			/* current directory */
int ispholder;			/* pholder flag */
int delflag;	/* tells if "^#DELETE#" gets added to end of entry or not */
{
	char *cur;		/* position in cur_dir of beginning of path */
	char *pos;		/* position of last '/' in action */
	int i;	/* misc counter through action field */
	/*
	 * record in the express mode log file that a new task/menu
	 * was added to the Interface structure.  
	 */


	if(strncmp(cur_dir, MAIN_PATH, strlen(MAIN_PATH)) == 0) {
		cur = cur_dir + strlen(MAIN_PATH);
	}
	else cur = cur_dir;

	if(ispholder == NONPHOLDER) {	/* just log executable for 3.0 */
		(void) sprintf(outstr, "%s^-^%s", itemptr->mname, 
			itemptr->maction);
	}
	else {	/* placeholder - cur_dir will include full path */
		pos = strrchr(itemptr->maction, (int) '/');
		if(pos == NULL) pos = itemptr->maction;
		else pos++;
		(void) sprintf(outstr, "%s^%s^%s", itemptr->mname, cur, pos);
	}

	(void) fputs(outstr, exprfile);
	if(delflag) {
		(void) fputs("^", exprfile);
		(void) fputs(VOID_STR, exprfile);
	}
	(void) fputs("\n", exprfile);

}

void
add_pkg(itemptr, pkginst)
struct item_def *itemptr;		/* pointer to menu item */
char *pkginst;				/* pkginst to add to list */
{
	if(strlen(itemptr->pkginsts) > 2) {	/* already a list */
		(void) strcat(itemptr->pkginsts, "^#");
		(void) strcat(itemptr->pkginsts, pkginst);
		(void) strcat(itemptr->pkginsts, "#");
	}
	else (void) sprintf(itemptr->pkginsts, "#%s#", pkginst);
}


int
srch_pkg(itemptr, pkginst)
struct item_def *itemptr;
char *pkginst;
{
	char *line;	/* pointer to pkginsts */

	line = itemptr->pkginsts;

	for(;;) {
		if(*line == NULL) return(NOTFOUND);
		if(*line == '^') line++;
		if(*line == '#') {
			line++;
			if(strncmp(line, pkginst, strlen(pkginst)) == 0) {
				return(FOUND);
			}
		}

		/* skip to next */
		while((*line != '^') && (*line != NULL)) line++;
	
	}
}


int
update_main()
{

	/*
	 * update_main() - update main menu to allow access to
	 *	OLD_SYS entry if any pre4.0 packages are
	 *	present on the system.
	 *	If no pre4.0 packages are present and the main 
	 *	OLD_SYS entry is present, then remove it.
	 */


	char *cur_dir;			/* pointer to position in current dir */
	int found;			/* set if pre-4.0 packages on system */
	int changed;			/* set if main menu file changed */
	int ret;			/* ret from strncmp() */
	struct menu_file *file_str;	/* return from input_menu() */
	struct menu_item *m_item;	/* return from find_menu() */
	struct item_def *ptr;		/* pointer in menu item list */
	struct item_def *prev;		/* prev pointer in menu item list */
	struct pkginfo info;		/* info struct for pkginfo() */

	found = 0;

	/* init pkginfo struct to prevent pkginfo() from trying to free it */
	info.pkginst = NULL;
	while(pkginfo(&info, "all", NULL, NULL) == 0) {
		if(info.status == PI_PRESVR4) {
			found++;
			break;
		}
	}
	pkginfo(&info, NULL);


	/* use find_menu instead of hardcoded path to main menu
	 * because if interface directory structure moves again,
	 * it won't be headache.
	 */

	m_item = find_menu(prog, main_menu);
	if(m_item == NULL) {
		inst_err(prog, ERR, INV_PATH, main_menu);
		return(ERR_RET);
	}


	cur_dir = m_item->path;

	/* build path name to temp file in cur_dir */
	if(strncmp(cur_dir, OAMBASE, sizeof(OAMBASE)-1) == 0) {
		/* skip "OAMBASE" in cur_dir string */
		cur_dir += sizeof(OAMBASE);
	}

	/* build temporary menu path */
	(void) sprintf(tmp_menu,"%s/%s/%s", oambase, cur_dir, t_name);

	/* path to main menu file */
	(void) sprintf(cur_menu, "%s/%s/%s", oambase, cur_dir, 
		m_item->menu_name);

	/* use input_menu to input 4.0 main menu menu - it will pick up
	 * temp file if one is there */
	if((file_str = input_menu(prog, cur_menu, tmp_menu)) == NULL)
		return(ERR_RET);

	ptr = file_str->entries;
	if(ptr == NULL) /* ??? not main menu!!! */ 
		return(ERR_RET);

	prev = NULL;
	changed = 0;
	for(;;) {
		ret = strncmp(ptr->mname, old_name, strlen(old_name));

		if(ret == 0) { /* found in menu */
			if(!found) {	/* shouldn't be there */
				prev->next = ptr->next;
				log_expr(ptr, cur_dir, NONPHOLDER, DELETE);
				changed = 1;
				break;
			}
			else {
				/*
				 * it's there & it should be 
				 */
				break;
			}
		}

		/* else new is alphabetically less than
		 * old menu name - keep going */
		else if(ret < 0) {
			prev = ptr;
			ptr = ptr->next;
		}
		else {
			/* past point where it should be */
			if(found) {
				prev->next = &old_entry;
				old_entry.next = ptr;
				log_expr(&old_entry, cur_dir, NONPHOLDER, 
					NO_DEL);
				changed = 1;
				break;
			}
			else {
				/*
				 * it's not there & it shouldn't be 
				 */
				break;
			}
		}
		if(ptr == NULL) /* end of list - shouldn't get here */
			return(ERR_RET);

	}

	if(changed) {
		if(output_menu(prog, file_str, tmp_menu) < 0) return(ERR_RET);

		if(move_tmp(tmp_menu, cur_menu) < 0) return(ERR_RET);
		free_menu(file_str);
	}
	return(SUCCESS_RET);

}
