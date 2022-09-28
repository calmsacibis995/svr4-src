/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#line 1 "../../src/norm.c"

/* <<cfront 1.2.1 2/16/87>> */
/* < ../../src/norm.c */

#ident	"@(#)sdb:cfront/scratch/src/norm..c	1.2"
char *_new(); char _delete(); char *_vec_new(); char _vec_delete();

#line 1 "../../src/norm.c"

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

#line 24 "../../src/norm.c"
extern char syn_init ();
extern char syn_init ()
#line 26 "../../src/norm.c"
{ 
#line 27 "../../src/norm.c"
any_type = (struct basetype *)_basetype__ctor ( (struct basetype *)0 , (unsigned char )141 , (struct name *)0 ) ;
#line 27 "../../src/norm.c"

#line 28 "../../src/norm.c"
any_type -> _node_permanent = 1 ;
dummy = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )144 , (struct expr *)0 , (struct expr *)0 ) ;
dummy -> _node_permanent = 1 ;
dummy -> _expr__O2.__C2_tp = (struct type *)any_type ;
zero = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )86 , (struct expr *)0 , (struct expr *)0 ) ;
zero -> _node_permanent = 1 ;
}
;
int stcount = 0 ;

#line 38 "../../src/norm.c"
extern char *make_name (_au0_c )TOK _au0_c ;
{ 
#line 40 "../../src/norm.c"
char *_au1_s ;

#line 46 "../../src/norm.c"
int _au1_count ;
int _au1_i ;

#line 40 "../../src/norm.c"
_au1_s = (((char *)_new ( (long )((sizeof (char ))* 8 )) ));

#line 42 "../../src/norm.c"
if (3100 <= (++ stcount ))errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"too many generatedNs", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ,
#line 42 "../../src/norm.c"
(struct ea *)ea0 ) ;

#line 44 "../../src/norm.c"
(_au1_s [0 ])= '_' ;
(_au1_s [1 ])= _au0_c ;
_au1_count = stcount ;

#line 46 "../../src/norm.c"
_au1_i = 2 ;

#line 48 "../../src/norm.c"
if (10000 <= _au1_count ){ 
#line 49 "../../src/norm.c"
(_au1_s [_au1_i ++ ])= ('0' + (_au1_count / 10000 ));
_au1_count %= 10000 ;
}
if (1000 <= _au1_count ){ 
#line 53 "../../src/norm.c"
(_au1_s [_au1_i ++ ])= ('0' + (_au1_count / 1000 ));
_au1_count %= 1000 ;
}
else if (2 < _au1_i )(_au1_s [_au1_i ++ ])= '0' ;

#line 58 "../../src/norm.c"
if (100 <= _au1_count ){ 
#line 59 "../../src/norm.c"
(_au1_s [_au1_i ++ ])= ('0' + (_au1_count / 100 ));
_au1_count %= 100 ;
}
else if (2 < _au1_i )(_au1_s [_au1_i ++ ])= '0' ;

#line 64 "../../src/norm.c"
if (10 <= _au1_count ){ 
#line 65 "../../src/norm.c"
(_au1_s [_au1_i ++ ])= ('0' + (_au1_count / 10 ));
_au1_count %= 10 ;
}
else if (2 < _au1_i )(_au1_s [_au1_i ++ ])= '0' ;

#line 70 "../../src/norm.c"
(_au1_s [_au1_i ++ ])= ('0' + _au1_count );
(_au1_s [_au1_i ])= 0 ;

#line 73 "../../src/norm.c"
return _au1_s ;
}
;
Pbase _basetype_type_adj (_au0_this , _au0_t )
#line 362 "../../src/cfront.h"
struct basetype *_au0_this ;

#line 76 "../../src/norm.c"
TOK _au0_t ;
{ 
#line 78 "../../src/norm.c"
switch (_au0_this -> _node_base ){ 
#line 79 "../../src/norm.c"
case 119 : 
#line 80 "../../src/norm.c"
case 121 : 
#line 81 "../../src/norm.c"
{ Pbase _au3_bt ;

#line 81 "../../src/norm.c"
_au3_bt = (struct basetype *)_basetype__ctor ( (struct basetype *)0 , (unsigned char )0 , (struct name *)0 ) ;
(*_au3_bt )= (*_au0_this );
if (_au0_this && (_au0_this -> _node_permanent == 0 ))_type_del ( (struct type *)_au0_this ) ;
_au0_this = _au3_bt ;
}
}

#line 88 "../../src/norm.c"
if (_au0_this -> _basetype_b_xname ){ 
#line 89 "../../src/norm.c"
if (_au0_this -> _node_base )
#line 90 "../../src/norm.c"
{ 
#line 135 "../../src/norm.c"
struct ea _au0__V10 ;

#line 135 "../../src/norm.c"
struct ea _au0__V11 ;

#line 90 "../../src/norm.c"
error ( (char *)"badBT:%n%k", (struct ea *)( ( ((& _au0__V10 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _basetype_b_xname )), (((& _au0__V10 ))))
#line 90 "../../src/norm.c"
) , (struct ea *)( ( ((& _au0__V11 )-> _ea__O1.__C1_i = ((int )_au0_t )), (((& _au0__V11 )))) ) ,
#line 90 "../../src/norm.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
} else { 
#line 92 "../../src/norm.c"
_au0_this -> _node_base = 97 ;
_au0_this -> _basetype_b_name = _au0_this -> _basetype_b_xname ;
}
_au0_this -> _basetype_b_xname = 0 ;
}

#line 98 "../../src/norm.c"
switch (_au0_t ){ 
#line 99 "../../src/norm.c"
case 35 : _au0_this -> _basetype_b_typedef = 1 ;

#line 99 "../../src/norm.c"
break ;
case 75 : _au0_this -> _basetype_b_inline = 1 ;

#line 100 "../../src/norm.c"
break ;
case 77 : _au0_this -> _basetype_b_virtual = 1 ;

#line 101 "../../src/norm.c"
break ;
case 26 : _au0_this -> _basetype_b_const = 1 ;

#line 102 "../../src/norm.c"
break ;
case 37 : _au0_this -> _basetype_b_unsigned = 1 ;

#line 103 "../../src/norm.c"
break ;
case 29 : _au0_this -> _basetype_b_short = 1 ;

#line 104 "../../src/norm.c"
break ;
case 22 : _au0_this -> _basetype_b_long = 1 ;

#line 105 "../../src/norm.c"
break ;
case 18 : 
#line 107 "../../src/norm.c"
case 76 : 
#line 108 "../../src/norm.c"
case 14 : 
#line 109 "../../src/norm.c"
case 31 : 
#line 110 "../../src/norm.c"
case 2 : 
#line 111 "../../src/norm.c"
case 27 : 
#line 112 "../../src/norm.c"
if (_au0_this ->
#line 112 "../../src/norm.c"
_basetype_b_sto )
#line 113 "../../src/norm.c"
{ 
#line 135 "../../src/norm.c"
struct ea _au0__V12 ;

#line 135 "../../src/norm.c"
struct ea _au0__V13 ;

#line 113 "../../src/norm.c"
error ( (char *)"badBT:%k%k", (struct ea *)( ( ((& _au0__V12 )-> _ea__O1.__C1_i = ((int )_au0_this -> _basetype_b_sto )), (((& _au0__V12 ))))
#line 113 "../../src/norm.c"
) , (struct ea *)( ( ((& _au0__V13 )-> _ea__O1.__C1_i = ((int )_au0_t )), (((& _au0__V13 )))) ) ,
#line 113 "../../src/norm.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
} else 
#line 115 "../../src/norm.c"
_au0_this -> _basetype_b_sto = _au0_t ;
break ;
case 38 : 
#line 118 "../../src/norm.c"
case 5 : 
#line 119 "../../src/norm.c"
case 21 : 
#line 120 "../../src/norm.c"
case 15 : 
#line 121 "../../src/norm.c"
case 11 : 
#line 122 "../../src/norm.c"
if (_au0_this -> _node_base )
#line 123 "../../src/norm.c"
{ 
#line 135 "../../src/norm.c"
struct
#line 135 "../../src/norm.c"
ea _au0__V14 ;

#line 135 "../../src/norm.c"
struct ea _au0__V15 ;

#line 123 "../../src/norm.c"
error ( (char *)"badBT:%k%k", (struct ea *)( ( ((& _au0__V14 )-> _ea__O1.__C1_i = ((int )_au0_this -> _node_base )), (((& _au0__V14 ))))
#line 123 "../../src/norm.c"
) , (struct ea *)( ( ((& _au0__V15 )-> _ea__O1.__C1_i = ((int )_au0_t )), (((& _au0__V15 )))) ) ,
#line 123 "../../src/norm.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
} else 
#line 125 "../../src/norm.c"
_au0_this -> _node_base = _au0_t ;
break ;
case 171 : 
#line 128 "../../src/norm.c"
case 170 : 
#line 129 "../../src/norm.c"
{ 
#line 135 "../../src/norm.c"
struct ea _au0__V16 ;

#line 129 "../../src/norm.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"\"%k\" not implemented (ignored)", (struct ea *)( ( ((& _au0__V16 )-> _ea__O1.__C1_i = ((int )_au0_t )), (((&
#line 129 "../../src/norm.c"
_au0__V16 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
break ;
default : 
#line 132 "../../src/norm.c"
{ 
#line 135 "../../src/norm.c"
struct ea _au0__V17 ;

#line 132 "../../src/norm.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"BT::type_adj(%k)", (struct ea *)( ( ((& _au0__V17 )-> _ea__O1.__C1_i = ((int )_au0_t )), (((&
#line 132 "../../src/norm.c"
_au0__V17 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} } }
return _au0_this ;
}
;
Pbase _basetype_name_adj (_au0_this , _au0_n )
#line 362 "../../src/cfront.h"
struct basetype *_au0_this ;

#line 137 "../../src/norm.c"
Pname _au0_n ;
{ 
#line 139 "../../src/norm.c"
if (_au0_this -> _basetype_b_xname ){ 
#line 140 "../../src/norm.c"
if (_au0_this -> _node_base )
#line 141 "../../src/norm.c"
{ 
#line 156 "../../src/norm.c"
struct ea _au0__V18 ;

#line 156 "../../src/norm.c"
struct ea _au0__V19 ;

#line 141 "../../src/norm.c"
error ( (char *)"badBT:%n%n", (struct ea *)( ( ((& _au0__V18 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _basetype_b_xname )), (((& _au0__V18 ))))
#line 141 "../../src/norm.c"
) , (struct ea *)( ( ((& _au0__V19 )-> _ea__O1.__C1_p = ((char *)_au0_n )), (((& _au0__V19 )))) ) ,
#line 141 "../../src/norm.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
} else { 
#line 143 "../../src/norm.c"
_au0_this -> _node_base = 97 ;
_au0_this -> _basetype_b_name = _au0_this -> _basetype_b_xname ;
}
_au0_this -> _basetype_b_xname = 0 ;
}

#line 149 "../../src/norm.c"
if ((! _au0_this -> _node_base )&& (_au0_n -> _expr__O2.__C2_tp -> _node_base != 119 )){ 
#line 150 "../../src/norm.c"
_au0_this -> _node_base = 97 ;
_au0_this -> _basetype_b_name = _au0_n ;
}
else _au0_this -> _basetype_b_xname = _au0_n ;

#line 155 "../../src/norm.c"
return _au0_this ;
}
;
Pbase _basetype_base_adj (_au0_this , _au0_b )
#line 362 "../../src/cfront.h"
struct basetype *_au0_this ;

#line 158 "../../src/norm.c"
Pbase _au0_b ;
{ 
#line 160 "../../src/norm.c"
Pname _au1_bn ;

#line 160 "../../src/norm.c"
_au1_bn = _au0_b -> _basetype_b_name ;

#line 162 "../../src/norm.c"
switch (_au0_this -> _node_base ){ 
#line 163 "../../src/norm.c"
case 119 : 
#line 164 "../../src/norm.c"
case 121 : 
#line 165 "../../src/norm.c"
{ 
#line 181 "../../src/norm.c"
struct ea _au0__V20 ;

#line 181 "../../src/norm.c"
struct ea _au0__V21 ;

#line 165 "../../src/norm.c"
error ( (char *)"NX after%k%n", (struct ea *)( ( ((& _au0__V20 )-> _ea__O1.__C1_i = ((int )_au0_this -> _node_base )), (((& _au0__V20 ))))
#line 165 "../../src/norm.c"
) , (struct ea *)( ( ((& _au0__V21 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _basetype_b_name )), (((& _au0__V21 )))) )
#line 165 "../../src/norm.c"
, (struct ea *)ea0 , (struct ea *)ea0 ) ;
return _au0_this ;
} }

#line 169 "../../src/norm.c"
if (_au0_this -> _node_base ){ 
#line 170 "../../src/norm.c"
if (_au0_this -> _basetype_b_name )
#line 171 "../../src/norm.c"
{ 
#line 181 "../../src/norm.c"
struct ea _au0__V22 ;

#line 181 "../../src/norm.c"
struct ea _au0__V23 ;

#line 181 "../../src/norm.c"
struct ea _au0__V24 ;

#line 181 "../../src/norm.c"
struct ea _au0__V25 ;

#line 171 "../../src/norm.c"
error ( (char *)"badBT:%k%n%k%n", (struct ea *)( ( ((& _au0__V22 )-> _ea__O1.__C1_i = ((int )_au0_this -> _node_base )), (((& _au0__V22 ))))
#line 171 "../../src/norm.c"
) , (struct ea *)( ( ((& _au0__V23 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _basetype_b_name )), (((& _au0__V23 )))) )
#line 171 "../../src/norm.c"
, (struct ea *)( ( ((& _au0__V24 )-> _ea__O1.__C1_i = ((int )_au0_b -> _node_base )), (((& _au0__V24 )))) ) ,
#line 171 "../../src/norm.c"
(struct ea *)( ( ((& _au0__V25 )-> _ea__O1.__C1_p = ((char *)_au1_bn )), (((& _au0__V25 )))) ) ) ;
} else 
#line 173 "../../src/norm.c"
{ 
#line 181 "../../src/norm.c"
struct ea _au0__V26 ;

#line 181 "../../src/norm.c"
struct ea _au0__V27 ;

#line 181 "../../src/norm.c"
struct ea _au0__V28 ;

#line 173 "../../src/norm.c"
error ( (char *)"badBT:%k%k%n", (struct ea *)( ( ((& _au0__V26 )-> _ea__O1.__C1_i = ((int )_au0_this -> _node_base )), (((& _au0__V26 ))))
#line 173 "../../src/norm.c"
) , (struct ea *)( ( ((& _au0__V27 )-> _ea__O1.__C1_i = ((int )_au0_b -> _node_base )), (((& _au0__V27 )))) )
#line 173 "../../src/norm.c"
, (struct ea *)( ( ((& _au0__V28 )-> _ea__O1.__C1_p = ((char *)_au1_bn )), (((& _au0__V28 )))) ) , (struct
#line 173 "../../src/norm.c"
ea *)ea0 ) ;
} }
else { 
#line 176 "../../src/norm.c"
_au0_this -> _node_base = _au0_b -> _node_base ;
_au0_this -> _basetype_b_name = _au1_bn ;
_au0_this -> _basetype_b_table = _au0_b -> _basetype_b_table ;
}
return _au0_this ;
}
;
Pbase _basetype_check (_au0_this , _au0_n )
#line 362 "../../src/cfront.h"
struct basetype *_au0_this ;

#line 183 "../../src/norm.c"
Pname _au0_n ;

#line 189 "../../src/norm.c"
{ 
#line 190 "../../src/norm.c"
_au0_this -> _basetype_b_inline = 0 ;
_au0_this -> _basetype_b_virtual = 0 ;

#line 193 "../../src/norm.c"
if (_au0_this -> _basetype_b_xname && (_au0_n -> _expr__O2.__C2_tp || _au0_n -> _expr__O3.__C3_string )){ 
#line 194 "../../src/norm.c"
if (_au0_this -> _node_base )
#line 195 "../../src/norm.c"
{ 
#line 316 "../../src/norm.c"
struct ea _au0__V29 ;

#line 316 "../../src/norm.c"
struct ea _au0__V30 ;

#line 195 "../../src/norm.c"
error ( (char *)"badBT:%k%n", (struct ea *)( ( ((& _au0__V29 )-> _ea__O1.__C1_i = ((int )_au0_this -> _node_base )), (((& _au0__V29 ))))
#line 195 "../../src/norm.c"
) , (struct ea *)( ( ((& _au0__V30 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _basetype_b_xname )), (((& _au0__V30 )))) )
#line 195 "../../src/norm.c"
, (struct ea *)ea0 , (struct ea *)ea0 ) ;
} else { 
#line 197 "../../src/norm.c"
_au0_this -> _node_base = 97 ;
_au0_this -> _basetype_b_name = _au0_this -> _basetype_b_xname ;
}
_au0_this -> _basetype_b_xname = 0 ;
}

#line 203 "../../src/norm.c"
if (_au0_this -> _basetype_b_xname ){ 
#line 204 "../../src/norm.c"
if (_au0_n -> _expr__O3.__C3_string )
#line 205 "../../src/norm.c"
{ 
#line 316 "../../src/norm.c"
struct ea _au0__V31 ;

#line 316 "../../src/norm.c"
struct ea _au0__V32 ;

#line 205 "../../src/norm.c"
error ( (char *)"twoNs inD:%n%n", (struct ea *)( ( ((& _au0__V31 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _basetype_b_xname )), (((& _au0__V31 ))))
#line 205 "../../src/norm.c"
) , (struct ea *)( ( ((& _au0__V32 )-> _ea__O1.__C1_p = ((char *)_au0_n )), (((& _au0__V32 )))) ) ,
#line 205 "../../src/norm.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
} else { 
#line 207 "../../src/norm.c"
_au0_n -> _expr__O3.__C3_string = _au0_this -> _basetype_b_xname -> _expr__O3.__C3_string ;
_name_hide ( _au0_this -> _basetype_b_xname ) ;
}
_au0_this -> _basetype_b_xname = 0 ;
}

#line 213 "../../src/norm.c"
if (((((ccl == 0 )&& _au0_n )&& (_au0_n -> _name_n_oper == 123 ))&& (_au0_n -> _name__O6.__C6_n_qualifier == 0 ))&& _au0_n -> _expr__O3.__C3_string )
#line 217 "../../src/norm.c"
{ 
#line 218 "../../src/norm.c"
Pname _au2_nx ;

#line 218 "../../src/norm.c"
_au2_nx = _table_look ( ktbl , _au0_n -> _expr__O3.__C3_string , (unsigned char )0 ) ;
if (_au2_nx )_name_hide ( _au2_nx ) ;
}

#line 222 "../../src/norm.c"
switch (_au0_this -> _node_base ){ 
#line 223 "../../src/norm.c"
case 0 : 
#line 224 "../../src/norm.c"
_au0_this -> _node_base = 21 ;
break ;
case 121 : 
#line 227 "../../src/norm.c"
case 119 : 
#line 228 "../../src/norm.c"
if (_au0_this -> _basetype_b_name -> _node_base == 123 )
#line 229 "../../src/norm.c"
{ 
#line 316 "../../src/norm.c"
struct ea _au0__V33 ;

#line 316 "../../src/norm.c"
struct ea _au0__V34 ;

#line 229 "../../src/norm.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"TN%n inCO %d", (struct ea *)( ( ((& _au0__V33 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _basetype_b_name )),
#line 229 "../../src/norm.c"
(((& _au0__V33 )))) ) , (struct ea *)( ( ((& _au0__V34 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V34 ))))
#line 229 "../../src/norm.c"
) , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }

#line 232 "../../src/norm.c"
if (_au0_this -> _basetype_b_long || _au0_this -> _basetype_b_short ){ 
#line 233 "../../src/norm.c"
TOK _au2_sl ;

#line 233 "../../src/norm.c"
_au2_sl = (_au0_this -> _basetype_b_short ? 29 : 22 );
if (_au0_this -> _basetype_b_long && _au0_this -> _basetype_b_short ){ 
#line 316 "../../src/norm.c"
struct ea _au0__V35 ;

#line 316 "../../src/norm.c"
struct ea _au0__V36 ;

#line 234 "../../src/norm.c"
error ( (char *)"badBT:long short%k%n", (struct ea *)( ( ((& _au0__V35 )-> _ea__O1.__C1_i = ((int )_au0_this -> _node_base )), (((& _au0__V35 ))))
#line 234 "../../src/norm.c"
) , (struct ea *)( ( ((& _au0__V36 )-> _ea__O1.__C1_p = ((char *)_au0_n )), (((& _au0__V36 )))) ) ,
#line 234 "../../src/norm.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
} if (_au0_this -> _node_base != 21 )
#line 236 "../../src/norm.c"
{ 
#line 316 "../../src/norm.c"
struct ea _au0__V37 ;

#line 316 "../../src/norm.c"
struct ea _au0__V38 ;

#line 316 "../../src/norm.c"
struct ea _au0__V39 ;

#line 236 "../../src/norm.c"
error ( (char *)"badBT:%k%k%n", (struct ea *)( ( ((& _au0__V37 )-> _ea__O1.__C1_i = ((int )_au2_sl )), (((& _au0__V37 )))) )
#line 236 "../../src/norm.c"
, (struct ea *)( ( ((& _au0__V38 )-> _ea__O1.__C1_i = ((int )_au0_this -> _node_base )), (((& _au0__V38 )))) ) ,
#line 236 "../../src/norm.c"
(struct ea *)( ( ((& _au0__V39 )-> _ea__O1.__C1_p = ((char *)_au0_n )), (((& _au0__V39 )))) ) , (struct ea *)ea0 )
#line 236 "../../src/norm.c"
;
} else 
#line 238 "../../src/norm.c"
_au0_this -> _node_base = _au2_sl ;
_au0_this -> _basetype_b_short = (_au0_this -> _basetype_b_long = 0 );
}

#line 242 "../../src/norm.c"
if (_au0_this -> _basetype_b_typedef && _au0_this -> _basetype_b_sto ){ 
#line 316 "../../src/norm.c"
struct ea _au0__V40 ;

#line 316 "../../src/norm.c"
struct ea _au0__V41 ;

#line 242 "../../src/norm.c"
error ( (char *)"badBT:Tdef%k%n", (struct ea *)( ( ((& _au0__V40 )-> _ea__O1.__C1_i = ((int )_au0_this -> _basetype_b_sto )), (((& _au0__V40 ))))
#line 242 "../../src/norm.c"
) , (struct ea *)( ( ((& _au0__V41 )-> _ea__O1.__C1_p = ((char *)_au0_n )), (((& _au0__V41 )))) ) ,
#line 242 "../../src/norm.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
} _au0_this -> _basetype_b_typedef = (_au0_this -> _basetype_b_sto = 0 );

#line 245 "../../src/norm.c"
if (Pfctvec_type == 0 )return _au0_this ;

#line 247 "../../src/norm.c"
if (_au0_this -> _basetype_b_const ){ 
#line 248 "../../src/norm.c"
if (_au0_this -> _basetype_b_unsigned ){ 
#line 249 "../../src/norm.c"
switch (_au0_this -> _node_base ){ 
#line 250 "../../src/norm.c"
default : 
#line 251 "../../src/norm.c"
{ 
#line 316 "../../src/norm.c"
struct ea _au0__V42 ;
#line 316 "../../src/norm.c"
struct ea _au0__V43 ;

#line 251 "../../src/norm.c"
error ( (char *)"badBT: unsigned const %k%n", (struct ea *)( ( ((& _au0__V42 )-> _ea__O1.__C1_i = ((int )_au0_this -> _node_base )), (((& _au0__V42 ))))
#line 251 "../../src/norm.c"
) , (struct ea *)( ( ((& _au0__V43 )-> _ea__O1.__C1_p = ((char *)_au0_n )), (((& _au0__V43 )))) ) ,
#line 251 "../../src/norm.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
_au0_this -> _basetype_b_unsigned = 0 ;
case 22 : 
#line 254 "../../src/norm.c"
case 29 : 
#line 255 "../../src/norm.c"
case 21 : 
#line 256 "../../src/norm.c"
case 5 : 
#line 257 "../../src/norm.c"
return _au0_this ;
} }
}
return _au0_this ;
}
else if (_au0_this -> _basetype_b_unsigned ){ 
#line 263 "../../src/norm.c"
switch (_au0_this -> _node_base ){ 
#line 264 "../../src/norm.c"
case 22 : 
#line 265 "../../src/norm.c"
_delete ( (char *)_au0_this ) ;
return ulong_type ;
case 29 : 
#line 268 "../../src/norm.c"
_delete ( (char *)_au0_this ) ;
return ushort_type ;
case 21 : 
#line 271 "../../src/norm.c"
_delete ( (char *)_au0_this ) ;
return uint_type ;
case 5 : 
#line 274 "../../src/norm.c"
_delete ( (char *)_au0_this ) ;
return uchar_type ;
default : 
#line 277 "../../src/norm.c"
{ 
#line 316 "../../src/norm.c"
struct ea _au0__V44 ;

#line 316 "../../src/norm.c"
struct ea _au0__V45 ;

#line 277 "../../src/norm.c"
error ( (char *)"badBT: unsigned%k%n", (struct ea *)( ( ((& _au0__V44 )-> _ea__O1.__C1_i = ((int )_au0_this -> _node_base )), (((& _au0__V44 ))))
#line 277 "../../src/norm.c"
) , (struct ea *)( ( ((& _au0__V45 )-> _ea__O1.__C1_p = ((char *)_au0_n )), (((& _au0__V45 )))) ) ,
#line 277 "../../src/norm.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
_au0_this -> _basetype_b_unsigned = 0 ;
return _au0_this ;
} }
}
else { 
#line 283 "../../src/norm.c"
switch (_au0_this -> _node_base ){ 
#line 284 "../../src/norm.c"
case 22 : 
#line 285 "../../src/norm.c"
_delete ( (char *)_au0_this ) ;
return long_type ;
case 29 : 
#line 288 "../../src/norm.c"
_delete ( (char *)_au0_this ) ;
return short_type ;
case 21 : 
#line 291 "../../src/norm.c"
if (_au0_this != int_type )_delete ( (char *)_au0_this ) ;
return int_type ;
case 5 : 
#line 294 "../../src/norm.c"
_delete ( (char *)_au0_this ) ;
return char_type ;
case 38 : 
#line 297 "../../src/norm.c"
_delete ( (char *)_au0_this ) ;
return void_type ;
case 97 : 
#line 302 "../../src/norm.c"
if (_au0_this -> _basetype_b_name -> _name__O6.__C6_n_qualifier ){ 
#line 303 "../../src/norm.c"
Pbase _au4_rv ;

#line 303 "../../src/norm.c"
_au4_rv = (((struct basetype *)_au0_this -> _basetype_b_name -> _name__O6.__C6_n_qualifier ));
_delete ( (char *)_au0_this ) ;
return _au4_rv ;
}
else { 
#line 308 "../../src/norm.c"
_au0_this -> _node_permanent = 1 ;
_au0_this -> _basetype_b_name -> _name__O6.__C6_n_qualifier = (((struct name *)_au0_this ));
return _au0_this ;
}
default : 
#line 313 "../../src/norm.c"
return _au0_this ;
}
}
}
;
Pname _basetype_aggr (_au0_this )
#line 362 "../../src/cfront.h"
struct basetype *_au0_this ;

#line 333 "../../src/norm.c"
{ 
#line 334 "../../src/norm.c"
Pname _au1_n ;

#line 336 "../../src/norm.c"
if (_au0_this -> _basetype_b_xname ){ 
#line 337 "../../src/norm.c"
if (_au0_this -> _node_base ){ 
#line 338 "../../src/norm.c"
Pname _au3_n ;

#line 338 "../../src/norm.c"
_au3_n = (struct name *)_name__ctor ( (struct name *)0 , _au0_this -> _basetype_b_xname -> _expr__O3.__C3_string ) ;
_name_hide ( _au0_this -> _basetype_b_xname ) ;
_au0_this -> _basetype_b_xname = 0 ;
return _name_normalize ( _au3_n , _au0_this , (struct block *)0 , (char )0 ) ;
}
else { 
#line 344 "../../src/norm.c"
_au0_this -> _node_base = 97 ;
_au0_this -> _basetype_b_name = _au0_this -> _basetype_b_xname ;
_au0_this -> _basetype_b_xname = 0 ;
}
}

#line 350 "../../src/norm.c"
switch (_au0_this -> _node_base ){ 
#line 351 "../../src/norm.c"
case 119 : 
#line 352 "../../src/norm.c"
{ Pclass _au3_cl ;
char *_au3_s ;

#line 352 "../../src/norm.c"
_au3_cl = (((struct classdef *)_au0_this -> _basetype_b_name -> _expr__O2.__C2_tp ));
_au3_s = _au3_cl -> _classdef_string ;

#line 355 "../../src/norm.c"
if (_au0_this -> _basetype_b_name -> _node_base == 123 ){ 
#line 414 "../../src/norm.c"
struct ea _au0__V46 ;

#line 355 "../../src/norm.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"TN%n inCO", (struct ea *)( ( ((& _au0__V46 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _basetype_b_name )),
#line 355 "../../src/norm.c"
(((& _au0__V46 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} if (_au0_this -> _basetype_b_const ){ 
#line 414 "../../src/norm.c"
struct ea _au0__V47 ;

#line 414 "../../src/norm.c"
struct ea _au0__V48 ;

#line 356 "../../src/norm.c"
error ( (char *)"const%k%n", (struct ea *)( ( ((& _au0__V47 )-> _ea__O1.__C1_i = ((int )_au3_cl -> _classdef_csu )), (((& _au0__V47 ))))
#line 356 "../../src/norm.c"
) , (struct ea *)( ( ((& _au0__V48 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _basetype_b_name )), (((& _au0__V48 )))) )
#line 356 "../../src/norm.c"
, (struct ea *)ea0 , (struct ea *)ea0 ) ;
} 
#line 358 "../../src/norm.c"
if (_au3_cl -> _classdef_c_body == 2 ){ 
#line 359 "../../src/norm.c"
if (((_au3_s [0 ])== '_' )&& ((_au3_s [1 ])== 'C' )){ 
#line 360 "../../src/norm.c"
char *_au5_ss ;
Pname _au5_obj ;

#line 360 "../../src/norm.c"
_au5_ss = (((char *)_new ( (long )((sizeof (char ))* 8 )) ));
_au5_obj = (struct name *)_name__ctor ( (struct name *)0 , _au5_ss ) ;
strcpy ( _au5_ss , (char *)_au3_s ) ;
if (_au3_cl -> _classdef_csu == 36 ){ 
#line 364 "../../src/norm.c"
(_au5_ss [1 ])= 'O' ;
_au3_cl -> _classdef_csu = 167 ;
return _name_normalize ( _au5_obj , _au0_this , (struct block *)0 , (char )0 ) ;
}
{ 
#line 414 "../../src/norm.c"
struct ea _au0__V49 ;

#line 368 "../../src/norm.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"un-usable%k ignored", (struct ea *)( ( ((& _au0__V49 )-> _ea__O1.__C1_i = ((int )_au3_cl -> _classdef_csu )),
#line 368 "../../src/norm.c"
(((& _au0__V49 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
_au3_cl -> _classdef_c_body = 1 ;
return _au0_this -> _basetype_b_name ;
}
else { 
#line 374 "../../src/norm.c"
if (_au0_this -> _basetype_b_sto == 18 )goto frr ;
return (struct name *)0 ;
}
}

#line 379 "../../src/norm.c"
case 121 : 
#line 380 "../../src/norm.c"
{ Penum _au3_en ;

#line 380 "../../src/norm.c"
_au3_en = (((struct enumdef *)_au0_this -> _basetype_b_name -> _expr__O2.__C2_tp ));

#line 382 "../../src/norm.c"
if (_au0_this -> _basetype_b_name -> _node_base == 123 ){ 
#line 414 "../../src/norm.c"
struct ea _au0__V50 ;

#line 382 "../../src/norm.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"TN%n in enumO", (struct ea *)( ( ((& _au0__V50 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _basetype_b_name )),
#line 382 "../../src/norm.c"
(((& _au0__V50 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} if (_au0_this -> _basetype_b_const ){ 
#line 414 "../../src/norm.c"
struct ea _au0__V51 ;

#line 383 "../../src/norm.c"
error ( (char *)"const enum%n", (struct ea *)( ( ((& _au0__V51 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _basetype_b_name )), (((& _au0__V51 ))))
#line 383 "../../src/norm.c"
) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} if (_au3_en -> _enumdef_e_body == 2 ){ 
#line 385 "../../src/norm.c"
_au3_en -> _enumdef_e_body = 1 ;
return _au0_this -> _basetype_b_name ;
}
else { 
#line 389 "../../src/norm.c"
if (_au0_this -> _basetype_b_sto == 18 )goto frr ;
return (struct name *)0 ;
}
}

#line 394 "../../src/norm.c"
default : 
#line 395 "../../src/norm.c"
if (_au0_this -> _basetype_b_typedef )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"illegalTdef ignored", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 395 "../../src/norm.c"
ea *)ea0 , (struct ea *)ea0 ) ;

#line 397 "../../src/norm.c"
if (_au0_this -> _basetype_b_sto == 18 ){ 
#line 398 "../../src/norm.c"
frr :
#line 399 "../../src/norm.c"
{ Pname _au3_fr ;

#line 399 "../../src/norm.c"
_au3_fr = _table_look ( ktbl , _au0_this -> _basetype_b_name -> _expr__O3.__C3_string , (unsigned char )0 ) ;
if (_au3_fr == 0 ){ 
#line 414 "../../src/norm.c"
struct ea _au0__V52 ;

#line 400 "../../src/norm.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"cannot find friend%n", (struct ea *)( ( ((& _au0__V52 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _basetype_b_name )),
#line 400 "../../src/norm.c"
(((& _au0__V52 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} _au1_n = (struct name *)_name__ctor ( (struct name *)0 , _au0_this -> _basetype_b_name -> _expr__O3.__C3_string ) ;
_au1_n -> _name_n_sto = 18 ;
_au1_n -> _expr__O2.__C2_tp = _au3_fr -> _expr__O2.__C2_tp ;
return _au1_n ;
}
}
else 
#line 406 "../../src/norm.c"
{ 
#line 407 "../../src/norm.c"
_au1_n = (struct name *)_name__ctor ( (struct name *)0 , make_name ( (unsigned char )'D' ) ) ;
_au1_n -> _expr__O2.__C2_tp = (struct type *)_au0_this ;

#line 410 "../../src/norm.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"NX inDL", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 410 "../../src/norm.c"

#line 411 "../../src/norm.c"
return _au1_n ;
}
}
}
;
char _name_hide (_au0_this )
#line 607 "../../src/cfront.h"
struct name *_au0_this ;

#line 420 "../../src/norm.c"
{ 
#line 420 "../../src/norm.c"
struct name_list *_au0__Xthis__ctor_name_list ;

#line 422 "../../src/norm.c"
if (_au0_this -> _node_base != 123 )return ;
if (_au0_this -> _node_n_key == 0 ){ 
#line 425 "../../src/norm.c"
if (_au0_this -> _name_lex_level == bl_level ){ 
#line 426 "../../src/norm.c"
if (_au0_this -> _expr__O2.__C2_tp -> _node_base != 119 ){ 
#line 431 "../../src/norm.c"
struct
#line 431 "../../src/norm.c"
ea _au0__V53 ;

#line 426 "../../src/norm.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"%n redefined", (struct ea *)( ( ((& _au0__V53 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((&
#line 426 "../../src/norm.c"
_au0__V53 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
modified_tn = (struct name_list *)( (_au0__Xthis__ctor_name_list = 0 ), ( (_au0__Xthis__ctor_name_list = (struct name_list *)_new ( (long )(sizeof (struct name_list))) ),
#line 428 "../../src/norm.c"
( (_au0__Xthis__ctor_name_list -> _name_list_f = _au0_this ), ( (_au0__Xthis__ctor_name_list -> _name_list_l = modified_tn ), ((_au0__Xthis__ctor_name_list ))) ) ) ) ;
_au0_this -> _node_n_key = 159 ;
}
}
;
Pname Ntncheck ;

#line 435 "../../src/norm.c"
extern char set_scope (_au0_tn )Pname _au0_tn ;

#line 439 "../../src/norm.c"
{ 
#line 440 "../../src/norm.c"
Plist _au1_l ;
Pname _au1_n ;
int _au1_i ;

#line 446 "../../src/norm.c"
Pbase _au1_b ;

#line 453 "../../src/norm.c"
Pclass _au1_cl ;

#line 454 "../../src/norm.c"
struct name_list *_au0__Xthis__ctor_name_list ;

#line 440 "../../src/norm.c"
_au1_l = 0 ;
_au1_n = 0 ;
_au1_i = 1 ;

#line 444 "../../src/norm.c"
if (_au0_tn -> _node_base != 123 )
#line 445 "../../src/norm.c"
{ 
#line 468 "../../src/norm.c"
struct ea _au0__V54 ;

#line 468 "../../src/norm.c"
struct ea _au0__V55 ;

#line 445 "../../src/norm.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"set_scope: not aTN %d %d", (struct ea *)( ( ((& _au0__V54 )-> _ea__O1.__C1_p = ((char *)_au0_tn )), (((&
#line 445 "../../src/norm.c"
_au0__V54 )))) ) , (struct ea *)( ( ((& _au0__V55 )-> _ea__O1.__C1_i = ((int )_au0_tn -> _node_base )), (((& _au0__V55 ))))
#line 445 "../../src/norm.c"
) , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} _au1_b = (((struct basetype *)_au0_tn -> _expr__O2.__C2_tp ));

#line 448 "../../src/norm.c"
if ((_au1_b -> _basetype_b_name == 0 )|| (_au1_b -> _basetype_b_name -> _expr__O2.__C2_tp -> _node_base != 6 ))
#line 449 "../../src/norm.c"
{ 
#line 468 "../../src/norm.c"
struct ea _au0__V56 ;

#line 449 "../../src/norm.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"T of%n not aC", (struct ea *)( ( ((& _au0__V56 )-> _ea__O1.__C1_p = ((char *)_au0_tn )), (((&
#line 449 "../../src/norm.c"
_au0__V56 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} 
#line 453 "../../src/norm.c"
_au1_cl = (((struct classdef *)_au1_b -> _basetype_b_name -> _expr__O2.__C2_tp ));
if ((! Ntncheck )|| strcmp ( (char *)_au0_tn -> _expr__O3.__C3_string , (char *)Ntncheck -> _expr__O3.__C3_string ) ){ 
#line 455 "../../src/norm.c"
{ Pname _au2_nn ;

#line 456 "../../src/norm.c"
struct name_list *_au0__Xthis__ctor_name_list ;

#line 455 "../../src/norm.c"
_au2_nn = _table_get_mem ( _au1_cl -> _classdef_memtbl , _au1_i ) ;

#line 455 "../../src/norm.c"
for(;_au2_nn ;_au2_nn = _table_get_mem ( _au1_cl -> _classdef_memtbl , ++ _au1_i ) ) 
#line 457 "../../src/norm.c"
if (_au1_n = _table_look ( ktbl , _au2_nn -> _expr__O3.__C3_string , (unsigned
#line 457 "../../src/norm.c"
char )0 ) )
#line 458 "../../src/norm.c"
_au1_l = (struct name_list *)( (_au0__Xthis__ctor_name_list = 0 ), ( (_au0__Xthis__ctor_name_list = (struct name_list *)_new ( (long )(sizeof (struct name_list)))
#line 458 "../../src/norm.c"
), ( (_au0__Xthis__ctor_name_list -> _name_list_f = _au1_n ), ( (_au0__Xthis__ctor_name_list -> _name_list_l = _au1_l ), ((_au0__Xthis__ctor_name_list ))) ) ) ) ;
#line 458 "../../src/norm.c"

#line 459 "../../src/norm.c"
_au1_cl -> _classdef_tn_list = _au1_l ;
Ntncheck = _au0_tn ;
}
}
for(_au1_l = _au1_cl -> _classdef_tn_list ;_au1_l ;_au1_l = _au1_l -> _name_list_l ) { 
#line 464 "../../src/norm.c"
_au1_n = _au1_l -> _name_list_f ;
_au1_n -> _node_n_key = (_au1_n -> _name_lex_level ? 0: 159 );
modified_tn = (struct name_list *)( (_au0__Xthis__ctor_name_list = 0 ), ( (_au0__Xthis__ctor_name_list = (struct name_list *)_new ( (long )(sizeof (struct name_list))) ), (
#line 466 "../../src/norm.c"
(_au0__Xthis__ctor_name_list -> _name_list_f = _au1_n ), ( (_au0__Xthis__ctor_name_list -> _name_list_l = modified_tn ), ((_au0__Xthis__ctor_name_list ))) ) ) ) ;
}
}
;
extern char restore ()
#line 471 "../../src/norm.c"
{ 
#line 472 "../../src/norm.c"
Plist _au1_l ;

#line 472 "../../src/norm.c"
for(_au1_l = modified_tn ;_au1_l ;_au1_l = _au1_l -> _name_list_l ) { 
#line 473 "../../src/norm.c"
Pname _au2_n ;

#line 473 "../../src/norm.c"
_au2_n = _au1_l -> _name_list_f ;
_au2_n -> _node_n_key = ((_au2_n -> _name_lex_level <= bl_level )? 0: 159 );
}
}
;
extern Pbase start_cl (_au0_t , _au0_c , _au0_b )TOK _au0_t ;

#line 478 "../../src/norm.c"
Pname _au0_c ;

#line 478 "../../src/norm.c"
Pname _au0_b ;
{ 
#line 482 "../../src/norm.c"
Pname _au1_n ;

#line 484 "../../src/norm.c"
Pbase _au1_bt ;

#line 489 "../../src/norm.c"
Pclass _au1_occl ;

#line 480 "../../src/norm.c"
if (_au0_c == 0 )_au0_c = (struct name *)_name__ctor ( (struct name *)0 , make_name ( (unsigned char )'C' ) ) ;

#line 482 "../../src/norm.c"
_au1_n = _name_tname ( _au0_c , _au0_t ) ;
_au1_n -> _name_where = curloc ;
_au1_bt = (((struct basetype *)_au1_n -> _expr__O2.__C2_tp ));
if (_au1_bt -> _node_base != 119 ){ 
#line 486 "../../src/norm.c"
{ 
#line 503 "../../src/norm.c"
struct ea _au0__V57 ;

#line 503 "../../src/norm.c"
struct ea _au0__V58 ;

#line 486 "../../src/norm.c"
error ( (char *)"twoDs of%n:%t andC", (struct ea *)( ( ((& _au0__V57 )-> _ea__O1.__C1_p = ((char *)_au1_n )), (((& _au0__V57 )))) )
#line 486 "../../src/norm.c"
, (struct ea *)( ( ((& _au0__V58 )-> _ea__O1.__C1_p = ((char *)_au1_bt )), (((& _au0__V58 )))) ) , (struct
#line 486 "../../src/norm.c"
ea *)ea0 , (struct ea *)ea0 ) ;
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"can't recover from previous errors", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 487 "../../src/norm.c"
} }

#line 489 "../../src/norm.c"
_au1_occl = ccl ;
ccl = (((struct classdef *)_au1_bt -> _basetype_b_name -> _expr__O2.__C2_tp ));
if (ccl -> _type_defined ){ 
#line 493 "../../src/norm.c"
ccl -> _type_defined |= 010 ;
}
ccl -> _type_defined |= 04 ;
if (ccl -> _classdef_in_class = _au1_occl )_au1_occl -> _classdef_tn_list = modified_tn ;
modified_tn = 0 ;
Ntncheck = 0 ;
ccl -> _classdef_string = _au1_n -> _expr__O3.__C3_string ;
ccl -> _classdef_csu = _au0_t ;
if (_au0_b )ccl -> _classdef_clbase = _name_tname ( _au0_b , _au0_t ) ;
return _au1_bt ;
}
;
extern char end_cl ()
#line 506 "../../src/norm.c"
{ 
#line 507 "../../src/norm.c"
Pclass _au1_occl ;
Plist _au1_ol ;

#line 507 "../../src/norm.c"
_au1_occl = ccl -> _classdef_in_class ;
_au1_ol = (_au1_occl ? _au1_occl -> _classdef_tn_list : (((struct name_list *)0 )));
ccl -> _classdef_c_body = 2 ;

#line 511 "../../src/norm.c"
if (modified_tn ){ 
#line 512 "../../src/norm.c"
Plist _au2_local ;
Plist _au2_l ;

#line 513 "../../src/norm.c"
Plist _au2_nl ;

#line 512 "../../src/norm.c"
_au2_local = 0 ;
{ _au2_l = modified_tn ;

#line 513 "../../src/norm.c"
_au2_nl = 0 ;
for(;_au2_l ;_au2_l = _au2_nl ) 
#line 513 "../../src/norm.c"
{ 
#line 514 "../../src/norm.c"
_au2_nl = _au2_l -> _name_list_l ;
{ Pname _au3_n ;

#line 515 "../../src/norm.c"
_au3_n = _au2_l -> _name_list_f ;
if (_table_look ( ktbl , _au3_n -> _expr__O3.__C3_string , (unsigned char )0 ) ){ 
#line 518 "../../src/norm.c"
_au2_l -> _name_list_l = _au1_ol ;
_au1_ol = _au2_l ;
}
else { 
#line 522 "../../src/norm.c"
_au2_l -> _name_list_l = _au2_local ;
_au2_local = _au2_l ;
}
}
}
} 
#line 526 "../../src/norm.c"
if (ccl -> _classdef_tn_list = (modified_tn = _au2_local ))restore ( ) ;
}
modified_tn = _au1_ol ;
ccl = _au1_occl ;
}
;
extern Pbase end_enum (_au0_n , _au0_b )Pname _au0_n ;

#line 532 "../../src/norm.c"
Pname _au0_b ;
{ 
#line 536 "../../src/norm.c"
Pbase _au1_bt ;

#line 541 "../../src/norm.c"
Penum _au1_en ;

#line 534 "../../src/norm.c"
if (_au0_n == 0 )_au0_n = (struct name *)_name__ctor ( (struct name *)0 , make_name ( (unsigned char )'E' ) ) ;
_au0_n = _name_tname ( _au0_n , (unsigned char )13 ) ;
_au1_bt = (((struct basetype *)_au0_n -> _expr__O2.__C2_tp ));
if (_au1_bt -> _node_base != 121 ){ 
#line 538 "../../src/norm.c"
{ 
#line 550 "../../src/norm.c"
struct ea _au0__V59 ;

#line 550 "../../src/norm.c"
struct ea _au0__V60 ;

#line 538 "../../src/norm.c"
error ( (char *)"twoDs of%n:%t and enum", (struct ea *)( ( ((& _au0__V59 )-> _ea__O1.__C1_p = ((char *)_au0_n )), (((& _au0__V59 )))) )
#line 538 "../../src/norm.c"
, (struct ea *)( ( ((& _au0__V60 )-> _ea__O1.__C1_p = ((char *)_au1_bt )), (((& _au0__V60 )))) ) , (struct
#line 538 "../../src/norm.c"
ea *)ea0 , (struct ea *)ea0 ) ;
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"can't recover from previous errors", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 539 "../../src/norm.c"
} }

#line 541 "../../src/norm.c"
_au1_en = (((struct enumdef *)_au1_bt -> _basetype_b_name -> _expr__O2.__C2_tp ));
_au1_en -> _enumdef_e_body = 2 ;
_au1_en -> _enumdef_mem = name_unlist ( ((struct nlist *)_au0_b )) ;
if (_au1_en -> _type_defined ){ 
#line 545 "../../src/norm.c"
{ 
#line 550 "../../src/norm.c"
struct ea _au0__V61 ;

#line 545 "../../src/norm.c"
error ( (char *)"enum%n defined twice", (struct ea *)( ( ((& _au0__V61 )-> _ea__O1.__C1_p = ((char *)_au0_n )), (((& _au0__V61 )))) )
#line 545 "../../src/norm.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
_au1_en -> _type_defined |= 010 ;
} }
_au1_en -> _type_defined |= 04 ;
return _au1_bt ;
}
;
Pname _name_tdef (_au0_this )
#line 607 "../../src/cfront.h"
struct name *_au0_this ;

#line 556 "../../src/norm.c"
{ 
#line 558 "../../src/norm.c"
Pname _au1_n ;

#line 559 "../../src/norm.c"
struct name_list *_au0__Xthis__ctor_name_list ;

#line 558 "../../src/norm.c"
_au1_n = _table_insert ( ktbl , _au0_this , (unsigned char )0 ) ;
if (_au0_this -> _expr__O2.__C2_tp == 0 ){ 
#line 565 "../../src/norm.c"
struct ea _au0__V62 ;

#line 559 "../../src/norm.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"Tdef%n tp==0", (struct ea *)( ( ((& _au0__V62 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((&
#line 559 "../../src/norm.c"
_au0__V62 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} _au1_n -> _node_base = (_au0_this -> _node_base = 123 );
_au1_n -> _node_permanent = 1 ;
_au0_this -> _expr__O2.__C2_tp -> _node_permanent = 1 ;
modified_tn = (struct name_list *)( (_au0__Xthis__ctor_name_list = 0 ), ( (_au0__Xthis__ctor_name_list = (struct name_list *)_new ( (long )(sizeof (struct name_list))) ), (
#line 563 "../../src/norm.c"
(_au0__Xthis__ctor_name_list -> _name_list_f = _au1_n ), ( (_au0__Xthis__ctor_name_list -> _name_list_l = modified_tn ), ((_au0__Xthis__ctor_name_list ))) ) ) ) ;
return _au1_n ;
}
;
Pname _name_tname (_au0_this , _au0_csu )
#line 607 "../../src/cfront.h"
struct name *_au0_this ;

#line 567 "../../src/norm.c"
TOK _au0_csu ;

#line 575 "../../src/norm.c"
{ 
#line 577 "../../src/norm.c"
switch (_au0_this -> _node_base ){ 
#line 578 "../../src/norm.c"
case 123 : 
#line 579 "../../src/norm.c"
return _au0_this ;
case 85 : 
#line 581 "../../src/norm.c"
{ Pname _au3_tn ;

#line 583 "../../src/norm.c"
Pname _au3_on ;

#line 584 "../../src/norm.c"
struct name_list *_au0__Xthis__ctor_name_list ;

#line 584 "../../src/norm.c"
struct enumdef *_au0__Xthis__ctor_enumdef ;

#line 581 "../../src/norm.c"
_au3_tn = _table_insert ( ktbl , _au0_this , (unsigned char )0 ) ;

#line 583 "../../src/norm.c"
_au3_on = (struct name *)_name__ctor ( (struct name *)0 , (char *)0 ) ;
_au3_tn -> _node_base = 123 ;
_au3_tn -> _name_lex_level = _au0_this -> _name_lex_level ;
modified_tn = (struct name_list *)( (_au0__Xthis__ctor_name_list = 0 ), ( (_au0__Xthis__ctor_name_list = (struct name_list *)_new ( (long )(sizeof (struct name_list))) ), (
#line 586 "../../src/norm.c"
(_au0__Xthis__ctor_name_list -> _name_list_f = _au3_tn ), ( (_au0__Xthis__ctor_name_list -> _name_list_l = modified_tn ), ((_au0__Xthis__ctor_name_list ))) ) ) ) ;
_au3_tn -> _name_n_list = (_au0_this -> _name_n_list = 0 );
_au0_this -> _expr__O3.__C3_string = _au3_tn -> _expr__O3.__C3_string ;
(*_au3_on )= (*_au0_this );
switch (_au0_csu ){ 
#line 591 "../../src/norm.c"
case 13 : 
#line 592 "../../src/norm.c"
_au3_tn -> _expr__O2.__C2_tp = (struct type *)_basetype__ctor ( (struct basetype *)0 , (unsigned char )121 , _au3_on )
#line 592 "../../src/norm.c"
;
_au3_on -> _expr__O2.__C2_tp = (struct type *)( (_au0__Xthis__ctor_enumdef = 0 ), ( (_au0__Xthis__ctor_enumdef = (struct enumdef *)_new ( (long )(sizeof (struct enumdef))) ),
#line 593 "../../src/norm.c"
( (_au0__Xthis__ctor_enumdef -> _node_base = 13 ), ( (_au0__Xthis__ctor_enumdef -> _enumdef_mem = ((struct name *)0 )), ((_au0__Xthis__ctor_enumdef ))) ) ) ) ;
#line 593 "../../src/norm.c"

#line 594 "../../src/norm.c"
break ;
default : 
#line 596 "../../src/norm.c"
_au3_on -> _expr__O2.__C2_tp = (struct type *)_classdef__ctor ( (struct classdef *)0 , _au0_csu ) ;
(((struct classdef *)_au3_on -> _expr__O2.__C2_tp ))-> _classdef_string = _au3_tn -> _expr__O3.__C3_string ;
_au3_tn -> _expr__O2.__C2_tp = (struct type *)_basetype__ctor ( (struct basetype *)0 , (unsigned char )119 , _au3_on ) ;
(((struct basetype *)_au3_tn -> _expr__O2.__C2_tp ))-> _basetype_b_table = (((struct classdef *)_au3_on -> _expr__O2.__C2_tp ))-> _classdef_memtbl ;
}
_au3_tn -> _node_permanent = 1 ;
_au3_tn -> _expr__O2.__C2_tp -> _node_permanent = 1 ;
_au3_on -> _node_permanent = 1 ;
_au3_on -> _expr__O2.__C2_tp -> _node_permanent = 1 ;

#line 606 "../../src/norm.c"
return _au3_tn ;
}
default : 
#line 609 "../../src/norm.c"
{ 
#line 611 "../../src/norm.c"
struct ea _au0__V63 ;

#line 611 "../../src/norm.c"
struct ea _au0__V64 ;

#line 611 "../../src/norm.c"
struct ea _au0__V65 ;

#line 609 "../../src/norm.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"tname(%s %d %k)", (struct ea *)( ( ((& _au0__V63 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _expr__O3.__C3_string )),
#line 609 "../../src/norm.c"
(((& _au0__V63 )))) ) , (struct ea *)( ( ((& _au0__V64 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V64 ))))
#line 609 "../../src/norm.c"
) , (struct ea *)( ( ((& _au0__V65 )-> _ea__O1.__C1_i = ((int )_au0_this -> _node_base )), (((& _au0__V65 )))) )
#line 609 "../../src/norm.c"
, (struct ea *)ea0 ) ;
} }
}
;

#line 614 "../../src/norm.c"
Pname _name_normalize (_au0_this , _au0_b , _au0_bl , _au0_cast )
#line 607 "../../src/cfront.h"
struct name *_au0_this ;

#line 614 "../../src/norm.c"
Pbase _au0_b ;

#line 614 "../../src/norm.c"
Pblock _au0_bl ;

#line 614 "../../src/norm.c"
bit _au0_cast ;

#line 629 "../../src/norm.c"
{ 
#line 630 "../../src/norm.c"
Pname _au1_n ;
Pname _au1_nn ;
TOK _au1_stc ;
bit _au1_tpdf ;
bit _au1_inli ;
bit _au1_virt ;

#line 648 "../../src/norm.c"
Pfct _au1_f ;
Pname _au1_nx ;

#line 636 "../../src/norm.c"
if (_au0_b ){ 
#line 637 "../../src/norm.c"
_au1_stc = _au0_b -> _basetype_b_sto ;
_au1_tpdf = _au0_b -> _basetype_b_typedef ;
_au1_inli = _au0_b -> _basetype_b_inline ;
_au1_virt = _au0_b -> _basetype_b_virtual ;
}
else { 
#line 643 "../../src/norm.c"
_au1_stc = 0 ;
_au1_tpdf = 0 ;
_au1_inli = 0 ;
_au1_virt = 0 ;
}
;

#line 648 "../../src/norm.c"
;

#line 654 "../../src/norm.c"
if (_au1_inli && (_au1_stc == 14 )){ 
#line 655 "../../src/norm.c"
error ( (char *)"both extern and inline", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 655 "../../src/norm.c"
ea *)ea0 ) ;
_au1_inli = 0 ;
}

#line 660 "../../src/norm.c"
if ((_au1_stc == 18 )&& (_au0_this -> _expr__O2.__C2_tp == 0 )){ 
#line 668 "../../src/norm.c"
if (_au0_b -> _node_base )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )0 , (char *)"T specified for friend", (struct
#line 668 "../../src/norm.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
if (_au0_this -> _name_n_list ){ 
#line 670 "../../src/norm.c"
error ( (char *)"L of friends", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 )
#line 670 "../../src/norm.c"
;
_au0_this -> _name_n_list = 0 ;
}
{ Pname _au2_nx ;

#line 673 "../../src/norm.c"
_au2_nx = _name_tname ( _au0_this , (unsigned char )6 ) ;
modified_tn = modified_tn -> _name_list_l ;
_au0_this -> _name_n_sto = 18 ;
_au0_this -> _expr__O2.__C2_tp = _au2_nx -> _expr__O2.__C2_tp ;
return _au0_this ;
}
}
if ((_au0_this -> _expr__O2.__C2_tp && (_au0_this -> _expr__O2.__C2_tp -> _node_base == 108 ))&& ((_au0_this -> _name_n_oper == 123 )|| (((struct fct *)_au0_this -> _expr__O2.__C2_tp ))-> _fct_returns ))
#line 682 "../../src/norm.c"
{
#line 682 "../../src/norm.c"

#line 683 "../../src/norm.c"
Pfct _au2_f ;
Pfct _au2_f2 ;

#line 683 "../../src/norm.c"
_au2_f = (((struct fct *)_au0_this -> _expr__O2.__C2_tp ));
_au2_f2 = (((struct fct *)_au2_f -> _fct_returns ));

#line 687 "../../src/norm.c"
if (_au2_f2 ){ 
#line 688 "../../src/norm.c"
Ptype _au3_pt ;
Ptype _au3_t ;

#line 689 "../../src/norm.c"
_au3_t = (struct type *)_au2_f2 ;
lxlx :
#line 692 "../../src/norm.c"
switch (_au3_t -> _node_base ){ 
#line 693 "../../src/norm.c"
case 125 : 
#line 694 "../../src/norm.c"
case 110 : 
#line 695 "../../src/norm.c"
if (_au3_pt = (((struct ptr *)_au3_t ))-> _pvtyp_typ ){ 
#line 696 "../../src/norm.c"
if (_au3_pt ->
#line 696 "../../src/norm.c"
_node_base == 97 ){ 
#line 697 "../../src/norm.c"
(((struct ptr *)_au3_t ))-> _pvtyp_typ = 0 ;
_au0_b = (((struct basetype *)_au3_pt ));
_au1_stc = _au0_b -> _basetype_b_sto ;
_au1_tpdf = _au0_b -> _basetype_b_typedef ;
_au1_inli = _au0_b -> _basetype_b_inline ;
_au1_virt = _au0_b -> _basetype_b_virtual ;
}
else { 
#line 705 "../../src/norm.c"
_au3_t = _au3_pt ;
goto lxlx ;
}
}
goto zse1 ;
case 108 : 
#line 711 "../../src/norm.c"
{ Pexpr _au5_e ;

#line 711 "../../src/norm.c"
_au5_e = (struct expr *)_au2_f2 -> _fct_argtype ;

#line 713 "../../src/norm.c"
if (_au5_e && (_au5_e -> _node_base == 140 )){ 
#line 714 "../../src/norm.c"
if (_au5_e -> _expr__O4.__C4_e2 )goto zse1 ;
if (_au5_e -> _expr__O3.__C3_e1 -> _node_base != 111 )goto zse1 ;
{ Pexpr _au6_ee ;
Ptype _au6_t ;
Ptype _au6_tx ;

#line 716 "../../src/norm.c"
_au6_ee = _au5_e -> _expr__O3.__C3_e1 ;
_au6_t = 0 ;

#line 719 "../../src/norm.c"
ldld :
#line 720 "../../src/norm.c"
switch (_au6_ee -> _node_base ){ 
#line 721 "../../src/norm.c"
case 111 : 
#line 722 "../../src/norm.c"
{ Ptype _au8_tt ;

#line 723 "../../src/norm.c"
struct ptr *_au0__Xthis__ctor_ptr ;

#line 723 "../../src/norm.c"
struct vec *_au0__Xthis__ctor_vec ;

#line 722 "../../src/norm.c"
_au8_tt = (_au6_ee -> _expr__O4.__C4_e2 ? (((struct type *)( (_au0__Xthis__ctor_vec = 0 ), ( (_au0__Xthis__ctor_vec = (struct vec *)_new ( (long )(sizeof (struct vec)))
#line 722 "../../src/norm.c"
), ( (Nt ++ ), ( (_au0__Xthis__ctor_vec -> _node_base = 110 ), ( (_au0__Xthis__ctor_vec -> _pvtyp_typ = ((struct type *)0 )), (
#line 722 "../../src/norm.c"
(_au0__Xthis__ctor_vec -> _vec_dim = _au6_ee -> _expr__O4.__C4_e2 ), ((_au0__Xthis__ctor_vec ))) ) ) ) ) ) )): (((struct type *)( (_au0__Xthis__ctor_ptr =
#line 722 "../../src/norm.c"
0 ), ( (_au0__Xthis__ctor_ptr = (struct ptr *)_new ( (long )(sizeof (struct ptr))) ), ( (Nt ++ ), ( (_au0__Xthis__ctor_ptr ->
#line 722 "../../src/norm.c"
_node_base = ((unsigned char )125 )), ( (_au0__Xthis__ctor_ptr -> _pvtyp_typ = ((struct type *)0 )), ( (_au0__Xthis__ctor_ptr -> _ptr_rdo = ((char )0 )),
#line 722 "../../src/norm.c"
((_au0__Xthis__ctor_ptr ))) ) ) ) ) ) )));
if (_au6_t )
#line 724 "../../src/norm.c"
(((struct ptr *)_au6_t ))-> _pvtyp_typ = _au8_tt ;
else 
#line 726 "../../src/norm.c"
_au6_tx = _au8_tt ;
_au6_t = _au8_tt ;
_au6_ee = _au6_ee -> _expr__O3.__C3_e1 ;
goto ldld ;
}
case 85 : 
#line 732 "../../src/norm.c"
{ Pname _au8_rn ;

#line 732 "../../src/norm.c"
_au8_rn = (((struct name *)_au6_ee ));
_au0_b = (struct basetype *)_basetype__ctor ( (struct basetype *)0 , (unsigned char )97 , _table_look ( ktbl , _au0_this -> _expr__O3.__C3_string , (unsigned char
#line 733 "../../src/norm.c"
)0 ) ) ;
_au2_f -> _fct_returns = _au6_tx ;
_au0_this -> _name_n_oper = 0 ;
_au0_this -> _expr__O3.__C3_string = _au8_rn -> _expr__O3.__C3_string ;
_au0_this -> _node_base = 85 ;
}
}
}
}
}
}
}
}

#line 745 "../../src/norm.c"
zse1 :
#line 747 "../../src/norm.c"
if (_au0_b == 0 ){ 
#line 748 "../../src/norm.c"
{ 
#line 947 "../../src/norm.c"
struct ea _au0__V66 ;

#line 748 "../../src/norm.c"
error ( (char *)"BTX for %s", (struct ea *)( ( ((& _au0__V66 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _expr__O3.__C3_string )), (((& _au0__V66 ))))
#line 748 "../../src/norm.c"
) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
_au0_b = (((struct basetype *)defa_type ));
} }
if (_au0_cast )_au0_this -> _expr__O3.__C3_string = "";
_au0_b = _basetype_check ( _au0_b , _au0_this ) ;

#line 754 "../../src/norm.c"
switch (_au0_b -> _node_base ){ 
#line 756 "../../src/norm.c"
case 119 : 
#line 757 "../../src/norm.c"
_au1_nn = _au0_b -> _basetype_b_name ;

#line 759 "../../src/norm.c"
if ((((struct classdef *)_au1_nn -> _expr__O2.__C2_tp ))-> _classdef_c_body == 2 ){ 
#line 760 "../../src/norm.c"
if (_au0_this -> _expr__O2.__C2_tp && (_au0_this -> _expr__O2.__C2_tp -> _node_base == 108 )){ 
#line 761 "../../src/norm.c"
{
#line 761 "../../src/norm.c"

#line 947 "../../src/norm.c"
struct ea _au0__V67 ;

#line 947 "../../src/norm.c"
struct ea _au0__V68 ;

#line 947 "../../src/norm.c"
struct ea _au0__V69 ;

#line 761 "../../src/norm.c"
errorFI_PCloc__PC_RCea__RCea__RCea__RCea___ ( (int )'s' , & _au0_this -> _name_where , (char *)"%k%n defined as returnT for%n (did you forget a ';' after '}' ?)", (struct ea *)( ( ((& _au0__V67 )-> _ea__O1.__C1_i =
#line 761 "../../src/norm.c"
((int )(((struct classdef *)_au1_nn -> _expr__O2.__C2_tp ))-> _classdef_csu )), (((& _au0__V67 )))) ) , (struct ea *)( ( ((& _au0__V68 )->
#line 761 "../../src/norm.c"
_ea__O1.__C1_p = ((char *)_au1_nn )), (((& _au0__V68 )))) ) , (struct ea *)( ( ((& _au0__V69 )-> _ea__O1.__C1_p = ((char
#line 761 "../../src/norm.c"
*)_au0_this )), (((& _au0__V69 )))) ) , (struct ea *)ea0 ) ;
_au1_nn = _au0_this ;
break ;
} }
_au1_nn -> _name_n_list = _au0_this ;
(((struct classdef *)_au1_nn -> _expr__O2.__C2_tp ))-> _classdef_c_body = 1 ;
}
else 
#line 769 "../../src/norm.c"
_au1_nn = _au0_this ;
break ;
case 121 : 
#line 772 "../../src/norm.c"
_au1_nn = _au0_b -> _basetype_b_name ;
if ((((struct enumdef *)_au1_nn -> _expr__O2.__C2_tp ))-> _enumdef_e_body == 2 ){ 
#line 774 "../../src/norm.c"
if (_au0_this -> _expr__O2.__C2_tp && (_au0_this -> _expr__O2.__C2_tp -> _node_base == 108 )){ 
#line 775 "../../src/norm.c"
{
#line 775 "../../src/norm.c"

#line 947 "../../src/norm.c"
struct ea _au0__V70 ;

#line 947 "../../src/norm.c"
struct ea _au0__V71 ;

#line 775 "../../src/norm.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"enum%n defined as returnT for%n (did you forget a ';'?)", (struct ea *)( ( ((& _au0__V70 )-> _ea__O1.__C1_p = ((char *)_au1_nn )), (((&
#line 775 "../../src/norm.c"
_au0__V70 )))) ) , (struct ea *)( ( ((& _au0__V71 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V71 )))) )
#line 775 "../../src/norm.c"
, (struct ea *)ea0 , (struct ea *)ea0 ) ;
_au1_nn = _au0_this ;
break ;
} }
_au1_nn -> _name_n_list = _au0_this ;
(((struct enumdef *)_au1_nn -> _expr__O2.__C2_tp ))-> _enumdef_e_body = 1 ;
}
else 
#line 783 "../../src/norm.c"
_au1_nn = _au0_this ;
break ;
default : 
#line 786 "../../src/norm.c"
_au1_nn = _au0_this ;
}

#line 789 "../../src/norm.c"
for(_au1_n = _au0_this ;_au1_n ;_au1_n = _au1_nx ) { 
#line 790 "../../src/norm.c"
Ptype _au2_t ;

#line 790 "../../src/norm.c"
_au2_t = _au1_n -> _expr__O2.__C2_tp ;
_au1_nx = _au1_n -> _name_n_list ;
_au1_n -> _name_n_sto = _au1_stc ;

#line 794 "../../src/norm.c"
if (_au1_n -> _node_base == 123 ){ 
#line 947 "../../src/norm.c"
struct ea _au0__V72 ;

#line 794 "../../src/norm.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"redefinition ofTN%n", (struct ea *)( ( ((& _au0__V72 )-> _ea__O1.__C1_p = ((char *)_au1_n )), (((&
#line 794 "../../src/norm.c"
_au0__V72 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} 
#line 796 "../../src/norm.c"
if (_au2_t == 0 ){ 
#line 797 "../../src/norm.c"
if (_au0_bl == 0 )
#line 798 "../../src/norm.c"
_au1_n -> _expr__O2.__C2_tp = (_au2_t = (struct type *)_au0_b );
else { 
#line 800 "../../src/norm.c"
{ 
#line 947 "../../src/norm.c"
struct ea _au0__V73 ;

#line 800 "../../src/norm.c"
error ( (char *)"body of nonF%n", (struct ea *)( ( ((& _au0__V73 )-> _ea__O1.__C1_p = ((char *)_au1_n )), (((& _au0__V73 )))) )
#line 800 "../../src/norm.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
_au2_t = (struct type *)_fct__ctor ( (struct fct *)0 , (struct type *)defa_type , (struct name *)0 , (unsigned char )0 ) ;
} }
}

#line 805 "../../src/norm.c"
switch (_au2_t -> _node_base ){ 
#line 806 "../../src/norm.c"
case 125 : 
#line 807 "../../src/norm.c"
case 158 : 
#line 808 "../../src/norm.c"
_au1_n -> _expr__O2.__C2_tp = _ptr_normalize ( ((struct ptr *)_au2_t ), (struct
#line 808 "../../src/norm.c"
type *)_au0_b ) ;
break ;
case 110 : 
#line 811 "../../src/norm.c"
_au1_n -> _expr__O2.__C2_tp = _vec_normalize ( ((struct vec *)_au2_t ), (struct type *)_au0_b ) ;
break ;
case 108 : 
#line 814 "../../src/norm.c"
_au1_n -> _expr__O2.__C2_tp = _fct_normalize ( ((struct fct *)_au2_t ), (struct type *)_au0_b ) ;
break ;
case 114 : 
#line 817 "../../src/norm.c"
if (_au1_n -> _expr__O3.__C3_string == 0 )_au1_n -> _expr__O3.__C3_string = make_name ( (unsigned char )'F' ) ;
_au1_n -> _expr__O2.__C2_tp = _au2_t ;
{ Pbase _au3_tb ;

#line 819 "../../src/norm.c"
_au3_tb = _au0_b ;
flatten :
#line 822 "../../src/norm.c"
switch (_au3_tb -> _node_base ){ 
#line 823 "../../src/norm.c"
case 97 : 
#line 824 "../../src/norm.c"
_au3_tb = (((struct basetype *)_au3_tb -> _basetype_b_name -> _expr__O2.__C2_tp ));
goto flatten ;
case 21 : 
#line 827 "../../src/norm.c"
(((struct basetype *)_au2_t ))-> _basetype_b_fieldtype = (struct type *)(_au0_b -> _basetype_b_unsigned ? uint_type : int_type );
goto iii ;
case 5 : 
#line 830 "../../src/norm.c"
(((struct basetype *)_au2_t ))-> _basetype_b_fieldtype = (struct type *)(_au0_b -> _basetype_b_unsigned ? uchar_type : char_type );
goto iii ;
case 29 : 
#line 833 "../../src/norm.c"
(((struct basetype *)_au2_t ))-> _basetype_b_fieldtype = (struct type *)(_au0_b -> _basetype_b_unsigned ? ushort_type : short_type );
goto iii ;
iii :
#line 836 "../../src/norm.c"
(((struct basetype *)_au2_t ))-> _basetype_b_unsigned = _au0_b -> _basetype_b_unsigned ;
(((struct basetype *)_au2_t ))-> _basetype_b_const = _au0_b -> _basetype_b_const ;
break ;
default : 
#line 840 "../../src/norm.c"
error ( (char *)"non-int field", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 840 "../../src/norm.c"

#line 841 "../../src/norm.c"
_au1_n -> _expr__O2.__C2_tp = (struct type *)defa_type ;
}
break ;
}
}
_au1_f = (((struct fct *)_au1_n -> _expr__O2.__C2_tp ));

#line 848 "../../src/norm.c"
if (_au1_f -> _node_base != 108 ){ 
#line 849 "../../src/norm.c"
if (_au0_bl ){ 
#line 850 "../../src/norm.c"
{ 
#line 947 "../../src/norm.c"
struct ea _au0__V74 ;

#line 850 "../../src/norm.c"
error ( (char *)"body for nonF%n", (struct ea *)( ( ((& _au0__V74 )-> _ea__O1.__C1_p = ((char *)_au1_n )), (((& _au0__V74 )))) )
#line 850 "../../src/norm.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
_au1_n -> _expr__O2.__C2_tp = (struct type *)(_au1_f = (struct fct *)_fct__ctor ( (struct fct *)0 , (struct type *)defa_type , (struct name *)0 , (unsigned char
#line 851 "../../src/norm.c"
)0 ) );
continue ;
} }
if (_au1_inli ){ 
#line 947 "../../src/norm.c"
struct ea _au0__V75 ;

#line 854 "../../src/norm.c"
error ( (char *)"inline nonF%n", (struct ea *)( ( ((& _au0__V75 )-> _ea__O1.__C1_p = ((char *)_au1_n )), (((& _au0__V75 )))) )
#line 854 "../../src/norm.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} if (_au1_virt ){ 
#line 947 "../../src/norm.c"
struct ea _au0__V76 ;

#line 855 "../../src/norm.c"
error ( (char *)"virtual nonF%n", (struct ea *)( ( ((& _au0__V76 )-> _ea__O1.__C1_p = ((char *)_au1_n )), (((& _au0__V76 )))) )
#line 855 "../../src/norm.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} 
#line 857 "../../src/norm.c"
if (_au1_tpdf ){ 
#line 858 "../../src/norm.c"
if (_au1_n -> _expr__O4.__C4_n_initializer ){ 
#line 859 "../../src/norm.c"
{ 
#line 947 "../../src/norm.c"
struct ea _au0__V77 ;

#line 859 "../../src/norm.c"
error ( (char *)"Ir forTdefN%n", (struct ea *)( ( ((& _au0__V77 )-> _ea__O1.__C1_p = ((char *)_au1_n )), (((& _au0__V77 )))) )
#line 859 "../../src/norm.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
_au1_n -> _expr__O4.__C4_n_initializer = 0 ;
} }
_name_tdef ( _au1_n ) ;
}
continue ;
}

#line 867 "../../src/norm.c"
_au1_f -> _fct_f_inline = _au1_inli ;
_au1_f -> _fct_f_virtual = _au1_virt ;

#line 870 "../../src/norm.c"
if (_au1_tpdf ){ 
#line 871 "../../src/norm.c"
if (_au1_f -> _fct_body = _au0_bl ){ 
#line 947 "../../src/norm.c"
struct ea _au0__V78 ;

#line 871 "../../src/norm.c"
error ( (char *)"Tdef%n { ... }", (struct ea *)( ( ((& _au0__V78 )-> _ea__O1.__C1_p = ((char *)_au1_n )), (((& _au0__V78 )))) )
#line 871 "../../src/norm.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} if (_au1_n -> _name__O6.__C6_n_qualifier ){ 
#line 876 "../../src/norm.c"
_au1_f -> _fct_memof = (((struct classdef *)(((struct basetype *)_au1_n -> _name__O6.__C6_n_qualifier -> _expr__O2.__C2_tp ))-> _basetype_b_name -> _expr__O2.__C2_tp ));
_au1_n -> _name__O6.__C6_n_qualifier = 0 ;
}
_name_tdef ( _au1_n ) ;
continue ;
}

#line 883 "../../src/norm.c"
if (_au1_f -> _fct_body = _au0_bl )continue ;

#line 893 "../../src/norm.c"
{ Pname _au3_cn ;
bit _au3_clob ;

#line 893 "../../src/norm.c"
_au3_cn = _type_is_cl_obj ( _au1_f -> _fct_returns ) ;
_au3_clob = (_au3_cn || cl_obj_vec );

#line 896 "../../src/norm.c"
if (_au1_f -> _fct_argtype ){ 
#line 897 "../../src/norm.c"
Pname _au4_nn ;

#line 898 "../../src/norm.c"
struct texpr *_au0__Xthis__ctor_texpr ;

#line 899 "../../src/norm.c"
for(_au4_nn = _au1_f -> _fct_argtype ;_au4_nn ;_au4_nn = _au4_nn -> _name_n_list ) { 
#line 900 "../../src/norm.c"
if (_au4_nn -> _node_base != 85 ){ 
#line 901 "../../src/norm.c"
if (! _au3_clob ){ 
#line 902 "../../src/norm.c"
{
#line 902 "../../src/norm.c"

#line 947 "../../src/norm.c"
struct ea _au0__V79 ;

#line 902 "../../src/norm.c"
error ( (char *)"ATX for%n", (struct ea *)( ( ((& _au0__V79 )-> _ea__O1.__C1_p = ((char *)_au1_n )), (((& _au0__V79 )))) )
#line 902 "../../src/norm.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
goto zzz ;
} }
goto is_obj ;
}

#line 913 "../../src/norm.c"
if (_au4_nn -> _expr__O2.__C2_tp )goto ok ;
}
if (! _au3_clob ){ 
#line 916 "../../src/norm.c"
error ( (char *)"FALX", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 )
#line 916 "../../src/norm.c"
;
goto zzz ;
}
is_obj :
#line 922 "../../src/norm.c"
_au1_n -> _expr__O2.__C2_tp = _au1_f -> _fct_returns ;
if (_au1_f -> _fct_argtype -> _node_base != 140 )_au1_f -> _fct_argtype = (((struct name *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , ((struct
#line 923 "../../src/norm.c"
expr *)_au1_f -> _fct_argtype ), (struct expr *)0 ) ));
_au1_n -> _expr__O4.__C4_n_initializer = (struct expr *)( (_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr = (struct texpr *)_expr__ctor ( ((struct
#line 924 "../../src/norm.c"
expr *)_au0__Xthis__ctor_texpr ), ((unsigned char )157 ), (((struct expr *)_au1_f -> _fct_argtype )), (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 =
#line 924 "../../src/norm.c"
_au3_cn -> _expr__O2.__C2_tp ), ((_au0__Xthis__ctor_texpr ))) ) ) ;
goto ok ;
zzz :
#line 927 "../../src/norm.c"
if (_au1_f -> _fct_argtype ){ 
#line 928 "../../src/norm.c"
if (_au1_f -> _fct_argtype && (_au1_f -> _fct_argtype -> _node_permanent == 0 ))_name_del ( _au1_f -> _fct_argtype ) ;
_au1_f -> _fct_argtype = 0 ;
_au1_f -> _fct_nargs = 0 ;
_au1_f -> _fct_nargs_known = (! fct_void );
}
}
else { }

#line 942 "../../src/norm.c"
ok :
#line 943 "../../src/norm.c"
;
}
}
return _au1_nn ;
}
;
Ptype _vec_normalize (_au0_this , _au0_vecof )
#line 433 "../../src/cfront.h"
struct vec *_au0_this ;

#line 949 "../../src/norm.c"
Ptype _au0_vecof ;
{ 
#line 952 "../../src/norm.c"
Ptype _au1_t ;

#line 952 "../../src/norm.c"
_au1_t = _au0_this -> _pvtyp_typ ;
_au0_this -> _pvtyp_typ = _au0_vecof ;
if (_au1_t == 0 )return (struct type *)_au0_this ;

#line 956 "../../src/norm.c"
xx :
#line 957 "../../src/norm.c"
switch (_au1_t -> _node_base ){ 
#line 958 "../../src/norm.c"
case 97 : _au1_t = (((struct basetype *)_au1_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 958 "../../src/norm.c"
goto xx ;
case 125 : 
#line 960 "../../src/norm.c"
case 158 : return _ptr_normalize ( ((struct ptr *)_au1_t ), (struct type *)_au0_this ) ;
case 110 : return _vec_normalize ( ((struct vec *)_au1_t ), (struct type *)_au0_this ) ;
case 108 : return _fct_normalize ( ((struct fct *)_au1_t ), (struct type *)_au0_this ) ;
default : { 
#line 965 "../../src/norm.c"
struct ea _au0__V80 ;

#line 963 "../../src/norm.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"bad vectorT(%d)", (struct ea *)( ( ((& _au0__V80 )-> _ea__O1.__C1_i = ((int )_au1_t -> _node_base )),
#line 963 "../../src/norm.c"
(((& _au0__V80 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}
;
Ptype _ptr_normalize (_au0_this , _au0_ptrto )
#line 444 "../../src/cfront.h"
struct ptr *_au0_this ;

#line 967 "../../src/norm.c"
Ptype _au0_ptrto ;
{ 
#line 969 "../../src/norm.c"
int _au1_tconst ;

#line 972 "../../src/norm.c"
Ptype _au1_t ;

#line 969 "../../src/norm.c"
_au1_tconst = 0 ;

#line 972 "../../src/norm.c"
_au1_t = _au0_this -> _pvtyp_typ ;
_au0_this -> _pvtyp_typ = _au0_ptrto ;

#line 975 "../../src/norm.c"
while (_au0_ptrto -> _node_base == 97 ){ 
#line 976 "../../src/norm.c"
if (! _au1_tconst )_au1_tconst = (((struct basetype *)_au0_ptrto ))-> _basetype_b_const ;
_au0_ptrto = (((struct basetype *)_au0_ptrto ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
}

#line 980 "../../src/norm.c"
if (_au0_ptrto -> _node_base == 108 )
#line 981 "../../src/norm.c"
if (_au0_this -> _ptr_memof )
#line 982 "../../src/norm.c"
if ((((struct fct *)_au0_ptrto ))-> _fct_memof ){ 
#line 983 "../../src/norm.c"
if (_au0_this -> _ptr_memof != (((struct fct *)_au0_ptrto ))->
#line 983 "../../src/norm.c"
_fct_memof ){ 
#line 1018 "../../src/norm.c"
struct ea _au0__V81 ;

#line 1018 "../../src/norm.c"
struct ea _au0__V82 ;

#line 983 "../../src/norm.c"
error ( (char *)"P toMF mismatch: %s and %s", (struct ea *)( ( ((& _au0__V81 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _ptr_memof -> _classdef_string )), (((&
#line 983 "../../src/norm.c"
_au0__V81 )))) ) , (struct ea *)( ( ((& _au0__V82 )-> _ea__O1.__C1_p = ((char *)(((struct fct *)_au0_ptrto ))-> _fct_memof -> _classdef_string )),
#line 983 "../../src/norm.c"
(((& _au0__V82 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
else 
#line 986 "../../src/norm.c"
(((struct fct *)_au0_ptrto ))-> _fct_memof = _au0_this -> _ptr_memof ;
else 
#line 988 "../../src/norm.c"
_au0_this -> _ptr_memof = (((struct fct *)_au0_ptrto ))-> _fct_memof ;

#line 990 "../../src/norm.c"
if (_au1_t == 0 ){ 
#line 991 "../../src/norm.c"
Pbase _au2_b ;

#line 991 "../../src/norm.c"
_au2_b = (((struct basetype *)_au0_ptrto ));
if ((((((Pfctvec_type && (_au0_this -> _ptr_rdo == 0 ))&& (_au2_b -> _basetype_b_unsigned == 0 ))&& (_au2_b -> _basetype_b_const == 0 ))&& (_au1_tconst == 0 ))&& (_au0_this ->
#line 992 "../../src/norm.c"
_ptr_memof == 0 ))&& (_au0_this -> _node_base == 125 ))
#line 998 "../../src/norm.c"
{ 
#line 999 "../../src/norm.c"
switch (_au2_b -> _node_base ){ 
#line 1000 "../../src/norm.c"
case 21 : _delete ( (char *)_au0_this ) ;
#line 1000 "../../src/norm.c"
return Pint_type ;
case 5 : _delete ( (char *)_au0_this ) ;

#line 1001 "../../src/norm.c"
return Pchar_type ;
case 38 : _delete ( (char *)_au0_this ) ;

#line 1002 "../../src/norm.c"
return Pvoid_type ;
}
}
if ((_au0_this -> _node_base == 158 )&& (_au2_b -> _node_base == 38 ))error ( (char *)"void& is not a validT", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 1005 "../../src/norm.c"
ea *)ea0 , (struct ea *)ea0 ) ;
return (struct type *)_au0_this ;
}

#line 1009 "../../src/norm.c"
xx :
#line 1010 "../../src/norm.c"
switch (_au1_t -> _node_base ){ 
#line 1011 "../../src/norm.c"
case 97 : _au1_t = (((struct basetype *)_au1_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 1011 "../../src/norm.c"
goto xx ;
case 125 : 
#line 1013 "../../src/norm.c"
case 158 : return _ptr_normalize ( ((struct ptr *)_au1_t ), (struct type *)_au0_this ) ;
case 110 : return _vec_normalize ( ((struct vec *)_au1_t ), (struct type *)_au0_this ) ;
case 108 : return _fct_normalize ( ((struct fct *)_au1_t ), (struct type *)_au0_this ) ;
default : { 
#line 1018 "../../src/norm.c"
struct ea _au0__V83 ;

#line 1016 "../../src/norm.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"badPT(%k)", (struct ea *)( ( ((& _au0__V83 )-> _ea__O1.__C1_i = ((int )_au1_t -> _node_base )),
#line 1016 "../../src/norm.c"
(((& _au0__V83 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}
;
Ptype _fct_normalize (_au0_this , _au0_ret )
#line 400 "../../src/cfront.h"
struct fct *_au0_this ;

#line 1020 "../../src/norm.c"
Ptype _au0_ret ;

#line 1024 "../../src/norm.c"
{ 
#line 1027 "../../src/norm.c"
register Ptype _au1_t ;

#line 1027 "../../src/norm.c"
_au1_t = _au0_this -> _fct_returns ;
_au0_this -> _fct_returns = _au0_ret ;
if (_au1_t == 0 )return (struct type *)_au0_this ;

#line 1031 "../../src/norm.c"
if (_au0_this -> _fct_argtype && (_au0_this -> _fct_argtype -> _node_base != 85 )){ 
#line 1032 "../../src/norm.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"syntax: ANX", (struct ea *)ea0 ,
#line 1032 "../../src/norm.c"
(struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
_au0_this -> _fct_argtype = 0 ;
_au0_this -> _fct_nargs = 0 ;
_au0_this -> _fct_nargs_known = 0 ;
}

#line 1038 "../../src/norm.c"
xx :
#line 1039 "../../src/norm.c"
switch (_au1_t -> _node_base ){ 
#line 1040 "../../src/norm.c"
case 125 : 
#line 1041 "../../src/norm.c"
case 158 : return _ptr_normalize ( ((struct ptr *)_au1_t ), (struct type *)_au0_this ) ;
#line 1041 "../../src/norm.c"

#line 1042 "../../src/norm.c"
case 110 : return _vec_normalize ( ((struct vec *)_au1_t ), (struct type *)_au0_this ) ;
case 108 : return _fct_normalize ( ((struct fct *)_au1_t ), (struct type *)_au0_this ) ;
case 97 : _au1_t = (((struct basetype *)_au1_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 1044 "../../src/norm.c"
goto xx ;
default : { 
#line 1048 "../../src/norm.c"
struct ea _au0__V84 ;

#line 1045 "../../src/norm.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"badFT:%k", (struct ea *)( ( ((& _au0__V84 )-> _ea__O1.__C1_i = ((int )_au1_t -> _node_base )),
#line 1045 "../../src/norm.c"
(((& _au0__V84 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}
;

#line 1051 "../../src/norm.c"
char _fct_argdcl (_au0_this , _au0_dcl , _au0_fn )
#line 400 "../../src/cfront.h"
struct fct *_au0_this ;

#line 1051 "../../src/norm.c"
Pname _au0_dcl ;

#line 1051 "../../src/norm.c"
Pname _au0_fn ;

#line 1058 "../../src/norm.c"
{ 
#line 1059 "../../src/norm.c"
Pname _au1_n ;

#line 1061 "../../src/norm.c"
switch (_au0_this -> _node_base ){ 
#line 1062 "../../src/norm.c"
case 108 : break ;
case 141 : return ;
default : { 
#line 1174 "../../src/norm.c"
struct ea _au0__V85 ;

#line 1064 "../../src/norm.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"fct::argdcl(%d)", (struct ea *)( ( ((& _au0__V85 )-> _ea__O1.__C1_i = ((int )_au0_this -> _node_base )),
#line 1064 "../../src/norm.c"
(((& _au0__V85 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }

#line 1067 "../../src/norm.c"
if (_au0_this -> _fct_argtype ){ 
#line 1068 "../../src/norm.c"
switch (_au0_this -> _fct_argtype -> _node_base ){ 
#line 1069 "../../src/norm.c"
case 85 : 
#line 1070 "../../src/norm.c"
if (_au0_dcl )error ( (char *)"badF definition syntax",
#line 1070 "../../src/norm.c"
(struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
for(_au1_n = _au0_this -> _fct_argtype ;_au1_n ;_au1_n = _au1_n -> _name_n_list ) { 
#line 1072 "../../src/norm.c"
if (_au1_n -> _expr__O3.__C3_string == 0 )_au1_n -> _expr__O3.__C3_string = make_name ( (unsigned char
#line 1072 "../../src/norm.c"
)'A' ) ;
}
return ;
case 140 : 
#line 1077 "../../src/norm.c"
{ Pexpr _au4_e ;
Pname _au4_nn ;
Pname _au4_tail ;

#line 1079 "../../src/norm.c"
_au4_tail = 0 ;
_au1_n = 0 ;
if (old_fct_accepted == 0 ){ 
#line 1174 "../../src/norm.c"
struct ea _au0__V86 ;

#line 1081 "../../src/norm.c"
errorFI_PCloc__PC_RCea__RCea__RCea__RCea___ ( (int )'w' , & _au0_fn -> _name_where , (char *)"old style definition of%n", (struct ea *)( ( ((& _au0__V86 )-> _ea__O1.__C1_p =
#line 1081 "../../src/norm.c"
((char *)_au0_fn )), (((& _au0__V86 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} for(_au4_e = (((struct expr *)_au0_this -> _fct_argtype ));_au4_e ;_au4_e = _au4_e -> _expr__O4.__C4_e2 ) { 
#line 1083 "../../src/norm.c"
Pexpr _au5_id ;

#line 1083 "../../src/norm.c"
_au5_id = _au4_e -> _expr__O3.__C3_e1 ;
if (_au5_id -> _node_base != 85 ){ 
#line 1085 "../../src/norm.c"
error ( (char *)"NX inAL", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 1085 "../../src/norm.c"
ea *)ea0 ) ;
_au0_this -> _fct_argtype = 0 ;
_au0_dcl = 0 ;
break ;
}
_au4_nn = (struct name *)_name__ctor ( (struct name *)0 , _au5_id -> _expr__O3.__C3_string ) ;
if (_au1_n )
#line 1092 "../../src/norm.c"
_au4_tail = (_au4_tail -> _name_n_list = _au4_nn );
else 
#line 1094 "../../src/norm.c"
_au4_tail = (_au1_n = _au4_nn );
}
_au0_this -> _fct_argtype = _au1_n ;
break ;
}
default : 
#line 1100 "../../src/norm.c"
{ 
#line 1174 "../../src/norm.c"
struct ea _au0__V87 ;

#line 1100 "../../src/norm.c"
error ( (char *)"ALX(%d)", (struct ea *)( ( ((& _au0__V87 )-> _ea__O1.__C1_i = ((int )_au0_this -> _fct_argtype -> _node_base )), (((&
#line 1100 "../../src/norm.c"
_au0__V87 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
_au0_this -> _fct_argtype = 0 ;
_au0_dcl = 0 ;
} }
}
else { 
#line 1106 "../../src/norm.c"
_au0_this -> _fct_nargs_known = (! fct_void );
_au0_this -> _fct_nargs = 0 ;
if (_au0_dcl )error ( (char *)"ADL forFWoutAs", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
return ;
}

#line 1112 "../../src/norm.c"
_au0_this -> _fct_nargs_known = 0 ;

#line 1114 "../../src/norm.c"
if (_au0_dcl ){ 
#line 1115 "../../src/norm.c"
Pname _au2_d ;
Pname _au2_dx ;

#line 1121 "../../src/norm.c"
for(_au1_n = _au0_this -> _fct_argtype ;_au1_n ;_au1_n = _au1_n -> _name_n_list ) { 
#line 1122 "../../src/norm.c"
char *_au3_s ;

#line 1122 "../../src/norm.c"
_au3_s = _au1_n -> _expr__O3.__C3_string ;
if (_au3_s == 0 ){ 
#line 1124 "../../src/norm.c"
error ( (char *)"AN missing inF definition", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 )
#line 1124 "../../src/norm.c"
;
_au1_n -> _expr__O3.__C3_string = (_au3_s = make_name ( (unsigned char )'A' ) );
}
else if (_au1_n -> _expr__O2.__C2_tp ){ 
#line 1174 "../../src/norm.c"
struct ea _au0__V88 ;

#line 1127 "../../src/norm.c"
error ( (char *)"twoTs forA %s", (struct ea *)( ( ((& _au0__V88 )-> _ea__O1.__C1_p = ((char *)_au1_n -> _expr__O3.__C3_string )), (((& _au0__V88 ))))
#line 1127 "../../src/norm.c"
) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} 
#line 1129 "../../src/norm.c"
for(_au2_d = _au0_dcl ;_au2_d ;_au2_d = _au2_d -> _name_n_list ) { 
#line 1130 "../../src/norm.c"
if (strcmp ( (char *)_au3_s , (char *)_au2_d -> _expr__O3.__C3_string ) ==
#line 1130 "../../src/norm.c"
0 ){ 
#line 1131 "../../src/norm.c"
if (_au2_d -> _expr__O2.__C2_tp -> _node_base == 38 ){ 
#line 1132 "../../src/norm.c"
{ 
#line 1174 "../../src/norm.c"
struct ea _au0__V89 ;

#line 1132 "../../src/norm.c"
error ( (char *)"voidA%n", (struct ea *)( ( ((& _au0__V89 )-> _ea__O1.__C1_p = ((char *)_au2_d )), (((& _au0__V89 )))) )
#line 1132 "../../src/norm.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
_au2_d -> _expr__O2.__C2_tp = (struct type *)any_type ;
} }
_au1_n -> _expr__O2.__C2_tp = _au2_d -> _expr__O2.__C2_tp ;
_au1_n -> _name_n_sto = _au2_d -> _name_n_sto ;
_au2_d -> _expr__O2.__C2_tp = 0 ;
goto xx ;
}
}
_au1_n -> _expr__O2.__C2_tp = (struct type *)defa_type ;
xx :;
if (_au1_n -> _expr__O2.__C2_tp == 0 ){ 
#line 1174 "../../src/norm.c"
struct ea _au0__V90 ;

#line 1143 "../../src/norm.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"noT for %s", (struct ea *)( ( ((& _au0__V90 )-> _ea__O1.__C1_p = ((char *)_au1_n -> _expr__O3.__C3_string )),
#line 1143 "../../src/norm.c"
(((& _au0__V90 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }

#line 1149 "../../src/norm.c"
for(_au2_d = _au0_dcl ;_au2_d ;_au2_d = _au2_dx ) { 
#line 1150 "../../src/norm.c"
_au2_dx = _au2_d -> _name_n_list ;
if (_au2_d -> _expr__O2.__C2_tp ){ 
#line 1153 "../../src/norm.c"
switch (_au2_d -> _expr__O2.__C2_tp -> _node_base ){ 
#line 1154 "../../src/norm.c"
case 6 : 
#line 1155 "../../src/norm.c"
case 13 : 
#line 1159 "../../src/norm.c"
_au2_d -> _name_n_list = _au0_this ->
#line 1159 "../../src/norm.c"
_fct_argtype ;
_au0_this -> _fct_argtype = _au2_d ;
break ;
default : 
#line 1163 "../../src/norm.c"
{ 
#line 1174 "../../src/norm.c"
struct ea _au0__V91 ;

#line 1163 "../../src/norm.c"
error ( (char *)"%n inADL not inAL", (struct ea *)( ( ((& _au0__V91 )-> _ea__O1.__C1_p = ((char *)_au2_d )), (((& _au0__V91 )))) )
#line 1163 "../../src/norm.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}
}
}

#line 1170 "../../src/norm.c"
for(_au1_n = _au0_this -> _fct_argtype ;_au1_n ;_au1_n = _au1_n -> _name_n_list ) { 
#line 1171 "../../src/norm.c"
if (_au1_n -> _expr__O2.__C2_tp == 0 )_au1_n -> _expr__O2.__C2_tp = (struct type *)defa_type ;
#line 1171 "../../src/norm.c"

#line 1172 "../../src/norm.c"
_au0_this -> _fct_nargs ++ ;
}
}
;
Pname cl_obj_vec ;
Pname eobj ;

#line 1179 "../../src/norm.c"
Pname _type_is_cl_obj (_au0_this )
#line 263 "../../src/cfront.h"
struct type *_au0_this ;

#line 1188 "../../src/norm.c"
{ 
#line 1189 "../../src/norm.c"
bit _au1_v ;
register Ptype _au1_t ;

#line 1189 "../../src/norm.c"
_au1_v = 0 ;
_au1_t = _au0_this ;

#line 1192 "../../src/norm.c"
eobj = 0 ;
cl_obj_vec = 0 ;
xx :
#line 1195 "../../src/norm.c"
switch (_au1_t -> _node_base ){ 
#line 1196 "../../src/norm.c"
case 97 : 
#line 1197 "../../src/norm.c"
_au1_t = (((struct basetype *)_au1_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
goto xx ;

#line 1200 "../../src/norm.c"
case 119 : 
#line 1201 "../../src/norm.c"
if (_au1_v ){ 
#line 1202 "../../src/norm.c"
cl_obj_vec = (((struct basetype *)_au1_t ))-> _basetype_b_name ;
return (struct name *)0 ;
}
else 
#line 1206 "../../src/norm.c"
return (((struct basetype *)_au1_t ))-> _basetype_b_name ;

#line 1208 "../../src/norm.c"
case 110 : 
#line 1209 "../../src/norm.c"
_au1_t = (((struct vec *)_au1_t ))-> _pvtyp_typ ;
_au1_v = 1 ;
goto xx ;

#line 1213 "../../src/norm.c"
case 121 : 
#line 1214 "../../src/norm.c"
eobj = (((struct basetype *)_au1_t ))-> _basetype_b_name ;
default : 
#line 1216 "../../src/norm.c"
return (struct name *)0 ;
}
}
;

/* the end */
