/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nifg:cg/common/nail.c	1.59.2.1"

/*nail.c: nail routines to adapt the pcc2 back end to nail cg*/

#include "mfile1.h"
#include "mfile2.h"

NODE *setswreg();
# define MAX(a,b) ( ( a < b ) ? b : a )
# define ISFLOAT(t) ((t) == TDOUBLE || (t) == TFLOAT)
# define EXCEP(p) ( (p)->in.strat & EXHONOR)
# define UNSIGNED_TYPES (TUCHAR | TUSHORT | TUNSIGNED | TULONG )
extern int swregno;
static inswitch=0;		/*1 if in switch/case*/
int maxarg=0;
struct cse cse_list[MAXCSE];	/*list of active cse's*/
struct cse *cse_ptr = cse_list;	/*points one past the top of the list*/
extern unsigned int strlen();
static int isnail(), off_error(), copyable(), asg_cse(), reg_num(); 
static int ordasg(), can_paint(), semichain();
static void extract_rhs(), extract_lhs(), paint(), swcase(), swend(); 
static void ftnent(), ftnend(), sinit(), doswitch(), fill_rgrays();
static void semi_to_first(), semiasg(), rewcse();
static int can_except();
#ifndef NODBG
static void oparen(), cparen(); 
#endif

int
p2nail(p)
NODE *p;
	{
	if ( inswitch && p->tn.op != SWCASE && p->tn.op != SWEND)
	{
		cerror("Bad node between SWBEG and SWEND");
	}
			/*handle special nodes for NAIL*/
	if ( costing)
		return isnail(p);
	switch(p->tn.op)
	{
			/*Performance hack:  detect the GENLABs that
			have nothing but empty COPYs under them, and do them
			without matching*/
	case LABELOP:
		if (p->in.strat & DOEXACT)
		{
			protect(p);
			deflab(p->bn.label);
			unprot(p);
		}
		else
			deflab(p->bn.label);
		break;
	case GENLAB:
		if ( ( p->in.left->in.op == NOP) 
		|| ( p->in.left->in.op == COPY && 
			( p->in.left->in.name == 0 || *(p->in.left->in.name) == 0 )
		) )
		{
			if (p->in.strat & DOEXACT)
			{
				protect(p);
				deflab(p->bn.label);
				unprot(p);
			}
			else
				deflab(p->bn.label);
			break;
		}
		else
			return 0;	/*Handle normally*/

	case BEGF:
		str_spot = -1;
		begf(p);
		break;

	case ENTRY:
		ftnent(p);
		break;

	case ENDF:
		ftnend(p);
		break;

	case DEFNAM:
		defnam(p);
		break;

	case NAMEINFO:
		definfo(p);
		break;

	case SINIT:
		sinit(p->tn.name,p->tn.lval);
		break;

	case LOCCTR:
		(void)locctr((int)p->tn.lval);
		break;

	case SWBEG:
		doswitch(p);
		return 0; 	/* Generate the code for switch expression*/

	case SWCASE:
		swcase(p);
		break;

	case SWEND:
		swend(p);
		break;
	case ALIGN:
		defalign(gtalign(p->in.type));
		break;
#ifdef MYINIT
			/*special inits- for performance*/
	case INIT:
		MYINIT (p);
		break;
#endif
	default:
		return 0;
	} /*end switch*/
	if (odebug)
		e2print(p);
	tfree(p);
	return 1;
} /*end p2nail*/
static isnail(p)
NODE *p;
{
			/*Return 1 if the node is a standalone node*/
	switch(p->in.op)
	{
	case BEGF:
	case ENTRY:
	case ENDF:
	case DEFNAM:
	case SINIT:
	case LOCCTR:
	case SWBEG:
	case SWCASE:
	case SWEND:
	case ALIGN:
#ifdef MYINIT
	case INIT:
#endif
		return 1;
	default:
		return 0;
	}
}
static void
ftnent(p)
NODE *p;
{
			/*set up function entry*/
	(void)locctr(PROG);
	bfcode(p);
	bfdata(p);		/*local routine to save regs,autos if needed*/
} /* end ftnent*/
static void
ftnend(p)
NODE  *p;
{
	efcode(p);
}
static void
sinit(string,length)
char *string;
CONSZ length;
{
	register int pos;
				/*Special node for character initializations.
				   length = number of characters; if length=0,
				   use the strlen and add a null*/
	defalign(ALCHAR);
	if (!length)
		length=strlen(string)+1;
	/* Treat characters in string as unsigned to avoid inadvertant
	** sign extension of 8-bit characters on signed-char hosts.
	*/
	for (pos=0; pos<length; pos++)
		bycode((int)(unsigned char) string[pos],pos);
	bycode(-1,pos);
}

/*	Switch/case management:  Need to keep track of the (possibly large)
	ranges;  do the large ranges by explicit GENBRs; fill the swtab array
	with the smaller ranges, and let genswitch() [local code] handle the
	small ranges.*/

static struct case_range init_case_ranges [INI_RNGSZ];
TD_INIT(td_case_range, INI_RNGSZ, sizeof(struct case_range),
	0,  init_case_ranges, "case ranges table" );

#if 0	/* do not generate a ladder */
static int big_ranges;		/*1 iff we must do a ladder*/
#endif
			/*Threshold for a small switch range:*/
static int top_ranges;
static int n_cases;
/*static TWORD switch_type;*/
static void
doswitch(p)
NODE *p;
{
			/*switch code; walk the tree setting up an array of
			  constant values and cases. The switch value is
			  in the left subtree of the SWBEG
			  node*/

			/*rewrite the SWBEG into an assignment to SNODE*/
	p->in.op = ASSIGN;
	p->in.right = p->in.left;
	p->in.left = talloc();
	p->in.left->in.op = SNODE;
			/*Types must be integer */
	if ( (szty(p->in.right->in.type)) != (szty(TINT)) ) 
	{
		cerror("Switch constant of illegal type");
	}
	/*switch_type =*/p->in.left->in.type = p->in.type = p->in.right->in.type;
			/*choose the register for the switch value*/
	p = setswreg(p);
	swidx = 1;
	inswitch = 1;
	top_ranges = 0;	/*Empty the case_ranges array*/
	n_cases = 0;
#if 0	
	big_ranges = 0;
#endif
			/*Now, we return and generate the code*/
}


static void  
swcase(p)
NODE *p;
{
register int i;
register struct case_range *range;
	int low,high;
	if (!inswitch)
	{
		cerror("SWCASE not in switch");
	}
	low = p->tn.lval;
	high = p->tn.rval;
	if (low > high)
	{
		cerror("Case range reversed");
	}
			/*Handle overlaps*/

	for (range = case_ranges; range < case_ranges+top_ranges; ++range)
	{
			/*If some of the values in the range have
			  already been accounted for, remove them*/

		/*Case 1: New case is subset*/
		if (low >= range->lower_bound
		&& high <= range->upper_bound)
			return;

		/*Case 2: New case is superset; split it into two*/
		if (low <= range->lower_bound
		&& high >= range->upper_bound)
		{
			p->tn.rval = range->lower_bound - 1;
			swcase(p);	/*Do Lower part*/
			p->tn.rval = high;
			p->tn.lval = range->upper_bound + 1;
			swcase(p);	/*Do upper part*/
			return;
		}

		/*Case 2: overlaps low*/
		if (low >= range->lower_bound
		&& low <= range->upper_bound )
			low = p->tn.lval = range->upper_bound + 1;

		/*Case 3: overlaps high*/
		if (high >= range->lower_bound
		&& high <= range->upper_bound )
			high = p->tn.rval = range->lower_bound - 1;
	
	}
		/*OK, now add the range to the ranges array*/
	if (top_ranges >= RNGSZ)
		td_enlarge(&td_case_range, 2*top_ranges);
#ifndef NODBG
	if (odebug)
		fprintf(outfile,"Add case from %d to %d\n", low, high);
#endif
	case_ranges[top_ranges].lower_bound = low;
	case_ranges[top_ranges].upper_bound = high;
	case_ranges[top_ranges].goto_label = p->bn.label;

			/*If there are more cases than an arbitrary
			  threshold, set a flag*/
			/*If "high" is high enough, and "low" is low
			  enough, n_cases can overflow, making it negative.*/

	n_cases += (high - low + 1 );
			
#if 0
	if ( n_cases  >= MAXSWIT || n_cases <= 0)
	{
#ifndef NODBG
		if (odebug && !big_ranges)
			fprintf(outfile,"Using genladder\n");
#endif
		big_ranges = 1;
	}

			/*If we are using the smart local code,
			  prepare the swtab array*/

	if ( ! big_ranges )
#endif	/* 0 */
	{
		if (swidx + high - low + 1 >= CSWITSZ)
		{
			td_enlarge(&td_swtab, 2*(swidx+high-low));	
		}
		for(i = low; i <= high; i++)
		{
			/*Subtle bug: if low == MAXINT == high, the "i++"
			  will overflow to zero and this loop will run forever.*/
			if (i < low)
				break;
			swtab[swidx].sval = i;
			swtab[swidx].slab = p->bn.label;
			swidx++;
		}
	}
	top_ranges++;
}

#if 0

			/*Compare two ranges; return relationship of
			  lower bound.  Used by qsort()*/
static int
compare_ranges(l,r)
struct case_range *l, *r;
{
    /* Beware integer overflows! */
    return (
	    l->lower_bound > r->lower_bound
	?  1
	:  (l->lower_bound == r->lower_bound) - 1
    );
}

#endif
			/*Compare two swtab entries: return relationship
			  of sval.  Used by qsort()*/
static int
compare_swtab(l,r)
struct sw *l, *r;
{
    /* Beware integer overflows! */
    return (
	    l->sval > r->sval
	?  1
	:  (l->sval == r->sval) - 1
    );
}


static void  
swend(p)
NODE *p;
{ /* end a switch block */


	if (!inswitch)
	{
		cerror("SWEND not in switch");
	}
	inswitch = 0;
	
			/*If too much to do in genswitch(), use a ladder*/
#if 0
	if (big_ranges)
	{
			/*Sort by lower range*/
		qsort((char*)case_ranges, (unsigned int)top_ranges ,
			sizeof(struct case_range), compare_ranges);
		genladder( case_ranges, top_ranges , (int)p->tn.lval);
		return;
	}
#endif
			/*If we don't have big ranges, use the smart
			  local code*/
		/*Get the default label*/
		/* -1 means fall thru*/

	swtab[0].slab = p->tn.lval;

	/* sort */
	/* Except, the 0th element contains the default (argh)*/
	qsort((char*)&swtab[1], (unsigned int)swidx - 1,
		sizeof(struct sw), compare_swtab);

	/*Call local code to output the switch*/
	genswitch( swtab, swidx-1 );
}
void
p2init()
{
	allo0();
	mkdope();
	tinit();
	fill_rgrays();
#ifdef LOCAL_INIT
	LOCAL_INIT();
#endif
}

extern int asmdebug;
extern int zflag;

void
p2flags(cp) /* Pass 2 (CG) debugging flags. */
char *cp;
{
        while (*cp) {
                switch( *cp ) {
#ifdef  IN_LINE
		case 'a':	++asmdebug; break;
#endif
		case 'e':	++e2debug; break;
		case 'o':	++odebug; break;
		case 'r':	++rdebug; break;
		case 's':	++sdebug; break;
		case 'z':	++zflag; break;
		default:	myflags(cp); break;
		}
		++cp;
	}
}

int
gtalign(t)
TWORD t;
{
			/*Get the alignment for this type*/
	switch(t)
	{
	case TVOID:
	case TCHAR:
	case TUCHAR:
		return ALCHAR;
	case TSHORT:
	case TUSHORT:
		return ALSHORT;
	case TINT:
	case TUNSIGNED:
		return ALINT;
	case TLONG:
	case TULONG:
		return ALLONG;
	case TSTRUCT:
		return ALSTRUCT;
	case TFLOAT:
		return ALFLOAT;
	case TDOUBLE:
		return ALDOUBLE;
	case TPOINT:
	case TPOINT2:		/*Don't see seperate alignments for
				  the two pointer types.*/
		return ALPOINT;
	case TFPTR:
		return ALFPTR;
	default:
		cerror("Bad type for alignment");
	}
	/*NOTREACHED*/
}
	
/* stack space management routines*/

/* Both cg and Nifty share the same routine for allocating temps
	One big difference between cg and pcc2 is that autos and temps
	run from the same base.
*/

static BITOFF temp_offset=AUTOINIT,	/*Offset, in bits, of the next temp*/
	      mx_t_offset=AUTOINIT;	/*Highest temp_offset since last set*/
static BITOFF arg_offset=ARGINIT,	/*Offset, in bits, of the next arg*/
	      mx_a_offset=ARGINIT;	/*Highest arg_offset since last set*/

/*convert larger offset to smaller offset of integral types */
OFFSET					
off_conv(space, o, from, to)
int space;
OFFSET o;
TWORD from, to;
{
	BITOFF fromsz, tosz;

	fromsz = gtsize( from );
	tosz = gtsize( to );
	
	/* this function only converts larger offset to smaller offset
	   and is only for integral types.
	*/
	if ( (fromsz < tosz) || !(from & OKTYPE) || !(to & OKTYPE) )
	      return( off_error( space ));

	switch (space)
	{
	case VAUTO:
	case TEMP:
	case VPARAM:
	case NAME:
	case ICON:
#ifdef  RTOLBYTES
		return ( o );
#else
		return ( o + BITOOR(fromsz - tosz) );
#endif 
	default:
		cerror("unknown memory space for off_conv()");
	}
	/*NOTREACHED*/
}

/* which one is bigger offset on the stack? 
   this function returns the bigger offset */
OFFSET
off_bigger(space, o1, o2)
int space;
OFFSET o1, o2;
{
	switch (space)
	{
	case VAUTO:
	case TEMP:
#ifdef  BACKTEMP
	   	return ((o1 > o2) ? o2 : o1);	/* the smaller the larger on
						   the downwards stack. */
#else
		return ((o1 > o2) ? o1 : o2);
#endif

	case VPARAM: 
#ifdef  BACKPARAM
		return ((o1 > o2) ? o2 : o1);
#else
		return ((o1 > o2) ? o1 : o2);
#endif
	case NAME:
	case ICON:
		return ((o1 > o2) ? o1 : o2);

	default:
		cerror("unknown stack space for off_bigger()");
	}
	/*NOTREACHED*/
}

/* provide the offset of a member in an aggregate */
OFFSET
off_incr(space, o, n)
int space;
OFFSET o;
long n;			/* n is bit offset */
{

	if (n % SZCHAR)
	    return( off_error( space ));

	switch (space)
	{
	case VAUTO:
	case TEMP:
	case VPARAM:
	case NAME:
	case ICON:
		return (o + BITOOR(n) );

	default:
		cerror ("unknown stack space for off_incr()");
	}
	/*NOTREACHED*/
}

/*  offset is error */
off_is_err(space, o)
int space;
OFFSET o;
{
	return (o == off_error(space));
}

static int
off_error(space)
int space;
{
	switch (space)
	{
	case VAUTO:
	case TEMP:
#ifdef  BACKTEMP
		return ( 1 );
#else
		return ( -1 );
#endif

	case VPARAM:
#ifdef  BACKPARAM
		return ( 1 );
#else
		return ( -1 );
#endif

	case NAME:
	case ICON:
		return ( -1 );

	default:
		cerror("unknown memory space for off_error()");
	}
	/*NOTREACHED*/
}

void
off_init( space )
int space;
{
	switch (space)
	{
	case VAUTO:
	case TEMP:
        	temp_offset=AUTOINIT,     /*Offset, in bits, of the next temp*/
        	mx_t_offset=AUTOINIT;     /*Highest temp_offset since last set*/
		break;
	case VPARAM:
        	arg_offset=ARGINIT,       /*Offset, in bits, of the next arg*/
        	mx_a_offset=ARGINIT;      /*Highest arg_offset since last set*/
		break;
	case NAME:
	case ICON:
		break;
	default:
		cerror("unknown stack space for off_init()");
	}
}

OFFSET
next_temp(type, size, alignment)
TWORD type;		/*Type of the temp*/
BITOFF size;		/*size of the temp, in bits*/
int alignment;		/*alignment of the temp. May be > gtalign(type)
			  if type is a TSTRUCT with a double as the first
			  element, e.g. */
{
	int t;
		
	if (type != TSTRUCT && size < gtsize(type))
		size = gtsize(type);

	if (alignment < gtalign(type))
		alignment = gtalign(type);

# ifndef BACKTEMP

	SETOFF(temp_offset, alignment);
	t = temp_offset;
	temp_offset += size;
	if( temp_offset > mx_t_offset ) mx_t_offset = temp_offset;
	return( BITOOR(t) );

# else
			/*Temps run backward*/
	temp_offset += size;
	SETOFF( temp_offset, alignment );
	if( temp_offset > mx_t_offset ) mx_t_offset = temp_offset;
	return( -(BITOOR(temp_offset)) );
# endif
}
void
set_next_temp(size)
OFFSET size;		/* size is byte size */
{
			/*Set the offset for temps.  This also resets
			  the maximum value*/
#ifdef BACKTEMP
	temp_offset = mx_t_offset = (-size) * SZCHAR;
#else
	temp_offset = mx_t_offset = size * SZCHAR;
#endif
}

OFFSET
max_temp()
{
			/*Return the highest value that temp_offset has
			  reached since the last set_next_temp.*/
#ifdef BACKTEMP
	return ( -(BITOOR(mx_t_offset)) );
#else
	return ( BITOOR(mx_t_offset) );
#endif
}

OFFSET
next_arg(type, actual_size , alignment)
TWORD type;		/*Type of the arg*/
BITOFF actual_size;	/*size of the arg, in bits*/
int alignment;		/*alignment of the arg, in bits*/
{
	int adj_size ;
	int t;

			/*Ignore the size passed in, except for STRUCTs*/
	if (type != TSTRUCT)
		actual_size = gtsize(type);

	adj_size = actual_size;

# ifdef INTARGS
			/*all args must be at least int size*/
	if (actual_size < SZINT)
		adj_size = SZINT;
	if (alignment < ALINT)
		alignment = ALINT;
# endif
# ifndef BACKPARAM

	SETOFF(arg_offset, alignment);
	t = arg_offset;
	arg_offset += adj_size;
	if( arg_offset > mx_a_offset ) mx_a_offset = arg_offset;
#   if defined(INTARGS) && !defined(RTOLBYTES)
			/*Adjust the offset: the actual value
			  is at the other end of the int*/
	if (actual_size < SZINT)
		return (BITOOR(t + SZINT - actual_size));
#   endif
	return(BITOOR(t));

# else
			/*args run backward*/
	arg_offset += adj_size;
	SETOFF( arg_offset, alignment );
	if( arg_offset > mx_a_offset ) mx_a_offset = arg_offset;
#   if defined(INTARGS) && !defined(RTOLBYTES)
			/*Adjust the offset*/
	if (actual_size < SZINT)
		return( -(BITOOR(arg_offset - SZINT + actual_size)) );
#   endif
	return( -(BITOOR(arg_offset)) );
# endif
}
void
set_next_arg(size)
OFFSET size;		/* size is byte size */
{
			/*Set the offset for args.  This also resets
			  the maximum value*/
#ifdef BACKPARAM
	arg_offset = mx_a_offset = (-size) * SZCHAR;
#else
	arg_offset = mx_a_offset = size * SZCHAR;
#endif
}

OFFSET
max_arg()
{
			/*Return the highest value that temp_offset has
			  reached since the last set_next_temp.*/
#ifdef BACKPARAM
	return ( -(BITOOR(mx_a_offset)) );
#else
	return ( BITOOR(mx_a_offset) );
#endif
}
/*	end of temp allocation functions*/

/* binding parameters - this function is a common version for parameters
**			binding.  This version has the following restrictions
**			1) The type conversion between declared_type and effective
**			   _type should both be integral OR floating point types.
**			   It does not handle languages allowed conversion between
**			   integral and floating point types.
**			2) It only handles that the alignment of double is multiple
**			   of float type.                                             
**			3) This version also only handles for the machines that
**			   arguments passing on the stack.  If there are any  
**			   specials in a machine architecture (e.g. SPARC - arguments
**			   passing in registers), the machine instance part of the CG
**			   should provide its local version of bind_param() and
**			   define MY_BIND_PARAM.
 */
#ifndef MY_BIND_PARAM
/* ARGSUSED */
void
bind_param(declared_type, effective_type, flag, stack_offset, pregno, reg_list)
TWORD declared_type, effective_type;
int flag;				/* not used here */
OFFSET stack_offset;
int *pregno;
char *reg_list;				/* not used here */
{
	NODE *p, *right, *left;
	int reg = *pregno;

	/* an assignment is necessary */
	if ( reg >= 0  || 
		((declared_type|effective_type) == (TFLOAT|TDOUBLE)) ) {

	    /* figure out the right side of the tree */
	    right = talloc();
	    right->tn.op = VPARAM;
	    right->tn.type = effective_type;
	    right->tn.lval = stack_offset;
	    if (declared_type != effective_type) {	
		if (declared_type == TFLOAT) {
		    /* floating point parameters conversion */
		    p = right;
		    right = talloc();
		    right->in.type = TFLOAT;
		    right->in.op = CONV;
		    right->in.left = p;
		    right->in.right = (NODE *)0;
		}
		else {
		    /* integral parameters conversion */
		    right->tn.lval = off_conv(VPARAM, stack_offset, effective_type, declared_type);
		    right->tn.type = declared_type;
		}
	    }

	    /* figure out the left side of the tree */
	    left = talloc();
	    left->tn.type = declared_type;
	    if ( reg >= 0 ) {
		left->tn.op = REG;
		left->tn.rval = reg;
	    }
	    else {
		left->tn.op = VPARAM;
		left->tn.lval = stack_offset;
	    }

	    /* build the assignment tree */
	    p = talloc();
	    p->in.type = declared_type;
	    p->in.op = ASSIGN;
	    p->in.right = right;
	    p->in.left = left;

	    /* generate code */
	    p2compile(p);
	}
}
#endif  /*MY_BIND_PARAM */

# ifndef NODBG
# define PL p2print(p->in.left);
# define PR p2print(p->in.right);
static int firstarg=0;
p2print(p)
NODE *p;
{
	if (!p) 
	{
		cerror("** Null pointer in p2print**");
		exit(-1);
	}


	switch (p->tn.op)
	{
	case NAME:
		acon(p);
		prstrat(p->tn.strat);
		break;
	case REG:
		fprintf(outfile,"Reg%d",p->tn.rval);
		prstrat(p->tn.strat);
		break;
	case TEMP:
		fprintf(outfile,"Temp%d",p->tn.lval);
		prstrat(p->tn.strat);
		break;
	case VAUTO:
		fprintf(outfile,"Auto%d",p->tn.lval);
		prstrat(p->tn.strat);
		break;
	case VPARAM:
		fprintf(outfile,"Param%d",p->tn.lval);
		prstrat(p->tn.strat);
		break;
	case CSE:
		fprintf(outfile,"Cse%d",p->csn.id);
		prstrat(p->tn.strat);
		break;
	case ENTRY:
	case BEGF:
	case ENDF:
		fprintf(outfile,"%s %s:",opst[p->tn.op],(p->tn.name? p->tn.name : " "));
		break;
	case ICON:
		acon(p);
		prstrat(p->tn.strat);
		break;
	case FCON:
		fprintf(outfile,"%lf", p->fpn.dval);
		prstrat(p->tn.strat);
		break;
	case DEFNAM:
		fprintf(outfile,"%s:",p->tn.name);
		break;
	case UNINIT:
		fprintf(outfile,"<Storage: %d>",p->tn.lval);
		break;
	case INIT:
		fprintf(outfile,"<");
		PL;
		fprintf(outfile,">");
		break;
	case CCODES:
		fprintf(outfile,"Ccodes");
		prstrat(p->tn.strat);
		break;
	case UNARY MINUS:
		item(p,"-");
		break;
	case UNARY MUL:
		item(p,"*");
		break;
	case UNARY AND:
		item(p,"&");
		break;
	case UNARY CALL:
		PL;
		fprintf(outfile,"()");
		prstrat(p->in.strat);
		break;
	case CALL:
		firstarg=1;
		PL;
		fprintf(outfile," ( ");
		PR;
		fprintf(outfile," ) ");
		prstrat(p->in.strat);
		break;
	case FUNARG:
		if (!firstarg)
			fprintf(outfile," , ");
		else
			firstarg=0;
		PL;
		break;

	case GENBR:
	case GENUBR:
	case GENLAB:
		PL;
		fprintf(outfile,";%s %d ",opst[p->tn.op], p->bn.label);
		if (p->tn.op == GENBR)
			fprintf(outfile," %s",opst[p->bn.lop]);
		break;

	case LABELOP:
		fprintf(outfile,";%s %d ",opst[p->tn.op], p->bn.label);
		break;

	case SWCASE:
		fprintf(outfile,"SWCASE(%d-%d):%d",p->tn.lval,p->tn.rval,p->bn.label);
		break;
	case LET:
		oparen();
		fprintf(outfile,"LET%d=",p->csn.id);
		PL;
		fprintf(outfile,",");
		PR;
		cparen();
		break;

	case 0:
		break;
	default:
		item(p,opst[p->tn.op]);
		break;
	}
	fprintf(outfile," ");
}
item(p,name)
NODE *p;
char *name;
{
		switch(optype(p->tn.op))
		{
		case LTYPE:
			fprintf(outfile,"%s ",name);
			break;
		case BITYPE:
			oparen();
			PL;
			fprintf(outfile," %s",name);
			prstrat(p->in.strat);
			fprintf(outfile," ");
			PR;
			cparen();
			break;
		case UTYPE:
			fprintf(outfile," %s",name);
			prstrat(p->in.strat);
			fprintf(outfile," ");
			oparen();
			PL;
			cparen();
			break;
		default:
			fprintf(outfile,"<bad op %d>",p->tn.op);
			break;
		}
}
prstrat(s)
int s;
{
	static int flags[]={ LTOR, RTOL, PAREN, CHGSTK, EXHONOR, EXIGNORE, 
			     DOEXACT, COPYOK, ALTRVAL, OCOPY, VOLATILE, 
#ifdef	ELF_OBJ
			     PIC_GOT, PIC_PLT, PIC_PC,
#endif
			     0};

	static char codes[]={'l',  'r',   'p',  'c',   'h',
			     'i',  'x', 'o', 'a', 'O', 'v',
#ifdef	ELF_OBJ
			     'G', 'P', 'C'
#endif
			    };

	int i;
	char done_any=0;
	for ( i=0; flags[i]; i++)
	{
		if ( s & flags[i])
		{
			if (!done_any)
			{
				PUTCHAR('{');
				done_any=1;
			}
			PUTCHAR(codes[i]);
		}
	}
	if (done_any)
		PUTCHAR('}');
}
static int parenlevel=0;
static void
oparen()
{
	if (parenlevel++)
		fprintf(outfile," ( ");
}
static void 
cparen()
{
	if (--parenlevel)
		fprintf(outfile," ) ");
}
#endif
int
gtsize(t)
TWORD t;
{
			/*Get the size for this type*/
	switch(t)
	{
	case TVOID:
		return 0;
	case TCHAR:
	case TUCHAR:
		return SZCHAR;
	case TSHORT:
	case TUSHORT:
		return SZSHORT;
	case TINT:
	case TUNSIGNED:
		return SZINT;
	case TLONG:
	case TULONG:
		return SZLONG;
	case TSTRUCT:
		cerror("gtsize(TSTRUCT)");
		/*FALLTHRU*/
	case TFLOAT:
		return SZFLOAT;
	case TDOUBLE:
		return SZDOUBLE;
	case TPOINT:
	case TPOINT2:		/*Don't see seperate alignments for
				  the two pointer types.*/
		return SZPOINT;
	case TFPTR:
		return SZFPTR;
	default:
		cerror("Bad type for size");
	}
	/*NOTREACHED*/
}

static void
local_ofile(f)
FILE *f;
{
			/*This file becomes the new output file.*/
	if ( f == NULL)
		cerror("NULL output file pointer");
	outfile = textfile = f;
	
}

void
ofile(f)
FILE *f;
{
			/*Change the output file: this is the "normal"
			  way to change the output stream.  Call local_ofile
			  to change the current output, and then
			  output the loc ctr*/
	local_ofile(f);
			/*Don't output the new loc ctr; just set the current
			  one to "UNK".  The next locctr call will output the
			  locctr.  This is because a call to beg_file may follow
			  here.*/
	(void)locctr( UNK );
}
void
dfile(f)
FILE *f;
{
			/*This file becomes the new debug output file.*/
	if ( f == NULL)
		cerror("NULL debug output file pointer");
	debugfile = f;
}
extern int proflag;
void
profile(flag)
int flag;
{
	proflag = flag;	/*for nifty to turn profiling on/off*/
}
void
fcons(p)
NODE *p;
{
	int lastloc, tptr, i;
	char temp[32];
	lastloc = locctr( FORCE_LC(CDATA) );  /* force into read-only data section */
	tptr = p->tn.type;
	defalign( tptr==TDOUBLE?ALDOUBLE:ALFLOAT );
	deflab( i = getlab() );
	fincode( p->fpn.dval, tptr==TDOUBLE?SZDOUBLE:SZFLOAT );
	p->tn.lval = 0;
#ifdef	ELF_OBJ
	p->tn.rval = NI_FLSTAT;
#endif
	sprintf(temp, LABFMT, i);
	p->tn.name = tstr(temp);
	p->in.op = NAME;
	if (lastloc != UNK)
		(void)locctr(lastloc);
}
void  
typecheck(p, initflag)
NODE *p;
int initflag;		/* on if we are under an INIT node*/
{
			/*This routine does various rewrites
			  for Nail-type nodes.*/
	int o = p->in.op;
	unsigned int type = p->in.type;
	NODE *l, *r, *last;
			/*add label for RETURNS's*/
	if ( o == RETURN || o == VLRETURN || o == NPRETURN )
	{
		p->bn.label = retlab;

#ifdef MYRET_TYPE  /* to set the return register number for optimizer */
		MYRET_TYPE( type );
#endif
	}
			/*If this machine has no shorts, change shorts to ints*/
#ifdef NOSHORT
	if ( type == TSHORT)
		type = p->in.type = TINT;
	else if ( type == TUSHORT)
		type = p->in.type = TUNSIGNED;
#endif
			/*If this machine has no longs, change longs to ints*/
#ifdef NOLONG
	if ( type == TLONG)
		type = p->in.type = TINT;
	else if ( type == TULONG)
		type = p->in.type = TUNSIGNED;
#endif
			/*rewrite FCONS to ICONS*/
#ifndef SETDCON
	if( o == FCON && !initflag)  fcons(p);
#endif

	if ( o == INIT)
		initflag = 1;
			/*rewrite non-parened, non-RTOL semicolons
			  to commas*/
			/*rewrite ;p to ;lp. LTOR always better*/
	if ( o == SEMI)
	{
		switch(p->in.strat & (PAREN|RTOL|LTOR))
		{
		case 0:	/* "vanilla ;" */
		case LTOR:	/* ";l" */
			o = p->in.op = COMOP;
			break;
		case PAREN:	/* ";p" */
			p->in.strat |= LTOR;
			break;
		}
	}
			/*Rewrite *(&A) or &(*A) into A*/
			/*But, for *(&A), only do the rewrite if the *
			  and A have the same type. */
	if ( (o == STAR && p->in.left->in.op == UNARY AND
	     && p->in.type == p->in.left->in.left->in.type )
	  || (o == UNARY AND && p->in.left->in.op == STAR) )
	{
		NODE *unary_and, *subtree;
#ifndef NODBG
		if (odebug)
		{
			fprintf(outfile,"Remove *& pair from node %d\n",node_no(p));
		}
#endif
		unary_and = p->in.left;
		subtree = unary_and->in.left;
		*p = *subtree;
		unary_and->in.op = subtree->in.op = FREE;
		typecheck(p, initflag);
		return;
	}
			

			/*For  commutative ops: put constants on right.
			  should also tower on left*/
	switch(o)
	{
	case MUL:
	case PLUS:
	case AND:
	case OR:
	case ER:
		l = p->in.left;
		r = p->in.right;
		if ( (l->tn.op == ICON  && r->tn.op != ICON)
		  || (o == r->tn.op ) )
			commute(p);
	}
	if ( o == PLUS)
	{
				/* Rewrite "(-b) + a"   to "a-b"
				   by commuting the node and letting the next
				   test do the rewrite*/

		if (p->in.left->in.op == UNARY MINUS)
			commute(p);

				/*Rewrite "a + (-b)" to "a - b " if the
				  U- does not need exceptions honored*/

		r = p->in.right;
		if ( r->in.op == UNARY MINUS  &&
			!(r->in.strat & EXHONOR))
		{
			NODE *dead;
			o = p->in.op = MINUS;
			dead = r;
			r = p->in.right = dead->in.left;
			dead->in.op = FREE;
#ifndef NODBG
			if (odebug)
			{
				fprintf(outfile,"a + (-b) rewritten to a-b:\n");
				e2print(p);
			}
#endif
		}
			/*Ditto, if the rhs is a constant; (if the lhs
			  was the only constant it was commuted above)*/
		if ( r->in.op == ICON && !r->tn.name && r->tn.lval < 0)
		{
			o = p->in.op = MINUS;
			r->tn.lval = - r->tn.lval;
		}
	}	/*end o == PLUS code*/
		

			/* Make sure LET nodes are parenthesized and
				left-to-right*/
	if ( o == LET)
	{
		p->in.strat |= (LTOR|PAREN);
	}
			/*Remove any spurious exception nodes*/
	if( p->in.strat & (EXHONOR|EXIGNORE) )
	{
		if ( !can_except(p))
		{
#ifndef NODBG
			if(odebug)
				fprintf(outfile,"Remove exceptions from node %d\n",
				node_no(p));
#endif
			p->in.strat &= ~(EXHONOR|EXIGNORE);
		}

	}	/*end if*/
	

			/*Now, check CONV nodes*/
	
	if ( o == CONV)
	{
		NODE *child = p->in.left;
		int ctype = child->in.type;
			/*No conversions between pointers and float/double*/
		if ( ( ISFLOAT(type) && ctype == TPOINT) ||
			(type == TPOINT && ISFLOAT(ctype)))
			uerror("conversion between float/pointer");
			/*No conversion from void*/
		if ( ctype == TVOID)
			uerror("conversion from void");
			/*Can we paint? OK if:
				both float or both not float;
				new alignment less restrictive than old;
				not a reg, asop, or bitfield;
				same size OR
				non-LTORBYTES , in mem, and smaller.
			*/
		if (  !(p->in.strat & (DOEXACT|EXHONOR))
		&& can_paint(child, type) )
		{
			paint(p);
			typecheck(p,0);
			return;
		}
	}
			/*If we are handed FSELECT(CURFRAME,stuff)
			  rewrite out the stuff.*/
	if ( o == FSELECT && p->in.left->in.op == CURFRAME)
	{
		NODE *r;
		int save_type;
			/*Free the CURFRAME*/
		p->in.left->in.op = FREE;
			/*copy the auto/param into p, but use the type that
			  was on the FSELECT*/
		save_type = p->in.type;
		r = p->in.right;
		*p = *r;
		p->in.type = save_type;
			/*Free the old auto/param*/
		r->in.op = FREE;
			/*This better be an AUTO or PARAM, so no children.*/
		return;
	}

		/*Structure returns are done with a STASG to and RNODE;
		  for TMPSRET, rewrite ( STASG(RNODE,expr)) to
		( STASG(AUTO0,expr) ;lp ( RNODE = AUTO0) )
		*/
#ifdef TMPSRET
	if ( o == STASG && p->in.left->tn.op == RNODE)
	{
		NODE *lhs, *rhs;
			/*First, do the left hand side of the semilp*/
			/*This is STASG( Auto0, Expr) */
		if (str_spot == -1)
			cerror("Unexpected structure return");
		lhs = talloc();
		*lhs = *p;
		p->in.op = SEMI;
		p->in.strat = LTOR|PAREN;
		p->in.left = lhs;
		lhs->in.left->tn.op = VAUTO;
		lhs->in.left->tn.lval = str_spot ;
		lhs->in.left->tn.type = TPOINT;
		lhs->in.left->tn.name = 0;
			/*Now, the rhs is an assignment of
				RNODE = AUTO0  */
		rhs = talloc();
		p->in.right = rhs;
		rhs->in.op = ASSIGN;
		rhs->in.type = TPOINT;
		rhs->in.left = talloc();
		rhs->in.left->in.op = RNODE;
		rhs->in.left->in.type = TPOINT;
		rhs->in.right = talloc();
		*(rhs->in.right) = *(lhs->in.left);	/*copy the AUTO0*/
# ifndef NODBG
		if ( odebug)
		{
			fprintf(outfile,"Structure return rewritten to:\n");
			e2print(p);
		}
#endif
	}
#endif
			/*If this is an ordered node, and the last side
			  is a semichain(list of semis terminated by a TEMP),
			  move the chain to the first side.  This is because
			  rewsto() is ineffective on the second side of an
			  ordered node.*/
			/*Exception: don't do this for LET nodes!  It moves
			  the use of the CSE into the definition of the CSE*/

	if ( (p->in.op != LET)
	&& (last = lastl(p))
	&& semichain(last) )
	{
		semi_to_first(p);
		typecheck(p,initflag);
		return;
	}
			/*Now, typecheck the children*/
	switch(optype(o))
	{
	case BITYPE:
		typecheck(p->in.right, initflag);
		/*FALLTHRU*/
	case UTYPE:
		typecheck(p->in.left, initflag);
		break;
	}
}
static void
paint(p)
NODE*p;
{
			/*p is a conv node. paint its type onto a child*/
	NODE *child = p->in.left;
	int newtype = p->in.type;
	if ( p->in.op != CONV)
		cerror("paint: bad node received");
			/*If MEMONLY, can only paint to TPOINT in memory*/


#ifndef NODBG
	if (odebug)
		fprintf(outfile,"paint node %d to %d,type 0%o\n",
		node_no(child), node_no(p), newtype);
#endif
	*p = *child;
	p->in.type = newtype;
	child->in.op = FREE;
}
static int
can_paint(p,to_type)
NODE *p;
TWORD to_type;
{
			/*returns a 1 iff we can paint node "p" to type "t".*/
	TWORD  from_type = p->in.type;
	if ( from_type == to_type) 
#ifdef FP_EXTENDED
		/* Must lose extra precision */
		return ISFLOAT(to_type) == 0;
#else
		return 1;
#endif
			/*Only allow paints in memory */

	switch(p->in.op)
	{
	case NAME:
	case ICON:
	case VAUTO:
	case TEMP:
	case VPARAM:
	case STAR:
		break;
	default:
		return 0;
	}

	if ( ISFLOAT(to_type) || ISFLOAT(from_type))
		return 0;
	if (gtalign(to_type) > gtalign(from_type) )
		return 0;

#ifndef RTOLBYTES
		if ( gtsize(to_type) != gtsize(from_type))
			return 0;
#else
		if ( gtsize(to_type) > gtsize(from_type))
			return 0;
#endif
			/* if exact semantics, no paint*/
	if ( p->in.strat & DOEXACT )
		return 0;
			/*dont paint to VOID*/
	if(to_type == TVOID)
	{
		return 0;
	}
#ifdef MEMONLY
	if ( to_type == TPOINT)
	{
		switch(op)
		{
		case STAR:
		case NAME:
		case VAUTO:
		case VPARAM:
			break;
		default:
			return 0;
		}
	}
#endif
			/*OK to paint this*/
	return 1;
}

pre_ex(p)
register NODE *p;
{
			/*Before expand: walk the tree; remove
			  any ;lp's with null left children. Also,
			  return a one if there are any nodes
			  with the DOEXACT bit set*/
	int any_exact = 0;
	register NODE *l = p->in.left;
	register NODE *r = p->in.right;

	if ( !p)
		return 0;
			/*Find exact bits*/
	switch( optype( p->in.op))
	{
	case BITYPE:
		if ( pre_ex(p->in.right))
			any_exact = 1;
		/*FALLTHRU*/
	case UTYPE:
		if ( pre_ex(p->in.left))
			any_exact = 1;
	}
	if ( p->in.strat & DOEXACT)
		any_exact = 1;

			/*Is there a hanging ;lp?*/
	switch(optype(p->in.op))
	{
	case BITYPE:
		if ( r && r->in.op == SEMI && r->in.left->in.op == FREE)
		{
#ifndef NODBG
			if (odebug)
				fprintf(outfile,"Remove ;lp %d\n", node_no(r));
#endif
				p->in.right = r->in.right;
				r->in.op = FREE;
		}
		/*FALLTHRU*/
	case UTYPE:
		if ( l && l->in.op == SEMI && l->in.left->in.op == FREE)
		{
#ifndef NODBG
			if (odebug)
				fprintf(outfile,"Remove ;lp %d\n", node_no(l));
#endif
				p->in.left = l->in.right;
				l->in.op = FREE;
		}
	}

	return any_exact;

}

NODE *firstl(p)
register NODE *p;
{
	if (optype(p->in.op) != BITYPE)
		return NULL;
			/* if this is an ordered node, return the first side;
				else return null*/
	switch ( p->in.strat & (LTOR|RTOL))
	{
	case LTOR:
		return p->in.left;
	case RTOL:
		return p->in.right;
	default:
		return NULL;
	}
	/*NOTREACHED*/
}

NODE *lastl(p)
register NODE *p;
{
	if (optype(p->in.op) != BITYPE)
		return NULL;
			/* if this is an ordered node, return the last side;
				else return null*/
	switch ( p->in.strat & (LTOR|RTOL))
	{
	case LTOR:
		return p->in.right;
	case RTOL:
		return p->in.left;
	default:
		return NULL;
	}
	/*NOTREACHED*/
}

semilp(p)
NODE *p;
{
			/*return true if p is a left-to-right semicolon.*/
	return ( p->in.op == SEMI && ! (p->in.strat & RTOL) );
}

tnumbers(p)
register NODE *p;
{
			/*This function does two things:
			1. Set the OCOPY flag on each node
			2. Do tree rewrites that must happen after rewcom.

			It returns 1 iff a tree rewrite is done; this causes
			us to start all over again (sigh)*/

	NODE *r, *l;
	NODE *first, *last;
	register int o;
	o = p->tn.op;
	r = p->in.right;
	l = p->in.left;
			/*Ignore ordered nodes if first is TEMP or ICON,
			  or last is ICON*/
			/*cannot unorder if last side is TEMP; the
			  first side might set the temp!*/
	if ( (first = firstl(p)) != NULL )
	{
		if ( first->in.op == TEMP || l->in.op == ICON || 
			r->in.op == ICON)
			unorder(p);
	}

			/*left side of asg ops: copy the adress not the value*/
	if ( p->in.strat & LTOR && asgop(o) )
		if (ordasg(p)) return 1;
			/*If we are assigning to a semicolon, do a rewrite*/
	if (asgop(o) && p->in.left->in.op == SEMI)
	{
		semiasg(p);
		return 1;
	}

			/*Make sure the OCOPY bit is cleared*/
	p->in.strat &= ~OCOPY;

	switch( optype(o))
	{
	case BITYPE:
		if (tnumbers(r)) return 1;
		/*FALLTHRU*/
	case UTYPE:
		if(tnumbers(l)) return 1;
	}

			/*Set the copy bit here, if this node's exception
			  behavior is inconsistent with being a shape*/
#ifdef EXSHP_H
			/*Shapes honor exceptions*/
	if (p->in.strat & EXIGNORE)
		p->in.strat |= OCOPY;
#endif
#ifdef EXSHP_I
			/*Shapes ignore exceptions*/
	if (p->in.strat & EXHONOR)
		p->in.strat |= OCOPY;
#endif
			/*Finally, set the copy bit in the child if the second side
			  has side effects*/

	if( (last = lastl(p)) != NULL && iseff( last) && o != LET )
		firstl(p)->in.strat |= OCOPY;
	
	return 0;
}
void 
unorder(p)
NODE *p;
{
	if (p->in.op == LET)
		return;	/*LETs are always ordered*/
#ifndef NODBG
	if (odebug > 1)
		fprintf(outfile,"Unorder node %d\n", node_no(p));
#endif
			/*Make p an unordered node*/
	p->in.strat &= ~( LTOR | RTOL);
}
static int
ordasg(p)
NODE *p;
{
	NODE *lhsto();
	NODE *q;
			 /*p is a LTOR asg op.
			  if neither side has side effects,
			  simply ignore the ordering.*/

	if ( !iseff(p->in.left) && !iseff(p->in.right))
	{
#ifndef NODBG
		if (odebug)
		{
			fprintf(outfile,"Ignoring ordered assign op %d\n", node_no(p));
		}
#endif
		unorder(p);
		return 0;
	}
#ifndef NODBG
	if (odebug)
	{
		fprintf(outfile,"Rewriting ordered assign op %d\n", node_no(p));
	}
#endif
			/*if the left hand side calculates an address,
			  do that into a temp*/
	if ( ( q = lhsto(p)) != 0)
	{
		if ( rewsto(q) )
		{
			rewcom(p,NRGS);
			return 1;
		}
			/*If lhs was already a temp, continue on*/
	}
			/*If the op is not =, rewrite it, preserving order*/
	if ( p->in.op != ASSIGN)
	{
		reweop(p);
		return 1;
	}
	unorder(p);
	return 0;
}

			/*The next two functions are used by to rewrite trees
			  to allow assignments to semicolons*/
static  void  
extract_lhs(p, semi_strat)
NODE *p;
int  semi_strat;
{
		/*Given a tree "(A;B) = C", rewrite it to "A; (B=C)".*/
	NODE *semi, *new_asg;
	if (!asgop(p->in.op))
		cerror("Bad extract_lhs");
			/*Need to rewrite in place*/
	semi = p->in.left;
	semi->in.strat = semi_strat;
	new_asg = talloc();
	*new_asg = *p;			/*new_asg is ((A;B)=C)*/
	new_asg->in.left = semi->in.right;	/*new_asg is (B=C)*/
	*p = *semi;			/*p is (A;B)*/
	p->in.right = new_asg;		/*p is (A;(B=C)) */
	semi->in.op = FREE;
	return ;
}

static void 
extract_rhs(p, semi_strat)
NODE *p;
int semi_strat;
{
		/*Given a tree "(A;B) = C", rewrite it to "B = (A;C)".*/
	NODE *semi;
	if (!asgop(p->in.op))
		cerror("Bad extract_rhs");
	semi = p->in.left;
			/*Toggle the semicolon strategy*/
	semi->in.strat = (semi_strat ^ (LTOR|RTOL));
	p->in.left = p->in.left->in.right;
	semi->in.right = p->in.right;
	p->in.right = semi;
	return ;
}
static void  
semiasg(p)
NODE *p;
{
			/*We are assigning to a semicolon. We are passed
			  a tree of the form (A;B)=C.   The rewrite to
			  use is very tricky, depending upon the ordering
			  of the various nodes.  Eventually we will call
			  either extract_rhs or extract_lhs*/
			/*This would be MUCH simpler if we could just do
				  *(A,&B) = C
			  but if B is a reg var or a bitfield we can't take
			  it's address*/

	int asg_strat, semi_strat, top_paren;

	if (!asgop(p->in.op) || (p->in.left->in.op != SEMI))
		cerror("Botched semiasg");
	asg_strat = p->in.strat ;
	semi_strat = p->in.left->in.strat ;
			/*Parenthesization: if the top is paren'ed coming in,
			  it must be paren'ed going out.
			  By luck, it doesn't matter if the semi is parened
			  or not; we always do A and B together.*/

	top_paren = (asg_strat & PAREN);

#ifndef NODBG
	if (odebug)
	{
		fprintf(outfile,"semiasg rewrites:\n");
		e2print(p);
	}
#endif

			/*The dominating factor is the order of the assignment*/
			/*Both the semi and the asg eventually end up with the
			  same ordering;  this also makes PARENs work out*/
			/* [note that extract_rhs changes the ordering of the
			  semi*/

	switch(asg_strat & (LTOR|RTOL) )
	{
	case 0:	/*Don't care*/
		extract_lhs(p, semi_strat);
		break;

	case LTOR: 
		if (semi_strat & RTOL)
			extract_rhs(p, semi_strat | top_paren);
		else
			extract_lhs(p, semi_strat | LTOR );
		break;

	case RTOL:
		if (semi_strat & LTOR)
			extract_rhs(p, semi_strat | top_paren);
		else
			extract_lhs(p, semi_strat | RTOL );
		break;
	}
			
#ifndef NODBG
	if (odebug)
	{
		fprintf(outfile,"rewritten to:\n");
		e2print(p);
	}
#endif
	return ;
}
static int
csecopy(p)
NODE *p;
{
	if ( p->in.op != LET)
		cerror("bad node passed to csecopy\n");

			/*Try to rewrite Let by copying the CSE.
			  Only legal if the COPYOK flag is set.*/
	if (! (p->in.strat & COPYOK))
		return 0;
			/*Now, is the lhs small enough to copy? default
			  is: leaf nodes only*/
#ifdef MYCOPY
	if (MYCOPY(p))
#else
	if (copyable(p))
#endif
	{
		NODE *l = p->in.left;
		NODE *r = p->in.right;
		int csenum = p->csn.id;
#ifndef NODBG
		if (odebug)
			fprintf(outfile,"Copy cse %d in tree %d \n", csenum,node_no(p));
#endif

			/*rewrite the cse's in the tree*/
                *p = *r;	/*zaps the LET node*/
                r->in.op = FREE;
                rewcse(p,l,csenum);
			/*Throw away the original left side*/
		tfree(l);
		return 1;
	}
	return 0; 	/*too big to rewrite*/
}

static int
copyable(p)
NODE *p;
{
	int ok = 0;
			/*return 1 iff this cse can be copied*/
	if (optype(p->in.left->in.op) == LTYPE)
		ok = 1;
			/*Also, U& of a leaf */
	if (p->in.left->in.op == UNARY AND
	&& optype(p->in.left->in.left->in.op) == LTYPE)
		ok = 1;
			/*Make sure that the cse is not assigned to.*/
	if(ok)
		ok =  ! asg_cse(p->in.right, p->csn.id );
	return ok;
}

static int
asg_cse(p, id)
NODE *p;
int id;
{
			/*walk tree, return 1 if there is an assignment to
			  cse "id".*/
	if (asgop(p->in.op) && p->in.left->in.op == CSE
		&& p->in.left->csn.id == id)
	{
		fprintf(stderr,"Warning: bad cse rewritten: %d\n",
			node_no(p));
		return 1;
	}
	switch ( optype(p->in.op))
	{
	case BITYPE:
		if ( asg_cse(p->in.right, id)) return 1;
		/*FALLTHRU*/
	case UTYPE:
		if ( asg_cse(p->in.left, id)) return 1;
	}
	return 0;
}
static void
csetemp(p)
NODE *p;
{
	int temp,csenum;
	NODE *l, *r, *asop, *tptr;
			/*Handle a LET node.*/
			/*For now, just use temps. when the presence or
			  absence of costing is decided, be smarter.*/
#ifndef NODBG
	if (odebug)
		fprintf(outfile,"Rewrite LET %d (cse %d) to temp\n", node_no(p),
		p->csn.id);
#endif
	temp = freetemp(argsize(p->in.left)/SZINT);
	csenum = p->csn.id;
	l = p->in.left;
	r = p->in.right;
			/*Make a node with a temp in it. This will be the lhs
			  of the assignment, and the copy passed to rewcse*/
	tptr = talloc();
	*tptr = *l;
	tptr->tn.lval = BITOOR(temp);
	tptr->in.op = TEMP;
			/*check for uninitialized let*/
	if ( l->in.op == NOP )
	{
			/*Skip initial assignment*/
		l->in.op =  FREE;
		*p = *r;
		r->in.op = FREE;
		rewcse(p,tptr,csenum);
			/*rewcse always copies tptr; so the original
			  must be put back*/
		tptr->in.op = FREE;
	}
	else
	{
			/*rewrite the lhs to temp = lhs*/
		p->in.op = SEMI;
			/*All let nodes are parenthesized*/
		p->in.strat = (LTOR|PAREN);
		asop = talloc();
		*asop = *l;
		p->in.left = asop;
		asop->in.op = ASSIGN;
		asop->in.left = tptr;
		asop->in.right = l;
		asop->in.strat = 0;
		rewcse(r,tptr,csenum);
	}
}

rewsemi(p)
NODE *p;
{
			/*Given a semilp; if the right child
			  of a chain is a temp, rewrite
				p;TEMP  to (p,TEMP)
			*/
	NODE *ptemp, *new, *save;
	ptemp = p;
	while (semilp(ptemp->in.right))
	{
		ptemp = ptemp->in.right;
	}
	if (ptemp->in.right->in.op != TEMP)
		return 0;
			/*Do the rewrite*/
	if ( ptemp == p )	/*Only the one ; exists*/
	{
		p->in.op = COMOP;
		p->in.strat &= ~(RTOL|LTOR|PAREN);
		return 1;
	}
	new = talloc();
	*new = *p;
	p->in.op = COMOP;
	p->in.left = new;
	p->in.right = ptemp->in.right;
	p->in.strat = 0;
			/*Now, get rid of the semi that used to point
			  to the temp*/
	save = ptemp->in.left;
	*ptemp = *ptemp->in.left;
	save->in.op = FREE;
#ifndef NODBG
	if ( odebug > 1)
	{
		fprintf(outfile,"rewsemi rewrites semilp:\n");
		e2print(p);
	}
#endif
	return 1;
}

static void  
rewcse(p,copy,csenum)
NODE *p, *copy;
int csenum;
{
	NODE *newcopy;
			/*walk the tree. rewrite cse's into copies of"copy".*/
	if ( p->in.op  == CSE && p->csn.id == csenum)
	{
			/*yech. copy the tree, but not the top node*/
		newcopy = tcopy(copy);
		*p = *newcopy;
		newcopy->in.op = FREE;
		return;
	}
	switch( optype(p->in.op))
	{
		case BITYPE:
			rewcse(p->in.right,copy,csenum);
			/*FALLTHRU*/
		case UTYPE:
			rewcse(p->in.left,copy,csenum);
	}
}
void
ret_type(t)
TWORD t;
{
#ifdef TMPSRET
	if (t == TSTRUCT && str_spot == -1)
	{
		str_spot = next_temp(TPOINT, (BITOFF) SZPOINT, ALPOINT);
	}
#endif
}

RST
docse(p, goal, haveregs, rs_avail, rs_want, left_want, rs_left)
register NODE *p;
int goal;
int haveregs;                               /* Numbers of regs available*/
RST rs_avail;                           /* these regs available */
RST rs_want;                            /* want result in one of these */
RST left_want;				/* want defn of LET in one of these*/
RST *rs_left;				/* Put the regs used by the left
					   subtree here*/
{
	register NODE *l, *r;
	int id;
	RST rc;
	struct cse * new_cse;
	l = getl(p);
	r = getr(p);
	id = p->csn.id;
	
#ifndef NODBG
	if (odebug)
	{
		fprintf(outfile,"Handle LET node %d\n", node_no(p));
	}
#endif
			/*Find the cse slot in the table.  If may already
			be there, because of rewrites; or we may have
			to add it*/
	if ( (new_cse = getcse(id)) == 0 && cse_ptr < cse_list+MAXCSE)
	{
		new_cse = cse_ptr;
					/*Remember the cse*/
		new_cse->id = id;
		++cse_ptr;
	}
			/*If this is a null let, or the list is full,
			  put it in memory*/
	if (l->in.op == NOP || ! new_cse )
	{
#ifndef NODBG
		if (odebug)
			fprintf(outfile,"Null cse to temp\n");
#endif
		csetemp(p);
		return REWROTE;
	}
			/*Generate the code for the definition*/
	if ( (rc = insout( l, NRGS, haveregs, rs_avail, left_want)) & RS_FAIL)
	{
			/*Problems in the definition- use a temp*/
#ifndef NODBG
		if (odebug)
			fprintf(outfile,"Problems on left side\n");
#endif
		if ( rc == REWROTE)
			return REWROTE;

		/*we are out of registers*/
		/*If we didn't start will them all, pass the buck*/
		if (haveregs < NRGS)
			return OUTOFREG;

		/*Otherwise, try to copy; if it fails, go to temp*/
		if (csecopy(p) == 0)
			csetemp(p);
		return REWROTE;
	}
			/*Stash the register for the left hand side*/
	*rs_left = rc;

	if ( (new_cse->reg = reg_num(rc)) == -1)
	{
		e2print(p);
		cerror("Bad register for cse");
	}

#ifndef NODBG
	if (odebug)
		fprintf(outfile,"Cse %d in reg %d\n",id, new_cse->reg);
#endif
			/*Remove, the registers used on the definition*/
	haveregs -= szty(l->in.type);
	rs_avail -= rc;
			/*Now, do the body of the CSE*/
	rc = insout(r, (goal!=CTEMP?goal:NRGS), haveregs, rs_avail, rs_want) ;
			/*free the cse entry*/

	if ( rc == REWROTE)
		return REWROTE;
	else if (rc == OUTOFREG)
	{
		/*we are out of registers*/
		/*If we didn't start will them all, pass the buck*/

		if ((haveregs + szty(l->in.type)) < NRGS)
			return OUTOFREG;

		/*First, try to copy; if it fails, go to temp*/
		if (csecopy(p) == 0)
			csetemp(p);
		return REWROTE;
	}
			/*If we get here it worked; there was a reg*/
	return rc;
}

struct cse *
getcse(id)
int id;
{
			/*Given an id, find the entry in the table*/
	struct cse *cp;
	for (cp = cse_list; cp < cse_ptr; ++cp)
	{
		if (cp->id == id)
			return cp;
	}
	return (struct cse *) 0;
}
static int
reg_num(rst)
RST rst;
{
	int i;
	for (i=0; i<TOTREGS; ++i)
	{
		if (RS_BIT(i) & rst)
			return i;
	}
	return -1;
}


NODE *
dolocal(p)
NODE *p;
{
			/* call clocal() on the whole tree, bottom up*/
	switch (optype(p->in.op))
	{
	case BITYPE:
		p->in.right = dolocal(p->in.right);
		/*FALLTHRU*/
	case UTYPE:
		p->in.left = dolocal(p->in.left);
	}
	return clocal(p);
}

static char  scratchregs[TOTREGS],  callersave[TOTREGS],  calleesave[TOTREGS];
extern int r_caller[];
#define YES 1
#define NO 0
			/*Initialize the three lists of register types.
			  called once by p2init. */
static void
fill_rgrays()
{
	register int i, *ip;
			/*Initially, all are scratch or calleesave*/
	for ( i = 0; i < NRGS; ++i)
	{
		scratchregs [i] = YES;
		callersave[i] = calleesave[i] = NO;
	}
	for ( i = NRGS; i < TOTREGS; ++i )
	{
		calleesave [i] = YES;
		callersave[i] = scratchregs[i] = NO;
	}

			/*Read the r_caller list (of caller save regs)*/
	for ( ip = r_caller; *ip != -1; ++ip)
	{
		if (*ip < 0 || *ip >= TOTREGS)
			cerror("Bad register number in r_caller array");
		else if (*ip <= NRGS)
			cerror("Register is both scratch and caller save");
		callersave[*ip] = YES;
		calleesave[*ip] = NO;
	}
}
char *
caller_save()
{
	return callersave;
}
char *
callee_save()
{
	return calleesave;
}
char *
scratch()
{
	return scratchregs;
}
static int
can_except(p)
NODE *p;
{
		/*return 1 iff p->in.op can cause an exception*/
	switch(p->in.op)
	{
	case UNARY MINUS:
	case CONV:
	case PLUS:
	case ASG PLUS:
	case MINUS:
	case ASG MINUS:
	case MUL:
	case ASG MUL:
	case DIV:
	case ASG DIV:
	case LS:
	case ASG LS:
	case INCR:
	case DECR:
	case CALL:
	case UNARY CALL:
	case STCALL:
	case UNARY STCALL:
	case ALLOC:
	case CAPCALL:
	case VLRETURN:
	case RSAVE:
		return 1;
	default:
		return 0;
	}
}
void
uncomma(p)
NODE *p;
{
			/*rewrite all of the commas to ;lp in this
			  tree. Called from rewcom.  Since rewcom has
			  already been called on this tree,
			  the COMOPs should all be at the top*/
	if (p->in.op != COMOP)
		cerror("bad node in uncomma");
	p->in.op = SEMI;
	p->in.strat |= (LTOR | PAREN);

	if (p->in.left->in.op == COMOP)
		uncomma(p->in.left);
	if (p->in.right->in.op == COMOP)
		uncomma(p->in.right);
}
			
#ifndef NODBG
			/*This function is NEVER CALLED.  It is here
			  to make sure that all of the functions that
			  are promised to nifty do, in fact, exist; it
			  makes a reference to each of them.*/
void
nevercall(){
	cerror("nevercall called!!!!!");
	rgsave();
	canbereg();
	tyreg();
	reglst();
	gtalign();
	gtsize();
	ofile();
	profile();
	set_next_temp();
	next_temp();
	max_temp();
	tcheck();
	tshow();
}

char *
typename(t)
TWORD t;
{
	/*return a pointer to a printable name for a type*/
	
	static int twords[]={ 	TCHAR, TUCHAR, TSHORT, TUSHORT,
				TLONG, TULONG, TINT, TUNSIGNED,
				TDOUBLE, TFLOAT, TSTRUCT, TPOINT,
				TPOINT2, TVOID, TANY, TFPTR, 0, -1};
	static char *tnames[]={"char","uchar","short","ushort",
	"long","ulong","int","unsigned","double","float","struct",
	"point","point2","void","any","fptr","null"};
	int i;
	for (i=0; twords[i] != -1; ++i)
	{
		if (twords[i] == t)
			return tnames[i];
	}
	return "(bad type)";
}
#define IN 0
#define TN 1
#define BN 2
#define STN 3
#define FPN 4
#define CSN 5

int
nodetype(op)
int op;
{
			/*Return the type (in, tn, etc.) of this node*/
			/*First, switch on the op for special nodes*/
	switch(op)
	{
	case LET:
	case CSE:
		return CSN;
	case FCON:
		return FPN;
	case STASG:
	case STARG:
	case STCALL:
	case UNARY STCALL:
		return STN;
	case GENLAB:
	case GENBR:
	case GENUBR:
	case JUMP:
	case GOTO:
	case SWCASE:
	case LABELOP:
		return BN;
	};
			/*Remaining: if a leaf, return "tn" else "in".*/
	if (optype(op) == LTYPE)
		return TN;
	else
		return IN;
}
char *
anyname(p)
NODE *p;
{
	static char name_buff[256];
	char *s = p->in.name;
	char short_name[20];
	int i;
	if (!s)
		return "-";
	switch(p->in.op)
	{
	case BEGF:
	case RREST:
	case RSAVE:
		strcpy(name_buff, "{");
		for (i=0; i<TOTREGS; ++i)
		{
			if (s[i])
			{
				sprintf(short_name, "%d,", i);
				strcat(name_buff, short_name);
			}
		}
		strcat(name_buff, "}");
		return name_buff;
	}
	if(strlen(s) >= 250)
		return "\"(BIG NAME)\"";
	strcpy(name_buff, "\"");
	strcat(name_buff,s);
	strcat(name_buff,"\"");
	return name_buff;
}
void
cgprint(p,indent)
NODE *p;
int indent;
{
	int i;
	FILE *saveofile;
	if (indent == 0)
	{
		fprintf(outfile,"\n");
		fflush(outfile);
		saveofile = outfile;

			/*Use local_ofile instead of ofile to avoid
			  the extra locctr output*/

		local_ofile(debugfile);
		fprintf(outfile,"*****\n");
	}
			/*Print the ctree in a format that can be read
			  (with minor changes) as a ctree file*/

	if(!p)
	{
		fprintf(outfile,"# **** NULL ****\n");
	}
	else
	{
				/*Print the left child first*/
		if(optype(p->in.op) == BITYPE)
			cgprint(p->in.right, indent+1);

				/*Next, print this node*/
		

		for(i=0; i<indent; ++i)
			fprintf(outfile, "  ");
		fprintf(outfile,"%d  %s", node_no(p), opst[p->in.op]);
		prstrat(p->in.strat);
		fprintf(outfile," %s ", typename(p->in.type));
		switch(nodetype(p->in.op))
		{
		case IN:
		case TN:
			fprintf(outfile,"%s",anyname(p));
			print_left_right(p);
			break;
		case BN:
			fprintf(outfile,"%d  %d", p->bn.label, p->bn.lop);
			print_left_right(p);
			break;
		case STN:
			fprintf(outfile,"%d  %d  %d", p->stn.stsize, p->stn.stalign,
			p->stn.argsize);
			print_left_right(p);
			break;
		case FPN:
			fprintf(outfile,"%lf", p->fpn.dval);
			break;
		case CSN:
			fprintf(outfile,"%d", p->csn.id);
			print_left_right(p);
			break;
		default:
			fprintf(outfile,"*** Bad node type ****");
			break;
		}
		switch (p->in.goal)
		{
		case 0:
			fprintf(outfile, "\t#(NULL)");
			break;
		case CCC:
			fprintf(outfile, "\t#(CC)");
			break;
		case CEFF:
			fprintf(outfile, "\t#(CEFF)");
			break;
		case NRGS:
			break;
		default:
			fprintf(outfile, "\t#(BAD GOAL: %d)", p->tn.goal);
			break;
		}
		fprintf(outfile, "\n");

				/*Print the right child last*/
		if(optype(p->in.op) != LTYPE)
			cgprint(p->in.left, indent+1);
	}
	if(indent == 0)
	{
		fprintf(outfile,"=====\n");
		fflush(outfile);
		local_ofile(saveofile);
	}
}

print_left_right(p)
NODE *p;
{
	switch(optype(p->in.op))
	{
	case BITYPE:
		fprintf(outfile,"  %d  %d",
			node_no(p->in.left),node_no(p->in.right));
		break;
	case UTYPE:
		fprintf(outfile,"  %d  %d", node_no(p->in.left), p->in.rval );
		break;
	case LTYPE:
		fprintf(outfile,"  %d  %d", p->in.lval,p->in.rval);
		break;
	default:
		cerror("Internal: bad op type");
	}
}

#endif
static int
semichain(p)
NODE *p;
{
			/*Return 1 iff this is a ;lp chain with a
			  temp at the bottom*/
	NODE *ptemp = p;

	if (!(semilp(ptemp)) && ptemp->in.op != COMOP)
		return 0;
	do
	{
		ptemp = ptemp->in.right;
	}
	while (semilp(ptemp) || ptemp->in.op == COMOP);
	return ( ptemp->in.op == TEMP);
}

static void
semi_to_first(p)
NODE *p;
{
			/*p is an ordered node, the last side of which is
			  a semichain.  Move the semichain to the first
			  side of the tree.*/
	NODE *first = firstl(p);
	NODE * last = lastl(p);
	NODE * last_semi, * rsemi, * temp, * kill_me;
#ifndef NODBG
	if (odebug)
	{
		fprintf(outfile,"semi_to_first rewrites:\n");
		e2print(p);
	}
#endif
			/*The rewrite is (if LTOR):
						OP{l}
					    A           ;{lp}
						    B          T


				becomes
	
						OP{l}
					    ;{rp}       T
					B        A

			the left and right are reversed if order is RTOL
			***/
						    
					
	last_semi = last;
	while ( last_semi->in.right->in.op == SEMI 
	|| last_semi->in.right->in.op == COMOP )
	{
		last_semi = last_semi->in.right;
	}
			/*Remember where the temp is*/
	temp = last_semi->in.right;
	if (temp->in.op != TEMP)
		cerror("semi_to_first");
			/*Now, we must remove the last_semi*/
	kill_me = last_semi->in.left;
	*(last_semi) = *(kill_me);
	kill_me->in.op = FREE;
			/*The temp is now dangling*/
			/*Make a rightfirst semicolon*/
	rsemi= talloc();
	rsemi->in.op = SEMI;
	rsemi->in.strat = (RTOL|PAREN);
	rsemi->in.type = first->in.type;
	rsemi->in.left = last;
	rsemi->in.right = first;

	if (p->in.strat & LTOR)
	{
		p->in.left = rsemi;
		p->in.right = temp;
	}
	else
	{
		p->in.right = rsemi;
		p->in.left = temp;
	}
#ifndef NODBG
	if (odebug)
	{
		fprintf(outfile,"rewritten to:\n");
		e2print(p);
	}
#endif

}

NODE *
leteff(p)
NODE *p;
{
		/*Do the condit()-type code for LET nodes done for effect.*/
		/*Analagous to andeff and oreff*/
		/*If the right side doesn't do anything, do the lhs for
		  effects.  If the right side does do something, do the lhs
		  for value*/

	NODE *condit();
	p->in.right = condit( p->in.right, CEFF, -1, -1 );
	if (!p->in.right)
	{
		/*rhs does nothing. Delete the LET and return the
		  (possible NIL) lhs */
		p->in.left = condit( p->in.left, CEFF, -1, -1 );
		p->in.op = FREE;
		return p->in.left;
	}
	p->in.left = condit( p->in.left, NRGS, -1, -1 );

		/*The lhs better have a value!*/
	return p;
}
