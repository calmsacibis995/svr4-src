/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:cksurg_rc.c	1.5.3.1"
/*
    NAME
	cksurg_rc - check the surrogate return code

    SYNOPSIS
	int cksurg_rc(int surr_num, int rc)

    DESCRIPTION
	Cksurg_rc() looks up the return code in the list
	of return codes for the given surrogate entry.
*/

#include "mail.h"
cksurg_rc (surr_num, rc)
int	surr_num, rc;
{
    return rc >= 0 ? surrfile[surr_num].statlist[rc] : FAILURE;
}
