/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:smtp/src/v9regexp.h	1.2.3.1"
/* the structure describing a sub-expression match */
typedef struct {
	char *sp;
	char *ep;
} regsubexp;

/* a compiled regular expression */
typedef char *regexp;

/* the routines */
extern regexp *regcomp();
extern int regexec();
extern void regsub();
