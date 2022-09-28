/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lp:cmd/lpadmin/done.c	1.5.2.1"

extern	void	exit();

#include "lp.h"
#include "msgs.h"

#include "lpadmin.h"

/**
 ** done() - CLEAN UP AND EXIT
 **/

void			done (rc)
	int			rc;
{
	(void)mclose ();
	exit (rc);
}
