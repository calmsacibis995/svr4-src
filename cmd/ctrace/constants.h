/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ctrace:constants.h	1.8"
/*	ctrace - C program debugging tool
 *
 *	preprocessor constant definitions
 *
 */

#define IDMAX		127	/* maximum significant identifier chars */
#define TOKENMAX	IDMAX	/* maximum non-string token length */
#define INDENTMAX	40	/* maximum left margin indentation chars */
#define LOOPMAX		20	/* statement trace buffer size */
#define SAVEMAX		80	/* parser function header and brace text storage size */
#define	STMTMAX		3000	/* maximum statement text length */
#define	TRACE_DFLT	10	/* default number of traced variables */
#define TRACEMAX	20	/* maximum number of traced variables */
#undef	YYLMAX		
#define YYLMAX		STMTMAX + TOKENMAX	/* scanner line buffer size */
#define YYMAXDEPTH	300	/* yacc stack size */

#define NO_LINENO	0
#define	PP_OPTIONS	"-C -E -DCTRACE"    /* preprocessor command */

#ifndef PP_COMMAND
#define PP_COMMAND	"cc"	/* C compiler */
#endif

#ifndef RUNTIME
#define	RUNTIME		"/usr/ccs/lib/ctrace/runtime.c" /* run-time code package file */
#endif
