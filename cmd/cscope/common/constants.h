/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cscope:common/constants.h	1.3"
/*	cscope - interactive C symbol cross-reference
 *
 *	preprocessor macro and constant definitions
 */

#define ctrl(x)	(x & 037)	/* control character macro */

/* fast string equality tests (avoids most strcmp() calls) */
#define	strequal(s1, s2)	(*(s1) == *(s2) && strcmp(s1, s2) == 0)
#define	strnotequal(s1, s2)	(*(s1) != *(s2) || strcmp(s1, s2) != 0)

/* set the mark character for searching the cross-reference file */
#define	setmark(c)	(blockmark = c, block[blocklen] = blockmark)

/* get the next character in the cross-reference */
/* note that blockp is assumed not to be null */
#define	getrefchar()	(*(++blockp + 1) != '\0' ? *blockp : \
			(readblock() != NULL ? *blockp : '\0'))

/* skip the next character in the cross-reference */
/* note that blockp is assumed not to be null and that
   this macro will always be in a statement by itself */
#define	skiprefchar()	if (*(++blockp + 1) == '\0') readblock()

#define	ESC	'\033'		/* escape character */
#define	MSGLEN	PATLEN + 80	/* displayed message length */
#define	NUMLEN	5		/* line number length */
#define	PATHLEN	250		/* file pathname length */
#define	PATLEN	250		/* symbol pattern length */
#define	REFFILE	"cscope.out"	/* cross-reference output file */
#define	NAMEFILE "cscope.files"	/* default list-of-files file */
#define	STMTMAX	5000		/* maximum source statement length */
#define	READ	4		/* access(2) parameter */
#define	WRITE	2		/* access(2) parameter */
#undef	YYLMAX		
#define YYLMAX	STMTMAX + PATLEN + 1	/* scanner line buffer size */

/* cross-reference database mark characters (when new ones are added, 
 * update the cscope.out format description in cscope.1)
 */
#define CLASSDEF	'c'
#define	DEFINE		'#'
#define	DEFINEEND	')'
#define ENUMDEF		'e'
#define FCNCALL		'`'
#define FCNDEF		'$'
#define FCNEND		'}'
#define GLOBALDEF	'g'
#define	INCLUDE		'~'
#define MEMBERDEF	'm'
#define NEWFILE		'@'
#define STRUCTDEF	's'
#define TYPEDEF		't'
#define UNIONDEF	'u'

/* other scanner token types */
#define LEXEOF	0
#define	IDENT	1
#define	NEWLINE	2

/* screen lines */
#define	FLDLINE	(LINES - FIELDS - 1)	/* first input field line */
#define	MSGLINE	0			/* message line */
#define	PRLINE	(LINES - 1)		/* input prompt line */
#define REFLINE	3			/* first displayed reference line */

/* input fields (value matches field order on screen) */
#define	SYMBOL		0
#define DEFINITION	1
#define	CALLEDBY	2
#define	CALLING		3
#define	STRING		4
#define	CHANGE		5
#define	REGEXP		6
#define FILENAME	7
#define INCLUDES	8
#define	FIELDS		9

#if BSD || V9
#define TERMINFO	0	/* no terminfo curses */
#else
#define TERMINFO	1
#endif

#if !TERMINFO
#define	KEY_BREAK	0400	/* easier to define than to add #if around the use */
#define	KEY_ENTER	0401
#define	KEY_BACKSPACE	0402

#if !sun
#define cbreak()	crmode()			/* name change */
#endif

#if UNIXPC
#define	erasechar()	(_tty.c_cc[VERASE])		/* equivalent */
#define	killchar()	(_tty.c_cc[VKILL])		/* equivalent */
#else
#define	erasechar()	(_tty.sg_erase)			/* equivalent */
#define	killchar()	(_tty.sg_kill)			/* equivalent */
#endif
#endif
