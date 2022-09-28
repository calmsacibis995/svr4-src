/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:include/streval.h	1.1.3.1"
/*
 * G. S. Fowler
 * D. G. Korn
 * AT&T Bell Laboratories
 *
 * long integer arithmetic expression evaluator
 */

/* The following only is needed for const */
#include	"sh_config.h"

struct lval
{
	char	*value;
	int	flag;
};

#ifdef FLOAT
typedef double number;
#else
typedef long number;
#endif /* FLOAT */

#define MAXPREC		15	/* maximum precision level */
#define SEQPOINT	0200	/* sequence point */
#define NOASSIGN	0100	/* assignment legal with this operator */
#define RASSOC		040	/* right associative */
#define NOFLOAT		020	/* illegal with floating point */
#define PRECMASK	017	/* precision bit mask */

#define  DEFAULT	0
#define  LPAREN		1
#define  RPAREN		2
#define  COMMA		3
#define  ASSIGNMENT	4
#define  MOD		5
#define  LSHIFT		6
#define  RSHIFT		7
#define  PLUS		8
#define  MINUS		9
#define  DIVIDE		10
#define  EQ		11
#define  NEQ		12
#define  LT		13
#define  GT		14
#define  LE		15
#define  GE		16
#define  AND		17
#define  OR		18
#define  XOR		19
#define  ANDAND		20
#define  OROR		21
#define  DONE		22
#define  NOT		23
#define  QUEST		24
#define  QCOLON		25
#define  TIMES		26
#define  ARROW		27
#define  DOT		28
#define  LBRACKET	29
#define  RBRACKET	30
#define  PLUSPLUS	31
#define  MINUSMINUS	32
#define  COLON		33


struct Optable
{
	char	opcode;
	char	precedence;
	char	name[2];
};


/* define error messages */
extern const char	e_moretokens[];
extern const char	e_paren[];
extern const char	e_number[];
extern const char	e_badcolon[];
extern const char	e_recursive[];
extern const char	e_divzero[];
extern const char	e_synbad[];
extern const char	e_notlvalue[];
#ifdef FLOAT
   extern const char	e_incompatible[];
#endif /* FLOAT */

/* function code for the convert function */

#define LOOKUP	0
#define ASSIGN	1
#define VALUE	2
#define ERRMSG	3

#ifdef PROTO
    extern number streval(char*,char**,number(*)());
#else
    extern number streval();
#endif /* PROTO */
