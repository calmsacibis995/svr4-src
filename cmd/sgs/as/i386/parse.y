/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

%{
#ident	"@(#)as:i386/parse.y	1.21"


%}
%{
%}
%{
#include <stdio.h>
#include <filehdr.h>
#include <sgs.h>
#include <ctype.h>
#include <libelf.h>
#include "symbols.h"
#include "gendefs.h"
#include "instab.h"
#include "instr.h"
#include "section.h"

extern int txtsec;
extern int bsssec;
static int datsec = -1;

#if 0 /* We may break too many old programs with this */
static void ckalign();
#else
#define ckalign(s)
#endif
static void alignmt();
static void assignmt();
static void obsolete_dir();
static void ck_nonneg_absexpr();
static symbol *get_temp_sym();

enum obsdir_warn { NOWARN, WARN };

extern void bss();
extern int previous;    /* previous section number */
extern int mksect();
extern cgsect();
extern void treeleaf();
extern void treenode();
extern rexpr *merge();
extern unsigned long Strtoul();

extern char *strtab;
#define RANGE(x)	( x >= -128L && x <= 127L )

extern char
	*filenames[];		/* contains the name of this file */
extern symbol
	*cfile;		/* contains C source name obtained from .file */

extern unsigned short
	line;	/* current line number */

extern short
	xflag,		/* indicates if x86 syntax is accepted */
	transvec,	/* indicate transfer vector program addressing */
	bitpos,	/* current bit position unused in outword */
	spanopt,	/* optimize */
	transition;

extern long
	newdot;	/* up-to-date value of "." */

extern symbol
	*dot;	/* "." */

extern FILE
	*fdin;

extern void
	deflab();

extern symbol *
	lookup();

extern unsigned short
	actreloc();

char	newname[9];	/* used to contain new instr. name */
short	nameindx,	/* index into newname array */
	dbit;	/* "d" bit in 8086 opcode */

union addrmdtag
	memloc;	/* contains addressing mode information for this instr. */
union sib_byte_tag
	memsib ;	/* contains the scale, index and base info for
			   the extended addressing modes */
instr *
	newins;	/* pointer to new instruction */

static short
	numbits,	/* total number of bits seen in bit packing exprs */
	spctype;	/* distinguishes between .byte, .half, or .word */

static long
	opcd;	/* opcode for this instruction */

static char as_version[] = "02.01";
int special287 = 0 ;

#define	EXPRERR(e,x)	{yyerror(x);\
			(e).expval = 0;\
			(e).symptr = NULL;\
			(e).exptype = ABS;\
			(e).unevaluated = 0;\
			(e).reltype = NORELTYPE;}

%}
%union {
	long uulong;
	rexpr uuexpr;
	symbol *uusymptr;
	instr *uuinsptr;
	addrmode uumode;
	char *uustrptr;
	floatval uufval;
	unsigned long uuattype;	/* '@' type */
	struct {
	  int atts;
	  unsigned long type;
	} uusect;
}

%token <uuinsptr> NO_OP 
%token <uuinsptr> BSTROP WSTROP LSTROP
%token <uuinsptr> PROT1 PROT2 PROT3
%token <uuinsptr> LSDT ARPL WBOUND LBOUND ENTER
%token <uuinsptr> FLOAT1 FLOAT2 FLOAT3 FLOAT4 FLOAT5 FLOAT6 FLOAT7 FREG
%token <uuinsptr> EXPOP1 EXPOP2 RETOP WSPOP1 LSPOP1 
%token <uuinsptr> WCLR LCLR WINCDEC LINCDEC WMULDIV LMULDIV
%token <uuinsptr> LOOP JMPOP1 JMPOP2 CALLOP LCALLOP LJMPOP
%token <uuinsptr> WMONOP1 LMONOP1 WLOGIC1 LLOGIC1 WLOGIC2 LLOGIC2 
%token <uuinsptr> BCLR BINCDEC BMONOP1 BLOGIC1 BMULDIV
%token <uuinsptr> WMOVOP WTEST LTEST WDYADIC LDYADIC WAOTOP LAOTOP WXCHG LXCHG  
%token <uuinsptr> LAOTOP1 WAOTOP1
%token <uuinsptr> BMOVOP BTEST BDYADIC BXCHG
%token <uuinsptr> PSGLOBAL PSLOCAL PSWEAK PSSET PSEVEN PSALIGN PSZERO
%token <uuinsptr> PSBSS PSFILE PSTEXT PSDATA PSIDENT PSCOMM PSLCOMM 
%token <uuinsptr> PSSECTION PSVERS PSPREV PSPUSHSECT PSPOPSECT
%token <uuinsptr> PSBYTE PSVALUE PSDEF PSVAL PSSCL PSTYPE PSSTRING
%token <uuinsptr> PS2BYTE PS4BYTE
%token <uufval> PSFLOAT PSDOUBLE PSEXT PSLONG PSBCD FNUMBER
%token <uuinsptr> PSTAG PSLINE PSSIZE PSDIM PSENDEF PSLN PSJMPTAB
%token <uulong> EXPOPT MONOPT WSPOPT LSPOPT BDYADOPT WDYADOPT LDYADOPT 
%token <uulong> INCOPT XCHGOPT JMPOPT
%token <uulong> IMMED WMOVOPT INDIRECT SEGMEM
%token <uulong> SP XCLAIM QUOTE SHARP DOLLAR AT REG AMP SQ LP RP MUL PLUS
%token <uulong> COMMA MINUS ALPH DIV DIG COLON SEMI QUEST LB RB
%token <uulong> LT GT OR NL SLASH STAR ESCAPE MOD
%token <uulong> SEGPART OFFPART HIFLOAT LOFLOAT
%token <uulong> RSHIFT LSHIFT HAT EQ PERCNT GRAVE
%token <uusymptr> ID
%token <uuinsptr> REG16 BREG16 AREG16 DREG16 IREG16 SREG
%token <uuinsptr> REG8 AREG8 CLREG8
%token <uulong> NUMBER
%token <uustrptr> STRING
%token <uulong> ERR
%token <uuattype> SECTTYPE SYMTYPE RELTYPE
%type <uuexpr> exprX expr term floatreg
%type <uumode> dualreg8 dualreg16 dual8opnd dual16opnd
%type <uumode> reg8mem reg16mem segdef segextaddr
%type <uumode> reg16 reg8
%type <uumode> immd reg16md reg8md
%type <uusymptr> dotcomm
%type <uusect> sect_atts sect_type

/* i386 10-30-85 */
%token <uuinsptr> LMOVOP SETOP
%token <uuinsptr> REG32 BREG32 AREG32 IREG32 DREG32 CTLREG DBREG TRREG
%token <uuinsptr> WBIT1 LBIT1 WBIT2 LBIT2 WBIT3 LBIT3
%token <uuinsptr> WMOVS LMOVS WMOVZ LMOVZ

/* 486 4-1-89	*/
%token <uuinsptr> BCMPXCHG WCMPXCHG LCMPXCHG BSWAP BXADD WXADD LXADD INVLD

%type <uumode> dualreg32 dual32opnd
%type <uumode> reg32 segmem32 mem32 reg32mem reg32md
%type <uumode> extaddr32md expdisp32md disp32md
%type <uumode> dispbase32 dualdispbase32
%type <uumode> dispindex32
%type <uumode> dualdispbasescl32

%%
program:	/* empty */
	|	program  linstr  endstmt = {
		  goto reset;
		}
	|	program  error  endstmt = {
		  yyerrok;
		reset:
		  dbit = DBITOFF;
		  memloc.addrmdfld = 0;
		  dot->value = newdot;		/* syncronize */
		}
	;

endstmt	:	NL = {
		  ++line;
		  generate(0,NEWSTMT,(long)line,NULLSYM);
		}
	|	SEMI = {
		  generate(0,NEWSTMT,(long)line,NULLSYM);
		}
	;

linstr :	instruction
	|	label  instruction
	;

label :		ID  COLON = {
		  if (!COMMON($1)) {
		    if (!UNDEF_SYM($1))
		      yyerror("multiply defined label");
		    /* common symbols by default should be global, */
		    /* so reset binding only for non-common symbols */
		    if (!BIT_IS_ON($1,BOUND))
		      $1->binding = STB_LOCAL;
		  }
		  if (TEMP_LABEL($1))	/* local and begins with `.' */
		      $1->flags &= ~GO_IN_SYMTAB;
		  $1->sectnum = dot->sectnum;
		  $1->value = newdot;
		  if (spanopt && (sectab[dot->sectnum].flags & SHF_EXECINSTR))
		    deflab($1);
		}
	;

 
instruction :	/* empty */
	|	NO_OP
		{
		  generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
		}
	|	BSTROP
		{
		  generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
		}
	|	BSTROP  SREG
		{
		  switch($2->opcode) {
		    case 0 :
		    case 1 :
		    case 2 :
		    case 3 :
		      generate(8, NOACTION,
		          ((long) SEGPFX | ($2->opcode << 3)), NULLSYM) ;
		      break ;
		    case 4 :
		      generate(8, NOACTION, (long) 0x64, NULLSYM) ;
		      break ;
		    case 5 :
		      generate(8, NOACTION, (long) 0x65, NULLSYM) ;
		      break ;
		    }
		  generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
		}
	|	WSTROP
		{
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
		}
	|	WSTROP  SREG
		{
		  switch($2->opcode) {
		    case 0 :
		    case 1 :
		    case 2 :
		    case 3 :
		      generate(8, NOACTION,
		          ((long) SEGPFX | ($2->opcode << 3)), NULLSYM) ;
		      break ;
		    case 4 :
		      generate(8, NOACTION, (long) 0x64, NULLSYM) ;
		      break ;
		    case 5 :
		      generate(8, NOACTION, (long) 0x65, NULLSYM) ;
		      break ;
		    }
		  generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
		}
	|	LSTROP
		{
		  generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
		}
	|	LSTROP  SREG
		{
		  switch($2->opcode) {
		    case 0 :
		    case 1 :
		    case 2 :
		    case 3 :
		      generate(8, NOACTION,
		          ((long) SEGPFX | ($2->opcode << 3)), NULLSYM) ;
		      break ;
		    case 4 :
		      generate(8, NOACTION, (long) 0x64, NULLSYM) ;
		      break ;
		    case 5 :
		      generate(8, NOACTION, (long) 0x65, NULLSYM) ;
		      break ;
		    }
		  generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
		}
	|	EXPOP1  immd
		{
		  /* this production is for interrupts only
		     int 3 (the break point interrupt) is a special
		     case 1 byte interrupt */
		  if (($2.adexpr.symptr == NULLSYM) && 
		      ($2.adexpr.expval == 3L)) {	
		    nameindx = copystr($1->name,newname);
		    newname[nameindx++] = 'I';
		    newname[nameindx] = '\0';
		    newins = instlookup(newname);
		    if (newins != NULL) {
		      generate(newins->nbits, NOACTION,
			newins->opcode,NULLSYM);
		    }
		    else 
		      yyerror("Illegal instruction");
		  }
		  else {
		    generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
		    generate(8,RESABS,$2.adexpr.expval,$2.adexpr.symptr);
		  }
		}
	|	EXPOP2  LP  DREG16  RP = {
		  if ($1->tag == INSTRW)
		    generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
		}
	|	EXPOP2  immd
		{
		  if (!ABSOLUTE($2.adexpr))
		    yyerror("invalid instruction argument");
		  if ($1->tag == INSTRW)
		    generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  nameindx = copystr($1->name,newname);
		  newname[nameindx++] = 'E';
		  newname[nameindx] = '\0';
		  newins = instlookup(newname);
		  if (newins != NULL) {
		    generate(newins->nbits, NOACTION,
		      newins->opcode,NULLSYM);
		    generate(8,NOACTION,$2.adexpr.expval,NULLSYM);
		  }
		  else 
		    yyerror("Illegal instruction");
		}
	|	RETOP
		{
		  generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
		}
	|	RETOP  immd
		{
		  nameindx = copystr($1->name,newname);
		  newname[nameindx++] = 'E';
		  newname[nameindx] = '\0';
		  newins = instlookup(newname);
		  if (newins != NULL) {
		    generate(newins->nbits,NOACTION,
		      newins->opcode,NULLSYM);
		    generate(16,actreloc($2.adexpr.reltype,SWAPB),
		      $2.adexpr.expval,$2.adexpr.symptr);
	  	  }
		  else
		    yyerror("Illegal ret");
		}
	|	WSPOP1  reg16mem
		{
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  rmongen32($1,&($2));
		}
	|	LSPOP1  reg32mem
		{
		  rmongen32($1,&($2));
		}
	|	WSPOP1 immd
		{
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  goto lspop1 ;
		}
	|	LSPOP1  SREG 
		{
		  nameindx = copystr($1->name,newname);
		  switch ($2->opcode) {
		    case 1 :
		      if ($1->name[1] == 'o')
		        yyerror("illegal use of %cs") ;
		      else
		        newname[nameindx++] = 'S';
		      break ;
		    case 0 :
		    case 2 :
		    case 3 :
		      newname[nameindx++] = 'S';
		      break ;
		    case 4 :
		    case 5 :
		      newname[nameindx++] = 'S';
		      newname[nameindx++] = '1' ;
		      break ;
		    }
		  newname[nameindx] = '\0';
		  newins = instlookup(newname);
		  opcd = newins->opcode;
		  opcd |= $2->opcode << 3;
		  generate(newins->nbits,NOACTION,opcd,NULLSYM);
		}
	|	LSPOP1 immd
		{
		long val ;
		lspop1:
		  if($1->name[1] == 'o')
		    yyerror("cannot pop to an immediate ");
		  nameindx = copystr($1->name,newname);
		  /*
		   * select one byte push immediate or 2 byte push immd 
		   */
		  if(($2.adexpr.symptr == NULLSYM ) &&
		    ((val = $2.adexpr.expval) < 128 && val > -127 )) {
		      newname[nameindx++] = 'I';
		      newname[nameindx++] = 'B';
		      newname[nameindx] = '\0';
		      newins = instlookup(newname);
		      generate(newins->nbits,NOACTION,
		        newins->opcode,NULLSYM);
  		      immedgen32(8,&($2));
		  }
		  else {
		    newname[nameindx++] = 'I';
		    newname[nameindx] = '\0';
		    newins = instlookup(newname);
		    generate(newins->nbits,NOACTION,
		      newins->opcode,NULLSYM);
		    if ($1->tag == INSTRW)
		      immedgen(16, &($2)) ;
		    else
		      immedgen32(32, &($2)) ;
	 	  }
		}
	|	WCLR  reg16mem
		{
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  goto lclr ;
		}

	|	LCLR  reg32mem 
		{
		lclr:
		  if ($2.admode & REGMASK) {
		    memloc.addrmd.reg = $2.adreg;
		    newins = instlookup(($1->tag == INSTRW)? "xorw": "xorl");
		    if (newins != NULL) {
		      generate(newins->nbits,NOACTION,
			(long)(newins->opcode | memloc.addrmdfld),NULLSYM);
		    }
		    else
		      aerror("Lost xor instr");
		  }
		  else {
		    newins = instlookup(($1->tag == INSTRW)? "movwIG": "movlIG");
		    if (newins != NULL) {
		      addrgen32(newins->opcode,&($2));
		      generate((($1->tag == INSTRW)? 16: 32),
			NOACTION,0L,NULLSYM);
		    }
		    else
		      aerror("Lost mov (immed) instr");
		  }
		}
	|	BCLR  reg8mem = {
		  if ($2.admode & REGMASK) {
		    memloc.addrmd.reg = $2.adreg;
		    newins = instlookup("xorb");
		    if (newins != NULL) {
		      generate(newins->nbits,NOACTION,
			(long)(newins->opcode | memloc.addrmdfld),NULLSYM);
		    }
		    else
		      aerror("Lost xorb instr");
		  }
		  else {
		    newins = instlookup("movbIG");
		    if (newins != NULL) {
		      addrgen32(newins->opcode,&($2));
		      generate(8,NOACTION,0L,NULLSYM);
		    }
		    else
		      aerror("Lost movb (immed) instr");
		  }
		}
	|	LINCDEC  reg32mem
		{
		  rmongen32($1, &($2));
		}
	|	WINCDEC  reg16mem
		{
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  rmongen32($1, &($2));
		}
	|	BINCDEC  reg8mem
		{
		  addrgen32($1->opcode,&($2));
		}
	|	LOOP  segextaddr = {
		  loopgen($1,&($2));
		}
	|	JMPOP1  segextaddr = {
		  jmp1opgen($1,&($2));
		}
	|	JMPOP2  segextaddr = {
		  jmp2opgen($1,&($2));
		}
	|	JMPOP2  segdef = {
		  defgen($1->name,&($2));
		}
	|	LJMPOP  segdef = {
		  defgen($1->name,&($2));
		}
	|	LJMPOP immd COMMA immd
		{
		  goto _lcallop ;
		}
	|	LCALLOP immd COMMA immd
		{
		  /* $2 is the selector, $4 is the offset */
		  _lcallop:
		  generate($1->nbits, NOACTION, $1->opcode, NULLSYM) ;
		  immedgen32(32, &($4));
		  immedgen(16, &($2));
		}
	|	CALLOP  segextaddr = {
		  generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
		  generate(32, actreloc($2.adexpr.reltype, LONGREL), $2.adexpr.expval, $2.adexpr.symptr);
		}
	|	CALLOP  segdef = {
		  defgen($1->name,&($2));
		}
	|	LCALLOP  segdef = {
		  defgen($1->name,&($2));
		}
	|	BMULDIV  reg8mem
		{
		  addrgen32($1->opcode,&($2));
		}
	|	LMULDIV  immd COMMA reg32md
		{
		    goto _muldiv1;
		}

	|	WMULDIV  immd COMMA reg16md
		{
		    int val;

		    /*
		     * generate prefix for 16 bit addressing
		     */
		    generate (8, NOACTION, (long) OSP, NULLSYM) ;

		_muldiv1:

		    nameindx = copystr($1->name,newname);
		    newname[nameindx++] = 'I';
		    newname[nameindx] = '\0';
		    newins = instlookup(newname);
		
		    /*
		     * select 1, 2 or 4 byte(s) multiply 
		     */
		    val = $1->val + (($1->val > 0) ? 256 : 512);
		    memloc.addrmd.reg = $4.adreg ;	/* destination */
		    /*
		     * if the immd operand fits in one byte, change the
		     * opcode and generate 8 bit operand
		     */
		    if(($2.adexpr.symptr == NULLSYM) &&
		        $2.adexpr.expval >= -127 && $2.adexpr.expval <= 128 ) {
		      addrgen32 (newins->opcode | 0x0200, &($4) );
		      immedgen(8,&($2));
	   	    }
		    /*
		     * else, leave the opcode, and generate 16 or 32 bit operand
		     */
		    else if (val == WMULDIV) {
		      addrgen32(newins->opcode, &($4));
		      immedgen(16,&($2));
		    }
		    else {
		      addrgen32(newins->opcode, &($4)) ;
		      immedgen32(32, &($2)) ;
		    }
		}

	|	WMULDIV  reg16mem
		{
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  addrgen32($1->opcode,&($2));
		}
	|	WMULDIV	immd COMMA reg16mem COMMA reg16 
		{ int val;
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		_muldiv:
		  if (strcmp($1->name, "imull") == 0 ||
		      strcmp($1->name, "imul") == 0 ||
		      strcmp($1->name, "imulw") == 0  ||
		      strcmp($1->name, "mull") == 0 ||
		      strcmp($1->name, "mul") == 0 ||
		      strcmp($1->name, "mulw") == 0 ) {

		    nameindx = copystr($1->name,newname);
		    newname[nameindx++] = 'I';
		    newname[nameindx] = '\0';
		    newins = instlookup(newname);
	
		    memloc.addrmd.reg = $6.adreg ;

		    /* select 1, 2 or 4 byte(s) multiply */
		    val = $1->val + (($1->val > 0) ? 256 : 512);

		    if(($2.adexpr.symptr == NULLSYM) &&
		        $2.adexpr.expval >= -127 && $2.adexpr.expval <= 128 ) {
		      addrgen32 (newins->opcode | 0x0200, &($4) );
		      immedgen(8,&($2));
	   	    }
		    else if (val == WMULDIV) {
		      addrgen32(newins->opcode, &($4));
		      immedgen(16,&($2));
		    }
		    else {
		      addrgen32(newins->opcode, &($4)) ;
		      immedgen32(32, &($2)) ;
		    }
		  }
		  else {
		    yyerror("illegal three operand multiply") ;
		  }
		}
	|	WMULDIV reg16mem COMMA reg16
		{
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		_muldiv2:

		  if (strcmp($1->name, "imull") == 0 ||
		      strcmp($1->name, "imul") == 0 ||
		      strcmp($1->name, "imulw") == 0 ) {

		    nameindx = copystr($1->name,newname);
		    newname[nameindx++] = 'R';
		    newname[nameindx] = '\0';
		    newins = instlookup(newname);
		    memloc.addrmd.reg = $4.adreg;
		    laddrgen32(newins->opcode, &($2)) ;
		  } 
		  else {
		    yyerror("illegal uncharacterized multiply") ;
		  }
		}
	|	LMULDIV  reg32mem
		{
		  addrgen32($1->opcode,&($2));
		}
	|	LMULDIV	immd COMMA reg32mem COMMA reg32 
		{
		  goto _muldiv ;
		}
	|	LMULDIV reg32mem COMMA reg32
		{
		  goto _muldiv2 ;
		}
	|	BMONOP1  reg8mem
		{
		  addrgen32($1->opcode,&($2));
		}
	|	WMONOP1  reg16mem
		{
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  addrgen32($1->opcode,&($2));
		}
	|	LMONOP1  reg32mem
		{
		  addrgen32($1->opcode,&($2));
		}
	/*
	 *	80287 - no operands
	 */
	|	FLOAT1 = {
		  generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
		}
	/*
	 *	80287 - single (memory access) operand
	 */
	|	FLOAT2 segmem32 = {
		  if (($1->opcode >> 16) == 0x9b)
		/* if wait form of instruction, generate fwait */
			generate(8, NOACTION, (long) 0x9b, NULLSYM);
		  addrgen32($1->opcode,&($2));
		}
	/*
	 *	80287 - two operands of form %st,%st(i) or %st(i),%st
	 *		destination %st	  - direction flag = 0
	 *		destination %st(i)- direction flag = 1
	 *	also:-
	 *		without any operands implies
	 *		destination %st(1) - direction flag = 1
	 */
	|	FLOAT3 = {
		  generate($1->nbits,NOACTION,$1->opcode|0x0e01,NULLSYM);
		}
	|	FLOAT3 FREG COMMA floatreg = {
		  generate($1->nbits,NOACTION,$1->opcode|$4.expval|0x0400,NULLSYM);
		}
	|	FLOAT3 floatreg COMMA FREG = {
		  generate($1->nbits,NOACTION,$1->opcode|$2.expval,NULLSYM);
		}
	/*
	 *	80287 - single operand of form %st(i) 
	 */
	|	FLOAT4 floatreg = {
		  generate($1->nbits,NOACTION,$1->opcode|$2.expval,NULLSYM);
		}
	/*
	 *	80287 - two operands %st,%st(i) only direction allowed
	 */
	|	FLOAT5 FREG COMMA floatreg = {
		  generate($1->nbits,NOACTION,$1->opcode|$4.expval,NULLSYM);
		}
	/*
	 *	80287 - stsw specials either %ax or memory access allowed
	 */
	|	FLOAT6 AREG16 = {
		  if (($1->opcode >> 16) == 0x9b)
		/* if wait form of instruction, generate fwait */
			generate(8, NOACTION, (long) 0x9b, NULLSYM);
		  generate(16,NOACTION,0xdfe0L,NULLSYM);
		}
	|	FLOAT6 segmem32 = {
		  if (($1->opcode >> 16) == 0x9b)
		/* if wait form of instruction, generate fwait */
			generate(8, NOACTION, (long) 0x9b, NULLSYM);
		  addrgen32($1->opcode,&($2));
		}
	/*
	 *	80287 special instructions - usually take just 1 parameter
	 *	  %st(i)
	 *	but can have implied default of %st(1) if no operands supplied
	 */
	|	FLOAT7 floatreg = {
		  generate($1->nbits,NOACTION,$1->opcode|$2.expval,NULLSYM);
		}
	|	FLOAT7  = {
		  generate($1->nbits,NOACTION,$1->opcode|0x0001,NULLSYM);
		}
	|	PROT3 	reg32mem
		{
		  laddrgen32($1->opcode,&($2));
		}
	|	PROT1 	reg16mem
		{
		  laddrgen32($1->opcode,&($2));
		}
	|	PROT2   dual32opnd 
		{
		  if(dbit == DBITOFF)
		    yyerror ( "Syntax error - use mem , reg");
		  laddrgen32($1->opcode | dbit,&($2));
		}
	|	PROT2 dualreg32 
		{
		  generate($1->nbits,NOACTION,
		    (long)($1->opcode | memloc.addrmdfld),NULLSYM);
		}
	|	LSDT	segmem32
		{
		  laddrgen32($1->opcode,&($2));
		}
	|	ARPL	dual16opnd
		{
		  if(dbit == DBITON)
		    yyerror ( "Syntax error - use arpl reg , mem");
		  addrgen32($1->opcode ,&($2));
		}
	|	WBOUND	dual16opnd
		{
		  if(dbit == DBITOFF)
		    yyerror ( "Syntax error - use bound mem , reg");
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  addrgen32($1->opcode, &($2));
		}
	|	LBOUND	dual32opnd
		{
		  if(dbit == DBITOFF)
		    yyerror ( "Syntax error - use bound mem , reg");
		  addrgen32($1->opcode, &($2));
		}
	|	ENTER immd COMMA immd
		{
		  generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
		  immedgen(16,&($2));
		  immedgen(8,&($4));
		}
	|	BLOGIC1  immd	COMMA  reg8mem
		{
		  if( ($2.adexpr.symptr == NULLSYM) &&
		    ($2.adexpr.expval == 1L) ) {
		    addrgen32($1->opcode,&($4));
		  }
		  else {
		    addrgen32($1->opcode & 0xc1ff ,&($4));
		    immedgen(8,&($2));
		  }
		}
	|	BLOGIC1  reg8mem
		{
		  addrgen32($1->opcode,&($2));
		}
	|	BLOGIC1  CLREG8  COMMA  reg8mem
		{
		  addrgen32($1->opcode ^ 0x0200,&($4));
		}
	|	WLOGIC1  reg16mem
		{
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  addrgen32($1->opcode,&($2));
		}
	|	WLOGIC1  CLREG8  COMMA  reg16mem
		{
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  addrgen32($1->opcode ^ 0x0200,&($4));
		}
	|	WLOGIC1  immd	COMMA  reg16mem
		{
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  if( ($2.adexpr.symptr == NULLSYM) &&
		    ($2.adexpr.expval == 1L) ) {
		    addrgen32($1->opcode,&($4));
		  }
		  else {
		    addrgen32($1->opcode & 0xc1ff ,&($4));
		    immedgen(8,&($2));
		  }
		}
	|	LLOGIC1  reg32mem
		{
		  addrgen32($1->opcode,&($2));
		}
	|	LLOGIC1  CLREG8  COMMA  reg32mem
		{
		  addrgen32($1->opcode ^ 0x0200,&($4));
		}
	|	LLOGIC1  immd	COMMA  reg32mem
		{
		  if( ($2.adexpr.symptr == NULLSYM) &&
		    ($2.adexpr.expval == 1L) ) {
		    addrgen32($1->opcode,&($4));
		  }
		  else {
		    addrgen32($1->opcode & 0xc1ff ,&($4));
		    immedgen(8,&($2));
		  }
		}
	|	BMOVOP dualreg8 
		{
		  generate($1->nbits, NOACTION,
		    (long)($1->opcode | dbit | memloc.addrmdfld), NULLSYM) ;
		}
	|	BMOVOP dual8opnd
		{
		  movgen32($1, &($2)) ;
		}
	|	BMOVOP immd COMMA reg8mem
		{
		  mov2gen32($1->name, &($4)) ;
		  immedgen(8, &($2)) ;
		}
	|	WMOVOP  dualreg16 
		{
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  generate($1->nbits,NOACTION,
		    (long)($1->opcode | dbit | memloc.addrmdfld),NULLSYM);
		}
	|	WMOVOP  dual16opnd = {
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  movgen32($1,&($2));
		}
	|	WMOVOP  immd  COMMA  reg16mem = {
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  mov2gen32($1->name,&($4));
		  immedgen(16,&($2));
		}
	|	WMOVOP  SREG  COMMA  reg16mem = {
		  nameindx = copystr($1->name,newname);
		  newname[nameindx++] = 'S';
		  newname[nameindx++] = 'G';
		  newname[nameindx] = '\0';
		  newins = instlookup(newname);
		  if (newins != NULL) {
		    memloc.addrmd.reg = $2->opcode;
		    addrgen32(newins->opcode,&($4));
		  }
		  else
		    aerror("Cannot find mov (seg) instr");
		}
	|	WMOVOP  segmem32  COMMA  SREG = {
		  goto smov;
		}
	|	WMOVOP  reg16  COMMA  SREG = {
		  memloc.addrmd.rm = $2.adreg;
		  memloc.addrmd.mod = REGMOD;
		smov:
		  nameindx = copystr($1->name,newname);
		  newname[nameindx++] = 'G';
		  newname[nameindx++] = 'S';
		  newname[nameindx] = '\0';
		  newins = instlookup(newname);
		  if (newins != NULL) {
		    memloc.addrmd.reg = $4->opcode;
		    addrgen32(newins->opcode,&($2));
		  }
		  else
		    aerror("Cannot find mov (seg) instr");
		}
	|	LMOVOP	dualreg32
		{
		  generate($1->nbits, NOACTION,
		    (long)($1->opcode | dbit | memloc.addrmdfld), NULLSYM) ;
		}
	|	LMOVOP	dual32opnd
		{
		  movgen32($1, &($2)) ;
		}
	|	LMOVOP	immd COMMA reg32mem
		{
		  mov2gen32($1, &($4)) ;
		  immedgen32(32, &($2)) ;
		}
	|	LMOVOP CTLREG COMMA reg32
		{
		  memloc.addrmd.mod = REGMOD ;
		  memloc.addrmd.reg = $2->opcode ;
		  memloc.addrmd.rm = $4.adreg ;

		  nameindx = copystr($1->name,newname);
		  newname[nameindx++] = 'C';
		  newname[nameindx] = '\0';
		  newins = instlookup(newname);
		  
		  opcd = newins->opcode | memloc.addrmdfld ;
		  generate (24, NOACTION, opcd, NULLSYM ) ;

		}
	|	LMOVOP reg32 COMMA CTLREG
		{
		  memloc.addrmd.mod = REGMOD ;
		  memloc.addrmd.reg = $4->opcode ;
		  memloc.addrmd.rm = $2.adreg ;

		  nameindx = copystr($1->name,newname);
		  newname[nameindx++] = 'C';
		  newname[nameindx] = '\0';
		  newins = instlookup(newname);
		  
		  opcd = newins->opcode | memloc.addrmdfld ;
		  opcd |= 0x0200 ;
		  generate (24, NOACTION, opcd, NULLSYM ) ;
		}
	|	LMOVOP DBREG COMMA reg32
		{
		  memloc.addrmd.mod = REGMOD ;
		  memloc.addrmd.reg = $2->opcode ;
		  memloc.addrmd.rm = $4.adreg ;

		  nameindx = copystr($1->name,newname);
		  newname[nameindx++] = 'D';
		  newname[nameindx] = '\0';
		  newins = instlookup(newname);
		  
		  opcd = newins->opcode | memloc.addrmdfld ;
		  generate (24, NOACTION, opcd, NULLSYM ) ;
		}
	|	LMOVOP reg32 COMMA DBREG
		{
		  memloc.addrmd.mod = REGMOD ;
		  memloc.addrmd.reg = $4->opcode ;
		  memloc.addrmd.rm = $2.adreg ;

		  nameindx = copystr($1->name,newname);
		  newname[nameindx++] = 'D';
		  newname[nameindx] = '\0';
		  newins = instlookup(newname);
		  
		  opcd = newins->opcode | memloc.addrmdfld ;
		  opcd |= 0x0200 ;
		  generate (24, NOACTION, opcd, NULLSYM ) ;
		}
	|	LMOVOP TRREG COMMA reg32
		{
		  memloc.addrmd.mod = REGMOD ;
		  memloc.addrmd.reg = $2->opcode ;
		  memloc.addrmd.rm = $4.adreg ;

		  nameindx = copystr($1->name,newname);
		  newname[nameindx++] = 'T';
		  newname[nameindx] = '\0';
		  newins = instlookup(newname);
		  
		  opcd = newins->opcode | memloc.addrmdfld ;
		  generate (24, NOACTION, opcd, NULLSYM ) ;
		}
	|	LMOVOP reg32 COMMA TRREG
		{
		  memloc.addrmd.mod = REGMOD ;
		  memloc.addrmd.reg = $4->opcode ;
		  memloc.addrmd.rm = $2.adreg ;

		  nameindx = copystr($1->name,newname);
		  newname[nameindx++] = 'T';
		  newname[nameindx] = '\0';
		  newins = instlookup(newname);
		  
		  opcd = newins->opcode | memloc.addrmdfld ;
		  opcd |= 0x0200 ;
		  generate (24, NOACTION, opcd, NULLSYM ) ;
		}
	|	WLOGIC2  dualreg16
		{
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  generate($1->nbits,NOACTION,
		    (long)($1->opcode | dbit | memloc.addrmdfld),NULLSYM);
		}
	|	WLOGIC2  dual16opnd
		{
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  addrgen32($1->opcode | dbit,&($2));
		}
	|	WLOGIC2  immd  COMMA  reg16mem
		{
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  dyad2gen32($1->name,&($4));
		  immedgen(16,&($2));
		}
	|	LLOGIC2  dualreg32
		{
		  generate($1->nbits,NOACTION,
		    (long)($1->opcode | dbit | memloc.addrmdfld),NULLSYM);
		}
	|	LLOGIC2  dual32opnd
		{
		  addrgen32($1->opcode | dbit,&($2));
		}
	|	LLOGIC2  immd  COMMA  reg32mem
		{
		  dyad2gen32($1->name,&($4));
		  immedgen32(32,&($2));
		}
	|	WTEST  dualreg16
		{
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  generate($1->nbits,NOACTION,
		    (long)($1->opcode | memloc.addrmdfld),NULLSYM);
		}
	|	WTEST  dual16opnd 
		{
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  addrgen32($1->opcode,&($2));
		}
	|	WTEST  immd  COMMA  reg16mem 
		{
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  testgen32($1->name,&($4));
		  immedgen(16,&($2));
		}
	|	LTEST  dualreg32 
		{
		  generate($1->nbits,NOACTION,
		    (long)($1->opcode | memloc.addrmdfld),NULLSYM);
		}
	|	LTEST  dual32opnd 
		{
		  addrgen32($1->opcode,&($2));
		}
	|	LTEST  immd  COMMA  reg32mem 
		{
		  testgen32($1->name,&($4));
		  immedgen32(32,&($2));
		}
	|	BTEST  dualreg8 
		{
		  generate($1->nbits,NOACTION,
		    (long)($1->opcode | memloc.addrmdfld),NULLSYM);
		}
	|	BTEST  dual8opnd 
		{
		  addrgen32($1->opcode,&($2));
		}
	|	BTEST  immd  COMMA  reg8mem 
		{
		  testgen32($1->name,&($4));
		  immedgen(8,&($2));
		}
	|	WDYADIC  dualreg16 
		{
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  generate($1->nbits,NOACTION,
		    (long)($1->opcode | dbit | memloc.addrmdfld),NULLSYM);
		}
	|	WDYADIC  dual16opnd 
		{
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  addrgen32($1->opcode | dbit,&($2));
		}
	|	WDYADIC  immd  COMMA  reg16mem 
		{
		  dbit = DBITOFF;
		  if (($2.adexpr.symptr == NULLSYM) &&	/* absolute */
		    spanopt && RANGE($2.adexpr.expval))
		    dbit = DBITON;
		  if (($2.adexpr.reltype & HI12TYPE)) 
		  {
		    if($2.adexpr.expval)
		      yyerror("Illegal fixup");
		    dbit = DBITOFF;
		  }
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  dyad2gen32($1->name,&($4));
		  immedgen(dbit ? 8 : 16,&($2));
		}
	|	LDYADIC  dualreg32 
		{
		  generate($1->nbits,NOACTION,
		    (long)($1->opcode | dbit | memloc.addrmdfld),NULLSYM);
		}
	|	LDYADIC  dual32opnd 
		{
		  addrgen32($1->opcode | dbit,&($2));
		}
	|	LDYADIC  immd  COMMA  reg32mem 
		{
		  dbit = DBITOFF;
		  if (($2.adexpr.symptr == NULLSYM) &&	/* absolute */
		    spanopt && RANGE($2.adexpr.expval))
		    dbit = DBITON;
		  if (($2.adexpr.reltype & HI12TYPE)) 
		  {
		    if($2.adexpr.expval)
		      yyerror("Illegal fixup");
		    dbit = DBITOFF;
		  }
		  dyad2gen32($1->name,&($4));
		  immedgen32(dbit ? 8 : 32,&($2));
		}
	|	BDYADIC  dualreg8 
		{
		  generate($1->nbits,NOACTION,
		    (long)($1->opcode | dbit | memloc.addrmdfld),NULLSYM);
		}
	|	BDYADIC  dual8opnd 
		{
		  addrgen32($1->opcode | dbit,&($2));
		}
	|	BDYADIC  immd  COMMA  reg8mem 
		{
		  dyad2gen32($1->name,&($4));
		  immedgen(8,&($2));
		}
	|	WAOTOP  dualreg16 
		{
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  generate($1->nbits,NOACTION,
		    (long)($1->opcode | memloc.addrmdfld),NULLSYM);
		}
	|	WAOTOP  segmem32  COMMA  reg16 
		{
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  memloc.addrmd.reg = $4.adreg;
		  addrgen32($1->opcode,&($2));
		}
	|	WAOTOP1 segmem32 COMMA reg16
		{
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  memloc.addrmd.reg = $4.adreg ;
		  laddrgen32($1->opcode, &($2)) ;
		}
	|	LAOTOP  dualreg32 
		{
		  generate($1->nbits,NOACTION,
		    (long)($1->opcode | memloc.addrmdfld),NULLSYM);
		}
	|	LAOTOP  segmem32  COMMA  reg32
		{
		  memloc.addrmd.reg = $4.adreg;
		  addrgen32($1->opcode,&($2));
		}
	|	LAOTOP1 segmem32 COMMA reg32
		{
		  memloc.addrmd.reg = $4.adreg ;
		  laddrgen32($1->opcode, &($2)) ;
		}
	|	BXCHG  dualreg8 
		{
		  generate($1->nbits,NOACTION,
		    (long)($1->opcode | memloc.addrmdfld),NULLSYM);
		}
	|	BXCHG  dual8opnd
		{
		  addrgen32($1->opcode,&($2));
		}
	|	WXCHG  dualreg16 
		{
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  if (spanopt && ($2.admode == AREG16MD)) {
		    xchgopt($1->name,$2.adreg);
		  }
		  else {
		    generate($1->nbits,NOACTION,
		      (long)($1->opcode | memloc.addrmdfld),NULLSYM);
		  }
		}
	|	WXCHG  dual16opnd 
		{
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  addrgen32($1->opcode,&($2));
		}
	|	LXCHG  dualreg32 
		{
		  if (spanopt && ($2.admode == AREG32MD)) {
		    xchgopt($1->name,$2.adreg);
		  }
		  else {
		    generate($1->nbits,NOACTION,
		      (long)($1->opcode | memloc.addrmdfld),NULLSYM);
		  }
		}
	|	LXCHG  dual32opnd 
		{
		  addrgen32($1->opcode,&($2));
		}
	|	BCMPXCHG reg8 COMMA reg8mem
		{
		  memloc.addrmd.reg = $2.adreg ;
		  laddrgen32($1->opcode, &($4)) ;
		}
	|	WCMPXCHG reg16 COMMA reg16mem
		{
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  memloc.addrmd.reg = $2.adreg ;
		  laddrgen32($1->opcode, &($4)) ;
		}
	|	LCMPXCHG reg32 COMMA reg32mem
		{
		  memloc.addrmd.reg = $2.adreg ;
		  laddrgen32($1->opcode, &($4)) ;
		}
	|	BXADD reg8md COMMA reg8mem
		{
		  memloc.addrmd.reg = $2.adreg ;
		  laddrgen32($1->opcode, &($4)) ;
		}
	|	WXADD reg16 COMMA reg16mem
		{
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  memloc.addrmd.reg = $2.adreg ;
		  laddrgen32($1->opcode, &($4)) ;
		}
	|	LXADD 	reg32 COMMA reg32mem
		{
		  memloc.addrmd.reg = $2.adreg ;
		  laddrgen32($1->opcode, &($4)) ;
		}

	|	BSWAP	reg32
		{
		  generate ($1->nbits, NOACTION, (long) $1->opcode | $2.adreg,
			    NULLSYM) ;
		}
	|	INVLD	segmem32
		{
		  laddrgen32($1->opcode, &($2));
		}
	|	WBIT1 reg16 COMMA reg16mem
		{
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  memloc.addrmd.reg = $2.adreg ;
		  laddrgen32($1->opcode, &($4)) ;
		}
	|	LBIT1 reg32 COMMA reg32mem
		{
		  memloc.addrmd.reg = $2.adreg ;
		  laddrgen32($1->opcode, &($4)) ;
		}
	|	WBIT1 immd COMMA reg16mem
		{

		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  $2.adexpr.expval &= 0xF ;
		_bit1:
		  nameindx = copystr($1->name,newname);
		  newname[nameindx++] = 'I';
		  newname[nameindx] = '\0';
		  newins = instlookup(newname);

		  if (newins != NULL)
		    memloc.addrmd.reg = $1->tag ;
		  else
		    yyerror("Illegal instruction") ;

		  laddrgen32(newins->opcode, &($4)) ;
		  immedgen32(8, &($2)) ;
		}
	|	LBIT1 immd COMMA reg32mem
		{
		  $2.adexpr.expval &= 0x1F ;
		  goto _bit1 ;
		}
	|	WBIT2 reg16mem COMMA reg16
		{
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  memloc.addrmd.reg = $4.adreg ;
		  laddrgen32($1->opcode, &($2)) ;
		}
	|	LBIT2 reg32mem COMMA reg32
		{
		  memloc.addrmd.reg = $4.adreg ;
		  laddrgen32($1->opcode, &($2)) ;
		}
	|	WBIT3 reg16 COMMA reg16mem
		{
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  memloc.addrmd.reg = $2.adreg ;
		  laddrgen32($1->opcode, &($4)) ;
		}
	|	WBIT3 immd COMMA reg16 COMMA reg16mem
		{

		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		  $2.adexpr.expval &= 0xF ;
		_bit3:
		  nameindx = copystr($1->name,newname);
		  newname[nameindx++] = 'I';
		  newname[nameindx] = '\0';
		  newins = instlookup(newname);

		  if (newins != NULL)
		    memloc.addrmd.reg = $4.adreg ;
		  else
		    yyerror("Illegal instruction") ;

		  laddrgen32(newins->opcode, &($6)) ;
		  immedgen32(8, &($2)) ;
		}
	|	LBIT3 reg32 COMMA reg32mem
		{
		  memloc.addrmd.reg = $2.adreg ;
		  laddrgen32($1->opcode, &($4)) ;
		}
	|	LBIT3 immd COMMA reg32 COMMA reg32mem
		{
		  $2.adexpr.expval &= 0x1F ;
		  goto _bit3 ;
		}
	|	WMOVS	reg8mem COMMA reg16
		{
		_movs1:
		  generate (8, NOACTION, (long) OSP, NULLSYM) ;
		_movs2:
		  memloc.addrmd.reg = $4.adreg ;
		  laddrgen32($1->opcode, &($2)) ;
		}
	|	WMOVZ 	reg8mem COMMA reg16
		{
		  goto _movs1 ;
		}
	|	LMOVS 	reg8md COMMA reg32
		{
		  goto _movs2 ;
		}
	|	LMOVZ 	reg8md COMMA reg32
		{
		  goto _movs2 ;
		}
	|	LMOVS 	reg16md COMMA reg32
		{
		  goto _movs2 ;
		}

	|	LMOVZ 	reg16md COMMA reg32
		{
		  goto _movs2 ;
		}
	|	LMOVS	segmem32 COMMA reg32
		{
		  goto _movs2 ;
		}
	|	LMOVZ	segmem32 COMMA reg32
		{
		  goto _movs2 ;
		}
	|	SETOP  reg8mem
		{
		  laddrgen32($1->opcode,&($2));
		}

	|	PSSECTION ID sect_atts sect_type = {
			if (transition && strcmp($2->name,".rodata") == 0)
				/* .rodata used to be an 'x' section */
				$3.atts &= ~SHF_EXECINSTR;
			cgsect(mksect($2,$3.atts,$4.type));
		}
	|	PSSECTION ID sect_atts = {
			if (transition && strcmp($2->name,".rodata") == 0)
				/* .rodata used to be an 'x' section */
				$3.atts &= ~SHF_EXECINSTR;
			cgsect(mksect($2,$3.atts,$3.type));
		}
	|	PSSECTION ID sect_type = {
			cgsect(mksect($2,0,$3.type));
		}
	|	PSSECTION ID = {
			cgsect(mksect($2,0,SHT_NULL));
		}
	|	PSPREV = {
		  if (!previous)
		    yyerror("'.previous' invoked before any other section");
		  cgsect(previous);
		}
	|	PSDATA = {
		  if (datsec < 0)
		    datsec = mksect(lookup(_DATA, INSTALL),
				    (SHF_ALLOC | SHF_WRITE), SHT_PROGBITS);
		  cgsect(datsec);
		}
	|	PSTEXT = {
		  if (txtsec < 0)
		    txtsec == mksect(lookup(_TEXT, INSTALL),
				     (SHF_ALLOC | SHF_EXECINSTR), SHT_PROGBITS);
		  cgsect(txtsec);
		}
	|	PSBSS = {
		  if (bsssec < 0)
		    bsssec = mksect(lookup(_BSS, INSTALL),
				    (SHF_ALLOC | SHF_WRITE), SHT_NOBITS);
		  cgsect(bsssec);
		}
	|	dotcomm = {
		  /* For compatibility, set the alignment to 4. */
		  $1->align = 4;
		}
	|	dotcomm COMMA exprX = {
	  	  ck_nonneg_absexpr(&($3),".comm alignment");
		  if ($3.expval == 0 || (($3.expval - 1) & $3.expval) != 0)
			 yyerror(".comm alignment has invalid value");
		  $1->align = $3.expval;
		}
	|	PSGLOBAL gidlist
	|	PSLOCAL lidlist
	|	PSWEAK widlist
	|	PSSIZE ID COMMA exprX = {
		  if (BIT_IS_ON($2,SIZE_SET))
		      yyerror("symbol already has a size");
		  if (ABSOLUTE($4)) {
		    if ($4.expval < 0)
		      yyerror(".size argument has negative value");
		    $2->size = $4.expval;
		  } else if (RELOCATABLE($4))
		    yyerror("Expression in '.size' cannot be relocatable");
		  else if (UNDEFINED($4)) {
		    remember_set_or_size($2,$4.symptr,SETTO_SYM,REMEMBER_SIZE);
		    $2->size = $4.expval;
		  } else /* (UNEVALUATED($4)) */ 
		    remember_set_or_size($2,$4.symptr,SETTO_XPTR,REMEMBER_SIZE);
		  $2->flags |= SIZE_SET;
		}
	|	PSTYPE ID COMMA SYMTYPE = {
		  if (BIT_IS_ON($2,TYPE_SET))
		      yyerror("symbol already has a type");
	  	  $2->type = $4;
		  $2->flags |= TYPE_SET;
		}
	|	PSBSS  ID  COMMA  exprX = {
	  	  ck_nonneg_absexpr(&($4),".bss size");
		  if (! UNDEF_SYM($2) && !COMMON($2) ){
		    yyerror("multiply defined label in `.bss'");
                    (void) fprintf(stderr,"\t\t... %s\n",$2->name);
		  }
		  bss($2,$4.expval,4);
		}
	|	PSBSS  ID  COMMA  exprX COMMA exprX = {
	  	  ck_nonneg_absexpr(&($6),".bss alignment");
	  	  ck_nonneg_absexpr(&($4),".bss size");
		  if (! UNDEF_SYM($2) && !COMMON($2) ) {
		    yyerror("multiply defined label in `.bss'");
                    (void) fprintf(stderr,"\t\t... %s\n",$2->name);
		  }
		  bss($2,$4.expval,$6.expval);
		}
	|	PSLCOMM  ID  COMMA  exprX = {
	  	  ck_nonneg_absexpr(&($4),".lcomm size");
		  if (! UNDEF_SYM($2) && !COMMON($2) )
		    yyerror("multiply defined label in `.lcomm'");
		  if (BIT_IS_ON($2,BOUND)  && (!LOCAL($2)))
		    yyerror("can not override symbol binding");
		  bss($2,$4.expval,4);
		}
	|	PSLCOMM  ID  COMMA  exprX COMMA exprX = {
	  	  ck_nonneg_absexpr(&($6),".lcomm alignment");
	  	  ck_nonneg_absexpr(&($4),".lcomm size");
		  if (! UNDEF_SYM($2) && !COMMON($2) )
		    yyerror("multiply defined label in `.lcomm'");
		  if (BIT_IS_ON($2,BOUND)  && (!LOCAL($2)))
		    yyerror("can not override symbol binding");
		  bss($2,$4.expval,$6.expval);
		}
	|	PSSET  ID  COMMA  exprX  =  {
	  	  assignmt($2,&($4));
		}
	|	ID  EQ  exprX  =  {
		  assignmt($1,&($3));
		}
	|	PSBYTE {
		  spctype = NBPW / 2;
		} exprlist = {
		  if (numbits) {
		    generate(8-numbits,DUMPBITS,8L,NULLSYM);
		    numbits = 0;
		  }
		}
	|	PSVALUE {
		  ckalign(2);
		  spctype = NBPW;
		} exprlist = {
		  if (numbits) {
		    generate(16-numbits,DUMPBITS,16L,NULLSYM);
		    numbits = 0;
		  }
		}
	|	PS2BYTE {
		  spctype = NBPW;
		} exprlist = {
		  if (numbits) {
		    generate(16-numbits,DUMPBITS,16L,NULLSYM);
		    numbits = 0;
		  }
		}
	|	PSLONG {
		  ckalign(4);
		  spctype = 2*NBPW ;
		} exprlist
	|	PS4BYTE {
		  spctype = 2*NBPW ;
		} exprlist
	|	PSFLOAT {
		  ckalign(4);
		  special287 = PSFLOAT ;
		} fexprlist = {
		  special287 = 0 ;
		}
	|	PSDOUBLE {
		  ckalign(4);	/* align on 4 or 8 bytes? */
		  special287 = PSDOUBLE ;
		} fexprlist = {
		  special287 = 0 ;
		}
	|	PSEXT {
		  ckalign(4);	/* align on 4, 8 or 10 bytes? */
		  special287 = PSEXT ;
		} fexprlist = {
		  special287 = 0 ;
		}
	|	PSBCD {
		  ckalign(4);	/* align on 4, 8 or 10 bytes? */
		  special287 = PSBCD ;
		} fexprlist = {
		  special287 = 0 ;
		}
	|	PSSTRING stringlist
	|	PSALIGN expr
		{
		  if ($2.expval == 0 || (($2.expval - 1) & $2.expval) != 0)
			 yyerror("invalid .align argument");
		  ck_nonneg_absexpr(&($2),".align argument");
		  alignmt($2.expval) ;
		}
	|	PSEVEN = {
		  alignmt(2L);
		}
	|	PSZERO exprX = {
                  if (!ABSOLUTE($2))
		    yyerror("'.zero' size not absolute");
		  if ($2.expval < 0L)
		    yyerror("Invalid .zero size");
		  exp_generate(0,DOTZERO, $2);
		  newdot += $2.expval;
		}
	|	PSIDENT STRING = {
		  comment($2);
		}
	|	PSFILE  STRING = {
		  if (cfile != NULL)
		    yyerror("Only 1 '.file' allowed");
		  else {
		    cfile = (symbol *) malloc(sizeof(symbol));
		    cfile->name = malloc(strlen($2) + 1);
		    strcpy(cfile->name,$2);
		    cfile->binding = STB_LOCAL;
		    cfile->flags = (~TYPE_SET) | (~SIZE_SET) | GO_IN_SYMTAB;
		    cfile->sectnum = SHN_ABS;
		    cfile->value = 0;
		    cfile->size = 0;
		    cfile->type = STT_FILE;
		  }
		}
	|	PSVERS STRING = {
		 	if (strcmp($2,as_version) > 0) {
				yyerror("inappropriate assembler version");
				(void) fprintf(stderr,"have %s expect %s or greater\n",
					$2,as_version);
			}
		}
	|	PSLN  exprX = {
		  obsolete_dir(".ln directive obsolete",WARN);
		}
	|	PSLN  exprX  COMMA  exprX = {
		  obsolete_dir(".ln directive obsolete",WARN);
		}
	|	PSDEF  ID = {
		  obsolete_dir(".def directive obsolete",WARN);
		  if (transition)
		    generate(0,(unsigned short) DEFINE,NULLVAL,$2);
		}
	|	dotdim = {
		  obsolete_dir(".dim directive obsolete",NOWARN);
		}
	|	PSLINE exprX = {
		  obsolete_dir(".line directive obsolete",NOWARN);
		}
	|	PSSCL exprX = {
		  obsolete_dir(".scl directive obsolete",NOWARN);
		  if (transition)
		    exp_generate(0,(unsigned short) SETSCL, $2);
		}
	|	PSSIZE exprX = {
		  obsolete_dir("obsolete use of .size directive",NOWARN);
 		}
	|	PSTAG  ID = {
		  obsolete_dir(".tag directive obsolete",NOWARN);
		}
	|	PSTYPE exprX = {
		  obsolete_dir("obsolete use of .type directive",NOWARN);
		}
	|	PSVAL exprX = {
		  obsolete_dir(".val directive obsolete",NOWARN);
		  if (transition)
		    exp_generate(0,(unsigned short) SETVAL, $2);
		}
	|	PSENDEF = {
		  obsolete_dir(".endef directive obsolete",NOWARN);
		  if (transition)
		    generate(0,(unsigned short) ENDEF, NULLVAL, NULLSYM);
		}
	|	SHARP  NUMBER = {
		  line = (unsigned short)($2) - 1;	/* what is this for? */
		}
	|	SHARP  NUMBER  STRING = {
		  line = (unsigned short)($2) - 1;	/* what is this for? */
		  strcpy(filenames[0],$3);
		}
	|	PSJMPTAB = {
		  generate($1->nbits,NOACTION,$1->opcode,NULLSYM);	/* what is this for? */
		}
	;

sect_atts :	COMMA STRING = { 
		  register char *s = $2;
		  register int att = 0;
		  int type = SHT_NULL;
		  if (transition) {
		    werror(".section has obsolete semantics in transition mode");
		    while (*s) switch(*s++) {
		      case 'a':
		      	/* allocatable */
		      	att |= SHF_ALLOC ;
			if ( (type != SHT_NULL) && (type != SHT_PROGBITS))
			  werror("section type has already been set");
			else
			  type = SHT_PROGBITS;
			break;
		      case 'b':
		      	/* zero initialized block */
		      	att |= SHF_ALLOC | SHF_WRITE ;
			if ( (type != SHT_NULL) && (type != SHT_NOBITS))
			  werror("section type has already been set");
			else
			  type = SHT_NOBITS;
			break;
		      case 'c': /* copy */
		      case 'd': /* dummy */
		      case 'n': /* noload */
		      case 'o': /* overlay */
		      case 'l': /* lib */
		      case 'i': /* info */
			if ( (type != SHT_NULL) && (type != SHT_PROGBITS))
			  werror("section type has already been set");
			else
			  type = SHT_PROGBITS;
			break;
		      case 'x':
			/* executable */
			att |= SHF_ALLOC | SHF_EXECINSTR;
			if ( (type != SHT_NULL) && (type != SHT_PROGBITS))
			  werror("section type has already been set");
			else
			  type = SHT_PROGBITS;
			break;
		      case 'w':
			/* writable */
			att |= SHF_ALLOC | SHF_WRITE;
			if ( (type != SHT_NULL) && (type != SHT_PROGBITS))
			  werror("section type has already been set");
			else
			  type = SHT_PROGBITS;
			break;
		      default:
			yyerror("invalid section attribute");
			break;
		    } 
		  } else 	{ /* no translation */
		    while (*s) switch(*s++) {
 		      case 'a':
			att |= SHF_ALLOC;
			break;
		      case 'x':
			/* executable */
			att |=  SHF_EXECINSTR;
			break;
		      case 'w':
			/* writable */
			att |=  SHF_WRITE;
			break;
		      default:
			yyerror("invalid section attribute");
			break;
		    }
		  } /* if (transition) */
		  $$.atts = att;
		  $$.type = type;
		}
	;

sect_type :	COMMA SECTTYPE = {
		  $$.type = $2;
		}
	;

dotcomm :	PSCOMM ID COMMA exprX = {
	  	  ck_nonneg_absexpr(&($4),".comm size");
		  if (!BIT_IS_ON($2,BOUND))
		    $2->binding = STB_GLOBAL;
		  $2->size = $4.expval;
		  $2->type = STT_OBJECT;
		  if (UNDEF_SYM($2))
		    $2->sectnum = SHN_COMMON;
		  if (BIT_IS_ON($2,SET_SYM)) {
		    yyerror("multiply defined symbol");
		    (void) fprintf(stderr,"\t\t...%s\n",$2->name);
		  }
		  $$ = $2;
		}
	;

gidlist :	ID = {
		if (BIT_IS_ON($1,BOUND)  && (!GLOBAL($1)))
			yyerror("can not override symbol binding");
		$1->binding = STB_GLOBAL;
		$1->flags |= BOUND | GO_IN_SYMTAB;
		}
	|	gidlist COMMA ID = {
		if (BIT_IS_ON($3,BOUND)  && (!GLOBAL($3)))
			yyerror("can not override symbol binding");
		$3->binding = STB_GLOBAL;
		$3->flags |= BOUND | GO_IN_SYMTAB;
		}
	;

lidlist :	ID = {
		if (BIT_IS_ON($1,BOUND)  && (!LOCAL($1)))
			yyerror("can not override symbol binding");
		$1->binding = STB_LOCAL;
		$1->flags |= BOUND | GO_IN_SYMTAB;
		}
	|	lidlist COMMA ID = {
		if (BIT_IS_ON($3,BOUND)  && (!LOCAL($3)))
			yyerror("can not override symbol binding");
		$3->binding = STB_LOCAL;
		$3->flags |= BOUND | GO_IN_SYMTAB;
		}
	;

widlist :	ID = {
		if (BIT_IS_ON($1,BOUND)  && (!WEAK($1)))
			yyerror("can not override symbol binding");
		$1->binding = STB_WEAK;
		$1->flags |= BOUND | GO_IN_SYMTAB;
		}
	|	widlist COMMA ID = {
		if (BIT_IS_ON($3,BOUND)  && (!WEAK($3)))
			yyerror("can not override symbol binding");
		$3->binding = STB_WEAK;
		$3->flags |= BOUND | GO_IN_SYMTAB;
		}
	;

stringlist :	STRING = {
		  genstring ( $1 ) ;
		}
	|	stringlist COMMA STRING = {
		  genstring ( $3 ) ;
		}
	;

dotdim	:	PSDIM  exprX
	|	dotdim  COMMA  exprX
	;

	/*
	 *	special exprlist type for the 80287 initialisers
	 */

fexprlist:	fgexpr
	|	fexprlist {
	          dot->value = newdot ;
	          generate(0,NEWSTMT,(long)line,NULLSYM);
		} COMMA fgexpr
	;

fgexpr	:	FNUMBER {
  		  short action, width;
		  switch(special287) {
		    case PSFLOAT:
		      width = 2 ;
		      break ;
		    case PSDOUBLE:
		      width = 4 ;
		      break ;
		    case PSEXT:
		      width = 5;
		      break;
		    case PSBCD:
		      width = 5 ;
		      break ;
		  }
		  for(action=0;action<width;action++)
		    generate(16,SWAPB,(unsigned long)$1.fvala[action],NULLSYM);
		}
	|	MINUS FNUMBER {
  		  short action, width;
		  switch(special287) {
		    case PSFLOAT:
		      $2.fvala[1] |= 0x8000 ;
		      width = 2 ;
		      break ;
		    case PSDOUBLE:
		      $2.fvala[3] |= 0x8000 ;
		      width = 4 ;
		      break ;
		    case PSEXT:
		      $2.fvala[4] |= 0x8000 ;
		      width = 5 ;
		      break ;
		    case PSBCD:
		      $2.fvala[4] |= 0x8000 ;
		      width = 5 ;
		      break ;
		  }
		  for(action=0;action<width;action++)
		    generate(16,SWAPB,(unsigned long)$2.fvala[action],NULLSYM);
		}
	;

exprlist :	dgexpr
	|	exprlist = {
		  dot->value = newdot;
		  generate(0,NEWSTMT,(long)line,NULLSYM);
		} COMMA  dgexpr
	;

dgexpr	:	exprX 
		{
		int i ;
		unsigned char action;
		  if (bitpos > 0)
		    yyerror("Expression crosses field boundary");

		  /*
		   * Figure out which action routine to use   
		   * in case there is an unresolved symbol.  
		   * If a full word is being used, then     
		   * a relocatable may be specified.       
		   * Otherwise it is restricted to being an
		   * absolute (forward reference). 
		   */
		  if ($1.reltype & X86TYPE)
		    action = ($1.reltype & LO32TYPE) ? LO32BITS : HI12BITS;
		  else {
		    if (spctype == NBPW) 
		      i = SWAPB ;
		    else if (spctype == 2*NBPW)
		      i = SWAPWB ;
		    else if ($1.symptr != NULLSYM)
		      i = RESABS ;
		    else
		      i = NOACTION ;
		    action = actreloc($1.reltype, i) ;
		  }

		  if((action==HI12BITS) && ($1.expval))
		    yyerror("Illegal fixup generation");
		  exp_generate(spctype,action,$1);
		}
	|	NUMBER  COLON  exprX = {
		  long width;
		  width = $1;
		  numbits += width;
		  if (bitpos + width > spctype)
		    yyerror("Expression crosses field boundary");
		  exp_generate(width,(spctype == NBPW / 2) ?
		    PACK8 :
		    PACK16,$3);
		}
	;


dualreg32 :	reg32 COMMA reg32 
		{
		_dualreg32:
		  memloc.addrmd.mod = REGMOD ;
		  memloc.addrmd.reg = $3.adreg ;
		  memloc.addrmd.rm = $1.adreg ;
		  dbit = DBITON ;
		  if ($1.adreg == AREG32MD)
		    $$.adreg = $3.adreg ;
		  else
		    $$.admode = $3.admode ;
		}
	;

dual32opnd :	reg32 COMMA segmem32
		{
		  memloc.addrmd.reg = $1.adreg ;
		  dbit = DBITOFF ;
		  $$ = $3 ;
		}
	|	segmem32 COMMA reg32
		{
		  memloc.addrmd.reg = $3.adreg ;
		  dbit = DBITON ;
		}
	;

reg32mem :	reg32md
	|	segmem32
	;

segmem32 :	mem32
	|	SREG COLON mem32
		{
		    switch($1->opcode) {
		      case 0 :
		      case 1 :
		      case 2 :
		      case 3 :
		        generate(8, NOACTION,
		          ((long) SEGPFX | ($1->opcode << 3)), NULLSYM) ;
			break ;
		      case 4 :
		        generate(8, NOACTION, (long) 0x64, NULLSYM) ;
			break ;
		      case 5 :
		        generate(8, NOACTION, (long) 0x65, NULLSYM) ;
			break ;
		    }
		  $$ = $3 ;
		}
	;

mem32 :		extaddr32md
	|	expdisp32md
	|	disp32md
	;

reg32md :	reg32
		{
		  memloc.addrmd.mod = REGMOD ;
		  memloc.addrmd.rm = $1.adreg ;
		}
	;

extaddr32md :	expr
		{
		  $$.admode = EXADMD ;
		  $$.adexpr = $1 ;
		  memloc.addrmd.mod = NODISP ;
		  memloc.addrmd.rm = EA32 ;
		}
	;

	;

disp32md :	dispbase32
		{
		  /*
		   * all the 32 bit registers except ebp and esp
		   * can be used as address registers with displacement
		   * information.
		   * To provide this functionality the ebp with no displacement
		   * is converted to ebp with a displacement.  Set to a 32
		   * bit displacement here converted to an 8 bit displacement
		   * in addrgen32 or laddrgen32.
		   * Esp is escaped into the sib byte with index register.
		   */
		  if (memloc.addrmd.rm == 5) {		/* EBP */
		    $$.admode = EDSPMD ;
		    memloc.addrmd.mod = DISP32 ;
		    $$.adexpr.expval = 0 ;
		    $$.adexpr.exptype = ABS ;
		    $$.adexpr.reltype = NORELTYPE ;
		    $$.adexpr.symptr = NULLSYM ;
		  }
		  else if (memloc.addrmd.rm == 4) {	/* ESP */
		    $$.admode = DSPMD ;
		    memloc.addrmd.mod = NODISP ;
		    memloc.addrmd.rm = ESC_TO_SIB ;
		    memsib.sib.ss = 0 ;
		    memsib.sib.base = SIB_ESP_BASE ;
		    memsib.sib.index = NO_IDX_REG ;
		  }
		  else {
		    $$.admode = DSPMD ;
		    memloc.addrmd.mod = NODISP ;
		  }
		}
	|	dispindex32
		{
		  /*
		   * this is a special use of the mod field
		   * to specify a 32 bit displacement with a scaled index
		   * and no base register 
		   * the NODISP in the mod field is what the instruction 
		   * requires, and it will force 32 bits of displacement when
		   * this production goes through addrgen32 and laddrgen32
		   */
		  $$.admode = EDSPMD ;
		  memloc.addrmd.mod = NODISP ;
		  $$.adexpr.expval = 0 ;
		  $$.adexpr.exptype = ABS ;
		  $$.adexpr.reltype = NORELTYPE ;
		  $$.adexpr.symptr = NULLSYM ;
		}
	|	dualdispbase32
		{
		  if ( memsib.sib.base == EBP) {
		    $$.admode = EDSPMD ;
		    memloc.addrmd.mod = DISP32 ;
		    $$.adexpr.expval = 0 ;
		    $$.adexpr.exptype = ABS ;
		    $$.adexpr.reltype = NORELTYPE ;
		    $$.adexpr.symptr = NULLSYM ;
		  }
		  else {
		    $$.admode = DSPMD ;
		    memloc.addrmd.mod = NODISP ;
		  }
		}
	|	dualdispbasescl32
		{
		  if ( memsib.sib.base == EBP) {
		    $$.admode = EDSPMD ;
		    memloc.addrmd.mod = DISP32 ;
		    $$.adexpr.expval = 0 ;
		    $$.adexpr.exptype = ABS ;
		    $$.adexpr.reltype = NORELTYPE ;
		    $$.adexpr.symptr = NULLSYM ;
		  }
		  else {
		    $$.admode = DSPMD ;
		    memloc.addrmd.mod = NODISP ;
		  }
		}
	;

expdisp32md :	expr dispbase32
		{
		  $$.adexpr = $1 ;
		  $$.admode = EDSPMD ;
		  memloc.addrmd.mod = DISP32 ;
		  if (memloc.addrmd.rm == 4) {		/* esp */
		    /*
		     * mod is set to DISP32, addrgen32 will convert this
		     * to disp 8 if the expresion is with in range.
		     * rm == 4 is the escape to the sib already.
		     */
		    memsib.sib.ss = 0 ;
		    memsib.sib.base = SIB_ESP_BASE ;
		    memsib.sib.index = NO_IDX_REG ;
		  }
		}
	|	expr dispindex32
		{
		  /*
		   * this is a special use of the mod field
		   * to specify a 32 bit displacement with a scaled index
		   * and no base register 
		   * the NODISP in the mod field is what the instruction 
		   * requires, and it will force 32 bits of displacement when
		   * this production goes through addrgen32 and laddrgen32 
		   */
		  $$.adexpr = $1 ;
		  $$.admode = EDSPMD ;
		  memloc.addrmd.mod = NODISP ;
		}
	|	expr dualdispbase32
		{
		  $$.adexpr = $1 ;
		  if (($1.symptr == NULLSYM) && ($1.expval == 0L)) {
		    if ( memsib.sib.base == EBP) {
		      $$.admode = EDSPMD ;
		      memloc.addrmd.mod = DISP32 ;
		      $$.adexpr.expval = 0 ;
		      $$.adexpr.exptype = ABS ;
		      $$.adexpr.reltype = NORELTYPE ;
		      $$.adexpr.symptr = NULLSYM ;
		    }
		    else {
		      $$.admode = DSPMD ;
		      memloc.addrmd.mod = NODISP ;
		    }
		  }
		  else {
		    $$.admode = EDSPMD ;
		    memloc.addrmd.mod = DISP32 ;
		  }
		}
	|	expr dualdispbasescl32
		{
		  $$.adexpr = $1 ;
		  if (($1.symptr == NULLSYM) && ($1.expval == 0L)) {
		    if ( memsib.sib.base == EBP) {
		      $$.admode = EDSPMD ;
		      memloc.addrmd.mod = DISP32 ;
		      $$.adexpr.expval = 0 ;
		      $$.adexpr.exptype = ABS ;
		      $$.adexpr.reltype = NORELTYPE ;
		      $$.adexpr.symptr = NULLSYM ;
		    }
		    else {
		      $$.admode = DSPMD ;
		      memloc.addrmd.mod = NODISP ;
		    }
		  }
		  else {
		    $$.admode = EDSPMD ;
		    memloc.addrmd.mod = DISP32 ;
		  }
		}

dispbase32 :	LP reg32 RP
		{
		  $$ = $2 ;
		  memloc.addrmd.rm = $2.adreg ;
		} 
	;

	/* |	THIS COMMENT APPLIES TO  dispindex32 :
	 * V
	 * this is the production that handles the scaled index with
	 * no base register mode.
	 * This mode is not really supported by the 80386.
	 * So to get around this we use the scaled index with a
	 * mandatory 32 bit displacement.  
	 * There are only two productions that use this production
	 * they are disp32md and expdisp32md.  Both of these
	 * productions set up their .admode and the memloc.addrmd.mod
	 * with the proper values to force addrgen32 and laddrgen32
	 * to put out a full 32 bits of displacement info.  Addrgen32
	 * and Laddrgen32 normally try to optimize for 8 bit displacements.
	 */
dispindex32 :	LP COMMA reg32 RP
		{
		  $$ = $3 ;
		  memloc.addrmd.rm = ESC_TO_SIB ;
		  memsib.sib.ss = 0 ;
		  memsib.sib.base = NO_BASE_REG ;
		  memsib.sib.index = (short) $3.adreg ;
		  if (memsib.sib.index == NO_IDX_REG)
		    yyerror("Illegal index reg") ;
		}
	|	LP COMMA reg32 COMMA expr RP
		{
		  $$ = $3 ;
		  switch($5.expval) {
		    case 1 :
		      memsib.sib.ss = 0 ;
		      break ;
		    case 2 :
		      memsib.sib.ss = 1 ;
		      break ;
		    case 4 :
		      memsib.sib.ss = 2 ;
		      break ;
		    case 8 :
		      memsib.sib.ss = 3 ;
		      break ;
		    default :
		      yyerror("Illegal scale value") ;
		      break ;
		  }
		  memloc.addrmd.rm = ESC_TO_SIB ;
		  memsib.sib.base = NO_BASE_REG ;
		  memsib.sib.index = (short) $3.adreg ;
		  if (memsib.sib.index == NO_IDX_REG)
		    yyerror("Illegal index reg") ;
		}
	;

dualdispbase32 : LP reg32 COMMA reg32 RP
		{
		  memloc.addrmd.rm = ESC_TO_SIB ;
		  memsib.sib.ss = 0 ;
		  memsib.sib.base = $2.adreg ;
		  memsib.sib.index = $4.adreg ;
		  if ($4.adreg == NO_IDX_REG)
		    yyerror("Illegal index reg") ;
		}
	;


dualdispbasescl32 : LP reg32 COMMA reg32 COMMA expr RP
		{
		  $$ = $2 ;
		  switch($6.expval) {
		    case 1 :
		      memsib.sib.ss = 0 ;
		      break ;
		    case 2 :
		      memsib.sib.ss = 1 ;
		      break ;
		    case 4 :
		      memsib.sib.ss = 2 ;
		      break ;
		    case 8 :
		      memsib.sib.ss = 3 ;
		      break ;
		    default :
		      yyerror("Illegal scale value") ;
		      break ;
		  }
		  memloc.addrmd.rm = ESC_TO_SIB ;
		  memsib.sib.base = $2.adreg ;
		  memsib.sib.index = $4.adreg ;
		  if ($4.adreg == NO_IDX_REG)
		    yyerror("Illegal index reg") ;
		}
	;

reg32 :		REG32
		{
		  $$.admode = REG32MD ;
		  $$.adreg = (short) ($1->opcode) ;
		}
	|	BREG32
		{
		  $$.admode = REG32MD ;
		  $$.adreg = (short) ($1->opcode) ;
		}
	|	AREG32
		{
		  $$.admode = AREG32MD ;
		  $$.adreg = (short) ($1->opcode) ;
		}
	|	IREG32
		{
		  $$.admode = REG32MD ;
		  $$.adreg = (short) ($1->opcode) ;
		}
	|	DREG32
		{
		  $$.admode = REG32MD ;
		  $$.adreg = (short) ($1->opcode) ;
		}
	;

dualreg16 :	reg16  COMMA  reg16 
		{
		  goto _dualreg32 ;
		}
	;

dual16opnd :	reg16  COMMA  segmem32 = {
		  memloc.addrmd.reg = $1.adreg;
		  dbit = DBITOFF;
	  	  $$ = $3;			
		}
	|	segmem32  COMMA  reg16 = {
		  memloc.addrmd.reg = $3.adreg;
		  dbit = DBITON;
		}
	;

dualreg8 :	reg8  COMMA  reg8 = {
		  memloc.addrmd.mod = REGMOD;
		  memloc.addrmd.reg = $3.adreg;
		  memloc.addrmd.rm = $1.adreg;
		  dbit = DBITON;
		  if ($1.admode == AREG8MD)
		    $$.adreg = $3.adreg;
		  else
		    $$.admode = $3.admode;
		}
	;

dual8opnd :	reg8  COMMA  segmem32 = {
		  memloc.addrmd.reg = $1.adreg;
		  dbit = DBITOFF;
		  $$ = $3;
		}
	|	segmem32  COMMA  reg8 = {
		  dbit = DBITON;
		  memloc.addrmd.reg = $3.adreg;
		}
	;

segextaddr :	extaddr32md
	|	SREG  COLON  extaddr32md
		{
		    switch($1->opcode) {
		      case 0 :
		      case 1 :
		      case 2 :
		      case 3 :
		        generate(8, NOACTION,
		          ((long) SEGPFX | ($1->opcode << 3)), NULLSYM) ;
			break ;
		      case 4 :
		        generate(8, NOACTION, (long) 0x64, NULLSYM) ;
			break ;
		      case 5 :
		        generate(8, NOACTION, (long) 0x65, NULLSYM) ;
			break ;
		    }
		  $$ = $3 ;
		}
	;

segdef :	STAR  reg32mem = {
		  $$ = $2;
		}
	;

reg16mem :	reg16md
	|	segmem32
	;

reg8mem :	reg8md
	|	segmem32
	;


immd	:	DOLLAR  expr = {
		  $$.admode = IMMD;
		  $$.adexpr = $2;
	 	}
	;

reg16md :	reg16 = {
		  memloc.addrmd.rm = $1.adreg;
		  memloc.addrmd.mod = REGMOD;
		}
	;

reg8md :	reg8 = {
		  memloc.addrmd.rm = $1.adreg;
		  memloc.addrmd.mod = REGMOD;
		}
	;

reg16 :		REG16 = {
		  $$.admode = REG16MD;
		  /* register number is in opcode field of instab */
		  $$.adreg = (short)($1->opcode);
		}
	|	BREG16 = {
		  $$.admode = REG16MD;
		  $$.adreg = (short)($1->opcode);
		}
	|	AREG16 = {
		  $$.admode = AREG16MD;
		  $$.adreg = (short)($1->opcode);
		}
	|	IREG16 = {
		  $$.admode = REG16MD;
		  $$.adreg = (short)($1->opcode);
		}
	|	DREG16 = {
		  $$.admode = REG16MD;
		  $$.adreg = (short)($1->opcode);
		}
	|	reg32 = {
		  $$ = $1 ;
		}
	;

reg8 :		AREG8 = {
		  $$.admode = AREG8MD;
		  $$.adreg = (short)($1->opcode);
		}
	|	REG8 = {
		  $$.admode = REG8MD;
		  $$.adreg = (short)($1->opcode);
		}
	|	CLREG8 = {
		  $$.admode = REG8MD;
		  $$.adreg = (short)($1->opcode);
		}
	;


	/*
	 *	80287 - floatreg 
	 *	%st(i)
	 *		0 <= i <= 15
	 */
floatreg:	FREG LP expr RP = {
		  if( ($3.symptr != NULLSYM ) ||
		    ($3.exptype != ABS ) ||
  		    ($3.expval & 0xfff8 ) ) {
		    yyerror ( "Syntax error - stack register index \
				should be constant <= 7 and >=0");
		  }
		  $$ = $3 ;
		}
	;

expr	:	exprX = {
		  if(( $1.reltype & HI12TYPE ) && ( $1.expval) )
		    yyerror("Can't relocate segment fixup");
		  $$ = $1 ;
		}
exprX	:	term
	|	exprX  PLUS  term = {
			/* one side of expr must be absolute */
			if ( ABSOLUTE($1) && !UNEVALUATED($3) ) {
			  /* abs+rel or abs+und */
			  $$ = $3;
			  $$.expval += $1.expval;
			} else if ( !UNEVALUATED($1) && ABSOLUTE($3) ) {
			  /* abs+abs, rel+abs or und+abs */
			  $$ = $1;
			  $$.expval += $3.expval;
			} else if ( RELOCATABLE($1) && RELOCATABLE($3) ) {
			  EXPRERR($$,"Illegal addition");
			} else { /* build tree */
			  if ( UNDEFINED($1) )
			    /*und+rel or und+und or und+uneval*/
			    treeleaf(&($1));
			  if ( UNDEFINED($3) )
			    /*rel+und or und+und or uneval+und*/
			    treeleaf(&($3));
			  /* und operands have become uneval */
			  if ( UNEVALUATED($1) || UNEVALUATED($3) ) {
			    if ( ABSOLUTE($1) || ABSOLUTE($3) ) {
			      /*abs+uneval or uneval+abs*/
			      $$ = *(merge(&($1),&($3)));
			    } else {
			      if ( !UNEVALUATED($1) )
				/*rel+uneval*/
				treeleaf(&($1));
			      else if (!UNEVALUATED($3) )
				/*uneval+rel*/
				treeleaf(&($3));
			      treenode(PLUS_OP,&($1),&($3),&($$));
			    }
			  } else
			    EXPRERR($$,"Illegal addition");
			  if ($1.reltype && $3.reltype)
			    EXPRERR($$,"Illegal addition");
			  $$.reltype = $1.reltype ? $1.reltype: $3.reltype;
			}
		}
	|	exprX  MINUS  term = {
			if ( !UNEVALUATED($1) && ABSOLUTE($3) ) {
			  /* abs-abs, rel-abs or und-abs */
			  $$ = $1;
			  $$.expval -= $3.expval;
			} else if ( ABSOLUTE($1) && RELOCATABLE($3) ) {
			  /* abs-rel */
			  EXPRERR($$,"Illegal subtraction");
			} else { /* build tree */
			  if ( UNDEFINED($1) )
			    /*und-rel, und-und, or und-uneval*/
			    treeleaf(&($1));
			  if ( UNDEFINED($3) )
			    /*abs-und, rel-und, und-und, or uneval-und*/
			    treeleaf(&($3));
			  /* und operands have become uneval */
			  if ( UNEVALUATED($1) || UNEVALUATED($3) ) {
			    if ( ABSOLUTE($3) ) {
			      /*uneval-abs*/
			      $3.expval = -($3.expval);
			      $$ = *(merge(&($1),&($3)));
			    } else {
			      if ( !UNEVALUATED($1) )
				/*abs-uneval or rel-uneval*/
				treeleaf(&($1));
			      else if ( !UNEVALUATED($3) )
				/*uneval-rel*/
				treeleaf(&($3));
			      treenode(MINUS_OP,&($1),&($3),&($$));
			    }
			  } else if ( $1.symptr->sectnum==$3.symptr->sectnum) { 
			    /* rel-rel */
			    treeleaf(&($1));
			    treeleaf(&($3));
			    treenode(MINUS_OP,&($1),&($3),&($$));
			  } else
			    EXPRERR($$,"Illegal subtraction");
			    if ($1.reltype && $3.reltype)
			       EXPRERR($$,"Illegal subtraction");
			    $$.reltype = $1.reltype ? $1.reltype: $3.reltype;
			}
		}
	|	exprX  MUL  term = {
			if ( ABSOLUTE($1) && ABSOLUTE($3) ) {
			  /* abs*abs */
			  $$ = $1;
			  $$.expval *= $3.expval;
			} else if ( RELOCATABLE($1) || RELOCATABLE($3) ) {
			  /* rel*abs, rel*rel, rel*und, rel*uneval,
			     abs*rel, und*rel, or uneval*rel */
			  EXPRERR($$,"Illegal multiplication");
			} else { /* build tree */
			  if ( UNDEFINED($1) )
			    /* und*abs, und*und, or und*uneval */
			    treeleaf(&($1));
			  if ( UNDEFINED($3) )
			    /* abs*und, und*und, or uneval*und */
			    treeleaf(&($3));
			  if ( UNEVALUATED($1) || UNEVALUATED($3) ) {
			    if ( !UNEVALUATED($1) )
			      /* abs*uneval */
			      treeleaf(&($1));
			    else if ( !UNEVALUATED($3) )
			      /* uneval*abs */
			      treeleaf(&($3));
			    treenode(MULT_OP,&($1),&($3),&($$));
			  }
			    if ($1.reltype && $3.reltype)
			       EXPRERR($$,"Illegal multiplication");
			    $$.reltype = $1.reltype ? $1.reltype: $3.reltype;
			}
		}
	|	exprX  DIV  term = {
			if ( ABSOLUTE($1) && ABSOLUTE($3) ) {
			  /* abs/abs */
			  $$ = $1;
			  $$.expval /= $3.expval;
			} else if ( RELOCATABLE($1) || RELOCATABLE($3) ) {
			  /* rel/abs, rel/rel, rel/und, rel/uneval,
			     abs/rel, und/rel, or uneval/rel */
			  EXPRERR($$,"Illegal division");
			} else { /* build tree */
			  if ( UNDEFINED($1) )
			    /* und/abs, und/und, or und/uneval */
			    treeleaf(&($1));
			  if ( UNDEFINED($3) )
			    /* abs/und, und/und, or uneval/und */
			    treeleaf(&($3));
			  if ( UNEVALUATED($1) || UNEVALUATED($3) ) {
			    if ( !UNEVALUATED($1) )
			      /* abs/uneval */
			      treeleaf(&($1));
			    else if ( !UNEVALUATED($3) )
			      /* uneval/abs */
			      treeleaf(&($3));
			    treenode(DIVIDE_OP,&($1),&($3),&($$));
			  }
			    if ($1.reltype && $3.reltype)
			       EXPRERR($$,"Illegal division");
			    $$.reltype = $1.reltype ? $1.reltype: $3.reltype;
			}
		}
	|	exprX  AMP  term = {
		  if ( !ABSOLUTE($1) || !ABSOLUTE($3) )
		    EXPRERR($$,"Illegal logical and");
		  $$ = $1;
		  $$.expval &= $3.expval;
		}
	|	exprX  OR  term = {
		  if ( !ABSOLUTE($1) || !ABSOLUTE($3) )
		    EXPRERR($$,"Illegal logical or");
		  $$ = $1;
		  $$.expval |= $3.expval;
		}
	|	exprX  RSHIFT  term = {
		  if ( !ABSOLUTE($1) || !ABSOLUTE($3) )
		    EXPRERR($$,"Illegal right shift");
		  $$ = $1;
		  $$.expval >>= $3.expval;
		}
	|	exprX  LSHIFT  term = {
		  if ( !ABSOLUTE($1) || !ABSOLUTE($3) )
		    EXPRERR($$,"Illegal left shift");
		  $$ = $1;
		  $$.expval <<= $3.expval;
		}
	|	exprX  MOD  term = {
		  if ( !ABSOLUTE($1) || !ABSOLUTE($3) )
		    EXPRERR($$,"Illegal modulo division");
		  $$ = $1;
		  $$.expval %= $3.expval;
		}
	|	exprX  XCLAIM  term = {
		  if ( !ABSOLUTE($1) || !ABSOLUTE($3) )
		    EXPRERR($$,"Illegal bang");
		  $$ = $1;
		  $$.expval &= ~($3.expval);
		}
	|	exprX  HAT  term = { /* ??? */
		  $$.symptr = $1.symptr;
		  $$.exptype = $3.exptype;
		  $$.reltype = $3.reltype;
		  $$.expval = $1.expval;
		  $$.unevaluated = 0;
		}
	;

term	:	ID = {
	
		  if (($$.exptype = get_sym_exptype($1)) == ABS) {
		    $$.expval = $1->value;
		    $$.symptr = NULLSYM;
		  }
		  else {  
			if ($1 == dot){
			   $$.symptr = get_temp_sym();
			   $$.exptype = get_sym_exptype($$.symptr);
			}
			else
		    	   $$.symptr = $1;
		        $$.expval = 0L;
		 }
		 $$.reltype = NORELTYPE;
		 $$.unevaluated = 0;
		}
	|	ID RELTYPE = {
		  if (($$.exptype = get_sym_exptype($1)) == ABS) {
		    $$.expval = $1->value;
		    $$.symptr = NULLSYM;
		  }
		  else {
			if ($1 == dot){
			   $$.symptr = get_temp_sym();
			   $$.exptype = get_sym_exptype($$.symptr);
			}
			else
		    	   $$.symptr = $1;
		        $$.expval = 0L;
		  }
		  $$.reltype = $2;
		  $$.unevaluated = 0;
		}
	|	NUMBER = {
		  $$.exptype = ABS;
		  $$.reltype = NORELTYPE;
		  $$.expval = $1;
		  $$.symptr = NULLSYM;
		  $$.unevaluated = 0;
		}
	|	MINUS  term = {
		  /* expression must eventually be absolute */
		  if ( ABSOLUTE($2) ) {
		    $$ = $2;
		    $$.expval = - $2.expval;
		  } else if ( RELOCATABLE($2) ) {
		    EXPRERR($$,"Illegal unary minus");
		  } else { /* build tree */
		    if ( UNDEFINED($2) )
		      treeleaf(&($2));
		    /* und operand has become uneval */
		    if ( UNEVALUATED($2) )
		      treenode(UMINUS_OP,(rexpr *) NULL,&($2),&($$));
		    $$.reltype = NORELTYPE;/* can't handle reltype now */
		  }
		}
	|	LB  exprX  RB = {
		  $$ = $2;
		}
	|	OFFPART  term = {
		  $$ = $2;
		  $$.reltype |= LO32TYPE;
		}
	|	SEGPART  term = {
		  $$ = $2;
		  $$.reltype |= HI12TYPE;
		}
	;

%%

char yytext[1026];	/* must be the same size as "file" in errors.c */

static short type[] = {
	EOF,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	SP,	NL,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	SP,	XCLAIM,	QUOTE,	SHARP,	DOLLAR,	PERCNT,	AMP,	SQ,
	LP,	RP,	STAR,	PLUS,	COMMA,	MINUS,	ALPH,	SLASH,
	DIG,	DIG,	DIG,	DIG,	DIG,	DIG,	DIG,	DIG,
	DIG,	DIG,	COLON,	SEMI,	LT,	EQ,	GT,	QUEST,
	AT,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
	ALPH,	ALPH,	ALPH,	LB,	ESCAPE,	RB,	HAT,	ALPH,
	GRAVE,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
	ALPH,	ALPH,	ALPH,	0,	OR,	0,	0,	0,
};

static int
yylex()
{
register short	c,
		ctype;
register char	*yychrptr;
register int	insttype;	/* temporary holding place for inst. type */
symbol	 	*symptr;
instr		*instptr;
static short	mnemseen = NO; /* indicates if mnemonic seen yet */
short		base;
long		val;
int		escaped = 0 ;

	while (type[(c = getc(fdin)) + 1] == SP);

	switch (ctype = type[c+1]) {
	  case ALPH:
	    yychrptr = yytext;
	    do {
	      if (yychrptr < &yytext[sizeof(yytext)])
	      	*yychrptr++ = c;
	    } while ((type[(c = getc(fdin))+1] == ALPH) || (type[c+1] == DIG));

	    *yychrptr = '\0';

	    while (type[c + 1] == SP)
	      c = getc(fdin);
	    ungetc(c,fdin);
	    if ((c == ':') || mnemseen || (c == '=')) {
	      /*	
	       * Found a label or the lhs of an assignment or 
	       * mnemonic already seen: lookup symbol
	       */
	      if ((symptr = lookup(yytext,INSTALL)) == NULL)
		return(ERR);
	      if (c == '=')
		mnemseen = YES; /* set for next time */
	      yylval.uusymptr = symptr;
	      return(ID);
	    } else {
	      /* Found mnemonic: lookup instruction */
	      instptr = instlookup(yytext);
	      if ((instptr == NULL) || (instptr->tag == 0)) {
		yyerror("Illegal mnemonic");
		return(ERR);
	      }
	      mnemseen = YES; /* set for next time */
	      yylval.uuinsptr = instptr;
	      insttype = instptr->val;
	      /*
	       * since we have more than 127 tokens,
	       * any token values>127 placed in
	       * instptr->val will appear negative
	       * because instptr->val is byte-sized,
	       * so we have to compensate 
	       */
	      insttype += (instptr->val > 0) ? 256 : 512;
	      return(insttype);
	    }

	  case DIG:
	    if(special287) {
	      for( yychrptr =yytext;
	           yychrptr < &yytext[sizeof(yytext)-1];
	           ++yychrptr ) {
	        if ((type[c+1] == DIG) ||
	      	    (('a' <= c) && (c <= 'f')) ||
		    (('A' <= c) && (c <= 'F')) ||
		    ((c == 'x') || (c == 'X') ||
		    (c == '.') || (c=='-') || (c == '+'))) {
		  *yychrptr = c ;
		  c = getc(fdin); 
		}
		else {
		  *yychrptr = '\0' ;
		  ungetc(c,fdin);
		  if(c = conv287(special287, yytext, yylval.uufval.fvala)) {
		    yyerror("conv287 errored");
		  }
		  return (FNUMBER);
		}
	      }

	      yyerror("Constant too long");
	      yylval.uufval.fvala[0] = 0 ;
	      yylval.uufval.fvala[1] = 0 ;
	      yylval.uufval.fvala[2] = 0 ;
	      yylval.uufval.fvala[3] = 0 ;
	      yylval.uufval.fvala[4] = 0 ;

	      return(FNUMBER);
	    }

	    val = c - '0';
	    if (c == '0') {
	      c = getc(fdin);
	      if (c == 'x' || c == 'X') {
		base = 16;
	      }
	      else if (c == 'b' || c == 'B') {
		base = 2;
	      }
	      else {
		ungetc(c,fdin);
		base = 8;
	      }
	    } else
	      base = 10;

	    while ((type[(c = getc(fdin))+1] == DIG) ||
		   ((base == 16) && ((('a'<=c) && (c<='f')) ||
		   (('A'<=c) && (c<='F'))))) {
	      if (base == 8)
		val <<= 3;
	      else if (base == 10)
		val *= 10;
	      else if (base == 2)
		val <<= 1;
	      else
		val <<= 4;

	      if ('a' <= c && c <= 'f')
		val += 10 + c - 'a';
	      else if ('A' <= c && c <= 'F')
		val += 10 + c - 'A';
	      else
		val += c - '0';
	    }

	    ungetc(c,fdin);
	    yylval.uulong = val;
	    return(NUMBER);

	  case PERCNT:
	    yychrptr = yytext;
	    while (type[(c = getc(fdin)) + 1] == ALPH | type[c+1] == DIG) {
	      if (yychrptr < &yytext[sizeof(yytext)])
	        *yychrptr++ = c;
	    };
	    *yychrptr = '\0';

	    ungetc(c,fdin);
	    instptr = instlookup(yytext);

	    if ((instptr == NULL) || (instptr->tag == 0)) {
	      yyerror("Illegal register");
	      return(ERR);
	    }

	    yylval.uuinsptr = instptr ;
	    {
	      int val = instptr->val;
	      /*
	       * since we have more than 127 tokens,
	       * any token values>127 placed in
	       * instptr->val will appear negative
	       * because instptr->val is byte-sized,
	       * so we have to compensate 
	       */
	      val += (instptr->val > 0) ? 256 : 512;
	      return(val);
	    }

	  case SEMI:
	    /* reinitialize for next statement */
	    mnemseen = NO;
	    return(SEMI);

	  case SLASH:
	    /* comment; skip rest of line */
	    while (getc(fdin) != '\n');

	    /* same as in case NL */
	    /* FALLTHROUGH */

	  case NL:
	    /* reinitialize for next statement */
	    mnemseen = NO;
	    return(NL);

	  case ESCAPE:
	    /* escaped operator */
	    switch (type[getc(fdin) + 1]) {
	      case STAR:
	      	return(MUL);
	      case SLASH:
	        return(DIV);
	      case PERCNT:
	        return(MOD);
	      default:
	      	return(ERR);
	    }

	  case QUOTE:
	    yychrptr = yytext;
	    yylval.uustrptr = yychrptr;
	    while (c = getc(fdin) ) {
	      if((c == '"') && (!escaped))
	        break;

	      /*
	       * escaped toggle is set when character is
	       * preceeded by backslash
	       * it is used to no find enclosed double quotes
	       */

	      escaped = ( !escaped && (c == '\\') ) ;

	      if ( (c == '\n' ) ) {
	        yyerror("Unterminated string");
	        ungetc(c,fdin) ;
	      	break;
	      }

	      if (yychrptr < &yytext[sizeof(yytext) - 1])
	        *yychrptr++ = c;
	    }
	    *yychrptr = '\0';

	    return(STRING);

	  case GT:
	    switch (getc(fdin)) {
	      case '>':
	    	return (RSHIFT);
	      default:
	    	return (ERR);
	    }

	  case LT:
	    switch (getc(fdin)) {
	      case '<':
	    	return(LSHIFT);
	      case 's':
	    	ctype = SEGPART;
	    	break;
	      case 'o':
	    	ctype = OFFPART;
	    	break;
	      case 'h':
	    	ctype = HIFLOAT;
	    	break;
	      case 'l':
	    	ctype = LOFLOAT;
	    	break;
	      default:
	    	yyerror("Illegal character after '<'");
	    	return(ERR);
	    }
	    if ((c = getc(fdin)) != '>')
	      yyerror("Missing '>'");
	    if (ctype != HIFLOAT && ctype != LOFLOAT)
	      return(ctype);
	    /* accumulate floating point number */
	    while (type[(c = getc(fdin)) + 1] == SP);
	    for (yychrptr = yytext;
	    	yychrptr < &yytext[sizeof(yytext)-1];
	    	++yychrptr)
	    {
	      if (type[c+1]==DIG || c=='.'
	      	|| c=='+' || c=='-'
	      	|| c=='E' || c=='e')
	    	{
	      	*yychrptr = c;
	      	c = getc(fdin);
	        }
	      else {
	      	*yychrptr = '\0';
	      	ungetc(c,fdin);
	      	/* convert to b16 format */
	      	c = atob16f(yytext,&val);
	      	if (c) {
	    	  yyerror("Error in floating point number");
		  yylval.uulong = 0;
	      	}
	        else
	          /* take high or low part */
	          yylval.uulong = (ctype == HIFLOAT) ?
			((val >> 16) & 0x0000FFFFL) : (val & 0x0000FFFFL);
		  return(NUMBER);
	      } /* else */
	    } /* for */

	    yyerror("Floating point number too long");
	    yylval.uulong = 0;
	    return(NUMBER);

	  case AT:
	    c = getc(fdin);
	    for( yychrptr =yytext;
		yychrptr < &yytext[sizeof(yytext)-1];
		++yychrptr ) {
	      if (type[c+1] != ALPH && type[c+1] != DIG)
		break;
	      *yychrptr = c ;
	      c = getc(fdin); 
	    }
	    *yychrptr = '\0' ;
	    ungetc(c,fdin);
	    if(strcmp(yytext,"object") == 0) {
	      yylval.uuattype = STT_OBJECT;
	      return(SYMTYPE);
	    } else if(strcmp(yytext,"function") == 0) {
	      yylval.uuattype = STT_FUNC;
	      return(SYMTYPE);
	    } else if(strcmp(yytext,"no_type") == 0) {
	      yylval.uuattype = STT_NOTYPE;
	      return(SYMTYPE);
	    } else if (strcmp(yytext, "progbits") == 0) {
	      yylval.uuattype = SHT_PROGBITS;
	      return(SECTTYPE);
	    } else if (strcmp(yytext, "nobits") == 0) {
	      yylval.uuattype = SHT_NOBITS;
	      return(SECTTYPE);
	    } else if (strcmp(yytext, "symtab") == 0) {
	      yylval.uuattype = SHT_SYMTAB;
	      return(SECTTYPE);
	    } else if (strcmp(yytext, "strtab") == 0) {
	      yylval.uuattype = SHT_STRTAB;
	      return(SECTTYPE);
	    } else if (strcmp(yytext, "note") == 0) {
	      yylval.uuattype = SHT_NOTE;
	      return(SECTTYPE);
	    } else if (isxdigit(*yytext))  {
	      unsigned long temp;
	      char *ahead=yytext+1;
	      if (*ahead == 'X' || *ahead == 'x') {
		if ( *yytext != '0') 
			return(ERR);
		else
			ahead++;
	      }
              while (isxdigit(*ahead)) ahead++;
	      if (!isspace(*ahead) && *ahead != '\0' 
		  && *ahead != SEMI && *ahead != SLASH)
{
		return(ERR);
}
	      temp = (unsigned long) Strtoul(yytext, (char **)NULL,0);
		yylval.uuattype = temp;
		return(SECTTYPE);
	    } else if (strcmp(yytext, "GOT") == 0) {
		yylval.uuattype = RELGOT32;
		return(RELTYPE);
	    } else if (strcmp(yytext, "GOTOFF") == 0) {
		yylval.uuattype = RELGOTOFF;
		return(RELTYPE);
	    } else if (strcmp(yytext, "PLT") == 0) {
		yylval.uuattype = RELPLT32;
		return(RELTYPE);
	    } else {
	      yyerror("illegal use of @");
	      return (ERR);
	    }
	  case 0:
	    yyerror("illegal character");
	    return(ERR);

	  default:
	    return(ctype);
	}
} /* yylex */

static void
fill(nbytes)
long nbytes;
{
long fillchar;

	if (sectab[dot->sectnum].type != SHT_NOBITS) {
	  fillchar =
    (sectab[dot->sectnum].flags & SHF_EXECINSTR) ? TXTFILL|0x90909000L : FILL;
	  while (nbytes >= 4) {
	  generate(4*BITSPBY,NOACTION,fillchar,NULLSYM);
	  nbytes -= 4;
	  }
	  while (nbytes--)
		generate(BITSPBY,NOACTION,fillchar,NULLSYM);
	}
	else 
	  newdot += nbytes;
} /* fill */

#if 0		/* We may break too many old programs with this */
static void
ckalign(val,directive)
long val;
char * directive;
{
long mod;
	if ((mod = newdot % val) != 0) {
	  sprintf(yytext,"%s must be aligned to a %d-byte boundary",directive,val);
	  if (transition)
	    werror(yytext);	/* Alignment wasn't checked before */
	  else
	    yyerror(yytext);
	}
	if (val > sectab[dot->sectnum].addralign)
	  sectab[dot->sectnum].addralign = val;
} /* alignmt */
#endif 

static void
alignmt(val)
long val;
{
long mod;
	if (val != 0 && val != 1) {
		if ((mod = newdot % val) != 0) 
	  		fill(val - mod);
		if (val > sectab[dot->sectnum].addralign)
	  		sectab[dot->sectnum].addralign = val;
	}
} /* alignmt */

static void
assignmt(sym,expr)
register symbol *sym;
register rexpr *expr;
{
  if (sym == dot) {
    long incr;

    if (ABSOLUTE(*expr))
      yyerror("Assignment to dot must be relocatable");
    else if (UNEVALUATED(*expr) || UNDEFINED(*expr))
       yyerror("expression cannot be evaluated");
    else if ((expr->symptr != NULL) && (sym->sectnum != expr->symptr->sectnum))
       yyerror("Incompatible section numbers");
    else if ((incr = expr->symptr->value + expr->expval - sym->value) < 0)
      yyerror("Cannot decrement '.'");
    else
      fill(incr);
  } else {
    if (!BIT_IS_ON(sym,BOUND)) {
      sym->binding = STB_LOCAL;
      sym->flags &= ~GO_IN_SYMTAB;
    }
    if (!UNDEF_SYM(sym) && !BIT_IS_ON(sym,SET_SYM)){
       yyerror("multiply defined symbol");
      (void) fprintf(stderr,"\t\t...%s\n",sym->name);
    }
    if (ABSOLUTE(*expr)) {
      sym->value = expr->expval;
      sym->sectnum = SHN_ABS;
    } else if (RELOCATABLE(*expr)) {
	sym->value = expr->expval;
	remember_set_or_size(sym,expr->symptr,SETTO_SYM,REMEMBER_SET);
    } else if (UNDEFINED(*expr)) {
      remember_set_or_size(sym,expr->symptr,SETTO_SYM,REMEMBER_SET);
      sym->value = expr->expval;
    } else { /* (UNEVALUATED(*expr)) */
      remember_set_or_size(sym,expr->symptr,SETTO_XPTR,REMEMBER_SET);
    }
  }
    sym->flags |= SET_SYM;
} /* assignmt */

static void
obsolete_dir(s,warn)
char *s;
enum obsdir_warn warn;
{
  if (transition) {
    if (warn == WARN)
      werror(s);	/* warn about some obsolete directives in translation mode */
  } else
    yyerror(s);		/* obsolete directives always errors in default mode */
} /* obsolete dir */

static void
ck_nonneg_absexpr(expr,name)
rexpr *expr;
char *name;
{
  if (!ABSOLUTE(*expr)) {
    strcpy(yytext,name);
    strcat(yytext," not absolute");
    yyerror(yytext);
  }
  if (expr->expval < 0) {
    strcpy(yytext,name);
    strcat(yytext," has negative value");
    yyerror(yytext);
  }
} /* ck_nonneg_absexpr */

static symbol *
get_temp_sym()
{

	static int uniq = 0;
	char newlab[20];
	register symbol * sym;
	(void) sprintf(newlab,".DOT-%d",uniq++);
	sym = lookup(newlab,INSTALL);
	sym->value = dot->value;
	sym->sectnum = dot->sectnum;
	sym->binding = STB_LOCAL;
	sym->flags &= ~GO_IN_SYMTAB;
	if (sectab[dot->sectnum].flags & SHF_EXECINSTR)
		deflab(sym);
	return(sym);
} /* get_temp_sym */
