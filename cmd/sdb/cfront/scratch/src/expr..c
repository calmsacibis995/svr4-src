/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#line 1 "../../src/expr.c"

/* <<cfront 1.2.1 2/16/87>> */
/* < ../../src/expr.c */

#ident	"@(#)sdb:cfront/scratch/src/expr..c	1.2"
char *_new(); char _delete(); char *_vec_new(); char _vec_delete();

#line 1 "../../src/expr.c"

#ident	"@(#)sdb:cfront/src/cfront.h	1.1"

#ident	"@(#)sdb:cfront/src/token.h	1.1"

#include <stdio.h>

#line 20 "../../src/token.h"
extern char lex_clear ();
extern char ktbl_init ();
extern char otbl_init ();

#line 33 "../../src/token.h"
extern char *keys [256];

#ident	"@(#)sdb:cfront/src/typedef.h	1.1"

#line 2 "../../src/typedef.h"
typedef unsigned char TOK ;
typedef struct node *PP ;
typedef char bit ;
typedef int (*PFI )();
typedef char (*PFV )();
typedef struct node *Pnode ;
typedef struct key *Pkey ;
typedef struct name *Pname ;
typedef struct basetype *Pbase ;
typedef struct type *Ptype ;
typedef struct fct *Pfct ;
typedef struct field *Pfield ;
typedef struct expr *Pexpr ;
typedef struct qexpr *Pqexpr ;
typedef struct texpr *Ptexpr ;
typedef struct classdef *Pclass ;
typedef struct enumdef *Penum ;
typedef struct stmt *Pstmt ;
typedef struct estmt *Pestmt ;
typedef struct tstmt *Ptstmt ;
typedef struct vec *Pvec ;
typedef struct ptr *Pptr ;
typedef struct block *Pblock ;
typedef struct table *Ptable ;
typedef struct loc Loc ;
typedef struct call *Pcall ;
typedef struct gen *Pgen ;
typedef struct ref *Pref ;
typedef struct name_list *Plist ;
typedef struct iline *Pin ;
typedef struct nlist *Pnlist ;
typedef struct slist *Pslist ;
typedef struct elist *Pelist ;

#line 29 "../../src/cfront.h"
extern bit old_fct_accepted ;

#line 33 "../../src/cfront.h"
extern bit fct_void ;

#line 43 "../../src/cfront.h"
extern char *prog_name ;
extern int inline_restr ;
extern bit emode ;

#line 48 "../../src/cfront.h"
extern Pname name_free ;
extern Pexpr expr_free ;
extern Pstmt stmt_free ;

#line 53 "../../src/cfront.h"
extern int Nspy ;
extern int Nfile ;

#line 54 "../../src/cfront.h"
extern int Nline ;

#line 54 "../../src/cfront.h"
extern int Ntoken ;

#line 54 "../../src/cfront.h"
extern int Nname ;

#line 54 "../../src/cfront.h"
extern int Nfree_store ;

#line 54 "../../src/cfront.h"
extern int Nalloc ;

#line 54 "../../src/cfront.h"
extern int Nfree ;
extern int Nn ;

#line 55 "../../src/cfront.h"
extern int Nbt ;

#line 55 "../../src/cfront.h"
extern int Nt ;

#line 55 "../../src/cfront.h"
extern int Ne ;

#line 55 "../../src/cfront.h"
extern int Ns ;

#line 55 "../../src/cfront.h"
extern int Nstr ;

#line 55 "../../src/cfront.h"
extern int Nc ;

#line 55 "../../src/cfront.h"
extern int Nl ;
extern int NFn ;

#line 56 "../../src/cfront.h"
extern int NFtn ;

#line 56 "../../src/cfront.h"
extern int NFpv ;

#line 56 "../../src/cfront.h"
extern int NFbt ;

#line 56 "../../src/cfront.h"
extern int NFf ;

#line 56 "../../src/cfront.h"
extern int NFs ;

#line 56 "../../src/cfront.h"
extern int NFc ;

#line 56 "../../src/cfront.h"
extern int NFe ;

#line 56 "../../src/cfront.h"
extern int NFl ;

#line 58 "../../src/cfront.h"
extern TOK lex ();
extern Pname syn ();

#line 61 "../../src/cfront.h"
extern char init_print ();
extern char init_lex ();
extern char int_syn ();
extern char ext ();

#line 66 "../../src/cfront.h"
extern char *make_name ();

#line 69 "../../src/cfront.h"
struct loc {	/* sizeof loc == 4 */

#line 70 "../../src/cfront.h"
short _loc_file ;
short _loc_line ;
};

#line 73 "../../src/cfront.h"
char _loc_put ();
char _loc_putline ();

#line 78 "../../src/cfront.h"
extern Loc curloc ;
extern int curr_file ;

#line 81 "../../src/cfront.h"
union _C1 {	/* sizeof _C1 == 4 */

#line 83 "../../src/cfront.h"
char *__C1_p ;
int __C1_i ;
};
struct ea {	/* sizeof ea == 4 */
union _C1 _ea__O1 ;
};

#line 87 "../../src/cfront.h"
	/* overload _ctor: */

#line 88 "../../src/cfront.h"

#line 89 "../../src/cfront.h"

#line 92 "../../src/cfront.h"
extern struct ea *ea0 ;

#line 95 "../../src/cfront.h"
int error ();
int errorFPCloc__PC_RCea__RCea__RCea__RCea___ ();
int errorFI_PC_RCea__RCea__RCea__RCea___ ();
int errorFI_PCloc__PC_RCea__RCea__RCea__RCea___ ();

#line 101 "../../src/cfront.h"
extern int error_count ;
extern bit debug ;
extern int vtbl_opt ;
extern FILE *out_file ;
extern FILE *in_file ;
extern char scan_started ;
extern bit warn ;

#line 110 "../../src/cfront.h"
extern int br_level ;
extern int bl_level ;
extern Ptable ktbl ;
extern Ptable gtbl ;
extern char *oper_name ();
extern Pclass ccl ;
extern Pbase defa_type ;
extern Pbase moe_type ;

#line 120 "../../src/cfront.h"
extern Pstmt Cstmt ;
extern Pname Cdcl ;
extern char put_dcl_context ();

#line 124 "../../src/cfront.h"
extern Ptable any_tbl ;
extern Pbase any_type ;

#line 128 "../../src/cfront.h"
extern Pbase int_type ;
extern Pbase char_type ;
extern Pbase short_type ;
extern Pbase long_type ;
extern Pbase uint_type ;
extern Pbase float_type ;
extern Pbase double_type ;
extern Pbase void_type ;

#line 138 "../../src/cfront.h"
extern Pbase uchar_type ;
extern Pbase ushort_type ;
extern Pbase ulong_type ;
extern Ptype Pchar_type ;
extern Ptype Pint_type ;
extern Ptype Pfctvec_type ;
extern Ptype Pfctchar_type ;
extern Ptype Pvoid_type ;
extern Pbase zero_type ;

#line 148 "../../src/cfront.h"
extern int byte_offset ;
extern int bit_offset ;
extern int max_align ;
extern int stack_size ;
extern int enum_count ;
extern int const_save ;

#line 156 "../../src/cfront.h"
extern Pexpr dummy ;
extern Pexpr zero ;
extern Pexpr one ;
extern Pname sta_name ;

#line 165 "../../src/cfront.h"
struct node {	/* sizeof node == 3 */

#line 166 "../../src/cfront.h"
TOK _node_base ;
TOK _node_n_key ;
bit _node_permanent ;
};

#line 172 "../../src/cfront.h"
extern Pclass Ebase ;

#line 172 "../../src/cfront.h"
extern Pclass Epriv ;

#line 175 "../../src/cfront.h"
struct table {	/* sizeof table == 32 */

#line 166 "../../src/cfront.h"
TOK _node_base ;
TOK _node_n_key ;
bit _node_permanent ;

#line 177 "../../src/cfront.h"
char _table_init_stat ;

#line 181 "../../src/cfront.h"
short _table_size ;
short _table_hashsize ;
short _table_free_slot ;
Pname *_table_entries ;
short *_table_hashtbl ;
Pstmt _table_real_block ;

#line 189 "../../src/cfront.h"
Ptable _table_next ;
Pname _table_t_name ;
};

#line 192 "../../src/cfront.h"
struct table *_table__ctor ();

#line 194 "../../src/cfront.h"
Pname _table_look ();
Pname _table_insert ();

#line 197 "../../src/cfront.h"
char _table_grow ();

#line 200 "../../src/cfront.h"
Pname _table_get_mem ();

#line 202 "../../src/cfront.h"
char _table_dcl_print ();
Pname _table_lookc ();
Pexpr _table_find_name ();
char _table_del ();

#line 210 "../../src/cfront.h"
extern bit Nold ;
extern bit vec_const ;

#line 211 "../../src/cfront.h"
extern bit fct_const ;

#line 214 "../../src/cfront.h"
extern char restore ();
extern char set_scope ();
extern Plist modified_tn ;
extern Pbase start_cl ();
extern char end_cl ();
extern Pbase end_enum ();

#line 224 "../../src/cfront.h"
extern bit new_type ;
extern Pname cl_obj_vec ;
extern Pname eobj ;

#line 236 "../../src/cfront.h"
struct type {	/* sizeof type == 4 */

#line 166 "../../src/cfront.h"
TOK _node_base ;
TOK _node_n_key ;
bit _node_permanent ;

#line 237 "../../src/cfront.h"
bit _type_defined ;
};

#line 240 "../../src/cfront.h"
char *_type_signature ();

#line 242 "../../src/cfront.h"
char _type_print ();
char _type_dcl_print ();
char _type_base_print ();
char _type_del ();

#line 247 "../../src/cfront.h"
Pname _type_is_cl_obj ();
int _type_is_ref ();
char _type_dcl ();
int _type_tsizeof ();
bit _type_tconst ();
TOK _type_set_const ();
int _type_align ();
TOK _type_kind ();

#line 258 "../../src/cfront.h"
bit _type_vec_type ();
bit _type_check ();
Ptype _type_deref ();
Pptr _type_addrof ();

#line 265 "../../src/cfront.h"
struct enumdef {	/* sizeof enumdef == 12 */

#line 166 "../../src/cfront.h"
TOK _node_base ;
TOK _node_n_key ;
bit _node_permanent ;

#line 237 "../../src/cfront.h"
bit _type_defined ;

#line 266 "../../src/cfront.h"
bit _enumdef_e_body ;
short _enumdef_no_of_enumerators ;
Pname _enumdef_mem ;
};

#line 271 "../../src/cfront.h"
char _enumdef_print ();
char _enumdef_dcl_print ();
char _enumdef_dcl ();
char _enumdef_simpl ();

#line 278 "../../src/cfront.h"
struct classdef {	/* sizeof classdef == 68 */

#line 166 "../../src/cfront.h"
TOK _node_base ;
TOK _node_n_key ;
bit _node_permanent ;

#line 237 "../../src/cfront.h"
bit _type_defined ;

#line 279 "../../src/cfront.h"
bit _classdef_pubbase ;
bit _classdef_c_body ;
TOK _classdef_csu ;
char _classdef_obj_align ;
char _classdef_bit_ass ;
char _classdef_virt_count ;

#line 286 "../../src/cfront.h"
Pname _classdef_clbase ;
char *_classdef_string ;
Pname _classdef_mem_list ;
Ptable _classdef_memtbl ;
int _classdef_obj_size ;
int _classdef_real_size ;
Plist _classdef_friend_list ;
Pname _classdef_pubdef ;
Plist _classdef_tn_list ;
Pclass _classdef_in_class ;
Ptype _classdef_this_type ;
Pname *_classdef_virt_init ;
Pname _classdef_itor ;
Pname _classdef_conv ;
};

#line 301 "../../src/cfront.h"
struct classdef *_classdef__ctor ();

#line 304 "../../src/cfront.h"
char _classdef_print ();
char _classdef_dcl_print ();
char _classdef_simpl ();

#line 308 "../../src/cfront.h"
char _classdef_print_members ();
char _classdef_dcl ();
bit _classdef_has_friend ();
	/* overload baseof: */
bit _classdef_baseofFPCname___ ();
bit _classdef_baseofFPCclassdef___ ();
Pname _classdef_has_oper ();

#line 317 "../../src/cfront.h"
Pname _classdef_has_ictor ();

#line 333 "../../src/cfront.h"
struct basetype {	/* sizeof basetype == 36 */

#line 166 "../../src/cfront.h"
TOK _node_base ;
TOK _node_n_key ;
bit _node_permanent ;

#line 237 "../../src/cfront.h"
bit _type_defined ;

#line 334 "../../src/cfront.h"
bit _basetype_b_unsigned ;
bit _basetype_b_const ;
bit _basetype_b_typedef ;
bit _basetype_b_inline ;
bit _basetype_b_virtual ;
bit _basetype_b_short ;
bit _basetype_b_long ;
char _basetype_b_bits ;
char _basetype_b_offset ;
TOK _basetype_b_sto ;
Pname _basetype_b_name ;
Ptable _basetype_b_table ;
Pexpr _basetype_b_field ;
Pname _basetype_b_xname ;
Ptype _basetype_b_fieldtype ;
};

#line 350 "../../src/cfront.h"
struct basetype *_basetype__ctor ();

#line 352 "../../src/cfront.h"
Pbase _basetype_type_adj ();
Pbase _basetype_base_adj ();
Pbase _basetype_name_adj ();
Pname _basetype_aggr ();
char _basetype_normalize ();

#line 358 "../../src/cfront.h"
Pbase _basetype_check ();
char _basetype_dcl_print ();
Pbase _basetype_arit_conv ();

#line 366 "../../src/cfront.h"
struct fct {	/* sizeof fct == 52 */

#line 166 "../../src/cfront.h"
TOK _node_base ;
TOK _node_n_key ;
bit _node_permanent ;

#line 237 "../../src/cfront.h"
bit _type_defined ;

#line 367 "../../src/cfront.h"
TOK _fct_nargs ;
TOK _fct_nargs_known ;
char _fct_f_virtual ;
char _fct_f_inline ;
Ptype _fct_returns ;
Pname _fct_argtype ;
Ptype _fct_s_returns ;
Pname _fct_f_this ;
Pclass _fct_memof ;
Pblock _fct_body ;
Pname _fct_f_init ;

#line 380 "../../src/cfront.h"
Pexpr _fct_b_init ;

#line 383 "../../src/cfront.h"
Pexpr _fct_f_expr ;
Pexpr _fct_last_expanded ;
Pname _fct_f_result ;
};

#line 387 "../../src/cfront.h"
struct fct *_fct__ctor ();

#line 389 "../../src/cfront.h"
char _fct_argdcl ();

#line 391 "../../src/cfront.h"
Ptype _fct_normalize ();
char _fct_dcl_print ();
char _fct_dcl ();
Pexpr _fct_base_init ();
Pexpr _fct_mem_init ();

#line 397 "../../src/cfront.h"
char _fct_simpl ();
Pexpr _fct_expand ();

#line 403 "../../src/cfront.h"
struct name_list {	/* sizeof name_list == 8 */

#line 404 "../../src/cfront.h"
Pname _name_list_f ;
Plist _name_list_l ;
};

#line 410 "../../src/cfront.h"
struct gen {	/* sizeof gen == 12 */

#line 166 "../../src/cfront.h"
TOK _node_base ;
TOK _node_n_key ;
bit _node_permanent ;

#line 237 "../../src/cfront.h"
bit _type_defined ;

#line 411 "../../src/cfront.h"
Plist _gen_fct_list ;
char *_gen_string ;
};
struct gen *_gen__ctor ();
Pname _gen_add ();
Pname _gen_find ();

#line 419 "../../src/cfront.h"
struct pvtyp {	/* sizeof pvtyp == 8 */

#line 166 "../../src/cfront.h"
TOK _node_base ;
TOK _node_n_key ;
bit _node_permanent ;

#line 237 "../../src/cfront.h"
bit _type_defined ;

#line 420 "../../src/cfront.h"
Ptype _pvtyp_typ ;
};

#line 425 "../../src/cfront.h"
struct vec {	/* sizeof vec == 16 */

#line 166 "../../src/cfront.h"
TOK _node_base ;
TOK _node_n_key ;
bit _node_permanent ;

#line 237 "../../src/cfront.h"
bit _type_defined ;

#line 420 "../../src/cfront.h"
Ptype _pvtyp_typ ;

#line 426 "../../src/cfront.h"
Pexpr _vec_dim ;
int _vec_size ;
};

#line 431 "../../src/cfront.h"
Ptype _vec_normalize ();

#line 436 "../../src/cfront.h"
struct ptr {	/* sizeof ptr == 16 */

#line 166 "../../src/cfront.h"
TOK _node_base ;
TOK _node_n_key ;
bit _node_permanent ;

#line 237 "../../src/cfront.h"
bit _type_defined ;

#line 420 "../../src/cfront.h"
Ptype _pvtyp_typ ;

#line 437 "../../src/cfront.h"
Pclass _ptr_memof ;
bit _ptr_rdo ;
};

#line 442 "../../src/cfront.h"
Ptype _ptr_normalize ();

#line 447 "../../src/cfront.h"

#line 449 "../../src/cfront.h"
extern bit vrp_equiv ;

#line 461 "../../src/cfront.h"
extern Pexpr next_elem ();
extern char new_list ();
extern char list_check ();
extern Pexpr ref_init ();
extern Pexpr class_init ();
extern Pexpr check_cond ();

#line 479 "../../src/cfront.h"
union _C2 {	/* sizeof _C2 == 4 */

#line 481 "../../src/cfront.h"
Ptype __C2_tp ;
int __C2_syn_class ;
};
union _C3 {	/* sizeof _C3 == 4 */

#line 485 "../../src/cfront.h"
Pexpr __C3_e1 ;
char *__C3_string ;
int __C3_i1 ;
};
union _C4 {	/* sizeof _C4 == 4 */

#line 490 "../../src/cfront.h"
Pexpr __C4_e2 ;
Pexpr __C4_n_initializer ;
char *__C4_string2 ;
};
union _C5 {	/* sizeof _C5 == 4 */

#line 495 "../../src/cfront.h"
Ptype __C5_tp2 ;
Pname __C5_fct_name ;
Pexpr __C5_cond ;
Pname __C5_mem ;
Ptype __C5_as_type ;
Ptable __C5_n_table ;
Pin __C5_il ;
};
struct expr {	/* sizeof expr == 20 */

#line 166 "../../src/cfront.h"
TOK _node_base ;
TOK _node_n_key ;
bit _node_permanent ;

#line 483 "../../src/cfront.h"
union _C2 _expr__O2 ;

#line 488 "../../src/cfront.h"
union _C3 _expr__O3 ;

#line 493 "../../src/cfront.h"
union _C4 _expr__O4 ;

#line 502 "../../src/cfront.h"
union _C5 _expr__O5 ;
};

#line 504 "../../src/cfront.h"
struct expr *_expr__ctor ();
char _expr__dtor ();

#line 507 "../../src/cfront.h"
char _expr_del ();
char _expr_print ();
Pexpr _expr_typ ();
int _expr_eval ();
int _expr_lval ();
Ptype _expr_fct_call ();
Pexpr _expr_address ();
Pexpr _expr_contents ();
char _expr_simpl ();
Pexpr _expr_expand ();
bit _expr_not_simple ();
Pexpr _expr_try_to_overload ();
Pexpr _expr_docast ();
Pexpr _expr_dovalue ();
Pexpr _expr_donew ();
char _expr_simpl_new ();
char _expr_simpl_delete ();

#line 527 "../../src/cfront.h"
struct texpr {	/* sizeof texpr == 20 */

#line 166 "../../src/cfront.h"
TOK _node_base ;
TOK _node_n_key ;
bit _node_permanent ;

#line 483 "../../src/cfront.h"
union _C2 _expr__O2 ;

#line 488 "../../src/cfront.h"
union _C3 _expr__O3 ;

#line 493 "../../src/cfront.h"
union _C4 _expr__O4 ;

#line 502 "../../src/cfront.h"
union _C5 _expr__O5 ;
};

#line 531 "../../src/cfront.h"
struct ival {	/* sizeof ival == 20 */

#line 166 "../../src/cfront.h"
TOK _node_base ;
TOK _node_n_key ;
bit _node_permanent ;

#line 483 "../../src/cfront.h"
union _C2 _expr__O2 ;

#line 488 "../../src/cfront.h"
union _C3 _expr__O3 ;

#line 493 "../../src/cfront.h"
union _C4 _expr__O4 ;

#line 502 "../../src/cfront.h"
union _C5 _expr__O5 ;
};

#line 535 "../../src/cfront.h"
struct call {	/* sizeof call == 20 */

#line 166 "../../src/cfront.h"
TOK _node_base ;
TOK _node_n_key ;
bit _node_permanent ;

#line 483 "../../src/cfront.h"
union _C2 _expr__O2 ;

#line 488 "../../src/cfront.h"
union _C3 _expr__O3 ;

#line 493 "../../src/cfront.h"
union _C4 _expr__O4 ;

#line 502 "../../src/cfront.h"
union _C5 _expr__O5 ;
};

#line 538 "../../src/cfront.h"
char _call_simpl ();
Pexpr _call_expand ();

#line 543 "../../src/cfront.h"
struct qexpr {	/* sizeof qexpr == 20 */

#line 166 "../../src/cfront.h"
TOK _node_base ;
TOK _node_n_key ;
bit _node_permanent ;

#line 483 "../../src/cfront.h"
union _C2 _expr__O2 ;

#line 488 "../../src/cfront.h"
union _C3 _expr__O3 ;

#line 493 "../../src/cfront.h"
union _C4 _expr__O4 ;

#line 502 "../../src/cfront.h"
union _C5 _expr__O5 ;
};

#line 547 "../../src/cfront.h"
struct ref {	/* sizeof ref == 20 */

#line 166 "../../src/cfront.h"
TOK _node_base ;
TOK _node_n_key ;
bit _node_permanent ;

#line 483 "../../src/cfront.h"
union _C2 _expr__O2 ;

#line 488 "../../src/cfront.h"
union _C3 _expr__O3 ;

#line 493 "../../src/cfront.h"
union _C4 _expr__O4 ;

#line 502 "../../src/cfront.h"
union _C5 _expr__O5 ;
};

#line 551 "../../src/cfront.h"
struct text_expr {	/* sizeof text_expr == 20 */

#line 166 "../../src/cfront.h"
TOK _node_base ;
TOK _node_n_key ;
bit _node_permanent ;

#line 483 "../../src/cfront.h"
union _C2 _expr__O2 ;

#line 488 "../../src/cfront.h"
union _C3 _expr__O3 ;

#line 493 "../../src/cfront.h"
union _C4 _expr__O4 ;

#line 502 "../../src/cfront.h"
union _C5 _expr__O5 ;
};

#line 557 "../../src/cfront.h"
union _C6 {	/* sizeof _C6 == 4 */

#line 578 "../../src/cfront.h"
Pname __C6_n_qualifier ;
Ptable __C6_n_realscope ;
};
struct name {	/* sizeof name == 60 */

#line 166 "../../src/cfront.h"
TOK _node_base ;
TOK _node_n_key ;
bit _node_permanent ;

#line 483 "../../src/cfront.h"
union _C2 _expr__O2 ;

#line 488 "../../src/cfront.h"
union _C3 _expr__O3 ;

#line 493 "../../src/cfront.h"
union _C4 _expr__O4 ;

#line 502 "../../src/cfront.h"
union _C5 _expr__O5 ;

#line 558 "../../src/cfront.h"
TOK _name_n_oper ;
TOK _name_n_sto ;
TOK _name_n_stclass ;
TOK _name_n_scope ;
unsigned char _name_n_union ;
bit _name_n_evaluated ;
bit _name_n_xref ;
unsigned char _name_lex_level ;
TOK _name_n_protect ;
short _name_n_addr_taken ;
short _name_n_used ;
short _name_n_assigned_to ;
Loc _name_where ;
int _name_n_val ;

#line 574 "../../src/cfront.h"
int _name_n_offset ;
Pname _name_n_list ;
Pname _name_n_tbl_list ;

#line 583 "../../src/cfront.h"
union _C6 _name__O6 ;
};

#line 585 "../../src/cfront.h"
struct name *_name__ctor ();
char _name__dtor ();

#line 588 "../../src/cfront.h"
Pname _name_normalize ();
Pname _name_tdef ();
Pname _name_tname ();
char _name_hide ();

#line 594 "../../src/cfront.h"
Pname _name_dcl ();
int _name_no_of_names ();

#line 597 "../../src/cfront.h"
char _name_assign ();

#line 599 "../../src/cfront.h"
char _name_check_oper ();
char _name_simpl ();
char _name_del ();
char _name_print ();
char _name_dcl_print ();
char _name_field_align ();
Pname _name_dofct ();

#line 610 "../../src/cfront.h"
extern int friend_in_class ;

#line 615 "../../src/cfront.h"
union _C7 {	/* sizeof _C7 == 4 */

#line 621 "../../src/cfront.h"
Pname __C7_d ;
Pexpr __C7_e2 ;
Pstmt __C7_has_default ;
int __C7_case_value ;
Ptype __C7_ret_tp ;
};
union _C8 {	/* sizeof _C8 == 4 */

#line 628 "../../src/cfront.h"
Pexpr __C8_e ;
bit __C8_own_tbl ;
Pstmt __C8_s2 ;
};
union _C9 {	/* sizeof _C9 == 4 */

#line 634 "../../src/cfront.h"
Pstmt __C9_for_init ;
Pstmt __C9_else_stmt ;
Pstmt __C9_case_list ;
bit __C9_empty ;
};
struct stmt {	/* sizeof stmt == 32 */

#line 166 "../../src/cfront.h"
TOK _node_base ;
TOK _node_n_key ;
bit _node_permanent ;

#line 617 "../../src/cfront.h"
Pstmt _stmt_s ;
Pstmt _stmt_s_list ;
Loc _stmt_where ;

#line 626 "../../src/cfront.h"
union _C7 _stmt__O7 ;

#line 631 "../../src/cfront.h"
union _C8 _stmt__O8 ;
Ptable _stmt_memtbl ;

#line 638 "../../src/cfront.h"
union _C9 _stmt__O9 ;
};

#line 640 "../../src/cfront.h"
struct stmt *_stmt__ctor ();
char _stmt__dtor ();

#line 643 "../../src/cfront.h"
char _stmt_del ();
char _stmt_print ();
char _stmt_dcl ();
char _stmt_reached ();
Pstmt _stmt_simpl ();
Pstmt _stmt_expand ();
Pstmt _stmt_copy ();

#line 654 "../../src/cfront.h"
extern char *Neval ;
extern Pname dcl_temp ();
extern char *temp ();
extern Ptable scope ;
extern Ptable expand_tbl ;
extern Pname expand_fn ;

#line 668 "../../src/cfront.h"
struct estmt {	/* sizeof estmt == 32 */

#line 166 "../../src/cfront.h"
TOK _node_base ;
TOK _node_n_key ;
bit _node_permanent ;

#line 617 "../../src/cfront.h"
Pstmt _stmt_s ;
Pstmt _stmt_s_list ;
Loc _stmt_where ;

#line 626 "../../src/cfront.h"
union _C7 _stmt__O7 ;

#line 631 "../../src/cfront.h"
union _C8 _stmt__O8 ;
Ptable _stmt_memtbl ;

#line 638 "../../src/cfront.h"
union _C9 _stmt__O9 ;
};

#line 675 "../../src/cfront.h"
struct ifstmt {	/* sizeof ifstmt == 32 */

#line 166 "../../src/cfront.h"
TOK _node_base ;
TOK _node_n_key ;
bit _node_permanent ;

#line 617 "../../src/cfront.h"
Pstmt _stmt_s ;
Pstmt _stmt_s_list ;
Loc _stmt_where ;

#line 626 "../../src/cfront.h"
union _C7 _stmt__O7 ;

#line 631 "../../src/cfront.h"
union _C8 _stmt__O8 ;
Ptable _stmt_memtbl ;

#line 638 "../../src/cfront.h"
union _C9 _stmt__O9 ;
};

#line 685 "../../src/cfront.h"
struct lstmt {	/* sizeof lstmt == 32 */

#line 166 "../../src/cfront.h"
TOK _node_base ;
TOK _node_n_key ;
bit _node_permanent ;

#line 617 "../../src/cfront.h"
Pstmt _stmt_s ;
Pstmt _stmt_s_list ;
Loc _stmt_where ;

#line 626 "../../src/cfront.h"
union _C7 _stmt__O7 ;

#line 631 "../../src/cfront.h"
union _C8 _stmt__O8 ;
Ptable _stmt_memtbl ;

#line 638 "../../src/cfront.h"
union _C9 _stmt__O9 ;
};

#line 689 "../../src/cfront.h"
struct forstmt {	/* sizeof forstmt == 32 */

#line 166 "../../src/cfront.h"
TOK _node_base ;
TOK _node_n_key ;
bit _node_permanent ;

#line 617 "../../src/cfront.h"
Pstmt _stmt_s ;
Pstmt _stmt_s_list ;
Loc _stmt_where ;

#line 626 "../../src/cfront.h"
union _C7 _stmt__O7 ;

#line 631 "../../src/cfront.h"
union _C8 _stmt__O8 ;
Ptable _stmt_memtbl ;

#line 638 "../../src/cfront.h"
union _C9 _stmt__O9 ;
};

#line 694 "../../src/cfront.h"
struct block {	/* sizeof block == 32 */

#line 166 "../../src/cfront.h"
TOK _node_base ;
TOK _node_n_key ;
bit _node_permanent ;

#line 617 "../../src/cfront.h"
Pstmt _stmt_s ;
Pstmt _stmt_s_list ;
Loc _stmt_where ;

#line 626 "../../src/cfront.h"
union _C7 _stmt__O7 ;

#line 631 "../../src/cfront.h"
union _C8 _stmt__O8 ;
Ptable _stmt_memtbl ;

#line 638 "../../src/cfront.h"
union _C9 _stmt__O9 ;
};

#line 697 "../../src/cfront.h"
char _block_dcl ();
Pstmt _block_simpl ();

#line 703 "../../src/cfront.h"
struct pair {	/* sizeof pair == 32 */

#line 166 "../../src/cfront.h"
TOK _node_base ;
TOK _node_n_key ;
bit _node_permanent ;

#line 617 "../../src/cfront.h"
Pstmt _stmt_s ;
Pstmt _stmt_s_list ;
Loc _stmt_where ;

#line 626 "../../src/cfront.h"
union _C7 _stmt__O7 ;

#line 631 "../../src/cfront.h"
union _C8 _stmt__O8 ;
Ptable _stmt_memtbl ;

#line 638 "../../src/cfront.h"
union _C9 _stmt__O9 ;
};

#line 708 "../../src/cfront.h"
struct nlist {	/* sizeof nlist == 8 */

#line 709 "../../src/cfront.h"
Pname _nlist_head ;
Pname _nlist_tail ;
};
struct nlist *_nlist__ctor ();

#line 713 "../../src/cfront.h"
char _nlist_add_list ();

#line 716 "../../src/cfront.h"
extern Pname name_unlist ();

#line 718 "../../src/cfront.h"
struct slist {	/* sizeof slist == 8 */

#line 719 "../../src/cfront.h"
Pstmt _slist_head ;
Pstmt _slist_tail ;
};

#line 725 "../../src/cfront.h"
extern Pstmt stmt_unlist ();

#line 727 "../../src/cfront.h"
struct elist {	/* sizeof elist == 8 */

#line 728 "../../src/cfront.h"
Pexpr _elist_head ;
Pexpr _elist_tail ;
};

#line 734 "../../src/cfront.h"
extern Pexpr expr_unlist ();

#line 737 "../../src/cfront.h"
extern struct dcl_context *cc ;

#line 739 "../../src/cfront.h"
struct dcl_context {	/* sizeof dcl_context == 24 */

#line 740 "../../src/cfront.h"
Pname _dcl_context_c_this ;
Ptype _dcl_context_tot ;
Pname _dcl_context_not ;
Pclass _dcl_context_cot ;
Ptable _dcl_context_ftbl ;
Pname _dcl_context_nof ;
};

#line 752 "../../src/cfront.h"
extern struct dcl_context ccvec [20];

#line 755 "../../src/cfront.h"
extern char yyerror ();
extern TOK back ;

#line 760 "../../src/cfront.h"
extern char *line_format ;

#line 762 "../../src/cfront.h"
extern Plist isf_list ;
extern Pstmt st_ilist ;
extern Pstmt st_dlist ;
extern Ptable sti_tbl ;
extern Ptable std_tbl ;
Pexpr try_to_coerce ();
extern bit can_coerce ();
extern Ptype np_promote ();
extern char new_key ();

#line 772 "../../src/cfront.h"
extern Pname dcl_list ;
extern int over_call ();
extern Pname Nover ;
extern Pname Ntncheck ;
extern Pname Ncoerce ;
extern int Nover_coerce ;

#line 779 "../../src/cfront.h"

#line 780 "../../src/cfront.h"
struct iline {	/* sizeof iline == 108 */

#line 781 "../../src/cfront.h"
Pname _iline_fct_name ;
Pin _iline_i_next ;
Ptable _iline_i_table ;
Pname _iline_local [8];
Pexpr _iline_arg [8];
Ptype _iline_tp [8];
};

#line 789 "../../src/cfront.h"
extern Pexpr curr_expr ;
extern Pin curr_icall ;

#line 793 "../../src/cfront.h"
extern Pstmt curr_loop ;
extern Pblock curr_block ;
extern Pstmt curr_switch ;
extern bit arg_err_suppress ;
extern struct loc last_line ;

#line 799 "../../src/cfront.h"
extern int no_of_undcl ;
extern int no_of_badcall ;
extern Pname undcl ;

#line 801 "../../src/cfront.h"
extern Pname badcall ;

#line 803 "../../src/cfront.h"
extern int strlen ();
extern char *strcpy ();
extern int str_to_int ();
extern int c_strlen ();

#line 809 "../../src/cfront.h"
extern int strcmp ();

#line 812 "../../src/cfront.h"
extern Pname vec_new_fct ;
extern Pname vec_del_fct ;

#line 815 "../../src/cfront.h"
extern int Nstd ;

#line 817 "../../src/cfront.h"
extern int stcount ;

#line 819 "../../src/cfront.h"
extern Pname find_hidden ();
Pexpr replace_temp ();
char make_res ();
Pexpr ptr_init ();

#line 826 "../../src/cfront.h"
extern bit fake_sizeof ;

#line 828 "../../src/cfront.h"
extern TOK lalex ();

#ident	"@(#)sdb:cfront/src/size.h	1.1"

#line 19 "../../src/size.h"
extern int BI_IN_WORD ;
extern int BI_IN_BYTE ;

#line 22 "../../src/size.h"
extern int SZ_CHAR ;
extern int AL_CHAR ;

#line 25 "../../src/size.h"
extern int SZ_SHORT ;
extern int AL_SHORT ;

#line 28 "../../src/size.h"
extern int SZ_INT ;
extern int AL_INT ;

#line 31 "../../src/size.h"
extern int SZ_LONG ;
extern int AL_LONG ;

#line 34 "../../src/size.h"
extern int SZ_FLOAT ;
extern int AL_FLOAT ;

#line 37 "../../src/size.h"
extern int SZ_DOUBLE ;
extern int AL_DOUBLE ;

#line 40 "../../src/size.h"
extern int SZ_STRUCT ;
extern int AL_STRUCT ;

#line 46 "../../src/size.h"
extern int SZ_WORD ;

#line 48 "../../src/size.h"
extern int SZ_WPTR ;
extern int AL_WPTR ;

#line 51 "../../src/size.h"
extern int SZ_BPTR ;
extern int AL_BPTR ;

#line 57 "../../src/size.h"
extern char *LARGEST_INT ;
extern int F_SENSITIVE ;
extern int F_OPTIMIZED ;

#line 286 "../../src/size.h"

#line 287 "../../src/size.h"
char *chunk ();

#line 21 "../../src/expr.c"
Pname make_tmp ();
Pexpr init_tmp ();

#line 24 "../../src/expr.c"
int const_save = 0 ;

#line 26 "../../src/expr.c"
Pexpr _expr_address (_au0_this )
#line 525 "../../src/cfront.h"
struct expr *_au0_this ;

#line 27 "../../src/expr.c"
{ 
#line 52 "../../src/expr.c"
register Pexpr _au1_ee ;

#line 53 "../../src/expr.c"
struct ptr *_au0__Xthis__ctor_ptr ;

#line 29 "../../src/expr.c"
switch (_au0_this -> _node_base ){ 
#line 30 "../../src/expr.c"
case 111 : 
#line 31 "../../src/expr.c"
if (_au0_this -> _expr__O4.__C4_e2 == 0 )return _au0_this -> _expr__O3.__C3_e1 ;
break ;
case 71 : 
#line 34 "../../src/expr.c"
case 147 : 
#line 35 "../../src/expr.c"
_au0_this -> _expr__O4.__C4_e2 = _expr_address ( _au0_this -> _expr__O4.__C4_e2 ) ;
return _au0_this ;
case 109 : 
#line 38 "../../src/expr.c"
case 146 : 
#line 39 "../../src/expr.c"
{ 
#line 40 "../../src/expr.c"
Pname _au3_fn ;
Pfct _au3_f ;

#line 40 "../../src/expr.c"
_au3_fn = _au0_this -> _expr__O5.__C5_fct_name ;
_au3_f = (_au3_fn ? (((struct fct *)_au3_fn -> _expr__O2.__C2_tp )): (((struct fct *)0 )));
if ((_au3_f && _au3_f -> _fct_f_inline )&& (_au3_fn -> _name_n_used > 1 ))
#line 43 "../../src/expr.c"
return _au0_this ;
break ;
}
case 85 : 
#line 47 "../../src/expr.c"
if ((((struct name *)_au0_this ))-> _name_n_stclass == 27 )
#line 48 "../../src/expr.c"
{ 
#line 68 "../../src/expr.c"
struct ea _au0__V10 ;

#line 48 "../../src/expr.c"
error ( (char *)"& register%n", (struct ea *)( ( ((& _au0__V10 )-> _ea__O1.__C1_p = ((char *)(((struct name *)_au0_this )))), (((& _au0__V10 ))))
#line 48 "../../src/expr.c"
) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} ( ((((struct name *)_au0_this ))-> _name_n_addr_taken ++ )) ;
}

#line 52 "../../src/expr.c"
_au1_ee = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )145 , (struct expr *)0 , _au0_this ) ;
if (_au0_this -> _expr__O2.__C2_tp ){ 
#line 54 "../../src/expr.c"
_au1_ee -> _expr__O2.__C2_tp = (struct type *)( (((struct ptr *)( (_au0__Xthis__ctor_ptr = 0 ), ( (_au0__Xthis__ctor_ptr = (struct
#line 54 "../../src/expr.c"
ptr *)_new ( (long )(sizeof (struct ptr))) ), ( (Nt ++ ), ( (_au0__Xthis__ctor_ptr -> _node_base = ((unsigned char )125 )),
#line 54 "../../src/expr.c"
( (_au0__Xthis__ctor_ptr -> _pvtyp_typ = _au0_this -> _expr__O2.__C2_tp ), ( (_au0__Xthis__ctor_ptr -> _ptr_rdo = ((char )0 )), ((_au0__Xthis__ctor_ptr ))) ) ) )
#line 54 "../../src/expr.c"
) ) ))) ;
switch (_au0_this -> _expr__O2.__C2_tp -> _node_base ){ 
#line 56 "../../src/expr.c"
case 125 : 
#line 57 "../../src/expr.c"
(((struct ptr *)_au1_ee -> _expr__O2.__C2_tp ))-> _ptr_memof = (((struct ptr *)_au0_this -> _expr__O2.__C2_tp ))-> _ptr_memof ;
#line 57 "../../src/expr.c"

#line 58 "../../src/expr.c"
break ;
case 108 : 
#line 60 "../../src/expr.c"
(((struct ptr *)_au1_ee -> _expr__O2.__C2_tp ))-> _ptr_memof = (((struct fct *)_au0_this -> _expr__O2.__C2_tp ))-> _fct_memof ;
break ;
case 76 : 
#line 63 "../../src/expr.c"
(((struct ptr *)_au1_ee -> _expr__O2.__C2_tp ))-> _ptr_memof = (((struct fct *)(((struct gen *)_au0_this -> _expr__O2.__C2_tp ))-> _gen_fct_list -> _name_list_f -> _expr__O2.__C2_tp ))-> _fct_memof ;
#line 63 "../../src/expr.c"
}
}

#line 67 "../../src/expr.c"
return _au1_ee ;
}
;
Pexpr _expr_contents (_au0_this )
#line 525 "../../src/cfront.h"
struct expr *_au0_this ;

#line 71 "../../src/expr.c"
{ 
#line 74 "../../src/expr.c"
register Pexpr _au1_ee ;

#line 73 "../../src/expr.c"
if ((_au0_this -> _node_base == 112 )|| (_au0_this -> _node_base == 145 ))return _au0_this -> _expr__O4.__C4_e2 ;
_au1_ee = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )111 , _au0_this , (struct expr *)0 ) ;
if (_au0_this -> _expr__O2.__C2_tp )_au1_ee -> _expr__O2.__C2_tp = (((struct ptr *)_au0_this -> _expr__O2.__C2_tp ))-> _pvtyp_typ ;
return _au1_ee ;
}
;
int bound = 0 ;
int chars_in_largest = 0 ;

#line 82 "../../src/expr.c"
Pexpr _expr_typ (_au0_this , _au0_tbl )
#line 525 "../../src/cfront.h"
struct expr *_au0_this ;

#line 82 "../../src/expr.c"
Ptable _au0_tbl ;

#line 88 "../../src/expr.c"
{ 
#line 90 "../../src/expr.c"
Pname _au1_n ;
Ptype _au1_t ;
Ptype _au1_t1 ;

#line 92 "../../src/expr.c"
Ptype _au1_t2 ;
TOK _au1_b ;
TOK _au1_r1 ;

#line 94 "../../src/expr.c"
TOK _au1_r2 ;

#line 95 "../../src/expr.c"
struct ptr *_au0__Xthis__ctor_ptr ;

#line 91 "../../src/expr.c"
_au1_t = 0 ;

#line 93 "../../src/expr.c"
_au1_b = _au0_this -> _node_base ;

#line 98 "../../src/expr.c"
if (_au0_tbl -> _node_base != 142 ){ 
#line 887 "../../src/expr.c"
struct ea _au0__V11 ;

#line 98 "../../src/expr.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"expr::typ(%d)", (struct ea *)( ( ((& _au0__V11 )-> _ea__O1.__C1_i = ((int )_au0_tbl -> _node_base )),
#line 98 "../../src/expr.c"
(((& _au0__V11 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} 
#line 100 "../../src/expr.c"
if (_au0_this -> _expr__O2.__C2_tp ){ 
#line 102 "../../src/expr.c"
if (_au1_b == 85 )( ((((struct name *)_au0_this ))-> _name_n_used ++ )) ;
return _au0_this ;
}

#line 106 "../../src/expr.c"
switch (_au1_b ){ 
#line 107 "../../src/expr.c"
case 144 : 
#line 108 "../../src/expr.c"
error ( (char *)"emptyE", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 108 "../../src/expr.c"
ea *)ea0 ) ;
_au0_this -> _expr__O2.__C2_tp = (struct type *)any_type ;
return _au0_this ;

#line 112 "../../src/expr.c"
case 86 : 
#line 113 "../../src/expr.c"
_au0_this -> _expr__O2.__C2_tp = (struct type *)zero_type ;
return _au0_this ;

#line 116 "../../src/expr.c"
case 150 : 
#line 117 "../../src/expr.c"
_au0_this -> _expr__O2.__C2_tp = (struct type *)int_type ;
return _au0_this ;

#line 120 "../../src/expr.c"
case 151 : 
#line 121 "../../src/expr.c"
_au0_this -> _expr__O2.__C2_tp = (struct type *)float_type ;
return _au0_this ;

#line 124 "../../src/expr.c"
case 82 : 
#line 130 "../../src/expr.c"
{ int _au3_ll ;

#line 131 "../../src/expr.c"
struct texpr *_au0__Xthis__ctor_texpr ;

#line 130 "../../src/expr.c"
_au3_ll = strlen ( (char *)_au0_this -> _expr__O3.__C3_string ) ;
switch (_au0_this -> _expr__O3.__C3_string [_au3_ll - 1 ]){ 
#line 132 "../../src/expr.c"
case 'l' : 
#line 133 "../../src/expr.c"
case 'L' : 
#line 134 "../../src/expr.c"
switch (_au0_this -> _expr__O3.__C3_string [_au3_ll - 2 ]){ 
#line 135 "../../src/expr.c"
case 'u' :
#line 135 "../../src/expr.c"

#line 136 "../../src/expr.c"
case 'U' : 
#line 137 "../../src/expr.c"
(_au0_this -> _expr__O3.__C3_string [_au3_ll - 2 ])= 0 ;
_au0_this = (struct expr *)( (_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr = (struct texpr *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_texpr ),
#line 138 "../../src/expr.c"
((unsigned char )113 ), _au0_this , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 = ((struct type *)ulong_type )), ((_au0__Xthis__ctor_texpr )))
#line 138 "../../src/expr.c"
) ) ;
return _expr_typ ( _au0_this , _au0_tbl ) ;
}
lng :
#line 142 "../../src/expr.c"
_au0_this -> _expr__O2.__C2_tp = (struct type *)long_type ;
goto save ;
case 'u' : 
#line 145 "../../src/expr.c"
case 'U' : 
#line 146 "../../src/expr.c"
switch (_au0_this -> _expr__O3.__C3_string [_au3_ll - 2 ]){ 
#line 147 "../../src/expr.c"
case 'l' : 
#line 148 "../../src/expr.c"
case 'L' : 
#line 149 "../../src/expr.c"
(_au0_this -> _expr__O3.__C3_string [_au3_ll -
#line 149 "../../src/expr.c"
2 ])= 0 ;
_au1_t = (struct type *)ulong_type ;
break ;
default : 
#line 153 "../../src/expr.c"
(_au0_this -> _expr__O3.__C3_string [_au3_ll - 1 ])= 0 ;
_au1_t = (struct type *)uint_type ;
}
_au0_this = (struct expr *)( (_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr = (struct texpr *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_texpr ),
#line 156 "../../src/expr.c"
((unsigned char )113 ), _au0_this , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 = _au1_t ), ((_au0__Xthis__ctor_texpr ))) )
#line 156 "../../src/expr.c"
) ;
return _expr_typ ( _au0_this , _au0_tbl ) ;
}

#line 160 "../../src/expr.c"
if ((_au0_this -> _expr__O3.__C3_string [0 ])== '0' ){ 
#line 161 "../../src/expr.c"
switch (_au0_this -> _expr__O3.__C3_string [1 ]){ 
#line 162 "../../src/expr.c"
case 'x' : 
#line 163 "../../src/expr.c"
case 'X' : 
#line 164 "../../src/expr.c"
if ((SZ_INT + SZ_INT )<
#line 164 "../../src/expr.c"
(_au3_ll - 2 ))goto lng ;
goto nrm ;
default : 
#line 167 "../../src/expr.c"
if ((BI_IN_BYTE * SZ_INT )< ((_au3_ll - 1 )* 3 ))goto lng ;
goto nrm ;
}
}
else { 
#line 172 "../../src/expr.c"
if (_au3_ll < chars_in_largest ){ 
#line 173 "../../src/expr.c"
nrm :
#line 174 "../../src/expr.c"
_au0_this -> _expr__O2.__C2_tp = (struct type *)int_type ;
goto save ;
}
if (_au3_ll > chars_in_largest )goto lng ;
{ char *_au4_p ;
char *_au4_q ;

#line 178 "../../src/expr.c"
_au4_p = _au0_this -> _expr__O3.__C3_string ;
_au4_q = LARGEST_INT ;
do if ((*(_au4_p ++ ))> (*(_au4_q ++ )))goto lng ;
while (*_au4_p );
}
}

#line 183 "../../src/expr.c"
goto nrm ;
}
case 84 : 
#line 186 "../../src/expr.c"
_au0_this -> _expr__O2.__C2_tp = (struct type *)char_type ;
goto save ;

#line 189 "../../src/expr.c"
case 83 : 
#line 190 "../../src/expr.c"
_au0_this -> _expr__O2.__C2_tp = (struct type *)double_type ;
goto save ;

#line 193 "../../src/expr.c"
case 81 : 
#line 195 "../../src/expr.c"
{ Pvec _au3_v ;

#line 196 "../../src/expr.c"
struct vec *_au0__Xthis__ctor_vec ;

#line 195 "../../src/expr.c"
_au3_v = (struct vec *)( (_au0__Xthis__ctor_vec = 0 ), ( (_au0__Xthis__ctor_vec = (struct vec *)_new ( (long )(sizeof (struct vec))) ), (
#line 195 "../../src/expr.c"
(Nt ++ ), ( (_au0__Xthis__ctor_vec -> _node_base = 110 ), ( (_au0__Xthis__ctor_vec -> _pvtyp_typ = ((struct type *)char_type )), ( (_au0__Xthis__ctor_vec -> _vec_dim =
#line 195 "../../src/expr.c"
((struct expr *)0 )), ((_au0__Xthis__ctor_vec ))) ) ) ) ) ) ;
_au3_v -> _vec_size = c_strlen ( (char *)_au0_this -> _expr__O3.__C3_string ) ;
_au0_this -> _expr__O2.__C2_tp = (struct type *)_au3_v ;
}
save :
#line 200 "../../src/expr.c"
if (const_save ){ 
#line 201 "../../src/expr.c"
char *_au3_p ;

#line 201 "../../src/expr.c"
_au3_p = (((char *)_new ( (long )((sizeof (char ))* (strlen ( (char *)_au0_this -> _expr__O3.__C3_string ) + 1 ))) ));
#line 201 "../../src/expr.c"

#line 202 "../../src/expr.c"
strcpy ( _au3_p , (char *)_au0_this -> _expr__O3.__C3_string ) ;
_au0_this -> _expr__O3.__C3_string = _au3_p ;
}

#line 206 "../../src/expr.c"
return _au0_this ;

#line 208 "../../src/expr.c"
case 34 : 
#line 209 "../../src/expr.c"
_expr__dtor ( _au0_this , 1) ;
if (cc -> _dcl_context_tot ){ 
#line 211 "../../src/expr.c"
( (cc -> _dcl_context_c_this -> _name_n_used ++ )) ;
return (struct expr *)cc -> _dcl_context_c_this ;
}
error ( (char *)"this used in nonC context", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
_au1_n = (struct name *)_name__ctor ( (struct name *)0 , "this") ;
_au1_n -> _expr__O2.__C2_tp = (struct type *)any_type ;
return (struct expr *)_table_insert ( _au0_tbl , _au1_n , (unsigned char )0 ) ;

#line 219 "../../src/expr.c"
case 85 : 
#line 220 "../../src/expr.c"
{ Pexpr _au3_ee ;

#line 220 "../../src/expr.c"
_au3_ee = _table_find_name ( _au0_tbl , ((struct name *)_au0_this ), (char )0 , (struct expr *)0 ) ;

#line 222 "../../src/expr.c"
if (_au3_ee -> _expr__O2.__C2_tp -> _node_base == 158 )return _expr_contents ( _au3_ee ) ;

#line 224 "../../src/expr.c"
if ((_au3_ee -> _node_base == 85 )&& (((struct name *)_au3_ee ))-> _name_n_xref ){ 
#line 226 "../../src/expr.c"
_au3_ee = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char
#line 226 "../../src/expr.c"
)111 , _au3_ee , (struct expr *)0 ) ;
_au3_ee -> _expr__O2.__C2_tp = _au3_ee -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp ;
}

#line 230 "../../src/expr.c"
return _au3_ee ;
}

#line 233 "../../src/expr.c"
case 112 : 
#line 234 "../../src/expr.c"
case 145 : 
#line 235 "../../src/expr.c"
if (_au0_this -> _expr__O4.__C4_e2 -> _node_base == 85 )_au0_this -> _expr__O4.__C4_e2 = _table_find_name ( _au0_tbl , ((struct name *)_au0_this ->
#line 235 "../../src/expr.c"
_expr__O4.__C4_e2 ), (char )3 , (struct expr *)0 ) ;
if ((_au0_this -> _expr__O4.__C4_e2 -> _node_base == 85 )&& (((struct name *)_au0_this -> _expr__O4.__C4_e2 ))-> _name_n_xref ){ 
#line 238 "../../src/expr.c"
_au0_this -> _expr__O4.__C4_e2 = (struct expr *)_expr__ctor ( (struct
#line 238 "../../src/expr.c"
expr *)0 , (unsigned char )111 , _au0_this -> _expr__O4.__C4_e2 , (struct expr *)0 ) ;
_au0_this -> _expr__O4.__C4_e2 -> _expr__O2.__C2_tp = _au0_this -> _expr__O4.__C4_e2 -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp ;
}
break ;

#line 243 "../../src/expr.c"
case 30 : 
#line 244 "../../src/expr.c"
if (_au0_this -> _expr__O5.__C5_tp2 ){ 
#line 245 "../../src/expr.c"
_type_dcl ( _au0_this -> _expr__O5.__C5_tp2 , _au0_tbl ) ;
if (_au0_this -> _expr__O3.__C3_e1 && (_au0_this -> _expr__O3.__C3_e1 != dummy )){ 
#line 247 "../../src/expr.c"
_au0_this -> _expr__O3.__C3_e1 = _expr_typ ( _au0_this -> _expr__O3.__C3_e1 , _au0_tbl ) ;
if (_au0_this -> _expr__O3.__C3_e1 && (_au0_this -> _expr__O3.__C3_e1 -> _node_permanent == 0 ))_expr_del ( _au0_this -> _expr__O3.__C3_e1 ) ;
_au0_this -> _expr__O3.__C3_e1 = dummy ;
}
}
else if (_au0_this -> _expr__O3.__C3_e1 == dummy ){ 
#line 253 "../../src/expr.c"
error ( (char *)"sizeof emptyE", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ,
#line 253 "../../src/expr.c"
(struct ea *)ea0 ) ;
_au0_this -> _expr__O2.__C2_tp = (struct type *)any_type ;
return _au0_this ;
}
else { 
#line 258 "../../src/expr.c"
_au0_this -> _expr__O3.__C3_e1 = _expr_typ ( _au0_this -> _expr__O3.__C3_e1 , _au0_tbl ) ;
_au0_this -> _expr__O5.__C5_tp2 = _au0_this -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp ;
}
_au0_this -> _expr__O2.__C2_tp = (struct type *)int_type ;
return _au0_this ;

#line 264 "../../src/expr.c"
case 113 : 
#line 265 "../../src/expr.c"
return _expr_docast ( _au0_this , _au0_tbl ) ;

#line 267 "../../src/expr.c"
case 157 : 
#line 268 "../../src/expr.c"
return _expr_dovalue ( _au0_this , _au0_tbl ) ;

#line 270 "../../src/expr.c"
case 23 : 
#line 271 "../../src/expr.c"
return _expr_donew ( _au0_this , _au0_tbl ) ;

#line 273 "../../src/expr.c"
case 9 : 
#line 274 "../../src/expr.c"
{ int _au3_i ;
if (_au0_this -> _expr__O3.__C3_e1 -> _node_base == 112 )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"delete &E", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 275 "../../src/expr.c"
ea *)ea0 , (struct ea *)ea0 ) ;
_au0_this -> _expr__O3.__C3_e1 = _expr_typ ( _au0_this -> _expr__O3.__C3_e1 , _au0_tbl ) ;
_au3_i = ( _type_kind ( _au0_this -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp , ((unsigned char )9 ), (unsigned char )'P' ) ) ;
#line 277 "../../src/expr.c"

#line 278 "../../src/expr.c"
if (_au3_i != 'P' )error ( (char *)"nonP deleted", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 278 "../../src/expr.c"

#line 279 "../../src/expr.c"
if (_au0_this -> _expr__O4.__C4_e2 ){ 
#line 280 "../../src/expr.c"
_au0_this -> _expr__O4.__C4_e2 = _expr_typ ( _au0_this -> _expr__O4.__C4_e2 , _au0_tbl ) ;
( _type_kind ( _au0_this -> _expr__O4.__C4_e2 -> _expr__O2.__C2_tp , ((unsigned char )9 ), (unsigned char )'I' ) ) ;
}
_au0_this -> _expr__O2.__C2_tp = (struct type *)void_type ;
return _au0_this ;
}

#line 287 "../../src/expr.c"
case 124 : 
#line 288 "../../src/expr.c"
_au0_this -> _expr__O3.__C3_e1 = _expr_typ ( _au0_this -> _expr__O3.__C3_e1 , _au0_tbl ) ;
_au0_this -> _expr__O2.__C2_tp = (struct type *)any_type ;
return _au0_this ;

#line 292 "../../src/expr.c"
case 140 : 
#line 293 "../../src/expr.c"
{ Pexpr _au3_e ;
Pexpr _au3_ex ;

#line 296 "../../src/expr.c"
if ((_au0_this -> _expr__O3.__C3_e1 == dummy )&& (_au0_this -> _expr__O4.__C4_e2 == 0 )){ 
#line 297 "../../src/expr.c"
error ( (char *)"emptyIrL", (struct ea *)ea0 , (struct ea *)ea0 ,
#line 297 "../../src/expr.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
_au0_this -> _expr__O2.__C2_tp = (struct type *)any_type ;
return _au0_this ;
}

#line 302 "../../src/expr.c"
for(_au3_e = _au0_this ;_au3_e ;_au3_e = _au3_ex ) { 
#line 303 "../../src/expr.c"
Pexpr _au4_ee ;

#line 303 "../../src/expr.c"
_au4_ee = _au3_e -> _expr__O3.__C3_e1 ;

#line 305 "../../src/expr.c"
if (_au3_e -> _node_base != 140 ){ 
#line 887 "../../src/expr.c"
struct ea _au0__V12 ;

#line 305 "../../src/expr.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"elist%k", (struct ea *)( ( ((& _au0__V12 )-> _ea__O1.__C1_i = ((int )_au3_e -> _node_base )),
#line 305 "../../src/expr.c"
(((& _au0__V12 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} if (_au3_ex = _au3_e -> _expr__O4.__C4_e2 ){ 
#line 307 "../../src/expr.c"
if (_au4_ee == dummy )error ( (char *)"EX in EL", (struct ea *)ea0 , (struct ea *)ea0 ,
#line 307 "../../src/expr.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
if ((_au3_ex -> _expr__O3.__C3_e1 == dummy )&& (_au3_ex -> _expr__O4.__C4_e2 == 0 )){ 
#line 310 "../../src/expr.c"
if (_au3_ex && (_au3_ex -> _node_permanent == 0 ))_expr_del ( _au3_ex ) ;
#line 310 "../../src/expr.c"

#line 311 "../../src/expr.c"
_au3_e -> _expr__O4.__C4_e2 = (_au3_ex = 0 );
}
}
_au3_e -> _expr__O3.__C3_e1 = _expr_typ ( _au4_ee , _au0_tbl ) ;
_au1_t = _au3_e -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp ;
}

#line 318 "../../src/expr.c"
_au0_this -> _expr__O2.__C2_tp = _au1_t ;
return _au0_this ;
}

#line 322 "../../src/expr.c"
case 45 : 
#line 323 "../../src/expr.c"
case 44 : 
#line 325 "../../src/expr.c"
if (_au0_this -> _expr__O4.__C4_e2 ){ 
#line 326 "../../src/expr.c"
Pexpr _au3_a ;

#line 326 "../../src/expr.c"
_au3_a = _expr_typ ( _au0_this -> _expr__O3.__C3_e1 , _au0_tbl ) ;
if (_au0_this -> _node_base == 45 )_au3_a = _expr_address ( _au3_a ) ;
{ Pexpr _au3_p ;
Ptype _au3_pt ;

#line 328 "../../src/expr.c"
_au3_p = _expr_typ ( _au0_this -> _expr__O4.__C4_e2 , _au0_tbl ) ;
_au3_pt = _au3_p -> _expr__O2.__C2_tp ;

#line 331 "../../src/expr.c"
while (_au3_pt -> _node_base == 97 )_au3_pt = (((struct basetype *)_au3_pt ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
if ((_au3_pt -> _node_base != 125 )|| ((((struct ptr *)_au3_pt ))-> _ptr_memof == 0 )){ 
#line 333 "../../src/expr.c"
{ 
#line 887 "../../src/expr.c"
struct ea _au0__V13 ;

#line 333 "../../src/expr.c"
error ( (char *)"P toMFX in .*E: %t", (struct ea *)( ( ((& _au0__V13 )-> _ea__O1.__C1_p = ((char *)_au3_pt )), (((& _au0__V13 )))) )
#line 333 "../../src/expr.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
_au0_this -> _expr__O2.__C2_tp = (struct type *)any_type ;
return _au0_this ;
} }
{ Pclass _au3_pm ;
Pname _au3_cn ;
Pclass _au3_mm ;

#line 337 "../../src/expr.c"
_au3_pm = (((struct ptr *)_au3_pt ))-> _ptr_memof ;
_au3_cn = _type_is_cl_obj ( (((struct ptr *)_au3_a -> _expr__O2.__C2_tp ))-> _pvtyp_typ ) ;
_au3_mm = (_au3_cn ? (((struct classdef *)_au3_cn -> _expr__O2.__C2_tp )): (((struct classdef *)0 )));
if ((_au3_mm != _au3_pm )&& (_classdef_baseofFPCclassdef___ ( _au3_pm , _au3_mm ) == 0 )){ 
#line 341 "../../src/expr.c"
{ 
#line 887 "../../src/expr.c"
struct ea _au0__V14 ;

#line 887 "../../src/expr.c"
struct ea _au0__V15 ;

#line 341 "../../src/expr.c"
error ( (char *)"badOT in .*E: %t (%s*X)", (struct ea *)( ( ((& _au0__V14 )-> _ea__O1.__C1_p = ((char *)_au3_a -> _expr__O2.__C2_tp )), (((& _au0__V14 ))))
#line 341 "../../src/expr.c"
) , (struct ea *)( ( ((& _au0__V15 )-> _ea__O1.__C1_p = ((char *)_au3_pm -> _classdef_string )), (((& _au0__V15 )))) )
#line 341 "../../src/expr.c"
, (struct ea *)ea0 , (struct ea *)ea0 ) ;
_au0_this -> _expr__O2.__C2_tp = (struct type *)any_type ;
return _au0_this ;
} }
{ Ptype _au3_tx ;

#line 346 "../../src/expr.c"
struct texpr *_au0__Xthis__ctor_texpr ;

#line 345 "../../src/expr.c"
_au3_tx = (((struct ptr *)_au3_pt ))-> _pvtyp_typ ;
while (_au3_tx -> _node_base == 97 )_au3_tx = (((struct basetype *)_au3_tx ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
if (_au3_tx -> _node_base == 108 ){ 
#line 348 "../../src/expr.c"
_au0_this -> _node_base = 173 ;
_au0_this -> _expr__O5.__C5_tp2 = (struct type *)_au3_mm ;
_au0_this -> _expr__O3.__C3_e1 = _au3_a ;
_au0_this -> _expr__O4.__C4_e2 = _au3_p ;
}
else { 
#line 354 "../../src/expr.c"
_au3_a = (struct expr *)( (_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr = (struct texpr *)_expr__ctor (
#line 354 "../../src/expr.c"
((struct expr *)_au0__Xthis__ctor_texpr ), ((unsigned char )113 ), _au3_a , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 = Pchar_type ),
#line 354 "../../src/expr.c"
((_au0__Xthis__ctor_texpr ))) ) ) ;
_au3_a -> _expr__O2.__C2_tp = Pchar_type ;
_au3_p = (struct expr *)( (_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr = (struct texpr *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_texpr ),
#line 356 "../../src/expr.c"
((unsigned char )113 ), _au3_p , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 = ((struct type *)int_type )), ((_au0__Xthis__ctor_texpr )))
#line 356 "../../src/expr.c"
) ) ;
_au3_p -> _expr__O2.__C2_tp = (struct type *)int_type ;
_au3_p = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )55 , _au3_p , one ) ;
_au3_p -> _expr__O2.__C2_tp = (struct type *)int_type ;
{ Pexpr _au4_pl ;

#line 361 "../../src/expr.c"
struct texpr *_au0__Xthis__ctor_texpr ;

#line 360 "../../src/expr.c"
_au4_pl = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )54 , _au3_a , _au3_p ) ;
_au4_pl -> _expr__O2.__C2_tp = Pint_type ;
_au0_this -> _node_base = 111 ;
_au0_this -> _expr__O3.__C3_e1 = (struct expr *)( (_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr = (struct texpr *)_expr__ctor ( ((struct
#line 363 "../../src/expr.c"
expr *)_au0__Xthis__ctor_texpr ), ((unsigned char )113 ), _au4_pl , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 = _au3_pt ), ((_au0__Xthis__ctor_texpr )))
#line 363 "../../src/expr.c"
) ) ;
_au0_this -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp = _au3_pt ;
_au0_this -> _expr__O4.__C4_e2 = 0 ;
}
}

#line 367 "../../src/expr.c"
_au0_this -> _expr__O2.__C2_tp = _au3_tx ;
if (_au0_this -> _expr__O2.__C2_tp -> _node_base == 158 )return _expr_contents ( _au0_this ) ;
return _au0_this ;
}
}
}
}
else 
#line 371 "../../src/expr.c"
{ 
#line 372 "../../src/expr.c"
Pbase _au3_b ;
Ptable _au3_atbl ;
Pname _au3_nn ;
char *_au3_s ;
Pclass _au3_cl ;

#line 378 "../../src/expr.c"
_au0_this -> _expr__O3.__C3_e1 = _expr_typ ( _au0_this -> _expr__O3.__C3_e1 , _au0_tbl ) ;
_au1_t = _au0_this -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp ;
if (_au0_this -> _node_base == 44 ){ 
#line 381 "../../src/expr.c"
xxx :
#line 383 "../../src/expr.c"
switch (_au1_t -> _node_base ){ 
#line 384 "../../src/expr.c"
case 97 : _au1_t = (((struct basetype *)_au1_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
#line 384 "../../src/expr.c"
goto xxx ;
default : { 
#line 887 "../../src/expr.c"
struct ea _au0__V16 ;

#line 385 "../../src/expr.c"
error ( (char *)"nonP ->%n", (struct ea *)( ( ((& _au0__V16 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _expr__O5.__C5_mem )), (((& _au0__V16 ))))
#line 385 "../../src/expr.c"
) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;

#line 385 "../../src/expr.c"
_au1_t = (struct type *)any_type ;
case 141 : goto qqq ;
case 125 : 
#line 388 "../../src/expr.c"
case 110 : _au3_b = (((struct basetype *)(((struct ptr *)_au1_t ))-> _pvtyp_typ ));

#line 388 "../../src/expr.c"
break ;
} }
}
else { 
#line 392 "../../src/expr.c"
qqq :
#line 393 "../../src/expr.c"
switch (_au1_t -> _node_base ){ 
#line 394 "../../src/expr.c"
case 97 : _au1_t = (((struct basetype *)_au1_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 394 "../../src/expr.c"
goto qqq ;
default : { 
#line 887 "../../src/expr.c"
struct ea _au0__V17 ;

#line 395 "../../src/expr.c"
error ( (char *)"nonO .%n", (struct ea *)( ( ((& _au0__V17 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _expr__O5.__C5_mem )), (((& _au0__V17 ))))
#line 395 "../../src/expr.c"
) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;

#line 395 "../../src/expr.c"
_au1_t = (struct type *)any_type ;
case 141 : 
#line 397 "../../src/expr.c"
case 119 : break ;
} }

#line 401 "../../src/expr.c"
switch (_au0_this -> _expr__O3.__C3_e1 -> _node_base ){ 
#line 402 "../../src/expr.c"
case 71 : 
#line 403 "../../src/expr.c"
{ Pexpr _au6_ex ;

#line 403 "../../src/expr.c"
_au6_ex = _au0_this -> _expr__O3.__C3_e1 ;
cfr :
#line 405 "../../src/expr.c"
switch (_au6_ex -> _expr__O4.__C4_e2 -> _node_base ){ 
#line 406 "../../src/expr.c"
case 85 : 
#line 407 "../../src/expr.c"
_au0_this -> _node_base = 44 ;
_au6_ex -> _expr__O4.__C4_e2 = _expr_address ( _au6_ex -> _expr__O4.__C4_e2 ) ;
_au0_this -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp = (_au1_t = _au6_ex -> _expr__O4.__C4_e2 -> _expr__O2.__C2_tp );
goto xxx ;
case 71 : 
#line 412 "../../src/expr.c"
_au6_ex = _au6_ex -> _expr__O4.__C4_e2 ;
goto cfr ;
}
break ;
}
case 109 : 
#line 418 "../../src/expr.c"
case 146 : 
#line 419 "../../src/expr.c"
{ Pname _au6_tmp ;

#line 419 "../../src/expr.c"
_au6_tmp = make_tmp ( 'T' , _au0_this -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp , _au0_tbl ) ;
_au0_this -> _expr__O3.__C3_e1 = init_tmp ( _au6_tmp , _au0_this -> _expr__O3.__C3_e1 , _au0_tbl ) ;
{ Pexpr _au6_aa ;

#line 421 "../../src/expr.c"
_au6_aa = _expr_address ( (struct expr *)_au6_tmp ) ;
_au0_this -> _expr__O3.__C3_e1 = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )71 , _au0_this -> _expr__O3.__C3_e1 , _au6_aa ) ;
_au0_this -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp = _au6_aa -> _expr__O2.__C2_tp ;
_au0_this -> _node_base = 44 ;
break ;
}
}
}

#line 428 "../../src/expr.c"
_au3_b = (((struct basetype *)_au1_t ));
}

#line 431 "../../src/expr.c"
xxxx :
#line 432 "../../src/expr.c"
switch (_au3_b -> _node_base ){ 
#line 433 "../../src/expr.c"
case 97 : 
#line 434 "../../src/expr.c"
_au3_b = (((struct basetype *)_au3_b -> _basetype_b_name -> _expr__O2.__C2_tp ));
goto xxxx ;
default : 
#line 437 "../../src/expr.c"
{ 
#line 887 "../../src/expr.c"
struct ea _au0__V18 ;

#line 887 "../../src/expr.c"
struct ea _au0__V19 ;

#line 887 "../../src/expr.c"
struct ea _au0__V20 ;

#line 887 "../../src/expr.c"
struct ea _au0__V21 ;

#line 437 "../../src/expr.c"
error ( (char *)"(%t) before %k%n (%n not aM)", (struct ea *)( ( ((& _au0__V18 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp )), (((&
#line 437 "../../src/expr.c"
_au0__V18 )))) ) , (struct ea *)( ( ((& _au0__V19 )-> _ea__O1.__C1_i = ((int )_au0_this -> _node_base )), (((& _au0__V19 ))))
#line 437 "../../src/expr.c"
) , (struct ea *)( ( ((& _au0__V20 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _expr__O5.__C5_mem )), (((& _au0__V20 )))) )
#line 437 "../../src/expr.c"
, (struct ea *)( ( ((& _au0__V21 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _expr__O5.__C5_mem )), (((& _au0__V21 )))) ) )
#line 437 "../../src/expr.c"
;
case 141 : 
#line 439 "../../src/expr.c"
_au3_atbl = any_tbl ;
break ;
case 119 : 
#line 442 "../../src/expr.c"
if (_au3_atbl = _au3_b -> _basetype_b_table )break ;
_au3_s = _au3_b -> _basetype_b_name -> _expr__O3.__C3_string ;
if (_au3_s == 0 ){ 
#line 887 "../../src/expr.c"
struct ea _au0__V22 ;

#line 444 "../../src/expr.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"%kN missing", (struct ea *)( ( ((& _au0__V22 )-> _ea__O1.__C1_i = 6 ), (((& _au0__V22 ))))
#line 444 "../../src/expr.c"
) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} 
#line 446 "../../src/expr.c"
_au3_nn = _table_look ( _au0_tbl , _au3_s , (unsigned char )6 ) ;
if (_au3_nn == 0 ){ 
#line 887 "../../src/expr.c"
struct ea _au0__V23 ;

#line 887 "../../src/expr.c"
struct ea _au0__V24 ;

#line 447 "../../src/expr.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"%k %sU", (struct ea *)( ( ((& _au0__V23 )-> _ea__O1.__C1_i = 6 ), (((& _au0__V23 ))))
#line 447 "../../src/expr.c"
) , (struct ea *)( ( ((& _au0__V24 )-> _ea__O1.__C1_p = ((char *)_au3_s )), (((& _au0__V24 )))) ) ,
#line 447 "../../src/expr.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
} if (_au3_nn != _au3_b -> _basetype_b_name )_au3_b -> _basetype_b_name = _au3_nn ;
_au3_cl = (((struct classdef *)_au3_nn -> _expr__O2.__C2_tp ));
_au3_cl -> _node_permanent = 1 ;
if (_au3_cl == 0 ){ 
#line 887 "../../src/expr.c"
struct ea _au0__V25 ;

#line 887 "../../src/expr.c"
struct ea _au0__V26 ;

#line 451 "../../src/expr.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"%k %s'sT missing", (struct ea *)( ( ((& _au0__V25 )-> _ea__O1.__C1_i = 6 ), (((& _au0__V25 ))))
#line 451 "../../src/expr.c"
) , (struct ea *)( ( ((& _au0__V26 )-> _ea__O1.__C1_p = ((char *)_au3_s )), (((& _au0__V26 )))) ) ,
#line 451 "../../src/expr.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
} _au3_b -> _basetype_b_table = (_au3_atbl = _au3_cl -> _classdef_memtbl );
} }

#line 455 "../../src/expr.c"
_au3_nn = (((struct name *)_table_find_name ( _au3_atbl , _au0_this -> _expr__O5.__C5_mem , (char )2 , (struct expr *)0 ) ));

#line 457 "../../src/expr.c"
if (_au3_nn -> _name_n_stclass == 0 ){ 
#line 458 "../../src/expr.c"
_au0_this -> _expr__O5.__C5_mem = _au3_nn ;
_au0_this -> _expr__O2.__C2_tp = _au3_nn -> _expr__O2.__C2_tp ;
if (_au0_this -> _expr__O2.__C2_tp -> _node_base == 158 )return _expr_contents ( _au0_this ) ;
return _au0_this ;
}
if (_au3_nn -> _expr__O2.__C2_tp -> _node_base == 158 )return _expr_contents ( _au0_this ) ;
return (struct expr *)_au3_nn ;
}

#line 467 "../../src/expr.c"
case 109 : 
#line 468 "../../src/expr.c"
if ((_au0_this -> _expr__O3.__C3_e1 -> _node_base == 85 )&& (_au0_this -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp == 0 )){ 
#line 470 "../../src/expr.c"
_au0_this -> _expr__O3.__C3_e1 = _table_find_name (
#line 470 "../../src/expr.c"
_au0_tbl , ((struct name *)_au0_this -> _expr__O3.__C3_e1 ), (char )1 , _au0_this -> _expr__O4.__C4_e2 ) ;
}
if ((_au0_this -> _expr__O3.__C3_e1 -> _node_base == 85 )&& (((struct name *)_au0_this -> _expr__O3.__C3_e1 ))-> _name_n_xref ){ 
#line 474 "../../src/expr.c"
_au0_this -> _expr__O3.__C3_e1 = (struct expr *)_expr__ctor ( (struct
#line 474 "../../src/expr.c"
expr *)0 , (unsigned char )111 , _au0_this -> _expr__O3.__C3_e1 , (struct expr *)0 ) ;
_au0_this -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp = _au0_this -> _expr__O3.__C3_e1 -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp ;
}
break ;

#line 479 "../../src/expr.c"
case 68 : 
#line 480 "../../src/expr.c"
_au0_this -> _expr__O5.__C5_cond = _expr_typ ( _au0_this -> _expr__O5.__C5_cond , _au0_tbl ) ;
}

#line 483 "../../src/expr.c"
if (_au0_this -> _expr__O3.__C3_e1 ){ 
#line 484 "../../src/expr.c"
_au0_this -> _expr__O3.__C3_e1 = _expr_typ ( _au0_this -> _expr__O3.__C3_e1 , _au0_tbl ) ;
if (_au0_this -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp -> _node_base == 158 )_au0_this -> _expr__O3.__C3_e1 = _expr_contents ( _au0_this -> _expr__O3.__C3_e1 ) ;
_au1_t1 = _au0_this -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp ;
}
else 
#line 489 "../../src/expr.c"
_au1_t1 = 0 ;

#line 491 "../../src/expr.c"
if (_au0_this -> _expr__O4.__C4_e2 ){ 
#line 492 "../../src/expr.c"
_au0_this -> _expr__O4.__C4_e2 = _expr_typ ( _au0_this -> _expr__O4.__C4_e2 , _au0_tbl ) ;
if (_au0_this -> _expr__O4.__C4_e2 -> _expr__O2.__C2_tp -> _node_base == 158 )_au0_this -> _expr__O4.__C4_e2 = _expr_contents ( _au0_this -> _expr__O4.__C4_e2 ) ;
_au1_t2 = _au0_this -> _expr__O4.__C4_e2 -> _expr__O2.__C2_tp ;
}
else 
#line 497 "../../src/expr.c"
_au1_t2 = 0 ;

#line 499 "../../src/expr.c"
switch (_au1_b ){ 
#line 500 "../../src/expr.c"
default : 
#line 501 "../../src/expr.c"
{ Pexpr _au3_x ;

#line 501 "../../src/expr.c"
_au3_x = _expr_try_to_overload ( _au0_this , _au0_tbl ) ;
if (_au3_x )return _au3_x ;
}
case 71 : 
#line 505 "../../src/expr.c"
case 147 : 
#line 506 "../../src/expr.c"
case 68 : 
#line 507 "../../src/expr.c"
case 145 : 
#line 508 "../../src/expr.c"
case 146 : 
#line 509 "../../src/expr.c"
break ;
}

#line 512 "../../src/expr.c"
_au1_t = ((_au1_t1 == 0 )? _au1_t2 : ((_au1_t2 == 0 )? _au1_t1 : (((struct type *)0 ))));

#line 515 "../../src/expr.c"
switch (_au1_b ){ 
#line 516 "../../src/expr.c"
case 146 : 
#line 517 "../../src/expr.c"
case 109 : 
#line 519 "../../src/expr.c"
_au0_this -> _expr__O2.__C2_tp = _expr_fct_call ( _au0_this , _au0_tbl ) ;
if (_au0_this -> _expr__O2.__C2_tp -> _node_base == 158 )return _expr_contents ( _au0_this ) ;
return _au0_this ;

#line 523 "../../src/expr.c"
case 111 : 
#line 525 "../../src/expr.c"
if (_au0_this -> _expr__O3.__C3_e1 == dummy )error ( (char *)"O missing before []\n", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ,
#line 525 "../../src/expr.c"
(struct ea *)ea0 ) ;
if (_au0_this -> _expr__O4.__C4_e2 == dummy )error ( (char *)"subscriptE missing", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 )
#line 526 "../../src/expr.c"
;
if (_au1_t ){ 
#line 528 "../../src/expr.c"
while (_au1_t -> _node_base == 97 )_au1_t = (((struct basetype *)_au1_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
_type_vec_type ( _au1_t ) ;
if ((((struct ptr *)_au1_t ))-> _ptr_memof )error ( (char *)"P toM deRd", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 )
#line 530 "../../src/expr.c"
;
_au0_this -> _expr__O2.__C2_tp = _type_deref ( _au1_t ) ;
}
else { 
#line 534 "../../src/expr.c"
if (_type_vec_type ( _au1_t1 ) ){ 
#line 535 "../../src/expr.c"
switch (_au1_t2 -> _node_base ){ 
#line 536 "../../src/expr.c"
case 5 : 
#line 537 "../../src/expr.c"
case 29 : 
#line 538 "../../src/expr.c"
case
#line 538 "../../src/expr.c"
21 : 
#line 539 "../../src/expr.c"
case 22 : 
#line 540 "../../src/expr.c"
case 121 : 
#line 541 "../../src/expr.c"
break ;
default : 
#line 543 "../../src/expr.c"
{ Pname _au6_cn ;

#line 543 "../../src/expr.c"
_au6_cn = _type_is_cl_obj ( _au1_t2 ) ;
if (_au6_cn )
#line 545 "../../src/expr.c"
_au0_this -> _expr__O4.__C4_e2 = check_cond ( _au0_this -> _expr__O4.__C4_e2 , (unsigned char )111 , _au0_tbl ) ;
else 
#line 547 "../../src/expr.c"
( _type_kind ( _au1_t2 , ((unsigned char )111 ), (unsigned char )'I' ) ) ;
}
}
while (_au1_t1 -> _node_base == 97 )_au1_t1 = (((struct basetype *)_au1_t1 ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
if ((((struct ptr *)_au1_t1 ))-> _ptr_memof )error ( (char *)"P toM deRd", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 )
#line 551 "../../src/expr.c"
;
_au0_this -> _expr__O2.__C2_tp = _type_deref ( _au1_t1 ) ;
}
else if (_type_vec_type ( _au1_t2 ) ){ 
#line 555 "../../src/expr.c"
( _type_kind ( _au1_t1 , ((unsigned char )111 ), (unsigned char )'I' )
#line 555 "../../src/expr.c"
) ;
while (_au1_t2 -> _node_base == 97 )_au1_t2 = (((struct basetype *)_au1_t2 ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
if ((((struct ptr *)_au1_t2 ))-> _ptr_memof )error ( (char *)"P toM deRd", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 )
#line 557 "../../src/expr.c"
;
_au0_this -> _expr__O2.__C2_tp = _type_deref ( _au1_t2 ) ;
}
else { 
#line 561 "../../src/expr.c"
{ 
#line 887 "../../src/expr.c"
struct ea _au0__V27 ;

#line 887 "../../src/expr.c"
struct ea _au0__V28 ;

#line 561 "../../src/expr.c"
error ( (char *)"[] applied to nonPT:%t[%t]", (struct ea *)( ( ((& _au0__V27 )-> _ea__O1.__C1_p = ((char *)_au1_t1 )), (((& _au0__V27 )))) )
#line 561 "../../src/expr.c"
, (struct ea *)( ( ((& _au0__V28 )-> _ea__O1.__C1_p = ((char *)_au1_t2 )), (((& _au0__V28 )))) ) , (struct
#line 561 "../../src/expr.c"
ea *)ea0 , (struct ea *)ea0 ) ;
_au0_this -> _expr__O2.__C2_tp = (struct type *)any_type ;
} }
}
if (_au0_this -> _expr__O2.__C2_tp -> _node_base == 158 )return _expr_contents ( _au0_this ) ;
return _au0_this ;

#line 568 "../../src/expr.c"
case 145 : 
#line 569 "../../src/expr.c"
case 112 : 
#line 570 "../../src/expr.c"
if (_expr_lval ( _au0_this -> _expr__O4.__C4_e2 , _au1_b ) == 0 ){ 
#line 571 "../../src/expr.c"
_au0_this -> _expr__O2.__C2_tp = (struct
#line 571 "../../src/expr.c"
type *)any_type ;
return _au0_this ;
}
_au0_this -> _expr__O2.__C2_tp = (struct type *)( (((struct ptr *)( (_au0__Xthis__ctor_ptr = 0 ), ( (_au0__Xthis__ctor_ptr = (struct ptr *)_new ( (long )(sizeof
#line 574 "../../src/expr.c"
(struct ptr))) ), ( (Nt ++ ), ( (_au0__Xthis__ctor_ptr -> _node_base = ((unsigned char )125 )), ( (_au0__Xthis__ctor_ptr -> _pvtyp_typ =
#line 574 "../../src/expr.c"
_au1_t ), ( (_au0__Xthis__ctor_ptr -> _ptr_rdo = ((char )0 )), ((_au0__Xthis__ctor_ptr ))) ) ) ) ) ) ))) ;
#line 574 "../../src/expr.c"

#line 575 "../../src/expr.c"
if ((_type_tconst ( _au1_t ) && (vec_const == 0 ))&& (fct_const == 0 ))(((struct ptr *)_au0_this -> _expr__O2.__C2_tp ))-> _ptr_rdo = 1 ;
switch (_au0_this -> _expr__O4.__C4_e2 -> _node_base ){ 
#line 577 "../../src/expr.c"
case 85 : 
#line 578 "../../src/expr.c"
mname :
#line 579 "../../src/expr.c"
{ Pname _au4_n2 ;
Pname _au4_cn ;

#line 581 "../../src/expr.c"
struct ival *_au0__Xthis__ctor_ival ;

#line 581 "../../src/expr.c"
struct ptr *_au0__Xthis__ctor_ptr ;

#line 579 "../../src/expr.c"
_au4_n2 = (((struct name *)_au0_this -> _expr__O4.__C4_e2 ));
_au4_cn = ((_au4_n2 -> _expr__O5.__C5_n_table != gtbl )? _au4_n2 -> _expr__O5.__C5_n_table -> _table_t_name : (((struct name *)0 )));
if (_au4_cn == 0 )break ;
(((struct ptr *)_au0_this -> _expr__O2.__C2_tp ))-> _ptr_memof = (((struct classdef *)_au4_cn -> _expr__O2.__C2_tp ));
if (_au1_t -> _node_base == 108 ){ 
#line 584 "../../src/expr.c"
_expr_lval ( (struct expr *)_au4_n2 , (unsigned char )112 ) ;
{ Pfct _au5_f ;

#line 586 "../../src/expr.c"
struct ival *_au0__Xthis__ctor_ival ;

#line 585 "../../src/expr.c"
_au5_f = (((struct fct *)_au1_t ));

#line 587 "../../src/expr.c"
if (_au5_f -> _fct_f_virtual )
#line 588 "../../src/expr.c"
_au0_this -> _expr__O3.__C3_e1 = (struct expr *)( (_au0__Xthis__ctor_ival = 0 ), ( ( (_au0__Xthis__ctor_ival = 0 ), (_au0__Xthis__ctor_ival = (struct
#line 588 "../../src/expr.c"
ival *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ival ), (unsigned char )150 , (struct expr *)0 , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ival ->
#line 588 "../../src/expr.c"
_expr__O3.__C3_i1 = ((int )_au5_f -> _fct_f_virtual )), ((_au0__Xthis__ctor_ival ))) ) ) ;
else 
#line 590 "../../src/expr.c"
break ;
}
}
else 
#line 592 "../../src/expr.c"
{ 
#line 593 "../../src/expr.c"
if (_au4_n2 -> _name_n_stclass != 31 )
#line 594 "../../src/expr.c"
_au0_this -> _expr__O3.__C3_e1 = (struct expr *)( (_au0__Xthis__ctor_ival = 0 ), ( ( (_au0__Xthis__ctor_ival =
#line 594 "../../src/expr.c"
0 ), (_au0__Xthis__ctor_ival = (struct ival *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ival ), (unsigned char )150 , (struct expr *)0 , (struct expr *)0 ) ))
#line 594 "../../src/expr.c"
, ( (_au0__Xthis__ctor_ival -> _expr__O3.__C3_i1 = (_au4_n2 -> _name_n_offset + 1 )), ((_au0__Xthis__ctor_ival ))) ) ) ;
else { 
#line 598 "../../src/expr.c"
_au0_this -> _expr__O2.__C2_tp = (struct type *)( (_au0__Xthis__ctor_ptr = 0 ), ( (_au0__Xthis__ctor_ptr = (struct ptr *)_new ( (long )(sizeof
#line 598 "../../src/expr.c"
(struct ptr))) ), ( (Nt ++ ), ( (_au0__Xthis__ctor_ptr -> _node_base = ((unsigned char )125 )), ( (_au0__Xthis__ctor_ptr -> _pvtyp_typ =
#line 598 "../../src/expr.c"
_au0_this -> _expr__O4.__C4_e2 -> _expr__O2.__C2_tp ), ( (_au0__Xthis__ctor_ptr -> _ptr_rdo = ((char )0 )), ((_au0__Xthis__ctor_ptr ))) ) ) ) ) )
#line 598 "../../src/expr.c"
;
return _au0_this ;
}
}
_au0_this -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp = (struct type *)int_type ;
_au0_this -> _expr__O4.__C4_e2 = 0 ;
_au0_this -> _expr__O5.__C5_tp2 = _au0_this -> _expr__O2.__C2_tp ;
_au0_this -> _node_base = 113 ;
return _au0_this ;
}
case 45 : 
#line 609 "../../src/expr.c"
case 44 : 
#line 610 "../../src/expr.c"
{ Pname _au4_m ;
Pfct _au4_f ;

#line 610 "../../src/expr.c"
_au4_m = _au0_this -> _expr__O4.__C4_e2 -> _expr__O5.__C5_mem ;
_au4_f = (((struct fct *)_au4_m -> _expr__O2.__C2_tp ));
if (_au4_f -> _node_base == 108 ){ 
#line 613 "../../src/expr.c"
if (((bound == 0 )&& (_au0_this -> _expr__O4.__C4_e2 -> _expr__O3.__C3_e1 == (struct expr *)cc -> _dcl_context_c_this ))&& _au4_m ->
#line 613 "../../src/expr.c"
_name__O6.__C6_n_qualifier ){ 
#line 615 "../../src/expr.c"
if (_au0_this -> _expr__O4.__C4_e2 && (_au0_this -> _expr__O4.__C4_e2 -> _node_permanent == 0 ))_expr_del ( _au0_this -> _expr__O4.__C4_e2 ) ;
_au0_this -> _expr__O4.__C4_e2 = (struct expr *)_au4_m ;
goto mname ;
}
{ 
#line 887 "../../src/expr.c"
struct ea _au0__V29 ;

#line 887 "../../src/expr.c"
struct ea _au0__V30 ;

#line 887 "../../src/expr.c"
struct ea _au0__V31 ;

#line 619 "../../src/expr.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"address of boundF (try %s::*PT and &%s::%s address)", (struct ea *)( ( ((& _au0__V29 )-> _ea__O1.__C1_p = ((char *)_au4_m -> _expr__O5.__C5_n_table ->
#line 619 "../../src/expr.c"
_table_t_name -> _expr__O3.__C3_string )), (((& _au0__V29 )))) ) , (struct ea *)( ( ((& _au0__V30 )-> _ea__O1.__C1_p = ((char *)_au4_m ->
#line 619 "../../src/expr.c"
_expr__O5.__C5_n_table -> _table_t_name -> _expr__O3.__C3_string )), (((& _au0__V30 )))) ) , (struct ea *)( ( ((& _au0__V31 )-> _ea__O1.__C1_p = ((char
#line 619 "../../src/expr.c"
*)_au4_m -> _expr__O3.__C3_string )), (((& _au0__V31 )))) ) , (struct ea *)ea0 ) ;
if ((_au4_f -> _fct_f_virtual == 0 )|| _au4_m -> _name__O6.__C6_n_qualifier ){ 
#line 622 "../../src/expr.c"
if (_au0_this -> _expr__O4.__C4_e2 && (_au0_this -> _expr__O4.__C4_e2 -> _node_permanent == 0 ))_expr_del ( _au0_this ->
#line 622 "../../src/expr.c"
_expr__O4.__C4_e2 ) ;
_au0_this -> _expr__O4.__C4_e2 = (struct expr *)_au4_m ;
}
} }
}
}
return _au0_this ;

#line 630 "../../src/expr.c"
case 107 : 
#line 631 "../../src/expr.c"
( _type_kind ( _au1_t , ((unsigned char )107 ), (unsigned char )'N' ) ) ;
_au0_this -> _expr__O2.__C2_tp = _au1_t ;
return _au0_this ;

#line 635 "../../src/expr.c"
case 172 : 
#line 636 "../../src/expr.c"
( _type_kind ( _au1_t , ((unsigned char )172 ), (unsigned char )'P' ) ) ;
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"unary + (ignored)", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 637 "../../src/expr.c"

#line 638 "../../src/expr.c"
_au0_this -> _expr__O2.__C2_tp = _au1_t ;
_au0_this -> _node_base = 54 ;
_au0_this -> _expr__O3.__C3_e1 = zero ;
return _au0_this ;

#line 643 "../../src/expr.c"
case 46 : 
#line 644 "../../src/expr.c"
_au0_this -> _expr__O4.__C4_e2 = check_cond ( _au0_this -> _expr__O4.__C4_e2 , (unsigned char )46 , _au0_tbl ) ;
_au0_this -> _expr__O2.__C2_tp = (struct type *)int_type ;
return _au0_this ;

#line 648 "../../src/expr.c"
case 47 : 
#line 649 "../../src/expr.c"
( _type_kind ( _au1_t , ((unsigned char )47 ), (unsigned char )'I' ) ) ;
_au0_this -> _expr__O2.__C2_tp = _au1_t ;
return _au0_this ;

#line 653 "../../src/expr.c"
case 48 : 
#line 654 "../../src/expr.c"
case 49 : 
#line 655 "../../src/expr.c"
if (_au0_this -> _expr__O3.__C3_e1 )_expr_lval ( _au0_this -> _expr__O3.__C3_e1 , _au1_b ) ;
if (_au0_this -> _expr__O4.__C4_e2 )_expr_lval ( _au0_this -> _expr__O4.__C4_e2 , _au1_b ) ;
_au1_r1 = ( _type_kind ( _au1_t , _au1_b , (unsigned char )'P' ) ) ;
_au0_this -> _expr__O2.__C2_tp = _au1_t ;
return _au0_this ;
}

#line 663 "../../src/expr.c"
if ((((_au0_this -> _expr__O3.__C3_e1 == dummy )|| (_au0_this -> _expr__O4.__C4_e2 == dummy ))|| (_au0_this -> _expr__O3.__C3_e1 == 0 ))|| (_au0_this -> _expr__O4.__C4_e2 == 0 )){ 
#line 887 "../../src/expr.c"
struct
#line 887 "../../src/expr.c"
ea _au0__V32 ;

#line 663 "../../src/expr.c"
error ( (char *)"operand missing for%k", (struct ea *)( ( ((& _au0__V32 )-> _ea__O1.__C1_i = ((int )_au1_b )), (((& _au0__V32 )))) )
#line 663 "../../src/expr.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} switch (_au1_b ){ 
#line 665 "../../src/expr.c"
case 50 : 
#line 666 "../../src/expr.c"
case 51 : 
#line 667 "../../src/expr.c"
_au1_r1 = ( _type_kind ( _au1_t1 , _au1_b , (unsigned char
#line 667 "../../src/expr.c"
)'N' ) ) ;
_au1_r2 = ( _type_kind ( _au1_t2 , _au1_b , (unsigned char )'N' ) ) ;
_au1_t = np_promote ( _au1_b , _au1_r1 , _au1_r2 , _au1_t1 , _au1_t2 , (unsigned char )1 ) ;
break ;

#line 672 "../../src/expr.c"
case 54 : 
#line 673 "../../src/expr.c"
_au1_r2 = ( _type_kind ( _au1_t2 , ((unsigned char )54 ), (unsigned char )'P' ) ) ;
#line 673 "../../src/expr.c"

#line 674 "../../src/expr.c"
_au1_r1 = ( _type_kind ( _au1_t1 , ((unsigned char )54 ), (unsigned char )'P' ) ) ;
if ((_au1_r1 == 'P' )&& (_au1_r2 == 'P' ))error ( (char *)"P +P", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 675 "../../src/expr.c"
ea *)ea0 ) ;
_au1_t = np_promote ( (unsigned char )54 , _au1_r1 , _au1_r2 , _au1_t1 , _au1_t2 , (unsigned char )1 ) ;
_au0_this -> _expr__O2.__C2_tp = _au1_t ;
break ;

#line 680 "../../src/expr.c"
case 55 : 
#line 681 "../../src/expr.c"
_au1_r2 = ( _type_kind ( _au1_t2 , ((unsigned char )55 ), (unsigned char )'P' ) ) ;
#line 681 "../../src/expr.c"

#line 682 "../../src/expr.c"
_au1_r1 = ( _type_kind ( _au1_t1 , ((unsigned char )55 ), (unsigned char )'P' ) ) ;
if (((_au1_r2 == 'P' )&& (_au1_r1 != 'P' ))&& (_au1_r1 != 'A' ))error ( (char *)"P - nonP", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 683 "../../src/expr.c"
ea *)ea0 , (struct ea *)ea0 ) ;
_au1_t = np_promote ( (unsigned char )55 , _au1_r1 , _au1_r2 , _au1_t1 , _au1_t2 , (unsigned char )1 ) ;
_au0_this -> _expr__O2.__C2_tp = _au1_t ;
break ;

#line 688 "../../src/expr.c"
case 56 : 
#line 689 "../../src/expr.c"
case 57 : 
#line 690 "../../src/expr.c"
case 52 : 
#line 691 "../../src/expr.c"
case 65 : 
#line 692 "../../src/expr.c"
case 64 : 
#line 693 "../../src/expr.c"
switch (_au0_this -> _expr__O3.__C3_e1 -> _node_base ){
#line 693 "../../src/expr.c"

#line 694 "../../src/expr.c"
case 58 : 
#line 695 "../../src/expr.c"
case 59 : 
#line 696 "../../src/expr.c"
case 60 : 
#line 697 "../../src/expr.c"
case 61 : 
#line 698 "../../src/expr.c"
case 62 : 
#line 699 "../../src/expr.c"
case 63 : 
#line 700 "../../src/expr.c"
{ 
#line 887 "../../src/expr.c"
struct
#line 887 "../../src/expr.c"
ea _au0__V33 ;

#line 887 "../../src/expr.c"
struct ea _au0__V34 ;

#line 700 "../../src/expr.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"%kE as operand for%k", (struct ea *)( ( ((& _au0__V33 )-> _ea__O1.__C1_i = ((int )_au0_this -> _expr__O3.__C3_e1 ->
#line 700 "../../src/expr.c"
_node_base )), (((& _au0__V33 )))) ) , (struct ea *)( ( ((& _au0__V34 )-> _ea__O1.__C1_i = ((int )_au1_b )), (((&
#line 700 "../../src/expr.c"
_au0__V34 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
switch (_au0_this -> _expr__O4.__C4_e2 -> _node_base ){ 
#line 703 "../../src/expr.c"
case 58 : 
#line 704 "../../src/expr.c"
case 59 : 
#line 705 "../../src/expr.c"
case 60 : 
#line 706 "../../src/expr.c"
case 61 : 
#line 707 "../../src/expr.c"
case
#line 707 "../../src/expr.c"
62 : 
#line 708 "../../src/expr.c"
case 63 : 
#line 709 "../../src/expr.c"
{ 
#line 887 "../../src/expr.c"
struct ea _au0__V35 ;

#line 887 "../../src/expr.c"
struct ea _au0__V36 ;

#line 709 "../../src/expr.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"%kE as operand for%k", (struct ea *)( ( ((& _au0__V35 )-> _ea__O1.__C1_i = ((int )_au0_this -> _expr__O4.__C4_e2 ->
#line 709 "../../src/expr.c"
_node_base )), (((& _au0__V35 )))) ) , (struct ea *)( ( ((& _au0__V36 )-> _ea__O1.__C1_i = ((int )_au1_b )), (((&
#line 709 "../../src/expr.c"
_au0__V36 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
case 53 : 
#line 712 "../../src/expr.c"
_au1_r1 = ( _type_kind ( _au1_t1 , _au1_b , (unsigned char )'I' ) ) ;
_au1_r2 = ( _type_kind ( _au1_t2 , _au1_b , (unsigned char )'I' ) ) ;
_au1_t = np_promote ( _au1_b , _au1_r1 , _au1_r2 , _au1_t1 , _au1_t2 , (unsigned char )1 ) ;
break ;

#line 717 "../../src/expr.c"
case 58 : 
#line 718 "../../src/expr.c"
case 59 : 
#line 719 "../../src/expr.c"
case 60 : 
#line 720 "../../src/expr.c"
case 61 : 
#line 721 "../../src/expr.c"
case 62 : 
#line 722 "../../src/expr.c"
case 63 : 
#line 723 "../../src/expr.c"
_au1_r1 = (
#line 723 "../../src/expr.c"
_type_kind ( _au1_t1 , _au1_b , (unsigned char )'P' ) ) ;
_au1_r2 = ( _type_kind ( _au1_t2 , _au1_b , (unsigned char )'P' ) ) ;
(np_promote ( _au1_b , _au1_r1 , _au1_r2 , _au1_t1 , _au1_t2 , (unsigned char )0 ) );
_au1_t = (struct type *)int_type ;
break ;

#line 729 "../../src/expr.c"
case 66 : 
#line 730 "../../src/expr.c"
case 67 : 
#line 731 "../../src/expr.c"
_au0_this -> _expr__O3.__C3_e1 = check_cond ( _au0_this -> _expr__O3.__C3_e1 , _au1_b , _au0_tbl ) ;
_au0_this -> _expr__O4.__C4_e2 = check_cond ( _au0_this -> _expr__O4.__C4_e2 , _au1_b , _au0_tbl ) ;
_au1_t = (struct type *)int_type ;
break ;

#line 736 "../../src/expr.c"
case 68 : 
#line 737 "../../src/expr.c"
{ 
#line 738 "../../src/expr.c"
Pname _au3_c1 ;

#line 738 "../../src/expr.c"
Pname _au3_c2 ;

#line 739 "../../src/expr.c"
struct texpr *_au0__Xthis__ctor_texpr ;
_au0_this -> _expr__O5.__C5_cond = check_cond ( _au0_this -> _expr__O5.__C5_cond , _au1_b , _au0_tbl ) ;

#line 741 "../../src/expr.c"
if ((_au1_t1 == _au1_t2 )|| (((_au3_c1 = _type_is_cl_obj ( _au1_t1 ) )&& (_au3_c2 = _type_is_cl_obj ( _au1_t2 ) ))&& (_au3_c1 -> _expr__O2.__C2_tp == _au3_c2 ->
#line 741 "../../src/expr.c"
_expr__O2.__C2_tp )))
#line 746 "../../src/expr.c"
_au1_t = _au1_t1 ;
else { 
#line 748 "../../src/expr.c"
_au1_r1 = ( _type_kind ( _au1_t1 , _au1_b , (unsigned char )'P' ) ) ;
_au1_r2 = ( _type_kind ( _au1_t2 , _au1_b , (unsigned char )'P' ) ) ;

#line 751 "../../src/expr.c"
if ((_au1_r1 == 108 )&& (_au1_r2 == 108 )){ 
#line 752 "../../src/expr.c"
if (_type_check ( _au1_t1 , _au1_t2 , (unsigned char )70 ) ){ 
#line 887 "../../src/expr.c"
struct
#line 887 "../../src/expr.c"
ea _au0__V37 ;

#line 887 "../../src/expr.c"
struct ea _au0__V38 ;

#line 752 "../../src/expr.c"
error ( (char *)"badTs in ?:E: %t and %t", (struct ea *)( ( ((& _au0__V37 )-> _ea__O1.__C1_p = ((char *)_au1_t1 )), (((& _au0__V37 )))) )
#line 752 "../../src/expr.c"
, (struct ea *)( ( ((& _au0__V38 )-> _ea__O1.__C1_p = ((char *)_au1_t2 )), (((& _au0__V38 )))) ) , (struct
#line 752 "../../src/expr.c"
ea *)ea0 , (struct ea *)ea0 ) ;
} _au1_t = _au1_t1 ;
}
else 
#line 756 "../../src/expr.c"
_au1_t = np_promote ( _au1_b , _au1_r1 , _au1_r2 , _au1_t1 , _au1_t2 , (unsigned char )1 ) ;

#line 758 "../../src/expr.c"
if ((_au1_t != _au1_t1 )&& _type_check ( _au1_t , _au1_t1 , (unsigned char )0 ) ){ 
#line 759 "../../src/expr.c"
_au0_this -> _expr__O3.__C3_e1 = (struct expr *)(
#line 759 "../../src/expr.c"
(_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr = (struct texpr *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_texpr ), ((unsigned char )113 ),
#line 759 "../../src/expr.c"
_au0_this -> _expr__O3.__C3_e1 , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 = _au1_t ), ((_au0__Xthis__ctor_texpr ))) ) ) ;
#line 759 "../../src/expr.c"

#line 760 "../../src/expr.c"
_au0_this -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp = _au1_t ;
}
if ((_au1_t != _au1_t2 )&& _type_check ( _au1_t , _au1_t2 , (unsigned char )0 ) ){ 
#line 763 "../../src/expr.c"
_au0_this -> _expr__O4.__C4_e2 = (struct expr *)(
#line 763 "../../src/expr.c"
(_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr = (struct texpr *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_texpr ), ((unsigned char )113 ),
#line 763 "../../src/expr.c"
_au0_this -> _expr__O4.__C4_e2 , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 = _au1_t ), ((_au0__Xthis__ctor_texpr ))) ) ) ;
#line 763 "../../src/expr.c"

#line 764 "../../src/expr.c"
_au0_this -> _expr__O4.__C4_e2 -> _expr__O2.__C2_tp = _au1_t ;
}
}
}

#line 769 "../../src/expr.c"
break ;

#line 771 "../../src/expr.c"
case 126 : 
#line 772 "../../src/expr.c"
_au1_r1 = ( _type_kind ( _au1_t1 , ((unsigned char )126 ), (unsigned char )'P' ) ) ;
#line 772 "../../src/expr.c"

#line 773 "../../src/expr.c"
_au1_r2 = ( _type_kind ( _au1_t2 , ((unsigned char )126 ), (unsigned char )'P' ) ) ;
if ((_au1_r1 == 'P' )&& (_au1_r2 == 'P' ))error ( (char *)"P +=P", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 774 "../../src/expr.c"
ea *)ea0 ) ;
_au1_t = np_promote ( (unsigned char )126 , _au1_r1 , _au1_r2 , _au1_t1 , _au1_t2 , (unsigned char )1 ) ;
goto ass ;

#line 778 "../../src/expr.c"
case 127 : 
#line 779 "../../src/expr.c"
_au1_r1 = ( _type_kind ( _au1_t1 , ((unsigned char )127 ), (unsigned char )'P' ) ) ;
#line 779 "../../src/expr.c"

#line 780 "../../src/expr.c"
_au1_r2 = ( _type_kind ( _au1_t2 , ((unsigned char )127 ), (unsigned char )'P' ) ) ;
if (((_au1_r2 == 'P' )&& (_au1_r1 != 'P' ))&& (_au1_r1 != 'A' ))error ( (char *)"P -= nonP", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 781 "../../src/expr.c"
ea *)ea0 , (struct ea *)ea0 ) ;
_au1_t = np_promote ( (unsigned char )127 , _au1_r1 , _au1_r2 , _au1_t1 , _au1_t2 , (unsigned char )1 ) ;
goto ass ;

#line 785 "../../src/expr.c"
case 128 : 
#line 786 "../../src/expr.c"
case 129 : 
#line 787 "../../src/expr.c"
_au1_r1 = ( _type_kind ( _au1_t1 , _au1_b , (unsigned char )'N' ) ) ;
#line 787 "../../src/expr.c"

#line 788 "../../src/expr.c"
_au1_r2 = ( _type_kind ( _au1_t1 , _au1_b , (unsigned char )'N' ) ) ;
_au1_t = np_promote ( _au1_b , _au1_r1 , _au1_r2 , _au1_t1 , _au1_t2 , (unsigned char )1 ) ;
goto ass ;

#line 792 "../../src/expr.c"
case 130 : 
#line 793 "../../src/expr.c"
_au1_r1 = ( _type_kind ( _au1_t1 , ((unsigned char )130 ), (unsigned char )'I' ) ) ;
#line 793 "../../src/expr.c"

#line 794 "../../src/expr.c"
_au1_r2 = ( _type_kind ( _au1_t2 , ((unsigned char )130 ), (unsigned char )'I' ) ) ;
_au1_t = np_promote ( (unsigned char )130 , _au1_r1 , _au1_r2 , _au1_t1 , _au1_t2 , (unsigned char )1 ) ;
goto ass ;

#line 798 "../../src/expr.c"
case 131 : 
#line 799 "../../src/expr.c"
case 132 : 
#line 800 "../../src/expr.c"
case 133 : 
#line 801 "../../src/expr.c"
case 134 : 
#line 802 "../../src/expr.c"
case 135 : 
#line 803 "../../src/expr.c"
_au1_r1 = ( _type_kind ( _au1_t1 ,
#line 803 "../../src/expr.c"
_au1_b , (unsigned char )'I' ) ) ;
_au1_r2 = ( _type_kind ( _au1_t2 , _au1_b , (unsigned char )'I' ) ) ;
(np_promote ( _au1_b , _au1_r1 , _au1_r2 , _au1_t1 , _au1_t2 , (unsigned char )0 ) );
_au1_t = (struct type *)int_type ;
goto ass ;
ass :
#line 809 "../../src/expr.c"
_au0_this -> _expr__O5.__C5_as_type = _au1_t ;
_au1_t2 = _au1_t ;

#line 812 "../../src/expr.c"
case 70 : 
#line 813 "../../src/expr.c"
if (_expr_lval ( _au0_this -> _expr__O3.__C3_e1 , _au1_b ) == 0 ){ 
#line 814 "../../src/expr.c"
_au0_this -> _expr__O2.__C2_tp = (struct type *)any_type ;
return _au0_this ;
}
lkj :
#line 818 "../../src/expr.c"
switch (_au1_t1 -> _node_base ){ 
#line 819 "../../src/expr.c"
case 97 : 
#line 820 "../../src/expr.c"
_au1_t1 = (((struct basetype *)_au1_t1 ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
goto lkj ;
case 21 : 
#line 823 "../../src/expr.c"
case 5 : 
#line 824 "../../src/expr.c"
case 29 : 
#line 825 "../../src/expr.c"
if ((_au0_this -> _expr__O4.__C4_e2 -> _node_base == 82 )&& (_au0_this -> _expr__O4.__C4_e2 -> _expr__O2.__C2_tp ==
#line 825 "../../src/expr.c"
(struct type *)long_type ))
#line 826 "../../src/expr.c"
{ 
#line 887 "../../src/expr.c"
struct ea _au0__V39 ;

#line 826 "../../src/expr.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"long constant assigned to%k", (struct ea *)( ( ((& _au0__V39 )-> _ea__O1.__C1_i = ((int )_au1_t1 -> _node_base )),
#line 826 "../../src/expr.c"
(((& _au0__V39 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} case 22 : 
#line 828 "../../src/expr.c"
if ((((_au1_b == 70 )&& (((struct basetype *)_au1_t1 ))-> _basetype_b_unsigned )&& (_au0_this -> _expr__O4.__C4_e2 -> _node_base == 107 ))&& (_au0_this ->
#line 828 "../../src/expr.c"
_expr__O4.__C4_e2 -> _expr__O4.__C4_e2 -> _node_base == 82 ))
#line 832 "../../src/expr.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"negative assigned to unsigned", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ,
#line 832 "../../src/expr.c"
(struct ea *)ea0 ) ;
break ;
case 125 : 
#line 835 "../../src/expr.c"
if (_au1_b == 70 ){ 
#line 836 "../../src/expr.c"
_au0_this -> _expr__O4.__C4_e2 = ptr_init ( ((struct ptr *)_au1_t1 ), _au0_this -> _expr__O4.__C4_e2 , _au0_tbl ) ;
#line 836 "../../src/expr.c"

#line 837 "../../src/expr.c"
_au1_t2 = _au0_this -> _expr__O4.__C4_e2 -> _expr__O2.__C2_tp ;
}
break ;
case 119 : 
#line 841 "../../src/expr.c"
{ Pname _au4_c1 ;

#line 841 "../../src/expr.c"
_au4_c1 = _type_is_cl_obj ( _au1_t1 ) ;

#line 843 "../../src/expr.c"
if (_au4_c1 ){ 
#line 844 "../../src/expr.c"
Pname _au5_c2 ;

#line 845 "../../src/expr.c"
struct texpr *_au0__Xthis__ctor_texpr ;

#line 844 "../../src/expr.c"
_au5_c2 = _type_is_cl_obj ( _au1_t2 ) ;

#line 846 "../../src/expr.c"
if (_au4_c1 != _au5_c2 ){ 
#line 847 "../../src/expr.c"
_au0_this -> _expr__O4.__C4_e2 = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , _au0_this -> _expr__O4.__C4_e2 ,
#line 847 "../../src/expr.c"
(struct expr *)0 ) ;
_au0_this -> _expr__O4.__C4_e2 = (struct expr *)( (_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr = (struct texpr *)_expr__ctor ( ((struct
#line 848 "../../src/expr.c"
expr *)_au0__Xthis__ctor_texpr ), ((unsigned char )157 ), _au0_this -> _expr__O4.__C4_e2 , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 = _au1_t1 ),
#line 848 "../../src/expr.c"
((_au0__Xthis__ctor_texpr ))) ) ) ;
_au0_this -> _expr__O4.__C4_e2 -> _expr__O4.__C4_e2 = _au0_this -> _expr__O3.__C3_e1 ;
_au0_this -> _expr__O4.__C4_e2 = _expr_typ ( _au0_this -> _expr__O4.__C4_e2 , _au0_tbl ) ;

#line 852 "../../src/expr.c"
_au0_this -> _expr__O2.__C2_tp = _au1_t1 ;
return _au0_this ;
}
else { 
#line 856 "../../src/expr.c"
Pclass _au6_cl ;

#line 856 "../../src/expr.c"
_au6_cl = (((struct classdef *)_au4_c1 -> _expr__O2.__C2_tp ));

#line 858 "../../src/expr.c"
if (_au6_cl -> _classdef_bit_ass == 0 )
#line 859 "../../src/expr.c"
{ 
#line 887 "../../src/expr.c"
struct ea _au0__V40 ;

#line 859 "../../src/expr.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"bitwise copy: %s has aMW operator=()", (struct ea *)( ( ((& _au0__V40 )-> _ea__O1.__C1_p = ((char *)_au6_cl -> _classdef_string )),
#line 859 "../../src/expr.c"
(((& _au0__V40 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} else if (_au6_cl -> _classdef_itor && ( _table_look ( _au6_cl -> _classdef_memtbl , "_dtor", (unsigned char )0 ) )
#line 860 "../../src/expr.c"
)
#line 861 "../../src/expr.c"
{ 
#line 887 "../../src/expr.c"
struct ea _au0__V41 ;

#line 887 "../../src/expr.c"
struct ea _au0__V42 ;

#line 887 "../../src/expr.c"
struct ea _au0__V43 ;

#line 861 "../../src/expr.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"bitwise copy: %s has destructor and %s(%s&) but not assignment", (struct ea *)( ( ((& _au0__V41 )-> _ea__O1.__C1_p = ((char *)_au6_cl -> _classdef_string )),
#line 861 "../../src/expr.c"
(((& _au0__V41 )))) ) , (struct ea *)( ( ((& _au0__V42 )-> _ea__O1.__C1_p = ((char *)_au6_cl -> _classdef_string )), (((&
#line 861 "../../src/expr.c"
_au0__V42 )))) ) , (struct ea *)( ( ((& _au0__V43 )-> _ea__O1.__C1_p = ((char *)_au6_cl -> _classdef_string )), (((& _au0__V43 ))))
#line 861 "../../src/expr.c"
) , (struct ea *)ea0 ) ;
} }
}
break ;
}
}

#line 869 "../../src/expr.c"
{ Pexpr _au3_x ;

#line 869 "../../src/expr.c"
_au3_x = try_to_coerce ( _au1_t1 , _au0_this -> _expr__O4.__C4_e2 , "assignment", _au0_tbl ) ;
if (_au3_x )
#line 871 "../../src/expr.c"
_au0_this -> _expr__O4.__C4_e2 = _au3_x ;
else if (_type_check ( _au0_this -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp , _au1_t2 , (unsigned char )70 ) )
#line 873 "../../src/expr.c"
{ 
#line 887 "../../src/expr.c"
struct ea _au0__V44 ;

#line 887 "../../src/expr.c"
struct ea _au0__V45 ;

#line 873 "../../src/expr.c"
error ( (char *)"bad assignmentT:%t =%t", (struct ea *)( ( ((& _au0__V44 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp )), (((&
#line 873 "../../src/expr.c"
_au0__V44 )))) ) , (struct ea *)( ( ((& _au0__V45 )-> _ea__O1.__C1_p = ((char *)_au1_t2 )), (((& _au0__V45 )))) )
#line 873 "../../src/expr.c"
, (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
_au1_t = _au0_this -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp ;
break ;
case 71 : 
#line 878 "../../src/expr.c"
case 147 : 
#line 879 "../../src/expr.c"
_au1_t = _au1_t2 ;
break ;
default : 
#line 882 "../../src/expr.c"
{ 
#line 887 "../../src/expr.c"
struct ea _au0__V46 ;

#line 882 "../../src/expr.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"unknown operator%k", (struct ea *)( ( ((& _au0__V46 )-> _ea__O1.__C1_i = ((int )_au1_b )), (((&
#line 882 "../../src/expr.c"
_au0__V46 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }

#line 885 "../../src/expr.c"
_au0_this -> _expr__O2.__C2_tp = _au1_t ;
return _au0_this ;
}
;

/* the end */
