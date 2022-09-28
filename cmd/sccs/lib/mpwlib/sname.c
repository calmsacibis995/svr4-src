/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sccs:lib/mpwlib/sname.c	6.3"
/*
	Returns pointer to "simple" name of path name; that is,
	pointer to first character after last "/".  If no slashes,
	returns pointer to first char of arg.
	If the string ends in a slash, returns a pointer to the first
	character after the preceeding slash, or the first character.
*/

char	*sname(s)
char *s;
{
	register char *p;
	register int n;
	register int j;
	unsigned int	strlen();

	n = strlen(s);
	--n;
	if (s[n] == '/') {
		for (j=n; j >= 0; --j)
			if (s[j] != '/') {
				s[++j] = '\0';
				break;
			}
	}

	for(p=s; *p; p++) if(*p == '/') s = p + 1;
	return(s);
}
