/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:intf_reloc/oldmenu.c	1.1.1.2"

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include "intf.h"
#include "oldmenu.h"


/*
 *  struct old_item *
 *  input_oldmenu(name)
 *  char *name;
 *
 *  struct old_item {
 *	char o_name[NAMELEN+1];		/* name of menu item *
 *	char o_desc[DESCLEN+1];		/* menu item descr *
 * 	struct old_item *o_next;	/* next menu item *
 *  }
 */

char inline[LNSZ];		/* input buffer */
char filename[LNSZ];		/* file or directory name */

char *get_desc();

struct old_item *
input_oldmenu(name)
char *name;
{
	DIR *dirptr;		/* ptr to dir stream - ret from opendir() */
	DIR *subptr;		/* ptr to subdirectory stream */
	FILE *fileptr;		/* file pointer to task file */
	struct dirent *entry;	/* directory entry */
	struct dirent *subentry;	/* sub-directory entry */
	struct old_item *item;	/* pointer to individual old_item */
	struct old_item *list;	/* linked list of items */
	struct old_item *prev;	/* previous item in list */
	char *desc;		/* description */
	int found;		/* flag if DESC line found */

	list = NULL;
	prev = NULL;
	if((dirptr = opendir(name)) == NULL) {
		return(NULL);
	}

	for(;;) {
		if((entry = readdir(dirptr)) == NULL) break;
		strcpy(filename, name);
		strcat(filename, "/");
		strcat(filename, entry->d_name);
		if(*(entry->d_name) == '.') continue;
		if(strncmp(entry->d_name, DESC, strlen(DESC)) == 0) continue;

		/* now process directory entry */
		if((subptr = opendir(filename)) == NULL) {
			
			/* not a directory - it's a file */
			desc = get_desc(filename);
			if(desc == NULL) /* bad file?  forget it */
				continue;
			
		}
		else {	/* it's a sub-directory */
			found = 0;
			for(;;) {
				if((subentry = readdir(subptr)) == NULL) {
					break;
				}
				else if(strncmp(subentry->d_name, DESC, 
					strlen(DESC)) == 0) {
					found = 1;
					break;
				}
			}
			if(found) {
				strcat(filename,"/");
				strcat(filename, DESC);
				desc = get_desc(filename);
			}
			else desc = NULL;
		}

		if((item = (struct old_item *) malloc(sizeof(struct old_item))) 			== NULL) return(NULL);
		/* copy name into old_item structure */
		strncpy(item->o_name, entry->d_name, NAMELEN);
		(item->o_name[NAMELEN]) = NULL;

		/* copy desc into old_item structure */
		if(desc != NULL) strncpy(item->o_desc, desc, DESCLEN);
		else *(item->o_desc) = NULL;
		(item->o_desc[DESCLEN]) = NULL;
		item->o_next = NULL;

		if(list == NULL) {
			list = item;	
			prev = item;
		}
		else {
			prev->o_next = item;
			prev = item;
		}
	}
	return(list);
}

char *
get_desc(filename)
char *filename;		/* name of file to get description from */
{
	int found;	/* found pointer */
	FILE *fptr;	/* file pointer */
	char *pos;		/* position of newline to remove */
	found = 0;

	if((fptr = fopen(filename, "r")) == NULL) return(NULL);	
	
	for(;;) {
		if(fgets(inline, sizeof(inline), fptr) == NULL) break;
		if(strncmp(inline, MENUHDR, strlen(MENUHDR)) == 0) {
			found = 1;
			pos = strrchr(inline, '\n');
			if(pos != NULL) *pos = NULL;
			break;
		}
	}
	if(found) return(inline + strlen(MENUHDR) + 1);
	else return(NULL);
		
}
