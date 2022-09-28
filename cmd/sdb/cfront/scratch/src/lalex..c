/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#line 1 "../../src/lalex.c"

/* <<cfront 1.2.1 2/16/87>> */
/* < ../../src/lalex.c */

#ident	"@(#)sdb:cfront/scratch/src/lalex..c	1.2"
char *_new(); char _delete(); char *_vec_new(); char _vec_delete();

#line 1 "../../src/lalex.c"

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

#ident	"@(#)sdb:cfront/src/yystype.h	1.1"

#line 2 "../../src/yystype.h"
union _C10 {	/* sizeof _C10 == 4 */

#line 3 "../../src/yystype.h"
char *__C10_s ;
TOK __C10_t ;
int __C10_i ;
struct loc __C10_l ;
Pname __C10_pn ;
Ptype __C10_pt ;
Pexpr __C10_pe ;
Pstmt __C10_ps ;
PP __C10_p ;
};
typedef union _C10 YYSTYPE ;
extern union _C10 yylval ;

#ident	"@(#)sdb:cfront/src/tqueue.h	1.1"

#line 6 "../../src/tqueue.h"
extern int printf ();
extern int fprintf ();

#line 10 "../../src/tqueue.h"
struct toknode {	/* sizeof toknode == 16 */

#line 11 "../../src/tqueue.h"
TOK _toknode_tok ;
union _C10 _toknode_retval ;
struct toknode *_toknode_next ;
struct toknode *_toknode_last ;
};
struct toknode *_toknode__ctor ();

#line 18 "../../src/tqueue.h"
extern struct toknode *front ;
extern struct toknode *rear ;

#line 21 "../../src/tqueue.h"
extern char addtok ();
extern TOK deltok ();

#line 25 "../../src/tqueue.h"
extern char tlex ();
extern TOK lalex ();
extern char *malloc ();

#line 29 "../../src/tqueue.h"
extern union _C10 yylval ;
extern TOK tk ;

#line 32 "../../src/tqueue.h"
extern char *image ();

#line 29 "../../src/lalex.c"
struct toknode *front = 0 ;
struct toknode *rear = 0 ;

#line 32 "../../src/lalex.c"
struct toknode *free_toks = 0 ;

#line 35 "../../src/lalex.c"
struct toknode *_toknode__ctor (_au0_this , _au0_t , _au0_r )
#line 17 "../../src/tqueue.h"
struct toknode *_au0_this ;

#line 35 "../../src/lalex.c"
TOK _au0_t ;

#line 35 "../../src/lalex.c"
union _C10 _au0_r ;
{ 
#line 37 "../../src/lalex.c"
if (free_toks == 0 ){ 
#line 38 "../../src/lalex.c"
int _au2_i ;
register struct toknode *_au2_q ;

#line 38 "../../src/lalex.c"
_au2_i = (sizeof (struct toknode ));
_au2_q = (((struct toknode *)malloc ( (unsigned int )(16 * _au2_i )) ));
free_toks = _au2_q ;
for(( (_au2_q += 15), (_au2_q -> _toknode_next = 0 )) ;_au2_q != free_toks ;_au2_q -- ) (_au2_q - 1 )-> _toknode_next = _au2_q ;
}
_au0_this = free_toks ;
free_toks = free_toks -> _toknode_next ;
_au0_this -> _toknode_tok = _au0_t ;
_au0_this -> _toknode_retval = _au0_r ;
_au0_this -> _toknode_next = (_au0_this -> _toknode_last = 0 );
return _au0_this ;
}
;

#line 50 "../../src/lalex.c"

#line 58 "../../src/lalex.c"
extern char addtok (_au0_t , _au0_r )TOK _au0_t ;

#line 58 "../../src/lalex.c"
union _C10 _au0_r ;
{ 
#line 60 "../../src/lalex.c"
struct toknode *_au1_T ;

#line 60 "../../src/lalex.c"
_au1_T = (struct toknode *)_toknode__ctor ( (struct toknode *)0 , _au0_t , _au0_r ) ;
if (front == 0 )
#line 62 "../../src/lalex.c"
front = (rear = _au1_T );
else { 
#line 64 "../../src/lalex.c"
rear -> _toknode_next = _au1_T ;
_au1_T -> _toknode_last = rear ;
rear = _au1_T ;
}
}
;

#line 71 "../../src/lalex.c"
TOK tk ;

#line 73 "../../src/lalex.c"
extern TOK deltok ()
#line 74 "../../src/lalex.c"
{ 
#line 75 "../../src/lalex.c"
struct toknode *_au1_T ;

#line 76 "../../src/lalex.c"
struct toknode *_au0__Xthis__dtor_toknode ;

#line 75 "../../src/lalex.c"
_au1_T = front ;
tk = _au1_T -> _toknode_tok ;
yylval = _au1_T -> _toknode_retval ;
if (front = front -> _toknode_next )front -> _toknode_last = 0 ;
( (_au0__Xthis__dtor_toknode = _au1_T ), ( (_au0__Xthis__dtor_toknode ? ( (_au0__Xthis__dtor_toknode -> _toknode_next = free_toks ), ( (free_toks = _au0__Xthis__dtor_toknode ), ( (_au0__Xthis__dtor_toknode =
#line 79 "../../src/lalex.c"
0 ), (_au0__Xthis__dtor_toknode ? (_delete ( (char *)_au0__Xthis__dtor_toknode ) ): 0 )) ) ) : 0 )) ) ;
return tk ;
}
;
int scan_type ();
int scan_mod ();
int scan_tlist ();
int scan_suf ();
char get_tag ();
char scan_e ();

#line 91 "../../src/lalex.c"
struct toknode *latok = 0 ;

#line 93 "../../src/lalex.c"

#line 100 "../../src/lalex.c"

#line 107 "../../src/lalex.c"

#line 113 "../../src/lalex.c"
char insert_tok (_au0_t )TOK _au0_t ;
{ 
#line 115 "../../src/lalex.c"
struct toknode *_au1_nt ;

#line 115 "../../src/lalex.c"
_au1_nt = (struct toknode *)_toknode__ctor ( (struct toknode *)0 , _au0_t , yylval ) ;
_au1_nt -> _toknode_last = latok -> _toknode_last ;
_au1_nt -> _toknode_last -> _toknode_next = _au1_nt ;
_au1_nt -> _toknode_next = latok ;
latok -> _toknode_last = _au1_nt ;
latok = _au1_nt ;
}
;
char rep_cast ()
#line 125 "../../src/lalex.c"
{ 
#line 126 "../../src/lalex.c"
struct toknode *_au1_tt ;
struct toknode *_au1_junk ;

#line 126 "../../src/lalex.c"
_au1_tt = front -> _toknode_next ;
_au1_junk = _au1_tt -> _toknode_next ;
if (_au1_junk == latok )return ;
_au1_tt -> _toknode_tok = 97 ;
_au1_tt -> _toknode_retval . __C10_pt = (struct type *)any_type ;
_au1_tt -> _toknode_next = latok ;
latok -> _toknode_last -> _toknode_next = 0 ;
latok -> _toknode_last = _au1_tt ;
_au1_tt = _au1_junk ;
while (_au1_tt ){ 
#line 136 "../../src/lalex.c"
register struct toknode *_au2_tx ;

#line 137 "../../src/lalex.c"
struct toknode *_au0__Xthis__dtor_toknode ;

#line 136 "../../src/lalex.c"
_au2_tx = _au1_tt -> _toknode_next ;
( (_au0__Xthis__dtor_toknode = _au1_tt ), ( (_au0__Xthis__dtor_toknode ? ( (_au0__Xthis__dtor_toknode -> _toknode_next = free_toks ), ( (free_toks = _au0__Xthis__dtor_toknode ), ( (_au0__Xthis__dtor_toknode =
#line 137 "../../src/lalex.c"
0 ), (_au0__Xthis__dtor_toknode ? (_delete ( (char *)_au0__Xthis__dtor_toknode ) ): 0 )) ) ) : 0 )) ) ;
_au1_tt = _au2_tx ;
}
}
;

#line 148 "../../src/lalex.c"
int bad_cast = 0 ;

#line 150 "../../src/lalex.c"
extern TOK lalex ()
#line 151 "../../src/lalex.c"
{ 
#line 152 "../../src/lalex.c"
static int _static_nocast = 0 ;

#line 154 "../../src/lalex.c"
static int _static_incast = 0 ;
static int _static_in_enum ;
static int _static_fr ;
char _au1_en ;

#line 157 "../../src/lalex.c"
_au1_en = 0 ;

#line 159 "../../src/lalex.c"
switch (( ((front == 0 )? tlex ( ) : 0 ), ( (latok = front ), latok -> _toknode_tok ) )
#line 159 "../../src/lalex.c"
){ 
#line 160 "../../src/lalex.c"
case 13 : 
#line 161 "../../src/lalex.c"
_au1_en = 1 ;
case 156 : 
#line 163 "../../src/lalex.c"
switch (tk ){ 
#line 170 "../../src/lalex.c"
case 0 : 
#line 171 "../../src/lalex.c"
case 40 : 
#line 172 "../../src/lalex.c"
case 71 : 
#line 173 "../../src/lalex.c"
case 23 : 
#line 174 "../../src/lalex.c"
case 113 :
#line 174 "../../src/lalex.c"

#line 175 "../../src/lalex.c"
case 41 : 
#line 176 "../../src/lalex.c"
case 24 : 
#line 177 "../../src/lalex.c"
case 10 : 
#line 178 "../../src/lalex.c"
case 97 : 
#line 179 "../../src/lalex.c"
case 69 : 
#line 180 "../../src/lalex.c"
case 72 : 
#line 181 "../../src/lalex.c"
case 74 :
#line 181 "../../src/lalex.c"

#line 182 "../../src/lalex.c"
case 73 : 
#line 183 "../../src/lalex.c"
break ;
default : 
#line 185 "../../src/lalex.c"
{ 
#line 562 "../../src/lalex.c"
struct ea _au0__V11 ;

#line 185 "../../src/lalex.c"
errorFPCloc__PC_RCea__RCea__RCea__RCea___ ( & curloc , (char *)"';' missing afterS orD before\"%k\"", (struct ea *)( ( ((& _au0__V11 )-> _ea__O1.__C1_i = ((int )latok -> _toknode_tok )),
#line 185 "../../src/lalex.c"
(((& _au0__V11 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
return (tk = 72 );
} }

#line 189 "../../src/lalex.c"
{ TOK _au3_t ;
TOK _au3_x ;

#line 189 "../../src/lalex.c"
_au3_t = ( ((latok == rear )? tlex ( ) : 0 ), ( (latok = latok -> _toknode_next ), latok -> _toknode_tok )
#line 189 "../../src/lalex.c"
) ;

#line 192 "../../src/lalex.c"
switch (_au3_t ){ 
#line 193 "../../src/lalex.c"
case 123 : 
#line 194 "../../src/lalex.c"
_au3_x = ( ((latok == rear )? tlex ( ) : 0 ), ( (latok =
#line 194 "../../src/lalex.c"
latok -> _toknode_next ), latok -> _toknode_tok ) ) ;
break ;
case 80 : 
#line 197 "../../src/lalex.c"
_au3_x = ( ((latok == rear )? tlex ( ) : 0 ), ( (latok = latok -> _toknode_next ),
#line 197 "../../src/lalex.c"
latok -> _toknode_tok ) ) ;
( ((latok -> _toknode_last == 0 )? errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"token q backup", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 198 "../../src/lalex.c"
ea *)ea0 , (struct ea *)ea0 ) : 0 ), (latok = latok -> _toknode_last )) ;

#line 200 "../../src/lalex.c"
switch (_au3_x ){ 
#line 201 "../../src/lalex.c"
case 73 : 
#line 202 "../../src/lalex.c"
_static_in_enum = _au1_en ;
case 69 : 
#line 204 "../../src/lalex.c"
( ((latok -> _toknode_last == 0 )? errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"token q backup", (struct ea *)ea0 , (struct
#line 204 "../../src/lalex.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) : 0 ), (latok = latok -> _toknode_last )) ;
_static_fr = 0 ;
goto ret ;
default : 
#line 208 "../../src/lalex.c"
{ Pname _au6_n ;

#line 208 "../../src/lalex.c"
_au6_n = _table_look ( ktbl , latok -> _toknode_retval . __C10_s , (unsigned char )159 ) ;
if (_au6_n == 0 ){ 
#line 210 "../../src/lalex.c"
_au6_n = (struct name *)_name__ctor ( (struct name *)0 , latok -> _toknode_retval . __C10_s ) ;
_au6_n -> _name_lex_level = 0 ;
_au6_n = _name_tname ( _au6_n , latok -> _toknode_last -> _toknode_retval . __C10_t ) ;
modified_tn = modified_tn -> _name_list_l ;
}
else { 
#line 216 "../../src/lalex.c"
switch (_au6_n -> _expr__O2.__C2_tp -> _node_base ){ 
#line 217 "../../src/lalex.c"
case 119 : 
#line 218 "../../src/lalex.c"
case 121 : 
#line 219 "../../src/lalex.c"
break ;
default : 
#line 221 "../../src/lalex.c"
{ 
#line 562 "../../src/lalex.c"
struct ea _au0__V12 ;

#line 562 "../../src/lalex.c"
struct ea _au0__V13 ;

#line 221 "../../src/lalex.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"hidden%n:%t", (struct ea *)( ( ((& _au0__V12 )-> _ea__O1.__C1_p = ((char *)_au6_n )), (((&
#line 221 "../../src/lalex.c"
_au0__V12 )))) ) , (struct ea *)( ( ((& _au0__V13 )-> _ea__O1.__C1_p = ((char *)_au6_n -> _expr__O2.__C2_tp )), (((& _au0__V13 ))))
#line 221 "../../src/lalex.c"
) , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}
latok -> _toknode_tok = 123 ;
latok -> _toknode_retval . __C10_pn = _au6_n ;
}
}
(( ((latok == rear )? tlex ( ) : 0 ), ( (latok = latok -> _toknode_next ), latok -> _toknode_tok ) )
#line 228 "../../src/lalex.c"
);
break ;
case 73 : 
#line 231 "../../src/lalex.c"
_static_in_enum = _au1_en ;
default : 
#line 233 "../../src/lalex.c"
_static_fr = 0 ;
goto ret ;
}

#line 235 "../../src/lalex.c"
;

#line 238 "../../src/lalex.c"
switch (_au3_x ){ 
#line 239 "../../src/lalex.c"
case 73 : 
#line 240 "../../src/lalex.c"
_static_in_enum = _au1_en ;
case 69 : 
#line 242 "../../src/lalex.c"
_static_fr = 0 ;
goto ret ;
case 72 : 
#line 245 "../../src/lalex.c"
if ((tk != 23 )&& (_static_fr == 0 )){ 
#line 247 "../../src/lalex.c"
switch (tk ){ 
#line 248 "../../src/lalex.c"
case 0 : 
#line 249 "../../src/lalex.c"
case 74 : 
#line 250 "../../src/lalex.c"
case
#line 250 "../../src/lalex.c"
73 : 
#line 251 "../../src/lalex.c"
case 72 : break ;
default : 
#line 253 "../../src/lalex.c"
{ 
#line 562 "../../src/lalex.c"
struct ea _au0__V14 ;

#line 253 "../../src/lalex.c"
error ( (char *)"syntax error: invalid token: %k", (struct ea *)( ( ((& _au0__V14 )-> _ea__O1.__C1_i = ((int )tk )), (((& _au0__V14 )))) )
#line 253 "../../src/lalex.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }

#line 256 "../../src/lalex.c"
deltok ( ) ;
deltok ( ) ;
deltok ( ) ;
return lalex ( ) ;
}

#line 262 "../../src/lalex.c"
default : 
#line 263 "../../src/lalex.c"
deltok ( ) ;
_static_fr = 0 ;
goto ret ;
}
}

#line 269 "../../src/lalex.c"
case 40 : 
#line 271 "../../src/lalex.c"
_static_fr = 0 ;
if (_static_nocast ){ 
#line 273 "../../src/lalex.c"
_static_nocast = 0 ;
goto ret ;
}
else 
#line 278 "../../src/lalex.c"
if (_static_incast )
#line 279 "../../src/lalex.c"
goto ret ;

#line 281 "../../src/lalex.c"
bad_cast = 0 ;
if (scan_type ( ) ){ 
#line 283 "../../src/lalex.c"
if (scan_mod ( ) ){ 
#line 284 "../../src/lalex.c"
if (( ((latok == rear )? tlex ( )
#line 284 "../../src/lalex.c"
: 0 ), ( (latok = latok -> _toknode_next ), latok -> _toknode_tok ) ) != 41 )goto ret ;
switch (( ((latok == rear )? tlex ( ) : 0 ), ( (latok = latok -> _toknode_next ), latok -> _toknode_tok )
#line 285 "../../src/lalex.c"
) ){ 
#line 286 "../../src/lalex.c"
case 71 : case 41 : case 72 : case 73 : case 70 : 
#line 288 "../../src/lalex.c"
if (tk !=
#line 288 "../../src/lalex.c"
30 )goto ret ;
break ;

#line 291 "../../src/lalex.c"
case 54 : case 55 : case 50 : case 52 : 
#line 292 "../../src/lalex.c"
case 23 : case 9 : case 30 :
#line 292 "../../src/lalex.c"
case 160 : 
#line 293 "../../src/lalex.c"
case 46 : case 47 : case 95 : 
#line 294 "../../src/lalex.c"
case 40 : case 113 : 
#line 295 "../../src/lalex.c"
case 80 :
#line 295 "../../src/lalex.c"
case 97 : case 123 : 
#line 296 "../../src/lalex.c"
case 34 : case 24 : case 86 : 
#line 297 "../../src/lalex.c"
case 82 : case 83 :
#line 297 "../../src/lalex.c"
case 84 : case 81 : 
#line 299 "../../src/lalex.c"
break ;
default : 
#line 304 "../../src/lalex.c"
if (bad_cast )goto ret ;
else break ;
}
( ((latok -> _toknode_last == 0 )? errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"token q backup", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 307 "../../src/lalex.c"
ea *)ea0 , (struct ea *)ea0 ) : 0 ), (latok = latok -> _toknode_last )) ;
front -> _toknode_tok = 113 ;
latok -> _toknode_tok = 122 ;
if (bad_cast ){ 
#line 311 "../../src/lalex.c"
error ( (char *)"can't cast toF", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 311 "../../src/lalex.c"

#line 312 "../../src/lalex.c"
rep_cast ( ) ;
}
_static_incast = 1 ;
}
}
goto ret ;
case 113 : 
#line 319 "../../src/lalex.c"
_static_incast ++ ;
goto ret ;
case 122 : 
#line 322 "../../src/lalex.c"
if ((-- _static_incast )== 0 )_static_nocast = 0 ;
goto ret ;
case 80 : 
#line 325 "../../src/lalex.c"
{ char *_au3_s ;

#line 325 "../../src/lalex.c"
_au3_s = front -> _toknode_retval . __C10_s ;

#line 327 "../../src/lalex.c"
_static_fr = 0 ;
_static_nocast = 1 ;
switch (( ((latok == rear )? tlex ( ) : 0 ), ( (latok = latok -> _toknode_next ), latok -> _toknode_tok )
#line 329 "../../src/lalex.c"
) ){ 
#line 330 "../../src/lalex.c"
case 80 : 
#line 331 "../../src/lalex.c"
{ 
#line 333 "../../src/lalex.c"
char *_au5_s2 ;

#line 333 "../../src/lalex.c"
_au5_s2 = latok -> _toknode_retval . __C10_s ;
( ((latok -> _toknode_last == 0 )? errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"token q backup", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 334 "../../src/lalex.c"
ea *)ea0 , (struct ea *)ea0 ) : 0 ), (latok = latok -> _toknode_last )) ;
{ Pname _au5_n ;

#line 335 "../../src/lalex.c"
_au5_n = _table_look ( ktbl , _au3_s , (unsigned char )159 ) ;
if (_au5_n == 0 ){ 
#line 337 "../../src/lalex.c"
{ 
#line 562 "../../src/lalex.c"
struct ea _au0__V15 ;

#line 562 "../../src/lalex.c"
struct ea _au0__V16 ;

#line 562 "../../src/lalex.c"
struct ea _au0__V17 ;

#line 337 "../../src/lalex.c"
error ( (char *)"%s %s:TX (%s is not a TN)", (struct ea *)( ( ((& _au0__V15 )-> _ea__O1.__C1_p = ((char *)_au3_s )), (((& _au0__V15 )))) )
#line 337 "../../src/lalex.c"
, (struct ea *)( ( ((& _au0__V16 )-> _ea__O1.__C1_p = ((char *)_au5_s2 )), (((& _au0__V16 )))) ) , (struct
#line 337 "../../src/lalex.c"
ea *)( ( ((& _au0__V17 )-> _ea__O1.__C1_p = ((char *)_au3_s )), (((& _au0__V17 )))) ) , (struct ea *)ea0 ) ;
#line 337 "../../src/lalex.c"

#line 338 "../../src/lalex.c"
_au5_n = (struct name *)_name__ctor ( (struct name *)0 , _au3_s ) ;
_au5_n -> _name_lex_level = 0 ;
_au5_n = _name_tname ( _au5_n , (unsigned char )0 ) ;
modified_tn = modified_tn -> _name_list_l ;
_au5_n -> _expr__O2.__C2_tp = (struct type *)any_type ;
} }
else { 
#line 345 "../../src/lalex.c"
{ 
#line 562 "../../src/lalex.c"
struct ea _au0__V18 ;

#line 562 "../../src/lalex.c"
struct ea _au0__V19 ;

#line 562 "../../src/lalex.c"
struct ea _au0__V20 ;

#line 345 "../../src/lalex.c"
error ( (char *)"%s %s: %s is hidden", (struct ea *)( ( ((& _au0__V18 )-> _ea__O1.__C1_p = ((char *)_au3_s )), (((& _au0__V18 )))) )
#line 345 "../../src/lalex.c"
, (struct ea *)( ( ((& _au0__V19 )-> _ea__O1.__C1_p = ((char *)_au5_s2 )), (((& _au0__V19 )))) ) , (struct
#line 345 "../../src/lalex.c"
ea *)( ( ((& _au0__V20 )-> _ea__O1.__C1_p = ((char *)_au3_s )), (((& _au0__V20 )))) ) , (struct ea *)ea0 ) ;
#line 345 "../../src/lalex.c"
} }

#line 347 "../../src/lalex.c"
latok -> _toknode_tok = 123 ;
latok -> _toknode_retval . __C10_pn = _au5_n ;
break ;
}
}

#line 351 "../../src/lalex.c"
case 73 : 
#line 352 "../../src/lalex.c"
( ((latok -> _toknode_last == 0 )? errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"token q backup", (struct ea *)ea0 , (struct
#line 352 "../../src/lalex.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) : 0 ), (latok = latok -> _toknode_last )) ;
front -> _toknode_retval . __C10_pn = (struct name *)_name__ctor ( (struct name *)0 , _au3_s ) ;
front -> _toknode_retval . __C10_pn -> _name_lex_level -- ;
break ;
default : 
#line 357 "../../src/lalex.c"
( ((latok -> _toknode_last == 0 )? errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"token q backup", (struct ea *)ea0 , (struct
#line 357 "../../src/lalex.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) : 0 ), (latok = latok -> _toknode_last )) ;
front -> _toknode_retval . __C10_pn = (struct name *)_name__ctor ( (struct name *)0 , _au3_s ) ;
}
goto ret ;
}
case 4 : 
#line 363 "../../src/lalex.c"
case 8 : 
#line 364 "../../src/lalex.c"
case 175 : 
#line 365 "../../src/lalex.c"
case 12 : 
#line 366 "../../src/lalex.c"
_static_fr = 0 ;
switch (tk ){ 
#line 368 "../../src/lalex.c"
case 69 : 
#line 369 "../../src/lalex.c"
case 72 : 
#line 370 "../../src/lalex.c"
case 74 : 
#line 371 "../../src/lalex.c"
case 73 : 
#line 372 "../../src/lalex.c"
goto ret ;
default : 
#line 374 "../../src/lalex.c"
{ 
#line 562 "../../src/lalex.c"
struct ea _au0__V21 ;

#line 374 "../../src/lalex.c"
errorFPCloc__PC_RCea__RCea__RCea__RCea___ ( & curloc , (char *)"';' missing afterS orD before\"%k\"", (struct ea *)( ( ((& _au0__V21 )-> _ea__O1.__C1_i = ((int )latok -> _toknode_tok )),
#line 374 "../../src/lalex.c"
(((& _au0__V21 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
return (tk = 72 );
} }
case 10 : 
#line 378 "../../src/lalex.c"
case 19 : 
#line 379 "../../src/lalex.c"
case 7 : 
#line 380 "../../src/lalex.c"
case 3 : 
#line 381 "../../src/lalex.c"
case 28 : 
#line 382 "../../src/lalex.c"
_static_fr = 0 ;
switch (tk ){ 
#line 384 "../../src/lalex.c"
case 12 : 
#line 385 "../../src/lalex.c"
case 10 : 
#line 386 "../../src/lalex.c"
case 69 : 
#line 387 "../../src/lalex.c"
case 41 : 
#line 388 "../../src/lalex.c"
case 72 : 
#line 389 "../../src/lalex.c"
case 74 :
#line 389 "../../src/lalex.c"

#line 390 "../../src/lalex.c"
case 73 : 
#line 391 "../../src/lalex.c"
goto ret ;
default : 
#line 393 "../../src/lalex.c"
{ 
#line 562 "../../src/lalex.c"
struct ea _au0__V22 ;

#line 393 "../../src/lalex.c"
errorFPCloc__PC_RCea__RCea__RCea__RCea___ ( & curloc , (char *)"';' missing afterS orD before\"%k\"", (struct ea *)( ( ((& _au0__V22 )-> _ea__O1.__C1_i = ((int )latok -> _toknode_tok )),
#line 393 "../../src/lalex.c"
(((& _au0__V22 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
return (tk = 72 );
} }
case 20 : 
#line 397 "../../src/lalex.c"
case 39 : 
#line 398 "../../src/lalex.c"
case 16 : 
#line 399 "../../src/lalex.c"
case 33 : 
#line 400 "../../src/lalex.c"
_static_fr = 0 ;
switch (tk ){ 
#line 402 "../../src/lalex.c"
case 12 : 
#line 403 "../../src/lalex.c"
case 10 : 
#line 404 "../../src/lalex.c"
case 69 : 
#line 405 "../../src/lalex.c"
case 41 : 
#line 406 "../../src/lalex.c"
case 72 : 
#line 407 "../../src/lalex.c"
case 74 :
#line 407 "../../src/lalex.c"

#line 408 "../../src/lalex.c"
case 73 : 
#line 409 "../../src/lalex.c"
_static_nocast = 1 ;
goto ret ;
default : 
#line 412 "../../src/lalex.c"
{ 
#line 562 "../../src/lalex.c"
struct ea _au0__V23 ;

#line 412 "../../src/lalex.c"
errorFPCloc__PC_RCea__RCea__RCea__RCea___ ( & curloc , (char *)"';' missing afterS orD before\"%k\"", (struct ea *)( ( ((& _au0__V23 )-> _ea__O1.__C1_i = ((int )latok -> _toknode_tok )),
#line 412 "../../src/lalex.c"
(((& _au0__V23 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
return (tk = 72 );
} }
case 97 : 
#line 417 "../../src/lalex.c"
_static_fr = 0 ;
switch (tk ){ 
#line 419 "../../src/lalex.c"
case 80 : 
#line 421 "../../src/lalex.c"
case 43 : 
#line 422 "../../src/lalex.c"
{ 
#line 562 "../../src/lalex.c"
struct ea _au0__V24 ;

#line 422 "../../src/lalex.c"
errorFPCloc__PC_RCea__RCea__RCea__RCea___ ( & curloc , (char *)"';' missing afterS orD before\"%k\"", (struct ea *)( ( ((& _au0__V24 )-> _ea__O1.__C1_i = ((int )latok -> _toknode_tok )),
#line 422 "../../src/lalex.c"
(((& _au0__V24 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
return (tk = 72 );
} }
if (latok -> _toknode_retval . __C10_t == 18 )_static_fr = 1 ;
_static_nocast = 1 ;
goto ret ;
case 123 : 
#line 429 "../../src/lalex.c"
{ Pname _au3_n ;

#line 429 "../../src/lalex.c"
_au3_n = latok -> _toknode_retval . __C10_pn ;
if (_static_fr ){ 
#line 431 "../../src/lalex.c"
_static_nocast = 1 ;
_static_fr = 0 ;
goto ret ;
}
_static_fr = 0 ;
{ TOK _au3_otk ;

#line 436 "../../src/lalex.c"
_au3_otk = tk ;
switch (( ((latok == rear )? tlex ( ) : 0 ), ( (latok = latok -> _toknode_next ), latok -> _toknode_tok )
#line 437 "../../src/lalex.c"
) ){ 
#line 438 "../../src/lalex.c"
case 160 : 
#line 440 "../../src/lalex.c"
switch (( ((latok == rear )? tlex ( ) : 0 ), ( (latok =
#line 440 "../../src/lalex.c"
latok -> _toknode_next ), latok -> _toknode_tok ) ) ){ 
#line 441 "../../src/lalex.c"
case 50 : 
#line 442 "../../src/lalex.c"
deltok ( ) ;
deltok ( ) ;

#line 445 "../../src/lalex.c"
front -> _toknode_tok = 173 ;
front -> _toknode_retval . __C10_pn = _au3_n ;
_static_nocast = 1 ;
goto ret ;
default : 
#line 450 "../../src/lalex.c"
( ((latok -> _toknode_last == 0 )? errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"token q backup", (struct ea *)ea0 , (struct
#line 450 "../../src/lalex.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) : 0 ), (latok = latok -> _toknode_last )) ;
}
break ;
default : ;
}

#line 456 "../../src/lalex.c"
( ((latok -> _toknode_last == 0 )? errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"token q backup", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 456 "../../src/lalex.c"
ea *)ea0 , (struct ea *)ea0 ) : 0 ), (latok = latok -> _toknode_last )) ;
tk = _au3_otk ;

#line 459 "../../src/lalex.c"
switch (tk ){ 
#line 460 "../../src/lalex.c"
case 97 : 
#line 462 "../../src/lalex.c"
switch (( ((latok == rear )? tlex ( ) : 0 ), ( (latok =
#line 462 "../../src/lalex.c"
latok -> _toknode_next ), latok -> _toknode_tok ) ) ){ 
#line 463 "../../src/lalex.c"
case 72 : 
#line 464 "../../src/lalex.c"
case 43 : 
#line 465 "../../src/lalex.c"
case 69 : 
#line 466 "../../src/lalex.c"
case 70 :
#line 466 "../../src/lalex.c"

#line 467 "../../src/lalex.c"
goto hid ;

#line 469 "../../src/lalex.c"
default : 
#line 470 "../../src/lalex.c"
_static_nocast = 1 ;
goto ret ;
}

#line 474 "../../src/lalex.c"
case 123 : 
#line 475 "../../src/lalex.c"
switch (( ((latok == rear )? tlex ( ) : 0 ), ( (latok = latok -> _toknode_next ),
#line 475 "../../src/lalex.c"
latok -> _toknode_tok ) ) ){ 
#line 476 "../../src/lalex.c"
case 160 : 
#line 477 "../../src/lalex.c"
case 45 : 
#line 478 "../../src/lalex.c"
_static_nocast = 1 ;
goto ret ;
}
hid :
#line 482 "../../src/lalex.c"
( ((latok -> _toknode_last == 0 )? errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"token q backup", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 482 "../../src/lalex.c"
ea *)ea0 , (struct ea *)ea0 ) : 0 ), (latok = latok -> _toknode_last )) ;
_name_hide ( _au3_n ) ;
_au3_n = (struct name *)_name__ctor ( (struct name *)0 , _au3_n -> _expr__O3.__C3_string ) ;
_au3_n -> _name_n_oper = 123 ;
latok -> _toknode_tok = 80 ;
latok -> _toknode_retval . __C10_pn = _au3_n ;
}
}
}

#line 490 "../../src/lalex.c"
case 23 : 
#line 491 "../../src/lalex.c"
_static_fr = 0 ;
_static_nocast = 1 ;
goto ret ;

#line 513 "../../src/lalex.c"
case 74 : 
#line 515 "../../src/lalex.c"
_static_fr = 0 ;
switch (tk ){ 
#line 517 "../../src/lalex.c"
case 74 : 
#line 518 "../../src/lalex.c"
case 73 : 
#line 519 "../../src/lalex.c"
case 72 : 
#line 520 "../../src/lalex.c"
break ;
default : 
#line 522 "../../src/lalex.c"
{ TOK _au4_t ;
struct loc _au4_x ;

#line 523 "../../src/lalex.c"
_au4_x = curloc ;
switch (_au4_t = ( ((latok == rear )? tlex ( ) : 0 ), ( (latok = latok -> _toknode_next ), latok ->
#line 524 "../../src/lalex.c"
_toknode_tok ) ) ){ 
#line 525 "../../src/lalex.c"
case 12 : 
#line 526 "../../src/lalex.c"
case 74 : 
#line 527 "../../src/lalex.c"
case 71 : 
#line 528 "../../src/lalex.c"
case 72 : 
#line 529 "../../src/lalex.c"
case 41 : 
#line 530 "../../src/lalex.c"
break
#line 530 "../../src/lalex.c"
;
default : 
#line 535 "../../src/lalex.c"
if (_static_in_enum == 0 ){ 
#line 536 "../../src/lalex.c"
errorFPCloc__PC_RCea__RCea__RCea__RCea___ ( & _au4_x , (char *)"';'X at end ofS orD before '}'", (struct ea *)ea0 , (struct ea *)ea0 ,
#line 536 "../../src/lalex.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
return (tk = 72 );
}
_static_in_enum = 0 ;
}
}
}
_static_in_enum = 0 ;
default : 
#line 545 "../../src/lalex.c"
_static_fr = 0 ;
_static_nocast = 0 ;
goto ret ;
}
ret :
#line 553 "../../src/lalex.c"
{ struct toknode *_au1_T ;

#line 554 "../../src/lalex.c"
struct toknode *_au0__Xthis__dtor_toknode ;

#line 553 "../../src/lalex.c"
_au1_T = front ;
tk = _au1_T -> _toknode_tok ;
yylval = _au1_T -> _toknode_retval ;
if (front = front -> _toknode_next )front -> _toknode_last = 0 ;
( (_au0__Xthis__dtor_toknode = _au1_T ), ( (_au0__Xthis__dtor_toknode ? ( (_au0__Xthis__dtor_toknode -> _toknode_next = free_toks ), ( (free_toks = _au0__Xthis__dtor_toknode ), ( (_au0__Xthis__dtor_toknode =
#line 557 "../../src/lalex.c"
0 ), (_au0__Xthis__dtor_toknode ? (_delete ( (char *)_au0__Xthis__dtor_toknode ) ): 0 )) ) ) : 0 )) ) ;

#line 559 "../../src/lalex.c"
return tk ;
}
}
;

#line 565 "../../src/lalex.c"
extern int scan_type ()
#line 574 "../../src/lalex.c"
{ 
#line 575 "../../src/lalex.c"
int _au1_is_type ;

#line 575 "../../src/lalex.c"
_au1_is_type = 0 ;
for(;;) 
#line 577 "../../src/lalex.c"
switch (( ((latok == rear )? tlex ( ) : 0 ), ( (latok = latok -> _toknode_next ), latok ->
#line 577 "../../src/lalex.c"
_toknode_tok ) ) ){ 
#line 578 "../../src/lalex.c"
case 123 : 
#line 579 "../../src/lalex.c"
if (( ((latok == rear )? tlex ( ) : 0 ), (
#line 579 "../../src/lalex.c"
(latok = latok -> _toknode_next ), latok -> _toknode_tok ) ) == 160 ){ 
#line 580 "../../src/lalex.c"
( ((latok -> _toknode_last == 0 )? errorFI_PC_RCea__RCea__RCea__RCea___ ( (int
#line 580 "../../src/lalex.c"
)'i' , (char *)"token q backup", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) : 0 ), (latok =
#line 580 "../../src/lalex.c"
latok -> _toknode_last )) ;
goto def ;
}
( ((latok -> _toknode_last == 0 )? errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"token q backup", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 583 "../../src/lalex.c"
ea *)ea0 , (struct ea *)ea0 ) : 0 ), (latok = latok -> _toknode_last )) ;
goto ttt ;
case 156 : 
#line 586 "../../src/lalex.c"
case 13 : 
#line 587 "../../src/lalex.c"
get_tag ( ) ;
case 97 : 
#line 589 "../../src/lalex.c"
ttt :
#line 590 "../../src/lalex.c"
_au1_is_type = 1 ;
continue ;
default : 
#line 593 "../../src/lalex.c"
def :
#line 594 "../../src/lalex.c"
( ((latok -> _toknode_last == 0 )? errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"token q backup", (struct ea *)ea0 , (struct
#line 594 "../../src/lalex.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) : 0 ), (latok = latok -> _toknode_last )) ;
return _au1_is_type ;
}
}
;

#line 600 "../../src/lalex.c"
extern int scan_mod ()
#line 608 "../../src/lalex.c"
{ 
#line 609 "../../src/lalex.c"
for(;;) 
#line 610 "../../src/lalex.c"
switch (( ((latok == rear )? tlex ( ) : 0 ), ( (latok =
#line 610 "../../src/lalex.c"
latok -> _toknode_next ), latok -> _toknode_tok ) ) ){ 
#line 611 "../../src/lalex.c"
case 52 : 
#line 612 "../../src/lalex.c"
case 50 : 
#line 613 "../../src/lalex.c"
continue ;
case 40 : 
#line 615 "../../src/lalex.c"
switch (( ((latok == rear )? tlex ( ) : 0 ), ( (latok = latok -> _toknode_next ),
#line 615 "../../src/lalex.c"
latok -> _toknode_tok ) ) ){ 
#line 616 "../../src/lalex.c"
case 52 : 
#line 617 "../../src/lalex.c"
case 50 : 
#line 618 "../../src/lalex.c"
case 40 : 
#line 619 "../../src/lalex.c"
case 42 : 
#line 620 "../../src/lalex.c"
( ((latok ->
#line 620 "../../src/lalex.c"
_toknode_last == 0 )? errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"token q backup", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 620 "../../src/lalex.c"
ea *)ea0 ) : 0 ), (latok = latok -> _toknode_last )) ;
if (! scan_mod ( ) )return (int )0 ;
if (( ((latok == rear )? tlex ( ) : 0 ), ( (latok = latok -> _toknode_next ), latok -> _toknode_tok )
#line 622 "../../src/lalex.c"
) != 41 )return (int )0 ;
if (! scan_suf ( ) )return (int )0 ;
return 1 ;
case 156 : case 13 : case 97 : case 123 : 
#line 626 "../../src/lalex.c"
( ((latok -> _toknode_last == 0 )? errorFI_PC_RCea__RCea__RCea__RCea___ ( (int
#line 626 "../../src/lalex.c"
)'i' , (char *)"token q backup", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) : 0 ), (latok =
#line 626 "../../src/lalex.c"
latok -> _toknode_last )) ;
if (! scan_tlist ( ) )return (int )0 ;
if (( ((latok == rear )? tlex ( ) : 0 ), ( (latok = latok -> _toknode_next ), latok -> _toknode_tok )
#line 628 "../../src/lalex.c"
) != 41 )return (int )0 ;

#line 630 "../../src/lalex.c"
case 41 : 
#line 631 "../../src/lalex.c"
bad_cast = 1 ;
if (! scan_suf ( ) )return (int )0 ;
return 1 ;
default : 
#line 635 "../../src/lalex.c"
return (int )0 ;
}
case 42 : 
#line 638 "../../src/lalex.c"
( ((latok -> _toknode_last == 0 )? errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"token q backup", (struct ea *)ea0 , (struct
#line 638 "../../src/lalex.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) : 0 ), (latok = latok -> _toknode_last )) ;
if (! scan_suf ( ) )return (int )0 ;
return 1 ;
case 41 : 
#line 642 "../../src/lalex.c"
case 71 : 
#line 643 "../../src/lalex.c"
case 155 : 
#line 644 "../../src/lalex.c"
( ((latok -> _toknode_last == 0 )? errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char
#line 644 "../../src/lalex.c"
*)"token q backup", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) : 0 ), (latok = latok -> _toknode_last ))
#line 644 "../../src/lalex.c"
;
return 1 ;
case 123 : 
#line 647 "../../src/lalex.c"
if (( ((latok == rear )? tlex ( ) : 0 ), ( (latok = latok -> _toknode_next ),
#line 647 "../../src/lalex.c"
latok -> _toknode_tok ) ) == 160 ){ 
#line 648 "../../src/lalex.c"
if (( ((latok == rear )? tlex ( ) : 0 ), (
#line 648 "../../src/lalex.c"
(latok = latok -> _toknode_next ), latok -> _toknode_tok ) ) == 50 )continue ;
( ((latok -> _toknode_last == 0 )? errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"token q backup", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 649 "../../src/lalex.c"
ea *)ea0 , (struct ea *)ea0 ) : 0 ), (latok = latok -> _toknode_last )) ;
}
( ((latok -> _toknode_last == 0 )? errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"token q backup", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 651 "../../src/lalex.c"
ea *)ea0 , (struct ea *)ea0 ) : 0 ), (latok = latok -> _toknode_last )) ;
default : 
#line 653 "../../src/lalex.c"
return (int )0 ;
}
}
;

#line 658 "../../src/lalex.c"
extern int scan_suf ()
#line 668 "../../src/lalex.c"
{ 
#line 669 "../../src/lalex.c"
int _au1_found ;

#line 669 "../../src/lalex.c"
_au1_found = 0 ;
for(;;) 
#line 671 "../../src/lalex.c"
switch (( ((latok == rear )? tlex ( ) : 0 ), ( (latok = latok -> _toknode_next ), latok ->
#line 671 "../../src/lalex.c"
_toknode_tok ) ) ){ 
#line 672 "../../src/lalex.c"
case 42 : 
#line 673 "../../src/lalex.c"
scan_e ( ) ;
_au1_found = 1 ;
continue ;
case 40 : 
#line 677 "../../src/lalex.c"
if (! scan_tlist ( ) )return (int )0 ;
if (( ((latok == rear )? tlex ( ) : 0 ), ( (latok = latok -> _toknode_next ), latok -> _toknode_tok )
#line 678 "../../src/lalex.c"
) != 41 )return (int )0 ;
if (_au1_found )
#line 680 "../../src/lalex.c"
bad_cast = 1 ;
else 
#line 682 "../../src/lalex.c"
_au1_found = 1 ;
continue ;
default : 
#line 685 "../../src/lalex.c"
( ((latok -> _toknode_last == 0 )? errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"token q backup", (struct ea *)ea0 , (struct
#line 685 "../../src/lalex.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) : 0 ), (latok = latok -> _toknode_last )) ;
return 1 ;
}
}
;

#line 691 "../../src/lalex.c"
extern int scan_tlist ()
#line 696 "../../src/lalex.c"
{ 
#line 697 "../../src/lalex.c"
for(;;) { 
#line 698 "../../src/lalex.c"
switch (( ((latok == rear )? tlex ( ) : 0 ), (
#line 698 "../../src/lalex.c"
(latok = latok -> _toknode_next ), latok -> _toknode_tok ) ) ){ 
#line 699 "../../src/lalex.c"
case 156 : 
#line 700 "../../src/lalex.c"
case 13 : 
#line 701 "../../src/lalex.c"
get_tag ( ) ;
#line 701 "../../src/lalex.c"

#line 702 "../../src/lalex.c"
case 97 : 
#line 703 "../../src/lalex.c"
case 123 : 
#line 704 "../../src/lalex.c"
scan_type ( ) ;
break ;
case 155 : 
#line 707 "../../src/lalex.c"
if (( ((latok == rear )? tlex ( ) : 0 ), ( (latok = latok -> _toknode_next ),
#line 707 "../../src/lalex.c"
latok -> _toknode_tok ) ) != 41 ){ 
#line 708 "../../src/lalex.c"
error ( (char *)"missing ')' after '...'", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ,
#line 708 "../../src/lalex.c"
(struct ea *)ea0 ) ;
insert_tok ( (unsigned char )41 ) ;
}
case 41 : 
#line 712 "../../src/lalex.c"
( ((latok -> _toknode_last == 0 )? errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"token q backup", (struct ea *)ea0 , (struct
#line 712 "../../src/lalex.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) : 0 ), (latok = latok -> _toknode_last )) ;
return 1 ;
default : 
#line 715 "../../src/lalex.c"
return (int )0 ;
}

#line 719 "../../src/lalex.c"
if (! scan_mod ( ) )return (int )0 ;

#line 721 "../../src/lalex.c"
switch (( ((latok == rear )? tlex ( ) : 0 ), ( (latok = latok -> _toknode_next ), latok -> _toknode_tok )
#line 721 "../../src/lalex.c"
) ){ 
#line 722 "../../src/lalex.c"
case 71 : 
#line 723 "../../src/lalex.c"
continue ;
case 155 : 
#line 725 "../../src/lalex.c"
if (( ((latok == rear )? tlex ( ) : 0 ), ( (latok = latok -> _toknode_next ),
#line 725 "../../src/lalex.c"
latok -> _toknode_tok ) ) != 41 ){ 
#line 726 "../../src/lalex.c"
error ( (char *)"missing ')' after '...'", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ,
#line 726 "../../src/lalex.c"
(struct ea *)ea0 ) ;
insert_tok ( (unsigned char )41 ) ;
}
case 41 : 
#line 730 "../../src/lalex.c"
( ((latok -> _toknode_last == 0 )? errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"token q backup", (struct ea *)ea0 , (struct
#line 730 "../../src/lalex.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) : 0 ), (latok = latok -> _toknode_last )) ;
return 1 ;
default : 
#line 733 "../../src/lalex.c"
return (int )0 ;
}
}
}
;
extern char get_tag ()
#line 740 "../../src/lalex.c"
{ 
#line 741 "../../src/lalex.c"
switch (( ((latok == rear )? tlex ( ) : 0 ), ( (latok = latok ->
#line 741 "../../src/lalex.c"
_toknode_next ), latok -> _toknode_tok ) ) ){ 
#line 742 "../../src/lalex.c"
default : 
#line 743 "../../src/lalex.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'e' , (char *)"missing tag", (struct ea *)ea0 ,
#line 743 "../../src/lalex.c"
(struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
insert_tok ( (unsigned char )80 ) ;
latok -> _toknode_retval . __C10_s = "__MISSING__";
case 80 : 
#line 747 "../../src/lalex.c"
{ 
#line 748 "../../src/lalex.c"
Pname _au3_n ;

#line 748 "../../src/lalex.c"
_au3_n = _table_look ( ktbl , latok -> _toknode_retval . __C10_s , (unsigned char )159 ) ;
if (_au3_n == 0 ){ 
#line 750 "../../src/lalex.c"
_au3_n = (struct name *)_name__ctor ( (struct name *)0 , latok -> _toknode_retval . __C10_s ) ;
_au3_n -> _name_lex_level = 0 ;
_au3_n = _name_tname ( _au3_n , latok -> _toknode_last -> _toknode_retval . __C10_t ) ;
modified_tn = modified_tn -> _name_list_l ;
}
else { 
#line 756 "../../src/lalex.c"
switch (_au3_n -> _expr__O2.__C2_tp -> _node_base ){ 
#line 757 "../../src/lalex.c"
case 119 : 
#line 758 "../../src/lalex.c"
case 121 : 
#line 759 "../../src/lalex.c"
break ;
default : 
#line 761 "../../src/lalex.c"
{ 
#line 792 "../../src/lalex.c"
struct ea _au0__V25 ;

#line 792 "../../src/lalex.c"
struct ea _au0__V26 ;

#line 761 "../../src/lalex.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"hidden%n:%t", (struct ea *)( ( ((& _au0__V25 )-> _ea__O1.__C1_p = ((char *)_au3_n )), (((&
#line 761 "../../src/lalex.c"
_au0__V25 )))) ) , (struct ea *)( ( ((& _au0__V26 )-> _ea__O1.__C1_p = ((char *)_au3_n -> _expr__O2.__C2_tp )), (((& _au0__V26 ))))
#line 761 "../../src/lalex.c"
) , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}
latok -> _toknode_tok = 123 ;
latok -> _toknode_retval . __C10_pn = _au3_n ;
break ;
}
case 123 : 
#line 769 "../../src/lalex.c"
break ;
}

#line 772 "../../src/lalex.c"
switch (( ((latok == rear )? tlex ( ) : 0 ), ( (latok = latok -> _toknode_next ), latok -> _toknode_tok )
#line 772 "../../src/lalex.c"
) ){ 
#line 773 "../../src/lalex.c"
default : 
#line 774 "../../src/lalex.c"
( ((latok -> _toknode_last == 0 )? errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"token q backup", (struct
#line 774 "../../src/lalex.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) : 0 ), (latok = latok -> _toknode_last )) ;

#line 774 "../../src/lalex.c"
return ;
case 69 : 
#line 776 "../../src/lalex.c"
switch (( ((latok == rear )? tlex ( ) : 0 ), ( (latok = latok -> _toknode_next ),
#line 776 "../../src/lalex.c"
latok -> _toknode_tok ) ) ){ 
#line 777 "../../src/lalex.c"
case 80 : case 123 : case 73 : 
#line 778 "../../src/lalex.c"
break ;
default : 
#line 780 "../../src/lalex.c"
( ((latok -> _toknode_last == 0 )? errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"token q backup", (struct ea *)ea0 , (struct
#line 780 "../../src/lalex.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) : 0 ), (latok = latok -> _toknode_last )) ;

#line 780 "../../src/lalex.c"
return ;
}
case 73 : { int _au3_level ;

#line 782 "../../src/lalex.c"
_au3_level = 1 ;
for(;;) switch (( ((latok == rear )? tlex ( ) : 0 ), ( (latok = latok -> _toknode_next ), latok ->
#line 783 "../../src/lalex.c"
_toknode_tok ) ) ){ 
#line 784 "../../src/lalex.c"
case 73 : _au3_level ++ ;

#line 784 "../../src/lalex.c"
break ;
case 74 : if ((-- _au3_level )== 0 )return ;
break ;
case 0 : 
#line 788 "../../src/lalex.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"unexpected eof", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 788 "../../src/lalex.c"
ea *)ea0 ) ;
}
}
}
}
;
extern char scan_e ()
#line 796 "../../src/lalex.c"
{ 
#line 797 "../../src/lalex.c"
long _au1_brcount ;
int _au1_localcast ;

#line 797 "../../src/lalex.c"
_au1_brcount = 1L ;
_au1_localcast = 0 ;
for(;;) 
#line 800 "../../src/lalex.c"
switch (( ((latok == rear )? tlex ( ) : 0 ), ( (latok = latok -> _toknode_next ), latok ->
#line 800 "../../src/lalex.c"
_toknode_tok ) ) ){ 
#line 801 "../../src/lalex.c"
case 43 : 
#line 802 "../../src/lalex.c"
if ((-- _au1_brcount )== 0L )return ;
continue ;
case 42 : 
#line 805 "../../src/lalex.c"
_au1_brcount ++ ;
continue ;
case 40 : 
#line 808 "../../src/lalex.c"
{ if (_au1_localcast )
#line 809 "../../src/lalex.c"
continue ;
{ struct toknode *_au3_mark ;

#line 810 "../../src/lalex.c"
_au3_mark = latok ;
if (scan_type ( ) )
#line 812 "../../src/lalex.c"
if (scan_mod ( ) )
#line 813 "../../src/lalex.c"
if (( ((latok == rear )? tlex ( ) : 0 ),
#line 813 "../../src/lalex.c"
( (latok = latok -> _toknode_next ), latok -> _toknode_tok ) ) == 41 )
#line 814 "../../src/lalex.c"
switch (( ((latok == rear )? tlex ( )
#line 814 "../../src/lalex.c"
: 0 ), ( (latok = latok -> _toknode_next ), latok -> _toknode_tok ) ) ){ 
#line 815 "../../src/lalex.c"
case 71 : 
#line 816 "../../src/lalex.c"
case 41 :
#line 816 "../../src/lalex.c"

#line 817 "../../src/lalex.c"
case 72 : 
#line 818 "../../src/lalex.c"
case 73 : 
#line 819 "../../src/lalex.c"
case 70 : 
#line 820 "../../src/lalex.c"
break ;
default : 
#line 822 "../../src/lalex.c"
( ((latok -> _toknode_last == 0 )? errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"token q backup", (struct ea *)ea0 , (struct
#line 822 "../../src/lalex.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) : 0 ), (latok = latok -> _toknode_last )) ;
_au3_mark -> _toknode_tok = 113 ;
latok -> _toknode_tok = 122 ;
}
continue ;
}
}

#line 828 "../../src/lalex.c"
case 113 : 
#line 833 "../../src/lalex.c"
_au1_localcast ++ ;
continue ;
case 122 : 
#line 836 "../../src/lalex.c"
_au1_localcast -- ;
continue ;
default : 
#line 839 "../../src/lalex.c"
continue ;
}
}
;

/* the end */
