/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


%{
#ident	"@(#)iconv:gram.y	1.1.1.1"

#include <stdio.h>
#include "symtab.h"
#include "kbd.h"
char *strsave(), *spack();
struct node *sortroot();

extern char *gettxt();
extern int nerrors;
extern unsigned char oneone[];	/* for one-one mapping */
extern int oneflag;
struct kbd_map maplist[30];
int curmap = -1;		/* CURRENT map entry */
struct sym *curparent;
char *childlist[5];	/* max args is 5 (not used!) */
int childp;			/* pointer to next in childlist */
int inamap = 0;
int fullflag = 0;
int timeflag = 0;
extern char textline[];
extern int linnum;	/* current line number */
extern struct node *root;
extern int numnode;	/* number of nodes in current map */

%}

/*
 * kbdcomp/gram.y	1.0 AT&T UNIX PACIFIC 1988/08
 *
 *	Statements:
 *
 *		map STYLE (name) {	<-- top level statement
 *			<expressions for the map>
 *		}
 *
 *		STYLE is "full" or "sparse" type
 *
 *		define(WORD STRING)	define WORD to produce STRING
 *
 *		WORD(x y)	func WORD + x yields y (many-many mapping)
 *
 *		keylist(x y)	bytes in x[i] become y[i] (one-one mapping)
 *				Done by lookup table.
 *
 *		string(x y)	NOT PART OF GRAMMAR.  Replaces "x" with "y".
 *				x & y may be one or more bytes.  The word
 *				"string" is pre-defined to be a NULL prefix.
 *				See "sym.c", the "s_init" routine.
 *
 *		strlist(x y)	as for "keylist", x[i] produces y[i]. The
 *				diff is that "strlist" does by tree; keylist
 *				does by lookup table.
 *
 *		error( string )	When module gets error, print string.
 *				(i.e., when a partial match does not end
 *				up succeeding.)
 *
 *		timed		Use timeout algorithm on the table.
 *
 *		link (STRING)	(top level statement) produces a link spec
 *
 *		extern (STRING)	Algorithm is external [RESERVED for future
 *				plans].
 */
%token MAP SPARSE FULL DEFINE STRING KEYLIST STRLIST TIMED NERROR LINK XTERN
%start prog

%%
prog		: /* empty */
		| map_list
		;

map_list	: map_list map
		| map
		;

map		: MAP
			{	fullflag = 0; }

		  style '(' STRING
			{
if (inamap) {
fprintf(stderr, gettxt("kbdcomp:1", "Error: 'map' used inside a map, line %d\n"), linnum);
exit(1);
}
				++curmap;
				maplist[curmap].mapname = strsave(textline);
if (! legalname(maplist[curmap].mapname)) {
fprintf(stderr, gettxt("kbdcomp:2", "Error: \"%s\": illegal map name, line %d\n"),
	maplist[curmap].mapname, linnum);
	++nerrors;
}
				maplist[curmap].maproot = (struct node *) 0;
				maplist[curmap].mapone = NULL;
				maplist[curmap].maperr = NULL;
				maplist[curmap].map_min = 0;
				maplist[curmap].map_max = 256;
				oneflag = 0;
				timeflag = 0;
			}
		  ')'
		  	{ inamap = 1; }
		  '{' expr_list '}'
			{	inamap = 0;
				numnode = 0;
				if (fullflag)
					root = sortroot(root);	/* CAREFUL! */
				numtree(root);
				/*		showtree(root); */
				buildtbl(root);
			 }
		| LINK '(' STRING 
			{
				/*
				 * Simple linkage, just stuff it in the
				 * structure.  It will get output when
				 * the rest of them do.
				 */
				++curmap;
				maplist[curmap].mapname = LINKAGE;
				maplist[curmap].maproot = (struct node *) 0;
				maplist[curmap].mapone = NULL;
				maplist[curmap].maperr = NULL;
				maplist[curmap].map_min = 0;
				maplist[curmap].maptext = (unsigned char *) spack(textline);
				maplist[curmap].map_max = strlen(maplist[curmap].maptext) + 1;
if (! legallink(maplist[curmap].maptext)) {
fprintf(stderr, gettxt("kbdcomp:3", "Error: \"%s\": illegal linkage spec, line %d.\n"),
	maplist[curmap].maptext, linnum);
	++nerrors;
}
				buildtbl(LINKAGE);
			}
		  ')'
		| XTERN '(' STRING
			{
				fprintf(stderr,
				  gettxt("kbdcomp:4", "Error: \"extern\" is a reserved word.\n"));
				++nerrors;
			}
		 ')'
		;

style		:	/* empty */
		| SPARSE
			{ fullflag = 0; }
		| FULL
			{ fullflag = 1; }
		;
expr_list	: expr_list expr
		| expr
		;

expr		: DEFINE '(' STRING
			{
				if (s_find(textline)) {
				  fprintf(stderr,
				  gettxt("kbdcomp:5", "Error: redefinition of %s, line %d\n"),
				  textline, linnum);
				  ++nerrors;
				}
				curparent = s_lookup(textline);
				curparent->s_type = S_FUNC;
				childp = 0;
			}
		  STRING
			{
				curparent->s_value = strsave(textline);
			}
		  ')'

		| STRING
			{
/*
 * A function call:
 * For these guys, the func is the name, the "values" are the parameters.
 */
			if (!(curparent = s_find(textline))) {
				fprintf(stderr,
				gettxt("kbdcomp:6", "Error: Undefined function %s, line %d\n"),
					textline, linnum);
				++nerrors;
				curparent = s_create("orphan");
				curparent->s_type = S_FUNC;
				curparent->s_value = strsave("dummy");
			}
			childp = 0;
			}
		  '(' expr_tail
			{
			if (childp) {
				childlist[childp] =  (char *) 0;
				buildtree(curparent, &(childlist[0]));
			}
			else {
				fprintf(stderr,
				gettxt("kbdcomp:7", "Error: arguments to %s, line %d\n"),
					curparent->s_value, linnum);
				++nerrors;
			}
			}

		| STRLIST '('
			{
			if (!(curparent = s_find("string"))) {
				fprintf(stderr,
				gettxt("kbdcomp:8", "Error: Undefined function %s, line %d\n"),
					textline, linnum);
				++nerrors;
				curparent = s_create("orphan");
				curparent->s_type = S_FUNC;
				curparent->s_value = strsave("dummy");
			}
			childp = 0;
			}
		  expr_tail
		  	{
				do_strlist();
			}
		| KEYLIST '('
			{
			childp = 0;
			}
		  expr_tail
			{
			register int i;
			if (childp) {
				if (strlen(childlist[0]) != strlen(childlist[1])) {
					fprintf(stderr,
					gettxt("kbdcomp:9", "Error: `keylist' string length unequal, line %d\n"),
					linnum);
					++nerrors;
				}
				else { /* do one-one mapping for each */
				    if (! oneflag) {
					for (i = 0; i < 256; i++)
						oneone[i] = (unsigned char) i;
					oneflag = 1;
				    }
				    while (*childlist[0]) {
					    oneone[*childlist[0]] = *childlist[1];
					    ++(childlist[0]); ++(childlist[1]);
				    }
				}
			}
			else {
				fprintf(stderr,
				gettxt("kbdcomp:10", "Error: arguments to `keylist', line %d\n"),
					linnum);
				++nerrors;
			}
			}

		| NERROR '(' STRING 
			{ maplist[curmap].maperr = (unsigned char *)strsave(textline); }
		  ')'
			
		| TIMED
			{
				timeflag = 1;	/* default to timed mode */
			}
		;

expr_tail	: STRING
			{ childlist[childp++] = strsave(textline); }
		  STRING
			{ childlist[childp++] = strsave(textline); }
		  ')'
		| error
			{ fprintf(stderr, gettxt("kbdcomp:11", "Error: null expression, line %d\n"),
				linnum);
			  ++nerrors;
			  yyclearin; }
		;
%%

/* put functions here */

char bttmp[512];
extern struct node *root;
extern int dup_error;	/* "tree" may set on dup errors */

buildtree(par, ch)

	struct sym *par;
	char **ch;
{
	register int rval;
	register int i;
	register char *t;

/*
 * Better have 2 children!
 */
	bttmp[0] = '\0';
/*
 * "Par" is null for "key()" nodes
 */
	if (par)
		strcat(bttmp, par->s_value);
	strcat(bttmp, *ch);	/* bttmp contains prefix + suffix */
	if (strlen(bttmp) >= KBDIMAX) {
		fprintf(stderr, gettxt("kbdcomp:12", "Error: input string too long: %s\n"), bttmp);
		++nerrors;
	}
	++ch;	/* point at result */
	rval = tree(root, bttmp, *ch);
	if (rval == E_DUP) {
		fprintf(stderr, gettxt("kbdcomp:13", "Error: dup. ( "));
		t = bttmp;
		while (*t)
			prinval(*t++, 1);
		fprintf(stderr, gettxt("kbdcomp:14", ") on line %d"), linnum);
		if (dup_error)
			fprintf(stderr, gettxt("kbdcomp:15", " (already defined on/near line %d)"),
				dup_error);
		fprintf(stderr, gettxt("kbdcomp:16", "\n"));
		++nerrors;
	}
}

/*
 * A map name is illegal if it contains a comma, colon, or space.
 */

legalname(s)
	char *s;
{
	while (*s) {
		if (*s == ',' || *s == ':' || *s == ' ')
			return(0);
		++s;
	}
	return(1);	/* legal */
}

/*
 * A link-string is illegal if it doesn't follow the form:
 *		s colon x comma y [ comma z ...]
 * this is a simple check, and won't catch ALL bad formats, just some.
 * Assumes that spack() has been called on the string.
 */

legallink(s)
	char *s;
{
	register int colon, comma;

	if (! *s)
		return(0);
	colon = comma = 0;
	while (*s) {
		if (*s == ',' && ! colon)
			return(0);
		if (*s == ':' && comma)
			return(0);
		if (*s == ',')
			++comma;
		if (*s == ':')
			++colon;
		++s;
	}
	if (*(s-1) == ',')
		return(0);
	if (colon + comma == 0)
		return(0);
	if (comma < 1)
		return(0);
	return(1);
}

/*
 * Subroutine to do "strlist" stuff.  Make a string out of a byte
 * of each childlist; pass these in pairs to "buildtree".  Don't
 * pass a "parent".
 */

do_strlist()

{
	unsigned char x[2], y[2]; /* input strings */
	unsigned char *s[2];	  /* input string ptr array for buildtree */

	if (childp) {
		if (strlen(childlist[0]) != strlen(childlist[1])) {
			fprintf(stderr,
			gettxt("kbdcomp:17", "Error: `strlist' string length unequal, line %d\n"),
			linnum);
			++nerrors;
		}
		else { /* build one-one mapping nodes for each */
		    s[0] = (unsigned char *) &x[0];	/* input */
		    s[1] = (unsigned char *) &y[0];	/* result */
		    x[1] = y[1] = '\0';
		    while (*childlist[0]) {
			x[0] = *childlist[0];
			y[0] = *childlist[1];
			buildtree((struct sym *) 0, &s[0]);
			++(childlist[0]); ++(childlist[1]);
		    }
		}
	}
	else {
		fprintf(stderr,
		gettxt("kbdcomp:18", "Error: arguments to `strlist', line %d\n"),
			linnum);
		++nerrors;
	}
}
