/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)acomp:common/optim.h	52.4"
/* optim.h */

/* Declarations for tree-opimization routines. */

extern ND1 * op_optim();
extern ND1 * op_init();
extern FP_DOUBLE op_dtofp();

/* Prefix to recognize for special built-in functions.
** All such functions are presumed to begin with the
** same prefix.
*/
#ifndef	BUILTIN_PREFIX
#define	BUILTIN_PREFIX "__builtin_"
#endif
