/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)acomp:common/trees.h	52.12"
/* trees.h */

/* Declarations for tree-building routines. */

/* These are for tr_conv(): */
#define TR_CAST 0			/* convert as if by cast. */
#define TR_ASSIGN 1			/* convert as if by assignment */
#define TR_ARG 2			/* convert a passed argument */

extern ND1 * tr_build();
extern ND1 * tr_chkname();
extern ND1 * tr_name();
extern ND1 * tr_symbol();
extern ND1 * tr_dotdotdot();
extern ND1 * tr_icon();
extern ND1 * tr_int_const();
extern ND1 * tr_fcon();
extern ND1 * tr_string();
extern ND1 * tr_ccon();
extern ND1 * tr_su_mbr();
extern ND1 * tr_type();
extern ND1 * tr_conv();
extern ND1 * tr_generic();
extern CONVAL tr_truncate();		/* pass value through "knot-hole" */
extern CONVAL tr_inconex();		/* integral constant expression */
extern ND1 * tr_sizeof();
extern ND1 * tr_copy();
extern ND1 * tr_return();
extern ND1 * tr_newnode();
extern void tr_e1print();
extern void tr_eprint();

/* flags to indicate special node conditions */
#define FF_ISFLD	02		/* flag to indicate field node */
#define FF_ISAMB	04		/* flag to indicate ambiguity */
#define FF_BADLVAL	010		/* flag to bad lvalue to . */
#define FF_ISVOL	020		/* flag to indicate volatile attribute */
#define FF_ISCAST	040		/* flag to indicate that a cast exists */
#define FF_ISCON	0100		/* flag to indicate an int constant */
#ifdef LINT
#define FF_PAREN        0200            /* flag to indicate expr was in ()'s */
#define FF_TRUNC	01000		/* constant was truncated	     */
#define FF_ORGUNS	02000		/* type was orig. unsigned 	     */
#endif
#define FF_SEFF		04000		/* flag to indicate side effects exist */
#define	FF_WASOPT	0400		/* set if node and descendents have
					** already been optimized
					*/
#define	FF_BUILTIN	010000		/* ICON/NAME node refers to builtin
					** ("magic") identifier
					*/

/* Bits for STRING node. */
#define	TR_ST_WIDE	0x1		/* Set for wide string, else 0. */
#define	TR_ST_ICON	0x2		/* Set if node should be treated as
					** ICON upon code generation, 0 if
					** as NAME.
					*/
