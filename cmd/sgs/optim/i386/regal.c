/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)optim:i386/regal.c	1.28"

#include "optim.h"
#include "optutil.h"
#include "regal.h"
#include <values.h>
#include "storclass.h"
#include "debug.h"	/* import mod_debug(); */

#ifdef IMPREGAL
static struct regal *new_regal();/* allocate space for a regal and zero the fields */
static void dead_regal();	/* destroy info in regal node */
static void dealloc_regals();	/* free all regal nodes */
static void remove_aliases();	/* find regals that match alias nodes and
				   discard regal nodes and all alias nodes */
static void add_global_regals();/* create regals for globals/statics */
static void Estim();
void ratable();
static int raavail();
static void rainsld();
static int ravassign();
static enum scratch_status raregok();
static boolean rald();
static void raremld();
#endif
static void raopn();
static void raparam();

/* following ugly macro checks if a string looks like the
   identifier for a global variable */
#define isglobal(ptr) ( (isalpha(*(ptr)) || *(ptr) == '_' \
			||  *(ptr) == '.' && (ptr)[1] != '.' ) \
			&& strchr((ptr),'%') == NULL \
			&& strchr((ptr),'$') == NULL)
/* following macro is used to check for the character following
   a global */
#define null_plus_or_minus(x) ( (x) == '\0' || (x) == '+' || (x) == '-' )

extern int asmflag; /* flag indicating presence of asm in function */


/* The following routines xxx_regal() perform the bookkeeping for the */
/* regal nodes. Two structures are maintained: an open hash table, for easy */
/* lookup, and a list, for node traversal.   */

#define RGLHASHSZ	1019		/* size of hashtab (make it prime) */
static struct regal *hashtab[RGLHASHSZ];	/* hash table of regals */
static struct regal *first_regal = NULL;	/* first node of regal list */
static struct regal *last_regal = NULL;	/* last node of regal list */

static struct regal rglinit = {NULL,0,SCLNULL,0,unk_scratch,NULL,NULL};
/* The last member, rgl_instr_type will be filled in by ravassign(). */


	static struct regal *
new_regal()		/* allocate space for a regal and zero the fields */
{
  struct regal *p;
  p = GETSTR(struct regal);
  *p = rglinit;
  return (p);
}
	static void
dead_regal(p)			/* destroy info in regal node */
register struct regal *p;
{
  p->rglname[0] = '\0';
  p->rglestim = 0;
  p->rglscl = SCLNULL;
  p->rgllen = 0;
}
	static void
dealloc_regals()		/* free all regal nodes */
{
				/* init() will free up memory */
  				/* so just nullify pointers. */
  register int i;
  first_regal = NULL;
  last_regal = NULL;
  for (i = 0; i < RGLHASHSZ; i++)
    hashtab[i] = NULL;
}
	struct regal *
lookup_regal(name,install)	/* look up regal in hash table */
char *name;		/* name of the form "n(%ebp)" or global ident */
boolean install;	/* install if not found? */
{
  register char *c;
  register struct regal *p,**hp;
  register unsigned h=0, g;

  for(c=name; *c != '\0'; c++ ) { /* apply hashpjw hash function,
				    Aho, Sethi, & Ullman p436 */
    h = (h<<4) + *c;
    if(g = h & 0xf0000000) {
	h ^= g ^ (g>>24);
    }
  }

  h = h % RGLHASHSZ;

  hp = &hashtab[h];
  for (p = *hp; p != NULL; p = p->rgl_hash_next)
    if (strcmp(p->rglname,name) == 0)
      return (p);

  if (install == false)
    return NULL;

  p = new_regal();
  p->rgl_hash_next = *hp;
  *hp = p;
  if (first_regal == NULL)
    first_regal = p;
  else
    last_regal->rglnext = p;
  last_regal = p;
  p->rglname = strdup(name);
  if(p->rglname == NULL)
    fatal("lookup_regal: installation failed - %s\n",name);
#ifdef DEBUGREGAL
{
struct regal *q = first_regal;
while(q != NULL)
	q = q->rglnext;
}
#endif
  return (p);

}

/* The following routines xxx_alias() perform the bookkeeping of alias nodes. */
/* Only a linked list is maintained for alias nodes. */

static struct regal *first_alias = NULL;	/* points to first in alias list */

	struct regal *
new_alias()			/* append alias node to list of aliases */
{
  register struct regal *a;
  a = new_regal();
  a->rglnext = first_alias;
  first_alias = a;
  return (a);
}


struct regals {
	char *name;
	struct regals *next;
} *first_regals;

	static void
remove_aliases()	/* find regals that match alias nodes and discard */
			/* regal nodes and all alias nodes */
{
 register struct regal *r, *a;
 register struct regals *p;
 a = first_alias;
 while (a != NULL) {
   r = lookup_regal(a->rglname,false);
   if (r != NULL) {
     if (a->rgllen != r->rgllen)
       fatal("remove_aliases: lengths not equal\n");
     dead_regal(r);
   }
   a = a->rglnext;
 }
 first_alias = NULL;

 first_regals = NULL;
 for (r = first_regal; r != NULL; r = r->rglnext)
	if (r->rglname[0]) {
		if (!(p = (struct regals *)malloc(sizeof(struct regals))))
			fatal("cannot create regals list\n");
		p->name = strdup(r->rglname);
		p->next = first_regals;
		first_regals = p;
	}
}


	boolean
lookup_regals(name)
char *name;
{
	struct regals *p;

	for (p = first_regals; p; p = p->next)
		if (!strcmp(p->name, name))
			return true;
	return false;
}

	static void
add_global_regals() /* look for globals ( identifiers used as addresses )
			and add them to the regals list */
{
    /* register */struct regal *r;
    /* register */NODE *pn;
    int j;
    char * q;
    for( ALLN(pn) ) {
	int thisop = pn->op;
	if( isprof(pn->forw) ) pn = pn->forw->forw;
	if( isbr(pn) ) continue;
	for( j = (thisop == LEAL)?2:1; j <= MAXOPS; j++) {
             /* The ugly initializer means: skip the first operand
                for LEAL (address arithmetic) */
	    /* Now check if no operand or the operand is the
	       second in a pushl instruction, i.e., the phoney
	       TMPSRET operand for the structure return hack. */
	    if( (q = pn->ops[j]) == NULL || 
		thisop == PUSHL && j == 2 ) break;
	/* Now look for identifier ( compiler will generate 
	   .Xxx for internal static variables ) */
#define INSTALL 1
 	    if ( isglobal(q) ) {
		while(!isspace(*q) && *q != '\0' && *q != '+' 
		   && *q != '@') ++q;
		if (*q == '@' && *++q == 'P') { 

	       	/*  Hack to check for PIC style
		    globals referencing PLT, e.g., xxx@PLT:
		    these can't be addressed indirectly thru reg
		    because they're PC relative */

		    continue;
		}
		else {
		   while(!isspace(*q) && *q != '\0' && *q != '+') ++q;
		}
		{
		char save;
		save = *q;
		*q = '\0';
		r = lookup_regal(pn->ops[j],INSTALL);
		*q = save;
		r->rglscl = GLOBAL;
		r->rgllen = 4;
		}
	    }
	    continue;
	}
    }
}

/* The routine Estim() computes the weights for the regal nodes */
#define	SOURCE	1
#define	MAXWEIGHT	(MAXINT - 1000)
						/* ARG_COST is the number */
						/* of cycles it takes to */
						/* load an argument into a */
						/* register.	*/
#define	ARG_COST	4
						/* GLOBAL_COST is the cost
						   of loading an effective
						   address into a register */
#define GLOBAL_COST	2
						/* PAYOFF is the difference */
						/* in cycles between memory */
						/* and register reference. */
#define	PAYOFF		3
						/* POINTER_PAYOFF is the */
						/* difference in cycles */
						/* between using a pointer */
						/* in memory and a pointer */
						/* in a register.	*/
#define	POINTER_PAYOFF	4
						/* WEIGHT is the assumed */
						/* number of times through */
						/* a loop.	*/
						/* There is no cycle payoff
						   for loading addresses of
						   globals into registers:
						   we just get a shorter
						   instruction.
						*/
#define GLOBAL_PAYOFF	1
#define	WEIGHT		8			/* To optimize multiplies and
				  		 divides, make it a power of 2. */
						/* MAXLDEPTH is the maximum loop*/
						/* depth for weight adjustments.*/
						/* (log MAXINT base WEIGHT).    */
#define MAXLDEPTH	10

	static void
Estim()
{
 register struct regal *rgl;
 register int i;
 int depth;			/* #LOOP depth.	*/
 long weight;			/* Weight of address use.	*/
 register NODE *pn;
 char type;
 long int new_estim;
 extern int optmode;		/* Size, Default, or Speed mode.	*/
 register int payoff;

 /* Charge cost of moving a parameter or the address of a global
    to a register. */
 for (rgl = first_regal; rgl != NULL; rgl = rgl->rglnext)
    switch(rgl->rglscl) {
    case PARAM:
	rgl->rglestim = -ARG_COST;
	break;
    case GLOBAL:
	rgl->rglestim = -GLOBAL_COST;
	break;
    default:
	rgl->rglestim = 0;
	break;
    }
 weight = 1;					/* Set initial payoff.	*/
 depth = 0;

						/* CALCULATE ESTIMATES.	*/
 for ( ALLN(pn) )	/* Scan all the text nodes. */
 {
   if (pn->op == LCMT)	/* Adjust weight for loops. */
     {
       if(optmode == OSIZE)		/* If optimizing for size, */
	 continue;		/* don't weigh loops.	*/
       type = *(pn->opcode);	/* Get loop type */
       if (type == 'H')		/* "HDR" - Increase weight for	*/
	 			/* loop entry.	*/
	 {++depth;
	  if(depth <= MAXLDEPTH)
	    weight *= WEIGHT;
	}	
       else if(type == 'E')		/* "END" - Decrease weight for	*/
					/* loop exit.	*/
	 {if(depth <= MAXLDEPTH)
	    weight /= WEIGHT;
	  --depth;
	}
       DELNODE(pn);		/* Delete loop node */
       continue;		/* LOOP nodes have no	*/
     }				/* interesting operands.*/

   for (i = 1; i < MAXOPS + 1; i++) {
     if ((pn->ops[i] == NULL) ||
	 (rgl = lookup_regal(pn->ops[i],false)) == NULL)       
       continue;		/* Just examine regals */

     if(rgl->rglscl == GLOBAL)
	 payoff = GLOBAL_PAYOFF;
     else {
	 			/* The next few lines adjust the weight	*/
				/* for this operand if it appears to be	*/
				/* a pointer as evidenced by being the	*/
				/* source of a move to a scratch register. */
         if((pn->op == MOVL) && 	/* If it's a MOVL instruction */
	    (i == SOURCE) && 	/* and we're looking at the source */
	    (isreg(pn->op2)) &&	/* and the dest is a scratch reg */
	    (pn->op2[3] == 'x') && 	/* (i.e. "%eax", "%ecx", or "%edx" */
	    (pn->op2[2] != 'b'))	/* but not "%ebx"), */
           payoff = POINTER_PAYOFF;	/* then found a pointer regal */
         else
           payoff = PAYOFF;		/* else found a regular regal */
     }
     new_estim = rgl->rglestim;
     if(((MAXWEIGHT - new_estim) / weight) < payoff)
       new_estim = MAXWEIGHT;
     else
       new_estim += (weight * payoff);
     rgl->rglestim = new_estim;
/*     printf("/Adding %d to %s\n",new_estim,rgl->rglname);*/
   }
 }
 
 return;
}

/* register allocation optimization
**
**   6/16/83 - for the 3b2
**   Modified  - for the i386
**
** This optimization accepts a list of autos and args from the
** compiler and assigns them to registers if there is sufficient estimated
** payoff.  
**
** Scratch registers are used first, if they are available, 
** because they are "free".  A variable can be put into a scratch register if
** 	(a) the register is not used by any instruction in the procedure,
**	(b) the variable is dead, i.e., not used before being set, 
**          following any call in the procedure
** 
** Next registers edi, esi, ebx are assigned.
**
** Register variables assigned by the compiler stay as they are.
** Variables in the list of autos and args obtained from the #REGAL statements
** are assigned by order of a computed weight.
** 
** All register variables assigned by the compiler remain in 
** registers.  Other variables are assigned only if the
** estimated payoff os positive.
** 
** Instruction operands are modified to reflect the additional assignments.
**
*/

#define MAXREGS 14
#define MAXVARS 20

/* register types */

#define SCRATCH 0
#define USER 1

/* register status */

#define AVAIL 0
#define COMPILER_REGISTER 1
/* COMPILER_REGISTER means the compiler allocated it and it does not
   figure in register allocation by optim. */
#define ALLOCATED 2
#define COMPANION_IS_ALLOCATED 3
/* In this status, one of the register's companions has
   been allocated, so this register is not available:
   e.g., %ebx %bx */

/* cycle estimates for i386 instructions */
#define SAVE_RESTORE_CYCLES 6	/* to save and restore an additional register */

static struct regal high[MAXVARS];

int vars = 0;		/* number of variables stored in 'high' array */

/*
 * The register number for this structure must be the same as those
 * register numbers in the 386 rcc C compiler.  WARNING IF THE 386 C
 * COMPILER CHANGES IT'S REGISTER NUMBERS, THIS FILE MUST ALSO CHANGE!!
 * This comes from outreg in local2.c of the C compiler.
 */
static struct assign asreg[MAXREGS] = {
/* 0 */	{ {"%eax","%ax","%al"}, EAX, reg_eax, SCRATCH, &asreg[6],&asreg[7] },		
/* 1 */	{ {"%edx", "%dx","%dl"}, EDX, reg_edx, SCRATCH, &asreg[12], &asreg[13] },
/* 2 */	{ {"%ecx", "%cx","%cl"}, ECX, reg_ecx, SCRATCH, &asreg[10], &asreg[11] },
/* 3 */	{ {"%edi", "%di", 0   }, EDI, reg_edi, USER, 0, 0 },
/* 4 */	{ {"%esi", "%si", 0   }, ESI, reg_esi, USER, 0, 0 },
/* 5 */	{ {"%ebx", "%bx","%bl"}, EBX, reg_ebx, USER, &asreg[8],&asreg[9] },
/* 6 */	{ {"%eax","%ax","%ah"}, EAX, reg_ah, SCRATCH, &asreg[0], 0 },
/* 7 */	{ {"%eax","%ax","%al"}, EAX, reg_al, SCRATCH, &asreg[0], 0 },
/* 8 */	{ {"%ebx", "%bx","%bh"}, EBX, reg_bh, USER, &asreg[5], 0 },
/* 9 */	{ {"%ebx", "%bx","%bl"}, EBX, reg_bl, USER, &asreg[5], 0 },
/* 10*/	{ {"%ecx", "%cx","%ch"}, ECX, reg_ch, SCRATCH, &asreg[2], 0 },
/* 11*/	{ {"%ecx", "%cx","%cl"}, ECX, reg_cl, SCRATCH, &asreg[2], 0 },
/* 12*/	{ {"%edx", "%dx","%dh"}, EDX, reg_dh, SCRATCH, &asreg[1], 0 },
/* 13*/	{ {"%edx", "%dx","%dl"}, EDX, reg_dl, SCRATCH, &asreg[1], 0 }
};

#define CHAR_REGS 6

#ifdef NOSPLIT
#undef MAXREGS
#define MAXREGS CHAR_REGS
#endif

#define	LEGALREG	(EAX|EDX|ECX|EBX|ESI|EDI)
#define	legalreg(x,y)	(usesreg(x,y) & LEGALREG)


/* main routine for register allocation optimization. */
/* Return number of registers saved/restored after transformations. */
int
raoptim( numnreg, numauto, noauto )
int numnreg;	/* input number of registers saved/restored */
int numauto; 	/* number of words of automatic variables */
int *noauto;

{
	int rcnt;	/* number of register free */
	register int ias;

	/* check for asms */
	if( asmflag ) {
		vars = 0;
		ratable();
		return( numnreg );
	}

	ratable();		/* Create table of topmost regals */

	/* initialize assignment table */
	for( ias = 0; ias < MAXREGS; ias++ ) {
		asreg[ias].asavail = AVAIL;
		asreg[ias].assigned_regal = NULL;
	}

	/* determine availability of registers */
	rcnt = raavail();
	if (rcnt == 0)		/* if no free registers, quit */
	  return (numnreg);

	/* insert branch pointers for live/dead analysis */
	rainsld();
	
	/* assign non-register variables, return byte count of
	   autos assigned, giving new auto count */
	numauto -= ravassign();
	*noauto = !numauto;

	/* remove remove branch pointers from live/dead analysis */
	raremld();

	/* modify the instruction operands */
	raopn();

	/* insert moves for parameters/load/stores etc. */
	raparam();

	mod_debug(); /* fix up the debugging info for this fn */

	/* re-initailize for next procedure */
	vars = 0;

	/* return new number of registers saved/restored */
	return(numnreg);
}

/* Use the lists of regals and aliases obtained from the #REGAL and */
/* #ALIAS statements to produce a table of variables (high[]), ordered */
/* from highest weight to lowest. */
	void
ratable()
{
  register struct regal *p, *maxregal;
  register int maxestim;


  remove_aliases();	/* removed aliased variables from regals list */
#ifdef DEBUG
	if(! getenv("NOGLOBALS"))
#endif
  add_global_regals();	/* look for global variables and add to the
			   regals list.  We will try to access these
			   with pointers in registers ( shorter instr ) */

      /* copy the top MAXVARS nodes with the greatest weights into high[]. */
      Estim();				/* Compute weights for regals */
      for (vars = 0; vars < MAXVARS; vars++) {
	maxestim = 0;
	for (p = first_regal; p != NULL; p = p->rglnext) {
	  if (p->rglestim > maxestim) {
	    maxestim = p->rglestim;
	    maxregal = p;
	  }
	}
	if (maxestim == 0)
	  break;
	else {
	  struct regal *temp = high + vars; /* address of high[vars] */
	  *temp = *maxregal; /* copy the struct */
	  temp->rglname = strdup(maxregal->rglname);
	  dead_regal(maxregal);
	}      
      }  
  dealloc_regals();			/* Throw out the regals list */
}



/* determine availability of registers */
/* Find all the registers that are used or set in the function and */
/* mark them unavailable.  Return number of registers left */
	static int
raavail( )
{
	register NODE *pn;
	register int iop, regused, rcnt;

	regused = 0;
	/* scan for uses */
	for( ALLN( pn ) ) { /* scan nodes */
		/* check for profiling code */
		if( isprof( pn->forw ) )
			pn = pn->forw->forw;

		/* check for use of registers */
		if (pn->op == CALL)	/* call sets scratch regs */
			regused |= uses(pn);
		else if (pn->op != RET) {/* ret turns on everything... */
			regused |= uses(pn) | sets(pn);
		}
	}
	rcnt = MAXREGS;
	for( iop = 0; iop < MAXREGS; iop++ ) {
		if (  asreg[iop].asavail == AVAIL &&
		      regused & asreg[iop].asrfld) {
			asreg[iop].asavail = COMPILER_REGISTER;
			rcnt--;
		}
	}
	return( rcnt );
}

/* insert pointers for live/dead analysis */
/* Place in each branch node a pointer to its target label node */
	static void
rainsld()
{
	register NODE *pn, *qn;
	register char *ppn;

	for( ALLN( pn ) ) {
		if( isbr( pn ) && !isret( pn ) ) {
			ppn = getp( pn );
			/* use wraparound scan when looking for label */
			for( qn = ( pn->forw != &ntail ) ? pn->forw : n0.forw; 
				qn != pn; qn = ( qn->forw != &ntail ) ?
				qn->forw : n0.forw ) {
				if( islabel( qn ) ) {
					if(*(ppn + 2) != *(qn->ops[0] + 2) &&
						*(ppn + 1) != '\0' ) 
						continue; /* for speed */
					if( strcmp( ppn, qn->ops[0] ) == 0 ){
						pn -> opm = (char *) qn;
						break;
					}
				}
			}
		}
	}
}


/* assign non-register variables 			*/
/* For each variable in high[]: 			*/
/*	if it is used in special instructions		*/
/* 		then skip the variable.			*/
/*	else find an available register to assign to it,*/
/* 		using a scratch reg if possible. 	*/
/* Return total bytes of AUTOs assigned			*/

	static int
ravassign()
{
	static enum valid_types rackinstr();
	int ivar, *ordptr;
	struct assign *pa;
	struct regal *ph;
	int assigned_auto_bytes = 0;

	/* following two arrays give the allocation ordering
	   for 1 byte vs. multibyte variables. */

#ifdef NOSPLIT
	static int charorder[] = {
	    0,	/* reg_eax */
	    1,	/* reg_edx */
	    2,	/* reg_ecx */
	    5,	/* reg_ebx */
	    -1  /* sentinel */
	};
#else
	static int charorder[] = {
	    6,	/* index of reg_eax in asreg */
	    7,	/* index of reg_edx in asreg */
	    10, /* reg_ecx */
	    11, /* reg_ebx */
	    12,
	    13,
	    8,
	    9,
	    -1	/* sentinel */
	};
#endif

	static int intorder[] = {
	    4,	/* reg_esi */
	    3,	/* reg_edi */
	    0,	/* index of reg_eax in asreg */
	    1,	/* reg_edx */
	    2,	/* reg_ecx */
	    5,	/* reg_ebx */
	    -1 /* sentinel */
	};

	/* go through variables */
    for( ivar = 0; ivar < vars; ivar++ ) {
	ph = &high[ivar];

	if ( (ph->rgl_instr_type = rackinstr(ph->rglname)) == NO_TYPES ) continue;
	/* Note (psp) rackinstr goes thru ALL nodes! */


	/* point at the correct ordering */
	if(ph->rgllen == 1) /* char variable */
	    ordptr = &charorder[0];
	else
	    ordptr = &intorder[0];
	for(; *ordptr != -1; ordptr++ ) {
	    pa = &asreg[*ordptr];
	    if( pa->asavail != AVAIL ) continue;
	    /* check register can be referenced as a byte */
	    if ( ph->rgl_instr_type == ONLY_BYTE && !pa->asrname[2])
		continue;

	    /* Check the cost */
	    if (pa->asrtype != SCRATCH) {
		/* always pays to assign to scratch */
	        if(ph->rglestim <= SAVE_RESTORE_CYCLES) {
		    /* maybe find a scratch? */
		    continue;
	        }
	    }
	    else { /* SCRATCH so have to check this regal */
	        if( ph->rgl_scratch_use == unk_scratch)
		    ph->rgl_scratch_use = raregok(ph);
			/* Sets member to no_scratch or ok_scratch */
		if(ph->rgl_scratch_use == no_scratch)
		   continue;
	    }
	    /* assign it */
	    pa->asavail = ALLOCATED;
	    /* make overlapping regs unavailable */
	    if (pa->h_reg)
		pa->h_reg->asavail = COMPANION_IS_ALLOCATED;
	    if (pa->l_reg)
		pa->l_reg->asavail = COMPANION_IS_ALLOCATED;
	    pa->assigned_regal = ph;
	    if(ph->rglscl == AUTO)
		assigned_auto_bytes += ph->rgllen;
	    break;
	}
    } /* for( ivar = */

    /* Now, make sure all scratch registers have been used,
       i.e., if we allocated esi or edi, we may be wasting
       scratch registers. */
    for(pa = asreg; pa <= &asreg[2]; ++pa) {
	struct assign * p;
	int i;
	/* relies on scratch registers being listed first in asreg */
	if(pa->asavail == AVAIL) { /* unassigned scratch */
	    for(i=3; i <= 4; ++i) { /* %edi, %esi */
		p= &asreg[i];
		if( p->asavail == ALLOCATED ) {
		    if(p->assigned_regal->rgl_scratch_use == unk_scratch)
		        p->assigned_regal->rgl_scratch_use =
			    raregok(p->assigned_regal);
		    if(p->assigned_regal->rgl_scratch_use == ok_scratch) break;
		}
	    }
	    if(i < 5) { /* found a regal to prempt */
		pa->asavail = ALLOCATED;
		pa->assigned_regal = p->assigned_regal;
		p->assigned_regal = NULL;
		p->asavail = AVAIL; /* not that we'll ever use */
	    }
	}
    } /* for(pa = asreg */
    return assigned_auto_bytes;
}

/* check if the variable is used in any special instructions, like */
/* non-intsize move cases or floating point instructions */
	static enum valid_types
rackinstr( name )
register char *name;	/* pointer to register or variable name */
{
	extern boolean isfp(), is_byte_instr();
	register NODE *pn;
	int name_length = (int)strlen(name);
	enum  valid_types r = ALL_TYPES;
	char *s;

	for( ALLN( pn ) ) { /* scan nodes */
	    boolean isfloat=0;
	    if (is_byte_instr(pn) || (isfloat = isfp(pn)) ) {
		int iop;
		
		for( iop = 1; iop < MAXOPS + 1; iop++ ) {
		   if((s=pn->ops[iop])==NULL) 
			break;
		   if(strncmp(s,name,name_length) == 0 )
			switch( *(pn->ops[iop] + name_length)) {
			case '\0': case '+': case '-':
			    if(isfloat) 
				return(NO_TYPES);
			    else
				r = ONLY_BYTE;
			default:
			    continue;
			}
		}
	    }
	}
	return( r );
}

/* Check if ok to put variable into a scratch reg, by checking if */
/* variable is live after any calls. */
	static enum scratch_status
raregok(rgl)
struct regal * rgl;
{
	register NODE *pn;
	char * name = rgl->rglname;
	int len;
	int global = isglobal(name);
	if(global) len = (int)strlen(name);
	else len = rgl->rgllen;

	for( ALLN( pn ) ) { /* scan nodes */
		switch( pn->op ) {
		default: continue;
		case CALL:
		case LCALL:
			/* check if 'name' is live */
			if(rald( pn->forw, name, len, global ))
			    return(no_scratch); /* it's live */
		}
	}
	/* dead */
	return(ok_scratch);
}

static int rausesglobal();
static boolean rausesoff();

/* check for variable dead in a block */
/* Note that this routine may visit a block more than
 * once, but that it cannot loop because any closed path
 * in the program contains at least one label, and the
 * routine will not scan a block beginning with a given
 * label more than once, even on subsequent calls, given the same
 * variable name. 
 */
	static boolean
rald( pn, name, len, global )
register NODE *pn;	/* pointer to block to be searched */
register char *name;	/* pointer to register or variable name */
register int len;	/* length of variable (0 for register)
			   looks fishy (psp) */
int global;		/* if true, len is strlen(name) */
{
	register int iop;
	register char *p;
	int srcsize, dstsize;
	/* scan block */
	for( ; pn != 0; pn = pn->forw ) {
		/*control recursion */
		if( islabel( pn ) ) {		/* beginning of new block */
			/* terminate recursion if this block has been visited */
			if( pn->opm == name ) return( false );
			/* mark block as visited */
			pn->opm = name;
			continue;
		}
		/* if no references, its dead */
		if( isret( pn ) ) return( false );
		if( isbr( pn ) ) break;		/* end of block */

		/* check operands of remaining instructions */
		for( iop = 1; iop < MAXOPS + 1; iop++ ) {
			p = pn->ops[iop];
			if( p == NULL ) continue;
			/* look for variable */
			if(global) {
			    if(rausesglobal(p,name,len)) return true;
			}
			else if( rausesoff( p, name, len ) ) {
				/* see if it's a destination */
				if( iop==2 && ( (int)strlength(name) == (int)strlength(p) )
					&& ismove( pn, &srcsize, &dstsize) )
					return( false );
				/* nope, it's live */
				return( true );
			}
		}
	}
	if (pn == NULL)
	  fatal("rald: fell off end of function\n");

	/* examine next block(s) recursively */
	if( pn->opm == NULL ) return( true ); /* dst not a simple label */
	
	if( rald( (NODE *) pn->opm, name, len, global) ) return true;
	if( isuncbr( pn ) ) return false;
	return rald( pn->forw, name, len, global );  /* cond. branch - check fall thru case */
}

/* Does oper use the global name, e.g., oper == X+5, name == X */
	static int
rausesglobal(oper,name,len)
char *oper, *name;
int len; /* strlen(name) */
{
	return(strncmp(oper,name,len) == 0 && null_plus_or_minus(oper[len]));
}

/* remove branch information from live/dead analysis */
	static void
raremld()
{
	register NODE *pn;

	for( pn = n0.forw; pn != 0; pn = pn->forw ) {
		if( isbr( pn ) || islabel( pn ) ) pn->opm = NULL;
	}
}

static char *get_reg();
/* modify the operands */
	static void
raopn()
{
    register char *q;
    NODE *pn;
    int i, j;
    
    for( ALLN( pn ) ) {

	boolean iscall = (pn->op == CALL || pn->op == LCALL);
	for( j = 1; j < MAXOPS + 1; j++ ) { /* scan operands */
	    if( ( q = pn->ops[j] ) == NULL ) continue;
	    /* check for address arithmetic */
	    if( pn->op == LEAL && j == 1 ) continue;

	    if ( scanreg(q, true) & EBP ) {
	        /* uses frame pointer */
	        for( i = 0; i < MAXREGS; i++ ) {
		    if (asreg[i].asavail != ALLOCATED ) continue;
		    if (strcmp(asreg[i].assigned_regal->rglname,q) == 0) {
			pn->ops[j] = get_reg(&asreg[i],pn->op);
			break;
		    }
		    if ( iscall && *q == '*' && 
		      strcmp(asreg[i].assigned_regal->rglname,&q[1]) == 0 ) {
			pn->ops[j] = getspace(NONSYMADDRSZ);
			strcpy(pn->ops[j],"*");
			strcat(pn->ops[j],asreg[i].asrname
			    [2-(asreg[i].assigned_regal->rgllen>>1)]);
		    }
		} /* for i */
	    }

	    else if (isglobal(q) && strpbrk(q,"*()%") == NULL) {
	        char *p, *ptemp;
	        int temp;
	        for (p=q; *p != '\0'; p++)
		if( *p == '+' || *p == '-' ) break;
	        for( i = 0; i < CHAR_REGS; i++ ) {
		    /* Don't have to check split regs */
		    if (asreg[i].asavail != ALLOCATED ) continue;

		    /* change + or - to \0 for strcmp */
		    temp = *p;
		    *p = '\0';

		    if( strcmp(q,asreg[i].assigned_regal->rglname) == 0) {
			if(temp == '+') ++p; /* point at (pos) increment */
		        else *p = (char) temp; /* point at - or digit */
		        temp = (int)strlen(p) + (iscall ? 6:7);
		        /* 7 == null plus reg name + two parentheses */
		        ptemp = pn->ops[j] = getspace((unsigned)temp);
		        strcpy(ptemp,p);
		        if(iscall) strcat(ptemp,"*");
		        else strcat(ptemp,"(");
		        strcat(ptemp,asreg[i].asrname[0]);
		        if(!iscall) strcat(ptemp,")");
		    }
		    else /* just restore q */ *p = (char) temp;

		} /* for i */
	    }
        } /* for( j */
    } /* for( ALLN */
}

/* uses offset addressed variable */
	static boolean
rausesoff( oper, var, len )
char* oper;	/* pointer to operand */
char* var;	/* pointer to offset addressed variable */
int len;	/* length of the variable in bytes */
{
	register int io, iv;

	if( *oper == '*' ) oper++;
	io = strtol( oper, &oper, 10 );
	iv = strtol( var, &var, 10 );
	if( *oper != '(' || *var != '(' || *(oper + 2) != *(var + 2) )
		return(false);
	if( io < iv || io > ( iv + len - 1 ) ) return(false);
	if ( *(oper + 3) != *(var +3) || *(oper + 4) != *(var +4) )
	  	return(false);
	return(true);
}

/* insert moves for parameters, globals */
	static void
raparam( )
{
	register NODE *pn;
	register int ias;
	register struct assign *pa;

	/* add the pushes and loads at beginning of subroutine */
	for (pn = n0.forw; pn != 0; pn = pn->forw) { /* scan nodes */

		/* skip over profiling code */
		if (isprof( pn->forw ))
			pn = pn->forw->forw;

		/* find the start of the subroutine code. */
		if (pn->op == POPL && usesvar("%eax",pn->op1) &&
		    pn->forw->op == XCHGL && usesvar("%eax",pn->forw->op1) &&
		    strcmp("0(%esp)",pn->forw->op2) == 0)
			pn = pn->forw->forw;
		if (pn->op != PUSHL || !usesvar("%ebp", pn->op1))
			continue;
		pn = pn->forw;
		if (pn->op != MOVL ||
		    !usesvar("%esp",pn->op1) || !usesvar("%ebp",pn->op2))
			continue;
		if (pn->forw->op == SUBL && usesvar("%esp",pn->forw->op2)) {
			pn = pn->forw;
		} else if (pn->forw->op == PUSHL &&
			   usesvar("%eax",pn->forw->op1)) {
			pn = pn->forw;
		}

		/* store any register variables away */
		for (ias = 0; ias < CHAR_REGS;  ias++) {
		    pa = &asreg[ias];
		    if (pa->asrtype != USER )
				continue;
		    if (pa->asavail == COMPILER_REGISTER) {
			/* compiler allocated register */
			pn = pn->forw;
			continue;
		    }

		    /* only possibilities:
		       AVAIL,ALLOCATED,COMPANION_ALLOCATED
		       Since we won't look at the companion
		       registers, we generate a push for the
		       COMPANION_ALLOCATED case as well.
		       E.g., push %ebx will save %bl as well. */

		    if (pa->asavail != AVAIL)
		        addi(pn, PUSHL, "pushl", pa->asrname[0], NULL);
			
		}

		/* for each global assigned, insert a load eff. addr instr,
		   insert move for each parameter assigned to reg */
		for (ias = 0; ias < MAXREGS;  ias++) {
			struct regal * prgl;
			pa = &asreg[ias];
			if(pa->asavail != ALLOCATED) continue;
			else prgl = pa->assigned_regal;
			if (prgl->rglscl == GLOBAL) {
			    addi(pn,LEAL,"leal",prgl->rglname,pa->asrname[0]);
			    continue;
			}
			if (prgl->rglscl != PARAM)
				continue;
			if (prgl->rgllen == 4)
				addi(pn, MOVL, "movl", prgl->rglname, (pa->asrname[0]))
			else if (prgl->rgllen == 2)
				addi(pn, MOVW, "movw", prgl->rglname, (pa->asrname[1]))
			else 
				addi(pn, MOVB, "movb", prgl->rglname, (pa->asrname[2]))
			
		}
		break;
	}

	/* add the pops at end of subroutine */
	for (pn = n0.forw; pn != 0; pn = pn->forw) { /* scan nodes */
		/* find the end of the subroutine code. */
		if (pn->op != LEAVE)
			continue;

		pn = pn->back;		/* found "leave", back up one */

		if (is_debug_label(pn))/* Skip .def-generated hard label and filter */
			pn = pn->back->back;	/* left for debugging */
		/* reload any register variables, in reverse */
		/* order of the stores */
		for (ias = 0; ias < CHAR_REGS;  ias++) {
			pa = &asreg[ias];
			if (pa->asrtype != USER)
				continue;
			if (pa->asavail == COMPILER_REGISTER) {
			  	if (pn->op != POPL
				    || strcmp(pn->op1,pa->asrname[0]) != 0)
				  fatal("raparam: can't find user register restores\n");
				pn = pn->back;
				continue;
			}
			if (pa->asavail == AVAIL)
				continue;
			addi(pn, POPL, "popl", pa->asrname[0], NULL);
			pn = pn->back;
		}
		break;
	}
}

	int
ra_assigned_to(scl,regid,disp) /* return register to which this var has
			been assigned (0 if none ).  Code is based on
			old radef() which also modified debugging info.
			The old code never broke out of the for loop:
			dumb linear search or was there a reason? */

int scl; /* AUTO, PARAM, or REGISTER ( GLOBAL is irrelevant ) */
int regid;
int disp; /* displacement for AUTO or PARAM */
{
	
    int ias;
    struct assign *pa;

    for( ias = 0; ias < MAXREGS; ias++ ) {
	/* look for a register assigned to this variable:
	   1) same storage class
	   2)   if it was originally on the stack, check the displacement
		otherwise check the original register assignment. */
	pa = &asreg[ias];
	if((pa->asavail) != ALLOCATED ) continue;
	if( scl != pa->assigned_regal->rglscl ) continue;

	switch (scl) {
	case AUTO:
	case PARAM:
	    if( disp == atoi(pa->assigned_regal->rglname) ) return pa->asrregn;
	    break;
	case REGISTER:
	    if ( regid == pa->asrregn) return regid;
	    break;
	}

	/* The struct assign data structure does not appear to
	   allow for the possibility that the optimizer might
	   move a ( user ) register variable to another register
	   other than that originally assigned by the compiler,
	   or into the stack for that  matter.  HALO is more
	   general. (psp) */
    }
    return 0; /* didn't find it */
}

	static char *
get_reg(assign,op)
struct assign *assign; unsigned short op;
{
	int op_leng;

	/* Check for instructions requiring 1 or 2 byte operands */
	switch (op) {
	case ADCB: case ADDB: case DIVB: case IDIVB: case MULB: 
	case SUBB: case SBBB: case ANDB: case ORB: case XORB:
	case CMPB: case TESTB: case MOVB: case IMULB:
	case MOVSBL: case MOVSBW: case MOVZBW: case MOVZBL:
		op_leng = 1;
		break;
	case ADCW: case ADDW: case DIVW: case IDIVW: case MULW: 
	case SUBW: case SBBW: case ANDW: case ORW: case XORW:
	case CMPW: case TESTW: case MOVW: case IMULW:
	case MOVSWL: case MOVZWL:
		op_leng = 2;
		break;
	default:
		op_leng = assign->assigned_regal->rgllen;
	}

	return assign->asrname [2 - (op_leng>>1)];
}
