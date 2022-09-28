/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)iconv:sym.c	1.1.1.1"

/*
 * symbol table manipulation routines.
 */
#include <stdio.h>
#include "symtab.h"
#include "kbd.h"

/*
 * Define a file that can be used for local "pre-defined"
 * symbols, in addition to "string".  If it's there, use it
 *
 */
#define PREDEF	"/usr/lib/kbd/kbd_sym"
#define ACCEPT	0	/* must be same as in lexan.c */

extern char *gettxt();
struct sym *s_head;	/* head of chain */
struct sym symtab;	/* dummy first element (assoc pointer) */
char *strsave(), *calloc();
extern char textline[];

/*
 * sym_init	Initialize the symbol table with a name.
 */

sym_init()

{
	struct sym *s_create();
	register struct sym *start;
	register int left, right;	/* tokens */
	register char *lstr, *rstr;	/* strings */
	extern FILE *lexfp;	/* WARNING: a re-use. It gets set
				   to the "real" file AFTER sym_init() */

	s_head = &symtab;
	s_head->s_name = strsave("GloBaL");
	s_head->s_type = S_NAME;
	s_head->s_value = (char *) 0;
	s_head->s_next = (struct sym *) 0;
	start = s_create("string");	/* create keyword "string"... */
	start->s_value = strsave("");	/* ...no value... */
	start->s_type = S_FUNC;		/* ...type func. */
	/*
	 * Look for pre-definitions, if any in the file PREDEF.
	 * Be aware that this code RE-USES "yylex()" to retrieve
	 * groups of strings from the definition file.  We're hoping
	 * it's just a file of strings, two per line.  We set things
	 * up to perform the equivalent of "define(left right)".
	 */
	if (lexfp = fopen(PREDEF, "r")) {	/* see if any pre-defs */
		while ((left = yylex()) != ACCEPT) {
			lstr = strsave(textline);
			if ((right = yylex()) == ACCEPT) {
				fprintf(stderr, gettxt("kbdcomp:47", "Warning: pre-definition entry mismatch; ignoring \"%s\"\n"), lstr);
				return;
			}
			rstr = strsave(textline);
			start = s_create(lstr);
			start->s_value = rstr;
			start->s_type = S_FUNC;
/* fprintf(stderr, "Predef: %s = %s\n", lstr, rstr); */
		}
		fclose(lexfp);
	}
}

/*
 * s_create	Create a new entry in the symbol table for name.
 */

struct sym *
s_create(name)

	char *name;
{
	struct sym *new_entry;

	new_entry = (struct sym *) calloc(1, sizeof(struct sym));
	if (new_entry) {
		new_entry->s_next = s_head->s_next;
		s_head->s_next = new_entry;
		new_entry->s_name = strsave(name);
		new_entry->s_value = (char *) 0;
		new_entry->s_type = UNDEF;
		return(new_entry);
	}
	fprintf(stderr, gettxt("kbdcomp:48", "Error: symbol table overflow (calloc failed!)\n"));
	exit(1);
}

/*
 * s_find	find a symbol by name, returning pointer to it.
 */

struct sym *
s_find(name)

	char *name;
{
	struct sym *ptr;

	/* search until match or end of chain */
	for (ptr = s_head->s_next; ptr; ptr = ptr->s_next) {
		if (! ptr->s_name)
			fprintf(stderr, gettxt("kbdcomp:49", "Internal error in s_find.\n"));
		else {
			/* return if match */
			if (strcmp(ptr->s_name, name) == 0) {
				return(ptr);
			}
		}
	}
	/* search failed */
	return (struct sym *) 0;
}

/*
 * s_lookup	- If a symbol can't be found, then make a new one
 *		  by that name.
 */

struct sym *
s_lookup(name)

	char *name;
{
	register struct sym *s;

	if (! (s = s_find(name)))
		s = s_create(name);
	return(s);
}

/*
 * strsave	Save a string with calloc.  The strings are the names
 *		or values of symbols.  Since calloc is used, and pointers
 *		are left in the symbol table entry, the length of names
 *		is virtually unlimited.
 */

char *
strsave(s)

	char *s;
{
	register char *cp;

	cp = calloc(strlen(s)+1, 1);
	if (cp) {
		strcpy(cp, s);
		return(cp);
	}
	fprintf(stderr, gettxt("kbdcomp:50", "Error: string-save overflow (calloc failed).\n"));
	exit(1);
}

/*
 * spack	Do a "strsave", but pack out spaces and tabs
 *		in the string while copying it.
 */

char *
spack(s)
	char *s;
{
	register char *cp, *t;

	cp = calloc(strlen(s)+1, 1);
	if (cp) {
		t = cp;
		while (*s) {
			if (*s != ' ' && *s != '\t')
				*t++ = *s;
			++s;
		}
		*t = '\0';
		return(cp);
	}
	fprintf(stderr, gettxt("kbdcomp:51", "Error: string overflow (calloc failed).\n"));
	exit(1);
}

#if 0
s_dump()
{
	struct sym *entry;
	char *s;

	fprintf(stderr, "SYMBOLS:\n");
	entry = s_head;
	while (entry) {
		fprintf(stderr, "%08X", entry);
		if (entry->s_type == S_SWTCH)
			fprintf(stderr, "; \\%03o            = ", *(entry->s_value));
		else
			fprintf(stderr, "; %-16s= ", entry->s_name);
		switch (entry->s_type) {
			case S_FUNC: s = "FUNC"; break;
			case S_PARM: s = "PARAM"; break;
			case S_NAME: s = "NAME"; break;
			case S_SWTCH:s = "SWTCH"; break;
			default:     s = "UNDEF ";
		}
		fprintf(stderr, "%-7s", s);
		if (entry->s_type != S_SWTCH)
			fprintf(stderr, "[%s]\n", entry->s_value);
		entry = entry->s_next;
	}
	fprintf(stderr, "\n");
}
#endif

struct sym *
s_value(s)	/* find symbol whose VALUE is "s" */

	char *s;
{
	struct sym *entry;

	entry = s_head;
	while (entry) {
		if (strcmp(entry->s_value, s) == 0)
			return(entry);
		entry = entry->s_next;
	}
	return((struct sym *) 0);
}
