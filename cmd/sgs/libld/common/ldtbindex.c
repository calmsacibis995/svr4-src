/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libld:common/ldtbindex.c	1.5"
#include	<stdio.h>
#include	"filehdr.h"
#include	"syms.h"
#include	"synsyms.h"
#include	"ldfcn.h"

long
ldtbindex(ldptr)

LDFILE	*ldptr;

{

    extern int		vldldptr( );

    long		position;


    if (vldldptr(ldptr) == SUCCESS) {
	if ((position = FTELL(ldptr) - OFFSET(ldptr) - HEADER(ldptr).f_symptr) 
	    >= 0) {

	    if ((position % SYMESZ) == 0) {
		return(position / SYMESZ);
	    }
	}
    }

    return(BADINDEX);
}

