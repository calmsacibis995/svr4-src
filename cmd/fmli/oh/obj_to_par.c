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

#ident	"@(#)fmli:oh/obj_to_par.c	1.5"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>		/* EFT abs k16 */
#include "but.h"
#include "wish.h"
#include "typetab.h"
#include "ifuncdefs.h"
#include "partabdefs.h"
#include "optabdefs.h"

/* Obj_to_parts should be used instead of obj_to_opt in those executables
 * that do not need access to the object operations table.  It only
 * reads the global flags and parts information for an object.
 */

/* The No_operations flag is set to TRUE if the externally read object
 * has not had its operations read.
 */
bool No_operations;

struct opt_entry *
obj_to_parts(objtype)
char *objtype;
{
	register int i;
	FILE *fp;
	extern struct opt_entry Partab[MAX_TYPES];
	extern char *externoot();

	for (i = 0; i < MAX_TYPES && Partab[i].objtype; i++) {
		if (strcmp(objtype, Partab[i].objtype) == 0 )
			return(Partab + i);
	}

	/* read in the external object table for this object, but
	 * only read in the parts information.
	 */

	if ((fp = fopen(externoot(objtype), "r")) == NULL)
		return(NULL);

	if (read_parts(fp, objtype) == O_FAIL) {
#ifdef _DEBUG
		_debug(stderr, "External Object not found\n");
#endif
		fclose(fp);
		return(NULL);
	} else {
#ifdef _DEBUG
		_debug(stderr, "External Object %s found\n", objtype);
#endif
		No_operations = TRUE;
		fclose(fp);
		return(Partab + MAX_TYPES - 1);
	}
}
