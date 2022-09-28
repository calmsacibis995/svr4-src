/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:tokdef.c	1.3.3.1"
/* Check for a field being redefined. */
#include "mail.h"

string *tokdef(fld, tok, name)
string *fld, *tok;
char *name;
{
    char *pn = "tokdef";

    Tout(pn, "Looking at field '%s'\n", s_to_c(tok));

    if (fld)
	{
	Tout(pn, "Field %s= redefined, was '%s', now '%s'\n", name, s_to_c(fld), s_to_c(tok));
	s_free(fld);
	}

    if (s_ptr_to_c(tok)[1] != '=')
	Tout(pn, "Field %s does not have '=' sign. The first character will be lost!\n", name);

    return tok;
}
