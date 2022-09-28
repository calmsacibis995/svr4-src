/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libld:common/ldnshread.c	1.7"
#include	<stdio.h>
#include	"filehdr.h"
#include	"scnhdr.h"
#include	"synsyms.h"
#include	"ldfcn.h"

int
ldnshread(ldptr, sectname, secthdr)

LDFILE	*ldptr;
const   char	*sectname;
SCNHDR	*secthdr; 

{

    extern int		vldldptr( );

    unsigned short	i;
    int			j;
    unsigned short	numsects;

    if (vldldptr(ldptr) == SUCCESS) {
	if (FSEEK(ldptr, (long)( FILHSZ + HEADER(ldptr).f_opthdr ),
            BEGINNING) == OKFSEEK) {
	    numsects = (HEADER(ldptr)).f_nscns;
	    for (i=0;
		(i < numsects) && (FREAD((char *)secthdr,SCNHSZ,1,ldptr) == 1);
		 ++i) {
		    for (j = 0;
		        (j < 8) && (secthdr->s_name[j] == sectname[j]);
		     	++j) {
			    if (secthdr->s_name[j] == '\0') {
				return(SUCCESS);
			    }
		    }
		    if (j == 8) {
		    	return(SUCCESS);
		    }
	    }
	}
    }

    return(FAILURE);
}

