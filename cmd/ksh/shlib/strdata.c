/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:shlib/strdata.c	1.1.3.1"
/*
 * data for string evaluator library
 */

#include	"streval.h"

const struct Optable optable[] =
	/* opcode	precedence,assignment  opname */
{
	{DEFAULT,	MAXPREC|NOASSIGN,	0177, 0 },
	{DONE,		0|NOASSIGN,		0 , 0 },
	{NEQ,		9|NOASSIGN,		'!', '=' },
	{NOT,		MAXPREC|NOASSIGN,	'!', 0 },
	{MOD,		13|NOFLOAT,		'%', 0 },
	{ANDAND,	5|NOASSIGN|SEQPOINT,	'&', '&' },
	{AND,		8|NOFLOAT,		'&', 0 },
	{LPAREN,	MAXPREC|NOASSIGN|SEQPOINT,'(', 0 },
	{RPAREN,	1|NOASSIGN,		')', 0 },
	{TIMES,		13,			'*', 0 },
#ifdef future
	{PLUSPLUS,	14|NOASSIGN|NOFLOAT|SEQPOINT, '+', '+'},
#endif
	{PLUS,		12,			'+', 0 },
#ifdef future
	{MINUSMINUS,	14|NOASSIGN|NOFLOAT|SEQPOINT, '-', '-'},
#endif
	{MINUS,		12,			'-', 0 },
	{DIVIDE,	13,			'/', 0 },
#ifdef future
	{COLON,		2|NOASSIGN,		':', 0 },
#endif
	{LSHIFT,	11|NOFLOAT,		'<', '<' },
	{LE,		10|NOASSIGN,		'<', '=' },
	{LT,		10|NOASSIGN,		'<', 0 },
	{EQ,		9|NOASSIGN,		'=', '=' },
	{ASSIGNMENT,	2|RASSOC,		'=', 0 },
	{RSHIFT,	11|NOFLOAT,		'>', '>' },
	{GE,		10|NOASSIGN,		'>', '=' },
	{GT,		10|NOASSIGN,		'>', 0 },
#ifdef future
	{QCOLON,	3|NOASSIGN|SEQPOINT,	'?', ':' },
	{QUEST,		3|NOASSIGN|SEQPOINT|RASSOC,	'?', 0 },
#endif
	{XOR,		7|NOFLOAT,		'^', 0 },
	{OROR,		5|NOASSIGN|SEQPOINT,	'|', '|' },
	{OR,		6|NOFLOAT,		'|', 0 }
};


#ifndef KSHELL
    const char e_number[]	= "bad number";
#endif /* KSHELL */
const char e_moretokens[]	= "more tokens expected";
const char e_paren[]		= "unbalanced parenthesis";
const char e_badcolon[]		= "invalid use of :";
const char e_divzero[]		= "divide by zero";
const char e_synbad[]		= "syntax error";
const char e_notlvalue[]	= "assignment requires lvalue";
const char e_recursive[]	= "recursion too deep";
#ifdef future
    const char e_questcolon[]	= ": expected for ? operator";
#endif
#ifdef FLOAT
    const char e_incompatible[]= "operands have incompatible types";
#endif /* FLOAT */

const char e_hdigits[] = "00112233445566778899aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ";
