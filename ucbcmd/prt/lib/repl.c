/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*      Portions Copyright (c) 1988, Sun Microsystems, Inc.     */ 
/*      All Rights Reserved.                                    */ 
 
#ident	"@(#)ucbprt:lib/repl.c	1.1.1.1"
/*
	Replace each occurrence of `old' with `new' in `str'.
	Return `str'.
*/

char *
repl(str,old,new)
char *str;
char old,new;
{
	char	*trnslat();
	return(trnslat(str, &old, &new, str));
}
