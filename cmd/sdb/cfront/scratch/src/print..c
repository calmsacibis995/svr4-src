/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#line 1 "../../src/print.c"

/* <<cfront 1.2.1 2/16/87>> */
/* < ../../src/print.c */

#ident	"@(#)sdb:cfront/scratch/src/print..c	1.2"
char *_new(); char _delete(); char *_vec_new(); char _vec_delete();

#line 1 "../../src/print.c"

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

#line 21 "../../src/print.c"
extern FILE *out_file ;
char emode = 0 ;
extern int ntok ;
int ntok = 0 ;
bit Cast ;
Pin curr_icall ;
static int last_ll = 1 ;

#line 29 "../../src/print.c"
int MAIN = 0 ;

#line 31 "../../src/print.c"
char puttok (_au0_t )TOK _au0_t ;

#line 35 "../../src/print.c"
{ 
#line 39 "../../src/print.c"
if (keys [_au0_t ])fputs ( (char *)(keys [_au0_t ]), out_file ) ;
if (12 < (ntok ++ )){ 
#line 41 "../../src/print.c"
ntok = 0 ;
_loc_putline ( & last_line ) ;
}
else if (_au0_t == 72 ){ 
#line 45 "../../src/print.c"
ntok = 0 ;
putc('\n', out_file);
if (last_ll )last_line . _loc_line ++ ;
}
else 
#line 50 "../../src/print.c"
putc(' ', out_file);
}
;

#line 55 "../../src/print.c"
struct dcl_buf {	/* sizeof dcl_buf == 216 */

#line 68 "../../src/print.c"
Pbase _dcl_buf_b ;
Pname _dcl_buf_n ;
TOK _dcl_buf_left [20];

#line 70 "../../src/print.c"
TOK _dcl_buf_right [20];
Pnode _dcl_buf_rnode [20];
Pclass _dcl_buf_lnode [20];
int _dcl_buf_li ;

#line 73 "../../src/print.c"
int _dcl_buf_ri ;
};

#line 77 "../../src/print.c"
	/* overload front: */

#line 78 "../../src/print.c"

#line 81 "../../src/print.c"
char _dcl_buf_put ();
struct dcl_buf *tbufvec [10]= { 0 } ;

#line 82 "../../src/print.c"
struct dcl_buf *tbuf = 0 ;

#line 84 "../../src/print.c"
int freetbuf = 0 ;

#line 86 "../../src/print.c"
char _dcl_buf_put (_au0_this )
#line 82 "../../src/print.c"
struct dcl_buf *_au0_this ;

#line 87 "../../src/print.c"
{ 
#line 88 "../../src/print.c"
int _au1_i ;

#line 90 "../../src/print.c"
if ((20 <= _au0_this -> _dcl_buf_li )|| (20 <= _au0_this -> _dcl_buf_ri ))errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"T buffer overflow", (struct ea *)ea0 , (struct
#line 90 "../../src/print.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
if (_au0_this -> _dcl_buf_b == 0 ){ 
#line 158 "../../src/print.c"
struct ea _au0__V10 ;

#line 91 "../../src/print.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"noBT%s", (struct ea *)( ( ((& _au0__V10 )-> _ea__O1.__C1_p = ((char *)(Cast ? " in cast":
#line 91 "../../src/print.c"
""))), (((& _au0__V10 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} 
#line 93 "../../src/print.c"
if ((_au0_this -> _dcl_buf_n && _au0_this -> _dcl_buf_n -> _name_n_sto )&& (_au0_this -> _dcl_buf_n -> _name_n_sto != 27 ))puttok ( _au0_this -> _dcl_buf_n -> _name_n_sto )
#line 93 "../../src/print.c"
;

#line 95 "../../src/print.c"
_basetype_dcl_print ( _au0_this -> _dcl_buf_b ) ;

#line 97 "../../src/print.c"
for(;_au0_this -> _dcl_buf_li ;_au0_this -> _dcl_buf_li -- ) { 
#line 98 "../../src/print.c"
switch (_au0_this -> _dcl_buf_left [_au0_this -> _dcl_buf_li ]){ 
#line 99 "../../src/print.c"
case 40 : 
#line 100 "../../src/print.c"
putc('(', out_file);
#line 100 "../../src/print.c"

#line 101 "../../src/print.c"
break ;
case 125 : 
#line 103 "../../src/print.c"
putc('*', out_file);
break ;
case 158 : 
#line 106 "../../src/print.c"
if (emode )
#line 107 "../../src/print.c"
putc('&', out_file);
else 
#line 109 "../../src/print.c"
putc('*', out_file);
break ;
case 163 : 
#line 112 "../../src/print.c"
if (emode )
#line 113 "../../src/print.c"
fputs ( (char *)"*const ", out_file ) ;
else 
#line 115 "../../src/print.c"
putc('*', out_file);
break ;
case 164 : 
#line 118 "../../src/print.c"
if (emode )
#line 119 "../../src/print.c"
fputs ( (char *)"&const ", out_file ) ;
else 
#line 121 "../../src/print.c"
putc('*', out_file);
break ;
case 173 : 
#line 124 "../../src/print.c"
if (_au0_this -> _dcl_buf_lnode [_au0_this -> _dcl_buf_li ])fprintf ( out_file , (char *)"%s::", (_au0_this -> _dcl_buf_lnode [_au0_this -> _dcl_buf_li ])-> _classdef_string ) ;
#line 124 "../../src/print.c"
}
}

#line 128 "../../src/print.c"
if (_au0_this -> _dcl_buf_n )_name_print ( _au0_this -> _dcl_buf_n ) ;

#line 130 "../../src/print.c"
for(_au1_i = 1 ;_au1_i <= _au0_this -> _dcl_buf_ri ;_au1_i ++ ) 
#line 131 "../../src/print.c"
switch (_au0_this -> _dcl_buf_right [_au1_i ]){ 
#line 132 "../../src/print.c"
case 41 : 
#line 133 "../../src/print.c"
putc(')', out_file);
break ;
case 110 : 
#line 136 "../../src/print.c"
putc('[', out_file);
{ Pvec _au3_v ;
Pexpr _au3_d ;
int _au3_s ;

#line 137 "../../src/print.c"
_au3_v = (((struct vec *)(_au0_this -> _dcl_buf_rnode [_au1_i ])));
_au3_d = _au3_v -> _vec_dim ;
_au3_s = _au3_v -> _vec_size ;
if (_au3_d )_expr_print ( _au3_d ) ;
if (_au3_s )fprintf ( out_file , (char *)"%d", _au3_s ) ;
}
putc(']', out_file);
break ;
case 108 : 
#line 146 "../../src/print.c"
_fct_dcl_print ( ((struct fct *)(_au0_this -> _dcl_buf_rnode [_au1_i ]))) ;
break ;
case 114 : 
#line 149 "../../src/print.c"
{ Pbase _au3_f ;
Pexpr _au3_d ;
int _au3_s ;

#line 149 "../../src/print.c"
_au3_f = (((struct basetype *)(_au0_this -> _dcl_buf_rnode [_au1_i ])));
_au3_d = (((struct expr *)_au3_f -> _basetype_b_name ));
_au3_s = _au3_f -> _basetype_b_bits ;
putc(':', out_file);
if (_au3_d )_expr_print ( _au3_d ) ;
if (_au3_s )fprintf ( out_file , (char *)"%d", _au3_s ) ;
}
break ;
}
}
;

#line 162 "../../src/print.c"
char Eprint (_au0_e )Pexpr _au0_e ;
{ 
#line 164 "../../src/print.c"
switch (_au0_e -> _node_base ){ 
#line 165 "../../src/print.c"
case 44 : 
#line 166 "../../src/print.c"
if ((((struct ref *)_au0_e ))-> _expr__O5.__C5_mem -> _expr__O2.__C2_tp -> _node_base == 108 ){ 
#line 169 "../../src/print.c"
_name_print (
#line 169 "../../src/print.c"
(((struct ref *)_au0_e ))-> _expr__O5.__C5_mem ) ;
break ;
}
case 85 : 
#line 173 "../../src/print.c"
case 80 : 
#line 174 "../../src/print.c"
case 86 : 
#line 175 "../../src/print.c"
case 82 : 
#line 176 "../../src/print.c"
case 84 : 
#line 177 "../../src/print.c"
case 83 : 
#line 178 "../../src/print.c"
case 81 :
#line 178 "../../src/print.c"

#line 179 "../../src/print.c"
case 150 : 
#line 180 "../../src/print.c"
case 165 : 
#line 181 "../../src/print.c"
case 71 : 
#line 182 "../../src/print.c"
case 147 : 
#line 183 "../../src/print.c"
case 140 : 
#line 184 "../../src/print.c"
case 69 : 
#line 185 "../../src/print.c"
case 124 :
#line 185 "../../src/print.c"

#line 186 "../../src/print.c"
case 45 : 
#line 187 "../../src/print.c"
case 34 : 
#line 188 "../../src/print.c"
case 109 : 
#line 189 "../../src/print.c"
case 146 : 
#line 190 "../../src/print.c"
case 168 : 
#line 191 "../../src/print.c"
case 169 : 
#line 192 "../../src/print.c"
_expr_print ( _au0_e )
#line 192 "../../src/print.c"
;
case 144 : 
#line 194 "../../src/print.c"
break ;
default : 
#line 196 "../../src/print.c"
putc('(', out_file);
_expr_print ( _au0_e ) ;
putc(')', out_file);
}
}
;
char _name_dcl_print (_au0_this , _au0_list )
#line 607 "../../src/cfront.h"
struct name *_au0_this ;

#line 202 "../../src/print.c"
TOK _au0_list ;

#line 215 "../../src/print.c"
{ 
#line 216 "../../src/print.c"
Pname _au1_n ;

#line 218 "../../src/print.c"
if (_au0_this == 0 )error ( (char *)"0->N::dcl_print()", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 218 "../../src/print.c"

#line 220 "../../src/print.c"
for(_au1_n = _au0_this ;_au1_n ;_au1_n = _au1_n -> _name_n_list ) { 
#line 221 "../../src/print.c"
Ptype _au2_t ;
int _au2_sm ;

#line 233 "../../src/print.c"
int _au2_tc ;
Ptype _au2_tt ;

#line 221 "../../src/print.c"
_au2_t = _au1_n -> _expr__O2.__C2_tp ;
_au2_sm = 0 ;

#line 224 "../../src/print.c"
if (_au2_t == 0 ){ 
#line 459 "../../src/print.c"
struct ea _au0__V11 ;

#line 224 "../../src/print.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"N::dcl_print(%n)T missing", (struct ea *)( ( ((& _au0__V11 )-> _ea__O1.__C1_p = ((char *)_au1_n )), (((&
#line 224 "../../src/print.c"
_au0__V11 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} if (_au1_n -> _name_n_stclass == 13 )continue ;

#line 227 "../../src/print.c"
if (_au1_n -> _name_where . _loc_line != last_line . _loc_line )
#line 228 "../../src/print.c"
if (last_ll = _au1_n -> _name_where . _loc_line )
#line 229 "../../src/print.c"
_loc_putline ( & _au1_n -> _name_where ) ;
#line 229 "../../src/print.c"
else 
#line 231 "../../src/print.c"
_loc_putline ( & last_line ) ;

#line 233 "../../src/print.c"
_au2_tc = (((struct basetype *)_au2_t ))-> _basetype_b_const ;
for(_au2_tt = _au2_t ;_au2_tt -> _node_base == 97 ;_au2_tt = (((struct basetype *)_au2_tt ))-> _basetype_b_name -> _expr__O2.__C2_tp ) 
#line 235 "../../src/print.c"
_au2_tc |= (((struct basetype *)_au2_tt ))-> _basetype_b_const ;

#line 237 "../../src/print.c"
switch (_au2_t -> _node_base ){ 
#line 238 "../../src/print.c"
case 6 : 
#line 239 "../../src/print.c"
if (_au1_n -> _node_base != 123 ){ 
#line 240 "../../src/print.c"
_classdef_dcl_print ( ((struct classdef *)_au2_t ), _au1_n ) ;
#line 240 "../../src/print.c"

#line 241 "../../src/print.c"
_au2_sm = 1 ;
}
break ;

#line 245 "../../src/print.c"
case 13 : 
#line 246 "../../src/print.c"
_enumdef_dcl_print ( ((struct enumdef *)_au2_t ), _au1_n ) ;
_au2_sm = 1 ;
break ;

#line 250 "../../src/print.c"
case 108 : 
#line 251 "../../src/print.c"
{ Pfct _au4_f ;

#line 252 "../../src/print.c"
struct name *_au0__Xthis_use_name ;

#line 251 "../../src/print.c"
_au4_f = (((struct fct *)_au2_t ));
if (_au1_n -> _node_base == 123 )puttok ( (unsigned char )35 ) ;

#line 254 "../../src/print.c"
if (_au4_f -> _fct_f_inline ){ 
#line 255 "../../src/print.c"
if (_au4_f -> _fct_f_virtual || _au1_n -> _name_n_addr_taken ){ 
#line 256 "../../src/print.c"
TOK _au6_st ;
Pblock _au6_b ;

#line 256 "../../src/print.c"
_au6_st = _au1_n -> _name_n_sto ;
_au6_b = _au4_f -> _fct_body ;
_au4_f -> _fct_body = 0 ;

#line 260 "../../src/print.c"
_type_dcl_print ( _au2_t , _au1_n ) ;
_au1_n -> _name_n_sto = _au6_st ;
_au4_f -> _fct_body = _au6_b ;
}
else 
#line 265 "../../src/print.c"
_au2_sm = 1 ;
}
else if ((_au1_n -> _expr__O5.__C5_n_table == gtbl )&& (strcmp ( (char *)_au1_n -> _expr__O3.__C3_string , (char *)"main") == 0 )){ 
#line 268 "../../src/print.c"
MAIN =
#line 268 "../../src/print.c"
1 ;
( (_au0__Xthis_use_name = _table_look ( gtbl , "main", (unsigned char )0 ) ), ( (_au0__Xthis_use_name -> _name_n_used ++ )) )
#line 269 "../../src/print.c"
;
_type_dcl_print ( _au2_t , _au1_n ) ;
MAIN = 0 ;
}
else 
#line 274 "../../src/print.c"
_type_dcl_print ( _au2_t , _au1_n ) ;
break ;
}

#line 278 "../../src/print.c"
case 76 : 
#line 279 "../../src/print.c"
{ Pgen _au4_g ;
Plist _au4_gl ;

#line 279 "../../src/print.c"
_au4_g = (((struct gen *)_au2_t ));

#line 281 "../../src/print.c"
fprintf ( out_file , (char *)"\t/* overload %s: */\n", _au4_g -> _gen_string ) ;
for(_au4_gl = _au4_g -> _gen_fct_list ;_au4_gl ;_au4_gl = _au4_gl -> _name_list_l ) { 
#line 283 "../../src/print.c"
Pname _au5_nn ;

#line 283 "../../src/print.c"
_au5_nn = _au4_gl -> _name_list_f ;
_name_dcl_print ( _au5_nn , (unsigned char )0 ) ;
_au2_sm = 1 ;
}
break ;
}

#line 290 "../../src/print.c"
case 1 : 
#line 291 "../../src/print.c"
fprintf ( out_file , (char *)"asm(\"%s\")\n", ((char *)(((struct basetype *)_au2_t ))-> _basetype_b_name )) ;
break ;

#line 294 "../../src/print.c"
case 21 : 
#line 295 "../../src/print.c"
case 5 : 
#line 296 "../../src/print.c"
case 22 : 
#line 297 "../../src/print.c"
case 29 : 
#line 298 "../../src/print.c"
tcx :
#line 300 "../../src/print.c"
if ((_au2_tc && (_au1_n -> _name_n_sto != 14 ))&& (((_au1_n ->
#line 300 "../../src/print.c"
_name_n_scope == 14 )|| (_au1_n -> _name_n_scope == 31 ))|| (_au1_n -> _name_n_scope == 108 )))
#line 309 "../../src/print.c"
{ 
#line 310 "../../src/print.c"
if (_au1_n -> _name_n_evaluated ){ 
#line 311 "../../src/print.c"
_au2_sm = 1 ;
break ;
}
}
_au2_tc = 0 ;

#line 318 "../../src/print.c"
default : 
#line 319 "../../src/print.c"
{ Pexpr _au4_i ;

#line 319 "../../src/print.c"
_au4_i = _au1_n -> _expr__O4.__C4_n_initializer ;

#line 321 "../../src/print.c"
if (_au2_tc ){ 
#line 322 "../../src/print.c"
switch (_au2_tt -> _node_base ){ 
#line 323 "../../src/print.c"
case 5 : 
#line 324 "../../src/print.c"
case 29 : 
#line 325 "../../src/print.c"
case 21 : 
#line 326 "../../src/print.c"
case 22 : 
#line 327 "../../src/print.c"
goto
#line 327 "../../src/print.c"
tcx ;
}
}
if (_au1_n -> _node_base == 123 )puttok ( (unsigned char )35 ) ;
if (_au1_n -> _name_n_stclass == 27 ){ 
#line 334 "../../src/print.c"
Pname _au5_cln ;

#line 334 "../../src/print.c"
_au5_cln = _type_is_cl_obj ( _au1_n -> _expr__O2.__C2_tp ) ;
if (_au5_cln ){ 
#line 336 "../../src/print.c"
Pclass _au6_cl ;

#line 336 "../../src/print.c"
_au6_cl = (((struct classdef *)_au5_cln -> _expr__O2.__C2_tp ));
if ((((_au6_cl -> _classdef_csu != 6 )&& (_au6_cl -> _classdef_clbase == 0 ))&& (_au6_cl -> _classdef_itor == 0 ))&& (_au6_cl -> _classdef_virt_count == 0 ))
#line 340 "../../src/print.c"
puttok ( (unsigned
#line 340 "../../src/print.c"
char )27 ) ;
}
else 
#line 343 "../../src/print.c"
puttok ( (unsigned char )27 ) ;
}

#line 346 "../../src/print.c"
if (_au4_i ){ 
#line 347 "../../src/print.c"
if ((_au1_n -> _name_n_sto == 14 )&& (_au1_n -> _name_n_stclass == 31 )){ 
#line 348 "../../src/print.c"
_au1_n -> _expr__O4.__C4_n_initializer = 0 ;
_type_dcl_print ( _au2_t , _au1_n ) ;
puttok ( (unsigned char )72 ) ;
_au1_n -> _expr__O4.__C4_n_initializer = _au4_i ;
_au1_n -> _name_n_sto = 0 ;
_type_dcl_print ( _au2_t , _au1_n ) ;
_au1_n -> _name_n_sto = 14 ;
}
else 
#line 357 "../../src/print.c"
_type_dcl_print ( _au2_t , _au1_n ) ;
}
else if (_au1_n -> _name_n_evaluated && (((struct basetype *)_au2_t ))-> _basetype_b_const ){ 
#line 360 "../../src/print.c"
if ((_au1_n -> _name_n_sto == 14 )&& (_au1_n -> _name_n_stclass == 31 )){
#line 360 "../../src/print.c"

#line 361 "../../src/print.c"
int _au6_v ;

#line 361 "../../src/print.c"
_au6_v = _au1_n -> _name_n_evaluated ;
_au1_n -> _name_n_evaluated = 0 ;
_type_dcl_print ( _au2_t , _au1_n ) ;
puttok ( (unsigned char )72 ) ;
_au1_n -> _name_n_evaluated = _au6_v ;
_au1_n -> _name_n_sto = 0 ;
_type_dcl_print ( _au2_t , _au1_n ) ;
_au1_n -> _name_n_sto = 14 ;
}
else 
#line 371 "../../src/print.c"
_type_dcl_print ( _au2_t , _au1_n ) ;
}
else { 
#line 374 "../../src/print.c"
if ((((fct_void == 0 )&& (_au1_n -> _name_n_sto == 0 ))&& (_au0_this -> _name_n_stclass == 31 ))&& (_au1_n -> _expr__O5.__C5_n_table == gtbl ))
#line 377 "../../src/print.c"
{
#line 377 "../../src/print.c"

#line 378 "../../src/print.c"
switch (_au2_t -> _node_base ){ 
#line 379 "../../src/print.c"
case 5 : 
#line 380 "../../src/print.c"
case 29 : 
#line 381 "../../src/print.c"
case 21 : 
#line 382 "../../src/print.c"
case 22 : 
#line 383 "../../src/print.c"
case 15 : 
#line 384 "../../src/print.c"
case
#line 384 "../../src/print.c"
11 : 
#line 385 "../../src/print.c"
case 121 : 
#line 386 "../../src/print.c"
case 125 : 
#line 388 "../../src/print.c"
_au1_n -> _expr__O4.__C4_n_initializer = (_au4_i = zero );
}
}
_type_dcl_print ( _au2_t , _au1_n ) ;
}

#line 394 "../../src/print.c"
if (_au1_n -> _name_n_scope != 136 ){ 
#line 395 "../../src/print.c"
if (_au4_i ){ 
#line 396 "../../src/print.c"
puttok ( (unsigned char )70 ) ;
if (((_au2_t != _au4_i -> _expr__O2.__C2_tp )&& (_au4_i -> _node_base != 86 ))&& (_au4_i -> _node_base != 124 ))
#line 399 "../../src/print.c"
{ 
#line 400 "../../src/print.c"
Ptype _au7_t1 ;

#line 400 "../../src/print.c"
_au7_t1 = _au1_n -> _expr__O2.__C2_tp ;
cmp :
#line 402 "../../src/print.c"
switch (_au7_t1 -> _node_base ){ 
#line 403 "../../src/print.c"
default : 
#line 404 "../../src/print.c"
_expr_print ( _au4_i ) ;
break ;
case 97 : 
#line 407 "../../src/print.c"
_au7_t1 = (((struct basetype *)_au7_t1 ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
goto cmp ;
case 110 : 
#line 410 "../../src/print.c"
if ((((struct vec *)_au7_t1 ))-> _pvtyp_typ -> _node_base == 5 ){ 
#line 411 "../../src/print.c"
_expr_print ( _au4_i ) ;
break ;
}
case 125 : 
#line 415 "../../src/print.c"
case 158 : 
#line 416 "../../src/print.c"
if ((_au4_i -> _expr__O2.__C2_tp == 0 )|| _type_check ( _au1_n -> _expr__O2.__C2_tp , _au4_i -> _expr__O2.__C2_tp , (unsigned
#line 416 "../../src/print.c"
char )0 ) ){ 
#line 417 "../../src/print.c"
putc('(', out_file);
{ bit _au9_oc ;

#line 418 "../../src/print.c"
_au9_oc = Cast ;
Cast = 1 ;
_type_print ( _au2_t ) ;
Cast = _au9_oc ;
putc(')', out_file);
}
}

#line 424 "../../src/print.c"
if (_au4_i )Eprint ( _au4_i ) ;
}
}
else 
#line 428 "../../src/print.c"
_expr_print ( _au4_i ) ;
}
else if (_au1_n -> _name_n_evaluated ){ 
#line 431 "../../src/print.c"
puttok ( (unsigned char )70 ) ;
if (_au1_n -> _expr__O2.__C2_tp -> _node_base != 21 ){ 
#line 433 "../../src/print.c"
fputs ( (char *)"((", out_file ) ;
{ bit _au7_oc ;

#line 434 "../../src/print.c"
_au7_oc = Cast ;
Cast = 1 ;
_type_print ( _au1_n -> _expr__O2.__C2_tp ) ;
Cast = _au7_oc ;
fprintf ( out_file , (char *)")%d)", _au1_n -> _name_n_val ) ;
}
}
else fprintf ( out_file , (char *)"%d", _au1_n -> _name_n_val ) ;
}
}
}
}

#line 447 "../../src/print.c"
switch (_au0_list ){ 
#line 448 "../../src/print.c"
case 72 : 
#line 449 "../../src/print.c"
if (_au2_sm == 0 )puttok ( (unsigned char )72 ) ;
break ;
case 0 : 
#line 452 "../../src/print.c"
if (_au2_sm == 0 )puttok ( (unsigned char )72 ) ;
return ;
case 71 : 
#line 455 "../../src/print.c"
if (_au1_n -> _name_n_list )puttok ( (unsigned char )71 ) ;
break ;
}
}
}
;
char _name_print (_au0_this )
#line 607 "../../src/cfront.h"
struct name *_au0_this ;

#line 465 "../../src/print.c"
{ 
#line 466 "../../src/print.c"
if (_au0_this == 0 )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"0->N::print()", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ,
#line 466 "../../src/print.c"
(struct ea *)ea0 ) ;

#line 468 "../../src/print.c"
if (_au0_this -> _expr__O3.__C3_string == 0 ){ 
#line 469 "../../src/print.c"
if (emode ) putc('?', out_file);
return ;
}

#line 473 "../../src/print.c"
switch (_au0_this -> _node_base ){ 
#line 474 "../../src/print.c"
default : 
#line 475 "../../src/print.c"
{ 
#line 597 "../../src/print.c"
struct ea _au0__V12 ;

#line 597 "../../src/print.c"
struct ea _au0__V13 ;

#line 475 "../../src/print.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"%p->N::print() base=%d", (struct ea *)( ( ((& _au0__V12 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((&
#line 475 "../../src/print.c"
_au0__V12 )))) ) , (struct ea *)( ( ((& _au0__V13 )-> _ea__O1.__C1_i = ((int )_au0_this -> _node_base )), (((& _au0__V13 ))))
#line 475 "../../src/print.c"
) , (struct ea *)ea0 , (struct ea *)ea0 ) ;
case 123 : 
#line 477 "../../src/print.c"
( fputs ( (char *)_au0_this -> _expr__O3.__C3_string , out_file ) , putc(' ', out_file)) ;
return ;
case 85 : 
#line 480 "../../src/print.c"
case 169 : 
#line 481 "../../src/print.c"
break ;
} }

#line 484 "../../src/print.c"
if (emode ){ 
#line 485 "../../src/print.c"
Ptable _au2_tbl ;
char *_au2_cs ;
bit _au2_f ;

#line 486 "../../src/print.c"
_au2_cs = 0 ;
_au2_f = 0 ;
if (_au0_this -> _expr__O2.__C2_tp ){ 
#line 489 "../../src/print.c"
switch (_au0_this -> _expr__O2.__C2_tp -> _node_base ){ 
#line 490 "../../src/print.c"
case 76 : 
#line 491 "../../src/print.c"
case 108 : 
#line 492 "../../src/print.c"
_au2_f = 1 ;
default : 
#line 494 "../../src/print.c"
if (_au2_tbl = _au0_this -> _expr__O5.__C5_n_table ){ 
#line 495 "../../src/print.c"
if (_au2_tbl == gtbl ){ 
#line 496 "../../src/print.c"
if (_au2_f == 0 )fputs ( (char *)"::",
#line 496 "../../src/print.c"
out_file ) ;
}
else { 
#line 499 "../../src/print.c"
if (_au2_tbl -> _table_t_name ){ 
#line 500 "../../src/print.c"
_au2_cs = _au2_tbl -> _table_t_name -> _expr__O3.__C3_string ;
fprintf ( out_file , (char *)"%s::", _au2_cs ) ;
}
}
}

#line 506 "../../src/print.c"
if ((_au0_this -> _name_n_scope == 136 )&& (strcmp ( (char *)_au0_this -> _expr__O3.__C3_string , (char *)"this") == 0 )){ 
#line 508 "../../src/print.c"
Ptype _au5_tt ;
Pname _au5_cn ;

#line 508 "../../src/print.c"
_au5_tt = (((struct ptr *)_au0_this -> _expr__O2.__C2_tp ))-> _pvtyp_typ ;
_au5_cn = (((struct basetype *)_au5_tt ))-> _basetype_b_name ;
fprintf ( out_file , (char *)"%s::", _au5_cn -> _expr__O3.__C3_string ) ;
}

#line 513 "../../src/print.c"
case 6 : 
#line 514 "../../src/print.c"
case 13 : 
#line 516 "../../src/print.c"
break ;
}
switch (_au0_this -> _name_n_oper ){ 
#line 519 "../../src/print.c"
case 97 : 
#line 520 "../../src/print.c"
fputs ( (char *)"operator ", out_file ) ;
_type_dcl_print ( (((struct fct *)_au0_this -> _expr__O2.__C2_tp ))-> _fct_returns , (struct name *)0 ) ;
break ;
case 0 : 
#line 524 "../../src/print.c"
fputs ( (char *)_au0_this -> _expr__O3.__C3_string , out_file ) ;
break ;
case 162 : 
#line 527 "../../src/print.c"
putc('~', out_file);
case 161 : 
#line 529 "../../src/print.c"
if (_au2_cs )
#line 530 "../../src/print.c"
fputs ( (char *)_au2_cs , out_file ) ;
else { 
#line 532 "../../src/print.c"
fputs ( (char *)"constructor", out_file ) ;
_au2_f = 0 ;
}
break ;
case 123 : 
#line 537 "../../src/print.c"
fputs ( (char *)_au0_this -> _expr__O3.__C3_string , out_file ) ;
break ;
default : 
#line 540 "../../src/print.c"
fputs ( (char *)"operator ", out_file ) ;
fputs ( (char *)(keys [_au0_this -> _name_n_oper ]), out_file ) ;
break ;
}
if (_au2_f )fputs ( (char *)"()", out_file ) ;
}
else 
#line 547 "../../src/print.c"
if (_au0_this -> _expr__O3.__C3_string )fputs ( (char *)_au0_this -> _expr__O3.__C3_string , out_file ) ;
return ;
}

#line 551 "../../src/print.c"
if (_au0_this -> _expr__O2.__C2_tp ){ 
#line 552 "../../src/print.c"
Ptable _au2_tbl ;
int _au2_i ;

#line 553 "../../src/print.c"
_au2_i = _au0_this -> _name_n_union ;

#line 555 "../../src/print.c"
switch (_au0_this -> _expr__O2.__C2_tp -> _node_base ){ 
#line 556 "../../src/print.c"
default : 
#line 557 "../../src/print.c"
if (_au2_tbl = _au0_this -> _expr__O5.__C5_n_table ){ 
#line 558 "../../src/print.c"
Pname _au4_tn ;
if (_au2_tbl == gtbl ){ 
#line 560 "../../src/print.c"
if (_au2_i )fprintf ( out_file , (char *)"_O%d.__C%d_", _au2_i , _au2_i ) ;
break ;
}
if (_au4_tn = _au2_tbl -> _table_t_name ){ 
#line 564 "../../src/print.c"
if (_au2_i )
#line 565 "../../src/print.c"
fprintf ( out_file , (char *)"_%s__O%d.__C%d_", _au4_tn -> _expr__O3.__C3_string , _au2_i , _au2_i ) ;
#line 565 "../../src/print.c"
else 
#line 567 "../../src/print.c"
fprintf ( out_file , (char *)"_%s_", _au4_tn -> _expr__O3.__C3_string ) ;
break ;
}
}

#line 572 "../../src/print.c"
switch (_au0_this -> _name_n_stclass ){ 
#line 573 "../../src/print.c"
case 31 : 
#line 574 "../../src/print.c"
case 14 : 
#line 575 "../../src/print.c"
if (_au2_i )
#line 576 "../../src/print.c"
fprintf ( out_file , (char *)"_O%d.__C%d_", _au2_i , _au2_i )
#line 576 "../../src/print.c"
;
else if ((_au0_this -> _name_n_sto == 31 )&& (_au0_this -> _expr__O2.__C2_tp -> _node_base != 108 ))
#line 579 "../../src/print.c"
fprintf ( out_file , (char *)"_st%d_", _au0_this -> _name_lex_level )
#line 579 "../../src/print.c"
;
break ;
default : 
#line 582 "../../src/print.c"
if (_au2_i )
#line 584 "../../src/print.c"
fprintf ( out_file , (char *)"_au%d__O%d.__C%d_", _au0_this -> _name_lex_level - 1 , _au2_i , _au2_i ) ;
else 
#line 587 "../../src/print.c"
fprintf ( out_file , (char *)"_au%d_", _au0_this -> _name_lex_level ) ;
}
break ;
case 6 : 
#line 591 "../../src/print.c"
case 13 : 
#line 592 "../../src/print.c"
break ;
}
}

#line 596 "../../src/print.c"
if (_au0_this -> _expr__O3.__C3_string )( fputs ( (char *)_au0_this -> _expr__O3.__C3_string , out_file ) , putc(' ', out_file)) ;
}
;

#line 600 "../../src/print.c"
char _type_print (_au0_this )
#line 263 "../../src/cfront.h"
struct type *_au0_this ;

#line 601 "../../src/print.c"
{ 
#line 602 "../../src/print.c"
switch (_au0_this -> _node_base ){ 
#line 603 "../../src/print.c"
case 125 : 
#line 604 "../../src/print.c"
case 158 : 
#line 605 "../../src/print.c"
_type_dcl_print ( (struct type *)(((struct ptr *)_au0_this )), (struct name *)0 )
#line 605 "../../src/print.c"
;
break ;
case 108 : 
#line 608 "../../src/print.c"
_fct_dcl_print ( ((struct fct *)_au0_this )) ;
break ;
case 110 : 
#line 611 "../../src/print.c"
_type_dcl_print ( (struct type *)(((struct vec *)_au0_this )), (struct name *)0 ) ;
break ;
case 6 : 
#line 614 "../../src/print.c"
case 13 : 
#line 615 "../../src/print.c"
if (emode )
#line 616 "../../src/print.c"
fprintf ( out_file , (char *)"%s", (_au0_this -> _node_base == 6 )? "class": "enum")
#line 616 "../../src/print.c"
;
else 
#line 618 "../../src/print.c"
{ 
#line 630 "../../src/print.c"
struct ea _au0__V14 ;

#line 630 "../../src/print.c"
struct ea _au0__V15 ;

#line 618 "../../src/print.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"%p->T::print(%k)", (struct ea *)( ( ((& _au0__V14 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((&
#line 618 "../../src/print.c"
_au0__V14 )))) ) , (struct ea *)( ( ((& _au0__V15 )-> _ea__O1.__C1_i = ((int )_au0_this -> _node_base )), (((& _au0__V15 ))))
#line 618 "../../src/print.c"
) , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} break ;
case 97 : 
#line 621 "../../src/print.c"
if (Cast ){ 
#line 622 "../../src/print.c"
if ((((struct basetype *)_au0_this ))-> _basetype_b_name -> _expr__O2.__C2_tp -> _node_base == 125 )
#line 623 "../../src/print.c"
{ _name_print ( (((struct basetype *)_au0_this ))->
#line 623 "../../src/print.c"
_basetype_b_name ) ;

#line 623 "../../src/print.c"
break ;
}

#line 624 "../../src/print.c"
_type_print ( (((struct basetype *)_au0_this ))-> _basetype_b_name -> _expr__O2.__C2_tp ) ;
break ;
}
default : 
#line 628 "../../src/print.c"
_basetype_dcl_print ( ((struct basetype *)_au0_this )) ;
}
}
;
char *_type_signature (_au0_this , _au0_p )
#line 263 "../../src/cfront.h"
struct type *_au0_this ;

#line 632 "../../src/print.c"
register char *_au0_p ;

#line 637 "../../src/print.c"
{ 
#line 640 "../../src/print.c"
Ptype _au1_t ;
int _au1_pp ;

#line 640 "../../src/print.c"
_au1_t = _au0_this ;
_au1_pp = 0 ;

#line 643 "../../src/print.c"
xx :
#line 645 "../../src/print.c"
switch (_au1_t -> _node_base ){ 
#line 646 "../../src/print.c"
case 97 : _au1_t = (((struct basetype *)_au1_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 646 "../../src/print.c"
goto xx ;
case 125 : (*(_au0_p ++ ))= 'P' ;

#line 647 "../../src/print.c"
_au1_t = (((struct ptr *)_au1_t ))-> _pvtyp_typ ;

#line 647 "../../src/print.c"
_au1_pp = 1 ;

#line 647 "../../src/print.c"
goto xx ;
case 158 : (*(_au0_p ++ ))= 'R' ;

#line 648 "../../src/print.c"
_au1_t = (((struct ptr *)_au1_t ))-> _pvtyp_typ ;

#line 648 "../../src/print.c"
_au1_pp = 1 ;

#line 648 "../../src/print.c"
goto xx ;
case 110 : (*(_au0_p ++ ))= 'V' ;

#line 649 "../../src/print.c"
_au1_t = (((struct vec *)_au1_t ))-> _pvtyp_typ ;

#line 649 "../../src/print.c"
_au1_pp = 1 ;

#line 649 "../../src/print.c"
goto xx ;
case 108 : 
#line 651 "../../src/print.c"
{ Pfct _au3_f ;

#line 651 "../../src/print.c"
_au3_f = (((struct fct *)_au1_t ));
(*(_au0_p ++ ))= 'F' ;

#line 658 "../../src/print.c"
if (_au1_pp ){ 
#line 659 "../../src/print.c"
(*(_au0_p ++ ))= 'T' ;
_au0_p = _type_signature ( _au3_f -> _fct_returns , _au0_p ) ;
(*(_au0_p ++ ))= '_' ;
}
{ Pname _au3_n ;

#line 663 "../../src/print.c"
_au3_n = _au3_f -> _fct_argtype ;

#line 663 "../../src/print.c"
for(;_au3_n ;_au3_n = _au3_n -> _name_n_list ) { 
#line 664 "../../src/print.c"
if (_au3_n -> _name_n_xref )(*(_au0_p ++ ))= 'X' ;
_au0_p = _type_signature ( _au3_n -> _expr__O2.__C2_tp , _au0_p ) ;
(*(_au0_p ++ ))= '_' ;
}
(*(_au0_p ++ ))= '_' ;
if (_au3_f -> _fct_nargs_known == 155 )(*(_au0_p ++ ))= 'E' ;
(*_au0_p )= 0 ;
return _au0_p ;
}
}
}
if ((((struct basetype *)_au1_t ))-> _basetype_b_unsigned )(*(_au0_p ++ ))= 'U' ;

#line 677 "../../src/print.c"
switch (_au1_t -> _node_base ){ 
#line 678 "../../src/print.c"
case 141 : (*(_au0_p ++ ))= 'A' ;

#line 678 "../../src/print.c"
break ;
case 138 : (*(_au0_p ++ ))= 'Z' ;

#line 679 "../../src/print.c"
break ;
case 38 : (*(_au0_p ++ ))= 'V' ;

#line 680 "../../src/print.c"
break ;
case 5 : (*(_au0_p ++ ))= (_au1_pp ? 'C' : 'I' );

#line 681 "../../src/print.c"
break ;
case 29 : (*(_au0_p ++ ))= (_au1_pp ? 'S' : 'I' );

#line 682 "../../src/print.c"
break ;
case 121 : 
#line 684 "../../src/print.c"
case 21 : (*(_au0_p ++ ))= 'I' ;

#line 684 "../../src/print.c"
break ;
case 22 : (*(_au0_p ++ ))= 'L' ;

#line 685 "../../src/print.c"
break ;
case 15 : (*(_au0_p ++ ))= 'F' ;

#line 686 "../../src/print.c"
break ;
case 11 : (*(_au0_p ++ ))= 'D' ;

#line 687 "../../src/print.c"
break ;
case 119 : (*(_au0_p ++ ))= 'C' ;
strcpy ( _au0_p , (char *)(((struct basetype *)_au1_t ))-> _basetype_b_name -> _expr__O3.__C3_string ) ;
while (*(_au0_p ++ ));
(*(_au0_p - 1 ))= '_' ;
break ;
case 114 : 
#line 694 "../../src/print.c"
default : 
#line 695 "../../src/print.c"
{ 
#line 700 "../../src/print.c"
struct ea _au0__V16 ;

#line 695 "../../src/print.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"signature of %k", (struct ea *)( ( ((& _au0__V16 )-> _ea__O1.__C1_i = ((int )_au1_t -> _node_base )),
#line 695 "../../src/print.c"
(((& _au0__V16 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }

#line 698 "../../src/print.c"
(*_au0_p )= 0 ;
return _au0_p ;
}
;
char _basetype_dcl_print (_au0_this )
#line 362 "../../src/cfront.h"
struct basetype *_au0_this ;

#line 703 "../../src/print.c"
{ 
#line 704 "../../src/print.c"
Pname _au1_nn ;
Pclass _au1_cl ;

#line 707 "../../src/print.c"
if (emode ){ 
#line 708 "../../src/print.c"
if (_au0_this -> _basetype_b_virtual )puttok ( (unsigned char )77 ) ;
if (_au0_this -> _basetype_b_inline )puttok ( (unsigned char )75 ) ;
if (_au0_this -> _basetype_b_const )puttok ( (unsigned char )26 ) ;
}
if (_au0_this -> _basetype_b_unsigned )puttok ( (unsigned char )37 ) ;

#line 714 "../../src/print.c"
switch (_au0_this -> _node_base ){ 
#line 715 "../../src/print.c"
case 141 : 
#line 716 "../../src/print.c"
fputs ( (char *)"any ", out_file ) ;
break ;

#line 719 "../../src/print.c"
case 138 : 
#line 720 "../../src/print.c"
fputs ( (char *)"zero ", out_file ) ;
break ;

#line 723 "../../src/print.c"
case 38 : 
#line 724 "../../src/print.c"
if (emode == 0 ){ 
#line 725 "../../src/print.c"
puttok ( (unsigned char )5 ) ;
break ;
}
case 5 : 
#line 729 "../../src/print.c"
case 29 : 
#line 730 "../../src/print.c"
case 21 : 
#line 731 "../../src/print.c"
case 22 : 
#line 732 "../../src/print.c"
case 15 : 
#line 733 "../../src/print.c"
case 11 : 
#line 734 "../../src/print.c"
puttok ( _au0_this ->
#line 734 "../../src/print.c"
_node_base ) ;
break ;

#line 737 "../../src/print.c"
case 121 : 
#line 738 "../../src/print.c"
_au1_nn = _au0_this -> _basetype_b_name ;
eob :
#line 740 "../../src/print.c"
if (emode == 0 )
#line 741 "../../src/print.c"
puttok ( (unsigned char )21 ) ;
else { 
#line 743 "../../src/print.c"
puttok ( (unsigned char )13 ) ;
_name_print ( _au1_nn ) ;
}
break ;

#line 748 "../../src/print.c"
case 119 : 
#line 749 "../../src/print.c"
_au1_nn = _au0_this -> _basetype_b_name ;
cob :
#line 751 "../../src/print.c"
_au1_cl = (((struct classdef *)_au1_nn -> _expr__O2.__C2_tp ));
switch (_au1_cl -> _classdef_csu ){ 
#line 753 "../../src/print.c"
case 36 : 
#line 754 "../../src/print.c"
case 167 : puttok ( (unsigned char )36 ) ;

#line 754 "../../src/print.c"
break ;
default : puttok ( (unsigned char )32 ) ;
}
( fputs ( (char *)_au1_cl -> _classdef_string , out_file ) , putc(' ', out_file)) ;
break ;

#line 760 "../../src/print.c"
case 97 : 
#line 761 "../../src/print.c"
if (emode == 0 ){ 
#line 762 "../../src/print.c"
switch (_au0_this -> _basetype_b_name -> _expr__O2.__C2_tp -> _node_base ){ 
#line 763 "../../src/print.c"
case 119 : 
#line 764 "../../src/print.c"
_au1_nn = (((struct
#line 764 "../../src/print.c"
basetype *)_au0_this -> _basetype_b_name -> _expr__O2.__C2_tp ))-> _basetype_b_name ;
goto cob ;
case 121 : 
#line 767 "../../src/print.c"
_au1_nn = (((struct basetype *)_au0_this -> _basetype_b_name -> _expr__O2.__C2_tp ))-> _basetype_b_name ;
goto eob ;
}
}
_name_print ( _au0_this -> _basetype_b_name ) ;
break ;

#line 774 "../../src/print.c"
default : 
#line 775 "../../src/print.c"
if (emode ){ 
#line 776 "../../src/print.c"
if (((0 < _au0_this -> _node_base )&& (_au0_this -> _node_base <= 255 ))&& (keys [_au0_this -> _node_base ]))
#line 777 "../../src/print.c"
fprintf ( out_file ,
#line 777 "../../src/print.c"
(char *)" %s", keys [_au0_this -> _node_base ]) ;
else 
#line 779 "../../src/print.c"
putc('?', out_file);
}
else 
#line 782 "../../src/print.c"
{ 
#line 784 "../../src/print.c"
struct ea _au0__V17 ;

#line 784 "../../src/print.c"
struct ea _au0__V18 ;

#line 782 "../../src/print.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"%p->BT::dcl_print(%d)", (struct ea *)( ( ((& _au0__V17 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((&
#line 782 "../../src/print.c"
_au0__V17 )))) ) , (struct ea *)( ( ((& _au0__V18 )-> _ea__O1.__C1_i = ((int )_au0_this -> _node_base )), (((& _au0__V18 ))))
#line 782 "../../src/print.c"
) , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}
;
char _type_dcl_print (_au0_this , _au0_n )
#line 263 "../../src/cfront.h"
struct type *_au0_this ;

#line 786 "../../src/print.c"
Pname _au0_n ;

#line 790 "../../src/print.c"
{ 
#line 791 "../../src/print.c"
Ptype _au1_t ;
Pfct _au1_f ;
Pvec _au1_v ;
Pptr _au1_p ;
TOK _au1_pre ;

#line 791 "../../src/print.c"
_au1_t = _au0_this ;

#line 795 "../../src/print.c"
_au1_pre = 0 ;

#line 797 "../../src/print.c"
if (_au1_t == 0 )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"0->dcl_print()", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 797 "../../src/print.c"
ea *)ea0 ) ;
if (_au0_n && (_au0_n -> _expr__O2.__C2_tp != _au1_t )){ 
#line 897 "../../src/print.c"
struct ea _au0__V19 ;

#line 897 "../../src/print.c"
struct ea _au0__V20 ;

#line 798 "../../src/print.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"not %n'sT (%p)", (struct ea *)( ( ((& _au0__V19 )-> _ea__O1.__C1_p = ((char *)_au0_n )), (((&
#line 798 "../../src/print.c"
_au0__V19 )))) ) , (struct ea *)( ( ((& _au0__V20 )-> _ea__O1.__C1_p = ((char *)_au1_t )), (((& _au0__V20 )))) )
#line 798 "../../src/print.c"
, (struct ea *)ea0 , (struct ea *)ea0 ) ;
} 
#line 800 "../../src/print.c"
if (_au0_this -> _node_base == 76 ){ 
#line 801 "../../src/print.c"
if (emode ){ 
#line 802 "../../src/print.c"
puttok ( (unsigned char )76 ) ;
return ;
}
{ Pgen _au2_g ;
Plist _au2_gl ;

#line 805 "../../src/print.c"
_au2_g = (((struct gen *)_au0_this ));

#line 807 "../../src/print.c"
fprintf ( out_file , (char *)"\t/* overload %s: */\n", _au2_g -> _gen_string ) ;
for(_au2_gl = _au2_g -> _gen_fct_list ;_au2_gl ;_au2_gl = _au2_gl -> _name_list_l ) { 
#line 809 "../../src/print.c"
Pname _au3_nn ;

#line 809 "../../src/print.c"
_au3_nn = _au2_gl -> _name_list_f ;
_type_dcl_print ( _au3_nn -> _expr__O2.__C2_tp , _au3_nn ) ;
if (_au2_gl -> _name_list_l )puttok ( (unsigned char )72 ) ;
}
return ;
}
}
tbuf = (tbufvec [freetbuf ]);
if (tbuf == 0 ){ 
#line 818 "../../src/print.c"
if (freetbuf == 9)errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"AT nesting overflow", (struct ea *)ea0 , (struct ea *)ea0 ,
#line 818 "../../src/print.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
(tbufvec [freetbuf ])= (tbuf = (((struct dcl_buf *)_new ( (long )(sizeof (struct dcl_buf ))) )));
}
freetbuf ++ ;
( (tbuf -> _dcl_buf_b = 0 ), ( (tbuf -> _dcl_buf_n = _au0_n ), (tbuf -> _dcl_buf_li = (tbuf -> _dcl_buf_ri = 0 ))) )
#line 822 "../../src/print.c"
;
if (_au0_n && _au0_n -> _name_n_xref )( ((tbuf -> _dcl_buf_left [++ tbuf -> _dcl_buf_li ])= ((unsigned char )125 ))) ;

#line 825 "../../src/print.c"
while (_au1_t ){ 
#line 826 "../../src/print.c"
TOK _au2_k ;

#line 828 "../../src/print.c"
switch (_au1_t -> _node_base ){ 
#line 829 "../../src/print.c"
case 125 : 
#line 830 "../../src/print.c"
_au1_p = (((struct ptr *)_au1_t ));
_au2_k = (_au1_p -> _ptr_rdo ? 163 : 125 );
goto ppp ;
case 158 : 
#line 834 "../../src/print.c"
_au1_p = (((struct ptr *)_au1_t ));
_au2_k = (_au1_p -> _ptr_rdo ? 164 : 158 );
ppp :
#line 837 "../../src/print.c"
( ((tbuf -> _dcl_buf_left [++ tbuf -> _dcl_buf_li ])= _au2_k )) ;
if (emode && _au1_p -> _ptr_memof )( ((tbuf -> _dcl_buf_left [++ tbuf -> _dcl_buf_li ])= 173 ), ((tbuf -> _dcl_buf_lnode [tbuf -> _dcl_buf_li ])= _au1_p -> _ptr_memof ))
#line 838 "../../src/print.c"
;
_au1_pre = 125 ;
_au1_t = _au1_p -> _pvtyp_typ ;
break ;
case 110 : 
#line 843 "../../src/print.c"
_au1_v = (((struct vec *)_au1_t ));
if (Cast ){ 
#line 845 "../../src/print.c"
( ((tbuf -> _dcl_buf_left [++ tbuf -> _dcl_buf_li ])= ((unsigned char )125 ))) ;
_au1_pre = 125 ;
}
else { 
#line 849 "../../src/print.c"
if (_au1_pre == 125 )( ( ((tbuf -> _dcl_buf_left [++ tbuf -> _dcl_buf_li ])= ((unsigned char )40 ))) ,
#line 849 "../../src/print.c"
( ((tbuf -> _dcl_buf_right [++ tbuf -> _dcl_buf_ri ])= ((unsigned char )41 )), ((tbuf -> _dcl_buf_rnode [tbuf -> _dcl_buf_ri ])= ((struct node *)0 ))) )
#line 849 "../../src/print.c"
;
( ((tbuf -> _dcl_buf_right [++ tbuf -> _dcl_buf_ri ])= ((unsigned char )110 )), ((tbuf -> _dcl_buf_rnode [tbuf -> _dcl_buf_ri ])= ((struct node *)_au1_v ))) ;
#line 850 "../../src/print.c"

#line 851 "../../src/print.c"
_au1_pre = 110 ;
}
_au1_t = _au1_v -> _pvtyp_typ ;
break ;
case 108 : 
#line 856 "../../src/print.c"
_au1_f = (((struct fct *)_au1_t ));
if (_au1_pre == 125 )
#line 858 "../../src/print.c"
( ( ((tbuf -> _dcl_buf_left [++ tbuf -> _dcl_buf_li ])= ((unsigned char )40 ))) , ( ((tbuf ->
#line 858 "../../src/print.c"
_dcl_buf_right [++ tbuf -> _dcl_buf_ri ])= ((unsigned char )41 )), ((tbuf -> _dcl_buf_rnode [tbuf -> _dcl_buf_ri ])= ((struct node *)0 ))) ) ;
else if (emode && _au1_f -> _fct_memof )
#line 860 "../../src/print.c"
( ((tbuf -> _dcl_buf_left [++ tbuf -> _dcl_buf_li ])= 173 ), ((tbuf -> _dcl_buf_lnode [tbuf -> _dcl_buf_li ])= _au1_f ->
#line 860 "../../src/print.c"
_fct_memof )) ;
( ((tbuf -> _dcl_buf_right [++ tbuf -> _dcl_buf_ri ])= ((unsigned char )108 )), ((tbuf -> _dcl_buf_rnode [tbuf -> _dcl_buf_ri ])= ((struct node *)_au1_f ))) ;
#line 861 "../../src/print.c"

#line 862 "../../src/print.c"
_au1_pre = 108 ;

#line 864 "../../src/print.c"
if (((_au1_f -> _fct_f_inline && _au1_f -> _fct_returns )&& (_au1_f -> _fct_returns -> _node_base == 38 ))&& (_au1_f -> _fct_s_returns && (_au1_f -> _fct_s_returns -> _node_base !=
#line 864 "../../src/print.c"
125 )))
#line 869 "../../src/print.c"
_au1_t = _au1_f -> _fct_returns ;
else 
#line 871 "../../src/print.c"
_au1_t = (_au1_f -> _fct_s_returns ? _au1_f -> _fct_s_returns : _au1_f -> _fct_returns );
break ;
case 114 : 
#line 874 "../../src/print.c"
( ((tbuf -> _dcl_buf_right [++ tbuf -> _dcl_buf_ri ])= ((unsigned char )114 )), ((tbuf -> _dcl_buf_rnode [tbuf -> _dcl_buf_ri ])= ((struct
#line 874 "../../src/print.c"
node *)_au1_t ))) ;
( (tbuf -> _dcl_buf_b = (((struct basetype *)(((struct basetype *)_au1_t ))-> _basetype_b_fieldtype )))) ;
_au1_t = 0 ;
break ;
case 6 : 
#line 879 "../../src/print.c"
case 13 : 
#line 880 "../../src/print.c"
{ 
#line 897 "../../src/print.c"
struct ea _au0__V21 ;

#line 880 "../../src/print.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"unexpected%k asBT", (struct ea *)( ( ((& _au0__V21 )-> _ea__O1.__C1_i = ((int )_au1_t -> _node_base )),
#line 880 "../../src/print.c"
(((& _au0__V21 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
case 0 : 
#line 882 "../../src/print.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"noBT(B=0)", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 882 "../../src/print.c"
ea *)ea0 ) ;
case 97 : 
#line 884 "../../src/print.c"
if (Cast ){ 
#line 885 "../../src/print.c"
_au1_t = (((struct basetype *)_au1_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
break ;
}
default : 
#line 889 "../../src/print.c"
( (tbuf -> _dcl_buf_b = (((struct basetype *)_au1_t )))) ;
_au1_t = 0 ;
break ;
} }
}

#line 895 "../../src/print.c"
_dcl_buf_put ( tbuf ) ;
freetbuf -- ;
}
;
char _fct_dcl_print (_au0_this )
#line 400 "../../src/cfront.h"
struct fct *_au0_this ;

#line 900 "../../src/print.c"
{ 
#line 901 "../../src/print.c"
Pname _au1_nn ;

#line 917 "../../src/print.c"
Pname _au1_at ;

#line 903 "../../src/print.c"
if (emode ){ 
#line 904 "../../src/print.c"
putc('(', out_file);
for(_au1_nn = _au0_this -> _fct_argtype ;_au1_nn ;) { 
#line 906 "../../src/print.c"
_type_dcl_print ( _au1_nn -> _expr__O2.__C2_tp , (struct name *)0 ) ;
if (_au1_nn = _au1_nn -> _name_n_list )puttok ( (unsigned char )71 ) ;
else 
#line 907 "../../src/print.c"
break ;
}
switch (_au0_this -> _fct_nargs_known ){ 
#line 910 "../../src/print.c"
case 0 : 
#line 911 "../../src/print.c"
case 155 : puttok ( (unsigned char )155 ) ;

#line 911 "../../src/print.c"
break ;
}
putc(')', out_file);
return ;
}

#line 917 "../../src/print.c"
_au1_at = (_au0_this -> _fct_f_this ? _au0_this -> _fct_f_this : (_au0_this -> _fct_f_result ? _au0_this -> _fct_f_result : _au0_this -> _fct_argtype ));
putc('(', out_file);
if (_au0_this -> _fct_body && (Cast == 0 )){ 
#line 921 "../../src/print.c"
for(_au1_nn = _au1_at ;_au1_nn ;) { 
#line 922 "../../src/print.c"
_name_print ( _au1_nn ) ;
if (_au1_nn = _au1_nn -> _name_n_list )puttok ( (unsigned char )71 ) ;
else 
#line 923 "../../src/print.c"
break ;
}
putc(')', out_file);

#line 927 "../../src/print.c"
if (_au1_at )_name_dcl_print ( _au1_at , (unsigned char )72 ) ;

#line 929 "../../src/print.c"
if (MAIN ){ 
#line 930 "../../src/print.c"
fputs ( (char *)"{ _main(); ", out_file ) ;
_stmt_print ( (struct stmt *)_au0_this -> _fct_body ) ;

#line 936 "../../src/print.c"
putc('}', out_file);
}
else 
#line 939 "../../src/print.c"
_stmt_print ( (struct stmt *)_au0_this -> _fct_body ) ;
}
else 
#line 942 "../../src/print.c"
putc(')', out_file);
}
;
char _classdef_print_members (_au0_this )
#line 319 "../../src/cfront.h"
struct classdef *_au0_this ;

#line 946 "../../src/print.c"
{ 
#line 947 "../../src/print.c"
int _au1_i ;

#line 954 "../../src/print.c"
Pname _au1_nn ;

#line 949 "../../src/print.c"
if (_au0_this -> _classdef_clbase ){ 
#line 950 "../../src/print.c"
Pclass _au2_bcl ;

#line 950 "../../src/print.c"
_au2_bcl = (((struct classdef *)_au0_this -> _classdef_clbase -> _expr__O2.__C2_tp ));
_classdef_print_members ( _au2_bcl ) ;
}

#line 954 "../../src/print.c"
for(_au1_nn = _table_get_mem ( _au0_this -> _classdef_memtbl , _au1_i = 1 ) ;_au1_nn ;_au1_nn = _table_get_mem ( _au0_this -> _classdef_memtbl , ++ _au1_i ) ) {
#line 954 "../../src/print.c"

#line 955 "../../src/print.c"
if (((((((_au1_nn -> _node_base == 85 )&& (_au1_nn -> _name_n_union == 0 ))&& (_au1_nn -> _expr__O2.__C2_tp -> _node_base != 108 ))&& (_au1_nn -> _expr__O2.__C2_tp -> _node_base !=
#line 955 "../../src/print.c"
76 ))&& (_au1_nn -> _expr__O2.__C2_tp -> _node_base != 6 ))&& (_au1_nn -> _expr__O2.__C2_tp -> _node_base != 13 ))&& (_au1_nn -> _name_n_stclass != 31 ))
#line 961 "../../src/print.c"
{ 
#line 962 "../../src/print.c"
Pexpr _au3_i ;

#line 962 "../../src/print.c"
_au3_i = _au1_nn -> _expr__O4.__C4_n_initializer ;
_au1_nn -> _expr__O4.__C4_n_initializer = 0 ;
_name_dcl_print ( _au1_nn , (unsigned char )0 ) ;
_au1_nn -> _expr__O4.__C4_n_initializer = _au3_i ;
}
}
}
;
char _classdef_dcl_print (_au0_this , _au0__A22 )
#line 319 "../../src/cfront.h"
struct classdef *_au0_this ;

#line 970 "../../src/print.c"
Pname _au0__A22 ;
{ 
#line 972 "../../src/print.c"
Plist _au1_l ;
TOK _au1_c ;

#line 975 "../../src/print.c"
int _au1_i ;

#line 978 "../../src/print.c"
Pname _au1_nn ;

#line 992 "../../src/print.c"
int _au1_sm ;
int _au1_sz ;

#line 973 "../../src/print.c"
_au1_c = ((_au0_this -> _classdef_csu == 6 )? (((unsigned int )32 )): (((unsigned int )_au0_this -> _classdef_csu )));

#line 978 "../../src/print.c"
for(_au1_nn = _table_get_mem ( _au0_this -> _classdef_memtbl , _au1_i = 1 ) ;_au1_nn ;_au1_nn = _table_get_mem ( _au0_this -> _classdef_memtbl , ++ _au1_i ) ) {
#line 978 "../../src/print.c"

#line 979 "../../src/print.c"
if ((_au1_nn -> _node_base == 85 )&& (_au1_nn -> _name_n_union == 0 )){ 
#line 980 "../../src/print.c"
if (_au1_nn -> _expr__O2.__C2_tp -> _node_base == 6 )_classdef_dcl_print ( ((struct classdef *)_au1_nn ->
#line 980 "../../src/print.c"
_expr__O2.__C2_tp ), _au1_nn ) ;
}
else if ((_au1_nn -> _node_base == 123 )&& ((((struct basetype *)_au1_nn -> _expr__O2.__C2_tp ))-> _node_base != 119 ))
#line 983 "../../src/print.c"
_name_dcl_print ( _au1_nn , (unsigned char )0 )
#line 983 "../../src/print.c"
;
}

#line 986 "../../src/print.c"
puttok ( _au1_c ) ;
( fputs ( (char *)_au0_this -> _classdef_string , out_file ) , putc(' ', out_file)) ;

#line 989 "../../src/print.c"
if (_au0_this -> _classdef_c_body == 0 )return ;
_au0_this -> _classdef_c_body = 0 ;

#line 992 "../../src/print.c"
_au1_sm = 0 ;

#line 992 "../../src/print.c"
_au1_sz = _type_tsizeof ( (struct type *)_au0_this ) ;

#line 995 "../../src/print.c"
fprintf ( out_file , (char *)"{\t/* sizeof %s == %d */\n", _au0_this -> _classdef_string , _au0_this -> _classdef_obj_size ) ;

#line 997 "../../src/print.c"
if (_au0_this -> _classdef_real_size )
#line 998 "../../src/print.c"
_classdef_print_members ( _au0_this ) ;
else 
#line 1000 "../../src/print.c"
fputs ( (char *)"char _dummy; ", out_file ) ;
fputs ( (char *)"};\n", out_file ) ;

#line 1003 "../../src/print.c"
if (_au0_this -> _classdef_virt_count ){ 
#line 1005 "../../src/print.c"
for(_au1_nn = _table_get_mem ( _au0_this -> _classdef_memtbl , _au1_i = 1 ) ;_au1_nn ;_au1_nn = _table_get_mem ( _au0_this -> _classdef_memtbl , ++
#line 1005 "../../src/print.c"
_au1_i ) ) { 
#line 1006 "../../src/print.c"
if ((_au1_nn -> _node_base == 85 )&& (_au1_nn -> _name_n_union == 0 )){ 
#line 1007 "../../src/print.c"
Ptype _au4_t ;

#line 1007 "../../src/print.c"
_au4_t = _au1_nn -> _expr__O2.__C2_tp ;
switch (_au4_t -> _node_base ){ 
#line 1009 "../../src/print.c"
case 108 : 
#line 1010 "../../src/print.c"
{ Pfct _au6_f ;

#line 1010 "../../src/print.c"
_au6_f = (((struct fct *)_au4_t ));
if (_au6_f -> _fct_f_virtual == 0 )break ;
if (_au6_f -> _fct_f_inline && (vtbl_opt == -1))puttok ( (unsigned char )31 ) ;
_type_print ( _au6_f -> _fct_returns ) ;
_name_print ( _au1_nn ) ;
fputs ( (char *)"()", out_file ) ;
puttok ( (unsigned char )72 ) ;
break ;
}
case 76 : 
#line 1020 "../../src/print.c"
{ Pgen _au6_g ;
Plist _au6_gl ;

#line 1020 "../../src/print.c"
_au6_g = (((struct gen *)_au4_t ));

#line 1022 "../../src/print.c"
for(_au6_gl = _au6_g -> _gen_fct_list ;_au6_gl ;_au6_gl = _au6_gl -> _name_list_l ) { 
#line 1023 "../../src/print.c"
Pfct _au7_f ;

#line 1023 "../../src/print.c"
_au7_f = (((struct fct *)_au6_gl -> _name_list_f -> _expr__O2.__C2_tp ));
if (_au7_f -> _fct_f_virtual == 0 )continue ;
if (_au7_f -> _fct_f_inline )puttok ( (unsigned char )31 ) ;
_type_print ( _au7_f -> _fct_returns ) ;
_name_print ( _au6_gl -> _name_list_f ) ;
fputs ( (char *)"()", out_file ) ;
puttok ( (unsigned char )72 ) ;
}
}
}
}
}

#line 1036 "../../src/print.c"
switch (vtbl_opt ){ 
#line 1037 "../../src/print.c"
case -1: 
#line 1038 "../../src/print.c"
fputs ( (char *)"static ", out_file ) ;
case 1 : 
#line 1040 "../../src/print.c"
fprintf ( out_file , (char *)"int (*%s__vtbl[])() = {", _au0_this -> _classdef_string ) ;
for(_au1_i = 0 ;_au1_i < _au0_this -> _classdef_virt_count ;_au1_i ++ ) { 
#line 1042 "../../src/print.c"
fputs ( (char *)"\n(int(*)()) ", out_file ) ;
_name_print ( _au0_this -> _classdef_virt_init [_au1_i ]) ;
puttok ( (unsigned char )71 ) ;
}
fputs ( (char *)"0}", out_file ) ;
puttok ( (unsigned char )72 ) ;
break ;
case 0 : 
#line 1050 "../../src/print.c"
fprintf ( out_file , (char *)"extern int (*%s__vtbl[])();", _au0_this -> _classdef_string ) ;
break ;
}
}

#line 1055 "../../src/print.c"
for(_au1_nn = _table_get_mem ( _au0_this -> _classdef_memtbl , _au1_i = 1 ) ;_au1_nn ;_au1_nn = _table_get_mem ( _au0_this -> _classdef_memtbl , ++ _au1_i ) ) {
#line 1055 "../../src/print.c"

#line 1056 "../../src/print.c"
if ((_au1_nn -> _node_base == 85 )&& (_au1_nn -> _name_n_union == 0 )){ 
#line 1057 "../../src/print.c"
Ptype _au3_t ;

#line 1057 "../../src/print.c"
_au3_t = _au1_nn -> _expr__O2.__C2_tp ;
switch (_au3_t -> _node_base ){ 
#line 1059 "../../src/print.c"
case 108 : 
#line 1060 "../../src/print.c"
case 76 : 
#line 1061 "../../src/print.c"
break ;
default : 
#line 1063 "../../src/print.c"
if (_au1_nn -> _name_n_stclass == 31 ){ 
#line 1064 "../../src/print.c"
TOK _au5_b ;

#line 1064 "../../src/print.c"
_au5_b = _au1_nn -> _name_n_sto ;
_au1_nn -> _name_n_sto = 0 ;
if (_type_tconst ( _au1_nn -> _expr__O2.__C2_tp ) ){ 
#line 1067 "../../src/print.c"
if (_au1_nn -> _name_n_assigned_to )
#line 1068 "../../src/print.c"
_au1_nn -> _name_n_sto = 31 ;
else { 
#line 1104 "../../src/print.c"
struct ea _au0__V23 ;

#line 1069 "../../src/print.c"
error ( (char *)"uninitialized const%n", (struct ea *)( ( ((& _au0__V23 )-> _ea__O1.__C1_p = ((char *)_au1_nn )), (((& _au0__V23 )))) )
#line 1069 "../../src/print.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }

#line 1070 "../../src/print.c"
_name_dcl_print ( _au1_nn , (unsigned char )0 ) ;
_au1_nn -> _name_n_sto = _au5_b ;
}
}
}
}

#line 1077 "../../src/print.c"
for(_au1_nn = _table_get_mem ( _au0_this -> _classdef_memtbl , _au1_i = 1 ) ;_au1_nn ;_au1_nn = _table_get_mem ( _au0_this -> _classdef_memtbl , ++ _au1_i ) ) {
#line 1077 "../../src/print.c"

#line 1078 "../../src/print.c"
if ((_au1_nn -> _node_base == 85 )&& (_au1_nn -> _name_n_union == 0 )){ 
#line 1079 "../../src/print.c"
Pfct _au3_f ;

#line 1079 "../../src/print.c"
_au3_f = (((struct fct *)_au1_nn -> _expr__O2.__C2_tp ));
switch (_au3_f -> _node_base ){ 
#line 1081 "../../src/print.c"
case 108 : 
#line 1083 "../../src/print.c"
if (_au3_f -> _fct_f_virtual || _au3_f -> _fct_f_inline )break ;
case 76 : 
#line 1085 "../../src/print.c"
_name_dcl_print ( _au1_nn , (unsigned char )0 ) ;
}
}
}

#line 1090 "../../src/print.c"
for(_au1_l = _au0_this -> _classdef_friend_list ;_au1_l ;_au1_l = _au1_l -> _name_list_l ) { 
#line 1091 "../../src/print.c"
Pname _au2_nn ;

#line 1091 "../../src/print.c"
_au2_nn = _au1_l -> _name_list_f ;
switch (_au2_nn -> _expr__O2.__C2_tp -> _node_base ){ 
#line 1093 "../../src/print.c"
case 108 : 
#line 1094 "../../src/print.c"
Cast = 1 ;
_name_dcl_print ( _au2_nn , (unsigned char )0 ) ;
Cast = 0 ;
break ;
case 76 : 
#line 1099 "../../src/print.c"
_au1_l -> _name_list_f = (_au2_nn = (((struct gen *)_au2_nn -> _expr__O2.__C2_tp ))-> _gen_fct_list -> _name_list_f );
_name_dcl_print ( _au2_nn , (unsigned char )0 ) ;
break ;
}
}
}
;

#line 1107 "../../src/print.c"
char _enumdef_dcl_print (_au0_this , _au0_n )
#line 276 "../../src/cfront.h"
struct enumdef *_au0_this ;

#line 1107 "../../src/print.c"
Pname _au0_n ;
{ 
#line 1109 "../../src/print.c"
if (_au0_this -> _enumdef_mem ){ 
#line 1110 "../../src/print.c"
fprintf ( out_file , (char *)"/* enum %s */\n", _au0_n -> _expr__O3.__C3_string ) ;
_name_dcl_print ( _au0_this -> _enumdef_mem , (unsigned char )72 ) ;
}
}
;
int addrof_cm = 0 ;

#line 1117 "../../src/print.c"
char _expr_print (_au0_this )
#line 525 "../../src/cfront.h"
struct expr *_au0_this ;

#line 1118 "../../src/print.c"
{ 
#line 1119 "../../src/print.c"
if (_au0_this == 0 )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"0->E::print()", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ,
#line 1119 "../../src/print.c"
(struct ea *)ea0 ) ;
if ((_au0_this == _au0_this -> _expr__O3.__C3_e1 )|| (_au0_this == _au0_this -> _expr__O4.__C4_e2 )){ 
#line 1689 "../../src/print.c"
struct ea _au0__V24 ;

#line 1689 "../../src/print.c"
struct ea _au0__V25 ;

#line 1689 "../../src/print.c"
struct ea _au0__V26 ;

#line 1689 "../../src/print.c"
struct ea _au0__V27 ;

#line 1120 "../../src/print.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"(%p%k)->E::print(%p %p)", (struct ea *)( ( ((& _au0__V24 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((&
#line 1120 "../../src/print.c"
_au0__V24 )))) ) , (struct ea *)( ( ((& _au0__V25 )-> _ea__O1.__C1_i = ((int )_au0_this -> _node_base )), (((& _au0__V25 ))))
#line 1120 "../../src/print.c"
) , (struct ea *)( ( ((& _au0__V26 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _expr__O3.__C3_e1 )), (((& _au0__V26 )))) )
#line 1120 "../../src/print.c"
, (struct ea *)( ( ((& _au0__V27 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _expr__O4.__C4_e2 )), (((& _au0__V27 )))) ) )
#line 1120 "../../src/print.c"
;
} switch (_au0_this -> _node_base ){ 
#line 1122 "../../src/print.c"
case 85 : 
#line 1123 "../../src/print.c"
{ Pname _au3_n ;

#line 1123 "../../src/print.c"
_au3_n = (((struct name *)_au0_this ));
if (_au3_n -> _name_n_evaluated && (_au3_n -> _name_n_scope != 136 )){ 
#line 1125 "../../src/print.c"
if (_au0_this -> _expr__O2.__C2_tp -> _node_base != 21 ){ 
#line 1126 "../../src/print.c"
fputs ( (char *)"((",
#line 1126 "../../src/print.c"
out_file ) ;
{ bit _au5_oc ;

#line 1127 "../../src/print.c"
_au5_oc = Cast ;
Cast = 1 ;
_type_print ( _au0_this -> _expr__O2.__C2_tp ) ;
Cast = _au5_oc ;
fprintf ( out_file , (char *)")%d)", _au3_n -> _name_n_val ) ;
}
}
else fprintf ( out_file , (char *)"%d", _au3_n -> _name_n_val ) ;
}
else 
#line 1137 "../../src/print.c"
_name_print ( _au3_n ) ;
break ;
}
case 169 : 
#line 1141 "../../src/print.c"
if (curr_icall ){ 
#line 1142 "../../src/print.c"
Pname _au3_n ;
int _au3_argno ;

#line 1145 "../../src/print.c"
Pin _au3_il ;

#line 1142 "../../src/print.c"
_au3_n = (((struct name *)_au0_this ));
_au3_argno = _au3_n -> _name_n_val ;

#line 1145 "../../src/print.c"
for(_au3_il = curr_icall ;_au3_il ;_au3_il = _au3_il -> _iline_i_next ) 
#line 1146 "../../src/print.c"
if (_au3_n -> _expr__O5.__C5_n_table == _au3_il -> _iline_i_table )goto aok ;
goto bok ;
aok :
#line 1149 "../../src/print.c"
if (_au3_n = (_au3_il -> _iline_local [_au3_argno ])){ 
#line 1150 "../../src/print.c"
_name_print ( _au3_n ) ;
}
else { 
#line 1153 "../../src/print.c"
Pexpr _au4_ee ;
Ptype _au4_t ;

#line 1153 "../../src/print.c"
_au4_ee = (_au3_il -> _iline_arg [_au3_argno ]);
_au4_t = (_au3_il -> _iline_tp [_au3_argno ]);
if ((_au4_ee == 0 )|| (_au4_ee == _au0_this )){ 
#line 1689 "../../src/print.c"
struct ea _au0__V28 ;

#line 1689 "../../src/print.c"
struct ea _au0__V29 ;

#line 1155 "../../src/print.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"%p->E::print(A %p)", (struct ea *)( ( ((& _au0__V28 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((&
#line 1155 "../../src/print.c"
_au0__V28 )))) ) , (struct ea *)( ( ((& _au0__V29 )-> _ea__O1.__C1_p = ((char *)_au4_ee )), (((& _au0__V29 )))) )
#line 1155 "../../src/print.c"
, (struct ea *)ea0 , (struct ea *)ea0 ) ;
} if ((_au4_ee -> _expr__O2.__C2_tp == 0 )|| ((((_au4_t != _au4_ee -> _expr__O2.__C2_tp )&& _type_check ( _au4_t , _au4_ee -> _expr__O2.__C2_tp , (unsigned char
#line 1156 "../../src/print.c"
)0 ) )&& (_type_is_cl_obj ( _au4_t ) == 0 ))&& (eobj == 0 )))
#line 1161 "../../src/print.c"
{ 
#line 1162 "../../src/print.c"
fputs ( (char *)"((", out_file ) ;
{ bit _au5_oc ;

#line 1163 "../../src/print.c"
_au5_oc = Cast ;
Cast = 1 ;
_type_print ( _au4_t ) ;
Cast = _au5_oc ;
putc(')', out_file);
if (_au4_ee )Eprint ( _au4_ee ) ;
putc(')', out_file);
}
}
else if (_au4_ee )Eprint ( _au4_ee ) ;
}
}
else { 
#line 1176 "../../src/print.c"
bok :
#line 1177 "../../src/print.c"
_name_print ( ((struct name *)_au0_this )) ;
}
break ;

#line 1181 "../../src/print.c"
case 168 : 
#line 1182 "../../src/print.c"
{ _au0_this -> _expr__O5.__C5_il -> _iline_i_next = curr_icall ;
curr_icall = _au0_this -> _expr__O5.__C5_il ;
if (_au0_this -> _expr__O5.__C5_il == 0 )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"E::print: iline missing", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ,
#line 1184 "../../src/print.c"
(struct ea *)ea0 ) ;
{ Pexpr _au3_a0 ;
int _au3_val ;

#line 1185 "../../src/print.c"
_au3_a0 = (_au0_this -> _expr__O5.__C5_il -> _iline_arg [0 ]);
_au3_val = 68 ;
if (_au0_this -> _expr__O5.__C5_il -> _iline_fct_name -> _name_n_oper != 161 )goto dumb ;

#line 1194 "../../src/print.c"
switch (_au3_a0 -> _node_base ){ 
#line 1195 "../../src/print.c"
case 86 : 
#line 1196 "../../src/print.c"
_au3_val = 0 ;
break ;
case 112 : 
#line 1199 "../../src/print.c"
case 145 : 
#line 1200 "../../src/print.c"
_au3_val = 1 ;
break ;
case 113 : 
#line 1203 "../../src/print.c"
if ((_au3_a0 -> _expr__O3.__C3_e1 -> _node_base == 169 )|| (_au3_a0 -> _expr__O3.__C3_e1 -> _node_base == 85 )){ 
#line 1204 "../../src/print.c"
Pname _au5_a ;

#line 1204 "../../src/print.c"
_au5_a = (((struct name *)_au3_a0 -> _expr__O3.__C3_e1 ));
if (_au5_a -> _name_n_assigned_to == 111 )_au3_val = 111 ;
}
}
if (_au3_val == 68 )goto dumb ;

#line 1218 "../../src/print.c"
{ Pexpr _au4_e ;

#line 1218 "../../src/print.c"
_au4_e = _au0_this -> _expr__O3.__C3_e1 ;
lx :
#line 1220 "../../src/print.c"
switch (_au4_e -> _node_base ){ 
#line 1221 "../../src/print.c"
case 71 : 
#line 1222 "../../src/print.c"
_au4_e = (((_au4_e -> _expr__O4.__C4_e2 -> _node_base == 68 )|| (_au4_e -> _expr__O3.__C3_e1 -> _node_base == 70 ))?
#line 1222 "../../src/print.c"
_au4_e -> _expr__O4.__C4_e2 : _au4_e -> _expr__O3.__C3_e1 );
goto lx ;

#line 1225 "../../src/print.c"
case 68 : 
#line 1226 "../../src/print.c"
{ Pexpr _au6_q ;

#line 1226 "../../src/print.c"
_au6_q = _au4_e -> _expr__O5.__C5_cond ;
if (((_au6_q -> _node_base == 62 )&& (_au6_q -> _expr__O3.__C3_e1 -> _node_base == 169 ))&& (_au6_q -> _expr__O4.__C4_e2 == zero )){ 
#line 1228 "../../src/print.c"
Pexpr _au7_saved ;
Pexpr _au7_from ;

#line 1228 "../../src/print.c"
_au7_saved = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )0 , (struct expr *)0 , (struct expr *)0 ) ;
_au7_from = ((_au3_val == 0 )? _au4_e -> _expr__O3.__C3_e1 : _au4_e -> _expr__O4.__C4_e2 );
(*_au7_saved )= (*_au4_e );
(*_au4_e )= (*_au7_from );
if (_au0_this -> _expr__O3.__C3_e1 )Eprint ( _au0_this -> _expr__O3.__C3_e1 ) ;
(*_au4_e )= (*_au7_saved );
_expr__dtor ( _au7_saved , 1) ;
curr_icall = _au0_this -> _expr__O5.__C5_il -> _iline_i_next ;
return ;
}
}
}
}
dumb :
#line 1242 "../../src/print.c"
if (_au0_this -> _expr__O3.__C3_e1 )Eprint ( _au0_this -> _expr__O3.__C3_e1 ) ;
if (_au0_this -> _expr__O4.__C4_e2 )_stmt_print ( ((struct stmt *)_au0_this -> _expr__O4.__C4_e2 )) ;
curr_icall = _au0_this -> _expr__O5.__C5_il -> _iline_i_next ;
break ;
}
}

#line 1247 "../../src/print.c"
case 44 : 
#line 1248 "../../src/print.c"
case 45 : 
#line 1249 "../../src/print.c"
if (_au0_this -> _expr__O3.__C3_e1 )Eprint ( _au0_this -> _expr__O3.__C3_e1 ) ;
puttok ( _au0_this -> _node_base ) ;
if (_au0_this -> _expr__O5.__C5_mem -> _node_base == 85 )
#line 1252 "../../src/print.c"
_name_print ( ((struct name *)_au0_this -> _expr__O5.__C5_mem )) ;
else 
#line 1254 "../../src/print.c"
_name_print ( _au0_this -> _expr__O5.__C5_mem ) ;
break ;

#line 1257 "../../src/print.c"
case 157 : 
#line 1258 "../../src/print.c"
_type_print ( _au0_this -> _expr__O5.__C5_tp2 ) ;
puttok ( (unsigned char )40 ) ;

#line 1266 "../../src/print.c"
if (_au0_this -> _expr__O3.__C3_e1 )_expr_print ( _au0_this -> _expr__O3.__C3_e1 ) ;
puttok ( (unsigned char )41 ) ;
break ;

#line 1270 "../../src/print.c"
case 30 : 
#line 1271 "../../src/print.c"
puttok ( (unsigned char )30 ) ;
if (_au0_this -> _expr__O3.__C3_e1 && (_au0_this -> _expr__O3.__C3_e1 != dummy )){ 
#line 1273 "../../src/print.c"
if (_au0_this -> _expr__O3.__C3_e1 )Eprint ( _au0_this -> _expr__O3.__C3_e1 ) ;
}
else if (_au0_this -> _expr__O5.__C5_tp2 ){ 
#line 1276 "../../src/print.c"
putc('(', out_file);
if (_au0_this -> _expr__O5.__C5_tp2 -> _node_base == 6 ){ 
#line 1278 "../../src/print.c"
if ((((struct classdef *)_au0_this -> _expr__O5.__C5_tp2 ))-> _classdef_csu == 36 )
#line 1279 "../../src/print.c"
fputs ( (char *)"union ", out_file )
#line 1279 "../../src/print.c"
;
else fputs ( (char *)"struct ", out_file ) ;
fputs ( (char *)(((struct classdef *)_au0_this -> _expr__O5.__C5_tp2 ))-> _classdef_string , out_file ) ;
}
else 
#line 1284 "../../src/print.c"
_type_print ( _au0_this -> _expr__O5.__C5_tp2 ) ;
putc(')', out_file);
}
break ;

#line 1289 "../../src/print.c"
case 23 : 
#line 1290 "../../src/print.c"
puttok ( (unsigned char )23 ) ;
_type_print ( _au0_this -> _expr__O5.__C5_tp2 ) ;
if (_au0_this -> _expr__O3.__C3_e1 ){ 
#line 1293 "../../src/print.c"
putc('(', out_file);
_expr_print ( _au0_this -> _expr__O3.__C3_e1 ) ;
putc(')', out_file);
}
break ;

#line 1299 "../../src/print.c"
case 9 : 
#line 1300 "../../src/print.c"
puttok ( (unsigned char )9 ) ;
_expr_print ( _au0_this -> _expr__O3.__C3_e1 ) ;
break ;

#line 1304 "../../src/print.c"
case 113 : 
#line 1305 "../../src/print.c"
putc('(', out_file);
if (_au0_this -> _expr__O5.__C5_tp2 -> _node_base != 38 ){ 
#line 1307 "../../src/print.c"
putc('(', out_file);
{ bit _au3_oc ;

#line 1308 "../../src/print.c"
_au3_oc = Cast ;
Cast = 1 ;
_type_print ( _au0_this -> _expr__O5.__C5_tp2 ) ;
Cast = _au3_oc ;
putc(')', out_file);
}
}

#line 1314 "../../src/print.c"
if (_au0_this -> _expr__O3.__C3_e1 )Eprint ( _au0_this -> _expr__O3.__C3_e1 ) ;
putc(')', out_file);
break ;

#line 1318 "../../src/print.c"
case 82 : 
#line 1319 "../../src/print.c"
case 83 : 
#line 1320 "../../src/print.c"
case 84 : 
#line 1321 "../../src/print.c"
case 80 : 
#line 1322 "../../src/print.c"
if (_au0_this -> _expr__O3.__C3_string )( fputs ( (char *)_au0_this ->
#line 1322 "../../src/print.c"
_expr__O3.__C3_string , out_file ) , putc(' ', out_file)) ;
break ;

#line 1325 "../../src/print.c"
case 81 : 
#line 1326 "../../src/print.c"
fprintf ( out_file , (char *)"\"%s\"", _au0_this -> _expr__O3.__C3_string ) ;
break ;

#line 1329 "../../src/print.c"
case 34 : 
#line 1330 "../../src/print.c"
case 86 : 
#line 1331 "../../src/print.c"
fputs ( (char *)"0 ", out_file ) ;
break ;

#line 1334 "../../src/print.c"
case 150 : 
#line 1335 "../../src/print.c"
fprintf ( out_file , (char *)"%d", _au0_this -> _expr__O3.__C3_i1 ) ;
break ;

#line 1338 "../../src/print.c"
case 165 : 
#line 1339 "../../src/print.c"
if (_au0_this -> _expr__O4.__C4_string2 )
#line 1340 "../../src/print.c"
fprintf ( out_file , (char *)" %s_%s", _au0_this -> _expr__O3.__C3_string , _au0_this -> _expr__O4.__C4_string2 ) ;
else 
#line 1342 "../../src/print.c"
fprintf ( out_file , (char *)" %s", _au0_this -> _expr__O3.__C3_string ) ;
break ;

#line 1345 "../../src/print.c"
case 144 : 
#line 1346 "../../src/print.c"
break ;

#line 1348 "../../src/print.c"
case 146 : 
#line 1349 "../../src/print.c"
case 109 : 
#line 1350 "../../src/print.c"
{ Pname _au3_fn ;
Pname _au3_at ;

#line 1350 "../../src/print.c"
_au3_fn = _au0_this -> _expr__O5.__C5_fct_name ;

#line 1352 "../../src/print.c"
if (_au3_fn ){ 
#line 1353 "../../src/print.c"
Pfct _au4_f ;

#line 1353 "../../src/print.c"
_au4_f = (((struct fct *)_au3_fn -> _expr__O2.__C2_tp ));

#line 1355 "../../src/print.c"
if (_au4_f -> _node_base == 76 ){ 
#line 1356 "../../src/print.c"
Pgen _au5_g ;

#line 1356 "../../src/print.c"
_au5_g = (((struct gen *)_au4_f ));
_au0_this -> _expr__O5.__C5_fct_name = (_au3_fn = _au5_g -> _gen_fct_list -> _name_list_f );
_au4_f = (((struct fct *)_au3_fn -> _expr__O2.__C2_tp ));
}
_name_print ( _au3_fn ) ;
_au3_at = (_au4_f -> _fct_f_this ? _au4_f -> _fct_f_this : (_au4_f -> _fct_f_result ? _au4_f -> _fct_f_result : _au4_f -> _fct_argtype ));
}
else { 
#line 1364 "../../src/print.c"
Pfct _au4_f ;

#line 1364 "../../src/print.c"
_au4_f = (((struct fct *)_au0_this -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp ));

#line 1366 "../../src/print.c"
if (_au4_f ){ 
#line 1367 "../../src/print.c"
while (_au4_f -> _node_base == 97 )_au4_f = (((struct fct *)(((struct basetype *)_au4_f ))-> _basetype_b_name -> _expr__O2.__C2_tp ));
if (_au4_f -> _node_base == 125 ){ 
#line 1369 "../../src/print.c"
fputs ( (char *)"(*", out_file ) ;
_expr_print ( _au0_this -> _expr__O3.__C3_e1 ) ;
putc(')', out_file);
_au4_f = (((struct fct *)(((struct ptr *)_au4_f ))-> _pvtyp_typ ));
while (_au4_f -> _node_base == 97 )_au4_f = (((struct fct *)(((struct basetype *)_au4_f ))-> _basetype_b_name -> _expr__O2.__C2_tp ));
}
else 
#line 1376 "../../src/print.c"
if (_au0_this -> _expr__O3.__C3_e1 )Eprint ( _au0_this -> _expr__O3.__C3_e1 ) ;

#line 1379 "../../src/print.c"
_au3_at = (_au4_f -> _fct_f_result ? _au4_f -> _fct_f_result : _au4_f -> _fct_argtype );
}
else 
#line 1382 "../../src/print.c"
{ 
#line 1384 "../../src/print.c"
_au3_at = ((_au0_this -> _expr__O3.__C3_e1 -> _node_base == 68 )? (((struct name *)_au0_this -> _expr__O3.__C3_e1 -> _expr__O3.__C3_e1 -> _expr__O5.__C5_tp2 )): (((struct name *)_au0_this ->
#line 1384 "../../src/print.c"
_expr__O3.__C3_e1 -> _expr__O5.__C5_tp2 )));

#line 1386 "../../src/print.c"
if (_au0_this -> _expr__O3.__C3_e1 )Eprint ( _au0_this -> _expr__O3.__C3_e1 ) ;
}
}

#line 1390 "../../src/print.c"
puttok ( (unsigned char )40 ) ;
if (_au0_this -> _expr__O4.__C4_e2 ){ 
#line 1392 "../../src/print.c"
if (_au3_at ){ 
#line 1393 "../../src/print.c"
Pexpr _au5_e ;

#line 1393 "../../src/print.c"
_au5_e = _au0_this -> _expr__O4.__C4_e2 ;
while (_au3_at ){ 
#line 1395 "../../src/print.c"
Pexpr _au6_ex ;
Ptype _au6_t ;

#line 1396 "../../src/print.c"
_au6_t = _au3_at -> _expr__O2.__C2_tp ;

#line 1398 "../../src/print.c"
if (_au5_e == 0 ){ 
#line 1689 "../../src/print.c"
struct ea _au0__V30 ;

#line 1398 "../../src/print.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"A missing for %s()", (struct ea *)( ( ((& _au0__V30 )-> _ea__O1.__C1_p = ((char *)(_au3_fn ? _au3_fn ->
#line 1398 "../../src/print.c"
_expr__O3.__C3_string : "??"))), (((& _au0__V30 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} if (_au5_e -> _node_base == 140 ){ 
#line 1400 "../../src/print.c"
_au6_ex = _au5_e -> _expr__O3.__C3_e1 ;
_au5_e = _au5_e -> _expr__O4.__C4_e2 ;
}
else 
#line 1404 "../../src/print.c"
_au6_ex = _au5_e ;

#line 1406 "../../src/print.c"
if (_au6_ex == 0 ){ 
#line 1689 "../../src/print.c"
struct ea _au0__V31 ;

#line 1406 "../../src/print.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"A ofT%t missing", (struct ea *)( ( ((& _au0__V31 )-> _ea__O1.__C1_p = ((char *)_au6_t )), (((&
#line 1406 "../../src/print.c"
_au0__V31 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} if (((((_au6_t != _au6_ex -> _expr__O2.__C2_tp )&& _au6_ex -> _expr__O2.__C2_tp )&& _type_check ( _au6_t , _au6_ex -> _expr__O2.__C2_tp , (unsigned char )0 )
#line 1407 "../../src/print.c"
)&& (_type_is_cl_obj ( _au6_t ) == 0 ))&& (eobj == 0 ))
#line 1411 "../../src/print.c"
{ 
#line 1412 "../../src/print.c"
putc('(', out_file);
{ bit _au7_oc ;

#line 1413 "../../src/print.c"
_au7_oc = Cast ;
Cast = 1 ;
_type_print ( _au6_t ) ;
putc(')', out_file);
Cast = _au7_oc ;

#line 1426 "../../src/print.c"
if (_au6_ex )Eprint ( _au6_ex ) ;
}
}
else _expr_print ( _au6_ex ) ;
_au3_at = _au3_at -> _name_n_list ;
if (_au3_at )puttok ( (unsigned char )71 ) ;
}
if (_au5_e ){ 
#line 1434 "../../src/print.c"
puttok ( (unsigned char )71 ) ;
_expr_print ( _au5_e ) ;
}
}
else 
#line 1439 "../../src/print.c"
_expr_print ( _au0_this -> _expr__O4.__C4_e2 ) ;
}
puttok ( (unsigned char )41 ) ;
break ;
}

#line 1445 "../../src/print.c"
case 70 : 
#line 1446 "../../src/print.c"
if ((_au0_this -> _expr__O3.__C3_e1 -> _node_base == 169 )&& ((((struct name *)_au0_this -> _expr__O3.__C3_e1 ))-> _name_n_assigned_to == 111 )){ 
#line 1448 "../../src/print.c"
Pname _au3_n ;
int _au3_argno ;
Pin _au3_il ;

#line 1448 "../../src/print.c"
_au3_n = (((struct name *)_au0_this -> _expr__O3.__C3_e1 ));
_au3_argno = _au3_n -> _name_n_val ;

#line 1451 "../../src/print.c"
for(_au3_il = curr_icall ;_au3_il ;_au3_il = _au3_il -> _iline_i_next ) 
#line 1452 "../../src/print.c"
if (_au3_il -> _iline_i_table == _au3_n -> _expr__O5.__C5_n_table )goto akk ;
goto bkk ;
akk :
#line 1455 "../../src/print.c"
if ((_au3_il -> _iline_local [_au3_argno ])== 0 ){ 
#line 1456 "../../src/print.c"
_expr_print ( _au0_this -> _expr__O4.__C4_e2 ) ;
break ;
}
}
case 62 : 
#line 1461 "../../src/print.c"
case 63 : 
#line 1462 "../../src/print.c"
case 60 : 
#line 1463 "../../src/print.c"
case 61 : 
#line 1464 "../../src/print.c"
case 59 : 
#line 1465 "../../src/print.c"
case 58 : 
#line 1466 "../../src/print.c"
bkk :
#line 1467 "../../src/print.c"
if (_au0_this ->
#line 1467 "../../src/print.c"
_expr__O3.__C3_e1 )Eprint ( _au0_this -> _expr__O3.__C3_e1 ) ;
puttok ( _au0_this -> _node_base ) ;

#line 1470 "../../src/print.c"
if ((_au0_this -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp != _au0_this -> _expr__O4.__C4_e2 -> _expr__O2.__C2_tp )&& (_au0_this -> _expr__O4.__C4_e2 -> _node_base != 86 )){ 
#line 1472 "../../src/print.c"
Ptype _au3_t1 ;

#line 1472 "../../src/print.c"
_au3_t1 = _au0_this -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp ;
cmp :
#line 1474 "../../src/print.c"
switch (_au3_t1 -> _node_base ){ 
#line 1475 "../../src/print.c"
default : 
#line 1476 "../../src/print.c"
break ;
case 97 : 
#line 1478 "../../src/print.c"
_au3_t1 = (((struct basetype *)_au3_t1 ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 1478 "../../src/print.c"
goto cmp ;
case 125 : 
#line 1480 "../../src/print.c"
case 158 : 
#line 1481 "../../src/print.c"
case 110 : 
#line 1482 "../../src/print.c"
if ((_au0_this -> _expr__O4.__C4_e2 -> _expr__O2.__C2_tp == 0 )|| (((((struct ptr *)_au3_t1 ))-> _pvtyp_typ !=
#line 1482 "../../src/print.c"
(((struct ptr *)_au0_this -> _expr__O4.__C4_e2 -> _expr__O2.__C2_tp ))-> _pvtyp_typ )&& _type_check ( _au3_t1 , _au0_this -> _expr__O4.__C4_e2 -> _expr__O2.__C2_tp , (unsigned char )0 ) ))
#line 1484 "../../src/print.c"
{
#line 1484 "../../src/print.c"

#line 1485 "../../src/print.c"
putc('(', out_file);
{ bit _au5_oc ;

#line 1486 "../../src/print.c"
_au5_oc = Cast ;
Cast = 1 ;
_type_print ( _au0_this -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp ) ;
Cast = _au5_oc ;
putc(')', out_file);
}
}
}
}
if (_au0_this -> _expr__O4.__C4_e2 )Eprint ( _au0_this -> _expr__O4.__C4_e2 ) ;
break ;

#line 1498 "../../src/print.c"
case 111 : 
#line 1499 "../../src/print.c"
if (_au0_this -> _expr__O4.__C4_e2 ){ 
#line 1500 "../../src/print.c"
if (_au0_this -> _expr__O3.__C3_e1 )Eprint ( _au0_this -> _expr__O3.__C3_e1 ) ;
putc('[', out_file);
_expr_print ( _au0_this -> _expr__O4.__C4_e2 ) ;
putc(']', out_file);
}
else { 
#line 1506 "../../src/print.c"
putc('*', out_file);
if (_au0_this -> _expr__O3.__C3_e1 )Eprint ( _au0_this -> _expr__O3.__C3_e1 ) ;
}
break ;

#line 1511 "../../src/print.c"
case 124 : 
#line 1512 "../../src/print.c"
puttok ( (unsigned char )73 ) ;
if (_au0_this -> _expr__O3.__C3_e1 )_expr_print ( _au0_this -> _expr__O3.__C3_e1 ) ;
puttok ( (unsigned char )74 ) ;
break ;

#line 1517 "../../src/print.c"
case 140 : 
#line 1518 "../../src/print.c"
{ Pexpr _au3_e ;

#line 1518 "../../src/print.c"
_au3_e = _au0_this ;
for(;;) { 
#line 1520 "../../src/print.c"
if (_au3_e -> _node_base == 140 ){ 
#line 1521 "../../src/print.c"
_expr_print ( _au3_e -> _expr__O3.__C3_e1 ) ;
if (_au3_e = _au3_e -> _expr__O4.__C4_e2 )
#line 1523 "../../src/print.c"
puttok ( (unsigned char )71 ) ;
else 
#line 1525 "../../src/print.c"
return ;
}
else { 
#line 1528 "../../src/print.c"
_expr_print ( _au3_e ) ;
return ;
}
}
}

#line 1534 "../../src/print.c"
case 68 : 
#line 1535 "../../src/print.c"
{ 
#line 1536 "../../src/print.c"
extern bit binary_val ;
Neval = 0 ;
binary_val = 1 ;
{ int _au3_i ;

#line 1539 "../../src/print.c"
_au3_i = _expr_eval ( _au0_this -> _expr__O5.__C5_cond ) ;
binary_val = 0 ;
if (Neval == 0 )
#line 1542 "../../src/print.c"
_expr_print ( _au3_i ? _au0_this -> _expr__O3.__C3_e1 : _au0_this -> _expr__O4.__C4_e2 ) ;
else { 
#line 1544 "../../src/print.c"
if (_au0_this -> _expr__O5.__C5_cond )Eprint ( _au0_this -> _expr__O5.__C5_cond ) ;
puttok ( (unsigned char )68 ) ;
if (_au0_this -> _expr__O3.__C3_e1 )Eprint ( _au0_this -> _expr__O3.__C3_e1 ) ;
puttok ( (unsigned char )69 ) ;
if (_au0_this -> _expr__O4.__C4_e2 )Eprint ( _au0_this -> _expr__O4.__C4_e2 ) ;
}
break ;
}
}
case 71 : 
#line 1554 "../../src/print.c"
case 147 : 
#line 1555 "../../src/print.c"
puttok ( (unsigned char )40 ) ;

#line 1557 "../../src/print.c"
switch (_au0_this -> _expr__O3.__C3_e1 -> _node_base ){ 
#line 1558 "../../src/print.c"
case 86 : 
#line 1559 "../../src/print.c"
case 150 : 
#line 1560 "../../src/print.c"
case 82 : 
#line 1561 "../../src/print.c"
case 85 : 
#line 1562 "../../src/print.c"
case 45 :
#line 1562 "../../src/print.c"

#line 1563 "../../src/print.c"
case 44 : 
#line 1564 "../../src/print.c"
case 83 : 
#line 1565 "../../src/print.c"
case 151 : 
#line 1566 "../../src/print.c"
case 81 : 
#line 1567 "../../src/print.c"
goto le2 ;
default : 
#line 1569 "../../src/print.c"
{ int _au4_oo ;

#line 1569 "../../src/print.c"
_au4_oo = addrof_cm ;
addrof_cm = 0 ;
if (_au0_this -> _expr__O3.__C3_e1 )Eprint ( _au0_this -> _expr__O3.__C3_e1 ) ;
addrof_cm = _au4_oo ;
}
puttok ( (unsigned char )71 ) ;
le2 :
#line 1576 "../../src/print.c"
if (addrof_cm ){ 
#line 1577 "../../src/print.c"
switch (_au0_this -> _expr__O4.__C4_e2 -> _node_base ){ 
#line 1578 "../../src/print.c"
case 113 : 
#line 1579 "../../src/print.c"
switch (_au0_this -> _expr__O4.__C4_e2 -> _expr__O4.__C4_e2 -> _node_base ){ 
#line 1580 "../../src/print.c"
case
#line 1580 "../../src/print.c"
71 : 
#line 1581 "../../src/print.c"
case 147 : 
#line 1582 "../../src/print.c"
case 168 : goto ec ;
}
case 85 : 
#line 1585 "../../src/print.c"
case 45 : 
#line 1586 "../../src/print.c"
case 111 : 
#line 1587 "../../src/print.c"
case 44 : 
#line 1588 "../../src/print.c"
case 169 : 
#line 1589 "../../src/print.c"
puttok ( (unsigned char )112 )
#line 1589 "../../src/print.c"
;
addrof_cm -- ;
if (_au0_this -> _expr__O4.__C4_e2 )Eprint ( _au0_this -> _expr__O4.__C4_e2 ) ;
addrof_cm ++ ;
break ;
case 168 : 
#line 1595 "../../src/print.c"
case 109 : 
#line 1596 "../../src/print.c"
case 71 : 
#line 1597 "../../src/print.c"
case 147 : 
#line 1598 "../../src/print.c"
ec :
#line 1599 "../../src/print.c"
if (_au0_this -> _expr__O4.__C4_e2 )Eprint ( _au0_this -> _expr__O4.__C4_e2 ) ;
#line 1599 "../../src/print.c"

#line 1600 "../../src/print.c"
break ;
case 146 : 
#line 1603 "../../src/print.c"
if (_au0_this -> _expr__O4.__C4_e2 -> _expr__O5.__C5_fct_name && (_au0_this -> _expr__O4.__C4_e2 -> _expr__O5.__C5_fct_name -> _name_n_oper == 161 ))
#line 1604 "../../src/print.c"
{ 
#line 1605 "../../src/print.c"
addrof_cm -- ;
if (_au0_this -> _expr__O4.__C4_e2 )Eprint ( _au0_this -> _expr__O4.__C4_e2 ) ;
addrof_cm ++ ;
break ;
}
default : 
#line 1611 "../../src/print.c"
{ 
#line 1689 "../../src/print.c"
struct ea _au0__V32 ;

#line 1611 "../../src/print.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"& inlineF call (%k)", (struct ea *)( ( ((& _au0__V32 )-> _ea__O1.__C1_i = ((int )_au0_this -> _expr__O4.__C4_e2 ->
#line 1611 "../../src/print.c"
_node_base )), (((& _au0__V32 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}
else 
#line 1616 "../../src/print.c"
if (_au0_this -> _expr__O4.__C4_e2 )Eprint ( _au0_this -> _expr__O4.__C4_e2 ) ;
puttok ( (unsigned char )41 ) ;
}
break ;

#line 1621 "../../src/print.c"
case 107 : 
#line 1622 "../../src/print.c"
case 46 : 
#line 1623 "../../src/print.c"
case 47 : 
#line 1624 "../../src/print.c"
puttok ( _au0_this -> _node_base ) ;
if (_au0_this -> _expr__O4.__C4_e2 )Eprint ( _au0_this -> _expr__O4.__C4_e2 ) ;
break ;
case 112 : 
#line 1628 "../../src/print.c"
case 145 : 
#line 1629 "../../src/print.c"
switch (_au0_this -> _expr__O4.__C4_e2 -> _node_base ){ 
#line 1630 "../../src/print.c"
case 111 : 
#line 1631 "../../src/print.c"
if (_au0_this -> _expr__O4.__C4_e2 -> _expr__O4.__C4_e2 ==
#line 1631 "../../src/print.c"
0 ){ 
#line 1632 "../../src/print.c"
_expr_print ( _au0_this -> _expr__O4.__C4_e2 -> _expr__O3.__C3_e1 ) ;
return ;
}
break ;
case 168 : 
#line 1637 "../../src/print.c"
addrof_cm ++ ;
if (_au0_this -> _expr__O4.__C4_e2 )Eprint ( _au0_this -> _expr__O4.__C4_e2 ) ;
addrof_cm -- ;
return ;
}

#line 1644 "../../src/print.c"
if ((_au0_this -> _expr__O4.__C4_e2 -> _expr__O2.__C2_tp == 0 )|| (_au0_this -> _expr__O4.__C4_e2 -> _expr__O2.__C2_tp -> _node_base != 108 ))puttok ( (unsigned char )112 ) ;
#line 1644 "../../src/print.c"

#line 1646 "../../src/print.c"
if (_au0_this -> _expr__O4.__C4_e2 )Eprint ( _au0_this -> _expr__O4.__C4_e2 ) ;
break ;

#line 1649 "../../src/print.c"
case 54 : 
#line 1650 "../../src/print.c"
case 55 : 
#line 1651 "../../src/print.c"
case 50 : 
#line 1652 "../../src/print.c"
case 51 : 
#line 1653 "../../src/print.c"
case 53 : 
#line 1654 "../../src/print.c"
case 56 : 
#line 1655 "../../src/print.c"
case 57 :
#line 1655 "../../src/print.c"

#line 1663 "../../src/print.c"
case 52 : 
#line 1664 "../../src/print.c"
case 65 : 
#line 1665 "../../src/print.c"
case 64 : 
#line 1666 "../../src/print.c"
case 66 : 
#line 1667 "../../src/print.c"
case 67 : 
#line 1669 "../../src/print.c"
case 132 : 
#line 1670 "../../src/print.c"
case 133 :
#line 1670 "../../src/print.c"

#line 1671 "../../src/print.c"
case 131 : 
#line 1672 "../../src/print.c"
case 126 : 
#line 1673 "../../src/print.c"
case 127 : 
#line 1674 "../../src/print.c"
case 128 : 
#line 1675 "../../src/print.c"
case 130 : 
#line 1676 "../../src/print.c"
case 129 : 
#line 1677 "../../src/print.c"
case 134 :
#line 1677 "../../src/print.c"

#line 1678 "../../src/print.c"
case 135 : 
#line 1679 "../../src/print.c"
case 49 : 
#line 1680 "../../src/print.c"
case 48 : 
#line 1681 "../../src/print.c"
if (_au0_this -> _expr__O3.__C3_e1 )Eprint ( _au0_this -> _expr__O3.__C3_e1 ) ;
puttok ( _au0_this -> _node_base ) ;
if (_au0_this -> _expr__O4.__C4_e2 )Eprint ( _au0_this -> _expr__O4.__C4_e2 ) ;
break ;

#line 1686 "../../src/print.c"
default : 
#line 1687 "../../src/print.c"
{ 
#line 1689 "../../src/print.c"
struct ea _au0__V33 ;

#line 1689 "../../src/print.c"
struct ea _au0__V34 ;

#line 1687 "../../src/print.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"%p->E::print%k", (struct ea *)( ( ((& _au0__V33 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((&
#line 1687 "../../src/print.c"
_au0__V33 )))) ) , (struct ea *)( ( ((& _au0__V34 )-> _ea__O1.__C1_i = ((int )_au0_this -> _node_base )), (((& _au0__V34 ))))
#line 1687 "../../src/print.c"
) , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}
;
Pexpr aval (_au0_a )Pname _au0_a ;
{ 
#line 1693 "../../src/print.c"
int _au1_argno ;
Pin _au1_il ;

#line 1693 "../../src/print.c"
_au1_argno = _au0_a -> _name_n_val ;

#line 1695 "../../src/print.c"
for(_au1_il = curr_icall ;_au1_il ;_au1_il = _au1_il -> _iline_i_next ) 
#line 1696 "../../src/print.c"
if (_au1_il -> _iline_i_table == _au0_a -> _expr__O5.__C5_n_table )goto aok ;
return (struct expr *)0 ;
aok :
#line 1699 "../../src/print.c"
{ Pexpr _au1_aa ;

#line 1699 "../../src/print.c"
_au1_aa = (_au1_il -> _iline_arg [_au1_argno ]);
ll :
#line 1701 "../../src/print.c"
switch (_au1_aa -> _node_base ){ 
#line 1702 "../../src/print.c"
case 113 : _au1_aa = _au1_aa -> _expr__O3.__C3_e1 ;

#line 1702 "../../src/print.c"
goto ll ;
case 169 : return aval ( ((struct name *)_au1_aa )) ;
default : return _au1_aa ;
}
}
}
;

#line 1710 "../../src/print.c"
char _stmt_print (_au0_this )
#line 651 "../../src/cfront.h"
struct stmt *_au0_this ;

#line 1711 "../../src/print.c"
{ 
#line 1713 "../../src/print.c"
if (_au0_this -> _stmt_where . _loc_line != last_line . _loc_line )
#line 1714 "../../src/print.c"
if (last_ll = _au0_this -> _stmt_where . _loc_line )
#line 1715 "../../src/print.c"
_loc_putline ( & _au0_this -> _stmt_where )
#line 1715 "../../src/print.c"
;
else 
#line 1717 "../../src/print.c"
_loc_putline ( & last_line ) ;

#line 1719 "../../src/print.c"
if (_au0_this -> _stmt_memtbl && (_au0_this -> _node_base != 116 )){ 
#line 1720 "../../src/print.c"
puttok ( (unsigned char )73 ) ;
{ Ptable _au2_tbl ;

#line 1723 "../../src/print.c"
int _au2_i ;
int _au2_bl ;
Pname _au2_n ;

#line 1721 "../../src/print.c"
_au2_tbl = _au0_this -> _stmt_memtbl ;
_au0_this -> _stmt_memtbl = 0 ;
;

#line 1723 "../../src/print.c"
_au2_bl = 1 ;

#line 1725 "../../src/print.c"
for(_au2_n = _table_get_mem ( _au2_tbl , _au2_i = 1 ) ;_au2_n ;_au2_n = _table_get_mem ( _au2_tbl , ++ _au2_i ) ) { 
#line 1726 "../../src/print.c"
if (_au2_n ->
#line 1726 "../../src/print.c"
_expr__O2.__C2_tp == (struct type *)any_type )continue ;

#line 1728 "../../src/print.c"
{ char *_au3_s ;

#line 1733 "../../src/print.c"
Pname _au3_cn ;

#line 1728 "../../src/print.c"
_au3_s = _au2_n -> _expr__O3.__C3_string ;
if (((_au3_s [0 ])!= '_' )|| ((_au3_s [1 ])!= 'X' )){ 
#line 1730 "../../src/print.c"
_name_dcl_print ( _au2_n , (unsigned char )0 ) ;
_au2_bl = 0 ;
}
;
if ((_au2_bl && (_au3_cn = _type_is_cl_obj ( _au2_n -> _expr__O2.__C2_tp ) ))&& ( _table_look ( (((struct classdef *)_au3_cn -> _expr__O2.__C2_tp ))-> _classdef_memtbl , "_dtor",
#line 1734 "../../src/print.c"
(unsigned char )0 ) ) )
#line 1736 "../../src/print.c"
_au2_bl = 0 ;
}
}
if (_au2_bl ){ 
#line 1740 "../../src/print.c"
Pstmt _au3_sl ;

#line 1740 "../../src/print.c"
_au3_sl = _au0_this -> _stmt_s_list ;
_au0_this -> _stmt_s_list = 0 ;
_stmt_print ( _au0_this ) ;
_au0_this -> _stmt_memtbl = _au2_tbl ;
puttok ( (unsigned char )74 ) ;
if (_au3_sl ){ 
#line 1746 "../../src/print.c"
_au0_this -> _stmt_s_list = _au3_sl ;
_stmt_print ( _au3_sl ) ;
}
}
else { 
#line 1751 "../../src/print.c"
_stmt_print ( _au0_this ) ;
_au0_this -> _stmt_memtbl = _au2_tbl ;
puttok ( (unsigned char )74 ) ;
}
return ;
}
}
switch (_au0_this -> _node_base ){ 
#line 1759 "../../src/print.c"
default : 
#line 1760 "../../src/print.c"
{ 
#line 1981 "../../src/print.c"
struct ea _au0__V35 ;

#line 1760 "../../src/print.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"S::print(base=%k)", (struct ea *)( ( ((& _au0__V35 )-> _ea__O1.__C1_i = ((int )_au0_this -> _node_base )),
#line 1760 "../../src/print.c"
(((& _au0__V35 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;

#line 1762 "../../src/print.c"
case 1 : 
#line 1763 "../../src/print.c"
fprintf ( out_file , (char *)"asm(\"%s\");\n", ((char *)_au0_this -> _stmt__O8.__C8_e )) ;
break ;

#line 1766 "../../src/print.c"
case 118 : 
#line 1767 "../../src/print.c"
_name_dcl_print ( _au0_this -> _stmt__O7.__C7_d , (unsigned char )72 ) ;
break ;

#line 1770 "../../src/print.c"
case 3 : 
#line 1771 "../../src/print.c"
case 7 : 
#line 1772 "../../src/print.c"
puttok ( _au0_this -> _node_base ) ;
puttok ( (unsigned char )72 ) ;
break ;

#line 1776 "../../src/print.c"
case 8 : 
#line 1777 "../../src/print.c"
puttok ( _au0_this -> _node_base ) ;
puttok ( (unsigned char )69 ) ;
_stmt_print ( _au0_this -> _stmt_s ) ;
break ;

#line 1782 "../../src/print.c"
case 72 : 
#line 1784 "../../src/print.c"
if (_au0_this -> _stmt__O8.__C8_e ){ 
#line 1785 "../../src/print.c"
_expr_print ( _au0_this -> _stmt__O8.__C8_e ) ;
if ((_au0_this -> _stmt__O8.__C8_e -> _node_base == 168 )&& _au0_this -> _stmt__O8.__C8_e -> _expr__O4.__C4_e2 )break ;
}
puttok ( (unsigned char )72 ) ;
break ;

#line 1791 "../../src/print.c"
case 39 : 
#line 1792 "../../src/print.c"
puttok ( (unsigned char )39 ) ;
putc('(', out_file);

#line 1793 "../../src/print.c"
_expr_print ( _au0_this -> _stmt__O8.__C8_e ) ;

#line 1793 "../../src/print.c"
putc(')', out_file);
if (_au0_this -> _stmt_s -> _stmt_s_list ){ 
#line 1795 "../../src/print.c"
puttok ( (unsigned char )73 ) ;
_stmt_print ( _au0_this -> _stmt_s ) ;
puttok ( (unsigned char )74 ) ;
}
else 
#line 1800 "../../src/print.c"
_stmt_print ( _au0_this -> _stmt_s ) ;
break ;

#line 1803 "../../src/print.c"
case 10 : 
#line 1804 "../../src/print.c"
puttok ( (unsigned char )10 ) ;
_stmt_print ( _au0_this -> _stmt_s ) ;
puttok ( (unsigned char )39 ) ;
putc('(', out_file);

#line 1807 "../../src/print.c"
_expr_print ( _au0_this -> _stmt__O8.__C8_e ) ;

#line 1807 "../../src/print.c"
putc(')', out_file);
puttok ( (unsigned char )72 ) ;
break ;

#line 1811 "../../src/print.c"
case 33 : 
#line 1812 "../../src/print.c"
puttok ( (unsigned char )33 ) ;
putc('(', out_file);

#line 1813 "../../src/print.c"
_expr_print ( _au0_this -> _stmt__O8.__C8_e ) ;

#line 1813 "../../src/print.c"
putc(')', out_file);
_stmt_print ( _au0_this -> _stmt_s ) ;
break ;

#line 1817 "../../src/print.c"
case 28 : 
#line 1826 "../../src/print.c"
{ 
#line 1827 "../../src/print.c"
puttok ( (unsigned char )28 ) ;
if (_au0_this -> _stmt__O8.__C8_e ){ 
#line 1829 "../../src/print.c"
if (_au0_this -> _stmt__O7.__C7_ret_tp && (_au0_this -> _stmt__O7.__C7_ret_tp != _au0_this -> _stmt__O8.__C8_e -> _expr__O2.__C2_tp )){ 
#line 1830 "../../src/print.c"
Ptype _au5_tt ;

#line 1830 "../../src/print.c"
_au5_tt = _au0_this -> _stmt__O7.__C7_ret_tp ;
gook :
#line 1832 "../../src/print.c"
switch (_au5_tt -> _node_base ){ 
#line 1833 "../../src/print.c"
case 97 : 
#line 1834 "../../src/print.c"
_au5_tt = (((struct basetype *)_au5_tt ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
goto gook ;
case 119 : 
#line 1837 "../../src/print.c"
break ;
case 158 : 
#line 1839 "../../src/print.c"
case 125 : 
#line 1840 "../../src/print.c"
if ((((struct ptr *)_au5_tt ))-> _pvtyp_typ == (((struct ptr *)_au0_this -> _stmt__O8.__C8_e -> _expr__O2.__C2_tp ))-> _pvtyp_typ )break ;
#line 1840 "../../src/print.c"

#line 1841 "../../src/print.c"
default : 
#line 1842 "../../src/print.c"
if ((_au0_this -> _stmt__O8.__C8_e -> _expr__O2.__C2_tp == 0 )|| _type_check ( _au0_this -> _stmt__O7.__C7_ret_tp , _au0_this -> _stmt__O8.__C8_e -> _expr__O2.__C2_tp , (unsigned
#line 1842 "../../src/print.c"
char )0 ) ){ 
#line 1843 "../../src/print.c"
int _au7_oc ;

#line 1843 "../../src/print.c"
_au7_oc = Cast ;
putc('(', out_file);
Cast = 1 ;
_type_print ( _au0_this -> _stmt__O7.__C7_ret_tp ) ;
Cast = _au7_oc ;
putc(')', out_file);
}
}
}
if (_au0_this -> _stmt__O8.__C8_e )Eprint ( _au0_this -> _stmt__O8.__C8_e ) ;
}
puttok ( (unsigned char )72 ) ;
}
while (_au0_this -> _stmt_s_list && (_au0_this -> _stmt_s_list -> _node_base == 72 ))_au0_this -> _stmt_s_list = _au0_this -> _stmt_s_list -> _stmt_s_list ;
break ;

#line 1859 "../../src/print.c"
case 4 : 
#line 1860 "../../src/print.c"
puttok ( (unsigned char )4 ) ;
if (_au0_this -> _stmt__O8.__C8_e )Eprint ( _au0_this -> _stmt__O8.__C8_e ) ;
puttok ( (unsigned char )69 ) ;
_stmt_print ( _au0_this -> _stmt_s ) ;
break ;

#line 1866 "../../src/print.c"
case 19 : 
#line 1867 "../../src/print.c"
puttok ( (unsigned char )19 ) ;
_name_print ( _au0_this -> _stmt__O7.__C7_d ) ;
puttok ( (unsigned char )72 ) ;
break ;

#line 1872 "../../src/print.c"
case 115 : 
#line 1873 "../../src/print.c"
_name_print ( _au0_this -> _stmt__O7.__C7_d ) ;
putc(':', out_file);
_stmt_print ( _au0_this -> _stmt_s ) ;
break ;

#line 1878 "../../src/print.c"
case 20 : 
#line 1879 "../../src/print.c"
{ int _au3_val ;

#line 1879 "../../src/print.c"
_au3_val = 68 ;
if (_au0_this -> _stmt__O8.__C8_e -> _node_base == 169 ){ 
#line 1881 "../../src/print.c"
Pname _au4_a ;
Pexpr _au4_arg ;

#line 1881 "../../src/print.c"
_au4_a = (((struct name *)_au0_this -> _stmt__O8.__C8_e ));
_au4_arg = aval ( _au4_a ) ;

#line 1884 "../../src/print.c"
if (_au4_arg )
#line 1885 "../../src/print.c"
switch (_au4_arg -> _node_base ){ 
#line 1886 "../../src/print.c"
case 86 : _au3_val = 0 ;

#line 1886 "../../src/print.c"
break ;
case 112 : 
#line 1888 "../../src/print.c"
case 145 : _au3_val = 1 ;

#line 1888 "../../src/print.c"
break ;
case 150 : _au3_val = (_au4_arg -> _expr__O3.__C3_i1 != 0 );
}
}

#line 1893 "../../src/print.c"
switch (_au3_val ){ 
#line 1894 "../../src/print.c"
case 1 : 
#line 1895 "../../src/print.c"
_stmt_print ( _au0_this -> _stmt_s ) ;
break ;
case 0 : 
#line 1898 "../../src/print.c"
if (_au0_this -> _stmt__O9.__C9_else_stmt )
#line 1899 "../../src/print.c"
_stmt_print ( _au0_this -> _stmt__O9.__C9_else_stmt ) ;
else 
#line 1901 "../../src/print.c"
puttok ( (unsigned char )72 ) ;
break ;
default : 
#line 1904 "../../src/print.c"
puttok ( (unsigned char )20 ) ;
putc('(', out_file);

#line 1905 "../../src/print.c"
_expr_print ( _au0_this -> _stmt__O8.__C8_e ) ;

#line 1905 "../../src/print.c"
putc(')', out_file);
if (_au0_this -> _stmt_s -> _stmt_s_list ){ 
#line 1907 "../../src/print.c"
puttok ( (unsigned char )73 ) ;
_stmt_print ( _au0_this -> _stmt_s ) ;
puttok ( (unsigned char )74 ) ;
}
else 
#line 1912 "../../src/print.c"
_stmt_print ( _au0_this -> _stmt_s ) ;
if (_au0_this -> _stmt__O9.__C9_else_stmt ){ 
#line 1914 "../../src/print.c"
puttok ( (unsigned char )12 ) ;
if (_au0_this -> _stmt__O9.__C9_else_stmt -> _stmt_s_list ){ 
#line 1916 "../../src/print.c"
puttok ( (unsigned char )73 ) ;
_stmt_print ( _au0_this -> _stmt__O9.__C9_else_stmt ) ;
puttok ( (unsigned char )74 ) ;
}
else 
#line 1921 "../../src/print.c"
_stmt_print ( _au0_this -> _stmt__O9.__C9_else_stmt ) ;
}
}
break ;
}

#line 1927 "../../src/print.c"
case 16 : 
#line 1928 "../../src/print.c"
{ int _au3_fi ;

#line 1928 "../../src/print.c"
_au3_fi = (_au0_this -> _stmt__O9.__C9_for_init && (((_au0_this -> _stmt__O9.__C9_for_init -> _node_base != 72 )|| _au0_this -> _stmt__O9.__C9_for_init -> _stmt_memtbl )|| _au0_this -> _stmt__O9.__C9_for_init -> _stmt_s_list ));

#line 1930 "../../src/print.c"
if (_au3_fi ){ 
#line 1931 "../../src/print.c"
puttok ( (unsigned char )73 ) ;
_stmt_print ( _au0_this -> _stmt__O9.__C9_for_init ) ;
}
fputs ( (char *)"for(", out_file ) ;
if ((_au3_fi == 0 )&& _au0_this -> _stmt__O9.__C9_for_init )_expr_print ( _au0_this -> _stmt__O9.__C9_for_init -> _stmt__O8.__C8_e ) ;
putc(';', out_file);
if (_au0_this -> _stmt__O8.__C8_e )_expr_print ( _au0_this -> _stmt__O8.__C8_e ) ;
putc(';', out_file);
if (_au0_this -> _stmt__O7.__C7_e2 )_expr_print ( _au0_this -> _stmt__O7.__C7_e2 ) ;
puttok ( (unsigned char )41 ) ;
_stmt_print ( _au0_this -> _stmt_s ) ;
if (_au3_fi )puttok ( (unsigned char )74 ) ;
break ;
}

#line 1946 "../../src/print.c"
case 166 : 
#line 1947 "../../src/print.c"
if (_au0_this -> _stmt_s && _au0_this -> _stmt__O8.__C8_s2 ){ 
#line 1948 "../../src/print.c"
puttok ( (unsigned char )73 ) ;
_stmt_print ( _au0_this -> _stmt_s ) ;
_stmt_print ( _au0_this -> _stmt__O8.__C8_s2 ) ;
puttok ( (unsigned char )74 ) ;
}
else { 
#line 1954 "../../src/print.c"
if (_au0_this -> _stmt_s )_stmt_print ( _au0_this -> _stmt_s ) ;
if (_au0_this -> _stmt__O8.__C8_s2 )_stmt_print ( _au0_this -> _stmt__O8.__C8_s2 ) ;
}
break ;

#line 1959 "../../src/print.c"
case 116 : 
#line 1960 "../../src/print.c"
puttok ( (unsigned char )73 ) ;
if (_au0_this -> _stmt__O7.__C7_d )_name_dcl_print ( _au0_this -> _stmt__O7.__C7_d , (unsigned char )72 ) ;
if (_au0_this -> _stmt_memtbl && _au0_this -> _stmt__O8.__C8_own_tbl ){ 
#line 1963 "../../src/print.c"
int _au3_i ;
{ Pname _au3_n ;

#line 1964 "../../src/print.c"
_au3_n = _table_get_mem ( _au0_this -> _stmt_memtbl , _au3_i = 1 ) ;

#line 1964 "../../src/print.c"
for(;_au3_n ;_au3_n = _table_get_mem ( _au0_this -> _stmt_memtbl , ++ _au3_i ) ) { 
#line 1965 "../../src/print.c"
if ((_au3_n -> _expr__O2.__C2_tp && (_au3_n -> _name_n_union == 0 ))&&
#line 1965 "../../src/print.c"
(_au3_n -> _expr__O2.__C2_tp != (struct type *)any_type ))
#line 1966 "../../src/print.c"
switch (_au3_n -> _name_n_scope ){ 
#line 1967 "../../src/print.c"
case 139 : 
#line 1968 "../../src/print.c"
case 136 : 
#line 1969 "../../src/print.c"
break ;
default : 
#line 1971 "../../src/print.c"
_name_dcl_print ( _au3_n , (unsigned char )0 ) ;
}
}
}
}

#line 1975 "../../src/print.c"
if (_au0_this -> _stmt_s )_stmt_print ( _au0_this -> _stmt_s ) ;
fputs ( (char *)"}\n", out_file ) ;
if (last_ll && _au0_this -> _stmt_where . _loc_line )last_line . _loc_line ++ ;
} }

#line 1980 "../../src/print.c"
if (_au0_this -> _stmt_s_list )_stmt_print ( _au0_this -> _stmt_s_list ) ;
}
;
char _table_dcl_print (_au0_this , _au0_s , _au0_pub )
#line 207 "../../src/cfront.h"
struct table *_au0_this ;

#line 1983 "../../src/print.c"
TOK _au0_s ;

#line 1983 "../../src/print.c"
TOK _au0_pub ;

#line 1988 "../../src/print.c"
{ 
#line 1989 "../../src/print.c"
register Pname *_au1_np ;
register int _au1_i ;

#line 1992 "../../src/print.c"
if (_au0_this == 0 )return ;

#line 1994 "../../src/print.c"
_au1_np = _au0_this -> _table_entries ;
for(_au1_i = 1 ;_au1_i < _au0_this -> _table_free_slot ;_au1_i ++ ) { 
#line 1996 "../../src/print.c"
register Pname _au2_n ;

#line 1996 "../../src/print.c"
_au2_n = (_au1_np [_au1_i ]);
switch (_au0_s ){ 
#line 1998 "../../src/print.c"
case 0 : 
#line 1999 "../../src/print.c"
_name_dcl_print ( _au2_n , (unsigned char )0 ) ;
break ;
case 62 : 
#line 2002 "../../src/print.c"
if (_au2_n -> _expr__O2.__C2_tp && (_au2_n -> _name_n_scope == _au0_pub ))_name_dcl_print ( _au2_n , (unsigned char )0 ) ;
break ;
case 63 : 
#line 2005 "../../src/print.c"
if (_au2_n -> _expr__O2.__C2_tp && (_au2_n -> _name_n_scope != _au0_pub ))_name_dcl_print ( _au2_n , (unsigned char )0 ) ;
break ;
}
}
}
;

/* the end */
