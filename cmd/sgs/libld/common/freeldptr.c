/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libld:common/freeldptr.c	1.5"
#include	<stdio.h>
#include	"filehdr.h"
#include	"ldfcn.h"
#include	"lddef.h"
#include 	"synsyms.h"

int
freeldptr(ldptr)

LDFILE	*ldptr;

{
    extern		free( );

    extern LDLIST	*_ldhead;

    LDLIST		*ldindx;

    if (ldptr != NULL) {
	if (_ldhead == (LDLIST *) ldptr) {
	    _ldhead = _ldhead->ld_next;
	    free((char *)ldptr);
	    return(SUCCESS);
	} else {
	    for (ldindx = _ldhead; ldindx != NULL; ldindx = ldindx->ld_next) {
		if (ldindx->ld_next == (LDLIST *) ldptr) {
		    ldindx->ld_next = ((LDLIST *) ldptr)->ld_next;
		    free((char *)ldptr);
		    return(SUCCESS);
		}
	    }
	}
    }

    return(FAILURE);
}

