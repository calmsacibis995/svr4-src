/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)alint:common/tables2.c	1.10.1.4"
#include "p1.h"
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <malloc.h>
#include "lnstuff.h"
#include "lpass2.h"
#include "tables2.h"

static int samety();
static void makeuneq();
static void makeeq();
static void push();
static void pop();
static int onstack();
static int eq_s();
static char *lst_save();
static char *lst_nlookup();
static int samename();

static STAB	stab[NSZ];
static ENT	*lsu_table[STRSIZ];
STYPE	*tary;
STYPE	tarychunk0[TYSZ];	/* initial chunk for tary	*/
ATYPE	(* atyp)[];
char	*(* strp)[];

typedef struct {
	T1WORD t1, t2;
} STACK;
static	STACK *stack;

int	args_alloc = 0;		/* no arguments allocated yet	*/
static	int stackptr = 0;
static	int maxstack = 0;


/*
** Arg_expand will expand the tables set up for the temporary holding
** of:
**	- s/u/e members;
**	- function parameters;
**	- function arguments.
**
** The table is increased by ARGS until the table is large enough
** to hold all 'size'.
*/
void
arg_expand(size)
int size;
{    
    LNBUG(ln_dbflag > 1, ("arg_expand"));
    /*
    ** Increase the table by ARGS each time until there is enough room.
    */
    while ((args_alloc += ARGS) < size);

    atyp = (ATYPE (*)[]) realloc((char *)atyp, args_alloc * sizeof(ATYPE));
    strp = (char *(*)[]) realloc((char *)strp, args_alloc * sizeof(char *));

    if ((atyp == NULL) || (strp == NULL))
	lerror(FATAL,"can't reallocate space for more arguments");
}
    


/*
** Return the symbol table entry for the item just read in (in r).
** If it is not found, add it to the table.
*/
STAB *
find()
{
    register STAB *q, *maybe = 0;
    register int h=0;
    LNBUG(ln_dbflag > 2,("find"));

    /*
    ** hashing scheme - for this to work, NSZ should be a power of 2
    */
    h = ((unsigned) curname) % NSZ;
    q = &stab[h];

    /*
    ** If this entry is already filled, chain along until the end.
    */
    while (q->decflag) {
	if (curname == q->name) {	/* name matches */

	    /*
	    ** Match if module id's are the same.
	    */
	    if (q->mno == cmno)
		return (q);

	    /*
	    ** Possible match if neither static.
	    */
	    if (! ((q->decflag&LDS) || (r.l.decflag&LDS)))
		maybe = q;

	    /*
	    ** Else there must be another stab entry with same name
	    ** (if there is NOT, then we will add an entry).
	    */
	    else if (!maybe) 
		    q->more = 1;

	    /*
	    ** At end of chain - so use global name.
	    */
	    if ((!(q->more) && maybe) && (! (r.l.decflag & LSU)))
		return(maybe);
	}

	/*
	** No match, so continue looking.
	**
	** If the "next" is NULL, allocate a STAB for it, and set decflag
	** to 0 to end the while loop.
	*/
	if (q->next == NULL) {
	    q->next = (STAB *) malloc(sizeof(STAB));
	    q->next->decflag = 0;
	}
	q = q->next;
    }

    /*
    ** q points to an available symbol table entry.
    ** Not all info must be put in now ... that is done in setuse().
    */
    q->name = curname;
    q->more = 0;
    q->next = NULL;
    return(q);
}



/*
** tget - return an address to put a type into for the symbol table.
*/
STYPE *
tget()
{
    static int tfree=0;
    LNBUG(ln_dbflag > 2, ("tget"));

    /*
    ** Out of entries, get another block the same size as before.
    */
    if (tfree == TYSZ) {
	tary = (STYPE *) malloc(TYSZ * sizeof(STYPE));
	tfree=0;
    }
    return (&tary[tfree++]);
}



/*
** cleanup - call lastone and die gracefully
**
** For each entry in stab[x], call lastone on each entry along the chain.
*/
void
cleanup()
{
    STAB *q, *p;
    LNBUG(ln_dbflag > 1, ("cleanup"));

    for (q=stab; q <= &stab[NSZ-1]; ++q)
	if (q->decflag) {
	    p=q;
	    while (p != NULL) {
		lastone(p);
		p = p->next;
	    }
	}

}



/*
** Build up the tables for definitions of structs/unions.
**
** The table looks something like this:
**  ___________________________
** | type # from pass1 - ty    |
** |                           |
** | symbol-table ptr  - st    |
** |                           |
** | module number     - mno   |
** |                           |
** | list of = types   - eqty  |-> ENT* (a circular list)
** |                           |
** | list of != types  - neqty |-> ENT* -> ENT* -> ... -> NULL
** |                           |
** | pointer to next   - next  |
**  ---------------------------
**             |
**             V
**    ENT* (just like above)
**
**
** - The type # is the type # assigned by the compiler.  
** - The symbol-table pointer is a STAB pointer that describes the item.
** - The module number is the module number the item is defined in.
** - The list of = types is a circular list pointing to types that have
**   been previously compared and found to be =.  This lets the following
**   happen:  a==b, b==c, therefore a==c.
** - The list of != types is a linear list pointing to types that have
**   been previously compared and found to be !=.  Note that we can *not*
**   say the following: a!=b, b!=c, therefore a!=c.
** - Pointer to next is just another entry.
*/
void
build_lsu(ty, cmno, st)
T1WORD ty;
MODULE cmno;
STAB *st;
{
    ENT *p;
    int hash;
    LNBUG(ln_dbflag > 1, ("build_lsu: %ld %d",ty,cmno));

    /*
    ** Start of chain.
    */
    hash = ty % STRSIZ;
    p = lsu_table[hash];
   
    /*
    ** Allocate space for a new entry, and put the right type, module
    ** number, and symbol table pointer in.
    */
    lsu_table[hash] = (ENT *) malloc(sizeof(ENT));
    lsu_table[hash]->ty = ty;
    lsu_table[hash]->mno = cmno;
    lsu_table[hash]->st = st;
    lsu_table[hash]->eqty = lsu_table[hash];	/* = to itself 		*/
    lsu_table[hash]->neqty = NULL;		/* nothing compared yet */
    lsu_table[hash]->next = p;
}



/*
** Check to see if the two structures referred to by e1 and e2 are
** the same.  If they have been compared before and were the same,
** return 1.
** If they have been compared before and were not the same,
** return 0.
** If they have never been compared before, return -1.
*/
static int
samety(e1, e2)
ENT *e1, *e2;
{
    ENT *p=e1->eqty;
    UNEQ *q=e1->neqty;
    LNBUG(ln_dbflag > 1, ("samety"));

    /*
    ** Look along the = list until either the desired (e2) entry is found
    ** or we are back at the beginning of the circular list again.
    ** If e2 was found, return 1
    */
    while ((p != e1) && (p != e2))
	p = p->eqty;
    if (p == e2)
	return 1;

    /*
    ** Look along the != list until the end of the list is found, or
    ** the desired (e2) item is found.  If they are !=, e1 is in e2's
    ** list and vice-versa.
    ** If found, return 0; o/w return -1 (haven't been compared before.)
    */
    while ((q != NULL) && (q->neqty != e2))
	q = q->next;
    if (q == NULL)
	return -1;
    else return 0;
}



/*
** The two types specified by e1 and e2 were found to be unequal.
** Add e2 to e1's list of neqty types, and e1 to e2's list.
** Unlike the circular list of equal types, for unequal, each type
** must have its own list.
**            _________         
** Before:   | ENT *e1 |
**           |  neqty  |-> ENT *? -> ENT *? -> ENT *? -> NULL
**            ---------
**            _________
**           | ENT *e2 |
**           |_________|-> NULL
**
**            _________         
** After:    | ENT *e1 |
**           |  neqty  |-> ENT *e2 -> ENT *? -> ENT *? -> ENT *? -> NULL
**            ---------
**            _________
**           | ENT *e2 |
**           |_________|-> ENT *e1 -> NULL
**
*/
static void
makeuneq(e1, e2)
ENT *e1, *e2;
{
    UNEQ *oldnext;
    LNBUG(ln_dbflag > 1, ("makeuneq"));

    oldnext = e2->neqty;
    e2->neqty = (UNEQ *) malloc(sizeof(UNEQ));
    e2->neqty->next = oldnext;
    e2->neqty->neqty = e1;

    oldnext = e1->neqty;
    e1->neqty = (UNEQ *) malloc(sizeof(UNEQ));
    e1->neqty->next = oldnext;
    e1->neqty->neqty = e2;
}



/*
** The two types e1 and e2 were found to be equal.  Combine the two
** equal lists of each into 1 (to allow: e1==e2; e2==e3; therefore,
** e1==e3);
**
** The following shows (e1 == e3 == e4 == e1), and (e5 == e6 == e5):
**
**            |-----------------------------|          |---------|
**            |                             |          |         |
**           _v_____   _______   _______   _|_____   __v____   __|____
**          |ENT *e1| |ENT *e2| |ENT *e3| |ENT *e4| |ENT *e5| |ENT *e6|
**           -------   -------   -^-----   -^-----   -------   --^----
**             |                  |  |      |          |         |
**             |------------------|  |------|          |---------|
**
**
** If e5 is found to be == to e3, then the following results:
**
**                                  |----------------------------|
**            |-----------------------------|                    |
**            |                     |       |                    |
**           _v_____   _______   ___v___   _|_____   _______   __|____
**          |ENT *e1| |ENT *e2| |ENT *e3| |ENT *e4| |ENT *e5| |ENT *e6|
**           -------   -------   -------   -^-----   -^-----   --^----
**             |                     |      |         |   |      |
**             |                     |------|         |   -------|
**             |--------------------------------------|
**
**  e1 == e5 == e6 == e3 == e4 == e1
*/
static void
makeeq(e1, e2)
ENT *e1, *e2;
{
    ENT *oldnext, *tmp;
    LNBUG(ln_dbflag > 1, ("makeeq"));

    oldnext = e2->eqty;
    e2->eqty = e1;
    tmp = e1->eqty;
    while (tmp->eqty != e1)
	tmp = tmp->eqty;
    tmp->eqty = oldnext;
}



/*
** This is called for both struct and union equivalence.
** The order of members in a union doesn't matter, while it
** does for a struct.  The members of the union have been sorted 
** in lint1, so order can be considered relevent at this point.
**
** To prevent infinite recursion, a stack is used to tell which
** types have been compared.
*/
int
eq_struct(ty1, mno1, ty2, mno2)
T1WORD ty1, ty2;
MODULE mno1, mno2;
{
    ENT *p1, *p2;
    LNBUG(ln_dbflag > 1, ("eq_struct: %ld %d  %ld %d",ty1,mno1,ty2,mno2));

    /*
    ** push the type #'s onto a stack.  Note that we don't have to worry
    ** about including the mno# because a type # is unique within a mno#
    ** and it is guarenteed that we will only be comparing two different
    ** mno #'s
    */
    push(ty1, ty2);

    /*
    ** If module numbers are the same, then the type numbers must be the
    ** same for equivalence.
    */
    if (mno1 == mno2) {
	pop();
	return (ty1 == ty2);
    }

    /*
    ** Find the start of each type list for each file.
    */
    p1 = tyfind(ty1, mno1);
    p2 = tyfind(ty2, mno2);

    /*
    ** If one of the structs was never defined, then assume they are
    ** the same struct - user was warned in pass1 about not defining
    ** the struct.
    */
    if ((p1 == NULL) || (p2 == NULL)) {
	LNBUG(ln_dbflag > 1, ("no def for one(both) structs"));
	pop();
	return 1;
    }

    /*
    ** If the types were defined in the same file, at the same line,
    ** they assume (bad assumption?) that they are the same struct.
    */
    if ((p1->st->fno == p2->st->fno) && (p1->st->fline == p2->st->fline)) {
	LNBUG(ln_dbflag > 1, ("defined in same file/line"));
	pop();
	return 1;
    }
    LNBUG(ln_dbflag > 1, ("defined in different file/line"));

    switch (samety(p1,p2)) {
	case 1: 
	    pop();
	    return 1;
	case 0: 
	    pop();
	    return 0;
    }

    if (eq_s(p1->st, p2->st)) {
	pop();
	makeeq(p1, p2);
	return 1;
    } else {
	pop();
	makeuneq(p1, p2);
	return 0;
    }
}



/*
** Check to see if the two symbol table entries pointed to by p1 and p2
** are the same.
** Return 1 if they are, 0 if they are not.
*/
static int
eq_s(p1, p2)
STAB *p1, *p2;
{
    STYPE *q1, *q2;
    LNBUG(ln_dbflag > 1, ("eq_s"));

    if (p1 == p2)		/* if pointing to same entry, then they	*/
	return 1;		/* are the same				*/

    /*
    ** If the # args differ, or the type differs, return 0
    */
    if ((p1->nargs != p2->nargs) ||
	(p1->symty.t.aty != p2->symty.t.aty))
	return 0;

    q1 = p1->symty.next;		/* start to check members of struct */
    q2 = p2->symty.next;

    while ((q1 != NULL) || (q2 != NULL)) {
	/*
	** members must have same tag name and same type
	*/
	if ((q1->tag_name != q2->tag_name) || 
	    (q1->t.aty != q2->t.aty) ||
	    (q1->t.dcl_mod != q2->t.dcl_mod) ||
	    (q1->t.dcl_con != q2->t.dcl_con) ||
	    (q1->t.dcl_vol != q2->t.dcl_vol))
	    return 0;

	/*
	** check for bit-field sizes as well (in extra)
	** ......(to do).......
	*/

	/*
	** If we are dealing with 2 structs, then worry about recursive
	** members.  If these are currently being compared, their type #'s
	** will be on the stack.   If they are, assume the types are the
	** same and go onto the next member (if they are not the same, they
	** will be caught later on.)
	*/
	if (((LN_TYPE(q1->t.aty) == LN_STRUCT) || 
	     (LN_TYPE(q1->t.aty) == LN_UNION)) && 
	    (!onstack(q1->t.extra.ty,q2->t.extra.ty))) {
	    if (! eq_struct(q1->t.extra.ty, p1->mno, q2->t.extra.ty, p2->mno))
		return 0;
	} 

	q1 = q1->next;
	q2 = q2->next;
    }
    return 1;
}



/*
** Return the entry for type ty from the struct/union definition table.
** Return NULL if the entry is not there.
*/
ENT *
tyfind(ty, mno)
T1WORD ty;
MODULE mno;
{
    ENT *p;
    LNBUG(ln_dbflag > 0, ("tyfind: %ld %d", ty, mno));

    p = lsu_table[ty % STRSIZ];

    while ((p != NULL) && ((p->mno != mno) || (p->ty != ty)))
	p = p->next;

    return p;
}
  


/*
** Push the two type numbers onto the stack (used to prevent recursive
** checking of the sames structs.)
*/
static void
push(ty1, ty2)
T1WORD ty1, ty2;
{
    LNBUG(ln_dbflag > 2, ("push"));

    if (stackptr >= maxstack) {
	if (maxstack == 0) {
	    maxstack += STACKSZ;
	    stack = (STACK *) malloc(sizeof(STACK) * maxstack);
	} else {
	    maxstack += STACKSZ;
	    stack = (STACK *) realloc((char *)stack, sizeof(STACK) * maxstack);
	}
    }
    stack[stackptr].t1 = ty1;
    stack[stackptr].t2 = ty2;
    stackptr++;
}

static void
pop()
{
    LNBUG(ln_dbflag > 2, ("pop"));

    stackptr--;
    if (stackptr < 0) {
	lerror(0, "lint internal stack underrflow");
	stackptr++;
    }
}


static int
onstack(ty1, ty2)
T1WORD ty1, ty2;
{
    int i;
    LNBUG(ln_dbflag > 2, ("onstack"));

    for (i=0;i<stackptr;i++)
	if ((stack[i].t1 == ty1) && (stack[i].t2 == ty2)) {
	    return 1;
	}
    return 0;
}



#include <ctype.h>

/* this code retained from CI4.2 */
/* read arb. len. name string from input & return ptr to it */
char *
getstr()
{
    char buf[BUFSIZ];
    register char *cp = buf;
    register int c;
    LNBUG(ln_dbflag > 2, ("getstr"));

    while ((c = getchar()) != EOF) {
	*cp++ = (char) c;
	if (c == '\0' || !isascii(c))
	    break;
    }
    if (c != '\0') {
	LNBUG(ln_dbflag,("bad string: %s\n",buf));
	formerr();	/* exits */
    }

    return (l_hash(buf));
}



/* this code stolen from front-end's sym.c */

#define ST_HEAPSIZE 4096
static char *
lst_save(s, len)
char *s;
unsigned int len;
/*
** Save the pointed-at string in permanent storage.  len is the 
** total length of the string to save.
*/
{
    /* savetab points at the current storage heap.  saveleft counts
    ** the number of bytes remaining in the current storage heap.
    */
    static char *savetab;
    static unsigned int saveleft; 
    char *retval = savetab;		/* expected return value */
    LNBUG(ln_dbflag > 2, ("lst_save"));

    if (len > saveleft) {
	/* Out of space in current storage heap.  If the name
	** is too long to fit in any heap, give it its own
	** space.  Otherwise allocate a new regular storage
	** heap and stick the string in.
	*/
	retval = malloc((unsigned) (len > ST_HEAPSIZE ? len : ST_HEAPSIZE));
	if (retval == 0) 
	    lerror(FATAL,"lst_save() out of memory");

	if (len < ST_HEAPSIZE) {		/* allocated new heap	*/
	    savetab = retval + len;		/* we have a new table	*/
	    saveleft = ST_HEAPSIZE - len;	/* with a new size	*/
	}
    } else {				/* In current heap; adjust */
	savetab += len;
	saveleft -= len;
    }
    return(memcpy(retval, s, len));	/* copy in the string and return */
}



/*
** The linked hash tables consist of pointers to strings.  A use
** count prevents the tables from getting excessively full.
** (We waste space to gain better search performance.)
*/
#define ST_HASHSIZE	1023

static struct st_hash
{
    char * st_hptrs[ST_HASHSIZE];	/* the pointers, proper		*/
    int st_lens[ST_HASHSIZE];		/* size of each saved string	*/
    int	st_hused;			/* slots used so far		*/

    int st_lno[ST_HASHSIZE];		/* line number defined at	*/
    char *st_fno[ST_HASHSIZE];		/* file number seen in		*/

    struct st_hash * st_hnext;		/* pointer to next table	*/
} st_ihash;				/* initial hash table		*/



/*
** Look for NUL-terminated string in linked hash tables,
** return pointer to saved version.
*/
char *
l_hash(s)
char * s;
{
    LNBUG(ln_dbflag > 2, ("l_hash"));
    return(lst_nlookup(s, strlen(s)+1));
}



/*
** Look for string s with length len in the linked hash tables.
** If the string is found, return pointer to it.  Otherwise
** save the string and return a pointer to the saved version.
*/
static char *
lst_nlookup( s, len )
char * s;
unsigned int len;
{
    unsigned long hashval;
    char *cp;
    char * bound;
    char upper[9];
    struct st_hash * hp;	/* pointer to current hash table */
    int i,maxchk;
    extern int ln_Xcflag;
    extern int clno;
    LNBUG(ln_dbflag > 2, ("lst_nlookup: %d %s",len,s));

    /*
    ** Hash on the correct number of characters.  Lint needs to be able
    ** to limit this so that it can note length of names for portablility
    ** concerns.  A similar requirement exists to test strict conformance
    ** to ANSI.
    */
    cp = s;
    hashval = 0;

    /*
    ** If the p or Xc flag was specified, then hash using only
    ** one case
    */
    if (LN_FLAG('p') || ln_Xcflag) {
	if (LN_FLAG('p'))
	    maxchk = (len < 8) ? len : 8;
	else maxchk = (len < 6) ? len : 6;
	bound = upper + maxchk;
	for (i =0 ;i < maxchk; i++)
	    upper[i] = toupper(cp[i]);

	cp = upper;

	while (cp < bound) {
	    unsigned long overflow;

	    hashval = ( hashval << 4 ) + *cp++;
	    if ((overflow = hashval & (((unsigned long) 0xF) << 28)) != 0)
		hashval ^= overflow | (overflow >> 24);
	}
    } else {
	bound = s + len;

	/* PJW hash function from Dragon book, second edition. */
	while (cp < bound) {
	    unsigned long overflow;

	    hashval = ( hashval << 4 ) + *cp++;
	    if ((overflow = hashval & (((unsigned long) 0xF) << 28)) != 0)
		hashval ^= overflow | (overflow >> 24);
	}
    }
    hashval %= ST_HASHSIZE;		/* choose start bucket */

    /*
    ** Look through each table, in turn, for the name.  If we fail,
    ** save the string, enter the string's pointer, and return it.
    */
    for (hp = &st_ihash; ; hp = hp->st_hnext) {
	/* use quadratic re-hash */
	int i;				/* next probe interval */
	int probeval = hashval + 0;	/* next probe value */
	char *indis = NULL;
	char *fno = NULL;
	int lno=0;

	for (i = 1; i < ST_HASHSIZE; i += 2) {
	    char ** probe = &hp->st_hptrs[probeval];

	    if (*probe == 0) {
		/* empty entry */
		if (hp->st_hused > (ST_HASHSIZE + 3) / 4)
		    break;		/* This table is too full; try next */
		/* Save the string, add to table. */
		hp->st_hused++;
		*probe = lst_save(s, len);
		hp->st_lens[probeval] = len;

		if (clno) {
		    hp->st_lno[probeval] = clno;
		    hp->st_fno[probeval] = cfno;
		    if ((ln_Xcflag || LN_FLAG('p')) && (lno != 0) &&
			(indis != NULL))
			BWERROR(13, s, clno, cfno, indis, lno, fno);
		}
		return( *probe );
	    }

	    if ((ln_Xcflag && samename(upper,*probe,maxchk)) ||
		(LN_FLAG('p') && samename(upper,*probe,maxchk))) {
		indis = *probe;
		fno = hp->st_fno[probeval];
		lno = hp->st_lno[probeval];
	    }

	    if ((hp->st_lens[probeval]==len) && (memcmp(s,*probe,len)==0))
		return(*probe);

	    /* Adjust probe point */
	    probeval += i;
	    if (probeval >= ST_HASHSIZE)
		probeval -= ST_HASHSIZE;
	}

	if (hp->st_hnext == 0) {
#ifndef __STDC__
	    extern char * calloc();
#endif
	    /* Need to add new hash table. */
	    hp->st_hnext = (struct st_hash *) calloc(1,sizeof(struct st_hash));
	    if (hp->st_hnext == 0)
		lerror(FATAL,"st_lookup() out of memory");
	}
    }
    /*NOTREACHED*/
}


static int
samename(up,c,len)
char *up, *c;
int len;
{
    int i;

    for (i=0;i<len;i++)
	if (up[i] != toupper(c[i]))
	    return 0;

    return 1;
}
