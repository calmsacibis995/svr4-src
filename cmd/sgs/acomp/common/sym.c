/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)acomp:common/sym.c	55.1"
/* sym.c */

/* This module contains routines that handle symbol table
** management.  In particular there are routines to save
** strings (identifiers) and return unique handles for them,
** lookup symbols, etc.
*/

#include <string.h>
#include <memory.h>
#include "p1.h"
#ifdef	__STDC__
#include <stdlib.h>
#else
extern char * malloc();
extern char * calloc();
#endif

/* These routines manage the saving of identifier strings.
** They return a pointer to the (unique) copy of the
** saved string.
**
** String storage is arranged as a potentially infinite
** number of storage heaps and a list of hash tables that
** point into those heaps.  Strings are searched for via
** the available hash tables, which are never purged.  If
** the string is found, its pointer is returned.  Otherwise
** a new entry is made in the current storage heap, if there's
** room, or a new one.  Then the pointer to the saved
** string is returned.
**
** Strings may be arbitrary collections of characters; they
** may have embedded NUL characters.
*/

/* ST_HEAPSIZE is the size of a storage heap. */
#if !defined(ST_HEAPSIZE) || ST_HEAPSIZE < 1024
#  undef ST_HEAPSIZE
#  define ST_HEAPSIZE 4096
#endif


static char *
st_save(s, len)
char * s;
unsigned int len;
/* Save the pointed-at string in permanent storage.  len is the
** total length of the string to save.
*/
{
    /* savetab points at the current storage heap.  saveleft counts
    ** the number of bytes remaining in the current storage heap.
    */
    static char * savetab;
    static unsigned int saveleft;

    char * retval = savetab;		/* expected return value */

    if ( len > saveleft )
    {
	/* Out of space in current storage heap.  If the name
	** is too long to fit in any heap, give it its own
	** space.  Otherwise allocate a new regular storage
	** heap and stick the string in.
	*/
/*	puts("allocating new string heap"); */
	retval =
	    malloc( (unsigned) (len > ST_HEAPSIZE ? len : ST_HEAPSIZE ));
        if ( retval == 0 ) cerror("st_save() out of memory");

	if (len < ST_HEAPSIZE) { 	/* allocated new heap */
	    savetab = retval + len;	/* we have a new table */
	    saveleft = ST_HEAPSIZE - len; /* with a new size */
	}
    }
    else {				/* In current heap; adjust */
	savetab += len;
	saveleft -= len;
    }
    return( memcpy( retval, s, len ) );	/* copy in the string and return */
}


/*
** The linked hash tables consist of pointers to strings.  A use
** count prevents the tables from getting excessively full.
** (We waste space to gain better search performance.)
*/
#ifndef	ST_HASHSIZE			/* number of pointers in hash table */
#  define ST_HASHSIZE	1023
#endif

static struct st_hash
{
    char * st_hptrs[ST_HASHSIZE]; 	/* the pointers, proper */
    unsigned int st_lens[ST_HASHSIZE]; 	/* size of each saved string */
    int	st_hused;			/* slots used so far */
    struct st_hash * st_hnext;		/* pointer to next table */
} st_ihash;				/* initial hash table */


char *
st_lookup( s )
char * s;
/* Look for NUL-terminated string in linked hash tables,
** return pointer to saved version.
*/
{
    return( st_nlookup(s, strlen(s)+1 ) );
}


char *
st_nlookup( s, len )
char * s;
unsigned int len;
/* Look for string s with length len in the linked hash tables.
** If the string is found, return pointer to it.  Otherwise
** save the string and return a pointer to the saved version.
*/
{
    unsigned long hashval;
    char *cp;
    char * bound;
    struct st_hash * hp;	/* pointer to current hash table */

    /*
    ** Hash on the correct number of characters.  Lint needs to be able
    ** to limit this so that it can note length of names for portablility
    ** concerns.  A similar requirement exists to test strict conformance
    ** to ANSI.
    */
    cp = s;
    bound = s + len;
    hashval = 0;
    /* PJW hash function from "Compilers: Principles, Techniques, and Tools" 
     * by Aho, Sethi, and Ullman, Second Edition.
     */
    while (cp < bound)
    {
	unsigned long overflow;

	hashval = ( hashval << 4 ) + *cp++;
	if ((overflow = hashval & (((unsigned long) 0xF) << 28)) != 0)
	    hashval ^= overflow | (overflow >> 24);
    }
    hashval %= ST_HASHSIZE;		/* choose start bucket */

    /* Look through each table, in turn, for the name.  If we fail,
    ** save the string, enter the string's pointer, and return it.
    */
    for (hp = &st_ihash; ; hp = hp->st_hnext) {
	/* use quadratic re-hash */
	int i;				/* next probe interval */
	int probeval = hashval + 0;	/* next probe value */

	for (i = 1; i < ST_HASHSIZE; i += 2) {
	    char ** probe = &hp->st_hptrs[probeval];
	    if (*probe == 0) {
		/* empty entry */
		if (hp->st_hused > (ST_HASHSIZE * 3) / 4)
		    break;		/* This table is too full; try next */
		/* Save the string, add to table. */
		hp->st_hused++;
		*probe = st_save(s, len);
		hp->st_lens[probeval] = len;
		return( *probe );
	    }

	    if (   hp->st_lens[probeval] == len
		&& memcmp(s, *probe, len) == 0
	    ) return( *probe );

	    /* Adjust probe point */
	    probeval += i;
	    if (probeval >= ST_HASHSIZE)
		probeval -= ST_HASHSIZE;
	}

	if (hp->st_hnext == 0) {
	    /* Need to add new hash table. */
/*	    puts("allocating new string hash table"); */
	    if ((hp->st_hnext =
		(struct st_hash *) calloc(1, sizeof(struct st_hash)))
		== 0)
		cerror("st_lookup() out of memory");
	}
    }
    /*NOTREACHED*/
}

/* Start of symbol table related routines. */


#ifndef	INI_SY_SIZE
#define	INI_SY_SIZE 500			/* initial symbol table size */
#endif

/* Symbol table.  It's extern because the SY_ macros access it directly. */
TABLE(/*extern*/, td_stab, static, in_stab,
		struct symtab, INI_SY_SIZE, 0, "symbol table");
	    

static SX sy_hash[SY_HASHSZ];	/* symbol hash buckets */
/* hash function based on name pointer */
#define sy_hashfunc(p) (((unsigned int) (p)) % SY_HASHSZ)
			/* unsigned int sy_hash(char *) */

/* sy_scopehead is a linked list of all symbols (except labels).
** The head of the list is the most recently defined symbol.
*/
static SX sy_scopehead = SY_NOSYM;

/* The symbol scope level is 0 for externs, 1 for function parameters,
** 2 for the first set of autos in a function, > 2 for inner blocks.
*/

static SY_LEVEL_t sy_scopelevel = 0;
static SX sy_labtab = SY_NOSYM;		/* singly linked label table */

/* The special handling for sy_1stnonlev0 doesn't seem worth
** the bookkeeping effort for the few times it comes up.
** Not many symbols have to move, and the number of non-level 0
** symbols tends to be small, anyway.  (Most major declarations
** happen at level 0.)
*/
#ifdef	SY_1STNONLEV0
/* sy_1stnonlev0 is the symbol index of the first symbol table
** entry above level 0 on the scope list.  Knowing this speeds
** table insertions in sy_insert().
*/

static SX sy_1stnonlev0 = SY_NOSYM;
#endif

static SX sy_freelist = SY_NOSYM;
#ifdef FAT_ACOMP
static SX sy_outofscope = SY_NOSYM;
#endif

static SX sy_getnew();
static void sy_insert();
#ifndef	NODBG
static void sy_prtab();
static void sy_print();
#endif

#define	SY_TABLES	(SY_TAG|SY_MOSU|SY_LABEL|SY_NORMAL)

SX
sy_lookup(name, tab, enter)
register char *name; 
SY_FLAGS_t tab;
int enter;
/* Look up name string "name" in virtual symbol table
** "tab".  Return the symbol index of the symbol that
** was found.  If none was found, return SY_NOSYM if "enter" is
** SY_LOOKUP, or return the index of a new entry if "enter"
** is SY_CREATE.
** Because they are used infrequently, and because their scoping
** is funny, labels are handled separately.
*/
{
    register SX i;
    register struct symtab *sp;
    unsigned long hashval = sy_hashfunc(name);
    int tabcheck = tab|SY_INSCOPE;

    DEBUG( d1debug > 2,
	( "sy_lookup( %s, %d, %d )\n", name, tab, enter));

    /* compute initial hash index */
    i = tab == SY_LABEL ? sy_labtab : sy_hash[hashval];
    
    for ( ; i != SY_NOSYM; i = sp->sy_next) {
	sp = &SY(i);

	/* look for name */
	if ( sp->sy_name != name )
	    continue;
	/* 3.1.2.1:  Variables become in-scope at the end of their
	** declarator.  Declaration processing must make them
	** visible explicitly.  Otherwise, skip them.
	*/
	if ( (sp->sy_flags & (SY_TABLES|SY_INSCOPE)) == tabcheck )
	    return( i );
    }

    /* Entry not found.  Return SY_NOSYM if we're not entering anything. */
    if (enter == SY_LOOKUP) return( SY_NOSYM );

    /* Get new entry, fill in partially.
    ** (Beware!! Must NOT do &stab[ i = sy_getnew() ],
    ** because the value of "stab" could change with the
    ** sy_getnew() ).
    */
    i = sy_getnew();
    sp = &SY(i);

    sp->sy_name = name;
    sp->sy_lineno = er_getline();
#ifdef LINT
    sp->sy_file = st_lookup(er_curname());
#endif
    sp->sy_flags = tab;			/* set table */
    sp->sy_type = TY_NONE;
    sp->sy_class = SC_NONE;
    sp->sy_level = sy_scopelevel;
    /* link in on current chain */
    if (tab == SY_LABEL) {
	sp->sy_next = sy_labtab;
	sy_labtab = i;
    }
    else {
	sp->sy_next = sy_hash[hashval];
	sy_hash[hashval] = i;
	sp->sy_scopelink = sy_scopehead;
	sy_scopehead = i;
#ifdef	SY_1STNONLEV0
	if (sy_1stnonlev0 == SY_NOSYM && sy_scopelevel != SL_EXTERN)
	    sy_1stnonlev0 = i;
#endif
    }
    return( i );
}


void
sy_clear( lev )
SY_LEVEL_t lev;
/* Clear symbols from block level "lev".  External symbols get
** moved to block level 0 as appropriate.  Also, check for undefined
** labels at this point.
*/
{
    /* clear entries of internal scope from the symbol table */
    register SX i;
    SX savehead = SY_NOSYM;		/* list header of entries to move */


    /* For each entry on the "sy_scopehead" list, if the level is
    ** >= the scope level in question, remove the symbol from the
    ** table.  This algorithm assumes that symbol entries, both
    ** on the scope list AND the various hash queues, are strictly
    ** nested.
    ** C wart:  move external definitions to appropriate outer scopes.
    ** Remove labels from their special table if lev < 2.
    */

    while ( (i = sy_scopehead) != SY_NOSYM) {
	register struct symtab * sp = &SY(i);
	SX * hashslot;
	
	if (sp->sy_level < lev) 
	    break;
	hashslot = &sy_hash[sy_hashfunc(sp->sy_name)];
	DEBUG(d1debug > 1, ("removing %s = ID %d, flags %o level %d\n",
			sp->sy_name, i, sp->sy_flags, sp->sy_level));
#ifdef LINT
	/*
	** "argument %s unused in function %s"
	** "variable %s unused in function %s"
	** "%s set but not used in function %s"
	** "enum never defined: %s"
	*/
	ln_symunused(i, lev);
#endif

	if (sp->sy_type == TY_NONE)
	    cerror("%s undefined", sp->sy_name);
	/* unlink from hash list and scope list */
	sy_scopehead = sp->sy_scopelink;
	if (*hashslot != i)
	    cerror("removing wrong entry");
	*hashslot = sp->sy_next;
#ifdef	SY_1STNONLEV0
	if (i == sy_1stnonlev0)
	    sy_1stnonlev0 = SY_NOSYM;
#endif

	/* Handle still-tentative definitions at file scope. */
	if (lev == SL_EXTERN && (sp->sy_flags & SY_TENTATIVE) != 0)
	    dcl_tentative(i);

#ifdef ELF_OBJ
	if ((sp->sy_flags & SY_DBOUT) == 0)
	    DB_SY_CLEAR(i);		/* do debugging for symbol */
#endif
#ifdef	OPTIM_SUPPORT
	OS_SYMBOL(i);			/* generate optim info. */
#endif

	/* Find static functions referenced but not defined. */
	if (   lev == SL_EXTERN
	    && sp->sy_class == SC_STATIC
	    && TY_ISFTN(sp->sy_type)
	    && (sp->sy_flags & (SY_REF|SY_DEFINED)) == SY_REF
	)
	    WERROR("static function called but not defined: %s()", sp->sy_name);

	/* Either make entry free, or move externals and function decls. */
	if ((sp->sy_flags & SY_TOMOVE) != 0) {
	    if (sp->sy_sameas == SY_NOSYM) {
		DEBUG(d1debug > 1, ("moving ID %d to level 0\n", i));
		sp->sy_level = SL_EXTERN;
		sp->sy_next = savehead;
		sp->sy_flags &= (SY_FLAGS_t) ~SY_TOMOVE;
		sp->sy_flags |= SY_MOVED;	/* remember it was moved */
		sp->sy_type = ty_mkhidden(sp->sy_type,TY_NONE);
		savehead = i;
	    }
	    else {				/* copy information and free */
		struct symtab * s2p = &SY(sp->sy_sameas);
		DEBUG(d1debug > 1, ("copying info from ID %d to ID %d\n",
					i, sp->sy_sameas));
#ifdef LINT
		s2p->sy_flags |= (sp->sy_flags & (SY_SET|SY_REF));
#endif
		/* Use information in expiring declaration to provide
		** hidden information in remaining one.
		*/
		s2p->sy_type = ty_mkhidden(sp->sy_type,s2p->sy_type);
#ifdef FAT_ACOMP
		sp->sy_next = sy_outofscope;
		sy_outofscope = i;
#else
		sp->sy_next = sy_freelist;	/* add to free list */
		sy_freelist = i;
#ifndef NODBG
		sp->sy_name = SY_EMPTY;
#endif
#endif	/* FAT_ACOMP */
	    }
	}
	else {
#ifdef FAT_ACOMP
	    sp->sy_next = sy_outofscope;
	    sy_outofscope = i;
#else
	    sp->sy_next = sy_freelist;		/* add to free list */
	    sy_freelist = i;
#ifndef NODBG
	    sp->sy_name = SY_EMPTY;
#endif
#endif	/* FAT_ACOMP */
	}
    }
    
    /* Note that symbols have been put on savehead in the
    ** reverse order of their declaration, which means the
    ** list reads in the order of original declaration.
    */

    for (i = savehead; i != SY_NOSYM; ) {
	SX j = i;			/* remember entry number */
	i = SY(i).sy_next;		/* for next iteration */
	sy_insert(j, 0);		/* insert behind other entries */
    }

    /* Clear out labels if level < SL_INFUNC, then clean out all
    ** out-of-scope symbols (make them free)
    */
    if (lev == SL_FUNARG) {
	SX last;
	for (i = sy_labtab; i != SY_NOSYM; i = SY(i).sy_next) {
	    /* Check for labels not yet defined. */
	    if (SY(i).sy_class == SC_LABEL && (SY(i).sy_flags & SY_DEFINED) == 0)
		UERROR("undefined label: %s", SY(i).sy_name);
	    last = i;
#ifndef NODBG
	    SY(i).sy_name = SY_EMPTY;
#endif
	}
	if (sy_labtab) {
#ifdef	FAT_ACOMP
	    SY(last).sy_next = sy_outofscope;
	    sy_outofscope = sy_labtab;
#else
	    SY(last).sy_next = sy_freelist;
	    sy_freelist = sy_labtab;
#endif
	    sy_labtab = SY_NOSYM;
	}
    }
    return;
}


#ifdef FAT_ACOMP

void
sy_discard()
/* Discard all entries on out-of-scope list. */
{
    SX last;
    SX i;

    /* Find end of out-of-scope list, paste that list
    ** to freelist.
    */
    last = sy_outofscope;
    for (i = sy_outofscope; i != SY_NOSYM; i = SY(i).sy_next) {
	last = i;
#ifndef NODBG
	SY(i).sy_name = SY_EMPTY;
#endif
    }
    if (last != SY_NOSYM) {
	SY(last).sy_next = sy_freelist;
	sy_freelist = sy_outofscope;
	sy_outofscope = SY_NOSYM;
    }
    return;
}

#endif	/* def FAT_ACOMP */

SX
sy_hide( idn )
SX idn;
/* Hide an entry:  used when a symbol in an inner block
** hides an outer declaration of the symbol.  The new
** symbol gets linked at the top of the scope links, and
** its level is the current level.
*/
{
    SX new = sy_getnew();		/* get another entry */
    register struct symtab *pold = &SY(idn); 
    register struct symtab *pnew = &SY(new);
    SX hashval = sy_hashfunc(pold->sy_name);

    *pnew = *pold;
    pnew->sy_level = sy_scopelevel;
    /* Only retain symbol table bits in flags word. */
    pnew->sy_flags &= SY_TABLES;
    pnew->sy_lineno = er_getline();
#ifdef LINT
    pnew->sy_file = st_lookup(er_curname());
#endif

    /* Link on appropriate bucket. */
    pnew->sy_next = sy_hash[hashval];
    sy_hash[hashval] = new;
    /* link on scope list */
    pnew->sy_scopelink = sy_scopehead;
    sy_scopehead = new;
#ifdef	SY_1STNONLEV0
    if (sy_1stnonlev0 == SY_NOSYM && sy_scopelevel != SL_EXTERN)
	sy_1stnonlev0 = new;
#endif

    DEBUG(d1debug, ( "	%d hidden by %d\n", idn, new ));
    return( new );
}

SX
sy_hidelev0( idn )
SX idn;
/* Hide an entry at level 0.  This routine does the same thing
** as sy_hide(), except the resulting symbol table entry gets
** put ahead of all others at scope level 0 with the same name.
** The hash and scope chains are adjusted accordingly.
*/
{
    SX cur = sy_getnew();		/* get new symbol entry */

    if (SY(idn).sy_level != SL_EXTERN)
	cerror("hiding non-top-level symbol");
    
    SY(cur) = SY(idn);			/* copy old entry */
    SY(cur).sy_flags &= SY_TABLES;	/* just keep tables info */
    sy_insert(cur, 1);			/* insert ahead of other entries */
    return( cur );
}


static void
sy_insert(sid, ahead)
SX sid;
int ahead;
/* Insert symbol table entry sid into the symbol table in the
** appropriate place for level 0.  Change sy_level to SL_EXTERN.
** The hash and scope linkages are adjusted to be correct for the
** place sid is inserted.  If ahead is non-zero, the symbol is placed
** ahead of all other level 0 entries of the same name.  Otherwise
** it is placed behind all non-SY_MOVED entries of the same name.
**
** The algorithm is:
**	1. Walk the hash list to find the proper place to insert the
**	symbol.  Link it in.
**	2. Find the corresponding place in the scope list.  Insert
**		the symbol in it.
** We assume that, most of the time, the symbol is not a duplicate
** name.
**
** We must maintain the fixed-point that the order of a hash list
** is preserved in the scope list as well.
*/
{
    SX * behind;			/* runs one behind current entry */
    char * name = SY(sid).sy_name;
    struct symtab *sp;
    SX next;				/* symbol ID of symbol after sid */

    for (behind = &sy_hash[sy_hashfunc(name)]; (next = *behind) != SY_NOSYM; ) {
	sp = &SY(next);
	if (sp->sy_name == name && sp->sy_level == SL_EXTERN) {
	    if (ahead || (sp->sy_flags & SY_MOVED))
		break;
	}
	behind = &sp->sy_next;
    }
    /* next is either SY_NOSYM, if there are no entries to put sid ahead
    ** of, or is the sid of an entry ahead of which to put entry.
    ** Link sid on hash bucket list here.
    */
    *behind = sid;
    SY(sid).sy_next = next;

    /* Link sid on the scope list just ahead of next. */
#ifdef	SY_1STNONLEV0
    if (sy_1stnonlev0 == SY_NOSYM)
	behind = &sy_scopehead;
    else
	behind = &SY(sy_1stnonlev0).sy_scopelink;
#else
    behind = &sy_scopehead;
#endif
    while (*behind != next && *behind != SY_NOSYM)
	behind = &SY(*behind).sy_scopelink;

    *behind = sid;
    SY(sid).sy_scopelink = next;
    SY(sid).sy_level = SL_EXTERN;
    return;
}

SX
sy_chkfunext(idn)
SX idn;
/* Look back through the symbol table to find the first
** symbol with the same name as idn, and that might
** be subjected to linkage:  any function, any object
** with symbol class SC_EXTERN, or any static object
** at file scope.
** Return its symbol ID or SY_NOSYM if none is found.
*/
{
    char * name = SY(idn).sy_name;
    SX i;

    for (i = idn; i != SY_NOSYM; i = SY(i).sy_next) {
	register struct symtab * pold = &SY(i);
	
	if (name == pold->sy_name) {
	    switch( pold->sy_class ){
	    case SC_STATIC:
		if (pold->sy_level == SL_EXTERN || TY_ISFTN(pold->sy_type))
		    return( i );
		break;
	    case SC_ASM:		/* behaves like external linkage */
	    case SC_NONE:		/* external linkage */
	    case SC_EXTERN:		/* external linkage */
		return( i );		/* return the matching name */
	    }
	}
    }
    return( SY_NOSYM );
}


void
sy_inclev()
/* Increment symbol table scope level. */
{
    sy_scopelevel++;
    DEBUG(d1debug, ("incrementing symbol table level to %d\n", sy_scopelevel));
#ifdef LINT
    /* Write a BLK_BEG record for cxref's use. */
    if (LN_FLAG('R'))
	cx_inclev();
#endif
    return;
}

#ifdef	NODBG
#define	SY_PRTAB(lev)
#else
#define	SY_PRTAB(lev) if(d1debug > lev) sy_prtab()
#endif

void
sy_declev()
/* Decrement symbol table scope level.  Remove symbols at the
** old level first.
*/
{
    if (sy_scopelevel == 0)
	cerror("attempt to leave scope level %d", sy_scopelevel);

    DEBUG(d1debug, ("decrementing symbol table level from %d\n", sy_scopelevel));
    DEBUG(d1debug > 1, ("starting table:\n"));
#ifdef LINT
    /* Write a BLK_END record for cxref's use. */
    if (LN_FLAG('R'))
	cx_declev();
#endif
    SY_PRTAB(1);
    sy_clear(sy_scopelevel);
    --sy_scopelevel;
    DEBUG(d1debug > 1, ("resulting table:\n"));
    SY_PRTAB(1);
    return;
}


SY_LEVEL_t
sy_getlev()
/* Return current scope level */
{
    return( sy_scopelevel );
}


SX
sy_sumbr(s)
char * s;
/* Look for a non-unique struct/union member named s.
** If there is a unique name, return its symbol index.
** If there is no such name, or if there is a duplicate,
** return SY_NOSYM.
*/
{
    SX retsid = sy_lookup(s, (SY_FLAGS_t) SY_MOSU, SY_LOOKUP);
    SX sid;

    if (! retsid) return( SY_NOSYM );	/* none found */

    /* Found one.  Look for more than one. */

    for (sid = SY(retsid).sy_next; sid; sid = SY(sid).sy_next) {
	if ((SY(sid).sy_flags & SY_MOSU) == SY_NOSYM) 
	    continue;
	/* Member is still considered unique if it has the same offset */
	if (   SY(sid).sy_name == s
	    && SY(sid).sy_offset != SY(retsid).sy_offset
	    ) return( SY_NOSYM );	/* found duplicate */
    }
    return( retsid );			/* Found only one.  It's unique. */
}

	
static SX
sy_getnew()
/* Get new symbol table entry.  Use free list, if possible.
** Otherwise return the next symbol from the new-symbol buffer.
*/
{
    static SX nextfree = 1;		/* first available entry:
					** 0 == SY_NOSYM
					*/
    register SX try = sy_freelist;

    if (try != SY_NOSYM)
	sy_freelist = SY(try).sy_next;
    else {
	try = nextfree;
	if (try == SY_SIZE) {
	    /* Need to reallocate table to larger size. */
	    td_enlarge(&td_stab,0);	/* need at least one entry */
	}
	++nextfree;
	TD_USED(td_stab)++;
	TD_CHKMAX(td_stab);
    }
    /* "try" is the index of a free slot */
    return( try );
}


#ifndef NODBG

static void
sy_prtab()
/* print symbol table information for debugging purposes */
{
    register int i;

    DPRINTF("+++++++\n\n");

#ifdef	SY_1STNONLEV0
    DPRINTF("scopehead = %d\n1stnonlev0 = %d\n\n", sy_scopehead, sy_1stnonlev0);
#else
    DPRINTF("scopehead = %d\n\n", sy_scopehead);
#endif
#ifdef FAT_ACOMP
    DPRINTF("outofscope = %d\n\n", sy_outofscope);
#endif

    for (i = 0; i < SY_HASHSZ; ++i) {
	if (sy_hash[i] != 0)
	    DPRINTF("hash[%3d]	%d\n", i, sy_hash[i]);
    }
    DPRINTF("\n");

    for (i = 1; i < SY_SIZE; ++i) {
	if (SY(i).sy_name != SY_EMPTY)
	    sy_print(i);
    }
    DPRINTF("\n------\n");
}


static void
sy_print(i)
int i;
/* Print one line description of symbol entry. */
{
    struct symtab * sp = &SY(i);
    DPRINTF("ID %3d	lev %2d, nxt %3d, scplk %3d, hsh %3d, \
cls %2d, typ %3d, flg %#4o, \"%s\"\n",
	i, (int) sp->sy_level, sp->sy_next, sp->sy_scopelink,
	sy_hashfunc(sp->sy_name), sp->sy_class, sp->sy_type, sp->sy_flags,
	sp->sy_name);
    return;
}

#endif
