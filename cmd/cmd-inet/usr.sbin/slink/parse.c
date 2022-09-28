/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.sbin/slink/parse.c	1.1.2.1"

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */

/*
 *
 *	Copyright 1987, 1988 Lachman Associates, Incorporated (LAI)
 *	All Rights Reserved.
 *
 *	The copyright above and this notice must be preserved in all
 *	copies of this source code.  The copyright above does not
 *	evidence any actual or intended publication of this source
 *	code.
 *
 *	This is unpublished proprietary trade secret source code of
 *	Lachman Associates.  This source code may not be copied,
 *	disclosed, distributed, demonstrated or licensed except as
 *	expressly authorized by Lachman Associates.
 *
 *	System V STREAMS TCP was jointly developed by Lachman
 *	Associates and Convergent Technologies.
 */

#include <stdio.h>
#include <fcntl.h>
#include <varargs.h>
#include <sys/types.h>
#include "defs.h"

#define LBRACE		'{'
#define RBRACE		'}'
#define EQUALS		'='

int             intval;
char           *strval;
int             line = 1;

/*
 * xmalloc - do malloc, but exit on failure.
 */
char           *
xmalloc(n)
	int             n;
{
	char           *m;
	char           *malloc();

	if (m = malloc(n))
		return m;
	else
		error(E_FATAL, "out of memory");
}

/*
 * savestr - xmalloc memory for string and copy it in.  Pointer to new memory
 * is returned.
 */
char           *
savestr(str)
	char           *str;
{
	char           *s;

	s = xmalloc(strlen(str) + 1);
	strcpy(s, str);
	return s;
}

/* function table stuff */

struct fntab   *fntab = NULL;

/*
 * findfunc - look up named function in function table, return pointer.
 */
struct fntab   *
findfunc(name)
	char           *name;
{
	struct fntab   *f;

	for (f = fntab; f; f = f->next) {
		if (strcmp(f->name, name) == 0)
			break;
	}
	return f;
}

/*
 * deffunc - define function addfunc(name,type,ptr) ptr specifies function
 * for user or built-in.  For other types, ptr should not be passed.
 */
struct fntab   *
deffunc(va_alist)
va_dcl
{
	va_list         args;
	struct fntab   *f;
	char           *name;
	int             type;
	int             addme;

	va_start(args);
	name = va_arg(args, char *);
	type = va_arg(args, int);
	if (f = findfunc(name)) {
		if (f->type != F_UNDEF)
			error(E_FATAL, "Function \"%s\" redefined.", name);
		addme = 0;
	} else {
		f = ALLOC(struct fntab);
		f->name = name;
		addme = 1;
	}
	f->type = type;
	switch (type) {
	case F_BUILTIN:
		f->u.bfunc = va_arg(args, struct bfunc *);
		break;
	case F_USER:
		f->u.ufunc = va_arg(args, struct func *);
		break;
	default:
		break;
	}
	va_end(args);
	if (addme) {
		f->next = fntab;
		fntab = f;
	}
	return f;
}


/*
 * gettok - get next token.  If eofok is set, returns 0 on EOF. Otherwise,
 * EOF is considered an error.
 */
int
gettok(eofok)
	int             eofok;
{
	int             t;

	t = yylex();
	if (t || eofok)
		return t;
	else
		error(E_FATAL, "syntax error: unexpected end-of-file");
}

/*
 * syntax - print syntax error message (including line number) and exit.
 */
syntax(msg)
	char           *msg;
{
	error(E_FATAL, "syntax error near line %d: %s", line, msg);
}

/*
 * findvar - find variable definition
 */
struct var     *
findvar(f, name)
	struct func    *f;
	char           *name;
{
	struct var     *v;

	for (v = f->varlist; v; v = v->next) {
		if (strcmp(name, v->name) == 0)
			break;
	}
	return v;
}

/*
 * defvar - define variable for function.  If already defined, a pointer to
 * it is returned.
 */
struct var     *
defvar(f, name)
	struct func    *f;
	char           *name;
{
	struct var     *v;

	if (!(v = findvar(f, name))) {
		v = ALLOC(struct var);
		v->name = name;
		v->index = f->nvars++;
		v->next = f->varlist;
		f->varlist = v;
	}
	return v;
}

/*
 * parsecmd - parse command, add to function.  Returns 1 if command parsed, 0
 * if closing brace encountered.
 */
parsecmd(f)
	struct func    *f;
{
	int             t;
	struct cmd     *c;
	char           *name;
	struct var     *v;
	struct arg     *a;

	while ((t = gettok(0)) == T_EOL);
	if (t == RBRACE)
		return 0;
	c = ALLOC(struct cmd);
	if (t != T_NAME)
		syntax("function or variable name expected");
	name = strval;
	if ((t = gettok(0)) == EQUALS) {
		v = defvar(f, name);
		c->rvar = v;
		if (gettok(0) != T_NAME)
			syntax("function name expected");
		name = strval;
		t = gettok(0);
	} else {
		c->rvar = NULL;
	}
	if (!(c->func = findfunc(name)))
		c->func = deffunc(name, F_UNDEF);
	c->nargs = 0;
	for (; t != T_EOL; t = gettok(0)) {
		if (t != T_NAME && t != T_PARAM && t != T_STRING)
			syntax("variable, param, or string expected");
		if (c->nargs == MAXARGS)
			syntax("too many arguments");
		a = &c->args[c->nargs++];
		if (t == T_PARAM) {
			a->type = A_PARAM;
			a->u.param = intval;
		} else if (t == T_NAME && (v = findvar(f, strval))) {
			a->type = A_VAR;
			a->u.var = v;
		} else {
			a->type = A_STR;
			a->u.strval = ALLOC(struct val);
			a->u.strval->vtype = V_STR;
			a->u.strval->u.sval = strval;
		}
	}

	if (f->cmdhead) {
		f->cmdtail->next = c;
		c->cmdno = f->cmdtail->cmdno + 1;
	} else {
		f->cmdhead = c;
		c->cmdno = 1;
	}
	f->cmdtail = c;
	c->next = NULL;
	return 1;
}

/*
 * parsefunc - parse function definition and create function table entry.
 * Returns 0 on end of input.
 */
int
parsefunc()
{
	int             t;
	char           *name;
	struct func    *f;

	while ((t = gettok(1)) == T_EOL);
	if (t == 0)
		return 0;
	if (t != T_NAME)
		syntax("function name expected");
	name = strval;
	if (gettok(0) != LBRACE)
		syntax("'{' expected");	/* } */
	f = ALLOC(struct func);
	f->varlist = NULL;
	f->nvars = 0;
	f->name = name;
	f->cmdhead = f->cmdtail = NULL;
	while (parsecmd(f));
	deffunc(name, F_USER, f);
	return 1;
}

parse()
{
	while (parsefunc());
}
