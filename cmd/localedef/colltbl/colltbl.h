/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)localedef:colltbl/colltbl.h	1.1.3.1"

/* Diagnostic mnemonics */
#define WARNING		0
#define ERROR		1
#define ABORT		2

enum errtype {
	GEN_ERR,
	DUPLICATE,
	EXPECTED,
	ILLEGAL,
	TOO_LONG,
	INSERTED,
	NOT_FOUND,
	NOT_DEFINED,
	TOO_MANY,
	INVALID	,
	BAD_OPEN,
	NO_SPACE,
	NEWLINE,
	REGERR,
	CWARN,
	YYERR,
	PRERR
};

/* Diagnostics Functions and Subroutines */
void	error();
void	regerr();
void	usage();
