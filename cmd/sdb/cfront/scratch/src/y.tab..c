/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#line 1 "../../src/y.tab.c"

/* <<cfront 1.2.1 2/16/87>> */
/* < ../../src/y.tab.c */

#ident	"@(#)sdb:cfront/scratch/src/y.tab..c	1.2"
char *_new(); char _delete(); char *_vec_new(); char _vec_delete();

#line 1 "gram.y"

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

#line 31 "gram.y"
static int cdi = 0 ;
static Pnlist cd = 0 ;

#line 32 "gram.y"
static Pnlist cd_vec [50];
static char stmt_seen = 0 ;

#line 33 "gram.y"
static char stmt_vec [50];
static Plist tn_vec [50];

#line 36 "gram.y"
char sig_name ();
Ptype tok_to_type ();
char memptrdcl ();

#line 124 "gram.y"
union _C10 {	/* sizeof _C10 == 4 */

#line 125 "gram.y"
char *__C10_s ;
TOK __C10_t ;
int __C10_i ;
struct loc __C10_l ;
Pname __C10_pn ;
Ptype __C10_pt ;
Pexpr __C10_pe ;
Pstmt __C10_ps ;
Pbase __C10_pb ;
Pnlist __C10_nl ;
Pslist __C10_sl ;
Pelist __C10_el ;
PP __C10_p ;
};
typedef union _C10 YYSTYPE ;

#line 140 "gram.y"
extern union _C10 yylval ;

#line 140 "gram.y"
extern union _C10 yyval ;
extern int yyparse ();

#line 143 "gram.y"
extern Pname syn ()
#line 144 "gram.y"
{ 
#line 145 "gram.y"
ll :
#line 146 "gram.y"
switch (yyparse ( ) ){ 
#line 147 "gram.y"
case 0 : return (struct name *)0 ;
case 1 : goto ll ;
default : return yyval . __C10_pn ;
}
}
;
char look_for_hidden (_au0_n , _au0_nn )Pname _au0_n ;

#line 153 "gram.y"
Pname _au0_nn ;
{ 
#line 155 "gram.y"
Pname _au1_nx ;

#line 155 "gram.y"
_au1_nx = _table_look ( ktbl , _au0_n -> _expr__O3.__C3_string , (unsigned char )159 ) ;
if (_au1_nx == 0 ){ 
#line 158 "gram.y"
struct ea _au0__V11 ;

#line 158 "gram.y"
struct ea _au0__V12 ;

#line 156 "gram.y"
error ( (char *)"nonTN%n before ::%n", (struct ea *)( ( ((& _au0__V11 )-> _ea__O1.__C1_p = ((char *)_au0_n )), (((& _au0__V11 )))) )
#line 156 "gram.y"
, (struct ea *)( ( ((& _au0__V12 )-> _ea__O1.__C1_p = ((char *)_au0_nn )), (((& _au0__V12 )))) ) , (struct
#line 156 "gram.y"
ea *)ea0 , (struct ea *)ea0 ) ;
} _au0_nn -> _name__O6.__C6_n_qualifier = _au1_nx ;
}
;

#line 229 "gram.y"
extern int yychar ;
extern int yyerrflag ;

#line 234 "gram.y"
union _C10 yylval ;

#line 234 "gram.y"
union _C10 yyval ;
typedef int yytabelem ;

#line 1239 "gram.y"
yytabelem yyexca [144]= { - 1 , 1 , 0 , - 1 , - 2 , 0 , - 1 , 31 ,
#line 1239 "gram.y"
71 , 21 , 72 , 21 , - 2 , 20 , - 1 , 45 , 155 , 236 , - 2 ,
#line 1239 "gram.y"
181 , - 1 , 52 , 155 , 236 , - 2 , 181 , - 1 , 229 , 1 , 129 ,
#line 1239 "gram.y"
3 , 129 , 4 , 129 , 7 , 129 , 8 , 129 , 9 , 129 , 10 , 129 , 13 , 129 ,
#line 1239 "gram.y"
16 , 129 , 19 , 129 , 20 , 129 , 23 , 129 , 24 , 129 , 28 , 129 , 30 , 129 ,
#line 1239 "gram.y"
33 , 129 , 34 , 129 , 39 , 129 , 40 , 129 , 46 , 129 , 47 , 129 , 50 , 129 ,
#line 1239 "gram.y"
52 , 129 , 54 , 129 , 55 , 129 , 64 , 129 , 65 , 129 , 66 , 129 , 67 , 129 ,
#line 1239 "gram.y"
68 , 129 , 70 , 129 , 71 , 129 , 72 , 129 , 73 , 129 , 113 , 129 , 80 , 129 ,
#line 1239 "gram.y"
81 , 129 , 82 , 129 , 83 , 129 , 84 , 129 , 86 , 129 , 90 , 129 , 91 , 129 ,
#line 1239 "gram.y"
92 , 129 , 93 , 129 , 94 , 129 , 95 , 129 , 97 , 129 , 123 , 129 , 156 , 129 ,
#line 1239 "gram.y"
160 , 129 , - 2 , 0 , - 1 , 256 , 72 , 52 , - 2 , 102 , -
#line 1239 "gram.y"
1 , 257 , 72 , 51 , - 2 , 63 } ;

#line 1315 "gram.y"
yytabelem yyact [1539]= { 133 , 385 , 46 , 44 , 140 , 17 , 170 , 231 , 144 , 63 , 64 , 108 ,
#line 1315 "gram.y"
299 , 350 , 296 , 240 , 53 , 51 , 308 , 143 , 210 , 207 , 20 , 21 , 122 , 301 ,
#line 1315 "gram.y"
262 , 390 , 355 , 95 , 169 , 10 , 243 , 36 , 37 , 24 , 205 , 25 , 53 , 51 ,
#line 1315 "gram.y"
206 , 106 , 24 , 243 , 25 , 24 , 223 , 25 , 149 , 14 , 132 , 159 , 96 , 24 ,
#line 1315 "gram.y"
328 , 25 , 62 , 150 , 23 , 99 , 346 , 28 , 53 , 51 , 53 , 51 , 57 , 98 ,
#line 1315 "gram.y"
298 , 184 , 28 , 28 , 241 , 39 , 96 , 138 , 135 , 146 , 263 , 92 , 91 , 22 ,
#line 1315 "gram.y"
401 , 241 , 146 , 100 , 394 , 52 , 208 , 47 , 171 , 369 , 49 , 48 , 160 , 97 ,
#line 1315 "gram.y"
94 , 23 , 243 , 259 , 306 , 174 , 326 , 96 , 152 , 173 , 180 , 235 , 24 , 377 ,
#line 1315 "gram.y"
25 , 177 , 190 , 259 , 257 , 319 , 258 , 97 , 223 , 197 , 22 , 419 , 16 , 131 ,
#line 1315 "gram.y"
222 , 212 , 213 , 214 , 215 , 216 , 217 , 218 , 219 , 138 , 327 , 28 , 28 , 237 ,
#line 1315 "gram.y"
241 , 227 , 28 , 224 , 225 , 209 , 211 , 19 , 97 , 229 , 7 , 289 , 172 , 221 ,
#line 1315 "gram.y"
242 , 153 , 19 , 34 , 206 , 256 , 26 , 248 , 247 , 36 , 37 , 301 , 228 , 26 ,
#line 1315 "gram.y"
160 , 383 , 26 , 36 , 37 , 139 , 24 , 24 , 25 , 25 , 26 , 178 , 176 , 130 ,
#line 1315 "gram.y"
139 , 239 , 266 , 267 , 268 , 269 , 270 , 271 , 272 , 273 , 274 , 275 , 276 , 277 ,
#line 1315 "gram.y"
278 , 279 , 280 , 281 , 255 , 282 , 309 , 283 , 323 , 261 , 29 , 265 , 284 , 50 ,
#line 1315 "gram.y"
264 , 291 , 292 , 232 , 250 , 29 , 29 , 297 , 168 , 300 , 57 , 34 , 34 , 236 ,
#line 1315 "gram.y"
290 , 253 , 19 , 252 , 8 , 285 , 288 , 305 , 295 , 26 , 36 , 37 , 36 , 37 ,
#line 1315 "gram.y"
293 , 311 , 131 , 226 , 209 , 302 , 165 , 317 , 23 , 239 , 239 , 316 , 242 , 242 ,
#line 1315 "gram.y"
139 , 233 , 211 , 304 , 310 , 28 , 238 , 315 , 320 , 321 , 18 , 325 , 260 , 23 ,
#line 1315 "gram.y"
286 , 294 , 324 , 22 , 54 , 146 , 24 , 155 , 25 , 103 , 104 , 354 , 6 , 392 ,
#line 1315 "gram.y"
29 , 29 , 254 , 146 , 314 , 29 , 411 , 96 , 22 , 353 , 5 , 40 , 154 , 249 ,
#line 1315 "gram.y"
105 , 47 , 130 , 26 , 26 , 48 , 333 , 340 , 38 , 334 , 343 , 297 , 337 , 338 ,
#line 1315 "gram.y"
300 , 300 , 363 , 54 , 342 , 137 , 341 , 344 , 345 , 347 , 348 , 372 , 370 , 56 ,
#line 1315 "gram.y"
166 , 96 , 396 , 317 , 317 , 158 , 376 , 374 , 97 , 184 , 378 , 136 , 373 , 182 ,
#line 1315 "gram.y"
183 , 371 , 201 , 287 , 200 , 19 , 202 , 203 , 379 , 61 , 340 , 339 , 336 , 343 ,
#line 1315 "gram.y"
343 , 47 , 363 , 184 , 335 , 48 , 332 , 182 , 183 , 11 , 342 , 387 , 313 , 389 ,
#line 1315 "gram.y"
55 , 381 , 97 , 139 , 245 , 393 , 31 , 164 , 191 , 189 , 190 , 188 , 234 , 58 ,
#line 1315 "gram.y"
60 , 318 , 398 , 397 , 175 , 163 , 157 , 400 , 399 , 386 , 363 , 403 , 363 , 136 ,
#line 1315 "gram.y"
363 , 199 , 408 , 26 , 190 , 188 , 363 , 388 , 29 , 402 , 184 , 404 , 384 , 406 ,
#line 1315 "gram.y"
182 , 183 , 363 , 249 , 363 , 410 , 363 , 380 , 47 , 363 , 107 , 421 , 48 , 363 ,
#line 1315 "gram.y"
28 , 414 , 423 , 415 , 418 , 417 , 312 , 425 , 420 , 363 , 111 , 131 , 422 , 28 ,
#line 1315 "gram.y"
184 , 204 , 185 , 120 , 182 , 183 , 30 , 129 , 427 , 191 , 23 , 190 , 188 , 123 ,
#line 1315 "gram.y"
187 , 186 , 192 , 193 , 196 , 117 , 118 , 412 , 409 , 113 , 391 , 114 , 18 , 116 ,
#line 1315 "gram.y"
115 , 179 , 184 , 407 , 405 , 22 , 182 , 183 , 24 , 395 , 25 , 43 , 307 , 191 ,
#line 1315 "gram.y"
189 , 190 , 188 , 47 , 134 , 375 , 47 , 48 , 141 , 145 , 48 , 130 , 127 , 125 ,
#line 1315 "gram.y"
126 , 128 , 88 , 124 , 161 , 142 , 351 , 45 , 179 , 47 , 15 , 349 , 119 , 48 ,
#line 1315 "gram.y"
147 , 101 , 352 , 190 , 364 , 361 , 32 , 246 , 365 , 362 , 107 , 368 , 162 , 19 ,
#line 1315 "gram.y"
28 , 93 , 112 , 358 , 27 , 12 , 367 , 356 , 1 , 102 , 111 , 131 , 148 , 230 ,
#line 1315 "gram.y"
2 , 366 , 45 , 120 , 47 , 0 , 359 , 129 , 48 , 0 , 0 , 13 , 357 , 123 ,
#line 1315 "gram.y"
0 , 45 , 322 , 47 , 0 , 117 , 118 , 48 , 0 , 113 , 45 , 114 , 47 , 116 ,
#line 1315 "gram.y"
115 , 0 , 48 , 29 , 41 , 0 , 42 , 121 , 45 , 156 , 47 , 0 , 0 , 0 ,
#line 1315 "gram.y"
48 , 0 , 29 , 0 , 229 , 382 , 151 , 0 , 0 , 0 , 0 , 360 , 127 , 125 ,
#line 1315 "gram.y"
126 , 128 , 0 , 124 , 0 , 26 , 0 , 167 , 0 , 0 , 0 , 0 , 119 , 0 ,
#line 1315 "gram.y"
147 , 0 , 352 , 0 , 364 , 361 , 0 , 0 , 365 , 362 , 107 , 368 , 0 , 0 ,
#line 1315 "gram.y"
28 , 0 , 112 , 358 , 0 , 0 , 367 , 356 , 0 , 0 , 111 , 131 , 148 , 0 ,
#line 1315 "gram.y"
0 , 366 , 0 , 120 , 0 , 0 , 359 , 129 , 0 , 0 , 0 , 0 , 357 , 123 ,
#line 1315 "gram.y"
0 , 0 , 23 , 23 , 0 , 117 , 118 , 0 , 0 , 113 , 0 , 114 , 0 , 116 ,
#line 1315 "gram.y"
115 , 0 , 0 , 29 , 18 , 18 , 0 , 121 , 0 , 4 , 9 , 22 , 22 , 0 ,
#line 1315 "gram.y"
24 , 24 , 25 , 25 , 229 , 0 , 0 , 0 , 28 , 28 , 0 , 360 , 127 , 125 ,
#line 1315 "gram.y"
126 , 128 , 0 , 124 , 0 , 23 , 23 , 0 , 0 , 0 , 0 , 0 , 119 , 0 ,
#line 1315 "gram.y"
147 , 0 , 15 , 15 , 0 , 0 , 0 , 18 , 18 , 0 , 0 , 0 , 0 , 0 ,
#line 1315 "gram.y"
22 , 22 , 112 , 24 , 24 , 25 , 25 , 0 , 0 , 0 , 0 , 0 , 148 , 0 ,
#line 1315 "gram.y"
0 , 107 , 0 , 0 , 0 , 28 , 0 , 0 , 0 , 40 , 0 , 3 , 33 , 0 ,
#line 1315 "gram.y"
0 , 111 , 131 , 61 , 59 , 15 , 38 , 0 , 120 , 0 , 0 , 0 , 129 , 0 ,
#line 1315 "gram.y"
0 , 0 , 0 , 29 , 123 , 0 , 0 , 121 , 19 , 34 , 117 , 118 , 0 , 0 ,
#line 1315 "gram.y"
113 , 0 , 114 , 0 , 116 , 115 , 0 , 0 , 184 , 0 , 185 , 0 , 182 , 183 ,
#line 1315 "gram.y"
0 , 0 , 0 , 0 , 0 , 0 , 13 , 35 , 187 , 134 , 0 , 26 , 26 , 0 ,
#line 1315 "gram.y"
0 , 0 , 130 , 127 , 125 , 126 , 128 , 0 , 124 , 0 , 0 , 0 , 0 , 107 ,
#line 1315 "gram.y"
0 , 0 , 0 , 119 , 0 , 147 , 0 , 191 , 189 , 190 , 188 , 29 , 29 , 111 ,
#line 1315 "gram.y"
131 , 0 , 0 , 0 , 0 , 0 , 120 , 112 , 0 , 0 , 129 , 0 , 0 , 0 ,
#line 1315 "gram.y"
26 , 26 , 123 , 148 , 0 , 0 , 0 , 0 , 117 , 118 , 0 , 0 , 113 , 0 ,
#line 1315 "gram.y"
114 , 0 , 116 , 115 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 184 , 0 ,
#line 1315 "gram.y"
185 , 0 , 182 , 183 , 0 , 0 , 0 , 134 , 29 , 0 , 0 , 0 , 121 , 107 ,
#line 1315 "gram.y"
130 , 127 , 125 , 126 , 128 , 0 , 124 , 0 , 0 , 0 , 0 , 0 , 0 , 111 ,
#line 1315 "gram.y"
131 , 119 , 0 , 109 , 0 , 0 , 120 , 0 , 0 , 0 , 129 , 191 , 189 , 190 ,
#line 1315 "gram.y"
188 , 0 , 123 , 0 , 0 , 112 , 0 , 0 , 117 , 118 , 0 , 0 , 113 , 0 ,
#line 1315 "gram.y"
114 , 110 , 116 , 115 , 0 , 0 , 0 , 184 , 0 , 185 , 0 , 182 , 183 , 0 ,
#line 1315 "gram.y"
0 , 0 , 0 , 0 , 0 , 0 , 0 , 187 , 186 , 192 , 193 , 196 , 0 , 181 ,
#line 1315 "gram.y"
130 , 127 , 125 , 126 , 128 , 0 , 124 , 111 , 131 , 0 , 121 , 0 , 0 , 0 ,
#line 1315 "gram.y"
120 , 119 , 0 , 109 , 129 , 194 , 191 , 189 , 190 , 188 , 123 , 0 , 198 , 0 ,
#line 1315 "gram.y"
0 , 0 , 117 , 118 , 0 , 112 , 113 , 0 , 114 , 0 , 116 , 115 , 0 , 0 ,
#line 1315 "gram.y"
184 , 110 , 185 , 0 , 182 , 183 , 184 , 0 , 185 , 0 , 182 , 183 , 0 , 0 ,
#line 1315 "gram.y"
187 , 186 , 192 , 0 , 0 , 0 , 187 , 186 , 130 , 127 , 125 , 126 , 128 , 0 ,
#line 1315 "gram.y"
124 , 111 , 131 , 0 , 0 , 0 , 0 , 0 , 120 , 119 , 121 , 109 , 129 , 191 ,
#line 1315 "gram.y"
189 , 190 , 188 , 0 , 123 , 191 , 189 , 190 , 188 , 0 , 117 , 118 , 0 , 112 ,
#line 1315 "gram.y"
113 , 0 , 114 , 0 , 116 , 115 , 0 , 0 , 0 , 110 , 0 , 0 , 0 , 0 ,
#line 1315 "gram.y"
0 , 0 , 0 , 0 , 111 , 131 , 0 , 0 , 0 , 0 , 0 , 120 , 0 , 0 ,
#line 1315 "gram.y"
0 , 129 , 130 , 127 , 125 , 126 , 128 , 123 , 124 , 111 , 131 , 0 , 0 , 117 ,
#line 1315 "gram.y"
118 , 0 , 120 , 119 , 121 , 109 , 129 , 0 , 0 , 0 , 0 , 0 , 123 , 0 ,
#line 1315 "gram.y"
0 , 0 , 0 , 0 , 117 , 118 , 0 , 112 , 113 , 0 , 114 , 0 , 116 , 115 ,
#line 1315 "gram.y"
0 , 0 , 0 , 110 , 0 , 130 , 127 , 125 , 126 , 128 , 0 , 124 , 0 , 0 ,
#line 1315 "gram.y"
0 , 0 , 0 , 0 , 0 , 0 , 119 , 0 , 109 , 0 , 130 , 127 , 125 , 126 ,
#line 1315 "gram.y"
128 , 0 , 124 , 0 , 0 , 85 , 0 , 0 , 0 , 0 , 112 , 119 , 121 , 109 ,
#line 1315 "gram.y"
0 , 0 , 0 , 0 , 0 , 84 , 110 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
#line 1315 "gram.y"
0 , 220 , 0 , 0 , 0 , 0 , 0 , 0 , 77 , 0 , 78 , 110 , 86 , 87 ,
#line 1315 "gram.y"
79 , 80 , 0 , 426 , 67 , 0 , 68 , 0 , 65 , 66 , 0 , 0 , 184 , 0 ,
#line 1315 "gram.y"
185 , 121 , 182 , 183 , 70 , 69 , 75 , 76 , 0 , 0 , 83 , 0 , 187 , 186 ,
#line 1315 "gram.y"
192 , 193 , 196 , 0 , 181 , 195 , 121 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
#line 1315 "gram.y"
0 , 0 , 82 , 74 , 72 , 73 , 71 , 81 , 0 , 89 , 194 , 191 , 189 , 190 ,
#line 1315 "gram.y"
188 , 0 , 184 , 0 , 185 , 0 , 182 , 183 , 0 , 0 , 0 , 0 , 0 , 0 ,
#line 1315 "gram.y"
0 , 0 , 187 , 186 , 192 , 193 , 196 , 90 , 181 , 195 , 424 , 0 , 0 , 0 ,
#line 1315 "gram.y"
0 , 0 , 0 , 0 , 0 , 184 , 0 , 185 , 0 , 182 , 183 , 0 , 0 , 0 ,
#line 1315 "gram.y"
194 , 191 , 189 , 190 , 188 , 187 , 186 , 192 , 193 , 196 , 416 , 181 , 195 , 413 ,
#line 1315 "gram.y"
0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 184 , 0 , 185 , 0 , 182 , 183 ,
#line 1315 "gram.y"
0 , 0 , 0 , 194 , 191 , 189 , 190 , 188 , 187 , 186 , 192 , 193 , 196 , 0 ,
#line 1315 "gram.y"
181 , 195 , 0 , 0 , 331 , 0 , 0 , 0 , 0 , 0 , 0 , 184 , 0 , 185 ,
#line 1315 "gram.y"
0 , 182 , 183 , 0 , 0 , 0 , 194 , 191 , 189 , 190 , 188 , 187 , 186 , 192 ,
#line 1315 "gram.y"
193 , 196 , 0 , 181 , 195 , 0 , 0 , 330 , 0 , 0 , 0 , 0 , 0 , 0 ,
#line 1315 "gram.y"
184 , 0 , 185 , 0 , 182 , 183 , 0 , 0 , 0 , 194 , 191 , 189 , 190 , 188 ,
#line 1315 "gram.y"
187 , 186 , 192 , 193 , 196 , 0 , 181 , 195 , 0 , 0 , 0 , 0 , 0 , 0 ,
#line 1315 "gram.y"
0 , 0 , 0 , 184 , 0 , 185 , 0 , 182 , 183 , 0 , 0 , 0 , 194 , 191 ,
#line 1315 "gram.y"
189 , 190 , 188 , 187 , 186 , 192 , 193 , 196 , 329 , 181 , 195 , 303 , 0 , 0 ,
#line 1315 "gram.y"
0 , 0 , 0 , 0 , 0 , 0 , 184 , 0 , 185 , 0 , 182 , 183 , 0 , 0 ,
#line 1315 "gram.y"
0 , 194 , 191 , 189 , 190 , 188 , 187 , 186 , 192 , 193 , 196 , 0 , 181 , 195 ,
#line 1315 "gram.y"
0 , 0 , 244 , 0 , 0 , 0 , 0 , 0 , 0 , 184 , 0 , 185 , 0 , 182 ,
#line 1315 "gram.y"
183 , 0 , 0 , 0 , 194 , 191 , 189 , 190 , 188 , 187 , 186 , 192 , 193 , 196 ,
#line 1315 "gram.y"
0 , 181 , 195 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 184 , 0 ,
#line 1315 "gram.y"
185 , 0 , 182 , 183 , 0 , 0 , 0 , 194 , 191 , 189 , 190 , 188 , 187 , 186 ,
#line 1315 "gram.y"
192 , 193 , 196 , 0 , 181 , 195 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
#line 1315 "gram.y"
0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 194 , 191 , 189 , 190 ,
#line 1315 "gram.y"
188 } ;

#line 1471 "gram.y"
yytabelem yypact [428]= { 665 , - 1000 , - 1000 , - 1000 , - 1000 , - 1000 , -
#line 1471 "gram.y"
1000 , - 1000 , - 1000 , 398 , 666 , 492 , - 1000 , 47 , - 1000 , 264 ,
#line 1471 "gram.y"
239 , 621 , 620 , - 1000 , - 1000 , - 1000 , - 67 , 1142 , - 17 ,
#line 1471 "gram.y"
- 18 , - 1000 , - 1000 , 23 , - 6 , 4 , 455 , 202 , - 1000 ,
#line 1471 "gram.y"
- 1000 , 47 , - 1000 , - 1000 , 223 , - 1000 , 870 , 800 , - 1000 ,
#line 1471 "gram.y"
242 , - 1000 , 718 , - 1000 , 870 , - 1000 , - 1000 , - 1000 , -
#line 1471 "gram.y"
1000 , 405 , - 1000 , 73 , 245 , 226 , - 1000 , 514 , 47 , 526 , 47 , -
#line 1471 "gram.y"
1000 , - 1000 , - 1000 , - 1000 , - 1000 , - 1000 , - 1000 , -
#line 1471 "gram.y"
1000 , - 1000 , - 1000 , - 1000 , - 1000 , - 1000 , - 1000 , -
#line 1471 "gram.y"
1000 , 341 , 282 , - 1000 , - 1000 , - 1000 , - 1000 , - 1000 , -
#line 1471 "gram.y"
1000 , - 1000 , - 1000 , - 1000 , - 5 , - 1000 , - 1000 , -
#line 1471 "gram.y"
1000 , - 1000 , 418 , 10 , 77 , - 1000 , - 1000 , - 1000 , 32 , 339 ,
#line 1471 "gram.y"
242 , 800 , 220 , - 1000 , 870 , 879 , 934 , 296 , 393 , - 4 , 48 , 57 ,
#line 1471 "gram.y"
998 , 998 , 998 , 998 , 998 , 998 , 998 , 1062 , - 28 , - 1000 , 870 , -
#line 1471 "gram.y"
1000 , - 1000 , - 1000 , - 1000 , - 1000 , - 1000 , - 114 , 1142 ,
#line 1471 "gram.y"
167 , 879 , 800 , 74 , - 1000 , 171 , 666 , - 1000 , 333 , 66 , 185 , -
#line 1471 "gram.y"
1000 , - 1000 , - 1000 , 58 , 393 , - 4 , 1413 , 325 , 620 , - 1000 ,
#line 1471 "gram.y"
- 1000 , - 1000 , - 1000 , 251 , - 1000 , - 1000 , - 1000 , -
#line 1471 "gram.y"
5 , - 1000 , - 1000 , - 1000 , 153 , 151 , - 1000 , 211 , 34 , 42 ,
#line 1471 "gram.y"
- 1000 , 192 , 10 , - 1000 , - 97 , 136 , 74 , - 1000 , - 1000 ,
#line 1471 "gram.y"
455 , 879 , 870 , 870 , 870 , 870 , 870 , 870 , 870 , 870 , 870 , 870 , 870 , 870 ,
#line 1471 "gram.y"
870 , 870 , 870 , 870 , 296 , 870 , - 1000 , 870 , 800 , 214 , 99 , 800 , 800 ,
#line 1471 "gram.y"
241 , - 1000 , 57 , 122 , - 54 , 123 , 296 , 296 , 296 , 296 , 296 , 296 ,
#line 1471 "gram.y"
296 , 296 , 57 , - 1000 , 1382 , 205 , - 1000 , - 1000 , - 1000 , 26 ,
#line 1471 "gram.y"
- 1000 , - 56 , 183 , - 1000 , 800 , 384 , - 1000 , - 1000 , 319 ,
#line 1471 "gram.y"
127 , 800 , - 1000 , 307 , - 1000 , - 8 , 3 , - 1000 , - 1000 ,
#line 1471 "gram.y"
505 , - 1000 , - 1000 , 57 , - 1000 , - 1000 , - 1000 , - 1000 ,
#line 1471 "gram.y"
- 1000 , 130 , 245 , 226 , - 1000 , 10 , 870 , 28 , 61 , - 69 , -
#line 1471 "gram.y"
1000 , - 1000 , 879 , 19 , 19 , - 1000 , 279 , 726 , 814 , 412 , 350 , -
#line 1471 "gram.y"
1000 , 301 , 948 , 942 , 879 , 879 , 1351 , 1320 , 1289 , 313 , - 1000 , 998 , -
#line 1471 "gram.y"
120 , - 1000 , 998 , - 120 , 311 , 305 , - 1000 , 1142 , 304 , 435 , -
#line 1471 "gram.y"
5 , 998 , 435 , - 15 , - 15 , - 62 , - 1000 , - 1000 , 1142 ,
#line 1471 "gram.y"
- 1000 , 601 , - 1000 , 17 , 171 , 294 , 800 , - 1000 , 291 , - 1000 ,
#line 1471 "gram.y"
- 1000 , - 1000 , 800 , - 1000 , 435 , 438 , 367 , - 1000 , - 1000 ,
#line 1471 "gram.y"
879 , - 1000 , - 1000 , 36 , 870 , 998 , - 1000 , - 1000 , 296 , 296 ,
#line 1471 "gram.y"
- 1000 , - 1000 , - 1000 , - 1000 , - 1000 , - 1000 , 435 , 296 ,
#line 1471 "gram.y"
- 1000 , 435 , 370 , 1043 , - 1000 , - 1000 , 503 , - 1000 , 95 , 362 ,
#line 1471 "gram.y"
- 1000 , - 1000 , - 1000 , 345 , 345 , 357 , 345 , - 42 , - 1000 ,
#line 1471 "gram.y"
208 , 1444 , - 1000 , - 1000 , 870 , 6 , - 1000 , - 1000 , - 1000 ,
#line 1471 "gram.y"
- 1000 , 281 , - 1000 , - 1000 , 251 , - 1000 , - 1000 , 382 , 296 ,
#line 1471 "gram.y"
251 , - 1000 , - 1000 , - 1000 , 1 , 601 , 870 , 601 , - 1000 , 601 ,
#line 1471 "gram.y"
- 1000 , 870 , - 1000 , 1444 , - 1000 , 601 , - 1000 , - 1000 , -
#line 1471 "gram.y"
1000 , - 1000 , - 1000 , 243 , 441 , 1258 , - 1000 , 601 , - 1000 , 601 ,
#line 1471 "gram.y"
1227 , 601 , 383 , 49 , 601 , - 1000 , 870 , - 1000 , 601 , - 1000 , 345 ,
#line 1471 "gram.y"
- 1000 , - 1000 , 1196 , - 1000 , - 1000 , 870 , 1150 , 601 , - 1000 }
#line 1471 "gram.y"
;

#line 1516 "gram.y"
yytabelem yypgo [58]= { 0 , 530 , 226 , 148 , 275 , 471 , 76 , 529 , 7 , 287 , 524 , 2 ,
#line 1516 "gram.y"
5 , 31 , 521 , 23 , 30 , 6 , 29 , 520 , 22 , 517 , 514 , 73 , 508 , 49 ,
#line 1516 "gram.y"
357 , 8 , 499 , 28 , 13 , 494 , 493 , 4 , 0 , 11 , 24 , 12 , 20 , 51 ,
#line 1516 "gram.y"
10 , 490 , 15 , 19 , 483 , 3 , 482 , 14 , 21 , 1 , 122 , 9 , 472 , 469 ,
#line 1516 "gram.y"
464 , 463 , 456 , 454 } ;

#line 1524 "gram.y"
yytabelem yyr1 [244]= { 0 , 10 , 10 , 10 , 1 , 1 , 1 , 1 , 1 , 2 , 2 , 4 ,
#line 1524 "gram.y"
3 , 6 , 6 , 7 , 7 , 8 , 8 , 5 , 5 , 23 , 23 , 23 , 23 , 24 ,
#line 1524 "gram.y"
24 , 9 , 9 , 14 , 14 , 14 , 14 , 13 , 13 , 13 , 13 , 13 , 15 , 15 ,
#line 1524 "gram.y"
16 , 16 , 17 , 17 , 17 , 20 , 20 , 19 , 19 , 19 , 19 , 18 , 18 , 21 ,
#line 1524 "gram.y"
21 , 22 , 22 , 22 , 22 , 22 , 22 , 22 , 22 , 25 , 25 , 25 , 25 , 51 ,
#line 1524 "gram.y"
51 , 51 , 51 , 51 , 51 , 51 , 51 , 51 , 51 , 51 , 51 , 51 , 51 , 51 ,
#line 1524 "gram.y"
51 , 51 , 51 , 51 , 51 , 51 , 51 , 51 , 50 , 50 , 50 , 50 , 26 , 26 ,
#line 1524 "gram.y"
26 , 26 , 26 , 26 , 26 , 26 , 26 , 26 , 26 , 26 , 26 , 26 , 26 , 42 ,
#line 1524 "gram.y"
42 , 42 , 42 , 42 , 42 , 42 , 47 , 47 , 47 , 37 , 37 , 37 , 37 , 37 ,
#line 1524 "gram.y"
39 , 39 , 28 , 28 , 49 , 52 , 29 , 29 , 29 , 31 , 31 , 31 , 31 , 31 ,
#line 1524 "gram.y"
53 , 31 , 30 , 30 , 30 , 30 , 30 , 30 , 30 , 30 , 54 , 30 , 30 , 55 ,
#line 1524 "gram.y"
30 , 56 , 30 , 57 , 30 , 33 , 32 , 32 , 27 , 27 , 34 , 34 , 34 , 34 ,
#line 1524 "gram.y"
34 , 34 , 34 , 34 , 34 , 34 , 34 , 34 , 34 , 34 , 34 , 34 , 34 , 34 ,
#line 1524 "gram.y"
34 , 34 , 35 , 35 , 35 , 35 , 35 , 35 , 35 , 35 , 35 , 35 , 35 , 35 ,
#line 1524 "gram.y"
35 , 35 , 35 , 35 , 35 , 35 , 35 , 35 , 35 , 35 , 35 , 35 , 35 , 35 ,
#line 1524 "gram.y"
35 , 35 , 35 , 35 , 35 , 35 , 36 , 36 , 36 , 36 , 36 , 36 , 36 , 36 ,
#line 1524 "gram.y"
36 , 38 , 41 , 41 , 40 , 48 , 44 , 44 , 45 , 45 , 45 , 46 , 46 , 43 ,
#line 1524 "gram.y"
43 , 12 , 12 , 12 , 12 , 12 , 11 , 11 } ;

#line 1551 "gram.y"
yytabelem yyr2 [244]= { 0 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 11 , 9 , 5 , 11 ,
#line 1551 "gram.y"
9 , 5 , 1 , 2 , 7 , 7 , 9 , 5 , 1 , 2 , 7 , 5 , 7 , 3 ,
#line 1551 "gram.y"
7 , 7 , 5 , 3 , 3 , 2 , 2 , 2 , 5 , 5 , 5 , 5 , 9 , 11 ,
#line 1551 "gram.y"
3 , 7 , 3 , 7 , 1 , 7 , 9 , 5 , 7 , 11 , 13 , 3 , 2 , 5 ,
#line 1551 "gram.y"
1 , 2 , 4 , 2 , 4 , 2 , 2 , 5 , 7 , 3 , 5 , 5 , 5 , 2 ,
#line 1551 "gram.y"
2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 5 , 5 , 2 ,
#line 1551 "gram.y"
2 , 2 , 2 , 2 , 3 , 3 , 3 , 3 , 5 , 5 , 7 , 7 , 5 , 5 ,
#line 1551 "gram.y"
9 , 9 , 13 , 2 , 7 , 5 , 5 , 5 , 5 , 5 , 5 , 9 , 9 , 3 ,
#line 1551 "gram.y"
5 , 1 , 5 , 5 , 9 , 9 , 1 , 5 , 5 , 1 , 5 , 5 , 9 , 9 ,
#line 1551 "gram.y"
1 , 5 , 5 , 3 , 7 , 1 , 9 , 5 , 7 , 3 , 3 , 3 , 5 , 5 ,
#line 1551 "gram.y"
1 , 11 , 4 , 11 , 3 , 3 , 2 , 7 , 11 , 7 , 1 , 19 , 7 , 1 ,
#line 1551 "gram.y"
9 , 1 , 11 , 1 , 9 , 3 , 3 , 7 , 2 , 7 , 7 , 7 , 7 , 7 ,
#line 1551 "gram.y"
7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 11 , 5 , 11 ,
#line 1551 "gram.y"
2 , 1 , 9 , 9 , 5 , 9 , 5 , 9 , 5 , 5 , 5 , 5 , 5 , 5 ,
#line 1551 "gram.y"
5 , 5 , 9 , 9 , 9 , 7 , 9 , 7 , 7 , 9 , 7 , 5 , 2 , 7 ,
#line 1551 "gram.y"
3 , 3 , 3 , 3 , 3 , 3 , 3 , 7 , 7 , 5 , 9 , 9 , 5 , 9 ,
#line 1551 "gram.y"
9 , 5 , 3 , 3 , 5 , 5 , 5 , 9 , 7 , 9 , 11 , 7 , 3 , 2 ,
#line 1551 "gram.y"
1 , 3 , 3 , 5 , 5 , 3 , 7 , 3 } ;

#line 1578 "gram.y"
yytabelem yychk [428]= { - 1000 , - 10 , - 1 , 72 , 0 , - 9 , - 4 ,
#line 1578 "gram.y"
- 3 , - 2 , 1 , - 13 , - 26 , - 14 , 123 , - 25 ,
#line 1578 "gram.y"
80 , - 50 , - 12 , 40 , 97 , - 20 , - 15 , 47 , 24 , 50 ,
#line 1578 "gram.y"
52 , 173 , - 19 , 13 , 156 , 40 , - 26 , - 24 , 72 , 97 , 123 ,
#line 1578 "gram.y"
- 20 , - 15 , 80 , - 23 , 69 , 70 , 72 , - 5 , - 45 ,
#line 1578 "gram.y"
40 , - 11 , 42 , 46 , 45 , 160 , - 45 , 40 , - 11 , 45 , 123 ,
#line 1578 "gram.y"
80 , - 25 , - 26 , 123 , - 26 , 123 , 123 , - 51 , - 40 ,
#line 1578 "gram.y"
54 , 55 , 50 , 52 , 65 , 64 , 94 , 92 , 93 , 91 , 66 , 67 , 40 , 42 ,
#line 1578 "gram.y"
46 , 47 , 95 , 90 , 70 , 23 , 9 , 44 , 45 , - 41 , 97 , 123 , 97 ,
#line 1578 "gram.y"
97 , - 21 , 73 , - 18 , 80 , 123 , 73 , - 18 , 81 , - 5 ,
#line 1578 "gram.y"
70 , 71 , 72 , 69 , - 34 , 9 , - 35 , 97 , 123 , 23 , 113 , 50 ,
#line 1578 "gram.y"
52 , 55 , 54 , 46 , 47 , 95 , 30 , 160 , - 36 , 40 , 86 , 82 , 83 ,
#line 1578 "gram.y"
81 , 84 , 34 , 80 , 24 , - 27 , - 34 , 73 , - 6 , - 9 ,
#line 1578 "gram.y"
69 , - 13 , 123 , - 33 , - 46 , - 32 , - 43 , - 27 ,
#line 1578 "gram.y"
- 44 , - 13 , 97 , 123 , - 34 , - 33 , 173 , - 25 , 80 ,
#line 1578 "gram.y"
45 , 45 , 41 , 41 , 43 , - 39 , - 12 , 74 , - 22 , - 9 ,
#line 1578 "gram.y"
- 4 , - 3 , - 2 , 175 , - 50 , - 16 , - 17 , 80 ,
#line 1578 "gram.y"
73 , 73 , 69 , 41 , - 6 , - 27 , - 23 , - 26 , - 34 ,
#line 1578 "gram.y"
70 , 54 , 55 , 50 , 52 , 65 , 64 , 94 , 92 , 93 , 91 , 66 , 67 , 90 ,
#line 1578 "gram.y"
71 , 68 , - 35 , 42 , 95 , 42 , 40 , 44 , 45 , 40 , 40 , 160 , -
#line 1578 "gram.y"
48 , 40 , - 13 , - 38 , - 13 , - 35 , - 35 , - 35 ,
#line 1578 "gram.y"
- 35 , - 35 , - 35 , - 35 , - 35 , 113 , - 18 , -
#line 1578 "gram.y"
34 , 160 , - 51 , - 40 , 72 , - 33 , - 29 , 73 , - 7 ,
#line 1578 "gram.y"
- 8 , 40 , 80 , 41 , 41 , 155 , 71 , 71 , 123 , - 42 , 80 , -
#line 1578 "gram.y"
12 , 40 , 43 , 41 , - 26 , - 45 , - 11 , 40 , - 39 , 97 ,
#line 1578 "gram.y"
72 , 72 , 69 , - 18 , 123 , 80 , 74 , 71 , 70 , - 16 , 123 , 175 ,
#line 1578 "gram.y"
72 , - 29 , - 34 , - 34 , - 34 , - 34 , - 34 , -
#line 1578 "gram.y"
34 , - 34 , - 34 , - 34 , - 34 , - 34 , - 34 , -
#line 1578 "gram.y"
34 , - 34 , - 34 , - 34 , - 34 , - 34 , - 33 , -
#line 1578 "gram.y"
36 , 50 , 123 , - 36 , 50 , 123 , - 33 , - 33 , - 18 , 24 ,
#line 1578 "gram.y"
- 48 , - 47 , - 12 , 122 , - 37 , - 12 , 40 , - 38 ,
#line 1578 "gram.y"
41 , - 18 , 24 , 74 , - 52 , 74 , 256 , 71 , - 33 , 40 , 41 ,
#line 1578 "gram.y"
155 , - 43 , - 27 , - 11 , 70 , 123 , - 42 , - 42 , 41 ,
#line 1578 "gram.y"
72 , - 17 , - 34 , 74 , 73 , 123 , 69 , 43 , 43 , 41 , - 35 ,
#line 1578 "gram.y"
- 35 , 41 , 41 , - 51 , - 40 , 41 , - 11 , - 47 , -
#line 1578 "gram.y"
35 , - 11 , - 37 , - 37 , 122 , - 51 , - 40 , - 28 ,
#line 1578 "gram.y"
- 30 , - 31 , 1 , - 9 , - 4 , - 29 , 20 , 39 , 16 ,
#line 1578 "gram.y"
33 , 80 , 4 , 8 , - 34 , 3 , 7 , 28 , 19 , 10 , 74 , - 8 ,
#line 1578 "gram.y"
41 , - 33 , 41 , - 27 , 41 , - 45 , 73 , - 34 , - 35 ,
#line 1578 "gram.y"
41 , - 30 , 74 , 72 , 40 , - 49 , 40 , - 49 , 40 , - 49 ,
#line 1578 "gram.y"
69 , - 56 , 69 , - 34 , 80 , - 53 , 41 , - 45 , - 11 ,
#line 1578 "gram.y"
- 45 , - 11 , 81 , - 30 , - 34 , - 30 , - 54 , -
#line 1578 "gram.y"
30 , - 55 , - 34 , - 57 , - 30 , 41 , 12 , 41 , - 30 ,
#line 1578 "gram.y"
- 30 , 69 , - 30 , 39 , 72 , - 30 , - 34 , - 30 , -
#line 1578 "gram.y"
49 , 72 , - 34 , 41 , - 30 } ;

#line 1623 "gram.y"
yytabelem yydef [428]= { 0 , - 2 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 0 , 0 ,
#line 1623 "gram.y"
20 , 33 , 30 , 99 , 63 , 0 , 0 , 0 , 29 , 31 , 32 , 0 , 0 , 237 ,
#line 1623 "gram.y"
238 , 241 , 54 , 0 , 0 , 0 , - 2 , 0 , 28 , 34 , 35 , 36 , 37 ,
#line 1623 "gram.y"
63 , 25 , 181 , 181 , 10 , 14 , 94 , - 2 , 106 , 181 , 243 , 90 , 91 ,
#line 1623 "gram.y"
95 , - 2 , 105 , 0 , 102 , 63 , 101 , 103 , 104 , 0 , 0 , 64 , 65 ,
#line 1623 "gram.y"
66 , 67 , 68 , 69 , 70 , 71 , 72 , 73 , 74 , 75 , 76 , 77 , 78 , 0 ,
#line 1623 "gram.y"
0 , 81 , 82 , 83 , 84 , 85 , 86 , 87 , 88 , 89 , 124 , 224 , 225 , 239 ,
#line 1623 "gram.y"
240 , 0 , 44 , 0 , 51 , 52 , 47 , 0 , 0 , 14 , 181 , 0 , 27 , 181 ,
#line 1623 "gram.y"
23 , 0 , 180 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
#line 1623 "gram.y"
0 , 0 , 206 , 181 , 208 , 209 , 210 , 211 , 212 , 213 , 214 , 0 , 0 , 160 ,
#line 1623 "gram.y"
181 , 0 , 19 , 0 , 0 , 30 , 0 , 0 , 157 , 234 , 158 , 235 , 111 , 29 ,
#line 1623 "gram.y"
30 , 0 , 0 , 0 , 100 , 63 , 92 , 93 , 0 , 79 , 80 , 226 , 124 , 45 ,
#line 1623 "gram.y"
53 , 55 , 57 , 59 , 60 , 0 , 0 , 0 , 40 , 42 , 44 , 48 , 0 , 0 ,
#line 1623 "gram.y"
0 , 24 , 26 , 21 , 22 , 181 , 181 , 181 , 181 , 181 , 181 , 181 , 181 , 181 ,
#line 1623 "gram.y"
181 , 181 , 181 , 181 , 181 , 181 , 181 , 178 , 181 , 186 , 181 , 181 , 0 , 0 ,
#line 1623 "gram.y"
181 , 181 , 0 , 184 , 0 , 116 , 0 , 119 , 188 , 189 , 190 , 191 , 192 , 193 ,
#line 1623 "gram.y"
194 , 195 , 0 , 205 , 0 , 0 , 217 , 220 , 9 , 0 , 12 , - 2 , 13 ,
#line 1623 "gram.y"
15 , 181 , 0 , 96 , 230 , 0 , 236 , 181 , 35 , 228 , 109 , 111 , 111 , 242 ,
#line 1623 "gram.y"
97 , 0 , 107 , 108 , 236 , 125 , 46 , 56 , 58 , 61 , 0 , - 2 , -
#line 1623 "gram.y"
2 , 38 , 44 , 181 , 0 , 0 , 0 , 8 , 11 , 162 , 163 , 164 , 165 , 166 ,
#line 1623 "gram.y"
167 , 168 , 169 , 170 , 171 , 172 , 173 , 174 , 175 , 176 , 0 , 0 , 0 , 0 ,
#line 1623 "gram.y"
199 , 0 , 201 , 202 , 0 , 204 , 0 , 0 , 215 , 0 , 0 , 227 , 116 , 0 ,
#line 1623 "gram.y"
223 , 119 , 119 , 0 , 207 , 216 , 0 , 161 , 181 , 131 , 0 , 0 , 0 , 181 ,
#line 1623 "gram.y"
231 , 0 , 233 , 159 , 113 , 181 , 110 , 112 , 0 , 0 , 62 , 41 , 43 , 39 ,
#line 1623 "gram.y"
49 , 0 , 181 , 0 , 197 , 198 , 200 , 203 , 182 , 183 , 218 , 221 , 185 , 118 ,
#line 1623 "gram.y"
117 , 187 , 121 , 120 , 0 , 196 , 219 , 222 , 181 , 127 , 0 , 0 , 142 , 143 ,
#line 1623 "gram.y"
144 , 0 , 0 , 0 , 0 , 214 , 153 , 0 , 133 , 134 , 135 , 181 , 0 , 138 ,
#line 1623 "gram.y"
132 , 16 , 17 , 0 , 232 , 229 , 0 , 98 , 50 , 177 , 179 , 0 , 126 , 130 ,
#line 1623 "gram.y"
140 , 0 , 181 , 181 , 181 , 148 , 181 , 151 , 181 , 155 , 136 , 137 , 181 , 18 ,
#line 1623 "gram.y"
114 , 115 , 122 , 123 , 0 , 145 , 0 , 147 , 181 , 150 , 181 , 0 , 181 , 0 ,
#line 1623 "gram.y"
0 , 181 , 128 , 181 , 152 , 181 , 156 , 0 , 141 , 146 , 0 , 154 , 139 , 181 ,
#line 1623 "gram.y"
0 , 181 , 149 } ;

#line 1668 "gram.y"
struct _C13 {	/* sizeof _C13 == 8 */
char *__C13_t_name ;

#line 1668 "gram.y"
int __C13_t_val ;
};

#line 1668 "gram.y"
typedef struct _C13 yytoktype ;

#ident "@(#)/usr/src/cmd/yacc/yaccpar	1.3 6/11/85 14:16:28 - Amdahl/UTS"

#line 2029 "gram.y"
int yydebug = 0 ;

#line 2039 "gram.y"
union _C10 yyv [300];
int yys [300];

#line 2042 "gram.y"
union _C10 *yypv = 0 ;
int *yyps = 0 ;

#line 2045 "gram.y"
int yystate = 0 ;
int yytmp = 0 ;

#line 2048 "gram.y"
int yynerrs = 0 ;
int yyerrflag = 0 ;
int yychar = 0 ;

#line 2058 "gram.y"
extern int yyparse ()
#line 2059 "gram.y"
{ 
#line 2060 "gram.y"
register union _C10 *_au1_yypvt ;

#line 2061 "gram.y"
struct slist *_au0__Xthis__ctor_slist ;

#line 2061 "gram.y"
struct block *_au0__Xthis__ctor_block ;

#line 2061 "gram.y"
struct estmt *_au0__Xthis__ctor_estmt ;

#line 2061 "gram.y"
struct lstmt *_au0__Xthis__ctor_lstmt ;

#line 2061 "gram.y"
struct ifstmt *_au0__Xthis__ctor_ifstmt ;

#line 2061 "gram.y"
struct forstmt *_au0__Xthis__ctor_forstmt ;

#line 2061 "gram.y"
struct elist *_au0__Xthis__ctor_elist ;

#line 2061 "gram.y"
Pexpr _au1__Xe__ctor_elist ;

#line 2061 "gram.y"
Pexpr _au1__Xe_add_elist ;

#line 2061 "gram.y"
struct qexpr *_au0__Xthis__ctor_qexpr ;

#line 2061 "gram.y"
struct texpr *_au0__Xthis__ctor_texpr ;

#line 2061 "gram.y"
Ptype _au1__Xtt__ctor_texpr ;

#line 2061 "gram.y"
struct ref *_au0__Xthis__ctor_ref ;

#line 2061 "gram.y"
Pname _au1__Xb__ctor_ref ;

#line 2061 "gram.y"
struct ptr *_au0__Xthis__ctor_ptr ;

#line 2061 "gram.y"
struct vec *_au0__Xthis__ctor_vec ;

#line 2065 "gram.y"
yypv = (& (yyv [-1]));
yyps = (& (yys [-1]));
yystate = 0 ;
yytmp = 0 ;
yynerrs = 0 ;
yyerrflag = 0 ;
yychar = -1;

#line 2073 "gram.y"
goto yystack ;
{ 
#line 2075 "gram.y"
register union _C10 *_au2_yy_pv ;
register int *_au2_yy_ps ;
register int _au2_yy_state ;
register int _au2_yy_n ;

#line 2084 "gram.y"
yynewstate :
#line 2085 "gram.y"
_au2_yy_pv = yypv ;
_au2_yy_ps = yyps ;
_au2_yy_state = yystate ;
goto yy_newstate ;

#line 2094 "gram.y"
yystack :
#line 2095 "gram.y"
_au2_yy_pv = yypv ;
_au2_yy_ps = yyps ;
_au2_yy_state = yystate ;

#line 2102 "gram.y"
yy_stack :
#line 2134 "gram.y"
if ((++ _au2_yy_ps )>= (& (yys [300 ])))
#line 2135 "gram.y"
{ 
#line 2136 "gram.y"
yyerror ( "yacc stack overflow") ;
return 1 ;
}
(*_au2_yy_ps )= _au2_yy_state ;
(*(++ _au2_yy_pv ))= yyval ;

#line 2145 "gram.y"
yy_newstate :
#line 2146 "gram.y"
if ((_au2_yy_n = (yypact [_au2_yy_state ]))<= -1000)
#line 2147 "gram.y"
goto yydefault ;

#line 2154 "gram.y"
if ((yychar < 0 )&& ((yychar = lalex ( ) )< 0 ))
#line 2155 "gram.y"
yychar = 0 ;

#line 2178 "gram.y"
if (((_au2_yy_n += yychar )< 0 )|| (_au2_yy_n >= 1539 ))
#line 2179 "gram.y"
goto yydefault ;
if ((yychk [_au2_yy_n = (yyact [_au2_yy_n ])])== yychar )
#line 2181 "gram.y"
{ 
#line 2182 "gram.y"
yychar = -1;
yyval = yylval ;
_au2_yy_state = _au2_yy_n ;
if (yyerrflag > 0 )
#line 2186 "gram.y"
yyerrflag -- ;
goto yy_stack ;
}

#line 2190 "gram.y"
yydefault :
#line 2191 "gram.y"
if ((_au2_yy_n = (yydef [_au2_yy_state ]))== -2)
#line 2192 "gram.y"
{ 
#line 2196 "gram.y"
if ((yychar < 0 )&& ((yychar = lalex ( ) )< 0 ))
#line 2197 "gram.y"
yychar = 0 ;

#line 2227 "gram.y"
{ 
#line 2228 "gram.y"
register int *_au4_yyxi ;

#line 2228 "gram.y"
_au4_yyxi = yyexca ;

#line 2230 "gram.y"
while (((*_au4_yyxi )!= -1)|| ((_au4_yyxi [1 ])!= _au2_yy_state ))
#line 2232 "gram.y"
{ 
#line 2233 "gram.y"
_au4_yyxi += 2 ;
}
while (((*(_au4_yyxi += 2 ))>= 0 )&& ((*_au4_yyxi )!= yychar ))
#line 2237 "gram.y"
;
if ((_au2_yy_n = (_au4_yyxi [1 ]))< 0 )
#line 2239 "gram.y"
return (int )0 ;
}
}

#line 2246 "gram.y"
if (_au2_yy_n == 0 )
#line 2247 "gram.y"
{ 
#line 2249 "gram.y"
switch (yyerrflag )
#line 2250 "gram.y"
{ 
#line 2251 "gram.y"
case 0 : 
#line 2252 "gram.y"
yyerror ( "syntax error") ;
goto skip_init ;
yyerrlab :
#line 2259 "gram.y"
_au2_yy_pv = yypv ;
_au2_yy_ps = yyps ;
_au2_yy_state = yystate ;
yynerrs ++ ;
skip_init :
#line 2264 "gram.y"
case 1 : 
#line 2265 "gram.y"
case 2 : 
#line 2267 "gram.y"
yyerrflag = 3 ;

#line 2272 "gram.y"
while (_au2_yy_ps >= yys )
#line 2273 "gram.y"
{ 
#line 2274 "gram.y"
_au2_yy_n = ((yypact [*_au2_yy_ps ])+ 256 );
if (((_au2_yy_n >= 0 )&& (_au2_yy_n < 1539 ))&& ((yychk [yyact [_au2_yy_n ]])== 256 ))
#line 2276 "gram.y"
{ 
#line 2280 "gram.y"
_au2_yy_state = (yyact [_au2_yy_n ]);
goto yy_stack ;
}

#line 2294 "gram.y"
_au2_yy_ps -- ;
_au2_yy_pv -- ;
}

#line 2301 "gram.y"
return 1 ;
case 3 : 
#line 2337 "gram.y"
if (yychar == 0 )
#line 2338 "gram.y"
return 1 ;
yychar = -1;
goto yy_newstate ;
}
}

#line 2357 "gram.y"
yytmp = _au2_yy_n ;
_au1_yypvt = _au2_yy_pv ;

#line 2371 "gram.y"
{ 
#line 2373 "gram.y"
register int _au3_yy_len ;

#line 2373 "gram.y"
_au3_yy_len = (yyr2 [_au2_yy_n ]);

#line 2375 "gram.y"
if (! (_au3_yy_len & 01 ))
#line 2376 "gram.y"
{ 
#line 2377 "gram.y"
_au3_yy_len >>= 1 ;
yyval = ((_au2_yy_pv -= _au3_yy_len )[1 ]);

#line 2380 "gram.y"
_au2_yy_state = (((yypgo [_au2_yy_n = (yyr1 [_au2_yy_n ])])+ (*(_au2_yy_ps -= _au3_yy_len )))+ 1 );
if ((_au2_yy_state >= 1539 )|| ((yychk [_au2_yy_state = (yyact [_au2_yy_state ])])!= (- _au2_yy_n )))
#line 2384 "gram.y"
{ 
#line 2385 "gram.y"
_au2_yy_state = (yyact [yypgo [_au2_yy_n ]]);
}
goto yy_stack ;
}
_au3_yy_len >>= 1 ;
yyval = ((_au2_yy_pv -= _au3_yy_len )[1 ]);

#line 2392 "gram.y"
_au2_yy_state = (((yypgo [_au2_yy_n = (yyr1 [_au2_yy_n ])])+ (*(_au2_yy_ps -= _au3_yy_len )))+ 1 );
if ((_au2_yy_state >= 1539 )|| ((yychk [_au2_yy_state = (yyact [_au2_yy_state ])])!= (- _au2_yy_n )))
#line 2395 "gram.y"
{ 
#line 2396 "gram.y"
_au2_yy_state = (yyact [yypgo [_au2_yy_n ]]);
}
}

#line 2400 "gram.y"
yystate = _au2_yy_state ;
yyps = _au2_yy_ps ;
yypv = _au2_yy_pv ;
}

#line 2407 "gram.y"
switch (yytmp )
#line 2408 "gram.y"
{ 
#line 2410 "gram.y"
case 1 : 
#line 309 "gram.y"
{ return 2 ;
}

#line 309 "gram.y"
break ;
case 2 : { return 1 ;
}

#line 310 "gram.y"
break ;
case 3 : { return (int )0 ;
}

#line 311 "gram.y"
break ;
case 4 : 
#line 315 "gram.y"
{ modified_tn = 0 ;

#line 315 "gram.y"
if ((_au1_yypvt [- 0 ]). __C10_pn == 0 )yyval . __C10_i = 1 ;
}

#line 315 "gram.y"
break ;
case 5 : 
#line 317 "gram.y"
{ goto mod ;
}

#line 317 "gram.y"
break ;
case 6 : 
#line 319 "gram.y"
{ goto mod ;
}

#line 319 "gram.y"
break ;
case 7 : 
#line 321 "gram.y"
{ mod :if (modified_tn ){ 
#line 322 "gram.y"
restore ( ) ;
modified_tn = 0 ;
}
}

#line 325 "gram.y"
break ;
case 8 : 
#line 327 "gram.y"
{ Pname _au3_n ;

#line 327 "gram.y"
_au3_n = (struct name *)_name__ctor ( (struct name *)0 , make_name ( (unsigned char )'A' ) ) ;
_au3_n -> _expr__O2.__C2_tp = (struct type *)_basetype__ctor ( (struct basetype *)0 , (unsigned char )1 , (struct name *)0 ) ;
(((struct basetype *)_au3_n -> _expr__O2.__C2_tp ))-> _basetype_b_name = (((struct name *)(_au1_yypvt [-2]). __C10_s ));
yyval . __C10_p = (struct node *)_au3_n ;
}

#line 331 "gram.y"
break ;
case 9 : 
#line 335 "gram.y"
{ errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"T ofIdE too complicated (useTdef or leave out theIr)", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ,
#line 335 "gram.y"
(struct ea *)ea0 ) ;
goto fix ;
}

#line 337 "gram.y"
break ;
case 10 : 
#line 339 "gram.y"
{ Pname _au3_n ;
Ptype _au3_t ;
fix :
#line 342 "gram.y"
if ((_au3_n = (_au1_yypvt [-1]). __C10_pn )== 0 ){ 
#line 343 "gram.y"
error ( (char *)"syntax error:TX", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ,
#line 343 "gram.y"
(struct ea *)ea0 ) ;
yyval . __C10_p = (struct node *)_name_normalize ( _au3_n , ((struct basetype *)defa_type ), (struct block *)0 , (char )0 ) ;
}
else if ((_au3_t = _au3_n -> _expr__O2.__C2_tp )== 0 ){ 
#line 347 "gram.y"
{ 
#line 1237 "gram.y"
struct ea _au0__V14 ;

#line 347 "gram.y"
error ( (char *)"TX for%n", (struct ea *)( ( ((& _au0__V14 )-> _ea__O1.__C1_p = ((char *)_au3_n )), (((& _au0__V14 )))) )
#line 347 "gram.y"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
yyval . __C10_p = (struct node *)_name_normalize ( _au3_n , ((struct basetype *)defa_type ), (struct block *)0 , (char )0 ) ;
} }
else if (_au3_t -> _node_base == 108 ){ 
#line 351 "gram.y"
if ((((struct fct *)_au3_t ))-> _fct_returns == 0 )
#line 352 "gram.y"
yyval . __C10_p = (struct node *)_name_normalize (
#line 352 "gram.y"
_au3_n , ((struct basetype *)defa_type ), ((struct block *)0 ), (char )0 ) ;
else 
#line 354 "gram.y"
yyval . __C10_p = (struct node *)_name_normalize ( _au3_n , ((struct basetype *)0 ), (struct block *)0 , (char )0 ) ;
}
else { 
#line 357 "gram.y"
{ 
#line 1237 "gram.y"
struct ea _au0__V15 ;

#line 1237 "gram.y"
struct ea _au0__V16 ;

#line 357 "gram.y"
error ( (char *)"syntax error:TX for%k%n", (struct ea *)( ( ((& _au0__V15 )-> _ea__O1.__C1_i = ((int )_au3_t -> _node_base )), (((& _au0__V15 ))))
#line 357 "gram.y"
) , (struct ea *)( ( ((& _au0__V16 )-> _ea__O1.__C1_p = ((char *)_au3_n )), (((& _au0__V16 )))) ) ,
#line 357 "gram.y"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
yyval . __C10_p = (struct node *)_name_normalize ( _au3_n , ((struct basetype *)defa_type ), (struct block *)0 , (char )0 ) ;
} }
}

#line 360 "gram.y"
break ;
case 11 : 
#line 365 "gram.y"
{ Pname _au3_n ;

#line 365 "gram.y"
_au3_n = _name_normalize ( (_au1_yypvt [-3]). __C10_pn , ((struct basetype *)(_au1_yypvt [-4]). __C10_p ), ((struct block *)(_au1_yypvt [- 0 ]). __C10_p ), (char )0 ) ;
#line 365 "gram.y"

#line 366 "gram.y"
_fct_argdcl ( ((struct fct *)_au3_n -> _expr__O2.__C2_tp ), name_unlist ( (_au1_yypvt [-2]). __C10_nl ) , _au3_n ) ;
(((struct fct *)_au3_n -> _expr__O2.__C2_tp ))-> _fct_f_init = (_au1_yypvt [-1]). __C10_pn ;
yyval . __C10_p = (struct node *)_au3_n ;
}

#line 369 "gram.y"
break ;
case 12 : 
#line 373 "gram.y"
{ Pname _au3_n ;

#line 373 "gram.y"
_au3_n = _name_normalize ( (_au1_yypvt [-3]). __C10_pn , ((struct basetype *)defa_type ), ((struct block *)(_au1_yypvt [- 0 ]). __C10_p ), (char )0 ) ;
_fct_argdcl ( ((struct fct *)_au3_n -> _expr__O2.__C2_tp ), name_unlist ( (_au1_yypvt [-2]). __C10_nl ) , _au3_n ) ;
(((struct fct *)_au3_n -> _expr__O2.__C2_tp ))-> _fct_f_init = (_au1_yypvt [-1]). __C10_pn ;
yyval . __C10_p = (struct node *)_au3_n ;
}

#line 377 "gram.y"
break ;
case 13 : 
#line 381 "gram.y"
{ yyval . __C10_p = (_au1_yypvt [- 0 ]). __C10_p ;
}

#line 381 "gram.y"
break ;
case 14 : 
#line 383 "gram.y"
{ yyval . __C10_p = 0 ;
}

#line 383 "gram.y"
break ;
case 16 : 
#line 388 "gram.y"
{ yyval . __C10_pn = (_au1_yypvt [- 0 ]). __C10_pn ;

#line 388 "gram.y"
yyval . __C10_pn -> _name_n_list = (_au1_yypvt [-2]). __C10_pn ;
}

#line 388 "gram.y"
break ;
case 17 : 
#line 392 "gram.y"
{ yyval . __C10_pn = (struct name *)_name__ctor ( (struct name *)0 , (char *)0 ) ;
yyval . __C10_pn -> _expr__O4.__C4_n_initializer = (_au1_yypvt [-1]). __C10_pe ;
}

#line 394 "gram.y"
break ;
case 18 : 
#line 396 "gram.y"
{ yyval . __C10_pn = (_au1_yypvt [-3]). __C10_pn ;
yyval . __C10_pn -> _expr__O4.__C4_n_initializer = (_au1_yypvt [-1]). __C10_pe ;
}

#line 398 "gram.y"
break ;
case 19 : 
#line 407 "gram.y"
{ if ((_au1_yypvt [- 0 ]). __C10_pn == 0 )
#line 408 "gram.y"
error ( (char *)"badAD", (struct ea *)ea0 , (struct ea *)ea0 ,
#line 408 "gram.y"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
else if ((_au1_yypvt [- 0 ]). __C10_pn -> _expr__O2.__C2_tp -> _node_base == 108 )
#line 410 "gram.y"
{ 
#line 1237 "gram.y"
struct ea _au0__V17 ;

#line 410 "gram.y"
error ( (char *)"FD inAL (%n)", (struct ea *)( ( ((& _au0__V17 )-> _ea__O1.__C1_p = ((char *)(_au1_yypvt [- 0 ]). __C10_pn )), (((&
#line 410 "gram.y"
_au0__V17 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} else if ((_au1_yypvt [-1]). __C10_p )
#line 412 "gram.y"
( ((_au1_yypvt [-1]). __C10_nl -> _nlist_tail -> _name_n_list = (_au1_yypvt [- 0 ]). __C10_pn ), ((_au1_yypvt [-1]). __C10_nl ->
#line 412 "gram.y"
_nlist_tail = (_au1_yypvt [- 0 ]). __C10_pn )) ;
else 
#line 414 "gram.y"
yyval . __C10_nl = (struct nlist *)_nlist__ctor ( (struct nlist *)0 , (_au1_yypvt [- 0 ]). __C10_pn ) ;
}

#line 415 "gram.y"
break ;
case 20 : 
#line 417 "gram.y"
{ yyval . __C10_p = 0 ;
}

#line 417 "gram.y"
break ;
case 22 : 
#line 422 "gram.y"
{ yyval . __C10_p = (struct node *)(_au1_yypvt [-2]). __C10_pn ;
yyval . __C10_pn -> _expr__O2.__C2_tp = (struct type *)_basetype__ctor ( (struct basetype *)0 , (unsigned char )114 , (_au1_yypvt [- 0 ]). __C10_pn ) ;
#line 423 "gram.y"
}
break ;
case 23 : 
#line 426 "gram.y"
{ yyval . __C10_p = (struct node *)_name__ctor ( (struct name *)0 , (char *)0 ) ;
yyval . __C10_pn -> _expr__O2.__C2_tp = (struct type *)_basetype__ctor ( (struct basetype *)0 , (unsigned char )114 , (_au1_yypvt [- 0 ]). __C10_pn ) ;
#line 427 "gram.y"
}
break ;
case 24 : 
#line 430 "gram.y"
{ Pexpr _au3_e ;

#line 430 "gram.y"
_au3_e = (_au1_yypvt [- 0 ]). __C10_pe ;
if (_au3_e == dummy )error ( (char *)"emptyIr", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 431 "gram.y"

#line 432 "gram.y"
(_au1_yypvt [-2]). __C10_pn -> _expr__O4.__C4_n_initializer = _au3_e ;
}

#line 433 "gram.y"
break ;
case 25 : 
#line 437 "gram.y"
{ if ((_au1_yypvt [- 0 ]). __C10_p )yyval . __C10_nl = (struct nlist *)_nlist__ctor ( (struct nlist *)0 , (_au1_yypvt [- 0 ]).
#line 437 "gram.y"
__C10_pn ) ;
}

#line 437 "gram.y"
break ;
case 26 : 
#line 439 "gram.y"
{ if ((_au1_yypvt [-2]). __C10_p )
#line 440 "gram.y"
if ((_au1_yypvt [- 0 ]). __C10_p )
#line 441 "gram.y"
( ((_au1_yypvt [-2]). __C10_nl -> _nlist_tail -> _name_n_list = (_au1_yypvt [-
#line 441 "gram.y"
0 ]). __C10_pn ), ((_au1_yypvt [-2]). __C10_nl -> _nlist_tail = (_au1_yypvt [- 0 ]). __C10_pn )) ;
else 
#line 443 "gram.y"
error ( (char *)"DL syntax", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
else { 
#line 445 "gram.y"
if ((_au1_yypvt [- 0 ]). __C10_p )yyval . __C10_nl = (struct nlist *)_nlist__ctor ( (struct nlist *)0 , (_au1_yypvt [- 0 ]). __C10_pn )
#line 445 "gram.y"
;
error ( (char *)"DL syntax", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
}
}

#line 448 "gram.y"
break ;
case 27 : 
#line 451 "gram.y"
{ yyval . __C10_p = (struct node *)_name_normalize ( name_unlist ( (_au1_yypvt [-1]). __C10_nl ) , ((struct basetype *)(_au1_yypvt [-2]). __C10_p ),
#line 451 "gram.y"
(struct block *)0 , (char )0 ) ;
}

#line 451 "gram.y"
break ;
case 28 : { yyval . __C10_p = (struct node *)_basetype_aggr ( (_au1_yypvt [-1]). __C10_pb ) ;
}

#line 452 "gram.y"
break ;
case 29 : 
#line 456 "gram.y"
{ yyval . __C10_p = (struct node *)_basetype__ctor ( (struct basetype *)0 , (_au1_yypvt [- 0 ]). __C10_t , (struct name *)0 )
#line 456 "gram.y"
;
}

#line 456 "gram.y"
break ;
case 30 : { yyval . __C10_p = (struct node *)_basetype__ctor ( (struct basetype *)0 , (unsigned char )97 , (_au1_yypvt [- 0 ]).
#line 457 "gram.y"
__C10_pn ) ;
}

#line 457 "gram.y"
break ;
case 34 : 
#line 463 "gram.y"
{ yyval . __C10_p = (struct node *)_basetype_type_adj ( (_au1_yypvt [-1]). __C10_pb , (_au1_yypvt [- 0 ]). __C10_t ) ;
}

#line 463 "gram.y"
break ;
case 35 : { yyval . __C10_p = (struct node *)_basetype_name_adj ( (_au1_yypvt [-1]). __C10_pb , (_au1_yypvt [- 0 ]). __C10_pn ) ;
}

#line 464 "gram.y"
break ;
case 36 : { yyval . __C10_p = (struct node *)_basetype_base_adj ( (_au1_yypvt [-1]). __C10_pb , (_au1_yypvt [- 0 ]). __C10_pb ) ;
}

#line 465 "gram.y"
break ;
case 37 : { yyval . __C10_p = (struct node *)_basetype_base_adj ( (_au1_yypvt [-1]). __C10_pb , (_au1_yypvt [- 0 ]). __C10_pb ) ;
}

#line 466 "gram.y"
break ;
case 38 : 
#line 475 "gram.y"
{ yyval . __C10_p = (struct node *)end_enum ( (struct name *)0 , (_au1_yypvt [-1]). __C10_pn ) ;
}

#line 475 "gram.y"
break ;
case 39 : { yyval . __C10_p = (struct node *)end_enum ( (_au1_yypvt [-3]). __C10_pn , (_au1_yypvt [-1]). __C10_pn ) ;
}

#line 476 "gram.y"
break ;
case 40 : 
#line 480 "gram.y"
{ if ((_au1_yypvt [- 0 ]). __C10_p )yyval . __C10_nl = (struct nlist *)_nlist__ctor ( (struct nlist *)0 , (_au1_yypvt [- 0 ]).
#line 480 "gram.y"
__C10_pn ) ;
}

#line 480 "gram.y"
break ;
case 41 : 
#line 482 "gram.y"
{ if ((_au1_yypvt [- 0 ]). __C10_p )
#line 483 "gram.y"
if ((_au1_yypvt [-2]). __C10_p )
#line 484 "gram.y"
( ((_au1_yypvt [-2]). __C10_nl -> _nlist_tail -> _name_n_list = (_au1_yypvt [-
#line 484 "gram.y"
0 ]). __C10_pn ), ((_au1_yypvt [-2]). __C10_nl -> _nlist_tail = (_au1_yypvt [- 0 ]). __C10_pn )) ;
else 
#line 486 "gram.y"
yyval . __C10_nl = (struct nlist *)_nlist__ctor ( (struct nlist *)0 , (_au1_yypvt [- 0 ]). __C10_pn ) ;
}

#line 487 "gram.y"
break ;
case 42 : 
#line 491 "gram.y"
{ yyval . __C10_p = (struct node *)(_au1_yypvt [- 0 ]). __C10_pn ;

#line 491 "gram.y"
yyval . __C10_pn -> _expr__O2.__C2_tp = (struct type *)moe_type ;
}

#line 491 "gram.y"
break ;
case 43 : 
#line 493 "gram.y"
{ yyval . __C10_p = (struct node *)(_au1_yypvt [-2]). __C10_pn ;
yyval . __C10_pn -> _expr__O2.__C2_tp = (struct type *)moe_type ;
yyval . __C10_pn -> _expr__O4.__C4_n_initializer = (_au1_yypvt [- 0 ]). __C10_pe ;
}

#line 496 "gram.y"
break ;
case 44 : 
#line 498 "gram.y"
{ yyval . __C10_p = 0 ;
}

#line 498 "gram.y"
break ;
case 45 : 
#line 503 "gram.y"
{ 
#line 504 "gram.y"
ccl -> _classdef_mem_list = name_unlist ( (_au1_yypvt [-1]). __C10_nl ) ;
end_cl ( ) ;
}

#line 506 "gram.y"
break ;
case 46 : 
#line 508 "gram.y"
{ 
#line 509 "gram.y"
ccl -> _classdef_mem_list = name_unlist ( (_au1_yypvt [-2]). __C10_nl ) ;
end_cl ( ) ;
error ( (char *)"`;' or declaratorX afterCD", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
back = (_au1_yypvt [- 0 ]). __C10_t ;
}
break ;
case 47 : 
#line 518 "gram.y"
{ yyval . __C10_p = (struct node *)start_cl ( (_au1_yypvt [-1]). __C10_t , (struct name *)0 , (struct name *)0 ) ;
#line 518 "gram.y"
}

#line 518 "gram.y"
break ;
case 48 : 
#line 520 "gram.y"
{ yyval . __C10_p = (struct node *)start_cl ( (_au1_yypvt [-2]). __C10_t , (_au1_yypvt [-1]). __C10_pn , (struct name *)0 ) ;
#line 520 "gram.y"
}

#line 520 "gram.y"
break ;
case 49 : 
#line 522 "gram.y"
{ yyval . __C10_p = (struct node *)start_cl ( (_au1_yypvt [-4]). __C10_t , (_au1_yypvt [-3]). __C10_pn , (_au1_yypvt [-1]). __C10_pn ) ;
#line 522 "gram.y"

#line 523 "gram.y"
if ((_au1_yypvt [-4]). __C10_t == 32 )ccl -> _classdef_pubbase = 1 ;
}

#line 524 "gram.y"
break ;
case 50 : 
#line 526 "gram.y"
{ 
#line 527 "gram.y"
yyval . __C10_p = (struct node *)start_cl ( (_au1_yypvt [-5]). __C10_t , (_au1_yypvt [-4]). __C10_pn , (_au1_yypvt [-1]). __C10_pn ) ;
#line 527 "gram.y"

#line 528 "gram.y"
ccl -> _classdef_pubbase = 1 ;
}

#line 529 "gram.y"
break ;
case 51 : 
#line 551 "gram.y"
{ yyval . __C10_p = (struct node *)(_au1_yypvt [- 0 ]). __C10_pn ;
}

#line 551 "gram.y"
break ;
case 53 : 
#line 556 "gram.y"
{ if ((_au1_yypvt [- 0 ]). __C10_p ){ 
#line 557 "gram.y"
if ((_au1_yypvt [-1]). __C10_p )
#line 558 "gram.y"
_nlist_add_list ( (_au1_yypvt [-1]). __C10_nl , (_au1_yypvt [- 0 ]).
#line 558 "gram.y"
__C10_pn ) ;
else 
#line 560 "gram.y"
yyval . __C10_nl = (struct nlist *)_nlist__ctor ( (struct nlist *)0 , (_au1_yypvt [- 0 ]). __C10_pn ) ;
}
}

#line 562 "gram.y"
break ;
case 54 : 
#line 564 "gram.y"
{ yyval . __C10_p = 0 ;
}

#line 564 "gram.y"
break ;
case 61 : 
#line 573 "gram.y"
{ yyval . __C10_p = (struct node *)_name__ctor ( (struct name *)0 , (char *)0 ) ;

#line 573 "gram.y"
yyval . __C10_pn -> _node_base = (_au1_yypvt [-1]). __C10_t ;
}

#line 573 "gram.y"
break ;
case 62 : 
#line 575 "gram.y"
{ Pname _au3_n ;

#line 575 "gram.y"
_au3_n = (((_au1_yypvt [-1]). __C10_pn -> _node_base == 123 )? _name__ctor ( (struct name *)0 , (_au1_yypvt [-1]). __C10_pn -> _expr__O3.__C3_string ) : (_au1_yypvt [-1]). __C10_pn );
#line 575 "gram.y"

#line 576 "gram.y"
_au3_n -> _name__O6.__C6_n_qualifier = (_au1_yypvt [-2]). __C10_pn ;
_au3_n -> _node_base = 25 ;
yyval . __C10_p = (struct node *)_au3_n ;
}

#line 579 "gram.y"
break ;
case 63 : 
#line 598 "gram.y"
{ yyval . __C10_p = (struct node *)(_au1_yypvt [- 0 ]). __C10_pn ;
}

#line 598 "gram.y"
break ;
case 64 : 
#line 600 "gram.y"
{ yyval . __C10_p = (struct node *)(((_au1_yypvt [- 0 ]). __C10_pn -> _node_base == 123 )? _name__ctor ( (struct name *)0 ,
#line 600 "gram.y"
(_au1_yypvt [- 0 ]). __C10_pn -> _expr__O3.__C3_string ) : (_au1_yypvt [- 0 ]). __C10_pn );
yyval . __C10_pn -> _name_n_oper = 162 ;
}

#line 602 "gram.y"
break ;
case 65 : 
#line 604 "gram.y"
{ yyval . __C10_p = (struct node *)_name__ctor ( (struct name *)0 , oper_name ( (_au1_yypvt [- 0 ]). __C10_t ) )
#line 604 "gram.y"
;
yyval . __C10_pn -> _name_n_oper = (_au1_yypvt [- 0 ]). __C10_t ;
}

#line 606 "gram.y"
break ;
case 66 : 
#line 608 "gram.y"
{ Pname _au3_n ;

#line 608 "gram.y"
_au3_n = (_au1_yypvt [- 0 ]). __C10_pn ;
_au3_n -> _expr__O3.__C3_string = "_type";
_au3_n -> _name_n_oper = 97 ;
_au3_n -> _expr__O4.__C4_n_initializer = (((struct expr *)_au3_n -> _expr__O2.__C2_tp ));
_au3_n -> _expr__O2.__C2_tp = 0 ;
yyval . __C10_p = (struct node *)_au3_n ;
}

#line 614 "gram.y"
break ;
case 79 : 
#line 629 "gram.y"
{ yyval . __C10_t = 109 ;
}

#line 629 "gram.y"
break ;
case 80 : { yyval . __C10_t = 111 ;
}

#line 630 "gram.y"
break ;
case 86 : 
#line 636 "gram.y"
{ yyval . __C10_t = 23 ;
}

#line 636 "gram.y"
break ;
case 87 : { yyval . __C10_t = 9 ;
}

#line 637 "gram.y"
break ;
case 88 : { yyval . __C10_t = 44 ;
}

#line 638 "gram.y"
break ;
case 89 : { yyval . __C10_t = 45 ;
}

#line 639 "gram.y"
break ;
case 90 : 
#line 643 "gram.y"
{ 
#line 644 "gram.y"
if ((_au1_yypvt [-1]). __C10_pn -> _expr__O2.__C2_tp -> _node_base != 119 )
#line 645 "gram.y"
{ 
#line 1237 "gram.y"
struct ea _au0__V18 ;

#line 645 "gram.y"
error ( (char *)"T of%n not aC", (struct ea *)( ( ((& _au0__V18 )-> _ea__O1.__C1_p = ((char *)(_au1_yypvt [-1]). __C10_pn )), (((& _au0__V18 ))))
#line 645 "gram.y"
) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }

#line 646 "gram.y"
break ;
case 91 : 
#line 648 "gram.y"
{ 
#line 649 "gram.y"
if ((_au1_yypvt [-1]). __C10_pn -> _expr__O2.__C2_tp -> _node_base != 119 )
#line 650 "gram.y"
{ 
#line 1237 "gram.y"
struct ea _au0__V19 ;

#line 650 "gram.y"
error ( (char *)"T of%n not aC", (struct ea *)( ( ((& _au0__V19 )-> _ea__O1.__C1_p = ((char *)(_au1_yypvt [-1]). __C10_pn )), (((& _au0__V19 ))))
#line 650 "gram.y"
) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }

#line 651 "gram.y"
break ;
case 92 : { error ( (char *)"CNs do not nest", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 )
#line 652 "gram.y"
;
}

#line 652 "gram.y"
break ;
case 93 : { error ( (char *)"CNs do not nest", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 )
#line 653 "gram.y"
;
}

#line 653 "gram.y"
break ;
case 94 : 
#line 658 "gram.y"
{ (((struct fct *)(_au1_yypvt [- 0 ]). __C10_p ))-> _fct_returns = (_au1_yypvt [-1]). __C10_pn -> _expr__O2.__C2_tp ;
(_au1_yypvt [-1]). __C10_pn -> _expr__O2.__C2_tp = (_au1_yypvt [- 0 ]). __C10_pt ;
}

#line 660 "gram.y"
break ;
case 95 : 
#line 662 "gram.y"
{ Pname _au3_n ;

#line 662 "gram.y"
_au3_n = (_au1_yypvt [-1]). __C10_pn ;
yyval . __C10_p = (struct node *)((_au3_n -> _node_base == 123 )? _name__ctor ( (struct name *)0 , _au3_n -> _expr__O3.__C3_string ) : _au3_n );
if (ccl && strcmp ( (char *)_au3_n -> _expr__O3.__C3_string , (char *)ccl -> _classdef_string ) )_name_hide ( _au3_n ) ;
yyval . __C10_pn -> _name_n_oper = 123 ;
(((struct fct *)(_au1_yypvt [- 0 ]). __C10_p ))-> _fct_returns = yyval . __C10_pn -> _expr__O2.__C2_tp ;
yyval . __C10_pn -> _expr__O2.__C2_tp = (_au1_yypvt [- 0 ]). __C10_pt ;
}

#line 668 "gram.y"
break ;
case 96 : 
#line 674 "gram.y"
{ TOK _au3_k ;
Pname _au3_l ;

#line 674 "gram.y"
_au3_k = 1 ;
_au3_l = (_au1_yypvt [-1]). __C10_pn ;
if (fct_void && (_au3_l == 0 ))_au3_k = 0 ;
(_au1_yypvt [-3]). __C10_pn -> _expr__O2.__C2_tp = (struct type *)_fct__ctor ( (struct fct *)0 , (_au1_yypvt [-3]). __C10_pn -> _expr__O2.__C2_tp , _au3_l , _au3_k ) ;
}

#line 678 "gram.y"
break ;
case 97 : 
#line 680 "gram.y"
{ TOK _au3_k ;
Pname _au3_l ;

#line 680 "gram.y"
_au3_k = 1 ;
_au3_l = (_au1_yypvt [-1]). __C10_pn ;
if (fct_void && (_au3_l == 0 ))_au3_k = 0 ;
yyval . __C10_p = (struct node *)(((_au1_yypvt [-3]). __C10_pn -> _node_base == 123 )? _name__ctor ( (struct name *)0 , (_au1_yypvt [-3]). __C10_pn -> _expr__O3.__C3_string ) :
#line 683 "gram.y"
(_au1_yypvt [-3]). __C10_pn );
yyval . __C10_pn -> _name_n_oper = 123 ;
yyval . __C10_pn -> _expr__O2.__C2_tp = (struct type *)_fct__ctor ( (struct fct *)0 , (struct type *)0 , _au3_l , _au3_k ) ;
}

#line 686 "gram.y"
break ;
case 98 : 
#line 688 "gram.y"
{ memptrdcl ( (_au1_yypvt [-3]). __C10_pn , (_au1_yypvt [-5]). __C10_pn , (_au1_yypvt [- 0 ]). __C10_pt , (_au1_yypvt [-2]). __C10_pn ) ;
#line 688 "gram.y"

#line 689 "gram.y"
yyval . __C10_p = (_au1_yypvt [-2]). __C10_p ;
}

#line 690 "gram.y"
break ;
case 100 : 
#line 693 "gram.y"
{ yyval . __C10_p = (struct node *)(((_au1_yypvt [- 0 ]). __C10_pn -> _node_base == 123 )? _name__ctor ( (struct name *)0 ,
#line 693 "gram.y"
(_au1_yypvt [- 0 ]). __C10_pn -> _expr__O3.__C3_string ) : (_au1_yypvt [- 0 ]). __C10_pn );
yyval . __C10_pn -> _name__O6.__C6_n_qualifier = (_au1_yypvt [-2]). __C10_pn ;
}

#line 695 "gram.y"
break ;
case 101 : 
#line 697 "gram.y"
{ yyval . __C10_p = (_au1_yypvt [- 0 ]). __C10_p ;
set_scope ( (_au1_yypvt [-1]). __C10_pn ) ;
yyval . __C10_pn -> _name__O6.__C6_n_qualifier = (_au1_yypvt [-1]). __C10_pn ;
}

#line 700 "gram.y"
break ;
case 102 : 
#line 702 "gram.y"
{ yyval . __C10_p = (struct node *)(((_au1_yypvt [- 0 ]). __C10_pn -> _node_base == 123 )? _name__ctor ( (struct name *)0 ,
#line 702 "gram.y"
(_au1_yypvt [- 0 ]). __C10_pn -> _expr__O3.__C3_string ) : (_au1_yypvt [- 0 ]). __C10_pn );
set_scope ( (_au1_yypvt [-1]). __C10_pn ) ;
yyval . __C10_pn -> _name_n_oper = 123 ;
yyval . __C10_pn -> _name__O6.__C6_n_qualifier = (_au1_yypvt [-1]). __C10_pn ;
}

#line 706 "gram.y"
break ;
case 103 : 
#line 708 "gram.y"
{ (((struct ptr *)(_au1_yypvt [-1]). __C10_p ))-> _pvtyp_typ = (_au1_yypvt [- 0 ]). __C10_pn -> _expr__O2.__C2_tp ;
(_au1_yypvt [- 0 ]). __C10_pn -> _expr__O2.__C2_tp = (_au1_yypvt [-1]). __C10_pt ;
yyval . __C10_p = (_au1_yypvt [- 0 ]). __C10_p ;
}

#line 711 "gram.y"
break ;
case 104 : 
#line 713 "gram.y"
{ yyval . __C10_p = (struct node *)(((_au1_yypvt [- 0 ]). __C10_pn -> _node_base == 123 )? _name__ctor ( (struct name *)0 ,
#line 713 "gram.y"
(_au1_yypvt [- 0 ]). __C10_pn -> _expr__O3.__C3_string ) : (_au1_yypvt [- 0 ]). __C10_pn );
yyval . __C10_pn -> _name_n_oper = 123 ;
_name_hide ( (_au1_yypvt [- 0 ]). __C10_pn ) ;
yyval . __C10_pn -> _expr__O2.__C2_tp = (_au1_yypvt [-1]). __C10_pt ;
}

#line 717 "gram.y"
break ;
case 105 : 
#line 719 "gram.y"
{ yyval . __C10_p = (struct node *)(((_au1_yypvt [-1]). __C10_pn -> _node_base == 123 )? _name__ctor ( (struct name *)0 , (_au1_yypvt [-1]).
#line 719 "gram.y"
__C10_pn -> _expr__O3.__C3_string ) : (_au1_yypvt [-1]). __C10_pn );
yyval . __C10_pn -> _name_n_oper = 123 ;
_name_hide ( (_au1_yypvt [-1]). __C10_pn ) ;
yyval . __C10_pn -> _expr__O2.__C2_tp = (_au1_yypvt [- 0 ]). __C10_pt ;
}

#line 723 "gram.y"
break ;
case 106 : 
#line 725 "gram.y"
{ (((struct vec *)(_au1_yypvt [- 0 ]). __C10_p ))-> _pvtyp_typ = (_au1_yypvt [-1]). __C10_pn -> _expr__O2.__C2_tp ;
(_au1_yypvt [-1]). __C10_pn -> _expr__O2.__C2_tp = (_au1_yypvt [- 0 ]). __C10_pt ;
}

#line 727 "gram.y"
break ;
case 107 : 
#line 729 "gram.y"
{ (((struct fct *)(_au1_yypvt [- 0 ]). __C10_p ))-> _fct_returns = (_au1_yypvt [-2]). __C10_pn -> _expr__O2.__C2_tp ;
(_au1_yypvt [-2]). __C10_pn -> _expr__O2.__C2_tp = (_au1_yypvt [- 0 ]). __C10_pt ;
yyval . __C10_p = (_au1_yypvt [-2]). __C10_p ;
}

#line 732 "gram.y"
break ;
case 108 : 
#line 734 "gram.y"
{ (((struct vec *)(_au1_yypvt [- 0 ]). __C10_p ))-> _pvtyp_typ = (_au1_yypvt [-2]). __C10_pn -> _expr__O2.__C2_tp ;
(_au1_yypvt [-2]). __C10_pn -> _expr__O2.__C2_tp = (_au1_yypvt [- 0 ]). __C10_pt ;
yyval . __C10_p = (_au1_yypvt [-2]). __C10_p ;
}

#line 737 "gram.y"
break ;
case 109 : 
#line 741 "gram.y"
{ yyval . __C10_p = (struct node *)(_au1_yypvt [- 0 ]). __C10_pn ;
}

#line 741 "gram.y"
break ;
case 110 : 
#line 743 "gram.y"
{ yyval . __C10_p = (struct node *)(((_au1_yypvt [- 0 ]). __C10_pn -> _node_base == 123 )? _name__ctor ( (struct name *)0 ,
#line 743 "gram.y"
(_au1_yypvt [- 0 ]). __C10_pn -> _expr__O3.__C3_string ) : (_au1_yypvt [- 0 ]). __C10_pn );
yyval . __C10_pn -> _name_n_oper = 123 ;
_name_hide ( (_au1_yypvt [- 0 ]). __C10_pn ) ;
yyval . __C10_pn -> _expr__O2.__C2_tp = (_au1_yypvt [-1]). __C10_pt ;
}

#line 747 "gram.y"
break ;
case 111 : 
#line 749 "gram.y"
{ yyval . __C10_p = (struct node *)_name__ctor ( (struct name *)0 , (char *)0 ) ;
}

#line 749 "gram.y"
break ;
case 112 : 
#line 751 "gram.y"
{ (((struct ptr *)(_au1_yypvt [-1]). __C10_p ))-> _pvtyp_typ = (_au1_yypvt [- 0 ]). __C10_pn -> _expr__O2.__C2_tp ;
(_au1_yypvt [- 0 ]). __C10_pn -> _expr__O2.__C2_tp = (((struct type *)(_au1_yypvt [-1]). __C10_p ));
yyval . __C10_p = (_au1_yypvt [- 0 ]). __C10_p ;
}

#line 754 "gram.y"
break ;
case 113 : 
#line 756 "gram.y"
{ (((struct vec *)(_au1_yypvt [- 0 ]). __C10_p ))-> _pvtyp_typ = (_au1_yypvt [-1]). __C10_pn -> _expr__O2.__C2_tp ;
(_au1_yypvt [-1]). __C10_pn -> _expr__O2.__C2_tp = (((struct type *)(_au1_yypvt [- 0 ]). __C10_p ));
}

#line 758 "gram.y"
break ;
case 114 : 
#line 760 "gram.y"
{ (((struct fct *)(_au1_yypvt [- 0 ]). __C10_p ))-> _fct_returns = (_au1_yypvt [-2]). __C10_pn -> _expr__O2.__C2_tp ;
(_au1_yypvt [-2]). __C10_pn -> _expr__O2.__C2_tp = (((struct type *)(_au1_yypvt [- 0 ]). __C10_p ));
yyval . __C10_p = (_au1_yypvt [-2]). __C10_p ;
}

#line 763 "gram.y"
break ;
case 115 : 
#line 765 "gram.y"
{ (((struct vec *)(_au1_yypvt [- 0 ]). __C10_p ))-> _pvtyp_typ = (_au1_yypvt [-2]). __C10_pn -> _expr__O2.__C2_tp ;
(_au1_yypvt [-2]). __C10_pn -> _expr__O2.__C2_tp = (((struct type *)(_au1_yypvt [- 0 ]). __C10_p ));
yyval . __C10_p = (_au1_yypvt [-2]). __C10_p ;
}

#line 768 "gram.y"
break ;
case 116 : 
#line 772 "gram.y"
{ yyval . __C10_p = (struct node *)_name__ctor ( (struct name *)0 , (char *)0 ) ;
}

#line 772 "gram.y"
break ;
case 117 : 
#line 774 "gram.y"
{ (((struct ptr *)(_au1_yypvt [-1]). __C10_p ))-> _pvtyp_typ = (_au1_yypvt [- 0 ]). __C10_pn -> _expr__O2.__C2_tp ;
(_au1_yypvt [- 0 ]). __C10_pn -> _expr__O2.__C2_tp = (((struct type *)(_au1_yypvt [-1]). __C10_p ));
yyval . __C10_p = (_au1_yypvt [- 0 ]). __C10_p ;
}

#line 777 "gram.y"
break ;
case 118 : 
#line 779 "gram.y"
{ (((struct vec *)(_au1_yypvt [- 0 ]). __C10_p ))-> _pvtyp_typ = (_au1_yypvt [-1]). __C10_pn -> _expr__O2.__C2_tp ;
(_au1_yypvt [-1]). __C10_pn -> _expr__O2.__C2_tp = (((struct type *)(_au1_yypvt [- 0 ]). __C10_p ));
}

#line 781 "gram.y"
break ;
case 119 : 
#line 785 "gram.y"
{ yyval . __C10_p = (struct node *)_name__ctor ( (struct name *)0 , (char *)0 ) ;
}

#line 785 "gram.y"
break ;
case 120 : 
#line 787 "gram.y"
{ (((struct ptr *)(_au1_yypvt [-1]). __C10_p ))-> _pvtyp_typ = (_au1_yypvt [- 0 ]). __C10_pn -> _expr__O2.__C2_tp ;
(_au1_yypvt [- 0 ]). __C10_pn -> _expr__O2.__C2_tp = (((struct type *)(_au1_yypvt [-1]). __C10_p ));
yyval . __C10_p = (_au1_yypvt [- 0 ]). __C10_p ;
}

#line 790 "gram.y"
break ;
case 121 : 
#line 792 "gram.y"
{ (((struct vec *)(_au1_yypvt [- 0 ]). __C10_p ))-> _pvtyp_typ = (_au1_yypvt [-1]). __C10_pn -> _expr__O2.__C2_tp ;
(_au1_yypvt [-1]). __C10_pn -> _expr__O2.__C2_tp = (((struct type *)(_au1_yypvt [- 0 ]). __C10_p ));
}

#line 794 "gram.y"
break ;
case 122 : 
#line 796 "gram.y"
{ (((struct fct *)(_au1_yypvt [- 0 ]). __C10_p ))-> _fct_returns = (_au1_yypvt [-2]). __C10_pn -> _expr__O2.__C2_tp ;
(_au1_yypvt [-2]). __C10_pn -> _expr__O2.__C2_tp = (_au1_yypvt [- 0 ]). __C10_pt ;
yyval . __C10_p = (_au1_yypvt [-2]). __C10_p ;
}

#line 799 "gram.y"
break ;
case 123 : 
#line 801 "gram.y"
{ (((struct vec *)(_au1_yypvt [- 0 ]). __C10_p ))-> _pvtyp_typ = (_au1_yypvt [-2]). __C10_pn -> _expr__O2.__C2_tp ;
(_au1_yypvt [-2]). __C10_pn -> _expr__O2.__C2_tp = (_au1_yypvt [- 0 ]). __C10_pt ;
yyval . __C10_p = (_au1_yypvt [-2]). __C10_p ;
}

#line 804 "gram.y"
break ;
case 124 : 
#line 808 "gram.y"
{ yyval . __C10_p = (struct node *)_name__ctor ( (struct name *)0 , (char *)0 ) ;
}

#line 808 "gram.y"
break ;
case 125 : 
#line 810 "gram.y"
{ (((struct ptr *)(_au1_yypvt [-1]). __C10_p ))-> _pvtyp_typ = (_au1_yypvt [- 0 ]). __C10_pn -> _expr__O2.__C2_tp ;
(_au1_yypvt [- 0 ]). __C10_pn -> _expr__O2.__C2_tp = (((struct type *)(_au1_yypvt [-1]). __C10_p ));
yyval . __C10_p = (_au1_yypvt [- 0 ]). __C10_p ;
}

#line 813 "gram.y"
break ;
case 126 : 
#line 821 "gram.y"
{ if ((_au1_yypvt [- 0 ]). __C10_p )
#line 822 "gram.y"
if ((_au1_yypvt [-1]). __C10_p )
#line 823 "gram.y"
( ((_au1_yypvt [-1]). __C10_sl -> _slist_tail -> _stmt_s_list = (_au1_yypvt [-
#line 823 "gram.y"
0 ]). __C10_ps ), ((_au1_yypvt [-1]). __C10_sl -> _slist_tail = (_au1_yypvt [- 0 ]). __C10_ps )) ;
else { 
#line 825 "gram.y"
yyval . __C10_sl = (struct slist *)( (_au0__Xthis__ctor_slist = 0 ), ( (_au0__Xthis__ctor_slist = (struct slist *)_new ( (long )(sizeof
#line 825 "gram.y"
(struct slist))) ), ( (Nl ++ ), ( (_au0__Xthis__ctor_slist -> _slist_head = (_au0__Xthis__ctor_slist -> _slist_tail = (_au1_yypvt [- 0 ]). __C10_ps )), ((_au0__Xthis__ctor_slist )))
#line 825 "gram.y"
) ) ) ;
stmt_seen = 1 ;
}
}

#line 828 "gram.y"
break ;
case 127 : 
#line 830 "gram.y"
{ if ((_au1_yypvt [- 0 ]). __C10_p ){ 
#line 831 "gram.y"
yyval . __C10_sl = (struct slist *)( (_au0__Xthis__ctor_slist = 0 ), (
#line 831 "gram.y"
(_au0__Xthis__ctor_slist = (struct slist *)_new ( (long )(sizeof (struct slist))) ), ( (Nl ++ ), ( (_au0__Xthis__ctor_slist -> _slist_head = (_au0__Xthis__ctor_slist ->
#line 831 "gram.y"
_slist_tail = (_au1_yypvt [- 0 ]). __C10_ps )), ((_au0__Xthis__ctor_slist ))) ) ) ) ;
stmt_seen = 1 ;
}
}

#line 834 "gram.y"
break ;
case 128 : 
#line 838 "gram.y"
{ yyval . __C10_p = (_au1_yypvt [-1]). __C10_p ;
if (yyval . __C10_pe == dummy )error ( (char *)"empty condition", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 )
#line 839 "gram.y"
;
stmt_seen = 1 ;
}

#line 841 "gram.y"
break ;
case 129 : 
#line 845 "gram.y"
{ (cd_vec [cdi ])= cd ;
(stmt_vec [cdi ])= stmt_seen ;
(tn_vec [cdi ++ ])= modified_tn ;
cd = 0 ;
stmt_seen = 0 ;
modified_tn = 0 ;
}

#line 851 "gram.y"
break ;
case 130 : 
#line 853 "gram.y"
{ Pname _au3_n ;
Pstmt _au3_ss ;

#line 855 "gram.y"
struct block *_au0__Xthis__ctor_block ;

#line 853 "gram.y"
_au3_n = name_unlist ( cd ) ;
_au3_ss = stmt_unlist ( (_au1_yypvt [-1]). __C10_sl ) ;
yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_block = 0 ), ( ( (_au0__Xthis__ctor_block = 0 ), (_au0__Xthis__ctor_block = (struct block *)_stmt__ctor ( ((struct
#line 855 "gram.y"
stmt *)_au0__Xthis__ctor_block ), (unsigned char )116 , (_au1_yypvt [-3]). __C10_l , _au3_ss ) )) , ( (_au0__Xthis__ctor_block -> _stmt__O7.__C7_d = _au3_n ), ((_au0__Xthis__ctor_block )))
#line 855 "gram.y"
) ) ;
if (modified_tn )restore ( ) ;
cd = (cd_vec [-- cdi ]);
stmt_seen = (stmt_vec [cdi ]);
modified_tn = (tn_vec [cdi ]);
if (cdi < 0 ){ 
#line 1237 "gram.y"
struct ea _au0__V20 ;

#line 860 "gram.y"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"block level(%d)", (struct ea *)( ( ((& _au0__V20 )-> _ea__O1.__C1_i = cdi ), (((& _au0__V20 ))))
#line 860 "gram.y"
) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }

#line 861 "gram.y"
break ;
case 131 : 
#line 863 "gram.y"
{ yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_block = 0 ), ( ( (_au0__Xthis__ctor_block = 0 ), (_au0__Xthis__ctor_block =
#line 863 "gram.y"
(struct block *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_block ), (unsigned char )116 , (_au1_yypvt [-1]). __C10_l , ((struct stmt *)0 )) )) , (
#line 863 "gram.y"
(_au0__Xthis__ctor_block -> _stmt__O7.__C7_d = ((struct name *)0 )), ((_au0__Xthis__ctor_block ))) ) ) ;
}

#line 863 "gram.y"
break ;
case 132 : 
#line 865 "gram.y"
{ yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_block = 0 ), ( ( (_au0__Xthis__ctor_block = 0 ), (_au0__Xthis__ctor_block =
#line 865 "gram.y"
(struct block *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_block ), (unsigned char )116 , (_au1_yypvt [-2]). __C10_l , ((struct stmt *)0 )) )) , (
#line 865 "gram.y"
(_au0__Xthis__ctor_block -> _stmt__O7.__C7_d = ((struct name *)0 )), ((_au0__Xthis__ctor_block ))) ) ) ;
}

#line 865 "gram.y"
break ;
case 133 : 
#line 869 "gram.y"
{ yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt =
#line 869 "gram.y"
(struct estmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_estmt ), ((unsigned char )72 ), curloc , ((struct stmt *)0 )) )) , ( (_au0__Xthis__ctor_estmt ->
#line 869 "gram.y"
_stmt__O8.__C8_e = (_au1_yypvt [- 0 ]). __C10_pe ), ((_au0__Xthis__ctor_estmt ))) ) ) ;
}

#line 869 "gram.y"
break ;
case 134 : 
#line 871 "gram.y"
{ yyval . __C10_p = (struct node *)_stmt__ctor ( (struct stmt *)0 , (unsigned char )3 , (_au1_yypvt [- 0 ]).
#line 871 "gram.y"
__C10_l , (struct stmt *)0 ) ;
}

#line 871 "gram.y"
break ;
case 135 : 
#line 873 "gram.y"
{ yyval . __C10_p = (struct node *)_stmt__ctor ( (struct stmt *)0 , (unsigned char )7 , (_au1_yypvt [- 0 ]).
#line 873 "gram.y"
__C10_l , (struct stmt *)0 ) ;
}

#line 873 "gram.y"
break ;
case 136 : 
#line 875 "gram.y"
{ yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt =
#line 875 "gram.y"
(struct estmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_estmt ), ((unsigned char )28 ), (_au1_yypvt [-1]). __C10_l , ((struct stmt *)0 )) )) , (
#line 875 "gram.y"
(_au0__Xthis__ctor_estmt -> _stmt__O8.__C8_e = (_au1_yypvt [- 0 ]). __C10_pe ), ((_au0__Xthis__ctor_estmt ))) ) ) ;
}

#line 875 "gram.y"
break ;
case 137 : 
#line 877 "gram.y"
{ yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_lstmt = 0 ), ( ( (_au0__Xthis__ctor_lstmt = 0 ), (_au0__Xthis__ctor_lstmt =
#line 877 "gram.y"
(struct lstmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_lstmt ), ((unsigned char )19 ), (_au1_yypvt [-1]). __C10_l , ((struct stmt *)0 )) )) , (
#line 877 "gram.y"
(_au0__Xthis__ctor_lstmt -> _stmt__O7.__C7_d = (_au1_yypvt [- 0 ]). __C10_pn ), ((_au0__Xthis__ctor_lstmt ))) ) ) ;
}

#line 877 "gram.y"
break ;
case 138 : { stmt_seen = 1 ;
}

#line 878 "gram.y"
break ;
case 139 : { yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt =
#line 879 "gram.y"
(struct estmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_estmt ), ((unsigned char )10 ), (_au1_yypvt [-4]). __C10_l , (_au1_yypvt [-2]). __C10_ps ) )) , (
#line 879 "gram.y"
(_au0__Xthis__ctor_estmt -> _stmt__O8.__C8_e = (_au1_yypvt [- 0 ]). __C10_pe ), ((_au0__Xthis__ctor_estmt ))) ) ) ;
}

#line 879 "gram.y"
break ;
case 141 : 
#line 884 "gram.y"
{ 
#line 885 "gram.y"
if (stmt_seen )
#line 886 "gram.y"
yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ),
#line 886 "gram.y"
(_au0__Xthis__ctor_estmt = (struct estmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_estmt ), ((unsigned char )1 ), curloc , ((struct stmt *)0 )) )) , (
#line 886 "gram.y"
(_au0__Xthis__ctor_estmt -> _stmt__O8.__C8_e = (((struct expr *)(_au1_yypvt [-2]). __C10_s ))), ((_au0__Xthis__ctor_estmt ))) ) ) ;
else { 
#line 888 "gram.y"
Pname _au4_n ;

#line 888 "gram.y"
_au4_n = (struct name *)_name__ctor ( (struct name *)0 , make_name ( (unsigned char )'A' ) ) ;
_au4_n -> _expr__O2.__C2_tp = (struct type *)_basetype__ctor ( (struct basetype *)0 , (unsigned char )1 , ((struct name *)(_au1_yypvt [-2]). __C10_s )) ;
if (cd )
#line 891 "gram.y"
_nlist_add_list ( cd , _au4_n ) ;
else 
#line 893 "gram.y"
cd = (struct nlist *)_nlist__ctor ( (struct nlist *)0 , _au4_n ) ;
yyval . __C10_p = 0 ;
}
}

#line 896 "gram.y"
break ;
case 142 : 
#line 898 "gram.y"
{ Pname _au3_n ;

#line 899 "gram.y"
struct block *_au0__Xthis__ctor_block ;

#line 898 "gram.y"
_au3_n = (_au1_yypvt [- 0 ]). __C10_pn ;
if (_au3_n )
#line 900 "gram.y"
if (stmt_seen ){ 
#line 901 "gram.y"
yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_block = 0 ), ( ( (_au0__Xthis__ctor_block = 0 ), (_au0__Xthis__ctor_block =
#line 901 "gram.y"
(struct block *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_block ), (unsigned char )116 , _au3_n -> _name_where , ((struct stmt *)0 )) )) , (
#line 901 "gram.y"
(_au0__Xthis__ctor_block -> _stmt__O7.__C7_d = _au3_n ), ((_au0__Xthis__ctor_block ))) ) ) ;
yyval . __C10_ps -> _node_base = 118 ;
}
else { 
#line 905 "gram.y"
if (cd )
#line 906 "gram.y"
_nlist_add_list ( cd , _au3_n ) ;
else 
#line 908 "gram.y"
cd = (struct nlist *)_nlist__ctor ( (struct nlist *)0 , _au3_n ) ;
yyval . __C10_p = 0 ;
}
}

#line 911 "gram.y"
break ;
case 143 : 
#line 913 "gram.y"
{ Pname _au3_n ;

#line 913 "gram.y"
_au3_n = (_au1_yypvt [- 0 ]). __C10_pn ;
back = 74 ;
{ 
#line 1237 "gram.y"
struct ea _au0__V21 ;

#line 915 "gram.y"
errorFPCloc__PC_RCea__RCea__RCea__RCea___ ( & _au3_n -> _name_where , (char *)"%n's definition is nested (did you forget a ``}''?)", (struct ea *)( ( ((& _au0__V21 )-> _ea__O1.__C1_p = ((char *)_au3_n )),
#line 915 "gram.y"
(((& _au0__V21 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
if (cd )
#line 917 "gram.y"
_nlist_add_list ( cd , _au3_n ) ;
else 
#line 919 "gram.y"
cd = (struct nlist *)_nlist__ctor ( (struct nlist *)0 , _au3_n ) ;
yyval . __C10_p = 0 ;
} }

#line 921 "gram.y"
break ;
case 145 : 
#line 924 "gram.y"
{ yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_ifstmt = 0 ), ( ( (_au0__Xthis__ctor_ifstmt = 0 ), (_au0__Xthis__ctor_ifstmt =
#line 924 "gram.y"
(struct ifstmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_ifstmt ), (unsigned char )20 , (_au1_yypvt [-2]). __C10_l , (_au1_yypvt [- 0 ]). __C10_ps ) )) ,
#line 924 "gram.y"
( (_au0__Xthis__ctor_ifstmt -> _stmt__O8.__C8_e = (_au1_yypvt [-1]). __C10_pe ), ( (_au0__Xthis__ctor_ifstmt -> _stmt__O9.__C9_else_stmt = ((struct stmt *)0 )), ((_au0__Xthis__ctor_ifstmt ))) ) ) )
#line 924 "gram.y"
;
}

#line 924 "gram.y"
break ;
case 146 : 
#line 926 "gram.y"
{ yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_ifstmt = 0 ), ( ( (_au0__Xthis__ctor_ifstmt = 0 ), (_au0__Xthis__ctor_ifstmt =
#line 926 "gram.y"
(struct ifstmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_ifstmt ), (unsigned char )20 , (_au1_yypvt [-4]). __C10_l , (_au1_yypvt [-2]). __C10_ps ) )) , (
#line 926 "gram.y"
(_au0__Xthis__ctor_ifstmt -> _stmt__O8.__C8_e = (_au1_yypvt [-3]). __C10_pe ), ( (_au0__Xthis__ctor_ifstmt -> _stmt__O9.__C9_else_stmt = (_au1_yypvt [- 0 ]). __C10_ps ), ((_au0__Xthis__ctor_ifstmt ))) ) ) )
#line 926 "gram.y"
;
}

#line 926 "gram.y"
break ;
case 147 : 
#line 928 "gram.y"
{ yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt =
#line 928 "gram.y"
(struct estmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_estmt ), ((unsigned char )39 ), (_au1_yypvt [-2]). __C10_l , (_au1_yypvt [- 0 ]). __C10_ps ) )) ,
#line 928 "gram.y"
( (_au0__Xthis__ctor_estmt -> _stmt__O8.__C8_e = (_au1_yypvt [-1]). __C10_pe ), ((_au0__Xthis__ctor_estmt ))) ) ) ;
}

#line 928 "gram.y"
break ;
case 148 : { stmt_seen = 1 ;
}

#line 929 "gram.y"
break ;
case 149 : { yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_forstmt = 0 ), ( ( (_au0__Xthis__ctor_forstmt = 0 ), (_au0__Xthis__ctor_forstmt =
#line 930 "gram.y"
(struct forstmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_forstmt ), (unsigned char )16 , (_au1_yypvt [-8]). __C10_l , (_au1_yypvt [- 0 ]). __C10_ps ) )) ,
#line 930 "gram.y"
( (_au0__Xthis__ctor_forstmt -> _stmt__O9.__C9_for_init = (_au1_yypvt [-5]). __C10_ps ), ( (_au0__Xthis__ctor_forstmt -> _stmt__O8.__C8_e = (_au1_yypvt [-4]). __C10_pe ), ( (_au0__Xthis__ctor_forstmt -> _stmt__O7.__C7_e2 = (_au1_yypvt [-2]).
#line 930 "gram.y"
__C10_pe ), ((_au0__Xthis__ctor_forstmt ))) ) ) ) ) ;
}

#line 930 "gram.y"
break ;
case 150 : 
#line 932 "gram.y"
{ yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt =
#line 932 "gram.y"
(struct estmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_estmt ), ((unsigned char )33 ), (_au1_yypvt [-2]). __C10_l , (_au1_yypvt [- 0 ]). __C10_ps ) )) ,
#line 932 "gram.y"
( (_au0__Xthis__ctor_estmt -> _stmt__O8.__C8_e = (_au1_yypvt [-1]). __C10_pe ), ((_au0__Xthis__ctor_estmt ))) ) ) ;
}

#line 932 "gram.y"
break ;
case 151 : { yyval . __C10_p = (struct node *)(_au1_yypvt [-1]). __C10_pn ;

#line 933 "gram.y"
stmt_seen = 1 ;
}

#line 933 "gram.y"
break ;
case 152 : { Pname _au3_n ;

#line 935 "gram.y"
struct lstmt *_au0__Xthis__ctor_lstmt ;

#line 934 "gram.y"
_au3_n = (_au1_yypvt [-1]). __C10_pn ;
yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_lstmt = 0 ), ( ( (_au0__Xthis__ctor_lstmt = 0 ), (_au0__Xthis__ctor_lstmt = (struct lstmt *)_stmt__ctor ( ((struct
#line 935 "gram.y"
stmt *)_au0__Xthis__ctor_lstmt ), ((unsigned char )115 ), _au3_n -> _name_where , (_au1_yypvt [- 0 ]). __C10_ps ) )) , ( (_au0__Xthis__ctor_lstmt -> _stmt__O7.__C7_d =
#line 935 "gram.y"
_au3_n ), ((_au0__Xthis__ctor_lstmt ))) ) ) ;
}

#line 936 "gram.y"
break ;
case 153 : { stmt_seen = 1 ;
}

#line 937 "gram.y"
break ;
case 154 : { if ((_au1_yypvt [-2]). __C10_pe == dummy )error ( (char *)"empty case label", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 938 "gram.y"
ea *)ea0 , (struct ea *)ea0 ) ;
yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt = (struct estmt *)_stmt__ctor ( ((struct
#line 939 "gram.y"
stmt *)_au0__Xthis__ctor_estmt ), ((unsigned char )4 ), (_au1_yypvt [-4]). __C10_l , (_au1_yypvt [- 0 ]). __C10_ps ) )) , ( (_au0__Xthis__ctor_estmt -> _stmt__O8.__C8_e =
#line 939 "gram.y"
(_au1_yypvt [-2]). __C10_pe ), ((_au0__Xthis__ctor_estmt ))) ) ) ;
}

#line 940 "gram.y"
break ;
case 155 : { stmt_seen = 1 ;
}

#line 941 "gram.y"
break ;
case 156 : { yyval . __C10_p = (struct node *)_stmt__ctor ( (struct stmt *)0 , (unsigned char )8 , (_au1_yypvt [-3]). __C10_l ,
#line 942 "gram.y"
(_au1_yypvt [- 0 ]). __C10_ps ) ;
}

#line 942 "gram.y"
break ;
case 157 : 
#line 951 "gram.y"
{ Pexpr _au3_e ;

#line 951 "gram.y"
_au3_e = expr_unlist ( (_au1_yypvt [- 0 ]). __C10_el ) ;
while (_au3_e && (_au3_e -> _expr__O3.__C3_e1 == dummy )){ 
#line 953 "gram.y"
register Pexpr _au4_ee2 ;

#line 953 "gram.y"
_au4_ee2 = _au3_e -> _expr__O4.__C4_e2 ;
if (_au4_ee2 )error ( (char *)"EX inEL", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
_expr__dtor ( _au3_e , 1) ;
_au3_e = _au4_ee2 ;
}
yyval . __C10_p = (struct node *)_au3_e ;
}

#line 959 "gram.y"
break ;
case 158 : 
#line 962 "gram.y"
{ yyval . __C10_el = (struct elist *)( (_au0__Xthis__ctor_elist = 0 ), ( (_au1__Xe__ctor_elist = (struct expr *)_expr__ctor ( (struct
#line 962 "gram.y"
expr *)0 , (unsigned char )140 , (_au1_yypvt [- 0 ]). __C10_pe , (struct expr *)0 ) ), ( (_au0__Xthis__ctor_elist = (struct elist *)_new (
#line 962 "gram.y"
(long )(sizeof (struct elist))) ), ( (Nl ++ ), ( (_au0__Xthis__ctor_elist -> _elist_head = (_au0__Xthis__ctor_elist -> _elist_tail = _au1__Xe__ctor_elist )), ((_au0__Xthis__ctor_elist )))
#line 962 "gram.y"
) ) ) ) ;
}

#line 962 "gram.y"
break ;
case 159 : 
#line 964 "gram.y"
{ ( (_au1__Xe_add_elist = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , (_au1_yypvt [- 0 ]).
#line 964 "gram.y"
__C10_pe , (struct expr *)0 ) ), ( ((_au1_yypvt [-2]). __C10_el -> _elist_tail -> _expr__O4.__C4_e2 = _au1__Xe_add_elist ), ((_au1_yypvt [-2]). __C10_el -> _elist_tail = _au1__Xe_add_elist ))
#line 964 "gram.y"
) ;
}

#line 964 "gram.y"
break ;
case 161 : 
#line 969 "gram.y"
{ Pexpr _au3_e ;
if ((_au1_yypvt [-1]). __C10_p )
#line 971 "gram.y"
_au3_e = (_au1_yypvt [-1]). __C10_pe ;
else 
#line 973 "gram.y"
_au3_e = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , dummy , (struct expr *)0 ) ;
yyval . __C10_p = (struct node *)_expr__ctor ( (struct expr *)0 , (unsigned char )124 , _au3_e , (struct expr *)0 ) ;
}

#line 975 "gram.y"
break ;
case 162 : 
#line 981 "gram.y"
{ binop :yyval . __C10_p = (struct node *)_expr__ctor ( (struct expr *)0 , (_au1_yypvt [-1]). __C10_t , (_au1_yypvt [-2]). __C10_pe , (_au1_yypvt [-
#line 981 "gram.y"
0 ]). __C10_pe ) ;
}

#line 981 "gram.y"
break ;
case 163 : { goto binop ;
}

#line 982 "gram.y"
break ;
case 164 : { goto binop ;
}

#line 983 "gram.y"
break ;
case 165 : { goto binop ;
}

#line 984 "gram.y"
break ;
case 166 : { goto binop ;
}

#line 985 "gram.y"
break ;
case 167 : { goto binop ;
}

#line 986 "gram.y"
break ;
case 168 : { goto binop ;
}

#line 987 "gram.y"
break ;
case 169 : { goto binop ;
}

#line 988 "gram.y"
break ;
case 170 : { goto binop ;
}

#line 989 "gram.y"
break ;
case 171 : { goto binop ;
}

#line 990 "gram.y"
break ;
case 172 : { goto binop ;
}

#line 991 "gram.y"
break ;
case 173 : { goto binop ;
}

#line 992 "gram.y"
break ;
case 174 : { goto binop ;
}

#line 993 "gram.y"
break ;
case 175 : { goto binop ;
}

#line 994 "gram.y"
break ;
case 176 : { goto binop ;
}

#line 995 "gram.y"
break ;
case 177 : 
#line 997 "gram.y"
{ yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_qexpr = 0 ), ( ( (_au0__Xthis__ctor_qexpr = 0 ), (_au0__Xthis__ctor_qexpr =
#line 997 "gram.y"
(struct qexpr *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_qexpr ), (unsigned char )68 , (_au1_yypvt [-2]). __C10_pe , (_au1_yypvt [- 0 ]). __C10_pe ) )) ,
#line 997 "gram.y"
( (_au0__Xthis__ctor_qexpr -> _expr__O5.__C5_cond = (_au1_yypvt [-4]). __C10_pe ), ((_au0__Xthis__ctor_qexpr ))) ) ) ;
}

#line 997 "gram.y"
break ;
case 178 : 
#line 999 "gram.y"
{ yyval . __C10_p = (struct node *)_expr__ctor ( (struct expr *)0 , (unsigned char )9 , (_au1_yypvt [- 0 ]).
#line 999 "gram.y"
__C10_pe , (struct expr *)0 ) ;
}

#line 999 "gram.y"
break ;
case 179 : 
#line 1001 "gram.y"
{ yyval . __C10_p = (struct node *)_expr__ctor ( (struct expr *)0 , (unsigned char )9 , (_au1_yypvt [- 0 ]).
#line 1001 "gram.y"
__C10_pe , (_au1_yypvt [-2]). __C10_pe ) ;
}

#line 1001 "gram.y"
break ;
case 181 : 
#line 1004 "gram.y"
{ yyval . __C10_p = (struct node *)dummy ;
}

#line 1004 "gram.y"
break ;
case 182 : 
#line 1008 "gram.y"
{ yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_texpr = 0 ), ( (_au1__Xtt__ctor_texpr = tok_to_type ( (_au1_yypvt [-3]). __C10_t )
#line 1008 "gram.y"
), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr = (struct texpr *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_texpr ), ((unsigned char )157 ), (_au1_yypvt [-1]).
#line 1008 "gram.y"
__C10_pe , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 = _au1__Xtt__ctor_texpr ), ((_au0__Xthis__ctor_texpr ))) ) ) ) ;
#line 1008 "gram.y"
}

#line 1008 "gram.y"
break ;
case 183 : 
#line 1010 "gram.y"
{ yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr =
#line 1010 "gram.y"
(struct texpr *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_texpr ), ((unsigned char )157 ), (_au1_yypvt [-1]). __C10_pe , (struct expr *)0 ) )) , (
#line 1010 "gram.y"
(_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 = (_au1_yypvt [-3]). __C10_pn -> _expr__O2.__C2_tp ), ((_au0__Xthis__ctor_texpr ))) ) ) ;
}

#line 1010 "gram.y"
break ;
case 184 : 
#line 1012 "gram.y"
{ Ptype _au3_t ;

#line 1013 "gram.y"
struct texpr *_au0__Xthis__ctor_texpr ;

#line 1012 "gram.y"
_au3_t = (_au1_yypvt [- 0 ]). __C10_pn -> _expr__O2.__C2_tp ;

#line 1012 "gram.y"
yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr = (struct texpr *)_expr__ctor ( ((struct
#line 1012 "gram.y"
expr *)_au0__Xthis__ctor_texpr ), ((unsigned char )23 ), ((struct expr *)0 ), (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 = _au3_t ),
#line 1012 "gram.y"
((_au0__Xthis__ctor_texpr ))) ) ) ;
}

#line 1012 "gram.y"
break ;
case 185 : 
#line 1014 "gram.y"
{ Ptype _au3_t ;

#line 1015 "gram.y"
struct texpr *_au0__Xthis__ctor_texpr ;

#line 1014 "gram.y"
_au3_t = (_au1_yypvt [-1]). __C10_pn -> _expr__O2.__C2_tp ;

#line 1014 "gram.y"
yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr = (struct texpr *)_expr__ctor ( ((struct
#line 1014 "gram.y"
expr *)_au0__Xthis__ctor_texpr ), ((unsigned char )23 ), ((struct expr *)0 ), (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 = _au3_t ),
#line 1014 "gram.y"
((_au0__Xthis__ctor_texpr ))) ) ) ;
}

#line 1014 "gram.y"
break ;
case 186 : 
#line 1016 "gram.y"
{ yyval . __C10_p = (struct node *)_expr__ctor ( (struct expr *)0 , (_au1_yypvt [- 0 ]). __C10_t , (_au1_yypvt [-1]). __C10_pe ,
#line 1016 "gram.y"
(struct expr *)0 ) ;
}

#line 1016 "gram.y"
break ;
case 187 : 
#line 1018 "gram.y"
{ yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr =
#line 1018 "gram.y"
(struct texpr *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_texpr ), ((unsigned char )113 ), (_au1_yypvt [- 0 ]). __C10_pe , (struct expr *)0 ) )) ,
#line 1018 "gram.y"
( (_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 = (_au1_yypvt [-2]). __C10_pn -> _expr__O2.__C2_tp ), ((_au0__Xthis__ctor_texpr ))) ) ) ;
}

#line 1018 "gram.y"
break ;
case 188 : 
#line 1020 "gram.y"
{ yyval . __C10_p = (struct node *)_expr__ctor ( (struct expr *)0 , (unsigned char )111 , (_au1_yypvt [- 0 ]).
#line 1020 "gram.y"
__C10_pe , (struct expr *)0 ) ;
}

#line 1020 "gram.y"
break ;
case 189 : 
#line 1022 "gram.y"
{ yyval . __C10_p = (struct node *)_expr__ctor ( (struct expr *)0 , (unsigned char )112 , (struct expr *)0 ,
#line 1022 "gram.y"
(_au1_yypvt [- 0 ]). __C10_pe ) ;
}

#line 1022 "gram.y"
break ;
case 190 : 
#line 1024 "gram.y"
{ yyval . __C10_p = (struct node *)_expr__ctor ( (struct expr *)0 , (unsigned char )107 , (struct expr *)0 ,
#line 1024 "gram.y"
(_au1_yypvt [- 0 ]). __C10_pe ) ;
}

#line 1024 "gram.y"
break ;
case 191 : 
#line 1026 "gram.y"
{ yyval . __C10_p = (struct node *)_expr__ctor ( (struct expr *)0 , (unsigned char )172 , (struct expr *)0 ,
#line 1026 "gram.y"
(_au1_yypvt [- 0 ]). __C10_pe ) ;
}

#line 1026 "gram.y"
break ;
case 192 : 
#line 1028 "gram.y"
{ yyval . __C10_p = (struct node *)_expr__ctor ( (struct expr *)0 , (unsigned char )46 , (struct expr *)0 ,
#line 1028 "gram.y"
(_au1_yypvt [- 0 ]). __C10_pe ) ;
}

#line 1028 "gram.y"
break ;
case 193 : 
#line 1030 "gram.y"
{ yyval . __C10_p = (struct node *)_expr__ctor ( (struct expr *)0 , (unsigned char )47 , (struct expr *)0 ,
#line 1030 "gram.y"
(_au1_yypvt [- 0 ]). __C10_pe ) ;
}

#line 1030 "gram.y"
break ;
case 194 : 
#line 1032 "gram.y"
{ yyval . __C10_p = (struct node *)_expr__ctor ( (struct expr *)0 , (_au1_yypvt [-1]). __C10_t , (struct expr *)0 , (_au1_yypvt [-
#line 1032 "gram.y"
0 ]). __C10_pe ) ;
}

#line 1032 "gram.y"
break ;
case 195 : 
#line 1034 "gram.y"
{ yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr =
#line 1034 "gram.y"
(struct texpr *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_texpr ), ((unsigned char )30 ), (_au1_yypvt [- 0 ]). __C10_pe , (struct expr *)0 ) )) ,
#line 1034 "gram.y"
( (_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 = ((struct type *)0 )), ((_au0__Xthis__ctor_texpr ))) ) ) ;
}

#line 1034 "gram.y"
break ;
case 196 : 
#line 1036 "gram.y"
{ yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr =
#line 1036 "gram.y"
(struct texpr *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_texpr ), ((unsigned char )30 ), ((struct expr *)0 ), (struct expr *)0 ) )) , (
#line 1036 "gram.y"
(_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 = (_au1_yypvt [-1]). __C10_pn -> _expr__O2.__C2_tp ), ((_au0__Xthis__ctor_texpr ))) ) ) ;
}

#line 1036 "gram.y"
break ;
case 197 : 
#line 1038 "gram.y"
{ yyval . __C10_p = (struct node *)_expr__ctor ( (struct expr *)0 , (unsigned char )111 , (_au1_yypvt [-3]). __C10_pe ,
#line 1038 "gram.y"
(_au1_yypvt [-1]). __C10_pe ) ;
}

#line 1038 "gram.y"
break ;
case 198 : 
#line 1043 "gram.y"
{ Pexpr _au3_ee ;
Pexpr _au3_e ;

#line 1045 "gram.y"
struct call *_au0__Xthis__ctor_call ;

#line 1043 "gram.y"
_au3_ee = (_au1_yypvt [-1]). __C10_pe ;
_au3_e = (_au1_yypvt [-3]). __C10_pe ;
if (_au3_e -> _node_base == 23 )
#line 1046 "gram.y"
_au3_e -> _expr__O3.__C3_e1 = _au3_ee ;
else 
#line 1048 "gram.y"
yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_call = 0 ), ( ( (_au0__Xthis__ctor_call = 0 ), (_au0__Xthis__ctor_call = (struct call *)_expr__ctor (
#line 1048 "gram.y"
((struct expr *)_au0__Xthis__ctor_call ), (unsigned char )109 , _au3_e , _au3_ee ) )) , ((_au0__Xthis__ctor_call ))) ) ;
}

#line 1049 "gram.y"
break ;
case 199 : 
#line 1051 "gram.y"
{ yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_ref = 0 ), ( ( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref =
#line 1051 "gram.y"
(struct ref *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ref ), ((unsigned char )44 ), (_au1_yypvt [-2]). __C10_pe , (struct expr *)0 ) )) , (
#line 1051 "gram.y"
(_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = (_au1_yypvt [- 0 ]). __C10_pn ), ((_au0__Xthis__ctor_ref ))) ) ) ;
}

#line 1051 "gram.y"
break ;
case 200 : 
#line 1053 "gram.y"
{ yyval . __C10_p = (struct node *)_expr__ctor ( (struct expr *)0 , (unsigned char )44 , (_au1_yypvt [-3]). __C10_pe ,
#line 1053 "gram.y"
(_au1_yypvt [- 0 ]). __C10_pe ) ;
}

#line 1053 "gram.y"
break ;
case 201 : 
#line 1055 "gram.y"
{ yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_ref = 0 ), ( (_au1__Xb__ctor_ref = (((_au1_yypvt [- 0 ]). __C10_pn ->
#line 1055 "gram.y"
_node_base == 123 )? _name__ctor ( (struct name *)0 , (_au1_yypvt [- 0 ]). __C10_pn -> _expr__O3.__C3_string ) : (_au1_yypvt [- 0 ]). __C10_pn )), (
#line 1055 "gram.y"
( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ref ), ((unsigned char )44 ), (_au1_yypvt [-2]). __C10_pe , (struct
#line 1055 "gram.y"
expr *)0 ) )) , ( (_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = _au1__Xb__ctor_ref ), ((_au0__Xthis__ctor_ref ))) ) ) ) ;
}

#line 1055 "gram.y"
break ;
case 202 : 
#line 1057 "gram.y"
{ yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_ref = 0 ), ( ( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref =
#line 1057 "gram.y"
(struct ref *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ref ), ((unsigned char )45 ), (_au1_yypvt [-2]). __C10_pe , (struct expr *)0 ) )) , (
#line 1057 "gram.y"
(_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = (_au1_yypvt [- 0 ]). __C10_pn ), ((_au0__Xthis__ctor_ref ))) ) ) ;
}

#line 1057 "gram.y"
break ;
case 203 : 
#line 1059 "gram.y"
{ yyval . __C10_p = (struct node *)_expr__ctor ( (struct expr *)0 , (unsigned char )45 , (_au1_yypvt [-3]). __C10_pe ,
#line 1059 "gram.y"
(_au1_yypvt [- 0 ]). __C10_pe ) ;
}

#line 1059 "gram.y"
break ;
case 204 : 
#line 1061 "gram.y"
{ yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_ref = 0 ), ( (_au1__Xb__ctor_ref = (((_au1_yypvt [- 0 ]). __C10_pn ->
#line 1061 "gram.y"
_node_base == 123 )? _name__ctor ( (struct name *)0 , (_au1_yypvt [- 0 ]). __C10_pn -> _expr__O3.__C3_string ) : (_au1_yypvt [- 0 ]). __C10_pn )), (
#line 1061 "gram.y"
( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ref ), ((unsigned char )45 ), (_au1_yypvt [-2]). __C10_pe , (struct
#line 1061 "gram.y"
expr *)0 ) )) , ( (_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = _au1__Xb__ctor_ref ), ((_au0__Xthis__ctor_ref ))) ) ) ) ;
}

#line 1061 "gram.y"
break ;
case 205 : 
#line 1063 "gram.y"
{ yyval . __C10_p = (struct node *)(((_au1_yypvt [- 0 ]). __C10_pn -> _node_base == 123 )? _name__ctor ( (struct name *)0 ,
#line 1063 "gram.y"
(_au1_yypvt [- 0 ]). __C10_pn -> _expr__O3.__C3_string ) : (_au1_yypvt [- 0 ]). __C10_pn );
yyval . __C10_pn -> _name__O6.__C6_n_qualifier = sta_name ;
}

#line 1065 "gram.y"
break ;
case 207 : 
#line 1068 "gram.y"
{ yyval . __C10_p = (_au1_yypvt [-1]). __C10_p ;
}

#line 1068 "gram.y"
break ;
case 208 : 
#line 1070 "gram.y"
{ yyval . __C10_p = (struct node *)zero ;
}

#line 1070 "gram.y"
break ;
case 209 : 
#line 1072 "gram.y"
{ yyval . __C10_p = (struct node *)_expr__ctor ( (struct expr *)0 , (unsigned char )82 , (struct expr *)0 ,
#line 1072 "gram.y"
(struct expr *)0 ) ;
yyval . __C10_pe -> _expr__O3.__C3_string = (_au1_yypvt [- 0 ]). __C10_s ;
}

#line 1074 "gram.y"
break ;
case 210 : 
#line 1076 "gram.y"
{ yyval . __C10_p = (struct node *)_expr__ctor ( (struct expr *)0 , (unsigned char )83 , (struct expr *)0 ,
#line 1076 "gram.y"
(struct expr *)0 ) ;
yyval . __C10_pe -> _expr__O3.__C3_string = (_au1_yypvt [- 0 ]). __C10_s ;
}

#line 1078 "gram.y"
break ;
case 211 : 
#line 1080 "gram.y"
{ yyval . __C10_p = (struct node *)_expr__ctor ( (struct expr *)0 , (unsigned char )81 , (struct expr *)0 ,
#line 1080 "gram.y"
(struct expr *)0 ) ;
yyval . __C10_pe -> _expr__O3.__C3_string = (_au1_yypvt [- 0 ]). __C10_s ;
}

#line 1082 "gram.y"
break ;
case 212 : 
#line 1084 "gram.y"
{ yyval . __C10_p = (struct node *)_expr__ctor ( (struct expr *)0 , (unsigned char )84 , (struct expr *)0 ,
#line 1084 "gram.y"
(struct expr *)0 ) ;
yyval . __C10_pe -> _expr__O3.__C3_string = (_au1_yypvt [- 0 ]). __C10_s ;
}

#line 1086 "gram.y"
break ;
case 213 : 
#line 1088 "gram.y"
{ yyval . __C10_p = (struct node *)_expr__ctor ( (struct expr *)0 , (unsigned char )34 , (struct expr *)0 ,
#line 1088 "gram.y"
(struct expr *)0 ) ;
}

#line 1088 "gram.y"
break ;
case 214 : 
#line 1092 "gram.y"
{ yyval . __C10_p = (struct node *)(_au1_yypvt [- 0 ]). __C10_pn ;
}

#line 1092 "gram.y"
break ;
case 215 : 
#line 1094 "gram.y"
{ yyval . __C10_p = (struct node *)(((_au1_yypvt [- 0 ]). __C10_pn -> _node_base == 123 )? _name__ctor ( (struct name *)0 ,
#line 1094 "gram.y"
(_au1_yypvt [- 0 ]). __C10_pn -> _expr__O3.__C3_string ) : (_au1_yypvt [- 0 ]). __C10_pn );
yyval . __C10_pn -> _name__O6.__C6_n_qualifier = (_au1_yypvt [-2]). __C10_pn ;
}

#line 1096 "gram.y"
break ;
case 216 : 
#line 1098 "gram.y"
{ yyval . __C10_p = (struct node *)(((_au1_yypvt [- 0 ]). __C10_pn -> _node_base == 123 )? _name__ctor ( (struct name *)0 ,
#line 1098 "gram.y"
(_au1_yypvt [- 0 ]). __C10_pn -> _expr__O3.__C3_string ) : (_au1_yypvt [- 0 ]). __C10_pn );
look_for_hidden ( (_au1_yypvt [-2]). __C10_pn , yyval . __C10_pn ) ;
}

#line 1100 "gram.y"
break ;
case 217 : 
#line 1102 "gram.y"
{ yyval . __C10_p = (struct node *)_name__ctor ( (struct name *)0 , oper_name ( (_au1_yypvt [- 0 ]). __C10_t ) )
#line 1102 "gram.y"
;
yyval . __C10_pn -> _name_n_oper = (_au1_yypvt [- 0 ]). __C10_t ;
}

#line 1104 "gram.y"
break ;
case 218 : 
#line 1106 "gram.y"
{ yyval . __C10_p = (struct node *)_name__ctor ( (struct name *)0 , oper_name ( (_au1_yypvt [- 0 ]). __C10_t ) )
#line 1106 "gram.y"
;
yyval . __C10_pn -> _name_n_oper = (_au1_yypvt [- 0 ]). __C10_t ;
yyval . __C10_pn -> _name__O6.__C6_n_qualifier = (_au1_yypvt [-3]). __C10_pn ;
}

#line 1109 "gram.y"
break ;
case 219 : 
#line 1111 "gram.y"
{ yyval . __C10_p = (struct node *)_name__ctor ( (struct name *)0 , oper_name ( (_au1_yypvt [- 0 ]). __C10_t ) )
#line 1111 "gram.y"
;
yyval . __C10_pn -> _name_n_oper = (_au1_yypvt [- 0 ]). __C10_t ;
look_for_hidden ( (_au1_yypvt [-3]). __C10_pn , yyval . __C10_pn ) ;
}

#line 1114 "gram.y"
break ;
case 220 : 
#line 1116 "gram.y"
{ yyval . __C10_p = (_au1_yypvt [- 0 ]). __C10_p ;
sig_name ( yyval . __C10_pn ) ;
}

#line 1118 "gram.y"
break ;
case 221 : 
#line 1120 "gram.y"
{ yyval . __C10_p = (_au1_yypvt [- 0 ]). __C10_p ;
sig_name ( yyval . __C10_pn ) ;
yyval . __C10_pn -> _name__O6.__C6_n_qualifier = (_au1_yypvt [-3]). __C10_pn ;
}

#line 1123 "gram.y"
break ;
case 222 : 
#line 1125 "gram.y"
{ yyval . __C10_p = (_au1_yypvt [- 0 ]). __C10_p ;
sig_name ( yyval . __C10_pn ) ;
look_for_hidden ( (_au1_yypvt [-3]). __C10_pn , yyval . __C10_pn ) ;
}

#line 1128 "gram.y"
break ;
case 223 : 
#line 1136 "gram.y"
{ yyval . __C10_p = (struct node *)_name_normalize ( (_au1_yypvt [- 0 ]). __C10_pn , ((struct basetype *)(_au1_yypvt [-1]). __C10_p ), (struct
#line 1136 "gram.y"
block *)0 , (char )1 ) ;
}

#line 1136 "gram.y"
break ;
case 224 : 
#line 1140 "gram.y"
{ yyval . __C10_p = (struct node *)_basetype__ctor ( (struct basetype *)0 , (_au1_yypvt [- 0 ]). __C10_t , (struct name *)0 )
#line 1140 "gram.y"
;
}

#line 1140 "gram.y"
break ;
case 225 : 
#line 1142 "gram.y"
{ yyval . __C10_p = (struct node *)_basetype__ctor ( (struct basetype *)0 , (unsigned char )97 , (_au1_yypvt [- 0 ]).
#line 1142 "gram.y"
__C10_pn ) ;
}

#line 1142 "gram.y"
break ;
case 226 : 
#line 1146 "gram.y"
{ yyval . __C10_p = (struct node *)_name_normalize ( (_au1_yypvt [- 0 ]). __C10_pn , ((struct basetype *)(_au1_yypvt [-1]). __C10_p ), (struct
#line 1146 "gram.y"
block *)0 , (char )1 ) ;
}

#line 1146 "gram.y"
break ;
case 227 : 
#line 1150 "gram.y"
{ yyval . __C10_p = (struct node *)_name_normalize ( (_au1_yypvt [- 0 ]). __C10_pn , ((struct basetype *)(_au1_yypvt [-1]). __C10_p ), (struct
#line 1150 "gram.y"
block *)0 , (char )1 ) ;
}

#line 1150 "gram.y"
break ;
case 228 : 
#line 1153 "gram.y"
{ yyval . __C10_p = (struct node *)_name_normalize ( (_au1_yypvt [- 0 ]). __C10_pn , ((struct basetype *)(_au1_yypvt [-1]). __C10_p ), (struct
#line 1153 "gram.y"
block *)0 , (char )0 ) ;
}

#line 1153 "gram.y"
break ;
case 229 : 
#line 1155 "gram.y"
{ yyval . __C10_p = (struct node *)_name_normalize ( (_au1_yypvt [-2]). __C10_pn , ((struct basetype *)(_au1_yypvt [-3]). __C10_p ), (struct block *)0 ,
#line 1155 "gram.y"
(char )0 ) ;
yyval . __C10_pn -> _expr__O4.__C4_n_initializer = (_au1_yypvt [- 0 ]). __C10_pe ;
}

#line 1157 "gram.y"
break ;
case 230 : 
#line 1161 "gram.y"
{ yyval . __C10_p = (struct node *)_fct__ctor ( (struct fct *)0 , (struct type *)0 , name_unlist ( (_au1_yypvt [-1]). __C10_nl )
#line 1161 "gram.y"
, (unsigned char )1 ) ;
}

#line 1161 "gram.y"
break ;
case 231 : 
#line 1163 "gram.y"
{ yyval . __C10_p = (struct node *)_fct__ctor ( (struct fct *)0 , (struct type *)0 , name_unlist ( (_au1_yypvt [-2]). __C10_nl )
#line 1163 "gram.y"
, (unsigned char )155 ) ;
}

#line 1163 "gram.y"
break ;
case 232 : 
#line 1165 "gram.y"
{ yyval . __C10_p = (struct node *)_fct__ctor ( (struct fct *)0 , (struct type *)0 , name_unlist ( (_au1_yypvt [-3]). __C10_nl )
#line 1165 "gram.y"
, (unsigned char )155 ) ;
}

#line 1165 "gram.y"
break ;
case 233 : 
#line 1169 "gram.y"
{ if ((_au1_yypvt [- 0 ]). __C10_p )
#line 1170 "gram.y"
if ((_au1_yypvt [-2]). __C10_p )
#line 1171 "gram.y"
( ((_au1_yypvt [-2]). __C10_nl -> _nlist_tail -> _name_n_list = (_au1_yypvt [-
#line 1171 "gram.y"
0 ]). __C10_pn ), ((_au1_yypvt [-2]). __C10_nl -> _nlist_tail = (_au1_yypvt [- 0 ]). __C10_pn )) ;
else { 
#line 1173 "gram.y"
error ( (char *)"AD syntax", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 1173 "gram.y"

#line 1174 "gram.y"
yyval . __C10_nl = (struct nlist *)_nlist__ctor ( (struct nlist *)0 , (_au1_yypvt [- 0 ]). __C10_pn ) ;
}
else 
#line 1177 "gram.y"
error ( (char *)"AD syntax", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
}

#line 1178 "gram.y"
break ;
case 234 : 
#line 1180 "gram.y"
{ if ((_au1_yypvt [- 0 ]). __C10_p )yyval . __C10_nl = (struct nlist *)_nlist__ctor ( (struct nlist *)0 , (_au1_yypvt [- 0 ]).
#line 1180 "gram.y"
__C10_pn ) ;
}

#line 1180 "gram.y"
break ;
case 236 : 
#line 1186 "gram.y"
{ yyval . __C10_p = 0 ;
}

#line 1186 "gram.y"
break ;
case 237 : 
#line 1191 "gram.y"
{ yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_ptr = 0 ), ( (_au0__Xthis__ctor_ptr = (struct ptr *)_new ( (long
#line 1191 "gram.y"
)(sizeof (struct ptr))) ), ( (Nt ++ ), ( (_au0__Xthis__ctor_ptr -> _node_base = ((unsigned char )125 )), ( (_au0__Xthis__ctor_ptr ->
#line 1191 "gram.y"
_pvtyp_typ = ((struct type *)0 )), ( (_au0__Xthis__ctor_ptr -> _ptr_rdo = ((char )0 )), ((_au0__Xthis__ctor_ptr ))) ) ) ) ) )
#line 1191 "gram.y"
;
}

#line 1191 "gram.y"
break ;
case 238 : 
#line 1193 "gram.y"
{ yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_ptr = 0 ), ( (_au0__Xthis__ctor_ptr = (struct ptr *)_new ( (long
#line 1193 "gram.y"
)(sizeof (struct ptr))) ), ( (Nt ++ ), ( (_au0__Xthis__ctor_ptr -> _node_base = ((unsigned char )158 )), ( (_au0__Xthis__ctor_ptr ->
#line 1193 "gram.y"
_pvtyp_typ = ((struct type *)0 )), ( (_au0__Xthis__ctor_ptr -> _ptr_rdo = ((char )0 )), ((_au0__Xthis__ctor_ptr ))) ) ) ) ) )
#line 1193 "gram.y"
;
}

#line 1193 "gram.y"
break ;
case 239 : 
#line 1195 "gram.y"
{ TOK _au3_t ;

#line 1196 "gram.y"
struct ptr *_au0__Xthis__ctor_ptr ;

#line 1195 "gram.y"
_au3_t = (_au1_yypvt [- 0 ]). __C10_t ;
switch (_au3_t ){ 
#line 1197 "gram.y"
case 170 : 
#line 1198 "gram.y"
{ 
#line 1237 "gram.y"
struct ea _au0__V22 ;

#line 1198 "gram.y"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"\"%k\" not implemented (ignored)", (struct ea *)( ( ((& _au0__V22 )-> _ea__O1.__C1_i = ((int )_au3_t )), (((&
#line 1198 "gram.y"
_au0__V22 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_ptr = 0 ), ( (_au0__Xthis__ctor_ptr = (struct ptr *)_new ( (long )(sizeof (struct ptr))) ),
#line 1199 "gram.y"
( (Nt ++ ), ( (_au0__Xthis__ctor_ptr -> _node_base = ((unsigned char )125 )), ( (_au0__Xthis__ctor_ptr -> _pvtyp_typ = ((struct type *)0 )),
#line 1199 "gram.y"
( (_au0__Xthis__ctor_ptr -> _ptr_rdo = ((char )0 )), ((_au0__Xthis__ctor_ptr ))) ) ) ) ) ) ;
break ;
default : 
#line 1202 "gram.y"
{ 
#line 1237 "gram.y"
struct ea _au0__V23 ;

#line 1202 "gram.y"
error ( (char *)"syntax error: *%k", (struct ea *)( ( ((& _au0__V23 )-> _ea__O1.__C1_i = ((int )_au3_t )), (((& _au0__V23 )))) )
#line 1202 "gram.y"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
case 26 : 
#line 1204 "gram.y"
yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_ptr = 0 ), ( (_au0__Xthis__ctor_ptr = (struct ptr *)_new ( (long )(sizeof
#line 1204 "gram.y"
(struct ptr))) ), ( (Nt ++ ), ( (_au0__Xthis__ctor_ptr -> _node_base = ((unsigned char )125 )), ( (_au0__Xthis__ctor_ptr -> _pvtyp_typ =
#line 1204 "gram.y"
((struct type *)0 )), ( (_au0__Xthis__ctor_ptr -> _ptr_rdo = ((char )1 )), ((_au0__Xthis__ctor_ptr ))) ) ) ) ) ) ;
#line 1204 "gram.y"
} } }
}
break ;
case 240 : 
#line 1208 "gram.y"
{ TOK _au3_t ;

#line 1209 "gram.y"
struct ptr *_au0__Xthis__ctor_ptr ;

#line 1208 "gram.y"
_au3_t = (_au1_yypvt [- 0 ]). __C10_t ;
switch (_au3_t ){ 
#line 1210 "gram.y"
case 170 : 
#line 1211 "gram.y"
{ 
#line 1237 "gram.y"
struct ea _au0__V24 ;

#line 1211 "gram.y"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"\"%k\" not implemented (ignored)", (struct ea *)( ( ((& _au0__V24 )-> _ea__O1.__C1_i = ((int )_au3_t )), (((&
#line 1211 "gram.y"
_au0__V24 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_ptr = 0 ), ( (_au0__Xthis__ctor_ptr = (struct ptr *)_new ( (long )(sizeof (struct ptr))) ),
#line 1212 "gram.y"
( (Nt ++ ), ( (_au0__Xthis__ctor_ptr -> _node_base = ((unsigned char )158 )), ( (_au0__Xthis__ctor_ptr -> _pvtyp_typ = ((struct type *)0 )),
#line 1212 "gram.y"
( (_au0__Xthis__ctor_ptr -> _ptr_rdo = ((char )0 )), ((_au0__Xthis__ctor_ptr ))) ) ) ) ) ) ;
break ;
default : 
#line 1215 "gram.y"
{ 
#line 1237 "gram.y"
struct ea _au0__V25 ;

#line 1215 "gram.y"
error ( (char *)"syntax error: &%k", (struct ea *)( ( ((& _au0__V25 )-> _ea__O1.__C1_i = ((int )_au3_t )), (((& _au0__V25 )))) )
#line 1215 "gram.y"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
case 26 : 
#line 1217 "gram.y"
yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_ptr = 0 ), ( (_au0__Xthis__ctor_ptr = (struct ptr *)_new ( (long )(sizeof
#line 1217 "gram.y"
(struct ptr))) ), ( (Nt ++ ), ( (_au0__Xthis__ctor_ptr -> _node_base = ((unsigned char )158 )), ( (_au0__Xthis__ctor_ptr -> _pvtyp_typ =
#line 1217 "gram.y"
((struct type *)0 )), ( (_au0__Xthis__ctor_ptr -> _ptr_rdo = ((char )1 )), ((_au0__Xthis__ctor_ptr ))) ) ) ) ) ) ;
#line 1217 "gram.y"
} } }
}
break ;
case 241 : 
#line 1221 "gram.y"
{ Pptr _au3_p ;

#line 1222 "gram.y"
struct ptr *_au0__Xthis__ctor_ptr ;

#line 1221 "gram.y"
_au3_p = (struct ptr *)( (_au0__Xthis__ctor_ptr = 0 ), ( (_au0__Xthis__ctor_ptr = (struct ptr *)_new ( (long )(sizeof (struct ptr))) ), (
#line 1221 "gram.y"
(Nt ++ ), ( (_au0__Xthis__ctor_ptr -> _node_base = ((unsigned char )125 )), ( (_au0__Xthis__ctor_ptr -> _pvtyp_typ = ((struct type *)0 )), (
#line 1221 "gram.y"
(_au0__Xthis__ctor_ptr -> _ptr_rdo = ((char )0 )), ((_au0__Xthis__ctor_ptr ))) ) ) ) ) ) ;
_au3_p -> _ptr_memof = (((struct classdef *)(((struct basetype *)(_au1_yypvt [- 0 ]). __C10_pn -> _expr__O2.__C2_tp ))-> _basetype_b_name -> _expr__O2.__C2_tp ));
yyval . __C10_p = (struct node *)_au3_p ;
}

#line 1224 "gram.y"
break ;
case 242 : 
#line 1229 "gram.y"
{ yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_vec = 0 ), ( (_au0__Xthis__ctor_vec = (struct vec *)_new ( (long
#line 1229 "gram.y"
)(sizeof (struct vec))) ), ( (Nt ++ ), ( (_au0__Xthis__ctor_vec -> _node_base = 110 ), ( (_au0__Xthis__ctor_vec -> _pvtyp_typ = ((struct
#line 1229 "gram.y"
type *)0 )), ( (_au0__Xthis__ctor_vec -> _vec_dim = (((_au1_yypvt [-1]). __C10_pe != dummy )? (_au1_yypvt [-1]). __C10_pe : (((struct expr *)0 )))), ((_au0__Xthis__ctor_vec ))) ) )
#line 1229 "gram.y"
) ) ) ;
}

#line 1229 "gram.y"
break ;
case 243 : 
#line 1234 "gram.y"
{ yyval . __C10_p = (struct node *)( (_au0__Xthis__ctor_vec = 0 ), ( (_au0__Xthis__ctor_vec = (struct vec *)_new ( (long
#line 1234 "gram.y"
)(sizeof (struct vec))) ), ( (Nt ++ ), ( (_au0__Xthis__ctor_vec -> _node_base = 110 ), ( (_au0__Xthis__ctor_vec -> _pvtyp_typ = ((struct
#line 1234 "gram.y"
type *)0 )), ( (_au0__Xthis__ctor_vec -> _vec_dim = ((struct expr *)0 )), ((_au0__Xthis__ctor_vec ))) ) ) ) ) ) ;
}

#line 1234 "gram.y"
break ;
}
goto yystack ;
}
;

/* the end */
