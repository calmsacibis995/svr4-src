/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#line 1 "../../src/typ.c"

/* <<cfront 1.2.1 2/16/87>> */
/* < ../../src/typ.c */

#ident	"@(#)sdb:cfront/scratch/src/typ..c	1.2"
char *_new(); char _delete(); char *_vec_new(); char _vec_delete();

#line 1 "../../src/typ.c"

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

#line 20 "../../src/typ.c"
Pbase short_type ;
Pbase int_type ;
Pbase char_type ;
Pbase long_type ;

#line 25 "../../src/typ.c"
Pbase uchar_type ;
Pbase ushort_type ;
Pbase uint_type ;
Pbase ulong_type ;

#line 30 "../../src/typ.c"
Pbase zero_type ;
Pbase float_type ;
Pbase double_type ;
Pbase void_type ;
Pbase any_type ;

#line 36 "../../src/typ.c"
Ptype Pint_type ;
Ptype Pchar_type ;
Ptype Pvoid_type ;
Ptype Pfctchar_type ;
Ptype Pfctvec_type ;

#line 42 "../../src/typ.c"
Ptable gtbl ;
Ptable any_tbl ;

#line 45 "../../src/typ.c"
Pname Cdcl = 0 ;
Pstmt Cstmt = 0 ;

#line 48 "../../src/typ.c"
extern int suppress_error ;

#line 50 "../../src/typ.c"
bit new_type = 0 ;

#line 52 "../../src/typ.c"
extern Ptype np_promote ();
extern Ptype np_promote (_au0_oper , _au0_r1 , _au0_r2 , _au0_t1 , _au0_t2 , _au0_p )TOK _au0_oper ;

#line 53 "../../src/typ.c"
TOK _au0_r1 ;

#line 53 "../../src/typ.c"
TOK _au0_r2 ;

#line 53 "../../src/typ.c"
Ptype _au0_t1 ;

#line 53 "../../src/typ.c"
Ptype _au0_t2 ;

#line 53 "../../src/typ.c"
TOK _au0_p ;

#line 66 "../../src/typ.c"
{ 
#line 67 "../../src/typ.c"
if (_au0_r2 == 'A' )return _au0_t1 ;

#line 69 "../../src/typ.c"
switch (_au0_r1 ){ 
#line 70 "../../src/typ.c"
case 'A' : return _au0_t2 ;
case 'Z' : 
#line 72 "../../src/typ.c"
switch (_au0_r2 ){ 
#line 73 "../../src/typ.c"
case 'Z' : return (struct type *)int_type ;
case 'I' : 
#line 75 "../../src/typ.c"
case 'F' : return (struct type *)(_au0_p ? _basetype_arit_conv ( ((struct basetype *)_au0_t2 ), (struct basetype *)0 ) : (((struct
#line 75 "../../src/typ.c"
basetype *)0 )));
case 'P' : return _au0_t2 ;
default : { 
#line 147 "../../src/typ.c"
struct ea _au0__V10 ;

#line 77 "../../src/typ.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"zero(%d)", (struct ea *)( ( ((& _au0__V10 )-> _ea__O1.__C1_i = ((int )_au0_r2 )), (((&
#line 77 "../../src/typ.c"
_au0__V10 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
case 'I' : 
#line 80 "../../src/typ.c"
switch (_au0_r2 ){ 
#line 81 "../../src/typ.c"
case 'Z' : _au0_t2 = 0 ;
case 'I' : 
#line 83 "../../src/typ.c"
case 'F' : return (struct type *)(_au0_p ? _basetype_arit_conv ( ((struct basetype *)_au0_t1 ), ((struct basetype *)_au0_t2 )) : (((struct
#line 83 "../../src/typ.c"
basetype *)0 )));
case 'P' : switch (_au0_oper ){ 
#line 85 "../../src/typ.c"
case 54 : 
#line 86 "../../src/typ.c"
case 126 : break ;
default : { 
#line 147 "../../src/typ.c"
struct ea _au0__V11 ;

#line 87 "../../src/typ.c"
error ( (char *)"int%kP", (struct ea *)( ( ((& _au0__V11 )-> _ea__O1.__C1_i = ((int )_au0_oper )), (((& _au0__V11 )))) )
#line 87 "../../src/typ.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;

#line 87 "../../src/typ.c"
return (struct type *)any_type ;
} }
return _au0_t2 ;
case 108 : { 
#line 147 "../../src/typ.c"
struct ea _au0__V12 ;

#line 90 "../../src/typ.c"
error ( (char *)"int%kF", (struct ea *)( ( ((& _au0__V12 )-> _ea__O1.__C1_i = ((int )_au0_oper )), (((& _au0__V12 )))) )
#line 90 "../../src/typ.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;

#line 90 "../../src/typ.c"
return (struct type *)any_type ;
default : { 
#line 147 "../../src/typ.c"
struct ea _au0__V13 ;

#line 91 "../../src/typ.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"int(%d)", (struct ea *)( ( ((& _au0__V13 )-> _ea__O1.__C1_i = ((int )_au0_r2 )), (((&
#line 91 "../../src/typ.c"
_au0__V13 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} } }
case 'F' : 
#line 94 "../../src/typ.c"
switch (_au0_r2 ){ 
#line 95 "../../src/typ.c"
case 'Z' : _au0_t2 = 0 ;
case 'I' : 
#line 97 "../../src/typ.c"
case 'F' : return (struct type *)(_au0_p ? _basetype_arit_conv ( ((struct basetype *)_au0_t1 ), ((struct basetype *)_au0_t2 )) : (((struct
#line 97 "../../src/typ.c"
basetype *)0 )));
case 'P' : { 
#line 147 "../../src/typ.c"
struct ea _au0__V14 ;

#line 98 "../../src/typ.c"
error ( (char *)"float%kP", (struct ea *)( ( ((& _au0__V14 )-> _ea__O1.__C1_i = ((int )_au0_oper )), (((& _au0__V14 )))) )
#line 98 "../../src/typ.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;

#line 98 "../../src/typ.c"
return (struct type *)any_type ;
case 108 : { 
#line 147 "../../src/typ.c"
struct ea _au0__V15 ;

#line 99 "../../src/typ.c"
error ( (char *)"float%kF", (struct ea *)( ( ((& _au0__V15 )-> _ea__O1.__C1_i = ((int )_au0_oper )), (((& _au0__V15 )))) )
#line 99 "../../src/typ.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;

#line 99 "../../src/typ.c"
return (struct type *)any_type ;
default : { 
#line 147 "../../src/typ.c"
struct ea _au0__V16 ;

#line 100 "../../src/typ.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"float(%d)", (struct ea *)( ( ((& _au0__V16 )-> _ea__O1.__C1_i = ((int )_au0_r2 )), (((&
#line 100 "../../src/typ.c"
_au0__V16 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} } } }
case 'P' : 
#line 103 "../../src/typ.c"
switch (_au0_r2 ){ 
#line 104 "../../src/typ.c"
case 'Z' : return _au0_t1 ;
case 'I' : 
#line 106 "../../src/typ.c"
switch (_au0_oper ){ 
#line 107 "../../src/typ.c"
case 54 : 
#line 108 "../../src/typ.c"
case 55 : 
#line 109 "../../src/typ.c"
case 126 : 
#line 110 "../../src/typ.c"
case 127 : break ;
#line 110 "../../src/typ.c"

#line 111 "../../src/typ.c"
default : { 
#line 147 "../../src/typ.c"
struct ea _au0__V17 ;

#line 111 "../../src/typ.c"
error ( (char *)"P%k int", (struct ea *)( ( ((& _au0__V17 )-> _ea__O1.__C1_i = ((int )_au0_oper )), (((& _au0__V17 )))) )
#line 111 "../../src/typ.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;

#line 111 "../../src/typ.c"
return (struct type *)any_type ;
} }
return _au0_t1 ;
case 'F' : { 
#line 147 "../../src/typ.c"
struct ea _au0__V18 ;

#line 114 "../../src/typ.c"
error ( (char *)"P%k float", (struct ea *)( ( ((& _au0__V18 )-> _ea__O1.__C1_i = ((int )_au0_oper )), (((& _au0__V18 )))) )
#line 114 "../../src/typ.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;

#line 114 "../../src/typ.c"
return (struct type *)any_type ;
case 'P' : 
#line 116 "../../src/typ.c"
if (_type_check ( _au0_t1 , _au0_t2 , (unsigned char )70 ) ){ 
#line 117 "../../src/typ.c"
switch (_au0_oper ){ 
#line 118 "../../src/typ.c"
case 62 :
#line 118 "../../src/typ.c"

#line 119 "../../src/typ.c"
case 63 : 
#line 120 "../../src/typ.c"
case 59 : 
#line 121 "../../src/typ.c"
case 61 : 
#line 122 "../../src/typ.c"
case 60 : 
#line 123 "../../src/typ.c"
case 58 : 
#line 124 "../../src/typ.c"
case 68 : 
#line 125 "../../src/typ.c"
if (_type_check (
#line 125 "../../src/typ.c"
_au0_t2 , _au0_t1 , (unsigned char )70 ) == 0 )goto zz ;
}
{ 
#line 147 "../../src/typ.c"
struct ea _au0__V19 ;

#line 147 "../../src/typ.c"
struct ea _au0__V20 ;

#line 147 "../../src/typ.c"
struct ea _au0__V21 ;

#line 127 "../../src/typ.c"
error ( (char *)"T mismatch:%t %k%t", (struct ea *)( ( ((& _au0__V19 )-> _ea__O1.__C1_p = ((char *)_au0_t1 )), (((& _au0__V19 )))) )
#line 127 "../../src/typ.c"
, (struct ea *)( ( ((& _au0__V20 )-> _ea__O1.__C1_i = ((int )_au0_oper )), (((& _au0__V20 )))) ) , (struct
#line 127 "../../src/typ.c"
ea *)( ( ((& _au0__V21 )-> _ea__O1.__C1_p = ((char *)_au0_t2 )), (((& _au0__V21 )))) ) , (struct ea *)ea0 ) ;
#line 127 "../../src/typ.c"

#line 128 "../../src/typ.c"
return (struct type *)any_type ;
} }
zz :
#line 131 "../../src/typ.c"
switch (_au0_oper ){ 
#line 132 "../../src/typ.c"
case 55 : 
#line 133 "../../src/typ.c"
case 127 : return (struct type *)int_type ;
case 54 : 
#line 135 "../../src/typ.c"
case 126 : error ( (char *)"P +P", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 135 "../../src/typ.c"
ea *)ea0 ) ;

#line 135 "../../src/typ.c"
return (struct type *)any_type ;
default : return _au0_t1 ;
}
case 108 : return _au0_t1 ;
default : { 
#line 147 "../../src/typ.c"
struct ea _au0__V22 ;

#line 139 "../../src/typ.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"P(%d)", (struct ea *)( ( ((& _au0__V22 )-> _ea__O1.__C1_i = ((int )_au0_r2 )), (((&
#line 139 "../../src/typ.c"
_au0__V22 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} } }
case 108 : 
#line 142 "../../src/typ.c"
{ 
#line 147 "../../src/typ.c"
struct ea _au0__V23 ;

#line 147 "../../src/typ.c"
struct ea _au0__V24 ;

#line 142 "../../src/typ.c"
error ( (char *)"F%k%t", (struct ea *)( ( ((& _au0__V23 )-> _ea__O1.__C1_i = ((int )_au0_oper )), (((& _au0__V23 )))) )
#line 142 "../../src/typ.c"
, (struct ea *)( ( ((& _au0__V24 )-> _ea__O1.__C1_p = ((char *)_au0_t2 )), (((& _au0__V24 )))) ) , (struct
#line 142 "../../src/typ.c"
ea *)ea0 , (struct ea *)ea0 ) ;
return (struct type *)any_type ;
default : 
#line 145 "../../src/typ.c"
{ 
#line 147 "../../src/typ.c"
struct ea _au0__V25 ;

#line 147 "../../src/typ.c"
struct ea _au0__V26 ;

#line 145 "../../src/typ.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"np_promote(%d,%d)", (struct ea *)( ( ((& _au0__V25 )-> _ea__O1.__C1_i = ((int )_au0_r1 )), (((&
#line 145 "../../src/typ.c"
_au0__V25 )))) ) , (struct ea *)( ( ((& _au0__V26 )-> _ea__O1.__C1_i = ((int )_au0_r2 )), (((& _au0__V26 )))) )
#line 145 "../../src/typ.c"
, (struct ea *)ea0 , (struct ea *)ea0 ) ;
} } }
}
;
TOK _type_kind (_au0_this , _au0_oper , _au0_v )
#line 263 "../../src/cfront.h"
struct type *_au0_this ;

#line 149 "../../src/typ.c"
TOK _au0_oper ;

#line 149 "../../src/typ.c"
TOK _au0_v ;

#line 154 "../../src/typ.c"
{ 
#line 155 "../../src/typ.c"
Ptype _au1_t ;

#line 155 "../../src/typ.c"
_au1_t = _au0_this ;
xx :
#line 157 "../../src/typ.c"
switch (_au1_t -> _node_base ){ 
#line 158 "../../src/typ.c"
case 141 : return (unsigned char )'A' ;
case 138 : return (unsigned char )'Z' ;
case 114 : 
#line 161 "../../src/typ.c"
case 5 : 
#line 162 "../../src/typ.c"
case 29 : 
#line 163 "../../src/typ.c"
case 21 : 
#line 164 "../../src/typ.c"
case 22 : 
#line 165 "../../src/typ.c"
case 121 : return (unsigned
#line 165 "../../src/typ.c"
char )'I' ;
case 15 : 
#line 167 "../../src/typ.c"
case 11 : if (_au0_v == 'I' ){ 
#line 208 "../../src/typ.c"
struct ea _au0__V27 ;

#line 167 "../../src/typ.c"
error ( (char *)"float operand for %k", (struct ea *)( ( ((& _au0__V27 )-> _ea__O1.__C1_i = ((int )_au0_oper )), (((& _au0__V27 )))) )
#line 167 "../../src/typ.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} 
#line 167 "../../src/typ.c"
return (unsigned char )'F' ;
case 125 : if (_au0_v != 'P' ){ 
#line 208 "../../src/typ.c"
struct ea _au0__V28 ;

#line 168 "../../src/typ.c"
error ( (char *)"P operand for %k", (struct ea *)( ( ((& _au0__V28 )-> _ea__O1.__C1_i = ((int )_au0_oper )), (((& _au0__V28 )))) )
#line 168 "../../src/typ.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} switch (_au0_oper ){ 
#line 170 "../../src/typ.c"
case 48 : 
#line 171 "../../src/typ.c"
case 49 : 
#line 172 "../../src/typ.c"
case 55 : 
#line 173 "../../src/typ.c"
case 54 : 
#line 174 "../../src/typ.c"
case 127 : 
#line 175 "../../src/typ.c"
case
#line 175 "../../src/typ.c"
126 : 
#line 176 "../../src/typ.c"
if ((((struct ptr *)_au1_t ))-> _ptr_memof || ((((struct ptr *)_au1_t ))-> _pvtyp_typ -> _node_base == 108 ))
#line 177 "../../src/typ.c"
{ 
#line 208 "../../src/typ.c"
struct ea _au0__V29 ;

#line 208 "../../src/typ.c"
struct ea _au0__V30 ;

#line 177 "../../src/typ.c"
error ( (char *)"%t operand of%k", (struct ea *)( ( ((& _au0__V29 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V29 )))) )
#line 177 "../../src/typ.c"
, (struct ea *)( ( ((& _au0__V30 )-> _ea__O1.__C1_i = ((int )_au0_oper )), (((& _au0__V30 )))) ) , (struct
#line 177 "../../src/typ.c"
ea *)ea0 , (struct ea *)ea0 ) ;
} else 
#line 179 "../../src/typ.c"
_type_tsizeof ( (((struct ptr *)_au1_t ))-> _pvtyp_typ ) ;
break ;
default : 
#line 182 "../../src/typ.c"
if ((((struct ptr *)_au1_t ))-> _ptr_memof || ((((struct ptr *)_au1_t ))-> _pvtyp_typ -> _node_base == 108 ))
#line 183 "../../src/typ.c"
{ 
#line 208 "../../src/typ.c"
struct ea _au0__V31 ;

#line 208 "../../src/typ.c"
struct ea _au0__V32 ;

#line 183 "../../src/typ.c"
error ( (char *)"%t operand of%k", (struct ea *)( ( ((& _au0__V31 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V31 )))) )
#line 183 "../../src/typ.c"
, (struct ea *)( ( ((& _au0__V32 )-> _ea__O1.__C1_i = ((int )_au0_oper )), (((& _au0__V32 )))) ) , (struct
#line 183 "../../src/typ.c"
ea *)ea0 , (struct ea *)ea0 ) ;
} case 66 : 
#line 185 "../../src/typ.c"
case 67 : 
#line 186 "../../src/typ.c"
case 70 : 
#line 187 "../../src/typ.c"
case 63 : 
#line 188 "../../src/typ.c"
case 62 : 
#line 189 "../../src/typ.c"
case 20 : 
#line 190 "../../src/typ.c"
case
#line 190 "../../src/typ.c"
39 : 
#line 191 "../../src/typ.c"
case 10 : 
#line 192 "../../src/typ.c"
case 16 : 
#line 193 "../../src/typ.c"
case 68 : 
#line 194 "../../src/typ.c"
case 46 : 
#line 195 "../../src/typ.c"
break ;
}

#line 198 "../../src/typ.c"
return (unsigned char )'P' ;
case 158 : { 
#line 208 "../../src/typ.c"
struct ea _au0__V33 ;

#line 199 "../../src/typ.c"
error ( (char *)"R operand for %k", (struct ea *)( ( ((& _au0__V33 )-> _ea__O1.__C1_i = ((int )_au0_oper )), (((& _au0__V33 )))) )
#line 199 "../../src/typ.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
return (unsigned char )'A' ;
case 110 : if (_au0_v != 'P' ){ 
#line 208 "../../src/typ.c"
struct ea _au0__V34 ;

#line 201 "../../src/typ.c"
error ( (char *)"V operand for %k", (struct ea *)( ( ((& _au0__V34 )-> _ea__O1.__C1_i = ((int )_au0_oper )), (((& _au0__V34 )))) )
#line 201 "../../src/typ.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} 
#line 201 "../../src/typ.c"
return (unsigned char )'P' ;
case 97 : _au1_t = (((struct basetype *)_au1_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 202 "../../src/typ.c"
goto xx ;
case 108 : if (_au0_v != 'P' ){ 
#line 208 "../../src/typ.c"
struct ea _au0__V35 ;

#line 203 "../../src/typ.c"
error ( (char *)"F operand for %k", (struct ea *)( ( ((& _au0__V35 )-> _ea__O1.__C1_i = ((int )_au0_oper )), (((& _au0__V35 )))) )
#line 203 "../../src/typ.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} 
#line 203 "../../src/typ.c"
return (unsigned char )108 ;
case 6 : 
#line 205 "../../src/typ.c"
case 13 : { 
#line 208 "../../src/typ.c"
struct ea _au0__V36 ;

#line 208 "../../src/typ.c"
struct ea _au0__V37 ;

#line 205 "../../src/typ.c"
error ( (char *)"%k operand for %k", (struct ea *)( ( ((& _au0__V36 )-> _ea__O1.__C1_i = ((int )_au0_this -> _node_base )), (((& _au0__V36 ))))
#line 205 "../../src/typ.c"
) , (struct ea *)( ( ((& _au0__V37 )-> _ea__O1.__C1_i = ((int )_au0_oper )), (((& _au0__V37 )))) ) ,
#line 205 "../../src/typ.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;

#line 205 "../../src/typ.c"
return (unsigned char )'A' ;
default : { 
#line 208 "../../src/typ.c"
struct ea _au0__V38 ;

#line 208 "../../src/typ.c"
struct ea _au0__V39 ;

#line 206 "../../src/typ.c"
error ( (char *)"%t operand for %k", (struct ea *)( ( ((& _au0__V38 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V38 )))) )
#line 206 "../../src/typ.c"
, (struct ea *)( ( ((& _au0__V39 )-> _ea__O1.__C1_i = ((int )_au0_oper )), (((& _au0__V39 )))) ) , (struct
#line 206 "../../src/typ.c"
ea *)ea0 , (struct ea *)ea0 ) ;

#line 206 "../../src/typ.c"
return (unsigned char )'A' ;
} } } }
}
;
char _type_dcl (_au0_this , _au0_tbl )
#line 263 "../../src/cfront.h"
struct type *_au0_this ;

#line 210 "../../src/typ.c"
Ptable _au0_tbl ;

#line 218 "../../src/typ.c"
{ 
#line 219 "../../src/typ.c"
Ptype _au1_t ;

#line 219 "../../src/typ.c"
_au1_t = _au0_this ;

#line 221 "../../src/typ.c"
if (_au0_this == 0 )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"T::dcl(this==0)", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 221 "../../src/typ.c"
ea *)ea0 ) ;
if (_au0_tbl -> _node_base != 142 ){ 
#line 327 "../../src/typ.c"
struct ea _au0__V40 ;

#line 222 "../../src/typ.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"T::dcl(%d)", (struct ea *)( ( ((& _au0__V40 )-> _ea__O1.__C1_i = ((int )_au0_tbl -> _node_base )),
#line 222 "../../src/typ.c"
(((& _au0__V40 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} 
#line 224 "../../src/typ.c"
xx :
#line 225 "../../src/typ.c"
switch (_au1_t -> _node_base ){ 
#line 226 "../../src/typ.c"
case 125 : 
#line 227 "../../src/typ.c"
case 158 : 
#line 228 "../../src/typ.c"
{ Pptr _au3_p ;

#line 228 "../../src/typ.c"
_au3_p = (((struct ptr *)_au1_t ));
_au1_t = _au3_p -> _pvtyp_typ ;
if (_au1_t -> _node_base == 97 ){ 
#line 231 "../../src/typ.c"
Ptype _au4_tt ;

#line 231 "../../src/typ.c"
_au4_tt = (((struct basetype *)_au1_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
if (_au4_tt -> _node_base == 108 )_au3_p -> _pvtyp_typ = _au4_tt ;
return ;
}
goto xx ;
}

#line 238 "../../src/typ.c"
case 110 : 
#line 239 "../../src/typ.c"
{ Pvec _au3_v ;
Pexpr _au3_e ;

#line 239 "../../src/typ.c"
_au3_v = (((struct vec *)_au1_t ));
_au3_e = _au3_v -> _vec_dim ;
if (_au3_e ){ 
#line 242 "../../src/typ.c"
Ptype _au4_et ;
_au3_v -> _vec_dim = (_au3_e = _expr_typ ( _au3_e , _au0_tbl ) );
_au4_et = _au3_e -> _expr__O2.__C2_tp ;
if (( _type_kind ( _au4_et , ((unsigned char )0 ), (unsigned char )'I' ) ) == 'A' ){ 
#line 246 "../../src/typ.c"
error (
#line 246 "../../src/typ.c"
(char *)"UN in array dimension", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
}
else if (! new_type ){ 
#line 249 "../../src/typ.c"
int _au5_i ;
Neval = 0 ;
_au5_i = _expr_eval ( _au3_e ) ;
if (Neval ){ 
#line 327 "../../src/typ.c"
struct ea _au0__V41 ;

#line 252 "../../src/typ.c"
error ( (char *)"%s", (struct ea *)( ( ((& _au0__V41 )-> _ea__O1.__C1_p = ((char *)Neval )), (((& _au0__V41 )))) )
#line 252 "../../src/typ.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} else if (_au5_i == 0 )
#line 254 "../../src/typ.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"array dimension == 0", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 254 "../../src/typ.c"
ea *)ea0 , (struct ea *)ea0 ) ;
else if (_au5_i < 0 ){ 
#line 256 "../../src/typ.c"
error ( (char *)"negative array dimension", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 256 "../../src/typ.c"
ea *)ea0 ) ;
_au5_i = 1 ;
}
_au3_v -> _vec_size = _au5_i ;
if (_au3_v -> _vec_dim && (_au3_v -> _vec_dim -> _node_permanent == 0 ))_expr_del ( _au3_v -> _vec_dim ) ;
_au3_v -> _vec_dim = 0 ;
}
}
_au1_t = _au3_v -> _pvtyp_typ ;
llx :
#line 266 "../../src/typ.c"
switch (_au1_t -> _node_base ){ 
#line 267 "../../src/typ.c"
case 97 : 
#line 268 "../../src/typ.c"
_au1_t = (((struct basetype *)_au1_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
goto llx ;
case 108 : 
#line 271 "../../src/typ.c"
_au3_v -> _pvtyp_typ = _au1_t ;
break ;
case 110 : 
#line 274 "../../src/typ.c"
if (((((struct vec *)_au1_t ))-> _vec_dim == 0 )&& ((((struct vec *)_au1_t ))-> _vec_size == 0 ))error ( (char *)"null dimension (something like [][] seen)", (struct
#line 274 "../../src/typ.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
}
goto xx ;
}

#line 279 "../../src/typ.c"
case 108 : 
#line 280 "../../src/typ.c"
{ Pfct _au3_f ;

#line 280 "../../src/typ.c"
_au3_f = (((struct fct *)_au1_t ));
{ Pname _au3_n ;

#line 281 "../../src/typ.c"
_au3_n = _au3_f -> _fct_argtype ;

#line 281 "../../src/typ.c"
for(;_au3_n ;_au3_n = _au3_n -> _name_n_list ) _type_dcl ( _au3_n -> _expr__O2.__C2_tp , _au0_tbl ) ;
{ Pname _au3_cn ;

#line 282 "../../src/typ.c"
_au3_cn = _type_is_cl_obj ( _au3_f -> _fct_returns ) ;
if (_au3_cn && ( (((struct classdef *)_au3_cn -> _expr__O2.__C2_tp ))-> _classdef_itor ) ){ 
#line 284 "../../src/typ.c"
Pname _au4_rv ;

#line 285 "../../src/typ.c"
struct ptr *_au0__Xthis__ctor_ptr ;

#line 284 "../../src/typ.c"
_au4_rv = (struct name *)_name__ctor ( (struct name *)0 , "_result") ;
_au4_rv -> _expr__O2.__C2_tp = (struct type *)( (_au0__Xthis__ctor_ptr = 0 ), ( (_au0__Xthis__ctor_ptr = (struct ptr *)_new ( (long )(sizeof (struct ptr))) ),
#line 285 "../../src/typ.c"
( (Nt ++ ), ( (_au0__Xthis__ctor_ptr -> _node_base = ((unsigned char )125 )), ( (_au0__Xthis__ctor_ptr -> _pvtyp_typ = _au3_f -> _fct_returns ),
#line 285 "../../src/typ.c"
( (_au0__Xthis__ctor_ptr -> _ptr_rdo = ((char )0 )), ((_au0__Xthis__ctor_ptr ))) ) ) ) ) ) ;
_au4_rv -> _name_n_scope = 108 ;
_au4_rv -> _name_n_used = 1 ;
_au4_rv -> _name_n_list = _au3_f -> _fct_argtype ;
if (_au3_f -> _fct_f_this )_au3_f -> _fct_f_this -> _name_n_list = _au4_rv ;
_au3_f -> _fct_f_result = _au4_rv ;
}
_au1_t = _au3_f -> _fct_returns ;
goto xx ;
}
}
}

#line 296 "../../src/typ.c"
case 114 : 
#line 297 "../../src/typ.c"
{ Pbase _au3_f ;
Pexpr _au3_e ;
int _au3_i ;
Ptype _au3_et ;

#line 297 "../../src/typ.c"
_au3_f = (((struct basetype *)_au1_t ));
_au3_e = (((struct expr *)_au3_f -> _basetype_b_name ));

#line 301 "../../src/typ.c"
_au3_e = _expr_typ ( _au3_e , _au0_tbl ) ;
_au3_f -> _basetype_b_name = (((struct name *)_au3_e ));
_au3_et = _au3_e -> _expr__O2.__C2_tp ;
if (( _type_kind ( _au3_et , ((unsigned char )0 ), (unsigned char )'I' ) ) == 'A' ){ 
#line 305 "../../src/typ.c"
error (
#line 305 "../../src/typ.c"
(char *)"UN in field size", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
_au3_i = 1 ;
}
else { 
#line 309 "../../src/typ.c"
Neval = 0 ;
_au3_i = _expr_eval ( _au3_e ) ;
if (Neval )
#line 312 "../../src/typ.c"
{ 
#line 327 "../../src/typ.c"
struct ea _au0__V42 ;

#line 312 "../../src/typ.c"
error ( (char *)"%s", (struct ea *)( ( ((& _au0__V42 )-> _ea__O1.__C1_p = ((char *)Neval )), (((& _au0__V42 )))) )
#line 312 "../../src/typ.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} else if (_au3_i < 0 ){ 
#line 314 "../../src/typ.c"
error ( (char *)"negative field size", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ,
#line 314 "../../src/typ.c"
(struct ea *)ea0 ) ;
_au3_i = 1 ;
}
else if ((_type_tsizeof ( _au3_f -> _basetype_b_fieldtype ) * BI_IN_BYTE )< _au3_i )
#line 318 "../../src/typ.c"
{ 
#line 327 "../../src/typ.c"
struct ea _au0__V43 ;

#line 318 "../../src/typ.c"
error ( (char *)"field size > sizeof(%t)", (struct ea *)( ( ((& _au0__V43 )-> _ea__O1.__C1_p = ((char *)_au3_f -> _basetype_b_fieldtype )), (((& _au0__V43 ))))
#line 318 "../../src/typ.c"
) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} if (_au3_e && (_au3_e -> _node_permanent == 0 ))_expr_del ( _au3_e ) ;
}
_au3_f -> _basetype_b_bits = _au3_i ;
_au3_f -> _basetype_b_name = 0 ;
break ;
}
}
}
;

#line 329 "../../src/typ.c"
bit vrp_equiv ;
bit const_problem ;
extern int t_const ;

#line 333 "../../src/typ.c"
bit _type_check (_au0_this , _au0_t , _au0_oper )
#line 263 "../../src/cfront.h"
struct type *_au0_this ;

#line 333 "../../src/typ.c"
Ptype _au0_t ;

#line 333 "../../src/typ.c"
TOK _au0_oper ;

#line 349 "../../src/typ.c"
{ 
#line 350 "../../src/typ.c"
Ptype _au1_t1 ;
Ptype _au1_t2 ;
TOK _au1_b1 ;

#line 352 "../../src/typ.c"
TOK _au1_b2 ;
bit _au1_t2_const ;

#line 353 "../../src/typ.c"
bit _au1_t1_const ;
bit _au1_first ;
TOK _au1_r ;

#line 350 "../../src/typ.c"
_au1_t1 = _au0_this ;
_au1_t2 = _au0_t ;

#line 353 "../../src/typ.c"
_au1_t2_const = 0 ;

#line 353 "../../src/typ.c"
_au1_t1_const = t_const ;

#line 353 "../../src/typ.c"
t_const = 0 ;
_au1_first = 1 ;

#line 354 "../../src/typ.c"
;

#line 357 "../../src/typ.c"
if ((_au1_t1 == 0 )|| (_au1_t2 == 0 )){ 
#line 788 "../../src/typ.c"
struct ea _au0__V44 ;

#line 788 "../../src/typ.c"
struct ea _au0__V45 ;

#line 788 "../../src/typ.c"
struct ea _au0__V46 ;

#line 357 "../../src/typ.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"check(%p,%p,%d)", (struct ea *)( ( ((& _au0__V44 )-> _ea__O1.__C1_p = ((char *)_au1_t1 )), (((&
#line 357 "../../src/typ.c"
_au0__V44 )))) ) , (struct ea *)( ( ((& _au0__V45 )-> _ea__O1.__C1_p = ((char *)_au1_t2 )), (((& _au0__V45 )))) )
#line 357 "../../src/typ.c"
, (struct ea *)( ( ((& _au0__V46 )-> _ea__O1.__C1_i = ((int )_au0_oper )), (((& _au0__V46 )))) ) , (struct
#line 357 "../../src/typ.c"
ea *)ea0 ) ;
} 
#line 359 "../../src/typ.c"
vrp_equiv = 0 ;
const_problem = 0 ;

#line 362 "../../src/typ.c"
while (_au1_t1 && _au1_t2 ){ 
#line 363 "../../src/typ.c"
top :
#line 365 "../../src/typ.c"
if (_au1_t1 == _au1_t2 )return (char )0 ;
if ((_au1_t1 -> _node_base == 141 )|| (_au1_t2 -> _node_base == 141 ))return (char )0 ;

#line 368 "../../src/typ.c"
_au1_b1 = _au1_t1 -> _node_base ;
if (_au1_b1 == 97 ){ 
#line 370 "../../src/typ.c"
if (! _au0_oper )_au1_t1_const = (((struct basetype *)_au1_t1 ))-> _basetype_b_const ;
_au1_t1 = (((struct basetype *)_au1_t1 ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
goto top ;
}

#line 375 "../../src/typ.c"
_au1_b2 = _au1_t2 -> _node_base ;
if (_au1_b2 == 97 ){ 
#line 377 "../../src/typ.c"
if (! _au0_oper )_au1_t2_const = (((struct basetype *)_au1_t2 ))-> _basetype_b_const ;
_au1_t2 = (((struct basetype *)_au1_t2 ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
goto top ;
}

#line 382 "../../src/typ.c"
if (_au1_b1 != _au1_b2 ){ 
#line 383 "../../src/typ.c"
switch (_au1_b1 ){ 
#line 384 "../../src/typ.c"
case 125 : 
#line 385 "../../src/typ.c"
vrp_equiv = 1 ;
switch (_au1_b2 ){ 
#line 387 "../../src/typ.c"
case 110 : 
#line 388 "../../src/typ.c"
_au1_t1 = (((struct ptr *)_au1_t1 ))-> _pvtyp_typ ;
_au1_t2 = (((struct vec *)_au1_t2 ))-> _pvtyp_typ ;
_au1_first = 0 ;
goto top ;
case 108 : 
#line 393 "../../src/typ.c"
_au1_t1 = (((struct ptr *)_au1_t1 ))-> _pvtyp_typ ;
if ((_au1_first == 0 )|| (_au1_t1 -> _node_base != _au1_b2 ))return (char )1 ;
_au1_first = 0 ;
goto top ;
}
_au1_first = 0 ;
break ;
case 110 : 
#line 401 "../../src/typ.c"
vrp_equiv = 1 ;
_au1_first = 0 ;
switch (_au1_b2 ){ 
#line 404 "../../src/typ.c"
case 125 : 
#line 405 "../../src/typ.c"
switch (_au0_oper ){ 
#line 406 "../../src/typ.c"
case 0 : 
#line 407 "../../src/typ.c"
case 136 : 
#line 408 "../../src/typ.c"
case 70 : 
#line 409 "../../src/typ.c"
case 78 :
#line 409 "../../src/typ.c"

#line 410 "../../src/typ.c"
break ;
case 76 : 
#line 412 "../../src/typ.c"
default : 
#line 413 "../../src/typ.c"
return (char )1 ;
}
_au1_t1 = (((struct vec *)_au1_t1 ))-> _pvtyp_typ ;
_au1_t2 = (((struct ptr *)_au1_t2 ))-> _pvtyp_typ ;
goto top ;
}
break ;
}
goto base_check ;
}

#line 424 "../../src/typ.c"
switch (_au1_b1 ){ 
#line 425 "../../src/typ.c"
case 110 : 
#line 426 "../../src/typ.c"
_au1_first = 0 ;
{ Pvec _au4_v1 ;
Pvec _au4_v2 ;

#line 427 "../../src/typ.c"
_au4_v1 = (((struct vec *)_au1_t1 ));
_au4_v2 = (((struct vec *)_au1_t2 ));
if (_au4_v1 -> _vec_size != _au4_v2 -> _vec_size )
#line 430 "../../src/typ.c"
switch (_au0_oper ){ 
#line 431 "../../src/typ.c"
case 76 : 
#line 432 "../../src/typ.c"
case 78 : 
#line 433 "../../src/typ.c"
return (char )1 ;
}
_au1_t1 = _au4_v1 -> _pvtyp_typ ;
_au1_t2 = _au4_v2 -> _pvtyp_typ ;
}
break ;

#line 440 "../../src/typ.c"
case 125 : 
#line 441 "../../src/typ.c"
case 158 : 
#line 442 "../../src/typ.c"
_au1_first = 0 ;
{ Pptr _au4_p1 ;
Pptr _au4_p2 ;

#line 443 "../../src/typ.c"
_au4_p1 = (((struct ptr *)_au1_t1 ));
_au4_p2 = (((struct ptr *)_au1_t2 ));
if (_au4_p1 -> _ptr_memof != _au4_p2 -> _ptr_memof ){ 
#line 460 "../../src/typ.c"
if (_classdef_baseofFPCclassdef___ ( _au4_p2 -> _ptr_memof , _au4_p1 -> _ptr_memof ) )return (char )1 ;
#line 460 "../../src/typ.c"

#line 461 "../../src/typ.c"
Nstd ++ ;
}
_au1_t1 = _au4_p1 -> _pvtyp_typ ;
_au1_t2 = _au4_p2 -> _pvtyp_typ ;
if ((_au0_oper == 136 )&& (_au1_t1 -> _node_base == 97 ))
#line 466 "../../src/typ.c"
_au1_t1_const = (((struct basetype *)_au4_p1 -> _pvtyp_typ ))-> _basetype_b_const ;
if ((_au0_oper == 136 )&& (_au1_t2 -> _node_base == 97 ))
#line 468 "../../src/typ.c"
_au1_t2_const = (((struct basetype *)_au4_p2 -> _pvtyp_typ ))-> _basetype_b_const ;

#line 470 "../../src/typ.c"
if (_au0_oper == 0 ){ 
#line 471 "../../src/typ.c"
if (_au4_p1 -> _ptr_rdo != _au4_p2 -> _ptr_rdo ){ 
#line 472 "../../src/typ.c"
const_problem = 1 ;
return (char )1 ;
}
if ((_au1_b1 == 158 )&& (_type_tconst ( _au1_t1 ) != _type_tconst ( _au1_t2 ) ))
#line 476 "../../src/typ.c"
const_problem = 1 ;
}
break ;
}

#line 481 "../../src/typ.c"
case 108 : 
#line 482 "../../src/typ.c"
_au1_first = 0 ;
{ Pfct _au4_f1 ;
Pfct _au4_f2 ;
Pname _au4_a1 ;
Pname _au4_a2 ;
TOK _au4_k1 ;
TOK _au4_k2 ;
int _au4_n1 ;
int _au4_n2 ;

#line 483 "../../src/typ.c"
_au4_f1 = (((struct fct *)_au1_t1 ));
_au4_f2 = (((struct fct *)_au1_t2 ));
_au4_a1 = _au4_f1 -> _fct_argtype ;
_au4_a2 = _au4_f2 -> _fct_argtype ;
_au4_k1 = _au4_f1 -> _fct_nargs_known ;
_au4_k2 = _au4_f2 -> _fct_nargs_known ;
_au4_n1 = _au4_f1 -> _fct_nargs ;
_au4_n2 = _au4_f2 -> _fct_nargs ;

#line 492 "../../src/typ.c"
if (_au4_f1 -> _fct_memof != _au4_f2 -> _fct_memof ){ 
#line 493 "../../src/typ.c"
if (_classdef_baseofFPCclassdef___ ( _au4_f2 -> _fct_memof , _au4_f1 -> _fct_memof ) )return (char )1 ;
#line 493 "../../src/typ.c"

#line 494 "../../src/typ.c"
Nstd ++ ;
}
if ((_au4_k1 && (_au4_k2 == 0 ))|| (_au4_k2 && (_au4_k1 == 0 ))){ 
#line 497 "../../src/typ.c"
if (_au4_f2 -> _fct_body == 0 )return (char )1 ;
}

#line 500 "../../src/typ.c"
if (((_au4_n1 != _au4_n2 )&& _au4_k1 )&& _au4_k2 ){ 
#line 501 "../../src/typ.c"
goto aaa ;
}
else if (_au4_a1 && _au4_a2 ){ 
#line 504 "../../src/typ.c"
int _au5_i ;

#line 504 "../../src/typ.c"
_au5_i = 0 ;
while (_au4_a1 && _au4_a2 ){ 
#line 506 "../../src/typ.c"
_au5_i ++ ;
if (_type_check ( _au4_a1 -> _expr__O2.__C2_tp , _au4_a2 -> _expr__O2.__C2_tp , (unsigned char )(_au0_oper ? 76 : 0)) )return (char )1 ;
#line 507 "../../src/typ.c"

#line 508 "../../src/typ.c"
_au4_a1 = _au4_a1 -> _name_n_list ;
_au4_a2 = _au4_a2 -> _name_n_list ;
}
if (_au4_a1 || _au4_a2 )goto aaa ;
}
else if (_au4_a1 || _au4_a2 ){ 
#line 514 "../../src/typ.c"
aaa :
#line 515 "../../src/typ.c"
if (_au4_k1 == 155 ){ 
#line 516 "../../src/typ.c"
switch (_au0_oper ){ 
#line 517 "../../src/typ.c"
case 0 : 
#line 518 "../../src/typ.c"
if (_au4_a2 && (_au4_k2 ==
#line 518 "../../src/typ.c"
0 ))break ;
return (char )1 ;
case 70 : 
#line 521 "../../src/typ.c"
if (_au4_a2 && (_au4_k2 == 0 ))break ;
return (char )1 ;
case 136 : 
#line 524 "../../src/typ.c"
if (_au4_a1 )return (char )1 ;
break ;
case 76 : 
#line 527 "../../src/typ.c"
case 78 : 
#line 528 "../../src/typ.c"
return (char )1 ;
}
}
else if (_au4_k2 == 155 ){ 
#line 532 "../../src/typ.c"
return (char )1 ;
}
else if (_au4_k1 || _au4_k2 ){ 
#line 535 "../../src/typ.c"
return (char )1 ;
}
}
_au1_t1 = _au4_f1 -> _fct_returns ;
_au1_t2 = _au4_f2 -> _fct_returns ;
}
break ;

#line 543 "../../src/typ.c"
case 114 : 
#line 544 "../../src/typ.c"
goto field_check ;
case 5 : 
#line 546 "../../src/typ.c"
case 29 : 
#line 547 "../../src/typ.c"
case 21 : 
#line 548 "../../src/typ.c"
case 22 : 
#line 549 "../../src/typ.c"
goto int_check ;
case 15 : 
#line 551 "../../src/typ.c"
case 11 : 
#line 552 "../../src/typ.c"
goto float_check ;
case 121 : 
#line 554 "../../src/typ.c"
goto enum_check ;
case 119 : 
#line 556 "../../src/typ.c"
goto cla_check ;
case 138 : 
#line 558 "../../src/typ.c"
case 38 : 
#line 559 "../../src/typ.c"
return (char )0 ;
default : 
#line 561 "../../src/typ.c"
{ 
#line 788 "../../src/typ.c"
struct ea _au0__V47 ;

#line 788 "../../src/typ.c"
struct ea _au0__V48 ;

#line 788 "../../src/typ.c"
struct ea _au0__V49 ;

#line 561 "../../src/typ.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"T::check(o=%d %d %d)", (struct ea *)( ( ((& _au0__V47 )-> _ea__O1.__C1_i = ((int )_au0_oper )), (((&
#line 561 "../../src/typ.c"
_au0__V47 )))) ) , (struct ea *)( ( ((& _au0__V48 )-> _ea__O1.__C1_i = ((int )_au1_b1 )), (((& _au0__V48 )))) )
#line 561 "../../src/typ.c"
, (struct ea *)( ( ((& _au0__V49 )-> _ea__O1.__C1_i = ((int )_au1_b2 )), (((& _au0__V49 )))) ) , (struct
#line 561 "../../src/typ.c"
ea *)ea0 ) ;
} }
}

#line 565 "../../src/typ.c"
if (_au1_t1 || _au1_t2 )return (char )1 ;
return (char )0 ;

#line 568 "../../src/typ.c"
field_check :
#line 569 "../../src/typ.c"
switch (_au0_oper ){ 
#line 570 "../../src/typ.c"
case 0 : 
#line 571 "../../src/typ.c"
case 136 : 
#line 572 "../../src/typ.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"check field?", (struct ea *)ea0 , (struct
#line 572 "../../src/typ.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
}
return (char )0 ;

#line 576 "../../src/typ.c"
float_check :
#line 577 "../../src/typ.c"
if (((_au1_first == 0 )&& (_au1_b1 != _au1_b2 ))&& (_au1_b2 != 138 ))return (char )1 ;

#line 579 "../../src/typ.c"
int_check :
#line 580 "../../src/typ.c"
if ((((struct basetype *)_au1_t1 ))-> _basetype_b_unsigned != (((struct basetype *)_au1_t2 ))-> _basetype_b_unsigned ){ 
#line 581 "../../src/typ.c"
if (_au0_oper )
#line 582 "../../src/typ.c"
Nstd ++ ;
else 
#line 584 "../../src/typ.c"
return (char )1 ;
}
enum_check :
#line 587 "../../src/typ.c"
const_check :
#line 589 "../../src/typ.c"
if (! _au1_t1_const )_au1_t1_const = _type_tconst ( _au1_t1 ) ;
if (! _au1_t2_const )_au1_t2_const = _type_tconst ( _au1_t2 ) ;

#line 592 "../../src/typ.c"
if (((_au0_oper == 0 )&& (_au1_t1_const != _au1_t2_const ))|| (((_au1_first == 0 )&& _au1_t2_const )&& (_au1_t1_const == 0 )))
#line 593 "../../src/typ.c"
{ 
#line 594 "../../src/typ.c"
const_problem = 1 ;
return (char )1 ;
}
return (char )0 ;

#line 599 "../../src/typ.c"
cla_check :
#line 600 "../../src/typ.c"
{ Pbase _au2_c1 ;
Pbase _au2_c2 ;
Pname _au2_n1 ;
Pname _au2_n2 ;

#line 600 "../../src/typ.c"
_au2_c1 = (((struct basetype *)_au1_t1 ));
_au2_c2 = (((struct basetype *)_au1_t2 ));
_au2_n1 = _au2_c1 -> _basetype_b_name ;
_au2_n2 = _au2_c2 -> _basetype_b_name ;

#line 605 "../../src/typ.c"
if (_au2_n1 == _au2_n2 )goto const_check ;
if (_au1_first )return (char )1 ;

#line 608 "../../src/typ.c"
switch (_au0_oper ){ 
#line 609 "../../src/typ.c"
case 0 : 
#line 610 "../../src/typ.c"
case 76 : 
#line 611 "../../src/typ.c"
return (char )1 ;
case 136 : 
#line 613 "../../src/typ.c"
case 70 : 
#line 614 "../../src/typ.c"
case 28 : 
#line 615 "../../src/typ.c"
case 78 : 
#line 616 "../../src/typ.c"
{ 
#line 618 "../../src/typ.c"
Pname _au4_b ;
Pclass _au4_cl ;

#line 618 "../../src/typ.c"
_au4_b = _au2_n2 ;

#line 620 "../../src/typ.c"
while (_au4_b ){ 
#line 621 "../../src/typ.c"
_au4_cl = (((struct classdef *)_au4_b -> _expr__O2.__C2_tp ));
_au4_b = _au4_cl -> _classdef_clbase ;

#line 624 "../../src/typ.c"
if (_au4_b && (_au4_cl -> _classdef_pubbase == 0 ))return (char )1 ;
if (_au4_b == _au2_n1 ){ 
#line 626 "../../src/typ.c"
Nstd ++ ;
goto const_check ;
}
}
return (char )1 ;
}
}
}
goto const_check ;

#line 636 "../../src/typ.c"
base_check :
#line 638 "../../src/typ.c"
if (_au0_oper )
#line 639 "../../src/typ.c"
if (_au1_first ){ 
#line 640 "../../src/typ.c"
if ((_au1_b1 == 38 )|| (_au1_b2 == 38 ))return (char )1 ;
}
else { 
#line 643 "../../src/typ.c"
if (_au1_b1 == 38 ){ 
#line 644 "../../src/typ.c"
register Ptype _au3_tx ;

#line 644 "../../src/typ.c"
_au3_tx = _au0_this ;
txloop :
#line 646 "../../src/typ.c"
switch (_au3_tx -> _node_base ){ 
#line 647 "../../src/typ.c"
default : return (char )1 ;
case 38 : break ;
case 125 : 
#line 650 "../../src/typ.c"
case 110 : _au3_tx = (((struct vec *)_au3_tx ))-> _pvtyp_typ ;

#line 650 "../../src/typ.c"
goto txloop ;
case 97 : _au3_tx = (((struct basetype *)_au3_tx ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 651 "../../src/typ.c"
goto txloop ;
}

#line 654 "../../src/typ.c"
_au3_tx = _au0_t ;
bloop :
#line 656 "../../src/typ.c"
switch (_au3_tx -> _node_base ){ 
#line 657 "../../src/typ.c"
default : return (char )1 ;
case 110 : 
#line 659 "../../src/typ.c"
case 125 : 
#line 660 "../../src/typ.c"
case 108 : Nstd ++ ;

#line 660 "../../src/typ.c"
goto const_check ;
case 97 : _au3_tx = (((struct basetype *)_au3_tx ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 661 "../../src/typ.c"
goto bloop ;
}
}
if (_au1_b2 != 138 )return (char )1 ;
}

#line 667 "../../src/typ.c"
switch (_au0_oper ){ 
#line 668 "../../src/typ.c"
case 0 : 
#line 669 "../../src/typ.c"
case 76 : 
#line 670 "../../src/typ.c"
return (char )1 ;

#line 672 "../../src/typ.c"
case 78 : 
#line 673 "../../src/typ.c"
switch (_au1_b1 ){ 
#line 674 "../../src/typ.c"
case 121 : 
#line 675 "../../src/typ.c"
case 138 : 
#line 676 "../../src/typ.c"
case 5 : 
#line 677 "../../src/typ.c"
case 29 : 
#line 678 "../../src/typ.c"
case 21 :
#line 678 "../../src/typ.c"

#line 679 "../../src/typ.c"
switch (_au1_b2 ){ 
#line 680 "../../src/typ.c"
case 121 : 
#line 681 "../../src/typ.c"
case 138 : 
#line 682 "../../src/typ.c"
case 5 : 
#line 683 "../../src/typ.c"
case 29 : 
#line 684 "../../src/typ.c"
case 21 : 
#line 685 "../../src/typ.c"
case 114 :
#line 685 "../../src/typ.c"

#line 686 "../../src/typ.c"
goto const_check ;
}
return (char )1 ;
case 22 : 
#line 690 "../../src/typ.c"
switch (_au1_b2 ){ 
#line 691 "../../src/typ.c"
case 138 : 
#line 692 "../../src/typ.c"
case 121 : 
#line 693 "../../src/typ.c"
case 5 : 
#line 694 "../../src/typ.c"
case 29 : 
#line 695 "../../src/typ.c"
case 21 :
#line 695 "../../src/typ.c"

#line 696 "../../src/typ.c"
case 114 : 
#line 697 "../../src/typ.c"
Nstd ++ ;
goto const_check ;
}
return (char )1 ;
case 15 : 
#line 702 "../../src/typ.c"
switch (_au1_b2 ){ 
#line 703 "../../src/typ.c"
case 138 : 
#line 704 "../../src/typ.c"
Nstd ++ ;
case 15 : 
#line 706 "../../src/typ.c"
case 11 : 
#line 707 "../../src/typ.c"
goto const_check ;
}
return (char )1 ;
case 11 : 
#line 711 "../../src/typ.c"
switch (_au1_b2 ){ 
#line 712 "../../src/typ.c"
case 138 : 
#line 713 "../../src/typ.c"
case 121 : 
#line 714 "../../src/typ.c"
case 5 : 
#line 715 "../../src/typ.c"
case 29 : 
#line 716 "../../src/typ.c"
case 21 :
#line 716 "../../src/typ.c"

#line 717 "../../src/typ.c"
Nstd ++ ;
case 15 : 
#line 719 "../../src/typ.c"
case 11 : 
#line 720 "../../src/typ.c"
goto const_check ;
}
return (char )1 ;
case 125 : 
#line 724 "../../src/typ.c"
switch (_au1_b2 ){ 
#line 725 "../../src/typ.c"
case 138 : 
#line 726 "../../src/typ.c"
Nstd ++ ;
goto const_check ;
}
case 158 : 
#line 730 "../../src/typ.c"
case 110 : 
#line 731 "../../src/typ.c"
case 119 : 
#line 732 "../../src/typ.c"
case 108 : 
#line 733 "../../src/typ.c"
return (char )1 ;
}
case 136 : 
#line 736 "../../src/typ.c"
case 70 : 
#line 737 "../../src/typ.c"
case 28 : 
#line 738 "../../src/typ.c"
switch (_au1_b1 ){ 
#line 739 "../../src/typ.c"
case 119 : 
#line 740 "../../src/typ.c"
return (char )1 ;
case 121 : 
#line 742 "../../src/typ.c"
case 138 : 
#line 743 "../../src/typ.c"
case 5 : 
#line 744 "../../src/typ.c"
case 29 : 
#line 745 "../../src/typ.c"
case 21 : 
#line 746 "../../src/typ.c"
case 22 : 
#line 747 "../../src/typ.c"
suppress_error ++ ;
#line 747 "../../src/typ.c"

#line 748 "../../src/typ.c"
_au1_r = ( _type_kind ( _au1_t2 , ((unsigned char )70 ), (unsigned char )'P' ) ) ;
suppress_error -- ;
switch (_au1_r ){ 
#line 751 "../../src/typ.c"
case 'F' : { 
#line 788 "../../src/typ.c"
struct ea _au0__V50 ;

#line 788 "../../src/typ.c"
struct ea _au0__V51 ;

#line 751 "../../src/typ.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"%t assigned to%t", (struct ea *)( ( ((& _au0__V50 )-> _ea__O1.__C1_p = ((char *)_au1_t2 )), (((&
#line 751 "../../src/typ.c"
_au0__V50 )))) ) , (struct ea *)( ( ((& _au0__V51 )-> _ea__O1.__C1_p = ((char *)_au1_t1 )), (((& _au0__V51 )))) )
#line 751 "../../src/typ.c"
, (struct ea *)ea0 , (struct ea *)ea0 ) ;

#line 751 "../../src/typ.c"
break ;
case 'A' : 
#line 753 "../../src/typ.c"
case 'P' : 
#line 754 "../../src/typ.c"
case 108 : return (char )1 ;
} }
break ;
case 15 : 
#line 758 "../../src/typ.c"
case 11 : 
#line 759 "../../src/typ.c"
suppress_error ++ ;
_au1_r = ( _type_kind ( _au1_t2 , ((unsigned char )70 ), (unsigned char )'N' ) ) ;
suppress_error -- ;
break ;
case 110 : 
#line 764 "../../src/typ.c"
return (char )1 ;
case 125 : 
#line 766 "../../src/typ.c"
suppress_error ++ ;
_au1_r = ( _type_kind ( _au1_t2 , ((unsigned char )70 ), (unsigned char )'P' ) ) ;
suppress_error -- ;
switch (_au1_r ){ 
#line 770 "../../src/typ.c"
case 'A' : 
#line 771 "../../src/typ.c"
case 'I' : 
#line 772 "../../src/typ.c"
case 'F' : return (char )1 ;
case 108 : if ((((struct ptr *)_au1_t1 ))-> _pvtyp_typ -> _node_base != 108 )return (char )1 ;
}
break ;
case 158 : 
#line 777 "../../src/typ.c"
return (char )1 ;
case 108 : 
#line 779 "../../src/typ.c"
switch (_au0_oper ){ 
#line 780 "../../src/typ.c"
case 136 : 
#line 781 "../../src/typ.c"
case 70 : 
#line 782 "../../src/typ.c"
return (char )1 ;
}
}
break ;
}
goto const_check ;
}
;

/* the end */
