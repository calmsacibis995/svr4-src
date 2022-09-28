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

#ident	"@(#)fmli:oh/nextpart.c	1.3"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>		/* EFT abs k16 */
#include "but.h"
#include "wish.h"
#include "typetab.h"
#include "optabdefs.h"
#include "partabdefs.h"

struct one_part *
opt_next_part(entry)
struct opt_entry *entry;
{
	static int partsleft;
	static int curoffset;
	struct one_part *retval;
	extern struct one_part  Parts[MAXPARTS];

	if (entry) {
		partsleft = entry->numparts;
		curoffset = entry->part_offset;
	}
	if (partsleft > 0) {
		retval = Parts + curoffset++;
		partsleft--;
	} else
		retval = NULL;

	return(retval);
}
