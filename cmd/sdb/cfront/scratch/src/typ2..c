/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#line 1 "../../src/typ2.c"

/* <<cfront 1.2.1 2/16/87>> */
/* < ../../src/typ2.c */

#ident	"@(#)sdb:cfront/scratch/src/typ2..c	1.2"
char *_new(); char _delete(); char *_vec_new(); char _vec_delete();

#line 1 "../../src/typ2.c"

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

#line 19 "../../src/typ2.c"
extern int chars_in_largest ;

#line 21 "../../src/typ2.c"
char typ_init ()
#line 22 "../../src/typ2.c"
{ 
#line 22 "../../src/typ2.c"
struct ptr *_au0__Xthis__ctor_ptr ;

#line 23 "../../src/typ2.c"
chars_in_largest = strlen ( (char *)LARGEST_INT ) ;

#line 25 "../../src/typ2.c"
defa_type = (int_type = (struct basetype *)_basetype__ctor ( (struct basetype *)0 , (unsigned char )21 , (struct name *)0 ) );
int_type -> _node_permanent = 1 ;

#line 28 "../../src/typ2.c"
moe_type = (struct basetype *)_basetype__ctor ( (struct basetype *)0 , (unsigned char )21 , (struct name *)0 ) ;
moe_type -> _node_permanent = 1 ;
moe_type -> _basetype_b_const = 1 ;
_basetype_check ( moe_type , (struct name *)0 ) ;

#line 33 "../../src/typ2.c"
uint_type = (struct basetype *)_basetype__ctor ( (struct basetype *)0 , (unsigned char )21 , (struct name *)0 ) ;
uint_type -> _node_permanent = 1 ;
_basetype_type_adj ( uint_type , (unsigned char )37 ) ;
_basetype_check ( uint_type , (struct name *)0 ) ;

#line 38 "../../src/typ2.c"
long_type = (struct basetype *)_basetype__ctor ( (struct basetype *)0 , (unsigned char )22 , (struct name *)0 ) ;
long_type -> _node_permanent = 1 ;
_basetype_check ( long_type , (struct name *)0 ) ;

#line 42 "../../src/typ2.c"
ulong_type = (struct basetype *)_basetype__ctor ( (struct basetype *)0 , (unsigned char )22 , (struct name *)0 ) ;
ulong_type -> _node_permanent = 1 ;
_basetype_type_adj ( ulong_type , (unsigned char )37 ) ;
_basetype_check ( ulong_type , (struct name *)0 ) ;

#line 47 "../../src/typ2.c"
short_type = (struct basetype *)_basetype__ctor ( (struct basetype *)0 , (unsigned char )29 , (struct name *)0 ) ;
short_type -> _node_permanent = 1 ;
_basetype_check ( short_type , (struct name *)0 ) ;

#line 51 "../../src/typ2.c"
ushort_type = (struct basetype *)_basetype__ctor ( (struct basetype *)0 , (unsigned char )29 , (struct name *)0 ) ;
ushort_type -> _node_permanent = 1 ;
_basetype_type_adj ( ushort_type , (unsigned char )37 ) ;
_basetype_check ( ushort_type , (struct name *)0 ) ;

#line 56 "../../src/typ2.c"
float_type = (struct basetype *)_basetype__ctor ( (struct basetype *)0 , (unsigned char )15 , (struct name *)0 ) ;
float_type -> _node_permanent = 1 ;

#line 59 "../../src/typ2.c"
double_type = (struct basetype *)_basetype__ctor ( (struct basetype *)0 , (unsigned char )11 , (struct name *)0 ) ;
double_type -> _node_permanent = 1 ;

#line 62 "../../src/typ2.c"
zero_type = (struct basetype *)_basetype__ctor ( (struct basetype *)0 , (unsigned char )138 , (struct name *)0 ) ;
zero_type -> _node_permanent = 1 ;
zero -> _expr__O2.__C2_tp = (struct type *)zero_type ;

#line 66 "../../src/typ2.c"
void_type = (struct basetype *)_basetype__ctor ( (struct basetype *)0 , (unsigned char )38 , (struct name *)0 ) ;
void_type -> _node_permanent = 1 ;

#line 69 "../../src/typ2.c"
char_type = (struct basetype *)_basetype__ctor ( (struct basetype *)0 , (unsigned char )5 , (struct name *)0 ) ;
char_type -> _node_permanent = 1 ;

#line 72 "../../src/typ2.c"
uchar_type = (struct basetype *)_basetype__ctor ( (struct basetype *)0 , (unsigned char )5 , (struct name *)0 ) ;
uchar_type -> _node_permanent = 1 ;
_basetype_type_adj ( uchar_type , (unsigned char )37 ) ;
_basetype_check ( uchar_type , (struct name *)0 ) ;

#line 77 "../../src/typ2.c"
Pchar_type = (struct type *)( (_au0__Xthis__ctor_ptr = 0 ), ( (_au0__Xthis__ctor_ptr = (struct ptr *)_new ( (long )(sizeof (struct ptr))) ), (
#line 77 "../../src/typ2.c"
(Nt ++ ), ( (_au0__Xthis__ctor_ptr -> _node_base = ((unsigned char )125 )), ( (_au0__Xthis__ctor_ptr -> _pvtyp_typ = ((struct type *)char_type )), (
#line 77 "../../src/typ2.c"
(_au0__Xthis__ctor_ptr -> _ptr_rdo = ((char )0 )), ((_au0__Xthis__ctor_ptr ))) ) ) ) ) ) ;
Pchar_type -> _node_permanent = 1 ;

#line 80 "../../src/typ2.c"
Pint_type = (struct type *)( (_au0__Xthis__ctor_ptr = 0 ), ( (_au0__Xthis__ctor_ptr = (struct ptr *)_new ( (long )(sizeof (struct ptr))) ), (
#line 80 "../../src/typ2.c"
(Nt ++ ), ( (_au0__Xthis__ctor_ptr -> _node_base = ((unsigned char )125 )), ( (_au0__Xthis__ctor_ptr -> _pvtyp_typ = ((struct type *)int_type )), (
#line 80 "../../src/typ2.c"
(_au0__Xthis__ctor_ptr -> _ptr_rdo = ((char )0 )), ((_au0__Xthis__ctor_ptr ))) ) ) ) ) ) ;
Pint_type -> _node_permanent = 1 ;

#line 83 "../../src/typ2.c"
Pvoid_type = (struct type *)( (_au0__Xthis__ctor_ptr = 0 ), ( (_au0__Xthis__ctor_ptr = (struct ptr *)_new ( (long )(sizeof (struct ptr))) ), (
#line 83 "../../src/typ2.c"
(Nt ++ ), ( (_au0__Xthis__ctor_ptr -> _node_base = ((unsigned char )125 )), ( (_au0__Xthis__ctor_ptr -> _pvtyp_typ = ((struct type *)void_type )), (
#line 83 "../../src/typ2.c"
(_au0__Xthis__ctor_ptr -> _ptr_rdo = ((char )0 )), ((_au0__Xthis__ctor_ptr ))) ) ) ) ) ) ;
Pvoid_type -> _node_permanent = 1 ;

#line 86 "../../src/typ2.c"
Pfctchar_type = (struct type *)_fct__ctor ( (struct fct *)0 , (struct type *)char_type , (struct name *)0 , (unsigned char )0 ) ;
Pfctchar_type = (struct type *)( (_au0__Xthis__ctor_ptr = 0 ), ( (_au0__Xthis__ctor_ptr = (struct ptr *)_new ( (long )(sizeof (struct ptr))) ), (
#line 87 "../../src/typ2.c"
(Nt ++ ), ( (_au0__Xthis__ctor_ptr -> _node_base = ((unsigned char )125 )), ( (_au0__Xthis__ctor_ptr -> _pvtyp_typ = Pfctchar_type ), ( (_au0__Xthis__ctor_ptr ->
#line 87 "../../src/typ2.c"
_ptr_rdo = ((char )0 )), ((_au0__Xthis__ctor_ptr ))) ) ) ) ) ) ;
Pfctchar_type -> _node_permanent = 1 ;

#line 90 "../../src/typ2.c"
Pfctvec_type = (struct type *)_fct__ctor ( (struct fct *)0 , (struct type *)int_type , (struct name *)0 , (unsigned char )0 ) ;
Pfctvec_type = (struct type *)( (_au0__Xthis__ctor_ptr = 0 ), ( (_au0__Xthis__ctor_ptr = (struct ptr *)_new ( (long )(sizeof (struct ptr))) ), (
#line 91 "../../src/typ2.c"
(Nt ++ ), ( (_au0__Xthis__ctor_ptr -> _node_base = ((unsigned char )125 )), ( (_au0__Xthis__ctor_ptr -> _pvtyp_typ = Pfctvec_type ), ( (_au0__Xthis__ctor_ptr ->
#line 91 "../../src/typ2.c"
_ptr_rdo = ((char )0 )), ((_au0__Xthis__ctor_ptr ))) ) ) ) ) ) ;
Pfctvec_type = (struct type *)( (_au0__Xthis__ctor_ptr = 0 ), ( (_au0__Xthis__ctor_ptr = (struct ptr *)_new ( (long )(sizeof (struct ptr))) ), (
#line 92 "../../src/typ2.c"
(Nt ++ ), ( (_au0__Xthis__ctor_ptr -> _node_base = ((unsigned char )125 )), ( (_au0__Xthis__ctor_ptr -> _pvtyp_typ = Pfctvec_type ), ( (_au0__Xthis__ctor_ptr ->
#line 92 "../../src/typ2.c"
_ptr_rdo = ((char )0 )), ((_au0__Xthis__ctor_ptr ))) ) ) ) ) ) ;
Pfctvec_type -> _node_permanent = 1 ;

#line 95 "../../src/typ2.c"
any_tbl = (struct table *)_table__ctor ( (struct table *)0 , (short )20 , (struct table *)0 , (struct name *)0 ) ;
gtbl = (struct table *)_table__ctor ( (struct table *)0 , (short )257 , (struct table *)0 , (struct name *)0 ) ;
gtbl -> _table_t_name = (struct name *)_name__ctor ( (struct name *)0 , "global") ;
}
;
Pbase _basetype_arit_conv (_au0_this , _au0_t )
#line 362 "../../src/cfront.h"
struct basetype *_au0_this ;

#line 100 "../../src/typ2.c"
Pbase _au0_t ;

#line 107 "../../src/typ2.c"
{ 
#line 112 "../../src/typ2.c"
bit _au1_l ;
bit _au1_u ;
bit _au1_f ;
bit _au1_l1 ;
bit _au1_u1 ;
bit _au1_f1 ;

#line 109 "../../src/typ2.c"
while (_au0_this -> _node_base == 97 )_au0_this = (((struct basetype *)(((struct basetype *)_au0_this ))-> _basetype_b_name -> _expr__O2.__C2_tp ));
while (_au0_t && (_au0_t -> _node_base == 97 ))_au0_t = (((struct basetype *)(((struct basetype *)_au0_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ));

#line 112 "../../src/typ2.c"
;

#line 112 "../../src/typ2.c"
;

#line 112 "../../src/typ2.c"
;

#line 112 "../../src/typ2.c"
_au1_l1 = (_au0_this -> _node_base == 22 );

#line 112 "../../src/typ2.c"
_au1_u1 = _au0_this -> _basetype_b_unsigned ;

#line 112 "../../src/typ2.c"
_au1_f1 = ((_au0_this -> _node_base == 15 )|| (_au0_this -> _node_base == 11 ));

#line 118 "../../src/typ2.c"
if (_au0_t ){ 
#line 119 "../../src/typ2.c"
bit _au2_l2 ;
bit _au2_u2 ;
bit _au2_f2 ;

#line 119 "../../src/typ2.c"
_au2_l2 = (_au0_t -> _node_base == 22 );
_au2_u2 = _au0_t -> _basetype_b_unsigned ;
_au2_f2 = ((_au0_t -> _node_base == 15 )|| (_au0_t -> _node_base == 11 ));
_au1_l = (_au1_l1 || _au2_l2 );
_au1_u = (_au1_u1 || _au2_u2 );
_au1_f = (_au1_f1 || _au2_f2 );
}
else { 
#line 127 "../../src/typ2.c"
_au1_l = _au1_l1 ;
_au1_u = _au1_u1 ;
_au1_f = _au1_f1 ;
}

#line 132 "../../src/typ2.c"
if (_au1_f )return double_type ;
if (_au1_l & _au1_u )return ulong_type ;
if (_au1_l & (! _au1_u ))return long_type ;
if (_au1_u )return uint_type ;
return int_type ;
}
;
bit vec_const = 0 ;
bit fct_const = 0 ;

#line 142 "../../src/typ2.c"
bit _type_tconst (_au0_this )
#line 263 "../../src/cfront.h"
struct type *_au0_this ;

#line 146 "../../src/typ2.c"
{ 
#line 147 "../../src/typ2.c"
Ptype _au1_t ;

#line 147 "../../src/typ2.c"
_au1_t = _au0_this ;
vec_const = 0 ;
fct_const = 0 ;
xxx :
#line 151 "../../src/typ2.c"
switch (_au1_t -> _node_base ){ 
#line 152 "../../src/typ2.c"
case 97 : if ((((struct basetype *)_au1_t ))-> _basetype_b_const )return (char )1 ;
_au1_t = (((struct basetype *)_au1_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 153 "../../src/typ2.c"
goto xxx ;
case 110 : vec_const = 1 ;

#line 154 "../../src/typ2.c"
return (char )1 ;
case 125 : 
#line 156 "../../src/typ2.c"
case 158 : return (((struct ptr *)_au1_t ))-> _ptr_rdo ;
case 108 : 
#line 158 "../../src/typ2.c"
case 76 : fct_const = 1 ;

#line 158 "../../src/typ2.c"
return (char )1 ;
default : return (((struct basetype *)_au1_t ))-> _basetype_b_const ;
}
}
;
TOK _type_set_const (_au0_this , _au0_mode )
#line 263 "../../src/cfront.h"
struct type *_au0_this ;

#line 163 "../../src/typ2.c"
bit _au0_mode ;

#line 167 "../../src/typ2.c"
{ 
#line 168 "../../src/typ2.c"
Ptype _au1_t ;

#line 168 "../../src/typ2.c"
_au1_t = _au0_this ;
xxx :
#line 170 "../../src/typ2.c"
switch (_au1_t -> _node_base ){ 
#line 171 "../../src/typ2.c"
case 97 : (((struct basetype *)_au1_t ))-> _basetype_b_const = _au0_mode ;
_au1_t = (((struct basetype *)_au1_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 172 "../../src/typ2.c"
goto xxx ;
case 141 : 
#line 174 "../../src/typ2.c"
case 158 : 
#line 175 "../../src/typ2.c"
case 110 : return _au1_t -> _node_base ;
case 125 : (((struct ptr *)_au1_t ))-> _ptr_rdo = _au0_mode ;

#line 176 "../../src/typ2.c"
return (unsigned char )0 ;
default : (((struct basetype *)_au1_t ))-> _basetype_b_const = _au0_mode ;

#line 177 "../../src/typ2.c"
return (unsigned char )0 ;
}
}
;
int _type_is_ref (_au0_this )
#line 263 "../../src/cfront.h"
struct type *_au0_this ;

#line 182 "../../src/typ2.c"
{ 
#line 183 "../../src/typ2.c"
Ptype _au1_t ;

#line 183 "../../src/typ2.c"
_au1_t = _au0_this ;
xxx :
#line 185 "../../src/typ2.c"
switch (_au1_t -> _node_base ){ 
#line 186 "../../src/typ2.c"
case 97 : _au1_t = (((struct basetype *)_au1_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 186 "../../src/typ2.c"
goto xxx ;
case 158 : return 1 ;
default : return (int )0 ;
}
}
;
int _type_align (_au0_this )
#line 263 "../../src/cfront.h"
struct type *_au0_this ;

#line 193 "../../src/typ2.c"
{ 
#line 194 "../../src/typ2.c"
Ptype _au1_t ;

#line 194 "../../src/typ2.c"
_au1_t = _au0_this ;
xx :
#line 197 "../../src/typ2.c"
switch (_au1_t -> _node_base ){ 
#line 198 "../../src/typ2.c"
case 97 : _au1_t = (((struct basetype *)_au1_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 198 "../../src/typ2.c"
goto xx ;
case 119 : _au1_t = (((struct basetype *)_au1_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 199 "../../src/typ2.c"
goto xx ;
case 110 : _au1_t = (((struct vec *)_au1_t ))-> _pvtyp_typ ;

#line 200 "../../src/typ2.c"
goto xx ;
case 141 : return 1 ;
case 5 : return AL_CHAR ;
case 29 : return AL_SHORT ;
case 21 : return AL_INT ;
case 22 : return AL_LONG ;
case 15 : return AL_FLOAT ;
case 11 : return AL_DOUBLE ;
case 125 : 
#line 209 "../../src/typ2.c"
case 158 : return AL_WPTR ;
case 6 : return (int )(((struct classdef *)_au1_t ))-> _classdef_obj_align ;
case 13 : 
#line 212 "../../src/typ2.c"
case 121 : return AL_INT ;
case 38 : error ( (char *)"illegal use of void", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 213 "../../src/typ2.c"
return AL_INT ;
default : { 
#line 216 "../../src/typ2.c"
struct ea _au0__V10 ;

#line 216 "../../src/typ2.c"
struct ea _au0__V11 ;

#line 214 "../../src/typ2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"(%d,%k)->type::align", (struct ea *)( ( ((& _au0__V10 )-> _ea__O1.__C1_p = ((char *)_au1_t )), (((&
#line 214 "../../src/typ2.c"
_au0__V10 )))) ) , (struct ea *)( ( ((& _au0__V11 )-> _ea__O1.__C1_i = ((int )_au1_t -> _node_base )), (((& _au0__V11 ))))
#line 214 "../../src/typ2.c"
) , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}
;
bit fake_sizeof ;

#line 220 "../../src/typ2.c"
int _type_tsizeof (_au0_this )
#line 263 "../../src/cfront.h"
struct type *_au0_this ;

#line 225 "../../src/typ2.c"
{ 
#line 226 "../../src/typ2.c"
Ptype _au1_t ;

#line 226 "../../src/typ2.c"
_au1_t = _au0_this ;
zx :
#line 228 "../../src/typ2.c"
if (_au1_t == 0 )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"typ.tsizeof(t==0)", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 228 "../../src/typ2.c"
ea *)ea0 ) ;
switch (_au1_t -> _node_base ){ 
#line 230 "../../src/typ2.c"
case 97 : 
#line 231 "../../src/typ2.c"
case 119 : _au1_t = (((struct basetype *)_au1_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 231 "../../src/typ2.c"
goto zx ;
case 141 : return 1 ;
case 38 : return (int )0 ;
case 138 : return SZ_WPTR ;
case 5 : return SZ_CHAR ;
case 29 : return SZ_SHORT ;
case 21 : return SZ_INT ;
case 22 : return SZ_LONG ;
case 15 : return SZ_FLOAT ;
case 11 : return SZ_DOUBLE ;
case 110 : 
#line 242 "../../src/typ2.c"
{ Pvec _au3_v ;

#line 242 "../../src/typ2.c"
_au3_v = (((struct vec *)_au1_t ));
if (_au3_v -> _vec_size == 0 ){ 
#line 244 "../../src/typ2.c"
if (fake_sizeof == 0 )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"sizeof vector with undeclared dimension", (struct ea *)ea0 , (struct
#line 244 "../../src/typ2.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
return SZ_WPTR ;
}
return (_au3_v -> _vec_size * _type_tsizeof ( _au3_v -> _pvtyp_typ ) );
}
case 125 : 
#line 250 "../../src/typ2.c"
case 158 : 
#line 251 "../../src/typ2.c"
_au1_t = (((struct ptr *)_au1_t ))-> _pvtyp_typ ;
xxx :
#line 253 "../../src/typ2.c"
switch (_au1_t -> _node_base ){ 
#line 254 "../../src/typ2.c"
default : return SZ_WPTR ;
case 5 : return SZ_BPTR ;
case 97 : _au1_t = (((struct basetype *)_au1_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 256 "../../src/typ2.c"
goto xxx ;
}
case 114 : 
#line 259 "../../src/typ2.c"
{ Pbase _au3_b ;

#line 259 "../../src/typ2.c"
_au3_b = (((struct basetype *)_au1_t ));
return ((_au3_b -> _basetype_b_bits / BI_IN_BYTE )+ 1 );
}
case 6 : 
#line 263 "../../src/typ2.c"
{ Pclass _au3_cl ;
int _au3_sz ;

#line 263 "../../src/typ2.c"
_au3_cl = (((struct classdef *)_au1_t ));
_au3_sz = _au3_cl -> _classdef_obj_size ;
if ((_au3_cl -> _type_defined & 3)== 0 ){ 
#line 266 "../../src/typ2.c"
{ 
#line 275 "../../src/typ2.c"
struct ea _au0__V12 ;

#line 266 "../../src/typ2.c"
error ( (char *)"%sU, size not known", (struct ea *)( ( ((& _au0__V12 )-> _ea__O1.__C1_p = ((char *)_au3_cl -> _classdef_string )), (((& _au0__V12 ))))
#line 266 "../../src/typ2.c"
) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
return SZ_INT ;
} }
return _au3_sz ;
}
case 121 : 
#line 272 "../../src/typ2.c"
case 13 : return SZ_INT ;
default : { 
#line 275 "../../src/typ2.c"
struct ea _au0__V13 ;

#line 273 "../../src/typ2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"sizeof(%d)", (struct ea *)( ( ((& _au0__V13 )-> _ea__O1.__C1_i = ((int )_au1_t -> _node_base )),
#line 273 "../../src/typ2.c"
(((& _au0__V13 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}
;
bit _type_vec_type (_au0_this )
#line 263 "../../src/cfront.h"
struct type *_au0_this ;

#line 278 "../../src/typ2.c"
{ 
#line 279 "../../src/typ2.c"
Ptype _au1_t ;

#line 279 "../../src/typ2.c"
_au1_t = _au0_this ;
xx :
#line 281 "../../src/typ2.c"
switch (_au1_t -> _node_base ){ 
#line 282 "../../src/typ2.c"
case 141 : 
#line 283 "../../src/typ2.c"
case 110 : 
#line 284 "../../src/typ2.c"
case 125 : 
#line 285 "../../src/typ2.c"
case 158 : return (char )1 ;
#line 285 "../../src/typ2.c"

#line 286 "../../src/typ2.c"
case 97 : _au1_t = (((struct basetype *)_au1_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 286 "../../src/typ2.c"
goto xx ;
default : return (char )0 ;
}
}
;
Ptype _type_deref (_au0_this )
#line 263 "../../src/cfront.h"
struct type *_au0_this ;

#line 295 "../../src/typ2.c"
{ 
#line 296 "../../src/typ2.c"
Ptype _au1_t ;

#line 296 "../../src/typ2.c"
_au1_t = _au0_this ;
xx :
#line 298 "../../src/typ2.c"
switch (_au1_t -> _node_base ){ 
#line 299 "../../src/typ2.c"
case 97 : 
#line 300 "../../src/typ2.c"
_au1_t = (((struct basetype *)_au1_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
goto xx ;
case 125 : 
#line 303 "../../src/typ2.c"
case 158 : 
#line 304 "../../src/typ2.c"
case 110 : 
#line 305 "../../src/typ2.c"
if (_au1_t == Pvoid_type )error ( (char *)"void* deRd", (struct ea *)ea0 , (struct
#line 305 "../../src/typ2.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
return (((struct vec *)_au1_t ))-> _pvtyp_typ ;
case 141 : 
#line 308 "../../src/typ2.c"
return _au1_t ;
default : 
#line 310 "../../src/typ2.c"
error ( (char *)"nonP deRd", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 310 "../../src/typ2.c"

#line 311 "../../src/typ2.c"
return (struct type *)any_type ;
}
}
;

/* the end */
