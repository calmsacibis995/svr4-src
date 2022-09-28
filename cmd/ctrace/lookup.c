/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ctrace:lookup.c	1.13"
/*	ctrace - C program debugging tool
 *
 *	keyword look-up routine for the scanner
 *
 */
#include "global.h"
#include "parser.h"

#ifdef __STDC__
#include <stdlib.h>
#else
extern char* malloc();
#endif

enum	bool stdio_preprocessed = no;	/* stdio.h already preprocessed */
enum	bool setjmp_preprocessed = no;	/* setjmp.h already preprocessed */
enum	bool signal_preprocessed = no;	/* signal.h already preprocessed */
enum	bool syssig_preprocessed = no;	/* sys/signal.h already preprocessed */
enum	bool types_preprocessed = no;	/* types.h already preprocessed */
enum	bool select_preprocessed = no;	/* select.h already preprocessed */
enum	bool timet_preprocessed = no;	/* time_t already preprocessed */
enum	bool clockt_preprocessed = no;	/* clock_t already preprocessed */
enum	bool sizet_preprocessed = no;	/* size_t already preprocessed */

static	struct	keystruct {
	char	*text;
	int	token;
	struct	keystruct *next;
} keyword[] = {
	"BADMAG",	MACRO,		NULL,
	"EOF",		L_INT_CONST,	NULL, 
	"NULL",		L_INT_CONST,	NULL, 
	"asm",		L_ASM,		NULL,
	"auto",		L_class,		NULL,
	"break",	L_CONTINUE,	NULL,
	"case",		L_CASE,		NULL,
	"char",		L_type,		NULL,
	"const",	L_type,		NULL,
	"continue",	L_CONTINUE,	NULL,
	"default",	L_DEFAULT,	NULL,
	"do",		L_DO,		NULL,
	"double",	L_type,		NULL,
	"else",		L_ELSE,		NULL,
	"enum",		L_ENUM,		NULL,
	"extern",	L_class,		NULL,
	"fgets",	STRRES,		NULL, 
	"float",	L_type,		NULL,
	"for",		L_FOR,		NULL,
	/* QW "fortran",	L_class,		NULL, */
	"gets",		STRRES,		NULL, 
	"goto",		L_GOTO,		NULL,
	"if",		L_IF,		NULL,
	"int",		L_type,		NULL,
	"long",		L_type,		NULL,
	"register",	L_class,		NULL,
	"return",	L_RETURN,		NULL,
	"short",	L_type,		NULL,
	"signed",	L_type,		NULL,
	"sizeof",	L_SIZEOF,		NULL,
	"static",	L_class,		NULL,
	"stderr",	L_INT_CONST,	NULL, 
	"stdin",	L_INT_CONST,	NULL, 
	"stdout",	L_INT_CONST,	NULL, 
	"strcat",	STRCAT,		NULL, 
	"strcmp",	STRCMP,		NULL, 
	"strcpy",	STRCPY,		NULL, 
	"strlen",	STRLEN,		NULL, 
	"strncat",	STRNCAT,	NULL, 
	"strncmp",	STRNCMP,	NULL, 
	"struct",	L_SORU,		NULL,
	"switch",	L_SWITCH,		NULL,
	"typedef",	TYPEDEF,	NULL,
	"union",	L_SORU,		NULL,
	"unsigned",	L_type,		NULL,
	"void",		L_type,		NULL,
	"volatile",	L_type,		NULL,
	"while",	L_WHILE,		NULL,
	"__asm",		L_ASM,		NULL,
	"_iobuf",	IOBUF,		NULL, 
	"jmp_buf",	JMP_BUF,	NULL, 
	"sig_atomic_t",	SIG_DEF,	NULL,
	"sigset_t",	SIG_SYS,	NULL,
	"daddr_t",	TYP_DEF,	NULL,
	"fd_set",	SEL_DEF,	NULL,
	"time_t",	TIME_T,		NULL,
	"clock_t",	CLOCK_T,	NULL,
	"size_t",	SIZE_T,		NULL,
};
#define KEYWORDS	(sizeof(keyword) / sizeof(struct keystruct))

#define HASHSIZE	KEYWORDS + 10	/* approximate number of keywords and typedef names */

static	struct	keystruct *hashtab[HASHSIZE]; /* pointer table */

#ifdef __STDC__
	void init_symtab(void);
	int lookup(register char *);
	int add_symbol(char *, int);
	char *strsave(char *);
	static void hashlink(register struct keystruct *);
	static hash(register char *);
#else
	void init_symtab();
	int lookup();
	int add_symbol();
	char *strsave();
	static void hashlink();	/* necessary for ANSI C */
	static hash();
#endif

/* put the keywords into the symbol table */
void
init_symtab()
{
	register int	i;
	
	for (i = 0; i < KEYWORDS; ++i)
		hashlink(&keyword[i]);
}

/* see if this identifier is a keyword or typedef name */
int
lookup(ident)
register char	*ident;
{
	register struct	keystruct *p;
	
	/* look up the identifier in the symbol table */
	for (p = hashtab[hash(ident)]; p != NULL; p = p->next)
		if (strcmp(ident, p->text) == 0) {
			if (p->token == IOBUF) {	/* Unix 3.0 check */
				stdio_preprocessed = yes;
				return(L_IDENT);
			}
			if (p->token == JMP_BUF) {
				setjmp_preprocessed = yes;
				return(L_IDENT);
			}
			if (p->token == SIG_DEF) {
				signal_preprocessed = yes;
				return(L_IDENT);
			}
			if (p->token == SIG_SYS) {
				syssig_preprocessed = yes;
				return(L_IDENT);
			}
			if (p->token == TYP_DEF) {
				types_preprocessed = yes;
				return(L_IDENT);
			}
			if (p->token == SEL_DEF) {
				select_preprocessed = yes;
				return(L_IDENT);
			}
			if (p->token == TIME_T) {
				timet_preprocessed = yes;
				return(L_IDENT);
			}
			if (p->token == CLOCK_T) {
				clockt_preprocessed = yes;
				return(L_IDENT);
			}
			if (p->token == SIZE_T) {
				sizet_preprocessed = yes;
				return(L_IDENT);
			}
			return(p->token);
			}
	/* this is an identifier */
	return(L_IDENT);
}

/* add an identifier to the symbol table */
int
add_symbol(ident, type)
char	*ident;
int	type;
{
	struct	keystruct *p;
	int	t;
	
	if ((t = lookup(ident)) == L_IDENT) {
		p = (struct keystruct *) malloc(sizeof(*p));
		if (p == NULL || (p->text = strsave(ident)) == NULL) {
			fatal("out of storage");
		}
		else {
			t = p->token = type;
			hashlink(p);
			if((type==L_TYPENAME || type==L_type) && strcmp(p->text,"FILE")==0) /* Unix 4.0 check */
				stdio_preprocessed = yes;
		}
	}
	return(t);
}
char *
strsave(s)
char	*s;
{
	char	*p;
	
	if ((p = (char *) malloc((unsigned) (strlen(s) + 1))) != NULL)
		(void)strcpy(p, s);
	return(p);
}
static void
hashlink(p)
register struct	keystruct *p;
{
	register int	hashval;
	
	hashval = hash(p->text);
	p->next = hashtab[hashval];
	hashtab[hashval] = p;
}
static
hash(s)	/* form hash value for string */
register char	*s;
{
	register int	hashval;
	register unsigned char *ss = (unsigned char *)s;
	
	for (hashval = 0; *ss != '\0'; )
		hashval += *ss++;
	return(hashval % HASHSIZE);
}
