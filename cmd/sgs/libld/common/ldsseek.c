/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libld:common/ldsseek.c	1.6"
#include	<stdio.h>
#include	"filehdr.h"
#include	"scnhdr.h"
#include	"synsyms.h"
#include	"ldfcn.h"

int
ldsseek(ldptr, sectnum)

LDFILE		*ldptr;
unsigned short	sectnum; 

{

	SCNHDR	shdr;

	if (ldshread(ldptr, sectnum, &shdr) == SUCCESS) {
		if (shdr.s_scnptr != 0) {
		    if (FSEEK(ldptr, shdr.s_scnptr, BEGINNING) == OKFSEEK) {
			    return(SUCCESS);
		    }
		}
	}

	return(FAILURE);
}

