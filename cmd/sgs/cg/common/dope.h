/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nifg:cg/common/dope.h	1.13"
long dope[ DSIZE ];
char *opst[DSIZE];

static struct dopest 
{
	int dopeop; 
	char *opst; 
#ifdef	STY
	char * opst2;		/* string corresponding to op's #define value */
#endif
	int dopeval; 
} indope[] = 
#ifndef	STY
#define	dopedef(value, printstring, flags) \
	value, printstring, flags
#else
#ifdef	__STDC__
#define	dopedef(value, printstring, flags) \
	value, printstring, #value, flags
#else
#define	dopedef(value, printstring, flags) \
	value, printstring, "value", flags
#endif
#endif
{
	dopedef(NAME, "NAME", LTYPE ),
	dopedef(STRING, "STRING", LTYPE ),
	dopedef(REG, "REG", LTYPE ),
	dopedef(TEMP, "TEMP", LTYPE ),
	dopedef(VAUTO, "AUTO", LTYPE ),
	dopedef(VPARAM, "PARAM", LTYPE ),
	dopedef(ICON, "ICON", LTYPE ),
	dopedef(FCON, "FCON", LTYPE ),
	dopedef(CCODES, "CCODES", LTYPE ),
	dopedef(UNARY MINUS, "U-", UTYPE ),
	dopedef(STAR, "STAR", UTYPE ),
	dopedef(UNARY AND, "U&", UTYPE ),
	dopedef(UNARY CALL, "UCALL", UTYPE|CALLFLG ),
	dopedef(UNARY FORTCALL, "UFCALL", UTYPE|CALLFLG ),
	dopedef(NOT, "!", UTYPE|LOGFLG ),
	dopedef(COMPL, "~", UTYPE ),
	dopedef(INIT, "INIT", UTYPE ),
	dopedef(CONV, "CONV", UTYPE ),
	dopedef(PLUS, "+", BITYPE|FLOFLG|SIMPFLG|COMMFLG ),
	dopedef(ASG PLUS, "+=", BITYPE|ASGFLG|ASGOPFLG|FLOFLG|SIMPFLG|COMMFLG ),
	dopedef(MINUS, "-", BITYPE|FLOFLG|SIMPFLG ),
	dopedef(ASG MINUS, "-=", BITYPE|FLOFLG|SIMPFLG|ASGFLG|ASGOPFLG ),
	dopedef(MUL, "*", BITYPE|FLOFLG|MULFLG ),
	dopedef(ASG MUL, "*=", BITYPE|FLOFLG|MULFLG|ASGFLG|ASGOPFLG ),
	dopedef(AND, "&", BITYPE|SIMPFLG|COMMFLG ),
	dopedef(ASG AND, "&=", BITYPE|SIMPFLG|COMMFLG|ASGFLG|ASGOPFLG ),
	dopedef(QUEST, "?", BITYPE ),
	dopedef(COLON, ":", BITYPE ),
	dopedef(ANDAND, "&&", BITYPE|LOGFLG ),
	dopedef(OROR, "||", BITYPE|LOGFLG ),
	dopedef(CM, ",", BITYPE ),
	dopedef(COMOP, ",OP", BITYPE ),
	dopedef(FREE, "FREE!?!", LTYPE ),
	dopedef(ASSIGN, "=", BITYPE|ASGFLG ),
	dopedef(DIV, "/", BITYPE|FLOFLG|MULFLG|DIVFLG|AMBFLG ),
	dopedef(ASG DIV, "/=", BITYPE|FLOFLG|MULFLG|DIVFLG|ASGFLG|ASGOPFLG|AMBFLG ),
	dopedef(MOD, "%", BITYPE|DIVFLG|AMBFLG ),
	dopedef(ASG MOD, "%=", BITYPE|DIVFLG|ASGFLG|ASGOPFLG|AMBFLG ),
	dopedef(LS, "<<", BITYPE|SHFFLG ),
	dopedef(ASG LS, "<<=", BITYPE|SHFFLG|ASGFLG|ASGOPFLG ),
	dopedef(RS, ">>", BITYPE|SHFFLG|AMBFLG ),
	dopedef(ASG RS, ">>=", BITYPE|SHFFLG|ASGFLG|ASGOPFLG|AMBFLG ),
	dopedef(OR, "|", BITYPE|COMMFLG|SIMPFLG ),
	dopedef(ASG OR, "|=", BITYPE|COMMFLG|SIMPFLG|ASGFLG|ASGOPFLG ),
	dopedef(ER, "^", BITYPE|COMMFLG|SIMPFLG ),
	dopedef(ASG ER, "^=", BITYPE|COMMFLG|SIMPFLG|ASGFLG|ASGOPFLG ),
	dopedef(INCR, "++", BITYPE|ASGFLG ),
	dopedef(DECR, "--", BITYPE|ASGFLG ),
	dopedef(STREF, "->", BITYPE ),
	dopedef(DOT, ".", BITYPE),
	dopedef(CALL, "CALL", BITYPE|CALLFLG ),
	dopedef(FORTCALL, "FCALL", BITYPE|CALLFLG ),
	dopedef(EQ, "==", BITYPE|LOGFLG ),
	dopedef(NE, "!=", BITYPE|LOGFLG ),
	dopedef(LE, "<=", BITYPE|LOGFLG|AMBFLG ),
	dopedef(LT, "<", BITYPE|LOGFLG|AMBFLG ),
	dopedef(GE, ">=", BITYPE|LOGFLG|AMBFLG ),
	dopedef(GT, ">", BITYPE|LOGFLG|AMBFLG ),
	dopedef(UGT, "UGT", BITYPE|LOGFLG|AMBFLG ),
	dopedef(UGE, "UGE", BITYPE|LOGFLG|AMBFLG ),
	dopedef(ULT, "ULT", BITYPE|LOGFLG|AMBFLG ),
	dopedef(ULE, "ULE", BITYPE|LOGFLG|AMBFLG ),
	dopedef(ARS, ">>A", BITYPE ),
	dopedef(ASG ARS, ">>=A", BITYPE|ASGFLG|ASGOPFLG ),
	dopedef(TYPE, "TYPE", LTYPE ),
	dopedef(LB, "[", BITYPE ),
	dopedef(CBRANCH, "CBRANCH", BITYPE ),
	dopedef(GENLAB, "GENLAB", UTYPE ),
	dopedef(GENUBR, "GENUBR", UTYPE ),
	dopedef(GENBR, "GENBR", UTYPE ),
	dopedef(CMP, "CMP", BITYPE ),
	dopedef(CMPE, "CMPE", BITYPE ),		/* IEEE exception raising fp comparison */
	dopedef(FLD, "FLD", UTYPE ),
	dopedef(PMUL, "P*", BITYPE ),
	dopedef(PDIV, "P/", BITYPE ),
#ifdef	CG
	dopedef(RETURN, "RETURN", LTYPE ),
#else
	dopedef(RETURN, "RETURN", BITYPE|ASGFLG ),
#endif
	dopedef(CAST, "CAST", BITYPE|ASGFLG ),
	dopedef(GOTO, "GOTO", UTYPE ),
	dopedef(STASG, "STASG", BITYPE|STRFLG ),
	dopedef(STARG, "STARG", UTYPE|STRFLG ),
	dopedef(STCALL, "STCALL", BITYPE|CALLFLG|STRFLG ),
	dopedef(UNARY STCALL, "USTCALL", UTYPE|CALLFLG|STRFLG ),
	dopedef(RNODE, "RNODE", LTYPE ),
	dopedef(SNODE, "SNODE", LTYPE ),
	dopedef(QNODE, "QNODE", LTYPE ),
	dopedef(MANY, "MANY", BITYPE ),
	dopedef(FUNARG, "ARG", UTYPE ),
	dopedef(UOP0, "UOP0", UTYPE|SPFLG ),
	dopedef(UOP1, "UOP1", UTYPE|SPFLG ),
	dopedef(UOP2, "UOP2", UTYPE|SPFLG ),
	dopedef(UOP3, "UOP3", UTYPE|SPFLG ),
	dopedef(UOP4, "UOP4", UTYPE|SPFLG ),
	dopedef(UOP5, "UOP5", UTYPE|SPFLG ),
	dopedef(UOP6, "UOP6", UTYPE|SPFLG ),
	dopedef(UOP7, "UOP7", UTYPE|SPFLG ),
	dopedef(UOP8, "UOP8", UTYPE|SPFLG ),
	dopedef(UOP9, "UOP9", UTYPE|SPFLG ),
#ifdef IN_LINE
	dopedef(INCALL, "INCALL", BITYPE|CALLFLG ),
	dopedef(UNARY INCALL, "UINCALL", UTYPE|CALLFLG ),
#endif
#ifdef	CG
	dopedef(COPY, "COPY", LTYPE ),
	dopedef(COPYASM, "COPYASM", LTYPE|CALLFLG ),
	dopedef(CURCAP, "CURCAP", LTYPE ),
	dopedef(NOP, "NOP", LTYPE ),
	dopedef(BEGF, "BEGF", LTYPE ),
	dopedef(ENTRY, "ENTRY", LTYPE),
	dopedef(PROLOG, "PROLOG", LTYPE),
	dopedef(ENDF, "ENDF", LTYPE ),
	dopedef(LOCCTR, "LOCCTR", LTYPE ),
	dopedef(SWBEG, "SWBEG", UTYPE ),
	dopedef(SWCASE, "SWCASE", LTYPE ),
	dopedef(SWEND, "SWEND", LTYPE ),
	dopedef(ALLOC, "ALLOC", UTYPE ),
	dopedef(DEFNAM, "DEFNAM", LTYPE ),
	dopedef(UNINIT, "UNINIT", LTYPE ),
	dopedef(CURFRAME, "CURFRAME", LTYPE ),
	dopedef(FCHAIN, "FCHAIN", UTYPE ),
	dopedef(FSELECT, "FSELECT", BITYPE ),
	dopedef(BMOVE, "BMOVE", BITYPE ),
	dopedef(BMOVEO, "BMOVEO", BITYPE ),
	dopedef(EXSETV, "EXSETV", UTYPE ),
	dopedef(EXGETV, "EXGETV", LTYPE ),
	dopedef(EXCLEAR, "EXCLEAR", LTYPE ),
	dopedef(EXTEST, "EXTEST", LTYPE ),
	dopedef(EXRAISE, "EXRAISE", LTYPE ),
	dopedef(JUMP, "JUMP", LTYPE ),
	dopedef(SINIT, "SINIT", LTYPE ),
	dopedef(LET, "LET", BITYPE ),
	dopedef(CSE, "CSE", LTYPE ),
	dopedef(ALIGN, "ALIGN", LTYPE ),
	dopedef(NPRETURN, "NPRETURN", LTYPE ),
	dopedef(VLRETURN, "VLRETURN", BITYPE ),
	dopedef(FCONV, "FCONV", UTYPE ),
	dopedef(BCMP, "BCMP", BITYPE|LOGFLG),
	dopedef(SEMI, ";", BITYPE ),
	dopedef(CAPCALL, "CAPCALL", UTYPE|CALLFLG),
	dopedef(CAPRET, "CAPRET", LTYPE),
	dopedef(RSAVE, "RSAVE", LTYPE),
	dopedef(RREST, "RREST", LTYPE),
	dopedef(LABELOP, "LABELOP", LTYPE),
#ifdef	UPLUS
	dopedef(UPLUS, "UPLUS", UTYPE),
#endif
	dopedef(NAMEINFO, "NAMEINFO", LTYPE),
#endif	/* def CG */

	-1,	0,	0
};
#undef	dopedef

void
mkdope()
{
	register struct dopest *q;

	for( q = indope; q->dopeop >= 0; ++q )
	{
		/* Sorry about this....  But, better to find out things are
		** messed up here, even if it takes funny code.
		*/
		if (q->dopeop >= DSIZE)
#ifdef	STY
		    yyerror
#else
		    cerror
#endif
		    	("bad dope[] OP %s\n", q->opst);
		dope[q->dopeop] = q->dopeval;
		opst[q->dopeop] = q->opst;
	}
}
