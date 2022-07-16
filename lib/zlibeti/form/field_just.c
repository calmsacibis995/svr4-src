/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:form/field_just.c	1.1"

#include "utility.h"

	/*******************
	*  set_field_just  *
	*******************/

int set_field_just (f, just)
FIELD * f;
int just;
{
	if (	just != NO_JUSTIFICATION
	&&	just != JUSTIFY_LEFT
	&&	just != JUSTIFY_CENTER
	&&	just != JUSTIFY_RIGHT	)

		return E_BAD_ARGUMENT;

	f = Field (f);

	if (Just (f) != just)
	{
		Just (f) = just;
		return _sync_attrs (f);
	}
	return E_OK;
}

int field_just (f)
FIELD * f;
{
	return Just (Field (f));
}

