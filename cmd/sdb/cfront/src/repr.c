/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/src/repr.c	1.1"
/*ident	"@(#)cfront:src/repr.c	1.4" */
/**************************************************************************

	repr.c: stage main (views: main err)
*/

#include "cfront.h"

char* oper_name(TOK op)
/*
	return the string representation of operator "op"
*/
{
	switch (op) {
	default:	error('i',"oper_name(%k)",op);
	case CM:	return "_comma";
	case NEW:	return "_new";
	case DELETE:	return "_delete";
	case MUL:	return "_mul";
	case DIV:	return "_div";
	case MOD:	return "_mod";
	case UPLUS:
	case PLUS:	return "_plus";
	case MINUS:
	case UMINUS:	return "_minus";
	case LS:	return "_lshift";
	case RS:	return "_rshift";
	case EQ:	return "_eq";
	case NE:	return "_ne";
	case LT:	return "_lt";
	case GT:	return "_gt";
	case LE:	return "_le";
	case GE:	return "_ge";
	case AND:
	case ADDROF:	return "_and";
	case OR:	return "_or";
	case ER:	return "_er";
	case ANDAND:	return "_andand";
	case OROR:	return "_oror";
	case NOT:	return "_not";
	case COMPL:	return "_compl";
	case INCR:	return "_incr";
	case DECR:	return "_decr";
	case CALL:	return "_call";
	case DEREF:	return "_vec";
	case ASSIGN:	return "_assign";
	case ASPLUS:	return "_asplus";
	case ASMINUS:	return "_asminus";
	case ASMUL:	return "_asmul";
	case ASDIV:	return "_asdiv";
	case ASMOD:	return "_asmod";
	case ASLS:	return "_asls";
	case ASRS:	return "_asrs";
	case ASAND:	return "_asand";
	case ASOR:	return "_asor";
	case ASER:	return "_aser";
	case SIZEOF:	return "sizeof";
	}
}

#define new_op(ss,v) keys[v]=ss

void otbl_init()
/*
	operator representation table
*/
{
	new_op("->",REF);
	new_op("." ,DOT);
	new_op("!" ,NOT);
	new_op("~" ,COMPL);
	new_op("++",INCR);
	new_op("--",DECR);
	new_op("*" ,MUL);
	new_op("&" ,AND);
	new_op("&" ,ADDROF);
	new_op("&" ,G_ADDROF);
	new_op("/" ,DIV);
	new_op("%" ,MOD);
	new_op("+" ,PLUS);
	new_op("-" ,MINUS);
	new_op("-" ,UMINUS);
	new_op("<<",LS);
	new_op(">>",RS);
	new_op("<" ,LT);
	new_op(">" ,GT);
	new_op("<=",LE);
	new_op(">=",GE);
	new_op("==",EQ);
	new_op("!=",NE);
	new_op("^" ,ER);
	new_op("|" ,OR);
	new_op("&&",ANDAND);
	new_op("||",OROR);
	new_op("?" ,QUEST);
	new_op(":" ,COLON);
	new_op("=" ,ASSIGN);
	new_op("," ,CM);
	new_op("," ,G_CM);

	new_op(";" ,SM);
	new_op("{" ,LC);
	new_op("}" ,RC);
	new_op("(" ,LP);
	new_op(")" ,RP);
	new_op("[" ,LB);
	new_op("]" ,RB);

	new_op("+=",ASPLUS);
	new_op("-=",ASMINUS);
	new_op("*=",ASMUL);
	new_op("/=",ASDIV);
	new_op("%=",ASMOD);
	new_op("&=",ASAND);
	new_op("|=",ASOR);
	new_op("^=",ASER);
	new_op(">>=",ASRS);
	new_op("<<=",ASLS);

	new_op("sizeof",SIZEOF);
	new_op("new",NEW);
	new_op("delete",DELETE);

	new_op("0" ,ZERO);
	new_op("," ,ELIST);
	new_op("[]" ,DEREF);
	new_op("expression list", ELIST);
	new_op("function call", CALL);
	new_op("generated function call",G_CALL);
	new_op("inline function call",ICALL);
	new_op("cast",CAST);
	new_op("inline argument",ANAME);
	new_op(".*",MEMPTR);

	new_op("class", COBJ);
	new_op("enum", EOBJ);
	new_op("union", ANON);

	new_op("function",FCT);
	new_op("pointer",PTR);
	new_op("reference",RPTR);
	new_op("vector",VEC);
	new_op("identifier",ID);
	new_op("name",NAME);
	new_op("...",ELLIPSIS);
	new_op("::",MEM);
	new_op("type name",TYPE);
	new_op("{}",BLOCK);
	new_op("pair",PAIR);
	new_op("declaration",DCL);
	new_op("character constant",CCON);
	new_op("integer constant",ICON);
	new_op("float constant",FCON);
	new_op("string",STRING);
	new_op("label",LABEL);
	new_op("'class', 'struct', or 'union'",AGGR);
}
