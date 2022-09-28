/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#line 1 "../../src/expand.c"

/* <<cfront 1.2.1 2/16/87>> */
/* < ../../src/expand.c */

#ident	"@(#)sdb:cfront/scratch/src/expand..c	1.2"
char *_new(); char _delete(); char *_vec_new(); char _vec_delete();

#line 1 "../../src/expand.c"

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

#line 20 "../../src/expand.c"
extern char *temp (_au0_vn , _au0_fn , _au0_cn )char *_au0_vn ;

#line 20 "../../src/expand.c"
char *_au0_fn ;

#line 20 "../../src/expand.c"
char *_au0_cn ;

#line 24 "../../src/expand.c"
{ if (((_au0_vn [0 ])!= '_' )|| ((_au0_vn [1 ])!= 'X' )){ 
#line 25 "../../src/expand.c"
int _au2_vnl ;
int _au2_fnl ;
int _au2_cnl ;
char *_au2_s ;

#line 25 "../../src/expand.c"
_au2_vnl = strlen ( (char *)_au0_vn ) ;
_au2_fnl = strlen ( (char *)_au0_fn ) ;
_au2_cnl = (_au0_cn ? strlen ( (char *)_au0_cn ) : 0);
_au2_s = (((char *)_new ( (long )((sizeof (char ))* (((_au2_vnl + _au2_fnl )+ _au2_cnl )+ 5 ))) ));

#line 30 "../../src/expand.c"
(_au2_s [0 ])= '_' ;
(_au2_s [1 ])= 'X' ;
strcpy ( _au2_s + 2 , (char *)_au0_vn ) ;
(_au2_s [_au2_vnl + 2 ])= '_' ;
strcpy ( (_au2_s + _au2_vnl )+ 3 , (char *)_au0_fn ) ;
if (_au2_cnl ){ 
#line 36 "../../src/expand.c"
(_au2_s [(_au2_vnl + _au2_fnl )+ 3 ])= '_' ;
strcpy ( ((_au2_s + _au2_vnl )+ _au2_fnl )+ 4 , (char *)_au0_cn ) ;
}
return _au2_s ;
}
else 
#line 42 "../../src/expand.c"
return _au0_vn ;
}
;

#line 46 "../../src/expand.c"
Pname dcl_local (_au0_scope , _au0_an , _au0_fn )Ptable _au0_scope ;

#line 46 "../../src/expand.c"
Pname _au0_an ;

#line 46 "../../src/expand.c"
Pname _au0_fn ;
{ 
#line 56 "../../src/expand.c"
Pname _au1_cn ;
char *_au1_s ;

#line 64 "../../src/expand.c"
Pname _au1_nx ;

#line 72 "../../src/expand.c"
Pname _au1_r ;

#line 48 "../../src/expand.c"
if (_au0_scope == 0 ){ 
#line 49 "../../src/expand.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"cannot expand inlineF needing temporaryV in nonF context", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ,
#line 49 "../../src/expand.c"
(struct ea *)ea0 ) ;
return _au0_an ;
}
if (_au0_an -> _name_n_stclass == 31 ){ 
#line 53 "../../src/expand.c"
{ 
#line 77 "../../src/expand.c"
struct ea _au0__V10 ;

#line 53 "../../src/expand.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"static%n in inlineF", (struct ea *)( ( ((& _au0__V10 )-> _ea__O1.__C1_p = ((char *)_au0_an )), (((&
#line 53 "../../src/expand.c"
_au0__V10 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
return _au0_an ;
} }
_au1_cn = _au0_fn -> _expr__O5.__C5_n_table -> _table_t_name ;

#line 56 "../../src/expand.c"
_au1_s = temp ( _au0_an -> _expr__O3.__C3_string , _au0_fn -> _expr__O3.__C3_string , _au1_cn ? _au1_cn -> _expr__O3.__C3_string : (((char *)0 ))) ;

#line 56 "../../src/expand.c"
_au1_nx = (struct name *)_name__ctor ( (struct name *)0 , _au1_s ) ;

#line 65 "../../src/expand.c"
_au1_nx -> _expr__O2.__C2_tp = _au0_an -> _expr__O2.__C2_tp ;
_au1_nx -> _expr__O2.__C2_tp -> _node_permanent = 1 ;
_au1_nx -> _name_n_used = _au0_an -> _name_n_used ;
_au1_nx -> _name_n_assigned_to = _au0_an -> _name_n_assigned_to ;
_au1_nx -> _name_n_addr_taken = _au0_an -> _name_n_addr_taken ;
_au1_nx -> _name_n_xref = _au0_an -> _name_n_xref ;
_au1_nx -> _name_lex_level = _au0_an -> _name_lex_level ;
_au1_r = _table_insert ( _au0_scope , _au1_nx , (unsigned char )0 ) ;
_au1_r -> _name_n_stclass = _au0_an -> _name_n_stclass ;
_name__dtor ( _au1_nx , 1) ;
_au1_r -> _name_where . _loc_line = 0 ;
return _au1_r ;
}
;
int needs_zero (_au0_e )Pexpr _au0_e ;

#line 85 "../../src/expand.c"
{ 
#line 86 "../../src/expand.c"
int _au1_val ;

#line 86 "../../src/expand.c"
int _au1_tmp ;
Pexpr _au1_ee ;

#line 86 "../../src/expand.c"
_au1_val = 0 ;

#line 86 "../../src/expand.c"
_au1_tmp = 1 ;
_au1_ee = _au0_e -> _expr__O3.__C3_e1 ;

#line 89 "../../src/expand.c"
xxx :
#line 90 "../../src/expand.c"
if (_au1_ee -> _expr__O2.__C2_tp == 0 )return _au1_val ;
switch (_au1_ee -> _expr__O2.__C2_tp -> _node_base ){ 
#line 92 "../../src/expand.c"
case 97 : 
#line 93 "../../src/expand.c"
case 123 : 
#line 94 "../../src/expand.c"
{ Pbase _au2_b ;

#line 94 "../../src/expand.c"
_au2_b = (((struct basetype *)_au1_ee -> _expr__O2.__C2_tp ));
if (_au2_b -> _basetype_b_name -> _expr__O2.__C2_tp -> _node_base == 119 )_au1_val += _au1_tmp ;
break ;
case 119 : 
#line 98 "../../src/expand.c"
_au1_val += _au1_tmp ;

#line 98 "../../src/expand.c"
break ;
}
}
if (_au1_tmp == 1 ){ 
#line 102 "../../src/expand.c"
_au1_tmp ++ ;

#line 102 "../../src/expand.c"
if (_au1_ee = _au0_e -> _expr__O4.__C4_e2 )goto xxx ;
}

#line 105 "../../src/expand.c"
return _au1_val ;
}
;
int ck_cast (_au0_t1 , _au0_t2 )Ptype _au0_t1 ;

#line 108 "../../src/expand.c"
Ptype _au0_t2 ;

#line 113 "../../src/expand.c"
{ 
#line 115 "../../src/expand.c"
while (_au0_t1 -> _node_base == 97 )_au0_t1 = (((struct basetype *)_au0_t1 ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
while (_au0_t2 -> _node_base == 97 )_au0_t2 = (((struct basetype *)_au0_t2 ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 118 "../../src/expand.c"
if (_au0_t1 == _au0_t2 )return (int )0 ;

#line 120 "../../src/expand.c"
if (_au0_t1 -> _node_base != _au0_t2 -> _node_base )return 1 ;

#line 122 "../../src/expand.c"
switch (_au0_t1 -> _node_base ){ 
#line 123 "../../src/expand.c"
case 5 : 
#line 124 "../../src/expand.c"
case 29 : 
#line 125 "../../src/expand.c"
case 21 : 
#line 126 "../../src/expand.c"
case 22 : 
#line 127 "../../src/expand.c"
if ((((struct basetype *)_au0_t1 ))->
#line 127 "../../src/expand.c"
_basetype_b_unsigned != (((struct basetype *)_au0_t2 ))-> _basetype_b_unsigned )return 1 ;
}

#line 130 "../../src/expand.c"
if (_au0_t1 -> _node_base == 119 ){ 
#line 131 "../../src/expand.c"
Pname _au2_nn ;

#line 131 "../../src/expand.c"
_au2_nn = (((struct basetype *)_au0_t1 ))-> _basetype_b_name ;

#line 133 "../../src/expand.c"
if ((((struct classdef *)_au2_nn -> _expr__O2.__C2_tp ))-> _classdef_csu == 36 )return (int )0 ;

#line 135 "../../src/expand.c"
if ((_au0_t2 -> _node_base == 119 )&& (_au2_nn -> _expr__O2.__C2_tp == (((struct basetype *)_au0_t2 ))-> _basetype_b_name -> _expr__O2.__C2_tp ))return (int )0 ;

#line 137 "../../src/expand.c"
return 1 ;
}

#line 140 "../../src/expand.c"
return (int )0 ;
}
;
Pstmt _stmt_expand (_au0_this )
#line 651 "../../src/cfront.h"
struct stmt *_au0_this ;

#line 151 "../../src/expand.c"
{ 
#line 152 "../../src/expand.c"
if (_au0_this == 0 ){ 
#line 301 "../../src/expand.c"
struct ea _au0__V11 ;

#line 152 "../../src/expand.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"0->S::expand() for%n", (struct ea *)( ( ((& _au0__V11 )-> _ea__O1.__C1_p = ((char *)expand_fn )), (((&
#line 152 "../../src/expand.c"
_au0__V11 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} 
#line 155 "../../src/expand.c"
if (_au0_this -> _stmt_memtbl ){ 
#line 156 "../../src/expand.c"
register Ptable _au2_t ;
register int _au2_i ;
register Pname _au2_n ;

#line 156 "../../src/expand.c"
_au2_t = _au0_this -> _stmt_memtbl ;

#line 158 "../../src/expand.c"
for(_au2_n = _table_get_mem ( _au2_t , _au2_i = 1 ) ;_au2_n ;_au2_n = _table_get_mem ( _au2_t , ++ _au2_i ) ) { 
#line 159 "../../src/expand.c"
if (_au2_n ->
#line 159 "../../src/expand.c"
_name_n_stclass == 31 ){ 
#line 160 "../../src/expand.c"
{ 
#line 301 "../../src/expand.c"
struct ea _au0__V12 ;

#line 160 "../../src/expand.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"static%n in inlineF", (struct ea *)( ( ((& _au0__V12 )-> _ea__O1.__C1_p = ((char *)_au2_n )), (((&
#line 160 "../../src/expand.c"
_au0__V12 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
_au2_n -> _name_n_stclass = 2 ;
} }
_au2_n -> _name_where . _loc_line = 0 ;
}
}

#line 167 "../../src/expand.c"
if (expand_tbl ){ 
#line 168 "../../src/expand.c"
Pexpr _au2_ee ;
static int _static_ret_seen = 0 ;

#line 171 "../../src/expand.c"
if (_au0_this -> _stmt_memtbl ){ 
#line 172 "../../src/expand.c"
int _au3_i ;
Pname _au3_n ;
Ptable _au3_tbl ;

#line 174 "../../src/expand.c"
_au3_tbl = _au0_this -> _stmt_memtbl ;
for(_au3_n = _table_get_mem ( _au3_tbl , _au3_i = 1 ) ;_au3_n ;_au3_n = _table_get_mem ( _au3_tbl , ++ _au3_i ) ) { 
#line 176 "../../src/expand.c"
if ((_au3_n ->
#line 176 "../../src/expand.c"
_node_base != 85 )|| (_au3_n -> _expr__O2.__C2_tp == (struct type *)any_type ))continue ;

#line 178 "../../src/expand.c"
if (((_au0_this -> _node_base == 116 )&& (_au3_tbl -> _table_real_block == _au0_this ))&& (_au3_n -> _name_lex_level < 2 ))
#line 180 "../../src/expand.c"
continue ;

#line 182 "../../src/expand.c"
{ char *_au4_s ;

#line 185 "../../src/expand.c"
Pname _au4_nn ;

#line 182 "../../src/expand.c"
_au4_s = _au3_n -> _expr__O3.__C3_string ;
if (((_au4_s [0 ])== '_' )&& ((_au4_s [1 ])== 'X' ))continue ;

#line 185 "../../src/expand.c"
_au4_nn = dcl_local ( scope , _au3_n , expand_fn ) ;
_au4_nn -> _node_base = 85 ;
_au3_n -> _expr__O3.__C3_string = _au4_nn -> _expr__O3.__C3_string ;
}
}
}
switch (_au0_this -> _node_base ){ 
#line 192 "../../src/expand.c"
default : 
#line 193 "../../src/expand.c"
{ 
#line 301 "../../src/expand.c"
struct ea _au0__V13 ;

#line 301 "../../src/expand.c"
struct ea _au0__V14 ;

#line 193 "../../src/expand.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"%kS in inline%n", (struct ea *)( ( ((& _au0__V13 )-> _ea__O1.__C1_i = ((int )_au0_this -> _node_base )),
#line 193 "../../src/expand.c"
(((& _au0__V13 )))) ) , (struct ea *)( ( ((& _au0__V14 )-> _ea__O1.__C1_p = ((char *)expand_fn )), (((& _au0__V14 ))))
#line 193 "../../src/expand.c"
) , (struct ea *)ea0 , (struct ea *)ea0 ) ;
return (((struct stmt *)dummy ));

#line 196 "../../src/expand.c"
case 116 : 
#line 197 "../../src/expand.c"
if (_au0_this -> _stmt_s_list ){ 
#line 198 "../../src/expand.c"
_au2_ee = (((struct expr *)_stmt_expand ( _au0_this -> _stmt_s_list ) ));
if (_au0_this -> _stmt_s ){ 
#line 200 "../../src/expand.c"
_au2_ee = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )71 , ((struct expr *)_stmt_expand ( _au0_this ->
#line 200 "../../src/expand.c"
_stmt_s ) ), _au2_ee ) ;
_au2_ee -> _node_permanent = 1 ;
}
return (((struct stmt *)_au2_ee ));
}

#line 206 "../../src/expand.c"
if (_au0_this -> _stmt_s )return _stmt_expand ( _au0_this -> _stmt_s ) ;

#line 208 "../../src/expand.c"
return (((struct stmt *)zero ));

#line 210 "../../src/expand.c"
case 166 : 
#line 211 "../../src/expand.c"
_au2_ee = (_au0_this -> _stmt__O8.__C8_s2 ? (((struct expr *)_stmt_expand ( _au0_this -> _stmt__O8.__C8_s2 ) )): (((struct expr *)0 )));
_au2_ee = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )71 , _au0_this -> _stmt_s ? (((struct expr *)_stmt_expand ( _au0_this -> _stmt_s )
#line 212 "../../src/expand.c"
)): (((struct expr *)0 )), _au2_ee ) ;
if (_au0_this -> _stmt_s_list )_au2_ee = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )71 , _au2_ee , ((struct expr *)_stmt_expand ( _au0_this ->
#line 213 "../../src/expand.c"
_stmt_s_list ) )) ;
_au2_ee -> _node_permanent = 1 ;
return (((struct stmt *)_au2_ee ));

#line 217 "../../src/expand.c"
case 28 : 
#line 218 "../../src/expand.c"
_static_ret_seen = 1 ;
_au0_this -> _stmt_s_list = 0 ;

#line 221 "../../src/expand.c"
if (_au0_this -> _stmt__O8.__C8_e == 0 )
#line 222 "../../src/expand.c"
_au2_ee = zero ;
else { 
#line 225 "../../src/expand.c"
_au2_ee = _expr_expand ( _au0_this -> _stmt__O8.__C8_e ) ;
{ Ptype _au4_tt ;

#line 227 "../../src/expand.c"
struct texpr *_au0__Xthis__ctor_texpr ;

#line 226 "../../src/expand.c"
_au4_tt = (((struct fct *)expand_fn -> _expr__O2.__C2_tp ))-> _fct_returns ;
if (ck_cast ( _au4_tt , _au2_ee -> _expr__O2.__C2_tp ) )
#line 228 "../../src/expand.c"
_au2_ee = (struct expr *)( (_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ),
#line 228 "../../src/expand.c"
(_au0__Xthis__ctor_texpr = (struct texpr *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_texpr ), ((unsigned char )113 ), _au2_ee , (struct expr *)0 ) )) , (
#line 228 "../../src/expand.c"
(_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 = _au4_tt ), ((_au0__Xthis__ctor_texpr ))) ) ) ;
}
}
return (((struct stmt *)_au2_ee ));

#line 233 "../../src/expand.c"
case 72 : 
#line 234 "../../src/expand.c"
if ((_au0_this -> _stmt__O8.__C8_e == 0 )|| (_au0_this -> _stmt__O8.__C8_e == dummy ))
#line 235 "../../src/expand.c"
_au2_ee = zero ;
else { 
#line 237 "../../src/expand.c"
if (_au0_this -> _stmt__O8.__C8_e -> _node_base == 111 )_au0_this -> _stmt__O8.__C8_e = _au0_this -> _stmt__O8.__C8_e -> _expr__O3.__C3_e1 ;
_au2_ee = _expr_expand ( _au0_this -> _stmt__O8.__C8_e ) ;
}
if (_au0_this -> _stmt_s_list ){ 
#line 241 "../../src/expand.c"
_au2_ee = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )71 , _au2_ee , ((struct expr *)_stmt_expand (
#line 241 "../../src/expand.c"
_au0_this -> _stmt_s_list ) )) ;
_au2_ee -> _node_permanent = 1 ;
}
return (((struct stmt *)_au2_ee ));

#line 246 "../../src/expand.c"
case 1 : 
#line 247 "../../src/expand.c"
if (_au0_this -> _stmt_s_list ){ 
#line 248 "../../src/expand.c"
_au2_ee = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )71 , _au2_ee ,
#line 248 "../../src/expand.c"
((struct expr *)_stmt_expand ( _au0_this -> _stmt_s_list ) )) ;
_au2_ee -> _node_permanent = 1 ;
}
return (((struct stmt *)_au2_ee ));

#line 253 "../../src/expand.c"
case 20 : 
#line 254 "../../src/expand.c"
{ _static_ret_seen = 0 ;
{ Pexpr _au4_qq ;

#line 255 "../../src/expand.c"
_au4_qq = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )68 , ((struct expr *)_stmt_expand ( _au0_this -> _stmt_s ) ), (struct
#line 255 "../../src/expand.c"
expr *)0 ) ;
_au4_qq -> _expr__O5.__C5_cond = _expr_expand ( _au0_this -> _stmt__O8.__C8_e ) ;
_au4_qq -> _expr__O4.__C4_e2 = (_au0_this -> _stmt__O9.__C9_else_stmt ? (((struct expr *)_stmt_expand ( _au0_this -> _stmt__O9.__C9_else_stmt ) )): zero );

#line 259 "../../src/expand.c"
switch (needs_zero ( _au4_qq ) ){ 
#line 260 "../../src/expand.c"
case 1 : _au4_qq -> _expr__O3.__C3_e1 = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char
#line 260 "../../src/expand.c"
)71 , _au4_qq -> _expr__O3.__C3_e1 , zero ) ;

#line 260 "../../src/expand.c"
break ;
case 2 : _au4_qq -> _expr__O4.__C4_e2 = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )71 , _au4_qq -> _expr__O4.__C4_e2 , zero )
#line 261 "../../src/expand.c"
;

#line 261 "../../src/expand.c"
break ;
}

#line 264 "../../src/expand.c"
if (_static_ret_seen && _au0_this -> _stmt_s_list )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"S after \"return\" in inlineF", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ,
#line 264 "../../src/expand.c"
(struct ea *)ea0 ) ;
_static_ret_seen = 0 ;
if (_au0_this -> _stmt_s_list )_au4_qq = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )71 , _au4_qq , ((struct expr *)_stmt_expand ( _au0_this ->
#line 266 "../../src/expand.c"
_stmt_s_list ) )) ;
_au4_qq -> _node_permanent = 1 ;
return (((struct stmt *)_au4_qq ));
}
}
} }
}
_au0_this -> _stmt_where . _loc_line = 0 ;

#line 275 "../../src/expand.c"
switch (_au0_this -> _node_base ){ 
#line 276 "../../src/expand.c"
default : 
#line 277 "../../src/expand.c"
if (_au0_this -> _stmt__O8.__C8_e )_au0_this -> _stmt__O8.__C8_e = _expr_expand ( _au0_this -> _stmt__O8.__C8_e ) ;
break ;
case 166 : 
#line 280 "../../src/expand.c"
if (_au0_this -> _stmt__O8.__C8_s2 )_au0_this -> _stmt__O8.__C8_s2 = _stmt_expand ( _au0_this -> _stmt__O8.__C8_s2 ) ;
break ;
case 1 : 
#line 283 "../../src/expand.c"
case 116 : 
#line 284 "../../src/expand.c"
break ;
case 16 : 
#line 286 "../../src/expand.c"
if (_au0_this -> _stmt__O9.__C9_for_init )_au0_this -> _stmt__O9.__C9_for_init = _stmt_expand ( _au0_this -> _stmt__O9.__C9_for_init ) ;
if (_au0_this -> _stmt__O7.__C7_e2 )_au0_this -> _stmt__O7.__C7_e2 = _expr_expand ( _au0_this -> _stmt__O7.__C7_e2 ) ;
break ;
case 115 : 
#line 290 "../../src/expand.c"
case 19 : 
#line 291 "../../src/expand.c"
case 28 : 
#line 292 "../../src/expand.c"
case 3 : 
#line 293 "../../src/expand.c"
case 7 : 
#line 294 "../../src/expand.c"
{ 
#line 301 "../../src/expand.c"
struct ea _au0__V15 ;

#line 301 "../../src/expand.c"
struct ea _au0__V16 ;

#line 294 "../../src/expand.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"%kS in inline%n", (struct ea *)( ( ((& _au0__V15 )-> _ea__O1.__C1_i = ((int )_au0_this -> _node_base )),
#line 294 "../../src/expand.c"
(((& _au0__V15 )))) ) , (struct ea *)( ( ((& _au0__V16 )-> _ea__O1.__C1_p = ((char *)expand_fn )), (((& _au0__V16 ))))
#line 294 "../../src/expand.c"
) , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }

#line 297 "../../src/expand.c"
if (_au0_this -> _stmt_s )_au0_this -> _stmt_s = _stmt_expand ( _au0_this -> _stmt_s ) ;
if (_au0_this -> _stmt_s_list )_au0_this -> _stmt_s_list = _stmt_expand ( _au0_this -> _stmt_s_list ) ;
_au0_this -> _node_permanent = 1 ;
return _au0_this ;
}
;
Pexpr _expr_expand (_au0_this )
#line 525 "../../src/cfront.h"
struct expr *_au0_this ;

#line 304 "../../src/expand.c"
{ 
#line 305 "../../src/expand.c"
if (_au0_this == 0 )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"E::expand(0)", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ,
#line 305 "../../src/expand.c"
(struct ea *)ea0 ) ;

#line 307 "../../src/expand.c"
switch (_au0_this -> _node_base ){ 
#line 308 "../../src/expand.c"
case 85 : 
#line 310 "../../src/expand.c"
if (expand_tbl && ((((struct name *)_au0_this ))-> _name_n_scope == 108 )){ 
#line 311 "../../src/expand.c"
Pname _au3_n ;
char *_au3_s ;

#line 314 "../../src/expand.c"
Pname _au3_cn ;

#line 311 "../../src/expand.c"
_au3_n = (((struct name *)_au0_this ));
_au3_s = _au3_n -> _expr__O3.__C3_string ;
if (((_au3_s [0 ])== '_' )&& ((_au3_s [1 ])== 'X' ))break ;
_au3_cn = expand_fn -> _expr__O5.__C5_n_table -> _table_t_name ;
_au3_n -> _expr__O3.__C3_string = temp ( _au3_s , expand_fn -> _expr__O3.__C3_string , _au3_cn ? _au3_cn -> _expr__O3.__C3_string : (((char *)0 ))) ;
}
case 144 : 
#line 318 "../../src/expand.c"
case 82 : 
#line 319 "../../src/expand.c"
case 83 : 
#line 320 "../../src/expand.c"
case 84 : 
#line 321 "../../src/expand.c"
case 150 : 
#line 322 "../../src/expand.c"
case 151 : 
#line 323 "../../src/expand.c"
case 152 :
#line 323 "../../src/expand.c"

#line 324 "../../src/expand.c"
case 81 : 
#line 325 "../../src/expand.c"
case 86 : 
#line 326 "../../src/expand.c"
case 165 : 
#line 327 "../../src/expand.c"
case 169 : 
#line 328 "../../src/expand.c"
break ;
case 168 : 
#line 330 "../../src/expand.c"
if (expand_tbl && (_au0_this -> _expr__O3.__C3_e1 == 0 )){ 
#line 331 "../../src/expand.c"
Pname _au3_fn ;
Pfct _au3_f ;

#line 331 "../../src/expand.c"
_au3_fn = _au0_this -> _expr__O5.__C5_il -> _iline_fct_name ;
_au3_f = (((struct fct *)_au3_fn -> _expr__O2.__C2_tp ));
if (((_au3_f -> _fct_returns == (struct type *)void_type )&& (_au3_f -> _fct_s_returns != (struct type *)int_type ))&& (_au3_fn -> _name_n_oper != 161 ))
#line 336 "../../src/expand.c"
{ 
#line 358 "../../src/expand.c"
struct ea _au0__V17 ;
#line 358 "../../src/expand.c"
struct ea _au0__V18 ;

#line 336 "../../src/expand.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"non-value-returning inline%n called in value-returning inline%n", (struct ea *)( ( ((& _au0__V17 )-> _ea__O1.__C1_p = ((char *)_au3_fn )), (((&
#line 336 "../../src/expand.c"
_au0__V17 )))) ) , (struct ea *)( ( ((& _au0__V18 )-> _ea__O1.__C1_p = ((char *)expand_fn )), (((& _au0__V18 )))) )
#line 336 "../../src/expand.c"
, (struct ea *)ea0 , (struct ea *)ea0 ) ;
} else 
#line 338 "../../src/expand.c"
{ 
#line 358 "../../src/expand.c"
struct ea _au0__V19 ;

#line 338 "../../src/expand.c"
error ( (char *)"inline%n called before defined", (struct ea *)( ( ((& _au0__V19 )-> _ea__O1.__C1_p = ((char *)_au3_fn )), (((& _au0__V19 )))) )
#line 338 "../../src/expand.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
break ;
case 30 : 
#line 342 "../../src/expand.c"
case 113 : 
#line 343 "../../src/expand.c"
if (_au0_this -> _expr__O5.__C5_tp2 )_au0_this -> _expr__O5.__C5_tp2 -> _node_permanent = 1 ;
goto rrr ;
case 68 : 
#line 346 "../../src/expand.c"
_au0_this -> _expr__O5.__C5_cond = _expr_expand ( _au0_this -> _expr__O5.__C5_cond ) ;
default : 
#line 348 "../../src/expand.c"
if (_au0_this -> _expr__O4.__C4_e2 )_au0_this -> _expr__O4.__C4_e2 = _expr_expand ( _au0_this -> _expr__O4.__C4_e2 ) ;
case 44 : 
#line 350 "../../src/expand.c"
case 45 : 
#line 351 "../../src/expand.c"
rrr :
#line 352 "../../src/expand.c"
if (_au0_this -> _expr__O3.__C3_e1 )_au0_this -> _expr__O3.__C3_e1 = _expr_expand ( _au0_this -> _expr__O3.__C3_e1 ) ;
break ;
}

#line 356 "../../src/expand.c"
_au0_this -> _node_permanent = 1 ;
return _au0_this ;
}
;
bit _expr_not_simple (_au0_this )
#line 525 "../../src/cfront.h"
struct expr *_au0_this ;

#line 367 "../../src/expand.c"
{ 
#line 368 "../../src/expand.c"
int _au1_s ;

#line 370 "../../src/expand.c"
switch (_au0_this -> _node_base ){ 
#line 371 "../../src/expand.c"
default : 
#line 372 "../../src/expand.c"
return (char )2 ;
case 86 : 
#line 374 "../../src/expand.c"
case 150 : 
#line 375 "../../src/expand.c"
case 151 : 
#line 376 "../../src/expand.c"
case 82 : 
#line 377 "../../src/expand.c"
case 84 : 
#line 378 "../../src/expand.c"
case 83 : 
#line 379 "../../src/expand.c"
case 81 :
#line 379 "../../src/expand.c"

#line 380 "../../src/expand.c"
case 85 : 
#line 381 "../../src/expand.c"
return (char )0 ;
case 30 : 
#line 383 "../../src/expand.c"
return (char )(((_au0_this -> _expr__O3.__C3_e1 == 0 )|| (_au0_this -> _expr__O3.__C3_e1 == dummy ))? 0: (((int )_expr_not_simple ( _au0_this ->
#line 383 "../../src/expand.c"
_expr__O3.__C3_e1 ) )));
case 145 : 
#line 385 "../../src/expand.c"
case 112 : 
#line 386 "../../src/expand.c"
return _expr_not_simple ( _au0_this -> _expr__O4.__C4_e2 ) ;
case 113 : 
#line 388 "../../src/expand.c"
case 45 : 
#line 389 "../../src/expand.c"
case 44 : 
#line 390 "../../src/expand.c"
return _expr_not_simple ( _au0_this -> _expr__O3.__C3_e1 ) ;
case 107 : 
#line 392 "../../src/expand.c"
case 46 : 
#line 393 "../../src/expand.c"
case 47 : 
#line 394 "../../src/expand.c"
return _expr_not_simple ( _au0_this -> _expr__O4.__C4_e2 ) ;
case 111 : 
#line 396 "../../src/expand.c"
_au1_s = _expr_not_simple ( _au0_this -> _expr__O3.__C3_e1 ) ;
if (1 < _au1_s )return (char )2 ;
if (_au0_this -> _expr__O4.__C4_e2 == 0 )return (char )_au1_s ;
return (char )(_au1_s |= _expr_not_simple ( _au0_this -> _expr__O4.__C4_e2 ) );
case 50 : 
#line 401 "../../src/expand.c"
case 51 : 
#line 402 "../../src/expand.c"
case 53 : 
#line 403 "../../src/expand.c"
case 54 : 
#line 404 "../../src/expand.c"
case 55 : 
#line 405 "../../src/expand.c"
case 56 : 
#line 406 "../../src/expand.c"
case 57 :
#line 406 "../../src/expand.c"

#line 407 "../../src/expand.c"
case 52 : 
#line 408 "../../src/expand.c"
case 65 : 
#line 409 "../../src/expand.c"
case 64 : 
#line 410 "../../src/expand.c"
case 58 : 
#line 411 "../../src/expand.c"
case 59 : 
#line 412 "../../src/expand.c"
case 60 : 
#line 413 "../../src/expand.c"
case 61 :
#line 413 "../../src/expand.c"

#line 414 "../../src/expand.c"
case 62 : 
#line 415 "../../src/expand.c"
case 63 : 
#line 416 "../../src/expand.c"
case 66 : 
#line 417 "../../src/expand.c"
case 67 : 
#line 418 "../../src/expand.c"
case 71 : 
#line 419 "../../src/expand.c"
_au1_s = _expr_not_simple ( _au0_this -> _expr__O3.__C3_e1 )
#line 419 "../../src/expand.c"
;
if (1 < _au1_s )return (char )2 ;
return (char )(_au1_s |= _expr_not_simple ( _au0_this -> _expr__O4.__C4_e2 ) );
case 68 : 
#line 423 "../../src/expand.c"
_au1_s = _expr_not_simple ( _au0_this -> _expr__O5.__C5_cond ) ;
if (1 < _au1_s )return (char )2 ;
_au1_s |= _expr_not_simple ( _au0_this -> _expr__O3.__C3_e1 ) ;
if (1 < _au1_s )return (char )2 ;
return (char )(_au1_s |= _expr_not_simple ( _au0_this -> _expr__O4.__C4_e2 ) );
case 169 : 
#line 429 "../../src/expand.c"
if (curr_icall ){ 
#line 430 "../../src/expand.c"
Pname _au3_n ;
int _au3_argno ;
Pin _au3_il ;

#line 430 "../../src/expand.c"
_au3_n = (((struct name *)_au0_this ));
_au3_argno = _au3_n -> _name_n_val ;
for(_au3_il = curr_icall ;_au3_il ;_au3_il = _au3_il -> _iline_i_next ) 
#line 433 "../../src/expand.c"
if (_au3_n -> _expr__O5.__C5_n_table == _au3_il -> _iline_i_table )goto aok ;
goto bok ;
aok :
#line 436 "../../src/expand.c"
return (char )((_au3_il -> _iline_local [_au3_argno ])? 0: (((int )_expr_not_simple ( _au3_il -> _iline_arg [_au3_argno ]) )));
}
bok :{ 
#line 460 "../../src/expand.c"
struct ea _au0__V20 ;

#line 438 "../../src/expand.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"expand aname%n", (struct ea *)( ( ((& _au0__V20 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((&
#line 438 "../../src/expand.c"
_au0__V20 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
case 147 : 
#line 440 "../../src/expand.c"
case 157 : 
#line 441 "../../src/expand.c"
case 23 : 
#line 442 "../../src/expand.c"
case 109 : 
#line 443 "../../src/expand.c"
case 146 : 
#line 444 "../../src/expand.c"
case 168 : 
#line 445 "../../src/expand.c"
case 70 :
#line 445 "../../src/expand.c"

#line 446 "../../src/expand.c"
case 48 : 
#line 447 "../../src/expand.c"
case 49 : 
#line 448 "../../src/expand.c"
case 126 : 
#line 449 "../../src/expand.c"
case 127 : 
#line 450 "../../src/expand.c"
case 128 : 
#line 451 "../../src/expand.c"
case 129 : 
#line 452 "../../src/expand.c"
case 130 :
#line 452 "../../src/expand.c"

#line 453 "../../src/expand.c"
case 131 : 
#line 454 "../../src/expand.c"
case 132 : 
#line 455 "../../src/expand.c"
case 133 : 
#line 456 "../../src/expand.c"
case 134 : 
#line 457 "../../src/expand.c"
case 135 : 
#line 458 "../../src/expand.c"
return (char )2 ;
} }
}
;

#line 463 "../../src/expand.c"
Pexpr _fct_expand (_au0_this , _au0_fn , _au0_scope , _au0_ll )
#line 400 "../../src/cfront.h"
struct fct *_au0_this ;

#line 463 "../../src/expand.c"
Pname _au0_fn ;

#line 463 "../../src/expand.c"
Ptable _au0_scope ;

#line 463 "../../src/expand.c"
Pexpr _au0_ll ;

#line 471 "../../src/expand.c"
{ 
#line 495 "../../src/expand.c"
Pin _au1_il ;
Pexpr _au1_ic ;

#line 500 "../../src/expand.c"
Pname _au1_n ;
Pname _au1_at ;

#line 503 "../../src/expand.c"
int _au1_i ;
int _au1_not_simple ;

#line 511 "../../src/expand.c"
Pexpr _au2_ee ;

#line 512 "../../src/expand.c"
struct texpr *_au0__Xthis__ctor_texpr ;

#line 473 "../../src/expand.c"
if ((((((_au0_this -> _fct_body == 0 )&& (_au0_this -> _fct_f_expr == 0 ))|| ((_au0_this -> _type_defined & 02 )== 0 ))|| ((((struct fct *)_au0_fn -> _expr__O2.__C2_tp ))->
#line 473 "../../src/expand.c"
_fct_body -> _stmt_memtbl == _au0_scope ))|| (_au0_this -> _fct_f_inline > 2 ))|| (_au0_this -> _fct_last_expanded && (_au0_this -> _fct_last_expanded == curr_expr )))
#line 478 "../../src/expand.c"
{ 
#line 479 "../../src/expand.c"
if ((_au0_this -> _fct_f_inline ==
#line 479 "../../src/expand.c"
2 )|| (_au0_this -> _fct_f_inline == 4 ))
#line 480 "../../src/expand.c"
{ 
#line 642 "../../src/expand.c"
struct ea _au0__V21 ;

#line 642 "../../src/expand.c"
struct ea _au0__V22 ;

#line 480 "../../src/expand.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"cannot expand%n twice inE: %n declared as non-inline but defined as inline", (struct ea *)( ( ((& _au0__V21 )-> _ea__O1.__C1_p = ((char *)_au0_fn )), (((&
#line 480 "../../src/expand.c"
_au0__V21 )))) ) , (struct ea *)( ( ((& _au0__V22 )-> _ea__O1.__C1_p = ((char *)_au0_fn )), (((& _au0__V22 )))) )
#line 480 "../../src/expand.c"
, (struct ea *)ea0 , (struct ea *)ea0 ) ;
} 
#line 482 "../../src/expand.c"
( (_au0_fn -> _name_n_addr_taken ++ )) ;
if (_au0_fn -> _name_n_addr_taken == 1 ){ 
#line 484 "../../src/expand.c"
Pname _au3_nn ;

#line 484 "../../src/expand.c"
_au3_nn = (struct name *)_name__ctor ( (struct name *)0 , (char *)0 ) ;
(*_au3_nn )= (*_au0_fn );
_au3_nn -> _name_n_list = dcl_list ;
_au3_nn -> _name_n_sto = 31 ;
dcl_list = _au3_nn ;
}
return (struct expr *)0 ;
}

#line 493 "../../src/expand.c"
_au0_this -> _fct_f_inline += 2 ;

#line 495 "../../src/expand.c"
_au1_il = (((struct iline *)_new ( (long )(sizeof (struct iline ))) ));

#line 495 "../../src/expand.c"
_au1_ic = (struct expr *)( (_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr = (struct texpr *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_texpr ),
#line 495 "../../src/expand.c"
((unsigned char )168 ), ((struct expr *)0 ), (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 = ((struct type *)0 )),
#line 495 "../../src/expand.c"
((_au0__Xthis__ctor_texpr ))) ) ) ;

#line 497 "../../src/expand.c"
_au1_il -> _iline_fct_name = _au0_fn ;
_au1_ic -> _expr__O5.__C5_il = _au1_il ;
_au1_ic -> _expr__O2.__C2_tp = _au0_this -> _fct_returns ;
;

#line 500 "../../src/expand.c"
_au1_at = (_au0_this -> _fct_f_this ? _au0_this -> _fct_f_this : (_au0_this -> _fct_f_result ? _au0_this -> _fct_f_result : _au0_this -> _fct_argtype ));

#line 502 "../../src/expand.c"
if (_au1_at )_au1_il -> _iline_i_table = _au1_at -> _expr__O5.__C5_n_table ;
_au1_i = 0 ;

#line 503 "../../src/expand.c"
_au1_not_simple = 0 ;

#line 506 "../../src/expand.c"
for(_au1_n = _au1_at ;_au1_n ;( (_au1_n = _au1_n -> _name_n_list ), (_au1_i ++ )) ) { 
#line 510 "../../src/expand.c"
if (_au0_ll == 0 ){ 
#line 642 "../../src/expand.c"
struct ea _au0__V23 ;
#line 642 "../../src/expand.c"

#line 510 "../../src/expand.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"F::expand(%n):AX", (struct ea *)( ( ((& _au0__V23 )-> _ea__O1.__C1_p = ((char *)_au0_fn )), (((&
#line 510 "../../src/expand.c"
_au0__V23 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} ;

#line 513 "../../src/expand.c"
if (_au0_ll -> _node_base == 140 ){ 
#line 514 "../../src/expand.c"
_au2_ee = _au0_ll -> _expr__O3.__C3_e1 ;
_au0_ll = _au0_ll -> _expr__O4.__C4_e2 ;
}
else { 
#line 518 "../../src/expand.c"
_au2_ee = _au0_ll ;
_au0_ll = 0 ;
}

#line 522 "../../src/expand.c"
{ int _au2_s ;
Pname _au2_nn ;

#line 523 "../../src/expand.c"
_au2_nn = 0 ;

#line 525 "../../src/expand.c"
if (_au1_n -> _name_n_assigned_to == 111 ){ 
#line 526 "../../src/expand.c"
if ((_au2_ee != zero )&& (_expr_not_simple ( _au2_ee ) == 0 )){ 
#line 528 "../../src/expand.c"
(_au1_il -> _iline_local [_au1_i ])= 0 ;
#line 528 "../../src/expand.c"

#line 529 "../../src/expand.c"
goto zxc ;
}
}

#line 533 "../../src/expand.c"
if (_au1_n -> _name_n_addr_taken || _au1_n -> _name_n_assigned_to ){ 
#line 534 "../../src/expand.c"
_au2_nn = dcl_local ( _au0_scope , _au1_n , _au0_fn ) ;
_au2_nn -> _node_base = 85 ;
(_au1_il -> _iline_local [_au1_i ])= _au2_nn ;
++ _au1_not_simple ;
}
else if ((_au1_n -> _name_n_used && (_au2_s = _expr_not_simple ( _au2_ee ) ))&& ((1 < _au2_s )|| (1 < _au1_n -> _name_n_used )))
#line 541 "../../src/expand.c"
{ 
#line 542 "../../src/expand.c"
_au2_nn =
#line 542 "../../src/expand.c"
dcl_local ( _au0_scope , _au1_n , _au0_fn ) ;
_au2_nn -> _node_base = 85 ;
(_au1_il -> _iline_local [_au1_i ])= _au2_nn ;
++ _au1_not_simple ;
}
else if (_expr_not_simple ( _au2_ee ) ){ 
#line 549 "../../src/expand.c"
_au2_nn = dcl_local ( _au0_scope , _au1_n , _au0_fn ) ;
_au2_nn -> _node_base = 85 ;
(_au1_il -> _iline_local [_au1_i ])= _au2_nn ;
++ _au1_not_simple ;
}
else 
#line 555 "../../src/expand.c"
(_au1_il -> _iline_local [_au1_i ])= 0 ;
zxc :
#line 557 "../../src/expand.c"
(_au1_il -> _iline_arg [_au1_i ])= _au2_ee ;
(_au1_il -> _iline_tp [_au1_i ])= _au1_n -> _expr__O2.__C2_tp ;
}
}
{ Ptable _au1_tbl ;

#line 611 "../../src/expand.c"
Pstmt _au2_ss ;

#line 561 "../../src/expand.c"
_au1_tbl = _au0_this -> _fct_body -> _stmt_memtbl ;
if (_au0_this -> _fct_f_expr ){ 
#line 563 "../../src/expand.c"
char _au2_loc_var ;

#line 580 "../../src/expand.c"
Pexpr _au2_ex ;

#line 563 "../../src/expand.c"
_au2_loc_var = 0 ;

#line 566 "../../src/expand.c"
for(_au1_n = _table_get_mem ( _au1_tbl , _au1_i = 1 ) ;_au1_n ;_au1_n = _table_get_mem ( _au1_tbl , ++ _au1_i ) ) { 
#line 568 "../../src/expand.c"
if ((_au1_n ->
#line 568 "../../src/expand.c"
_node_base == 85 )&& ((_au1_n -> _name_n_used || _au1_n -> _name_n_assigned_to )|| _au1_n -> _name_n_addr_taken ))
#line 569 "../../src/expand.c"
{ 
#line 570 "../../src/expand.c"
Pname _au4_nn ;

#line 570 "../../src/expand.c"
_au4_nn = dcl_local ( _au0_scope , _au1_n , _au0_fn ) ;
_au4_nn -> _node_base = 85 ;
_au1_n -> _expr__O3.__C3_string = _au4_nn -> _expr__O3.__C3_string ;
_au2_loc_var ++ ;
}
}

#line 578 "../../src/expand.c"
if (_au1_not_simple || _au2_loc_var )_au0_this -> _fct_last_expanded = curr_expr ;

#line 580 "../../src/expand.c"
;
if (_au1_not_simple ){ 
#line 582 "../../src/expand.c"
Pexpr _au3_etail ;

#line 582 "../../src/expand.c"
_au3_etail = (_au2_ex = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )71 , (struct expr *)0 , (struct expr *)0 ) );
#line 582 "../../src/expand.c"

#line 583 "../../src/expand.c"
for(_au1_i = 0 ;_au1_i < 8;_au1_i ++ ) { 
#line 584 "../../src/expand.c"
Pname _au4_n ;

#line 584 "../../src/expand.c"
_au4_n = (_au1_il -> _iline_local [_au1_i ]);
if (_au4_n == 0 )continue ;
{ Pexpr _au4_e ;

#line 586 "../../src/expand.c"
_au4_e = (_au1_il -> _iline_arg [_au1_i ]);

#line 588 "../../src/expand.c"
_au3_etail -> _expr__O3.__C3_e1 = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )70 , (struct expr *)_au4_n , _au4_e ) ;
if (-- _au1_not_simple )
#line 590 "../../src/expand.c"
_au3_etail = (_au3_etail -> _expr__O4.__C4_e2 = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )71 , (struct expr *)0 ,
#line 590 "../../src/expand.c"
(struct expr *)0 ) );
else 
#line 592 "../../src/expand.c"
break ;
}
}

#line 594 "../../src/expand.c"
_au3_etail -> _expr__O4.__C4_e2 = _au0_this -> _fct_f_expr ;
}
else { 
#line 597 "../../src/expand.c"
_au2_ex = _au0_this -> _fct_f_expr ;
}
_au1_ic -> _expr__O3.__C3_e1 = _au2_ex ;
}
else { 
#line 602 "../../src/expand.c"
for(_au1_n = _table_get_mem ( _au1_tbl , _au1_i = 1 ) ;_au1_n ;_au1_n = _table_get_mem ( _au1_tbl , ++ _au1_i ) ) {
#line 602 "../../src/expand.c"

#line 605 "../../src/expand.c"
if ((_au1_n -> _node_base == 85 )&& ((_au1_n -> _name_n_used || _au1_n -> _name_n_assigned_to )|| _au1_n -> _name_n_addr_taken ))
#line 606 "../../src/expand.c"
{ 
#line 607 "../../src/expand.c"
Pname _au4_cn ;

#line 607 "../../src/expand.c"
_au4_cn = _au0_fn -> _expr__O5.__C5_n_table -> _table_t_name ;
_au1_n -> _expr__O3.__C3_string = temp ( _au1_n -> _expr__O3.__C3_string , _au0_fn -> _expr__O3.__C3_string , _au4_cn ? _au4_cn -> _expr__O3.__C3_string : (((char *)0 ))) ;
}
}
;
if (_au1_not_simple ){ 
#line 613 "../../src/expand.c"
_au0_this -> _fct_last_expanded = curr_expr ;
{ Pstmt _au3_st ;

#line 615 "../../src/expand.c"
struct estmt *_au0__Xthis__ctor_estmt ;

#line 614 "../../src/expand.c"
_au3_st = (struct stmt *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt = (struct estmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_estmt ),
#line 614 "../../src/expand.c"
((unsigned char )72 ), curloc , ((struct stmt *)0 )) )) , ( (_au0__Xthis__ctor_estmt -> _stmt__O8.__C8_e = ((struct expr *)0 )), ((_au0__Xthis__ctor_estmt )))
#line 614 "../../src/expand.c"
) ) ;
_au3_st -> _stmt_where . _loc_line = 0 ;
{ Pstmt _au3_stail ;

#line 617 "../../src/expand.c"
struct block *_au0__Xthis__ctor_block ;

#line 616 "../../src/expand.c"
_au3_stail = _au3_st ;
for(_au1_i = 0 ;_au1_i < 8;_au1_i ++ ) { 
#line 618 "../../src/expand.c"
Pname _au4_n ;

#line 618 "../../src/expand.c"
_au4_n = (_au1_il -> _iline_local [_au1_i ]);
if (_au4_n == 0 )continue ;
{ Pexpr _au4_e ;

#line 621 "../../src/expand.c"
struct estmt *_au0__Xthis__ctor_estmt ;

#line 620 "../../src/expand.c"
_au4_e = (_au1_il -> _iline_arg [_au1_i ]);
_au3_stail -> _stmt__O8.__C8_e = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )70 , (struct expr *)_au4_n , _au4_e ) ;

#line 623 "../../src/expand.c"
if (-- _au1_not_simple ){ 
#line 624 "../../src/expand.c"
_au3_stail = (_au3_stail -> _stmt_s_list = (struct stmt *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ),
#line 624 "../../src/expand.c"
(_au0__Xthis__ctor_estmt = (struct estmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_estmt ), ((unsigned char )72 ), curloc , ((struct stmt *)0 )) )) , (
#line 624 "../../src/expand.c"
(_au0__Xthis__ctor_estmt -> _stmt__O8.__C8_e = ((struct expr *)0 )), ((_au0__Xthis__ctor_estmt ))) ) ) );
_au3_stail -> _stmt_where . _loc_line = 0 ;
}
else 
#line 628 "../../src/expand.c"
break ;
}
}

#line 630 "../../src/expand.c"
_au3_stail -> _stmt_s_list = (struct stmt *)_au0_this -> _fct_body ;
_au2_ss = (struct stmt *)( (_au0__Xthis__ctor_block = 0 ), ( ( (_au0__Xthis__ctor_block = 0 ), (_au0__Xthis__ctor_block = (struct block *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_block ),
#line 631 "../../src/expand.c"
(unsigned char )116 , curloc , _au3_st ) )) , ( (_au0__Xthis__ctor_block -> _stmt__O7.__C7_d = ((struct name *)0 )), ((_au0__Xthis__ctor_block ))) )
#line 631 "../../src/expand.c"
) ;
_au2_ss -> _stmt_where . _loc_line = 0 ;
}
}
}
else 
#line 634 "../../src/expand.c"
{ 
#line 635 "../../src/expand.c"
_au2_ss = (struct stmt *)_au0_this -> _fct_body ;
}
_au1_ic -> _expr__O4.__C4_e2 = (((struct expr *)_au2_ss ));
}

#line 640 "../../src/expand.c"
_au0_this -> _fct_f_inline -= 2 ;
return _au1_ic ;
}
}
;

/* the end */
