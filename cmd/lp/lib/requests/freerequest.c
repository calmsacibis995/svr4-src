/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/requests/freerequest.c	1.6.3.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "sys/types.h"
#include "stdlib.h"

#include "lp.h"
#include "requests.h"

/**
 ** freerequest() - FREE STRUCTURE ALLOCATED FOR A REQUEST STRUCTURE
 **/

void
#if	defined(__STDC__)
freerequest (
	REQUEST *		reqbufp
)
#else
freerequest (reqbufp)
	register REQUEST	*reqbufp;
#endif
{
	if (!reqbufp)
		return;
	if (reqbufp->destination)
		Free (reqbufp->destination);
	if (reqbufp->file_list)
		freelist (reqbufp->file_list);
	if (reqbufp->form)
		Free (reqbufp->form);
	if (reqbufp->alert)
		Free (reqbufp->alert);
	if (reqbufp->options)
		Free (reqbufp->options);
	if (reqbufp->pages)
		Free (reqbufp->pages);
	if (reqbufp->charset)
		Free (reqbufp->charset);
	if (reqbufp->modes)
		Free (reqbufp->modes);
	if (reqbufp->title)
		Free (reqbufp->title);
	if (reqbufp->input_type)
		Free (reqbufp->input_type);
	if (reqbufp->user)
		Free (reqbufp->user);
	return;
}
