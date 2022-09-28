/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#line 1 "../../src/expr2.c"

/* <<cfront 1.2.1 2/16/87>> */
/* < ../../src/expr2.c */

#ident	"@(#)sdb:cfront/scratch/src/expr2..c	1.2"
char *_new(); char _delete(); char *_vec_new(); char _vec_delete();

#line 1 "../../src/expr2.c"

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

#line 20 "../../src/expr2.c"
static int refd ;
int tsize = 0 ;
int t_const = 0 ;

#line 24 "../../src/expr2.c"
Pname make_tmp (_au0_c , _au0_t , _au0_tbl )char _au0_c ;

#line 24 "../../src/expr2.c"
Ptype _au0_t ;

#line 24 "../../src/expr2.c"
Ptable _au0_tbl ;
{ 
#line 30 "../../src/expr2.c"
Pname _au1_tmpx ;

#line 32 "../../src/expr2.c"
Pname _au1_tmp ;
Pname _au1_nn ;

#line 26 "../../src/expr2.c"
if (Cstmt ){ 
#line 27 "../../src/expr2.c"
if (Cstmt -> _stmt_memtbl == 0 )Cstmt -> _stmt_memtbl = (struct table *)_table__ctor ( (struct table *)0 , (short )4 , _au0_tbl ,
#line 27 "../../src/expr2.c"
(struct name *)0 ) ;
_au0_tbl = Cstmt -> _stmt_memtbl ;
}
_au1_tmpx = (struct name *)_name__ctor ( (struct name *)0 , make_name ( (unsigned char )_au0_c ) ) ;
_au1_tmpx -> _expr__O2.__C2_tp = _au0_t ;
_au1_tmp = _name_dcl ( _au1_tmpx , _au0_tbl , (unsigned char )136 ) ;

#line 32 "../../src/expr2.c"
_au1_nn = _au0_tbl -> _table_t_name ;

#line 34 "../../src/expr2.c"
if (_au1_nn && (_au1_nn -> _expr__O2.__C2_tp -> _node_base == 6 ))tsize = _type_tsizeof ( _au1_tmp -> _expr__O2.__C2_tp ) ;
_name__dtor ( _au1_tmpx , 1) ;
_au1_tmp -> _name_n_scope = 108 ;
return _au1_tmp ;
}
;
Pexpr init_tmp (_au0_tmp , _au0_init , _au0_tbl )Pname _au0_tmp ;

#line 40 "../../src/expr2.c"
Pexpr _au0_init ;

#line 40 "../../src/expr2.c"
Ptable _au0_tbl ;
{ 
#line 42 "../../src/expr2.c"
Pname _au1_cn ;
Pname _au1_ct ;
Pexpr _au1_ass ;

#line 42 "../../src/expr2.c"
_au1_cn = _type_is_cl_obj ( _au0_tmp -> _expr__O2.__C2_tp ) ;
_au1_ct = (_au1_cn ? ( (((struct classdef *)_au1_cn -> _expr__O2.__C2_tp ))-> _classdef_itor ) : (((struct name *)0 )));

#line 46 "../../src/expr2.c"
_au0_tmp -> _name_n_assigned_to = 1 ;

#line 48 "../../src/expr2.c"
if (_au1_ct ){ 
#line 49 "../../src/expr2.c"
if (refd && ((_au0_init -> _expr__O3.__C3_e1 -> _node_base == 85 )|| (_au0_init -> _expr__O3.__C3_e1 -> _node_base == 44 )))
#line 51 "../../src/expr2.c"
_au0_init = (struct expr *)_expr__ctor (
#line 51 "../../src/expr2.c"
(struct expr *)0 , (unsigned char )147 , _au0_init , _expr_address ( _au0_init -> _expr__O3.__C3_e1 ) ) ;

#line 53 "../../src/expr2.c"
if (refd )_au0_tbl = 0 ;
{ Pref _au2_r ;
Pexpr _au2_a ;

#line 56 "../../src/expr2.c"
struct ref *_au0__Xthis__ctor_ref ;

#line 54 "../../src/expr2.c"
_au2_r = (struct ref *)( (_au0__Xthis__ctor_ref = 0 ), ( ( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ref ),
#line 54 "../../src/expr2.c"
((unsigned char )45 ), ((struct expr *)_au0_tmp ), (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = _au1_ct ), ((_au0__Xthis__ctor_ref )))
#line 54 "../../src/expr2.c"
) ) ;
_au2_a = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , _au0_init , (struct expr *)0 ) ;
_au1_ass = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )146 , (struct expr *)_au2_r , _au2_a ) ;
_au1_ass -> _expr__O5.__C5_fct_name = _au1_ct ;
if (_au0_tbl )_au1_ass = _expr_typ ( _au1_ass , _au0_tbl ) ;
}
}
else 
#line 60 "../../src/expr2.c"
{ 
#line 61 "../../src/expr2.c"
_au1_ass = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )70 , (struct expr *)_au0_tmp , _au0_init ) ;
#line 61 "../../src/expr2.c"

#line 62 "../../src/expr2.c"
_au1_ass -> _expr__O2.__C2_tp = _au0_tmp -> _expr__O2.__C2_tp ;
}
return _au1_ass ;
}
;
char _name_assign (_au0_this )
#line 607 "../../src/cfront.h"
struct name *_au0_this ;

#line 68 "../../src/expr2.c"
{ 
#line 69 "../../src/expr2.c"
if ((_au0_this -> _name_n_assigned_to ++ )== 0 ){ 
#line 70 "../../src/expr2.c"
switch (_au0_this -> _name_n_scope ){ 
#line 71 "../../src/expr2.c"
case 108 : 
#line 72 "../../src/expr2.c"
if (_au0_this -> _name_n_used &&
#line 72 "../../src/expr2.c"
(_au0_this -> _name_n_addr_taken == 0 )){ 
#line 73 "../../src/expr2.c"
Ptype _au4_t ;

#line 73 "../../src/expr2.c"
_au4_t = _au0_this -> _expr__O2.__C2_tp ;
ll :
#line 75 "../../src/expr2.c"
switch (_au4_t -> _node_base ){ 
#line 76 "../../src/expr2.c"
case 97 : 
#line 77 "../../src/expr2.c"
_au4_t = (((struct basetype *)_au4_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 77 "../../src/expr2.c"
goto ll ;
case 110 : 
#line 79 "../../src/expr2.c"
break ;
default : 
#line 81 "../../src/expr2.c"
if (curr_loop )
#line 82 "../../src/expr2.c"
{ 
#line 89 "../../src/expr2.c"
struct ea _au0__V10 ;

#line 82 "../../src/expr2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"%n may have been used before set", (struct ea *)( ( ((& _au0__V10 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((&
#line 82 "../../src/expr2.c"
_au0__V10 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} else 
#line 84 "../../src/expr2.c"
{ 
#line 89 "../../src/expr2.c"
struct ea _au0__V11 ;

#line 84 "../../src/expr2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"%n used before set", (struct ea *)( ( ((& _au0__V11 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((&
#line 84 "../../src/expr2.c"
_au0__V11 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}
}
}
}
;
int _expr_lval (_au0_this , _au0_oper )
#line 525 "../../src/cfront.h"
struct expr *_au0_this ;

#line 91 "../../src/expr2.c"
TOK _au0_oper ;
{ 
#line 93 "../../src/expr2.c"
register Pexpr _au1_ee ;
register Pname _au1_n ;
int _au1_deref ;
char *_au1_es ;

#line 93 "../../src/expr2.c"
_au1_ee = _au0_this ;

#line 95 "../../src/expr2.c"
_au1_deref = 0 ;

#line 100 "../../src/expr2.c"
switch (_au0_oper ){ 
#line 101 "../../src/expr2.c"
case 112 : 
#line 102 "../../src/expr2.c"
case 145 : _au1_es = "address of";

#line 102 "../../src/expr2.c"
break ;
case 48 : 
#line 104 "../../src/expr2.c"
case 49 : _au1_es = "increment of";

#line 104 "../../src/expr2.c"
goto def ;
case 111 : _au1_es = "dereference of";

#line 105 "../../src/expr2.c"
break ;
default : _au1_es = "assignment to";
def :
#line 108 "../../src/expr2.c"
if (_type_tconst ( _au0_this -> _expr__O2.__C2_tp ) ){ 
#line 109 "../../src/expr2.c"
if (_au0_oper ){ 
#line 110 "../../src/expr2.c"
if (_au0_this -> _node_base == 85 )
#line 111 "../../src/expr2.c"
{ 
#line 292 "../../src/expr2.c"
struct ea _au0__V12 ;

#line 292 "../../src/expr2.c"
struct ea _au0__V13 ;

#line 111 "../../src/expr2.c"
error ( (char *)"%s constant%n", (struct ea *)( ( ((& _au0__V12 )-> _ea__O1.__C1_p = ((char *)_au1_es )), (((& _au0__V12 )))) )
#line 111 "../../src/expr2.c"
, (struct ea *)( ( ((& _au0__V13 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V13 )))) ) , (struct
#line 111 "../../src/expr2.c"
ea *)ea0 , (struct ea *)ea0 ) ;
} else 
#line 113 "../../src/expr2.c"
{ 
#line 292 "../../src/expr2.c"
struct ea _au0__V14 ;

#line 113 "../../src/expr2.c"
error ( (char *)"%s constant", (struct ea *)( ( ((& _au0__V14 )-> _ea__O1.__C1_p = ((char *)_au1_es )), (((& _au0__V14 )))) )
#line 113 "../../src/expr2.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
return (int )0 ;
}
}

#line 119 "../../src/expr2.c"
for(;;) { 
#line 121 "../../src/expr2.c"
switch (_au1_ee -> _node_base ){ 
#line 122 "../../src/expr2.c"
case 146 : 
#line 123 "../../src/expr2.c"
case 109 : 
#line 124 "../../src/expr2.c"
if (_au1_deref == 0 ){ 
#line 125 "../../src/expr2.c"
switch (_au0_oper ){
#line 125 "../../src/expr2.c"

#line 126 "../../src/expr2.c"
case 112 : 
#line 127 "../../src/expr2.c"
case 145 : 
#line 128 "../../src/expr2.c"
case 0 : 
#line 129 "../../src/expr2.c"
if (_au1_ee -> _expr__O5.__C5_fct_name && (((struct fct *)_au1_ee -> _expr__O5.__C5_fct_name -> _expr__O2.__C2_tp ))-> _fct_f_inline )
#line 130 "../../src/expr2.c"
return
#line 130 "../../src/expr2.c"
1 ;
}
}
default : 
#line 134 "../../src/expr2.c"
if (_au1_deref == 0 ){ 
#line 135 "../../src/expr2.c"
if (_au0_oper ){ 
#line 292 "../../src/expr2.c"
struct ea _au0__V15 ;

#line 292 "../../src/expr2.c"
struct ea _au0__V16 ;

#line 135 "../../src/expr2.c"
error ( (char *)"%s %k (not an lvalue)", (struct ea *)( ( ((& _au0__V15 )-> _ea__O1.__C1_p = ((char *)_au1_es )), (((& _au0__V15 )))) )
#line 135 "../../src/expr2.c"
, (struct ea *)( ( ((& _au0__V16 )-> _ea__O1.__C1_i = ((int )_au1_ee -> _node_base )), (((& _au0__V16 )))) ) ,
#line 135 "../../src/expr2.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
} return (int )0 ;
}
return 1 ;
case 86 : 
#line 140 "../../src/expr2.c"
case 84 : 
#line 141 "../../src/expr2.c"
case 82 : 
#line 142 "../../src/expr2.c"
case 83 : 
#line 143 "../../src/expr2.c"
if (_au0_oper ){ 
#line 292 "../../src/expr2.c"
struct ea _au0__V17 ;

#line 143 "../../src/expr2.c"
error ( (char *)"%s numeric constant", (struct ea *)( ( ((& _au0__V17 )-> _ea__O1.__C1_p = ((char *)_au1_es )), (((& _au0__V17 )))) )
#line 143 "../../src/expr2.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} return (int )0 ;
case 81 : 
#line 146 "../../src/expr2.c"
if (_au0_oper ){ 
#line 292 "../../src/expr2.c"
struct ea _au0__V18 ;

#line 146 "../../src/expr2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"%s string constant", (struct ea *)( ( ((& _au0__V18 )-> _ea__O1.__C1_p = ((char *)_au1_es )), (((&
#line 146 "../../src/expr2.c"
_au0__V18 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} return 1 ;

#line 149 "../../src/expr2.c"
case 111 : 
#line 150 "../../src/expr2.c"
{ Pexpr _au4_ee1 ;

#line 150 "../../src/expr2.c"
_au4_ee1 = _au1_ee -> _expr__O3.__C3_e1 ;
if (_au4_ee1 -> _node_base == 112 )
#line 152 "../../src/expr2.c"
_au1_ee = _au4_ee1 -> _expr__O4.__C4_e2 ;
else { 
#line 154 "../../src/expr2.c"
_au1_ee = _au4_ee1 ;
_au1_deref = 1 ;
}
break ;
}

#line 160 "../../src/expr2.c"
case 45 : 
#line 162 "../../src/expr2.c"
switch (_au1_ee -> _expr__O3.__C3_e1 -> _node_base ){ 
#line 163 "../../src/expr2.c"
case 85 : 
#line 165 "../../src/expr2.c"
switch (_au0_oper ){ 
#line 166 "../../src/expr2.c"
case 112 : 
#line 167 "../../src/expr2.c"
case 145 :
#line 167 "../../src/expr2.c"
( ((((struct name *)_au1_ee -> _expr__O3.__C3_e1 ))-> _name_n_addr_taken ++ )) ;
case 0 : break ;
case 70 : (((struct name *)_au1_ee -> _expr__O3.__C3_e1 ))-> _name_n_used -- ;
default : _name_assign ( ((struct name *)_au1_ee -> _expr__O3.__C3_e1 )) ;
}
break ;
case 45 : 
#line 174 "../../src/expr2.c"
{ Pexpr _au4_e ;

#line 174 "../../src/expr2.c"
_au4_e = _au1_ee -> _expr__O3.__C3_e1 ;
do _au4_e = _au4_e -> _expr__O3.__C3_e1 ;
while (_au4_e -> _node_base == 45 );

#line 176 "../../src/expr2.c"
if (_au4_e -> _node_base == 85 ){ 
#line 178 "../../src/expr2.c"
switch (_au0_oper ){ 
#line 179 "../../src/expr2.c"
case 112 : 
#line 180 "../../src/expr2.c"
case 145 : ( ((((struct name *)_au4_e ))-> _name_n_addr_taken ++
#line 180 "../../src/expr2.c"
)) ;
case 0 : break ;
case 70 : (((struct name *)_au4_e ))-> _name_n_used -- ;
default : _name_assign ( ((struct name *)_au4_e )) ;
}
}
}
}

#line 187 "../../src/expr2.c"
_au1_n = _au1_ee -> _expr__O5.__C5_mem ;
if ((_au1_deref == 0 )&& _type_tconst ( _au1_ee -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp ) ){ 
#line 189 "../../src/expr2.c"
switch (_au0_oper ){ 
#line 190 "../../src/expr2.c"
case 0 : 
#line 191 "../../src/expr2.c"
case 112 :
#line 191 "../../src/expr2.c"

#line 192 "../../src/expr2.c"
case 145 : 
#line 193 "../../src/expr2.c"
case 111 : 
#line 194 "../../src/expr2.c"
break ;
default : 
#line 196 "../../src/expr2.c"
{ 
#line 292 "../../src/expr2.c"
struct ea _au0__V19 ;

#line 292 "../../src/expr2.c"
struct ea _au0__V20 ;

#line 292 "../../src/expr2.c"
struct ea _au0__V21 ;

#line 196 "../../src/expr2.c"
error ( (char *)"%sM%n of%t", (struct ea *)( ( ((& _au0__V19 )-> _ea__O1.__C1_p = ((char *)_au1_es )), (((& _au0__V19 )))) )
#line 196 "../../src/expr2.c"
, (struct ea *)( ( ((& _au0__V20 )-> _ea__O1.__C1_p = ((char *)_au1_n )), (((& _au0__V20 )))) ) , (struct
#line 196 "../../src/expr2.c"
ea *)( ( ((& _au0__V21 )-> _ea__O1.__C1_p = ((char *)_au1_ee -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp )), (((& _au0__V21 )))) ) , (struct
#line 196 "../../src/expr2.c"
ea *)ea0 ) ;
} }
return (int )0 ;
}
goto xx ;

#line 202 "../../src/expr2.c"
case 44 : 
#line 203 "../../src/expr2.c"
_au1_n = _au1_ee -> _expr__O5.__C5_mem ;
if (_au1_deref == 0 ){ 
#line 205 "../../src/expr2.c"
Ptype _au4_p ;

#line 205 "../../src/expr2.c"
_au4_p = _au1_ee -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp ;
zxc :
#line 207 "../../src/expr2.c"
switch (_au4_p -> _node_base ){ 
#line 208 "../../src/expr2.c"
case 97 : _au4_p = (((struct basetype *)_au4_p ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 208 "../../src/expr2.c"
goto zxc ;
case 125 : 
#line 210 "../../src/expr2.c"
case 110 : break ;
default : { 
#line 292 "../../src/expr2.c"
struct ea _au0__V22 ;

#line 292 "../../src/expr2.c"
struct ea _au0__V23 ;

#line 211 "../../src/expr2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"%t->%n", (struct ea *)( ( ((& _au0__V22 )-> _ea__O1.__C1_p = ((char *)_au4_p )), (((&
#line 211 "../../src/expr2.c"
_au0__V22 )))) ) , (struct ea *)( ( ((& _au0__V23 )-> _ea__O1.__C1_p = ((char *)_au1_n )), (((& _au0__V23 )))) )
#line 211 "../../src/expr2.c"
, (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
if (_type_tconst ( (((struct ptr *)_au4_p ))-> _pvtyp_typ ) ){ 
#line 214 "../../src/expr2.c"
switch (_au0_oper ){ 
#line 215 "../../src/expr2.c"
case 0 : 
#line 216 "../../src/expr2.c"
case 112 : 
#line 217 "../../src/expr2.c"
case
#line 217 "../../src/expr2.c"
145 : 
#line 218 "../../src/expr2.c"
case 111 : 
#line 219 "../../src/expr2.c"
break ;
default : 
#line 221 "../../src/expr2.c"
{ 
#line 292 "../../src/expr2.c"
struct ea _au0__V24 ;

#line 292 "../../src/expr2.c"
struct ea _au0__V25 ;

#line 292 "../../src/expr2.c"
struct ea _au0__V26 ;

#line 221 "../../src/expr2.c"
error ( (char *)"%sM%n of%t", (struct ea *)( ( ((& _au0__V24 )-> _ea__O1.__C1_p = ((char *)_au1_es )), (((& _au0__V24 )))) )
#line 221 "../../src/expr2.c"
, (struct ea *)( ( ((& _au0__V25 )-> _ea__O1.__C1_p = ((char *)_au1_n )), (((& _au0__V25 )))) ) , (struct
#line 221 "../../src/expr2.c"
ea *)( ( ((& _au0__V26 )-> _ea__O1.__C1_p = ((char *)(((struct ptr *)_au4_p ))-> _pvtyp_typ )), (((& _au0__V26 )))) ) , (struct
#line 221 "../../src/expr2.c"
ea *)ea0 ) ;
} }
return (int )0 ;
}
}
goto xx ;
case 85 : 
#line 228 "../../src/expr2.c"
_au1_n = (((struct name *)_au1_ee ));
xx :
#line 231 "../../src/expr2.c"
if (_au1_deref || (_au0_oper == 0 ))return 1 ;

#line 233 "../../src/expr2.c"
if ((_au1_n -> _expr__O2.__C2_tp -> _node_base == 114 )&& ((((struct basetype *)_au1_n -> _expr__O2.__C2_tp ))-> _basetype_b_bits == 0 )){ 
#line 234 "../../src/expr2.c"
{ 
#line 292 "../../src/expr2.c"
struct ea _au0__V27 ;

#line 292 "../../src/expr2.c"
struct ea _au0__V28 ;

#line 234 "../../src/expr2.c"
error ( (char *)"%s 0-length field%n", (struct ea *)( ( ((& _au0__V27 )-> _ea__O1.__C1_p = ((char *)_au1_es )), (((& _au0__V27 )))) )
#line 234 "../../src/expr2.c"
, (struct ea *)( ( ((& _au0__V28 )-> _ea__O1.__C1_p = ((char *)_au1_n )), (((& _au0__V28 )))) ) , (struct
#line 234 "../../src/expr2.c"
ea *)ea0 , (struct ea *)ea0 ) ;
return (int )0 ;
} }
switch (_au0_oper ){ 
#line 238 "../../src/expr2.c"
case 112 : 
#line 239 "../../src/expr2.c"
case 145 : 
#line 240 "../../src/expr2.c"
{ Pfct _au5_f ;

#line 240 "../../src/expr2.c"
_au5_f = (((struct fct *)_au1_n -> _expr__O2.__C2_tp ));
if (_au1_n -> _name_n_sto == 27 ){ 
#line 242 "../../src/expr2.c"
{ 
#line 292 "../../src/expr2.c"
struct ea _au0__V29 ;

#line 242 "../../src/expr2.c"
error ( (char *)"& register%n", (struct ea *)( ( ((& _au0__V29 )-> _ea__O1.__C1_p = ((char *)_au1_n )), (((& _au0__V29 )))) )
#line 242 "../../src/expr2.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
return (int )0 ;
} }
if (_au5_f == 0 ){ 
#line 246 "../../src/expr2.c"
{ 
#line 292 "../../src/expr2.c"
struct ea _au0__V30 ;

#line 246 "../../src/expr2.c"
error ( (char *)"& label%n", (struct ea *)( ( ((& _au0__V30 )-> _ea__O1.__C1_p = ((char *)_au1_n )), (((& _au0__V30 )))) )
#line 246 "../../src/expr2.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
return (int )0 ;
} }
if (_au1_n -> _name_n_stclass == 13 ){ 
#line 250 "../../src/expr2.c"
{ 
#line 292 "../../src/expr2.c"
struct ea _au0__V31 ;

#line 250 "../../src/expr2.c"
error ( (char *)"& enumerator%n", (struct ea *)( ( ((& _au0__V31 )-> _ea__O1.__C1_p = ((char *)_au1_n )), (((& _au0__V31 )))) )
#line 250 "../../src/expr2.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
return (int )0 ;
} }
if (_au1_n -> _expr__O2.__C2_tp -> _node_base == 114 ){ 
#line 254 "../../src/expr2.c"
{ 
#line 292 "../../src/expr2.c"
struct ea _au0__V32 ;

#line 292 "../../src/expr2.c"
struct ea _au0__V33 ;

#line 254 "../../src/expr2.c"
error ( (char *)"& field%n", (struct ea *)( ( ((& _au0__V32 )-> _ea__O1.__C1_p = ((char *)_au1_es )), (((& _au0__V32 )))) )
#line 254 "../../src/expr2.c"
, (struct ea *)( ( ((& _au0__V33 )-> _ea__O1.__C1_p = ((char *)_au1_n )), (((& _au0__V33 )))) ) , (struct
#line 254 "../../src/expr2.c"
ea *)ea0 , (struct ea *)ea0 ) ;
return (int )0 ;
} }
_au1_n -> _name_n_used -- ;
if (_au1_n -> _name__O6.__C6_n_qualifier )
#line 259 "../../src/expr2.c"
_au1_n = _table_look ( (((struct classdef *)_au1_n -> _expr__O5.__C5_n_table -> _table_t_name -> _expr__O2.__C2_tp ))-> _classdef_memtbl , _au1_n -> _expr__O3.__C3_string , (unsigned char
#line 259 "../../src/expr2.c"
)0 ) ;
( (_au1_n -> _name_n_addr_taken ++ )) ;

#line 262 "../../src/expr2.c"
if ((_au1_n -> _name_n_evaluated && (_au1_n -> _name_n_scope != 136 ))|| ((_au5_f -> _node_base == 108 )&& _au5_f -> _fct_f_inline ))
#line 263 "../../src/expr2.c"
{ 
#line 265 "../../src/expr2.c"
Pname _au6_nn ;

#line 266 "../../src/expr2.c"
struct ival *_au0__Xthis__ctor_ival ;

#line 265 "../../src/expr2.c"
_au6_nn = (struct name *)_name__ctor ( (struct name *)0 , (char *)0 ) ;
if (_au1_n -> _name_n_evaluated && (_au1_n -> _name_n_scope != 136 )){ 
#line 267 "../../src/expr2.c"
_au1_n -> _name_n_evaluated = 0 ;
_au1_n -> _expr__O4.__C4_n_initializer = (struct expr *)( (_au0__Xthis__ctor_ival = 0 ), ( ( (_au0__Xthis__ctor_ival = 0 ), (_au0__Xthis__ctor_ival = (struct ival *)_expr__ctor ( ((struct
#line 268 "../../src/expr2.c"
expr *)_au0__Xthis__ctor_ival ), (unsigned char )150 , (struct expr *)0 , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ival -> _expr__O3.__C3_i1 = _au1_n ->
#line 268 "../../src/expr2.c"
_name_n_val ), ((_au0__Xthis__ctor_ival ))) ) ) ;
}
(*_au6_nn )= (*_au1_n );

#line 272 "../../src/expr2.c"
_au6_nn -> _name_n_sto = 31 ;
_au6_nn -> _name_n_list = dcl_list ;
dcl_list = _au6_nn ;
}
break ;
}
case 70 : 
#line 279 "../../src/expr2.c"
_au1_n -> _name_n_used -- ;
_name_assign ( _au1_n ) ;
break ;
default : 
#line 283 "../../src/expr2.c"
if (cc -> _dcl_context_tot && (_au1_n == cc -> _dcl_context_c_this )){ 
#line 284 "../../src/expr2.c"
{ 
#line 292 "../../src/expr2.c"
struct ea _au0__V34 ;

#line 292 "../../src/expr2.c"
struct ea _au0__V35 ;

#line 284 "../../src/expr2.c"
error ( (char *)"%n%k", (struct ea *)( ( ((& _au0__V34 )-> _ea__O1.__C1_p = ((char *)_au1_n )), (((& _au0__V34 )))) )
#line 284 "../../src/expr2.c"
, (struct ea *)( ( ((& _au0__V35 )-> _ea__O1.__C1_i = ((int )_au0_oper )), (((& _au0__V35 )))) ) , (struct
#line 284 "../../src/expr2.c"
ea *)ea0 , (struct ea *)ea0 ) ;
return (int )0 ;
} }
_name_assign ( _au1_n ) ;
}
return 1 ;
}
}
}
;
Pexpr Ninit ;
int Nstd = 0 ;

#line 297 "../../src/expr2.c"
bit gen_match (_au0_n , _au0_arg )Pname _au0_n ;

#line 297 "../../src/expr2.c"
Pexpr _au0_arg ;

#line 301 "../../src/expr2.c"
{ 
#line 302 "../../src/expr2.c"
Pfct _au1_f ;
register Pexpr _au1_e ;
register Pname _au1_nn ;

#line 302 "../../src/expr2.c"
_au1_f = (((struct fct *)_au0_n -> _expr__O2.__C2_tp ));

#line 306 "../../src/expr2.c"
for(( (_au1_e = _au0_arg ), (_au1_nn = _au1_f -> _fct_argtype )) ;_au1_e ;( (_au1_e = _au1_e -> _expr__O4.__C4_e2 ), (_au1_nn = _au1_nn -> _name_n_list )) )
#line 306 "../../src/expr2.c"
{ 
#line 307 "../../src/expr2.c"
Pexpr _au2_a ;
Ptype _au2_at ;

#line 312 "../../src/expr2.c"
Ptype _au2_nt ;

#line 307 "../../src/expr2.c"
_au2_a = _au1_e -> _expr__O3.__C3_e1 ;
_au2_at = _au2_a -> _expr__O2.__C2_tp ;
if (_au2_at -> _node_base == 141 )return (char )0 ;
if (_au1_nn == 0 )return (char )(_au1_f -> _fct_nargs_known == 155 );

#line 312 "../../src/expr2.c"
_au2_nt = _au1_nn -> _expr__O2.__C2_tp ;

#line 314 "../../src/expr2.c"
switch (_au2_nt -> _node_base ){ 
#line 315 "../../src/expr2.c"
case 158 : 
#line 316 "../../src/expr2.c"
if (_au2_at == (struct type *)zero_type )return (char )0 ;
if (_type_check ( _au2_nt , _au2_at , (unsigned char )78 ) ){ 
#line 318 "../../src/expr2.c"
Pptr _au4_pt ;

#line 319 "../../src/expr2.c"
struct ptr *_au0__Xthis__ctor_ptr ;

#line 318 "../../src/expr2.c"
_au4_pt = ( (((struct ptr *)( (_au0__Xthis__ctor_ptr = 0 ), ( (_au0__Xthis__ctor_ptr = (struct ptr *)_new ( (long )(sizeof (struct ptr))) ),
#line 318 "../../src/expr2.c"
( (Nt ++ ), ( (_au0__Xthis__ctor_ptr -> _node_base = ((unsigned char )125 )), ( (_au0__Xthis__ctor_ptr -> _pvtyp_typ = _au2_at ), (
#line 318 "../../src/expr2.c"
(_au0__Xthis__ctor_ptr -> _ptr_rdo = ((char )0 )), ((_au0__Xthis__ctor_ptr ))) ) ) ) ) ) ))) ;
_au2_nt -> _node_base = 125 ;
if (_type_check ( _au2_nt , (struct type *)_au4_pt , (unsigned char )78 ) ){ 
#line 321 "../../src/expr2.c"
_au2_nt -> _node_base = 158 ;
_delete ( (char *)_au4_pt ) ;
return (char )0 ;
}
_au2_nt -> _node_base = 158 ;
_delete ( (char *)_au4_pt ) ;
}
break ;
default : 
#line 330 "../../src/expr2.c"
switch (_au2_at -> _node_base )
#line 331 "../../src/expr2.c"
{ 
#line 332 "../../src/expr2.c"
default : 
#line 333 "../../src/expr2.c"
if (_type_check ( _au2_nt , _au2_at , (unsigned char )78 )
#line 333 "../../src/expr2.c"
)return (char )0 ;
break ;
case 76 : 
#line 336 "../../src/expr2.c"
{ 
#line 337 "../../src/expr2.c"
register Plist _au5_gl ;
Pgen _au5_g ;
int _au5_no_match ;

#line 338 "../../src/expr2.c"
_au5_g = (((struct gen *)_au2_at ));
_au5_no_match = 1 ;

#line 341 "../../src/expr2.c"
for(_au5_gl = _au5_g -> _gen_fct_list ;_au5_gl ;_au5_gl = _au5_gl -> _name_list_l ) 
#line 342 "../../src/expr2.c"
{ 
#line 343 "../../src/expr2.c"
Pname _au6_nn ;
Ptype _au6_t ;

#line 343 "../../src/expr2.c"
_au6_nn = _au5_gl -> _name_list_f ;
_au6_t = _au6_nn -> _expr__O2.__C2_tp ;
if (_type_check ( _au2_nt , _au6_t , (unsigned char )78 ) == 0 )
#line 346 "../../src/expr2.c"
{ _au5_no_match = 0 ;

#line 346 "../../src/expr2.c"
break ;
}
}
if (_au5_no_match )return (char )0 ;
}
}
}
}

#line 355 "../../src/expr2.c"
if (_au1_nn ){ 
#line 356 "../../src/expr2.c"
Ninit = _au1_nn -> _expr__O4.__C4_n_initializer ;
return (char )(Ninit != 0 );
}

#line 360 "../../src/expr2.c"
return (char )1 ;
}
;
Pname Ncoerce ;

#line 365 "../../src/expr2.c"
extern bit can_coerce (_au0_t1 , _au0_t2 )Ptype _au0_t1 ;

#line 365 "../../src/expr2.c"
Ptype _au0_t2 ;

#line 369 "../../src/expr2.c"
{ 
#line 370 "../../src/expr2.c"
Ncoerce = 0 ;
if (_au0_t2 -> _node_base == 141 )return (char )0 ;

#line 373 "../../src/expr2.c"
switch (_au0_t1 -> _node_base ){ 
#line 374 "../../src/expr2.c"
case 158 : 
#line 375 "../../src/expr2.c"
rloop :
#line 376 "../../src/expr2.c"
switch (_au0_t2 -> _node_base ){ 
#line 377 "../../src/expr2.c"
case 97 : 
#line 378 "../../src/expr2.c"
_au0_t2 = (((struct basetype *)_au0_t2 ))-> _basetype_b_name ->
#line 378 "../../src/expr2.c"
_expr__O2.__C2_tp ;
goto rloop ;

#line 384 "../../src/expr2.c"
default : 
#line 385 "../../src/expr2.c"
{ Ptype _au4_tt2 ;

#line 386 "../../src/expr2.c"
struct ptr *_au0__Xthis__ctor_ptr ;

#line 385 "../../src/expr2.c"
_au4_tt2 = (struct type *)( (((struct ptr *)( (_au0__Xthis__ctor_ptr = 0 ), ( (_au0__Xthis__ctor_ptr = (struct ptr *)_new ( (long )(sizeof (struct ptr)))
#line 385 "../../src/expr2.c"
), ( (Nt ++ ), ( (_au0__Xthis__ctor_ptr -> _node_base = ((unsigned char )125 )), ( (_au0__Xthis__ctor_ptr -> _pvtyp_typ = _au0_t2 ),
#line 385 "../../src/expr2.c"
( (_au0__Xthis__ctor_ptr -> _ptr_rdo = ((char )0 )), ((_au0__Xthis__ctor_ptr ))) ) ) ) ) ) ))) ;
if (_type_check ( _au0_t1 , _au4_tt2 , (unsigned char )78 ) == 0 )return (char )1 ;
{ Ptype _au4_tt1 ;
int _au4_i ;

#line 387 "../../src/expr2.c"
_au4_tt1 = (((struct ptr *)_au0_t1 ))-> _pvtyp_typ ;
_au4_i = can_coerce ( _au4_tt1 , _au0_t2 ) ;
return (char )_au4_i ;
}
}
}
}
{ Pname _au1_c1 ;
Pname _au1_c2 ;
int _au1_val ;

#line 394 "../../src/expr2.c"
_au1_c1 = _type_is_cl_obj ( _au0_t1 ) ;
_au1_c2 = _type_is_cl_obj ( _au0_t2 ) ;
_au1_val = 0 ;

#line 398 "../../src/expr2.c"
if (_au1_c1 ){ 
#line 399 "../../src/expr2.c"
Pclass _au2_cl ;

#line 399 "../../src/expr2.c"
_au2_cl = (((struct classdef *)_au1_c1 -> _expr__O2.__C2_tp ));
if (_au1_c2 && (_au1_c2 -> _expr__O2.__C2_tp == (struct type *)_au2_cl ))return (char )1 ;

#line 407 "../../src/expr2.c"
{ Pname _au2_ctor ;

#line 407 "../../src/expr2.c"
_au2_ctor = ( _table_look ( _au2_cl -> _classdef_memtbl , "_ctor", (unsigned char )0 ) ) ;
if (_au2_ctor == 0 )goto oper_coerce ;
{ register Pfct _au2_f ;

#line 409 "../../src/expr2.c"
_au2_f = (((struct fct *)_au2_ctor -> _expr__O2.__C2_tp ));

#line 411 "../../src/expr2.c"
switch (_au2_f -> _node_base ){ 
#line 412 "../../src/expr2.c"
case 108 : 
#line 413 "../../src/expr2.c"
switch (_au2_f -> _fct_nargs ){ 
#line 414 "../../src/expr2.c"
case 1 : 
#line 415 "../../src/expr2.c"
one :
#line 416 "../../src/expr2.c"
{ Ptype _au5_tt ;

#line 416 "../../src/expr2.c"
_au5_tt = _au2_f -> _fct_argtype -> _expr__O2.__C2_tp ;
if (_type_check ( _au5_tt , _au0_t2 , (unsigned char )78 ) == 0 )_au1_val = 1 ;
if (_au5_tt -> _node_base == 158 ){ 
#line 419 "../../src/expr2.c"
Pptr _au6_pt ;

#line 420 "../../src/expr2.c"
struct ptr *_au0__Xthis__ctor_ptr ;

#line 419 "../../src/expr2.c"
_au6_pt = ( (((struct ptr *)( (_au0__Xthis__ctor_ptr = 0 ), ( (_au0__Xthis__ctor_ptr = (struct ptr *)_new ( (long )(sizeof (struct ptr))) ),
#line 419 "../../src/expr2.c"
( (Nt ++ ), ( (_au0__Xthis__ctor_ptr -> _node_base = ((unsigned char )125 )), ( (_au0__Xthis__ctor_ptr -> _pvtyp_typ = _au0_t2 ), (
#line 419 "../../src/expr2.c"
(_au0__Xthis__ctor_ptr -> _ptr_rdo = ((char )0 )), ((_au0__Xthis__ctor_ptr ))) ) ) ) ) ) ))) ;
_au5_tt -> _node_base = 125 ;
if (_type_check ( _au5_tt , (struct type *)_au6_pt , (unsigned char )78 ) == 0 )_au1_val = 1 ;
_au5_tt -> _node_base = 158 ;
_delete ( (char *)_au6_pt ) ;
}
goto oper_coerce ;
}
default : 
#line 428 "../../src/expr2.c"
if (_au2_f -> _fct_argtype -> _name_n_list -> _expr__O4.__C4_n_initializer )goto one ;
case 0 : 
#line 430 "../../src/expr2.c"
goto oper_coerce ;
}
case 76 : 
#line 433 "../../src/expr2.c"
{ register Plist _au4_gl ;

#line 435 "../../src/expr2.c"
for(_au4_gl = (((struct gen *)_au2_f ))-> _gen_fct_list ;_au4_gl ;_au4_gl = _au4_gl -> _name_list_l ) { 
#line 436 "../../src/expr2.c"
Pname _au5_nn ;
Pfct _au5_ff ;

#line 436 "../../src/expr2.c"
_au5_nn = _au4_gl -> _name_list_f ;
_au5_ff = (((struct fct *)_au5_nn -> _expr__O2.__C2_tp ));
switch (_au5_ff -> _fct_nargs ){ 
#line 439 "../../src/expr2.c"
case 0 : 
#line 440 "../../src/expr2.c"
break ;
case 1 : 
#line 442 "../../src/expr2.c"
over_one :
#line 443 "../../src/expr2.c"
{ Ptype _au7_tt ;

#line 443 "../../src/expr2.c"
_au7_tt = _au5_ff -> _fct_argtype -> _expr__O2.__C2_tp ;
if (_type_check ( _au7_tt , _au0_t2 , (unsigned char )78 ) == 0 )_au1_val = 1 ;
if (_au7_tt -> _node_base == 158 ){ 
#line 446 "../../src/expr2.c"
Pptr _au8_pt ;

#line 447 "../../src/expr2.c"
struct ptr *_au0__Xthis__ctor_ptr ;

#line 446 "../../src/expr2.c"
_au8_pt = ( (((struct ptr *)( (_au0__Xthis__ctor_ptr = 0 ), ( (_au0__Xthis__ctor_ptr = (struct ptr *)_new ( (long )(sizeof (struct ptr))) ),
#line 446 "../../src/expr2.c"
( (Nt ++ ), ( (_au0__Xthis__ctor_ptr -> _node_base = ((unsigned char )125 )), ( (_au0__Xthis__ctor_ptr -> _pvtyp_typ = _au0_t2 ), (
#line 446 "../../src/expr2.c"
(_au0__Xthis__ctor_ptr -> _ptr_rdo = ((char )0 )), ((_au0__Xthis__ctor_ptr ))) ) ) ) ) ) ))) ;
_au7_tt -> _node_base = 125 ;
if (_type_check ( _au7_tt , (struct type *)_au8_pt , (unsigned char )78 ) == 0 ){ 
#line 449 "../../src/expr2.c"
_au7_tt -> _node_base = 158 ;
_delete ( (char *)_au8_pt ) ;
_au1_val = 1 ;
goto oper_coerce ;
}
_au7_tt -> _node_base = 158 ;
_delete ( (char *)_au8_pt ) ;
}
break ;
}
default : 
#line 460 "../../src/expr2.c"
if (_au5_ff -> _fct_argtype -> _name_n_list -> _expr__O4.__C4_n_initializer )goto over_one ;
}
}
goto oper_coerce ;
}
default : 
#line 466 "../../src/expr2.c"
{ 
#line 502 "../../src/expr2.c"
struct ea _au0__V36 ;

#line 466 "../../src/expr2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"cannot_coerce(%k)\n", (struct ea *)( ( ((& _au0__V36 )-> _ea__O1.__C1_i = ((int )_au2_f -> _node_base )),
#line 466 "../../src/expr2.c"
(((& _au0__V36 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}
}
}

#line 469 "../../src/expr2.c"
oper_coerce :
#line 470 "../../src/expr2.c"
if (_au1_c2 ){ 
#line 471 "../../src/expr2.c"
Pclass _au2_cl ;
int _au2_std ;

#line 471 "../../src/expr2.c"
_au2_cl = (((struct classdef *)_au1_c2 -> _expr__O2.__C2_tp ));
_au2_std = 0 ;
{ register Pname _au2_on ;

#line 473 "../../src/expr2.c"
_au2_on = _au2_cl -> _classdef_conv ;

#line 473 "../../src/expr2.c"
for(;_au2_on ;_au2_on = _au2_on -> _name_n_list ) { 
#line 475 "../../src/expr2.c"
Pfct _au3_f ;

#line 475 "../../src/expr2.c"
_au3_f = (((struct fct *)_au2_on -> _expr__O2.__C2_tp ));
Nstd = 0 ;
if (_type_check ( _au0_t1 , _au3_f -> _fct_returns , (unsigned char )78 ) == 0 ){ 
#line 478 "../../src/expr2.c"
if (Nstd == 0 ){ 
#line 479 "../../src/expr2.c"
if
#line 479 "../../src/expr2.c"
(_au2_std ){ 
#line 480 "../../src/expr2.c"
_au1_val = 1 ;
_au2_std = 0 ;
}
else 
#line 484 "../../src/expr2.c"
_au1_val ++ ;
Ncoerce = _au2_on ;
}
else { 
#line 488 "../../src/expr2.c"
if ((_au1_val == 0 )|| _au2_std ){ 
#line 489 "../../src/expr2.c"
Ncoerce = _au2_on ;
_au1_val ++ ;
_au2_std = 1 ;
}
}
}
}
}
}
if (_au1_val )return (char )_au1_val ;
if (_au1_c1 && ( (((struct classdef *)_au1_c1 -> _expr__O2.__C2_tp ))-> _classdef_itor ) )return (char )0 ;
if (_type_check ( _au0_t1 , _au0_t2 , (unsigned char )78 ) )return (char )0 ;
return (char )1 ;
}
}
;

#line 504 "../../src/expr2.c"
int gen_coerce (_au0_n , _au0_arg )Pname _au0_n ;

#line 504 "../../src/expr2.c"
Pexpr _au0_arg ;

#line 510 "../../src/expr2.c"
{ 
#line 511 "../../src/expr2.c"
Pfct _au1_f ;
register Pexpr _au1_e ;
register Pname _au1_nn ;

#line 511 "../../src/expr2.c"
_au1_f = (((struct fct *)_au0_n -> _expr__O2.__C2_tp ));

#line 515 "../../src/expr2.c"
for(( (_au1_e = _au0_arg ), (_au1_nn = _au1_f -> _fct_argtype )) ;_au1_e ;( (_au1_e = _au1_e -> _expr__O4.__C4_e2 ), (_au1_nn = _au1_nn -> _name_n_list )) )
#line 515 "../../src/expr2.c"
{ 
#line 516 "../../src/expr2.c"
if (_au1_nn == 0 )return (_au1_f -> _fct_nargs_known == 155 );
{ Pexpr _au2_a ;
Ptype _au2_at ;
int _au2_i ;

#line 517 "../../src/expr2.c"
_au2_a = _au1_e -> _expr__O3.__C3_e1 ;
_au2_at = _au2_a -> _expr__O2.__C2_tp ;
_au2_i = can_coerce ( _au1_nn -> _expr__O2.__C2_tp , _au2_at ) ;

#line 521 "../../src/expr2.c"
if (_au2_i != 1 )return (int )0 ;
}
}

#line 523 "../../src/expr2.c"
if (_au1_nn && (_au1_nn -> _expr__O4.__C4_n_initializer == 0 ))return (int )0 ;
return 1 ;
}
;

#line 528 "../../src/expr2.c"
Pname Nover ;
int Nover_coerce = 0 ;

#line 531 "../../src/expr2.c"
extern int over_call (_au0_n , _au0_arg )Pname _au0_n ;

#line 531 "../../src/expr2.c"
Pexpr _au0_arg ;

#line 538 "../../src/expr2.c"
{ 
#line 539 "../../src/expr2.c"
register Plist _au1_gl ;
Pgen _au1_g ;

#line 554 "../../src/expr2.c"
Pname _au1_exact ;
int _au1_no_exact ;

#line 540 "../../src/expr2.c"
_au1_g = (((struct gen *)_au0_n -> _expr__O2.__C2_tp ));
if (_au0_arg && (_au0_arg -> _node_base != 140 ))errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"ALX", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 541 "../../src/expr2.c"
ea *)ea0 , (struct ea *)ea0 ) ;

#line 543 "../../src/expr2.c"
Nover_coerce = 0 ;
switch (_au1_g -> _node_base ){ 
#line 545 "../../src/expr2.c"
default : { 
#line 593 "../../src/expr2.c"
struct ea _au0__V37 ;

#line 545 "../../src/expr2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"over_call(%t)\n", (struct ea *)( ( ((& _au0__V37 )-> _ea__O1.__C1_p = ((char *)_au1_g )), (((&
#line 545 "../../src/expr2.c"
_au0__V37 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
case 76 : break ;
case 108 : 
#line 548 "../../src/expr2.c"
Nover = _au0_n ;
Ninit = 0 ;
if (gen_match ( _au0_n , _au0_arg ) && (Ninit == 0 ))return 2 ;
return gen_coerce ( _au0_n , _au0_arg ) ;
} }

#line 554 "../../src/expr2.c"
_au1_exact = 0 ;

#line 554 "../../src/expr2.c"
_au1_no_exact = 0 ;

#line 556 "../../src/expr2.c"
for(_au1_gl = _au1_g -> _gen_fct_list ;_au1_gl ;_au1_gl = _au1_gl -> _name_list_l ) { 
#line 557 "../../src/expr2.c"
Nover = _au1_gl -> _name_list_f ;
Ninit = 0 ;
Nstd = 0 ;

#line 561 "../../src/expr2.c"
if (gen_match ( Nover , _au0_arg ) && (Ninit == 0 )){ 
#line 563 "../../src/expr2.c"
if (Nstd == 0 )return 2 ;
if (_au1_exact )
#line 565 "../../src/expr2.c"
_au1_no_exact ++ ;
else 
#line 567 "../../src/expr2.c"
_au1_exact = Nover ;
}
}

#line 572 "../../src/expr2.c"
if (_au1_exact ){ 
#line 574 "../../src/expr2.c"
if (_au1_no_exact ){ 
#line 593 "../../src/expr2.c"
struct ea _au0__V38 ;

#line 574 "../../src/expr2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"more than one standard conversion possible for%n", (struct ea *)( ( ((& _au0__V38 )-> _ea__O1.__C1_p = ((char *)_au0_n )), (((&
#line 574 "../../src/expr2.c"
_au0__V38 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} Nover = _au1_exact ;
return 2 ;
}

#line 579 "../../src/expr2.c"
Nover = 0 ;
for(_au1_gl = _au1_g -> _gen_fct_list ;_au1_gl ;_au1_gl = _au1_gl -> _name_list_l ) { 
#line 581 "../../src/expr2.c"
Pname _au2_nn ;

#line 581 "../../src/expr2.c"
_au2_nn = _au1_gl -> _name_list_f ;

#line 583 "../../src/expr2.c"
if (gen_coerce ( _au2_nn , _au0_arg ) ){ 
#line 584 "../../src/expr2.c"
if (Nover ){ 
#line 585 "../../src/expr2.c"
Nover_coerce = 2 ;
return (int )0 ;
}
Nover = _au2_nn ;
}
}

#line 592 "../../src/expr2.c"
return (Nover ? 1 : 0);
}
;
char visible_check (_au0_found , _au0_string , _au0_e , _au0_no_virt )Pname _au0_found ;

#line 595 "../../src/expr2.c"
char *_au0_string ;

#line 595 "../../src/expr2.c"
Pexpr _au0_e ;

#line 595 "../../src/expr2.c"
bit _au0_no_virt ;

#line 599 "../../src/expr2.c"
{ 
#line 601 "../../src/expr2.c"
Pexpr _au1_e1 ;
Pbase _au1_b ;

#line 601 "../../src/expr2.c"
_au1_e1 = _au0_e -> _expr__O3.__C3_e1 ;

#line 604 "../../src/expr2.c"
if (_au0_e -> _node_base == 23 ){ 
#line 605 "../../src/expr2.c"
if (_au0_found -> _expr__O2.__C2_tp -> _node_base == 76 ){ 
#line 606 "../../src/expr2.c"
Ninit = 0 ;

#line 606 "../../src/expr2.c"
{ int _au3_no_match ;
Plist _au3_gl ;

#line 606 "../../src/expr2.c"
_au3_no_match = 1 ;
for(_au3_gl = (((struct gen *)_au0_found -> _expr__O2.__C2_tp ))-> _gen_fct_list ;_au3_gl ;_au3_gl = _au3_gl -> _name_list_l ) 
#line 608 "../../src/expr2.c"
{ 
#line 610 "../../src/expr2.c"
if (_au0_e -> _expr__O3.__C3_e1 == 0 ){ 
#line 611 "../../src/expr2.c"
if ((((struct
#line 611 "../../src/expr2.c"
fct *)_au3_gl -> _name_list_f -> _expr__O2.__C2_tp ))-> _fct_nargs_known && ((((struct fct *)_au3_gl -> _name_list_f -> _expr__O2.__C2_tp ))-> _fct_nargs == 0 ))
#line 613 "../../src/expr2.c"
{ _au0_found = _au3_gl -> _name_list_f ;

#line 613 "../../src/expr2.c"
break ;
}
else 
#line 614 "../../src/expr2.c"
continue ;
}

#line 617 "../../src/expr2.c"
if (gen_match ( _au3_gl -> _name_list_f , _au0_e ) && (Ninit == 0 ))
#line 618 "../../src/expr2.c"
{ _au0_found = _au3_gl -> _name_list_f ;

#line 618 "../../src/expr2.c"
break ;
}
if (_au3_no_match && gen_coerce ( _au3_gl -> _name_list_f , _au0_e ) )
#line 621 "../../src/expr2.c"
{ _au3_no_match = 0 ;

#line 621 "../../src/expr2.c"
_au0_found = _au3_gl -> _name_list_f ;
}
}
}
}

#line 624 "../../src/expr2.c"
{ Pfct _au2_fn ;

#line 627 "../../src/expr2.c"
Ptype _au2_pt ;

#line 624 "../../src/expr2.c"
_au2_fn = ((_au0_found -> _expr__O2.__C2_tp -> _node_base == 76 )? (((struct fct *)(((struct gen *)_au0_found -> _expr__O2.__C2_tp ))-> _gen_fct_list -> _name_list_f -> _expr__O2.__C2_tp )): (((struct fct *)_au0_found ->
#line 624 "../../src/expr2.c"
_expr__O2.__C2_tp )));

#line 627 "../../src/expr2.c"
_au2_pt = _au2_fn -> _fct_s_returns ;
_au1_b = (((struct basetype *)(((struct ptr *)_au2_pt ))-> _pvtyp_typ ));
goto xxxx ;
}
}
if (_au1_e1 )
#line 633 "../../src/expr2.c"
switch (_au1_e1 -> _node_base ){ 
#line 634 "../../src/expr2.c"
default : 
#line 635 "../../src/expr2.c"
if (_au0_no_virt )_au0_e -> _expr__O3.__C3_e1 = (struct expr *)_au0_found ;
return ;

#line 638 "../../src/expr2.c"
case 44 : 
#line 639 "../../src/expr2.c"
{ if (_au0_no_virt )_au1_e1 -> _expr__O5.__C5_mem = _au0_found ;
if (_au1_e1 -> _expr__O3.__C3_e1 == 0 )return ;
{ Ptype _au3_pt ;

#line 641 "../../src/expr2.c"
_au3_pt = _au1_e1 -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp ;

#line 641 "../../src/expr2.c"
for(;_au3_pt -> _node_base == 97 ;_au3_pt = (((struct basetype *)_au3_pt ))-> _basetype_b_name -> _expr__O2.__C2_tp ) ;
_au1_b = (((struct basetype *)(((struct ptr *)_au3_pt ))-> _pvtyp_typ ));
break ;
}
}

#line 645 "../../src/expr2.c"
case 45 : 
#line 646 "../../src/expr2.c"
if (_au0_no_virt )_au1_e1 -> _expr__O5.__C5_mem = _au0_found ;
_au1_b = (((struct basetype *)_au1_e1 -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp ));
}

#line 650 "../../src/expr2.c"
xxxx :
#line 651 "../../src/expr2.c"
switch (_au1_b -> _node_base ){ 
#line 652 "../../src/expr2.c"
case 97 : _au1_b = (((struct basetype *)_au1_b -> _basetype_b_name -> _expr__O2.__C2_tp ));

#line 652 "../../src/expr2.c"
goto xxxx ;
case 141 : return ;
case 119 : break ;
default : { 
#line 676 "../../src/expr2.c"
struct ea _au0__V39 ;

#line 655 "../../src/expr2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"no tblx %p", (struct ea *)( ( ((& _au0__V39 )-> _ea__O1.__C1_p = ((char *)_au1_b )), (((&
#line 655 "../../src/expr2.c"
_au0__V39 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }

#line 658 "../../src/expr2.c"
{ Ptable _au1_tblx ;

#line 658 "../../src/expr2.c"
_au1_tblx = _au1_b -> _basetype_b_table ;
if (_au1_tblx -> _node_base != 142 ){ 
#line 676 "../../src/expr2.c"
struct ea _au0__V40 ;

#line 676 "../../src/expr2.c"
struct ea _au0__V41 ;

#line 659 "../../src/expr2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"tblx %p %d", (struct ea *)( ( ((& _au0__V40 )-> _ea__O1.__C1_p = ((char *)_au1_tblx )), (((&
#line 659 "../../src/expr2.c"
_au0__V40 )))) ) , (struct ea *)( ( ((& _au0__V41 )-> _ea__O1.__C1_i = ((int )_au1_tblx -> _node_base )), (((& _au0__V41 ))))
#line 659 "../../src/expr2.c"
) , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} if (_table_lookc ( _au1_tblx , _au0_string , (unsigned char )0 ) == 0 )return ;

#line 662 "../../src/expr2.c"
switch (_au0_found -> _name_n_scope ){ 
#line 663 "../../src/expr2.c"
case 0 : 
#line 664 "../../src/expr2.c"
if (((Epriv && (Epriv != cc -> _dcl_context_cot ))&& (! _classdef_has_friend ( Epriv , cc ->
#line 664 "../../src/expr2.c"
_dcl_context_nof ) ))&& (! (_au0_found -> _name_n_protect && _classdef_baseofFPCname___ ( Epriv , cc -> _dcl_context_nof ) )))
#line 667 "../../src/expr2.c"
{ 
#line 668 "../../src/expr2.c"
{ 
#line 676 "../../src/expr2.c"
struct ea _au0__V42 ;

#line 676 "../../src/expr2.c"
struct ea _au0__V43 ;

#line 668 "../../src/expr2.c"
error ( (char *)"%n is %s", (struct ea *)( ( ((& _au0__V42 )-> _ea__O1.__C1_p = ((char *)_au0_found )), (((& _au0__V42 )))) )
#line 668 "../../src/expr2.c"
, (struct ea *)( ( ((& _au0__V43 )-> _ea__O1.__C1_p = ((char *)(_au0_found -> _name_n_protect ? "protected": "private"))), (((& _au0__V43 ))))
#line 668 "../../src/expr2.c"
) , (struct ea *)ea0 , (struct ea *)ea0 ) ;
break ;
} }

#line 672 "../../src/expr2.c"
case 25 : 
#line 673 "../../src/expr2.c"
if (Ebase && ((cc -> _dcl_context_cot == 0 )|| ((Ebase != (struct classdef *)cc -> _dcl_context_cot -> _classdef_clbase -> _expr__O2.__C2_tp )&&
#line 673 "../../src/expr2.c"
(! _classdef_has_friend ( Ebase , cc -> _dcl_context_nof ) ))))
#line 674 "../../src/expr2.c"
{ 
#line 676 "../../src/expr2.c"
struct ea _au0__V44 ;

#line 674 "../../src/expr2.c"
error ( (char *)"%n is from a privateBC", (struct ea *)( ( ((& _au0__V44 )-> _ea__O1.__C1_p = ((char *)_au0_found )), (((& _au0__V44 )))) )
#line 674 "../../src/expr2.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}
}
;

#line 678 "../../src/expr2.c"
Ptype _expr_fct_call (_au0_this , _au0_tbl )
#line 525 "../../src/cfront.h"
struct expr *_au0_this ;

#line 678 "../../src/expr2.c"
Ptable _au0_tbl ;

#line 684 "../../src/expr2.c"
{ 
#line 685 "../../src/expr2.c"
Pfct _au1_f ;
Pname _au1_fn ;
int _au1_x ;
int _au1_k ;
Pname _au1_nn ;
Pexpr _au1_e ;
Ptype _au1_t ;
Pexpr _au1_arg ;
Ptype _au1_t1 ;
int _au1_argno ;
Pexpr _au1_etail ;
Pname _au1_no_virt ;

#line 692 "../../src/expr2.c"
_au1_arg = _au0_this -> _expr__O4.__C4_e2 ;

#line 695 "../../src/expr2.c"
_au1_etail = 0 ;

#line 698 "../../src/expr2.c"
switch (_au0_this -> _node_base ){ 
#line 699 "../../src/expr2.c"
case 109 : 
#line 700 "../../src/expr2.c"
case 146 : break ;
default : { 
#line 964 "../../src/expr2.c"
struct ea _au0__V45 ;

#line 701 "../../src/expr2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"fct_call(%k)", (struct ea *)( ( ((& _au0__V45 )-> _ea__O1.__C1_i = ((int )_au0_this -> _node_base )),
#line 701 "../../src/expr2.c"
(((& _au0__V45 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }

#line 704 "../../src/expr2.c"
if ((_au0_this -> _expr__O3.__C3_e1 == 0 )|| ((_au1_t1 = _au0_this -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp )== 0 )){ 
#line 964 "../../src/expr2.c"
struct ea _au0__V46 ;

#line 964 "../../src/expr2.c"
struct ea _au0__V47 ;

#line 704 "../../src/expr2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"fct_call(e1=%d,e1->tp=%t)", (struct ea *)( ( ((& _au0__V46 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _expr__O3.__C3_e1 )),
#line 704 "../../src/expr2.c"
(((& _au0__V46 )))) ) , (struct ea *)( ( ((& _au0__V47 )-> _ea__O1.__C1_p = ((char *)_au1_t1 )), (((& _au0__V47 ))))
#line 704 "../../src/expr2.c"
) , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} if (_au1_arg && (_au1_arg -> _node_base != 140 )){ 
#line 964 "../../src/expr2.c"
struct ea _au0__V48 ;

#line 964 "../../src/expr2.c"
struct ea _au0__V49 ;

#line 705 "../../src/expr2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"badAL%d%k", (struct ea *)( ( ((& _au0__V48 )-> _ea__O1.__C1_p = ((char *)_au1_arg )), (((&
#line 705 "../../src/expr2.c"
_au0__V48 )))) ) , (struct ea *)( ( ((& _au0__V49 )-> _ea__O1.__C1_i = ((int )_au1_arg -> _node_base )), (((& _au0__V49 ))))
#line 705 "../../src/expr2.c"
) , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} 
#line 708 "../../src/expr2.c"
switch (_au0_this -> _expr__O3.__C3_e1 -> _node_base ){ 
#line 709 "../../src/expr2.c"
case 85 : 
#line 710 "../../src/expr2.c"
_au1_fn = (((struct name *)_au0_this -> _expr__O3.__C3_e1 ));
switch (_au1_fn -> _name_n_oper ){ 
#line 712 "../../src/expr2.c"
case 0 : 
#line 713 "../../src/expr2.c"
case 161 : 
#line 714 "../../src/expr2.c"
case 162 : 
#line 715 "../../src/expr2.c"
case 97 : 
#line 716 "../../src/expr2.c"
break ;
default : 
#line 718 "../../src/expr2.c"
if (_au1_arg == 0 )break ;
{ Pexpr _au3_a ;

#line 719 "../../src/expr2.c"
_au3_a = _au1_arg -> _expr__O3.__C3_e1 ;
if (_type_is_cl_obj ( _au3_a -> _expr__O2.__C2_tp ) || _type_is_ref ( _au3_a -> _expr__O2.__C2_tp ) )break ;
_au3_a = _au1_arg -> _expr__O4.__C4_e2 ;
if (_au3_a == 0 )
#line 723 "../../src/expr2.c"
{ 
#line 964 "../../src/expr2.c"
struct ea _au0__V50 ;

#line 723 "../../src/expr2.c"
error ( (char *)"%k of basicT", (struct ea *)( ( ((& _au0__V50 )-> _ea__O1.__C1_i = ((int )_au1_fn -> _name_n_oper )), (((& _au0__V50 ))))
#line 723 "../../src/expr2.c"
) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} else { 
#line 725 "../../src/expr2.c"
_au3_a = _au3_a -> _expr__O3.__C3_e1 ;
if (_type_is_cl_obj ( _au3_a -> _expr__O2.__C2_tp ) || _type_is_ref ( _au3_a -> _expr__O2.__C2_tp ) )break ;
{ 
#line 964 "../../src/expr2.c"
struct ea _au0__V51 ;

#line 727 "../../src/expr2.c"
error ( (char *)"%k of basicTs", (struct ea *)( ( ((& _au0__V51 )-> _ea__O1.__C1_i = ((int )_au1_fn -> _name_n_oper )), (((& _au0__V51 ))))
#line 727 "../../src/expr2.c"
) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
break ;
}
}

#line 731 "../../src/expr2.c"
_au1_no_virt = _au1_fn -> _name__O6.__C6_n_qualifier ;
break ;
case 44 : 
#line 734 "../../src/expr2.c"
case 45 : 
#line 735 "../../src/expr2.c"
_au1_fn = _au0_this -> _expr__O3.__C3_e1 -> _expr__O5.__C5_mem ;
_au1_no_virt = _au1_fn -> _name__O6.__C6_n_qualifier ;
break ;
case 173 : 
#line 739 "../../src/expr2.c"
default : 
#line 740 "../../src/expr2.c"
_au1_fn = 0 ;
_au1_no_virt = 0 ;
}

#line 742 "../../src/expr2.c"
;

#line 744 "../../src/expr2.c"
lll :
#line 745 "../../src/expr2.c"
switch (_au1_t1 -> _node_base ){ 
#line 746 "../../src/expr2.c"
case 97 : 
#line 747 "../../src/expr2.c"
_au1_t1 = (((struct basetype *)_au1_t1 ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
goto lll ;

#line 750 "../../src/expr2.c"
case 125 : 
#line 751 "../../src/expr2.c"
if ((((struct ptr *)_au1_t1 ))-> _pvtyp_typ -> _node_base == 108 ){ 
#line 752 "../../src/expr2.c"
if ((((struct ptr *)_au1_t1 ))-> _ptr_memof )error ( (char *)"O missing in call throughP toMF",
#line 752 "../../src/expr2.c"
(struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
_au1_t1 = (((struct ptr *)_au1_t1 ))-> _pvtyp_typ ;
_au1_fn = 0 ;
goto case_fct ;
}

#line 758 "../../src/expr2.c"
default : 
#line 759 "../../src/expr2.c"
{ 
#line 964 "../../src/expr2.c"
struct ea _au0__V52 ;

#line 964 "../../src/expr2.c"
struct ea _au0__V53 ;

#line 964 "../../src/expr2.c"
struct ea _au0__V54 ;

#line 759 "../../src/expr2.c"
error ( (char *)"call of%n;%n is a%t", (struct ea *)( ( ((& _au0__V52 )-> _ea__O1.__C1_p = ((char *)_au1_fn )), (((& _au0__V52 )))) )
#line 759 "../../src/expr2.c"
, (struct ea *)( ( ((& _au0__V53 )-> _ea__O1.__C1_p = ((char *)_au1_fn )), (((& _au0__V53 )))) ) , (struct
#line 759 "../../src/expr2.c"
ea *)( ( ((& _au0__V54 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp )), (((& _au0__V54 )))) ) , (struct
#line 759 "../../src/expr2.c"
ea *)ea0 ) ;

#line 761 "../../src/expr2.c"
case 141 : 
#line 762 "../../src/expr2.c"
return (struct type *)any_type ;

#line 764 "../../src/expr2.c"
case 76 : 
#line 765 "../../src/expr2.c"
{ register Plist _au3_gl ;
Pgen _au3_g ;
Pname _au3_found ;
Pname _au3_exact ;
int _au3_no_exact ;

#line 766 "../../src/expr2.c"
_au3_g = (((struct gen *)_au1_t1 ));
_au3_found = 0 ;
_au3_exact = 0 ;
_au3_no_exact = 0 ;

#line 771 "../../src/expr2.c"
for(_au3_gl = _au3_g -> _gen_fct_list ;_au3_gl ;_au3_gl = _au3_gl -> _name_list_l ) { 
#line 772 "../../src/expr2.c"
register Pname _au4_nn ;

#line 772 "../../src/expr2.c"
_au4_nn = _au3_gl -> _name_list_f ;
Ninit = 0 ;
Nstd = 0 ;

#line 776 "../../src/expr2.c"
if (gen_match ( _au4_nn , _au1_arg ) ){ 
#line 777 "../../src/expr2.c"
if (Nstd == 0 ){ 
#line 778 "../../src/expr2.c"
_au3_found = _au4_nn ;
goto fnd ;
}
if (_au3_exact )
#line 782 "../../src/expr2.c"
_au3_no_exact ++ ;
else 
#line 784 "../../src/expr2.c"
_au3_exact = _au4_nn ;
}
}

#line 789 "../../src/expr2.c"
if (_au3_exact ){ 
#line 790 "../../src/expr2.c"
if (_au3_no_exact ){ 
#line 964 "../../src/expr2.c"
struct ea _au0__V55 ;

#line 964 "../../src/expr2.c"
struct ea _au0__V56 ;

#line 790 "../../src/expr2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"%d standard conversion possible for%n", (struct ea *)( ( ((& _au0__V55 )-> _ea__O1.__C1_i = (_au3_no_exact + 1 )), (((&
#line 790 "../../src/expr2.c"
_au0__V55 )))) ) , (struct ea *)( ( ((& _au0__V56 )-> _ea__O1.__C1_p = ((char *)_au1_fn )), (((& _au0__V56 )))) )
#line 790 "../../src/expr2.c"
, (struct ea *)ea0 , (struct ea *)ea0 ) ;
} _au3_found = _au3_exact ;
goto fnd ;
}

#line 796 "../../src/expr2.c"
for(_au3_gl = _au3_g -> _gen_fct_list ;_au3_gl ;_au3_gl = _au3_gl -> _name_list_l ) { 
#line 797 "../../src/expr2.c"
register Pname _au4_nn ;

#line 797 "../../src/expr2.c"
_au4_nn = _au3_gl -> _name_list_f ;

#line 799 "../../src/expr2.c"
if (gen_coerce ( _au4_nn , _au1_arg ) ){ 
#line 800 "../../src/expr2.c"
if (_au3_found ){ 
#line 801 "../../src/expr2.c"
{ 
#line 964 "../../src/expr2.c"
struct ea _au0__V57 ;

#line 801 "../../src/expr2.c"
error ( (char *)"ambiguousA for overloaded%n", (struct ea *)( ( ((& _au0__V57 )-> _ea__O1.__C1_p = ((char *)_au1_fn )), (((& _au0__V57 )))) )
#line 801 "../../src/expr2.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
goto fnd ;
} }
_au3_found = _au4_nn ;
}
}

#line 808 "../../src/expr2.c"
fnd :
#line 809 "../../src/expr2.c"
if (_au0_this -> _expr__O5.__C5_fct_name = _au3_found ){ 
#line 811 "../../src/expr2.c"
_au1_f = (((struct fct *)_au3_found -> _expr__O2.__C2_tp ));
visible_check ( _au3_found , _au3_g -> _gen_string , _au0_this , (char )(_au1_no_virt != 0 )) ;
}
else { 
#line 815 "../../src/expr2.c"
{ 
#line 964 "../../src/expr2.c"
struct ea _au0__V58 ;

#line 815 "../../src/expr2.c"
error ( (char *)"badAL for overloaded%n", (struct ea *)( ( ((& _au0__V58 )-> _ea__O1.__C1_p = ((char *)_au1_fn )), (((& _au0__V58 )))) )
#line 815 "../../src/expr2.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
return (struct type *)any_type ;
} }
break ;
}
case 108 : 
#line 821 "../../src/expr2.c"
case_fct :
#line 823 "../../src/expr2.c"
_au1_f = (((struct fct *)_au1_t1 ));
if ((_au0_this -> _expr__O5.__C5_fct_name = _au1_fn )&& (_au1_fn -> _name_n_oper == 161 )){ 
#line 825 "../../src/expr2.c"
visible_check ( _au1_fn , _au1_fn -> _expr__O3.__C3_string , _au0_this , (char )0 )
#line 825 "../../src/expr2.c"
;
}
} }

#line 829 "../../src/expr2.c"
if (_au1_no_virt )_au0_this -> _expr__O5.__C5_fct_name = 0 ;

#line 831 "../../src/expr2.c"
_au1_t = _au1_f -> _fct_returns ;
_au1_x = _au1_f -> _fct_nargs ;
_au1_k = _au1_f -> _fct_nargs_known ;

#line 836 "../../src/expr2.c"
if (_au1_k == 0 ){ 
#line 837 "../../src/expr2.c"
if (((fct_void && _au1_fn )&& (_au1_x == 0 ))&& _au1_arg )
#line 838 "../../src/expr2.c"
if ((no_of_badcall ++ )== 0 )badcall = _au1_fn ;
goto rlab ;
}

#line 842 "../../src/expr2.c"
for(( ( (_au1_e = _au1_arg ), (_au1_nn = _au1_f -> _fct_argtype )) , (_au1_argno = 1 )) ;_au1_e || _au1_nn ;( ( (_au1_nn =
#line 842 "../../src/expr2.c"
_au1_nn -> _name_n_list ), (_au1_e = _au1_etail -> _expr__O4.__C4_e2 )) , (_au1_argno ++ )) ) { 
#line 843 "../../src/expr2.c"
Pexpr _au2_a ;

#line 845 "../../src/expr2.c"
if (_au1_e ){ 
#line 846 "../../src/expr2.c"
_au2_a = _au1_e -> _expr__O3.__C3_e1 ;

#line 848 "../../src/expr2.c"
_au1_etail = _au1_e ;

#line 850 "../../src/expr2.c"
if (_au1_nn ){ 
#line 851 "../../src/expr2.c"
Ptype _au4_t1 ;

#line 851 "../../src/expr2.c"
_au4_t1 = _au1_nn -> _expr__O2.__C2_tp ;
lx :
#line 853 "../../src/expr2.c"
switch (_au4_t1 -> _node_base ){ 
#line 854 "../../src/expr2.c"
case 97 : 
#line 855 "../../src/expr2.c"
if (! t_const )t_const = (((struct basetype *)_au4_t1 ))-> _basetype_b_const ;
_au4_t1 = (((struct basetype *)_au4_t1 ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
goto lx ;
case 158 : 
#line 859 "../../src/expr2.c"
_au2_a = ref_init ( ((struct ptr *)_au4_t1 ), _au2_a , _au0_tbl ) ;
goto cbcb ;
case 119 : 
#line 862 "../../src/expr2.c"
if ((_au2_a -> _node_base != 147 )|| _type_check ( _au4_t1 , _au2_a -> _expr__O2.__C2_tp , (unsigned char )70 ) )
#line 864 "../../src/expr2.c"
_au2_a =
#line 864 "../../src/expr2.c"
class_init ( (struct expr *)0 , _au4_t1 , _au2_a , _au0_tbl ) ;
if (_au1_nn -> _name_n_xref ){ 
#line 867 "../../src/expr2.c"
_au2_a = _expr_address ( _au2_a ) ;
}
else { 
#line 872 "../../src/expr2.c"
Pname _au6_cln ;

#line 872 "../../src/expr2.c"
_au6_cln = (((struct basetype *)_au4_t1 ))-> _basetype_b_name ;
if (_au6_cln && ( (((struct classdef *)_au6_cln -> _expr__O2.__C2_tp ))-> _classdef_itor ) ){ 
#line 875 "../../src/expr2.c"
_au1_nn -> _name_n_xref = 1 ;
_au2_a = _expr_address ( _au2_a ) ;
}
}
cbcb :
#line 880 "../../src/expr2.c"
if (_au2_a -> _node_base == 147 ){ 
#line 881 "../../src/expr2.c"
if (_au2_a -> _expr__O3.__C3_e1 -> _node_base == 111 )_au2_a -> _expr__O3.__C3_e1 = _au2_a -> _expr__O3.__C3_e1 -> _expr__O4.__C4_e2 ;
if ((((_au2_a -> _expr__O3.__C3_e1 -> _node_base == 146 )&& (((struct name *)_au2_a -> _expr__O3.__C3_e1 -> _expr__O5.__C5_fct_name )))&& ((((struct name *)_au2_a -> _expr__O3.__C3_e1 -> _expr__O5.__C5_fct_name ))-> _name_n_oper ==
#line 882 "../../src/expr2.c"
161 ))&& ((_au2_a -> _expr__O4.__C4_e2 -> _node_base == 145 )|| (_au2_a -> _expr__O4.__C4_e2 -> _node_base == 112 )))
#line 886 "../../src/expr2.c"
_au2_a = _au2_a -> _expr__O3.__C3_e1 ;
}
_au1_e -> _expr__O3.__C3_e1 = _au2_a ;
break ;
case 141 : 
#line 891 "../../src/expr2.c"
goto rlab ;
case 125 : 
#line 893 "../../src/expr2.c"
_au1_e -> _expr__O3.__C3_e1 = (_au2_a = ptr_init ( ((struct ptr *)_au4_t1 ), _au2_a , _au0_tbl ) );
goto def ;
case 5 : 
#line 896 "../../src/expr2.c"
case 29 : 
#line 897 "../../src/expr2.c"
case 21 : 
#line 898 "../../src/expr2.c"
if ((_au2_a -> _node_base == 82 )&& (_au2_a -> _expr__O2.__C2_tp == (struct type *)long_type ))
#line 899 "../../src/expr2.c"
{
#line 899 "../../src/expr2.c"

#line 964 "../../src/expr2.c"
struct ea _au0__V59 ;

#line 964 "../../src/expr2.c"
struct ea _au0__V60 ;

#line 899 "../../src/expr2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"long constantA for%n,%kX", (struct ea *)( ( ((& _au0__V59 )-> _ea__O1.__C1_p = ((char *)_au1_fn )), (((&
#line 899 "../../src/expr2.c"
_au0__V59 )))) ) , (struct ea *)( ( ((& _au0__V60 )-> _ea__O1.__C1_i = ((int )_au4_t1 -> _node_base )), (((& _au0__V60 ))))
#line 899 "../../src/expr2.c"
) , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} case 22 : 
#line 901 "../../src/expr2.c"
if (((((struct basetype *)_au4_t1 ))-> _basetype_b_unsigned && (_au2_a -> _node_base == 107 ))&& (_au2_a -> _expr__O4.__C4_e2 -> _node_base == 82 ))
#line 904 "../../src/expr2.c"
{
#line 904 "../../src/expr2.c"

#line 964 "../../src/expr2.c"
struct ea _au0__V61 ;

#line 904 "../../src/expr2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"negativeA for%n, unsignedX", (struct ea *)( ( ((& _au0__V61 )-> _ea__O1.__C1_p = ((char *)_au1_fn )), (((&
#line 904 "../../src/expr2.c"
_au0__V61 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} default : 
#line 906 "../../src/expr2.c"
def :
#line 907 "../../src/expr2.c"
{ Pexpr _au6_x ;

#line 907 "../../src/expr2.c"
_au6_x = try_to_coerce ( _au4_t1 , _au2_a , "argument", _au0_tbl ) ;
if (_au6_x )
#line 909 "../../src/expr2.c"
_au1_e -> _expr__O3.__C3_e1 = _au6_x ;
else if (_type_check ( _au4_t1 , _au2_a -> _expr__O2.__C2_tp , (unsigned char )136 ) ){ 
#line 911 "../../src/expr2.c"
if (arg_err_suppress == 0 ){ 
#line 964 "../../src/expr2.c"
struct
#line 964 "../../src/expr2.c"
ea _au0__V62 ;

#line 964 "../../src/expr2.c"
struct ea _au0__V63 ;

#line 964 "../../src/expr2.c"
struct ea _au0__V64 ;

#line 964 "../../src/expr2.c"
struct ea _au0__V65 ;

#line 911 "../../src/expr2.c"
error ( (char *)"badA %dT for%n:%t (%tX)", (struct ea *)( ( ((& _au0__V62 )-> _ea__O1.__C1_i = _au1_argno ), (((& _au0__V62 )))) ) ,
#line 911 "../../src/expr2.c"
(struct ea *)( ( ((& _au0__V63 )-> _ea__O1.__C1_p = ((char *)_au1_fn )), (((& _au0__V63 )))) ) , (struct ea *)(
#line 911 "../../src/expr2.c"
( ((& _au0__V64 )-> _ea__O1.__C1_p = ((char *)_au2_a -> _expr__O2.__C2_tp )), (((& _au0__V64 )))) ) , (struct ea *)( (
#line 911 "../../src/expr2.c"
((& _au0__V65 )-> _ea__O1.__C1_p = ((char *)_au1_nn -> _expr__O2.__C2_tp )), (((& _au0__V65 )))) ) ) ;
} return (struct type *)any_type ;
}
}
}
}
else { 
#line 918 "../../src/expr2.c"
if (_au1_k != 155 ){ 
#line 919 "../../src/expr2.c"
if (arg_err_suppress == 0 ){ 
#line 964 "../../src/expr2.c"
struct ea _au0__V66 ;

#line 964 "../../src/expr2.c"
struct ea _au0__V67 ;

#line 919 "../../src/expr2.c"
error ( (char *)"unexpected %dA for%n", (struct ea *)( ( ((& _au0__V66 )-> _ea__O1.__C1_i = _au1_argno ), (((& _au0__V66 )))) ) ,
#line 919 "../../src/expr2.c"
(struct ea *)( ( ((& _au0__V67 )-> _ea__O1.__C1_p = ((char *)_au1_fn )), (((& _au0__V67 )))) ) , (struct ea *)ea0 ,
#line 919 "../../src/expr2.c"
(struct ea *)ea0 ) ;
} return (struct type *)any_type ;
}
goto rlab ;
}
}
else { 
#line 926 "../../src/expr2.c"
_au2_a = _au1_nn -> _expr__O4.__C4_n_initializer ;

#line 928 "../../src/expr2.c"
if (_au2_a == 0 ){ 
#line 929 "../../src/expr2.c"
if (arg_err_suppress == 0 ){ 
#line 964 "../../src/expr2.c"
struct ea _au0__V68 ;

#line 964 "../../src/expr2.c"
struct ea _au0__V69 ;

#line 964 "../../src/expr2.c"
struct ea _au0__V70 ;

#line 929 "../../src/expr2.c"
error ( (char *)"A %d ofT%tX for%n", (struct ea *)( ( ((& _au0__V68 )-> _ea__O1.__C1_i = _au1_argno ), (((& _au0__V68 )))) ) ,
#line 929 "../../src/expr2.c"
(struct ea *)( ( ((& _au0__V69 )-> _ea__O1.__C1_p = ((char *)_au1_nn -> _expr__O2.__C2_tp )), (((& _au0__V69 )))) ) , (struct
#line 929 "../../src/expr2.c"
ea *)( ( ((& _au0__V70 )-> _ea__O1.__C1_p = ((char *)_au1_fn )), (((& _au0__V70 )))) ) , (struct ea *)ea0 ) ;
#line 929 "../../src/expr2.c"
} 
#line 930 "../../src/expr2.c"
return (struct type *)any_type ;
}

#line 933 "../../src/expr2.c"
_au2_a -> _node_permanent = 2 ;
_au1_e = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , _au2_a , (struct expr *)0 ) ;
if (_au1_etail )
#line 936 "../../src/expr2.c"
_au1_etail -> _expr__O4.__C4_e2 = _au1_e ;
else 
#line 938 "../../src/expr2.c"
_au0_this -> _expr__O4.__C4_e2 = _au1_e ;
_au1_etail = _au1_e ;
}
}
rlab :
#line 944 "../../src/expr2.c"
if (_au1_fn && (_au1_f -> _fct_f_result == 0 )){ 
#line 946 "../../src/expr2.c"
Pname _au2_cn ;

#line 946 "../../src/expr2.c"
_au2_cn = _type_is_cl_obj ( _au1_f -> _fct_returns ) ;
if (_au2_cn && ( (((struct classdef *)_au2_cn -> _expr__O2.__C2_tp ))-> _classdef_itor ) ){ 
#line 948 "../../src/expr2.c"
int _au3_ll ;

#line 948 "../../src/expr2.c"
_au3_ll = ((_au0_this -> _expr__O5.__C5_fct_name -> _expr__O5.__C5_n_table == gtbl )? 2 : 1 );
if (_au3_ll < _au0_this -> _expr__O5.__C5_fct_name -> _name_n_used ){ 
#line 964 "../../src/expr2.c"
struct ea _au0__V71 ;

#line 964 "../../src/expr2.c"
struct ea _au0__V72 ;

#line 964 "../../src/expr2.c"
struct ea _au0__V73 ;

#line 964 "../../src/expr2.c"
struct ea _au0__V74 ;

#line 949 "../../src/expr2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"%n returning %n called before %s(%s&)D seen", (struct ea *)( ( ((& _au0__V71 )-> _ea__O1.__C1_p = ((char *)_au1_fn )), (((&
#line 949 "../../src/expr2.c"
_au0__V71 )))) ) , (struct ea *)( ( ((& _au0__V72 )-> _ea__O1.__C1_p = ((char *)_au2_cn )), (((& _au0__V72 )))) )
#line 949 "../../src/expr2.c"
, (struct ea *)( ( ((& _au0__V73 )-> _ea__O1.__C1_p = ((char *)_au2_cn -> _expr__O3.__C3_string )), (((& _au0__V73 )))) ) ,
#line 949 "../../src/expr2.c"
(struct ea *)( ( ((& _au0__V74 )-> _ea__O1.__C1_p = ((char *)_au2_cn -> _expr__O3.__C3_string )), (((& _au0__V74 )))) ) ) ;
#line 949 "../../src/expr2.c"
} 
#line 950 "../../src/expr2.c"
make_res ( _au1_f ) ;
}
}
if (_au1_f -> _fct_f_result ){ 
#line 954 "../../src/expr2.c"
Pname _au2_tn ;

#line 954 "../../src/expr2.c"
_au2_tn = make_tmp ( 'R' , _au1_f -> _fct_returns , _au0_tbl ) ;
_au0_this -> _expr__O4.__C4_e2 = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , _expr_address ( (struct expr *)_au2_tn ) , _au0_this ->
#line 955 "../../src/expr2.c"
_expr__O4.__C4_e2 ) ;
{ Pexpr _au2_ee ;

#line 956 "../../src/expr2.c"
_au2_ee = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )0 , (struct expr *)0 , (struct expr *)0 ) ;
(*_au2_ee )= (*_au0_this );
_au0_this -> _node_base = 147 ;
_au0_this -> _expr__O3.__C3_e1 = _au2_ee ;
_au0_this -> _expr__O4.__C4_e2 = (struct expr *)_au2_tn ;
}
}
return _au1_t ;
}
;
extern Pexpr ref_init (_au0_p , _au0_init , _au0_tbl )Pptr _au0_p ;

#line 966 "../../src/expr2.c"
Pexpr _au0_init ;

#line 966 "../../src/expr2.c"
Ptable _au0_tbl ;

#line 970 "../../src/expr2.c"
{ 
#line 971 "../../src/expr2.c"
register Ptype _au1_it ;
Ptype _au1_p1 ;
Pname _au1_c1 ;

#line 971 "../../src/expr2.c"
_au1_it = _au0_init -> _expr__O2.__C2_tp ;
_au1_p1 = _au0_p -> _pvtyp_typ ;
_au1_c1 = _type_is_cl_obj ( _au1_p1 ) ;

#line 975 "../../src/expr2.c"
rloop :
#line 976 "../../src/expr2.c"
switch (_au1_it -> _node_base ){ 
#line 977 "../../src/expr2.c"
case 97 : 
#line 978 "../../src/expr2.c"
_au1_it = (((struct basetype *)_au1_it ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 978 "../../src/expr2.c"
goto rloop ;
default : 
#line 980 "../../src/expr2.c"
{ Ptype _au3_tt ;

#line 981 "../../src/expr2.c"
struct ptr *_au0__Xthis__ctor_ptr ;

#line 980 "../../src/expr2.c"
_au3_tt = (struct type *)( (((struct ptr *)( (_au0__Xthis__ctor_ptr = 0 ), ( (_au0__Xthis__ctor_ptr = (struct ptr *)_new ( (long )(sizeof (struct ptr)))
#line 980 "../../src/expr2.c"
), ( (Nt ++ ), ( (_au0__Xthis__ctor_ptr -> _node_base = ((unsigned char )125 )), ( (_au0__Xthis__ctor_ptr -> _pvtyp_typ = _au1_it ),
#line 980 "../../src/expr2.c"
( (_au0__Xthis__ctor_ptr -> _ptr_rdo = ((char )0 )), ((_au0__Xthis__ctor_ptr ))) ) ) ) ) ) ))) ;
_au0_p -> _node_base = 125 ;

#line 983 "../../src/expr2.c"
{ int _au3_x ;

#line 983 "../../src/expr2.c"
_au3_x = _type_check ( (struct type *)_au0_p , _au3_tt , (unsigned char )78 ) ;
if (_au3_x == 0 ){ 
#line 985 "../../src/expr2.c"
if (_type_tconst ( _au0_init -> _expr__O2.__C2_tp ) && (vec_const == 0 )){ 
#line 987 "../../src/expr2.c"
(((struct ptr *)_au3_tt ))-> _ptr_rdo = 1 ;
#line 987 "../../src/expr2.c"

#line 988 "../../src/expr2.c"
if (_type_tconst ( _au0_p -> _pvtyp_typ ) == 0 )error ( (char *)"R to constO", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ,
#line 988 "../../src/expr2.c"
(struct ea *)ea0 ) ;
}
if (_type_check ( (struct type *)_au0_p , _au3_tt , (unsigned char )78 ) )error ( (char *)"R to constO", (struct ea *)ea0 , (struct
#line 990 "../../src/expr2.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
_au0_p -> _node_base = 158 ;

#line 993 "../../src/expr2.c"
if (_expr_lval ( _au0_init , (unsigned char )0 ) )return _expr_address ( _au0_init ) ;

#line 1001 "../../src/expr2.c"
goto xxx ;
}
_au0_p -> _node_base = 158 ;
}
}
}
if (_au1_c1 ){ 
#line 1008 "../../src/expr2.c"
refd = 1 ;
{ Pexpr _au2_a ;

#line 1009 "../../src/expr2.c"
_au2_a = class_init ( (struct expr *)0 , _au1_p1 , _au0_init , _au0_tbl ) ;
refd = 0 ;
if ((_au2_a == _au0_init )&& (_au0_init -> _expr__O2.__C2_tp != (struct type *)any_type ))goto xxx ;
switch (_au2_a -> _node_base ){ 
#line 1013 "../../src/expr2.c"
case 146 : 
#line 1014 "../../src/expr2.c"
case 71 : 
#line 1015 "../../src/expr2.c"
_au0_init = _au2_a ;
goto xxx ;
}
return _expr_address ( _au2_a ) ;
}
}
if (_type_check ( _au1_p1 , _au1_it , (unsigned char )0 ) ){ 
#line 1022 "../../src/expr2.c"
{ 
#line 1117 "../../src/expr2.c"
struct ea _au0__V75 ;

#line 1117 "../../src/expr2.c"
struct ea _au0__V76 ;

#line 1022 "../../src/expr2.c"
error ( (char *)"badIrT:%t (%tX)", (struct ea *)( ( ((& _au0__V75 )-> _ea__O1.__C1_p = ((char *)_au1_it )), (((& _au0__V75 )))) )
#line 1022 "../../src/expr2.c"
, (struct ea *)( ( ((& _au0__V76 )-> _ea__O1.__C1_p = ((char *)_au0_p )), (((& _au0__V76 )))) ) , (struct
#line 1022 "../../src/expr2.c"
ea *)ea0 , (struct ea *)ea0 ) ;
if (_au0_init -> _node_base != 85 )_au0_init -> _expr__O2.__C2_tp = (struct type *)any_type ;
return _au0_init ;
} }

#line 1027 "../../src/expr2.c"
xxx :
#line 1034 "../../src/expr2.c"
switch (_au0_init -> _node_base ){ 
#line 1035 "../../src/expr2.c"
case 85 : 
#line 1036 "../../src/expr2.c"
case 111 : 
#line 1037 "../../src/expr2.c"
case 44 : 
#line 1038 "../../src/expr2.c"
case 45 : 
#line 1039 "../../src/expr2.c"
if (_type_tconst (
#line 1039 "../../src/expr2.c"
_au1_it ) && (vec_const == 0 ))goto def ;
_expr_lval ( _au0_init , (unsigned char )112 ) ;

#line 1042 "../../src/expr2.c"
case 147 : 
#line 1043 "../../src/expr2.c"
return _expr_address ( _au0_init ) ;

#line 1045 "../../src/expr2.c"
case 68 : 
#line 1047 "../../src/expr2.c"
switch (_au0_init -> _expr__O3.__C3_e1 -> _node_base ){ 
#line 1048 "../../src/expr2.c"
case 85 : 
#line 1049 "../../src/expr2.c"
case 111 : 
#line 1050 "../../src/expr2.c"
case 44 : 
#line 1051 "../../src/expr2.c"
case 45 :
#line 1051 "../../src/expr2.c"

#line 1052 "../../src/expr2.c"
if (_type_tconst ( _au0_init -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp ) && (vec_const == 0 ))break ;
case 147 : 
#line 1054 "../../src/expr2.c"
switch (_au0_init -> _expr__O4.__C4_e2 -> _node_base ){ 
#line 1055 "../../src/expr2.c"
case 85 : 
#line 1056 "../../src/expr2.c"
case 111 : 
#line 1057 "../../src/expr2.c"
case 44 : 
#line 1058 "../../src/expr2.c"
case 45 :
#line 1058 "../../src/expr2.c"

#line 1059 "../../src/expr2.c"
if (_type_tconst ( _au0_init -> _expr__O4.__C4_e2 -> _expr__O2.__C2_tp ) && (vec_const == 0 ))break ;
case 147 : 
#line 1061 "../../src/expr2.c"
_au0_init -> _expr__O3.__C3_e1 = _expr_address ( _au0_init -> _expr__O3.__C3_e1 ) ;
_au0_init -> _expr__O4.__C4_e2 = _expr_address ( _au0_init -> _expr__O4.__C4_e2 ) ;
return _au0_init ;
}
}

#line 1067 "../../src/expr2.c"
if (( (((struct classdef *)_au1_c1 -> _expr__O2.__C2_tp ))-> _classdef_itor ) ){ 
#line 1068 "../../src/expr2.c"
{ 
#line 1117 "../../src/expr2.c"
struct ea _au0__V77 ;

#line 1117 "../../src/expr2.c"
struct ea _au0__V78 ;

#line 1117 "../../src/expr2.c"
struct ea _au0__V79 ;

#line 1068 "../../src/expr2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"?:E ofT%n: %s(%s&)Dd", (struct ea *)( ( ((& _au0__V77 )-> _ea__O1.__C1_p = ((char *)_au1_c1 )), (((&
#line 1068 "../../src/expr2.c"
_au0__V77 )))) ) , (struct ea *)( ( ((& _au0__V78 )-> _ea__O1.__C1_p = ((char *)_au1_c1 -> _expr__O3.__C3_string )), (((& _au0__V78 ))))
#line 1068 "../../src/expr2.c"
) , (struct ea *)( ( ((& _au0__V79 )-> _ea__O1.__C1_p = ((char *)_au1_c1 -> _expr__O3.__C3_string )), (((& _au0__V79 )))) )
#line 1068 "../../src/expr2.c"
, (struct ea *)ea0 ) ;
return _au0_init ;
} }
goto def ;
case 109 : 
#line 1073 "../../src/expr2.c"
case 146 : 
#line 1075 "../../src/expr2.c"
goto def ;

#line 1077 "../../src/expr2.c"
case 71 : 
#line 1078 "../../src/expr2.c"
{ Pexpr _au3_ee ;

#line 1078 "../../src/expr2.c"
_au3_ee = _au0_init -> _expr__O4.__C4_e2 ;
cml :
#line 1080 "../../src/expr2.c"
switch (_au3_ee -> _node_base ){ 
#line 1081 "../../src/expr2.c"
case 71 : _au3_ee = _au3_ee -> _expr__O4.__C4_e2 ;

#line 1081 "../../src/expr2.c"
goto cml ;
case 147 : 
#line 1083 "../../src/expr2.c"
case 85 : 
#line 1084 "../../src/expr2.c"
case 111 : return _expr_address ( _au0_init ) ;
}
}

#line 1089 "../../src/expr2.c"
default : 
#line 1090 "../../src/expr2.c"
def :
#line 1091 "../../src/expr2.c"
{ 
#line 1092 "../../src/expr2.c"
if (_au0_tbl == gtbl )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"Ir for staticR not an lvalue", (struct ea *)ea0 , (struct ea *)ea0 ,
#line 1092 "../../src/expr2.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
{ Pname _au3_n ;
Pexpr _au3_a ;

#line 1095 "../../src/expr2.c"
struct texpr *_au0__Xthis__ctor_texpr ;

#line 1093 "../../src/expr2.c"
_au3_n = make_tmp ( 'I' , _au1_p1 , _au0_tbl ) ;

#line 1095 "../../src/expr2.c"
if (_au1_c1 != _type_is_cl_obj ( _au0_init -> _expr__O2.__C2_tp ) ){ 
#line 1097 "../../src/expr2.c"
_au3_n -> _expr__O2.__C2_tp = _au0_init -> _expr__O2.__C2_tp ;
_au3_a = _expr_address ( (struct expr *)_au3_n ) ;
_au0_p -> _node_permanent = 1 ;
_au3_a = (struct expr *)( (_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr = (struct texpr *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_texpr ),
#line 1100 "../../src/expr2.c"
((unsigned char )113 ), _au3_a , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 = ((struct type *)_au0_p )), ((_au0__Xthis__ctor_texpr )))
#line 1100 "../../src/expr2.c"
) ) ;
_au3_a -> _expr__O2.__C2_tp = (struct type *)_au0_p ;
}
else 
#line 1104 "../../src/expr2.c"
_au3_a = _expr_address ( (struct expr *)_au3_n ) ;

#line 1106 "../../src/expr2.c"
if ((_au0_init -> _node_base == 70 )&& (_au0_init -> _expr__O3.__C3_e1 -> _node_base == 111 ))
#line 1107 "../../src/expr2.c"
_au0_init = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char
#line 1107 "../../src/expr2.c"
)147 , _au0_init , _au0_init -> _expr__O3.__C3_e1 -> _expr__O3.__C3_e1 ) ;

#line 1109 "../../src/expr2.c"
refd = 1 ;
{ Pexpr _au3_as ;

#line 1110 "../../src/expr2.c"
_au3_as = init_tmp ( _au3_n , _au0_init , _au0_tbl ) ;
refd = 0 ;
_au3_a = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )71 , _au3_as , _au3_a ) ;
_au3_a -> _expr__O2.__C2_tp = _au3_a -> _expr__O4.__C4_e2 -> _expr__O2.__C2_tp ;
return _au3_a ;
}
}
}
}
}
;

#line 1119 "../../src/expr2.c"
extern Pexpr class_init (_au0_nn , _au0_tt , _au0_init , _au0_tbl )Pexpr _au0_nn ;

#line 1119 "../../src/expr2.c"
Ptype _au0_tt ;

#line 1119 "../../src/expr2.c"
Pexpr _au0_init ;

#line 1119 "../../src/expr2.c"
Ptable _au0_tbl ;

#line 1125 "../../src/expr2.c"
{ Pname _au1_c1 ;
Pname _au1_c2 ;

#line 1125 "../../src/expr2.c"
_au1_c1 = _type_is_cl_obj ( _au0_tt ) ;
_au1_c2 = _type_is_cl_obj ( _au0_init -> _expr__O2.__C2_tp ) ;

#line 1128 "../../src/expr2.c"
if (_au1_c1 ){ 
#line 1129 "../../src/expr2.c"
if ((_au1_c1 != _au1_c2 )|| ((refd == 0 )&& ( (((struct classdef *)_au1_c1 -> _expr__O2.__C2_tp ))-> _classdef_itor ) ))
#line 1130 "../../src/expr2.c"
{ 
#line 1134 "../../src/expr2.c"
int
#line 1134 "../../src/expr2.c"
_au3_i ;

#line 1134 "../../src/expr2.c"
_au3_i = can_coerce ( _au0_tt , _au0_init -> _expr__O2.__C2_tp ) ;
switch (_au3_i ){ 
#line 1136 "../../src/expr2.c"
default : 
#line 1137 "../../src/expr2.c"
{ 
#line 1202 "../../src/expr2.c"
struct ea _au0__V80 ;

#line 1202 "../../src/expr2.c"
struct ea _au0__V81 ;

#line 1202 "../../src/expr2.c"
struct ea _au0__V82 ;

#line 1137 "../../src/expr2.c"
error ( (char *)"%d ways of making a%n from a%t", (struct ea *)( ( ((& _au0__V80 )-> _ea__O1.__C1_i = _au3_i ), (((& _au0__V80 )))) ) ,
#line 1137 "../../src/expr2.c"
(struct ea *)( ( ((& _au0__V81 )-> _ea__O1.__C1_p = ((char *)_au1_c1 )), (((& _au0__V81 )))) ) , (struct ea *)(
#line 1137 "../../src/expr2.c"
( ((& _au0__V82 )-> _ea__O1.__C1_p = ((char *)_au0_init -> _expr__O2.__C2_tp )), (((& _au0__V82 )))) ) , (struct ea *)ea0 ) ;
#line 1137 "../../src/expr2.c"

#line 1138 "../../src/expr2.c"
_au0_init -> _expr__O2.__C2_tp = (struct type *)any_type ;
return _au0_init ;
case 0 : 
#line 1141 "../../src/expr2.c"
{ 
#line 1202 "../../src/expr2.c"
struct ea _au0__V83 ;

#line 1202 "../../src/expr2.c"
struct ea _au0__V84 ;

#line 1141 "../../src/expr2.c"
error ( (char *)"cannot make a%n from a%t", (struct ea *)( ( ((& _au0__V83 )-> _ea__O1.__C1_p = ((char *)_au1_c1 )), (((& _au0__V83 )))) )
#line 1141 "../../src/expr2.c"
, (struct ea *)( ( ((& _au0__V84 )-> _ea__O1.__C1_p = ((char *)_au0_init -> _expr__O2.__C2_tp )), (((& _au0__V84 )))) ) ,
#line 1141 "../../src/expr2.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
_au0_init -> _expr__O2.__C2_tp = (struct type *)any_type ;
return _au0_init ;
case 1 : 
#line 1145 "../../src/expr2.c"
if (Ncoerce == 0 ){ 
#line 1146 "../../src/expr2.c"
Pexpr _au5_a ;

#line 1147 "../../src/expr2.c"
struct texpr *_au0__Xthis__ctor_texpr ;

#line 1146 "../../src/expr2.c"
_au5_a = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , _au0_init , (struct expr *)0 ) ;
_au5_a = (struct expr *)( (_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr = (struct texpr *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_texpr ),
#line 1147 "../../src/expr2.c"
((unsigned char )157 ), _au5_a , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 = _au0_tt ), ((_au0__Xthis__ctor_texpr ))) )
#line 1147 "../../src/expr2.c"
) ;
_au5_a -> _expr__O4.__C4_e2 = _au0_nn ;
return _expr_typ ( _au5_a , _au0_tbl ) ;
}

#line 1152 "../../src/expr2.c"
switch (_au0_init -> _node_base ){ 
#line 1157 "../../src/expr2.c"
case 71 : 
#line 1158 "../../src/expr2.c"
case 85 : 
#line 1159 "../../src/expr2.c"
{ Pref _au6_r ;
Pexpr _au6_rr ;

#line 1161 "../../src/expr2.c"
struct ref *_au0__Xthis__ctor_ref ;

#line 1159 "../../src/expr2.c"
_au6_r = (struct ref *)( (_au0__Xthis__ctor_ref = 0 ), ( ( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ref ),
#line 1159 "../../src/expr2.c"
((unsigned char )45 ), _au0_init , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = Ncoerce ), ((_au0__Xthis__ctor_ref ))) )
#line 1159 "../../src/expr2.c"
) ;
_au6_rr = _expr_typ ( (struct expr *)_au6_r , _au0_tbl ) ;
_au0_init = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )146 , _au6_rr , (struct expr *)0 ) ;
_au0_init -> _expr__O5.__C5_fct_name = Ncoerce ;
break ;
}
default : 
#line 1166 "../../src/expr2.c"
{ Pname _au6_tmp ;
Pexpr _au6_ass ;
Pref _au6_r ;
Pexpr _au6_rr ;
Pexpr _au6_c ;

#line 1171 "../../src/expr2.c"
struct ref *_au0__Xthis__ctor_ref ;

#line 1166 "../../src/expr2.c"
_au6_tmp = make_tmp ( 'U' , _au0_init -> _expr__O2.__C2_tp , _au0_tbl ) ;
_au6_ass = init_tmp ( _au6_tmp , _au0_init , _au0_tbl ) ;
_au6_r = (struct ref *)( (_au0__Xthis__ctor_ref = 0 ), ( ( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ref ),
#line 1168 "../../src/expr2.c"
((unsigned char )45 ), ((struct expr *)_au6_tmp ), (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = Ncoerce ), ((_au0__Xthis__ctor_ref )))
#line 1168 "../../src/expr2.c"
) ) ;
_au6_rr = _expr_typ ( (struct expr *)_au6_r , _au0_tbl ) ;
_au6_c = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )146 , _au6_rr , (struct expr *)0 ) ;
_au6_c -> _expr__O5.__C5_fct_name = Ncoerce ;
_au0_init = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )71 , _au6_ass , _au6_c ) ;
}
}
if (_au0_nn ){ 
#line 1176 "../../src/expr2.c"
Pexpr _au5_a ;

#line 1177 "../../src/expr2.c"
struct texpr *_au0__Xthis__ctor_texpr ;

#line 1176 "../../src/expr2.c"
_au5_a = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , _au0_init , (struct expr *)0 ) ;
_au5_a = (struct expr *)( (_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr = (struct texpr *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_texpr ),
#line 1177 "../../src/expr2.c"
((unsigned char )157 ), _au5_a , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 = _au0_tt ), ((_au0__Xthis__ctor_texpr ))) )
#line 1177 "../../src/expr2.c"
) ;
_au5_a -> _expr__O4.__C4_e2 = _au0_nn ;
return _expr_typ ( _au5_a , _au0_tbl ) ;
}
} } }
return _expr_typ ( _au0_init , _au0_tbl ) ;
}
else if (refd == 0 ){ 
#line 1185 "../../src/expr2.c"
Pclass _au3_cl ;

#line 1185 "../../src/expr2.c"
_au3_cl = (((struct classdef *)_au1_c1 -> _expr__O2.__C2_tp ));
if (_au3_cl -> _classdef_itor == 0 ){ 
#line 1187 "../../src/expr2.c"
if (_au3_cl -> _classdef_bit_ass == 0 )
#line 1188 "../../src/expr2.c"
{ 
#line 1202 "../../src/expr2.c"
struct ea _au0__V85 ;

#line 1188 "../../src/expr2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"bitwise copy: %s has a memberW operator=()", (struct ea *)( ( ((& _au0__V85 )-> _ea__O1.__C1_p = ((char *)_au3_cl -> _classdef_string )),
#line 1188 "../../src/expr2.c"
(((& _au0__V85 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} else if (( _table_look ( _au3_cl -> _classdef_memtbl , "_dtor", (unsigned char )0 ) ) && _classdef_has_oper (
#line 1189 "../../src/expr2.c"
_au3_cl , (unsigned char )70 ) )
#line 1190 "../../src/expr2.c"
{ 
#line 1202 "../../src/expr2.c"
struct ea _au0__V86 ;

#line 1202 "../../src/expr2.c"
struct ea _au0__V87 ;

#line 1202 "../../src/expr2.c"
struct ea _au0__V88 ;

#line 1190 "../../src/expr2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"bitwise copy: %s has assignment and destructor but not %s(%s&)", (struct ea *)( ( ((& _au0__V86 )-> _ea__O1.__C1_p = ((char *)_au3_cl -> _classdef_string )),
#line 1190 "../../src/expr2.c"
(((& _au0__V86 )))) ) , (struct ea *)( ( ((& _au0__V87 )-> _ea__O1.__C1_p = ((char *)_au3_cl -> _classdef_string )), (((&
#line 1190 "../../src/expr2.c"
_au0__V87 )))) ) , (struct ea *)( ( ((& _au0__V88 )-> _ea__O1.__C1_p = ((char *)_au3_cl -> _classdef_string )), (((& _au0__V88 ))))
#line 1190 "../../src/expr2.c"
) , (struct ea *)ea0 ) ;
} }
}

#line 1194 "../../src/expr2.c"
return _au0_init ;
}

#line 1197 "../../src/expr2.c"
if (_type_check ( _au0_tt , _au0_init -> _expr__O2.__C2_tp , (unsigned char )70 ) && (refd == 0 )){ 
#line 1198 "../../src/expr2.c"
{ 
#line 1202 "../../src/expr2.c"
struct ea _au0__V89 ;
#line 1202 "../../src/expr2.c"
struct ea _au0__V90 ;

#line 1198 "../../src/expr2.c"
error ( (char *)"badIrT:%t (%tX)", (struct ea *)( ( ((& _au0__V89 )-> _ea__O1.__C1_p = ((char *)_au0_init -> _expr__O2.__C2_tp )), (((& _au0__V89 ))))
#line 1198 "../../src/expr2.c"
) , (struct ea *)( ( ((& _au0__V90 )-> _ea__O1.__C1_p = ((char *)_au0_tt )), (((& _au0__V90 )))) ) ,
#line 1198 "../../src/expr2.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
_au0_init -> _expr__O2.__C2_tp = (struct type *)any_type ;
} }
return _au0_init ;
}
;
int char_to_int (_au0_s )char *_au0_s ;

#line 1213 "../../src/expr2.c"
{ 
#line 1214 "../../src/expr2.c"
register int _au1_i ;
register char _au1_c ;

#line 1215 "../../src/expr2.c"
register char _au1_d ;

#line 1215 "../../src/expr2.c"
register char _au1_e ;

#line 1214 "../../src/expr2.c"
_au1_i = 0 ;

#line 1217 "../../src/expr2.c"
switch (*_au0_s ){ 
#line 1218 "../../src/expr2.c"
default : 
#line 1219 "../../src/expr2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"char constant store corrupted", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 1219 "../../src/expr2.c"
ea *)ea0 , (struct ea *)ea0 ) ;
case '`' : 
#line 1221 "../../src/expr2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"bcd constant", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 1221 "../../src/expr2.c"
ea *)ea0 ) ;
return (int )0 ;
case '\'' : 
#line 1224 "../../src/expr2.c"
break ;
}

#line 1227 "../../src/expr2.c"
for(;;) 
#line 1228 "../../src/expr2.c"
switch (_au1_c = (*(++ _au0_s ))){ 
#line 1229 "../../src/expr2.c"
case '\'' : 
#line 1230 "../../src/expr2.c"
return _au1_i ;
case '\\' : 
#line 1232 "../../src/expr2.c"
switch (_au1_c = (*(++ _au0_s ))){ 
#line 1233 "../../src/expr2.c"
case '0' : case '1' : case '2' : case '3' :
#line 1233 "../../src/expr2.c"
case '4' : 
#line 1234 "../../src/expr2.c"
case '5' : case '6' : case '7' : 
#line 1235 "../../src/expr2.c"
_au1_c -= '0' ;
switch (_au1_d = (*(++ _au0_s ))){ 
#line 1238 "../../src/expr2.c"
case '0' : case '1' : case '2' : case '3' : case '4' :
#line 1238 "../../src/expr2.c"

#line 1239 "../../src/expr2.c"
case '5' : case '6' : case '7' : 
#line 1240 "../../src/expr2.c"
_au1_d -= '0' ;
switch (_au1_e = (*(++ _au0_s ))){ 
#line 1243 "../../src/expr2.c"
case '0' : case '1' : case '2' : case '3' : case '4' :
#line 1243 "../../src/expr2.c"

#line 1244 "../../src/expr2.c"
case '5' : case '6' : case '7' : 
#line 1245 "../../src/expr2.c"
_au1_c = ((((_au1_c * 64 )+ (_au1_d * 8 ))+ _au1_e )- '0' );
break ;
default : 
#line 1248 "../../src/expr2.c"
_au1_c = ((_au1_c * 8 )+ _au1_d );
_au0_s -- ;
}
break ;
default : 
#line 1253 "../../src/expr2.c"
_au0_s -- ;
}
break ;
case 'b' : 
#line 1257 "../../src/expr2.c"
_au1_c = '\b' ;
break ;
case 'f' : 
#line 1260 "../../src/expr2.c"
_au1_c = '\f' ;
break ;
case 'n' : 
#line 1263 "../../src/expr2.c"
_au1_c = '\n' ;
break ;
case 'r' : 
#line 1266 "../../src/expr2.c"
_au1_c = '\r' ;
break ;
case 't' : 
#line 1269 "../../src/expr2.c"
_au1_c = '\t' ;
break ;
case '\\' : 
#line 1272 "../../src/expr2.c"
_au1_c = '\\' ;
break ;
case '\'' : 
#line 1275 "../../src/expr2.c"
_au1_c = '\'' ;
break ;
}

#line 1279 "../../src/expr2.c"
default : 
#line 1280 "../../src/expr2.c"
if (_au1_i )_au1_i <<= BI_IN_BYTE ;
_au1_i += _au1_c ;
}
}
;

#line 1286 "../../src/expr2.c"

#line 1288 "../../src/expr2.c"
extern int str_to_int (_au0_p )register char *_au0_p ;

#line 1292 "../../src/expr2.c"
{ 
#line 1293 "../../src/expr2.c"
register int _au1_c ;
register int _au1_i ;

#line 1294 "../../src/expr2.c"
_au1_i = 0 ;

#line 1296 "../../src/expr2.c"
if ((_au1_c = (*(_au0_p ++ )))== '0' ){ 
#line 1297 "../../src/expr2.c"
switch (_au1_c = (*(_au0_p ++ ))){ 
#line 1298 "../../src/expr2.c"
case 0 : 
#line 1299 "../../src/expr2.c"
return (int )0 ;
#line 1299 "../../src/expr2.c"

#line 1301 "../../src/expr2.c"
case 'l' : 
#line 1302 "../../src/expr2.c"
case 'L' : 
#line 1303 "../../src/expr2.c"
return (int )0 ;

#line 1305 "../../src/expr2.c"
case 'x' : 
#line 1306 "../../src/expr2.c"
case 'X' : 
#line 1307 "../../src/expr2.c"
while (_au1_c = (*(_au0_p ++ )))
#line 1308 "../../src/expr2.c"
switch (_au1_c ){ 
#line 1309 "../../src/expr2.c"
case 'l' : 
#line 1310 "../../src/expr2.c"
case 'L' : 
#line 1311 "../../src/expr2.c"
return
#line 1311 "../../src/expr2.c"
_au1_i ;
case 'A' : 
#line 1313 "../../src/expr2.c"
case 'B' : 
#line 1314 "../../src/expr2.c"
case 'C' : 
#line 1315 "../../src/expr2.c"
case 'D' : 
#line 1316 "../../src/expr2.c"
case 'E' : 
#line 1317 "../../src/expr2.c"
case 'F' : 
#line 1318 "../../src/expr2.c"
_au1_i = (((_au1_i *
#line 1318 "../../src/expr2.c"
16 )+ _au1_c )- 55);
break ;
case 'a' : 
#line 1321 "../../src/expr2.c"
case 'b' : 
#line 1322 "../../src/expr2.c"
case 'c' : 
#line 1323 "../../src/expr2.c"
case 'd' : 
#line 1324 "../../src/expr2.c"
case 'e' : 
#line 1325 "../../src/expr2.c"
case 'f' : 
#line 1326 "../../src/expr2.c"
_au1_i = (((_au1_i *
#line 1326 "../../src/expr2.c"
16 )+ _au1_c )- 87);
break ;
default : 
#line 1329 "../../src/expr2.c"
_au1_i = (((_au1_i * 16 )+ _au1_c )- '0' );
}
return _au1_i ;

#line 1333 "../../src/expr2.c"
default : 
#line 1334 "../../src/expr2.c"
do 
#line 1335 "../../src/expr2.c"
switch (_au1_c ){ 
#line 1336 "../../src/expr2.c"
case 'l' : 
#line 1337 "../../src/expr2.c"
case 'L' : 
#line 1338 "../../src/expr2.c"
return _au1_i ;
default : 
#line 1340 "../../src/expr2.c"
_au1_i = (((_au1_i * 8 )+ _au1_c )- '0' );
}
while (_au1_c = (*(_au0_p ++ )));
return _au1_i ;
}
}

#line 1347 "../../src/expr2.c"
_au1_i = (_au1_c - '0' );
while (_au1_c = (*(_au0_p ++ )))
#line 1349 "../../src/expr2.c"
switch (_au1_c ){ 
#line 1350 "../../src/expr2.c"
case 'l' : 
#line 1351 "../../src/expr2.c"
case 'L' : 
#line 1352 "../../src/expr2.c"
return _au1_i ;
default : 
#line 1354 "../../src/expr2.c"
_au1_i = (((_au1_i * 10 )+ _au1_c )- '0' );
}
return _au1_i ;
}
;

#line 1361 "../../src/expr2.c"
char *Neval = 0 ;
bit binary_val ;

#line 1364 "../../src/expr2.c"
int _expr_eval (_au0_this )
#line 525 "../../src/cfront.h"
struct expr *_au0_this ;

#line 1365 "../../src/expr2.c"
{ 
#line 1366 "../../src/expr2.c"
if (Neval )return 1 ;

#line 1368 "../../src/expr2.c"
switch (_au0_this -> _node_base ){ 
#line 1369 "../../src/expr2.c"
case 86 : return (int )0 ;
case 150 : return _au0_this -> _expr__O3.__C3_i1 ;
case 82 : return str_to_int ( (char *)_au0_this -> _expr__O3.__C3_string ) ;
case 84 : return char_to_int ( _au0_this -> _expr__O3.__C3_string ) ;
case 83 : Neval = "float in constant expression";

#line 1373 "../../src/expr2.c"
return 1 ;
case 81 : Neval = "string in constant expression";

#line 1374 "../../src/expr2.c"
return 1 ;
case 121 : return (((struct name *)_au0_this ))-> _name_n_val ;
case 30 : return _type_tsizeof ( _au0_this -> _expr__O5.__C5_tp2 ) ;
case 85 : 
#line 1378 "../../src/expr2.c"
{ Pname _au3_n ;

#line 1378 "../../src/expr2.c"
_au3_n = (((struct name *)_au0_this ));
if (_au3_n -> _name_n_evaluated && (_au3_n -> _name_n_scope != 136 ))return _au3_n -> _name_n_val ;
if (((_au3_n -> _expr__O4.__C4_n_initializer && (_au3_n -> _name_n_scope != 136 ))&& (_au3_n -> _expr__O4.__C4_n_initializer -> _node_base == 150 ))&& (_au3_n -> _expr__O4.__C4_n_initializer -> _expr__O3.__C3_i1 == _au3_n ->
#line 1380 "../../src/expr2.c"
_name_n_val ))
#line 1383 "../../src/expr2.c"
return _au3_n -> _name_n_val ;
if (binary_val && (strcmp ( (char *)_au0_this -> _expr__O3.__C3_string , (char *)"_result") == 0 ))return 8888 ;
Neval = "cannot evaluate constant";
return 1 ;
}
case 168 : 
#line 1389 "../../src/expr2.c"
if (_au0_this -> _expr__O3.__C3_e1 ){ 
#line 1390 "../../src/expr2.c"
_au0_this -> _expr__O5.__C5_il -> _iline_i_next = curr_icall ;
curr_icall = _au0_this -> _expr__O5.__C5_il ;
{ int _au3_i ;

#line 1392 "../../src/expr2.c"
_au3_i = _expr_eval ( _au0_this -> _expr__O3.__C3_e1 ) ;
curr_icall = _au0_this -> _expr__O5.__C5_il -> _iline_i_next ;
return _au3_i ;
}
}

#line 1396 "../../src/expr2.c"
Neval = "void inlineF";
return 1 ;
case 169 : 
#line 1399 "../../src/expr2.c"
{ Pname _au3_n ;
int _au3_argno ;
Pin _au3_il ;

#line 1399 "../../src/expr2.c"
_au3_n = (((struct name *)_au0_this ));
_au3_argno = _au3_n -> _name_n_val ;

#line 1402 "../../src/expr2.c"
for(_au3_il = curr_icall ;_au3_il ;_au3_il = _au3_il -> _iline_i_next ) 
#line 1403 "../../src/expr2.c"
if (_au3_il -> _iline_i_table == _au3_n -> _expr__O5.__C5_n_table )goto aok ;
goto bok ;
aok :
#line 1406 "../../src/expr2.c"
if (_au3_il -> _iline_local [_au3_argno ]){ 
#line 1407 "../../src/expr2.c"
bok :
#line 1408 "../../src/expr2.c"
Neval = "inlineF call too complicated for constant expression";
return 1 ;
}
{ Pexpr _au3_aa ;

#line 1411 "../../src/expr2.c"
_au3_aa = (_au3_il -> _iline_arg [_au3_argno ]);
return _expr_eval ( _au3_aa ) ;
}
}

#line 1414 "../../src/expr2.c"
case 113 : 
#line 1415 "../../src/expr2.c"
{ int _au3_i ;
Ptype _au3_tt ;

#line 1415 "../../src/expr2.c"
_au3_i = _expr_eval ( _au0_this -> _expr__O3.__C3_e1 ) ;
_au3_tt = _au0_this -> _expr__O5.__C5_tp2 ;
strip :
#line 1418 "../../src/expr2.c"
switch (_au3_tt -> _node_base ){ 
#line 1419 "../../src/expr2.c"
case 97 : 
#line 1420 "../../src/expr2.c"
_au3_tt = (((struct basetype *)_au3_tt ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
goto strip ;
case 22 : 
#line 1423 "../../src/expr2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"cast to long in constantE (ignored)", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 1423 "../../src/expr2.c"
ea *)ea0 ) ;
break ;
case 21 : 
#line 1426 "../../src/expr2.c"
case 5 : 
#line 1427 "../../src/expr2.c"
case 29 : 
#line 1428 "../../src/expr2.c"
if ((((struct basetype *)_au0_this -> _expr__O5.__C5_tp2 ))-> _basetype_b_unsigned && (_au3_i < 0 ))
#line 1429 "../../src/expr2.c"
Neval = "cast to unsigned in constant expression";
#line 1429 "../../src/expr2.c"
else 
#line 1430 "../../src/expr2.c"
{ 
#line 1431 "../../src/expr2.c"
int _au5_diff ;

#line 1431 "../../src/expr2.c"
_au5_diff = (_type_tsizeof ( (struct type *)int_type ) - _type_tsizeof ( _au0_this -> _expr__O5.__C5_tp2 ) );
if (_au5_diff ){ 
#line 1433 "../../src/expr2.c"
int _au6_bits ;
int _au6_div ;

#line 1433 "../../src/expr2.c"
_au6_bits = (_au5_diff * BI_IN_BYTE );
_au6_div = 256 ;
if (BI_IN_BYTE != 8 ){ 
#line 1436 "../../src/expr2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"expr::eval() assumes 8 bit bytes please re-write it", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ,
#line 1436 "../../src/expr2.c"
(struct ea *)ea0 ) ;
}
while (-- _au5_diff )_au6_div *= 256 ;
_au3_i = ((_au3_i << _au6_bits )/ _au6_div );
}
}
}
return _au3_i ;
}
case 107 : 
#line 1446 "../../src/expr2.c"
case 172 : 
#line 1447 "../../src/expr2.c"
case 46 : 
#line 1448 "../../src/expr2.c"
case 47 : 
#line 1449 "../../src/expr2.c"
case 54 : 
#line 1450 "../../src/expr2.c"
case 55 : 
#line 1451 "../../src/expr2.c"
case 50 :
#line 1451 "../../src/expr2.c"

#line 1452 "../../src/expr2.c"
case 56 : 
#line 1453 "../../src/expr2.c"
case 57 : 
#line 1454 "../../src/expr2.c"
case 63 : 
#line 1455 "../../src/expr2.c"
case 58 : 
#line 1456 "../../src/expr2.c"
case 59 : 
#line 1457 "../../src/expr2.c"
case 60 : 
#line 1458 "../../src/expr2.c"
case 61 :
#line 1458 "../../src/expr2.c"

#line 1459 "../../src/expr2.c"
case 52 : 
#line 1460 "../../src/expr2.c"
case 65 : 
#line 1461 "../../src/expr2.c"
case 64 : 
#line 1462 "../../src/expr2.c"
case 51 : 
#line 1463 "../../src/expr2.c"
case 53 : 
#line 1464 "../../src/expr2.c"
case 68 : 
#line 1465 "../../src/expr2.c"
case 62 :
#line 1465 "../../src/expr2.c"

#line 1466 "../../src/expr2.c"
case 66 : 
#line 1467 "../../src/expr2.c"
case 67 : 
#line 1468 "../../src/expr2.c"
break ;
case 145 : 
#line 1470 "../../src/expr2.c"
case 112 : 
#line 1471 "../../src/expr2.c"
if (binary_val ){ 
#line 1472 "../../src/expr2.c"
switch (_au0_this -> _expr__O4.__C4_e2 -> _node_base ){ 
#line 1473 "../../src/expr2.c"
case 85 : 
#line 1474 "../../src/expr2.c"
case 45 :
#line 1474 "../../src/expr2.c"

#line 1475 "../../src/expr2.c"
case 44 : return 9999 ;
}
}
default : 
#line 1479 "../../src/expr2.c"
Neval = "bad operator in constant expression";
return 1 ;
}

#line 1483 "../../src/expr2.c"
{ int _au1_i1 ;
int _au1_i2 ;

#line 1483 "../../src/expr2.c"
_au1_i1 = (_au0_this -> _expr__O3.__C3_e1 ? _expr_eval ( _au0_this -> _expr__O3.__C3_e1 ) : 0);
_au1_i2 = (_au0_this -> _expr__O4.__C4_e2 ? _expr_eval ( _au0_this -> _expr__O4.__C4_e2 ) : 0);

#line 1486 "../../src/expr2.c"
switch (_au0_this -> _node_base ){ 
#line 1487 "../../src/expr2.c"
case 107 : return (- _au1_i2 );
case 172 : return _au1_i2 ;
case 46 : return (! _au1_i2 );
case 47 : return (~ _au1_i2 );
case 113 : return _au1_i1 ;
case 54 : return (_au1_i1 + _au1_i2 );
case 55 : return (_au1_i1 - _au1_i2 );
case 50 : return (_au1_i1 * _au1_i2 );
case 56 : return (_au1_i1 << _au1_i2 );
case 57 : return (_au1_i1 >> _au1_i2 );
case 63 : return (_au1_i1 != _au1_i2 );
case 62 : return (_au1_i1 == _au1_i2 );
case 58 : return (_au1_i1 < _au1_i2 );
case 59 : return (_au1_i1 <= _au1_i2 );
case 60 : return (_au1_i1 > _au1_i2 );
case 61 : return (_au1_i1 >= _au1_i2 );
case 52 : return (_au1_i1 & _au1_i2 );
case 66 : return (_au1_i1 && _au1_i2 );
case 65 : return (_au1_i1 | _au1_i2 );
case 67 : return (_au1_i1 || _au1_i2 );
case 64 : return (_au1_i1 ^ _au1_i2 );
case 53 : return ((_au1_i2 == 0 )? 1 : (_au1_i1 % _au1_i2 ));
case 68 : return (_expr_eval ( _au0_this -> _expr__O5.__C5_cond ) ? _au1_i1 : _au1_i2 );
case 51 : if (_au1_i2 == 0 ){ 
#line 1511 "../../src/expr2.c"
Neval = "divide by zero";
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"divide by zero", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 1512 "../../src/expr2.c"

#line 1513 "../../src/expr2.c"
return 1 ;
}
return (_au1_i1 / _au1_i2 );
}
}
}
;

#line 1519 "../../src/expr2.c"
bit _classdef_baseofFPCname___ (_au0_this , _au0_f )
#line 319 "../../src/cfront.h"
struct classdef *_au0_this ;

#line 1519 "../../src/expr2.c"
Pname _au0_f ;

#line 1524 "../../src/expr2.c"
{ 
#line 1525 "../../src/expr2.c"
Ptable _au1_ctbl ;
Pname _au1_b ;

#line 1525 "../../src/expr2.c"
_au1_ctbl = _au0_f -> _expr__O5.__C5_n_table ;
_au1_b = _au1_ctbl -> _table_t_name ;

#line 1528 "../../src/expr2.c"
for(;;) { 
#line 1529 "../../src/expr2.c"
if (_au1_b == 0 )return (char )0 ;
{ Pclass _au2_cl ;

#line 1530 "../../src/expr2.c"
_au2_cl = (((struct classdef *)_au1_b -> _expr__O2.__C2_tp ));
if (_au2_cl == 0 )return (char )0 ;
if (_au2_cl == _au0_this )return (char )1 ;

#line 1534 "../../src/expr2.c"
if (_au2_cl -> _classdef_pubbase == 0 )
#line 1535 "../../src/expr2.c"
return (char )(_au2_cl -> _classdef_clbase && (_au2_cl -> _classdef_clbase -> _expr__O2.__C2_tp == (struct type *)_au0_this ));
_au1_b = _au2_cl -> _classdef_clbase ;
}
}
}
;

#line 1540 "../../src/expr2.c"
bit _classdef_baseofFPCclassdef___ (_au0_this , _au0_cl )
#line 319 "../../src/cfront.h"
struct classdef *_au0_this ;

#line 1540 "../../src/expr2.c"
Pclass _au0_cl ;

#line 1544 "../../src/expr2.c"
{ 
#line 1545 "../../src/expr2.c"
for(;;) { 
#line 1546 "../../src/expr2.c"
if (_au0_cl == 0 )return (char )0 ;
if (_au0_cl == _au0_this )return (char )1 ;
if (((_au0_cl -> _classdef_pubbase == 0 )&& _au0_this -> _classdef_clbase )&& (_au0_cl != (struct classdef *)_au0_this -> _classdef_clbase -> _expr__O2.__C2_tp ))return (char )0 ;

#line 1551 "../../src/expr2.c"
{ Pname _au2_b ;

#line 1551 "../../src/expr2.c"
_au2_b = _au0_cl -> _classdef_clbase ;
if (_au2_b == 0 )return (char )0 ;
_au0_cl = (((struct classdef *)_au2_b -> _expr__O2.__C2_tp ));
}
}
}
;

#line 1557 "../../src/expr2.c"
bit _classdef_has_friend (_au0_this , _au0_f )
#line 319 "../../src/cfront.h"
struct classdef *_au0_this ;

#line 1557 "../../src/expr2.c"
Pname _au0_f ;

#line 1561 "../../src/expr2.c"
{ 
#line 1563 "../../src/expr2.c"
Ptable _au1_ctbl ;
Plist _au1_l ;

#line 1562 "../../src/expr2.c"
if (_au0_f == 0 )return (char )0 ;
_au1_ctbl = _au0_f -> _expr__O5.__C5_n_table ;
for(_au1_l = _au0_this -> _classdef_friend_list ;_au1_l ;_au1_l = _au1_l -> _name_list_l ) { 
#line 1565 "../../src/expr2.c"
Pname _au2_fr ;

#line 1565 "../../src/expr2.c"
_au2_fr = _au1_l -> _name_list_f ;
switch (_au2_fr -> _expr__O2.__C2_tp -> _node_base ){ 
#line 1567 "../../src/expr2.c"
case 6 : 
#line 1568 "../../src/expr2.c"
if ((((struct classdef *)_au2_fr -> _expr__O2.__C2_tp ))-> _classdef_memtbl == _au1_ctbl )return (char )1 ;
#line 1568 "../../src/expr2.c"

#line 1569 "../../src/expr2.c"
break ;
case 119 : 
#line 1571 "../../src/expr2.c"
if ((((struct basetype *)_au2_fr -> _expr__O2.__C2_tp ))-> _basetype_b_table == _au1_ctbl )return (char )1 ;
break ;
case 108 : 
#line 1574 "../../src/expr2.c"
if (_au2_fr == _au0_f )return (char )1 ;
break ;
case 76 : 
#line 1577 "../../src/expr2.c"
_au1_l -> _name_list_f = (_au2_fr = (((struct gen *)_au2_fr -> _expr__O2.__C2_tp ))-> _gen_fct_list -> _name_list_f );
if (_au2_fr == _au0_f )return (char )1 ;
break ;
default : 
#line 1581 "../../src/expr2.c"
{ 
#line 1585 "../../src/expr2.c"
struct ea _au0__V91 ;

#line 1581 "../../src/expr2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"bad friend %k", (struct ea *)( ( ((& _au0__V91 )-> _ea__O1.__C1_i = ((int )_au2_fr -> _expr__O2.__C2_tp ->
#line 1581 "../../src/expr2.c"
_node_base )), (((& _au0__V91 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}
return (char )0 ;
}
;
Pexpr pt (_au0_ef , _au0_e , _au0_tbl )Pfct _au0_ef ;

#line 1587 "../../src/expr2.c"
Pexpr _au0_e ;

#line 1587 "../../src/expr2.c"
Ptable _au0_tbl ;

#line 1591 "../../src/expr2.c"
{ 
#line 1592 "../../src/expr2.c"
Pfct _au1_f ;
Pname _au1_n ;

#line 1593 "../../src/expr2.c"
_au1_n = 0 ;

#line 1595 "../../src/expr2.c"
switch (_au0_e -> _node_base ){ 
#line 1596 "../../src/expr2.c"
case 85 : 
#line 1597 "../../src/expr2.c"
_au1_f = (((struct fct *)_au0_e -> _expr__O2.__C2_tp ));
_au1_n = (((struct name *)_au0_e ));
switch (_au1_f -> _node_base ){ 
#line 1600 "../../src/expr2.c"
case 108 : 
#line 1601 "../../src/expr2.c"
case 76 : 
#line 1602 "../../src/expr2.c"
_au0_e = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char
#line 1602 "../../src/expr2.c"
)145 , (struct expr *)0 , _au0_e ) ;
_au0_e -> _expr__O2.__C2_tp = (struct type *)_au1_f ;
}
goto ad ;

#line 1607 "../../src/expr2.c"
case 45 : 
#line 1608 "../../src/expr2.c"
case 44 : 
#line 1609 "../../src/expr2.c"
_au1_f = (((struct fct *)_au0_e -> _expr__O5.__C5_mem -> _expr__O2.__C2_tp ));
switch (_au1_f -> _node_base ){ 
#line 1611 "../../src/expr2.c"
case 108 : 
#line 1612 "../../src/expr2.c"
case 76 : 
#line 1613 "../../src/expr2.c"
_au1_n = (((struct name *)_au0_e -> _expr__O5.__C5_mem ));
_au0_e = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )145 , (struct expr *)0 , _au0_e ) ;
_au0_e = _expr_typ ( _au0_e , _au0_tbl ) ;
}
goto ad ;

#line 1619 "../../src/expr2.c"
case 112 : 
#line 1620 "../../src/expr2.c"
case 145 : 
#line 1621 "../../src/expr2.c"
_au1_f = (((struct fct *)_au0_e -> _expr__O4.__C4_e2 -> _expr__O2.__C2_tp ));
ad :
#line 1623 "../../src/expr2.c"
if (_au1_f -> _node_base == 76 ){ 
#line 1624 "../../src/expr2.c"
Pgen _au3_g ;

#line 1624 "../../src/expr2.c"
_au3_g = (((struct gen *)_au1_f ));
_au1_n = _gen_find ( _au3_g , _au0_ef , (char )0 ) ;
if (_au1_n == 0 ){ 
#line 1633 "../../src/expr2.c"
struct ea _au0__V92 ;

#line 1626 "../../src/expr2.c"
error ( (char *)"cannot deduceT for &overloaded %s()", (struct ea *)( ( ((& _au0__V92 )-> _ea__O1.__C1_p = ((char *)_au3_g -> _gen_string )), (((& _au0__V92 ))))
#line 1626 "../../src/expr2.c"
) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} _au0_e -> _expr__O4.__C4_e2 = (struct expr *)_au1_n ;
_au0_e -> _expr__O2.__C2_tp = _au1_n -> _expr__O2.__C2_tp ;
}
if (_au1_n )_expr_lval ( (struct expr *)_au1_n , (unsigned char )112 ) ;
}
return _au0_e ;
}
;
extern Pexpr ptr_init (_au0_p , _au0_init , _au0_tbl )Pptr _au0_p ;

#line 1635 "../../src/expr2.c"
Pexpr _au0_init ;

#line 1635 "../../src/expr2.c"
Ptable _au0_tbl ;

#line 1642 "../../src/expr2.c"
{ 
#line 1643 "../../src/expr2.c"
Ptype _au1_it ;

#line 1643 "../../src/expr2.c"
_au1_it = _au0_init -> _expr__O2.__C2_tp ;
itl :
#line 1646 "../../src/expr2.c"
switch (_au1_it -> _node_base ){ 
#line 1647 "../../src/expr2.c"
case 97 : 
#line 1648 "../../src/expr2.c"
_au1_it = (((struct basetype *)_au1_it ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 1648 "../../src/expr2.c"
goto itl ;
case 138 : 
#line 1650 "../../src/expr2.c"
if (_au0_init == zero )break ;
case 21 : 
#line 1652 "../../src/expr2.c"
case 5 : 
#line 1653 "../../src/expr2.c"
case 29 : 
#line 1654 "../../src/expr2.c"
{ Neval = 0 ;
{ int _au3_i ;

#line 1655 "../../src/expr2.c"
_au3_i = _expr_eval ( _au0_init ) ;
if (Neval )
#line 1657 "../../src/expr2.c"
{ 
#line 1676 "../../src/expr2.c"
struct ea _au0__V93 ;

#line 1657 "../../src/expr2.c"
error ( (char *)"badPIr: %s", (struct ea *)( ( ((& _au0__V93 )-> _ea__O1.__C1_p = ((char *)Neval )), (((& _au0__V93 )))) )
#line 1657 "../../src/expr2.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} else if (_au3_i )
#line 1659 "../../src/expr2.c"
{ 
#line 1676 "../../src/expr2.c"
struct ea _au0__V94 ;

#line 1659 "../../src/expr2.c"
error ( (char *)"badPIr value %d", (struct ea *)( ( ((& _au0__V94 )-> _ea__O1.__C1_i = _au3_i ), (((& _au0__V94 )))) ) ,
#line 1659 "../../src/expr2.c"
(struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} else { 
#line 1661 "../../src/expr2.c"
if (_au0_init && (_au0_init -> _node_permanent == 0 ))_expr_del ( _au0_init ) ;
_au0_init = zero ;
}
break ;
}
}

#line 1666 "../../src/expr2.c"
case 22 : 
#line 1667 "../../src/expr2.c"
if (((_au0_init -> _node_base == 82 )&& ((_au0_init -> _expr__O3.__C3_string [0 ])== '0' ))&& (((_au0_init -> _expr__O3.__C3_string [1 ])== 'L' )|| ((_au0_init -> _expr__O3.__C3_string [1 ])==
#line 1667 "../../src/expr2.c"
'l' )))
#line 1669 "../../src/expr2.c"
{ 
#line 1670 "../../src/expr2.c"
if (_au0_init && (_au0_init -> _node_permanent == 0 ))_expr_del ( _au0_init ) ;
_au0_init = zero ;
}
}

#line 1675 "../../src/expr2.c"
return ((_au0_p -> _pvtyp_typ -> _node_base == 108 )? pt ( ((struct fct *)_au0_p -> _pvtyp_typ ), _au0_init , _au0_tbl ) : _au0_init );
}
;
Pexpr _expr_try_to_overload (_au0_this , _au0_tbl )
#line 525 "../../src/cfront.h"
struct expr *_au0_this ;

#line 1678 "../../src/expr2.c"
Ptable _au0_tbl ;
{ 
#line 1680 "../../src/expr2.c"
TOK _au1_bb ;

#line 1682 "../../src/expr2.c"
Pname _au1_n1 ;
Ptype _au1_t1 ;

#line 1691 "../../src/expr2.c"
Pname _au1_n2 ;
Ptype _au1_t2 ;

#line 1703 "../../src/expr2.c"
Pexpr _au1_oe2 ;
Pexpr _au1_ee2 ;
Pexpr _au1_ee1 ;
char *_au1_obb ;
Pname _au1_gname ;
int _au1_go ;
int _au1_nc ;

#line 1711 "../../src/expr2.c"
int _au1_ns ;

#line 1680 "../../src/expr2.c"
_au1_bb = (((_au0_this -> _node_base == 111 )&& (_au0_this -> _expr__O4.__C4_e2 == 0 ))? (((unsigned int )50 )): (((unsigned int )_au0_this -> _node_base )));
#line 1680 "../../src/expr2.c"

#line 1682 "../../src/expr2.c"
_au1_n1 = 0 ;
_au1_t1 = 0 ;
if (_au0_this -> _expr__O3.__C3_e1 ){ 
#line 1685 "../../src/expr2.c"
_au1_t1 = _au0_this -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp ;
{ Ptype _au2_tx ;

#line 1686 "../../src/expr2.c"
_au2_tx = _au1_t1 ;
while (_au2_tx -> _node_base == 97 )_au2_tx = (((struct basetype *)_au2_tx ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
_au1_n1 = _type_is_cl_obj ( _au2_tx ) ;
}
}
_au1_n2 = 0 ;

#line 1691 "../../src/expr2.c"
_au1_t2 = 0 ;

#line 1693 "../../src/expr2.c"
if (_au0_this -> _expr__O4.__C4_e2 ){ 
#line 1694 "../../src/expr2.c"
_au1_t2 = _au0_this -> _expr__O4.__C4_e2 -> _expr__O2.__C2_tp ;
{ Ptype _au2_tx ;

#line 1695 "../../src/expr2.c"
_au2_tx = _au1_t2 ;
while (_au2_tx -> _node_base == 97 )_au2_tx = (((struct basetype *)_au2_tx ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
_au1_n2 = _type_is_cl_obj ( _au2_tx ) ;
}
}
if ((_au1_n1 == 0 )&& (_au1_n2 == 0 ))return (struct expr *)0 ;

#line 1703 "../../src/expr2.c"
_au1_oe2 = _au0_this -> _expr__O4.__C4_e2 ;

#line 1703 "../../src/expr2.c"
_au1_ee2 = ((_au0_this -> _expr__O4.__C4_e2 && (_au0_this -> _expr__O4.__C4_e2 -> _node_base != 140 ))? (_au0_this -> _expr__O4.__C4_e2 = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned
#line 1703 "../../src/expr2.c"
char )140 , _au0_this -> _expr__O4.__C4_e2 , (struct expr *)0 ) ): (((struct expr *)0 )));

#line 1703 "../../src/expr2.c"
_au1_ee1 = (_au0_this -> _expr__O3.__C3_e1 ? _expr__ctor ( (struct expr *)0 , (unsigned char )140 , _au0_this -> _expr__O3.__C3_e1 , _au0_this -> _expr__O4.__C4_e2 ) :
#line 1703 "../../src/expr2.c"
_au1_ee2 );

#line 1703 "../../src/expr2.c"
_au1_obb = oper_name ( _au1_bb ) ;

#line 1703 "../../src/expr2.c"
_au1_gname = _table_look ( gtbl , _au1_obb , (unsigned char )0 ) ;

#line 1703 "../../src/expr2.c"
_au1_go = (_au1_gname ? over_call ( _au1_gname , _au1_ee1 ) : 0);

#line 1703 "../../src/expr2.c"
_au1_nc = Nover_coerce ;

#line 1710 "../../src/expr2.c"
if (_au1_go )_au1_gname = Nover ;
_au1_ns = Nstd ;

#line 1713 "../../src/expr2.c"
if (_au1_n1 ){ 
#line 1714 "../../src/expr2.c"
Ptable _au2_ctbl ;
Pname _au2_mname ;

#line 1714 "../../src/expr2.c"
_au2_ctbl = (((struct classdef *)_au1_n1 -> _expr__O2.__C2_tp ))-> _classdef_memtbl ;
_au2_mname = _table_look ( _au2_ctbl , _au1_obb , (unsigned char )0 ) ;
if (_au2_mname == 0 )goto glob ;
switch (_au2_mname -> _name_n_scope ){ 
#line 1718 "../../src/expr2.c"
default : goto glob ;
case 0 : 
#line 1720 "../../src/expr2.c"
case 25 : break ;
}

#line 1723 "../../src/expr2.c"
{ int _au2_mo ;

#line 1724 "../../src/expr2.c"
struct ref *_au0__Xthis__ctor_ref ;

#line 1723 "../../src/expr2.c"
_au2_mo = over_call ( _au2_mname , _au0_this -> _expr__O4.__C4_e2 ) ;

#line 1725 "../../src/expr2.c"
switch (_au2_mo ){ 
#line 1726 "../../src/expr2.c"
case 0 : 
#line 1727 "../../src/expr2.c"
if (_au1_go == 2 )goto glob ;
if (1 < Nover_coerce )goto am1 ;
goto glob ;
case 1 : if (_au1_go == 2 )goto glob ;
if (_au1_go == 1 ){ 
#line 1732 "../../src/expr2.c"
am1 :
#line 1733 "../../src/expr2.c"
{ 
#line 1925 "../../src/expr2.c"
struct ea _au0__V95 ;

#line 1925 "../../src/expr2.c"
struct ea _au0__V96 ;

#line 1925 "../../src/expr2.c"
struct ea _au0__V97 ;

#line 1733 "../../src/expr2.c"
error ( (char *)"ambiguous operandTs%n and%t for%k", (struct ea *)( ( ((& _au0__V95 )-> _ea__O1.__C1_p = ((char *)_au1_n1 )), (((& _au0__V95 )))) )
#line 1733 "../../src/expr2.c"
, (struct ea *)( ( ((& _au0__V96 )-> _ea__O1.__C1_p = ((char *)_au1_t2 )), (((& _au0__V96 )))) ) , (struct
#line 1733 "../../src/expr2.c"
ea *)( ( ((& _au0__V97 )-> _ea__O1.__C1_i = ((int )_au1_bb )), (((& _au0__V97 )))) ) , (struct ea *)ea0 ) ;
#line 1733 "../../src/expr2.c"

#line 1734 "../../src/expr2.c"
_au0_this -> _expr__O2.__C2_tp = (struct type *)any_type ;
return _au0_this ;
} }
else if ((((struct classdef *)_au1_n1 -> _expr__O2.__C2_tp ))-> _classdef_conv ){ 
#line 1738 "../../src/expr2.c"
switch (_au1_bb ){ 
#line 1739 "../../src/expr2.c"
case 70 : 
#line 1740 "../../src/expr2.c"
case 126 : 
#line 1741 "../../src/expr2.c"
case
#line 1741 "../../src/expr2.c"
127 : 
#line 1742 "../../src/expr2.c"
case 128 : 
#line 1743 "../../src/expr2.c"
case 129 : 
#line 1744 "../../src/expr2.c"
case 130 : 
#line 1745 "../../src/expr2.c"
case 131 : 
#line 1746 "../../src/expr2.c"
case 132 : 
#line 1747 "../../src/expr2.c"
case 133 : 
#line 1748 "../../src/expr2.c"
case
#line 1748 "../../src/expr2.c"
134 : 
#line 1749 "../../src/expr2.c"
case 135 : 
#line 1751 "../../src/expr2.c"
break ;
default : 
#line 1753 "../../src/expr2.c"
if (_type_is_cl_obj ( (((struct fct *)(((struct classdef *)_au1_n1 -> _expr__O2.__C2_tp ))-> _classdef_conv -> _expr__O2.__C2_tp ))-> _fct_returns ) )break ;
{ 
#line 1925 "../../src/expr2.c"
struct ea _au0__V98 ;

#line 1925 "../../src/expr2.c"
struct ea _au0__V99 ;

#line 1754 "../../src/expr2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"overloaded%k may be ambiguous.FWT%tused", (struct ea *)( ( ((& _au0__V98 )-> _ea__O1.__C1_i = ((int )_au1_bb )), (((&
#line 1754 "../../src/expr2.c"
_au0__V98 )))) ) , (struct ea *)( ( ((& _au0__V99 )-> _ea__O1.__C1_p = ((char *)Nover -> _expr__O2.__C2_tp )), (((& _au0__V99 ))))
#line 1754 "../../src/expr2.c"
) , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}
break ;
case 2 : 
#line 1759 "../../src/expr2.c"
if ((_au1_go == 2 )&& (_au1_ns <= Nstd ))
#line 1760 "../../src/expr2.c"
{ 
#line 1925 "../../src/expr2.c"
struct ea _au0__V100 ;

#line 1925 "../../src/expr2.c"
struct ea _au0__V101 ;

#line 1925 "../../src/expr2.c"
struct ea _au0__V102 ;

#line 1760 "../../src/expr2.c"
error ( (char *)"%k defined both as%n and%n", (struct ea *)( ( ((& _au0__V100 )-> _ea__O1.__C1_i = ((int )_au1_bb )), (((& _au0__V100 )))) )
#line 1760 "../../src/expr2.c"
, (struct ea *)( ( ((& _au0__V101 )-> _ea__O1.__C1_p = ((char *)_au1_gname )), (((& _au0__V101 )))) ) , (struct
#line 1760 "../../src/expr2.c"
ea *)( ( ((& _au0__V102 )-> _ea__O1.__C1_p = ((char *)Nover )), (((& _au0__V102 )))) ) , (struct ea *)ea0 ) ;
#line 1760 "../../src/expr2.c"
} }

#line 1763 "../../src/expr2.c"
if ((_au1_bb == 70 )&& (_au2_mname -> _expr__O5.__C5_n_table != _au2_ctbl )){ 
#line 1764 "../../src/expr2.c"
{ 
#line 1925 "../../src/expr2.c"
struct ea _au0__V103 ;

#line 1764 "../../src/expr2.c"
error ( (char *)"assignment not defined for class%n", (struct ea *)( ( ((& _au0__V103 )-> _ea__O1.__C1_p = ((char *)_au1_n1 )), (((& _au0__V103 )))) )
#line 1764 "../../src/expr2.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
_au0_this -> _expr__O2.__C2_tp = (struct type *)any_type ;
return _au0_this ;
} }

#line 1769 "../../src/expr2.c"
_au0_this -> _node_base = 146 ;
_au0_this -> _expr__O3.__C3_e1 = (struct expr *)( (_au0__Xthis__ctor_ref = 0 ), ( ( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct
#line 1770 "../../src/expr2.c"
expr *)_au0__Xthis__ctor_ref ), ((unsigned char )45 ), _au0_this -> _expr__O3.__C3_e1 , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = Nover ),
#line 1770 "../../src/expr2.c"
((_au0__Xthis__ctor_ref ))) ) ) ;
if (_au1_ee1 )_expr__dtor ( _au1_ee1 , 1) ;
return _expr_typ ( _au0_this , _au0_tbl ) ;
}
}
if (_au1_n2 && (_au0_this -> _expr__O3.__C3_e1 == 0 )){ 
#line 1776 "../../src/expr2.c"
Ptable _au2_ctbl ;
Pname _au2_mname ;

#line 1776 "../../src/expr2.c"
_au2_ctbl = (((struct classdef *)_au1_n2 -> _expr__O2.__C2_tp ))-> _classdef_memtbl ;
_au2_mname = _table_look ( _au2_ctbl , _au1_obb , (unsigned char )0 ) ;
if (_au2_mname == 0 )goto glob ;
switch (_au2_mname -> _name_n_scope ){ 
#line 1780 "../../src/expr2.c"
default : goto glob ;
case 0 : 
#line 1782 "../../src/expr2.c"
case 25 : break ;
}

#line 1785 "../../src/expr2.c"
{ int _au2_mo ;

#line 1786 "../../src/expr2.c"
struct ref *_au0__Xthis__ctor_ref ;

#line 1785 "../../src/expr2.c"
_au2_mo = over_call ( _au2_mname , (struct expr *)0 ) ;

#line 1787 "../../src/expr2.c"
switch (_au2_mo ){ 
#line 1788 "../../src/expr2.c"
case 0 : 
#line 1789 "../../src/expr2.c"
if (1 < Nover_coerce )goto am2 ;
goto glob ;
case 1 : if (_au1_go == 2 )goto glob ;
if (_au1_go == 1 ){ 
#line 1793 "../../src/expr2.c"
am2 :
#line 1794 "../../src/expr2.c"
{ 
#line 1925 "../../src/expr2.c"
struct ea _au0__V104 ;

#line 1925 "../../src/expr2.c"
struct ea _au0__V105 ;

#line 1794 "../../src/expr2.c"
error ( (char *)"ambiguous operandT%n for%k", (struct ea *)( ( ((& _au0__V104 )-> _ea__O1.__C1_p = ((char *)_au1_n2 )), (((& _au0__V104 )))) )
#line 1794 "../../src/expr2.c"
, (struct ea *)( ( ((& _au0__V105 )-> _ea__O1.__C1_i = ((int )_au1_bb )), (((& _au0__V105 )))) ) , (struct
#line 1794 "../../src/expr2.c"
ea *)ea0 , (struct ea *)ea0 ) ;
_au0_this -> _expr__O2.__C2_tp = (struct type *)any_type ;
return _au0_this ;
} }
break ;
case 2 : 
#line 1800 "../../src/expr2.c"
if ((_au1_go == 2 )&& (_au1_ns <= Nstd ))
#line 1801 "../../src/expr2.c"
{ 
#line 1925 "../../src/expr2.c"
struct ea _au0__V106 ;

#line 1925 "../../src/expr2.c"
struct ea _au0__V107 ;

#line 1925 "../../src/expr2.c"
struct ea _au0__V108 ;

#line 1801 "../../src/expr2.c"
error ( (char *)"%k defined both as%n and%n", (struct ea *)( ( ((& _au0__V106 )-> _ea__O1.__C1_i = ((int )_au1_bb )), (((& _au0__V106 )))) )
#line 1801 "../../src/expr2.c"
, (struct ea *)( ( ((& _au0__V107 )-> _ea__O1.__C1_p = ((char *)_au1_gname )), (((& _au0__V107 )))) ) , (struct
#line 1801 "../../src/expr2.c"
ea *)( ( ((& _au0__V108 )-> _ea__O1.__C1_p = ((char *)Nover )), (((& _au0__V108 )))) ) , (struct ea *)ea0 ) ;
#line 1801 "../../src/expr2.c"
} }

#line 1804 "../../src/expr2.c"
_au0_this -> _node_base = 146 ;
_au0_this -> _expr__O3.__C3_e1 = (struct expr *)( (_au0__Xthis__ctor_ref = 0 ), ( ( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct
#line 1805 "../../src/expr2.c"
expr *)_au0__Xthis__ctor_ref ), ((unsigned char )45 ), _au1_oe2 , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = Nover ), ((_au0__Xthis__ctor_ref )))
#line 1805 "../../src/expr2.c"
) ) ;
_au0_this -> _expr__O4.__C4_e2 = 0 ;
if (_au1_ee2 )_expr__dtor ( _au1_ee2 , 1) ;
if (_au1_ee1 && (_au1_ee1 != _au1_ee2 ))_expr__dtor ( _au1_ee1 , 1) ;
return _expr_typ ( _au0_this , _au0_tbl ) ;
}
}
glob :
#line 1813 "../../src/expr2.c"
if (1 < _au1_nc ){ 
#line 1814 "../../src/expr2.c"
{ 
#line 1925 "../../src/expr2.c"
struct ea _au0__V109 ;

#line 1925 "../../src/expr2.c"
struct ea _au0__V110 ;

#line 1925 "../../src/expr2.c"
struct ea _au0__V111 ;

#line 1814 "../../src/expr2.c"
error ( (char *)"ambiguous operandTs%t and%t for%k", (struct ea *)( ( ((& _au0__V109 )-> _ea__O1.__C1_p = ((char *)_au1_t1 )), (((& _au0__V109 )))) )
#line 1814 "../../src/expr2.c"
, (struct ea *)( ( ((& _au0__V110 )-> _ea__O1.__C1_p = ((char *)_au1_t2 )), (((& _au0__V110 )))) ) , (struct
#line 1814 "../../src/expr2.c"
ea *)( ( ((& _au0__V111 )-> _ea__O1.__C1_i = ((int )_au1_bb )), (((& _au0__V111 )))) ) , (struct ea *)ea0 ) ;
#line 1814 "../../src/expr2.c"

#line 1815 "../../src/expr2.c"
_au0_this -> _expr__O2.__C2_tp = (struct type *)any_type ;
return _au0_this ;
} }
if (_au1_go ){ 
#line 1819 "../../src/expr2.c"
if (_au1_go == 1 ){ 
#line 1821 "../../src/expr2.c"
if (((_au1_n1 && (((struct classdef *)_au1_n1 -> _expr__O2.__C2_tp ))-> _classdef_conv )&& (_type_is_cl_obj ( (((struct
#line 1821 "../../src/expr2.c"
fct *)(((struct classdef *)_au1_n1 -> _expr__O2.__C2_tp ))-> _classdef_conv -> _expr__O2.__C2_tp ))-> _fct_returns ) == 0 ))|| ((_au1_n2 && (((struct classdef *)_au1_n2 -> _expr__O2.__C2_tp ))-> _classdef_conv )&& (_type_is_cl_obj (
#line 1821 "../../src/expr2.c"
(((struct fct *)(((struct classdef *)_au1_n2 -> _expr__O2.__C2_tp ))-> _classdef_conv -> _expr__O2.__C2_tp ))-> _fct_returns ) == 0 )))
#line 1827 "../../src/expr2.c"
{ 
#line 1925 "../../src/expr2.c"
struct ea _au0__V112 ;

#line 1925 "../../src/expr2.c"
struct ea _au0__V113 ;

#line 1827 "../../src/expr2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"overloaded%k may be ambiguous.FWT%tused", (struct ea *)( ( ((& _au0__V112 )-> _ea__O1.__C1_i = ((int )_au1_bb )), (((&
#line 1827 "../../src/expr2.c"
_au0__V112 )))) ) , (struct ea *)( ( ((& _au0__V113 )-> _ea__O1.__C1_p = ((char *)_au1_gname -> _expr__O2.__C2_tp )), (((& _au0__V113 ))))
#line 1827 "../../src/expr2.c"
) , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
_au0_this -> _node_base = 146 ;
_au0_this -> _expr__O3.__C3_e1 = (struct expr *)_au1_gname ;
_au0_this -> _expr__O4.__C4_e2 = _au1_ee1 ;
return _expr_typ ( _au0_this , _au0_tbl ) ;
}

#line 1835 "../../src/expr2.c"
if (_au1_ee2 )_expr__dtor ( _au1_ee2 , 1) ;
if (_au1_ee1 && (_au1_ee1 != _au1_ee2 ))_expr__dtor ( _au1_ee1 , 1) ;
_au0_this -> _expr__O4.__C4_e2 = _au1_oe2 ;

#line 1839 "../../src/expr2.c"
switch (_au1_bb ){ 
#line 1840 "../../src/expr2.c"
case 70 : 
#line 1841 "../../src/expr2.c"
case 112 : 
#line 1842 "../../src/expr2.c"
break ;
case 111 : 
#line 1844 "../../src/expr2.c"
if (_au0_this -> _expr__O4.__C4_e2 ){ 
#line 1845 "../../src/expr2.c"
_au0_this -> _node_base = 46 ;
{ Pexpr _au3_x ;

#line 1846 "../../src/expr2.c"
_au3_x = _expr_try_to_overload ( _au0_this , _au0_tbl ) ;
if (_au3_x )return _au3_x ;
_au0_this -> _node_base = 111 ;
}
}

#line 1850 "../../src/expr2.c"
case 109 : 
#line 1851 "../../src/expr2.c"
if (_au1_n1 == 0 )break ;
default : 
#line 1853 "../../src/expr2.c"
{ int _au3_found ;

#line 1853 "../../src/expr2.c"
_au3_found = 0 ;
if (_au1_n1 ){ 
#line 1855 "../../src/expr2.c"
int _au4_val ;
Pclass _au4_cl ;

#line 1855 "../../src/expr2.c"
_au4_val = 0 ;
_au4_cl = (((struct classdef *)_au1_n1 -> _expr__O2.__C2_tp ));
{ Pname _au4_on ;

#line 1857 "../../src/expr2.c"
_au4_on = _au4_cl -> _classdef_conv ;

#line 1857 "../../src/expr2.c"
for(;_au4_on ;_au4_on = _au4_on -> _name_n_list ) { 
#line 1858 "../../src/expr2.c"
Pfct _au5_f ;

#line 1858 "../../src/expr2.c"
_au5_f = (((struct fct *)_au4_on -> _expr__O2.__C2_tp ));
if ((_au1_bb == 66 )|| (_au1_bb == 67 )){ 
#line 1860 "../../src/expr2.c"
_au0_this -> _expr__O3.__C3_e1 = check_cond ( _au0_this -> _expr__O3.__C3_e1 , _au1_bb , _au0_tbl ) ;
return (struct expr *)0 ;
}
if ((_au1_n2 || (_au1_t2 && (_type_check ( _au5_f -> _fct_returns , _au1_t2 , (unsigned char )70 ) == 0 )))|| (_au1_t2 && (_type_check (
#line 1863 "../../src/expr2.c"
_au1_t2 , _au5_f -> _fct_returns , (unsigned char )70 ) == 0 )))
#line 1865 "../../src/expr2.c"
{ 
#line 1866 "../../src/expr2.c"
Ncoerce = _au4_on ;
_au4_val ++ ;
}
}
switch (_au4_val ){ 
#line 1871 "../../src/expr2.c"
case 0 : 
#line 1872 "../../src/expr2.c"
if (_au0_this -> _node_base == 46 )return (struct expr *)0 ;
break ;
case 1 : 
#line 1875 "../../src/expr2.c"
{ Pref _au6_r ;
Pexpr _au6_rr ;

#line 1877 "../../src/expr2.c"
struct ref *_au0__Xthis__ctor_ref ;

#line 1875 "../../src/expr2.c"
_au6_r = (struct ref *)( (_au0__Xthis__ctor_ref = 0 ), ( ( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ref ),
#line 1875 "../../src/expr2.c"
((unsigned char )45 ), _au0_this -> _expr__O3.__C3_e1 , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = Ncoerce ), ((_au0__Xthis__ctor_ref )))
#line 1875 "../../src/expr2.c"
) ) ;
_au6_rr = _expr_typ ( (struct expr *)_au6_r , _au0_tbl ) ;
_au0_this -> _expr__O3.__C3_e1 = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )146 , _au6_rr , (struct expr *)0 ) ;
_au3_found = 1 ;
break ;
}
default : 
#line 1882 "../../src/expr2.c"
{ 
#line 1925 "../../src/expr2.c"
struct ea _au0__V114 ;

#line 1882 "../../src/expr2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"ambiguous coercion of%n to basicT", (struct ea *)( ( ((& _au0__V114 )-> _ea__O1.__C1_p = ((char *)_au1_n1 )), (((&
#line 1882 "../../src/expr2.c"
_au0__V114 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}
}

#line 1885 "../../src/expr2.c"
if (_au1_n2 ){ 
#line 1886 "../../src/expr2.c"
int _au4_val ;
Pclass _au4_cl ;

#line 1886 "../../src/expr2.c"
_au4_val = 0 ;
_au4_cl = (((struct classdef *)_au1_n2 -> _expr__O2.__C2_tp ));
{ Pname _au4_on ;

#line 1888 "../../src/expr2.c"
_au4_on = _au4_cl -> _classdef_conv ;

#line 1888 "../../src/expr2.c"
for(;_au4_on ;_au4_on = _au4_on -> _name_n_list ) { 
#line 1889 "../../src/expr2.c"
Pfct _au5_f ;

#line 1889 "../../src/expr2.c"
_au5_f = (((struct fct *)_au4_on -> _expr__O2.__C2_tp ));
if (((_au1_bb == 66 )|| (_au1_bb == 67 ))|| (_au1_bb == 46 )){ 
#line 1891 "../../src/expr2.c"
_au0_this -> _expr__O4.__C4_e2 = check_cond ( _au0_this -> _expr__O4.__C4_e2 , _au1_bb , _au0_tbl )
#line 1891 "../../src/expr2.c"
;
return (struct expr *)0 ;
}
if ((_au1_n1 || (_au1_t1 && (_type_check ( _au5_f -> _fct_returns , _au1_t1 , (unsigned char )70 ) == 0 )))|| (_au1_t1 && (_type_check (
#line 1894 "../../src/expr2.c"
_au1_t1 , _au5_f -> _fct_returns , (unsigned char )70 ) == 0 )))
#line 1896 "../../src/expr2.c"
{ 
#line 1897 "../../src/expr2.c"
Ncoerce = _au4_on ;
_au4_val ++ ;
}
}
switch (_au4_val ){ 
#line 1902 "../../src/expr2.c"
case 0 : 
#line 1903 "../../src/expr2.c"
break ;
case 1 : 
#line 1905 "../../src/expr2.c"
{ Pref _au6_r ;
Pexpr _au6_rr ;

#line 1907 "../../src/expr2.c"
struct ref *_au0__Xthis__ctor_ref ;

#line 1905 "../../src/expr2.c"
_au6_r = (struct ref *)( (_au0__Xthis__ctor_ref = 0 ), ( ( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ref ),
#line 1905 "../../src/expr2.c"
((unsigned char )45 ), _au0_this -> _expr__O4.__C4_e2 , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = Ncoerce ), ((_au0__Xthis__ctor_ref )))
#line 1905 "../../src/expr2.c"
) ) ;
_au6_rr = _expr_typ ( (struct expr *)_au6_r , _au0_tbl ) ;
_au0_this -> _expr__O4.__C4_e2 = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )146 , _au6_rr , (struct expr *)0 ) ;
_au3_found ++ ;
break ;
}
default : 
#line 1912 "../../src/expr2.c"
{ 
#line 1925 "../../src/expr2.c"
struct ea _au0__V115 ;

#line 1912 "../../src/expr2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"ambiguous coercion of%n to basicT", (struct ea *)( ( ((& _au0__V115 )-> _ea__O1.__C1_p = ((char *)_au1_n2 )), (((&
#line 1912 "../../src/expr2.c"
_au0__V115 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}
}

#line 1915 "../../src/expr2.c"
if (_au3_found )return _expr_typ ( _au0_this , _au0_tbl ) ;
if (_au1_t1 && _au1_t2 )
#line 1917 "../../src/expr2.c"
{ 
#line 1925 "../../src/expr2.c"
struct ea _au0__V116 ;

#line 1925 "../../src/expr2.c"
struct ea _au0__V117 ;

#line 1925 "../../src/expr2.c"
struct ea _au0__V118 ;

#line 1917 "../../src/expr2.c"
error ( (char *)"bad operandTs%t%t for%k", (struct ea *)( ( ((& _au0__V116 )-> _ea__O1.__C1_p = ((char *)_au1_t1 )), (((& _au0__V116 )))) )
#line 1917 "../../src/expr2.c"
, (struct ea *)( ( ((& _au0__V117 )-> _ea__O1.__C1_p = ((char *)_au1_t2 )), (((& _au0__V117 )))) ) , (struct
#line 1917 "../../src/expr2.c"
ea *)( ( ((& _au0__V118 )-> _ea__O1.__C1_i = ((int )_au1_bb )), (((& _au0__V118 )))) ) , (struct ea *)ea0 ) ;
#line 1917 "../../src/expr2.c"
} else 
#line 1919 "../../src/expr2.c"
{ 
#line 1925 "../../src/expr2.c"
struct ea _au0__V119 ;

#line 1925 "../../src/expr2.c"
struct ea _au0__V120 ;

#line 1919 "../../src/expr2.c"
error ( (char *)"bad operandT%t for%k", (struct ea *)( ( ((& _au0__V119 )-> _ea__O1.__C1_p = ((char *)(_au1_t1 ? _au1_t1 : _au1_t2 ))), (((&
#line 1919 "../../src/expr2.c"
_au0__V119 )))) ) , (struct ea *)( ( ((& _au0__V120 )-> _ea__O1.__C1_i = ((int )_au1_bb )), (((& _au0__V120 )))) )
#line 1919 "../../src/expr2.c"
, (struct ea *)ea0 , (struct ea *)ea0 ) ;
} _au0_this -> _expr__O2.__C2_tp = (struct type *)any_type ;
return _au0_this ;
}
}
return (struct expr *)0 ;
}
;
extern int bound ;

#line 1929 "../../src/expr2.c"
Pexpr _expr_docast (_au0_this , _au0_tbl )
#line 525 "../../src/cfront.h"
struct expr *_au0_this ;

#line 1929 "../../src/expr2.c"
Ptable _au0_tbl ;
{ 
#line 1931 "../../src/expr2.c"
Ptype _au1_t ;
Ptype _au1_tt ;

#line 1932 "../../src/expr2.c"
_au1_tt = (_au1_t = _au0_this -> _expr__O5.__C5_tp2 );
_type_dcl ( _au1_tt , _au0_tbl ) ;
zaq :
#line 1936 "../../src/expr2.c"
switch (_au1_tt -> _node_base ){ 
#line 1937 "../../src/expr2.c"
case 97 : 
#line 1938 "../../src/expr2.c"
_au1_tt = (((struct basetype *)_au1_tt ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 1938 "../../src/expr2.c"
goto zaq ;
case 158 : 
#line 1940 "../../src/expr2.c"
case 125 : 
#line 1941 "../../src/expr2.c"
if ((((struct ptr *)_au1_tt ))-> _ptr_rdo )error ( (char *)"*const in cast", (struct ea *)ea0 , (struct ea *)ea0 ,
#line 1941 "../../src/expr2.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
case 110 : 
#line 1943 "../../src/expr2.c"
_au1_tt = (((struct ptr *)_au1_tt ))-> _pvtyp_typ ;
goto zaq ;
case 108 : 
#line 1948 "../../src/expr2.c"
break ;
default : 
#line 1950 "../../src/expr2.c"
if ((((struct basetype *)_au1_tt ))-> _basetype_b_const )error ( (char *)"const in cast", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ,
#line 1950 "../../src/expr2.c"
(struct ea *)ea0 ) ;
}

#line 1956 "../../src/expr2.c"
_au1_tt = _au1_t ;

#line 1958 "../../src/expr2.c"
if (_au0_this -> _expr__O3.__C3_e1 == dummy ){ 
#line 1959 "../../src/expr2.c"
error ( (char *)"E missing for cast", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 1959 "../../src/expr2.c"
ea *)ea0 ) ;
_au0_this -> _expr__O2.__C2_tp = (struct type *)any_type ;
return _au0_this ;
}

#line 1964 "../../src/expr2.c"
{ int _au1_pmf ;
Pexpr _au1_ee ;

#line 1964 "../../src/expr2.c"
_au1_pmf = 0 ;
_au1_ee = _au0_this -> _expr__O3.__C3_e1 ;
switch (_au1_ee -> _node_base ){ 
#line 1967 "../../src/expr2.c"
case 112 : 
#line 1968 "../../src/expr2.c"
_au1_ee = _au1_ee -> _expr__O4.__C4_e2 ;
switch (_au1_ee -> _node_base ){ 
#line 1970 "../../src/expr2.c"
case 85 : goto nm ;
case 44 : goto rf ;
}
break ;

#line 1975 "../../src/expr2.c"
case 85 : 
#line 1976 "../../src/expr2.c"
nm :
#line 1977 "../../src/expr2.c"
if ((((struct name *)_au1_ee ))-> _name__O6.__C6_n_qualifier )_au1_pmf = 1 ;
break ;

#line 1980 "../../src/expr2.c"
case 44 : 
#line 1981 "../../src/expr2.c"
rf :
#line 1982 "../../src/expr2.c"
if (_au1_ee -> _expr__O3.__C3_e1 -> _node_base == 34 )bound = 1 ;
break ;
}
_au0_this -> _expr__O3.__C3_e1 = _expr_typ ( _au0_this -> _expr__O3.__C3_e1 , _au0_tbl ) ;

#line 1987 "../../src/expr2.c"
{ int _au1_b ;

#line 1987 "../../src/expr2.c"
_au1_b = bound ;
bound = 0 ;
_au1_pmf = (_au1_pmf && (_au0_this -> _expr__O3.__C3_e1 -> _node_base == 113 ));

#line 1991 "../../src/expr2.c"
{ Ptype _au1_etp ;

#line 1991 "../../src/expr2.c"
_au1_etp = _au0_this -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp ;
while (_au1_etp -> _node_base == 97 )_au1_etp = (((struct basetype *)_au1_etp ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 1994 "../../src/expr2.c"
switch (_au1_etp -> _node_base ){ 
#line 1995 "../../src/expr2.c"
case 119 : 
#line 1996 "../../src/expr2.c"
{ Pexpr _au3_x ;

#line 1996 "../../src/expr2.c"
_au3_x = try_to_coerce ( _au1_tt , _au0_this -> _expr__O3.__C3_e1 , "cast", _au0_tbl ) ;
if (_au3_x )return _au3_x ;

#line 2014 "../../src/expr2.c"
break ;
}
case 38 : 
#line 2017 "../../src/expr2.c"
if (_au1_tt -> _node_base == 38 ){ 
#line 2018 "../../src/expr2.c"
_au0_this -> _expr__O2.__C2_tp = _au1_t ;
return _au0_this ;
}
error ( (char *)"cast of void value", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
case 141 : 
#line 2023 "../../src/expr2.c"
_au0_this -> _expr__O2.__C2_tp = (struct type *)any_type ;
return _au0_this ;
}

#line 2027 "../../src/expr2.c"
legloop :
#line 2029 "../../src/expr2.c"
switch (_au1_tt -> _node_base ){ 
#line 2030 "../../src/expr2.c"
case 97 : 
#line 2031 "../../src/expr2.c"
_au1_tt = (((struct basetype *)_au1_tt ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 2031 "../../src/expr2.c"
goto legloop ;
case 125 : 
#line 2033 "../../src/expr2.c"
switch (_au1_etp -> _node_base ){ 
#line 2034 "../../src/expr2.c"
case 119 : 
#line 2035 "../../src/expr2.c"
error ( (char *)"cannot castCO toP", (struct ea *)ea0 , (struct ea *)ea0 ,
#line 2035 "../../src/expr2.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
break ;
case 108 : 
#line 2038 "../../src/expr2.c"
_au0_this -> _expr__O3.__C3_e1 = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )145 , (struct expr *)0 , _au0_this ->
#line 2038 "../../src/expr2.c"
_expr__O3.__C3_e1 ) ;
bound = _au1_b ;
_au0_this -> _expr__O3.__C3_e1 = _expr_typ ( _au0_this -> _expr__O3.__C3_e1 , _au0_tbl ) ;
bound = 0 ;
if (_au0_this -> _expr__O3.__C3_e1 -> _node_base == 113 )
#line 2043 "../../src/expr2.c"
_au1_pmf = 1 ;
else 
#line 2045 "../../src/expr2.c"
break ;
case 125 : 
#line 2047 "../../src/expr2.c"
if (_au1_pmf ){ 
#line 2048 "../../src/expr2.c"
zaqq :
#line 2049 "../../src/expr2.c"
switch (_au1_tt -> _node_base ){ 
#line 2050 "../../src/expr2.c"
case 97 : 
#line 2051 "../../src/expr2.c"
_au1_tt = (((struct basetype *)_au1_tt ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
#line 2051 "../../src/expr2.c"
goto zaqq ;
case 125 : 
#line 2053 "../../src/expr2.c"
if ((((struct ptr *)_au1_tt ))-> _ptr_memof )break ;
default : 
#line 2055 "../../src/expr2.c"
{ 
#line 2098 "../../src/expr2.c"
struct ea _au0__V121 ;

#line 2098 "../../src/expr2.c"
struct ea _au0__V122 ;

#line 2098 "../../src/expr2.c"
struct ea _au0__V123 ;

#line 2055 "../../src/expr2.c"
error ( (char *)"%t cast to%t (%t is not aP toM)", (struct ea *)( ( ((& _au0__V121 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp )), (((&
#line 2055 "../../src/expr2.c"
_au0__V121 )))) ) , (struct ea *)( ( ((& _au0__V122 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _expr__O5.__C5_tp2 )), (((& _au0__V122 ))))
#line 2055 "../../src/expr2.c"
) , (struct ea *)( ( ((& _au0__V123 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _expr__O5.__C5_tp2 )), (((& _au0__V123 )))) )
#line 2055 "../../src/expr2.c"
, (struct ea *)ea0 ) ;
} }
}
}
break ;

#line 2061 "../../src/expr2.c"
case 158 : 
#line 2063 "../../src/expr2.c"
if (((((_au0_this -> _expr__O3.__C3_e1 -> _node_base == 147 )|| (_au0_this -> _expr__O3.__C3_e1 -> _node_base == 109 ))|| (_au0_this -> _expr__O3.__C3_e1 -> _node_base ==
#line 2063 "../../src/expr2.c"
146 ))|| _expr_lval ( _au0_this -> _expr__O3.__C3_e1 , (unsigned char )0 ) )&& (_type_tsizeof ( (((struct ptr *)_au1_tt ))-> _pvtyp_typ ) <= _type_tsizeof (
#line 2063 "../../src/expr2.c"
_au1_etp ) ))
#line 2067 "../../src/expr2.c"
{ 
#line 2068 "../../src/expr2.c"
_au0_this -> _expr__O3.__C3_e1 = _expr_address ( _au0_this -> _expr__O3.__C3_e1 ) ;
_au0_this -> _expr__O2.__C2_tp = _au1_t ;
return _expr_contents ( _au0_this ) ;
}
else 
#line 2073 "../../src/expr2.c"
{ 
#line 2098 "../../src/expr2.c"
struct ea _au0__V124 ;

#line 2098 "../../src/expr2.c"
struct ea _au0__V125 ;

#line 2073 "../../src/expr2.c"
error ( (char *)"cannot cast%t to%t", (struct ea *)( ( ((& _au0__V124 )-> _ea__O1.__C1_p = ((char *)_au1_etp )), (((& _au0__V124 )))) )
#line 2073 "../../src/expr2.c"
, (struct ea *)( ( ((& _au0__V125 )-> _ea__O1.__C1_p = ((char *)_au1_t )), (((& _au0__V125 )))) ) , (struct
#line 2073 "../../src/expr2.c"
ea *)ea0 , (struct ea *)ea0 ) ;
} break ;

#line 2076 "../../src/expr2.c"
case 119 : 
#line 2078 "../../src/expr2.c"
_au0_this -> _node_base = 157 ;
_au0_this -> _expr__O3.__C3_e1 = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , _au0_this -> _expr__O3.__C3_e1 , (struct expr *)0 ) ;
#line 2079 "../../src/expr2.c"

#line 2080 "../../src/expr2.c"
return _expr_typ ( _au0_this , _au0_tbl ) ;

#line 2082 "../../src/expr2.c"
case 5 : 
#line 2083 "../../src/expr2.c"
case 21 : 
#line 2084 "../../src/expr2.c"
case 29 : 
#line 2085 "../../src/expr2.c"
case 22 : 
#line 2086 "../../src/expr2.c"
case 15 : 
#line 2087 "../../src/expr2.c"
case 11 : 
#line 2088 "../../src/expr2.c"
switch (_au1_etp ->
#line 2088 "../../src/expr2.c"
_node_base ){ 
#line 2089 "../../src/expr2.c"
case 119 : 
#line 2090 "../../src/expr2.c"
{ 
#line 2098 "../../src/expr2.c"
struct ea _au0__V126 ;

#line 2090 "../../src/expr2.c"
error ( (char *)"cannot castCO to%k", (struct ea *)( ( ((& _au0__V126 )-> _ea__O1.__C1_i = ((int )_au1_tt -> _node_base )), (((& _au0__V126 ))))
#line 2090 "../../src/expr2.c"
) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
break ;
} }
break ;
}

#line 2096 "../../src/expr2.c"
_au0_this -> _expr__O2.__C2_tp = _au1_t ;
return _au0_this ;
}
}
}
}
;

#line 2100 "../../src/expr2.c"
Pexpr _expr_dovalue (_au0_this , _au0_tbl )
#line 525 "../../src/cfront.h"
struct expr *_au0_this ;

#line 2100 "../../src/expr2.c"
Ptable _au0_tbl ;
{ 
#line 2102 "../../src/expr2.c"
Ptype _au1_tt ;
Pclass _au1_cl ;
Pname _au1_cn ;

#line 2102 "../../src/expr2.c"
_au1_tt = _au0_this -> _expr__O5.__C5_tp2 ;

#line 2107 "../../src/expr2.c"
_type_dcl ( _au1_tt , _au0_tbl ) ;
vv :
#line 2109 "../../src/expr2.c"
switch (_au1_tt -> _node_base ){ 
#line 2110 "../../src/expr2.c"
case 97 : 
#line 2111 "../../src/expr2.c"
_au1_tt = (((struct basetype *)_au1_tt ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
goto vv ;

#line 2114 "../../src/expr2.c"
case 121 : 
#line 2115 "../../src/expr2.c"
default : 
#line 2116 "../../src/expr2.c"
if (_au0_this -> _expr__O3.__C3_e1 == 0 ){ 
#line 2117 "../../src/expr2.c"
{ 
#line 2190 "../../src/expr2.c"
struct ea _au0__V127 ;

#line 2117 "../../src/expr2.c"
error ( (char *)"value missing in conversion to%t", (struct ea *)( ( ((& _au0__V127 )-> _ea__O1.__C1_p = ((char *)_au1_tt )), (((& _au0__V127 )))) )
#line 2117 "../../src/expr2.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
_au0_this -> _expr__O2.__C2_tp = (struct type *)any_type ;
return _au0_this ;
} }
_au0_this -> _node_base = 113 ;
_au0_this -> _expr__O3.__C3_e1 = _au0_this -> _expr__O3.__C3_e1 -> _expr__O3.__C3_e1 ;
return _expr_typ ( _au0_this , _au0_tbl ) ;

#line 2125 "../../src/expr2.c"
case 6 : 
#line 2126 "../../src/expr2.c"
_au1_cl = (((struct classdef *)_au1_tt ));
break ;

#line 2129 "../../src/expr2.c"
case 119 : 
#line 2130 "../../src/expr2.c"
_au1_cn = (((struct basetype *)_au1_tt ))-> _basetype_b_name ;
_au1_cl = (((struct classdef *)_au1_cn -> _expr__O2.__C2_tp ));
}

#line 2134 "../../src/expr2.c"
if (_au0_this -> _expr__O3.__C3_e1 && (_au0_this -> _expr__O3.__C3_e1 -> _expr__O4.__C4_e2 == 0 )){ 
#line 2135 "../../src/expr2.c"
_au0_this -> _expr__O3.__C3_e1 -> _expr__O3.__C3_e1 = _expr_typ ( _au0_this -> _expr__O3.__C3_e1 -> _expr__O3.__C3_e1 ,
#line 2135 "../../src/expr2.c"
_au0_tbl ) ;
if (_au1_tt -> _node_base == 119 ){ 
#line 2137 "../../src/expr2.c"
Pexpr _au3_x ;

#line 2137 "../../src/expr2.c"
_au3_x = try_to_coerce ( _au1_tt , _au0_this -> _expr__O3.__C3_e1 -> _expr__O3.__C3_e1 , "type conversion", _au0_tbl ) ;
if (_au3_x )return _au3_x ;
}
{ Pname _au2_acn ;

#line 2140 "../../src/expr2.c"
_au2_acn = _type_is_cl_obj ( _au0_this -> _expr__O3.__C3_e1 -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp ) ;

#line 2142 "../../src/expr2.c"
if ((_au2_acn && (_au2_acn -> _expr__O2.__C2_tp == (struct type *)_au1_cl ))&& (( _au1_cl -> _classdef_itor ) == 0 )){ 
#line 2143 "../../src/expr2.c"
if (_au0_this -> _expr__O4.__C4_e2 ){
#line 2143 "../../src/expr2.c"

#line 2144 "../../src/expr2.c"
_au0_this -> _node_base = 70 ;
{ Pexpr _au4_ee ;

#line 2145 "../../src/expr2.c"
_au4_ee = _au0_this -> _expr__O3.__C3_e1 -> _expr__O3.__C3_e1 ;
_au0_this -> _expr__O3.__C3_e1 = _au0_this -> _expr__O4.__C4_e2 ;
_au0_this -> _expr__O4.__C4_e2 = _au4_ee ;
_au0_this -> _expr__O2.__C2_tp = _au0_this -> _expr__O5.__C5_tp2 ;
return _au0_this ;
}
}

#line 2151 "../../src/expr2.c"
return _au0_this -> _expr__O3.__C3_e1 -> _expr__O3.__C3_e1 ;
}
}
}

#line 2156 "../../src/expr2.c"
{ Pname _au1_ctor ;

#line 2156 "../../src/expr2.c"
_au1_ctor = ( _table_look ( _au1_cl -> _classdef_memtbl , "_ctor", (unsigned char )0 ) ) ;

#line 2158 "../../src/expr2.c"
if (_au1_ctor == 0 ){ 
#line 2159 "../../src/expr2.c"
{ 
#line 2190 "../../src/expr2.c"
struct ea _au0__V128 ;

#line 2159 "../../src/expr2.c"
error ( (char *)"cannot make a%n", (struct ea *)( ( ((& _au0__V128 )-> _ea__O1.__C1_p = ((char *)_au1_cn )), (((& _au0__V128 )))) )
#line 2159 "../../src/expr2.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
_au0_this -> _node_base = 72 ;
_au0_this -> _expr__O3.__C3_e1 = dummy ;
_au0_this -> _expr__O4.__C4_e2 = 0 ;
return _au0_this ;
} }

#line 2166 "../../src/expr2.c"
{ Pexpr _au1_ee ;
int _au1_tv ;

#line 2167 "../../src/expr2.c"
_au1_tv = 0 ;
if (_au0_this -> _expr__O4.__C4_e2 == 0 ){ 
#line 2169 "../../src/expr2.c"
Pname _au2_n ;

#line 2169 "../../src/expr2.c"
_au2_n = make_tmp ( 'V' , _au0_this -> _expr__O5.__C5_tp2 , _au0_tbl ) ;
_name_assign ( _au2_n ) ;
if (_au0_tbl == gtbl )_name_dcl_print ( _au2_n , (unsigned char )0 ) ;
_au0_this -> _expr__O4.__C4_e2 = (struct expr *)_au2_n ;
_au1_ee = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )147 , _au0_this , (struct expr *)_au2_n ) ;
_au1_tv = 1 ;
}
else 
#line 2177 "../../src/expr2.c"
_au1_ee = _au0_this ;

#line 2179 "../../src/expr2.c"
{ Pexpr _au1_a ;

#line 2180 "../../src/expr2.c"
struct ref *_au0__Xthis__ctor_ref ;

#line 2179 "../../src/expr2.c"
_au1_a = _au0_this -> _expr__O3.__C3_e1 ;
_au0_this -> _node_base = 146 ;
_au0_this -> _expr__O3.__C3_e1 = (struct expr *)( (_au0__Xthis__ctor_ref = 0 ), ( ( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct
#line 2181 "../../src/expr2.c"
expr *)_au0__Xthis__ctor_ref ), ((unsigned char )45 ), _au0_this -> _expr__O4.__C4_e2 , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = _au1_ctor ),
#line 2181 "../../src/expr2.c"
((_au0__Xthis__ctor_ref ))) ) ) ;
_au0_this -> _expr__O4.__C4_e2 = _au1_a ;
_au1_ee = _expr_typ ( _au1_ee , _au0_tbl ) ;

#line 2185 "../../src/expr2.c"
if (_au1_tv == 0 ){ 
#line 2186 "../../src/expr2.c"
_au1_ee = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )111 , _au1_ee , (struct expr *)0 )
#line 2186 "../../src/expr2.c"
;
_au1_ee -> _expr__O2.__C2_tp = _au1_ee -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp ;
}
return _au1_ee ;
}
}
}
}
;

#line 2192 "../../src/expr2.c"
Pexpr _expr_donew (_au0_this , _au0_tbl )
#line 525 "../../src/cfront.h"
struct expr *_au0_this ;

#line 2192 "../../src/expr2.c"
Ptable _au0_tbl ;
{ 
#line 2194 "../../src/expr2.c"
Ptype _au1_tt ;
Ptype _au1_tx ;
bit _au1_v ;
bit _au1_old ;

#line 2198 "../../src/expr2.c"
struct ptr *_au0__Xthis__ctor_ptr ;

#line 2194 "../../src/expr2.c"
_au1_tt = _au0_this -> _expr__O5.__C5_tp2 ;
_au1_tx = _au1_tt ;
_au1_v = 0 ;
_au1_old = new_type ;
new_type = 1 ;
_type_dcl ( _au1_tt , _au0_tbl ) ;
new_type = _au1_old ;
if (_au0_this -> _expr__O3.__C3_e1 )_au0_this -> _expr__O3.__C3_e1 = _expr_typ ( _au0_this -> _expr__O3.__C3_e1 , _au0_tbl ) ;
ll :
#line 2203 "../../src/expr2.c"
switch (_au1_tt -> _node_base ){ 
#line 2204 "../../src/expr2.c"
default : 
#line 2205 "../../src/expr2.c"
if (_au0_this -> _expr__O3.__C3_e1 ){ 
#line 2206 "../../src/expr2.c"
error ( (char *)"Ir for nonCO created using \"new\"", (struct ea *)ea0 , (struct
#line 2206 "../../src/expr2.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
_au0_this -> _expr__O3.__C3_e1 = 0 ;
}
break ;
case 110 : 
#line 2211 "../../src/expr2.c"
_au1_v = 1 ;
_au1_tt = (((struct vec *)_au1_tt ))-> _pvtyp_typ ;
goto ll ;
case 97 : 
#line 2215 "../../src/expr2.c"
_au1_tt = (((struct basetype *)_au1_tt ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
goto ll ;
case 119 : 
#line 2218 "../../src/expr2.c"
{ Pname _au3_cn ;
Pclass _au3_cl ;

#line 2218 "../../src/expr2.c"
_au3_cn = (((struct basetype *)_au1_tt ))-> _basetype_b_name ;
_au3_cl = (((struct classdef *)_au3_cn -> _expr__O2.__C2_tp ));

#line 2221 "../../src/expr2.c"
if ((_au3_cl -> _type_defined & 3)== 0 ){ 
#line 2222 "../../src/expr2.c"
{ 
#line 2256 "../../src/expr2.c"
struct ea _au0__V129 ;

#line 2256 "../../src/expr2.c"
struct ea _au0__V130 ;

#line 2222 "../../src/expr2.c"
error ( (char *)"new%n;%n isU", (struct ea *)( ( ((& _au0__V129 )-> _ea__O1.__C1_p = ((char *)_au3_cn )), (((& _au0__V129 )))) )
#line 2222 "../../src/expr2.c"
, (struct ea *)( ( ((& _au0__V130 )-> _ea__O1.__C1_p = ((char *)_au3_cn )), (((& _au0__V130 )))) ) , (struct
#line 2222 "../../src/expr2.c"
ea *)ea0 , (struct ea *)ea0 ) ;
} }
else { 
#line 2225 "../../src/expr2.c"
Pname _au4_ctor ;
TOK _au4_su ;

#line 2227 "../../src/expr2.c"
struct call *_au0__Xthis__ctor_call ;

#line 2225 "../../src/expr2.c"
_au4_ctor = ( _table_look ( _au3_cl -> _classdef_memtbl , "_ctor", (unsigned char )0 ) ) ;

#line 2227 "../../src/expr2.c"
if (_au4_ctor ){ 
#line 2228 "../../src/expr2.c"
if (_au1_v ){ 
#line 2229 "../../src/expr2.c"
Pname _au6_ic ;
if (_au0_this -> _expr__O3.__C3_e1 )
#line 2231 "../../src/expr2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"Ir for vector ofCO created using \"new\"", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 2231 "../../src/expr2.c"
ea *)ea0 ) ;
else if ((_au6_ic = _classdef_has_ictor ( _au3_cl ) )== 0 )
#line 2233 "../../src/expr2.c"
{ 
#line 2256 "../../src/expr2.c"
struct ea _au0__V131 ;

#line 2233 "../../src/expr2.c"
error ( (char *)"vector ofC%n that does not have aK taking noAs", (struct ea *)( ( ((& _au0__V131 )-> _ea__O1.__C1_p = ((char *)_au3_cn )), (((& _au0__V131 )))) )
#line 2233 "../../src/expr2.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} else if ((((struct fct *)_au6_ic -> _expr__O2.__C2_tp ))-> _fct_nargs )
#line 2235 "../../src/expr2.c"
{ 
#line 2256 "../../src/expr2.c"
struct ea _au0__V132 ;

#line 2235 "../../src/expr2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"defaultAs forK for vector ofC%n", (struct ea *)( ( ((& _au0__V132 )-> _ea__O1.__C1_p = ((char *)_au3_cn )), (((&
#line 2235 "../../src/expr2.c"
_au0__V132 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }

#line 2238 "../../src/expr2.c"
if (cc -> _dcl_context_cot != _au3_cl )
#line 2239 "../../src/expr2.c"
visible_check ( _au4_ctor , _au4_ctor -> _expr__O3.__C3_string , _au0_this , (char )0 ) ;
_au0_this -> _expr__O3.__C3_e1 = (struct expr *)( (_au0__Xthis__ctor_call = 0 ), ( ( (_au0__Xthis__ctor_call = 0 ), (_au0__Xthis__ctor_call = (struct call *)_expr__ctor ( ((struct
#line 2240 "../../src/expr2.c"
expr *)_au0__Xthis__ctor_call ), (unsigned char )109 , ((struct expr *)_au4_ctor ), _au0_this -> _expr__O3.__C3_e1 ) )) , ((_au0__Xthis__ctor_call ))) ) ;
_au0_this -> _expr__O3.__C3_e1 = _expr_typ ( _au0_this -> _expr__O3.__C3_e1 , _au0_tbl ) ;
}
else 
#line 2244 "../../src/expr2.c"
if (_au4_su = ( (((unsigned char )((_au3_cl -> _classdef_csu == 6 )? (((unsigned int )0 )): (((unsigned int
#line 2244 "../../src/expr2.c"
)_au3_cl -> _classdef_csu )))))) ){ 
#line 2245 "../../src/expr2.c"
if (_au0_this -> _expr__O3.__C3_e1 ){ 
#line 2256 "../../src/expr2.c"
struct ea _au0__V133 ;

#line 2245 "../../src/expr2.c"
error ( (char *)"new%nWIr", (struct ea *)( ( ((& _au0__V133 )-> _ea__O1.__C1_p = ((char *)_au3_cn )), (((& _au0__V133 )))) )
#line 2245 "../../src/expr2.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
else { }
}
}
}

#line 2254 "../../src/expr2.c"
_au0_this -> _expr__O2.__C2_tp = (_au1_v ? (((struct type *)_au1_tx )): (((struct type *)( (_au0__Xthis__ctor_ptr = 0 ), ( (_au0__Xthis__ctor_ptr =
#line 2254 "../../src/expr2.c"
(struct ptr *)_new ( (long )(sizeof (struct ptr))) ), ( (Nt ++ ), ( (_au0__Xthis__ctor_ptr -> _node_base = ((unsigned char
#line 2254 "../../src/expr2.c"
)125 )), ( (_au0__Xthis__ctor_ptr -> _pvtyp_typ = _au1_tx ), ( (_au0__Xthis__ctor_ptr -> _ptr_rdo = ((char )0 )), ((_au0__Xthis__ctor_ptr ))) ) ) )
#line 2254 "../../src/expr2.c"
) ) )));
return _au0_this ;
}
;

/* the end */
