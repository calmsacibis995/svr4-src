/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:form/field_stat.c	1.2"

#include "utility.h"

	/*********************
	*  set_field_status  *
	*********************/

int set_field_status (f, status)
FIELD * f;
int status;
{
	f = Field (f);

	if (status)
		Set (f, USR_CHG);
	else
		Clr (f, USR_CHG);

	return E_OK;
}

int field_status (f)
FIELD * f;
{
/*
	field_status may not be accurate on the current field unless
	called from within the check validation function or the
	form/field init/term functions.

	field_status is always accurate on validated fields.
*/
	if (Status (Field (f), USR_CHG))
		return TRUE;
	else
		return FALSE;
}

