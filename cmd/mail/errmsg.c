/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:errmsg.c	1.5.3.1"
#include "mail.h"
/*
    NAME
	errmsg - print error message

    SYNOPSIS
	void errmsg(int error_value, char *error_message)

    DESCRIPTION
	Errmsg() prints error messages. If error_message is supplied,
	that is taken as the text for the message, otherwise the
	text for the err_val message is gotten from the errlist[] array.
*/
void errmsg(err_val,err_txt)
char	*err_txt;
{
	static char pn[] = "errmsg";
	error = err_val;
	if (err_txt && *err_txt) {
		fprintf(stderr,"%s: %s\n",program,err_txt);
		Dout(pn, 0, "%s\n",err_txt);
	} else {
		fprintf(stderr,"%s: %s\n",program,errlist[err_val]);
		Dout(pn, 0,"%s\n",errlist[err_val]);
	}
	Dout(pn, 0,"error set to %d\n", error);
}
