/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)optim:i386/optutil.c	1.1.2.17"
/* optutil.c
**
**	Optimizer utilities for Intel 386 code improver (PCI).
**
**
** This module contains utility routines for the various peephole
** window optimizations.
*/

/* #include <ctype.h> -- optim.h takes care of this */
/* #include "defs" -- optim.h takes care of this; machine-dependent defs */
#include "optim.h"			/* machine independent optimizer defs */
#include "optutil.h"


/* isreg -- is operand string a register reference?
**
** This routine tells whether an operand string is a register reference.
*/

boolean
isreg(s)
char * s;				/* operand string */
{
/* assume operand is a register reference if it begins with a % */

	return(s != NULL && *s == '%');
}

/* insert -- insert new instruction node
**
** This routine inserts a new instruction node after the one pointed
** to.  The fields in the node are initialized to null values.
*/

NODE *					/* return pointer to the new node */
insert(pn)
NODE * pn;				/* node to add pointer after */
{
    NODE * Saveop();			/* in machine-independent part */
    NODE * new = Saveop(0,"",0,GHOST);	/* create isolated node, init. */

    APPNODE(new,pn);			/* append new node after current */

    return(new);			/* return pointer to the new one */
}
/* chgop -- change op code number and op code string in node
**
** This routine changes the op code information in a node.  The
** operand information is unaffected.
*/

void
chgop(n,op,op_code)
NODE * n;				/* pointer to node to change */
int op;					/* op code number */
char * op_code;				/* pointer to op code string */
{
    n->op = (unsigned short)op;		/* set op code number */
    n->opcode = op_code;		/* and string */
    return;
}
/* isdyadic -- is instruction dyadic
**
** This routine tells whether an instruction is dyadic according
** to the following conditions:
**
**	1.	It takes two operands.
**	2.	It reads the first operand, writes the second.
**	3.	It sets the condition codes according to the value stored in
**		the second operand.
**	4.	The operands have normal address mode meanings:  disallow,
**		for example, LEAL, which does address arithmetic of op1.
*/

boolean
isdyadic(n)
NODE * n;				/* instruction node to test */
{
    switch(n->op)			/* dispatch on op code number */
    {
    case ADDB: case ADDW: case ADDL:
    case ANDB: case ANDW: case ANDL:
    case MOVB: case MOVW: case MOVL:
    case MOVSBW: case MOVSBL: case MOVSWL:
    case MOVZBL: case MOVZBW: case MOVZWL:
    case ORB:  case ORW:  case ORL:
    case SARB: case SARW: case SARL:
    case SHLB: case SHLW: case SHLL:
    case SHRB: case SHRW: case SHRL:
    case SUBB: case SUBW: case SUBL:
    case XORB: case XORW: case XORL:
	return(true);			/* all of these are dyadic */
    default:
	return(false);			/* others are not */
    }
}
/* istriadic -- is instruction triadic?
**
** This routine serves a similar function to isdyadic, except that
** it also returns the related dyadic op code number and string.
** The only instructions included here are the obvious ones.  They
** are assumed to have integer operands (by callers).
*/

/* define macro to use below */

#define CHANGE(op,new,newst)	case op: *pnewop=new;*pnewopcode=newst;break;


boolean
istriadic(n,pnewop,pnewopcode)
NODE * n;				/* node to test */
int * pnewop;				/* place to put equiv. dyadic # */
char ** pnewopcode;			/* place to put equiv. dyadic string */
{
	if ( ( n->op == IMULL ||
	       n->op == IMULW ||
	       n->op == IMULB ) &&
	     isnumlit(n->op1) &&
	     isreg(n->op3) ) {		/* imul[bwl]  $X,op,reg case */
		*pnewop = n->op;
		switch ( n->op ) {
		    case IMULL:
			*pnewopcode = "imull";
			break;
		    case IMULW:
			*pnewopcode = "imulw";
			break;
		    case IMULB:
			*pnewopcode = "imulb";
			break;
		}
		return( true );
	}

	return(false);
}
/* is2dyadic -- is instruction dyadic
**
** This routine is similar to isdyadic() but has the following
** conditions:
**
**	1.	It takes two operands.
**	2.	It reads at least the first operand (and doesn't
**		write it).
**	3.	The first operand can be register, memory, or
**		immediate mode (if it's memory or immediate, the
**		other operand must be register).
**	4.	The operands have normal address mode meanings:  disallow,
**		for example, LEAL, which does address arithmetic of op1.
*/

boolean
is2dyadic(n)
NODE * n;				/* instruction node to test */
{
    switch(n->op)			/* dispatch on op code number */
    {
    case ADDB: case ADDW: case ADDL:
    case ANDB: case ANDW: case ANDL:
    case CMPB: case CMPW: case CMPL:
    case MOVB: case MOVW: case MOVL:
    case ORB:  case ORW:  case ORL:
    case SUBB: case SUBW: case SUBL:
    case XORB: case XORW: case XORL:
	return(true);			/* all of these are dyadic */
    default:
	return(false);			/* others are not */
    }
}
/* isfp -- is instruction a floating point instruction? */

	boolean
isfp(n)
NODE *n;
{
    switch(n->op)
    {
    case F2XM1: case FABS: case FADD: case FADDL:
    case FADDP: case FADDS: case FBLD: case FBSTP: case FCHS:
    case FCLEX: case FCOM: case FCOML: case FCOMP: case FCOMPL:
    case FCOMPP: case FCOMPS: case FCOMS: case FDECSTP: case FDIV:
    case FDIVL: case FDIVP: case FDIVR: case FDIVRL: case FDIVRP:
    case FDIVRS: case FDIVS: case FFREE: case FIADD: case FIADDL:
    case FICOM: case FICOML: case FICOMP: case FICOMPL: case FIDIV:
    case FIDIVL: case FIDIVR: case FIDIVRL: case FILD: case FILDL:
    case FILDLL: case FIMUL: case FIMULL: case FINCSTP: case FINIT:
    case FIST: case FISTL: case FISTP: case FISTPL: case FISTPLL:
    case FISUB: case FISUBL: case FISUBR: case FISUBRL: case FLD:
    case FLD1: case FLDCW: case FLDENV: case FLDL: case FLDL2E:
    case FLDL2T: case FLDLG2: case FLDLN2: case FLDPI: case FLDS:
    case FLDT: case FLDZ: case FMUL: case FMULL: case FMULP:
    case FMULS: case FNCLEX: case FNINIT: case FNOP: case FNSAVE:
    case FNSTCW: case FNSTENV: case FNSTSW: case FPATAN: case FPREM:
    case FPTAN: case FRNDINT: case FRSTOR: case FSAVE: case FSCALE:
    case FSETPM: case FSQRT: case FST: case FSTCW: case FSTENV:
    case FSTL: case FSTP: case FSTPL: case FSTPS: case FSTPT:
    case FSTS: case FSTSW: case FSUB: case FSUBL: case FSUBP:
    case FSUBR: case FSUBRL: case FSUBRP: case FSUBRS: case FSUBS:
    case FTST: case FWAIT: case FXAM: case FXCH: case FXTRACT:
    case FYL2X: case FYL2XP1:
      return(true);
    default:
      return(false);
    }
}

	boolean
is_byte_instr(n)
NODE *n;
{
	switch (n->op) {
	case ADCB: case ADDB: case DIVB: case IDIVB: case MULB: 
	case SUBB: case SBBB: case ANDB: case ORB: case XORB:
	case CMPB: case TESTB: case MOVB: case IMULB:
	case MOVSBL: case MOVSBW: case MOVZBW: case MOVZBL:
		return true;
	default:
		return false;
	}
}
/*
 * setreg -- return the index of the named register (if one)
 */
int
setreg(cp)
char *cp;
{
	int reg;

	if (cp != NULL && *cp == '%' && (reg = scanreg(cp, false)))
		return(reg);
	return(0);
}
/* makelive -- make register live
**
** This routine marks a register operand as live.
*/

void
makelive(s,n)
char * s;				/* operand string (to %r0) */
NODE * n;				/* pointer to node to enliven */
{
    n->nlive |= setreg(s);
    return;
}



/* makedead -- make register dead
**
** This routine marks a register operand as dead.
*/

void
makedead(s,n)
char * s;				/* operand string (to %rn) */
NODE * n;				/* pointer to node to kill */
{
    n->nlive &= ~ setreg(s);
    return;
}
/* isindex -- is operand an index off a register?
**
** This routine checks whether an operand string is of the
** form:
**
**	n(%rn)
**
** and whether it uses a designated register.
**
** Actually, it cheats:  it just checks for an initial digit or sign followed
** by a (.  In fact the ( test is a cheat, because we just check to see
** if there's a ( in the string.
*/

boolean
isindex(s,r)
char * s;				/* operand string to test */
char * r;				/* register operand string to check
					** for
					*/

{
    if(s != NULL && *s == '*')
		s++;
    return(
		usesreg(s,r)
	    &&	(isdigit(*s) || *s == '-') /* could be leading sign, too */
	    &&  strchr(s,'(') != (char *) 0
	    &&  strchr(s,',') == (char *) 0	/* discard n(%rx,%ry,m) */
	    );
}
/* ismove -- is instruction a move?
**
** This routine determines whether an instruction is a move-type.
** There is a multiplicity of move instructions which copy, zero-
** or sign-extend, and truncate.  This routine returns the natural
** size of the source and destination operands.
*/

boolean
ismove(n,srcsize,dstsize)
NODE * n;				/* pointer to instruction node */
int * srcsize;				/* place to put size of source
					** operand (in bytes)
					*/
int * dstsize;				/* place to put size of destination */
{
    register int srctemp = 1;		/* temporary source size */
    register int dsttemp = 4;		/* temporary destination size */
/* Initial values chosen on basis of most common value of each. */

    switch (n->op)			/* dispatch on op-code number */
    {
    case MOVZBW:
			dsttemp = 2;	break;
    case MOVZBL:
					break;
    case MOVZWL:
	srctemp = 2;			break;

    case MOVB:
			dsttemp = 1;	break;
    case MOVW:
	srctemp = 2;	dsttemp = 2;	break;
    case MOVL:
	srctemp = 4;			break;

    case MOVSBW:
			dsttemp = 2;	break;
    case MOVSBL:
					break;
    case MOVSWL:
	srctemp = 2;			break;

    default:
	return(false);			/* not a move instruction */
    }
    *srcsize = srctemp;			/* copy out correct values */
    *dstsize = dsttemp;
    return(true);			/* found a move */
}
/* is_legal_op1 -- check that operand size in bytes is legal for this
		     opcode.  This is a very conservative check which
		     fixes a bug in one of the 2 instruction peepholes
		     (reorder loads) */
	boolean
is_legal_op1(oprndsize,n)
int oprndsize;
NODE *n;
{
	switch(oprndsize) {
	case 1: return (is_byte_instr(n));
	case 2: /* inclusion of single op instructions is harmless,
		   albeit (perhaps) useless */
		switch(n->op) {
		case ADCW: case ADDW: case DECW: case DIVW: case IDIVW:
		case IMULW: case MULW: case INCW: case NEGW: case SBBW:
		case SUBW: case ANDW: case ORW: case XORW: case CLRW:
		case RCLW: case RCRW: case ROLW: case RORW: case SALW:
		case SARW: case SHLW: case SHRW:
		case CMPW: case TESTW: case CWTL: case CWTD: case LEAW:
		case MOVW: case MOVSWL: case MOVZWL: case NOTW: case PUSHW:
		case XCHGW: case SCAW: case SCMPW: case SLODW: case SMOVW:
			return true;
		default:
			return false;
		}
	case 4:
		switch(n->op) {
		case ADCL: case ADDL: case DECL: case DIVL: case IDIVL:
		case IMULL: case MULL: case INCL: case NEGL: case SBBL:
		case SUBL:
		case ANDL: case ORL: case XORL: case CLRL: case RCLL:
		case RCRL: case ROLL: case RORL: case SALL: case SARL:
		case SHLL: case SHRL: case CMPL:
		case TESTL: case CLTD: case LEAL:
		case MOVW:
		case NOTL: case PUSHL: case XCHGL: case SCAL:
		case SCMPL: case SLODL: case SMOVL:
			return true;
		default:
			return false;
		}
	default:
		return false;
	}
}
/* iszoffset -- is operand zero offset from register
**
** This routine determines whether a given operand is an indexed
** operation from a designated register, where the index is zero.
*/

boolean
iszoffset(operand,reg)
register char * operand;		/* operand string */
char * reg;				/* register string */
{

/* We use a quick and dirty strategy here:  we test the operand for
** zero, followed by (, with matching register numbers in the right
** place.  We assume register designations are "%eXX".
*/

    return(	operand != NULL
	    &&  *operand == '('
	    &&  *(operand+1) == *reg
	    &&  *(operand+2) == *(reg+1)
	    &&  *(operand+3) == *(reg+2)
	    &&  *(operand+4) == *(reg+3)
	    &&  *(operand+5) == ')'
	  );
}
/* exchange -- exchange node and successor
**
** This routine reverses the linkages of two nodes so the first is
** the second and the second is the first.
*/

void
exchange(first)
NODE * first;				/* pointer to first of two nodes */
{

/* This can probably be done with fewer temporaries, but it's a lot
** easier to understand what's going on this way....
*/

    NODE * prev = first->back;		/* predecessor of first node */
    NODE * second = first->forw;	/* second node of pair to exchange */
    NODE * next = second->forw;		/* successor of second node */

    prev->forw = second;		/* second will now be first */
    second->back = prev;

    first->forw = next;			/* successor of old first is now
					** 'next'
					*/
    next->back=first;			/* 'next's' predecessor now 'first' */

    second->forw = first;
    first->back = second;		/* re-link connection of node pair */

    return;
}
/* isnumlit -- is operand a numeric literal
**
** This routine tests operand strings for the form
**
**	$[-]n
**
** where n is a positive integer.
*/

boolean
isnumlit(s)
char * s;				/* pointer to operand string */
{
    if (s == NULL || *s++ != '$')
	return(false);
    
    if (*s == '-')			/* sign okay */
	s++;
    
    while (*s != '\0')			/* check for all digits */
	if ( ! isdigit(*s++) )
	    return(false);
    
    return(true);			/* passed all tests */
}
/* usesvar -- does first operand use second?
**
** This routine is a generalization of "usesreg":  it returns true
** if the first operand uses the second.  An example is
** v1 = "*p", v2 = "p".  v1 uses the contents of v2.
*/

boolean
usesvar(v1,v2)
char * v1;				/* using operand */
char * v2;				/* used operand */
{
    if (v1 == NULL || v2 == NULL)
	return(false);
    if (isreg(v2) && usesreg(v1,v2))
	return(true);			/* handle register case specially */

    if (*v1 == '*')			/* check for initial indirect */
	v1++;

    return(strcmp(v1,v2) == 0);		/* result is result of compare */
}


/* These routines try to preserve the line number information.
   The criterion for saving or discarding line number information is
   that the state of the machine from a C point of view should be the
   same at any given line both before and after the optimization.
   If it would not be then the line number is deleted.
   The exceptions to the above are that the state of the condition codes 
   and of dead registers is ignored. 
   
	May 1983 */

/* NOTE: All instruction nodes that these routines assume will be tossed
	  have their uniqids set to IDVAL.  For example, in the merges,
	  the uniqids of the original instructions are set to IDVAL
	  and then the uniqid of the resultant instruction is computed.
	  This has the effect of setting the uniqid of the instruction 
	  not corresponding to the resultant to IDVAL. This fact is used 
	  in some instances where lnmrginst2 is called -- see w2opt.c */

/* delete instruction -- save line number
	if the instruction below has no line number or has a line number
	greater than the present instruction, the present number 
	is transfered down */

void
ldelin(nodep)
NODE * nodep;
{
	if(nodep->uniqid == IDVAL)
		return;

	if(nodep->forw != &ntail && 
	   (nodep->forw->uniqid > nodep->uniqid 
		 || nodep->forw->uniqid == IDVAL))
		nodep->forw->uniqid = nodep->uniqid;

	nodep->uniqid = IDVAL;

}

/* more conservative version of lndelinst that does not overwrite
	line numbers below */
void
ldelin2(nodep)
NODE *nodep;
{
	if(nodep == IDVAL)
		return;

	if(nodep->forw != &ntail && nodep->forw->uniqid == IDVAL)
		nodep->forw->uniqid = nodep->uniqid;

	nodep->uniqid = IDVAL;

}


/* exchange instructions
	The line number of the first instruction, if any, is given to the
	second instruction.  The line number of the second instruction is 
	deleted. */
void
lexchin(nodep1,nodep2)
NODE *nodep1, *nodep2;
{
	if(nodep1->uniqid != IDVAL) {
		nodep2->uniqid = nodep1->uniqid;
		nodep1->uniqid = IDVAL;
	}
	else {
		nodep2->uniqid = IDVAL;
	}
}


/* merge instructions - case 1
	The result instruction is given the line number of the
	top instruction, if it exists, or else it is given the line
	number of the bottom instruction. */
void
lmrgin1(nodep1,nodep2,resnode)
NODE *nodep1, *nodep2, *resnode;
{
	IDTYPE val1, val2;

	val1 = nodep1->uniqid;
	val2 = nodep2->uniqid;

	nodep1->uniqid = nodep2->uniqid = IDVAL;

	if(val1 != IDVAL)
		resnode->uniqid = val1;
	else 
		resnode->uniqid = val2;

}


/* instruction merge - case 2
	The resultant instruction is given the line number of the
	top instruction, if it exists, else it is not given a line number */
/*
void
lmrgin2(nodep1,nodep2,resnode)
NODE *nodep1, *nodep2, *resnode;
{
	IDTYPE val1;

	val1 = nodep1->uniqid;

	nodep1->uniqid = nodep2->uniqid = IDVAL;

	resnode->uniqid = val1;

}
*/

/* instruction merge - case 3
	The resultant instruction is given the line number of the first
	instruction, if it exists. The instruction below the pair to be 
	merged is given the line number of the second instruction, 
	if it exists */
void
lmrgin3(nodep1,nodep2,resnode)
NODE *nodep1, *nodep2, *resnode;
{
	IDTYPE val1;

	ldelin2(nodep2);

	val1 = nodep1->uniqid;

	nodep1->uniqid = nodep2->uniqid = IDVAL;

	resnode->uniqid = val1;
}

/* isiros  --  is operand either immediate, register or on stack?
**
** This routine returns true if the the operand 
** pointed to by its argument has address mode
**
**		immediate,
**		register, or
**		offset from a stack pointer (sp).
**
** This test is provided for checking operands to see whether they
** can be memory mapped i/o references.
*/

static boolean
isiros( op )
char *op;
{
	return( op != NULL &&
		(
		  *op == '$' ||
		  *op == '%' ||
		  ( *op != '*' && usesreg( op, "%esp" ) ) ||
		  ( *op != '*' && usesreg( op, "%ebp" ) )
		)
	      );
}

/* Check if the operand of an instruction node is volatile. */
	boolean
isvolatile(node,opno)
NODE *node;
int opno;
{
extern enum CC_Mode ccmode;
if (is_vol_opnd(node,opno))
	return true;		/* operand is marked volatile */
else if (ccmode == Transition && !isiros(node->ops[opno]))
	return true;		/* transition mode: any static duration */
				/* object is treated as volatile */
else return false;
}

	void
revbr(p) NODE *p; { /* reverse jump in node p */

	switch (p->op) {
	    case JZ: p->op = JNZ; p->opcode = "jnz"; break;
	    case JNZ: p->op = JZ; p->opcode = "jz"; break;
	    case JS: p->op = JNS; p->opcode = "jns"; break;
	    case JNS: p->op = JS; p->opcode = "js"; break;
	    case JP: p->op = JNP; p->opcode = "jnp"; break;
	    case JNP: p->op = JP; p->opcode = "jp"; break;
	    case JO: p->op = JNO; p->opcode = "jno"; break;
	    case JNO: p->op = JO; p->opcode = "jo"; break;
	    case JPO: p->op = JPE; p->opcode = "jpe"; break;
	    case JPE: p->op = JPO; p->opcode = "jpo"; break;
	    case JE: p->op = JNE; p->opcode = "jne"; break;
	    case JNE: p->op = JE; p->opcode = "je"; break;
	    case JL: p->op = JGE; p->opcode = "jge"; break;
	    case JLE: p->op = JG; p->opcode = "jg"; break;
	    case JG: p->op = JLE; p->opcode = "jle"; break;
	    case JGE: p->op = JL; p->opcode = "jl"; break;
	    case JB: p->op = JNB; p->opcode = "jnb"; break;
	    case JBE: p->op = JNBE; p->opcode = "jnbe"; break;
	    case JNB: p->op = JB; p->opcode = "jb"; break;
	    case JNBE: p->op = JBE; p->opcode = "jbe"; break;
	    case JA: p->op = JNA; p->opcode = "jna"; break;
	    case JAE: p->op = JNAE; p->opcode = "jnae"; break;
	    case JNA: p->op = JA; p->opcode = "ja"; break;
	    case JNAE: p->op = JAE; p->opcode = "jae"; break;
	    case JC: p->op = JNC; p->opcode = "jnc"; break;
	    case JNC: p->op = JC; p->opcode = "jc"; break;
	    }
	}


	char *
dst(p) NODE *p; { /* return pointer to dst operand string */

  static char	*nullstr = "";
	switch (p->op) {
	    case AAA: case AAD: case AAM: case AAS:
	    case CBTW: case CLTD: case CWTD: case CWTL:
	    case DAA: case DAS: case LAHF:
	    case DIVB: case IDIVB: case IMULB: case MULB:
	    case INB: case INW: case INL:
		return("%eax");
	    case DIVW: case DIVL: case IDIVW: case IDIVL:
	    case IMULW: case IMULL: case MULW: case MULL:
		if ( (p->op == IMULW || p->op == IMULL) && p->op3 != NULL )
			return (p->op3);
		if (p->op2 != NULL)
			return(p->op2);
		return("%eax");
	    case FABS: case FADD: case FADDL: case FADDP:
	    case FADDS: case FCHS: case FDIV: case FDIVL:
	    case FDIVP: case FDIVR: case FDIVRL: case FDIVRP:
	    case FDIVRS: case FDIVS: case FIADD: case FIADDL:
	    case FIDIV: case FIDIVL: case FIDIVR: case FIDIVRL:
	    case FILD: case FILDL: case FILDLL: case FIMUL:
	    case FIMULL: case FISUB: case FISUBL: case FISUBR:
	    case FISUBRL: case FLD: case FLD1: case FLDL:
	    case FLDL2E: case FLDL2T: case FLDLG2: case FLDLN2:
	    case FLDPI: case FLDS: case FLDT: case FLDZ:
	    case FMUL: case FMULL: case FMULP: case FMULS:
	    case FSUB: case FSUBL: case FSUBP: case FSUBR:
	    case FSUBRL: case FSUBRP: case FSUBRS: case FSUBS:
		if (p->op2 != NULL && strcmp(p->op2, "%st") != 0)
			return(p->op2);
		return("%st(0)");
	    case CLRB: case CLRL: case CLRW:
	    case DECB: case DECL: case DECW:
	    case FIST: case FISTL: case FISTP: case FISTPL:
	    case FISTPLL: case FST: case FSTL: case FSTP:
	    case FSTPL: case FSTPS: case FSTPT: case FSTS:
	    case INCB: case INCL: case INCW:
	    case NEGB: case NEGL: case NEGW:
	    case NOTB: case NOTL: case NOTW:
	    case POPL: case POPW:
	    case FSTSW:
		return (p->op1);
	    case ADCB: case ADCL: case ADCW:
	    case ADDB: case ADDL: case ADDW:
	    case ANDB: case ANDL: case ANDW:
	    case LEAL: case LEAW:
	    case MOVB: case MOVL: case MOVSBL:
	    case MOVSBW: case MOVSWL: case MOVW:
	    case MOVZBL: case MOVZBW: case MOVZWL:
	    case ORB: case ORL: case ORW:
	    case RCLB: case RCLL: case RCLW:
	    case RCRB: case RCRL: case RCRW:
	    case ROLB: case ROLL: case ROLW:
	    case RORB: case RORL: case RORW:
	    case SALB: case SALL: case SALW:
	    case SARB: case SARL: case SARW:
	    case SBBB: case SBBL: case SBBW:
	    case SHLB: case SHLL: case SHLW:
	    case SHRB: case SHRL: case SHRW:
	    case SUBB: case SUBL: case SUBW:
	    case XCHGB: case XCHGL: case XCHGW:
	    case XORB: case XORL: case XORW:
		return (p->op2);
	    default:
		return (nullstr);
	}
}


	int
stype(cop) int cop; { /* return implied type of op code */

	switch (cop) {
	    case ADCB: case ADDB: case ANDB:
	    case CLRB: case CMPB: case DECB:
	    case DIVB: case IDIVB: case IMULB:
	    case INB: case INCB: case MOVB:
	    case MULB: case NEGB: case NOTB:
	    case ORB: case OUTB: case RCLB:
	    case RCRB: case ROLB: case RORB:
	    case SALB: case SARB: case SBBB:
	    case SCAB: case SCMPB: case SHLB:
	    case SHRB: case SLODB: case SMOVB:
	    case SSTOB: case SUBB: case TESTB:
	    case XCHGB: case XORB:
		return (1);
	    case ADCW: case ADDW: case ANDW:
	    case CBTW: case CLRW: case CMPW:
	    case DECW: case DIVW: case IDIVW:
	    case IMULW: case INCW: case INW:
	    case LEAW: case LMSW: case MOVSBW:
	    case MOVW: case MOVZBW: case MULW:
	    case NEGW: case NOTW: case ORW:
	    case OUTW: case POPW: case PUSHW:
	    case RCLW: case RCRW: case ROLW:
	    case RORW: case SALW: case SARW:
	    case SBBW: case SCAW: case SCMPW:
	    case SHLDW: case SHLW: case SHRDW:
	    case SHRW: case SLODW: case SMOVW:
	    case SMSW: case SSTOW: case SUBW:
	    case TESTW: case XCHGW: case XORW:
		return (2);
	    default:
		return (4);
	    case FSUB: case FISUB: case FSTSW:
	    case FSTCW: case FNSTSW: case FNSTCW:
	    case FLDCW: case FSUBRL: case FSUBL:
	    case FSTPL: case FSTL: case FMULL:
	    case FMUL: case FLDL: case FISUBRL:
	    case FISUBL: case FISTPLL: case FIMULL:
	    case FIMUL: case FILDLL: case FILDL:
	    case FIDIVRL: case FIDIVL: case FICOMPL:
	    case FICOML: case FIADDL: case FDIVRL:
	    case FDIVL: case FCOMPL: case FCOML:
	    case FADDL: case F2XM1: case FABS:
	    case FADD: case FADDP: case FADDS:
	    case FBLD: case FBSTP: case FCHS:
	    case FCLEX: case FCOM: case FCOMP:
	    case FCOMPP: case FCOMPS: case FCOMS:
	    case FDECSTP: case FDIV: case FDIVP:
	    case FDIVR: case FDIVRP: case FDIVRS:
	    case FDIVS: case FFREE: case FIADD:
	    case FICOM: case FICOMP: case FIDIV:
	    case FIDIVR: case FILD: case FINCSTP:
	    case FINIT: case FIST: case FISTP:
	    case FISUBR: case FLD: case FLD1:
	    case FLDENV: case FLDL2E: case FLDL2T:
	    case FLDLG2: case FLDLN2: case FLDPI:
	    case FLDS: case FLDT: case FLDZ:
	    case FMULP: case FMULS: case FNCLEX:
	    case FNINIT: case FNOP: case FNSAVE:
	    case FNSTENV: case FPATAN: case FPREM:
	    case FPTAN: case FRNDINT: case FRSTOR:
	    case FSAVE: case FSCALE: case FSETPM:
	    case FSQRT: case FST: case FSTENV:
	    case FSTP: case FSTPS: case FSTPT:
	    case FSTS: case FSUBP: case FSUBR:
	    case FSUBRP: case FSUBRS: case FSUBS:
	    case FTST: case FWAIT: case FXAM:
	    case FXCH: case FXTRACT:
	    case FYL2X: case FYL2XP1:
		return(8);
	    }
	}

	boolean
samereg(cp1,cp2) register char *cp1, *cp2; { /* return true if same register */

	if (cp1 == NULL || cp2 == NULL || *cp1 != '%' || *cp2 != '%')
		return(false);
	if (scanreg(cp1,false) & scanreg(cp2,false))
		return(true);
	return(false);
}

	boolean
usesreg(cp1,cp2) register char *cp1, *cp2; { /*return true if cp2 used in cp1*/

	while(*cp1 != '\0') {
		if (samereg(cp2,cp1) == true)
			return(true);
		cp1++;
		}
	return(false);
	}


	int
uses(p) NODE *p; { /* set register use bits */

  	extern int rvregs;
	unsigned using;

	using = scanreg(p->op1,true) | scanreg(p->op2,true);
	switch (p->op) {
	case ROLL:  case ROLW:  case ROLB:
	case RORL:  case RORW:  case RORB:
	case SALL:  case SALW:  case SALB:
	case SARL:  case SARW:  case SARB:
	case SHLL:  case SHLW:  case SHLB:
	case SHRL:  case SHRW:  case SHRB:
	case XCHGL: case XCHGW: case XCHGB:
	case ADDL:  case ADDW:  case ADDB:
	case ANDL:  case ANDW:  case ANDB:
	case ORL:   case ORW:   case ORB:
	case SUBL:  case SUBW:  case SUBB:
	case CMPL:  case CMPW:  case CMPB:
		using |= (scanreg(p->op1,false) | scanreg(p->op2,false));
		break;
	case ADCL:  case ADCW:  case ADCB:
	case RCLL:  case RCLW:  case RCLB:
	case RCRL:  case RCRW:  case RCRB:
	case SBBL:  case SBBW:  case SBBB:
		using |= (scanreg(p->op1,false) |
				scanreg(p->op2,false) | CONCODES);
		break;
	case MULB:  case IMULB:
		using |= EAX | scanreg(p->op1,false);
		break;
	case MULW:  case MULL:
	case IMULW: case IMULL:
		using |= scanreg(p->op1,false);
		if (p->op2 == NULL)
			using |= EAX|EDX;
		else
			using |= scanreg(p->op2,false);
		if ( (p->op == IMULW || p->op == IMULL) && p->op3 != NULL)
			using |= scanreg(p->op3,true);
		break;
	case DIVB:
	case IDIVB:
		using |= EAX | scanreg(p->op1,false);
		break;
	case DIVW:  case DIVL:
	case IDIVW: case IDIVL:
		using |= EAX|EDX | scanreg(p->op1,false);
		break;
	case DECL:  case DECW:  case DECB:
	case INCL:  case INCW:  case INCB:
	case NEGL:  case NEGW:  case NEGB:
	case NOTL:  case NOTW:  case NOTB:
	case MOVSBL:  case MOVSBW:  case MOVSWL:
	case MOVZBL:  case MOVZBW:  case MOVZWL:
	case MOVL:  case MOVW:  case MOVB:
	case LEAL:  case LEAW:
		using |= scanreg(p->op1,false);
		break;
	case PUSHL: case PUSHW:
		using |= scanreg(p->op1,false) | ESP;
		break;
	case PUSHA:
		using = ESP | EAX | EBX | ECX | EDX | EDI | ESI | EBP;
		break;
	case POPL: case POPW: case POPA:
		using |= ESP;
		break;
	case SAHF:
		using = EAX;
		break;
	case LCALL: case CALL:
		using |= ESP;
		break;
	case LDS:   case LES:
	case JMP:   case LJMP:
		break;
	case AAD:  case AAS: case AAM:
 	case CBTW:  case CWTL:
	case CWTD:  case CLTD:
		using = EAX;
		break;
	case AAA:  case DAA:  case DAS:
		using = EAX | CONCODES;
		break;
	case XLAT:
		using = EBX | EAX;
		break;
	case LOOP: case JCXZ:
		using = ECX;
		break;
	case LOOPE:  case LOOPNE:  case LOOPNZ:
	case LOOPZ:  case REP:  case REPNZ:  case REPZ:
		using = ECX;
		break;
	case SCAB:  case SCAW:  case SCAL:
		using = EDI | EAX;
		break;
	case SSTOB: case SSTOW: case SSTOL:
		using = EDI;
		break;
	case SLODB:  case SLODW:  case SLODL:
		using = ESI;
		break;
	case SCMPL:  case SCMPW:  case SCMPB:
	case SMOVL:  case SMOVW:  case SMOVB:
		using = ESI | EDI;
		break;
	case ENTER:
		using = ESP | EBP;
		break;
	case LEAVE:
		using = EBP;
		break;
	case OUTB:  case OUTW:  case OUTL:
		using |= EAX;
		if (p->op1 == NULL)
			using |= EDX;
		break;
	case INB:  case INW:  case INL:
		if (p->op1 == NULL)
			using |= EDX;
		break;
	case XORL:  case XORW:  case XORB:
		/* if xor %ax,%ax (say) then value in %ax is irrelevent */ 
		if(strcmp(p->op1,p->op2) != 0)
			using |= scanreg(p->op2,false) | scanreg(p->op1,false);;
		break;
	case TESTL:  case TESTW:  case TESTB:
		using |= scanreg(p->op1,false);
		if (p->op2 != NULL)
			using |= scanreg(p->op2,false);
		break;

	/*
	 * The following instructions are for the iAPX287 math
	 * co-processor. These instructions can use the iAPX286
	 * registers for memory reference. Therefore return 'usage'.
	 */
	case FIADD:	case FIADDL:	case FICOM:	case FICOML:
	case FICOMP:	case FICOMPL:	case FIDIV:	case FIDIVL:
	case FIDIVR:	case FIDIVRL:	case FILD:	case FILDL:
	case FILDLL:	case FIMUL:	case FIMULL:	case FIST:
	case FISTL:	case FISTP:	case FISTPL:	case FISTPLL:
	case FISUB:	case FISUBL:	case FISUBR:	case FISUBRL:
	case FLDCW:	case FSTCW:	case FNSTCW:	case FSTSW:
	case FNSTSW:	case FSTENV:	case FNSTENV:	case FLDENV:
	case FSAVE:	case FNSAVE:	case FRSTOR:
	case FBLD:	case FBSTP:
	case FADDS:	case FADDL:	case FCOMS:	case FCOML:
	case FCOMPS:	case FCOMPL:	case FDIVS:	case FDIVL:
	case FDIVRS:	case FDIVRL:	case FLDS:	case FLDL:
	case FLDT:	case FMULS:	case FMULL:	case FSTS:
	case FSTL:	case FSTPS:	case FSTPL:	case FSTPT:
	case FSUBS:	case FSUBL:	case FSUBRS:	case FSUBRL:
		using |= scanreg(p->op1,false) | FP0;
		break;
	/*
	 * The instructions beginning with an F are for the iAPX287
	 * math co-processor. These instructions make no use of
	 * iAPX386 registers. These instructions all use the
	 * iAPX287 stack. (These are the fp register direct insts.)
	 */
	case FADD:	case FADDP:	case FCOM:
	case FCOMP:	case FDIV:	case FDIVP:
	case FDIVR:	case FDIVRP:	case FFREE:
	case FLD:	case FMUL:	case FMULP:
	case FST:	case FSTP:	case FSUB:
	case FSUBP:	case FSUBR:	case FSUBRP:
	case FXCH:
		using |= scanreg(p->op1,false) | scanreg(p->op2,false);
		break;
	
	case ARPL:   case BOUND:  case CLC:   case CLD:     case CLI:   
	case CLRL:   case CLRW:   case CLRB:
	case CMC:    case CTS:    case ESC:   
	/*
	 * The instructions beginning with an F are for the iAPX287
	 * math co-processor. These instructions make no use of
	 * iAPX286 registers. The ones up to the next comment take
	 * no operands.
	 */

	case FINIT:	case FCLEX:	case F2XM1:	case FABS:
	case FCHS:	case FCOMPP:	case FDECSTP:	case FINCSTP:
	case FLD1:	case FLDL2E:	case FLDL2T:	case FLDLG2:
	case FLDLN2:	case FLDPI:	case FLDZ:	case FNCLEX:
	case FNINIT:	case FNOP:	case FPATAN:	case FPREM:
	case FPTAN:	case FRNDINT:	case FSCALE:	case FSETPM:
	case FSQRT:	case FTST:	case FWAIT:	case FXAM:
	case FXTRACT:	case FYL2X:	case FYL2XP1:
	case HLT:    case INS:    case INT:   case INTO:    case IRET:   
	case LAR:    case LGDT:   case LIDT:  case LLDT:    case LMSW:   
	case LOCK:   case LRET:   case LSL:   case LTR:     case OUTS:   
	case WAIT:   case SGDT:   case SIDT:  case SLDT:    case SMSW:   
	case STC:    case STD:    case STI:   case STR:     case VERR:   
		using = 0;
		break ;
	case JA:     case JAE:    case JB:    case JBE:     case JC:
	case JE:     case JG:     case JGE:   case JL:      case JLE:   
	case JNA:    case JNAE:   case JNB:   case JNBE:    case JNE:   
	case JNG:    case JNGE:   case JNL:   case JNLE:    case JNO:   
	case JNP:    case JNS:    case JNZ:   case JO:      case JP:   
	case JPE:    case JPO:    case JS:    case JZ:      case JNC:
	case LAHF:
		using |= CONCODES;
		break;
	case GHOST:/*case TAIL:*/ case MISC:  case FILTER:  
	case LABEL:  case HLABEL: case DHLABEL:
		using = 0;
		break;
	case RET:
#ifdef pre50
		using = REGS & ~(EDX | ECX | FP1 | EBX | ESI | EDI | CONCODES);
#else
		if (rvregs)
		  using = REGS & ~(EDX | ECX | FP1 | EBX | ESI | EDI | CONCODES);
		else
		  using = REGS & ~(EAX | EDX | ECX | FP1 | EBX | ESI | EDI | CONCODES);
#endif
		break;
	default:
		using = REGS;
	}
	return(using);
}

/* returned value is a bitvector. Each bit corresponds to a register name, for
   each of %eax, %ebc, %ecx, and %edx there are three bits. For example %eax:
   %ah, %al, %Eax. If scanreg finds %ah it sets on AH, if scanreg finds
   %al it sets on AL, if scanreg finds %ax or %eax it sets on Eax|AH|AL
*/
	int
scanreg(cp,flag) register char *cp; int flag; { /* determine registers 
						   referenced in operand */

	int set;
					/*	operand	    flag	*/
	if (cp == NULL)			/*		true	false	*/
		return (0);		/*				*/
	if (flag && *cp == '%')		/*	%r	dead	live	*/
		return (0);		/*	n(%r)	live	live	*/
	set = 0;			/*	addr	dead	dead	*/
	while (*cp != '\0') {		/*	'\0'	dead	dead	*/
		if (*cp == '%') {
			if (*++cp == 'e') {
				cp++;
			}
			switch (*cp++) {
			    case 'a':
				if (*cp == 'x')
					set |= EAX;
				else if (*cp == 'l')
					set |= AL;
				else
					set |= AH;
				break;
			    case 'c':
				if (*cp == 'x')
					set |= ECX;
				else if (*cp == 'l')
					set |= CL;
				else
					set |= CH;
				break;
			    case 'b':
				if (*cp == 'p')
				    set |= EBP;
				else if (*cp == 'x')
					set |= EBX;
				else if (*cp == 'l')
					set |= BL;
				else
					set |= BH;
				break;
			    case 'd':
				if (*cp == 'i')
				    set |= EDI;
				else if (*cp == 'x')
					set |= EDX;
				else if (*cp == 'l')
					set |= DL;
				else
					set |= DH;
				break;
			    case 's':
				if (*cp == 'p')
				    set |= ESP;
				else if (*cp == 'i')
				    set |= ESI;
				else {			/* st(x) */
				    if (cp[1] != '(')
					set |= FP0;
				    else {
					cp += 2;
					switch (*cp) {
					case '0':
					    set |= FP0;
					    break;
					case '1':
					    set |= FP1;
					    break;
					default:
					    set |= 0x100 << *cp - '2';
					    break;
					}
					cp++;
				    }
				}
				break;
			    }
			}
		cp++;
		}
	return(set);
	}


	unsigned int
sets(p) NODE *p; { /* set register destination bits */

  	extern int rvregs;
	char *cp;

	switch (p->op) {
	case LAHF:  case CBTW:  case CWTL:
	case INB:  case XLAT:
	case INL:   case INW:
		return(EAX);
	case AAA:  case AAD:  case AAM:  case AAS:
	case DAA:  case DAS: 
	case DIVB:  case IDIVB:  case IMULB:
	case MULB:
		return(EAX | CONCODES);
	case IMULW:  case IMULL:
		if (p->op3 != NULL)
			return( setreg(p->op3) | CONCODES);
		else if(p->op2 != NULL)
			return( setreg(p->op2) | CONCODES);
		else
			return(EAX | EDX | CONCODES);
	case DIVL:  case IDIVL:
	case DIVW:  case IDIVW:
	case MULL:  case MULW:
		return(EAX | EDX | CONCODES);
	case SLODB:  case SLODW:  case SLODL:
		return(EAX | ESI);
	case LOOP:  case LOOPE:  case LOOPNE:  case LOOPNZ:
	case LOOPZ: case REP:  case REPNZ:  case REPZ:
		return(ECX);
	case SCAL:  case SCAW:  case SCAB:
		return(CONCODES | EDI);
	case CWTD:  case CLTD:
		return(EAX | EDX);
	case XCHGL: case XCHGW: case XCHGB:
		return( setreg(p->op1) | setreg(p->op2) );
	case SCMPL:  case SCMPW:  case SCMPB:
		return(CONCODES | ESI | EDI);
	case SMOVL:  case SMOVW:  case SMOVB:
		return(ESI | EDI);
	case SSTOL:  case SSTOW:  case SSTOB:
		return(EDI);
	case ENTER:  case LEAVE: 
		return( ESP | EBP);
	case RET: /* maybe other 387 registers */
#ifdef pre50
		return (EDX | ECX | FP1 | EBX | ESI | EDI | CONCODES);
#else
		if (rvregs)
		  return (EDX | ECX | FP1 | EBX | ESI | EDI | CONCODES);
		else
		  return (EAX | EDX | ECX | FP1 | EBX | ESI | EDI | CONCODES);
#endif
	case CALL:   case LCALL:
		return( EAX | ECX | EDX | FP0 | CONCODES ) ;
	case CMPL:   case CMPW:   case CMPB:
	case TESTL:  case TESTW:  case TESTB:
	case SAHF:
		return(CONCODES);
	case INCB:  case INCW:  case INCL:
	case DECB:  case DECW:  case DECL:
	case NEGB:  case NEGW:  case NEGL:
		return( CONCODES | setreg(p->op1) );
	case ADDB:  case ADDW:  case ADDL:
	case ADCB:  case ADCW:  case ADCL:
	case SUBB:  case SUBW:  case SUBL:
	case SBBB:  case SBBW:  case SBBL:
	case ANDB:  case ANDW:  case ANDL:
	case XORB:  case XORW:  case XORL:
	case ORB:   case ORW:   case ORL:
	case RCLL:  case RCLW:  case RCLB:
	case RCRL:  case RCRW:  case RCRB:
	case ROLL:  case ROLW:  case ROLB:
	case RORL:  case RORW:  case RORB:
	case SALL:  case SALW:  case SALB:
	case SARL:  case SARW:  case SARB:
	case SHLL:  case SHLW:  case SHLB:
	case SHRL:  case SHRW:  case SHRB:
		return( CONCODES | setreg(p->op2) );
	case MOVB:  case MOVW:  case MOVL:
	case MOVSBL: case MOVSBW: case MOVSWL:
	case MOVZBL: case MOVZBW: case MOVZWL:
		return( setreg(p->op2) );
	case PUSHW: case PUSHL: case PUSHA:
		return( ESP );
	case POPW:  case POPL:
		return( ESP | setreg(dst(p)) );
	case POPA:
		return( ESP | EDI | ESI | EBP | EBX | EDX | ECX | EAX );
	default:
		if ( (cp = dst(p)) == NULL )
			return(0);
		else
			return( setreg(cp) );
	}
}

	boolean
isdead(cp,p) char *cp; NODE *p; { /* true iff *cp is dead after p */

	int reg;

	if (cp == NULL || *cp != '%')
		return(false);
	reg = scanreg(cp, false);
	if ((p->nlive & reg) == 0)
		return(true);
	    else
		return(false);
	}


	char *
getp(p) NODE *p; { /* return pointer to jump destination operand */

	switch (p->op) {
	    default:
		return(p->op1);
	    case RET:  case LRET: case LJMP:
	        return(NULL);
	    }
	}


	char *
newlab() { /* generate a new label */

	static int lbn = 0;
	char *c;

	c = getspace((unsigned int)(lbn < 100 ? 6 : (lbn < 1000 ? 7 : 8)));
	sprintf(c, "..%d", lbn);
	lbn++;
	return(c);
	}


#include "debug.h" 	/* for line info routines */

     	void
prinst(p) register NODE *p; { /* print instruction */
	if (p->uniqid != IDVAL)
		print_line_info((int)p->uniqid);
#ifdef	IMPIL
	if (p->op == CALL ) {
	  	/* For inline expansion, we provide a tag line before a */
	  	/* call instruction, naming the function and the number of */
	  	/* bytes of argument */
		printf("@	%d	%s\n", (int)p->opm, p->op1);
		p->opm = NULL;
	}
#endif /* IMPIL */
	switch (p->op) {
		case LABEL:
			printf("%s:\n", p->opcode);
			break;
		case ASMS:
		case HLABEL:
		case DHLABEL:
			printf("%s\n", p->opcode);
			break;
		case MISC:
			printf("	%s\n", p->opcode);
			break;
		case JMP:
		case LJMP:
		case RET:
			printf("	%s	", p->opcode);
			if (p->op1 != NULL)
				printf("%s", p->op1);
			printf("\n	.align	4\n");
			break;
		default:
			printf("	%s	", p->opcode);
			if (p->op1 != NULL)
				printf("%s", p->op1);
			if (p->op2 != NULL)
				printf(",%s", p->op2);
			if (p->op3 != NULL)
				printf(",%s", p->op3);
			if (p->op4 != NULL)
				printf(",%s", p->op4);
			printf("\n");
			break;
	}
}

	boolean
ishlp(p) register NODE *p; { /* return true if a fixed label present */

	for (; islabel(p); p=p->forw)
		if (ishl(p))
			return(true);
	return(false);
	}

#include <values.h>

/* lists the displacements into the assembly of a a function that contains
   the enter leave optimization
*/
struct enter_leave {
	long start;	/* byte count of beginning of function */
	long stop;	/* byte count of end of function */
	struct enter_leave *next;
};

static struct enter_leave dummy;

/* these point to first and last entries which list the functions changed by
   enter leave 
*/
static struct enter_leave *enter_leave_last = &dummy;
static struct enter_leave *enter_leave_first = &dummy;


/* adds a function to the enter_leave list */
	void
add_enter_leave(start,stop) long start, stop;
{
	struct enter_leave *new = 
	    (struct enter_leave *) xalloc(sizeof(struct enter_leave));
	new->start = start;
	new->stop = stop;
	new->next = 0;
	enter_leave_last->next = new;
	enter_leave_last = new;
}

/* gets the next entry in the enter leave list */
	void
get_enter_leave(start,stop) long *start, *stop;
{
	enter_leave_first = enter_leave_first->next;
	if (!enter_leave_first) {
		*start = MAXLONG;
		*stop = MAXLONG;
	}
	else {
		*start = enter_leave_first->start;
		*stop = enter_leave_first->stop;
	}
	
}

#ifdef DEBUG
#include <stdio.h>
dprtext(first,cnt) NODE *first; int cnt;
{
    static void plive();
    register NODE * p;
	FILE save;
	save = *stdout;
	*stdout = *stderr;

    if (!first)
	first = n0.forw;

	fprintf(stderr,"\n\n");

    for (p = first; p != &ntail; p = p->forw) {
	plive(p->nlive,0);
	prinst(p);
	--cnt;
	if (cnt == 0)
		break;
    }
	*stdout = save;
	fprintf(stderr,"\n\n");
	fflush(stderr);
    return;
}

	static void
plive(vector,flush) unsigned int vector; int flush;
{
	if (vector&Eax)
		fprintf(stderr,"AX ");
	else
		fprintf(stderr,"   ");
	if (vector&Edx)
		fprintf(stderr,"DX ");
	else
		fprintf(stderr,"   ");
	if (vector&Ecx)
		fprintf(stderr,"CX ");
	else
		fprintf(stderr,"   ");
	if (vector&Ebx)
		fprintf(stderr,"BX ");
	else
		fprintf(stderr,"   ");

	if (vector&AL)
		fprintf(stderr,"AL ");
	else
		fprintf(stderr,"   ");
	if (vector&DL)
		fprintf(stderr,"DL ");
	else
		fprintf(stderr,"   ");
	if (vector&CL)
		fprintf(stderr,"CL ");
	else
		fprintf(stderr,"   ");
	if (vector&BL)
		fprintf(stderr,"BL ");
	else
		fprintf(stderr,"   ");
	if (vector&AH)
		fprintf(stderr,"AH ");
	else
		fprintf(stderr,"   ");
	if (vector&DH)
		fprintf(stderr,"DH ");
	else
		fprintf(stderr,"   ");
	if (vector&CH)
		fprintf(stderr,"CH ");
	else
		fprintf(stderr,"   ");
	if (vector&BH)
		fprintf(stderr,"BH ");
	else
		fprintf(stderr,"   ");
	if (vector&ESI)
		fprintf(stderr,"SI ");
	else
		fprintf(stderr,"   ");
	if (vector&EDI)
		fprintf(stderr,"DI ");
	else
		fprintf(stderr,"   ");
	if (vector&CONCODES)
		fprintf(stderr,"CC ");
	else
		fprintf(stderr,"   ");
	fprintf(stderr,":    ");
	if (flush)
		fputc('\n', stderr);
	fflush(stderr);
}
#endif
