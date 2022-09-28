/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libld:common/ldfhread.c	1.6"
#include    <stdio.h>
#include    "filehdr.h"
#include	"synsyms.h"
#include    "ldfcn.h"

int
ldfhread(ldptr, filehead)

LDFILE    *ldptr;
FILHDR    *filehead; 

{

    extern int		vldldptr( );

    if (vldldptr(ldptr) == SUCCESS) {
	if (FSEEK(ldptr, 0L, BEGINNING) == OKFSEEK) {
	    if (FREAD((char *)filehead, FILHSZ, 1, ldptr) == 1) {
	    	return(SUCCESS);
	    }
	}
    }

    return(FAILURE);
}

