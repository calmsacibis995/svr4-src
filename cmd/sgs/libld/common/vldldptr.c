/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libld:common/vldldptr.c	1.5"
#include	<stdio.h>
#include	"filehdr.h"
#include	"ldfcn.h"
#include	"lddef.h"
#include 	"synsyms.h"

int
vldldptr(ldptr)
LDFILE	*ldptr;

{
    extern LDLIST	*_ldhead;

    LDLIST		*ldindx;

    for (ldindx = _ldhead; ldindx != NULL; ldindx = ldindx->ld_next) {
	if (ldindx == (LDLIST *) ldptr) {
	    return(SUCCESS);
	}
    }

    return(FAILURE);
}

