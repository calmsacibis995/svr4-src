/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#line 1 "../../src/dcl.c"

/* <<cfront 1.2.1 2/16/87>> */
/* < ../../src/dcl.c */

#ident	"@(#)sdb:cfront/scratch/src/dcl..c	1.2"
char *_new(); char _delete(); char *_vec_new(); char _vec_delete();

#line 1 "../../src/dcl.c"

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

#line 28 "../../src/dcl.c"
struct dcl_context ccvec [20];

#line 28 "../../src/dcl.c"
struct dcl_context *cc = ccvec ;
int byte_offset = 0 ;
int bit_offset = 0 ;
int max_align = 0 ;
int stack_size = 0 ;
int enum_count = 0 ;
int friend_in_class = 0 ;

#line 36 "../../src/dcl.c"
Pname dclass ();
Pname denum ();
char dargs ();
char merge_init ();

#line 41 "../../src/dcl.c"
Pname _name_dcl (_au0_this , _au0_tbl , _au0_scope )
#line 607 "../../src/cfront.h"
struct name *_au0_this ;

#line 41 "../../src/dcl.c"
Ptable _au0_tbl ;

#line 41 "../../src/dcl.c"
TOK _au0_scope ;

#line 78 "../../src/dcl.c"
{ 
#line 79 "../../src/dcl.c"
Pname _au1_nn ;
Ptype _au1_nnt ;
Pname _au1_odcl ;

#line 80 "../../src/dcl.c"
_au1_nnt = 0 ;
_au1_odcl = Cdcl ;

#line 88 "../../src/dcl.c"
Cdcl = _au0_this ;
switch (_au0_this -> _node_base ){ 
#line 90 "../../src/dcl.c"
case 123 : 
#line 91 "../../src/dcl.c"
_type_dcl ( _au0_this -> _expr__O2.__C2_tp , _au0_tbl ) ;
_au0_this -> _expr__O2.__C2_tp -> _node_permanent = 1 ;
_au1_nn = (struct name *)_name__ctor ( (struct name *)0 , _au0_this -> _expr__O3.__C3_string ) ;
_au1_nn -> _node_base = 123 ;
_au1_nn -> _expr__O2.__C2_tp = _au0_this -> _expr__O2.__C2_tp ;
(_table_insert ( _au0_tbl , _au1_nn , (unsigned char )0 ) );
_name__dtor ( _au1_nn , 1) ;
Cdcl = _au1_odcl ;
return _au0_this ;
case 85 : 
#line 101 "../../src/dcl.c"
switch (_au0_this -> _name_n_oper ){ 
#line 102 "../../src/dcl.c"
case 123 : 
#line 103 "../../src/dcl.c"
if (_au0_this -> _expr__O2.__C2_tp -> _node_base != 108 )_au0_this -> _name_n_oper = 0 ;
#line 103 "../../src/dcl.c"

#line 104 "../../src/dcl.c"
break ;
case 47 : 
#line 106 "../../src/dcl.c"
if (_au0_this -> _expr__O2.__C2_tp -> _node_base != 108 ){ 
#line 107 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V10 ;

#line 107 "../../src/dcl.c"
error ( (char *)"~%s notF", (struct ea *)( ( ((& _au0__V10 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _expr__O3.__C3_string )), (((& _au0__V10 ))))
#line 107 "../../src/dcl.c"
) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
_au0_this -> _name_n_oper = 0 ;
} }
break ;
}
break ;
default : 
#line 114 "../../src/dcl.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"NX in N::dcl()", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 114 "../../src/dcl.c"
ea *)ea0 ) ;
}

#line 117 "../../src/dcl.c"
if (_au0_this -> _name__O6.__C6_n_qualifier ){ 
#line 118 "../../src/dcl.c"
if (_au0_this -> _expr__O2.__C2_tp -> _node_base != 108 ){ 
#line 119 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V11 ;

#line 119 "../../src/dcl.c"
error ( (char *)"QdN%n inD of nonF", (struct ea *)( ( ((& _au0__V11 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V11 )))) )
#line 119 "../../src/dcl.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
Cdcl = _au1_odcl ;
return (struct name *)0 ;
} }

#line 124 "../../src/dcl.c"
{ Pname _au2_cn ;

#line 140 "../../src/dcl.c"
Pclass _au2_cl ;

#line 124 "../../src/dcl.c"
_au2_cn = _au0_this -> _name__O6.__C6_n_qualifier ;
switch (_au2_cn -> _node_base ){ 
#line 126 "../../src/dcl.c"
case 123 : 
#line 127 "../../src/dcl.c"
break ;
case 85 : 
#line 129 "../../src/dcl.c"
_au2_cn = _table_look ( gtbl , _au2_cn -> _expr__O3.__C3_string , (unsigned char )0 ) ;
if (_au2_cn && (_au2_cn -> _node_base == 123 ))break ;
default : 
#line 132 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V12 ;

#line 1011 "../../src/dcl.c"
struct ea _au0__V13 ;

#line 132 "../../src/dcl.c"
error ( (char *)"badQr%n for%n", (struct ea *)( ( ((& _au0__V12 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _name__O6.__C6_n_qualifier )), (((& _au0__V12 ))))
#line 132 "../../src/dcl.c"
) , (struct ea *)( ( ((& _au0__V13 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V13 )))) ) ,
#line 132 "../../src/dcl.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
Cdcl = _au1_odcl ;
return (struct name *)0 ;
} }

#line 137 "../../src/dcl.c"
_au2_cn = (((struct basetype *)_au2_cn -> _expr__O2.__C2_tp ))-> _basetype_b_name ;
if (_au0_this -> _name_n_oper )_name_check_oper ( _au0_this , _au2_cn ) ;

#line 140 "../../src/dcl.c"
_au2_cl = (((struct classdef *)_au2_cn -> _expr__O2.__C2_tp ));
if (_au2_cl == cc -> _dcl_context_cot ){ 
#line 142 "../../src/dcl.c"
_au0_this -> _name__O6.__C6_n_qualifier = 0 ;
goto xdr ;
}
else if ((_au2_cl -> _type_defined & 3)== 0 ){ 
#line 146 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V14 ;

#line 146 "../../src/dcl.c"
error ( (char *)"C%nU", (struct ea *)( ( ((& _au0__V14 )-> _ea__O1.__C1_p = ((char *)_au2_cn )), (((& _au0__V14 )))) )
#line 146 "../../src/dcl.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
Cdcl = _au1_odcl ;
return (struct name *)0 ;
} }

#line 151 "../../src/dcl.c"
{ Ptable _au2_etbl ;
Pname _au2_x ;

#line 151 "../../src/dcl.c"
_au2_etbl = _au2_cl -> _classdef_memtbl ;
_au2_x = _table_look ( _au2_etbl , _au0_this -> _expr__O3.__C3_string , (unsigned char )0 ) ;
if ((_au2_x == 0 )|| (_au2_x -> _expr__O5.__C5_n_table != _au2_etbl )){ 
#line 154 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V15 ;

#line 1011 "../../src/dcl.c"
struct ea _au0__V16 ;

#line 154 "../../src/dcl.c"
error ( (char *)"%n is not aM of%n", (struct ea *)( ( ((& _au0__V15 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V15 )))) )
#line 154 "../../src/dcl.c"
, (struct ea *)( ( ((& _au0__V16 )-> _ea__O1.__C1_p = ((char *)_au2_cn )), (((& _au0__V16 )))) ) , (struct
#line 154 "../../src/dcl.c"
ea *)ea0 , (struct ea *)ea0 ) ;
Cdcl = _au1_odcl ;
return (struct name *)0 ;
} }
}
}
}

#line 159 "../../src/dcl.c"
xdr :
#line 160 "../../src/dcl.c"
if ((_au0_this -> _name_n_oper && (_au0_this -> _expr__O2.__C2_tp -> _node_base != 108 ))&& (_au0_this -> _name_n_sto != 76 ))
#line 161 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V17 ;

#line 161 "../../src/dcl.c"
error ( (char *)"operator%k not aF", (struct ea *)( ( ((& _au0__V17 )-> _ea__O1.__C1_i = ((int )_au0_this -> _name_n_oper )), (((& _au0__V17 ))))
#line 161 "../../src/dcl.c"
) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} 
#line 171 "../../src/dcl.c"
_au0_this -> _name_n_stclass = _au0_this -> _name_n_sto ;
_au0_this -> _name_n_scope = _au0_scope ;

#line 174 "../../src/dcl.c"
switch (_au0_this -> _name_n_sto ){ 
#line 175 "../../src/dcl.c"
default : 
#line 176 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V18 ;

#line 176 "../../src/dcl.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"unexpected %k", (struct ea *)( ( ((& _au0__V18 )-> _ea__O1.__C1_i = ((int )_au0_this -> _name_n_sto )),
#line 176 "../../src/dcl.c"
(((& _au0__V18 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
case 18 : 
#line 178 "../../src/dcl.c"
{ 
#line 179 "../../src/dcl.c"
Pclass _au3_cl ;

#line 180 "../../src/dcl.c"
struct name_list *_au0__Xthis__ctor_name_list ;

#line 179 "../../src/dcl.c"
_au3_cl = cc -> _dcl_context_cot ;

#line 181 "../../src/dcl.c"
switch (_au0_scope ){ 
#line 182 "../../src/dcl.c"
case 0 : 
#line 183 "../../src/dcl.c"
case 25 : 
#line 184 "../../src/dcl.c"
break ;
default : 
#line 186 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V19 ;

#line 1011 "../../src/dcl.c"
struct ea _au0__V20 ;

#line 186 "../../src/dcl.c"
error ( (char *)"friend%n not inCD(%k)", (struct ea *)( ( ((& _au0__V19 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V19 )))) )
#line 186 "../../src/dcl.c"
, (struct ea *)( ( ((& _au0__V20 )-> _ea__O1.__C1_i = ((int )_au0_scope )), (((& _au0__V20 )))) ) , (struct
#line 186 "../../src/dcl.c"
ea *)ea0 , (struct ea *)ea0 ) ;
_au0_this -> _node_base = 0 ;
Cdcl = _au1_odcl ;
return (struct name *)0 ;
} }

#line 192 "../../src/dcl.c"
switch (_au0_this -> _name_n_oper ){ 
#line 193 "../../src/dcl.c"
case 0 : 
#line 194 "../../src/dcl.c"
case 23 : 
#line 195 "../../src/dcl.c"
case 9 : 
#line 196 "../../src/dcl.c"
case 161 : 
#line 197 "../../src/dcl.c"
case 162 :
#line 197 "../../src/dcl.c"

#line 198 "../../src/dcl.c"
case 97 : 
#line 199 "../../src/dcl.c"
_au0_this -> _name_n_sto = 0 ;
break ;
default : 
#line 202 "../../src/dcl.c"
_au0_this -> _name_n_sto = 76 ;
}

#line 205 "../../src/dcl.c"
switch (_au0_this -> _expr__O2.__C2_tp -> _node_base ){ 
#line 211 "../../src/dcl.c"
case 119 : 
#line 212 "../../src/dcl.c"
_au1_nn = (((struct basetype *)_au0_this -> _expr__O2.__C2_tp ))-> _basetype_b_name ;
break ;
case 6 : 
#line 215 "../../src/dcl.c"
_au1_nn = _au0_this ;
break ;
case 108 : 
#line 218 "../../src/dcl.c"
( (cc ++ ), ((*cc )= (*(cc - 1 )))) ;
cc -> _dcl_context_not = 0 ;
cc -> _dcl_context_tot = 0 ;
cc -> _dcl_context_cot = 0 ;
friend_in_class ++ ;
_au0_this -> _name_n_sto = 14 ;
_au1_nn = _name_dcl ( _au0_this , gtbl , (unsigned char )14 ) ;
friend_in_class -- ;
( (cc -- )) ;
if (_au1_nn -> _expr__O2.__C2_tp -> _node_base == 76 ){ 
#line 228 "../../src/dcl.c"
Pgen _au5_g ;

#line 228 "../../src/dcl.c"
_au5_g = (((struct gen *)_au1_nn -> _expr__O2.__C2_tp ));
_au1_nn = _gen_find ( _au5_g , ((struct fct *)_au0_this -> _expr__O2.__C2_tp ), (char )1 ) ;
}
break ;
default : 
#line 233 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V21 ;

#line 1011 "../../src/dcl.c"
struct ea _au0__V22 ;

#line 233 "../../src/dcl.c"
error ( (char *)"badT%t of friend%n", (struct ea *)( ( ((& _au0__V21 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _expr__O2.__C2_tp )), (((& _au0__V21 ))))
#line 233 "../../src/dcl.c"
) , (struct ea *)( ( ((& _au0__V22 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V22 )))) ) ,
#line 233 "../../src/dcl.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
_au1_nn -> _node_permanent = 1 ;
_au3_cl -> _classdef_friend_list = (struct name_list *)( (_au0__Xthis__ctor_name_list = 0 ), ( (_au0__Xthis__ctor_name_list = (struct name_list *)_new ( (long )(sizeof (struct name_list))) ),
#line 236 "../../src/dcl.c"
( (_au0__Xthis__ctor_name_list -> _name_list_f = _au1_nn ), ( (_au0__Xthis__ctor_name_list -> _name_list_l = _au3_cl -> _classdef_friend_list ), ((_au0__Xthis__ctor_name_list ))) ) ) ) ;
#line 236 "../../src/dcl.c"

#line 237 "../../src/dcl.c"
Cdcl = _au1_odcl ;
return _au1_nn ;
}
case 76 : 
#line 241 "../../src/dcl.c"
_au0_this -> _name_n_sto = 0 ;
switch (_au0_scope ){ 
#line 243 "../../src/dcl.c"
case 0 : 
#line 244 "../../src/dcl.c"
case 25 : 
#line 245 "../../src/dcl.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"overload inCD (ignored)", (struct ea *)ea0 , (struct
#line 245 "../../src/dcl.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
switch (_au0_this -> _expr__O2.__C2_tp -> _node_base ){ 
#line 247 "../../src/dcl.c"
case 21 : 
#line 248 "../../src/dcl.c"
_au0_this -> _node_base = 0 ;
Cdcl = _au1_odcl ;
return _au0_this ;
case 108 : 
#line 252 "../../src/dcl.c"
return _name_dcl ( _au0_this , _au0_tbl , _au0_scope ) ;
}
}
if (_au0_this -> _name_n_oper && (_au0_this -> _expr__O2.__C2_tp -> _node_base == 108 ))break ;
_au1_nn = _table_insert ( _au0_tbl , _au0_this , (unsigned char )0 ) ;

#line 258 "../../src/dcl.c"
if (Nold ){ 
#line 259 "../../src/dcl.c"
if (_au1_nn -> _expr__O2.__C2_tp -> _node_base != 76 ){ 
#line 260 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V23 ;

#line 260 "../../src/dcl.c"
error ( (char *)"%n redefined as overloaded", (struct ea *)( ( ((& _au0__V23 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V23 )))) )
#line 260 "../../src/dcl.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
_au1_nn -> _expr__O2.__C2_tp = (struct type *)_gen__ctor ( (struct gen *)0 , _au0_this -> _expr__O3.__C3_string ) ;
} }
}
else { 
#line 265 "../../src/dcl.c"
_au1_nn -> _expr__O2.__C2_tp = (struct type *)_gen__ctor ( (struct gen *)0 , _au0_this -> _expr__O3.__C3_string ) ;
}

#line 268 "../../src/dcl.c"
switch (_au0_this -> _expr__O2.__C2_tp -> _node_base ){ 
#line 269 "../../src/dcl.c"
case 21 : 
#line 270 "../../src/dcl.c"
_au0_this -> _node_base = 0 ;
Cdcl = _au1_odcl ;
return _au1_nn ;
case 108 : 
#line 274 "../../src/dcl.c"
break ;
default : 
#line 276 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V24 ;

#line 1011 "../../src/dcl.c"
struct ea _au0__V25 ;

#line 276 "../../src/dcl.c"
error ( (char *)"N%n ofT%k cannot be overloaded", (struct ea *)( ( ((& _au0__V24 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V24 )))) )
#line 276 "../../src/dcl.c"
, (struct ea *)( ( ((& _au0__V25 )-> _ea__O1.__C1_i = ((int )_au0_this -> _expr__O2.__C2_tp -> _node_base )), (((& _au0__V25 )))) )
#line 276 "../../src/dcl.c"
, (struct ea *)ea0 , (struct ea *)ea0 ) ;
Cdcl = _au1_odcl ;
return _au1_nn ;
} }
break ;
case 27 : 
#line 282 "../../src/dcl.c"
if (_au0_this -> _expr__O2.__C2_tp -> _node_base == 108 ){ 
#line 283 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V26 ;

#line 283 "../../src/dcl.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"%n: register (ignored)", (struct ea *)( ( ((& _au0__V26 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((&
#line 283 "../../src/dcl.c"
_au0__V26 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
goto ddd ;
} }
case 2 : 
#line 287 "../../src/dcl.c"
switch (_au0_scope ){ 
#line 288 "../../src/dcl.c"
case 0 : 
#line 289 "../../src/dcl.c"
case 25 : 
#line 290 "../../src/dcl.c"
case 14 : 
#line 291 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V27 ;
#line 1011 "../../src/dcl.c"

#line 291 "../../src/dcl.c"
error ( (char *)"%k not inF", (struct ea *)( ( ((& _au0__V27 )-> _ea__O1.__C1_i = ((int )_au0_this -> _name_n_sto )), (((& _au0__V27 ))))
#line 291 "../../src/dcl.c"
) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
goto ddd ;
} }
if (_au0_this -> _name_n_sto != 27 )_au0_this -> _name_n_sto = 0 ;
break ;
case 14 : 
#line 297 "../../src/dcl.c"
switch (_au0_scope ){ 
#line 298 "../../src/dcl.c"
case 136 : 
#line 299 "../../src/dcl.c"
error ( (char *)"externA", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 299 "../../src/dcl.c"
ea *)ea0 , (struct ea *)ea0 ) ;
goto ddd ;
case 0 : 
#line 302 "../../src/dcl.c"
case 25 : 
#line 304 "../../src/dcl.c"
if (_au0_this -> _expr__O2.__C2_tp -> _node_base != 108 ){ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V28 ;

#line 304 "../../src/dcl.c"
error ( (char *)"externM%n", (struct ea *)( ( ((& _au0__V28 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V28 )))) )
#line 304 "../../src/dcl.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} goto ddd ;
}
_au0_this -> _name_n_stclass = 31 ;
_au0_this -> _name_n_scope = 14 ;
break ;
case 31 : 
#line 311 "../../src/dcl.c"
switch (_au0_scope ){ 
#line 312 "../../src/dcl.c"
case 136 : 
#line 313 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V29 ;

#line 313 "../../src/dcl.c"
error ( (char *)"static used forA%n", (struct ea *)( ( ((& _au0__V29 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V29 )))) )
#line 313 "../../src/dcl.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
goto ddd ;
case 0 : 
#line 316 "../../src/dcl.c"
case 25 : 
#line 317 "../../src/dcl.c"
_au0_this -> _name_n_stclass = 31 ;
_au0_this -> _name_n_scope = _au0_scope ;
break ;
default : 
#line 321 "../../src/dcl.c"
_au0_this -> _name_n_scope = 31 ;
} }
break ;
case 0 : 
#line 325 "../../src/dcl.c"
ddd :
#line 326 "../../src/dcl.c"
switch (_au0_scope ){ 
#line 327 "../../src/dcl.c"
case 14 : 
#line 328 "../../src/dcl.c"
_au0_this -> _name_n_scope = 14 ;
_au0_this -> _name_n_stclass = 31 ;
break ;
case 108 : 
#line 332 "../../src/dcl.c"
if (_au0_this -> _expr__O2.__C2_tp -> _node_base == 108 ){ 
#line 333 "../../src/dcl.c"
_au0_this -> _name_n_stclass = 31 ;
_au0_this -> _name_n_scope = 14 ;
}
else 
#line 337 "../../src/dcl.c"
_au0_this -> _name_n_stclass = 2 ;
break ;
case 136 : 
#line 340 "../../src/dcl.c"
_au0_this -> _name_n_stclass = 2 ;
break ;
case 0 : 
#line 343 "../../src/dcl.c"
case 25 : 
#line 344 "../../src/dcl.c"
_au0_this -> _name_n_stclass = 0 ;
break ;
}
} }

#line 357 "../../src/dcl.c"
switch (_au0_this -> _expr__O2.__C2_tp -> _node_base ){ 
#line 358 "../../src/dcl.c"
case 1 : 
#line 359 "../../src/dcl.c"
{ Pbase _au3_b ;
Pname _au3_n ;

#line 359 "../../src/dcl.c"
_au3_b = (((struct basetype *)_au0_this -> _expr__O2.__C2_tp ));
_au3_n = _table_insert ( _au0_tbl , _au0_this , (unsigned char )0 ) ;
_name_assign ( _au3_n ) ;
( (_au3_n -> _name_n_used ++ )) ;
{ char *_au3_s ;
int _au3_ll ;
char *_au3_s2 ;

#line 363 "../../src/dcl.c"
_au3_s = (((char *)_au3_b -> _basetype_b_name ));
_au3_ll = strlen ( (char *)_au3_s ) ;
_au3_s2 = (((char *)_new ( (long )((sizeof (char ))* (_au3_ll + 1 ))) ));
strcpy ( _au3_s2 , (char *)_au3_s ) ;
_au3_b -> _basetype_b_name = (((struct name *)_au3_s2 ));
return _au0_this ;
}
}
case 6 : 
#line 372 "../../src/dcl.c"
_au1_nn = dclass ( _au0_this , _au0_tbl ) ;
Cdcl = _au1_odcl ;
return _au1_nn ;

#line 376 "../../src/dcl.c"
case 13 : 
#line 377 "../../src/dcl.c"
_au1_nn = denum ( _au0_this , _au0_tbl ) ;
Cdcl = _au1_odcl ;
return _au1_nn ;

#line 381 "../../src/dcl.c"
case 108 : 
#line 382 "../../src/dcl.c"
_au1_nn = _name_dofct ( _au0_this , _au0_tbl , _au0_scope ) ;
if (_au1_nn == 0 ){ 
#line 384 "../../src/dcl.c"
Cdcl = _au1_odcl ;
return (struct name *)0 ;
}
break ;

#line 389 "../../src/dcl.c"
case 114 : 
#line 390 "../../src/dcl.c"
switch (_au0_this -> _name_n_stclass ){ 
#line 391 "../../src/dcl.c"
case 0 : 
#line 392 "../../src/dcl.c"
case 25 : 
#line 393 "../../src/dcl.c"
break ;
default : 
#line 395 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V30 ;

#line 395 "../../src/dcl.c"
error ( (char *)"%k field", (struct ea *)( ( ((& _au0__V30 )-> _ea__O1.__C1_i = ((int )_au0_this -> _name_n_stclass )), (((& _au0__V30 ))))
#line 395 "../../src/dcl.c"
) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
_au0_this -> _name_n_stclass = 0 ;
} }

#line 399 "../../src/dcl.c"
if ((cc -> _dcl_context_not == 0 )|| (cc -> _dcl_context_cot -> _classdef_csu == 36 )){ 
#line 400 "../../src/dcl.c"
error ( (char *)(cc -> _dcl_context_not ? "field in union":
#line 400 "../../src/dcl.c"
"field not inC"), (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
_au0_this -> _expr__O2.__C2_tp -> _node_permanent = 1 ;
Cdcl = _au1_odcl ;
return _au0_this ;
}

#line 406 "../../src/dcl.c"
if (_au0_this -> _expr__O3.__C3_string ){ 
#line 407 "../../src/dcl.c"
_au1_nn = _table_insert ( _au0_tbl , _au0_this , (unsigned char )0 ) ;
_au0_this -> _expr__O5.__C5_n_table = _au1_nn -> _expr__O5.__C5_n_table ;
if (Nold ){ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V31 ;

#line 409 "../../src/dcl.c"
error ( (char *)"twoDs of field%n", (struct ea *)( ( ((& _au0__V31 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V31 )))) )
#line 409 "../../src/dcl.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }

#line 412 "../../src/dcl.c"
_type_dcl ( _au0_this -> _expr__O2.__C2_tp , _au0_tbl ) ;
_name_field_align ( _au0_this ) ;
break ;

#line 416 "../../src/dcl.c"
case 119 : 
#line 417 "../../src/dcl.c"
{ Pclass _au3_cl ;

#line 417 "../../src/dcl.c"
_au3_cl = (((struct classdef *)(((struct basetype *)_au0_this -> _expr__O2.__C2_tp ))-> _basetype_b_name -> _expr__O2.__C2_tp ));

#line 419 "../../src/dcl.c"
if (_au3_cl -> _classdef_csu == 167 ){ 
#line 422 "../../src/dcl.c"
char *_au4_p ;

#line 422 "../../src/dcl.c"
_au4_p = _au3_cl -> _classdef_string ;
while ((*(_au4_p ++ ))!= 'C' );
{ int _au4_uindex ;

#line 424 "../../src/dcl.c"
_au4_uindex = str_to_int ( (char *)_au4_p ) ;

#line 427 "../../src/dcl.c"
(((struct basetype *)_au0_this -> _expr__O2.__C2_tp ))-> _basetype_b_name -> _name_n_used = 1 ;
(((struct basetype *)_au0_this -> _expr__O2.__C2_tp ))-> _basetype_b_name -> _name_n_assigned_to = 1 ;

#line 430 "../../src/dcl.c"
{ Ptable _au4_mtbl ;
int _au4_i ;

#line 430 "../../src/dcl.c"
_au4_mtbl = _au3_cl -> _classdef_memtbl ;

#line 432 "../../src/dcl.c"
{ Pname _au4_nn ;

#line 432 "../../src/dcl.c"
_au4_nn = _table_get_mem ( _au4_mtbl , _au4_i = 1 ) ;

#line 432 "../../src/dcl.c"
for(;_au4_nn ;_au4_nn = _table_get_mem ( _au4_mtbl , ++ _au4_i ) ) { 
#line 433 "../../src/dcl.c"
if (_au4_nn -> _expr__O2.__C2_tp -> _node_base == 108 ){ 
#line 434 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct
#line 1011 "../../src/dcl.c"
ea _au0__V32 ;

#line 434 "../../src/dcl.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"M%n for anonymous union", (struct ea *)( ( ((& _au0__V32 )-> _ea__O1.__C1_p = ((char *)_au4_nn )), (((&
#line 434 "../../src/dcl.c"
_au0__V32 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
break ;
} }
{ Ptable _au5_tb ;

#line 437 "../../src/dcl.c"
_au5_tb = _au4_nn -> _expr__O5.__C5_n_table ;
_au4_nn -> _expr__O5.__C5_n_table = 0 ;
{ Pname _au5_n ;

#line 439 "../../src/dcl.c"
_au5_n = _table_insert ( _au0_tbl , _au4_nn , (unsigned char )0 ) ;
if (Nold ){ 
#line 441 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V33 ;

#line 441 "../../src/dcl.c"
error ( (char *)"twoDs of%n (one in anonymous union)", (struct ea *)( ( ((& _au0__V33 )-> _ea__O1.__C1_p = ((char *)_au4_nn )), (((& _au0__V33 )))) )
#line 441 "../../src/dcl.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
break ;
} }
_au5_n -> _name_n_union = _au4_uindex ;
_au4_nn -> _expr__O5.__C5_n_table = _au5_tb ;
}
}
}
}
}
}
}

#line 448 "../../src/dcl.c"
goto cde ;
}

#line 451 "../../src/dcl.c"
case 110 : 
#line 452 "../../src/dcl.c"
case 125 : 
#line 453 "../../src/dcl.c"
case 158 : 
#line 454 "../../src/dcl.c"
_type_dcl ( _au0_this -> _expr__O2.__C2_tp , _au0_tbl ) ;

#line 456 "../../src/dcl.c"
default : 
#line 457 "../../src/dcl.c"
cde :
#line 458 "../../src/dcl.c"
_au1_nn = _table_insert ( _au0_tbl , _au0_this , (unsigned char )0 ) ;

#line 461 "../../src/dcl.c"
_au0_this -> _expr__O5.__C5_n_table = _au1_nn -> _expr__O5.__C5_n_table ;
if (Nold ){ 
#line 463 "../../src/dcl.c"
if (_au1_nn -> _expr__O2.__C2_tp -> _node_base == 141 )goto zzz ;

#line 465 "../../src/dcl.c"
if (_type_check ( _au0_this -> _expr__O2.__C2_tp , _au1_nn -> _expr__O2.__C2_tp , (unsigned char )0 ) ){ 
#line 466 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V34 ;

#line 1011 "../../src/dcl.c"
struct ea _au0__V35 ;

#line 1011 "../../src/dcl.c"
struct ea _au0__V36 ;

#line 466 "../../src/dcl.c"
error ( (char *)"twoDs of%n;Ts:%t and%t", (struct ea *)( ( ((& _au0__V34 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V34 )))) )
#line 466 "../../src/dcl.c"
, (struct ea *)( ( ((& _au0__V35 )-> _ea__O1.__C1_p = ((char *)_au1_nn -> _expr__O2.__C2_tp )), (((& _au0__V35 )))) ) ,
#line 466 "../../src/dcl.c"
(struct ea *)( ( ((& _au0__V36 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _expr__O2.__C2_tp )), (((& _au0__V36 )))) ) , (struct
#line 466 "../../src/dcl.c"
ea *)ea0 ) ;
Cdcl = _au1_odcl ;
return (struct name *)0 ;
} }

#line 471 "../../src/dcl.c"
if (_au0_this -> _name_n_sto && (_au0_this -> _name_n_sto != _au1_nn -> _name_n_scope ))
#line 474 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V37 ;

#line 1011 "../../src/dcl.c"
struct ea _au0__V38 ;

#line 1011 "../../src/dcl.c"
struct ea _au0__V39 ;

#line 474 "../../src/dcl.c"
error ( (char *)"%n declared as both%k and%k", (struct ea *)( ( ((& _au0__V37 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V37 )))) )
#line 474 "../../src/dcl.c"
, (struct ea *)( ( ((& _au0__V38 )-> _ea__O1.__C1_i = ((int )_au0_this -> _name_n_sto )), (((& _au0__V38 )))) ) ,
#line 474 "../../src/dcl.c"
(struct ea *)( ( ((& _au0__V39 )-> _ea__O1.__C1_i = ((int )(_au1_nn -> _name_n_sto ? (((unsigned int )_au1_nn -> _name_n_sto )): (((unsigned
#line 474 "../../src/dcl.c"
int )((_au0_scope == 108 )? 2 : 14 )))))), (((& _au0__V39 )))) ) , (struct ea *)ea0 ) ;
} else if ((_au1_nn -> _name_n_scope == 31 )&& (_au0_this -> _name_n_scope == 14 ))
#line 476 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V40 ;

#line 476 "../../src/dcl.c"
error ( (char *)"%n both static and extern", (struct ea *)( ( ((& _au0__V40 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V40 )))) )
#line 476 "../../src/dcl.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} else if ((_au1_nn -> _name_n_sto == 31 )&& (_au0_this -> _name_n_sto == 31 ))
#line 478 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V41 ;

#line 478 "../../src/dcl.c"
error ( (char *)"static%n declared twice", (struct ea *)( ( ((& _au0__V41 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V41 )))) )
#line 478 "../../src/dcl.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} else { 
#line 480 "../../src/dcl.c"
if ((((_au0_this -> _name_n_sto == 0 )&& (_au1_nn -> _name_n_sto == 14 ))&& _au0_this -> _expr__O4.__C4_n_initializer )&& _type_tconst ( _au0_this ->
#line 480 "../../src/dcl.c"
_expr__O2.__C2_tp ) )
#line 484 "../../src/dcl.c"
_au0_this -> _name_n_sto = 14 ;
_au0_this -> _name_n_scope = _au1_nn -> _name_n_scope ;

#line 487 "../../src/dcl.c"
switch (_au0_scope ){ 
#line 488 "../../src/dcl.c"
case 108 : 
#line 489 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V42 ;

#line 489 "../../src/dcl.c"
error ( (char *)"twoDs of%n", (struct ea *)( ( ((& _au0__V42 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V42 )))) )
#line 489 "../../src/dcl.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
Cdcl = _au1_odcl ;
return (struct name *)0 ;
case 136 : 
#line 493 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V43 ;

#line 493 "../../src/dcl.c"
error ( (char *)"twoAs%n", (struct ea *)( ( ((& _au0__V43 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V43 )))) )
#line 493 "../../src/dcl.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
Cdcl = _au1_odcl ;
return (struct name *)0 ;
case 0 : 
#line 497 "../../src/dcl.c"
case 25 : 
#line 498 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V44 ;

#line 498 "../../src/dcl.c"
error ( (char *)"twoDs ofM%n", (struct ea *)( ( ((& _au0__V44 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V44 )))) )
#line 498 "../../src/dcl.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
Cdcl = _au1_odcl ;
return (struct name *)0 ;
case 14 : 
#line 502 "../../src/dcl.c"
if (((fct_void == 0 )&& (_au0_this -> _name_n_sto == 0 ))&& (_au1_nn -> _name_n_sto == 0 ))
#line 504 "../../src/dcl.c"
{ 
#line 505 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V45 ;
#line 1011 "../../src/dcl.c"

#line 505 "../../src/dcl.c"
error ( (char *)"two definitions of%n", (struct ea *)( ( ((& _au0__V45 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V45 )))) )
#line 505 "../../src/dcl.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
Cdcl = _au1_odcl ;
return (struct name *)0 ;
} }
} } } }
}
_au0_this -> _name_n_scope = _au1_nn -> _name_n_scope ;

#line 513 "../../src/dcl.c"
if (_au0_this -> _expr__O4.__C4_n_initializer ){ 
#line 514 "../../src/dcl.c"
if (_au1_nn -> _expr__O4.__C4_n_initializer || _au1_nn -> _name_n_val ){ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V46 ;

#line 514 "../../src/dcl.c"
error ( (char *)"twoIrs for%n", (struct ea *)( ( ((& _au0__V46 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V46 )))) )
#line 514 "../../src/dcl.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} _au1_nn -> _expr__O4.__C4_n_initializer = _au0_this -> _expr__O4.__C4_n_initializer ;
}
if (_au0_this -> _expr__O2.__C2_tp -> _node_base == 110 ){ 
#line 520 "../../src/dcl.c"
Ptype _au4_ntp ;

#line 520 "../../src/dcl.c"
_au4_ntp = _au1_nn -> _expr__O2.__C2_tp ;
while (_au4_ntp -> _node_base == 97 )_au4_ntp = (((struct basetype *)_au4_ntp ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 523 "../../src/dcl.c"
if ((((struct vec *)_au4_ntp ))-> _vec_dim == 0 )(((struct vec *)_au4_ntp ))-> _vec_dim = (((struct vec *)_au0_this -> _expr__O2.__C2_tp ))-> _vec_dim ;
if ((((struct vec *)_au4_ntp ))-> _vec_size == 0 )(((struct vec *)_au4_ntp ))-> _vec_size = (((struct vec *)_au0_this -> _expr__O2.__C2_tp ))-> _vec_size ;
}
}
else { 
#line 534 "../../src/dcl.c"
if (((((_au0_scope != 136 )&& (_au0_this -> _name_n_sto != 14 ))&& (_au0_this -> _expr__O4.__C4_n_initializer == 0 ))&& (_au0_this -> _expr__O2.__C2_tp -> _node_base ==
#line 534 "../../src/dcl.c"
110 ))&& ((((struct vec *)_au0_this -> _expr__O2.__C2_tp ))-> _vec_size == 0 ))
#line 535 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V47 ;

#line 535 "../../src/dcl.c"
errorFPCloc__PC_RCea__RCea__RCea__RCea___ ( & _au0_this -> _name_where , (char *)"dimension missing for vector%n", (struct ea *)( ( ((& _au0__V47 )-> _ea__O1.__C1_p = ((char *)_au0_this )),
#line 535 "../../src/dcl.c"
(((& _au0__V47 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }

#line 538 "../../src/dcl.c"
zzz :
#line 539 "../../src/dcl.c"
if (_au0_this -> _node_base != 123 ){ 
#line 540 "../../src/dcl.c"
Ptype _au3_t ;

#line 540 "../../src/dcl.c"
_au3_t = _au1_nn -> _expr__O2.__C2_tp ;

#line 542 "../../src/dcl.c"
if (_au3_t -> _node_base == 97 ){ 
#line 543 "../../src/dcl.c"
Ptype _au4_tt ;

#line 543 "../../src/dcl.c"
_au4_tt = (((struct basetype *)_au3_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
if (_au4_tt -> _node_base == 108 )_au1_nn -> _expr__O2.__C2_tp = (_au3_t = _au4_tt );
}

#line 547 "../../src/dcl.c"
switch (_au3_t -> _node_base ){ 
#line 548 "../../src/dcl.c"
case 108 : 
#line 549 "../../src/dcl.c"
case 76 : 
#line 550 "../../src/dcl.c"
break ;
default : 
#line 552 "../../src/dcl.c"
fake_sizeof = 1 ;
switch (_au1_nn -> _name_n_stclass ){ 
#line 554 "../../src/dcl.c"
default : 
#line 555 "../../src/dcl.c"
if (_au1_nn -> _name_n_scope != 136 ){ 
#line 556 "../../src/dcl.c"
int _au6_x ;
int _au6_y ;

#line 556 "../../src/dcl.c"
_au6_x = _type_align ( _au3_t ) ;
_au6_y = _type_tsizeof ( _au3_t ) ;

#line 559 "../../src/dcl.c"
if (max_align < _au6_x )max_align = _au6_x ;

#line 561 "../../src/dcl.c"
while (0 < bit_offset ){ 
#line 562 "../../src/dcl.c"
byte_offset ++ ;
bit_offset -= BI_IN_BYTE ;
}
bit_offset = 0 ;

#line 567 "../../src/dcl.c"
if (byte_offset && (1 < _au6_x ))byte_offset = ((((byte_offset - 1 )/ _au6_x )* _au6_x )+ _au6_x );
_au1_nn -> _name_n_offset = byte_offset ;
byte_offset += _au6_y ;
}
break ;
case 31 : 
#line 573 "../../src/dcl.c"
_type_tsizeof ( _au3_t ) ;
}
fake_sizeof = 0 ;
}
}

#line 579 "../../src/dcl.c"
{ Ptype _au3_t ;
int _au3_const_old ;
bit _au3_vec_seen ;
Pexpr _au3_init ;

#line 579 "../../src/dcl.c"
_au3_t = _au1_nn -> _expr__O2.__C2_tp ;
_au3_const_old = const_save ;
_au3_vec_seen = 0 ;
_au3_init = _au0_this -> _expr__O4.__C4_n_initializer ;

#line 584 "../../src/dcl.c"
if (_au3_init ){ 
#line 585 "../../src/dcl.c"
switch (_au0_this -> _name_n_scope ){ 
#line 586 "../../src/dcl.c"
case 0 : 
#line 587 "../../src/dcl.c"
case 25 : 
#line 588 "../../src/dcl.c"
if (_au0_this -> _name_n_stclass != 31 ){ 
#line 1011 "../../src/dcl.c"
struct
#line 1011 "../../src/dcl.c"
ea _au0__V48 ;

#line 588 "../../src/dcl.c"
error ( (char *)"Ir forM%n", (struct ea *)( ( ((& _au0__V48 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V48 )))) )
#line 588 "../../src/dcl.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} break ;
}
}

#line 595 "../../src/dcl.c"
lll :
#line 596 "../../src/dcl.c"
switch (_au3_t -> _node_base ){ 
#line 597 "../../src/dcl.c"
case 158 : 
#line 598 "../../src/dcl.c"
if (_au3_init ){ 
#line 599 "../../src/dcl.c"
if (_au1_nn -> _name_n_scope == 136 )break ;
_au3_init = _expr_typ ( _au3_init , _au0_tbl ) ;
if ((_au0_this -> _name_n_sto == 31 )&& (_expr_lval ( _au3_init , (unsigned char )0 ) == 0 ))
#line 602 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V49 ;

#line 602 "../../src/dcl.c"
error ( (char *)"Ir for staticR%n not an lvalue", (struct ea *)( ( ((& _au0__V49 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V49 )))) )
#line 602 "../../src/dcl.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} else 
#line 604 "../../src/dcl.c"
_au1_nn -> _expr__O4.__C4_n_initializer = (_au0_this -> _expr__O4.__C4_n_initializer = ref_init ( ((struct ptr *)_au3_t ), _au3_init , _au0_tbl ) );
_name_assign ( _au1_nn ) ;
}
else { 
#line 608 "../../src/dcl.c"
switch (_au1_nn -> _name_n_scope ){ 
#line 609 "../../src/dcl.c"
default : 
#line 610 "../../src/dcl.c"
if (_au0_this -> _name_n_sto == 14 )break ;
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V50 ;

#line 611 "../../src/dcl.c"
error ( (char *)"unIdR%n", (struct ea *)( ( ((& _au0__V50 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V50 )))) )
#line 611 "../../src/dcl.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
case 136 : 
#line 613 "../../src/dcl.c"
break ;
case 25 : 
#line 615 "../../src/dcl.c"
case 0 : 
#line 616 "../../src/dcl.c"
if (_au0_this -> _name_n_sto == 31 ){ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V51 ;

#line 616 "../../src/dcl.c"
error ( (char *)"a staticM%n cannot be aR", (struct ea *)( ( ((& _au0__V51 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V51 )))) )
#line 616 "../../src/dcl.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} break ;
} }
}
break ;
case 119 : 
#line 622 "../../src/dcl.c"
{ Pname _au5_cn ;
Pclass _au5_cl ;
Pname _au5_ctor ;
Pname _au5_dtor ;

#line 622 "../../src/dcl.c"
_au5_cn = (((struct basetype *)_au3_t ))-> _basetype_b_name ;
_au5_cl = (((struct classdef *)_au5_cn -> _expr__O2.__C2_tp ));
_au5_ctor = ( _table_look ( _au5_cl -> _classdef_memtbl , "_ctor", (unsigned char )0 ) ) ;
_au5_dtor = ( _table_look ( _au5_cl -> _classdef_memtbl , "_dtor", (unsigned char )0 ) ) ;

#line 627 "../../src/dcl.c"
if (_au5_dtor ){ 
#line 628 "../../src/dcl.c"
Pstmt _au6_dls ;
switch (_au1_nn -> _name_n_scope ){ 
#line 630 "../../src/dcl.c"
case 14 : 
#line 631 "../../src/dcl.c"
if (_au0_this -> _name_n_sto == 14 )break ;
case 31 : 
#line 633 "../../src/dcl.c"
{ Ptable _au8_otbl ;

#line 633 "../../src/dcl.c"
_au8_otbl = _au0_tbl ;

#line 637 "../../src/dcl.c"
if (std_tbl == 0 )std_tbl = (struct table *)_table__ctor ( (struct table *)0 , (short )8 , gtbl , (struct name *)0 ) ;
_au0_tbl = std_tbl ;
if (_au3_vec_seen ){ 
#line 640 "../../src/dcl.c"
int _au9_esz ;
Pexpr _au9_noe ;
Pexpr _au9_sz ;
Pexpr _au9_arg ;

#line 644 "../../src/dcl.c"
struct call *_au0__Xthis__ctor_call ;

#line 644 "../../src/dcl.c"
struct estmt *_au0__Xthis__ctor_estmt ;

#line 644 "../../src/dcl.c"
struct ival *_au0__Xthis__ctor_ival ;

#line 644 "../../src/dcl.c"
int _au1__Xii__ctor_ival ;

#line 640 "../../src/dcl.c"
_au9_esz = _type_tsizeof ( (struct type *)_au5_cl ) ;
_au9_noe = (struct expr *)( (_au0__Xthis__ctor_ival = 0 ), ( (_au1__Xii__ctor_ival = (_type_tsizeof ( _au1_nn -> _expr__O2.__C2_tp ) / _au9_esz )), ( (
#line 641 "../../src/dcl.c"
(_au0__Xthis__ctor_ival = 0 ), (_au0__Xthis__ctor_ival = (struct ival *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ival ), (unsigned char )150 , (struct expr *)0 , (struct expr *)0 )
#line 641 "../../src/dcl.c"
)) , ( (_au0__Xthis__ctor_ival -> _expr__O3.__C3_i1 = _au1__Xii__ctor_ival ), ((_au0__Xthis__ctor_ival ))) ) ) ) ;
_au9_sz = (struct expr *)( (_au0__Xthis__ctor_ival = 0 ), ( ( (_au0__Xthis__ctor_ival = 0 ), (_au0__Xthis__ctor_ival = (struct ival *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ival ),
#line 642 "../../src/dcl.c"
(unsigned char )150 , (struct expr *)0 , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ival -> _expr__O3.__C3_i1 = _au9_esz ), ((_au0__Xthis__ctor_ival )))
#line 642 "../../src/dcl.c"
) ) ;
_au9_arg = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , (struct expr *)_au5_dtor , zero ) ;
_expr_lval ( (struct expr *)_au5_dtor , (unsigned char )112 ) ;
_au9_arg = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , _au9_sz , _au9_arg ) ;
_au9_arg = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , _au9_noe , _au9_arg ) ;
_au9_arg = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , (struct expr *)_au1_nn , _au9_arg ) ;
_au9_arg = (struct expr *)( (_au0__Xthis__ctor_call = 0 ), ( ( (_au0__Xthis__ctor_call = 0 ), (_au0__Xthis__ctor_call = (struct call *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_call ),
#line 648 "../../src/dcl.c"
(unsigned char )109 , ((struct expr *)vec_del_fct ), _au9_arg ) )) , ((_au0__Xthis__ctor_call ))) ) ;
_au9_arg -> _node_base = 146 ;
_au9_arg -> _expr__O5.__C5_fct_name = vec_del_fct ;
_au9_arg -> _expr__O2.__C2_tp = (struct type *)any_type ;
_au6_dls = (struct stmt *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt = (struct estmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_estmt ),
#line 652 "../../src/dcl.c"
((unsigned char )72 ), _au1_nn -> _name_where , ((struct stmt *)0 )) )) , ( (_au0__Xthis__ctor_estmt -> _stmt__O8.__C8_e = _au9_arg ), ((_au0__Xthis__ctor_estmt )))
#line 652 "../../src/dcl.c"
) ) ;
}
else { 
#line 655 "../../src/dcl.c"
Pref _au9_r ;
Pexpr _au9_ee ;
Pcall _au9_dl ;

#line 658 "../../src/dcl.c"
struct estmt *_au0__Xthis__ctor_estmt ;

#line 658 "../../src/dcl.c"
struct ref *_au0__Xthis__ctor_ref ;

#line 658 "../../src/dcl.c"
struct call *_au0__Xthis__ctor_call ;

#line 655 "../../src/dcl.c"
_au9_r = (struct ref *)( (_au0__Xthis__ctor_ref = 0 ), ( ( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ref ),
#line 655 "../../src/dcl.c"
((unsigned char )45 ), ((struct expr *)_au1_nn ), (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = _au5_dtor ), ((_au0__Xthis__ctor_ref )))
#line 655 "../../src/dcl.c"
) ) ;
_au9_ee = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , zero , (struct expr *)0 ) ;
_au9_dl = (struct call *)( (_au0__Xthis__ctor_call = 0 ), ( ( (_au0__Xthis__ctor_call = 0 ), (_au0__Xthis__ctor_call = (struct call *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_call ),
#line 657 "../../src/dcl.c"
(unsigned char )109 , ((struct expr *)_au9_r ), _au9_ee ) )) , ((_au0__Xthis__ctor_call ))) ) ;
_au6_dls = (struct stmt *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt = (struct estmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_estmt ),
#line 658 "../../src/dcl.c"
((unsigned char )72 ), _au1_nn -> _name_where , ((struct stmt *)0 )) )) , ( (_au0__Xthis__ctor_estmt -> _stmt__O8.__C8_e = ((struct expr *)_au9_dl )),
#line 658 "../../src/dcl.c"
((_au0__Xthis__ctor_estmt ))) ) ) ;
_au9_dl -> _node_base = 146 ;
_au9_dl -> _expr__O5.__C5_fct_name = _au5_dtor ;
_au9_dl -> _expr__O2.__C2_tp = (struct type *)any_type ;
}

#line 664 "../../src/dcl.c"
if (st_dlist )_au6_dls -> _stmt_s_list = st_dlist ;
st_dlist = _au6_dls ;
_au0_tbl = _au8_otbl ;
}
}
}
if (_au5_ctor ){ 
#line 671 "../../src/dcl.c"
Pexpr _au6_oo ;

#line 671 "../../src/dcl.c"
_au6_oo = (struct expr *)_au1_nn ;
{ int _au6_vi ;

#line 672 "../../src/dcl.c"
_au6_vi = _au3_vec_seen ;

#line 672 "../../src/dcl.c"
for(;_au6_vi ;_au6_vi -- ) _au6_oo = _expr_contents ( _au6_oo ) ;
{ int _au6_sti ;

#line 673 "../../src/dcl.c"
_au6_sti = 0 ;

#line 675 "../../src/dcl.c"
switch (_au1_nn -> _name_n_scope ){ 
#line 676 "../../src/dcl.c"
case 14 : 
#line 677 "../../src/dcl.c"
if ((_au3_init == 0 )&& (_au0_this -> _name_n_sto == 14 ))goto ggg ;
case 31 : 
#line 679 "../../src/dcl.c"
_au6_sti = 1 ;
if (_au0_tbl != gtbl ){ 
#line 683 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V52 ;

#line 683 "../../src/dcl.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"local static%n ofCWK", (struct ea *)( ( ((& _au0__V52 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((&
#line 683 "../../src/dcl.c"
_au0__V52 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
default : 
#line 686 "../../src/dcl.c"
if (_au3_vec_seen && _au3_init ){ 
#line 687 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V53 ;

#line 687 "../../src/dcl.c"
error ( (char *)"Ir forCO%n\[\]", (struct ea *)( ( ((& _au0__V53 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V53 )))) )
#line 687 "../../src/dcl.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
_au0_this -> _expr__O4.__C4_n_initializer = (_au3_init = 0 );
} }
break ;
case 136 : 
#line 704 "../../src/dcl.c"
case 25 : 
#line 705 "../../src/dcl.c"
case 0 : 
#line 706 "../../src/dcl.c"
goto ggg ;
}
const_save = 1 ;
_name_assign ( _au1_nn ) ;

#line 711 "../../src/dcl.c"
{ Ptable _au6_otbl ;

#line 759 "../../src/dcl.c"
Pname _au6_c ;

#line 760 "../../src/dcl.c"
struct texpr *_au0__Xthis__ctor_texpr ;

#line 711 "../../src/dcl.c"
_au6_otbl = _au0_tbl ;
if (_au6_sti ){ 
#line 715 "../../src/dcl.c"
if (sti_tbl == 0 )sti_tbl = (struct table *)_table__ctor ( (struct table *)0 , (short )8 , gtbl , (struct name *)0 )
#line 715 "../../src/dcl.c"
;
_au0_tbl = sti_tbl ;
if (_au0_this -> _name_n_sto == 14 )_au1_nn -> _name_n_sto = (_au0_this -> _name_n_sto = 0 );
}

#line 720 "../../src/dcl.c"
if (_au3_init ){ 
#line 721 "../../src/dcl.c"
if (_au3_init -> _node_base == 157 ){ 
#line 723 "../../src/dcl.c"
switch (_au3_init -> _expr__O5.__C5_tp2 -> _node_base ){ 
#line 724 "../../src/dcl.c"
case 6 : 
#line 725 "../../src/dcl.c"
if ((((struct
#line 725 "../../src/dcl.c"
classdef *)_au3_init -> _expr__O5.__C5_tp2 ))!= _au5_cl )goto inin ;
break ;
default : 
#line 728 "../../src/dcl.c"
{ Pname _au9_n2 ;

#line 728 "../../src/dcl.c"
_au9_n2 = _type_is_cl_obj ( _au3_init -> _expr__O5.__C5_tp2 ) ;
if ((_au9_n2 == 0 )|| ((((struct classdef *)_au9_n2 -> _expr__O2.__C2_tp ))!= _au5_cl ))goto inin ;
}
}

#line 731 "../../src/dcl.c"
_au3_init -> _expr__O4.__C4_e2 = _au6_oo ;
_au3_init = _expr_typ ( _au3_init , _au0_tbl ) ;
if (_au3_init -> _node_base == 147 )
#line 734 "../../src/dcl.c"
switch (_au3_init -> _expr__O5.__C5_tp2 -> _node_base ){ 
#line 735 "../../src/dcl.c"
case 6 : 
#line 736 "../../src/dcl.c"
if ((((struct classdef *)_au3_init -> _expr__O5.__C5_tp2 ))!= _au5_cl )goto
#line 736 "../../src/dcl.c"
inin ;
break ;
default : 
#line 739 "../../src/dcl.c"
{ Pname _au9_n2 ;

#line 739 "../../src/dcl.c"
_au9_n2 = _type_is_cl_obj ( _au3_init -> _expr__O5.__C5_tp2 ) ;
if ((_au9_n2 == 0 )|| ((((struct classdef *)_au9_n2 -> _expr__O2.__C2_tp ))!= _au5_cl ))goto inin ;
}
}
}
else 
#line 743 "../../src/dcl.c"
{ 
#line 744 "../../src/dcl.c"
inin :
#line 746 "../../src/dcl.c"
_au3_init = _expr_typ ( _au3_init , _au0_tbl ) ;
if ((_au3_init -> _node_base == 147 )&& (_type_check ( _au1_nn -> _expr__O2.__C2_tp , _au3_init -> _expr__O2.__C2_tp , (unsigned char )70 ) == 0 ))
#line 748 "../../src/dcl.c"
(replace_temp (
#line 748 "../../src/dcl.c"
_au3_init , _expr_address ( (struct expr *)_au1_nn ) ) );
else 
#line 750 "../../src/dcl.c"
_au3_init = class_init ( (struct expr *)_au1_nn , _au1_nn -> _expr__O2.__C2_tp , _au3_init , _au0_tbl ) ;
}
}
else 
#line 754 "../../src/dcl.c"
{ 
#line 755 "../../src/dcl.c"
_au3_init = (struct expr *)( (_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr = (struct texpr *)_expr__ctor (
#line 755 "../../src/dcl.c"
((struct expr *)_au0__Xthis__ctor_texpr ), ((unsigned char )157 ), ((struct expr *)0 ), (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 =
#line 755 "../../src/dcl.c"
((struct type *)_au5_cl )), ((_au0__Xthis__ctor_texpr ))) ) ) ;
_au3_init -> _expr__O4.__C4_e2 = _au6_oo ;
_au3_init = _expr_typ ( _au3_init , _au0_tbl ) ;
}
;
if (_au3_vec_seen ){ 
#line 761 "../../src/dcl.c"
_au6_c = _classdef_has_ictor ( _au5_cl ) ;
if (_au6_c == 0 )
#line 763 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V54 ;

#line 763 "../../src/dcl.c"
error ( (char *)"vector ofC%n that does not have aK taking noAs", (struct ea *)( ( ((& _au0__V54 )-> _ea__O1.__C1_p = ((char *)_au5_cn )), (((& _au0__V54 )))) )
#line 763 "../../src/dcl.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} else if ((((struct fct *)_au6_c -> _expr__O2.__C2_tp ))-> _fct_nargs )
#line 765 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V55 ;

#line 765 "../../src/dcl.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"defaultAs forK for vector ofC%n", (struct ea *)( ( ((& _au0__V55 )-> _ea__O1.__C1_p = ((char *)_au5_cn )), (((&
#line 765 "../../src/dcl.c"
_au0__V55 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }

#line 768 "../../src/dcl.c"
if (_au6_sti ){ 
#line 769 "../../src/dcl.c"
if (_au3_vec_seen ){ 
#line 770 "../../src/dcl.c"
int _au8_esz ;
Pexpr _au8_noe ;
Pexpr _au8_sz ;
Pexpr _au8_arg ;

#line 774 "../../src/dcl.c"
struct call *_au0__Xthis__ctor_call ;

#line 774 "../../src/dcl.c"
struct ival *_au0__Xthis__ctor_ival ;

#line 774 "../../src/dcl.c"
int _au1__Xii__ctor_ival ;

#line 770 "../../src/dcl.c"
_au8_esz = _type_tsizeof ( (struct type *)_au5_cl ) ;
_au8_noe = (struct expr *)( (_au0__Xthis__ctor_ival = 0 ), ( (_au1__Xii__ctor_ival = (_type_tsizeof ( _au1_nn -> _expr__O2.__C2_tp ) / _au8_esz )), ( (
#line 771 "../../src/dcl.c"
(_au0__Xthis__ctor_ival = 0 ), (_au0__Xthis__ctor_ival = (struct ival *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ival ), (unsigned char )150 , (struct expr *)0 , (struct expr *)0 )
#line 771 "../../src/dcl.c"
)) , ( (_au0__Xthis__ctor_ival -> _expr__O3.__C3_i1 = _au1__Xii__ctor_ival ), ((_au0__Xthis__ctor_ival ))) ) ) ) ;
_au8_sz = (struct expr *)( (_au0__Xthis__ctor_ival = 0 ), ( ( (_au0__Xthis__ctor_ival = 0 ), (_au0__Xthis__ctor_ival = (struct ival *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ival ),
#line 772 "../../src/dcl.c"
(unsigned char )150 , (struct expr *)0 , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ival -> _expr__O3.__C3_i1 = _au8_esz ), ((_au0__Xthis__ctor_ival )))
#line 772 "../../src/dcl.c"
) ) ;
_au8_arg = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , (struct expr *)_au6_c , (struct expr *)0 ) ;
_expr_lval ( (struct expr *)_au6_c , (unsigned char )112 ) ;
_au8_arg = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , _au8_sz , _au8_arg ) ;
_au8_arg = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , _au8_noe , _au8_arg ) ;
_au8_arg = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , (struct expr *)_au1_nn , _au8_arg ) ;
_au3_init = (struct expr *)( (_au0__Xthis__ctor_call = 0 ), ( ( (_au0__Xthis__ctor_call = 0 ), (_au0__Xthis__ctor_call = (struct call *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_call ),
#line 778 "../../src/dcl.c"
(unsigned char )109 , ((struct expr *)vec_new_fct ), _au8_arg ) )) , ((_au0__Xthis__ctor_call ))) ) ;
_au3_init -> _node_base = 146 ;
_au3_init -> _expr__O5.__C5_fct_name = vec_new_fct ;
_au3_init -> _expr__O2.__C2_tp = (struct type *)any_type ;
}
else { 
#line 785 "../../src/dcl.c"
switch (_au3_init -> _node_base ){ 
#line 786 "../../src/dcl.c"
case 111 : 
#line 787 "../../src/dcl.c"
if (_au3_init -> _expr__O3.__C3_e1 -> _node_base == 146 ){ 
#line 788 "../../src/dcl.c"
Pname _au10_fn ;

#line 788 "../../src/dcl.c"
_au10_fn = _au3_init -> _expr__O3.__C3_e1 -> _expr__O5.__C5_fct_name ;
if ((_au10_fn == 0 )|| (_au10_fn -> _name_n_oper != 161 ))goto as ;
_au3_init = _au3_init -> _expr__O3.__C3_e1 ;
break ;
}
goto as ;
case 70 : 
#line 795 "../../src/dcl.c"
if (_au3_init -> _expr__O3.__C3_e1 == (struct expr *)_au1_nn )break ;
as :
#line 797 "../../src/dcl.c"
default : 
#line 798 "../../src/dcl.c"
_au3_init = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )70 , (struct expr *)_au1_nn , _au3_init ) ;
#line 798 "../../src/dcl.c"
}
}

#line 801 "../../src/dcl.c"
{ Pstmt _au7_ist ;

#line 803 "../../src/dcl.c"
static Pstmt _static_itail = 0 ;

#line 804 "../../src/dcl.c"
struct estmt *_au0__Xthis__ctor_estmt ;

#line 801 "../../src/dcl.c"
_au7_ist = (struct stmt *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt = (struct estmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_estmt ),
#line 801 "../../src/dcl.c"
((unsigned char )72 ), _au1_nn -> _name_where , ((struct stmt *)0 )) )) , ( (_au0__Xthis__ctor_estmt -> _stmt__O8.__C8_e = _au3_init ), ((_au0__Xthis__ctor_estmt )))
#line 801 "../../src/dcl.c"
) ) ;

#line 804 "../../src/dcl.c"
if (st_ilist == 0 )
#line 805 "../../src/dcl.c"
st_ilist = _au7_ist ;
else 
#line 807 "../../src/dcl.c"
_static_itail -> _stmt_s_list = _au7_ist ;
_static_itail = _au7_ist ;
_au3_init = 0 ;
}
}

#line 811 "../../src/dcl.c"
_au1_nn -> _expr__O4.__C4_n_initializer = (_au0_this -> _expr__O4.__C4_n_initializer = _au3_init );
const_save = _au3_const_old ;
_au0_tbl = _au6_otbl ;
}
}
}
}
else 
#line 815 "../../src/dcl.c"
if (_au3_init == 0 )
#line 816 "../../src/dcl.c"
goto str ;
else if ((( (((unsigned char )((_au5_cl -> _classdef_csu == 6 )? (((unsigned int )0 )): (((unsigned int )_au5_cl ->
#line 817 "../../src/dcl.c"
_classdef_csu )))))) && (_au5_cl -> _classdef_csu != 36 ))&& (_au5_cl -> _classdef_csu != 167 ))
#line 819 "../../src/dcl.c"
{ 
#line 820 "../../src/dcl.c"
if ((_au3_init -> _node_base == 124 )&& ((_au5_cl -> _type_defined &
#line 820 "../../src/dcl.c"
3)== 0 ))
#line 822 "../../src/dcl.c"
{ 
#line 823 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V56 ;

#line 1011 "../../src/dcl.c"
struct ea _au0__V57 ;

#line 823 "../../src/dcl.c"
error ( (char *)"struct%nU: cannotI%n", (struct ea *)( ( ((& _au0__V56 )-> _ea__O1.__C1_p = ((char *)_au5_cn )), (((& _au0__V56 )))) )
#line 823 "../../src/dcl.c"
, (struct ea *)( ( ((& _au0__V57 )-> _ea__O1.__C1_p = ((char *)_au1_nn )), (((& _au0__V57 )))) ) , (struct
#line 823 "../../src/dcl.c"
ea *)ea0 , (struct ea *)ea0 ) ;
Cdcl = _au1_odcl ;
return (struct name *)0 ;
} }
_au3_init = _expr_typ ( _au3_init , _au0_tbl ) ;
if ((_type_check ( _au1_nn -> _expr__O2.__C2_tp , _au3_init -> _expr__O2.__C2_tp , (unsigned char )70 ) == 0 )&& (_au3_init -> _node_base == 147 ))
#line 830 "../../src/dcl.c"
(replace_temp (
#line 830 "../../src/dcl.c"
_au3_init , _expr_address ( (struct expr *)_au1_nn ) ) );
else 
#line 832 "../../src/dcl.c"
goto str ;
}
else if (_au3_init -> _node_base == 124 ){ 
#line 835 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V58 ;

#line 835 "../../src/dcl.c"
error ( (char *)"cannotI%nWIrL", (struct ea *)( ( ((& _au0__V58 )-> _ea__O1.__C1_p = ((char *)_au1_nn )), (((& _au0__V58 )))) )
#line 835 "../../src/dcl.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
else { 
#line 840 "../../src/dcl.c"
_au3_init = _expr_typ ( _au3_init , _au0_tbl ) ;
if (_type_check ( _au1_nn -> _expr__O2.__C2_tp , _au3_init -> _expr__O2.__C2_tp , (unsigned char )70 ) == 0 ){ 
#line 842 "../../src/dcl.c"
if (_au3_init -> _node_base ==
#line 842 "../../src/dcl.c"
147 )
#line 843 "../../src/dcl.c"
(replace_temp ( _au3_init , _expr_address ( (struct expr *)_au1_nn ) ) );
else 
#line 845 "../../src/dcl.c"
goto str ;
}
else 
#line 848 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V59 ;

#line 1011 "../../src/dcl.c"
struct ea _au0__V60 ;

#line 1011 "../../src/dcl.c"
struct ea _au0__V61 ;

#line 848 "../../src/dcl.c"
error ( (char *)"cannotI%n:%k %s has noK", (struct ea *)( ( ((& _au0__V59 )-> _ea__O1.__C1_p = ((char *)_au1_nn )), (((& _au0__V59 )))) )
#line 848 "../../src/dcl.c"
, (struct ea *)( ( ((& _au0__V60 )-> _ea__O1.__C1_i = ((int )_au5_cl -> _classdef_csu )), (((& _au0__V60 )))) ) ,
#line 848 "../../src/dcl.c"
(struct ea *)( ( ((& _au0__V61 )-> _ea__O1.__C1_p = ((char *)_au5_cl -> _classdef_string )), (((& _au0__V61 )))) ) , (struct
#line 848 "../../src/dcl.c"
ea *)ea0 ) ;
} }
break ;
}
case 110 : 
#line 853 "../../src/dcl.c"
_au3_t = (((struct vec *)_au3_t ))-> _pvtyp_typ ;
_au3_vec_seen ++ ;
goto lll ;
case 97 : 
#line 857 "../../src/dcl.c"
if ((_au3_init == 0 )&& (((struct basetype *)_au3_t ))-> _basetype_b_const ){ 
#line 858 "../../src/dcl.c"
switch (_au0_this -> _name_n_scope ){ 
#line 859 "../../src/dcl.c"
case 136 : 
#line 860 "../../src/dcl.c"
case
#line 860 "../../src/dcl.c"
0 : 
#line 861 "../../src/dcl.c"
case 25 : 
#line 862 "../../src/dcl.c"
break ;
default : 
#line 864 "../../src/dcl.c"
if ((_au0_this -> _name_n_sto != 14 )&& (_type_is_cl_obj ( _au3_t ) == 0 )){ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V62 ;

#line 864 "../../src/dcl.c"
error ( (char *)"unId const%n", (struct ea *)( ( ((& _au0__V62 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V62 )))) )
#line 864 "../../src/dcl.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}
_au3_t = (((struct basetype *)_au3_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
goto lll ;
default : 
#line 870 "../../src/dcl.c"
str :
#line 872 "../../src/dcl.c"
if (_au3_init == 0 ){ 
#line 873 "../../src/dcl.c"
switch (_au0_this -> _name_n_scope ){ 
#line 874 "../../src/dcl.c"
case 136 : 
#line 875 "../../src/dcl.c"
case 0 : 
#line 876 "../../src/dcl.c"
case 25 :
#line 876 "../../src/dcl.c"

#line 877 "../../src/dcl.c"
break ;
default : 
#line 879 "../../src/dcl.c"
if ((_au0_this -> _name_n_sto != 14 )&& _type_tconst ( _au3_t ) ){ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V63 ;

#line 879 "../../src/dcl.c"
error ( (char *)"unId const%n", (struct ea *)( ( ((& _au0__V63 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V63 )))) )
#line 879 "../../src/dcl.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }

#line 882 "../../src/dcl.c"
break ;
}

#line 887 "../../src/dcl.c"
const_save = ((const_save || (_au0_this -> _name_n_scope == 136 ))|| (_type_tconst ( _au3_t ) && (vec_const == 0 )));
_au1_nn -> _expr__O4.__C4_n_initializer = (_au0_this -> _expr__O4.__C4_n_initializer = (_au3_init = _expr_typ ( _au3_init , _au0_tbl ) ));
if (const_save )_au3_init -> _node_permanent = 1 ;
_name_assign ( _au1_nn ) ;
const_save = _au3_const_old ;

#line 893 "../../src/dcl.c"
switch (_au3_init -> _node_base ){ 
#line 894 "../../src/dcl.c"
case 124 : 
#line 895 "../../src/dcl.c"
new_list ( _au3_init ) ;
list_check ( _au1_nn , _au1_nn -> _expr__O2.__C2_tp , (struct expr *)0 ) ;
if (next_elem ( ) )error ( (char *)"IrL too long", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 )
#line 897 "../../src/dcl.c"
;
break ;
case 81 : 
#line 900 "../../src/dcl.c"
if (_au1_nn -> _expr__O2.__C2_tp -> _node_base == 110 ){ 
#line 901 "../../src/dcl.c"
Pvec _au6_v ;

#line 901 "../../src/dcl.c"
_au6_v = (((struct vec *)_au1_nn -> _expr__O2.__C2_tp ));
if (_au6_v -> _pvtyp_typ -> _node_base == 5 ){ 
#line 904 "../../src/dcl.c"
int _au7_sz ;
int _au7_isz ;

#line 904 "../../src/dcl.c"
_au7_sz = _au6_v -> _vec_size ;
_au7_isz = (((struct vec *)_au3_init -> _expr__O2.__C2_tp ))-> _vec_size ;
if (_au7_sz == 0 )
#line 907 "../../src/dcl.c"
_au6_v -> _vec_size = _au7_isz ;
else if (_au7_sz < _au7_isz )
#line 909 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V64 ;

#line 1011 "../../src/dcl.c"
struct ea _au0__V65 ;

#line 1011 "../../src/dcl.c"
struct ea _au0__V66 ;

#line 909 "../../src/dcl.c"
error ( (char *)"Ir too long (%d characters) for%n[%d]", (struct ea *)( ( ((& _au0__V64 )-> _ea__O1.__C1_i = _au7_isz ), (((& _au0__V64 )))) ) ,
#line 909 "../../src/dcl.c"
(struct ea *)( ( ((& _au0__V65 )-> _ea__O1.__C1_p = ((char *)_au1_nn )), (((& _au0__V65 )))) ) , (struct ea *)(
#line 909 "../../src/dcl.c"
( ((& _au0__V66 )-> _ea__O1.__C1_i = _au7_sz ), (((& _au0__V66 )))) ) , (struct ea *)ea0 ) ;
} break ;
}
}
default : 
#line 914 "../../src/dcl.c"
{ Ptype _au6_nt ;
int _au6_ntc ;

#line 914 "../../src/dcl.c"
_au6_nt = _au1_nn -> _expr__O2.__C2_tp ;
_au6_ntc = (((struct basetype *)_au6_nt ))-> _basetype_b_const ;

#line 917 "../../src/dcl.c"
if (_au3_vec_seen ){ 
#line 918 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V67 ;

#line 918 "../../src/dcl.c"
error ( (char *)"badIr for vector%n", (struct ea *)( ( ((& _au0__V67 )-> _ea__O1.__C1_p = ((char *)_au1_nn )), (((& _au0__V67 )))) )
#line 918 "../../src/dcl.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
break ;
} }
tlx :
#line 922 "../../src/dcl.c"
switch (_au6_nt -> _node_base ){ 
#line 923 "../../src/dcl.c"
case 97 : 
#line 924 "../../src/dcl.c"
_au6_nt = (((struct basetype *)_au6_nt ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
_au6_ntc |= (((struct basetype *)_au6_nt ))-> _basetype_b_const ;
goto tlx ;
case 21 : 
#line 928 "../../src/dcl.c"
case 5 : 
#line 929 "../../src/dcl.c"
case 29 : 
#line 930 "../../src/dcl.c"
if ((_au3_init -> _node_base == 82 )&& (_au3_init -> _expr__O2.__C2_tp == (struct type *)long_type ))
#line 931 "../../src/dcl.c"
{
#line 931 "../../src/dcl.c"

#line 1011 "../../src/dcl.c"
struct ea _au0__V68 ;

#line 1011 "../../src/dcl.c"
struct ea _au0__V69 ;

#line 931 "../../src/dcl.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"longIr constant for%k%n", (struct ea *)( ( ((& _au0__V68 )-> _ea__O1.__C1_i = ((int )_au1_nn -> _expr__O2.__C2_tp ->
#line 931 "../../src/dcl.c"
_node_base )), (((& _au0__V68 )))) ) , (struct ea *)( ( ((& _au0__V69 )-> _ea__O1.__C1_p = ((char *)_au1_nn )), (((&
#line 931 "../../src/dcl.c"
_au0__V69 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} case 22 : 
#line 933 "../../src/dcl.c"
if (((((struct basetype *)_au6_nt ))-> _basetype_b_unsigned && (_au3_init -> _node_base == 107 ))&& (_au3_init -> _expr__O4.__C4_e2 -> _node_base == 82 ))
#line 936 "../../src/dcl.c"
{
#line 936 "../../src/dcl.c"

#line 1011 "../../src/dcl.c"
struct ea _au0__V70 ;

#line 936 "../../src/dcl.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"negativeIr for unsigned%n", (struct ea *)( ( ((& _au0__V70 )-> _ea__O1.__C1_p = ((char *)_au1_nn )), (((&
#line 936 "../../src/dcl.c"
_au0__V70 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} if (_au6_ntc && (_au0_scope != 136 )){ 
#line 938 "../../src/dcl.c"
int _au8_i ;
Neval = 0 ;
_au8_i = _expr_eval ( _au3_init ) ;
if (Neval == 0 ){ 
#line 942 "../../src/dcl.c"
if (_au3_init && (_au3_init -> _node_permanent == 0 ))_expr_del ( _au3_init ) ;
_au1_nn -> _name_n_evaluated = (_au0_this -> _name_n_evaluated = 1 );
_au1_nn -> _name_n_val = (_au0_this -> _name_n_val = _au8_i );
_au1_nn -> _expr__O4.__C4_n_initializer = (_au0_this -> _expr__O4.__C4_n_initializer = 0 );
}
}
break ;
case 125 : 
#line 950 "../../src/dcl.c"
_au0_this -> _expr__O4.__C4_n_initializer = (_au3_init = ptr_init ( ((struct ptr *)_au6_nt ), _au3_init , _au0_tbl ) );
}

#line 953 "../../src/dcl.c"
{ Pexpr _au7_x ;

#line 953 "../../src/dcl.c"
_au7_x = try_to_coerce ( _au6_nt , _au3_init , "initializer", _au0_tbl ) ;
if (_au7_x ){ 
#line 955 "../../src/dcl.c"
_au0_this -> _expr__O4.__C4_n_initializer = _au7_x ;
goto stgg ;
}
}

#line 960 "../../src/dcl.c"
if (_type_check ( _au6_nt , _au3_init -> _expr__O2.__C2_tp , (unsigned char )70 ) )
#line 961 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V71 ;

#line 1011 "../../src/dcl.c"
struct ea _au0__V72 ;

#line 1011 "../../src/dcl.c"
struct ea _au0__V73 ;

#line 961 "../../src/dcl.c"
error ( (char *)"badIrT%t for%n (%tX)", (struct ea *)( ( ((& _au0__V71 )-> _ea__O1.__C1_p = ((char *)_au3_init -> _expr__O2.__C2_tp )), (((& _au0__V71 ))))
#line 961 "../../src/dcl.c"
) , (struct ea *)( ( ((& _au0__V72 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V72 )))) ) ,
#line 961 "../../src/dcl.c"
(struct ea *)( ( ((& _au0__V73 )-> _ea__O1.__C1_p = ((char *)_au1_nn -> _expr__O2.__C2_tp )), (((& _au0__V73 )))) ) , (struct
#line 961 "../../src/dcl.c"
ea *)ea0 ) ;
} else { 
#line 963 "../../src/dcl.c"
stgg :
#line 964 "../../src/dcl.c"
if (_au3_init && (_au0_this -> _name_n_stclass == 31 )){ 
#line 967 "../../src/dcl.c"
switch (_au3_init -> _node_base ){ 
#line 968 "../../src/dcl.c"
case 85 : 
#line 969 "../../src/dcl.c"
if
#line 969 "../../src/dcl.c"
(_type_tconst ( _au3_init -> _expr__O2.__C2_tp ) == 0 ){ 
#line 1011 "../../src/dcl.c"
struct ea _au0__V74 ;

#line 1011 "../../src/dcl.c"
struct ea _au0__V75 ;

#line 969 "../../src/dcl.c"
error ( (char *)"V%n used inIr for%n", (struct ea *)( ( ((& _au0__V74 )-> _ea__O1.__C1_p = ((char *)_au3_init )), (((& _au0__V74 )))) )
#line 969 "../../src/dcl.c"
, (struct ea *)( ( ((& _au0__V75 )-> _ea__O1.__C1_p = ((char *)_au1_nn )), (((& _au0__V75 )))) ) , (struct
#line 969 "../../src/dcl.c"
ea *)ea0 , (struct ea *)ea0 ) ;
} break ;
case 111 : 
#line 972 "../../src/dcl.c"
case 45 : 
#line 973 "../../src/dcl.c"
case 44 : 
#line 974 "../../src/dcl.c"
case 109 : 
#line 975 "../../src/dcl.c"
case 146 : 
#line 976 "../../src/dcl.c"
case 23 : 
#line 977 "../../src/dcl.c"
{ 
#line 1011 "../../src/dcl.c"
struct
#line 1011 "../../src/dcl.c"
ea _au0__V76 ;

#line 1011 "../../src/dcl.c"
struct ea _au0__V77 ;

#line 977 "../../src/dcl.c"
error ( (char *)"%k inIr of static%n", (struct ea *)( ( ((& _au0__V76 )-> _ea__O1.__C1_i = ((int )_au3_init -> _node_base )), (((& _au0__V76 ))))
#line 977 "../../src/dcl.c"
) , (struct ea *)( ( ((& _au0__V77 )-> _ea__O1.__C1_p = ((char *)_au1_nn )), (((& _au0__V77 )))) ) ,
#line 977 "../../src/dcl.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}
}
}
}
}
}
}

#line 987 "../../src/dcl.c"
ggg :
#line 988 "../../src/dcl.c"
_au1_nn -> _node_permanent = 1 ;
switch (_au0_this -> _name_n_scope ){ 
#line 990 "../../src/dcl.c"
case 108 : 
#line 991 "../../src/dcl.c"
_au1_nn -> _expr__O4.__C4_n_initializer = _au0_this -> _expr__O4.__C4_n_initializer ;
break ;
default : 
#line 994 "../../src/dcl.c"
{ 
#line 995 "../../src/dcl.c"
Ptype _au3_t ;

#line 995 "../../src/dcl.c"
_au3_t = _au1_nn -> _expr__O2.__C2_tp ;

#line 997 "../../src/dcl.c"
px :
#line 998 "../../src/dcl.c"
_au3_t -> _node_permanent = 1 ;
switch (_au3_t -> _node_base ){ 
#line 1000 "../../src/dcl.c"
case 125 : 
#line 1001 "../../src/dcl.c"
case 158 : 
#line 1002 "../../src/dcl.c"
case 110 : _au3_t = (((struct ptr *)_au3_t ))-> _pvtyp_typ ;

#line 1002 "../../src/dcl.c"
goto px ;
case 97 : _au3_t = (((struct basetype *)_au3_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 1003 "../../src/dcl.c"
goto px ;
case 108 : _au3_t = (((struct fct *)_au3_t ))-> _fct_returns ;

#line 1004 "../../src/dcl.c"
goto px ;
}
}
}

#line 1009 "../../src/dcl.c"
Cdcl = _au1_odcl ;
return _au1_nn ;
}
;

/* the end */
