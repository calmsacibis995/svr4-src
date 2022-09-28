/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:smtp/src/regprog.h	1.3.3.1"
/* ident "@(#)regprog.h	1.3 'attmail mail(1) command'" */
#include "v9regexp.h"

/* useful constants */
#define NULL 0
#define	TRUE	1
#define	FALSE	0
#define	HAT	'\177'		/* character to mark real beg. of line */

/*
 *	Sub expression matches
 */
#define NSUBEXP 10
typedef struct {
	regsubexp m[NSUBEXP];
}Subexp;

/*
 *	character class
 */
typedef struct {
	char	map[16];	/* 16 bytes == 128 bits, one bit per char */
}Class;

/*
 *	Machine instructions
 */
typedef struct Inst{
	int	type;			/* < 0200 ==> literal, otherwise action */
	union	{
		int sid;		/* sub-expression id for RBRA and LBRA */
		struct Inst *other;	/* instructions pointer */
	} u;
	struct Inst *left;
}Inst;
#define	next	left	/* Left branch is usually just next ptr */
#define	subid	u.sid
#define right	u.other

/*
 *	Program definition
 */
#define	NCLASS	16
typedef struct Program{
	Inst	*startinst;	/* start pc */
	Class	class[NCLASS];	/* .data */
	Inst	firstinst[5];	/* .text */
}Prog;

/*
 * Actions and Tokens
 *
 *	02xx are operators, value == precedence
 *	03xx are tokens, i.e. operands for operators
 */
#define	OPERATOR	0200	/* Bitmask of all operators */
#define	START		0200	/* Start, used for marker on stack */
#define	RBRA		0201	/* Right bracket, ) */
#define	LBRA		0202	/* Left bracket, ( */
#define	OR		0203	/* Alternation, | */
#define	CAT		0204	/* Concatentation, implicit operator */
#define	STAR		0205	/* Closure, * */
#define	PLUS		0206	/* a+ == aa* */
#define	QUEST		0207	/* a? == a|nothing, i.e. 0 or 1 a's */
#define	ANY		0300	/* Any character, . */
#define	NOP		0301	/* No operation, internal use only */
#define	BOL		0302	/* Beginning of line, ^ */
#define	EOL		0303	/* End of line, $ */
#define	CCLASS		0304	/* Character class, [] */
#define	END		0377	/* Terminate: match found */
