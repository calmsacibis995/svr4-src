/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cg:i386/local.c	1.32"
/*	local.c - machine dependent stuff for front end
 *	i386 CG
 *		Intel iAPX386
 */

#include <signal.h>
#include "mfile1.h"
#include "mfile2.h"
#include <string.h>
#include <memory.h>

/* register numbers in the compiler and their external numbers:
**
**	comp	name
**	0-2	eax,edx,ecx
**	3	fp0
**	5-7	ebx,esi,edi
*/

#ifndef TMPDIR
#define TMPDIR	"/tmp"		/* to get TMPDIR, directory for temp files */
#endif

/* bit vectors for register variables */

#define REGREGS		(RS_BIT(REG_EBX)|RS_BIT(REG_ESI)|RS_BIT(REG_EDI))
#define CREGREGS	(RS_BIT(REG_EBX))

/* *** i386 *** */
RST regstused = RS_NONE;
RST regvar = RS_NONE;
/* *** i386 *** */

int tmp_start;			/* start of temp locations on stack */
int swregno;			/* switch value register */
static char request[TOTREGS];	/* list of reg vars used by the fct.
				   Given on either BEGF node or ENDF node */
int r_caller[]={-1};		/* no caller save register */
static int biggest_return;

static void jmplab();
static void makeheap();
static void walkheap();

static char *tmpfn;
static FILE *tmpfp;

FILE *fopen();
int proflag = 0;
int picflag = 0;
int gotflag;

#if 0

void
myexit(n)
{
	(void) unlink(tmpfn);

	if (n == 1)
		n = 51;
	exit(n);
	/*NOTREACHED*/
}

/*ARGSUSED*/
static void
getout(i)
{
	myexit(55);
	/*NOTREACHED*/
}

/*ARGSUSED*/
static void
catch_fpe(i)
{
    uerror("floating point constant folding causes exception");
    myexit( 1 );
    /*NOTREACHED*/
}

extern int singflag;	/* flag for turning on single precision floating arith */
extern void pack_string();

beg_file()
{
	/* called as the very first thing by the parser to do machine
	 * dependent stuff
	 */
	register char *p, *s;


	/* catch signals if they're not now being ignored */

	if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
	    signal(SIGHUP, getout);
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
	    signal(SIGINT, getout);
	if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
	    signal(SIGQUIT, getout);
	if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
	    signal(SIGTERM, getout);

	/* catch floating error */

	if (signal(SIGFPE, SIG_IGN) != SIG_IGN)
	    signal(SIGFPE, catch_fpe);

			/* note: double quotes already in ftitle... */
	p = ftitle + strlen( ftitle ) - 2;
	s = p - 14;	/* max name length */
	while ( p > s && *p != '"' && *p != '/' )
		--p;
	fprintf(outfile, "\t.file\t\"%.15s\n", p + 1 );
	if(!(tmpfp= fopen(tmpfn = tempnam( TMPDIR , "pcc2S"), "w")))
		cerror("can't create string file\n");
}
#endif

void
p2abort()
{
	extern int unlink();
	if (tmpfp)
		(void) unlink(tmpfn);
	return;
}

void
myflags(cp)
char *cp;
{
	while (*cp) {
		switch (*cp)
		{
			case 'k':	picflag = 1; break;
			default:	break;
		}
		++cp;
	}
}

static char *locnames[] =
{
#ifdef ELF_OBJ
/*PROG*/	"	.text\n",
/*ADATA*/	"	.data\n",
/*DATA*/	"	.data\n",
/*ISTRNG*/	"	.section	.data1\n",
/*STRNG*/	"	.section	.data1\n",
/*CDATA*/	"	.section	.rodata\n",	/* read-only data */
/*CSTRNG*/	"	.section	.rodata1\n"	/* read-only strings */

#else
/*PROG*/	"	.text\n",
/*ADATA*/	"	.section	.data1,\"w\"\n",
/*DATA*/	"	.data\n",
/*ISTRNG*/	"	.data	1\n",
/*STRNG*/	"	.data	1\n",
#	ifdef RODATA
/*CDATA*/	"	.section	.rodata,\"x\"\n",	/* read-only data */
/*CSTRNG*/	"	.section	.rodata1,\"x\"\n"	/* read-only strings */
#	else
/*CDATA*/	"	.data\n",
/*CSTRNG*/	"	.data	1\n"
#       endif

#endif
};

#ifdef ELF_OBJ
# define FIRST_ISTR	"\t.section	.data1,\"aw\"\n"
# define FIRST_CSTR	"\t.section	.rodata1,\"a\"\n"
#endif

static int lastalign = -1;

/* location counters for PROG, ADATA, DATA, ISTRNG, STRNG, CDATA, and CSTRNG */
locctr(l)		/* output the location counter */
{
#ifdef  ELF_OBJ
	static int first_istring = 1;
	static int first_cstring = 1;
#endif
	static int lastloc = UNK;
	int retval = lastloc;		/* value to return at end */
#if defined(RODATA) && !defined(ELF_OBJ)
	static int lasttmploc = UNK;
#endif

	curloc = l;
	if (curloc != lastloc) lastalign = -1;

	switch (l)
	{
	case CURRENT:
		return ( retval );

	case PROG:
		lastalign = -1;
		/* FALLTHRU */
	case ADATA:
	case CDATA:
	case DATA:
		if (lastloc == l)
			break;
		outfile = textfile;
		if (picflag) {
		    if (curloc == CDATA) {
			curloc = DATA;
			emit_str(locnames[DATA]);
		    }
		    else
                        emit_str(locnames[l]);
                }
		else
		    emit_str(locnames[l]);
		break;

	case FORCE_LC(CDATA):
		if (lastloc == l)
			break;
		outfile = textfile;
		emit_str(locnames[CDATA]);
		break; 

	case STRNG:
	case ISTRNG:
	case CSTRNG:
		/* output string initializers to a temporary file for now
		 * don't update lastloc
		 */
		if (lastloc == l)
			break;
#ifdef	ELF_OBJ
		outfile = textfile;
		if (curloc == CSTRNG && first_cstring)
		{
			emit_str(FIRST_CSTR);
			first_cstring = 0;
		}
		else if ( ((curloc == ISTRNG)||(curloc == STRNG)) && first_istring )
		{
			emit_str(FIRST_ISTR);
			first_istring = 0;
		}
		else
			emit_str(locnames[l]);
#else
		if (! tmpfp)
		   if(!(tmpfp= fopen(tmpfn = tempnam( TMPDIR , "pcc2S"), "w")))
			cerror("can't create string file\n");
		outfile = tmpfp;
#	ifdef RODATA
		if (lasttmploc == curloc)
			break;
		if (curloc == CSTRNG)
			emit_str("	.section	.rodata,\"x\"\n");
		else
			emit_str("	.data\n");
		lasttmploc = curloc;
#	endif
#endif
		break;

	case UNK:
		break;

	default:
		cerror( "illegal location counter" );
	}

	lastloc = l;
	return( retval );		/* declare previous loc. ctr. */
}

/* Can object of type t go in a register?  rbusy is array
** of registers in use.  Return -1 if not in register,
** else return register number.  Also fill in rbusy[].
*/
/*ARGSUSED*/
int
cisreg(op, t, off, rbusy)
int op;					/* not used */
TWORD t;
OFFSET off;				/* not used */
char rbusy[TOTREGS];
{
	int i;

	if (picflag)
		rbusy[BASEREG] = 1;

	if ( t & ( TSHORT | TUSHORT
		 | TINT   | TUNSIGNED
		 | TLONG  | TULONG
		 | TPOINT | TPOINT2) )
	{
		/* Have a type we can put in register. */
		for (i = USRREGHI-1; i >= NRGS; --i)
			if (!rbusy[i]) break;
	
		/* If i >= NRGS, i is the register number to
		** allocate.
		*/
	
		/* If candidate is suitable number grab it, adjust rbusy[]. */
		if (i >= NRGS) {
			rbusy[i] = 1;
			regvar |= RS_BIT(i);
			return( i );
		}
	}
	else
	if ( t & ( TCHAR | TUCHAR ) )
	{
		if (! rbusy[NRGS] ) {
			rbusy[NRGS] = 1;
			return( NRGS );
		}
	}

	/* Bad type or no register to allocate. */
	return( -1 );
}

NODE *
setswreg( p )
NODE *p;
{
	swregno = 0;
	return( p );
}

static int prot_nest=0;	/*nesting level for protections*/
void
protect(p)
NODE *p;
{
			/*Leave a marker telling optimizers (both LION
			  and pci, a.out optimizers) to keep
			  their paws off of this code. */
			/*calls to this routine nest, i.e. two protect()s
			  require two unprot() calls before optimizers
			  will be let loose again*/
			/*For 3b20: hard labels have a different syntax.
			  this is due to an optim bug that will remove
			  some labels even is \ASMs*/
	if ( (p->in.op == GENLAB) || (p->in.op == LABELOP) )
	{
		emit_str("/HARD\t");
		fprintf(outfile,LABFMT, p->bn.label);
		emit_str("\n");
	}
	else if ( !(prot_nest++))
	{
		fprintf(outfile,"%s\n", PROT_START);
	}
}
void
unprot(p)
NODE *p;
{
			/*label protection is the /HARD; no trailing
			  string needed*/
	if (p->in.op == GENLAB)
		return;

	switch( --prot_nest)
	{
	case -1:
		cerror("unprot() called without matching protect()");
		/*NOTREACHED*/
	case 0:
		fprintf(outfile,"%s\n", PROT_END);
	/*default: just decrement the prot_nest count*/
	}
}


NODE *
clocal(p)			/* manipulate the parse tree for local xforms */
register NODE *p;
{
	register NODE *l, *r;

	/* make enum constants into hard ints */

	if (p->in.op == ICON && p->tn.type == ENUMTY) {
	    p->tn.type = INT;
	    return( p );
	}

	if (!asgbinop(p->in.op) && p->in.op != ASSIGN)
		return (p);
	r = p->in.right;
	if (optype(r->in.op) == LTYPE)
		return (p);
	l = r->in.left;
	if (r->in.op == QUEST ||
		(r->in.op == CONV && l->in.op == QUEST) ||
		(r->in.op == CONV && l->in.op == CONV &&
		l->in.left->in.op == QUEST))
				/* distribute assigns over colons */
	{
		register NODE *pwork;
		extern NODE * tcopy();
		NODE *pcpy = tcopy(p), *pnew;
		int astype = p->in.type;	/* remember result type of asgop */
#ifndef NODBG
		extern int xdebug;
		if (xdebug)
		{
			emit_str("Entering [op]=?: distribution\n");
			e2print(p);
		}
#endif
		pnew = pcpy->in.right;
		while (pnew->in.op != QUEST)
			pnew = pnew->in.left;
		/*
		* pnew is top of new tree
		*/

		/* type of resulting ?: will be same as original type of asgop.
		** type of : must be changed, too
		*/
		pnew->in.type = astype;
		pnew->in.right->in.type = astype;

		if ((pwork = p)->in.right->in.op == QUEST)
		{
			tfree(pwork->in.right);
			pwork->in.right = pnew->in.right->in.left;
			pnew->in.right->in.left = pwork;
			/* at this point, 1/2 distributed. Tree looks like:
			*		ASSIGN|ASGOP
			*	LVAL			QUEST
			*		EXPR1		COLON
			*			ASSIGN|ASGOP	EXPR3
			*		LVAL		EXPR2
			* pnew "holds" new tree from QUEST node
			*/
		}
		else
		{
			NODE *pholdtop = pwork;

			pwork = pwork->in.right;
			while (pwork->in.left->in.op != QUEST)
				pwork = pwork->in.left;
			tfree(pwork->in.left);
			pwork->in.left = pnew->in.right->in.left;
			pnew->in.right->in.left = pholdtop;
			/* at this point, 1/2 distributed. Tree looks like:
			*		ASSIGN|ASGOP
			*	LVAL			ANY # OF CONVs
			*			QUEST
			*		EXPR1		COLON
			*			ASSIGN|ASGOP	EXPR3
			*		LVAL		ANY # OF CONVs
			*			EXPR2
			* pnew "holds" new tree from QUEST node
			*/
		}
		if ((pwork = pcpy)->in.right->in.op == QUEST)
		{
			pwork->in.right = pnew->in.right->in.right;
			pnew->in.right->in.right = pwork;
			/*
			* done with the easy case
			*/
		}
		else
		{
			NODE *pholdtop = pwork;

			pwork = pwork->in.right;
			while (pwork->in.left->in.op != QUEST)
				pwork = pwork->in.left;
			pwork->in.left = pnew->in.right->in.right;
			pnew->in.right->in.right = pholdtop;
			/*
			* done with the CONVs case
			*/
		}
		p = pnew;
#ifndef NODBG
		if (xdebug)
		{
			emit_str("Leaving [op]=?: distribution\n");
			e2print(p);
		}
#endif
	}
	return(p);
}

/* Simple initialization.  We're handed an INIT node. */
void
sincode(p)
NODE * p;
{
    int sz;
    if (p->in.op != INIT || ((p=p->in.left)->in.op != ICON && p->in.op != FCON) )
        cerror("sincode:  got funny node\n");

    sz = gtsize(p->in.type);
    if (p->in.op == FCON)
    {
	fincode(p->fpn.dval, sz);
	return;
    }

    switch ( sz )
    {
    case SZCHAR:
	emit_str("	.byte	");
	break;
    case SZSHORT:
	emit_str("	.value	");
	break;
    case SZINT:
	emit_str("	.long	");
	break;
    case SZDOUBLE:
	emit_str("	.double	");
	break;
    default:
	cerror("sincode: bad size for initializer");
    }
    acon(p);			/* output constant appropriately */
    putc('\n', outfile);
    inoff += sz;
    return;
}

void
fincode(d, sz)		/* floating initialization */
double d;
int sz;
{
#if !(defined(vax) || (defined(uts) && !defined(FP_EMULATE)))
        union { FP_DOUBLE d; FP_FLOAT f; int i[2]; } cheat;

        if (sz == SZDOUBLE)
        {
		/* the 'byteorder'declared below is for different byte order 
		   machines to output correct order of double type floating 
		   point number in IEEE format.
		*/
		static union { short s; char addr[sizeof(short)]; } byteorder;

		/* this assignment to make addr[0] and addr[1] contain
	 	   either 0 or 1 according to the byte order of the
		   host machine.
		*/
		byteorder.s = 0x0001;

                cheat.d = d;

		/* to output the correct order of the double type 
		   floating point number in IEEE format.  The order 
		   is according to the host machine's byte order 
		   contained in byteorder.addr[1] and byteorder.addr[0].
		*/
		fprintf(outfile,"\t.long\t0x%x,0x%x\n", 
			cheat.i[byteorder.addr[1]], cheat.i[byteorder.addr[0]]);
        }
        else
        {
                cheat.f = FP_DTOF(d);
                fprintf(outfile,"\t.long\t0x%x\n", cheat.i[0]);
        }
#else
	fprintf(outfile,"\t.%s\t%.15e\n", sz == SZDOUBLE ? "double" : "float", d);
#endif
}

char *
exname(p)			/* a name using external naming conventions */
char *p;
{
    return( p );
}


#if 0
void
branch(n)			/* branch to label n or return */
int n;
{
	jmplab(n);
}
#endif

void
defalign(n)			/* align to multiple of n */
int n;
{
	if ((n /= SZCHAR) > 1 && lastalign != n )
	     fprintf(outfile,"\t.align	%d\n", n);
	lastalign = n;
}

static void
jmplab(n)			/* produce jump to label n */
int n;
{
    fprintf(outfile, "	jmp	.L%d\n", n);
}
void
deflab(n)			/* label n */
int n;
{
	fprintf(outfile, ".L%d:\n", n);
}

static int	toplab;
static int	botlab;
static int	piclab;
static int	picpc;		/* stack temp for pc */
static int	efoff[TOTREGS];
void
efcode(p)			/* wind up a function */
NODE *p;
{
	extern int strftn;	/* non-zero if function is structure function,
				** contains label number of local static value
				*/
	register int i;
	int stack;

	if (p->in.name)
		memcpy(request, p->in.name, sizeof(request));
	if (picflag)
                request[BASEREG] = 1;

	stack = -(p->tn.lval) * SZCHAR;

	deflab(retlab);

	for (i = REG_EDI; i >= REG_EBX; i--)
		if (request[i]) {
			stack += SZINT;
			efoff[i] = -(stack/SZCHAR);
		}

	stack = -(p->tn.lval) * SZCHAR;
	SETOFF(stack, ALSTACK);

        /* Restore old 386 register variables */
        /* Must be reverse order of the register saves */

	/* %ebx not saved if picflag is set but there was not GOT reference */
        if (request[REG_EBX] && !(picflag && gotflag == NOGOTREF))
                emit_str("\tpopl\t%ebx\n");
        if (request[REG_ESI] || (regstused & RS_BIT(REG_ESI)) )
                emit_str("\tpopl\t%esi\n");
        if (request[REG_EDI] || (regstused & RS_BIT(REG_EDI)) )
                emit_str("\tpopl\t%edi\n");

	fprintf(outfile,"\tleave\n\tret/%d\n",biggest_return);

	/*
	 * The entry sequence according to the 387 spec sheet is always
	 * faster (by 4 cycles!) to do a push/movl/sub rather than an
	 * enter and occasionally a sub also.  Fastest enter is 10 cycles
	 * versus 2/2/2...
	 */
	deflab(botlab);
#ifndef	OLDTMPSRET
	if (strftn)
		emit_str("\tpopl\t%eax\n\txchgl\t%eax,0(%esp)\n");
#endif
	emit_str("\tpushl\t%ebp\n\tmovl\t%esp,%ebp\n");
	if (stack/SZCHAR > 0) {
		if (stack / SZCHAR == 4)
			emit_str("\tpushl\t%eax\n");
		else
			fprintf(outfile, "\tsubl\t$%d,%%esp\n", stack/SZCHAR);
	} 

	/* Save old 386 register variables */
	if (request[REG_EDI] || (regstused & RS_BIT(REG_EDI)) )
		emit_str("\tpushl\t%edi\n");
	if (request[REG_ESI] || (regstused & RS_BIT(REG_ESI)) )
		emit_str("\tpushl\t%esi\n");
	/* %ebx not saved if picflag is set but there was not GOT reference */
        if (request[REG_EBX] && !(picflag && gotflag == NOGOTREF))
		emit_str("\tpushl\t%ebx\n");

	regstused = RS_NONE;
	regvar = RS_NONE;
#ifdef	ELF_OBJ
	if (gotflag != NOGOTREF)
	{
	    fprintf(outfile,"\tcall	.L%d\n", piclab);
	    fprintf(outfile,".L%d:\n", piclab);
	    fprintf(outfile,"\tpopl	%s\n", rnames[BASEREG]);
	    if (gotflag & GOTSWREF)
		fprintf(outfile,"\tmovl    %s,%d(%%ebp)\n", rnames[BASEREG],picpc);
	    fprintf(outfile,"\taddl	$_GLOBAL_OFFSET_TABLE_+[.-.L%d],%s\n",
				piclab, rnames[BASEREG]);
	}
#endif
	jmplab(toplab);
}
void
bfcode(p)			/* begin function code. a is array of n stab */
NODE * p;
{
	extern void initfp();
	retlab = getlab();	/* common return point */
	toplab = getlab();
	botlab = getlab();
	gotflag = NOGOTREF;
	if (picflag) {
		piclab = getlab();
		picpc  = freetemp(1) / SZCHAR;	/* stack temp for pc */
	}
	jmplab(botlab);
	deflab(toplab);
	if (p->in.type == TSTRUCT)
	{
		/*Save place for structure return on stack*/
		strftn = 1;
		fprintf(outfile,"	movl	%s,%d(%%ebp)\n",rnames[AUXREG],str_spot);
	}
	if (proflag)
	{
	        int temp;

		emit_str("/ASM\n");
		emit_str("	.data\n");
		temp = getlab();
		emit_str("	.align	4\n");
		deflab(temp);
		emit_str("	.long	0\n	.text\n");
		if (picflag) {
		    fprintf(outfile,"	leal	.L%d@GOTOFF(%s),%%edx\n",temp, rnames[BASEREG]);
		    fprintf(outfile,"	call	*_mcount@GOT(%s)\n", rnames[BASEREG]);
		    gotflag |= GOTREF;
		}
		else {
		    fprintf(outfile,"	movl	$.L%d,%%edx\n",temp);
		    emit_str("	call	_mcount\n");
		}
		emit_str("/ASMEND\n");
	}

	initfp();			/* Init the floating point stack */
}
void
bycode(ch, loc)			/* byte ch into string location loc */
int ch, loc;
{
	if (ch < 0)		/* eos */
	{
		if (loc)
			putc('\n', outfile);
	}
	else
	{
		if ((loc % 10) == 0)
			emit_str("\n	.byte	");
		else
			putc(',', outfile);
		fprintf(outfile, "0x%.2x", ch);
	}
}

void
zecode(n) /* n bytes of 0, NOT in words */
register int n;
{
	if (n <= 0)             /* this is possible, folks */
		return;
#ifdef	ELF_OBJ
	fprintf(outfile,"	.zero	%d\n", n);
#else
	fprintf(outfile,"	.set	.,.+%d\n", n);
#endif
	inoff += n * SZINT;
}

void
begf(p) /*called for BEGF nodes*/
NODE *p;
{
                        /*save the used registers*/
	if (p->in.name)
        	memcpy(request, p->in.name, sizeof(request));
	else
		memset(request, 0, sizeof(request));
	if (picflag)
		request[BASEREG] = 1;

        (void) locctr(PROG);
        strftn = 0;
	biggest_return = 0;
}

/*ARGSUSED*/
void
bfdata(p) 
NODE *p;
{
}

void
myret_type(t)
TWORD t;
{
	if (! (t & (TVOID | TFLOAT | TDOUBLE | TFPTR)) )
	{
		if (!biggest_return)
			biggest_return = 1;
	}
}

#ifndef	INI_HSWITSZ
#define	INI_HSWITSZ 25
#endif

static struct sw heapsw_init[INI_HSWITSZ];

#ifndef STATSOUT
static
#endif
TD_INIT( td_heapsw, INI_HSWITSZ, sizeof(struct sw),
		0, heapsw_init, "heap switch table");
#define HSWITSZ (td_heapsw.td_allo)
#define heapsw ((struct sw *)(td_heapsw.td_start))


void		
genswitch(p, n)
register struct sw *p;
int n;
{
	/* p points to an array of structures, each consisting	*/
	/* of a constant value and a label. 			*/
	/* The first is >=0 if there is a default label; its	*/
	/* value is the label number. The entries p[1] to p[n]	*/
	/* are the nontrivial cases				*/

	register int i;
	register CONSZ j;
	unsigned long range;		/* room for MAXINT - (-MAXINT) */
	register int dlab, swlab;

	range = p[n].sval-p[1].sval;
	if (range != 0 && range <= (3 * n) && n >= 4)
	{				/* implement a direct switch */
		dlab = (p->slab >= 0) ? p->slab : getlab();
		if (p[1].sval)
			fprintf(outfile,"\tsubl\t$%ld,%%eax\n", p[1].sval);
		swlab = getlab();
		fprintf(outfile,"\tcmpl\t$%ld,%%eax\n\tja\t.L%d\n", range, dlab);
/*
		fprintf(outfile,"\tcmpl\t$0,%%eax \n\tjl\t.L%d\n", dlab);
*/
		if (picflag) {
		    fprintf(outfile, "\tleal	.L%d@GOTOFF(%s),%%edx\n",swlab, rnames[BASEREG]);
		    fprintf(outfile, "\tmovl	(%%edx,%%eax,4),%%eax\n");
		    fprintf(outfile, "\taddl	%d(%%ebp),%%eax\n", picpc);
		    fprintf(outfile, "\tjmp	*%%eax\n");
		    gotflag |= GOTSWREF;;	
		}
		else
		    fprintf(outfile,"\tjmp\t*.L%d(,%%eax,4)\n", swlab);
		(void) locctr(FORCE_LC(CDATA));
		defalign(ALPOINT);
		emit_str("/SWBEG\n");
		deflab(swlab);
		for (i = 1, j = p[1].sval; i <= n; ++j)
		     if (picflag)
			fprintf(outfile,"	.long	.L%d-.L%d\n",
			    (j == p[i].sval) ? p[i++].slab : dlab, piclab);
		     else
			fprintf(outfile,"	.long	.L%d\n",
			    (j == p[i].sval) ? p[i++].slab : dlab );
		emit_str("/SWEND\n");
		(void) locctr(PROG);
		if (p->slab < 0)
			deflab(dlab);
	}
	else if ( n > 8 )
	{
		heapsw[0].slab = dlab = p->slab >= 0 ? p->slab : getlab();
		makeheap( p, n );	/* build heap */
		walkheap( 1, n );	/* produce code */
		if( p->slab >= 0 )
			jmplab( dlab );
		else
			deflab( dlab );
	}
	else					/* simple switch code */
	{
		for (i = 1; i <= n; ++i)
			fprintf(outfile,"\tcmpl\t$%ld,%%eax\n\tje\t.L%d\n",
				p[i].sval, p[i].slab);
		if (p->slab >= 0)
			jmplab(p->slab);
	}
}

static void
makeheap( p, n )
struct sw * p;				/* point at default label for
					** current switch table
					*/
int n;					/* number of cases */
{
    static void make1heap();

    if (n+1 > HSWITSZ)			/* make sure table is big enough */
	td_enlarge(&td_heapsw, n+1);

#ifdef STATSOUT
    if (td_heapsw.td_max < n) td_heapsw.td_max = n;
#endif

    make1heap( p, n, 1 );		/* do the rest of the work */
    return;
}

static void
make1heap( p, m, n )
	register struct sw *p;
{
    register int q = select( m );

    heapsw[n] = p[q];

    if( q > 1 )
	make1heap( p, q-1, 2*n );
    if (q < m )
	make1heap( p+q, m-q, 2*n+1 );
}

static int
select( m )
{
        register int l, i, k;

        for( i=1; ; i*=2 )
                if( (i-1) > m ) break;
        l = ((k = i/2 - 1) + 1)/2;
        return( l + (m-k < l ? m-k : l) );
}

static void
walkheap( start, limit )
{
	int label;

	if( start > limit )
		return;
	fprintf( outfile, "\tcmpl\t$%ld,%%eax\n\tje\t.L%d\n",
		heapsw[start].sval, heapsw[start].slab );
	if( (2*start) > limit )
	{
		fprintf( outfile, "	jmp	.L%d\n", heapsw[0].slab );
		return;
	}
	if( (2*start+1) <= limit )
	{
		label = getlab();
		fprintf( outfile, "	jg	.L%d\n", label );
	}
	else
		fprintf( outfile, "	jg	.L%d\n", heapsw[0].slab );
	walkheap( 2*start, limit );
	if( (2*start+1) <= limit )
	{
		fprintf( outfile, ".L%d:\n", label );
		walkheap( 2*start+1, limit );
	}
}

#if 0

void
genladder( range_array, range_l, default_label)
struct case_range range_array[];
int range_l, default_label;
{
			/*Given the sorted array of case ranges, construct
			  a ladder that tests each case*/
	int around;
	int previous_upper_bound = range_array->lower_bound;
	register struct case_range *ranges;
					/* initialize to make first test fail */
	if (default_label == -1)
		around = getlab();
	else
		around = default_label;

	for (ranges = range_array; ranges < range_array + range_l; ++ranges)
	{
			/*If the case value is less than the lower bound,
			  skip to the "around" point*/
			/*But, skip this comparison if the previous
			  upper_bound was lower_bound - 1*/
	
		if (previous_upper_bound != (ranges->lower_bound - 1))
			fprintf(outfile,"\tcmpl\t$%ld,%%eax\n\tjl\t.L%d\n",
				ranges->lower_bound, around);
			/* If the case value is less than or 
			  equal than the upper bound,
			  skip to the case label*/

		/* Must have the second check if the first one was omitted,
		** or if the range has more than one value.
		*/
		if (   ranges->lower_bound != ranges->upper_bound
		    || previous_upper_bound == ranges->lower_bound - 1
		    )
		    fprintf(outfile, "\tcmpl\t$%ld,%%eax\n",
				ranges->upper_bound);
		fprintf(outfile, "\tjle\t.L%d\n", ranges->goto_label);

			/*Save the previous upper bound*/

		previous_upper_bound = ranges->upper_bound;

	}
			/*If we fall thru: if there is a default label,
			  jump to it; otherwise define the "around" case*/

	if (default_label != -1)
	{
		jmplab(default_label);
	}
	else
		deflab(around);
}

#endif

p2done()
{
        char buf[BUFSIZ];
        int m;
	if (tmpfp)
	{
        	fclose(tmpfp);
        	if (!(tmpfp = fopen(tmpfn, "r")))
                	cerror("string file disappeared???");
#ifndef RODATA
		(void) locctr(DATA);
#endif
        	while (m = fread(buf, 1, BUFSIZ, tmpfp))
                	fwrite(buf, 1, m, outfile);
        	(void) unlink(tmpfn);
	}
	return( ferror(outfile) );
}
