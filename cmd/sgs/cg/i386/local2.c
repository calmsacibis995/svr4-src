/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cg:i386/local2.c	1.44.1.2"
/*	local2.c - machine dependent grunge for back end
 *	i386 CG
 *              Intel iAPX386
 */

# include "mfile2.h"
# include "string.h"

typedef	long	OFFSZ;		/* should be same as that defined in
				 * mfile1.h.  This became necessary
				 * to redefine with the change from
				 * 5.3.4 to 5.3.12
				 */
# define istnode(p) ((p)->in.op==REG && istreg((p)->tn.rval))

extern char *exname();
extern int zflag;       /* true if we print comments in assembly output */
extern int edebug;      /* true if we print expression debug info       */
static void blockmove();
static void starput();

extern int canbereg();

#ifdef	GENMUL
static void genmul();
static void genleal();
static void genleal2();
static void genshift();
static void genmov();
static void gllfact();
static int chkleal();
static int cllfact();
#endif

static struct fpstype {
	TWORD	fpop;
	OFFSZ	fpoff1;
	OFFSZ	fpoff2;
	int	isqnode;
	TWORD	ftype;
}	fpstack[8];	/* current contents of FP stack */
static int	fpsp = 0;	/* current FP stack pointer */

#define CHKUFLOW	\
	if (fpsp < 0) cerror("floating point stack underflow")
#define CHKOFLOW	\
	if (fpsp > 7) cerror("floating point stack overflow")

#ifdef VOL_SUPPORT
int vol_opnd = 0;        /* volatile operand */
int cur_opnd = 1;               /* current operand  */

#define VOL_CLEAN()     {vol_opnd = 0; cur_opnd = 1;}

#endif

extern RST regstused;	/* List of registers used for structure moves */

void
lineid(l, fn) 		/* identify line l and file fn */
int l; 
char *fn;  
{
    fprintf(outfile, "/\tline %d, file %s\n", l, fn);
}

/* The outreg array maps a number for each value of rnames[] for
 * sdb debugging purposes.  We use PUSHA order for cpu registers.
 */
int outreg[] = {
    0,      /* %eax */
    1,      /* %ecx */
    2,      /* %edx */
    9,      /* %st0 */
    3,      /* %ebx */
    6,      /* %esi */
    7,      /* %edi */
    4,      /* %esp */
    5       /* %ebp */
};

char *rnames[] = {  /* normal access register names */
    "%eax", "%edx", "%ecx", "%st(0)",		/*scratch registers*/
    "%ebx", "%esi", "%edi",			/*user registers*/
    "%esp", "%ebp"                              /*other registers*/
};

static const char * const rsnames[] = { /* register names for shorts */
    "%ax",  "%dx",  "%cx",  "ERROR",
    "%bx",  "%si",  "%di",
    "%sp",  "%bp"
};

static const char * const rcnames[] = { /* register names for chars */
    "%al",  "%dl",  "%cl",  "ERROR",
    "%bl",  "ERROR","ERROR",
    "ERROR","ERROR"
};

static const char * const ccbranches[] = {
    "je",   "jne",  "jle",  "jl",   "jge",  "jg",
		    "jna", "jnae",  "jnb",  "jnbe"
};

static const char * const usccbranches[] = {
    "je",   "jne",  "jbe",  "jb",   "jae",  "ja",
		    "jna", "jnae",  "jnb",  "jnbe"
};


static zFGflag = 0;		/* For interactions with ZF, and ZG */

/*
 * Init the floating point stack (At routine start)
 */

void
initfp()
{
	register int i;

	for(i = 0; i < 8; i++) {
		fpstack[i].fpop = 0;
		fpstack[i].fpoff1 = 0;
		fpstack[i].fpoff2 = 0;
		fpstack[i].isqnode = 0;
		fpstack[i].ftype = TVOID;
	}
	fpsp = 0;
	zFGflag = 0;
}

static lasttype = 0;
static lastcmpe = 0;
static NODE *zzzgaddr;		/* Flag set by Zg to save tmp to stack */
#define NEXTZZZCHAR	( *++(*ppc) )
#define PEEKZZZCHAR	(*((*ppc)+1))

/*
 * The following codes are currently used in zzzcode:
 *		ABbCcDdEeFfGgHIiLlMOPpQRrSsTtvVWwXxY
 */
void
zzzcode( pnode, ppc, q )        /* hard stuff escaped from templates */
register NODE *pnode;   /* tree node being expanded */
char **ppc;             /* code string, at 'Z' of macro name */
OPTAB *q;               /* template */
{
    register NODE *pl, *pr;
    register int temp1, temp2;
    static OFFSZ ctmp2;
    static OFFSZ ctmp1;
    struct fpstype fptmp;
    static int save_lastcmpe;

/*
    fprintf(stderr, "\nzzzcode(): called with %c\n", PEEKZZZCHAR);
*/

    switch( NEXTZZZCHAR ) {

    case 'a': {
	/* Check if %ax is busy.  If so, spill, otherwise, %ax can be
	 * used - This is for fstsw instruction.
	 */
	OFFSZ tmp = 0;

	if (busy[REG_EAX]) {
		tmp = ( freetemp( 1 ) - maxboff ) / SZCHAR;
		fprintf(outfile, "\tmovl	%%eax,%ld(%%ebp)\n", tmp);
	}
	fprintf(outfile, "\tfstsw	%%ax\n\tsahf\n");
	if (tmp)
		fprintf(outfile, "\tmovl	%ld(%%ebp),%%eax\n",tmp);
	break;
    }

    case 'E':
	/* Print out the address of a free temp location.  This is
	 * used mainly in floating point conversions, where registers
	 * must be placed somewhere on the stack temporarily.
	 *
	 * ZEs:	print temp for single-precision
	 * ZEd: print temp for double-precision
	 */
	{
		char type = NEXTZZZCHAR;
		if (ctmp2 == 0)
		{
			int words;
			switch (type) {
			case 's':	words = 1;	break;
			case 'd':	words = 2;	break;
			default:	cerror("illegal argument to ZE");
			}
			ctmp2 = ( freetemp( words ) - maxboff ) / SZCHAR;
		}
		fprintf(outfile,  "%ld(%%ebp)", ctmp2 );
		break;
	}
    case 'e':
	/* Reset the temp location address.
	 */
	ctmp2 = 0;
	break;
    case 'F':
	/* Push a floating point stack temporary value on the
	 * simulated stack.
	 */
	pl = getadr(pnode, ppc);
	pr = getadr(pnode, ppc);
	temp1 = *++(*ppc);
	if (zzzgaddr && pr == zzzgaddr) {
		fprintf(outfile, "\tfstp%c\t",temp1);
		adrput(pl);
		if( zflag ) {
		    emit_str( "\t\t/ ZF expansion\n");
		}
		putc( '\n', outfile );
		lastcmpe = 0;		/* zero out, wasn't a ZC */
		zzzgaddr = (NODE *)0;	/* VOID extra uses */
		fpsp--;	/* pop stack */
		CHKUFLOW;
		break;
	}
	/* If the stack is too full, put the value out to the
	 * real stack location.
	 */
	if (fpsp >= 6) {
		fprintf(outfile, "\tfstp%c\t",temp1);
		adrput(pl);
		if( zflag ) {
		    emit_str( "\t\t/ ZF expansion\n");
		}
		putc('\n',outfile );
		zzzgaddr = (NODE *)0;		/* VOID extra uses */
		for(temp1 = 1; temp1 <= fpsp; temp1++)
			fpstack[temp1-1] = fpstack[temp1];
		fpsp--;
		fpstack[fpsp].fpop = 0;
		fpstack[fpsp].fpoff1 = 0;
		fpstack[fpsp].fpoff2 = 0;
		fpstack[fpsp].isqnode = 0;
		fpstack[fpsp].ftype = TVOID;
		break;
	}
	if( zflag ) {
	    emit_str("\t\t/ ZF expansion\n");
	}
	fpstack[0].fpop = pl->tn.op;
	fpstack[0].fpoff1 = pl->tn.lval;
	fpstack[0].fpoff2 = 0;
	fpstack[0].isqnode = 0;
	fpstack[0].ftype = pl->tn.type;
	zFGflag = 1;			/* used if ZF followed immed. by ZG */
	lastcmpe = 0;			/* zero out, wasn't a ZC */
	zzzgaddr = (NODE *)0;
	return;
    case 'f':
	/* Generate the necessary floating point stack pop operand,
	 * and the necessary stack manipulation operands.  Pop the
	 * simulated stack.
	 */
	pl = getadr(pnode, ppc);
	temp2 = NEXTZZZCHAR;
	zzzgaddr = (NODE *)0;		/* VOID extra uses */
	zFGflag = 0;			/* VOID extra uses */
	for(temp1 = 0; temp1 < fpsp; temp1++)
		if (fpstack[temp1].fpop == TEMP &&
		    pl->tn.op == TEMP &&
		    (pl->tn.lval == fpstack[temp1].fpoff1 ||
		     pl->tn.lval == fpstack[temp1].fpoff2) )
			break;
	if (temp1 >= fpsp) {		/* output std temp location */
		fprintf(outfile, "%c\t", temp2);
		adrput(pl);
		break;
	}
	/* output special stack address */
	fprintf(outfile, "p\t%%st,%%st(%d)", temp1);
	if (temp1 != 1) {
		fprintf(outfile, "\n\tfxch\t%%st(%d)", temp1-1);
		fptmp = fpstack[temp1];
		fpstack[temp1] = fpstack[1];
		fpstack[1] = fptmp;
	}
	for(temp1 = 1; temp1 <= fpsp; temp1++)
		fpstack[temp1-1] = fpstack[temp1];
	fpsp--;
	CHKUFLOW;
	fpstack[fpsp].fpop = 0;
	fpstack[fpsp].fpoff1 = 0;
	fpstack[fpsp].fpoff2 = 0;
	fpstack[fpsp].isqnode = 0;
	fpstack[fpsp].ftype = TVOID;
	break;
    case 'I':
	/* Generate the necessary stack address if it is a register
	 * and thein increment the stack. if temp2 == 'N' don't
	 * generate an address
	 */
	pl = getadr(pnode, ppc);
	temp2 = NEXTZZZCHAR;
	zzzgaddr = (NODE *)0;		/* VOID extra uses */
	if (temp2 != 'N') {
		for(temp1 = 0; temp1 < fpsp; temp1++)
			if (fpstack[temp1].fpop == TEMP &&
			    pl->tn.op == TEMP &&
			    (pl->tn.lval == fpstack[temp1].fpoff1 ||
			     pl->tn.lval == fpstack[temp1].fpoff2) )
				break;
		if (temp1 >= fpsp) {		/* output std temp location */
			fprintf(outfile, "%c\t", temp2);
			adrput(pl);
			break;
		}
		fprintf(outfile, "%%st(%d)", temp1);
	}
	if (zFGflag == 1 && fpsp == 0 && fpstack[0].fpop == TEMP) {
		fpstack[1] = fpstack[0];
		fpsp++;
	} else {
		for(temp1 = fpsp; temp1 >= 0; temp1--)
			fpstack[temp1+1] = fpstack[temp1];
	}
	fpstack[0].fpop = pl->tn.op;
	fpstack[0].fpoff1 = pl->tn.lval;
	fpstack[0].fpoff2 = 0;
	fpstack[0].isqnode = 0;
	fpstack[0].ftype = TVOID;
	fpsp++;
	CHKOFLOW;			/* avoid write to fpstack[8] above */
	zFGflag = 0;			/* VOID extra uses */
	break;
    case 'i':
	/* Pop a stack item.
	 */
	zzzgaddr = (NODE *)0;		/* VOID extra uses */
	zFGflag = 0;			/* VOID extra uses */
	for(temp1 = 1; temp1 <= fpsp; temp1++)
		fpstack[temp1-1] = fpstack[temp1];
	fpsp--;
	CHKUFLOW;
	fpstack[fpsp].fpop = 0;
	fpstack[fpsp].fpoff1 = 0;
	fpstack[fpsp].fpoff2 = 0;
	fpstack[fpsp].isqnode = 0;
	fpstack[fpsp].ftype = TVOID;
	break;
    case 'Q':
	/* Increment the number of Qnodes currently loaded.  This
	 * is needed when ?: statements pare used as args to subroutines
	 * passing floats (extra stack elements popped from ZG).
	 */
	zFGflag = 0;			/* VOID extra uses */
	if (fpstack[1].isqnode == 1) {
		fpstack[1].fpoff2 = fpstack[0].fpoff1;
		fpstack[1].isqnode = 2;
		for(temp1 = 1; temp1 <= fpsp; temp1++)
			fpstack[temp1-1] = fpstack[temp1];
		fpsp--;
		CHKUFLOW;
		fpstack[fpsp].fpop = 0;
		fpstack[fpsp].fpoff1 = 0;
		fpstack[fpsp].fpoff2 = 0;
		fpstack[fpsp].isqnode = 0;
		fpstack[fpsp].ftype = TVOID;
	}else
		fpstack[0].isqnode = 1;
	break;
    case 'p':
        /* Clear simulated stack without saving contents.  This is
        ** to be used, for example, after assigning to RNODE.
        */
        for (temp1 = 0; temp1 <= fpsp; temp1++) {
                fpstack[temp1].fpop = 0;
                fpstack[temp1].fpoff1 = 0;
                fpstack[temp1].fpoff2 = 0;
                fpstack[temp1].isqnode = 0;
                fpstack[fpsp].ftype = TVOID;
        }
        fpsp = 0;
        zFGflag = 0;
        break;
    case 'G':
	/* This code is also part of the ZF, Zg package (See Zg).
	 * The attempt is to catch all cases, where temps must
	 * really be stored on the stack, and when they shouldn't.
	 */
	if (fpsp > 0) {
		for(temp1 = 0; temp1 < fpsp; temp1++) {
			if (temp1 == 0 && fpstack[0].isqnode) {
				fpstack[temp1].fpop = 0;
				fpstack[temp1].fpoff1 = 0;
				fpstack[temp1].fpoff2 = 0;
				fpstack[temp1].isqnode = 0;
				fpstack[fpsp].ftype = TVOID;
				break;
			}
			if (fpstack[temp1].fpop != 0) {
				if( fpstack[temp1].ftype == TFLOAT)
				    fprintf(outfile, "\tfstps\t%ld(%%ebp)", 
						fpstack[temp1].fpoff1 - (maxboff/SZCHAR));
				else
				    fprintf(outfile, "\tfstpl\t%ld(%%ebp)", fpstack[temp1].fpoff1 - (maxboff /SZCHAR));
				if( zflag ) {
				    emit_str("\t\t/ ZG expansion\n");
				}
				putc('\n',outfile );
			}
			fpstack[temp1].fpop = 0;
			fpstack[temp1].fpoff1 = 0;
			fpstack[temp1].fpoff2 = 0;
			fpstack[temp1].isqnode = 0;
			fpstack[temp1].ftype = TVOID;
		}
	} else if (fpsp == 0 && zFGflag) {
		if (fpstack[0].fpop != 0) {
			fprintf(outfile, "\tfstp%c\t%ld(%%ebp)",
			    (fpstack[0].ftype == TFLOAT) ? 's' : 'l',
			    fpstack[0].fpoff1 - (maxboff/SZCHAR));
			if( zflag ) {
			    emit_str("\t\t/ ZG expansion\n");
			}
			putc('\n',outfile );
		}
		fpstack[0].fpop = 0;
		fpstack[0].fpoff1 = 0;
		fpstack[0].fpoff2 = 0;
		fpstack[0].isqnode = 0;
		fpstack[0].ftype = TVOID;
	}
	fpsp = 0;
	zzzgaddr = (NODE *)0;		/* VOID extra uses */
	zFGflag = 0;			/* VOID extra uses */
	break;
    case 'g':
	/* tHis code is A Cavalier attempt to Kombine the ZF macro
	 * with the templates used to generate a function call that
	 * returns a double.  The ZF macro needs to know when it is
	 * following a function call, and when it isn't.  When it is
	 * following a function call the code must really generate a
	 * real temp store.  In the other cases, a fake store to the
	 * FP stack needs to take place
	 */
	lastcmpe = 0;			/* zero out, wasn't a ZC */
	zzzgaddr = pnode;
	zFGflag = 0;			/* VOID extra uses */
	return;

    case 'L': {
#define MAXLAB  5
	int tempval = NEXTZZZCHAR;
	switch( tempval ) {
	        static int labno[MAXLAB+1]; /* saved generated labels */
	        int i, n;
	case '.':                       /* produce labels */
	        /* generate labels from 1 to n */
	        n = NEXTZZZCHAR - '0';
	        if (n <= 0 || n > MAXLAB)
	            cerror("Bad ZL count");
	        for (i = 1; i <= n; ++i)
	            labno[i] = getlab();
	        break;
	
	default:
	        cerror("bad ZL");
		/*NOTREACHED*/
	
	/* generate selected label number */
	case '1':
	case '2':
	case '3':
	case '4':          /* must have enough cases for MAXLAB */
	case '5':
	        fprintf(outfile,".L%d", labno[tempval - '0']); 
	        break;
	}         
	break;
    }
    case 'R':
	/* Read and change the rounding mode in the 80?87 to chop; remember
	 * old value at ctmp1(%ebp).  Use Zr to restore.
	 */
	ctmp1 = ( freetemp( 1 ) - maxboff ) / SZCHAR;
	fprintf(outfile,  "\tfstcw\t%ld(%%ebp)\n\tmovw\t%ld(%%ebp),",
	    ctmp1, ctmp1 );
	temp1 = lasttype;
	lasttype = TSHORT;
	expand( pnode, FOREFF, "ZA2\n\torw\t$0x0c00,ZA2\n\tmovw\tZA2,", q );
	lasttype = temp1;
	fprintf(outfile,  "%ld(%%ebp)\n\tfldcw\t%ld(%%ebp)",
	    ctmp1+2, ctmp1+2 );
	if( zflag )
	    emit_str("\t\t/ ZR expansion");
	putc('\n',outfile );
	break;
    case 'r':
	/* restore old rounding mode */
	fprintf(outfile,  "\tfldcw\t%ld(%%ebp)", ctmp1 );
	if( zflag )
	    emit_str("\t\t/ Zr expansion");
	putc('\n',outfile );
	break;

    case 'u':
	/* Set rounding mode to -Inf for unsigned conversion magic.
	** Assume ctmp1+2 already contains the "chop" rounding mode.
	*/
	fprintf(outfile, "\txorw\t$0x800,%ld(%%ebp)\n", ctmp1+2);
	fprintf(outfile, "\tfldcw\t%ld(%%ebp)", ctmp1+2);
	if( zflag )
	    emit_str("\t\t/ Zu expansion");
	putc('\n',outfile );
	break;

    case 'c':
	/* Pop the appropriate argsize from sp */
	if (pnode->stn.argsize == SZINT)
	    fprintf(outfile, "\tpopl\t%%ecx\n");
	else
	    fprintf(outfile, "\taddl\t$%d,%%esp\n",
		    (((unsigned)pnode->stn.argsize+(SZINT-SZCHAR))/SZINT)*(SZINT/SZCHAR));
	break;

    case 'd':
	/* Output floating constant for INT_MAX or UINT_MAX:
	** Zd.i:  output constant (INT_MAX); Zdi:  output address
	** Zd.u:  output constant (UINT_MAX); Zdu:  output address
	** Zd.iu: output both constants; Zdiu:  output both addresses (sic)
	*/
    {
	static int labs[2];	/* [0] for int; [1] for uint */
	static const char * const fmts[2] = {
	    ".L%d:	.long	0x0,0x41e00000\n",	/* int */
	    ".L%d:	.long	0x0,0x41f00000\n",	/* uint */
	};
	int c;
	int gendata = 0;
	int didprefix = 0;

	if ((c = NEXTZZZCHAR) == '.') {
	    gendata = 1;
	    c = NEXTZZZCHAR;
	}

	for (;;) {
	    int index;
	    int lab;

	    switch( c ){
	    case 'i':	index = 0; break;
	    case 'u':	index = 1; break;
	    default:	cerror("bad Zd%c", c);
	    }
	    
	    lab = labs[index];
	    if (gendata) {
		if (lab == 0) {
		    labs[index] = lab = getlab();
		    if (!didprefix) {
			(void) locctr(FORCE_LC(CDATA));
			emit_str("	.align	4\n");
		    }
		    fprintf(outfile, fmts[index], lab);
		    didprefix = 1;
		}
	    }
	    else {
		if (picflag) {
		    fprintf(outfile, ".L%d@GOTOFF(%s)", lab, rnames[BASEREG]);
		    gotflag |= GOTREF;
		}
		else
		    fprintf(outfile, ".L%d", lab);
	    }
	    if (PEEKZZZCHAR == 'u')
		c = NEXTZZZCHAR;
	    else
		break;
	}
	if (didprefix)
	    (void) locctr(PROG);
	break;
    }
    case 'P':
	/* Allocate and generate the address for the appropriate amount
	 * of space for a return struct field.  This is a replacement
	 * for the 'T' macro when TMPSRET is defined.
	 */
	pl = talloc();
	pl->tn.op = TEMP;
	pl->tn.type = TSTRUCT;
	pl->tn.lval = freetemp( (pnode->stn.stsize+(SZINT-SZCHAR))/SZINT );
	pl->tn.lval /= SZCHAR;
	adrput(pl);
	tfree(pl);
	break;
    case 'B':
	/* The B and b macros set and reset the lasttype flag.  This
	 * variable is used by the T,t,A zzzcode macros to determine
	 * register name size to output.  T and t output the register
	 * names according to parameters in various subtrees, where as
	 * A prints the register names according to the last T,t,B.
	 */
	switch( *++(*ppc) ) {
	    case '1':
		lasttype = TCHAR;
		break;
	    case '2':
		lasttype = TSHORT;
		break;
	    case '4':
		lasttype = TINT;
		break;
	    case '6':
		lasttype = TFLOAT;	/* can't be 4, but will probably
					 * never be used anyways... */
		break;
	    case '8':
		lasttype = TDOUBLE;
		break;
	}
	break;

    case 'b':
	/* reset the lasttype flag. */
	lasttype = 0;
	break;

    case 'T':
	/* Output the appropriate size for the given operation */
	/* 'T' Does not try to do zero/sign extend */
	pl = getadr(pnode, ppc);
	lasttype = pl->in.type;
	switch (pl->in.type) {
	    case TCHAR:
	    case TUCHAR:
		putc('b',outfile);
		break;
	    case TSHORT:
	    case TUSHORT:
		putc('w',outfile);
		break;
	    case TINT:
	    case TUNSIGNED:
	    case TLONG:
	    case TULONG:
	    case TPOINT:
		putc('l',outfile);
		break;
	    case TFLOAT:
		if (pl->in.op != REG)
		    putc('s',outfile);
		break;
	    case TDOUBLE:
		if (pl->in.op != REG)
		    putc('l',outfile);
		break;
	    default:
		emit_str("ERROR");
		break;
	}
	break;
    case 't':
	/* Output the appropriate size for the given operation */
	/* 't' Does zero/sign extend  When appropriate */
	pl = getadr(pnode, ppc);
	lasttype = pl->in.type;
	if (pl->in.type != pnode->in.type) {
	    switch( pl->in.type) {
		case TCHAR:
		case TSHORT:
		    putc('s',outfile);
		    break;
		case TUCHAR:
		case TUSHORT:
		    putc('z',outfile);
		    break;
		default:
		    break;
	    }
	}
	switch (pl->in.type) {
	    case TCHAR:
	    case TUCHAR:
		putc('b',outfile);
		break;
	    case TSHORT:
	    case TUSHORT:
		putc('w',outfile);
		break;
	    case TINT:
	    case TUNSIGNED:
	    case TLONG:
	    case TULONG:
	    case TPOINT:
		putc('l',outfile);
		break;
	    default:
		emit_str("ERROR");
		break;
	}
	if (pl->in.type != pnode->in.type) {
	    switch( pnode->in.type) {
		case TSHORT:
		case TUSHORT:
		    if ((pl->in.type & (TSHORT|TUSHORT)) &&
			(pnode->in.type & (TSHORT|TUSHORT)))
			    break;
		    putc('w',outfile);
		    break;
		case TLONG:
		case TULONG:
		case TINT:
		case TUNSIGNED:
		case TPOINT:
		case TFLOAT:		/* floats doubles/ go to int regs    */
		case TDOUBLE:		/* first and the templates know this */
		    if ((pl->in.type & (TLONG|TULONG|TINT|TUNSIGNED|TPOINT)) &&
			(pnode->in.type & (TLONG|TULONG|TINT|TUNSIGNED|TPOINT)))
			    break;
		    putc('l',outfile);
		    break;
		default:
		    break;
	    }
	}
	break;

    case 'A':
	/*
	 * This is a special output mode that allows registers of
	 * a lower type (specifically char) to me used in a movl.
	 * This is used in a case such as when you want to:
	 *	movl	%edi, %al	but really must do:
	 *	movl	%edi, %eax
	 * The assembler can't handle the first case.
	 */
	pl = getadr(pnode, ppc);
	switch (lasttype) {
	    case TCHAR:
	    case TUCHAR:
		emit_str( rcnames[pl->tn.rval]);
		break;
	    case TSHORT:
	    case TUSHORT:
		emit_str( rsnames[pl->tn.rval]);
		break;
	    default:
		emit_str( rnames[pl->tn.rval]);
		break;
	}
	break;

#if 0
    case 'd':
	/* This entry will print a ',%st' operand out if the
	 * specified location is a register.  This is necessary
	 * in some floating point operations, such as:
	 *	fmul	%st(6),%st	but:
	 *	fmull	.L10,%st	is illegal...
	 *	fmull	.L10		is the correct version.
	 */
	pl = getlr( pnode, NEXTZZZCHAR );
	if (pl->in.op == REG)
		fprintf(outfile, ",%%st");
	break;
#endif /* 0 */

    case 'H':
	/* Put out a shifted constant that has been properly
	 * modified for FLD ops.  This constant is anded (and
	 * overwritten) so it will fit into the field.  Then
	 * it is shifted for the field location and written out.
	 */
	{
	extern int fldsz, fldshf;
	/* table of masks to truncate field to size[i] */
	static unsigned long bits[33] = {
	    0x00000000, 0x00000001, 0x00000003, 0x00000007,
	    0x0000000f, 0x0000001f, 0x0000003f, 0x0000007f,
	    0x000000ff, 0x000001ff, 0x000003ff, 0x000007ff,
	    0x00000fff, 0x00001fff, 0x00003fff, 0x00007fff,
	    0x0000ffff, 0x0001ffff, 0x0003ffff, 0x0007ffff,
	    0x000fffff, 0x001fffff, 0x003fffff, 0x007fffff,
	    0x00ffffff, 0x01ffffff, 0x03ffffff, 0x07ffffff,
	    0x0fffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff,
	    0xffffffff
	};


	pl = getlr( pnode, NEXTZZZCHAR );		/* FLD operator */
	pr = getlr( pnode, NEXTZZZCHAR );		/* ICON operator */
	if( pl->tn.op != FLD ) {
	    cerror( "bad FLD for ZH" );
	    /*NOTREACHED*/
	    }
	fldsz = UPKFSZ(pl->tn.rval);
	fldshf = UPKFOFF(pl->tn.rval);
	pr->tn.lval &= bits[fldsz];		/* truncate constant to size */
	fprintf(outfile, "%ld", pr->tn.lval << fldshf);
	}
	break;

    case 'O':
	pl = getadr(pnode, ppc);
	fprintf(outfile,  "%d", 1 << pl->tn.lval );
	break;

    case 'C':
	if (lastcmpe || save_lastcmpe) {
		/* Generate a unsigned (floating point) conditional branch */
		fprintf(outfile,  "%s\t.L%d",
		    usccbranches[pnode->bn.lop - EQ], pnode->bn.label );
	} else {
		/* Generate a conditional branch */
		fprintf(outfile,  "%s\t.L%d",
		   ccbranches[pnode->bn.lop - EQ], pnode->bn.label );
	}
	if( zflag ) {
	    emit_str( "\t\t/ ZC expansion\n");
	}
	putc('\n',outfile );
	save_lastcmpe = lastcmpe = 0;
	break;

    case 'D':
	/* The next zzzcode better be a ZC or Zl or this is useless */
	lastcmpe = 1;
	zzzgaddr = (NODE *)0;	/* zero out if the last op wasn't a ZF */
	zFGflag = 0;
	return;

    case 'l':
	/* Save lastcmpe until ZC is seen. */
	if (lastcmpe) 
	    save_lastcmpe = 1;
	break;

    case 'S':
	/* STASG - structure assignment */
fprintf(outfile,"/STASG**************:\n");
#ifdef VOL_SUPPORT
        /* If the root strat field contains a VOLATILE flag,
               move the strat field to the left and right nodes */

        if ( pnode->in.strat & VOLATILE )
        {
                pnode->in.left->in.strat |= VOLATILE;
                pnode->in.right->in.strat |= VOLATILE;
        }
#endif
        /* now call stasg for the actual structure assignemnt */
	stasg( pnode->in.left, pnode->in.right, pnode->stn.stsize, q );
#ifdef VOL_SUPPORT
                VOL_CLEAN();
#endif 
fprintf(outfile,"/End STASG^^^^^^^^^^^^^^:\n");
	break;

    case 's':
	/* STARG - structure argument */
	{   /* build a lhs to stasg to, on the stack */
	NODE px, pxl, pxr;

	px.in.op = PLUS;
	pxl.tn.type = px.in.type = pnode->in.left->in.type;
	px.in.left = &pxl;
	px.in.right = &pxr;
	px.in.strat = 0;
	pxl.tn.op = REG;
	pxl.tn.rval = REG_ESP;
	pxl.tn.strat = 0;
	pxr.tn.op = ICON;
	pxr.tn.type = TINT;
	pxr.tn.name = 0;
	pxr.tn.lval = 0;
	pxr.tn.strat = 0;

fprintf(outfile,"/STARG**************:\n");
	fprintf(outfile,  "\tsubl\t$%d,%%esp",
		((pnode->stn.stsize+(SZINT-SZCHAR))/SZINT)*(SZINT/SZCHAR));
	if( zflag ) {
	    emit_str( "\t\t/ STARG setup\n");
	}
	putc('\n',outfile );

#ifdef	VOL_SUPPORT
	if (pnode->in.strat & VOLATILE) {
	    px.tn.strat = VOLATILE;
	    pnode->in.left->in.strat |= VOLATILE;
	}
#endif
	stasg( &px, pnode->in.left, pnode->stn.stsize, q );
#ifdef VOL_SUPPORT
                VOL_CLEAN();
#endif 
fprintf(outfile,"/End STARG^^^^^^^^^^^^^^:\n");
	break;
	}

	/*
	 * This macro generates the optimal multiply by a constant
	 * sequence.  The calling sequence is:
	 *        ZM'SIZE','CONSTANT','OP','RESULT'.
	 * For instance ZML,R,L,1[,2] might generate the proper shift
	 * sequence or the sequence imulZTL $CR,AL,A1.  The break off
	 * point for generating shifts/adds/subs/moves over imulx is
	 * nine cycles.  pr is the constant's node, pl is the left
	 * side, and pt is the temp location node.  [,2] is an optional
	 * second temporary which may be useful.
	 */
    case 'M':
	{
		register NODE *pt, *pst;
		char multype;

		pr = getadr(pnode, ppc);
		switch (pr->in.type) {
			case TCHAR: case TUCHAR:
				multype = 'b';
				break;
			case TSHORT: case TUSHORT:
				multype = 'w';
				break;
			default:
				multype = 'l';
				break;
		}
		if (NEXTZZZCHAR != ',')
			cerror( "botched ZM macro");
		pr = getadr(pnode, ppc);
		if (NEXTZZZCHAR != ',')
			cerror( "botched ZM macro");
		pl = getadr(pnode, ppc);
		if (NEXTZZZCHAR != ',')
			cerror( "botched ZM macro");
		pt = getadr(pnode, ppc);
		if ( **ppc == ',' ) {
			++(*ppc);
			pst = getadr(pnode, ppc);
		} else
			pst = NULL;
#ifdef GENMUL
		if ( chkmul(pr->tn.lval, pl, pt, pst) )
			genmul(pl, pt, pst);
		else
#endif
		{
			fprintf(outfile, "\timul%c\t", multype);
			adrput(pr);
			putc(',',outfile);
#ifdef VOL_SUPPORT
			vol_opnd_end();
#endif
			adrput(pl);
			putc(',',outfile);
#ifdef VOL_SUPPORT
			vol_opnd_end();
#endif
			adrput(pt);
			putc('\n',outfile);
#ifdef VOL_SUPPORT
                        vol_instr_end();
                        VOL_CLEAN();
#endif
		}
	}
	break;			/* OUT of 'M' */

    case 'v':
#ifdef VOL_SUPPORT
        pl = getadr(pnode, ppc);
        if (pl->in.strat & VOLATILE)  vol_opnd |= cur_opnd;
#endif
	break;

    case 'V':
	/*
	 * This command is an if statement used to determine if
	 * a specified temporary variable is actually on the FP stack
	 * or if the line should be generated.  If the FP temp is
	 * on the stack then the remainder of the line should be gobbled.
	 */
	pl = getadr( pnode, ppc );
	for(temp1 = 0; temp1 < fpsp; temp1++)
		if (fpstack[temp1].fpop == TEMP &&
		    pl->tn.op == TEMP &&
		    (pl->tn.lval == fpstack[temp1].fpoff1 ||
		     pl->tn.lval == fpstack[temp1].fpoff2) )
			break;
	if (temp1 >= fpsp) 	/* Generate the line */
		break;
	/* Fall through and clobber the line */
cream:
	/* clobber a line to newline */
	while( NEXTZZZCHAR != '\n' ) {
		if( **ppc == '\\' ) ++(*ppc); /* hide next char */
		if( !**ppc ) return;
	}
	break;

    case 'W':
fprintf(outfile,"/BLOCK MOVE*************:\n");
		blockmove(pnode);
#ifdef VOL_SUPPORT
                VOL_CLEAN();
#endif 
fprintf(outfile,"/END BMOVE^^^^^^^^^^^^^^:\n");
		break;

    case 'w':
		blockmove(pnode);
		break;

    /* Put out section directive for either .data or .rodata.
    ** Next character is 'd' for data, 't' for text.
    */
    case 'o':
		(void)locctr(NEXTZZZCHAR == 'd' ? FORCE_LC(CDATA) : PROG);
		break;

    /* Put out uninitialized data to "zero" */
    case 'z':
		zecode(pnode->tn.lval);
		break;
    /* Treat operand as address mode under * */
    case '*':
	pl = getadr( pnode, ppc );		/* operand */
	starput( pl, (CONSZ) 0 );
	break;
    
    /* Operand is ++/-- node.  Produce appropriate increment/decrement
    ** code for the left operand of the ++/-- (actually, *(R++ )).
    */
    case '+':
    {
	int isincr;

	pl = getadr(pnode, ppc)->in.left;
	switch (pl->in.op) {
	case INCR:	isincr = 1; break;
	case DECR:	isincr = 0; break;
	default:
	    cerror("Bad Z+ operator %s", opst[pl->in.op]);
	}

	if (pl->in.right->tn.lval == 1)
	    fprintf(outfile, "	%s	", isincr ? "incl" : "decl");
	else
	    fprintf(outfile, "	%s	$%d,", isincr ? "addl" : "subl",
				pl->in.right->tn.lval);
	adrput(pl->in.left);
	putc('\n', outfile);
	break;
    }

    default:
	cerror( "botched Z macro %c", **ppc );
	/*NOTREACHED*/
    }
    lastcmpe = 0;		/* zero out if the last op wasn't a ZC */
    zzzgaddr = (NODE *)0;	/* zero out if the last op wasn't a ZF */
}
void
conput( p ) 			/* name for a known */
register NODE *p; 
{
#ifdef VOL_SUPPORT
        if (p->in.strat & VOLATILE) vol_opnd |= cur_opnd;
#endif
    switch( p->in.op ) 
    {
    case ICON:
	acon( p );
	break;
    case REG:
	switch( p->tn.type )   /* get the right name for a register */
	{
	default:        /* extended register, e.g., %eax */
	    emit_str( rnames[p->tn.rval]);
	    break;
	case TSHORT:
	case TUSHORT:   /* short register, %ax */
	    emit_str( rsnames[p->tn.rval]);
	    break;
	case TCHAR:
	case TUCHAR:    /* char register, %al */
	    emit_str( rcnames[p->tn.rval]);
	    break;
	}
	break;
    case UNINIT:
        acon(p);
        break;
    case CURCAP:
        emit_str( "%ebp");		/* this output may not be right */
        break;
    case CSE:
        {
             struct cse *cs_ptr;
             if ( (cs_ptr = getcse(p->csn.id)) == NULL)
             {
                   e2print(p);
                   cerror("Uknown CSE id");
             }
             emit_str(rnames[cs_ptr->reg]);   /* check this output */
        }
        break;
    case COPY:
    case COPYASM:
        if ( p->tn.name)
             emit_str(p->tn.name);
        break;

    default:
	cerror( "confused node in conput" );
	/*NOTREACHED*/
    }
}
/*ARGSUSED*/
void
insput( p ) NODE *p; {
                        /* we don't have any */
                        /* print the upper word of a two-word value.
                          used for stack frame funnies.  should really
                          be upput but that has been taken already*/
#ifdef VOL_SUPPORT
        if (p->in.strat & VOLATILE) vol_opnd |= cur_opnd;
#endif
    cerror( "insput" );
    /*NOTREACHED*/
}

/* Generate an addressing mode that has an implied STAR on top. */

static int baseregno;			/* base register number */
static int indexregno;			/* index register number */
static int scale;			/* scale factor */
static CONSZ offset;			/* current offset */
static char *name;			/* for named constant */
static CONSZ findstar();

static void
starput( p, inoff )
NODE *p; 
int inoff;				/* additional offset (bytes) */
{
    int flag = 0;
    /* enter with node below STAR */
    baseregno = -1;
    indexregno = -1;
    scale = 1;
    offset = inoff;
    name = (char *) 0;

    /* Find the pieces, then generate output. */
    offset += findstar(p,0,&flag);

    /* Prefer base register to index register. */
    if (indexregno >= 0 && baseregno < 0 && scale == 1) {
	baseregno = indexregno;
	indexregno = -1;
    }

    if (name) {
	emit_str(name);
#ifdef  ELF_OBJ
	if (picflag) {
	    char *outstring = "";
	    if (flag & PIC_GOT) {
		gotflag |= GOTREF;
		if (flag & (NI_FLSTAT|NI_BKSTAT))
		    outstring = "@GOTOFF"; 
		else if (offset)
		    cerror("starput: illegal offset in ICON node: %s", name);
		else 
		    outstring = "@GOT";
	    } 
	    else if (flag & PIC_PLT) {
		gotflag |= GOTREF;
		outstring = "@PLT";
	    }
	    fprintf(outfile, outstring);
	}
#endif
	if (offset > 0)
	    putc('+',outfile);
    }
    if (offset)
	fprintf(outfile, "%ld", offset);
    else if (baseregno == REG_ESP)
	putc('0',outfile );
    

    putc('(',outfile);
    if (baseregno >= 0)
	fprintf(outfile, "%s", rnames[baseregno]);
    if (indexregno >= 0) {
	fprintf(outfile, ",%s", rnames[indexregno]);
	if (scale > 1)
	    fprintf(outfile, ",%d", scale);
    }
    putc(')',outfile );
    sideff = 0;
    return;
}


/* Find the pieces for address modes under *.  Return
** the numeric part of any constant found.  If "scaled"
** set, any register must be an index register.
*/
static CONSZ
findstar(p,scaled,flag)
NODE * p;
int scaled;
int *flag;
{
    switch( p->in.op ){
    case REG:
	/* Base register has pointer type, index is other.
	** With double indexing and an address constant, there
	** may be two index registers.  Be careful not to overwrite
	** the index register, and make sure the index register is
	** the one that gets scaled, if necessary.
	*/
	if (scaled) {
	    /* Double-indexed with address constant case. */
	    if (indexregno >= 0)
		baseregno = indexregno;
	    indexregno = p->tn.rval;
	}
	else if (baseregno < 0)
	    baseregno = p->tn.rval;
	else
	    indexregno = p->tn.rval;
	return( 0 );
    case ICON:
	name = p->tn.name;
	*flag = p->tn.strat | (p->tn.rval & (NI_BKSTAT|NI_FLSTAT));
	return( p->tn.lval );
    {
	CONSZ temp;
    /* Force specific ordering of tree walk. */
    case PLUS:
	temp = findstar(p->in.left,0,flag);
	return(temp + findstar(p->in.right,0,flag));
    case MINUS:
	temp = findstar(p->in.left,0,flag);
	return(temp - findstar(p->in.right,0,flag));
    }
    case LS:
	/* Assume ICON on right. */
	scale = 1 << p->in.right->tn.lval;
	return(findstar(p->in.left,1,flag));
    case UNARY AND:
	p = p->in.left;
	if (p->in.op != VAUTO)
	    break;
	if (baseregno >= 0)
	    indexregno = baseregno;
	baseregno = REG_EBP;
	return( p->tn.lval );
    }
    cerror("confused findstar, op %s", opst[p->in.op]);
}


void
adrput( p ) 			/* output address from p */
NODE *p; 
{      

    sideff = 0;

again:
    while( p->in.op == FLD || p->in.op == CONV ) {
	p = p->in.left;
    }

#ifdef VOL_SUPPORT
        if (p->in.strat & VOLATILE)  vol_opnd |= cur_opnd;
#endif

    switch( p->in.op ) 
    {
    case ICON:               /* value of the constant */
	putc('$',outfile );
	/*FALLTHRU*/
    case NAME:
	acon( p );
	break;
    case CURCAP:
    case CSE:
    case REG:
	conput( p );
	break;
    case STAR:
	if( p->in.left->in.op == UNARY AND ) {
	    p = p->in.left->in.left;
	    goto again;
	}
	starput( p->in.left, (CONSZ) 0 );
	break;
    case UNARY AND:
	switch( p->in.left->in.op) {
	case STAR:
	    p = p->in.left->in.left;
	    goto again;
	case TEMP:
	case VAUTO:     /* these appear as part of STARG and STASG */
	case VPARAM:
	    p = p->in.left;
	    goto again;
	case NAME:
	    p = p->in.left;
	    p->in.op = ICON;
	    goto again;
	default:
	    cerror( "adrput:  U& over nonaddressable node" );
	    /*NOTREACHED*/
	}
	break;
    case TEMP:
	fprintf(outfile,  "%ld(%%ebp)", p->tn.lval /*- ( maxboff / SZCHAR )*/ );
	break;
    case VAUTO:
    case VPARAM:
	fprintf(outfile,  "%ld(%%ebp)", p->tn.lval );
	break;
    default:
	cerror( "adrput: illegal address" );
	/* NOTREACHED */
    }
}

    /* Output address of word after p.  Used for double moves. */
void
upput( p ) register NODE *p; {
    register NODE *psav = p;

    while( p->in.op == FLD || p->in.op == CONV ) {
	p = p->in.left;
    }
#ifdef VOL_SUPPORT
        if ((p->in.strat & VOLATILE) || (psav->in.strat & VOLATILE))
                vol_opnd |= cur_opnd;
#endif

recurse:
    switch( p->in.op ) {
    case NAME:
    case VAUTO:
    case VPARAM:
	p->tn.lval += SZINT/SZCHAR;
	adrput( psav );
	p->tn.lval -= SZINT/SZCHAR;
	break;
    case REG:
	fprintf(outfile,  "%d", SZINT/SZCHAR );
	adrput( psav );
	break;
    case TEMP:
	p->tn.lval += SZINT/SZCHAR;
	adrput( psav );
	p->tn.lval -= SZINT/SZCHAR;
	break;
    case STAR:
	starput( p->in.left, (CONSZ) SZINT/SZCHAR );
	break;
    case UNARY AND:
	p = p->in.left;
	goto recurse;

    default:
	cerror( "upput:  confused addressing mode" );
	/*NOTREACHED*/
    }
}
void
acon( p ) 			/* print out a constant */
register NODE *p; 
{       
    register OFFSZ off;

    if( p->in.name == (char *) 0 ) 	/* constant only */
    {            
	if( p->in.op == ICON &&
		( p->in.type == TCHAR || p->in.type == TUCHAR ) ) {
	    p->tn.lval &= 0xff;
	}
	fprintf(outfile,  "%ld", p->tn.lval );
    } 
    else {
	char *outstring = "";
#ifdef  ELF_OBJ
	if (picflag) {
	    register flag = p->tn.strat;
	    if (flag & PIC_GOT) {
		gotflag |= GOTREF;
		if (p->tn.rval & (NI_FLSTAT|NI_BKSTAT))
		    outstring = "@GOTOFF";
		else if (p->tn.lval)
		    cerror("acon: illegal offset in ICON node: %s", p->in.name);
		else
		    outstring = "@GOT";
	    } 
	    else if (flag & PIC_PLT) {
		gotflag |= GOTREF;
		outstring = "@PLT";
	    }
	}
#endif
	if( ( off = p->tn.lval ) == 0 )     /* name only */
	    fprintf(outfile,  "%s%s", p->in.name, outstring );
	else if( off > 0 )                       /* name + offset */
	    fprintf(outfile,  "%s%s+%ld", p->in.name, outstring, off );
	else                                     /* name - offset */
	    fprintf(outfile,  "%s%s%ld", p->in.name, outstring, off );
    }
}

/*ARGSUSED*/
special( sc, p )
NODE *p;
{
    cerror( "special shape used" );
#if lint
    return( 0 );
#endif
}

#ifdef	GENMUL

/*
 * The comment sections of the following routines will a special notation
 * defined here.  The notation for the variables i,j,k,l is as follows:
 *
 *	i,j,k,l		- from one of the set of leal  = { 2,3,4,5,8,9 }
 *	I,J,K,L		- from one of the set of leal2 = { 2,4,8 }
 *	i^2,j^2,k^2,l^2	- A constant size power of 2
 *
 *	The factors f_i,f_j,f_k,f_l will hold the actual values
 *	where as the variables i,j,k,l will only be indexes to
 *	important things.
 */

struct aXb {
	int	result, f1, f2;
};

struct aXb lxl[] = {
	{  4, 2, 2 }, {  6, 2, 3 }, {  8, 2, 4 }, {  9, 3, 3 },
	{ 10, 2, 5 }, { 12, 3, 4 }, { 15, 3, 5 }, { 16, 2, 8 },
	{ 18, 2, 9 }, { 20, 4, 5 }, { 24, 3, 8 }, { 25, 5, 5 },
	{ 27, 3, 9 }, { 32, 4, 8 }, { 36, 4, 9 }, { 40, 5, 8 },
	{ 45, 5, 9 }, { 64, 8, 8 }, { 72, 8, 9 }, { 81, 9, 9 },
	{  0, 0, 0 },
};

struct aXb l2xl2[] = {
	{  4, 2, 2 }, {  8, 2, 4 }, { 16, 2, 8 }, { 32, 4, 8 },
	{ 64, 8, 8 }, {  0, 0, 0 },
};

struct aXb lxl2[] = {
	{  4, 2, 2 }, {  6, 3, 2 }, {  8, 2, 4 }, { 10, 5, 2 },
	{ 12, 3, 4 }, { 16, 2, 8 }, { 18, 9, 2 }, { 20, 5, 4 },
	{ 24, 3, 8 }, { 32, 4, 8 }, { 36, 9, 4 }, { 40, 5, 8 },
	{ 64, 8, 8 }, { 72, 9, 8 }, {  0, 0, 0 },
};

/*
 * The leal set, is the set of possible mulitplication factors
 * in a single leal instruction.  This is usefule ofr looping
 * through the possible cases.
 */

int	lealset[] =  {	2, 3, 4, 5, 8, 9, 0 };
int	lealset2[] = {	2, 4, 8, 0 };

#define	chkpow2(v)	((v) != 0 && ((v) & ((v)-1)) == 0)

#endif	/* def GENMUL */

/*
 * Multiplication types
 * See "32-Bit Multiplication on the 386", by Dan Lau of Intel,
 * Internal correspondence memo.  For further information.
 */

#define	MUL_leal		1	/* leal only */
#define	MUL_shl			2	/* shift only */
#define	MUL_ixj			3	/* (i*j) */
#define	MUL_ipj			4	/* (i+j) */
#define	MUL_i2xj		5	/* (i^2*j) */
#define	MUL_ixjxk		6	/* (i*j*k) */
#define	MUL_ixjp1		7	/* (i*j+1) */
#define	MUL_ixjxKp1		8	/* (i*j*K+1) */
#define	MUL_ipJxk		9	/* ((i+J)*k */
#define	MUL_ixKpj		10	/* (i*K+j) */
#define	MUL_i2xjxk		11	/* (i^2*j*k) */
#define	MUL_im1S		12	/* ((i-1)^2) */
#define	MUL_ipj2		13	/* (i+j^2) */
#define	MUL_j2mi		14	/* (j^2-i) */
#define	MUL_ixj2p1		15	/* (i*j^2+1) */
#define	MUL_Kxipj2		16	/* (K*i+j^2) */
#define	MUL_ixjxkxl		17	/* (i*j*k*l) */
#define	MUL_ixjm1		18	/* (i*j-1) */
#define	MUL_jxkpi		19	/* (i+j*k) */
#define	MUL_ipjxk		20	/* ((i+j)*k) */
#define	MUL_ixjp1xk		21	/* ((i*j+1)*k) */
#define	MUL_ixjxLpk		22	/* (i*j*L+k) */
#define	MUL_Kxipjxl		23	/* ((K*i+j)*l) */
#define	MUL_j2pim1		24	/* (j^2+i-1) */
#define	MUL_j2mim1		25	/* (j^2-i-1) */
#define	MUL_j2mip1		26	/* (j^2-i+1) */
#define	MUL_i2p1xj		27	/* ((i^2+1)*j) */
#define	MUL_i2m1xj		28	/* ((i^2-1)*j) */
#define	MUL_k2mjxi		29	/* (k^2-j*i) */
#define	MUL_k2mJpi		30	/* ((k^2-(J+i)) */
#define	MUL_kxj2pi		31	/* (k*j^2+i) */
#define	MUL_kxj2mi		32	/* (k*j^2-i) */
#define	MUL_kxmj2i		33	/* (k*(j^2-i)) */
#define	MUL_ixj2p1xk		34	/* ((i*j^2+1)*k) */
#define	MUL_k2pjxi		35	/* (k^2+j*i) */
#define	MUL_kxpj2i		36	/* (k*(j^2+i)) */
#define	MUL_ixjxLpk2		37	/* (i*j*L+k^2) */
#define	MUL_Kxipj2xl		38	/* ((K*i+j^2)*l) */

/*
 * The following structure is used in determining whether a constant
 * sized multiply may be generated with shifts/leals/adds etc. faster
 * as well as generating the correct code.  See chkmul/genmul.
 */

struct mul_info	{
	int	mul_type;	/* Type of multiply chosen */
	CONSZ	f_i,		/* Factor i,I,i^2 */
		f_j,		/* Factor j,J,j^2 */
		f_k,		/* Factor k,K,k^2 */
		f_l;		/* Factor l,L,l^2 */
};

static struct mul_info mul_var;


/*
 * This subroutine checks to see if the specified multiply constant
 * may be generated more effectively with various combinations of
 * add/sub/leal/shifts/moves.  This system will only check the
 * combination of instructions whose the total cost is <=  9 clocks.
 * The imulx instruction takes between 9 - 22 clocks to perform.
 *
 * p1 is the starting register, p2 is the result register.
 * pst is a possible second temporary variable.
 * val is the constant.  A pointer to a mul_info is returned if
 * the multiply is found, otherwise a (struct mul_info *)0
 * is returned.
 */

#ifdef	GENMUL

static int
chkmul(val, p1, p2, pst)
CONSZ val;
NODE *p1, *p2, *pst;
{
	struct mul_info *mi = &mul_var;
	register lj,lk;			/* Loop Variables */

	if (chkleal(val)) {
		if( zflag )
			emit_str( "\t\t/ CASE MUL_leal\n");
		mi->mul_type = MUL_leal;
		mi->f_i = val;
		return(MUL_leal);
	}
	if (chkpow2(val)) {
		if( zflag )
			emit_str( "\t\t/ CASE MUL_shl\n");
		mi->mul_type = MUL_shl;
		mi->f_i = val;
		return(MUL_shl);
	}
	if (cllfact(val)) {
		if( zflag )
			emit_str( "\t\t/ CASE MUL_ixj\n");
		mi->mul_type = MUL_ixj;
		gllfact(val, &mi->f_i, &mi->f_j);
		return(MUL_ixj);
	}
	for( lj = 0; lealset2[lj] != 0; lj++ )
		if (chkleal(val - lealset2[lj]) && (pst ||
		    (p1->in.op == REG && p1->tn.rval != p2->tn.rval)) ) {
			if( zflag )
				emit_str( "\t\t/ CASE MUL_ipj\n");
			mi->mul_type = MUL_ipj;
			mi->f_i = val - lealset2[lj];
			mi->f_j = lealset2[lj];
			return(MUL_ipj);
		}
	for( lj = 0; lealset[lj] != 0; lj++ ) {
		if (val/lealset[lj] <= 0)
			break;
		if (val%lealset[lj] == 0 && chkpow2(val/lealset[lj])) {
			if( zflag )
				emit_str( "\t\t/ CASE MUL_i2xj\n");
			mi->mul_type = MUL_i2xj;
			mi->f_i = val / lealset[lj];
			mi->f_j = lealset[lj];
			return(MUL_i2xj);
		}
	}
	/*
	 * Anything past this point must start out in a register.
	 * If not, then it is possible, that the imul instructio
	 * will be faster.  The remaining multiplies are >= 6 clock
	 * cycles in length.  Memory based operands would take
	 * at least 6+4=10 cycles, when minimum inul is 9 ...
	 */
	if (p1->in.op != REG)
		return(0);
	/*
	 * Cases that require p1 to start in a register.
	 */
	for(lk = 0; lealset[lk] != 0; lk++ ) {
		if (val/lealset[lk] <= 0)
			break;
		if (val%lealset[lk] == 0 && cllfact(val/lealset[lk])) {
			if( zflag )
				emit_str( "\t\t/ CASE MUL_ixjxk\n");
			mi->mul_type = MUL_ixjxk;
			gllfact(val/lealset[lk], &mi->f_i, &mi->f_j);
			mi->f_k = lealset[lk];
			return(MUL_ixjxk);
		}
	}
	/*
	 * Some of the cases are bundled to gether so that the
	 * looping tests are not phenominal.  This is done only
	 * any of the combinations take the same number of clock
	 * cycles.
	 */
	/* Couldn't be generated */
	return(0);
}

#endif

/*
 * This subroutine takes a struct mul_info as well as the appropriate
 * node pointers, and generates the correct multiply sequence.
 */

#ifdef GENMUL
static void
genmul(p1, p2, pst)
NODE *p1, *p2, *pst;
{
	struct mul_info *mi = &mul_var;

	switch(mi->mul_type) {
		case MUL_leal:
			if (p1->in.op != REG) {
				genmov(p1, p2);
				genleal(mi->f_i, p2, p2);
			} else if (p1->tn.rval != p2->tn.rval)
				genleal(mi->f_i, p1, p2);
			else
				genleal(mi->f_i, p2, p2);
			break;
		case MUL_shl:
			if (p1->in.op != REG || p1->tn.rval != p2->tn.rval)
				genmov(p1, p2);
			genshift(mi->f_i, p2);
			break;
		case MUL_ixj:
			if (p1->in.op != REG) {
				genmov(p1, p2);
				genleal(mi->f_i, p2, p2);
			} else if (p1->tn.rval != p2->tn.rval)
				genleal(mi->f_i, p1, p2);
			else
				genleal(mi->f_i, p2, p2);
			genleal(mi->f_j, p2, p2);
			break;
		case MUL_ipj:
			if (pst) {
				genmov(p1, pst);
				genleal(mi->f_i, pst, p2);
				genleal2(mi->f_j, p2, pst, p2);
			} else {
				genleal(mi->f_i, p1, p2);
				genleal2(mi->f_j, p2, p1, p2);
			}
			break;
		case MUL_i2xj:
			if (p1->in.op != REG || p1->tn.rval != p2->tn.rval)
				genmov(p1, p2);
			genshift(mi->f_i, p2);
			genleal(mi->f_j, p2, p2);
			break;
		case MUL_ixjxk:
			if (p1->tn.rval != p2->tn.rval)
				genleal(mi->f_k, p1, p2);
			else
				genleal(mi->f_k, p2, p2);
			genleal(mi->f_i, p2, p2);
			genleal(mi->f_j, p2, p2);
			break;
		case MUL_ixjp1:
			break;
		case MUL_ixjxKp1:
			break;
		case MUL_ipJxk:
			break;
		case MUL_ixKpj:
			break;
		case MUL_i2xjxk:
			break;
		case MUL_im1S:
			break;
		case MUL_ipj2:
			break;
		case MUL_j2mi:
			break;
		case MUL_ixj2p1:
			break;
		case MUL_Kxipj2:
			break;
		case MUL_ixjxkxl:
			break;
		case MUL_ixjm1:
			break;
		case MUL_jxkpi:
			break;
		case MUL_ipjxk:
			break;
		case MUL_ixjp1xk:
			break;
		case MUL_ixjxLpk:
			break;
		case MUL_Kxipjxl:
			break;
		case MUL_j2pim1:
			break;
		case MUL_j2mim1:
			break;
		case MUL_j2mip1:
			break;
		case MUL_i2p1xj:
			break;
		case MUL_i2m1xj:
			break;
		case MUL_k2mjxi:
			break;
		case MUL_k2mJpi:
			break;
		case MUL_kxj2pi:
			break;
		case MUL_kxj2mi:
			break;
		case MUL_kxmj2i:
			break;
		case MUL_ixj2p1xk:
			break;
		case MUL_k2pjxi:
			break;
		case MUL_kxpj2i:
			break;
		case MUL_ixjxLpk2:
			break;
		case MUL_Kxipj2xl:
			break;
	}
}
#endif

#ifdef	GENMUL

static int
chkleal(val)
CONSZ val;
{
	switch((int)val) {
		case 2:  case 3:  case 4:
		case 5:  case 8:  case 9:
			return(2);
		default:
			return(0);
	}
}

#endif	/* def GENMUL */

#if 0
chkleal2(val)
CONSZ val;
{
	switch((int)val) {
		case 2:  case 4:  case 8:
			return(2);
		default:
			return(0);
	}
}
#endif

#ifdef	GENMUL

/*
 * The c*fact routines check to see if the value is one of the possible
 * set of factors generated by leal instructions. ll = leal*leal etc.
 */
static int
cllfact(val)
CONSZ val;
{
	switch((int)val) {
		case  4: case  6: case  8: case  9:
		case 10: case 12: case 15: case 16:
		case 18: case 20: case 24: case 25:
		case 27: case 32: case 36: case 40:
		case 45: case 64: case 72: case 81:
			return(4);
		default:
			return(0);
	}
}

cl2l2fact(val)
CONSZ val;
{
	switch((int)val) {
		case 4: case  8: case 16: case 32: case 64:
			return(4);
		default:
			return(0);
	}
}

cll2fact(val)
CONSZ val;
{
	switch((int)val) {
		case  4: case  6: case  8: case 10:
		case 12: case 16: case 18: case 20:
		case 24: case 32: case 36: case 40:
		case 64: case 72:
			return(4);
		default:
			return(0);
	}
}

/*
 * The g*fact routines return the factors for a given value. Based on the
 * set of factors generated by leal instructions. ll = leal*leal etc.
 */
static void
gllfact(val, f1, f2)
CONSZ val;
CONSZ *f1, *f2;
{
	register struct aXb *lp = &lxl[0];

	while (lp->result && lp->result != (int)val)
		lp++;
	if (!lp->result) {
		emit_str("ERROR");
		return;
	}
	*f1 = lp->f1;
	*f2 = lp->f2;
}

void
gl2l2fact(val, f1, f2)
CONSZ val;
int *f1, *f2;
{
	register struct aXb *lp = &l2xl2[0];

	while (lp->result && lp->result != (int)val)
		lp++;
	if (!lp->result) {
		emit_str("ERROR");
		return;
	}
	*f1 = lp->f1;
	*f2 = lp->f2;
}

void
gll2fact(val, f1, f2)
CONSZ val;
int *f1, *f2;
{
	register struct aXb *lp = &lxl2[0];

	while (lp->result && lp->result != (int)val)
		lp++;
	if (!lp->result) {
		emit_str("ERROR");
		return;
	}
	*f1 = lp->f1;
	*f2 = lp->f2;
}


/*
 * Generate the shift instruction
 */
static void
genshift(val, p)
CONSZ val;
NODE *p;
{
	register int i;
	register unsigned long ul;

	ul = val;
	for(i = 0; ul != 1; i++)
		ul >>= 1;

	switch(i) {
		case 0:
			break;
		case 1:
			fprintf(outfile, "\taddl\t%s,%s\n",
				rnames[p->tn.rval],
				rnames[p->tn.rval]);
			break;
		default:
			fprintf(outfile, "\tshll\t$%d,%s\n", i,
				rnames[p->tn.rval]);
			break;
	}
}

/*
 * Generate a leal with 1 sources, and a destination
 */
static void
genleal(val, s, d)
CONSZ val;
NODE *s, *d;
{
	switch((int)val) {
	case 2:
		fprintf(outfile, "\tgen2leal\t(,%s,2),%s\n",
			rnames[s->tn.rval],
			rnames[d->tn.rval]);
		break;
	case 3:
		fprintf(outfile, "\tgen3leal\t(%s,%s,2),%s\n",
			rnames[s->tn.rval],
			rnames[s->tn.rval],
			rnames[d->tn.rval]);
		break;
	case 4:
		fprintf(outfile, "\tgen4leal\t(,%s,4),%s\n",
			rnames[s->tn.rval],
			rnames[d->tn.rval]);
		break;
	case 5:
		fprintf(outfile, "\tgen5leal\t(%s,%s,4),%s\n",
			rnames[s->tn.rval],
			rnames[s->tn.rval],
			rnames[d->tn.rval]);
		break;
	case 8:
		fprintf(outfile, "\tgen8leal\t(,%s,8),%s\n",
			rnames[s->tn.rval],
			rnames[d->tn.rval]);
		break;
	case 9:
		fprintf(outfile, "\tgen9leal\t(%s,%s,8),%s\n",
			rnames[s->tn.rval],
			rnames[s->tn.rval],
			rnames[d->tn.rval]);
		break;
	}
}

/*
 * Generate a two source operand leal.  The first source operand
 * is added to the second shifted operand.
 */
static void
genleal2(val, s1, s2, d)
CONSZ val;
NODE *s1, *s2, *d;
{
	switch((int)val) {
	case 2:
		fprintf(outfile, "\tgen22leal\t(%s,%s,2),%s\n",
			rnames[s1->tn.rval],
			rnames[s2->tn.rval],
			rnames[d->tn.rval]);
		break;
	case 4:
		fprintf(outfile, "\tgen24leal\t(%s,%s,4),%s\n",
			rnames[s1->tn.rval],
			rnames[s2->tn.rval],
			rnames[d->tn.rval]);
		break;
	case 8:
		fprintf(outfile, "\tgen28leal\t(%s,%s,8),%s\n",
			rnames[s1->tn.rval],
			rnames[s2->tn.rval],
			rnames[d->tn.rval]);
		break;
	}
}
#endif

#if 0
/*
 * Generate an add instruction, add src to destination.
 */
void
genadd(s, d)
NODE *s, *d;
{
	fprintf(outfile, "\taddl\t%s,%s\n", rnames[s->tn.rval], rnames[d->tn.rval]);
}

/*
 * Generate an sub instruction, sub src to destination.
 */
void
gensub(s, d)
NODE *s, *d;
{
	fprintf(outfile, "\tsubl\t%s,%s\n", rnames[s->tn.rval], rnames[d->tn.rval]);
}
#endif

/*
 * Generate an move instruction, move src to destination.
 */
#ifdef GENMUL
static void
genmov(s, d)
NODE *s, *d;
{
	if (s->tn.rval == d->tn.rval)
		return;
	emit_str("\tmovl\t");
	adrput(s);
	fprintf(outfile, ",%s\n", rnames[d->tn.rval]);
}
#endif

#if 0
/*
 * This routine checks to see if enough registers have
 * been passed in.  Return 1 if all is OK, 0 if not enough
 * registers passed in.
 */
chkmreg(p1,p2,pst,num)
NODE *p1, *p2, *pst;
int num;
{
	switch(num) {
		case 0:
		case 1:
			return(1);	/* p2 must always be a register */
		case 2:
			if (p1->in.op == REG && !istreg(p1->tn.rval) &&
			    pst != NULL && p2->tn.rval != pst->tn.rval)
				return(1);
			else
			if ( (p1->in.op == REG && p1->tn.rval != p2->tn.rval) ||
			     (pst != NULL && p2->tn.rval != pst->tn.rval) )
					return(1);
			return(0);
		default:
			return(0);
	}
}
#endif

/* Move an intermediate result in a register to a different register. */
void 
rs_move( pnode, newregno ) 
NODE * pnode; int newregno; 
{
    register int type = pnode->tn.type;

    if( type & (TINT|TUNSIGNED|TPOINT) ) {
	fprintf(outfile,  "\tmovl\t%s,%s",
	    rnames[pnode->tn.rval], rnames[newregno] );
    } else if( type & (TSHORT|TUSHORT) ) {
	fprintf(outfile, "\tmovw\t%s,%s",
	    rsnames[pnode->tn.rval], rsnames[newregno] );
    } else if( type & (TCHAR|TUCHAR) ) {
	fprintf(outfile, "\tmovb\t%s,%s",
	    rcnames[pnode->tn.rval], rcnames[newregno] );
    } else {
	cerror( "bad rs_move" );
	/*NOTREACHED*/
    }
    
    if( zflag ) {           /* if commenting on source of lines */
	emit_str( "\t\t/ RS_MOVE\n");
    }
    putc('\n',outfile );
}
void
defnam(p)
NODE *p;
{
        CONSZ flags;
        char *name;             /*define this symbol*/
        int size;
        int alignment;
        name = p->tn.name;
        flags = p->tn.lval;
        size = p->tn.rval;

        alignment = gtalign(p->in.type);
        if (flags & EXTNAM)
		fprintf(outfile,"\t.globl	%s\n", exname(name));
        if (flags & COMMON)
        {
                /*make sure the size is a multiple of the alignment.
                  This is because ld uses the size to determine the alignment
                  of unresolved .comms*/
                if ( size % alignment)
                {
                   size += (alignment - (size % alignment));
                }
                fprintf(outfile,
#ifdef	ELF_OBJ
		"\t.comm	%s,%d,%d\n", exname(name), size/SZCHAR, alignment/SZCHAR);
#else
                "\t.comm	%s,%d\n", exname(name), size/SZCHAR);
#endif
        }
#ifndef ELF_OBJ
        else if (flags & ICOMMON)
        {
                int curloc;
                char *ex;
                if ( (curloc = locctr(CURRENT)) != DATA && curloc != CDATA)
                        cerror(
                        "initialized common not in data: not implemented");
                ex = exname(name);
                fprintf(outfile,"\t.icomm       %s,.data\n", ex);
        	defalign(alignment);
		fprintf(outfile,"%s:\n", ex);
        }
#endif
	else if (flags & LCOMMON)
	{
#ifdef  ELF_OBJ
		fprintf(outfile, "\t.local	%s\n", name);
		fprintf(outfile, "\t.comm	%s,%d,%d\n", name, size/SZCHAR,
							     alignment/SZCHAR);
#else
#if 1
		fprintf(outfile,
		"\t.lcomm\t%s,%d,%d\n", exname(name), size/SZCHAR, alignment/SZCHAR);
#else	/* if no .lcomm support present */
		defalign(alignment);
		fprintf(outfile, "%s:\n",exname(name));
		zecode(size / SZCHAR);
#endif
#endif  /* ELF_OBJ */
	}
        else if (flags & DEFINE)
	{
        	defalign(alignment);
                fprintf(outfile,"%s:\n", exname(name));
	}
}

void
definfo(p)
NODE *p;
{
#ifdef  ELF_OBJ
	int type = p->tn.rval;
	CONSZ size = p->tn.lval;
	char *name = p->tn.name;

	/* print out "type" information - function or object */
	if (type & NI_FUNCT) {
		/* make sure in a right section for output "size" info later. */
		(void)locctr(PROG);
		fprintf(outfile, "\t.type	%s,@function\n", name);
	}
	else if ( (type & NI_OBJCT) && (type & (NI_GLOBAL|NI_FLSTAT)) )
		fprintf(outfile, "\t.type	%s,@object\n", name);

	/* print out "size" information for function or object */
	if (! (type & NI_BKSTAT) ) {
		if (!size)
			/* this may be used for function size.
			* it should be directed to the right location counter.
			*/
			fprintf(outfile, "\t.size	%s,.-%s\n", name, name);
		else
			fprintf(outfile, "\t.size	%s,%ld\n", name, size/SZCHAR);
	}
#endif
}

getlab()
{
	static int labels = 1;
	return labels++;
}

int
tyreg(t)
TWORD t;
{
				/*Given a type, return:
				0 if it cannot be a register variable.
				#regs to hold it if it can be a reg variable*/
	if ( !canbereg(t))
		return 0;
	else
		return szty(t);
}

int
canbereg(t)
TWORD t;
{
        register int *typ ;
                        /*can this type be a register variable?*/
                        /*can't use sw/case becuase some types are identical*/
        static int oktypes[]= {
                TCHAR , TSHORT , TINT , TLONG , TUCHAR , TUSHORT ,
                TUNSIGNED , TULONG , TPOINT , TPOINT2, 0 }; 
        for ( typ = oktypes; *typ; ++typ)
        {
                if ( t == *typ)
                        return 1;
        }
        return 0;
}

char *
reglst(t)
TWORD t;
{
				/* return a list of registers that can be used 
				   for register variables*/
	static char list[TOTREGS];
	register int r;
	char canbe = canbereg(t);
	int size = tyreg(t);

	for(r=0; r<TOTREGS; ++r)
		list[r]=0;

	for ( r=0; r<USRREGHI; ++r)
	{
		list[r] = (canbe
			&& (r >= NRGS)
				&& ( r+size <= USRREGHI) );
	}
	return list;
}

rgsave(request)
register char *request;
{
				/* Figure out how many registers have been used.
				   But registers may not be contiguous, because 
			 	   character type register variable can only be 
				   %bl. */
	int rgs=0;
	register int i;

	if (! request[NRGS] ) rgs = 1;   /* this is for character type */
	for( i = NRGS+1; ( ! request[i] )  && i<USRREGHI; ++i)
		;

	rgs += (USRREGHI - i);
	return rgs;
}


			/*For clever block moves: a list of opcodes,
			  their alignments, an the amount they move*/
static struct {
	char * opcode;	/*the opcode that does the move*/
	int alignment;	/* the alignment neccessary for the move*/
	int count;	/* the number of bytes moved*/
} opcodes[] = {
		{ "movl",	ALINT,		(SZINT/SZCHAR)	},
		{ "movw",	ALSHORT,	(SZSHORT/SZCHAR) },
		{ "movb",	ALCHAR,		1}
	};
#define LASTOP 3		/*end of the opcodes table*/
#define BTHRESH 6

static void
blockmove(pnode)
NODE *pnode;
{
			/*Do a block move.  from and to are scratch
			  registers;  count is either a scratch register
			  or a constant.*/
	NODE *pcount, *pfrom, *pto;	/*node pointers */
	extern RST regvar;
	int pushed_esi = 0;
	int pushed_edi = 0;
	int align = gtalign(pnode->in.type);	/*promised alignment*/
	int move_size = 1;	/*number of bytes moved each time*/
			/*This can be either a block move, or
			  VLRETURN node*/
			/*For block moves, the count may or may not
			  be a scratch register; if not, a1 is available
			  for a copy;
			/*For VLRETURN, the count must be in a scratch reg;
			  A1 contains the "to" adress (%fp)*/
#ifdef VOL_SUPPORT
                int bk_vol_opnd = 0;
#endif

	switch ( pnode->in.op)
	{
	case BMOVE:
	case BMOVEO:
#ifdef VOL_SUPPORT
                if (pnode->in.strat & VOLATILE) bk_vol_opnd |= VOL_OPND2;
#endif
		pcount = pnode->in.left;
		pfrom = pnode->in.right->in.right;
		pto = pnode->in.right->in.left;
		break;
	default:
		cerror("Bad node passed to ZM macro");
	};

			/*First, try to be smart about short structures*/
			/*We can do this if the count is a constant ,
			  the move is properly aligned,
			  and the size of the move is small enough*/

	if ( pcount->tn.op == ICON && pcount->tn.name == 0)
	{
		int i;
			/*How many bytes to move?*/
		int move_count = pcount->tn.lval;
			/*First, find out the largest opcode consistent with
			  the alignment*/
		for ( i=0; opcodes[i].alignment > align; ++i)
		{
			if ( i >= LASTOP)
				cerror("blockmove: bad alignment");
		}
			/*Is it small enough to unroll?*/
		if ( move_count / opcodes[i].count < BTHRESH)
		{
			int done;
			move_size = opcodes[i].count;
			for ( done = 0; done < move_count; )
			{
				/*Must we change op size?*/
				while ( done+move_size > move_count)
				{
					if ( i >= LASTOP)
						cerror("blockmove: bad move count");
					move_size = opcodes[++i].count;
				}
				fprintf(outfile, "\t%s\t%d(%s),",
				opcodes[i].opcode, done, rnames[pfrom->tn.rval]);

				switch (move_size)
				{
				case 4:
					fprintf(outfile, "%%ecx\n\t%s\t%%ecx,",
						opcodes[i].opcode);
					break;
				case 2:
					fprintf(outfile, "%%cx\n\t%s\t%%cx,",
						opcodes[i].opcode);
					break;
				case 1:
					fprintf(outfile, "%%cl\n\t%s\t%%cl,",
						opcodes[i].opcode);
					break;
				default:
					cerror("blockmove: incorrect move size");
				}
				
				fprintf(outfile, "%d(%s)\n", done, rnames[pto->tn.rval]);
#ifdef VOL_SUPPORT
                                if (bk_vol_opnd) fprintf(outfile,"/VOL_OPND\t2\n");
#endif
				done += move_size;
			}
			return;
		}
			/*Can't unroll. Can we use a different opcode?*/
			/*Can use an opcode if the number of bytes we move
			  is a multiple of the number of bytes the opcode
			  moves.*/
		for ( ; move_count % opcodes[i].count ; ++i)
		{
			if ( i >= LASTOP)
				cerror("blockmove: Bad move count (rolled)");
		}
		move_size = opcodes[i].count;
	}

	if (regvar & RS_BIT(REG_ESI))
	{
		fprintf(outfile, "\tpushl\t%%esi\n");
		pushed_esi = 1;
	}
	if (regvar & RS_BIT(REG_EDI))
	{
		fprintf(outfile, "\tpushl\t%%edi\n");
		pushed_edi = 1;
	}
	regstused |= RS_BIT(REG_ESI);
	regstused |= RS_BIT(REG_EDI);
	
#ifdef VOL_SUPPORT
        if (bk_vol_opnd) fprintf(outfile,"/VOL_OPND\t2\n");
#endif
	fprintf(outfile, "\tmovl\t%s,%%esi\n", rnames[pfrom->tn.rval] );
	fprintf(outfile, "\tmovl\t%s,%%edi\n", rnames[pto->tn.rval] );
	fprintf(outfile, "\tmovl\t$%ld,%%ecx\n\trep\n", pcount->tn.lval/move_size);
	expand( pnode, FOREFF, "\tsmovZT.\n", (OPTAB *) 0 );

	if (pushed_edi)
		fprintf(outfile, "\tpopl\t%%edi\n");
	if (pushed_esi)
		fprintf(outfile, "\tpopl\t%%esi\n");
}

void
blockcmp(pnode)
NODE *pnode;
{
	int lastlab = getlab();
	int looplab = getlab();
	NODE *count, *from, *to;
		/*Stolen from the STASG code.*/
		/*All three inputs are in scratch registers*/
	from = pnode->in.right->in.right;
	to = pnode->in.right->in.left;

		/*Figure out the count*/
	count = pnode->in.left;

			/*generate the loop*/
	deflab(looplab);
        fprintf(outfile,"\tcmpl\t&0,%s\n",rnames[count->tn.rval]);
        fprintf(outfile,"\tjz\t.L%d\n",lastlab);
	fprintf(outfile,"\tcmpb\t0(%s),0(%s)\n",
		rnames[from->tn.rval], rnames[to->tn.rval]);
        fprintf(outfile,"\tjne\t.L%d\n",lastlab);
        fprintf(outfile,"\tsubl2\t&1,%s\n",rnames[count->tn.rval]);
        fprintf(outfile,"\taddl2\t&1,%s\n",rnames[from->tn.rval]);
        fprintf(outfile,"\taddl2\t&1,%s\n",rnames[to->tn.rval]);
	fprintf(outfile,"\tjmp\t.L%d\n",looplab);
	deflab(lastlab);

}

void
end_icommon()
{
	(void) locctr(locctr(UNK));
}

costex()
{
	cerror("costex(): not ported for i386 CG\n");
	/*NOTREACHED*/
}
#ifdef VOL_SUPPORT
/* output volatile operand information at the end of an instruction */
void
vol_instr_end()
{
        int opnd;
        int first = 1;
        for (opnd=0; vol_opnd; ++opnd)
        {
            if (vol_opnd & (1<<opnd))
            {
                   /* first time output the information for the instruction */
                   if ( first )
                   {
                        PUTS("/VOL_OPND ");
                        first = 0;
                   }
                   else
                        PUTCHAR(',');
                   vol_opnd &= ~(1<<opnd);      /* clean up the checked operand bit */
                   fprintf(outfile, "%d", opnd+1);
            }
        }
        if ( !first ) PUTCHAR('\n');
        VOL_CLEAN();    /* reset the initial values for bookkeeping variables */
}
#endif /* VOL_SUPPORT */

/* Routines to support HALO optimizer. */

#ifdef	OPTIM_SUPPORT

#ifndef	INI_OIBSIZE
#define	INI_OIBSIZE 100
#endif

static char oi_buf_init[INI_OIBSIZE];	/* initial buffer */
static
TD_INIT(td_oibuf, INI_OIBSIZE, sizeof(char), 0, oi_buf_init, "optim support buf");

#define	OI_NEED(n) if (td_oibuf.td_used + (n) > td_oibuf.td_allo) \
			td_enlarge(&td_oibuf, td_oibuf.td_used+(n)) ;
#define	OI_BUF ((char *)(td_oibuf.td_start))
#define	OI_USED (td_oibuf.td_used)

/* Produce comment for loop code. */

char *
oi_loop(code)
int code;
{
    char * s;

    switch( code ) {
    case OI_LSTART:	s = "/LOOP	BEG\n"; break;
    case OI_LBODY:	s = "/LOOP	HDR\n"; break;
    case OI_LCOND:	s = "/LOOP	COND\n"; break;
    case OI_LEND:	s = "/LOOP	END\n"; break;
    default:
	cerror("bad op_loop code %d", code);
    }
    return( s );
}


/* Analog of adrput, but this one takes limited address modes (leaves
** only) and writes to a buffer.  It returns a pointer to just past the
** end of the buffer.
*/
static void
sadrput(p)
NODE * p;				/* node to produce output for */
{
    int n;

    /* Assume need space for auto/param at a minimum. */
    /*      % n ( % ebp) NUL */
    OI_NEED(1+8+1+1+ 3+1+1);

    switch( p->tn.op ){
    case VAUTO:
    case VPARAM:	OI_USED += sprintf(OI_BUF+OI_USED, "%ld(%%ebp)",
								p->tn.lval);
			break;
    case NAME:		n = strlen(p->tn.name);
			OI_NEED(n+1);
			(void) strcpy(OI_BUF+OI_USED, p->tn.name);
			OI_USED += n;
			if (p->tn.lval != 0) {
			    OI_NEED(1+8+1);
			    OI_USED += sprintf(OI_BUF+OI_USED, "%s%ld",
					(p->tn.lval > 0 ? "+" : ""),
					p->tn.lval);
			}
			if (   picflag
			    && (p->tn.rval & (NI_GLOBAL))) {
			    OI_NEED(4+1);
			    (void) strcpy(OI_BUF+OI_USED, "@GOT");
			    OI_USED += 4;
			}
			else if (   picflag
			    && (p->tn.rval & (NI_FLSTAT|NI_BKSTAT))) {
			    OI_NEED(7+1);
			    (void) strcpy(OI_BUF+OI_USED, "@GOTOFF");
			    OI_USED += 7;
			}
			break;
    default:
	cerror("bad op %d in sadrput()", p->in.op);
    }
    return;
}

#ifndef	TLDOUBLE
#define	TLDOUBLE TDOUBLE
#endif

/* Note that the address of an object was taken. */

char *
oi_alias(p)
NODE * p;
{
    BITOFF size = (p->tn.type & (TVOID|TSTRUCT)) ? 0 : gtsize(p->tn.type);

    OI_USED = 0;			/* start buffer */
    /*	    /ALIAS\t	*/
    OI_NEED(1+   5+1+1);
    OI_USED += sprintf(OI_BUF, "/ALIAS	");
    sadrput(p);
    /*	    \t% n\tFP\n */
    OI_NEED(1+1+8+1+2+1+1);
    (void) sprintf(OI_BUF+OI_USED, "	%ld%s\n", size/SZCHAR,
		(long)(p->tn.type & (TFLOAT|TDOUBLE|TLDOUBLE)) ? "	FP" : "");
    return( OI_BUF );
}

/* Produce #REGAL information for a symbol. */

char *
oi_symbol(p, class)
NODE * p;
int class;
{
    char * s_class;

    switch( class ) {
    case OI_AUTO:	s_class = "AUTO"; break;
    case OI_PARAM:	s_class = "PARAM"; break;
    case OI_EXTERN:	s_class = "EXTERN"; break;
    case OI_EXTDEF:	s_class = "EXTDEF"; break;
    case OI_STATEXT:	s_class = "STATEXT"; break;
    case OI_STATLOC:	s_class = "STATLOC"; break;
    default:
	cerror("bad class %d in op_symbol", class);
    }

    OI_USED = 0;			/* initialize */
    /*		/REGAL\t 0\tSTATLOC\t	*/
    OI_NEED(	1+   5+1+1+1+     7+1+1 );
    OI_USED += sprintf(OI_BUF, "/REGAL	0	%s	", s_class);
    sadrput(p);
    /*	    \t% n\tFP\n */
    OI_NEED(1+1+8+1+2+1+1);
    (void) sprintf(OI_BUF+OI_USED, "	%d%s\n", gtsize(p->tn.type)/SZCHAR,
		(p->tn.type & (TFLOAT|TDOUBLE|TLDOUBLE)) ? "	FP" : "");
    return( OI_BUF );
}

#endif	/* def OPTIM_SUPPORT */
