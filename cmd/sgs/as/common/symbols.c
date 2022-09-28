/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:common/symbols.c	1.12"

#include <stdio.h>
#include <string.h>
#include "systems.h"
#include "symbols.h"
#include "symbols2.h"
#include <sys/stat.h>
#include <ctype.h>
/*
 *	"symbols.c" is a file containing functions for accessing the
 *	symbol table.  The following functions are provided:
 *
 *	newent(string)
 *		Creates a new symbol table entry for the symbol with name
 *		"string".
 *
 *	lookup(string,install_flag)
 *		Looks up the symbol whose name is "string" in the symbol
 *		table and returns a pointer to the entry for that symbol.
 *		Install_flag is INSTALL if symbol is to be installed,
 *		N_INSTALL if it is only to be looked up.
 *
 *	traverse(func)
 *		Goes through the symbol table and calls the function "func"
 *		for each entry.
 *
 *	creasyms()
 *		Enters the instruction mnemonics into the instruction hash
 *		table.  Then it malloc()'s the symbol table and hash symbol
 *		table.
 *
 *	addstr(string)
 *		Enters the "string" into the string table.  Called by
 *		newent().  Space for the string table is initially
 *		malloc()-ed in strtabinit().  If "string" would exceed
 *		the available space, then the string table is realloc()-ed
 *		with a larger size.  This procedure is only used in the
 *		flexnames version of the assembler.
 *
 *	strtabinit()
 *		Sets up the symbol table and section header string tables, 
 *		with space malloc()-ed.
 *
 */

extern symbol *dot;
extern long newdot;

/* SYMFACTOR is an empirically determined average ratio of number of
 * bytes of input to number of symbols. */
#define SYMFACTOR 75

/* The symbol table is actually an array of pointers to tables. 
 * "tablesize" is the number of symbols each table can hold.
 */
static unsigned long tablesize;

/* We determine NTABLES based on the fact that an upper bound to the
 * number of symbols we may have is the number of bytes in the input. */
#define NTABLES SYMFACTOR
static symbol *symtab[NTABLES];

static short tabletop = 0; /* index in symtab of next table to allocate */

static unsigned long numsyms = NSYMS;	/* current max. number of symbols */
static unsigned long numhash = NSYMS;	/* size of hash table */

static unsigned long symcnt;	/* index of next free symbol entry */

static symbol **symhashtab;	/* hash table for symbols */
/* instr **insthashtab;	hash table for instructions */

/* static allocations for tables - may turn out too small */
static symbol statsymtab[NSYMS];
static symbol *statsymhashtab[NSYMS];

extern void aerror();

char *strtab;
long currindex = 0;



static void incr_symtab();


static symbol *
newent(strptr)
	register char *strptr;
{
	register symbol *symptr;

	if (symcnt >= numsyms)
		incr_symtab();
	GETSYMPTR(symcnt,symptr);
	symcnt++;
	symptr->name = malloc(strlen(strptr) + 1);
	(void) strcpy(symptr->name,strptr);

	/* Provide default field values */
	symptr->binding = STB_GLOBAL;
	symptr->sectnum = SHN_UNDEF;
	symptr->value = 0;
	symptr->size = 0;
	symptr->type = STT_NOTYPE;
	symptr->stindex = 0;
	symptr->align = 0;
	symptr->flags = (~TYPE_SET) & (~SIZE_SET) & GO_IN_SYMTAB;
	return(symptr);
}

#if STATS
unsigned long numids,	/* number of identifiers entered */
	numretr,	/* number of retrieval lookups */
	numlnksretr,	/* number of links traversed (collisions)
			in retrievals */
	numins,		/* number of insertion lookups */
	numlnksins,	/* number of links traversed (collisions)
			in insertions */
	numreallocs,	/* number of reallocs of symbol table */
	inputsz;	/* number of bytes of input */
#endif

/*ARGSUSED*/
symbol *
lookup(sptr,install)
	char *sptr;
	BYTE install;
{
	register symbol *p, *q = NULL;
	register char *ptr1;
	register unsigned long  ihash = 0;
	register unsigned long hash;

/* search for user name in symhashtab[] */
#if STATS
if (install)
	numins++;
else
	numretr++;
#endif

	/* hash sptr */
	ptr1 = sptr;
	while (*ptr1) {
		ihash = ihash*4 + *ptr1++;
	}
	ihash += *--ptr1 * 32;
	hash = ihash % numhash;

	/* Search thru chain.  If symbol with correct name is found,
	 * return pointer to it.  Otherwise fall thru. */
	for (p = symhashtab[hash]; p!=NULL; p=p->next) {
		/* Compare the string given with the symbol string.	*/
		if (strcmp(sptr,p->name) == 0)
			return p;
		q = p;
#if STATS
		if (install)
			numlnksins++;
		else
			numlnksretr++;
#endif
	} /* for (p */

	/* Not found, so install new symbol entry if flag set. */
	if (install) {
#if STATS
		numids++;
#endif
		if (q) 
			q->next = p = newent(sptr);
		else
			symhashtab[hash] = p = newent(sptr);
	} else
		p = NULL;
	return p;
}

void
traverse(func)
	void (*func)();
{
	register short table, index, nsyms ;

	for (table=0; table<tabletop-1; table++)
		for (index=0; index < tablesize; ++index)
			(*func)(&symtab[table][index]);
	nsyms = (symcnt-1) % tablesize;
	for (index=0; index <= nsyms; ++index) 
		(*func)(&symtab[table][index]);
	
}

/* creasyms() sets the initial size of the symbol table and symbol
hash table to either (1) a statically determined initial value, or
(2) an estimated size, based on the input file size, if (1) appears
too small.  We bias the calculation to use the statically allocated
tables by a factor of 3 so that we malloc() new initial tables only
if we are sure that the static sizes are way off.
*/
extern FILE *fdin;

void
creasyms()
{
	struct stat buf;
	(void) fstat(fileno(fdin),&buf);
#if STATS
	inputsz = buf.st_size;
#endif
	tablesize = buf.st_size/SYMFACTOR;
	if (tablesize > NSYMS*3) {
		/* reinitialize tables to larger values */
		/* make tablesize odd for hashing */
		tablesize += tablesize % 2 + 1;
		numsyms = numhash = tablesize;
		if ((symtab[tabletop++] = (symbol *)calloc((unsigned int) numsyms,sizeof(symbol))) == NULL)
			aerror("Cannot malloc symbol table");
		if ((symhashtab = (symbol **)calloc((unsigned int) numhash,sizeof(symbol *))) == NULL)
			aerror("Cannot malloc symbol hash table");
	} else {
		/* just use statically allocated tables */
		tablesize = NSYMS;
		symtab[tabletop++] = statsymtab;
		symhashtab = statsymhashtab;
	}
}

static long	size,
		basicsize = 4 * BUFSIZ;
unsigned long
addstr(strptr)
	register char	*strptr;
{
	register int	length;
	register long	offset;


	length = strlen(strptr);
	while (length + currindex >= size)
		size += basicsize;
	if ((strtab = realloc(strtab,(unsigned)size )) == NULL)
			aerror("cannot realloc string table");
	(void) strcpy(&strtab[currindex],strptr);
	offset = currindex;
	currindex += length + 1;
	return(offset);
}	/* addstr(strptr) */

void
initdot()
{
	dot = lookup(".", INSTALL);
	dot->value = newdot = 0L;
	dot->sectnum = 0;
	dot->binding = STB_LOCAL;
	dot->flags &= ~GO_IN_SYMTAB;
}

void
strtabinit()
{
	if ((strtab = malloc((unsigned)(size = basicsize))) == NULL)
		aerror("cannot malloc string table");
	strtab[currindex++] = '\0';
}	/* strtabinit() */

/* Increase size of symbol table by tablesize. */
static void
incr_symtab()
{
#if STATS
	numreallocs++;
#endif
	if (tabletop >= NTABLES) {
		aerror("Out of symbol table");
	}
	if ((symtab[tabletop++] = (symbol *)calloc((unsigned int)tablesize,sizeof(symbol))) == NULL)
		aerror("Cannot malloc more symbol table");
	numsyms += tablesize;
}

/*
 * 	this function allocates common symbols with local binding
 *	in the bss section.
 */


void
alloc_lcomm(ptr)
symbol *ptr;
{
	extern void bss();


	if ((LOCAL(ptr)) && (ptr->type == (STT_OBJECT)) && COMMON(ptr)) {

		bss(ptr,ptr->size,(long) ptr->align);
	}
}

#define ULONG_MAX       4294967295      /* max value of an "unsigned long int" */

#define DIGIT(x)	(isdigit(x) ? (x) - '0' : \
			islower(x) ? (x) + 10 - 'a' : (x) + 10 - 'A')
#define MBASE	('z' - 'a' + 1 + 10)

/* The following macro is a local version of isalnum() which limits
 * alphabetic characters to the ranges a-z and A-Z; locale dependent
 * characters will not return 1. The members of a-z and A-Z are
 * assumed to be in ascending order and contiguous
 */
#define lisalnum(x)	(isdigit(x) || \
			 ((x)>='a' && (x)<='z') || ((x)>='A' && (x)<='Z'))

unsigned long
Strtoul(str, nptr, base)
register const char *str;
char **nptr;
register int base;
{
	register unsigned long val;
	register int c;
	int xx;
	unsigned long	multmax;
	const char 	**ptr = (const char **)nptr;

	if (ptr != (const char **)0)
		*ptr = str; /* in case no number is formed */
	if (base < 0 || base > MBASE) {
		yyerror("invalid base");
		return (0); /* base is invalid -- should be a fatal error */
	}
	c = *str;
	while (isspace(c))
		c = *++str;
	if (base == 0)
		if (c != '0')
			base = 10;
		else if (str[1] == 'x' || str[1] == 'X')
			base = 16;
		else
			base = 8;
	/*
	 * for any base > 10, the digits incrementally following
	 *	9 are assumed to be "abc...z" or "ABC...Z"
	 */
	if (!lisalnum(c) || (xx = DIGIT(c)) >= base)
		return (0); /* no number formed */
	if (base == 16 && c == '0' && (str[1] == 'x' || str[1] == 'X') &&
		isxdigit(str[2]))
		c = *(str += 2); /* skip over leading "0x" or "0X" */

	multmax = ULONG_MAX / (unsigned)base;
	val = DIGIT(c);
	for (c = *++str; lisalnum(c) && (xx = DIGIT(c)) < base; ) {
		if (val > multmax)
			goto overflow;
		val *= base;
		if (ULONG_MAX - val < xx)
			goto overflow;
		val += xx;
		c = *++str;
	}
	if (ptr != (const char **)0)
		*ptr = str;
	return (val);

overflow:
	if (ptr != (const char **)0)
		*ptr = str;
	yyerror("value out of range");
	return(ULONG_MAX);
}
