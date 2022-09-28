/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libld:common/ldtbseek.c	1.5"
#include	<stdio.h>
#include	"filehdr.h"
#include	"synsyms.h"
#include	"ldfcn.h"

int
ldtbseek(ldptr)

LDFILE		*ldptr;

{

    extern int		vldldptr( );

    if (vldldptr(ldptr) == SUCCESS) {
	if (HEADER(ldptr).f_symptr != 0L) {
	    if (FSEEK(ldptr, HEADER(ldptr).f_symptr, BEGINNING) == OKFSEEK) {
		return(SUCCESS);
	    }
	}
    }

    return(FAILURE);
}

