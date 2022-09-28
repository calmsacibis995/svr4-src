/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)acomp:common/decl.h	52.8"
/* decl.h */


/* These are codes to dcl_norm() to describe the state of the parse: */
#define	DS_NORM		0	/* Normal declarator */
#define	DS_FUNC		1	/* Declarator for a function being defined */
#define	DS_ABS		2	/* Abstract declarator */
#define	DS_INIT		3	/* Declarator with initializer following */

typedef I32 TYCL;
extern void dcl_s_ptrtsl();
typedef I16 DN;
extern DN dcl_ptrto(), dcl_arrof(), dcl_func();
extern DN dcl_ofptr(), dcl_dcor();
extern DN dcl_param(), dcl_plist(), dcl_vparam(), dcl_ident();
extern DN dcl_mbr(), dcl_mlist();
extern DN dcl_menu(), dcl_eexpr(), dcl_elist();
extern char * dcl_type();
extern char * dcl_name();
extern void dcl_s_absdcl();
extern T1WORD dcl_e_absdcl();
extern T1WORD dcl_norm();
extern void dcl_start(), dcl_s_dcor();
extern void dcl_e_ds();
extern void dcl_tycl();
extern void dcl_chkend();
extern void dcl_topnull();
extern void dcl_tag(), dcl_s_enu(), dcl_e_enu();
extern void dcl_noenumcomma();
extern void dcl_s_soru(), dcl_e_soru();
extern void dcl_s_formal(), dcl_e_formal();
extern void dcl_f_lp(), dcl_f_rp(), dcl_lp(), dcl_rp();
extern void dcl_s_func(), dcl_e_func();
extern void dcl_nosusemi();
extern void dcl_tentative();
extern SX dcl_defid();
extern SX dcl_g_arg();
extern int dcl_oktype();
extern void dcl_set_type(), dcl_clr_type();
#ifdef LINT
extern T1WORD dcl_efftype();
#endif

#define	DN_NULL	0	/* null pointer for declaration trees */

/* Magic numbers for various functions. */

/* For dcl_tag(): */
#define	D_NOLIST	0		/* s/u/e decl has no {} list */
#define	D_LIST		1		/* s/u/e decl has {} list */
#define	D_NOFORCE	0		/* don't force new tag decl. */
#define	D_FORCE		1		/* force new tag decl. */

/* For dcl_mbr(): */
#define	D_NOFIELD	0		/* member is not bitfield */
#define	D_FIELD		1		/* member is bitfield */

/* For dcl_s_enu():  use D_[NO]LIST from above. */
/* For dcl_menu(): */
#define	D_NOEXPR	0		/* enumerator has no expression */
#define	D_EXPR		1		/* enumerator has expression */

/* For dcl_chkend(): */
#define	D_ABSDECL	0		/* checking abstract declarator */
#define	D_DECL		1		/* checking regular declarator */

#ifdef	PACK_PRAGMA
extern BITOFF Pack_align;		/* current s/u alignment constraint */
extern BITOFF Pack_default;		/* default constraint to use */
extern BITOFF Pack_string();		/* actually in sharp.c */
#endif
