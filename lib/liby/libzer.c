/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liby:libzer.c	1.6"
# include <stdio.h>

#ifdef __cplusplus
void yyerror(const char *s)
#else
yyerror( s ) char *s; 
#endif
{
	fprintf(  stderr, "%s\n", s );
	}
