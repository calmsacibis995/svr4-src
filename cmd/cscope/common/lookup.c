/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cscope:common/lookup.c	1.4"
/*	cscope - interactive C symbol cross-reference
 *
 *	keyword look-up routine for the C symbol scanner
 */

#include "global.h"

/* keyword text for fast testing of keywords in the scanner */
char	enumtext[] = "enum";
char	externtext[] = "extern";
char	structtext[] = "struct";
char	typedeftext[] = "typedef";
char	uniontext[] = "union";

/* This keyword table is also used for keyword text compression.  Keywords
 * with an index less than the numeric value of a space are replaced with the
 * control character corresponding to the index, so they cannot be moved
 * without changing the database file version and adding compatibility code
 * for old databases.
 */
struct	keystruct keyword[] = {
	"#define",	' ',	NULL,	/* must be table entry 0 */
	"#include",	' ',	NULL,	/* must be table entry 1 */
	"break",	'\0',	NULL,	/* rarely in cross-reference */
	"case",		' ',	NULL,
	"char",		' ',	NULL,
	"continue",	'\0',	NULL,	/* rarely in cross-reference */
	"default",	'\0',	NULL,	/* rarely in cross-reference */
	"do",		'\0',	NULL,
	"double",	' ',	NULL,
	"\t",		'\0',	NULL,	/* must be the table entry 9 */
	"\n",		'\0',	NULL,	/* must be the table entry 10 */
	"else",		' ',	NULL,
	enumtext,	' ',	NULL,
	externtext,	' ',	NULL,
	"float",	' ',	NULL,
	"for",		'(',	NULL,
	"goto",		' ',	NULL,
	"if",		'(',	NULL,
	"int",		' ',	NULL,
	"long",		' ',	NULL,
	"register",	' ',	NULL,
	"return",	'\0',	NULL,
	"short",	' ',	NULL,
	"sizeof",	'\0',	NULL,
	"static",	' ',	NULL,
	structtext,	' ',	NULL,
	"switch",	'(',	NULL,
	typedeftext,	' ',	NULL,
	uniontext,	' ',	NULL,
	"unsigned",	' ',	NULL,
	"void",		' ',	NULL,
	"while",	'(',	NULL,
	
	/* these keywords are not compressed */
	"auto",		' ',	NULL,
	"fortran",	' ',	NULL,
	"const",	' ',	NULL,
	"signed",	' ',	NULL,
	"volatile",	' ',	NULL,
};
#define KEYWORDS	(sizeof(keyword) / sizeof(struct keystruct))

#define HASHMOD	(KEYWORDS * 2 + 1)

static	struct	keystruct *hashtab[HASHMOD]; /* pointer table */

/* put the keywords into the symbol table */

void
initsymtab()
{
	register int	i, j;
	register struct	keystruct *p;
	
	for (i = 0; i < KEYWORDS; ++i) {
		p = &keyword[i];
		j = hash(p->text) % HASHMOD;
		p->next = hashtab[j];
		hashtab[j] = p;
	}
}

/* see if this identifier is a keyword */

char *
lookup(ident)
register char	*ident;
{
	register struct	keystruct *p;
	int	c;
	
	/* look up the identifier in the keyword table */
	for (p = hashtab[hash(ident) % HASHMOD]; p != NULL; p = p->next) {
		if (strequal(ident, p->text)) {
			if (compress == YES && (c = p - keyword) < ' ') {
				ident[0] = c;	/* compress the keyword */
			}
			return(p->text);
		}
	}
	/* this is an identifier */
	return(NULL);
}

/* form hash value for string */

hash(ss)
register char	*ss;
{
	register int	i;
	register unsigned char 	*s = (unsigned char *)ss;
	
	for (i = 0; *s != '\0'; )
		i += *s++;	/* += is faster than <<= for cscope */
	return(i);
}
