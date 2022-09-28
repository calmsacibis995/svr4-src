/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)acomp:common/p1allo.c	1.4"
/* p1allo.c */

/* Pass 1 offset/register allocation routines. */

#include "p1.h"
#include "mfile2.h"
#include <memory.h>

int a1debug = 0;		/* debug flag to show allocation */

typedef char reg_t;		/* type for register arrays; really
				** should be unsigned char
				*/

static reg_t al_regsused[TOTREGS];	/* entry i is 1 if i-th register was
					** used somewhere in current function
					*/
static OFFSET al_maxauto;	/* maximum allocated offset */

#ifdef FAT_ACOMP

/* Allocation takes place all at one time, at the end of a function:
**	1) al_param()/al_auto() record symbol ID's that need allocation.
**	2) An allocation structure gets built that describes the block
**		structure of the program.
**	3) Registers get allocated.  There are three styles:
**		RA_GLOBAL	global allocation
**		RA_OLDSTYLE	old-style:  try to honor "register"
**		RA_NONE		no allocation
**
**	4) Stack offsets get allocated for any remaining objects.
**
** The primary data structures are the object list and the
** block list.
*/

#ifdef NODBG
#define	AL_PR_OBJ()
#define	AL_PR_OFFBLK()
#define AL_PR_ORDER()
#else
#define	AL_PR_OBJ() al_pr_obj()
#define	AL_PR_OFFBLK() al_pr_offblk()
#define AL_PR_ORDER() al_pr_order()
static void al_pr_obj(), al_pr_offblk(), al_pr_order();
#endif

static OFFSET al_offsets(), al_off();
static void al_regset();
static void al_oldregs(), al_regtry();
static void al_regs();
static void al_regsort(), al_regassign(), al_regexploit();
static void al_regmark();

/* Structure to represent each objects, and its weight. */
typedef struct {
    SX obj_sid;			/* SID of object */
    int obj_weight;		/* object's weight */
    int obj_block;		/* object's owning block */
} obj_t;

/* Structure to represent a lexical block. */
typedef struct {
    BITOFF blk_curoff;		/* current offset, this block */
    BITOFF blk_maxoff;		/* maximum offset, this block */
    reg_t blk_allregs[TOTREGS];	/* set of registers active in current block,
				** children, and parents
				*/
    reg_t blk_regs_inscope[TOTREGS];
				/* set of registers active in this block
				** and its parents:  for callER save
				*/
    int blk_parent;		/* index of block's parent */
    int blk_sibling;		/* index of block's sibling */
    int blk_child;		/* index of block's child block */
    int blk_lastchild;		/* index of block's last child block */
    int blk_first;		/* index (in obj_list) of first symbol in
				** block
				*/
    int blk_last;		/* index of last symbol in block */
    int blk_calls;		/* number of function calls in block */
} blk_t;

/* Set up object list. */
#ifndef	INI_OBJLIST
#define	INI_OBJLIST	20
#endif

TABLE(Static, td_objlist, static, objlist_init, obj_t, INI_OBJLIST, 0,
		"object list");

#define OBJ(i) (TD_ELEM(td_objlist, obj_t, (i)))
#define	CUR_OBJ (TD_USED(td_objlist))

/* Set up offset structure. */
#ifndef	INI_BLOCK_LIST
#define	INI_BLOCK_LIST 2
#endif

TABLE(Static, td_blklist, static, blklist_init, blk_t, INI_BLOCK_LIST, 0,
		"block list");

#define BLK(i) (TD_ELEM(td_blklist, blk_t, (i)))
#define	CUR_BLK TD_USED(td_blklist)

static int al_curblock;		/* index of current block */
static int al_saw_setjmp;	/* setjmp() seen in -Xt mode:  suppress
				** global register allocation, use old-style
				*/

static OFFSET al_maxtemp;	/* maximum offset due to expressions */
static int * al_order;		/* ordered list of object indices by weight */
int al_regallo = RA_DEFAULT;	/* selected register allocation style */

void
al_auto(sid)
SX sid;
/* Add sid for automatic (or parameter) to object list, update current offset
** structure.
*/
{
    ++CUR_OBJ;
    TD_NEED1(td_objlist);
    OBJ(CUR_OBJ).obj_sid = sid;
    OBJ(CUR_OBJ).obj_block = al_curblock;
    /* Weight, predecessor, successor values get filled in
    ** at allocating time.
    */
    BLK(al_curblock).blk_last = CUR_OBJ;	/* last object in block */
    TD_CHKMAX(td_objlist);
    return;
}

reg_t *
al_g_regs()
/* Return set of registers in scope in current block. */
{
    return ( BLK(al_curblock).blk_regs_inscope );
}


OFFSET
al_g_maxtemp()
/* Return maximum temp offset in function. */
{
    return ( al_maxtemp );
}


void
al_begf()
/* Start allocations for a function.  Actually, offsets get initialized
** when we first start considering parameters.
*/
{
    al_saw_setjmp = 0;			/* no setjmp() seen, this function */
    return;
}


void
al_s_block()
/* Start a new lexical block. */
{
    blk_t * op;
    static const blk_t zeroblock;

    ++CUR_BLK;
    TD_NEED1(td_blklist);
    TD_CHKMAX(td_blklist);

    op = &BLK(CUR_BLK);

    *op = zeroblock;			/* clean out everything */
    op->blk_first = CUR_OBJ+1;		/* next one to be allocated */
    op->blk_parent = al_curblock;
    if (al_curblock != 0) {		/* there is a parent */
	blk_t * pp = &BLK(al_curblock);

	if (pp->blk_child == 0)
	    pp->blk_child = CUR_BLK;	/* first child */
	else
	    BLK(pp->blk_lastchild).blk_sibling = CUR_BLK;
	pp->blk_lastchild = CUR_BLK;
    }

    al_curblock = CUR_BLK;
    return;
}


void
al_e_block()
/* End a lexical block.  Restore the "current block" to be
** our parent.
*/
{
    al_curblock = BLK(al_curblock).blk_parent;
    return;
}
    


OFFSET
al_endf()
/* Handle stuff at end of function.  Assign registers and
** offsets to everything that needs one.
*/
{
    OFFSET max;

    al_e_block();			/* closes block with parameters */

    max = al_offsets();
#ifndef NODBG
    if (a1debug) {
	AL_PR_OBJ();			/* print object list */
	if (al_regallo == RA_GLOBAL)
	    AL_PR_ORDER();		/* print object weight order */
	AL_PR_OFFBLK();			/* print offset list */
    }
#endif
    return( max );
}

void
al_call(p)
ND1 * p;
/* Note a function call to the function designated by p.
** In particular, keep track of the number of functions
** called in a block, and whether setjmp() was called
** (-Xt only).  The incoming tree is the CALL tree,
** unoptimized.
*/
{
    ++(BLK(al_curblock).blk_calls);

    if (version & V_CI4_1) {
	static char * setjmpname;
	ND1 * l = p->left;

	if (setjmpname == (char *) 0)
	    setjmpname = st_nlookup("setjmp", sizeof("setjmp"));
	
	if (   l->op == UNARY AND
	    && (l = l->left)->op == NAME
	    && SY_NAME(l->rval) == setjmpname
	    && TY_ISFTN(l->type)
	)
	    al_saw_setjmp = 1;
    }
    return;
}


static OFFSET
al_offsets()
/* Allocate offsets or registers for all automatics, parameters.
** Return high-water mark for autos.
*/
{
    off_init(VAUTO);			/* start off allocation */
    al_maxauto = max_temp();
    if (CUR_OBJ != 0) {			/* we have at least one object */
	/* Choose desired style of register allocation.  Don't do
	** global allocation in -Xt mode if setjmp() call seen.
	*/
	switch( al_regallo ) {
	case RA_GLOBAL:
	    if (! al_saw_setjmp) {
		al_regs();
		break;
	    }
	    /*FALLTHRU*/
	case RA_OLDSTYLE:	al_oldregs(); break;
	}
	al_maxauto = al_off(1, al_maxauto); /* allocate auto offsets */
    }
    al_maxtemp = al_maxauto;
    return( al_maxauto );
}


static void
al_regtry(obj)
int obj;
/* Try to put object obj in a register, return register number or
** -1.  Update all associated structures if successful. 
*/
{
    obj_t * obp = &OBJ(obj);
    SX sid = obp->obj_sid;
    blk_t * op = &BLK(obp->obj_block);
    TWORD cgtype = cg_tconv(SY_TYPE(sid), 0);
    int regno = -1;

    switch ( SY_CLASS(sid) ){
    case SC_AUTO:
	regno = cisreg(VAUTO, cgtype, (OFFSET) 0, op->blk_allregs);
	break;
    case SC_PARAM:
	regno = cisreg(VPARAM, cgtype, (OFFSET) SY_OFFSET(sid), op->blk_allregs);
	break;
    }

    if (regno >= 0) {			/* successful allocation */
	int parent;
	int nregs = tyreg(cgtype);

	/* We allocated a register. */
	SY_REGNO(sid) = regno;
	if (SY_CLASS(sid) == SC_AUTO)
	    SY_OFFSET(sid) = 0;	/* sanitation */
	/* Mark registers busy in child blocks, parent blocks. */
	al_regmark(op->blk_child, regno, nregs);
	al_regupdate(regno, nregs);	/* mark globally used */
	al_regset(op->blk_regs_inscope, regno, nregs);
	while ((parent = op->blk_parent) != 0) {
	    op = &BLK(parent);
	    al_regset(op->blk_allregs, regno, nregs);
	}
    }
    return;
}


static void
al_oldregs()
/* Simple-minded register allocation:  believe user's requests. */
{
    int i;

    /* Walk the object list in lexical order. */
    for (i = 1; i <= CUR_OBJ; ++i) {
	SX sid = OBJ(i).obj_sid;
	T1WORD t = SY_TYPE(sid);

	if ((SY_FLAGS(sid) & SY_ISREG) == 0)
	    continue;
	if (SY_FLAGS(sid) & SY_UAND)
	    continue;
	if (TY_ISARY(t) || TY_ISVOLATILE(t))
	    continue;
	
	al_regtry(i);
    }
    return;
}


static void
al_regs()
/* Attempt to allocate registers by weight.  The process goes
** in several steps:
**	1) Sort object array by weight, using predecessor and
**		successor indices to maintain linkage.
**	2) Starting with highest weight, try to assign
**		registers by weight for objects above threshold.
**	3) Try to make opportunistic use of registers where
**		a register is used in one parallel block and not
**		in another:  use register even if not above
**		threshold.
**
** There's an assumption here that bindparam() can be called in
** an arbitrary order, and that the offset in the parameter's
** symbol table entry is enough information to bindparam() to
** identify each parameter.
*/
{
    al_regsort();
    al_regassign();
    al_regexploit();
    return;
}


static int
al_regcompare(left,right)
const myVOID * left;
const myVOID * right;
/* Comparison for quick-sort.  We actually want
** a reversed sense, because we want highest first.
** Also, give preference to lower numbered objects
** with equal weights.  This favors earlier declarations
** for objects of the same weight.
*/
{
    obj_t * lobj = &OBJ(* (int *) left);
    obj_t * robj = &OBJ(* (int *) right);

    if (lobj->obj_weight == robj->obj_weight)
	return ( lobj - robj );
    else if (robj->obj_weight > lobj->obj_weight)
	return 1;
    else
	return -1;
}

    
static void
al_regsort()
/* Sort objects by weight, setting predecessor and successor
** values, as well as the weight field for register allocation.
*/
{
    int i;

    /* Fill in weights for objects in object list. */

    for (i = 1; i <= CUR_OBJ; ++i) {
	obj_t * obp = &OBJ(i);
	SX sid = obp->obj_sid;
	obp->obj_weight = SY_WEIGHT(sid);
	/* Only interested in scalars, non-volatile, non-address taken. */
	if (   (SY_FLAGS(sid) & SY_UAND) != 0
	    || ! TY_ISSCALAR(SY_TYPE(sid))
	    || TY_ISVOLATILE(SY_TYPE(sid))
	    )
	    obp->obj_weight = 0;
    }
    
    /* Prepare ordered list of objects. */

    if (al_order)
	free(al_order);
    if ((al_order = (int *) malloc((CUR_OBJ+1) * sizeof(int))) == NULL)
	cerror("al_regsort(): out of memory");

    /* The al_order array contains object indices.  Initially they
    ** are unsorted; then they are sorted by weight.
    */
    for (i = 0; i < CUR_OBJ; ++i)
	al_order[i] = i+1;
    al_order[CUR_OBJ] = 0;		/* sentinel */
    qsort((myVOID *) al_order, (unsigned int) CUR_OBJ, sizeof(al_order[0]),
		al_regcompare);

    return;
}

static void
al_regset(reglist, regno, nregs)
reg_t reglist[];
int regno;
int nregs;
/* Update list of registers used after a register is allocated.
** regno is the register that was allocated, nregs is the number
** of registers the object takes, reglist is the list to update.
*/
{
    int i;

    for (i = 0; i < nregs; ++i)
	reglist[regno+i] = 1;		/* mark them all used */
    return;
}


static void
al_regassign()
/* Assign registers to objects by highest weight. */
{
    int i;

    for (i = 0; i < CUR_OBJ; ++i) {
	int obj = al_order[i];

	if (OBJ(obj).obj_weight < SM_WT_THRESHOLD)
	    break;			/* remaining are below threshold */
	/* Try to allocate for this object. */
	al_regtry(obj);
    }
    return;
}

static void
al_regfreeze(bn)
int bn;
/* Make registers that are free in the parent block and
** throughout the function appear to be busy so they
** won't be allocated.
*/
{
    if (bn != 0) {
	blk_t * op = &BLK(bn);
	blk_t * pp = &BLK(op->blk_parent);
	int i;

	/* Do all children, then all siblings. */
	al_regfreeze(op->blk_child);
	al_regfreeze(op->blk_sibling);

	/* Now mark current block. */
	for (i = 0; i < TOTREGS; ++i)
	    if (pp->blk_allregs[i] == 0 && al_regsused[i] == 0)
		op->blk_allregs[i] = 1;
    }
    return;
}


static void
al_regexploit()
/* Capitalize on registers that are busy in the parent (because
** of some sibling block) and free in the child.  Try to allocate
** objects in the child to such free registers (we presume
** that incurs no cost, namely that there's a callEE save
** convention) regardless of weights.
*/
{
    int i;

    al_regfreeze(1);

    for (i = 0; i < CUR_OBJ; ++i) {
	obj_t * obp = &OBJ(al_order[i]);
	SX sid = obp->obj_sid;

	if (obp->obj_weight <= 0)
	    break;			/* don't do such objects */

	if (SY_REGNO(sid) != SY_NOREG)
	    continue;			/* already allocated */
	
	if (SY_CLASS(sid) != SC_AUTO)
	    continue;			/* worry about others later */

	al_regtry(al_order[i]);
    }
    return;
}


static void
al_regmark(bn, regno, nregs)
int bn;
int regno;
int nregs;
/* For block number bn, mark register regno (and the
** next nregs-1 registers) busy.  Propagate that information
** to all child blocks.
*/
{
    while (bn != 0) {
	blk_t * op = &BLK(bn);

	al_regset(op->blk_allregs, regno, nregs);
	al_regset(op->blk_regs_inscope, regno, nregs);
	al_regmark(op->blk_child, regno, nregs);
	bn = op->blk_sibling;
    }
    return;
}


void
al_regupdate(regno, nregs)
/* Update global register list after a register is allocated.
** regno is the register that was allocated, nregs is the
** number of registers used.
*/
int regno;
int nregs;
{
    al_regset(al_regsused, regno, nregs);
    return;
}


static OFFSET
al_off(blk, off)
int blk;
OFFSET off;
/* Allocate offsets for block blk (and its children), starting
** with offset off.  Return the high-water mark reached.
*/
{
    blk_t * op = &BLK(blk);
    OFFSET newmax = off;		/* high-water mark so far */
    int o;				/* objects in block */
    int last = op->blk_last;

    set_next_temp(off);

    /* Allocate offsets for autos in current block. */
    for (o = op->blk_first; o <= last; ++o) {
	SX sid = OBJ(o).obj_sid;

	if (SY_CLASS(sid) != SC_AUTO)
	    continue;

	if (SY_REGNO(sid) == SY_NOREG) {
	    T1WORD t = SY_TYPE(sid);
	    int align = TY_ALIGN(t);
	    BITOFF tsz = TY_SIZE(t);
	    OFFSET newoff = next_temp(cg_tconv(t,0), tsz, align);

	    SY_OFFSET(sid) = newoff;
	    newmax = off_bigger(VAUTO, newmax, max_temp());
	}
    }

    /* Now allocate offsets for child blocks. */
    if (op->blk_child)
	newmax = al_off(op->blk_child, newmax);
    
    /* Allocate offsets for sibling block. */
    if (op->blk_sibling) {
	OFFSET newoff = al_off(op->blk_sibling, off);
	newmax = off_bigger(VAUTO, newoff, newmax);
    }
    return( newmax );
}


void
al_s_param()
/* Begin allocation of parameter offsets. */
{
    off_init(VPARAM);

    /* Zero in each array is reserved for "none".  These
    ** values are pre-incremented.
    */
    CUR_OBJ = 0;
    CUR_BLK = 0;
    al_curblock = 0;			/* at outer-most block */

    memset(al_regsused, 0, TOTREGS);	/* no registers allocated yet */
    al_s_block();			/* equivalent to a block start:
					** create block to hold parameters
					*/

    return;
}


#ifndef NODBG

static void
al_pr_obj()
/* Print objects list. */
{
    int i;

    printf("%s Objects\n%s =======\n", COMMENTSTR, COMMENTSTR);
    for (i = 1; i <= CUR_OBJ; ++i) {
	obj_t * obp = &OBJ(i);
	SX sid = obp->obj_sid;
	printf("%s %3d	[%3d]	\"%s\"\t# %3d (%3d)\t",
			COMMENTSTR, i, sid, SY_NAME(sid),
			SY_WEIGHT(sid), obp->obj_weight);
	if (SY_REGNO(sid) == SY_NOREG)
	    printf("%d\n", SY_OFFSET(sid));
	else {
	    printf("%%%d%s\n", SY_REGNO(sid),
		OBJ(i).obj_weight < SM_WT_THRESHOLD ? " +" : "");
	}
    }
    return;
}

static void
al_pr_offblk()
/* Print block list */
{
    int i;

    printf("%s Blocks\n%s ======\n", COMMENTSTR, COMMENTSTR);
    for (i = 1; i <= CUR_BLK; ++i) {
	printf("%s [%3d] par=%3d, chd=%3d, sib=%3d, fst=%3d, lst=%3d\n",
		COMMENTSTR, i, BLK(i).blk_parent, BLK(i).blk_child,
		BLK(i).blk_sibling, BLK(i).blk_first, BLK(i).blk_last);
    }
    return;
}


static void
al_pr_order()
/* Print object ordering. */
{
    int i;

#define PERLINE 8
    printf("%s Object order\n%s ============\n%s", COMMENTSTR, COMMENTSTR,
		COMMENTSTR);
    for (i = 0; i < CUR_OBJ; ++i) {
	if (i % PERLINE == 0 && i != 0)
	    printf("\n%s", COMMENTSTR);
	printf(" %3d", al_order[i]);
    }
    printf("\n");
    return;
}
 
#endif	/* ndef NODBG */

#else	/* !FAT_ACOMP */

/* Pass 1 statement-at-a-time offset/register allocation routines. */


/* Maintain a stack that contains current allocation information.
** Stack frame[i] contains the high-water mark for autos and the
** set of allocated registers for block[i].
** Assume that block 0 is always zero:  no registers allocated, no
** automatics.  This reduces the cost of function start.
*/
#ifndef	INI_ALLOSTK
#define	INI_ALLOSTK 20			/* initial size */
#endif

struct allostk {
    OFFSET al_curauto;			/* max. auto offset in current block */
    reg_t al_regs[TOTREGS];		/* register allocations in current block */
};

#define ty_allostk struct allostk
TABLE(Static, allostk, static, allostk_init,
		ty_allostk, INI_ALLOSTK, 0, "allocation stack");
#define AA(i) (TD_ELEM(allostk, ty_allostk, (i)))

#define al_sp TD_USED(allostk)	/* stack pointer for allocation operations */



/* Memory allocation routines.
** We keep track of the maximum auto offset particularly, since
** it must be passed in the ENDF node.
*/

void
al_begf()
/* Set offsets at beginning of function.  Must be called before
** cg_begf().
*/
{
    int i;

    /* al_maxauto, AA(0).al_curauto set in cg_begf(). */
    off_init(VAUTO);			/* set to beginning of autos */
    al_sp = 0;				/* current allocation stack frame */

    /* No register variables have been used yet.  Reset first allocation
    ** stack frame and maximal list of registers.
    */
    for (i = 0; i < TOTREGS; ++i) {
	AA(0).al_regs[i] = 0;
	al_regsused[i] = 0;
    }
    return;
}



OFFSET
al_endf()
/* Return maximum extent of automatics used in function. */
{
    return( al_maxauto );
}


void
al_auto(sid)
SX sid;
/* Do allocation for automatic.  If it's declared to be
** a register variable, attempt to allocate same.
*/
{
    T1WORD t = SY_TYPE(sid);
    int regno;

    if (   (SY_FLAGS(sid) & SY_ISREG) != 0
	&& (regno = al_reg(VAUTO, (OFFSET) 0, t)) >= 0
    ) {
	SY_OFFSET(sid) = 0;
	SY_REGNO(sid) = regno;
    }
    else {
	int align = TY_ALIGN(t);
	BITOFF tsz = TY_SIZE(t);
	OFFSET newoff = next_temp(cg_tconv(t,0), tsz, align);
	OFFSET newend = max_temp();

	al_maxauto = off_bigger(VAUTO, al_maxauto, newend);
	AA(al_sp).al_curauto = newend;	 /* current end of automatics */
	
	SY_OFFSET(sid) = newoff;
    }
    return;
}

    
void
al_s_block()
/* Remember current allocation information on allocation stack.
** The current auto offset is remembered on the fly.
** Register information is propagated to the next block.
*/
{
    /* Register information is in AA(al_sp).al_regs. */

    al_sp++;
    TD_NEED1(allostk);			/* make sure entry exists */
    TD_CHKMAX(allostk);			/* keep statistics */
    AA(al_sp) = AA(al_sp-1);		/* Copy previous top of stack to
					** get currently active registers
					*/
    return;
}


void
al_e_block()
/* Restore allocation information to its previous state.  Also, remember
** high-water mark for auto offsets.
*/
{
    al_maxauto = off_bigger(VAUTO, al_maxauto, max_temp());

    al_sp--;				/* Back up allocation stack. */
    if (al_sp < 0)
	cerror("confused al_e_block()");

    /* Register information for allocation restored implicitly.
    ** Tell CG what current auto offset is.
    */
    set_next_temp(AA(al_sp).al_curauto);
    return;
}


void
al_s_param()
/* Begin allocation of parameter offsets. */
{
    off_init(VPARAM);
    return;
}


reg_t *
al_g_regs()
/* Return current register allocation array. */
{
    return( AA(al_sp).al_regs );
}


int
al_reg(op, off, t)
int op;
OFFSET off;
T1WORD t;
/* Attempt to allocate a register variable for an object of type t.
** op tells whether it's an AUTO or parameter; off is the expected
** offset.
** Maintain al_regsused to remember what registers have been used.
** Return the register number, on success, or -1.
*/
{
    int regno;
    TWORD cgtype = cg_tconv(t, 0);
    
    /* cisreg() updates pointed-at array if register is allocated.
    ** Arrays and volatile objects cannot be put in registers.
    */
    if (TY_ISARY(t) || TY_ISVOLATILE(t))
	regno = -1;
    else {
	regno = cisreg(op, cgtype, off, AA(al_sp).al_regs);

	/* Keep track of greatest number of registers used. */
	if (regno >= 0)
	    al_regupdate(regno, tyreg(cgtype));
    }
    return( regno );
}


void
al_regupdate(regno, nregs)
int regno;
int nregs;
/* Update list of all registers used after a register is allocated.
** regno is the register that was allocated, nregs is the number of
** registers used.
*/
{
    int i;

    for (i = 0; i < nregs; ++i)
	al_regsused[regno+i] = 1;	/* mark them all used */
    return;
}

#endif	/* def FAT_ACOMP */


/* The following allocation routines are common to statement-
** at-a-time and function-at-a-time compilation.
*/

void
al_param(sid)
SX sid;
/* Allocate parameter offset for parameter sid, update current
** offset.  Rely on next_arg() to handle all this.
*/
{
    T1WORD t = (SY_FLAGS(sid) & SY_ISDOUBLE) ? TY_DOUBLE : SY_TYPE(sid);
    BITOFF tsz = TY_SIZE(t);
    int align = TY_ALIGN(t);
    OFFSET offset;

    offset = next_arg(cg_tconv(t, 0), tsz, align);

    /* In some error cases, the storage class is not PARAM;
    ** don't clobber existing offset (such as typedef, whose
    ** bits are special type bits).  Nevertheless, we must allocate
    ** an offset for it.
    */
    if (SY_CLASS(sid) == SC_PARAM)
	SY_OFFSET(sid) = offset;

#ifdef FAT_ACOMP
    al_auto(sid);		/* Record parameter's sid */
#endif	/* def FAT_ACOMP */

    SY_REGNO(sid) = SY_NOREG;	/* No register assigned yet. */
    return;
}

void
al_e_param()
/* Mark end of parameter offset assignment. */
{
    /* Don't do anything. */
    return;
}

OFFSET
al_g_param()
/* Return current parameter offset.  The assumption here is that
** this offset is for an argument that would be subjected to the
** default argument promotions, so it is int/double/pointer/struct.
** This is a bit painful:  do so by getting offset for a int,
** then resetting the value.
*/
{
    OFFSET retval = next_arg(T2_INT, TY_SIZE(TY_INT), ALINT);

    set_next_arg(retval);
    return( retval );
}


reg_t *
al_g_maxreg()
/* Get register allocation array for entire function. */
{
    return( al_regsused );
}


void
al_s_fcode()
/* Set up allocation at start of function.  For FAT_ACOMP,
** get temp high-water mark, in case it changed because of
** the return-type (e.g., structure).  For statement-at-a
** time, initialize current (first) block likewise.
*/
{
    al_maxauto = max_temp();
#ifdef FAT_ACOMP
    al_maxtemp = al_maxauto;
#else
    AA(al_sp).al_curauto = al_maxauto;
#endif
    return;
}


void
al_e_expr()
/* Reset temp high-water mark at the end of an expression to keep
** CG from advancing with each new TEMP.  For statement-at-a-time,
** look for new al_maxauto, and reset to current block's end.
** For function-at-a-time, look for new al_maxtemp, reset to end
** of autos.
*/
{
#ifdef FAT_ACOMP
    al_maxtemp = off_bigger(VAUTO, al_maxtemp, max_temp());
    set_next_temp(al_maxauto);
#else
    al_maxauto = off_bigger(VAUTO, al_maxauto, max_temp());
    set_next_temp(AA(al_sp).al_curauto);
#endif
    return;
}
