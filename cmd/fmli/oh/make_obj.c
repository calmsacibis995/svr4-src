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

#ident	"@(#)fmli:oh/make_obj.c	1.4"

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
#include "obj.h"
#include "moremacros.h"
#include "sizes.h"

extern struct ott_tab *Cur_ott;
extern struct ott_entry *Cur_entry;

int
make_object(path, name, type, force_flag, amask, nmask, fmask)
char *path, *name, *type;
int	force_flag;	/* for FORCE getting of the parent dir and if for parent */
long	amask, nmask, fmask;
{
	struct ott_entry newentry, *ott_dup(), *name_to_ott(), *sendentry;
	struct ott_tab *sendtable, *ott_get_current();
	struct opt_entry *optab, *obj_to_opt();
	extern struct one_part Parts[MAXPARTS];
	char *getcwd();
	int	ret, objflag;
	
	if (!path)
		path = getcwd(NULL, PATHSIZ + 2);
	
	if (force_flag & PARENT)
		objflag = SAME_OBJECT;
	else
		objflag = DIFF_OBJECT;
	/* save current table so can pass to call_objprocess */
	sendtable = ott_get_current();
	
	if (type) {	/* if type is given, use it in creating entry */
		if ((optab = obj_to_opt(type)) == NULL) {
			return(NULL);
		}
		if (optab->int_class & CL_DIR) {
			/* for CL_DIR, no table needed but can use the real one if in core */
			if (ott_in_core(path) != O_FAIL) {	/* real one is in core */
				make_current(path);
				if (sendentry = name_to_ott(name)) {
					sendentry = ott_dup(sendentry);
					ret = call_objprocess(sendtable, sendentry, amask, nmask, fmask, objflag);
					ott_free(sendentry);
					return(ret);
				}
			}
		}
		else if (force_flag & FORCE) {
			/* if forcing a non-dir, make the path current no matter what */
			if (ott_get(path, 0, 0, 0, 0) == NULL)
				return(NULL);
			if (sendentry = name_to_ott(name)) {	/* use it if it exists */
				sendentry = ott_dup(sendentry);
				ret = call_objprocess(sendtable, sendentry, amask, nmask, fmask, objflag);
				ott_free(sendentry);
				return(ret);
			}
		}
		/* Fell out of above code if force_flag or if of classs CL_DIR, and
		 * not found yet - ie, object does not exist(flag) or ott is not in
		 * core (DIR); OR got here by being neither force_flag nor CL_DIR.
		 * We have a hint as to what the object is (know type), so create
		 * a "fake" entry.
		 */
		newentry.objmask = optab->int_class;
		newentry.dirpath = path;
		strcpy(newentry.name, name);
		/* strsave needed because it is not duped */
		newentry.dname = strsave(part_match(name, Parts[optab->part_offset].part_template));
		newentry.objtype = type;
		newentry.next_part = OTTNIL;
		newentry.mtime = (time_t)0L; /* EFT abs k16 */
		newentry.odi = NULL;
		newentry.display = strsave(optab->objdisp);

		/* no dup needed since it is automatic */

		ret = call_objprocess(sendtable, &newentry, amask, nmask, fmask, objflag);
		free(newentry.dname);
		free(newentry.display);
		return(ret);
	}
	/* no type given, the object must already exist since we can't make
	 * a fake one without knowing the type
	 */
	if (ott_get(path, 0, 0, 0, 0) == NULL)
		return(NULL);
	if (sendentry = name_to_ott(name)) {
		sendentry = ott_dup(sendentry);
		ret = call_objprocess(sendtable, sendentry, amask, nmask, fmask, objflag);
		ott_free(sendentry);
		return(ret);
	}
	else	/* if no type, and does not already exist, can't make entry */
		return(NULL);
}
