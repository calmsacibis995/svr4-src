/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident "@(#)acpp:common/group.c	1.18"
/* group.c - conditional inclusion directive groups */

#include <memory.h>
#include <stdio.h>
#include "cpp.h"
#include "buf.h"
#include "file.h"
#include "group.h"
#include "syms.h"

#if defined(DEBUG) && defined(__STDC__)
#	define	DBGCALL(funcname)	\
		if (DEBUG('g') > 0)	\
			(void)fprintf(stderr, #funcname"(tp=%#lx)\n", tp)
#	define	DBGRET(funcname)	\
		if (DEBUG('g') > 1)	\
		{			\
			(void)fprintf(stderr, #funcname" returns\n" ); \
			prifgroup();	\
		}
#else 
#	if defined(DEBUG)
#		define	DBGCALL(funcname)	\
			if (DEBUG('g') > 0)	\
				(void)fprintf(stderr, "funcname(tp=%#lx)\n", tp)
#		define	DBGRET(funcname)	\
			if (DEBUG('g') > 1)	\
			{			\
				(void)fprintf(stderr, "funcname returns\n", tp); \
				prifgroup();	\
			}
#	else
#		define	DBGCALL(funcname)
#		define	DBGRET(funcname)
#	endif
#endif
#define	STACKSZ	10

/* This file maintains a data structure that keeps track of the
** if-groups formed by conditional inclusion directives
** (#if[[n]def], #else, #elif, and #endif). The syntax of each
** directive is checked by a function in this file, which also
** operates on the data structure and checks that the syntax
** of an if-group has not been violated.
*/
typedef struct _section_ {
	FILE*	file;		/* file that this group is in	*/
	long	lineno;		/* line number of #if[n[def]]	*/
	char	current;	/* boolean: currently true?	*/
	char	hadtrue;	/* boolean: have been true?	*/
	char	hadelse;	/* boolean: had a #else?	*/
	char	dir;		/* directive that began if-group*/
} Section;

#ifdef __STDC__
enum {	/* values for Section.dir */
	D_if,
	D_ifdef,
	D_ifndef
};
#else	/* 2.1 enum members can't be assigned to int objects */
#	define	D_if		0
#	define	D_ifdef		1
#	define	D_ifndef	2
#endif

static char *	sayif[] = {
	"#if",
	"#ifdef",
	"#ifndef"
};

/* these 3 variables maintain the dynamic if-group stack */
static Section*	section;	/* pointer to base of stack */
static int	sx;		/* current if-group array index */
static int	sxlimit=STACKSZ;/* limit to `sx' */

/* As a speed optimization, this variable is maintained
** as always equal to section[sx].current.
** Because acpp queries the current if-group status on almost every
** logical line, the gain in accessing this object
** instead of accessing an offset of a stack pointer
** more than pays for the effort to maintain this extra
** variable when processing the (relatively) infrequent
** conditional inclusion directives.
*/
static int gr_current;	/* section[sx].current */

static Section* newifgroup( /* void */ );
#ifdef DEBUG
static	void	prifgroup(  /* void */ );
#endif

void
gr_check()
/* Called when the end of the current file has been reached, this routine
** checks for if-groups in the current file that are missing a #endif.
** It diagnoses any errors and then adjusts the state of the preprocessor
** as if the missing #endif's were present at the end of the file.
*/
{
	register FILE *	   file;	/* current file		*/
	register Section * sp;		/* current if_group	*/
	static	char	msg[52];	/* error message	*/

	sp = section + sx;
	file = fl_curfile();
	while (file == sp->file)
	{
		(void)sprintf(msg, "%s on line %ld has no matching #endif",
		 sayif[sp->dir], sp->lineno);
		UERROR(msg);
		sp--;
		sx--;
	}
	COMMENT(sx >= 0);
}

Token *
gr_elif(tp)
	register Token *tp;
/* Given a pointer to a Token list from a #elif directive, this routine
** checks the syntax of the Token list, records the prescence of the 
** directive and checks for violations of if-group syntax.
** A pointer to undeleted Tokens is returned.
*/
/* Truth Table:	("*" means any value)
** _____________________________________________________________________
** [sx-1].current	current	hadtrue xp_value() |	current	hadtrue	
** ________________________________________________|____________________
**	0		*	*	*	   |	0	0	
**	1		0	0	0          |	0	0	
**	1		0	0	1	   |	1	1	
**	1		0	1	0	   |	0	1	
**	1		0	1	1	   |	0	1	
**	1		1	0	0	   |	see NOTE	
**	1		1	0	1	   |	see NOTE
**	1		1	1	0	   |	0	1	
**	1		1	1	1          |	0	1	
** ________________________________________________|____________________
**
** NOTE: invalid state, not reached because hadtrue is set to 1 if
** current is 1.
*/
{
	register Section * sp;	/* currently active ifgroup	*/
	register int call;	/* boolean: was xp_value called?*/

	call = 0;
	sp = section + sx;
	DBGCALL(gr_elif);
	COMMENT((sp->current != 0 && sp->hadtrue == 0) == 0);
	if (sp->file != fl_curfile())
	{
		UERROR( "#elif has no preceding #if" );
		DBGRET(gr_elif);
		return tp;
	}
	if (tp == 0)
		WARN("#elif must be followed by a constant expression");
	if (sp->hadelse)
	{
		WARN( "#elif follows #else" );
		gr_current = sp->current = 0;
		DBGRET(gr_elif);
		return tp;
	}
	if (sp->current != 0)
		gr_current = sp->current = 0;
	else if ((sp-1)->current != 0 && sp->hadtrue == 0
		 && sp->current == 0 && (tp?call++,xp_value(tp):1))
		gr_current = sp->current = sp->hadtrue = 1;
	DBGRET(gr_elif);
	return call?(Token *)0:tp;
}

Token *
gr_else(tp)
	register Token *tp;
/* Given a pointer to a Token list from a #else directive, this routine
** checks the syntax of the Token list, records the prescence of the 
** directive and checks for violations of if-group syntax.
** A pointer to undeleted Tokens is returned.
*/
/* Truth Table:	("*" means any value)
** ____________________________________________________________
** [sx-1].current	current	hadtrue |	current	hadtrue	
** _____________________________________|______________________
**	0		*	*	|	0	0	
**	1		0	0	|	1	1	
**	1		0	1	|	0	1	
**	1		1	0	|	see NOTE	
**	1		1	1	|	0	1	
** _____________________________________|______________________
**
** NOTE: invalid state, not reached because hadtrue is set to 1 if
** current is 1.
*/
{
	register Section * sp;	/* currently active ifgroup */

	sp = section + sx;
	DBGCALL(gr_else);
	COMMENT((sp->current != 0 && sp->hadtrue == 0) == 0);
	if (sp->file != fl_curfile())
	{
		UERROR( "#else has no preceding #if" );
		DBGRET(gr_else);
		return (tp);
	}
	if ( sp->hadelse )
	{
		WARN( "too many #else's" );
		gr_current = sp->current = 0;
		DBGRET(gr_else);
		return tp;
	}
	if ( sp->current!=0 )
		gr_current = sp->current = 0;
	else if ( sp->hadtrue==0 && (sp-1)->current!=0)
		gr_current = sp->current = sp->hadtrue = 1;
	sp->hadelse = 1;
#ifdef __STDC__
	(pp_flags & F_Xc ? tk_extra : tk_rml)(tp);
#else
	if (pp_flags & F_Xc)
		tk_extra(tp);
	else
		tk_rml(tp);
#endif
	DBGRET(gr_else);
	return (Token *)0;
}

Token *
gr_endif(tp)
	Token *tp;
/* Given a pointer to a Token list from a #endif directive, this routine
** checks the syntax of the Token list, records the prescence of the 
** directive and checks for violations of if-group syntax.
** A pointer to undeleted Tokens is returned.
*/
{
	register Section * sp;	/* currently active ifgroup */

	sp = section + sx;
	DBGCALL(gr_endif);
	if ( sp->file != fl_curfile() )
		UERROR( "#if-less #endif" );
	else
	{
		sx--;
		sp--;
		gr_current = sp->current;
	}
#ifdef __STDC__
	(pp_flags & F_Xc ? tk_extra : tk_rml)(tp);
#else
	if (pp_flags & F_Xc)
		tk_extra(tp);
	else
		tk_rml(tp);
#endif
	COMMENT(sx >= 0);
	DBGRET(gr_endif);
	return (Token *)0;
}

Token *
gr_if(tp)
	register Token *tp;
/* Given a pointer to a Token list from a #if directive, this routine
** checks the syntax of the Token list, records the prescence of the 
** directive and checks for violations of if-group syntax.
** A pointer to undeleted Tokens is returned.
*/
/* Truth Table:
** _____________________________________________________
** [sx-1].current	xp_value() |	current	hadtrue	
** ________________________________|____________________
**	0		0	   |	0	0	
**	0		1	   |	0	0	
**	1		0	   |	0	0	
**	1		1	   |	1	1	
** ________________________________|____________________
*/
{
	register Section * sp;	/* currently active ifgroup	*/
	register int call;	/* boolean: was xp_value() called ? */

	call = 0;
	sp = newifgroup();
	DBGCALL(gr_if);
	COMMENT(sp->current == 0);
	COMMENT(sp->hadtrue == 0);
	COMMENT(sp->hadelse == 0);
	sp->lineno = bf_lineno;
	sp->file = fl_curfile();
	sp->dir = D_if;
	if ( tp == 0 )
		WARN("#if must be followed by a constant expression");
	if ( (sp-1)->current && (tp?call++,xp_value(tp):1) )
		gr_current = sp->current = sp->hadtrue = 1;
	DBGRET(gr_if);
	return (call?(Token *)0:tp);
}

Token *
gr_ifdef(tp)
	register Token *tp;
/* Given a pointer to a Token list from a #ifdef directive, this routine
** checks the syntax of the Token list, records the prescence of the 
** directive and checks for violations of if-group syntax.
** A pointer to undeleted Tokens is returned.
*/
/* Truth Table:
** _____________________________________________________
** [sx-1].current	st_ismacro()|	current	hadtrue	
** ________________________________|____________________
**	0		0	   |	0	0	
**	0		1	   |	0	0	
**	1		0	   |	0	0	
**	1		1	   |	1	1	
** ________________________________|____________________
*/
{
	register Section * sp;	/* currently active ifgroup	*/
	register Macro * mp;

	sp = newifgroup();
	DBGCALL(gr_ifdef);
	COMMENT(sp->current == 0);
	COMMENT(sp->hadtrue == 0);
	COMMENT(sp->hadelse == 0);
	sp->lineno = bf_lineno;
	sp->file = fl_curfile();
	sp->dir = D_ifdef;
	if (tp && tp->code == C_Identifier)
	{
		if ( (mp = st_ismacro(tp)) != 0 ) {
#ifdef CXREF
			if (pp_flags & F_CXREF)
				pp_xref(mp, tp->number, 'R');
#endif
			if ( (sp-1)->current != 0 )
				gr_current = sp->current = sp->hadtrue = 1;
		}
	}
	else
		WARN("#ifdef must be followed by an identifier");
	if (tp != 0)	tk_extra(tk_rm(tp));
	DBGRET(gr_ifdef);
	return (Token *)0;
}

Token *
gr_ifndef(tp)
	register Token *tp;
/* Given a pointer to a Token list from a #ifndef directive, this routine
** checks the syntax of the Token list, records the prescence of the 
** directive and checks for violations of if-group syntax.
** A pointer to undeleted Tokens is returned.
*/
/* Truth Table:
** _____________________________________________________
** [sx-1].current	st_ismacro()|	current	hadtrue	
** ________________________________|____________________
**	0		0	   |	0	0	
**	0		1	   |	0	0	
**	1		0	   |	1	1	
**	1		1	   |	0	0	
** ________________________________|____________________
*/
{
	register Section * sp;	/* currently active ifgroup	*/
	register Macro * mp;

	sp = newifgroup();
	DBGCALL(gr_ifndef);
	COMMENT(sp->current == 0);
	COMMENT(sp->hadtrue == 0);
	COMMENT(sp->hadelse == 0);
	sp->lineno = bf_lineno;
	sp->file = fl_curfile();
	sp->dir = D_ifndef;
	if (tp && tp->code == C_Identifier)
	{
		if ( (mp = st_ismacro(tp)) == 0 ) {
			if ( (sp-1)->current != 0 )
				gr_current = sp->current = sp->hadtrue = 1;
		}
#ifdef CXREF
		else {
			if (pp_flags & F_CXREF)
				pp_xref(mp, tp->number, 'R');
		}
#endif
	}
	else
	{
		WARN( "#ifndef must be followed by an identifier" );
		if ((sp-1)->current != 0)
			gr_current = sp->current = sp->hadtrue = 1;
	}
	if (tp != 0)	tk_extra(tk_rm(tp));
	DBGCALL(gr_ifndef);
	return (Token *)0;
}

void
gr_init()
/* Initializes group.c's data structures. */
/* The first Ifgroup corresponds to pre-directive text (that is never
** conditionally removed), so current is 1. The FILE* is set to 0
** (instead of fl_curfile()), so that gr_endif() will not allow an extra
** #endif in the original source file.
*/
{
	register Section * gp;	/* pointer to base of ifgroup stack */

	section = gp = (Section *)pp_malloc(sizeof(Section)*STACKSZ);
	gp->file = (FILE*)0;
	gr_current = gp->current = gp->hadtrue = 1;
	COMMENT(gp->dir?1:1);
	COMMENT(gp->lineno?1:1);
	COMMENT(gp->hadelse?1:1);
	COMMENT(sx == 0);
	COMMENT(sxlimit == STACKSZ);
}



int
gr_truepart()
/* Returns a boolean indication of whether the current if-group part
** is "true" (conditionally included) or not. If there is no
** currently active if-group
** (because there are no active #if[[n]def's), this routine will return "true".
*/
{
#ifdef DEBUG
	if ( DEBUG('g') > 2 )
		(void)fprintf(stderr,
		 "line %d:gr_truepart() returns section[%d].current=%d\n",
		 bf_lineno, sx, gr_current);
#endif
	return gr_current;
}

static Section *
newifgroup()
/* Returns a pointer to the next unused data structure that
** represents an if-group.
*/
{
	register Section * gp;	/* pointer to new ifgroup */

	if (++sx >= sxlimit)
	{
		sxlimit += STACKSZ;
		section = (Section *)pp_realloc((char*) section, sizeof(Section) * sxlimit);
	} 
	gp = section + sx;
	gr_current = gp->current = 0;
	gp->hadtrue = 0;
	gp->hadelse = 0;
	COMMENT(gp->lineno?1:1);
	COMMENT(gp->file?1:1);
	COMMENT(gp->dir?1:1);
	return gp;
}

#ifdef DEBUG
static void
prifgroup()
/* Prints debugging information about the current if-group. */
{
	(void)fprintf(stderr, "%d\n", section[sx].current);
}
#endif
