/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:form/field_user.c	1.1"

#include "utility.h"

	/**********************
	*  set_field_userptr  *
	**********************/

int set_field_userptr (f, userptr)
FIELD * f;
char * userptr;
{
	Field (f) -> usrptr = userptr;
	return E_OK;
}

char * field_userptr (f)
FIELD * f;
{
	return Field (f) -> usrptr;
}

