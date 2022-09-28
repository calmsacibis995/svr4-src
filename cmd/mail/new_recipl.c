/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:new_recipl.c	1.3.3.1"
#include "mail.h"


/*
    NAME
	new_reciplist - initialize a recipient list

    SYNOPSIS
	new_reciplist (reciplist *list)

    DESCRIPTION
	Initialize a recipient list to have no recipients.
*/

void new_reciplist (plist)
reciplist	*plist;
{
	static char	pn[] = "new_reciplist";
	Dout(pn, 0, "entered\n");
	plist->recip_list.next = 0;
	plist->recip_list.name = 0;
	plist->last_recip = &plist->recip_list;
}
