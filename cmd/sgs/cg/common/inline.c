/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nifg:cg/common/inline.c	1.7"
/* inline.c */


#include "mfile2.h"
#include <string.h>
#include <memory.h>
#include <malloc.h>
#include <ctype.h>

#ifdef	IN_LINE

/* Handling for new-style asm's:
**	The front-end defines a new-style asm in several
**	steps:
**	   1) as_start(name) names a new-style asm macro.
**		The name is assumed to be unique.
**	   2) The name of each parameter is passed, in
**		sequence, via asm_param().  Parameter names
**		are assumed unique for a given macro.
**	   3) as_e_param() marks the end of the parameters.
**	   4) Subsequently, the characters comprising the body
**		of the asm are passed via as_char().  The macro
**		may not contain NUL's, but otherwise has no
**		restrictions.
**	   5) The end of the macro definition is marked by a
**		call to as_end().
**
** The following assumptions apply to this implementation:
**	1) asm's are an extra-cost frill:  how quickly they are
**	handled is not a major issue.
**	2) asm's occur infrequently.
**	3) Few asm's are defined.
**	4) asm's are small.
**
** Based on these assumptions, this implementation is relatively
** simple-minded in its storage management:
**
**	1) Definitions are strung together on a singly-linked list,
**	stored in memory, and the list is searched linearly, last-in
**	first-found.
**	2) Storage is allocated for reasonable-guess first tries and
**	is reallocated as necessary.
**	3) The body of the macro is not pre-processed in any way,
**	particularly to identify parameter names.
*/



int asmdebug;				/* debugging flag */

#ifndef	ASM_LABEL			/* generated label format */
#define	ASM_LABEL	".ASM%d"
#endif

#ifndef	DPRINTF
#define DPRINTF printf
#endif


#ifndef	UERROR
#define UERROR uerror
#endif


static void as_output();
static void as_expand();

#ifndef	INI_N_MACARG
#define	INI_N_MACARG	2	/* initially expected number of macro arguments */
#endif

#ifndef	INI_MACBUFSIZE
#define	INI_MACBUFSIZE	64	/* initial size to hold macro text */
#endif

/* Anticipate ANSI C:  do malloc or realloc, as appropriate */
#define	MYREALLOC(ptr,size)	myrealloc((myVOID *) ptr, (unsigned int) size)

/* This structure contains the definition information for an
** asm.  These structures are linked together LIFO.
*/
typedef struct as_elem {
    char * as_name;		/* pointer to asm's name string */
    struct as_elem * as_next;	/* pointer to next asm on chain */
    char ** as_param;		/* pointer to list of parameter string ptrs */
    int as_nparam;		/* number of named parameters */
    char * as_text;		/* body of the asm macro */
} AS_ELEM;

static AS_ELEM * as_list = (AS_ELEM *) 0;
				/* asm list head */
static AS_ELEM * as_current;	/* pointer to current asm, defining or expanding */
static int as_textsize;		/* current size of macro text -- defining */
static int as_textmax;		/* current maximum size of macro text */


/* An implementation of inline asm expansion with arguments.
** 'asm' pseudo-function definitions, whose bodies contain code 
** to be expanded into the assembly code output depending on the
** storage locations of their arguments, are recognized by the
** compiler front end and stored away.
**
** When asm function calls are expanded in-line,
** arguments are evaluated left-to-right if they're
** not LTYPEs (leaf types, i.e. easy addressing modes).
** These complex trees are rewritten into TEMPs.
**
*/

void
as_gencode(p, outfile)
register ND2 *p;
FILE * outfile;
/* Expand the INCALL pointed to by p.  In this implementation, force
** all arguments into leaf nodes.  Write output to outfile.
** Recursive asm calls are prohibited.
*/
{
	static int expanding = 0;	/* non-zero if currently expanding */
	static void genfargs();

	if (expanding) {
	    UERROR("nested asm calls not now supported");
	    return;
	}
	expanding = 1;

	/* Check for a sane tree. */
	if (   (p->op != INCALL && p->op != UNARY INCALL)
	    || p->left->op != ICON	/* assume explicit name */
	    || p->left->lval != 0
	)
	    cerror("bad asm tree, as_gencode()");

	if (p->op == INCALL)
	    genfargs(p->right);		/* make sure ARG trees are okay */

	as_expand(p, outfile);
	expanding = 0;
	return;
}


static void
genfargs(p)
register ND2 *p;
/* Generate code to force arguments into leaf nodes. */
{
    ND2 *aop, *l, *r;
    int i, flag;
    int is_comop = 0;

    switch (p->op) {
    case CM:
	genfargs(p->left);
	genfargs(p->right);
	return;
    case COMOP:		/* assume the top COMOP has ARG on the right */
	r = p->right;
	p->right = r->left;
	r->op = FREE;
   	is_comop = 1;
	break;
    case FUNARG:
    case STARG:
     	/* want the left subtree of the ARG node */
	p = p->left;
	break;
    default:
	cerror("confused genfargs()");
    }

    /* had better be an evaluable expression */
    if (optype(p->op) != LTYPE)		/* don't bother if it's a leaf */
    {
	do {
	    /* Attempt to rewrite tree into a suitable form.
	    ** Bubble ,OPs to the top.
	    */
	    rewcom( (NODE *) p, NRGS );

#ifndef	STINCC
	    if( costs((NODE *) p) ) continue;

	    nins = 0;
	    insout((NODE *) p,NRGS);
#else
	    nins = 0;
	    switch (INSOUT( (NODE *) p, NRGS ))
	    {
	    case REWROTE:
		continue;		/* tree was rewritten */

	    case OUTOFREG:
		e2print( (NODE *) p );
		cerror("In-line runs out of registers\n");
	    /* default:  fall thru:  result is in register */
	    }
#endif

	    /* Check whether any of the instructions needed to
	    ** get the argument into a register required a rewrite
	    ** to TEMP.  If so, rewrite and try again.
	    */
	    flag = 0;
	    for( i=0; i<nins; ++i)
	    {
		if( inst[i].goal == CTEMP )
		    if( rewsto( inst[i].p ) ) 
			flag = 1;
	    }

	} while (flag);

	/* Generate instructions to get ARG into register. */
	insprt();

	/* Have result in a register.  Create a tree storing it to
	** a TEMP, and copy the TEMP into the argument tree.
	** Musical nodes:
	** Create a TEMP for the lhs of the store, copy the register 
	** result in p into the rhs, copy the TEMP into the argument
	** tree, then fill in the assignment node and produce
	** code for the store.
	*/
	l = (ND2 *) talloc();
	l->op = TEMP;
	l->lval = freetemp( argsize((NODE *) p)/SZINT );
	l->lval = BITOOR( l->lval );
	l->name = (char *) 0;
	l->type = p->type;

	r = (ND2 *) talloc();
	*r = *p;
	*p = *l;

	aop = (ND2 *) talloc();
	aop->op = ASSIGN;
	aop->left = l;
	aop->right = r;
	aop->type = p->type;

#ifndef	STINCC
	costs(aop);
	nins = 0;
	insout(aop,CEFF);
#else
	nins = 0;
	if (INSOUT((NODE *) aop, CEFF))	/* expect zero return */
	    cerror("In-line fails to assign to TEMP\n");
#endif
	/* Generate the assignment. */
	insprt();

	/* If COMOP, put back the ARG node for counting argno in
	 * as_doargs().
	*/
	if (is_comop) {
	    l = (ND2 *) talloc();
	    *l = *p;
	    p->op = FUNARG;
	    p->left = l;
	}
    }
    return;
}



static char *
as_save(s, len)
char * s;
unsigned int len;
/* Permanently save characters pointed to by s of length len.
** Return a NUL-terminated string that contains the characters.
** Be really simple-minded:  allocate space with malloc and
** copy in.
*/
{
    char * new = (char *) malloc(len+1);
    if (!new)
	cerror("as_save() out of space");
    
    (void) memcpy(new, s, len);
    new[len] = 0;			/* NUL terminate */
    return( new );
}


static myVOID *
myrealloc(p, size)
myVOID * p;
unsigned int size;
/* (Re)allocate a block of storage of size "size" for "p".  If "p"
** is null, just allocated.  Complain if no memory is allocated.
*/
{
    p = p ? realloc(p,size) : malloc(size);
    if (!p)
	cerror("myrealloc failed");
    return( p );
}


void
as_start(name)
char * name;
/* Mark start of asm function whose name is "name".
** Set up an AS_ELEM structure, fill in the name and initialize
** information.
*/
{
    register AS_ELEM * asp;

    if (as_current)		/* shouldn't be defining or expanding */
	cerror("as_start() confused");

    /* Allocate space for new macro element, fill it in. */
    asp = as_current = (AS_ELEM *) malloc(sizeof(AS_ELEM));
    if (! asp)
	cerror("out of space defining asm %s", name);
    
    asp->as_name = as_save(name, strlen(name));
    asp->as_nparam = 0;
    asp->as_text = (char *) 0;
    asp->as_param = (char **) 0;
    return;
}


void
as_param(name)
char * name;
/* Declare next parameter name for asm function.  Enlarge the
** array that contains parameter names, as necessary.
*/
{
    register AS_ELEM * asp = as_current;

    if (! asp)
	cerror("as_param() sequence");

    /* Check whether to expand the array of macro argument names.
    ** Size increases linearly in INI_N_MACARG.
    */
    if ((asp->as_nparam % INI_N_MACARG) == 0) {
	unsigned int newsize = (asp->as_nparam + INI_N_MACARG) * sizeof(char *);
	asp->as_param = (char **) MYREALLOC(asp->as_param, newsize);
    }
    asp->as_param[asp->as_nparam] = as_save(name, strlen(name));
    ++asp->as_nparam;
    return;
}


void
as_e_param()
/* Mark end of asm function parameters.  Prepare for asm text. */
{
    if (!as_current)
	cerror("as_e_param() sequence");

    /* Prepare for asm text. */
    as_textsize = 0;
    as_textmax = 0;
    return;
}


void
as_putc(c)
char c;
/* Get next asm character from front-end.  Save in text for
** macro, enlarge buffer as necessary.
*/
{
    if (! as_current)
	cerror("as_putc() sequence");

    /* Make sure current buffer is big enough. */
    if (as_textsize >= as_textmax) {
	as_textmax += INI_MACBUFSIZE;
	as_current->as_text = MYREALLOC(as_current->as_text, as_textmax);
    }
    as_current->as_text[as_textsize] = c;
    ++as_textsize;
    return;
}


void
as_end()
/* End of current asm definition.  Tidy up the details for the current
** definition, link on list, prepare for next asm.
*/
{
    if (! as_current)
	cerror("as_end() sequence");

    as_putc(0);				/* terminate text portion of macro */
    as_current->as_next = as_list;
    as_list = as_current;
    as_current = (AS_ELEM *) 0;		/* for internal checking */

#ifndef	NODBG
    if (asmdebug) {
	int paramno;
	register AS_ELEM * asp = as_list;
	DPRINTF("ASM macro %s(", asp->as_name);
	for (paramno = 0; paramno < asp->as_nparam; ++paramno) {
	    if (paramno > 0)
		DPRINTF(",");
	    DPRINTF("%s", asp->as_param[paramno]);
	}
	DPRINTF(")\nBODY:\n%sEND\n", asp->as_text);
    }
#endif
    return;
}


/* These definitions pertain to expansion data structures. */

/* This structure contains per-actual information.  The names
** are duplicated because the particular expansion could
** introduce new names in the form of labels.
*/
typedef struct {
    ND2 * as_node;			/* node to expand when replacing formal */
    int as_class;			/* pseudo-class of symbol */
    char * as_name;			/* copy of name string pointer */
} AS_EXPAND;

#define	AC_NONE	0			/* no storage class */
#define	AC_UREG	1			/* user register */
#define	AC_TREG	2			/* temporary (scratch) register */
#define	AC_REG	3			/* any register */
#define	AC_ICON	4			/* integer constant */
#define	AC_MEM	5			/* any memory location */
#define	AC_LABEL 6			/* generated label */
#define	AC_ERROR 7			/* %error */
/* label is denoted by negative number -- the label number -- in the
** structure
*/


static void
as_expand(p, file)
ND2 * p;
FILE * file;
/* Expand the INCALL or UNARY INCALL node pointed to by p.
** Output text directly to "file".  Assume that address modes
** may be output via adrput().
**
** Processing takes these phases:
**	1) Find the macro definition.  (Fatal error if not found.)
**	2) Search the definition for a % line that matches the
**		incoming arguments.
**	3) Expand lines, replacing formals with actuals, until
**	another %, or a NUL, is encountered.
**
*/
{
    AS_ELEM * asp;
    AS_EXPAND * asargs;
    static int as_doargs();
    static int as_findmatch();
    int argno;
    int nargs;				/* number of actuals */

#ifndef	NODBG
    if (asmdebug > 1)
	DPRINTF("in as_expand()\n");
#endif

    /* Find the named asm. */
    for (asp = as_list; asp; asp = asp->as_next) {
	if (strcmp(asp->as_name, p->left->name) == 0)
	    break;
    }

    if (!asp)
	cerror("unknown asm %s", p->left->name);


    /* Prepare to find actuals for the macro.  Allocate argument block
    ** one larger than declared number of formals.
    */
    asargs = (AS_EXPAND *)
	calloc((unsigned int) asp->as_nparam+1, sizeof(AS_EXPAND));
    if (! asargs)
	cerror("can't get arg block, as_expand()");;
    
    /* Fill in names of formals in argument block. */
    for (argno = 0; argno < asp->as_nparam; ++argno)
	asargs[argno].as_name = asp->as_param[argno];

    /* Insert arguments into asargs structure, remember number of
    ** actuals.
    */
    nargs = 0;				/* assume UNARY INCALL */
    if (p->op == INCALL)
	nargs = as_doargs(p->right, 0, asp->as_nparam, asargs) + 1;

#ifndef NODBG
    if (asmdebug > 1)
	DPRINTF("as_expand:  found; ready to match pattern\n");
#endif

    /* Try to expand macro.  If no matching set of arguments found,
    ** or if there are more actuals than formals, turn it into a
    ** CALL/UNARY CALL.  A macro that has no formals
    ** and that's called with no arguments always matches
    ** and always generates stuff from the beginning of the macro.
    */
    if (nargs > asp->as_nparam || ! as_findmatch(asp, asargs, file)) {
	if (asp->as_nparam == 0 && nargs == 0) {
	    asargs[0].as_name = (char *) 0;	/* no parameter names */
	    as_output(asp->as_text, asargs, file);
	}
	else
	    p->op = p->op == INCALL ? CALL : UNARY CALL;
    }
    free(asargs);			/* discard argument block */
    return;
}


static int
as_doargs(p, argno, maxargs, asargs)
ND2 * p;
int argno;
int maxargs;
AS_EXPAND * asargs;
/* Put the next argument into the appropriate place in asargs.
** Make sure the argument number isn't too large.
** Return the number of the argument we actually fill in.
** The code assumes that arguments cascade like this:
**
**		CM
**	       /  \
**	     CM	   ARG
**	    /  \    |
**	  ...	ARG tree
**	  /	 |
**	ARG	tree
**	 |
**	tree
*/
{
    /* Our argument number is 1 more than the children on the left. */
    if (p->op == CM) {
	argno = as_doargs(p->left, argno, maxargs, asargs) + 1;
	p = p->right;
    }
    
    if (argno < maxargs) {
	if (p->op != FUNARG && p->op != STARG)
	    cerror("confused as_doargs()");
	asargs[argno].as_node = p->left;
    }
    return( argno );
}


static int
as_findmatch(asp, asargs, file)
AS_ELEM * asp;
AS_EXPAND * asargs;
FILE * file;
/* Expand the macro whose AS_ELEM is asp.  Find the pattern in the
** macro definition that matches the actuals.  Use the
** (empty) argument block asargs for the argument information.
** p is the incoming expression tree.  file is the FILE * to write
** expanded text to.
** Return 1 if the macro has a storage class list that matches
** the incoming arguments.  (The macro is expanded.)  Return 0
** otherwise.
**
** The argument block (asargs) looks like this:
**	1)  There are at least number-of-formals entries at the top,
**		one for each formal.
**	2)  The argument tree that corresponds to each formal
**		is filled in.
**	3)  If the formal matches a % line class, the class
**		is filled in.
**	4)  If a % line matches successfully but doesn't
**		mention all formals, the corresponding formal
**		has class AC_NONE.  If there is no corresponding
**		actual, no substitution occurs for the name.  If
**		there IS a corresponding actual, an error
**		results only if the formal is used in the macro body.
**	5)  If a % line contains a "lab" class, the names that appear
**		after it get filled into the argument block as extra
**		names.  The class value is a unique negative value.
**		The absolute value is the label number.
**	6)  The end of the argument block is signified by null name
**		pointer.
*/
{
    static int as_patmatch();
    char * s = asp->as_text;		/* point at macro's text */

#ifndef NODBG
    if (asmdebug > 1)
	DPRINTF("in as_findmatch()\n");
#endif

    /* Loop through lines, looking for % that matches a winning
    ** pattern.
    */
    while (*s) {			/* continue while more characters */
	if (*s == '%') {		/* line that marks argument classes */
	    ++s;			/* skip % */
	    if (as_patmatch(&s, asp, &asargs)) {
#ifndef	NODBG
		if (asmdebug > 1)
		    DPRINTF("as_findmatch() found match\n");
#endif
		as_output(s, asargs, file);
		return( 1 );
	    }
	}
	else {				/* line with no % */
	    if (!(s = strchr(s, '\n')))	/* find end of current line */
		break;			/* can't find new line; done */
	    ++s;			/* skip to beginning of next line */
	}
    }
    return( 0 );			/* no match */
}

static int
as_patmatch(ps, asp, pasargs)
char ** ps;
AS_ELEM * asp;
AS_EXPAND ** pasargs;
/* Attempt to match the line that ps points to against the
** trees pointed to in pasargs.  Return 1 on match, 0 on
** no match.  The argument block pointed to by pasargs gets
** updated if there are label parameters.  The line pointer
** that ps points to gets updated to point to the beginning
** of the next line.
**
** The format of the pattern line is supposed to be
**	[%] class name[,name][; class name[,name]]
*/
{
    int i;
    char * s = *ps;
    char * end = strchr(s, '\n');	/* end of this line */
    int retval = 0;			/* assume failure */
    static int as_getclass();

#ifndef	NODBG
    if (asmdebug > 1)
	DPRINTF("in as_patmatch()\n");
#endif

    /* Update the caller's string pointer.  Point past newline or
    ** at null if no more newlines.
    */
    *ps = end ? end+1 : "";

    /* Initialize the argument block so there are no matches,
    ** and there are no labels.
    */
    for (i = 0; i < asp->as_nparam; ++i)
	(*pasargs)[i].as_class = AC_NONE;
    (*pasargs)[i].as_name = (char *) 0;

    if (end)
	*end = 0;			/* simplifies end check */

#ifndef	NODBG
    if (asmdebug > 1)
	DPRINTF("as_patmach():  try to match %s\n", s);
#endif

    /* Fixed point:  ready for a storage class. */
    for (;;) {
	if (!as_getclass(&s,pasargs))
	    break;			/* mismatch */
	
	while (isspace(*s))
	    ++s;

	if (*s != ';') {		/* error or done */
	    if (*s)
		UERROR("error in asm; expected \";\" or \"\\n\", saw '%c'",
								*s);
	    else
		retval = 1;		/* success */
	    break;
	}
	/* Saw ;.  Is there more on this line? */
	do {
	    ++s;
	} while (isspace(*s));
	if (*s == 0) {
	    retval = 1;			/* success and exit loop */
	    break;
	}
    }
    if (end)
	*end = '\n';			/* restore newline at end */

    return( retval );			/* return success indicator */
}


static int
as_getclass(ps, pasargs)
char ** ps;
AS_EXPAND ** pasargs;
/* Scan over storage class specifier and formal names in 
** current line.  The line is terminated by a 0.
** Update the argument block with the class information.
** On success, update the line pointer to point to the next
** character.
** Check for double-specification.
** Return 1 if the storage class specifier matches the tree(s),
** 0 otherwise.
*/
{
    char savechar;
    char * start;
    unsigned int namelen;
    int class;
    AS_EXPAND * curarg;
    static int labno = 0;
    char * s = *ps;

    while (isspace(*s))
	++s;
    
    if (!isalpha(*s)) {
	if (*s)
	    UERROR("unexpected character in asm %% line: '%c'", *s);
	return( 0 );
    }

    start = s++;
    /* Get the rest of a possible class (all alpha). */
    while (isalpha(*s))
	s++;
    
    /* Identify the class. */
    class = AC_NONE;
    savechar = *s;
    *s = 0;
    if      (strcmp(start, "reg") == 0)
	class = AC_REG;
    else if (strcmp(start, "ureg") == 0)
	class = AC_UREG;
    else if (strcmp(start, "treg") == 0)
	class = AC_TREG;
    else if (strcmp(start, "mem") == 0)
	class = AC_MEM;
    else if (strcmp(start, "con") == 0)
	class = AC_ICON;
    else if (strcmp(start, "lab") == 0)
	class = AC_LABEL;
    else if (strcmp(start, "error") == 0)
	class = AC_ERROR;
    *s = savechar;

#ifndef	NODBG
    if (asmdebug > 1)
	DPRINTF("as_getclass() saw class %d\n", class);
#endif

    if (class == AC_NONE) {
	UERROR("invalid class in asm %% line: %.*s", s-start, start);
	return( 0 );
    }
    else if (class == AC_ERROR) {
	UERROR("%%error encountered in asm function");
	return( 0 );
    }

    /* Ready for a list of identifiers. */

    for ( ; ; ) {
	/* Now eat spaces up to identifiers. */
	while (isspace(*s))
	    ++s;
	
	if (!(isalpha(*s) || *s == '_')) {
	    UERROR("missing formal name in %% line");
	    return( 0 );
	}
	start = s;
	while (++s, (isalnum(*s) || *s == '_'))
	    ;

	namelen = s-start;

	/* There are two cases:  either the name must be
	** a bonafide parameter, or it may be a new label
	** parameter.
	*/
	for (curarg = *pasargs; ; ++curarg) {
	    if (! curarg->as_name) {
		/* Reached end of list of names.  This is okay
		** if the class is label (and we have to expand
		** the list) but not for other classes.
		*/
		if (class == AC_LABEL) {
		    int curindex = curarg - *pasargs;
		    /* Need to add another null pointer at end. */
		    curarg->as_name = as_save(start, namelen);
		    curarg->as_class = --labno;
		    *pasargs = (AS_EXPAND *)
			MYREALLOC(*pasargs, (curindex+2) * sizeof(AS_EXPAND));
		    (*pasargs)[curindex+1].as_name = (char *) 0;
		    break;
		}
		else {
		    UERROR("name in asm %% line is not a formal: %.*s",
							namelen, start);
		    return( 0 );
		}
	    }
	    else if (   strncmp(curarg->as_name, start, namelen) == 0
		     && curarg->as_name[namelen] == 0
	    ) {				/* name found in list */
		if (class == AC_LABEL || curarg->as_class != AC_NONE) {
		    UERROR("duplicate name in %% line specification: %.*s",
							namelen, start);
		    return( 0 );
		}
		else if (! curarg->as_node) {
		    UERROR("no actual for asm formal: %.*s", namelen, start);
		    return( 0 );
		}

		/* Check whether chosen class matches tree. */
		switch( class ) {
		    int regno;
		case AC_UREG:
		case AC_TREG:
		case AC_REG:
		    if (   curarg->as_node->op != REG
			|| ((regno = curarg->as_node->rval),
			   class == AC_UREG && istreg(regno))
			|| class == AC_TREG && !istreg(regno)
			)
			return( 0 );		/* not suitable register */
		    break;
		case AC_ICON:
		    if (curarg->as_node->op != ICON)
			return( 0 );
		    break;
		case AC_MEM:
		    break;		/* matches all */
		default:
		    cerror("asm storage class?");
		}
		/* Tree matches class. */
		curarg->as_class = class;	/* node assumes this class */
		break;
	    }
	} /* end for */
	while (isspace(*s))
	    ++s;
	if (*s != ',') break;			/* no further names, this class */
	++s;					/* skip over , */
    }
    *ps = s;
    return( 1 );				/* announce success */
}


static void
as_output(s, asargs, file)
char * s;
AS_EXPAND * asargs;
FILE * file;
/* Output the macro text pointed to by s.  asargs points to
** the argument block for the macro.  The basic idea here is
** to walk through the text, trying to match identifiers against
** argument names.  When they match, substitute the compiler's
** idea of an address mode in place of the identifier.
** Output gets written to file.
*/
{
    char * lastout = s;			/* output up to this point so far */

#ifdef	ASM_COMMENT
    (void) fprintf(file, "%s\n", ASM_COMMENT);
#endif

    for (;;) {
	AS_EXPAND * argp;
	int namelen;
	
	/* Try to find identifier.  Write characters up to it.  Stop
	** if we find a % that follows a newline:  it's the start of
	** a new pattern and replacement.
	*/
	for ( ; *s; ++s) {
	    if (isalnum(*s) || *s == '_')
		break;
	    if (*s == '%' && s[-1] == '\n')
		break;
	}

	if (s != lastout) {
	    (void) fwrite(lastout, sizeof(char), s-lastout, file);
	    lastout = s;
	}
	/* Current character is NUL, % (following newline),
	** alphanumeric, or _.
	*/
	if (*s == 0 || *s == '%')
	    break;			/* done expanding */

	while (isalnum(*s) || *s == '_')
	    ++s;
	
	namelen = s-lastout;
	for (argp = asargs; argp->as_name; ++argp) {
	    if (   strncmp(argp->as_name, lastout, namelen) == 0
		&& argp->as_name[namelen] == 0
		) break;		/* we matched a formal */
	}

	/* Name is an actual if it matches the name of a formal
	** and it was mentioned in a % specification line,
	** signified by a non-NONE class.
	*/
	if (argp->as_name && argp->as_class != AC_NONE) {
	    if (argp->as_class < 0)	/* label number */
		fprintf(file, ASM_LABEL, -argp->as_class);
	    else
		adrput((NODE *) argp->as_node);
	    lastout = s;		/* consider name as being output */
	}
    }

#ifdef	ASM_END
    (void) fprintf(file, "%s\n", ASM_END);
#endif
    return;
}

#endif	/* def IN_LINE */
