/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-os:pmstub.c	1.1"

#include "sys/types.h"
#include "sys/cred.h"
#include "sys/user.h"
#include "sys/debug.h"

/*
 * Test whether the supplied credentials identify a privileged process.
*/ 
int
pm_denied(cr, priv)
register struct cred *cr;
int	priv;
{
	return suser(cr);
}
