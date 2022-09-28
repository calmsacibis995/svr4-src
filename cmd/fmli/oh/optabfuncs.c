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

#ident	"@(#)fmli:oh/optabfuncs.c	1.6"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>		/* EFT abs k16 */
#include "but.h"
#include "wish.h"
#include "typetab.h"
#include "ifuncdefs.h"
#include "optabdefs.h"
#include "partabdefs.h"

extern bool No_operations;
extern int Vflag;

/* functions pertaining to the object operations table (oot) and object parts
 * table (opt).
 */

struct operation **
oot_get()
{
	extern struct operation *Optab[MAX_TYPES][MAX_OPERS];
	void fcn_init();

	fcn_init();
	odftread();
	return((struct operation **)Optab);
}

struct operation **
obj_to_oot(objtype)
char *objtype;
{
	register int i;
	extern struct operation *Optab[MAX_TYPES][MAX_OPERS];
	extern struct opt_entry Partab[MAX_TYPES];

	for (i = 0; i < MAX_TYPES && Partab[i].objtype; i++) {
		if (strcmp(objtype, Partab[i].objtype) == 0 ) {
			if (!Vflag && !(Partab[i].int_class & CL_FMLI))
				return(NULL);
			if (i != MAX_TYPES-1 || No_operations == FALSE)
				return(Optab[i]);
		}
	}

	if (ootread(objtype) == O_FAIL) {
		return(NULL);
	} else {
		_debug(stderr, "External read of %s succeeded\n", objtype);
		No_operations = FALSE;
		return(Optab[MAX_TYPES - 1]);
	}
}
