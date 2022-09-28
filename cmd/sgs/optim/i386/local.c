/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)optim:i386/local.c	1.2.4.1"

#include "optim.h" 	/* includes "defs" */
#include "optutil.h"
#include "sgs.h"
#include "paths.h"
#include "regal.h"
#include "storclass.h"
#include "debug.h"
#include <malloc.h>

#define ASDATE ""

static char * linptr, * line;	/* pointers to current statement linptr
				   moves, line doesn't */
static int opn;			/* current operand, starting at 1 */ 
static int lineno = IDVAL;
static int first_src_line = 0;	/* Value is set each time we see a
				   ..FS.. label */

#ifdef STATS			/* statistics gathering */
/* Except for ndisc, these variables are not presently used */
/* but should be updated at the appropriate places in the future in */
/* order to do statistics gathering. */
int nusel = 0;		/* semantically useless instructions */
int nspinc = 0;		/* useless sp increments */
int nmc = 0;		/* move followed by compare */
int nmal = 0;		/* move followed by arithmetic or logical */
int nredcmp = 0;	/* redundant compares */
int nadpsh = 0;		/* addw or subw followed by push */
int nadmsh = 0;		/* addw or subw followed by mov */
int nadmv = 0;		/* replace addw or subw by mov */
extern int ndisc;		/* instructions discarded */
				/* only ndisc is properly updated at this time. */
#endif /* STATS */

static int numauto;		/* number of bytes of automatic vars. */
static int numnreg;		/* number of registers */

int optmode = ODEFAULT;		/* optimization mode */
enum CC_Mode ccmode = Transition;/* compilation mode */      

#ifdef IMPIL
boolean zflag = false;		/* debug flag for in--line expansion */
#endif /* IMPIL */
#ifdef LIVEDEAD
extern void	ldanal();
#endif

#define	BIGREGS	16	/* Any number that signifies many registers */
#define	BIGAUTO	(1<<30)	/* Any number that signifies many auto vars */
#define NEWSIZE	(11+1+4+1+4+1+1+1+1)	/* for "ddddddddddd(%exx,%exx,8)\0" */


static char *optbl[] = {
	"aaa", "aad", "aam", "aas", "adcb",
	"adcl", "adcw", "addb", "addl", "addw",
	"andb", "andl", "andw", "arpl", "bound",
	"call", "cbtw", "clc", "cld", "cli",
	"clrb", "clrl", "clrw", "cltd", "cmc",
	"cmpb", "cmpl", "cmpw", "cts", "cwtd",
	"cwtl", "daa", "das", "decb", "decl",
	"decw", "divb", "divl", "divw", "enter",
	"esc", "f2xm1", "fabs", "fadd", "faddl",
	"faddp", "fadds", "fbld", "fbstp", "fchs",
	"fclex", "fcom", "fcoml", "fcomp", "fcompl",
	"fcompp", "fcomps", "fcoms", "fdecstp", "fdiv",
	"fdivl", "fdivp", "fdivr", "fdivrl", "fdivrp",
	"fdivrs", "fdivs", "ffree", "fiadd", "fiaddl",
	"ficom", "ficoml", "ficomp", "ficompl", "fidiv",
	"fidivl", "fidivr", "fidivrl", "fild", "fildl",
	"fildll", "fimul", "fimull", "fincstp", "finit",
	"fist", "fistl", "fistp", "fistpl", "fistpll",
	"fisub", "fisubl", "fisubr", "fisubrl", "fld",
	"fld1", "fldcw", "fldenv", "fldl", "fldl2e",
	"fldl2t", "fldlg2", "fldln2", "fldpi", "flds",
	"fldt", "fldz", "fmul", "fmull", "fmulp",
	"fmuls", "fnclex", "fninit", "fnop", "fnsave",
	"fnstcw", "fnstenv", "fnstsw", "fpatan", "fprem",
	"fptan", "frndint", "frstor", "fsave", "fscale",
	"fsetpm", "fsqrt", "fst", "fstcw", "fstenv",
	"fstl", "fstp", "fstpl", "fstps", "fstpt",
	"fsts", "fstsw", "fsub", "fsubl", "fsubp",
	"fsubr", "fsubrl", "fsubrp", "fsubrs", "fsubs",
	"ftst", "fwait", "fxam", "fxch", "fxtract",
	"fyl2x", "fyl2xp1", "hlt", "idivb", "idivl",
	"idivw", "imulb", "imull", "imulw",
	"inb", "incb", "incl", "incw", "inl",
	"ins", "int", "into", "inw", "iret",
	"ja", "jae", "jb", "jbe", "jc",
	"jcxz", "je", "jg", "jge", "jl",
	"jle", "jmp", "jna", "jnae", "jnb",
	"jnbe", "jnc", "jne", "jng", "jnge",
	"jnl", "jnle", "jno", "jnp", "jns",
	"jnz", "jo", "jp", "jpe", "jpo",
	"js", "jz", "lahf", "lar", "lcall",
	"lds", "leal", "leave", "leaw", "les",
	"lgdt", "lidt", "ljmp", "lldt", "lmsw",
	"lock", "loop", "loope", "loopne", "loopnz",
	"loopz", "lret", "lsl", "ltr", "movb",
	"movl", "movsbl", "movsbw", "movswl", "movw",
	"movzbl", "movzbw", "movzwl", "mulb", "mull",
	"mulw", "negb", "negl", "negw", "notb",
	"notl", "notw", "orb", "orl", "orw",
	"outb", "outl", "outs", "outw", "popa",
	"popf", "popl", "popw", "pusha", "pushf",
	"pushl", "pushw", "rclb", "rcll", "rclw",
	"rcrb", "rcrl", "rcrw", "rep", "repnz",
	"repz", "ret", "rolb", "roll", "rolw",
	"rorb", "rorl", "rorw", "sahf", "salb",
	"sall", "salw", "sarb", "sarl", "sarw",
	"sbbb", "sbbl", "sbbw", "scab", "scal",
	"scaw", "scmpb", "scmpl", "scmpw", "sgdt",
	"shlb", "shldl", "shldw", "shll", "shlw",
	"shrb", "shrdl", "shrdw", "shrl", "shrw",
	"sidt", "sldt", "slodb", "slodl", "slodw",
	"smovb", "smovl", "smovw", "smsw", "sstob",
	"sstol", "sstow", "stc", "std", "sti",
	"str", "subb", "subl", "subw", "testb",
	"testl", "testw", "verr", "wait", "xchgb",
	"xchgl", "xchgw", "xlat", "xorb", "xorl",
	"xorw"
};

# define numops (sizeof(optbl) / sizeof(char *)) /* number of mnemonics */

extern char *getenv();
#ifdef DEBUG
static char *get_label();
#endif
extern char *tempnam();
static FILE *tmpopen();
static FILE *stmpfile;	/* Temporary storage for switch tables that are in the text
			 * section. The tables are collected and printed at
			 * the end of a function. */
static char tmpname[50];	/* name of file for storing switch */
static FILE *atmpfile; /* Temporary storage for input while scanning for presence of
			 * #ASM in function if aflag on */
static char atmpname[50];

int asmflag = false;	/* indicates whether an 'asm' has been encountered */
static long asmotell;	/* location in the output file where the last function ended */
int aflag = false;	/* indicates whether -a flag was entered, if true
			   no atempt will be made to optimize functions
			   containing ASMs */

			/* Next two flags apply globally -- set on
			   command line by  -Sr, -Se */
static int never_register_allocation = false;
static int never_enter_leave = false; 

			/* Next two flags apply on a per function basis:
			   suppress ( only under -Xt ) register allocation
			   if fn contains a setjmp.  Similarly, suppress
			   enter/leave removal if a function contains
			   a longjmp (for -Xa and -Xt as well) */
static int suppress_register_allocation = false;
static int suppress_enter_leave = false;

int tflag = false; 	/* suppress removal of redundant loads (rmrdld())? */
int Tflag = false; 	/* trace removal of redundand loads routine? */
static boolean identflag = false;/* output .ident string? */

static boolean inswitch = false;/* currently in switch table */

enum Section section=CStext, prev_section;      /* Control sections. */

#ifdef IMPIL
boolean swflag = false;		/* switch table appears in function */
extern void pcdecode();
#endif /* IMPIL */
int rvregs = 1;			/* # registers used to hold return value */
#ifdef pre50
#ifdef IMPREGAL
boolean comp_provided_weights = false;
				/* indicates comp provided #REGAL weights */
#endif /* IMPREGAL */
#endif
/* function declarations */
extern int unlink();

#ifdef IMPIL
extern void ilinit();		/* initialization for in-line substitution */
extern void ilmark();
extern void ilstat();
extern void ilfile();
#endif /* IMPIL */
#ifdef IMPREGAL
extern struct regal *lookup_regal();
extern int raoptim();
#endif /* IMPREGAL */
extern void rmrdld();
extern void wrapup();	/* print unprocessed text and update statistics file */
extern void dstats();

extern char *getstmnt(); /* also called from debug.c */
/* Input function (getline()), lifted from HALO */
static char *ExtendCopyBuf(); /* Used only by getstmnt */

int plookup();			/* look up pseudo op code */

static void parse_com();
#ifdef IMPREGAL
static void parse_regal();		/* Parses a #REGAL line. */
static void parse_alias();		/* Parses a #ALIAS line. */
#endif /* IMPREGAL */
static void parse_op();
static void parse_pseudo();
static int remove_enter_leave();
static enum Section parse_section();	/* Parse the argument to .section. */
static int lookup();			/* look up op code ordinal */
static void asmopen();	/* opens the temp file for storing input while looking for 'asm' */
int putasm();	/* writes to temp file for 'asm' processing */
static void asmchk();	/* checks for 'asm' in files and undoes code movement */
static void setautoreg();	/* set indicator for number of autos/regs */
static void reordtop();
static void reordbot();
static void copyafter();
static void putstr();
static void prstr();
static void filter();
void peep();


	void
yylex()
{
    extern int cflag;			/* enable/disable common tail */
    register char * s;			/* temporary string ptr */
    register char * start;		/* another temporary string ptr */
    int first_label = 1;

#ifdef IMPCOMTAIL
    if( optmode == OSIZE )
      cflag = 0;
    else
#endif /* IMPCOMTAIL */
      cflag = -1;	/* turn off common tail merging:  improve
			** speed, not space. */

#ifdef IMPIL
    ilinit();
#endif /* IMPIL */

    if( aflag ) asmopen();		/* open temp file for asm treatment */
    line = linptr = getstmnt();
    while ( linptr 
		&& ( !aflag || putasm(linptr) ))
    {
	if (section == CSdebug) {	/* hook to handle an entire debug
					   section.  We do this, because
					   parse_debug wants to do its own
					   I/O and parsing */
	    parse_debug(linptr);
	}		
	else {
	    switch (*linptr)		/* dispatch on first char in line */
	    {
	    case '\n':			/* ignore blank lines */
	    case '\0':
	        break;
	        
	    case CC:			/* check for special comments */
	        parse_com(linptr);
	        break;
    
	    default:			/* label */
	        s = strchr(linptr,':');	/* find : */
	        if (s == NULL)		/* error if not found */
	          fatal("Bad input format\n");
	        *s++ = '\0';		/* change : to null, skip past */
	        if (section == CStext) {
		    if(strncmp(linptr,"..LN",4) == 0)  /* linenumber label */
		        lineno = atoi(linptr+4);
		    else if(strncmp(linptr,"..FS",4) == 0)  
		    /* 1st source line for function */
		        first_src_line = atoi(linptr+4);
		    else {
			if(first_label) {
			    save_text_begin_label(linptr);
			    first_label = 0;
			}
	                applbl(linptr,s-linptr);  /* append label node */
	                lastnode->uniqid = IDVAL;
		    }
	        }
	        else if(section != CSline) {
	            printf("%s:\n",linptr);
	            addref(linptr,(unsigned int)(s-linptr));
		}
		linptr = s;
	        continue;			/* next iteration */
    /* handle more usual case of line beginning with space/tab */
	    case ' ':
	    case '\t':
	        s = linptr;
			    /* linptr now points to beginning of line after label */
	        SkipWhite(s);
	    	
	        switch(*s)			/* dispatch on next character */
	        {
	        case '\0':			/* line of white space */
	        case CC:			/* comment */
	        case '\n':			/* new line */
		    break;			/* ignore */
	    	
	        case '.':			/* start of pseudo-op */
		    parse_pseudo(s);	/* do pseudo-ops */
		    break;
    
	        default:			/* normal instruction not */
		    if (section != CStext) { /* in .text section this is weird case */
			if (section != CSline) printf("\t%s\n",s); /* just write line */
		    }
		    else			/* normal instruction in text */
		    {
		        char lastchar;
		        int opc, m;
		        for (start = s; isalnum(*s); s++)
			    ;		/* skip over instruction mnemonic */
					    /* start points to mnemonic */
    
		        lastchar = *s;
		        *s = '\0';	/* demarcate with null */
    
		        if ((opc = lookup(start,&m)) == OTHER)
			    saveop(0,strdup(start),0,opc);
					    /* save funny instruction */
		        else
			    saveop(0,optbl[m],0,opc);
					    /* save normal inst */
		        *s = lastchar;
    
		        SkipWhite(s);
			/* Check if this is a call to a local label,
			   if so we need to prevent it from being
			   removed.  This only arises when the compiler
			   is generating PIC.
			   
			   Also check for call to setjmp if compiling
			   in transition mode (-Xt).  Such calls must
			   disable register allocation. */

			if( opc == CALL || opc == LCALL ) {
			    if(*s == '.' ) {
			       char *p = s+1; /* point at 'L' */
			       while ( isalnum(*p) ) p++;
			       *p = '\0';
			       addref(s,(unsigned int)(p-s+1));
			    }
			    else if(ccmode == Transition && *s == 's' && strcmp(s,"setjmp")==0)
				suppress_register_allocation = true;
			    else if(*s == 'l' && strcmp(s,"longjmp") == 0)
				suppress_enter_leave = true;
			    else if(*s == 'a' && strcmp(s,"alloca") == 0)
				suppress_enter_leave = true;
			    /* hack to avoid confusing this function,
			       which makes assumptions about the frame
			       pointer of its caller */
			}
		        opn = 1;		/* now save operands */
		        parse_op(s);
		    }
		    break;
	        }   /* end space/tab case */
	    break;
	    }	/* end first character switch */
	}	/* end if (section ...) ... else */
	line = linptr = getstmnt();
    }   /* end while */
    return;				/* just exit */
}

	static void
parse_com(s)
register char *s;
{
 extern void fatal();
 extern boolean swflag;

	switch(*++s){
	case 'A':	/* #ALIAS, #ASM, or #ASMEND ? */
#ifdef IMPREGAL
		if(strncmp(s,"ALIAS",5) == 0){
			parse_alias(s+5);
			break;
		}
		else
#endif /* IMPREGAL */
		if(strncmp(s,"ASM",3) != 0) 
			break;
		if(strncmp(s+3,"END",3) == 0)
			/* since #ASM processing chews up everything to #ASMEND ... */
			fatal("parse_com: unbalanced asm.\n");
		asmflag = true;		/* #ASM appears in function */
		/* here if #ASM<stuff> */
		s = linptr;
		do {
			while(*linptr != '\0') linptr++;
			/* *linptr++ = '\0'; take this out (psp) */
			saveop(0,strdup(s),0,ASMS);
			if(strncmp(s+1,"ASMEND",6) == 0)
				break;
    		} while ( (line=linptr=getstmnt()) != NULL
			&& ( !aflag || putasm(s) ));
		if(linptr == NULL)
			fatal("parse_com: unbalanced asm.\n");
		break;
#ifdef IMPREGAL
	 case 'L':	/* #LOOP ? */
		if(strncmp(s,"LOOP",4) != 0) break;
		s += 4;		/* enter it into the list. */
		SkipWhite(s);	/* Skip white space to retrieve arg.	*/
		saveop(0,s,3,LCMT);	/* append loop-cmt node with its arg */
		break;
	 case 'R':	/* #REGAL ? */
		if(strncmp(s,"REGAL",5) != 0) break;
		parse_regal(s+5);	/*Read NAQ info in #REGAL stmt*/
		break;
#endif /* IMPREGAL */
	 case 'S':	/* #SWBEG or #SWEND ? */
		if(strncmp(s,"SWBEG",5) == 0)
		  {
			inswitch = true; 	/* Now in a switch table. */
#ifdef	IMPIL
			swflag = true;
#endif /* IMPIL */
		      }
		else if(strncmp(s,"SWEND",5) == 0)
			inswitch = false;	/*Out of switch table.*/
		break;
#ifdef IMPIL
	 case 'T':	/* #TMPSRET ? */
		if (lastnode == NULL)
		  	break;
		if (strncmp(s,"TMPSRET",7) == 0)
		  	if (lastnode->op == PUSHL)	/* identify push of addr */
				saveop(2,s,7,TSRET);	/* for struct funct ret. */
		break;
#endif /* IMPIL */
 	 case 'V':	/* #VOL_OPND ? */
		if(strncmp(s,"VOL_OPND", 8) != 0) break;
		s += 8;
		SkipWhite(s);
		if (lastnode == NULL)
		  	break;
		while( *s != '\0' && *s != '\n' ){
			/* set volatile bit in operand */
			mark_vol_opnd(lastnode,*s - '0');
			s++;
			SkipWhite(s);
			if( *s == ',' ){ 
				s++;
				SkipWhite(s);
			}
		}
		break;
	} /* end of switch(*++s) */
}

#ifdef IMPREGAL
	static void
parse_regal( p ) 		/* read #REGAL comments */
register char *p;

{
#ifdef pre50
 extern boolean comp_provided_weights;
 int estim;
#endif
 register struct regal *r;
 extern void fatal();
 char *q;
 char *name;
 int len,rt;

	/* the formats recognized are:
	 * 1) #REGAL <wt> NODBL
	 * 2) #REGAL <wt> {AUTO,PARAM,EXTERN,EXTDEF,STATEXT,STATLOC} <name> <size> [FP]
	 *
	 * However only the following subset is used by the 386 optimizer:
	 * #REGAL <wt> {AUTO,PARAM} <name> <size>
	 * and the other formats are ignored.
	 */

			/* scan to estimator and read it */
	SkipWhite(p);
#ifdef pre50
	estim = strtol( p, &q, 0 );
#else
	strtol( p, &q, 0 );
#endif
	if(p == q)
		fatal("parse_regal: missing weight field\n");
	p = q;


			/* scan to storage class and read it */
	SkipWhite(p);
	rt = SCLNULL;
	switch(*p){
	case 'A':
		if(strncmp(p,"AUTO",4) == 0)
			rt = AUTO;
		break;
	case 'E':
		if(strncmp(p,"EXTDEF",6) == 0)
			return;			/* ignore */
		else if(strncmp(p,"EXTERN",6) == 0)
			return;			/* ignore */
		break;
	case 'N':
		if(strncmp(p,"NODBL",5) == 0)
			return;			/* ignore */
		break;
	case 'P':
		if(strncmp(p,"PARAM",5) == 0)
			rt = PARAM;
		break;
	case 'S':
		if(strncmp(p,"STATEXT",7) == 0)
			return;			/* ignore */
		else if(strncmp(p,"STATLOC",7) == 0)
			return;			/* ignore */
		break;
	}
	if( rt == SCLNULL )
		fatal( "parse_regal:  illegal #REGAL type:\n\t%s\n",p);

			/* scan to name and delimit it */
	FindWhite(p);
	SkipWhite(p);
	name = p;
	FindWhite(p);
	*p++ = '\0';
	if ( ((q = (strchr(name,'('))) == NULL) ||
	     (strncmp(q,"(%ebp)",6) != 0) )
	    fatal("parse_regal:  illegal name in #REGAL\n");
			/* only "nnn(%ebp) currently allowed */

			/* scan to length in bytes */
	SkipWhite(p);
	len = strtol( p, &q, 0 );
	if(p == q)
		fatal("parse_regal: missing length\n");
	p = q;

			/* read floating point indicator */
	SkipWhite(p);
	if( (p[0] != '\0' && p[0] == 'F') &&
	   (p[1] != '\0' && p[1] == 'P') )
		return;			/* ignore */

			/* install regal node */
	if ((r = lookup_regal(name,true)) == NULL)
	    fatal("parse_regal:  trouble installing a regal\n");
	r->rglscl = rt;
 	r->rgllen = len;
#ifdef pre50
	if (estim) {
		/* If the compiler provides a non-zero weight in any */
		/* #REGAL statement, then we assume that it has computed */
		/* all the weight information.  Otherwise we must compute */
		/* the information in Estim(). */
		comp_provided_weights = true;
		r->rglestim = estim;
	}
#endif
 return;
      }

	static void
parse_alias(s)
register char *s;

{
 extern struct regal *new_alias();
 char *name,*t;
 int len;
 register struct regal *a;

			/* scan to name and delimit it */
 SkipWhite(s);
 name = s;
 FindWhite(s);
 *s++ = '\0';
 if (strchr(name,'(') == NULL)
   return;		/* catch only AUTOs and PARAMs - "n(%ebp)" */


			/* scan to length in bytes */
 SkipWhite(s);
 len = strtol( s, &t, 0 );
 if(s == t)
   fatal("parse_alias: missing length\n");
 s = t;

			/* read floating point indicator */
 SkipWhite(s);
 if( s[0] == 'F' && s[1] == 'P' )
   return;

			/* append node to list of aliases */
 a = new_alias();
 a->rglname = strdup(name);
 a->rgllen = len;

 return;
}
#endif	/* IMPREGAL */

	static void
parse_op(s)
    register char *s;
{
    register char *t;
    register more, prflg;

    more=true;
    prflg=false;

    while (more) { /* scan to end of operand and save */	

	SkipWhite(s);
	t = s;		/* remember effective operand start */

	while(*s && (*s != ',') ) { /* read to end of this operand */
	    switch ( *s++ ) {
	    case CC: /* process comment */
		if( isret( lastnode ) ) rvregs = *s - '0';
		*s = *(s-1) = '\0'; /* that's what the old code did ??? */
		break;
	    case '(':
		prflg = true;
		break;
	    case ')':
		prflg = false;
		break;
	    }
	    if(prflg && (*s==',')) s++;
	}

	if(*s ) *s = '\0';	/* change ',' to '\0' */
	else more = false;	/* found the end of instruction */
	/* now s points to null at end of operand */
	saveop(opn++, t, (++s)-t, 0);
	if (is_label_text(t) && inswitch)
	    addref(t,(unsigned int)(s-t));
    } /* while(more) */
    lastnode->uniqid = lineno;
    lineno = IDVAL;
}



	static void
parse_pseudo (s)
	register char *s;		/* points at pseudo-op */
{
#define NEWSECTION(x) prev_section=section; section=(x);
	void add_enter_leave();		/* in optutil.c */
	register int pop;		/* pseudo-op code */
	char savechar;			/* saved char that follows pseudo */
	char * word = s;		/* pointer to beginning of pseudo */
	int m;
	enum Section save;		/* for swaps */
	int auto_elim=0;		/* true if function has no autos
					   or all autos have been placed
					   into registers by raoptim
					*/


	FindWhite(s);			/* scan to white space char */
	savechar = *s;			/* mark end of pseudo-op */
	*s = '\0';
	pop = plookup(word);		/* identify pseudo-op */
	*s = savechar;			/* restore saved character */

	if(section == CSline) {
	    if (pop != (int) PREVIOUS)  return;
	    else if (! init_line_flag) init_line_section();
	    /* assume .line entries are simple mindedly bracketted
	       by .section .line ... .previous 
	       The .previous will get printed by the code that
	       comes next. */
	}
	if (section != CStext) {
	    switch(pop) { /* check for section changes and .long,
			     otherwise just print it. */
	    case TEXT:
	        NEWSECTION(CStext);
	        printf("%s\n", line);
		/* don't print: is this right??? (psp) */
		break;
	    case PREVIOUS:
	        save=section;
	        section=prev_section;
	        prev_section=save;
	        printf("%s\n", linptr); /* flush to output */
		break;
	    case SECTION:
	        printf("%s\n", line);
	        prev_section=section;
	        section=parse_section(s);
		break;
	    case LONG: /* FALLTHRU */
		break;
	    default:
		printf("%s\n",line);
		break;
	    } /* switch */
	    if(pop != (int) LONG) return;
	} /* non text sections done ( except for .long ) */

	switch (pop) {			/* dispatch on pseudo-op in text */
					/* and on .long in any section */
		case TEXT:
		    NEWSECTION(CStext);
		    break;
		case BYTE:
			printf("%s\n",line);
			break;
		case LOCAL:
		case GLOBL:
			appfl(line, strlength(line)); /* filter node - why */
			break;
		case FIL:
		case BSS: /* assume .bss has an argument,
			     this will get .bss <no arg> wrong */
			printf("%s\n", line);
			break;
		case ALIGN:
			m=strlength(line);
			if (inswitch) {
				saveop(0, line+1, m-1, OTHER);
				opn = 1;
				lastnode->uniqid = lineno;
				lineno = IDVAL;
			}
			else
				appfl(line, m);
			break;
		case SET:
			m=strlength(line);
			appfl(line, m);
			break;
		case DATA:
			printf("%s\n", line);
			NEWSECTION(CSdata);
			break;
		case SECTION:
			printf("%s\n", line);
			prev_section=section;
			section=parse_section(s);
			break;
		case PREVIOUS:
			save=section;
			section=prev_section;
			prev_section=save;
			printf("%s\n", linptr); /* flush to output */
			break;
		case LONG:
			SkipWhite(s);

/* we have to deal with whether .long is within a switch (SWBEG/SWEND) or
** not, and whether or not it appears in a data section.
*/

			if (inswitch) {		/* always add reference */
			    char *t;
			    if(t = strchr(s,'-')) {
				/* Assume we are looking at .long .Lxx-.Lyy,
				   where .Lyy is the target of a call ( PIC
				   code ).  So we make the first label
				   hard.  The second label is handled
				   when the call is parsed. */
				*t = '\0';
				addref(s,(unsigned int)(t-s+1));
				*t = '-';
			    }
			    else
				addref(s,(unsigned int)(strlen(s)+1));
					/* we assume only one arg per .long */
			}

			if (section != CStext) 	/* not in .text, flush to output */
				printf("%s\n",linptr); /* print line */
			else if (inswitch)  	/* text and switch */
				putstr(linptr);	/* flush to special file */
			else {			/* text, not in switch */
				saveop(0,".long",6,OTHER);
				opn = 1;	/* doing first operand */
				parse_op(s);
			}
			break;

		/*
		* ELF new pseudo_op
		*
		* do optimizations, then spit out the 
		* input .size line which is assumed to be
		* of the form .size foo,.-foo ( no intervening white space )
		*/
		case SIZE: /* For clarity, this code should be placed in a separate function. */
		{
			char * ptr;
			SkipWhite(s);
			ptr=strchr(s,',');
			if(ptr == NULL) { /* not the right format */
				printf("%s\n",line);
				break;
			}
			if(*(ptr+1) != '.' || *(ptr+2) != '-') {
				printf("%s\n",line);
				break;
			}
			*ptr = '\0'; /* s points to function name, now */
			if(strcmp(s,ptr+3) != 0) { 
			/* Some kind of .size in text unknown to us. */
				*ptr = ',';
				printf("%s\n",line);
				fatal("parse_pseudo(): unrecognized .size directive\n");
				break;
			}
			if (first_src_line != 0) {
			    print_FS_line_info(first_src_line,s);
			    first_src_line = 0;
			}
			*ptr = ',';
		}
			printf("	.text\n");
#ifdef DEBUG
			{
				void ratable();
				static int d;
				int min, max;
				char * str;
				++d;
				str=getenv("max");
				if (str)
					max=atoi(str);
				else
					max=9999;
				str=getenv("min");
				if (str)
					min=atoi(str);
				else
					min=0;
				if (d> max || d < min) {
					ratable();
					goto noopt;
				}
				else if (min)
					fprintf(stderr,"%d %s ", d,get_label());
			}
#endif
			if( !asmflag || !aflag  ) {
				reordtop();
				setautoreg();	/* set numnreg and numauto */
				auto_elim= !numauto;
#ifdef IMPREGAL
				if(!suppress_register_allocation) 
				 numnreg = raoptim(numnreg, numauto, &auto_elim);
#endif /* IMPREGAL */
#ifdef IMPIL
				ilmark();
#endif /* IMPIL */
				filter();
				if(suppress_register_allocation) {
					void ratable();
					ratable();		
					suppress_register_allocation = never_register_allocation;
				}
				optim(); /* do standard improvements */
				peep();	 /* do peephole improvements */
				rmrdld();/* remove redundant loads   */
					 /* make one more pass	     */
#ifdef LIVEDEAD
				ldanal();
#endif
				peep();	 /* do peephole improvements */
#ifdef IMPIL
				ilstat(numnreg, numauto);
#endif /* IMPIL */
				if(suppress_enter_leave) {
				    auto_elim = false;
				    suppress_enter_leave = never_enter_leave; /* reset */
				}
				if (auto_elim ) {
					/* conditionally remove the enter from the
					   function, and rewrite frame pointer refs
					   to stack pointer refs. auto_elim is set
					   non-zero is this done
					*/
					if (asmflag)
						auto_elim = 0;
					else
						auto_elim = remove_enter_leave(numauto);
				}
#ifndef IMPIL
				numauto = 0;  /* reset for next rtn */
#endif /* IMPIL */
				
				/* argument indictes whether to remove the "leave"
				   statements from the function */
				reordbot(auto_elim);
			} /* if( !asmflag || !aflag ) ) */

#ifdef DEBUG
			noopt: 
#endif
			{
				long start, end;
				if (auto_elim)
					start= ftell(stdout);
				prtext();
				if (auto_elim) {	/* if enter leave done */
					end = ftell(stdout);
					/* records the first and last bytes of
					   the function containing the enter leave
					   optimization. This info used to suppress
					   inlining in a function that has no
					   enter-leave
					*/
					add_enter_leave(start,end);
				}
			}

			prstr();
			printf("%s\n", line);
			if( aflag ) asmchk();
			asmflag = false;
			rvregs = 1; /* re-init ret value reg cnt */
			init();
			break;
		default:			/* all unrecognized text
						** pseudo-ops
						*/
			if (! inswitch)
			    printf("%s\n", linptr); /* flush to output */
			else
			    putstr(linptr);	/* in switch:  to
						** special file
						*/
			break;
	}
}

	static void
filter() /* print FILTER nodes and remove from list, also
	    clean up loop comment nodes, normally done in
	    raoptim. */
{
    register NODE *p;
    for(ALLN(p)) {
	if (p->op == FILTER) {
	    (void) puts(p->ops[0]);
	    DELNODE(p);
	}
	else if(suppress_register_allocation && p->op == LCMT)
	    DELNODE(p);
    }
}
	static enum Section
parse_section(s)
register char * s; /* string argument of .section */
{
	char savechar, *sname;

	SkipWhite(s);
	if(*s == NULL) fatal("parse_section(): no section name\n");
   	for(sname = s; *s != '\0'; ++s)
            if(*s == ',' || isspace(*s)) break;
   	savechar = *s;
   	*s = '\0';
	/* If this is too slow, look at first and last char
	   to identify string. */
   	if(strcmp(sname, ".debug") == 0)
	    section=CSdebug;
   	else if(strcmp(sname, ".line") == 0)
	    section=CSline;
   	else if(strcmp(sname, ".text") == 0)
     	    section = CStext;
	else if(strcmp(sname, ".data") == 0)
     	    section = CSdata;
   	else if(strcmp(sname, ".rodata") == 0)
     	    section = CSrodata;
   	else if(strcmp(sname, ".data1") == 0)
     	    section = CSdata1;
   	else if(strcmp(sname, ".bss") == 0)
	    section=CSbss;
   	else	/* unknown section */
     	    section = CSother;
   	*s = savechar;
	return section;
}
	int
plookup(s)	/* look up pseudo op code */
	char *s;

{
	static char *pops[POTHER] = {
		    "2byte",
		    "4byte",
		    "align",
		    "bcd",
		    "bss",
		    "byte",
		    "comm",
		    "data",
		    "double",
		    "even",
		    "ext",
		    "file",
		    "float",
		    "globl",
		    "ident",
		    "lcomm",
		    "local",
		    "long",
		    "previous",
		    "section",
		    "set",
		    "size",
		    "string",
		    "text",
		    "type",
		    "value",
		    "version",
		    "weak",
		    "word", 
		    "zero",
	};

	register int l,r,m,x; /* temps for binary search */

	l = 0; r = (int) POTHER-1;
	while (l <= r) {
	    m = (l+r)/2;
	    x = strcmp(s+1, pops[m]); /* s points at . */
	    if (x<0) r = m-1;
	    else if (x>0) l = m+1;
	    else return(m);
	}
 	fatal("plookup(): illegal pseudo_op: %s\n",s);	
/* NOTREACHED */
}

	void
yyinit(flags) char * flags; {

	section = CStext;	/* Assembler assumes the current section 
				   is .text at the top of the file. */

	for (; *flags != '\0'; flags++) {
		switch( *flags ) {
		case 'V':			/* Want version ID.	*/
			fprintf(stderr,"optim: %s%s\n",SGU_PKG,SGU_REL);
			break;
		case 'a':
			aflag = true;
			break;
		case 't':	/* suppress rmrdld() */
			tflag = true;
			break;
		case 'T':	/* trace rmrdld */
			Tflag = true;
			break;
#ifdef IMPIL
		case 'z':
			zflag = true;
			break;
#endif /* IMPIL */
		default:
			fprintf(stderr,"Optimizer: invalid flag '%c'\n",*flags);
		}
	}
}

int pic_flag;	/* set by this function only */

	char **
yysflgs( p ) char **p; /* parse flags with sub fields */
{
	extern void fatal();
	register char *s;	/* points to sub field */

	s = *p + 1;
	if (*s == NULL) {
		switch(**p) {
		case 'K': case 'Q': case 'X': case 'S': case 'y':
			fatal("-%c suboption missing\n",**p);
			break;
		}
	}
	switch( **p ) {
	case 'K':
		if( *s == 's' )
			switch( *++s ) {
			case 'd': optmode = OSPEED; return( p );
			case 'z': optmode = OSIZE; return( p );
			}
		else if ((*s == 'P' && strcmp(s,"PIC") == 0) ||
			 (*s == 'p' && strcmp(s,"pic") == 0)) {
			pic_flag=true;
			return p;
		}
		fatal("-K suboption invalid\n");
		break;
	case 'Q':
		switch( *s ) {
		case 'y': identflag = true; break;
		case 'n': identflag = false; break;
	        default: 
		  	fatal("-Q suboption invalid\n");
		  	break;
		}
		return( p );
	case 'X':	/* set ansi flags */
		switch( *s ){
		case 't': ccmode = Transition; break;
		case 'a': ccmode = Ansi; break;
		case 'c': ccmode = Conform; break;
		default: 
			fatal("-X requires 't', 'a', or 'c'\n");
			break;
		}
		return( p );
#ifdef IMPIL
	case '_':	/* suppress optimizations (for debugging) */
		SkipWhite(s);
		for(;*s;s++)
			switch(*s) {
			case 'r':
				never_register_allocation = true;
				suppress_register_allocation = true;
				break;
			case 'e':
				never_enter_leave = true;
				suppress_enter_leave = true;
				break;
			default:
				fatal("-_ requires 'r' or 'e'\n");
				break;
			}
		return(p);
	case 'y':
		pcdecode( s );
		return( p );
#endif /* IMPIL */
	default:
		return( p );
	}
/* NOTREACHED */
}

	static int
lookup(op,indx)		/* look up op code and return opcode ordinal */
     char *op;		/* mnemonic name */
     int *indx;		/* returned index into optab[] */
{

	register int f,l,om,m,x;
	static int ocode[numops] = {
		AAA, AAD, AAM, AAS, ADCB,
		ADCL, ADCW, ADDB, ADDL, ADDW,
		ANDB, ANDL, ANDW, ARPL, BOUND,
		CALL, CBTW, CLC, CLD, CLI,
		CLRB, CLRL, CLRW, CLTD, CMC,
		CMPB, CMPL, CMPW, CTS, CWTD,
		CWTL, DAA, DAS, DECB, DECL,
		DECW, DIVB, DIVL, DIVW, ENTER,
		ESC, F2XM1, FABS, FADD, FADDL,
		FADDP, FADDS, FBLD, FBSTP, FCHS,
		FCLEX, FCOM, FCOML, FCOMP, FCOMPL,
		FCOMPP, FCOMPS, FCOMS, FDECSTP, FDIV,
		FDIVL, FDIVP, FDIVR, FDIVRL, FDIVRP,
		FDIVRS, FDIVS, FFREE, FIADD, FIADDL,
		FICOM, FICOML, FICOMP, FICOMPL, FIDIV,
		FIDIVL, FIDIVR, FIDIVRL, FILD, FILDL,
		FILDLL, FIMUL, FIMULL, FINCSTP, FINIT,
		FIST, FISTL, FISTP, FISTPL, FISTPLL,
		FISUB, FISUBL, FISUBR, FISUBRL, FLD,
		FLD1, FLDCW, FLDENV, FLDL, FLDL2E,
		FLDL2T, FLDLG2, FLDLN2, FLDPI, FLDS,
		FLDT, FLDZ, FMUL, FMULL, FMULP,
		FMULS, FNCLEX, FNINIT, FNOP, FNSAVE,
		FNSTCW, FNSTENV, FNSTSW, FPATAN, FPREM,
		FPTAN, FRNDINT, FRSTOR, FSAVE, FSCALE,
		FSETPM, FSQRT, FST, FSTCW, FSTENV,
		FSTL, FSTP, FSTPL, FSTPS, FSTPT,
		FSTS, FSTSW, FSUB, FSUBL, FSUBP,
		FSUBR, FSUBRL, FSUBRP, FSUBRS, FSUBS,
		FTST, FWAIT, FXAM, FXCH, FXTRACT,
		FYL2X, FYL2XP1, HLT, IDIVB, IDIVL,
		IDIVW, IMULB, IMULL, IMULW,
		INB, INCB, INCL, INCW, INL,
		INS, INT, INTO, INW, IRET,
		JA, JAE, JB, JBE, JC,
		JCXZ, JE, JG, JGE, JL,
		JLE, JMP, JNA, JNAE, JNB,
		JNBE, JNC, JNE, JNG, JNGE,
		JNL, JNLE, JNO, JNP, JNS,
		JNZ, JO, JP, JPE, JPO,
		JS, JZ, LAHF, LAR, LCALL,
		LDS, LEAL, LEAVE, LEAW, LES,
		LGDT, LIDT, LJMP, LLDT, LMSW,
		LOCK, LOOP, LOOPE, LOOPNE, LOOPNZ,
		LOOPZ, LRET, LSL, LTR, MOVB,
		MOVL, MOVSBL, MOVSBW, MOVSWL, MOVW,
		MOVZBL, MOVZBW, MOVZWL, MULB, MULL,
		MULW, NEGB, NEGL, NEGW, NOTB,
		NOTL, NOTW, ORB, ORL, ORW,
		OUTB, OUTL, OUTS, OUTW, POPA,
		POPF, POPL, POPW, PUSHA, PUSHF,
		PUSHL, PUSHW, RCLB, RCLL, RCLW,
		RCRB, RCRL, RCRW, REP, REPNZ,
		REPZ, RET, ROLB, ROLL, ROLW,
		RORB, RORL, RORW, SAHF, SALB,
		SALL, SALW, SARB, SARL, SARW,
		SBBB, SBBL, SBBW, SCAB, SCAL,
		SCAW, SCMPB, SCMPL, SCMPW, SGDT,
		SHLB, SHLDL, SHLDW, SHLL, SHLW,
		SHRB, SHRDL, SHRDW, SHRL, SHRW,
		SIDT, SLDT, SLODB, SLODL, SLODW,
		SMOVB, SMOVL, SMOVW, SMSW, SSTOB,
		SSTOL, SSTOW, STC, STD, STI,
		STR, SUBB, SUBL, SUBW, TESTB,
		TESTL, TESTW, VERR, WAIT, XCHGB,
		XCHGL, XCHGW, XLAT, XORB, XORL,
		XORW
		};

	f = 0;
	l = numops;
	om = 0;
	m = (f+l)/2;
	while (m != om) {
		x = strcmp(op,optbl[m]);
		if (x == 0) {
			*indx = m;
			return(ocode[m]);
		      }
		else if (x < 0)
			l = m-1;
		    else
			f = m+1;
		om = m;
		m = (f+l)/2;
		}
	*indx = m;
	return(OTHER);
	}

	static FILE *
tmpopen() {
	strcpy( tmpname, tempnam( TMPDIR, "25cc" ) );
	return( fopen( tmpname, "w" ) );
	}

	static void
putstr(string)   char *string; {
	/* Place string from the text section into a temporary file
	 * to be output at the end of the function */

	if( stmpfile == NULL )
		stmpfile = tmpopen();
	fprintf(stmpfile,"%s",string);
	}

	static void
prstr() {
/* print the strings stored in stmpfile at the end of the function */

	if( stmpfile != NULL ) {
		register int c;

		stmpfile = freopen( tmpname, "r", stmpfile );
		if( stmpfile != NULL )
			while( (c=getc(stmpfile)) != EOF )
				putchar( c );
		else
			{
			fprintf( stderr, "optimizer error: ");
			fprintf( stderr, "lost temp file\n");
			}
		(void) fclose( stmpfile );	/* close and delete file */
		unlink( tmpname );
		stmpfile = NULL;
		}
}

/* opens the temp file for storing input while looking for 'asm' */
	static void
asmopen() {
	strcpy( atmpname, tempnam( TMPDIR, "asm" ) );
	atmpfile = fopen( atmpname, "w" );
	asmotell = ftell( stdout );
}

/* writes to temp file for 'asm' processing */
	int
putasm( lptr )
char *lptr;
{
	if(section == CSdebug) return true;
	if (*lptr == ' ') *lptr = '\t';
	if(fputs( lptr, atmpfile ) == EOF) return 0;
	else return (fputc('\n',atmpfile) != EOF);
}

/* checks for 'asm' in files and undoes code movement */
	static void
asmchk() 
{
	register c;
	long endotell;
#ifdef IMPREGAL
	extern int vars;
#endif /* IMPREGAL */

	if( asmflag ) {
		if( freopen( atmpname, "r", atmpfile ) != NULL ) {
			endotell = ftell( stdout );
			fseek( stdout, asmotell, 0 ); /* This is okay as long 
				as IMPIL is defined because it 
				is not really stdout, it is the file used by
				in-line expansion.  That file is still used, 
				even when in-line expansion is suppressed. 
				If IMPIL is not defined, optim will not work
				correctly to a terminal, but it will work
				correctly to a file.  
				This should be fixed.  */
			while( ( c = getc( atmpfile ) ) != EOF ) putchar( c );
			while( ftell( stdout ) < endotell ) printf( "!\n" );/* ? */
		}
		else fprintf( stderr, "optimizer error: asm temp file lost\n" );
	}
	freopen( atmpname, "w", atmpfile );
	asmotell = ftell( stdout );
#ifdef IMPREGAL
	vars=0; 	/* reinitialize for global reg allocation */
#endif /* IMPREGAL */
}

	void
putp(p,c) register NODE *p; char *c; {  /* insert pointer into jump node */

	p->op1 = c;
}

	void
dstats() { /* print stats on machine dependent optimizations */

#if STATS
	fprintf(stderr,"%d semantically useless instructions(s)\n", nusel);
	fprintf(stderr,"%d useless move(s) before compare(s)\n", nmc);
	fprintf(stderr,"%d merged move-arithmetic/logical(s)\n", nmal);
	fprintf(stderr,"%d useless sp increment(s)\n", nspinc);
	fprintf(stderr,"%d redundant compare(s)\n", nredcmp);
#endif /* STATS */
	}

	void
wrapup() { /* print unprocessed text and update statistics file */

#ifdef STATS
	FILE *sp;
	int mc,mal,usel,spinc,redcmp,disc,inst;
#endif

	if (n0.forw != NULL) {
		printf("	.text\n");
		filter();
		prtext();
		prstr();
		if(identflag)			/* Output ident info.	*/
			printf("\t.ident\t\"optim: %s\"\n",SGU_REL);
		}
#ifdef IMPIL
	ilfile();
#endif /* IMPIL */


	if( aflag ) {
		(void) fclose( atmpfile );	/* close and delete file */
		unlink( atmpname );
	}

	exit_line_section(); /* print the last .line entry */
	print_debug(); /* Dump out the debugging info for the whole file. */

#ifdef STATS
	/* STATSFILE should be defined to be a string representing a pathname */
	sp = fopen(STATSFILE,"r");
	if (sp != NULL) {
		fscanf(sp, "%d %d %d %d %d %d %d",
		   &mc,&mal,&usel,&spinc,&redcmp,&disc,&inst);
		fclose(sp);
		mc += nmc;
		mal += nmal;
		usel += nusel;
		spinc += nspinc;
		redcmp += nredcmp;
		disc += ndisc;
		inst += ninst;
		sp = fopen(STATSFILE,"w");
		if (sp != NULL) {
			fprintf(sp, "%d %d %d %d %d %d %d \n",
			   mc,mal,usel,spinc,redcmp,disc,inst);
			fclose(sp);
			}
		}
#endif /* STATS */
	}

	static void
setautoreg() { /* set indicator for number of autos/regs */

	register NODE *p;

	numauto = 0;
	for(p = n0.forw; p != &ntail && (is_debug_label(p) || !islabel(p)) ; p = p->forw)
		;	/* Skip over initial .text and stuff */
	if ( 		/* this line assumes left->right execution */
		 p == NULL
	     ||  p == &ntail
	     ||  !islabel(p)
	     ||  !ishl(p)
	   ) {					/* Can't determine sizes */
		numauto = BIGAUTO;
		numnreg = BIGREGS;
		return;
	}
	if (p->forw->op == POPL && usesvar("%eax",p->forw->op1) &&
	    p->forw->forw->op == XCHGL && usesvar("%eax",p->forw->forw->op1) &&
	    strcmp("0(%esp)",p->forw->forw->op2) == 0)
		p = p->forw->forw;
	if ( 		/* this line assumes left->right execution */
	         (p = p->forw)->op != PUSHL
	     ||  strcmp(p->op1,"%ebp") != 0
	     ||  (p = p->forw)->op != MOVL
	     ||  strcmp(p->op1,"%esp") != 0
	     ||  strcmp(p->op2,"%ebp") != 0
	   ) {					/* Can't determine sizes */
		numauto = BIGAUTO;
		numnreg = BIGREGS;
		return;
	}
	p = p->forw;
	if (p->op == SUBL && p->op1[0] == '$' && strcmp(p->op2,"%esp") == 0) {
		numauto = atoi(&p->op1[1]);	/* remember # of bytes */
		p = p->forw;
	} else if (p->op == PUSHL && strcmp(p->op1,"%eax") == 0) {
		numauto = INTSIZE;
		p = p->forw;
	}
	numnreg = 0;
	while ( p != &ntail
		&&  p->op == PUSHL
		&&  ( strcmp(p->op1,"%ebx") == 0
		   || strcmp(p->op1,"%esi") == 0
		   || strcmp(p->op1,"%edi") == 0 ) ) {
		numnreg++;
		p = p->forw;
	}
	return;
	}

/*
 * The following code reorders the Top of a C subroutine.
 * So that the 2 extraneous jumps are removed.
 */
	static void
reordtop()
{
	register NODE *pt, *st, *end;

	for(pt = n0.forw; pt != &ntail; pt = pt->forw)
		if ( islabel(pt) && !is_debug_label(pt))
			break;
	if ( islabel(pt) && ishl(pt) ) {
		pt = pt->forw;
		if ( !isuncbr(pt) && pt->op != JMP )
			return;
		if ( pt->op1[0] != '.' )
			return;
		if ( !islabel(pt->forw) || ishl(pt->forw) )
			return;
		for (st = pt->forw; st != &ntail; st = st->forw) {
			if ( !islabel(st) )
				continue;
			if ( strcmp(pt->op1, st->opcode) != 0 )
				continue;
			/* Found the beginning of code to move */
			for( end = st->forw;
				end != &ntail && !isuncbr(end);
				end = end->forw )
				;
			if ( end == &ntail )
				return;
			if ( end->op != JMP ||
				strcmp(end->op1,pt->forw->opcode) != 0 )
				return;

			/* Relink various sections */
			st->back->forw = end->forw;
			end->forw->back = st->back;

			pt->back->forw = st->forw;
			st->forw->back = pt->back;

			end->forw = pt->forw;
			pt->forw->back = end;
			return;			/* Real Exit */
		}
	}
}

/* The code generated for returns is as follows:
 * 	jmp Ln
 *	...
 *
 * Ln:
 * 	popl %ebx	/optional
 * 	popl %esi	/optional
 * 	popl %edi	/optional
 * 	leave
 * 	ret
 *
 * This optimization replaces unconditional jumps to the return
 * sequence with the actual sequence.
 */
	static void
reordbot(no_leave) int no_leave;
{
	NODE	*retlp,		/* ptr to first label node in return seq. */
		*firstp;	/* ptr to first non-label node in return seq. */
	register NODE *endp,	/* ptr to last node in return seq. */
		*lp,		/* ptr to label nodes in return seq. */
		*p, *q;		/* ptrs to nodes in function */
	endp = &ntail;
	do {			/* scan backwards looking for RET */
		endp = endp->back;
		if (endp == &n0)
			return;
	} while (endp->op != RET);	
				/* Now endp points to RET node */
	firstp = endp->back;
	if (firstp->op != LEAVE)
		return;
	if (no_leave) {
		DELNODE(firstp);
		firstp=endp;
	}
#ifdef pre50
	if (is_debug_label(firstp->back->op))/* Skip .def-generated hard label */
		firstp = firstp->back;	/* left for debugging */
#endif
	while (firstp->back->op == POPL) /* Restores of saved registers */
		firstp = firstp->back;
				/* Now firstp points to first non-label */
	retlp = firstp->back;
	if (!islabel(retlp) || ishl(retlp))
		return;
	do
		retlp = retlp->back;
	while (islabel(retlp) && !ishl(retlp));
				/* Now retlp points before first label */

	for (p = ntail.back; p != &n0; p = p->back) {
				/* Scan backwds for JMP's */
		if (p->op != JMP)
			continue;
		for (lp = retlp->forw; lp != firstp; lp = lp->forw) {
				/* Scan thru labels in ret seq. */
			if (strcmp(p->op1,lp->opcode) == 0) {
				/* found a JMP to the ret. seq., so copy
				it over the JMP node */
				copyafter(firstp,endp,p);
				q = p;
				p = p->back;
				DELNODE(q);
				break;
			}
		}
	}
}
	static void
copyafter(first,last,current) 
NODE *first; 
register NODE *last;
NODE *current;
{
	register NODE *p, *q;
	q = current;
	for(p = first; p != last->forw; p = p->forw) {
	  	if (is_debug_label(p)) continue;
		q = insert(q);
		chgop(q,(int)p->op, p->opcode);
		q->op1 = p->op1;
		q->op2 = p->op2;
		q->uniqid = IDVAL;
	}
}


#define ENTER_LEAVE_THRESH 30

#ifdef DEBUG
static char *get_label() {
	NODE *pn;
	for( ALLN( pn ) ) {
		/* first label has the function name */
		if( islabel( pn ) && !is_debug_label(pn) )
			return pn->opcode;
	}
}
#endif


static void do_remove_enter_leave();
static process_body();
static process_header();

/* driver which conditionally performs enter-leave removal */
static
remove_enter_leave(autos_pres) int autos_pres;
{
	NODE *pn;
	int depth;	/* change in stack due to register saves */
#ifdef DEBUG
	static int dcount;
	static int lcount= -2;
	static int hcount=999999;


	if (lcount == -2) {
		char *h=getenv("hcount");
		char *l=getenv("lcount");
		if (h) hcount=atoi(h);
		if (l) {
			lcount=atoi(l);
			fprintf(stderr,"count=[%d,%d]\n",lcount,hcount);
		}
		else   lcount = -1;
	}
	++dcount;
	if ( dcount < lcount || dcount > hcount)
		return;
	if (lcount != -1)
		fprintf(stderr,"%d(%s) ", dcount, get_label());
#endif


	/* check if function prolog is as expected */
	if (!process_header(&pn,&depth,autos_pres))
		return 0;

	/* check if rest of function as as expected, mark statements that contain
	   frame pointer references; these will be rewritten as stack pointer
	   references
	*/
	if (process_body(pn,depth,ENTER_LEAVE_THRESH)) {

		/* rewrite the prolog and body of a function, epilog is rewritten
		   in reordbot
		*/
		do_remove_enter_leave(depth);

		return 1;
	}
	return 0;
}

/* given a node that effects the stack pointer, returns the change caused by this
   function
*/

static
process_push_pop(node)
NODE *node;
{
	switch(node->op) {
	case PUSHL:
		return 4;
	case PUSHW:
		return 2;
	case POPL:
		return -4;
	case POPW:
		return -2;
	case SUBL:
		if (*node->op1 == '$' || !strcmp(node->op2,"%esp")) {
			return (atoi(node->op1+1));
		}
	/* FALLTHRU */
	case ADDL:
		if (*node->op1 == '$' || !strcmp(node->op2,"%esp")) {
			return (-atoi(node->op1+1));
		}
	/* FALLTHRU */
	case LEAVE:
		return 0;
	/* FALLTHRU */
	default: fatal("process_push_pop(): improper push pop");
	}
	/* NOTREACHED */
}
/* rewrite operands of the format:
	*n(%ebp)
	n(%ebp)
	n(%ebp,r)
to use the stack pointer
*/
static char *
rewrite(rand,depth)
char *rand;	/* operand to rewrite */
int depth;	/* stack growth since entry into the called address, this
		   includes saving registers, and any growth causes by pushing
		   arguments for function calls
		*/
{
	char *star="";
	char *lparen;
	int offset;
	if (*rand == '*') {
		++rand;
		star = "*";
	}
	offset=strtol(rand,&lparen,10);

	/* check if operand is of the desired format */
	if (!lparen || lparen[0] != '(' || lparen[1]!='%' || lparen[2]!='e' 
	    || lparen[3]!='b' || lparen[4] != 'p')
		return rand;

	rand = xalloc(NEWSIZE);

	/* the new offset. The original offset assumes the function return address and
	   old frame pointer are on stack. With this optimization only the
	   function return address are on stack. This is the reason for the "-4".
	   depth
	*/
	offset += depth -4;

	lparen[3]='s';
	sprintf(rand,"%s%d%s",star,offset,lparen);
	return rand;
}
static void
do_remove_enter_leave(depth) int depth; /* depth of stack after prolog */
{
	NODE *pn, *pf = NULL;
	for (pn=n0.forw; pn != 0; pn = pf) {
		pf = pn->forw;
		if (pn->extra == REMOVE) {
			DELNODE(pn);
		}
		else if (pn->extra > REMOVE) {
			if (pn->op1)
				pn->op1 = rewrite(pn->op1,pn->extra+depth);
			if (pn->op2)
				pn->op2 = rewrite(pn->op2,pn->extra+depth);
			if (pn->op3)
				pn->op3 = rewrite(pn->op3,pn->extra+depth);
		
		}
			
	}
}
/* For each instruction after the prolog, this routine calculates the runtime stack
   depth before this instruction. We can make this calculation,
   because we  require that the execution of each basic block of the function
   has a net change of 0 on the stack. This routine returns 1 if this requirement
   is met, and the function does not have too many insturctions (> limit).
   We except the basic block containing the function's epilog from this requirement.
*/
static 
process_body (pn,init_depth,limit)
register NODE *pn; 
int init_depth; /* depth of stack after the prolog */
int limit;	/* maximum instructions to consider */
{
	
	int depth = 0;	/* current stack growth from state after register saves
			   on function entry
			*/
	int count=0;
	for (; pn != 0; pn = pn->forw) {
		++count;
		if (count > limit)
			return 0;

		/*  check that depth at a "leave" is depth obtained by popping
		    all saved registers. If check is ok, then reset depth to 0,
		    so check of depth at a label subsequent to "leave" does not
		    fail, since control cannot flow from leave to such a label
		*/
		if (pn->op == LEAVE) {
			if (-depth != init_depth) {
				return 0;
			}
			depth = 0;
		}
		
		/* mark references to frame pointer; save current stack depth */
		if (uses(pn)&EBP && pn->op != PUSHA)
			pn->extra = (short) depth;

		/* Accumulate the effect of the current instruction on the stack.
		   If TMPSRET, then a push instruction which is popped in the called
		   function, all other pushes are popped in the caller
		*/
		if (sets(pn)&ESP && pn->extra != TMPSRET )
			depth += process_push_pop(pn);

		/*  depth must be 0 at beginning and end of each basic block */
		else if ( ( islabel(pn) || isbr(pn) ) && depth) {
			return 0;
		}
			
	}
	return 1;
}

/* Calculates the stack depth after the function prolog; flags instructions in
   the "enter" sequence for removal. Does not handle functions that return
   structures, or functions with auto's.
*/
static 
process_header(first_node, stack_level,autos_pres)
NODE **first_node; 
int *stack_level; /* stack level after the prolog */
int autos_pres; /* function initially had auto's, but regal assigned these
  		   to registers
		*/
{
	register NODE *pn;
	int count=0;
	long pushes=0;

	/* get to beginning of prolog */
	for (pn = n0.forw; pn != 0; pn = pn->forw) {
		if (pn->op == MOVL || pn->op == PUSHL || pn->op == POPL) break;
	}

	if (!pn)
		return 0;

	/* skip over profiling code */
	if (isprof( pn->forw ))
		pn = pn->forw->forw;

	/* expect saving of frame pointer */
	if (pn->op != PUSHL || !usesvar("%ebp", pn->op1))
		return 0;
	pn->extra = REMOVE;
	pn = pn->forw;

	/* expect adjust the frame pointer */
	if (pn->op != MOVL ||
	    !usesvar("%esp",pn->op1) || !usesvar("%ebp",pn->op2))
		return 0;
	pn->extra = REMOVE;
	pn = pn->forw;

	if (!autos_pres)
	/* EMPTY */
		;
	/* expect set up the locals */
	else if (pn->op == SUBL && usesvar("%esp",pn->op2)) {
		pn->extra = REMOVE;
		pn = pn->forw;
	} else if (pn->op == PUSHL &&
		   usesvar("%eax",pn->op1)) {
		pn->extra = REMOVE;
		pn = pn->forw;
	}
	else
		return 0;

	/* process the register saves. Register saves start by pushing non-scratch
	   registers and end when:
		a. A scratch register is pushed.
		b. A non-scratch register is pushed for the second time
	*/
	for (; pn != 0; pn = pn->forw) {
		int reg;
		if (pn->op != PUSHL || !isreg(pn->op1) ||
		    (reg=scanreg(pn->op1,0)&pushes) || reg&(EAX|EDX|ECX|FP0|FP1))
			break;
		count += 4;
		pushes |= reg;

	}
	*first_node = pn;
	*stack_level = count;
	return 1;
}
 char *
getstmnt() 
{
 register char *s;
 register int c;
 static char *front, *back;		/* Line buffer */ 

#define eoi(c) ((c=='\n')||(c==';')||(c==NULL)||(c==CC)) /* end of instruction */
#define eol(c) ((c=='\n')||(c==NULL)) /* end of line */
	/* Each line of input can contain multiple instructions separated
	 * by semicolons.
	 * getstmnt() returns the next instruction as a null terminated
	 * string.
	 */

	if( front == NULL )	/* initialize buffer */
		{front = (char *)malloc(LINELEN+1);
		 if(front == NULL)
			fatal("getstmnt: out of buffer space\n");
		 back = front + LINELEN;
		}
	/* read until end of instruction */
	s = front;
	while( (c = getchar()) != EOF )
		{if(s >= back)
			s = ExtendCopyBuf(&front,&back,(unsigned)2*(back-front+1));
		 if(eoi(c))
			{switch(c)
				{case ';':
				 case NULL:
				 case '\n':
					*s = NULL;
					return(front);
				 case CC:
					*s++ = (char)c;
					break;
				}
			 /* here if CC, read to end of line */
			 while((c = getchar()) != EOF)
				{if(s >= back)
					s = ExtendCopyBuf(&front,&back,(unsigned)2*(back-front+1));
				 if(eol(c))
					{*s = NULL;
					 return(front);
					}
				 else
					*s++ = (char)c;
				}
			 /* premature EOF */
			 if(s > front)
				{*s = NULL;
				 return(front);
				}
			 return(NULL);
			}
		 else
			*s++ = (char)c;
		}
	/* EOF */
	if(s > front)	/* premature */
		{*s = NULL;
		 return(front);
		}
	return(NULL);
}
static char *
ExtendCopyBuf(p0,pn,nsize)
char **p0;
char **pn;
unsigned int nsize;
{
 char *b0 = *p0;
 char *bn = *pn;
 unsigned int osize;
 extern void fatal();

 /* input buffer looks like:
  *
  *	-----------------------
  *	|   |   | ... |   |   |
  *	-----------------------
  *       ^                 ^
  *       |                 |
  *	 *p0               *pn == s
  *
  * where the current user pointer, s, is at the end of buffer.
  * after buffer extension, the new buffer looks like:
  *
  *	---------------------------------------------
  *	|   |   | ... |   |   |   |   | ... |   |   |
  *	---------------------------------------------
  *       ^                 ^                     ^
  *       |                 |                     |
  *      *p0                s                    *pn
  *
  * where s is at the same distance from the beginning of the buffer,
  * and the contents of the buffer from *p0 to s is unchanged,
  * and s is returned.
  */

 osize = bn - b0 + 1;
 if(nsize <= osize)
	fatal("ExtendCopyBuf: new size <= old size\n");
 b0 = realloc(b0,nsize);
 if(b0 == NULL)
	fatal("ExtendCopyBuf: out of space\n");
 bn = b0 + nsize - 1;
 *p0 = b0;
 *pn = bn;
 return(b0 + osize - 1);
}
