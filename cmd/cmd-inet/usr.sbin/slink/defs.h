/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.sbin/slink/defs.h	1.1.2.1"

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

/*
 * Token types
 */

#define	T_STRING	257
#define T_NAME		258
#define T_PARAM		259
#define T_EOL		260


/* value types */

#define V_NONE		0	/* no value */
#define	V_STR		1	/* string */
#define	V_FD		2	/* file descriptor */
#define V_MUXID		3	/* muxid returned from link */

struct val {
	int             vtype;	/* type (see above) */
	union {
		char           *sval;	/* string value */
		int             val;	/* other value */
	}               u;
};

/* variables */

struct var {
	struct var     *next;
	char           *name;
	int             index;
};

/* command argument */

#define A_VAR		0	/* variable */
#define A_PARAM		1	/* formal parameter */
#define A_STR		2	/* string */

struct arg {
	int             type;
	union {
		struct var     *var;
		int             param;
		struct val     *strval;
	}               u;
};

/* command */

#define MAXARGS		99

struct cmd {
	struct cmd     *next;	/* next command in func */
	struct var     *rvar;	/* var to store returned value in */
	struct fntab   *func;	/* function */
	int             cmdno;	/* number of command in function */
	int             nargs;	/* number of args */
	struct arg      args[MAXARGS];	/* args */
};

/* function definition */

struct func {
	char           *name;	/* function name */
	struct var     *varlist;/* variables declared */
	int             nvars;	/* number of vars declared */
	struct cmd     *cmdhead, *cmdtail;	/* command list */
};

/* info about builtin function */

struct bfunc {
	struct val     *(*func) ();	/* the function */
	int             minargs;/* minimum arguments required */
	int             maxargs;/* maximum arguments required */
	int            *argtypes;	/* required argument types */
};

/* function table */

#define F_UNDEF		0	/* referenced but not yet defined */
#define F_BUILTIN	1	/* built-in function */
#define F_USER		2	/* user-defined function */
#define F_RETURN	3	/* the return command */

struct fntab {
	struct fntab   *next;	/* function table link */
	char           *name;	/* function name */
	int             type;	/* type of function (see above) */
	union {
		struct func    *ufunc;
		struct bfunc   *bfunc;
	}               u;
};

/* stuff related to an instance of a function */

struct finst {
	struct func    *func;	/* function definition */
	struct val     *vars;	/* variable values */
	int             nargs;	/* number of args */
	struct val     *args;	/* argument values */
};


/* flags for error() */

#define E_SYS		0x01	/* print system error message */
#define E_FATAL		0x02	/* exit */
#define E_FSYS		(E_SYS | E_FATAL)


char           *savestr();
char           *xmalloc();
#define ALLOC(type) ((type *) xmalloc(sizeof(type)))

extern char    *strval;		/* value of STRING or NAME token */
extern int      intval;		/* integer token value */
extern int      line;		/* input line number */
extern char    *cmdname;	/* argv[0] */
extern int      verbose;	/* verbose mode flag */

#define CONFIGFILE	"/etc/strcf"

#ifndef NULL
#define NULL	0
#endif
