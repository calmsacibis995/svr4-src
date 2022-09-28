/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cscope:common/mygetenv.c	1.1"
/* return the non-null environment value or the default argument */

char	*
mygetenv(variable, deflt)
char	*variable, *deflt;
{
	char	*value;
	char	*getenv();

	value = getenv(variable);
	if (value == (char *) 0 || *value == '\0') {
		return(deflt);
	}
	return(value);
}
