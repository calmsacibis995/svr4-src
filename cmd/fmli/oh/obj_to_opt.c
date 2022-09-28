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

#ident	"@(#)fmli:oh/obj_to_opt.c	1.4"

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

struct opt_entry *
obj_to_opt(objtype)
char *objtype;
{
	register int i;
	extern struct opt_entry Partab[MAX_TYPES];

	for (i = 0; i < MAX_TYPES && Partab[i].objtype; i++) {
		if (strcmp(objtype, Partab[i].objtype) == 0) {
			if (i != MAX_TYPES-1 || No_operations == FALSE)
				return(Partab + i);
		}
	}

#ifdef _DEBUG
	_debug(stderr, "Object %s Not Found, searching external\n",objtype);
#endif

	if (ootread(objtype) == O_FAIL) {
#ifdef _DEBUG
		_debug(stderr, "Failed to find extern\n");
#endif
		return(NULL);
	} else {
#ifdef _DEBUG
		_debug(stderr, "Found object extern\n");
#endif
		No_operations = FALSE;
		return(Partab + MAX_TYPES - 1);
	}
}
