/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)alint:common/hooks.c	1.22"
#include <stdio.h>
#include <ctype.h>
#include <malloc.h>
#include "p1.h"

FILE * outfile = stdout;
int db_symlevel, db_linelevel;
int odebug,sdebug,asmdebug,zflag;
void mkdope(), tinit();
unsigned int strlen();
#ifdef __STDC__
void *memcpy(), *memset();
#endif

/*
**      HOOKS
**
** below are hooks that the front end calls; they either code-generate or
** generate debugging information that is not useful to lint
**
*/

/* from nail.c */
/* ARGSUSED */
OFFSET
off_incr(space, o, n)
int space;
OFFSET o;
long n;
{
    return (o + BITOOR(n));
}

/* ARGSUSED */
off_is_err(space, o)
int space;
OFFSET o;
{
  return (1);
}

/* from reader.c */
/* ARGSUSED */
void
e222print(down, p, s)
NODE *p;
char *s;
{}

/* Same as in local2.c */
int 
getlab() 
{
    static int lab=1;
    LNBUG(ln_dbflag>4,("getlab"));
    return lab++;
}


/* Only do things "interesting" to lint. */
void
p2init()
{
    LNBUG(ln_dbflag>4,("p2init"));
    mkdope();
    tinit();
}


/*
** Most cg_* functions are ifdef'ed out for lint.  Replace with stubs
** unless the function does something interestint for lint.
*/

void
cg_begfile() {}

/* ARGSUSED */
void
cg_copyprm(sid)
SX sid;
{}

/* ARGSUSED */
char *
cg_extname(sid)
SX sid;
{
return (char *)0;
}

/* ARGSUSED */
void
cg_nameinfo(sid)
SX sid;
{}

void
cg_treeok()
{
}

void
cg_profile()
{
  LNBUG(ln_dbflag > 4, ("cg_profile"));
}


void
cg_asmold(p) 
ND1 *p; 
{
    LNBUG(ln_dbflag > 4, ("cg_asmold"));
    if (p->op != STRING)
	cerror("confused cg_asmold()");
    if (strlen(p->string) != p->lval)
	UERROR("embedded NUL not permitted in asm()");
    t1free(p);
    ln_ifflg = ln_elseflg = 0;
}

/* ARGSUSED */
ND1 *
cg_strinit(p, len, wantname, readonly)
ND1 * p;
SIZE len;
int wantname;
int readonly;
{
  ND1 *retval = ND1NIL;
  LNBUG(ln_dbflag>4,("cg_strinit"));

  if (wantname) {
	retval = t1alloc();
	retval->op = NAME;
	retval->type = p->type;
	retval->lval = 0;
	retval->rval = -1 * sm_genlab();

	if (len)
	    retval->type = ty_mkaryof(TY_CHAR, len);
  } 

  p->op = FREE;
  return( retval );
}

void
cg_eof(flag)
int flag;
{
  LNBUG(ln_dbflag>4, ("cg_eof"));
  if (flag == C_NOTOKEN)
	WERROR("empty translation unit");
  ln2_def();
}


void 
cg_incode(p,len)
ND1 *p; 
BITOFF len; 
{
    LNBUG(ln_dbflag>4, ("cg_incode"));

    if (((p->op == ICON && p->rval != ND_NOSYMBOL) || p->op == STRING) &&
	len < TY_SIZE(TY_VOIDSTAR))
	UERROR("invalid initializer");
    t1free(p);
}


/* Copied in from xdefs.c */

/* function to enlarge a table described by a table descriptor */

int
td_enlarge(tp,minsize)
register struct td * tp;
int minsize;				/* minimum size needed:  0 means 1 more
					** than current
					*/
{
    int oldsize = tp->td_allo;		/* old size (for return) */
    unsigned int ocharsize = tp->td_allo * tp->td_size; /* old size in bytes */
    int newsize;			/* new size in storage units */
    unsigned int ncharsize;		/* size of new array in bytes */

    /* Realloc() previously malloc'ed tables, malloc() new one.
    ** If "end" were part of the C library, a check would have been
    ** done on the current value of the pointer, instead of having a
    ** bit in the td flags.
    */

/*    printf("%s changes from	%#lx - %#lx\n", tp->td_name,
			tp->td_start, tp->td_start+ocharsize); */

    /* determine new size:  must be "large enough" */
    newsize = tp->td_allo;		/* start at old size */
    do {
	newsize *= 2;
    } while (newsize < minsize);	/* note:  always false for minsize==0 */
    ncharsize = newsize * tp->td_size;	/* size of new array in bytes */

    if (tp->td_flags & TD_MALLOC)
	tp->td_start = realloc(tp->td_start, ncharsize);
    else {
	myVOID * oldptr = tp->td_start;

	/* copy old static array */
	if ((tp->td_start = malloc(ncharsize)) != 0)  /*lint*/
	    (void)memcpy(tp->td_start, oldptr, ocharsize);
    }
    tp->td_flags |= TD_MALLOC;		/* array now unconditionally malloc'ed */

    if (!tp->td_start)
	cerror("can't get more room for %s", tp->td_name);
    
/*    printf("		to	%#lx - %#lx\n",
			tp->td_start, tp->td_start+ncharsize); */
    /* zero out new part of array:  node and symbol tables expect this */
    if (tp->td_flags & TD_ZERO)
	(void)memset((char *) tp->td_start + ocharsize,0,(ncharsize-ocharsize));

    tp->td_allo = newsize;
    return oldsize;
}





/* ARGSUSED */
OFFSET next_temp(t, s, a)  TWORD t; BITOFF s; int a; { return 0; }

/* ARGSUSED */
OFFSET next_arg(t, s, a) TWORD t; BITOFF s; int a; { return 0; }

OFFSET max_temp() { return 0; }

/* ARGSUSED */ 
OFFSET off_conv(s,o,f,t) int s; OFFSET o; TWORD f,t; { return o; }

/* ARGSUSED */ 
OFFSET off_bigger(s,o1,o2) int s; OFFSET o1, o2; { return o1 > o2 ? o1 : o2; } 

/* ARGSUSED */ 
cisreg(op, t, off, rbusy) int op; TWORD t; OFFSET off; char *rbusy; { return 1; }

/* ARGSUSED */ 
int tyreg(t) TWORD t; { return 1; } 

/* ARGSUSED */ 
void db_sue(t) T1WORD t; {}

/* ARGSUSED */ 
void db_begf(s) SX s; {}

/* ARGSUSED */ 
void db_symbol(s) SX s; {}

/* ARGSUSED */ 
void db_sy_clear(s) SX s; {}

/* ARGSUSED */ 
void db_s_file(s) char *s; {}

void db_e_fcode() {}
void db_e_file() {}
void db_s_fcode() {}
void db_endf() {}
void db_s_block() {}
void db_e_block() {}
void db_lineno() {}

/* ARGSUSED */ 
void set_next_arg(s) OFFSET s; {}

/* ARGSUSED */ 
void set_next_temp(s) OFFSET s; {}

void p2abort() {}

/* ARGSUSED */ 
void off_init(s) OFFSET s; {}

#ifdef IN_LINE
/* ARGSUSED */ void as_start(s) char *s; {}
/* ARGSUSED */ void as_param(s) char *s; {}
/* ARGSUSED */ void as_putc(c) char c; {}
void as_e_param() {}
void as_end() {}
#endif

/*
** From cgstuff.c
*/
/* ARGSUSED */ 
ND1 *cg_defstat(t) T1WORD t; { return ((ND1 *) 0); }

/* ARGSUSED */ 
void cg_ecode(p) ND1 *p; { t1free(p); }

/* ARGSUSED */ 
void cg_instart(a,b) T1WORD a; int b; {}

void cg_inend() {}

/* ARGSUSED */ 
void cg_defnam(s) SX s; {}

/* ARGSUSED */ 
void cg_zecode(f) BITOFF f; {}

/* ARGSUSED */ 
void cg_begf(s) SX s; {}

/* ARGSUSED */ 
void cg_endf(m) OFFSET m; {}

/* ARGSUSED */ 
void cg_bmove(s,p) SX s; ND1 *p; {}

/* ARGSUSED */ 
void cg_deflab(l) int l; {}

/* ARGSUSED */ 
void cg_goto(l) int l; {}

/* ARGSUSED */ 
void cg_filename(f) char *f; {}

/* ARGSUSED */ 
void cg_ident(i) char *i; {}
