/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libld:common/ldshread.c	1.6"
#include	<stdio.h>
#include	"filehdr.h"
#include	"scnhdr.h"
#include	"synsyms.h"
#include	"ldfcn.h"

int
ldshread(ldptr, sectnum, secthdr)

LDFILE	*ldptr;
unsigned short	sectnum;
SCNHDR	*secthdr; 

{

    extern int		vldldptr( );

    if (vldldptr(ldptr) == SUCCESS) {
	if ((sectnum != 0) && (sectnum <= HEADER(ldptr).f_nscns)) {
	    if (FSEEK(ldptr,
		(long)(FILHSZ + HEADER(ldptr).f_opthdr + (sectnum - 1L) * SCNHSZ),
		BEGINNING) == OKFSEEK) {
		if (FREAD((char *)secthdr, SCNHSZ, 1, ldptr) == 1) {
		    return(SUCCESS);
		}
	    }
	}
    }

    return(FAILURE);
}

