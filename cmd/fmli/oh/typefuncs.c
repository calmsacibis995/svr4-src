/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 *  Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */

#ident	"@(#)fmli:oh/typefuncs.c	1.5"

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "mio.h"
#include "wish.h"
#include "var_arrays.h"
#include "typetab.h"
#include "partabdefs.h"
#include "moremacros.h"
#include "sizes.h"


char Holdpath[PATHSIZ];

struct ott_entry *
ott_dup(entry)
struct ott_entry *entry;
{
	char *def_display();
	struct ott_entry *newentry = (struct ott_entry *)
						array_create(sizeof(struct ott_entry), 1);

	if (!entry)
		return(NULL);

	memcpy(newentry, entry, sizeof(struct ott_entry));

	if (entry->dname)
		newentry->dname = strsave(entry->dname);

	if (entry->display && entry->objtype && 
			entry->display != def_display(entry->objtype))
		newentry->display = strsave(entry->display);

	if (entry->odi)
		newentry->odi = strsave(entry->odi);

	if (entry->dirpath)
		newentry->dirpath = strsave(entry->dirpath);

	return(newentry);
}

void
ott_int_free(entry)			/* free the internals of an ott_entry */
struct ott_entry *entry;
{
	char *def_display();

	if (entry->dname)
		free(entry->dname);
	if (entry->odi)
		free(entry->odi);
	if (entry->display && entry->objtype && 
				entry->display != def_display(entry->objtype))
		free(entry->display);
}

void
ott_free(entry)
struct ott_entry *entry;
{
	if (!entry)
		return;
	if (entry->dirpath)
		/* can't be in ott_int_free because called by other things than
		 * those that were ott_dup'd */
		free(entry->dirpath);
	ott_int_free(entry);
	array_destroy(entry);
}

char *
ott_to_path(entry)
struct ott_entry *entry;
{
	strcpy(Holdpath, entry->dirpath);
	strcat(Holdpath, "/");
	strcat(Holdpath, entry->name);
	return(Holdpath);
}

void
ott_mark(entry, mask, value)
struct ott_entry *entry;
long mask;
bool value;
{
	register int i;
	extern struct ott_tab *Cur_ott;
	extern struct ott_entry *Cur_entry;
	void ott_mtime();
	
	ott_lock_dsk(Cur_ott->path);

	if (value)
		entry->objmask |= mask;
	else
		entry->objmask &= ~mask;
	utime(ott_to_path(entry), NULL);
	ott_mtime(entry);

	/* if it is a parent, kill all children */

	if (entry->dname) {
		for(i = entry->next_part; i != OTTNIL; i = Cur_entry[i].next_part) {
			if (value)
				Cur_entry[i].objmask |= mask;
			else
				Cur_entry[i].objmask &= ~mask;
			utime(ott_to_path(&(Cur_entry[i])), NULL);
			ott_mtime(&(Cur_entry[i]));
		}
	}
	Cur_ott->modes |= OTT_DIRTY;
	ott_synch(FALSE);
}

void
ott_mtime(entry)		/* update the mod time on the entry */
struct ott_entry *entry;
{
	struct ott_entry *name_to_ott();

	extern struct ott_tab *Cur_ott;
	extern struct ott_entry *Cur_entry;
	struct stat sbuf;

	if (!Cur_ott || !Cur_entry)
		return;

	/* do nothing if mtime is not wrong */
	if (stat(ott_to_path(entry), &sbuf) == -1 || sbuf.st_mtime == entry->mtime)
		return;

	ott_lock_dsk(Cur_ott->path);
	if ((entry = name_to_ott(entry->name)) != NULL)
		entry->mtime = sbuf.st_mtime;
	Cur_ott->modes |= OTT_DIRTY;
	ott_synch(FALSE);
}

/* The next few functions change various fields of an ott entry */

#define ODI_FLD		0
#define OBJM_FLD	1
#define DNAME_FLD	2
#define DISPLAY_FLD	3

static void
ott_chg_item(entry, field)
struct ott_entry *entry;
short field;
{	
	extern struct ott_tab *Cur_ott;
	extern struct ott_entry *Cur_entry;
	struct ott_entry *name_to_ott();
	struct ott_entry *old_entry = entry;
	char *def_display();
	char *newodi = entry->odi;
	char *newdname = entry->dname;
	char *newdisplay = entry->display;
	long newobjm = entry->objmask;

	ott_lock_dsk(Cur_ott->path);

	if (old_entry != (entry = name_to_ott(entry->name))) 
	{	switch(field)
		{	case ODI_FLD:
				if (entry->odi)
					free(entry->odi);
				entry->odi = strsave(newodi);
				break;
			case OBJM_FLD:
				entry->objmask = newobjm;
				break;
			case DNAME_FLD:
				if (entry->dname)
					free(entry->dname);
				entry->dname = strsave(newdname);
				break;
			case DISPLAY_FLD:
				if (entry->display && entry->objtype &&
					entry->display != def_display(entry->objtype))
					free(entry->display);
				entry->display = strsave(newdisplay);
				break;
			default: break;
		}
	}
	Cur_ott->modes |= OTT_DIRTY;
	ott_synch(FALSE);
}

void
ott_chg_odi(entry)
struct ott_entry *entry;
{
	(void)ott_chg_item(entry, ODI_FLD);
}

void
ott_chg_objm(entry)
struct ott_entry *entry;
{
	(void)ott_chg_item(entry, OBJM_FLD);
}

void
ott_chg_dname(entry)
struct ott_entry *entry;
{
	(void)ott_chg_item(entry, DNAME_FLD);
}

void
ott_chg_display(entry)
struct ott_entry *entry;
{
	(void)ott_chg_item(entry, DISPLAY_FLD);
}
