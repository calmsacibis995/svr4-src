/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:devices/erase/cmderr.h	1.1"

/*
	Include file for error message interface.
	Command and library version.
*/

#define	cmderr	errtag( __FILE__, __LINE__ ),  errtext

extern	void	erradvice();
extern	void	errafter();	/* routine run after text is printed */
extern	void	errbefore();	/* routine run before text is printed */
extern	void	errtag();
extern	void	errtext();
extern	void	errtofix();
extern	void	errverb();
extern	int	errexit;	/* exit(2) code to use if error causes exit */
extern	char	**errmessage;	/* error messages that depend on severity */
extern	char	*pgm_name;

/* severities */
#define	CINFO	0
#define	CWARN	1
#define	CERROR	2
#define	CHALT	3

/* special errtext() argument that prints a standard message based on errno */
#define	CERRNO	1

/* list of additional arguments, after format, to errtext(), errbefore() and
   errafter() */
#define	ErrArgList	a1, a2, a3, a4, a5, a6, a7, a8, a9, a10
