/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:form/form_init.c	1.1"

#include "utility.h"

	/******************
	*  set_form_init  *
	******************/

int set_form_init (f, func)
FORM * f;
PTF_void func;
{
	Form (f) -> forminit = func;
	return E_OK;
}

PTF_void form_init (f)
FORM * f;
{
	return Form (f) -> forminit;
}

