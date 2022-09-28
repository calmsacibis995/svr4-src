/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cscope:common/basename.c	1.1"
/* get a file's base name from its path name */

#if BSD
#define	strrchr	rindex
#endif

char *
basename(path)
char	*path;
{
	char	*s, *strrchr();
	
	if ((s = strrchr(path, '/')) != 0) {
		return(s + 1);
	}
	return(path);
}
