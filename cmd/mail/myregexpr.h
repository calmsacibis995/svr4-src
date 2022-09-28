/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:myregexpr.h	1.4.3.1"
#ifndef _REGEXPR_H
#define _REGEXPR_H
extern char	**braslist;	/* start of \(,\) pair */
extern char	**braelist;	/* end of \(,\) pair */
extern int	nbra;		/* number of \(,\) pairs */
extern int	regerrno;	/* for compile() errors */
extern char	*loc1;
extern char	*loc2;
extern char	*locs;
#ifdef __STDC__
extern int step(char *string, char *expbuf); 
extern int advance(char *string, char *expbuf);
extern char *compile(char *instring, char *expbuf, char *endbuf);
#else
extern int step(); 
extern int advance();
extern char *compile();
#endif /* __STDC__ */
#endif /* _REGEXPR_H */
