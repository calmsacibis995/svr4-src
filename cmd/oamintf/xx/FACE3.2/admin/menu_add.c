/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:xx/FACE3.2/admin/menu_add.c	1.1"
/*
	menu_add & menu_del

	Summmary: allows ISV's to add menu items to existing viewmaster
		  menus on 80386 user interface.

 */
#include <stdio.h>

/* default location of viewmaster */
#define DEF_VMSYS	"/usr/vmsys"
/* two possible names of this program (determines action) */
#define ADD_NAME	"menu_add"
#define DEL_NAME	"menu_del"
/* keywords which identify special menus (allow upper or lower case) */
#define PROGRAMS1	"Programs"
#define PROGRAMS2	"programs"
#define SETUP1	"Setup"
#define SETUP2	"setup"

char *program;			/* points to argv[0] */
char office_menu[BUFSIZ];	/* path to office menu */
char admin_menu[BUFSIZ];	/* path to admin menu */

extern char *getenv();

char *create_menu_addition();

main(argc, argv)
char **argv;
{
char vmsys[BUFSIZ];	/* viewmaster location */
char *p;		/* for getenv */
char menu[BUFSIZ];	/* menu name specified on cmd line */
char entry[BUFSIZ];	/* entry or file name specified on cmd line */
/*
 * flags which say add/delete reference to program in office menu, or
 * reference to setup in admin menu
 */
int change_office = 0;
int change_admin = 0;
int adding = 0;			/* 1 = adding, 0 = deleting */
/*
 * name of menu we are adding to office or admin menu.
 * Will be either "Programs" or "Application Setup".
 */
char menu_name[BUFSIZ];

	program = argv[0];
	if (argc != 3) {
		fprintf(stderr, "usage: menu_add <menu> <filename>\n");
		fprintf(stderr, "       menu_del <menu> <entry name>\n");
		exit(1);
	}
	strcpy(menu, argv[1]);
	strcpy(entry, argv[2]);

	/*
	 * find viewmaster location
	 */
	if ((p = getenv("VMSYS")) == NULL)
		strcpy(vmsys, DEF_VMSYS);
	else
		strcpy(vmsys, p);
	strcpy(office_menu, vmsys);
	strcat(office_menu, "/OBJECTS/Menu.office");
	strcpy(admin_menu, vmsys);
	strcat(admin_menu, "/OBJECTS/Menu.admin");

	/*
	 * cannot explicitly add to office or admin menus
	 */
	if (strcmp(menu, office_menu) == 0 ||
	    strcmp(menu, admin_menu) == 0) {
		fprintf(stderr, "Cannot modify the %s menu file.\n", menu);
		fprintf(stderr, "This is a special system menu.\n");
		exit(1);
	}

	/*
	 * adding or deleting?
	 */
	if (strcmp(program, DEL_NAME) == 0)
		adding = 0;
	else
		adding = 1;

	/*
	 * Decode menu-to-be-added-to name
	 * If its the PROGRAMS or SETUP keyword, then set up flags
	 * to modify office or admin menu and change menu to path to
	 * true Programs or Setup menu.
	 * Otherwise, we are working on another menu and we don't have
	 * to change office or admin, but we have to check to see if it's
	 * a valid menu file.
	 */
	if (strcmp(menu, PROGRAMS1) == 0 ||
	    strcmp(menu, PROGRAMS2) == 0) {
		strcpy(menu, vmsys);
		strcat(menu, "/OBJECTS/Menu.programs");
		strcpy(menu_name, "Programs");
		change_office = 1;
	} else if (strcmp(menu, SETUP1) == 0 ||
		   strcmp(menu, SETUP2) == 0) {
		strcpy(menu, vmsys);
		strcat(menu, "/OBJECTS/Menu.appsetup");
		strcpy(menu_name, "Application Setup");
		change_admin = 1;
	} else if (!valid_menu_file(menu) &&
		   !(adding && (access(menu, 0) < 0))) {
		/*
		 * It is an explicitly specified menu file to be added to.
		 * Error if this is an invalid menu, except if we are
		 * newly creating it (adding and it doesn't exist).
		 */
		fprintf(stderr, "%s is not a valid menu file.\n", menu);
		if (adding)
			fprintf(stderr, "Cannot add new entry.\n");
		else
			fprintf(stderr, "Cannot delete entry.\n");
		exit(1);
	}

	/*
	 * if we're adding a menu item, check that the item to
	 * be added is valid.
	 */
	if (adding && !valid_menu_file(entry)) {
		fprintf(stderr, "%s does not contain a valid menu entry.\n",
			entry);
		fprintf(stderr, "Cannot add to %s.\n", menu);
		exit(1);
	}

	/*
	 * do work
	 */
	if (adding) {
		int needed_to_create;

		/*
		 * we specified PROGRAMS or SETUP keyword, so this
		 * new menu may need to be created
		 */
		if (change_office || change_admin)
			needed_to_create = new_menu(menu, menu_name);
		menu_add(menu, entry);
		if (needed_to_create) {
			/*
			 * menu had to be created, so add
			 * reference to it in office or admin menu
			 * if we specified the PROGRAMS or SETUP keyword.
			 */
			if (change_office)
				menu_add(office_menu,
					create_menu_addition(menu_name, menu));
			else if (change_admin)
				menu_add(admin_menu,
					create_menu_addition(menu_name, menu));
			delete_temp_file();
		}
	} else {
		if (menu_del(menu, entry)) {
			/*
			 * that was the last entry in menu, so
			 * delete the reference to that menu in
			 * office or admin menu.
			 */
			if (change_office)
				menu_del(office_menu, menu_name);
			else if (change_admin)
				menu_del(admin_menu, menu_name);
		}
	}
}

/*
 * add contents of filename to menu
 */
menu_add(menu, filename)
char *menu;
char *filename;
{
FILE *mp;	/* menu file */
FILE *fp;	/* file reading from */
FILE *op;	/* output temp file */
char s[BUFSIZ];
char entry_name[BUFSIZ];
char greater_name[BUFSIZ];
int eof_menu;
int eof_file;

	/*
	 * create file we are adding to if necessary
	 */
	if (access(menu, 0) < 0) {
		if ((mp = fopen(menu, "w")) == NULL) {
			fprintf(stderr, "%s: cannot create menu %s\n",
				program, menu);
			exit(1);
		}
		fclose(mp);
	}
	/*
	 * open the file we are adding to
	 */
	if ((mp = fopen(menu, "r")) == NULL) {
		fprintf(stderr, "%s: cannot open menu %s\n", program, menu);
		exit(1);
	}
	/*
	 * the file we are reading from (already checked its existence)
	 */
	if ((fp = fopen(filename, "r")) == NULL) {
		fprintf(stderr, "%s: IMPOSSIBLE cannot open %s\n",
			program, filename);
		exit(1);
	}
	/*
	 * open temp file
	 */
	if ((op = tmpfile()) == NULL) {
		fprintf(stderr, "%s: menu_add() cannot create temp file\n",
			program);
		exit(1);
	}
	/*
	 * merge the original menu with the addition in filename
	 */
	/*
	 * take from menu up to line before first "name="
	 */
	strcpy(entry_name, " ");
	eof_menu = copy_up_to(mp, op, entry_name, greater_name);
	do {
		/*
		 * take from addition up to first "name=entry" where
		 * entry is alphabetically after the "name=entry"
		 * last found in the menu
		 */
		eof_file = copy_up_to(fp, op, greater_name, entry_name);
		/*
		 * stick in menu's "name=" entry and take more from
		 * menu, up to a "name=" that is greater than the
		 * addition's last "name=" entry.
		 */
		if (strlen(greater_name))
			fprintf(op, "name=%s", greater_name);
		if (!eof_menu)
			eof_menu = copy_up_to(mp, op, entry_name, greater_name);
		if (strlen(entry_name))
			fprintf(op, "name=%s", entry_name);
	} while (!eof_menu || !eof_file);

	/*
	 * copy temp file back to menu
	 */
	fclose(fp);
	fclose(mp);
	copy_file(op, menu);
}

/*
 * Copy lines from inf to outf until reaching a line of the form
 * "name=entry" where entry is greater than the string upto,
 * or until EOF.
 * if upto is null string, just copy everything from inf.
 * save entry in greater_name for caller.
 * SPECIAL CASE::  "Exit" is greater than anything so that
 *     Programs will be added to the Office menu before "Exit".
 * If EOF greater_name is set to null string.
 * Returns 1 if EOF reached, 0 otherwise.
 */
copy_up_to(inf, outf, upto, greater_name)
FILE *inf;
FILE *outf;
char *upto;
char *greater_name;
{
char s[BUFSIZ];
int need_to_compare = 0;

	if (strlen(upto)) {
		need_to_compare = 1;
	}
	while (fgets(s, sizeof(s), inf) != NULL) {
		if ((strncmp(s, "name=", sizeof("name=")-1) == 0) &&
		     need_to_compare &&
		    (strcmp(s + sizeof("name=") -1, upto) > 0 ||
		     strncmp(s + sizeof("name=") -1, "Exit", 4) == 0))
		{
			strcpy(greater_name, s + sizeof("name=") -1);
			return 0;
		}
		fprintf(outf, s);
	}
	/*
	 * EOF reached
	 */
	greater_name[0] = '\0';
	return 1;
}

/*
 * copy from an open file stream to a filename which needs to be opened
 */
copy_file(src, dest_fname)
FILE *src;
char *dest_fname;
{
FILE *dest;
char s[BUFSIZ];

	fseek(src, 0L, 0);	/* rewind input file */
	/*
	 * open output file
	 */
	if ((dest = fopen(dest_fname, "w")) == NULL) {
		fprintf(stderr, "%s: cannot write %s\n", program, dest_fname);
		exit(1);
	}
	/*
	 * copy
	 */
	while (fgets(s, sizeof(s), src) != NULL)
		fprintf(dest, s);
	fclose(dest);
	fclose(src);
}
/*
 * delete entry from menu
 * return 1 if menu is now empty (and so removed), 0 otherwise
 */
menu_del(menu, entry)
char *menu;
char *entry;
{
FILE *fp;
FILE *op;
int empty = 1;
short c;
char s[BUFSIZ];

	/*
	 * the file we are reading from (already checked its existence)
	 */
	if ((fp = fopen(menu, "r")) == NULL) {
		fprintf(stderr, "%s: cannot open %s\n", program, menu);
		exit(1);
	}
	if ((op = tmpfile()) == NULL) {
		fprintf(stderr, "%s: menu_del() cannot create temp file\n",
			program);
		exit(1);
	}
	/*
	 * copy menu to temp file except for the menu item entry
	 */
	while (fgets(s, sizeof(s), fp) != NULL) {
		if (strncmp(s, "name=", sizeof("name=")-1) == 0) {
			if (strncmp(s + sizeof("name=") -1, entry,
					strlen(s) - sizeof("name=")) ==0 &&
			    strncmp(s + sizeof("name=") -1, entry,
					strlen(entry)) ==0) {
				fgets(s, sizeof(s), fp); /* eat action= */
				fgets(s, sizeof(s), fp); /* eat itemmsg= */
			} else {
				empty = 0;
				fprintf(op, s);
			}
		} else {
			fprintf(op, s);
		}
	}
	if (empty) {
		/*
		 * no more menu entries in menu, so remove it
		 */
		fclose(fp);
		fclose(op);
		if (unlink(menu) < 0) {
			fprintf(stderr, "%s: cannot unlink menu\n", program);
			exit(1);
		}
		return 1;
	} else {
		/*
		 * copy temp file to menu
		 */
		fclose(fp);
		copy_file(op, menu);
		return 0;	/* still has stuff in it */
	}
}

/*
 * see if filename looks like a viewmaster menu.
 * if it contains "^menu=" or "^name=" assume it is.
 */
valid_menu_file(filename)
char *filename;
{
FILE *fp;
char s[BUFSIZ];

	if ((fp = fopen(filename, "r")) == NULL)
		return 0;
	while (fgets(s, sizeof(s), fp) != NULL) {
		if (strncmp(s, "menu=", sizeof("menu=")-1) == 0)
			return 1;
		if (strncmp(s, "name=", sizeof("name=")-1) == 0)
			return 1;
	}
	fclose(fp);
	return 0;
}

/*
 * if menu filename doesn't exist, create and initialize it, and return 1
 * else just return 0.
 */
new_menu(filename, title)
char *filename;
char *title;
{
FILE *fp;

	if (access(filename, 0) == 0)
		return 0;
	if ((fp = fopen(filename, "w")) == NULL) {
		fprintf(stderr, "%s: cannot create %s\n", program, filename);
		exit(1);
	}
	fprintf(fp, "menu=%s\n\n", title);
	fprintf(fp, "`message \"Move to an item with arrow keys and strike ENTER key to select.\"`\n\n");
	fclose(fp);
	return 1;
}

/*
 * Stuff the menu item we want to add to office or admin menu into
 * a temporary file, so we can use menu_add() routine to merge it in.
 * returns the file name used.
 */
char *
create_menu_addition(name, entry)
char *name;
char *entry;
{
FILE *fp;
static char temp_menu[80];

	sprintf(temp_menu, "/tmp/menu_add%d", getpid());
	/*
	 * open the file we're appending to
	 */
	if ((fp = fopen(temp_menu, "w")) == NULL) {
		fprintf(stderr, "%s: cannot open temp menu %s\n",
			program, temp_menu);
		exit(1);
	}
	fprintf(fp, "name=%s\n", name);
	fprintf(fp, "action=open menu %s\n", entry);
	fclose(fp);
	return temp_menu;
}

/*
	Delete temp file created by create_menu_addition()
 */
delete_temp_file()
{
static char temp_menu[80];

	sprintf(temp_menu, "/tmp/menu_add%d", getpid());
	unlink(temp_menu);
}
