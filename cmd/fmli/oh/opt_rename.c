/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */

#ident	"@(#)fmli:oh/opt_rename.c	1.3"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>		/* EFT abs k16 */
#include "but.h"
#include "wish.h"
#include "sizes.h"
#include "typetab.h"
#include "ifuncdefs.h"
#include "optabdefs.h"
#include "partabdefs.h"

bool
opt_rename(entry, newbase, allnames)
struct ott_entry *entry[MAXOBJPARTS+1];
char *newbase;
char allnames[MAXOBJPARTS][FILE_NAME_SIZ];
{
	char *part_construct();
	register int i = 0, n = 0;
	struct opt_entry *partab;
	int part_offset;
	char *base, *p;
	extern struct one_part  Parts[MAXPARTS];
	struct opt_entry *obj_to_parts();
	char *part_match();
	

	if ((partab = obj_to_parts(entry[0]->objtype)) == NULL)
		return(O_FAIL);
	part_offset = partab->part_offset;

	if (base = part_match(entry[0]->name, Parts[part_offset].part_template)) {
		strcpy(allnames[n++], 
			part_construct(newbase, Parts[part_offset+i].part_template));
		if (++entry == NULL)
			return(O_OK);
	} else
		return(O_FAIL);

	for (i = 1; i < partab->numparts; i++) {
		p = part_construct(base, Parts[part_offset+i].part_template);
		if (strcmp(entry[0]->name, p) == 0) {
			strcpy(allnames[n++], 
				part_construct(newbase, Parts[part_offset+i].part_template));
			if (++entry == NULL)
				return(O_OK);
		} else if (!(Parts[part_offset+i].part_flags & PRT_OPT) ) {
			return(O_FAIL);
		}
	}
	return(O_OK);
}
