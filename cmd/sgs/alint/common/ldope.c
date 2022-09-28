/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)alint:common/ldope.c	1.1"
#include "p1.h"
#include "ldope.h"

/*
** Special dope array for lint's routines.  Only those ops that have
** some sort of special meaning are in the ln_dope[] array; the bulk
** of the array will be blank.
**
** GLOBAL ENTRY POINTS:
**	ldope()
*/


long 	ln_dope[DSIZE];
char 	*ln_opst[DSIZE];

/* 
** DOPE stuff for lint
*/

#define LN_DEF(ind,flags,str) ln_dope[ind] = flags; ln_opst[ind] = str

void
ldope()
{
	LN_DEF(ANDAND,		LOGSHRT|SIDE|LOG|LNSIDE,"");
	LN_DEF(OROR,		LOGSHRT|SIDE|LOG|LNSIDE,"");
	LN_DEF(QUEST,		SIDE|LNSIDE,"");
	LN_DEF(COLON,		LNSIDE,"");
	LN_DEF(COMOP,		LNSIDE,"");
	LN_DEF(EQ,		EQUALITY|LOG,"==");
	LN_DEF(NE,		EQUALITY|LOG,"!=");
	LN_DEF(LT,		RLOP|LOG,"<");
	LN_DEF(LE,		RLOP|LOG,"<=");
	LN_DEF(GT,		RLOP|LOG,">");
	LN_DEF(GE,		RLOP|LOG,">=");
	LN_DEF(ULT,		RLOP|LOG,"<");
	LN_DEF(UGT,		RLOP|LOG,">");
	LN_DEF(ULE,		RLOP|LOG,"<=");
	LN_DEF(UGE,		RLOP|LOG,">=");
	LN_DEF(AND,		BITW,"");
	LN_DEF(OR,		BITW,"");
	LN_DEF(ER,		BITW,"");
	LN_DEF(ASG AND,		BITW|LNSIDE,"");
	LN_DEF(ASG OR,		BITW|LNSIDE,"");
	LN_DEF(ASG ER,		BITW|LNSIDE,"");
	LN_DEF(RS,		BITW|SHIFT,"");
	LN_DEF(LS,		BITW|SHIFT,"");
	LN_DEF(ARS,		BITW|SHIFT,"");
	LN_DEF(ASG LS,		BITW|SHIFT|LNSIDE,"");
	LN_DEF(ASG RS,		BITW|SHIFT|LNSIDE,"");
	LN_DEF(ASG ARS,		BITW|SHIFT|LNSIDE,"");
	LN_DEF(INCR,		CPY|LNSIDE,"");
	LN_DEF(DECR,		CPY|LNSIDE,"");
	LN_DEF(ASG PLUS,	CPY|LNSIDE,"");
	LN_DEF(ASG MINUS,	CPY|LNSIDE,"");
	LN_DEF(UNARY CALL,	LNSIDE,"");
	LN_DEF(UNARY FORTCALL,	LNSIDE,"");
	LN_DEF(CALL,		LNSIDE,"");
	LN_DEF(FORTCALL,	LNSIDE,"");
	LN_DEF(STCALL,		LNSIDE,"");
	LN_DEF(UNARY STCALL,	LNSIDE,"");
	LN_DEF(INCALL,		LNSIDE,"");
	LN_DEF(UNARY INCALL,	LNSIDE,"");
	LN_DEF(COPYASM,		LNSIDE,"");
	LN_DEF(CAPCALL,		LNSIDE,"");
	LN_DEF(CBRANCH,		LNSIDE,"");
	LN_DEF(RETURN,		LNSIDE,"");
	LN_DEF(ASG MUL,		LNSIDE,"");
	LN_DEF(ASG AND,		LNSIDE,"");
	LN_DEF(ASSIGN,		LNSIDE,"");
	LN_DEF(CAST,		LNSIDE,"");
	LN_DEF(ASG DIV,		LNSIDE,"");
	LN_DEF(ASG MOD,		LNSIDE,"");
}
