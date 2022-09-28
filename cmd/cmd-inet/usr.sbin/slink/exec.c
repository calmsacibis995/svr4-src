/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.sbin/slink/exec.c	1.2.3.1"

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
#include <varargs.h>
#include <stdio.h>
#include "defs.h"


/*
 * xerr - report execution error & exit
 */
xerr(va_alist)
va_dcl
{
	va_list         args;
	struct finst   *fi;
	struct cmd     *c;
	int             flags;
	char           *fmt;
	char            buf[256];

	va_start(args);
	fi = va_arg(args, struct finst *);
	c = va_arg(args, struct cmd *);
	flags = va_arg(args, int);
	fmt = va_arg(args, char *);
	sprintf(buf, "Function \"%s\", command %d: ", fi->func->name, c->cmdno);
	vsprintf(&buf[strlen(buf)], fmt, args);
	va_end(args);
	error(flags | E_FATAL, buf);
}

/*
 * showval - print value on stderr
 */
showval(v)
	struct val     *v;
{
	extern int	pflag;
	extern int	uflag;

	switch (v->vtype) {
	case V_NONE:
		fprintf(stderr, " <NONE>");
		break;
	case V_STR:
		fprintf(stderr, " %s", v->u.sval);
		break;
	case V_FD:
		fprintf(stderr, " <FD %d>", v->u.val);
		break;
	case V_MUXID:
		fprintf(stderr, " <%s%sLINK %d>",
			(pflag ? "P" : ""), (uflag ? "UN" : ""), v->u.val);
		break;
	}
}

/*
 * makeargv - make argument vector
 */
struct val     *
makeargv(fi, c)
	struct finst   *fi;
	struct cmd     *c;
{
	struct val     *argv;
	struct arg     *a;
	int             i;

	if (c->nargs) {
		argv = (struct val *) xmalloc(c->nargs * sizeof(struct val));
		for (a = c->args, i = 0; i < c->nargs; a++, i++) {
			switch (a->type) {
			case A_VAR:
				argv[i] = fi->vars[a->u.var->index];
				break;
			case A_PARAM:
				if (a->u.param >= fi->nargs) {
					xerr(fi, c, 0, "$%d not defined",
					     a->u.param + 1);
				}
				argv[i] = fi->args[a->u.param];
				break;
			case A_STR:
				argv[i] = *a->u.strval;
				break;
			}
			if (verbose)
				showval(&argv[i]);
		}
		if (verbose)
			putc('\n', stderr);
		return argv;
	} else {
		if (verbose)
			putc('\n', stderr);
		return (struct val *) 0;
	}
}

/*
 * chkargs - check arguments to builtin function
 */
chkargs(fi, c, bf, argc, argv)
	struct finst   *fi;
	struct cmd     *c;
	struct bfunc   *bf;
	int             argc;
	struct val     *argv;
{
	int             i;
	int             atype;

	if (argc < bf->minargs || argc > bf->maxargs)
		xerr(fi, c, 0, "Incorrect argument count");
	for (i = 0; i < argc; i++, argv++) {
		if (argv->vtype != bf->argtypes[i])
			xerr(fi, c, 0, "Incorrect argument type, arg %d", i);
	}
}

/*
 * docmd - execute a command
 */
docmd(fi, c, rval)
	struct finst   *fi;
	struct cmd     *c;
	struct val     *rval;
{
	struct val     *argv;
	struct fntab   *f;
	struct val     *cval;
	struct val     *userfunc();
	extern struct val val_none;

	f = c->func;
	if (verbose)
		fprintf(stderr, "%s", f->name);
	argv = makeargv(fi, c);
	switch (f->type) {
	case F_UNDEF:
		xerr(fi, c, 0, "Undefined function called");
	case F_BUILTIN:
		chkargs(fi, c, f->u.bfunc, c->nargs, argv);
		cval = (f->u.bfunc->func) (fi, c, c->nargs, argv);
		break;
	case F_USER:
		cval = userfunc(f->u.ufunc, c->nargs, argv);
		break;
	case F_RETURN:
		if (c->nargs != 1)
			xerr(fi, c, 0, "Incorrect argument count");
		*rval = argv[0];
		cval = &val_none;
		break;
	}
	if (verbose && cval->vtype != V_NONE) {
		fprintf(stderr, "%s returns", f->name);
		showval(cval);
		putc('\n', stderr);
	}
	if (c->rvar)
		fi->vars[c->rvar->index] = *cval;
	if (argv)
		free(argv);
}

/*
 * userfunc - execute a user function
 */
struct val     *
userfunc(f, argc, argv)
	struct func    *f;
	int             argc;
	struct val     *argv;
{
	struct finst   *fi;
	struct cmd     *c;
	static struct val rval;

	fi = ALLOC(struct finst);
	fi->func = f;
	fi->vars = (struct val *) xmalloc(f->nvars * sizeof(struct val));
	fi->nargs = argc;
	fi->args = argv;
	rval.vtype = V_NONE;
	for (c = f->cmdhead; c; c = c->next)
		docmd(fi, c, &rval);
	free(fi->vars);
	free(fi);
	return &rval;
}
