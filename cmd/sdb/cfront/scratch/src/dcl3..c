/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#line 1 "../../src/dcl3.c"

/* <<cfront 1.2.1 2/16/87>> */
/* < ../../src/dcl3.c */

#ident	"@(#)sdb:cfront/scratch/src/dcl3..c	1.2"
char *_new(); char _delete(); char *_vec_new(); char _vec_delete();

#line 1 "../../src/dcl3.c"

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

#line 20 "../../src/dcl3.c"
extern char make_res (_au0_f )Pfct _au0_f ;

#line 25 "../../src/dcl3.c"
{ 
#line 26 "../../src/dcl3.c"
Pname _au1_rv ;

#line 27 "../../src/dcl3.c"
struct ptr *_au0__Xthis__ctor_ptr ;

#line 26 "../../src/dcl3.c"
_au1_rv = (struct name *)_name__ctor ( (struct name *)0 , "_result") ;
_au1_rv -> _expr__O2.__C2_tp = (struct type *)( (_au0__Xthis__ctor_ptr = 0 ), ( (_au0__Xthis__ctor_ptr = (struct ptr *)_new ( (long )(sizeof (struct ptr))) ),
#line 27 "../../src/dcl3.c"
( (Nt ++ ), ( (_au0__Xthis__ctor_ptr -> _node_base = ((unsigned char )125 )), ( (_au0__Xthis__ctor_ptr -> _pvtyp_typ = _au0_f -> _fct_returns ),
#line 27 "../../src/dcl3.c"
( (_au0__Xthis__ctor_ptr -> _ptr_rdo = ((char )0 )), ((_au0__Xthis__ctor_ptr ))) ) ) ) ) ) ;
_au1_rv -> _name_n_scope = 108 ;
_au1_rv -> _name_n_used = 1 ;
_au1_rv -> _name_n_list = _au0_f -> _fct_argtype ;
if (_au0_f -> _fct_f_this )_au0_f -> _fct_f_this -> _name_n_list = _au1_rv ;
_au0_f -> _fct_f_result = _au1_rv ;
}
;
char _name_check_oper (_au0_this , _au0_cn )
#line 607 "../../src/cfront.h"
struct name *_au0_this ;

#line 35 "../../src/dcl3.c"
Pname _au0_cn ;

#line 39 "../../src/dcl3.c"
{ 
#line 40 "../../src/dcl3.c"
switch (_au0_this -> _name_n_oper ){ 
#line 41 "../../src/dcl3.c"
case 109 : 
#line 42 "../../src/dcl3.c"
if (_au0_cn == 0 )error ( (char *)"operator() must be aM", (struct ea *)ea0 , (struct
#line 42 "../../src/dcl3.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
break ;
case 111 : 
#line 45 "../../src/dcl3.c"
if (_au0_cn == 0 )error ( (char *)"operator[] must be aM", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 45 "../../src/dcl3.c"
ea *)ea0 ) ;
break ;
case 0 : 
#line 48 "../../src/dcl3.c"
case 123 : 
#line 49 "../../src/dcl3.c"
if (_au0_cn && (strcmp ( (char *)_au0_cn -> _expr__O3.__C3_string , (char *)_au0_this -> _expr__O3.__C3_string ) ==
#line 49 "../../src/dcl3.c"
0 )){ 
#line 50 "../../src/dcl3.c"
if (_au0_this -> _expr__O2.__C2_tp -> _node_base == 108 ){ 
#line 51 "../../src/dcl3.c"
Pfct _au4_f ;

#line 51 "../../src/dcl3.c"
_au4_f = (((struct fct *)_au0_this -> _expr__O2.__C2_tp ));
if ((_au4_f -> _fct_returns != (struct type *)defa_type )&& (fct_void == 0 ))
#line 53 "../../src/dcl3.c"
{ 
#line 129 "../../src/dcl3.c"
struct ea _au0__V10 ;

#line 129 "../../src/dcl3.c"
struct ea _au0__V11 ;

#line 53 "../../src/dcl3.c"
error ( (char *)"%s::%s()W returnT", (struct ea *)( ( ((& _au0__V10 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _expr__O3.__C3_string )), (((& _au0__V10 ))))
#line 53 "../../src/dcl3.c"
) , (struct ea *)( ( ((& _au0__V11 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _expr__O3.__C3_string )), (((& _au0__V11 )))) )
#line 53 "../../src/dcl3.c"
, (struct ea *)ea0 , (struct ea *)ea0 ) ;
} _au4_f -> _fct_returns = (struct type *)void_type ;
_au0_this -> _expr__O3.__C3_string = "_ctor";
_au0_this -> _name_n_oper = 161 ;
}
else 
#line 59 "../../src/dcl3.c"
{ 
#line 129 "../../src/dcl3.c"
struct ea _au0__V12 ;

#line 129 "../../src/dcl3.c"
struct ea _au0__V13 ;

#line 59 "../../src/dcl3.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"struct%cnM%n", (struct ea *)( ( ((& _au0__V12 )-> _ea__O1.__C1_p = ((char *)_au0_cn )), (((&
#line 59 "../../src/dcl3.c"
_au0__V12 )))) ) , (struct ea *)( ( ((& _au0__V13 )-> _ea__O1.__C1_p = ((char *)_au0_cn )), (((& _au0__V13 )))) )
#line 59 "../../src/dcl3.c"
, (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
else 
#line 62 "../../src/dcl3.c"
_au0_this -> _name_n_oper = 0 ;
break ;
case 162 : 
#line 65 "../../src/dcl3.c"
if (_au0_cn == 0 ){ 
#line 66 "../../src/dcl3.c"
_au0_this -> _name_n_oper = 0 ;
{ 
#line 129 "../../src/dcl3.c"
struct ea _au0__V14 ;

#line 67 "../../src/dcl3.c"
error ( (char *)"destructor ~%s() not inC", (struct ea *)( ( ((& _au0__V14 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _expr__O3.__C3_string )), (((& _au0__V14 ))))
#line 67 "../../src/dcl3.c"
) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
else if (strcmp ( (char *)_au0_cn -> _expr__O3.__C3_string , (char *)_au0_this -> _expr__O3.__C3_string ) == 0 ){ 
#line 70 "../../src/dcl3.c"
dto :
#line 71 "../../src/dcl3.c"
{ Pfct _au3_f ;
#line 71 "../../src/dcl3.c"
_au3_f = (((struct fct *)_au0_this -> _expr__O2.__C2_tp ));
_au0_this -> _expr__O3.__C3_string = "_dtor";
if (_au0_this -> _expr__O2.__C2_tp -> _node_base != 108 ){ 
#line 74 "../../src/dcl3.c"
{ 
#line 129 "../../src/dcl3.c"
struct ea _au0__V15 ;

#line 129 "../../src/dcl3.c"
struct ea _au0__V16 ;

#line 74 "../../src/dcl3.c"
error ( (char *)"%s::~%s notF", (struct ea *)( ( ((& _au0__V15 )-> _ea__O1.__C1_p = ((char *)_au0_cn -> _expr__O3.__C3_string )), (((& _au0__V15 ))))
#line 74 "../../src/dcl3.c"
) , (struct ea *)( ( ((& _au0__V16 )-> _ea__O1.__C1_p = ((char *)_au0_cn -> _expr__O3.__C3_string )), (((& _au0__V16 )))) )
#line 74 "../../src/dcl3.c"
, (struct ea *)ea0 , (struct ea *)ea0 ) ;
_au0_this -> _expr__O2.__C2_tp = (struct type *)_fct__ctor ( (struct fct *)0 , (struct type *)void_type , (struct name *)0 , (unsigned char )1 ) ;
#line 75 "../../src/dcl3.c"
} }
else 
#line 77 "../../src/dcl3.c"
if (((_au3_f -> _fct_returns != (struct type *)defa_type )&& (_au3_f -> _fct_returns != (struct type *)void_type ))&& (fct_void == 0 ))
#line 78 "../../src/dcl3.c"
{ 
#line 129 "../../src/dcl3.c"
struct
#line 129 "../../src/dcl3.c"
ea _au0__V17 ;

#line 129 "../../src/dcl3.c"
struct ea _au0__V18 ;

#line 78 "../../src/dcl3.c"
error ( (char *)"%s::~%s()W returnT", (struct ea *)( ( ((& _au0__V17 )-> _ea__O1.__C1_p = ((char *)_au0_cn -> _expr__O3.__C3_string )), (((& _au0__V17 ))))
#line 78 "../../src/dcl3.c"
) , (struct ea *)( ( ((& _au0__V18 )-> _ea__O1.__C1_p = ((char *)_au0_cn -> _expr__O3.__C3_string )), (((& _au0__V18 )))) )
#line 78 "../../src/dcl3.c"
, (struct ea *)ea0 , (struct ea *)ea0 ) ;
} if (_au3_f -> _fct_argtype ){ 
#line 80 "../../src/dcl3.c"
if (fct_void == 0 ){ 
#line 129 "../../src/dcl3.c"
struct ea _au0__V19 ;

#line 129 "../../src/dcl3.c"
struct ea _au0__V20 ;

#line 80 "../../src/dcl3.c"
error ( (char *)"%s::~%s()WAs", (struct ea *)( ( ((& _au0__V19 )-> _ea__O1.__C1_p = ((char *)_au0_cn -> _expr__O3.__C3_string )), (((& _au0__V19 ))))
#line 80 "../../src/dcl3.c"
) , (struct ea *)( ( ((& _au0__V20 )-> _ea__O1.__C1_p = ((char *)_au0_cn -> _expr__O3.__C3_string )), (((& _au0__V20 )))) )
#line 80 "../../src/dcl3.c"
, (struct ea *)ea0 , (struct ea *)ea0 ) ;
} _au3_f -> _fct_nargs = 0 ;
_au3_f -> _fct_nargs_known = 1 ;
_au3_f -> _fct_argtype = 0 ;
}
_au3_f -> _fct_returns = (struct type *)void_type ;
}
}
else 
#line 87 "../../src/dcl3.c"
{ 
#line 88 "../../src/dcl3.c"
if (strcmp ( (char *)_au0_this -> _expr__O3.__C3_string , (char *)"_dtor") == 0 )goto dto ;
{ 
#line 129 "../../src/dcl3.c"
struct ea _au0__V21 ;

#line 129 "../../src/dcl3.c"
struct ea _au0__V22 ;

#line 89 "../../src/dcl3.c"
error ( (char *)"~%s in %s", (struct ea *)( ( ((& _au0__V21 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _expr__O3.__C3_string )), (((& _au0__V21 ))))
#line 89 "../../src/dcl3.c"
) , (struct ea *)( ( ((& _au0__V22 )-> _ea__O1.__C1_p = ((char *)_au0_cn -> _expr__O3.__C3_string )), (((& _au0__V22 )))) )
#line 89 "../../src/dcl3.c"
, (struct ea *)ea0 , (struct ea *)ea0 ) ;
_au0_this -> _name_n_oper = 0 ;
} }
break ;
case 97 : 
#line 94 "../../src/dcl3.c"
if (_au0_cn == 0 ){ 
#line 95 "../../src/dcl3.c"
{ 
#line 129 "../../src/dcl3.c"
struct ea _au0__V23 ;

#line 95 "../../src/dcl3.c"
error ( (char *)"operator%t() not aM", (struct ea *)( ( ((& _au0__V23 )-> _ea__O1.__C1_p = ((char *)(((struct type *)_au0_this -> _expr__O4.__C4_n_initializer )))), (((&
#line 95 "../../src/dcl3.c"
_au0__V23 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
_au0_this -> _name_n_oper = 0 ;
_au0_this -> _expr__O4.__C4_n_initializer = 0 ;
} }
else { 
#line 100 "../../src/dcl3.c"
Pfct _au3_f ;
Ptype _au3_tx ;

#line 100 "../../src/dcl3.c"
_au3_f = (((struct fct *)_au0_this -> _expr__O2.__C2_tp ));
_au3_tx = (((struct type *)_au0_this -> _expr__O4.__C4_n_initializer ));

#line 103 "../../src/dcl3.c"
_au0_this -> _expr__O4.__C4_n_initializer = 0 ;
if (_au3_f -> _node_base != 108 ){ 
#line 129 "../../src/dcl3.c"
struct ea _au0__V24 ;

#line 129 "../../src/dcl3.c"
struct ea _au0__V25 ;

#line 104 "../../src/dcl3.c"
error ( (char *)"badT for%n::operator%t()", (struct ea *)( ( ((& _au0__V24 )-> _ea__O1.__C1_p = ((char *)_au0_cn )), (((& _au0__V24 )))) )
#line 104 "../../src/dcl3.c"
, (struct ea *)( ( ((& _au0__V25 )-> _ea__O1.__C1_p = ((char *)_au3_tx )), (((& _au0__V25 )))) ) , (struct
#line 104 "../../src/dcl3.c"
ea *)ea0 , (struct ea *)ea0 ) ;
} if (_au3_f -> _fct_returns != (struct type *)defa_type ){ 
#line 106 "../../src/dcl3.c"
if (_type_check ( _au3_f -> _fct_returns , _au3_tx , (unsigned char )0 )
#line 106 "../../src/dcl3.c"
){ 
#line 129 "../../src/dcl3.c"
struct ea _au0__V26 ;

#line 129 "../../src/dcl3.c"
struct ea _au0__V27 ;

#line 106 "../../src/dcl3.c"
error ( (char *)"bad resultT for%n::operator%t()", (struct ea *)( ( ((& _au0__V26 )-> _ea__O1.__C1_p = ((char *)_au0_cn )), (((& _au0__V26 )))) )
#line 106 "../../src/dcl3.c"
, (struct ea *)( ( ((& _au0__V27 )-> _ea__O1.__C1_p = ((char *)_au3_tx )), (((& _au0__V27 )))) ) , (struct
#line 106 "../../src/dcl3.c"
ea *)ea0 , (struct ea *)ea0 ) ;
} if (_au3_f -> _fct_returns && (_au3_f -> _fct_returns -> _node_permanent == 0 ))_type_del ( _au3_f -> _fct_returns ) ;
}
if (_au3_f -> _fct_argtype ){ 
#line 110 "../../src/dcl3.c"
{ 
#line 129 "../../src/dcl3.c"
struct ea _au0__V28 ;

#line 129 "../../src/dcl3.c"
struct ea _au0__V29 ;

#line 110 "../../src/dcl3.c"
error ( (char *)"%n::operator%t()WAs", (struct ea *)( ( ((& _au0__V28 )-> _ea__O1.__C1_p = ((char *)_au0_cn )), (((& _au0__V28 )))) )
#line 110 "../../src/dcl3.c"
, (struct ea *)( ( ((& _au0__V29 )-> _ea__O1.__C1_p = ((char *)_au3_tx )), (((& _au0__V29 )))) ) , (struct
#line 110 "../../src/dcl3.c"
ea *)ea0 , (struct ea *)ea0 ) ;
_au3_f -> _fct_argtype = 0 ;
} }
_au3_f -> _fct_returns = _au3_tx ;
{ Pname _au3_nx ;

#line 114 "../../src/dcl3.c"
_au3_nx = _type_is_cl_obj ( _au3_tx ) ;
if (_au3_nx && can_coerce ( _au3_tx , _au0_cn -> _expr__O2.__C2_tp ) ){ 
#line 129 "../../src/dcl3.c"
struct ea _au0__V30 ;

#line 129 "../../src/dcl3.c"
struct ea _au0__V31 ;

#line 129 "../../src/dcl3.c"
struct ea _au0__V32 ;

#line 129 "../../src/dcl3.c"
struct ea _au0__V33 ;

#line 115 "../../src/dcl3.c"
error ( (char *)"both %n::%n(%n) and %n::operator%t()", (struct ea *)( ( ((& _au0__V30 )-> _ea__O1.__C1_p = ((char *)_au0_cn )), (((& _au0__V30 )))) )
#line 115 "../../src/dcl3.c"
, (struct ea *)( ( ((& _au0__V31 )-> _ea__O1.__C1_p = ((char *)_au0_cn )), (((& _au0__V31 )))) ) , (struct
#line 115 "../../src/dcl3.c"
ea *)( ( ((& _au0__V32 )-> _ea__O1.__C1_p = ((char *)_au3_nx )), (((& _au0__V32 )))) ) , (struct ea *)( (
#line 115 "../../src/dcl3.c"
((& _au0__V33 )-> _ea__O1.__C1_p = ((char *)_au3_tx )), (((& _au0__V33 )))) ) ) ;
} { char _au3_buf [256];
char *_au3_bb ;
int _au3_l2 ;

#line 117 "../../src/dcl3.c"
_au3_bb = _type_signature ( _au3_tx , _au3_buf ) ;
_au3_l2 = (_au3_bb - _au3_buf );
if ((*_au3_bb )!= 0 )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'d' , (char *)"impossible", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 119 "../../src/dcl3.c"
ea *)ea0 ) ;
if (255 < _au3_l2 )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"N::check_oper():N buffer overflow", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 120 "../../src/dcl3.c"
ea *)ea0 ) ;
{ char *_au3_p ;

#line 121 "../../src/dcl3.c"
_au3_p = (((char *)_new ( (long )((sizeof (char ))* (_au3_l2 + 3 ))) ));
(_au3_p [0 ])= '_' ;
(_au3_p [1 ])= 'O' ;
strcpy ( _au3_p + 2 , (char *)_au3_buf ) ;
_au0_this -> _expr__O3.__C3_string = _au3_p ;
}
}
}
}

#line 127 "../../src/dcl3.c"
break ;
}
}
;

#line 132 "../../src/dcl3.c"
int inline_restr = 0 ;

#line 136 "../../src/dcl3.c"
char _fct_dcl (_au0_this , _au0_n )
#line 400 "../../src/cfront.h"
struct fct *_au0_this ;

#line 136 "../../src/dcl3.c"
Pname _au0_n ;
{ 
#line 138 "../../src/dcl3.c"
int _au1_nmem ;
Pname _au1_a ;
Pname _au1_ll ;
Ptable _au1_ftbl ;

#line 143 "../../src/dcl3.c"
Pptr _au1_cct ;
int _au1_const_old ;

#line 146 "../../src/dcl3.c"
int _au1_bit_old ;
int _au1_byte_old ;
int _au1_max_old ;
int _au1_stack_old ;

#line 186 "../../src/dcl3.c"
Pname _au1_ax ;

#line 187 "../../src/dcl3.c"
struct name_list *_au0__Xthis__ctor_name_list ;

#line 138 "../../src/dcl3.c"
_au1_nmem = 20 ;

#line 143 "../../src/dcl3.c"
_au1_cct = 0 ;
_au1_const_old = const_save ;

#line 146 "../../src/dcl3.c"
_au1_bit_old = bit_offset ;
_au1_byte_old = byte_offset ;
_au1_max_old = max_align ;
_au1_stack_old = stack_size ;

#line 151 "../../src/dcl3.c"
if (_au0_this -> _node_base != 108 ){ 
#line 303 "../../src/dcl3.c"
struct ea _au0__V34 ;

#line 151 "../../src/dcl3.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"F::dcl(%d)", (struct ea *)( ( ((& _au0__V34 )-> _ea__O1.__C1_i = ((int )_au0_this -> _node_base )),
#line 151 "../../src/dcl3.c"
(((& _au0__V34 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} if (_au0_this -> _fct_body == 0 ){ 
#line 303 "../../src/dcl3.c"
struct ea _au0__V35 ;

#line 152 "../../src/dcl3.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"F::dcl(body=%d)", (struct ea *)( ( ((& _au0__V35 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _fct_body )),
#line 152 "../../src/dcl3.c"
(((& _au0__V35 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} if ((_au0_n == 0 )|| (_au0_n -> _node_base != 85 )){ 
#line 303 "../../src/dcl3.c"
struct ea _au0__V36 ;

#line 303 "../../src/dcl3.c"
struct ea _au0__V37 ;

#line 153 "../../src/dcl3.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"F::dcl(N=%d %d)", (struct ea *)( ( ((& _au0__V36 )-> _ea__O1.__C1_p = ((char *)_au0_n )), (((&
#line 153 "../../src/dcl3.c"
_au0__V36 )))) ) , (struct ea *)( ( ((& _au0__V37 )-> _ea__O1.__C1_i = ((int )(_au0_n ? (((unsigned int )_au0_n ->
#line 153 "../../src/dcl3.c"
_node_base )): (((unsigned int )0 ))))), (((& _au0__V37 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} 
#line 155 "../../src/dcl3.c"
if (_au0_this -> _fct_body -> _stmt_memtbl == 0 )_au0_this -> _fct_body -> _stmt_memtbl = (struct table *)_table__ctor ( (struct table *)0 , (short )(_au1_nmem +
#line 155 "../../src/dcl3.c"
3 ), _au0_n -> _expr__O5.__C5_n_table , (struct name *)0 ) ;
_au1_ftbl = _au0_this -> _fct_body -> _stmt_memtbl ;
_au0_this -> _fct_body -> _stmt__O8.__C8_own_tbl = 1 ;
_au1_ftbl -> _table_real_block = (struct stmt *)_au0_this -> _fct_body ;

#line 160 "../../src/dcl3.c"
max_align = 0 ;
stack_size = (byte_offset = 0 );
bit_offset = 0 ;

#line 164 "../../src/dcl3.c"
( (cc ++ ), ((*cc )= (*(cc - 1 )))) ;
cc -> _dcl_context_nof = _au0_n ;
cc -> _dcl_context_ftbl = _au1_ftbl ;

#line 168 "../../src/dcl3.c"
switch (_au0_n -> _name_n_scope ){ 
#line 169 "../../src/dcl3.c"
case 0 : 
#line 170 "../../src/dcl3.c"
case 25 : 
#line 171 "../../src/dcl3.c"
cc -> _dcl_context_not = _au0_n -> _expr__O5.__C5_n_table -> _table_t_name ;
cc -> _dcl_context_cot = (((struct classdef *)cc -> _dcl_context_not -> _expr__O2.__C2_tp ));
cc -> _dcl_context_tot = cc -> _dcl_context_cot -> _classdef_this_type ;
if ((_au0_this -> _fct_f_this == 0 )|| (cc -> _dcl_context_tot == 0 )){ 
#line 303 "../../src/dcl3.c"
struct ea _au0__V38 ;

#line 303 "../../src/dcl3.c"
struct ea _au0__V39 ;

#line 303 "../../src/dcl3.c"
struct ea _au0__V40 ;

#line 174 "../../src/dcl3.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"F::dcl(%n): f_this=%d cc->tot=%d", (struct ea *)( ( ((& _au0__V38 )-> _ea__O1.__C1_p = ((char *)_au0_n )), (((&
#line 174 "../../src/dcl3.c"
_au0__V38 )))) ) , (struct ea *)( ( ((& _au0__V39 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _fct_f_this )), (((& _au0__V39 ))))
#line 174 "../../src/dcl3.c"
) , (struct ea *)( ( ((& _au0__V40 )-> _ea__O1.__C1_p = ((char *)cc -> _dcl_context_tot )), (((& _au0__V40 )))) )
#line 174 "../../src/dcl3.c"
, (struct ea *)ea0 ) ;
} _au0_this -> _fct_f_this -> _expr__O5.__C5_n_table = _au1_ftbl ;
cc -> _dcl_context_c_this = _au0_this -> _fct_f_this ;
}

#line 179 "../../src/dcl3.c"
if (_au0_this -> _fct_f_result == 0 ){ 
#line 181 "../../src/dcl3.c"
Pname _au2_rcln ;

#line 181 "../../src/dcl3.c"
_au2_rcln = _type_is_cl_obj ( _au0_this -> _fct_returns ) ;
if (_au2_rcln && ( (((struct classdef *)_au2_rcln -> _expr__O2.__C2_tp ))-> _classdef_itor ) )make_res ( _au0_this ) ;
}
if (_au0_this -> _fct_f_result )_au0_this -> _fct_f_result -> _expr__O5.__C5_n_table = _au1_ftbl ;

#line 186 "../../src/dcl3.c"
;
for(( (_au1_a = _au0_this -> _fct_argtype ), (_au1_ll = 0 )) ;_au1_a ;_au1_a = _au1_ax ) { 
#line 188 "../../src/dcl3.c"
_au1_ax = _au1_a -> _name_n_list ;
{ Pname _au2_nn ;

#line 189 "../../src/dcl3.c"
_au2_nn = _name_dcl ( _au1_a , _au1_ftbl , (unsigned char )136 ) ;
_au2_nn -> _name_n_assigned_to = (_au2_nn -> _name_n_used = (_au2_nn -> _name_n_addr_taken = 0 ));
_au2_nn -> _name_n_list = 0 ;
switch (_au1_a -> _expr__O2.__C2_tp -> _node_base ){ 
#line 193 "../../src/dcl3.c"
case 6 : 
#line 194 "../../src/dcl3.c"
case 13 : 
#line 195 "../../src/dcl3.c"
_au1_a -> _name_n_list = dcl_list ;
dcl_list = _au1_a ;
break ;
default : 
#line 199 "../../src/dcl3.c"
if (_au1_ll )
#line 200 "../../src/dcl3.c"
_au1_ll -> _name_n_list = _au2_nn ;
else 
#line 202 "../../src/dcl3.c"
_au0_this -> _fct_argtype = _au2_nn ;
_au1_ll = _au2_nn ;
_name__dtor ( _au1_a , 1) ;
}
}
}

#line 209 "../../src/dcl3.c"
if (_au0_this -> _fct_f_result )_au0_this -> _fct_f_result -> _name_n_list = _au0_this -> _fct_argtype ;
if (_au0_this -> _fct_f_this )_au0_this -> _fct_f_this -> _name_n_list = (_au0_this -> _fct_f_result ? _au0_this -> _fct_f_result : _au0_this -> _fct_argtype );

#line 220 "../../src/dcl3.c"
if (_au0_n -> _name_n_oper != 161 ){ 
#line 221 "../../src/dcl3.c"
if (_au0_this -> _fct_f_init )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )0 , (char *)"unexpectedAL: not aK", (struct ea *)ea0 , (struct
#line 221 "../../src/dcl3.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
}
else { 
#line 225 "../../src/dcl3.c"
if (_au0_this -> _fct_f_init ){ 
#line 226 "../../src/dcl3.c"
Pname _au3_bn ;
Ptable _au3_tbl ;
Pexpr _au3_binit ;
Pname _au3_nx ;

#line 231 "../../src/dcl3.c"
Pname _au3_nn ;

#line 226 "../../src/dcl3.c"
_au3_bn = cc -> _dcl_context_cot -> _classdef_clbase ;
_au3_tbl = cc -> _dcl_context_cot -> _classdef_memtbl ;
_au3_binit = 0 ;

#line 230 "../../src/dcl3.c"
const_save = 1 ;
for(_au3_nn = _au0_this -> _fct_f_init ;_au3_nn ;_au3_nn = _au3_nx ) { 
#line 232 "../../src/dcl3.c"
_au3_nx = _au3_nn -> _name_n_list ;
{ Pexpr _au4_i ;
char *_au4_s ;

#line 233 "../../src/dcl3.c"
_au4_i = _au3_nn -> _expr__O4.__C4_n_initializer ;
_au4_s = _au3_nn -> _expr__O3.__C3_string ;

#line 236 "../../src/dcl3.c"
if (_au4_s ){ 
#line 237 "../../src/dcl3.c"
Pname _au5_m ;

#line 237 "../../src/dcl3.c"
_au5_m = _table_look ( _au3_tbl , _au4_s , (unsigned char )0 ) ;
if (_au5_m ){ 
#line 240 "../../src/dcl3.c"
if (_au5_m -> _expr__O5.__C5_n_table == _au3_tbl )
#line 241 "../../src/dcl3.c"
_au3_nn -> _expr__O4.__C4_n_initializer = _fct_mem_init ( _au0_this , _au5_m , _au4_i , _au1_ftbl ) ;
else { 
#line 243 "../../src/dcl3.c"
{ 
#line 303 "../../src/dcl3.c"
struct ea _au0__V41 ;

#line 303 "../../src/dcl3.c"
struct ea _au0__V42 ;

#line 243 "../../src/dcl3.c"
error ( (char *)"%n not inC%n", (struct ea *)( ( ((& _au0__V41 )-> _ea__O1.__C1_p = ((char *)_au5_m )), (((& _au0__V41 )))) )
#line 243 "../../src/dcl3.c"
, (struct ea *)( ( ((& _au0__V42 )-> _ea__O1.__C1_p = ((char *)_au0_n )), (((& _au0__V42 )))) ) , (struct
#line 243 "../../src/dcl3.c"
ea *)ea0 , (struct ea *)ea0 ) ;
_au3_nn -> _expr__O4.__C4_n_initializer = 0 ;
} }
}
else if (_au5_m = _table_look ( ktbl , _au4_s , (unsigned char )0 ) ){ 
#line 249 "../../src/dcl3.c"
_au3_binit = _fct_base_init ( _au0_this ,
#line 249 "../../src/dcl3.c"
_au3_bn , _au4_i , _au1_ftbl ) ;
_au3_nn -> _expr__O4.__C4_n_initializer = 0 ;
}
else { 
#line 253 "../../src/dcl3.c"
{ 
#line 303 "../../src/dcl3.c"
struct ea _au0__V43 ;

#line 303 "../../src/dcl3.c"
struct ea _au0__V44 ;

#line 253 "../../src/dcl3.c"
error ( (char *)"%n not inC%n", (struct ea *)( ( ((& _au0__V43 )-> _ea__O1.__C1_p = ((char *)_au5_m )), (((& _au0__V43 )))) )
#line 253 "../../src/dcl3.c"
, (struct ea *)( ( ((& _au0__V44 )-> _ea__O1.__C1_p = ((char *)_au0_n )), (((& _au0__V44 )))) ) , (struct
#line 253 "../../src/dcl3.c"
ea *)ea0 , (struct ea *)ea0 ) ;
_au3_nn -> _expr__O4.__C4_n_initializer = 0 ;
} }
}
else if (_au3_bn ){ 
#line 259 "../../src/dcl3.c"
_au3_binit = _fct_base_init ( _au0_this , _au3_bn , _au4_i , _au1_ftbl ) ;
_au3_nn -> _expr__O4.__C4_n_initializer = 0 ;
}
else 
#line 263 "../../src/dcl3.c"
error ( (char *)"unexpectedAL: noBC", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
}
}

#line 265 "../../src/dcl3.c"
const_save = _au1_const_old ;
_au0_this -> _fct_b_init = _au3_binit ;
}

#line 269 "../../src/dcl3.c"
if (_au0_this -> _fct_b_init == 0 ){ 
#line 270 "../../src/dcl3.c"
Pname _au3_bn ;

#line 270 "../../src/dcl3.c"
_au3_bn = cc -> _dcl_context_cot -> _classdef_clbase ;
if (_au3_bn && ( _table_look ( (((struct classdef *)_au3_bn -> _expr__O2.__C2_tp ))-> _classdef_memtbl , "_ctor", (unsigned char )0 ) ) )
#line 272 "../../src/dcl3.c"
_au0_this ->
#line 272 "../../src/dcl3.c"
_fct_b_init = _fct_base_init ( _au0_this , _au3_bn , (struct expr *)0 , _au1_ftbl ) ;
}
}

#line 276 "../../src/dcl3.c"
_au0_this -> _fct_returns -> _node_permanent = 1 ;
const_save = (_au0_this -> _fct_f_inline ? 1 : 0);
inline_restr = 0 ;
_block_dcl ( _au0_this -> _fct_body , _au1_ftbl ) ;
if ((_au0_this -> _fct_f_inline && inline_restr )&& (_au0_this -> _fct_returns -> _node_base != 38 )){ 
#line 281 "../../src/dcl3.c"
_au0_this -> _fct_f_inline = 0 ;
{ char *_au2_s ;

#line 282 "../../src/dcl3.c"
_au2_s = ((inline_restr & 8 )? "loop": ((inline_restr & 4 )? "switch": ((inline_restr & 2 )? "goto": ((inline_restr & 1 )? "label": ""))));
#line 282 "../../src/dcl3.c"

#line 287 "../../src/dcl3.c"
{ 
#line 303 "../../src/dcl3.c"
struct ea _au0__V45 ;

#line 303 "../../src/dcl3.c"
struct ea _au0__V46 ;

#line 287 "../../src/dcl3.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"\"inline\" ignored, %n contains %s", (struct ea *)( ( ((& _au0__V45 )-> _ea__O1.__C1_p = ((char *)_au0_n )), (((&
#line 287 "../../src/dcl3.c"
_au0__V45 )))) ) , (struct ea *)( ( ((& _au0__V46 )-> _ea__O1.__C1_p = ((char *)_au2_s )), (((& _au0__V46 )))) )
#line 287 "../../src/dcl3.c"
, (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}

#line 289 "../../src/dcl3.c"
const_save = _au1_const_old ;

#line 291 "../../src/dcl3.c"
if (_au0_this -> _fct_f_inline )isf_list = (struct name_list *)( (_au0__Xthis__ctor_name_list = 0 ), ( (_au0__Xthis__ctor_name_list = (struct name_list *)_new ( (long )(sizeof (struct name_list)))
#line 291 "../../src/dcl3.c"
), ( (_au0__Xthis__ctor_name_list -> _name_list_f = _au0_n ), ( (_au0__Xthis__ctor_name_list -> _name_list_l = isf_list ), ((_au0__Xthis__ctor_name_list ))) ) ) ) ;
#line 291 "../../src/dcl3.c"

#line 293 "../../src/dcl3.c"
_au0_this -> _type_defined |= 01 ;

#line 297 "../../src/dcl3.c"
bit_offset = _au1_bit_old ;
byte_offset = _au1_byte_old ;
max_align = _au1_max_old ;
stack_size = _au1_stack_old ;

#line 302 "../../src/dcl3.c"
( (cc -- )) ;
}
;

#line 306 "../../src/dcl3.c"
Pexpr _fct_base_init (_au0_this , _au0_bn , _au0_i , _au0_ftbl )
#line 400 "../../src/cfront.h"
struct fct *_au0_this ;

#line 306 "../../src/dcl3.c"
Pname _au0_bn ;

#line 306 "../../src/dcl3.c"
Pexpr _au0_i ;

#line 306 "../../src/dcl3.c"
Ptable _au0_ftbl ;

#line 312 "../../src/dcl3.c"
{ 
#line 313 "../../src/dcl3.c"
Pclass _au1_bcl ;
Pname _au1_bnw ;

#line 313 "../../src/dcl3.c"
_au1_bcl = (((struct classdef *)_au0_bn -> _expr__O2.__C2_tp ));
_au1_bnw = ( _table_look ( _au1_bcl -> _classdef_memtbl , "_ctor", (unsigned char )0 ) ) ;

#line 316 "../../src/dcl3.c"
if (_au1_bnw ){ 
#line 317 "../../src/dcl3.c"
Ptype _au2_t ;
Pfct _au2_f ;
Ptype _au2_ty ;
Pexpr _au2_th ;
Pexpr _au2_v ;

#line 322 "../../src/dcl3.c"
struct texpr *_au0__Xthis__ctor_texpr ;

#line 317 "../../src/dcl3.c"
_au2_t = _au1_bnw -> _expr__O2.__C2_tp ;
_au2_f = (((struct fct *)((_au2_t -> _node_base == 108 )? _au2_t : (((struct gen *)_au2_t ))-> _gen_fct_list -> _name_list_f -> _expr__O2.__C2_tp )));
_au2_ty = _au2_f -> _fct_f_this -> _expr__O2.__C2_tp ;
_au2_th = (struct expr *)( (_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr = (struct texpr *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_texpr ),
#line 320 "../../src/dcl3.c"
((unsigned char )113 ), ((struct expr *)_au0_this -> _fct_f_this ), (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 = _au2_ty ),
#line 320 "../../src/dcl3.c"
((_au0__Xthis__ctor_texpr ))) ) ) ;
_au2_v = (struct expr *)( (_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr = (struct texpr *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_texpr ),
#line 321 "../../src/dcl3.c"
((unsigned char )157 ), _au0_i , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 = ((struct type *)_au1_bcl )), ((_au0__Xthis__ctor_texpr )))
#line 321 "../../src/dcl3.c"
) ) ;
_au2_v -> _expr__O4.__C4_e2 = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )111 , _au2_th , (struct expr *)0 ) ;
_au2_v = _expr_typ ( _au2_v , _au0_ftbl ) ;

#line 325 "../../src/dcl3.c"
switch (_au2_v -> _node_base ){ 
#line 326 "../../src/dcl3.c"
case 111 : 
#line 327 "../../src/dcl3.c"
{ Pexpr _au4_vv ;

#line 327 "../../src/dcl3.c"
_au4_vv = _au2_v ;
_au2_v = _au2_v -> _expr__O3.__C3_e1 ;

#line 330 "../../src/dcl3.c"
break ;
}
case 70 : 
#line 333 "../../src/dcl3.c"
_au2_th = (struct expr *)( (_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr = (struct texpr *)_expr__ctor (
#line 333 "../../src/dcl3.c"
((struct expr *)_au0__Xthis__ctor_texpr ), ((unsigned char )113 ), ((struct expr *)_au0_this -> _fct_f_this ), (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_texpr ->
#line 333 "../../src/dcl3.c"
_expr__O5.__C5_tp2 = _au2_ty ), ((_au0__Xthis__ctor_texpr ))) ) ) ;
_au2_v = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )71 , _au2_v , _au2_th ) ;
_au2_v = _expr_typ ( _au2_v , _au0_ftbl ) ;
break ;
default : 
#line 338 "../../src/dcl3.c"
{ 
#line 344 "../../src/dcl3.c"
struct ea _au0__V47 ;

#line 338 "../../src/dcl3.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"F::base_init: unexpected %k", (struct ea *)( ( ((& _au0__V47 )-> _ea__O1.__C1_i = ((int )_au2_v -> _node_base )),
#line 338 "../../src/dcl3.c"
(((& _au0__V47 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
return _au2_v ;
}
else errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )0 , (char *)"unexpectedAL: noBCK", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 )
#line 342 "../../src/dcl3.c"
;
return (struct expr *)0 ;
}
;

#line 347 "../../src/dcl3.c"
Pexpr _fct_mem_init (_au0_this , _au0_member , _au0_i , _au0_ftbl )
#line 400 "../../src/cfront.h"
struct fct *_au0_this ;

#line 347 "../../src/dcl3.c"
Pname _au0_member ;

#line 347 "../../src/dcl3.c"
Pexpr _au0_i ;

#line 347 "../../src/dcl3.c"
Ptable _au0_ftbl ;

#line 351 "../../src/dcl3.c"
{ 
#line 355 "../../src/dcl3.c"
Pname _au1_cn ;

#line 389 "../../src/dcl3.c"
Pref _au1_tn ;

#line 390 "../../src/dcl3.c"
struct ref *_au0__Xthis__ctor_ref ;

#line 353 "../../src/dcl3.c"
if (_au0_member -> _name_n_stclass == 31 ){ 
#line 416 "../../src/dcl3.c"
struct ea _au0__V48 ;

#line 353 "../../src/dcl3.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"MIr for static%n", (struct ea *)( ( ((& _au0__V48 )-> _ea__O1.__C1_p = ((char *)_au0_member )), (((&
#line 353 "../../src/dcl3.c"
_au0__V48 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} if (_au0_i )_au0_i = _expr_typ ( _au0_i , _au0_ftbl ) ;
_au1_cn = _type_is_cl_obj ( _au0_member -> _expr__O2.__C2_tp ) ;
if (_au1_cn ){ 
#line 357 "../../src/dcl3.c"
Pclass _au2_mcl ;
Pname _au2_ctor ;
Pname _au2_icn ;

#line 357 "../../src/dcl3.c"
_au2_mcl = (((struct classdef *)_au1_cn -> _expr__O2.__C2_tp ));
_au2_ctor = ( _table_look ( _au2_mcl -> _classdef_memtbl , "_ctor", (unsigned char )0 ) ) ;

#line 361 "../../src/dcl3.c"
if ((((( _au2_mcl -> _classdef_itor ) == 0 )&& _au0_i )&& (_au2_icn = _type_is_cl_obj ( _au0_i -> _expr__O2.__C2_tp ) ))&& ((((struct classdef *)_au2_icn ->
#line 361 "../../src/dcl3.c"
_expr__O2.__C2_tp ))== _au2_mcl ))
#line 364 "../../src/dcl3.c"
{ 
#line 365 "../../src/dcl3.c"
Pref _au3_tn ;
Pexpr _au3_init ;

#line 367 "../../src/dcl3.c"
struct ref *_au0__Xthis__ctor_ref ;

#line 365 "../../src/dcl3.c"
_au3_tn = (struct ref *)( (_au0__Xthis__ctor_ref = 0 ), ( ( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ref ),
#line 365 "../../src/dcl3.c"
((unsigned char )44 ), ((struct expr *)_au0_this -> _fct_f_this ), (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = _au0_member ),
#line 365 "../../src/dcl3.c"
((_au0__Xthis__ctor_ref ))) ) ) ;
_au3_init = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )70 , (struct expr *)_au3_tn , _au0_i ) ;
return _expr_typ ( _au3_init , _au0_ftbl ) ;
}
else if (_au2_ctor ){ 
#line 370 "../../src/dcl3.c"
Pref _au3_tn ;
Pref _au3_ct ;
Pexpr _au3_c ;

#line 373 "../../src/dcl3.c"
struct ref *_au0__Xthis__ctor_ref ;

#line 370 "../../src/dcl3.c"
_au3_tn = (struct ref *)( (_au0__Xthis__ctor_ref = 0 ), ( ( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ref ),
#line 370 "../../src/dcl3.c"
((unsigned char )44 ), ((struct expr *)_au0_this -> _fct_f_this ), (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = _au0_member ),
#line 370 "../../src/dcl3.c"
((_au0__Xthis__ctor_ref ))) ) ) ;
_au3_ct = (struct ref *)( (_au0__Xthis__ctor_ref = 0 ), ( ( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ref ),
#line 371 "../../src/dcl3.c"
((unsigned char )45 ), ((struct expr *)_au3_tn ), (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = _au2_ctor ), ((_au0__Xthis__ctor_ref )))
#line 371 "../../src/dcl3.c"
) ) ;
_au3_c = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )146 , (struct expr *)_au3_ct , _au0_i ) ;
return _expr_typ ( _au3_c , _au0_ftbl ) ;
}
else { 
#line 376 "../../src/dcl3.c"
{ 
#line 416 "../../src/dcl3.c"
struct ea _au0__V49 ;

#line 376 "../../src/dcl3.c"
error ( (char *)"Ir forM%nW noK", (struct ea *)( ( ((& _au0__V49 )-> _ea__O1.__C1_p = ((char *)_au0_member )), (((& _au0__V49 )))) )
#line 376 "../../src/dcl3.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
return (struct expr *)0 ;
} }
}

#line 381 "../../src/dcl3.c"
if (cl_obj_vec ){ 
#line 382 "../../src/dcl3.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"Ir forCM vectorWK", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ,
#line 382 "../../src/dcl3.c"
(struct ea *)ea0 ) ;
return (struct expr *)0 ;
}

#line 387 "../../src/dcl3.c"
if (_au0_i -> _expr__O4.__C4_e2 ){ 
#line 416 "../../src/dcl3.c"
struct ea _au0__V50 ;

#line 387 "../../src/dcl3.c"
error ( (char *)"Ir for%m not a simpleE", (struct ea *)( ( ((& _au0__V50 )-> _ea__O1.__C1_p = ((char *)_au0_member )), (((& _au0__V50 )))) )
#line 387 "../../src/dcl3.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} _au0_i = _au0_i -> _expr__O3.__C3_e1 ;
_au1_tn = (struct ref *)( (_au0__Xthis__ctor_ref = 0 ), ( ( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ref ),
#line 389 "../../src/dcl3.c"
((unsigned char )44 ), ((struct expr *)_au0_this -> _fct_f_this ), (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = _au0_member ),
#line 389 "../../src/dcl3.c"
((_au0__Xthis__ctor_ref ))) ) ) ;

#line 391 "../../src/dcl3.c"
if (_type_tconst ( _au0_member -> _expr__O2.__C2_tp ) ){ 
#line 392 "../../src/dcl3.c"
TOK _au2_t ;

#line 392 "../../src/dcl3.c"
_au2_t = _type_set_const ( _au0_member -> _expr__O2.__C2_tp , (char )0 ) ;
switch (_au2_t ){ 
#line 394 "../../src/dcl3.c"
case 141 : 
#line 395 "../../src/dcl3.c"
case 110 : 
#line 396 "../../src/dcl3.c"
case 158 : 
#line 397 "../../src/dcl3.c"
{ 
#line 416 "../../src/dcl3.c"
struct ea _au0__V51 ;

#line 397 "../../src/dcl3.c"
error ( (char *)"MIr for%kM%n", (struct ea *)( ( ((& _au0__V51 )-> _ea__O1.__C1_p = ((char *)_au0_member )), (((& _au0__V51 )))) )
#line 397 "../../src/dcl3.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
return (struct expr *)0 ;
} }
_au0_i = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )70 , (struct expr *)_au1_tn , _au0_i ) ;
_au0_i = _expr_typ ( _au0_i , _au0_ftbl ) ;
_type_set_const ( _au0_member -> _expr__O2.__C2_tp , (char )1 ) ;
return _au0_i ;
}

#line 406 "../../src/dcl3.c"
if (_type_is_ref ( _au0_member -> _expr__O2.__C2_tp ) ){ 
#line 407 "../../src/dcl3.c"
_au0_i = ref_init ( ((struct ptr *)_au0_member -> _expr__O2.__C2_tp ), _au0_i , _au0_ftbl ) ;
_au0_i = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )70 , (struct expr *)_au1_tn , _au0_i ) ;
_name_assign ( _au0_member ) ;
return _au0_i ;
}

#line 413 "../../src/dcl3.c"
_au0_i = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )70 , (struct expr *)_au1_tn , _au0_i ) ;
return _expr_typ ( _au0_i , _au0_ftbl ) ;
}
;

#line 418 "../../src/dcl3.c"
extern Pexpr replace_temp (_au0_e , _au0_n )Pexpr _au0_e ;

#line 418 "../../src/dcl3.c"
Pexpr _au0_n ;

#line 431 "../../src/dcl3.c"
{ 
#line 432 "../../src/dcl3.c"
Pexpr _au1_c ;
Pexpr _au1_ff ;
Pexpr _au1_a ;
Pexpr _au1_tmp ;

#line 432 "../../src/dcl3.c"
_au1_c = _au0_e -> _expr__O3.__C3_e1 ;
_au1_ff = _au1_c -> _expr__O3.__C3_e1 ;
_au1_a = _au1_c -> _expr__O4.__C4_e2 ;
_au1_tmp = _au0_e -> _expr__O4.__C4_e2 ;

#line 438 "../../src/dcl3.c"
if (_au1_tmp -> _node_base == 111 )_au1_tmp = _au1_tmp -> _expr__O3.__C3_e1 ;
if (_au1_tmp -> _node_base == 113 )_au1_tmp = _au1_tmp -> _expr__O3.__C3_e1 ;
if ((_au1_tmp -> _node_base == 112 )|| (_au1_tmp -> _node_base == 145 ))_au1_tmp = _au1_tmp -> _expr__O4.__C4_e2 ;
if (_au1_tmp -> _node_base != 85 ){ 
#line 459 "../../src/dcl3.c"
struct ea _au0__V52 ;

#line 441 "../../src/dcl3.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"replace %k", (struct ea *)( ( ((& _au0__V52 )-> _ea__O1.__C1_i = ((int )_au1_tmp -> _node_base )),
#line 441 "../../src/dcl3.c"
(((& _au0__V52 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} _au1_tmp -> _expr__O2.__C2_tp = (struct type *)any_type ;

#line 445 "../../src/dcl3.c"
switch (_au1_ff -> _node_base ){ 
#line 446 "../../src/dcl3.c"
case 44 : 
#line 447 "../../src/dcl3.c"
if ((_au1_ff -> _expr__O3.__C3_e1 -> _node_base == 145 )&& (_au1_ff -> _expr__O3.__C3_e1 -> _expr__O4.__C4_e2 == _au1_tmp ))
#line 448 "../../src/dcl3.c"
_au1_a =
#line 448 "../../src/dcl3.c"
_au1_ff ;
break ;
case 45 : 
#line 451 "../../src/dcl3.c"
if ((_au1_ff -> _expr__O3.__C3_e1 -> _node_base == 85 )&& (_au1_ff -> _expr__O3.__C3_e1 == _au1_tmp )){ 
#line 452 "../../src/dcl3.c"
_au1_a = _au1_ff ;
_au1_a -> _node_base = 44 ;
}
break ;
}
_au1_a -> _expr__O3.__C3_e1 = _au0_n ;
return _au1_c ;
}
;
Pname _classdef_has_ictor (_au0_this )
#line 319 "../../src/cfront.h"
struct classdef *_au0_this ;

#line 465 "../../src/dcl3.c"
{ 
#line 466 "../../src/dcl3.c"
Pname _au1_c ;
Pfct _au1_f ;
Plist _au1_l ;

#line 466 "../../src/dcl3.c"
_au1_c = ( _table_look ( _au0_this -> _classdef_memtbl , "_ctor", (unsigned char )0 ) ) ;

#line 470 "../../src/dcl3.c"
if (_au1_c == 0 )return (struct name *)0 ;

#line 472 "../../src/dcl3.c"
_au1_f = (((struct fct *)_au1_c -> _expr__O2.__C2_tp ));

#line 474 "../../src/dcl3.c"
switch (_au1_f -> _node_base ){ 
#line 475 "../../src/dcl3.c"
default : 
#line 476 "../../src/dcl3.c"
{ 
#line 496 "../../src/dcl3.c"
struct ea _au0__V53 ;

#line 496 "../../src/dcl3.c"
struct ea _au0__V54 ;

#line 476 "../../src/dcl3.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"%s: badK (%k)", (struct ea *)( ( ((& _au0__V53 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _classdef_string )),
#line 476 "../../src/dcl3.c"
(((& _au0__V53 )))) ) , (struct ea *)( ( ((& _au0__V54 )-> _ea__O1.__C1_i = ((int )_au1_c -> _expr__O2.__C2_tp -> _node_base )),
#line 476 "../../src/dcl3.c"
(((& _au0__V54 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 ) ;

#line 478 "../../src/dcl3.c"
case 108 : 
#line 479 "../../src/dcl3.c"
switch (_au1_f -> _fct_nargs ){ 
#line 480 "../../src/dcl3.c"
case 0 : return _au1_c ;
default : if (_au1_f -> _fct_argtype -> _expr__O4.__C4_n_initializer )return _au1_c ;
}
return (struct name *)0 ;

#line 485 "../../src/dcl3.c"
case 76 : 
#line 486 "../../src/dcl3.c"
for(_au1_l = (((struct gen *)_au1_f ))-> _gen_fct_list ;_au1_l ;_au1_l = _au1_l -> _name_list_l ) { 
#line 487 "../../src/dcl3.c"
Pname _au3_n ;

#line 487 "../../src/dcl3.c"
_au3_n = _au1_l -> _name_list_f ;
_au1_f = (((struct fct *)_au3_n -> _expr__O2.__C2_tp ));
switch (_au1_f -> _fct_nargs ){ 
#line 490 "../../src/dcl3.c"
case 0 : return _au3_n ;
default : if (_au1_f -> _fct_argtype -> _expr__O4.__C4_n_initializer )return _au3_n ;
}
}
return (struct name *)0 ;
} }
}
;
struct gen *_gen__ctor (_au0_this , _au0_s )
#line 416 "../../src/cfront.h"
struct gen *_au0_this ;

#line 498 "../../src/dcl3.c"
char *_au0_s ;
{ 
#line 500 "../../src/dcl3.c"
char *_au1_p ;

#line 499 "../../src/dcl3.c"
if (_au0_this == 0 )_au0_this = (struct gen *)_new ( (long )(sizeof (struct gen))) ;
_au1_p = (((char *)_new ( (long )((sizeof (char ))* (strlen ( (char *)_au0_s ) + 1 ))) ));
strcpy ( _au1_p , (char *)_au0_s ) ;
_au0_this -> _gen_string = _au1_p ;
_au0_this -> _node_base = 76 ;
_au0_this -> _gen_fct_list = 0 ;
return _au0_this ;
}
;

#line 507 "../../src/dcl3.c"
Pname _gen_add (_au0_this , _au0_n , _au0_sig )
#line 416 "../../src/cfront.h"
struct gen *_au0_this ;

#line 507 "../../src/dcl3.c"
Pname _au0_n ;

#line 507 "../../src/dcl3.c"
int _au0_sig ;

#line 516 "../../src/dcl3.c"
{ 
#line 517 "../../src/dcl3.c"
Pfct _au1_f ;
Pname _au1_nx ;

#line 517 "../../src/dcl3.c"
_au1_f = (((struct fct *)_au0_n -> _expr__O2.__C2_tp ));

#line 520 "../../src/dcl3.c"
if (_au1_f -> _node_base != 108 ){ 
#line 556 "../../src/dcl3.c"
struct ea _au0__V55 ;

#line 520 "../../src/dcl3.c"
error ( (char *)"%n: overloaded nonF", (struct ea *)( ( ((& _au0__V55 )-> _ea__O1.__C1_p = ((char *)_au0_n )), (((& _au0__V55 )))) )
#line 520 "../../src/dcl3.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} 
#line 522 "../../src/dcl3.c"
if (_au0_this -> _gen_fct_list && (_au1_nx = _gen_find ( _au0_this , _au1_f , (char )1 ) ))
#line 523 "../../src/dcl3.c"
Nold = 1 ;
else { 
#line 525 "../../src/dcl3.c"
char *_au2_s ;

#line 526 "../../src/dcl3.c"
struct name_list *_au0__Xthis__ctor_name_list ;

#line 525 "../../src/dcl3.c"
_au2_s = _au0_this -> _gen_string ;

#line 527 "../../src/dcl3.c"
if ((_au0_this -> _gen_fct_list || _au0_sig )|| _au0_n -> _name_n_oper ){ 
#line 528 "../../src/dcl3.c"
char _au3_buf [256];
char *_au3_bb ;
int _au3_l2 ;

#line 533 "../../src/dcl3.c"
int _au3_l1 ;
char *_au3_p ;

#line 529 "../../src/dcl3.c"
_au3_bb = _type_signature ( _au0_n -> _expr__O2.__C2_tp , _au3_buf ) ;
_au3_l2 = (_au3_bb - _au3_buf );
if ((*_au3_bb )!= 0 )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'d' , (char *)"impossible sig", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 531 "../../src/dcl3.c"
ea *)ea0 ) ;
if (255 < _au3_l2 )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"gen::add():N buffer overflow", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 532 "../../src/dcl3.c"
ea *)ea0 ) ;
_au3_l1 = strlen ( (char *)_au2_s ) ;

#line 533 "../../src/dcl3.c"
_au3_p = (((char *)_new ( (long )((sizeof (char ))* ((_au3_l1 + _au3_l2 )+ 1 ))) ));

#line 535 "../../src/dcl3.c"
strcpy ( _au3_p , (char *)_au2_s ) ;
strcpy ( _au3_p + _au3_l1 , (char *)_au3_buf ) ;
_au0_n -> _expr__O3.__C3_string = _au3_p ;
}
else 
#line 540 "../../src/dcl3.c"
_au0_n -> _expr__O3.__C3_string = _au2_s ;

#line 542 "../../src/dcl3.c"
_au1_nx = (struct name *)_name__ctor ( (struct name *)0 , (char *)0 ) ;
(*_au1_nx )= (*_au0_n );
_au1_nx -> _node_permanent = 1 ;
Nold = 0 ;
if (_au0_this -> _gen_fct_list ){ 
#line 547 "../../src/dcl3.c"
Plist _au3_gl ;

#line 548 "../../src/dcl3.c"
struct name_list *_au0__Xthis__ctor_name_list ;
for(_au3_gl = _au0_this -> _gen_fct_list ;_au3_gl -> _name_list_l ;_au3_gl = _au3_gl -> _name_list_l ) ;

#line 549 "../../src/dcl3.c"
_au3_gl -> _name_list_l = (struct name_list *)( (_au0__Xthis__ctor_name_list = 0 ), ( (_au0__Xthis__ctor_name_list = (struct name_list *)_new ( (long )(sizeof (struct name_list))) ),
#line 549 "../../src/dcl3.c"
( (_au0__Xthis__ctor_name_list -> _name_list_f = _au1_nx ), ( (_au0__Xthis__ctor_name_list -> _name_list_l = ((struct name_list *)0 )), ((_au0__Xthis__ctor_name_list ))) ) ) ) ;
#line 549 "../../src/dcl3.c"
}
else 
#line 552 "../../src/dcl3.c"
_au0_this -> _gen_fct_list = (struct name_list *)( (_au0__Xthis__ctor_name_list = 0 ), ( (_au0__Xthis__ctor_name_list = (struct name_list *)_new ( (long )(sizeof (struct name_list)))
#line 552 "../../src/dcl3.c"
), ( (_au0__Xthis__ctor_name_list -> _name_list_f = _au1_nx ), ( (_au0__Xthis__ctor_name_list -> _name_list_l = ((struct name_list *)0 )), ((_au0__Xthis__ctor_name_list ))) ) ) )
#line 552 "../../src/dcl3.c"
;
_au1_nx -> _name_n_list = 0 ;
}
return _au1_nx ;
}
;
extern bit const_problem ;

#line 560 "../../src/dcl3.c"
Pname _gen_find (_au0_this , _au0_f , _au0_warn )
#line 416 "../../src/cfront.h"
struct gen *_au0_this ;

#line 560 "../../src/dcl3.c"
Pfct _au0_f ;

#line 560 "../../src/dcl3.c"
bit _au0_warn ;
{ 
#line 562 "../../src/dcl3.c"
Plist _au1_gl ;

#line 562 "../../src/dcl3.c"
for(_au1_gl = _au0_this -> _gen_fct_list ;_au1_gl ;_au1_gl = _au1_gl -> _name_list_l ) { 
#line 563 "../../src/dcl3.c"
Pname _au2_nx ;
Pfct _au2_fx ;
Pname _au2_a ;

#line 565 "../../src/dcl3.c"
Pname _au2_ax ;
int _au2_vp ;
int _au2_cp ;
int _au2_op ;
int _au2_xp ;
int _au2_ma ;

#line 576 "../../src/dcl3.c"
int _au2_acnt ;

#line 563 "../../src/dcl3.c"
_au2_nx = _au1_gl -> _name_list_f ;
_au2_fx = (((struct fct *)_au2_nx -> _expr__O2.__C2_tp ));

#line 566 "../../src/dcl3.c"
_au2_vp = 0 ;
_au2_cp = 0 ;
_au2_op = 0 ;
_au2_xp = 0 ;
_au2_ma = 0 ;

#line 572 "../../src/dcl3.c"
if (((_au2_fx -> _fct_nargs_known != _au0_f -> _fct_nargs_known )&& _au2_fx -> _fct_nargs )&& (_au2_fx -> _fct_nargs_known != 155 ))
#line 574 "../../src/dcl3.c"
continue ;

#line 576 "../../src/dcl3.c"
_au2_acnt = ((_au2_fx -> _fct_nargs > _au0_f -> _fct_nargs )? _au2_fx -> _fct_nargs : _au0_f -> _fct_nargs );
for(( (_au2_ax = _au2_fx -> _fct_argtype ), (_au2_a = _au0_f -> _fct_argtype )) ;_au2_a && _au2_ax ;( (_au2_ax = _au2_ax -> _name_n_list ), (_au2_a = _au2_a ->
#line 577 "../../src/dcl3.c"
_name_n_list )) ) 
#line 578 "../../src/dcl3.c"
{ 
#line 580 "../../src/dcl3.c"
Ptype _au3_at ;
Ptype _au3_atp ;

#line 583 "../../src/dcl3.c"
int _au3_atpc ;
int _au3_rr ;

#line 580 "../../src/dcl3.c"
_au3_at = _au2_ax -> _expr__O2.__C2_tp ;
_au3_atp = _au2_a -> _expr__O2.__C2_tp ;

#line 583 "../../src/dcl3.c"
_au3_atpc = 0 ;
_au3_rr = 0 ;
_au2_acnt -- ;

#line 600 "../../src/dcl3.c"
aaa :
#line 601 "../../src/dcl3.c"
switch (_au3_atp -> _node_base ){ 
#line 602 "../../src/dcl3.c"
case 97 : 
#line 603 "../../src/dcl3.c"
if ((((struct basetype *)_au3_atp ))-> _basetype_b_const )_au3_atpc = 1 ;
_au3_atp = (((struct basetype *)_au3_atp ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
goto aaa ;
case 158 : 
#line 607 "../../src/dcl3.c"
if (_au0_warn && (_type_check ( (((struct ptr *)_au3_atp ))-> _pvtyp_typ , _au3_at , (unsigned char )0 ) == 0 ))_au2_op +=
#line 607 "../../src/dcl3.c"
1 ;
_au3_rr = 1 ;
}

#line 611 "../../src/dcl3.c"
atl :
#line 612 "../../src/dcl3.c"
switch (_au3_at -> _node_base ){ 
#line 613 "../../src/dcl3.c"
case 97 : 
#line 614 "../../src/dcl3.c"
if ((((struct basetype *)_au3_at ))-> _basetype_b_const != _au3_atpc )_au2_cp = 1 ;
_au3_at = (((struct basetype *)_au3_at ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
goto atl ;
case 158 : 
#line 618 "../../src/dcl3.c"
if (_au0_warn && (_type_check ( (((struct ptr *)_au3_at ))-> _pvtyp_typ , _au3_atp , (unsigned char )0 ) == 0 ))_au2_op +=
#line 618 "../../src/dcl3.c"
1 ;
_au3_rr = 1 ;
goto defa ;
case 15 : 
#line 622 "../../src/dcl3.c"
case 11 : 
#line 623 "../../src/dcl3.c"
if (_au0_warn ){ 
#line 624 "../../src/dcl3.c"
if (_au3_atp -> _node_base != _au3_at -> _node_base ){ 
#line 625 "../../src/dcl3.c"
switch (_au3_atp -> _node_base ){
#line 625 "../../src/dcl3.c"

#line 626 "../../src/dcl3.c"
case 15 : 
#line 627 "../../src/dcl3.c"
case 11 : _au2_op += 1 ;
}
}
else _au2_xp += 1 ;
}
goto defa ;
case 121 : 
#line 634 "../../src/dcl3.c"
if (_au0_warn ){ 
#line 635 "../../src/dcl3.c"
if (_au3_atp -> _node_base != _au3_at -> _node_base ){ 
#line 636 "../../src/dcl3.c"
switch (_au3_atp -> _node_base ){ 
#line 637 "../../src/dcl3.c"
case 5 :
#line 637 "../../src/dcl3.c"

#line 638 "../../src/dcl3.c"
case 29 : 
#line 639 "../../src/dcl3.c"
case 21 : _au2_op += 1 ;
}
}
else if ((((struct basetype *)_au3_atp ))-> _basetype_b_name -> _expr__O2.__C2_tp != (((struct basetype *)_au3_at ))-> _basetype_b_name -> _expr__O2.__C2_tp )
#line 643 "../../src/dcl3.c"
_au2_op += 1 ;
else _au2_xp += 1 ;
}
goto defa ;
case 5 : 
#line 648 "../../src/dcl3.c"
case 29 : 
#line 649 "../../src/dcl3.c"
case 21 : 
#line 650 "../../src/dcl3.c"
if (_au0_warn ){ 
#line 651 "../../src/dcl3.c"
if (_au3_atp -> _node_base != _au3_at -> _node_base ){ 
#line 652 "../../src/dcl3.c"
switch
#line 652 "../../src/dcl3.c"
(_au3_atp -> _node_base ){ 
#line 653 "../../src/dcl3.c"
case 29 : 
#line 654 "../../src/dcl3.c"
case 21 : 
#line 655 "../../src/dcl3.c"
case 5 : 
#line 656 "../../src/dcl3.c"
case 121 : _au2_op += 1 ;
}
}
else _au2_xp += 1 ;
}

#line 662 "../../src/dcl3.c"
case 22 : 
#line 663 "../../src/dcl3.c"
if (_type_check ( _au3_at , _au3_atp , (unsigned char )0 ) ){ 
#line 664 "../../src/dcl3.c"
if (const_problem )_au2_cp = 1 ;
if (_au2_acnt )
#line 666 "../../src/dcl3.c"
{ _au2_ma = 1 ;

#line 666 "../../src/dcl3.c"
break ;
}
else 
#line 667 "../../src/dcl3.c"
goto xx ;
}
default : 
#line 670 "../../src/dcl3.c"
defa :
#line 671 "../../src/dcl3.c"
if (_type_check ( _au3_at , _au3_atp , (unsigned char )0 ) ){ 
#line 672 "../../src/dcl3.c"
if (const_problem && (_au3_rr == 0 ))_au2_cp =
#line 672 "../../src/dcl3.c"
1 ;
if (_au2_acnt )
#line 674 "../../src/dcl3.c"
{ _au2_ma = 1 ;

#line 674 "../../src/dcl3.c"
break ;
}
else 
#line 675 "../../src/dcl3.c"
goto xx ;
}
if (const_problem )_au2_cp = 1 ;
if (vrp_equiv )_au2_vp = 1 ;
}
}

#line 683 "../../src/dcl3.c"
if (_au2_ma )goto xx ;

#line 685 "../../src/dcl3.c"
if (_au2_ax ){ 
#line 686 "../../src/dcl3.c"
if (_au0_warn && _au2_ax -> _expr__O4.__C4_n_initializer )
#line 687 "../../src/dcl3.c"
{ 
#line 715 "../../src/dcl3.c"
struct ea _au0__V56 ;

#line 687 "../../src/dcl3.c"
error ( (char *)"Ir makes overloaded%n ambiguous", (struct ea *)( ( ((& _au0__V56 )-> _ea__O1.__C1_p = ((char *)_au2_nx )), (((& _au0__V56 )))) )
#line 687 "../../src/dcl3.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} continue ;
}

#line 691 "../../src/dcl3.c"
if (_au2_a ){ 
#line 692 "../../src/dcl3.c"
if (_au0_warn && _au2_a -> _expr__O4.__C4_n_initializer )
#line 693 "../../src/dcl3.c"
{ 
#line 715 "../../src/dcl3.c"
struct ea _au0__V57 ;

#line 693 "../../src/dcl3.c"
error ( (char *)"Ir makes overloaded%n ambiguous", (struct ea *)( ( ((& _au0__V57 )-> _ea__O1.__C1_p = ((char *)_au2_nx )), (((& _au0__V57 )))) )
#line 693 "../../src/dcl3.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} continue ;
}

#line 697 "../../src/dcl3.c"
if (_au0_warn && _type_check ( _au2_fx -> _fct_returns , _au0_f -> _fct_returns , (unsigned char )0 ) )
#line 698 "../../src/dcl3.c"
{ 
#line 715 "../../src/dcl3.c"
struct ea _au0__V58 ;

#line 715 "../../src/dcl3.c"
struct ea _au0__V59 ;

#line 715 "../../src/dcl3.c"
struct ea _au0__V60 ;

#line 698 "../../src/dcl3.c"
error ( (char *)"two different return valueTs for overloaded%n: %t and %t", (struct ea *)( ( ((& _au0__V58 )-> _ea__O1.__C1_p = ((char *)_au2_nx )), (((& _au0__V58 )))) )
#line 698 "../../src/dcl3.c"
, (struct ea *)( ( ((& _au0__V59 )-> _ea__O1.__C1_p = ((char *)_au2_fx -> _fct_returns )), (((& _au0__V59 )))) ) ,
#line 698 "../../src/dcl3.c"
(struct ea *)( ( ((& _au0__V60 )-> _ea__O1.__C1_p = ((char *)_au0_f -> _fct_returns )), (((& _au0__V60 )))) ) , (struct
#line 698 "../../src/dcl3.c"
ea *)ea0 ) ;
} if (_au0_warn ){ 
#line 700 "../../src/dcl3.c"
if (_au2_vp )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"ATs differ (only): [] vs *", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 700 "../../src/dcl3.c"
ea *)ea0 , (struct ea *)ea0 ) ;
if (_au2_op || _au2_cp ){ 
#line 715 "../../src/dcl3.c"
struct ea _au0__V61 ;

#line 715 "../../src/dcl3.c"
struct ea _au0__V62 ;

#line 701 "../../src/dcl3.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"the overloading mechanism cannot tell a%t from a%t", (struct ea *)( ( ((& _au0__V61 )-> _ea__O1.__C1_p = ((char *)_au2_fx )), (((&
#line 701 "../../src/dcl3.c"
_au0__V61 )))) ) , (struct ea *)( ( ((& _au0__V62 )-> _ea__O1.__C1_p = ((char *)_au0_f )), (((& _au0__V62 )))) )
#line 701 "../../src/dcl3.c"
, (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
return _au2_nx ;
xx :
#line 705 "../../src/dcl3.c"
if ((_au0_warn && (_au2_fx -> _fct_nargs <= (((_au2_vp + _au2_op )+ _au2_cp )+ _au2_xp )))&& (_au2_fx -> _fct_nargs == _au0_f -> _fct_nargs ))
#line 706 "../../src/dcl3.c"
{ 
#line 709 "../../src/dcl3.c"
if (_au2_vp )errorFI_PC_RCea__RCea__RCea__RCea___ (
#line 709 "../../src/dcl3.c"
(int )'w' , (char *)"ATs differ (only): [] vs *", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
if (_au2_op || _au2_cp ){ 
#line 715 "../../src/dcl3.c"
struct ea _au0__V63 ;

#line 715 "../../src/dcl3.c"
struct ea _au0__V64 ;

#line 710 "../../src/dcl3.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"the overloading mechanism cannot tell a%t from a%t", (struct ea *)( ( ((& _au0__V63 )-> _ea__O1.__C1_p = ((char *)_au2_fx )), (((&
#line 710 "../../src/dcl3.c"
_au0__V63 )))) ) , (struct ea *)( ( ((& _au0__V64 )-> _ea__O1.__C1_p = ((char *)_au0_f )), (((& _au0__V64 )))) )
#line 710 "../../src/dcl3.c"
, (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}

#line 714 "../../src/dcl3.c"
return (struct name *)0 ;
}
;
int _name_no_of_names (_au0_this )
#line 607 "../../src/cfront.h"
struct name *_au0_this ;

#line 718 "../../src/dcl3.c"
{ 
#line 719 "../../src/dcl3.c"
register int _au1_i ;
register Pname _au1_n ;

#line 719 "../../src/dcl3.c"
_au1_i = 0 ;

#line 721 "../../src/dcl3.c"
for(_au1_n = _au0_this ;_au1_n ;_au1_n = _au1_n -> _name_n_list ) _au1_i ++ ;
return _au1_i ;
}
;
static Pexpr lvec [20];

#line 725 "../../src/dcl3.c"
static Pexpr *lll ;
static Pexpr list_back = 0 ;

#line 728 "../../src/dcl3.c"
extern char new_list (_au0_lx )Pexpr _au0_lx ;
{ 
#line 730 "../../src/dcl3.c"
if (_au0_lx -> _node_base != 124 )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"IrLX", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 730 "../../src/dcl3.c"
ea *)ea0 , (struct ea *)ea0 ) ;

#line 732 "../../src/dcl3.c"
lll = lvec ;
lll ++ ;
(*lll )= _au0_lx -> _expr__O3.__C3_e1 ;
}
;
extern Pexpr next_elem ()
#line 738 "../../src/dcl3.c"
{ 
#line 739 "../../src/dcl3.c"
Pexpr _au1_e ;
Pexpr _au1_lx ;

#line 742 "../../src/dcl3.c"
if (lll == lvec )return (struct expr *)0 ;

#line 744 "../../src/dcl3.c"
_au1_lx = (*lll );

#line 746 "../../src/dcl3.c"
if (list_back ){ 
#line 747 "../../src/dcl3.c"
_au1_e = list_back ;
list_back = 0 ;
return _au1_e ;
}

#line 752 "../../src/dcl3.c"
if (_au1_lx == 0 ){ 
#line 753 "../../src/dcl3.c"
lll -- ;
return (struct expr *)0 ;
}

#line 757 "../../src/dcl3.c"
switch (_au1_lx -> _node_base ){ 
#line 758 "../../src/dcl3.c"
case 140 : 
#line 759 "../../src/dcl3.c"
_au1_e = _au1_lx -> _expr__O3.__C3_e1 ;
(*lll )= _au1_lx -> _expr__O4.__C4_e2 ;
switch (_au1_e -> _node_base ){ 
#line 762 "../../src/dcl3.c"
case 124 : 
#line 763 "../../src/dcl3.c"
lll ++ ;
(*lll )= _au1_e -> _expr__O3.__C3_e1 ;
return (((struct expr *)1 ));
case 140 : 
#line 767 "../../src/dcl3.c"
error ( (char *)"nestedEL", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 767 "../../src/dcl3.c"

#line 768 "../../src/dcl3.c"
return (struct expr *)0 ;
default : 
#line 770 "../../src/dcl3.c"
return _au1_e ;
}
default : 
#line 773 "../../src/dcl3.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"IrL", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 773 "../../src/dcl3.c"
ea *)ea0 ) ;
}
}
;
extern char list_check (_au0_nn , _au0_t , _au0_il )Pname _au0_nn ;

#line 777 "../../src/dcl3.c"
Ptype _au0_t ;

#line 777 "../../src/dcl3.c"
Pexpr _au0_il ;

#line 783 "../../src/dcl3.c"
{ 
#line 784 "../../src/dcl3.c"
Pexpr _au1_e ;
bit _au1_lst ;
int _au1_i ;
int _au1_tdef ;
Pclass _au1_cl ;

#line 785 "../../src/dcl3.c"
_au1_lst = 0 ;

#line 787 "../../src/dcl3.c"
_au1_tdef = 0 ;

#line 790 "../../src/dcl3.c"
if (_au0_il == (((struct expr *)1 )))
#line 791 "../../src/dcl3.c"
_au1_lst = 1 ;
else if (_au0_il )
#line 793 "../../src/dcl3.c"
list_back = _au0_il ;

#line 795 "../../src/dcl3.c"
zzz :
#line 796 "../../src/dcl3.c"
switch (_au0_t -> _node_base ){ 
#line 797 "../../src/dcl3.c"
case 97 : 
#line 798 "../../src/dcl3.c"
_au0_t = (((struct basetype *)_au0_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
_au1_tdef += 1 ;
goto zzz ;

#line 802 "../../src/dcl3.c"
case 110 : 
#line 803 "../../src/dcl3.c"
{ Pvec _au3_v ;
Ptype _au3_vt ;

#line 803 "../../src/dcl3.c"
_au3_v = (((struct vec *)_au0_t ));
_au3_vt = _au3_v -> _pvtyp_typ ;

#line 806 "../../src/dcl3.c"
if (_au3_v -> _vec_size ){ 
#line 807 "../../src/dcl3.c"
if (_au3_v -> _pvtyp_typ -> _node_base == 5 ){ 
#line 808 "../../src/dcl3.c"
_au1_e = next_elem ( ) ;
if (_au1_e -> _node_base == 81 ){ 
#line 810 "../../src/dcl3.c"
int _au6_isz ;

#line 810 "../../src/dcl3.c"
_au6_isz = (((struct vec *)_au1_e -> _expr__O2.__C2_tp ))-> _vec_size ;
if (_au3_v -> _vec_size < _au6_isz ){ 
#line 939 "../../src/dcl3.c"
struct ea _au0__V65 ;

#line 939 "../../src/dcl3.c"
struct ea _au0__V66 ;

#line 939 "../../src/dcl3.c"
struct ea _au0__V67 ;

#line 811 "../../src/dcl3.c"
error ( (char *)"Ir too long (%d characters) for%n[%d]", (struct ea *)( ( ((& _au0__V65 )-> _ea__O1.__C1_i = _au6_isz ), (((& _au0__V65 )))) ) ,
#line 811 "../../src/dcl3.c"
(struct ea *)( ( ((& _au0__V66 )-> _ea__O1.__C1_p = ((char *)_au0_nn )), (((& _au0__V66 )))) ) , (struct ea *)(
#line 811 "../../src/dcl3.c"
( ((& _au0__V67 )-> _ea__O1.__C1_i = _au3_v -> _vec_size ), (((& _au0__V67 )))) ) , (struct ea *)ea0 ) ;
} break ;
}
else 
#line 815 "../../src/dcl3.c"
list_back = _au1_e ;
}
for(_au1_i = 0 ;_au1_i < _au3_v -> _vec_size ;_au1_i ++ ) { 
#line 818 "../../src/dcl3.c"
ee :
#line 819 "../../src/dcl3.c"
_au1_e = next_elem ( ) ;
if (_au1_e == 0 )goto xsw ;
vtz :
#line 823 "../../src/dcl3.c"
switch (_au3_vt -> _node_base ){ 
#line 824 "../../src/dcl3.c"
case 97 : 
#line 825 "../../src/dcl3.c"
_au3_vt = (((struct basetype *)_au3_vt ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
goto vtz ;
case 110 : 
#line 828 "../../src/dcl3.c"
case 119 : 
#line 829 "../../src/dcl3.c"
list_check ( _au0_nn , _au3_vt , _au1_e ) ;
break ;
default : 
#line 832 "../../src/dcl3.c"
if (_au1_e == (((struct expr *)1 ))){ 
#line 833 "../../src/dcl3.c"
error ( (char *)"unexpectedIrL", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 833 "../../src/dcl3.c"
ea *)ea0 , (struct ea *)ea0 ) ;
goto ee ;
}
if (_type_check ( _au3_vt , _au1_e -> _expr__O2.__C2_tp , (unsigned char )70 ) )
#line 837 "../../src/dcl3.c"
{ 
#line 939 "../../src/dcl3.c"
struct ea _au0__V68 ;

#line 939 "../../src/dcl3.c"
struct ea _au0__V69 ;

#line 939 "../../src/dcl3.c"
struct ea _au0__V70 ;

#line 837 "../../src/dcl3.c"
error ( (char *)"badIrT for%n:%t (%tX)", (struct ea *)( ( ((& _au0__V68 )-> _ea__O1.__C1_p = ((char *)_au0_nn )), (((& _au0__V68 )))) )
#line 837 "../../src/dcl3.c"
, (struct ea *)( ( ((& _au0__V69 )-> _ea__O1.__C1_p = ((char *)_au1_e -> _expr__O2.__C2_tp )), (((& _au0__V69 )))) ) ,
#line 837 "../../src/dcl3.c"
(struct ea *)( ( ((& _au0__V70 )-> _ea__O1.__C1_p = ((char *)_au3_vt )), (((& _au0__V70 )))) ) , (struct ea *)ea0 )
#line 837 "../../src/dcl3.c"
;
} }
}
if (_au1_lst && (_au1_e = next_elem ( ) ))error ( (char *)"end ofIrLX after vector", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 840 "../../src/dcl3.c"
ea *)ea0 , (struct ea *)ea0 ) ;
xsw :;
}
else { 
#line 844 "../../src/dcl3.c"
_au1_i = 0 ;
xx :
#line 846 "../../src/dcl3.c"
while (_au1_e = next_elem ( ) ){ 
#line 847 "../../src/dcl3.c"
_au1_i ++ ;
vtzz :
#line 850 "../../src/dcl3.c"
switch (_au3_vt -> _node_base ){ 
#line 851 "../../src/dcl3.c"
case 97 : 
#line 852 "../../src/dcl3.c"
_au3_vt = (((struct basetype *)_au3_vt ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
goto vtzz ;
case 110 : 
#line 855 "../../src/dcl3.c"
case 119 : 
#line 856 "../../src/dcl3.c"
list_check ( _au0_nn , _au3_vt , _au1_e ) ;
break ;
default : 
#line 859 "../../src/dcl3.c"
if (_au1_e == (((struct expr *)1 ))){ 
#line 860 "../../src/dcl3.c"
error ( (char *)"unexpectedIrL", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 860 "../../src/dcl3.c"
ea *)ea0 , (struct ea *)ea0 ) ;
goto xx ;
}
if (_type_check ( _au3_vt , _au1_e -> _expr__O2.__C2_tp , (unsigned char )70 ) )
#line 864 "../../src/dcl3.c"
{ 
#line 939 "../../src/dcl3.c"
struct ea _au0__V71 ;

#line 939 "../../src/dcl3.c"
struct ea _au0__V72 ;

#line 939 "../../src/dcl3.c"
struct ea _au0__V73 ;

#line 864 "../../src/dcl3.c"
error ( (char *)"badIrT for%n:%t (%tX)", (struct ea *)( ( ((& _au0__V71 )-> _ea__O1.__C1_p = ((char *)_au0_nn )), (((& _au0__V71 )))) )
#line 864 "../../src/dcl3.c"
, (struct ea *)( ( ((& _au0__V72 )-> _ea__O1.__C1_p = ((char *)_au1_e -> _expr__O2.__C2_tp )), (((& _au0__V72 )))) ) ,
#line 864 "../../src/dcl3.c"
(struct ea *)( ( ((& _au0__V73 )-> _ea__O1.__C1_p = ((char *)_au3_vt )), (((& _au0__V73 )))) ) , (struct ea *)ea0 )
#line 864 "../../src/dcl3.c"
;
} }
}
if (! _au1_tdef )_au3_v -> _vec_size = _au1_i ;
}
break ;
}

#line 872 "../../src/dcl3.c"
case 6 : 
#line 873 "../../src/dcl3.c"
_au1_cl = (((struct classdef *)_au0_t ));
goto ccc ;

#line 876 "../../src/dcl3.c"
case 119 : 
#line 877 "../../src/dcl3.c"
_au1_cl = (((struct classdef *)(((struct basetype *)_au0_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ));
ccc :
#line 879 "../../src/dcl3.c"
{ Ptable _au3_tbl ;
Pname _au3_m ;

#line 879 "../../src/dcl3.c"
_au3_tbl = _au1_cl -> _classdef_memtbl ;

#line 882 "../../src/dcl3.c"
if (_au1_cl -> _classdef_clbase )list_check ( _au0_nn , _au1_cl -> _classdef_clbase -> _expr__O2.__C2_tp , (struct expr *)0 ) ;

#line 884 "../../src/dcl3.c"
for(_au3_m = _table_get_mem ( _au3_tbl , _au1_i = 1 ) ;_au3_m ;_au3_m = _table_get_mem ( _au3_tbl , ++ _au1_i ) ) { 
#line 885 "../../src/dcl3.c"
Ptype _au4_mt ;

#line 885 "../../src/dcl3.c"
_au4_mt = _au3_m -> _expr__O2.__C2_tp ;
switch (_au4_mt -> _node_base ){ 
#line 887 "../../src/dcl3.c"
case 108 : 
#line 888 "../../src/dcl3.c"
case 76 : 
#line 889 "../../src/dcl3.c"
case 6 : 
#line 890 "../../src/dcl3.c"
case 13 : 
#line 891 "../../src/dcl3.c"
continue ;
}
if (_au3_m -> _name_n_stclass == 31 )continue ;

#line 895 "../../src/dcl3.c"
dd :
#line 896 "../../src/dcl3.c"
_au1_e = next_elem ( ) ;
if (_au1_e == 0 )return ;
mtz :
#line 900 "../../src/dcl3.c"
switch (_au4_mt -> _node_base ){ 
#line 901 "../../src/dcl3.c"
case 97 : 
#line 902 "../../src/dcl3.c"
_au4_mt = (((struct basetype *)_au4_mt ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
goto mtz ;
case 6 : 
#line 905 "../../src/dcl3.c"
case 13 : 
#line 906 "../../src/dcl3.c"
break ;
case 110 : 
#line 908 "../../src/dcl3.c"
case 119 : 
#line 909 "../../src/dcl3.c"
list_check ( _au0_nn , _au3_m -> _expr__O2.__C2_tp , _au1_e ) ;
break ;
default : 
#line 912 "../../src/dcl3.c"
if (_au1_e == (((struct expr *)1 ))){ 
#line 913 "../../src/dcl3.c"
error ( (char *)"unexpectedIrL", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 913 "../../src/dcl3.c"
ea *)ea0 , (struct ea *)ea0 ) ;
goto dd ;
}
if (_type_check ( _au4_mt , _au1_e -> _expr__O2.__C2_tp , (unsigned char )70 ) )
#line 917 "../../src/dcl3.c"
{ 
#line 939 "../../src/dcl3.c"
struct ea _au0__V74 ;

#line 939 "../../src/dcl3.c"
struct ea _au0__V75 ;

#line 939 "../../src/dcl3.c"
struct ea _au0__V76 ;

#line 917 "../../src/dcl3.c"
error ( (char *)"badIrT for%n:%t (%tX)", (struct ea *)( ( ((& _au0__V74 )-> _ea__O1.__C1_p = ((char *)_au3_m )), (((& _au0__V74 )))) )
#line 917 "../../src/dcl3.c"
, (struct ea *)( ( ((& _au0__V75 )-> _ea__O1.__C1_p = ((char *)_au1_e -> _expr__O2.__C2_tp )), (((& _au0__V75 )))) ) ,
#line 917 "../../src/dcl3.c"
(struct ea *)( ( ((& _au0__V76 )-> _ea__O1.__C1_p = ((char *)_au3_m -> _expr__O2.__C2_tp )), (((& _au0__V76 )))) ) , (struct
#line 917 "../../src/dcl3.c"
ea *)ea0 ) ;
} }
}
if (_au1_lst && (_au1_e = next_elem ( ) ))error ( (char *)"end ofIrLX afterCO", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 920 "../../src/dcl3.c"
ea *)ea0 , (struct ea *)ea0 ) ;
break ;
}
default : 
#line 924 "../../src/dcl3.c"
_au1_e = next_elem ( ) ;

#line 926 "../../src/dcl3.c"
if (_au1_e == 0 ){ 
#line 927 "../../src/dcl3.c"
error ( (char *)"noIr forO", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 )
#line 927 "../../src/dcl3.c"
;
break ;
}

#line 931 "../../src/dcl3.c"
if (_au1_e == (((struct expr *)1 ))){ 
#line 932 "../../src/dcl3.c"
error ( (char *)"unexpectedIrL", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 932 "../../src/dcl3.c"
ea *)ea0 ) ;
break ;
}
if (_type_check ( _au0_t , _au1_e -> _expr__O2.__C2_tp , (unsigned char )70 ) ){ 
#line 939 "../../src/dcl3.c"
struct ea _au0__V77 ;

#line 939 "../../src/dcl3.c"
struct ea _au0__V78 ;

#line 939 "../../src/dcl3.c"
struct ea _au0__V79 ;

#line 935 "../../src/dcl3.c"
error ( (char *)"badIrT for%n:%t (%tX)", (struct ea *)( ( ((& _au0__V77 )-> _ea__O1.__C1_p = ((char *)_au0_nn )), (((& _au0__V77 )))) )
#line 935 "../../src/dcl3.c"
, (struct ea *)( ( ((& _au0__V78 )-> _ea__O1.__C1_p = ((char *)_au1_e -> _expr__O2.__C2_tp )), (((& _au0__V78 )))) ) ,
#line 935 "../../src/dcl3.c"
(struct ea *)( ( ((& _au0__V79 )-> _ea__O1.__C1_p = ((char *)_au0_t )), (((& _au0__V79 )))) ) , (struct ea *)ea0 )
#line 935 "../../src/dcl3.c"
;
} if (_au1_lst && (_au1_e = next_elem ( ) ))error ( (char *)"end ofIrLX afterO", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 936 "../../src/dcl3.c"
ea *)ea0 , (struct ea *)ea0 ) ;
break ;
}
}
;
Pname dclass (_au0_n , _au0_tbl )Pname _au0_n ;

#line 941 "../../src/dcl3.c"
Ptable _au0_tbl ;
{ 
#line 943 "../../src/dcl3.c"
Pclass _au1_cl ;
Pbase _au1_bt ;
Pname _au1_bn ;
Pname _au1_nx ;

#line 946 "../../src/dcl3.c"
_au1_nx = _table_look ( ktbl , _au0_n -> _expr__O3.__C3_string , (unsigned char )0 ) ;
if (_au1_nx == 0 ){ 
#line 952 "../../src/dcl3.c"
int _au2_tn ;

#line 952 "../../src/dcl3.c"
_au2_tn = 0 ;
for(_au1_nx = _table_look ( ktbl , _au0_n -> _expr__O3.__C3_string , (unsigned char )159 ) ;_au1_nx ;_au1_nx = _au1_nx -> _name_n_tbl_list ) { 
#line 954 "../../src/dcl3.c"
if (_au1_nx ->
#line 954 "../../src/dcl3.c"
_node_n_key != 159 )continue ;
if (_au1_nx -> _expr__O2.__C2_tp -> _node_base != 119 ){ 
#line 956 "../../src/dcl3.c"
_au2_tn = 1 ;
continue ;
}
_au1_bt = (((struct basetype *)_au1_nx -> _expr__O2.__C2_tp ));
_au1_bn = _au1_bt -> _basetype_b_name ;
_au1_cl = (((struct classdef *)_au1_bn -> _expr__O2.__C2_tp ));
if (_au1_cl == 0 )continue ;
goto bbb ;
}

#line 966 "../../src/dcl3.c"
if (_au2_tn )
#line 967 "../../src/dcl3.c"
{ 
#line 996 "../../src/dcl3.c"
struct ea _au0__V80 ;

#line 967 "../../src/dcl3.c"
error ( (char *)"%n redefined using typedef", (struct ea *)( ( ((& _au0__V80 )-> _ea__O1.__C1_p = ((char *)_au0_n )), (((& _au0__V80 )))) )
#line 967 "../../src/dcl3.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} else 
#line 969 "../../src/dcl3.c"
{ 
#line 996 "../../src/dcl3.c"
struct ea _au0__V81 ;

#line 969 "../../src/dcl3.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"%n is not aCN", (struct ea *)( ( ((& _au0__V81 )-> _ea__O1.__C1_p = ((char *)_au0_n )), (((&
#line 969 "../../src/dcl3.c"
_au0__V81 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
else { 
#line 978 "../../src/dcl3.c"
_au1_bt = (((struct basetype *)_au1_nx -> _expr__O2.__C2_tp ));
_au1_bn = _au1_bt -> _basetype_b_name ;
}
bbb :
#line 982 "../../src/dcl3.c"
{ Pname _au1_ln ;

#line 982 "../../src/dcl3.c"
_au1_ln = _table_look ( _au0_tbl , _au1_bn -> _expr__O3.__C3_string , (unsigned char )0 ) ;
if (_au1_ln && (_au1_ln -> _expr__O5.__C5_n_table == _au0_tbl )){ 
#line 996 "../../src/dcl3.c"
struct ea _au0__V82 ;

#line 983 "../../src/dcl3.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"%n redefined", (struct ea *)( ( ((& _au0__V82 )-> _ea__O1.__C1_p = ((char *)_au1_ln )), (((&
#line 983 "../../src/dcl3.c"
_au0__V82 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} _au1_bn -> _name_where = _au1_nx -> _name_where ;
{ Pname _au1_bnn ;

#line 985 "../../src/dcl3.c"
_au1_bnn = _table_insert ( _au0_tbl , _au1_bn , (unsigned char )6 ) ;
_au1_cl = (((struct classdef *)_au1_bn -> _expr__O2.__C2_tp ));

#line 988 "../../src/dcl3.c"
if (_au1_cl -> _type_defined & 3)
#line 989 "../../src/dcl3.c"
{ 
#line 996 "../../src/dcl3.c"
struct ea _au0__V83 ;

#line 989 "../../src/dcl3.c"
error ( (char *)"C%n defined twice", (struct ea *)( ( ((& _au0__V83 )-> _ea__O1.__C1_p = ((char *)_au0_n )), (((& _au0__V83 )))) )
#line 989 "../../src/dcl3.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} else { 
#line 991 "../../src/dcl3.c"
if (_au1_bn -> _name_n_scope == 136 )_au1_bn -> _name_n_scope = 139 ;
_classdef_dcl ( _au1_cl , _au1_bn , _au0_tbl ) ;
}
_au0_n -> _expr__O2.__C2_tp = (struct type *)_au1_cl ;
return _au1_bnn ;
}
}
}
;

#line 998 "../../src/dcl3.c"
Pname denum (_au0_n , _au0_tbl )Pname _au0_n ;

#line 998 "../../src/dcl3.c"
Ptable _au0_tbl ;
{ 
#line 1000 "../../src/dcl3.c"
Pname _au1_nx ;

#line 1002 "../../src/dcl3.c"
Pbase _au1_bt ;
Pname _au1_bn ;
Pname _au1_bnn ;
Penum _au1_en ;

#line 1000 "../../src/dcl3.c"
_au1_nx = _table_look ( ktbl , _au0_n -> _expr__O3.__C3_string , (unsigned char )0 ) ;
if (_au1_nx == 0 )_au1_nx = _table_look ( ktbl , _au0_n -> _expr__O3.__C3_string , (unsigned char )159 ) ;
_au1_bt = (((struct basetype *)_au1_nx -> _expr__O2.__C2_tp ));

#line 1002 "../../src/dcl3.c"
_au1_bn = _au1_bt -> _basetype_b_name ;

#line 1002 "../../src/dcl3.c"
_au1_bnn = _table_insert ( _au0_tbl , _au1_bn , (unsigned char )6 ) ;

#line 1002 "../../src/dcl3.c"
_au1_en = (((struct enumdef *)_au1_bn -> _expr__O2.__C2_tp ));

#line 1006 "../../src/dcl3.c"
if (_au1_en -> _type_defined & 3)
#line 1007 "../../src/dcl3.c"
{ 
#line 1014 "../../src/dcl3.c"
struct ea _au0__V84 ;

#line 1007 "../../src/dcl3.c"
error ( (char *)"enum%n defined twice", (struct ea *)( ( ((& _au0__V84 )-> _ea__O1.__C1_p = ((char *)_au0_n )), (((& _au0__V84 )))) )
#line 1007 "../../src/dcl3.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} else { 
#line 1009 "../../src/dcl3.c"
if (_au1_bn -> _name_n_scope == 136 )_au1_bn -> _name_n_scope = 139 ;
_enumdef_dcl ( _au1_en , _au1_bn , _au0_tbl ) ;
}
_au0_n -> _expr__O2.__C2_tp = (struct type *)_au1_en ;
return _au1_bnn ;
}
;
char dargs (_au0__A85 , _au0_f , _au0_tbl )Pname _au0__A85 ;

#line 1016 "../../src/dcl3.c"
Pfct _au0_f ;

#line 1016 "../../src/dcl3.c"
Ptable _au0_tbl ;
{ 
#line 1018 "../../src/dcl3.c"
int _au1_oo ;

#line 1020 "../../src/dcl3.c"
Pname _au1_a ;

#line 1018 "../../src/dcl3.c"
_au1_oo = const_save ;
const_save = 1 ;
for(_au1_a = _au0_f -> _fct_argtype ;_au1_a ;_au1_a = _au1_a -> _name_n_list ) { 
#line 1021 "../../src/dcl3.c"
Pexpr _au2_init ;
Pname _au2_cln ;

#line 1022 "../../src/dcl3.c"
_au2_cln = _type_is_cl_obj ( _au1_a -> _expr__O2.__C2_tp ) ;
if (_au2_cln && ( (((struct classdef *)_au2_cln -> _expr__O2.__C2_tp ))-> _classdef_itor ) ){ 
#line 1025 "../../src/dcl3.c"
_au1_a -> _name_n_xref = 1 ;
}
if (_au2_init = _au1_a -> _expr__O4.__C4_n_initializer ){ 
#line 1028 "../../src/dcl3.c"
if (_au2_cln ){ 
#line 1029 "../../src/dcl3.c"
if (_au2_init -> _node_base == 157 ){ 
#line 1030 "../../src/dcl3.c"
switch (_au2_init -> _expr__O5.__C5_tp2 -> _node_base ){
#line 1030 "../../src/dcl3.c"

#line 1031 "../../src/dcl3.c"
case 6 : 
#line 1032 "../../src/dcl3.c"
if ((((struct classdef *)_au2_init -> _expr__O5.__C5_tp2 ))!= (((struct classdef *)_au2_cln -> _expr__O2.__C2_tp )))goto inin2 ;
break ;
default : 
#line 1035 "../../src/dcl3.c"
{ Pname _au6_n2 ;

#line 1035 "../../src/dcl3.c"
_au6_n2 = _type_is_cl_obj ( _au2_init -> _expr__O5.__C5_tp2 ) ;
if ((_au6_n2 == 0 )|| ((((struct classdef *)_au6_n2 -> _expr__O2.__C2_tp ))!= (((struct classdef *)_au2_cln -> _expr__O2.__C2_tp ))))goto inin2 ;
}
}

#line 1038 "../../src/dcl3.c"
_au2_init -> _expr__O4.__C4_e2 = (struct expr *)_au1_a ;
_au2_init = _expr_typ ( _au2_init , _au0_tbl ) ;
_expr_simpl ( _au2_init ) ;
_au2_init -> _node_permanent = 2 ;
_au1_a -> _expr__O4.__C4_n_initializer = _au2_init ;
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"K as defaultA", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 1043 "../../src/dcl3.c"
}
else 
#line 1045 "../../src/dcl3.c"
{ 
#line 1046 "../../src/dcl3.c"
inin2 :
#line 1047 "../../src/dcl3.c"
if (_au2_init -> _node_base == 124 )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"list as AIr", (struct ea *)ea0 , (struct ea *)ea0 ,
#line 1047 "../../src/dcl3.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
{ Pexpr _au5_i ;

#line 1048 "../../src/dcl3.c"
_au5_i = _expr_typ ( _au2_init , _au0_tbl ) ;
_au2_init = class_init ( (struct expr *)_au1_a , _au1_a -> _expr__O2.__C2_tp , _au5_i , _au0_tbl ) ;
if ((_au5_i != _au2_init )&& (_au2_init -> _node_base == 111 ))errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"K needed forAIr", (struct ea *)ea0 , (struct ea *)ea0 ,
#line 1050 "../../src/dcl3.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
_expr_simpl ( _au2_init ) ;
_au2_init -> _node_permanent = 2 ;
_au1_a -> _expr__O4.__C4_n_initializer = _au2_init ;
}
}
}
else 
#line 1056 "../../src/dcl3.c"
if (_type_is_ref ( _au1_a -> _expr__O2.__C2_tp ) ){ 
#line 1057 "../../src/dcl3.c"
_au2_init = _expr_typ ( _au2_init , _au0_tbl ) ;
{ int _au4_tcount ;

#line 1058 "../../src/dcl3.c"
_au4_tcount = stcount ;
_au2_init = ref_init ( ((struct ptr *)_au1_a -> _expr__O2.__C2_tp ), _au2_init , _au0_tbl ) ;
if (_au4_tcount != stcount )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"needs temporaryV to evaluateAIr", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 1060 "../../src/dcl3.c"
ea *)ea0 ) ;
_expr_simpl ( _au2_init ) ;
_au2_init -> _node_permanent = 2 ;
_au1_a -> _expr__O4.__C4_n_initializer = _au2_init ;
}
}
else 
#line 1065 "../../src/dcl3.c"
{ 
#line 1066 "../../src/dcl3.c"
_au2_init = _expr_typ ( _au2_init , _au0_tbl ) ;
if (_type_check ( _au1_a -> _expr__O2.__C2_tp , _au2_init -> _expr__O2.__C2_tp , (unsigned char )136 ) ){ 
#line 1068 "../../src/dcl3.c"
int _au5_i ;

#line 1068 "../../src/dcl3.c"
_au5_i = can_coerce ( _au1_a -> _expr__O2.__C2_tp , _au2_init -> _expr__O2.__C2_tp ) ;

#line 1070 "../../src/dcl3.c"
switch (_au5_i ){ 
#line 1071 "../../src/dcl3.c"
case 1 : 
#line 1072 "../../src/dcl3.c"
if (Ncoerce ){ 
#line 1073 "../../src/dcl3.c"
Pname _au7_cn ;
Pclass _au7_cl ;
Pref _au7_r ;

#line 1076 "../../src/dcl3.c"
struct ref *_au0__Xthis__ctor_ref ;

#line 1073 "../../src/dcl3.c"
_au7_cn = _type_is_cl_obj ( _au2_init -> _expr__O2.__C2_tp ) ;
_au7_cl = (((struct classdef *)_au7_cn -> _expr__O2.__C2_tp ));
_au7_r = (struct ref *)( (_au0__Xthis__ctor_ref = 0 ), ( ( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ref ),
#line 1075 "../../src/dcl3.c"
((unsigned char )45 ), _au2_init , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = Ncoerce ), ((_au0__Xthis__ctor_ref ))) )
#line 1075 "../../src/dcl3.c"
) ;
_au2_init = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )146 , (struct expr *)_au7_r , (struct expr *)0 ) ;
_au2_init -> _expr__O5.__C5_fct_name = Ncoerce ;
_au2_init -> _expr__O2.__C2_tp = _au1_a -> _expr__O2.__C2_tp ;
}
break ;
default : 
#line 1082 "../../src/dcl3.c"
{ 
#line 1105 "../../src/dcl3.c"
struct ea _au0__V86 ;

#line 1082 "../../src/dcl3.c"
error ( (char *)"%d possible conversions for defaultA", (struct ea *)( ( ((& _au0__V86 )-> _ea__O1.__C1_i = _au5_i ), (((& _au0__V86 )))) ) ,
#line 1082 "../../src/dcl3.c"
(struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
case 0 : 
#line 1084 "../../src/dcl3.c"
{ 
#line 1105 "../../src/dcl3.c"
struct ea _au0__V87 ;

#line 1105 "../../src/dcl3.c"
struct ea _au0__V88 ;

#line 1084 "../../src/dcl3.c"
error ( (char *)"badIrT%t forA%n", (struct ea *)( ( ((& _au0__V87 )-> _ea__O1.__C1_p = ((char *)_au2_init -> _expr__O2.__C2_tp )), (((& _au0__V87 ))))
#line 1084 "../../src/dcl3.c"
) , (struct ea *)( ( ((& _au0__V88 )-> _ea__O1.__C1_p = ((char *)_au1_a )), (((& _au0__V88 )))) ) ,
#line 1084 "../../src/dcl3.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
if (_au2_init && (_au2_init -> _node_permanent == 0 ))_expr_del ( _au2_init ) ;
_au1_a -> _expr__O4.__C4_n_initializer = (_au2_init = 0 );
} } }
}

#line 1090 "../../src/dcl3.c"
if (_au2_init ){ 
#line 1091 "../../src/dcl3.c"
_expr_simpl ( _au2_init ) ;
_au2_init -> _node_permanent = 2 ;
_au1_a -> _expr__O4.__C4_n_initializer = _au2_init ;
Neval = 0 ;
{ int _au5_i ;

#line 1095 "../../src/dcl3.c"
_au5_i = _expr_eval ( _au2_init ) ;
if (Neval == 0 ){ 
#line 1097 "../../src/dcl3.c"
_au1_a -> _name_n_evaluated = 1 ;
_au1_a -> _name_n_val = _au5_i ;
}
}
}
}
}
}

#line 1104 "../../src/dcl3.c"
const_save = _au1_oo ;
}
;

#line 1108 "../../src/dcl3.c"
char merge_init (_au0_nn , _au0_f , _au0_nf )Pname _au0_nn ;

#line 1108 "../../src/dcl3.c"
Pfct _au0_f ;

#line 1108 "../../src/dcl3.c"
Pfct _au0_nf ;
{ 
#line 1110 "../../src/dcl3.c"
Pname _au1_a1 ;
Pname _au1_a2 ;

#line 1110 "../../src/dcl3.c"
_au1_a1 = _au0_f -> _fct_argtype ;
_au1_a2 = _au0_nf -> _fct_argtype ;
for(;_au1_a1 ;( (_au1_a1 = _au1_a1 -> _name_n_list ), (_au1_a2 = _au1_a2 -> _name_n_list )) ) { 
#line 1113 "../../src/dcl3.c"
int _au2_i1 ;
int _au2_i2 ;

#line 1113 "../../src/dcl3.c"
_au2_i1 = (_au1_a1 -> _expr__O4.__C4_n_initializer || _au1_a1 -> _name_n_evaluated );
_au2_i2 = (_au1_a2 -> _expr__O4.__C4_n_initializer || _au1_a2 -> _name_n_evaluated );
if (_au2_i1 ){ 
#line 1116 "../../src/dcl3.c"
if (_au2_i2 && (((_au1_a1 -> _name_n_evaluated == 0 )|| (_au1_a2 -> _name_n_evaluated == 0 ))|| (_au1_a1 -> _name_n_val != _au1_a2 -> _name_n_val )))
#line 1121 "../../src/dcl3.c"
{
#line 1121 "../../src/dcl3.c"

#line 1129 "../../src/dcl3.c"
struct ea _au0__V89 ;

#line 1129 "../../src/dcl3.c"
struct ea _au0__V90 ;

#line 1121 "../../src/dcl3.c"
error ( (char *)"twoIrs for%nA%n", (struct ea *)( ( ((& _au0__V89 )-> _ea__O1.__C1_p = ((char *)_au0_nn )), (((& _au0__V89 )))) )
#line 1121 "../../src/dcl3.c"
, (struct ea *)( ( ((& _au0__V90 )-> _ea__O1.__C1_p = ((char *)_au1_a1 )), (((& _au0__V90 )))) ) , (struct
#line 1121 "../../src/dcl3.c"
ea *)ea0 , (struct ea *)ea0 ) ;
} }
else if (_au2_i2 ){ 
#line 1124 "../../src/dcl3.c"
_au1_a1 -> _expr__O4.__C4_n_initializer = _au1_a2 -> _expr__O4.__C4_n_initializer ;
_au1_a1 -> _name_n_evaluated = _au1_a2 -> _name_n_evaluated ;
_au1_a1 -> _name_n_val = _au1_a2 -> _name_n_val ;
}
}
}
;
extern Pexpr try_to_coerce (_au0_rt , _au0_e , _au0_s , _au0_tbl )Ptype _au0_rt ;

#line 1131 "../../src/dcl3.c"
Pexpr _au0_e ;

#line 1131 "../../src/dcl3.c"
char *_au0_s ;

#line 1131 "../../src/dcl3.c"
Ptable _au0_tbl ;

#line 1135 "../../src/dcl3.c"
{ 
#line 1136 "../../src/dcl3.c"
int _au1_i ;
Pname _au1_cn ;

#line 1139 "../../src/dcl3.c"
if (((_au1_cn = _type_is_cl_obj ( _au0_e -> _expr__O2.__C2_tp ) )&& (_au1_i = can_coerce ( _au0_rt , _au0_e -> _expr__O2.__C2_tp ) ))&& Ncoerce )
#line 1141 "../../src/dcl3.c"
{ 
#line 1143 "../../src/dcl3.c"
if
#line 1143 "../../src/dcl3.c"
(1 < _au1_i ){ 
#line 1153 "../../src/dcl3.c"
struct ea _au0__V91 ;

#line 1153 "../../src/dcl3.c"
struct ea _au0__V92 ;

#line 1143 "../../src/dcl3.c"
error ( (char *)"%d possible conversions for %s", (struct ea *)( ( ((& _au0__V91 )-> _ea__O1.__C1_i = _au1_i ), (((& _au0__V91 )))) ) ,
#line 1143 "../../src/dcl3.c"
(struct ea *)( ( ((& _au0__V92 )-> _ea__O1.__C1_p = ((char *)_au0_s )), (((& _au0__V92 )))) ) , (struct ea *)ea0 ,
#line 1143 "../../src/dcl3.c"
(struct ea *)ea0 ) ;
} { Pclass _au2_cl ;
Pref _au2_r ;
Pexpr _au2_rr ;
Pexpr _au2_c ;

#line 1148 "../../src/dcl3.c"
struct ref *_au0__Xthis__ctor_ref ;

#line 1144 "../../src/dcl3.c"
_au2_cl = (((struct classdef *)_au1_cn -> _expr__O2.__C2_tp ));
_au2_r = (struct ref *)( (_au0__Xthis__ctor_ref = 0 ), ( ( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ref ),
#line 1145 "../../src/dcl3.c"
((unsigned char )45 ), _au0_e , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = Ncoerce ), ((_au0__Xthis__ctor_ref ))) )
#line 1145 "../../src/dcl3.c"
) ;
_au2_rr = _expr_typ ( (struct expr *)_au2_r , _au0_tbl ) ;
_au2_c = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )146 , _au2_rr , (struct expr *)0 ) ;
_au2_c -> _expr__O5.__C5_fct_name = Ncoerce ;

#line 1150 "../../src/dcl3.c"
return _expr_typ ( _au2_c , _au0_tbl ) ;
}
}

#line 1152 "../../src/dcl3.c"
return (struct expr *)0 ;
}
;
int in_class_dcl = 0 ;
Pname _name_dofct (_au0_this , _au0_tbl , _au0_scope )
#line 607 "../../src/cfront.h"
struct name *_au0_this ;

#line 1156 "../../src/dcl3.c"
Ptable _au0_tbl ;

#line 1156 "../../src/dcl3.c"
TOK _au0_scope ;
{ 
#line 1158 "../../src/dcl3.c"
Pfct _au1_f ;
Pname _au1_class_name ;
Ptable _au1_etbl ;
int _au1_can_overload ;
int _au1_just_made ;

#line 1158 "../../src/dcl3.c"
_au1_f = (((struct fct *)_au0_this -> _expr__O2.__C2_tp ));

#line 1162 "../../src/dcl3.c"
_au1_just_made = 0 ;

#line 1164 "../../src/dcl3.c"
in_class_dcl = (cc -> _dcl_context_not != 0 );
if (_au1_f -> _fct_f_inline )_au0_this -> _name_n_sto = 31 ;

#line 1167 "../../src/dcl3.c"
if (_au1_f -> _fct_argtype )dargs ( _au0_this , _au1_f , _au0_tbl ) ;

#line 1169 "../../src/dcl3.c"
_type_dcl ( _au0_this -> _expr__O2.__C2_tp , _au0_tbl ) ;
if (_au0_this -> _name__O6.__C6_n_qualifier ){ 
#line 1171 "../../src/dcl3.c"
if (in_class_dcl ){ 
#line 1172 "../../src/dcl3.c"
{ 
#line 1556 "../../src/dcl3.c"
struct ea _au0__V93 ;

#line 1172 "../../src/dcl3.c"
error ( (char *)"unexpectedQdN%n", (struct ea *)( ( ((& _au0__V93 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V93 )))) )
#line 1172 "../../src/dcl3.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
return (struct name *)0 ;
} }
_au1_class_name = (((struct basetype *)_au0_this -> _name__O6.__C6_n_qualifier -> _expr__O2.__C2_tp ))-> _basetype_b_name ;
_au1_etbl = (((struct classdef *)_au1_class_name -> _expr__O2.__C2_tp ))-> _classdef_memtbl ;
if (_au1_f -> _fct_f_virtual ){ 
#line 1179 "../../src/dcl3.c"
{ 
#line 1556 "../../src/dcl3.c"
struct ea _au0__V94 ;

#line 1556 "../../src/dcl3.c"
struct ea _au0__V95 ;

#line 1179 "../../src/dcl3.c"
error ( (char *)"virtual outsideCD %s::%n", (struct ea *)( ( ((& _au0__V94 )-> _ea__O1.__C1_p = ((char *)_au1_class_name -> _expr__O3.__C3_string )), (((& _au0__V94 ))))
#line 1179 "../../src/dcl3.c"
) , (struct ea *)( ( ((& _au0__V95 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V95 )))) ) ,
#line 1179 "../../src/dcl3.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
_au1_f -> _fct_f_virtual = 0 ;
} }
}
else { 
#line 1184 "../../src/dcl3.c"
_au1_class_name = cc -> _dcl_context_not ;

#line 1186 "../../src/dcl3.c"
if (_au1_class_name && (_au0_tbl != cc -> _dcl_context_cot -> _classdef_memtbl )){ 
#line 1187 "../../src/dcl3.c"
_au1_class_name = 0 ;
in_class_dcl = 0 ;
}
if (_au0_this -> _name_n_oper )_name_check_oper ( _au0_this , _au1_class_name ) ;
_au1_etbl = _au0_tbl ;
}

#line 1194 "../../src/dcl3.c"
(((struct fct *)_au0_this -> _expr__O2.__C2_tp ))-> _fct_memof = (_au1_class_name ? (((struct classdef *)_au1_class_name -> _expr__O2.__C2_tp )): (((struct classdef *)0 )));

#line 1196 "../../src/dcl3.c"
if ((_au1_etbl == 0 )|| (_au1_etbl -> _node_base != 142 )){ 
#line 1556 "../../src/dcl3.c"
struct ea _au0__V96 ;

#line 1196 "../../src/dcl3.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"N::dcl: etbl=%d", (struct ea *)( ( ((& _au0__V96 )-> _ea__O1.__C1_p = ((char *)_au1_etbl )), (((&
#line 1196 "../../src/dcl3.c"
_au0__V96 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} 
#line 1198 "../../src/dcl3.c"
switch (_au0_this -> _name_n_oper ){ 
#line 1199 "../../src/dcl3.c"
case 23 : 
#line 1200 "../../src/dcl3.c"
case 9 : 
#line 1201 "../../src/dcl3.c"
switch (_au0_scope ){ 
#line 1202 "../../src/dcl3.c"
case 0 : 
#line 1203 "../../src/dcl3.c"
case 25 :
#line 1203 "../../src/dcl3.c"

#line 1204 "../../src/dcl3.c"
{ 
#line 1556 "../../src/dcl3.c"
struct ea _au0__V97 ;

#line 1204 "../../src/dcl3.c"
error ( (char *)"%nMF", (struct ea *)( ( ((& _au0__V97 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V97 )))) )
#line 1204 "../../src/dcl3.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
case 0 : 
#line 1207 "../../src/dcl3.c"
_au1_can_overload = in_class_dcl ;
break ;
case 161 : 
#line 1210 "../../src/dcl3.c"
if (_au1_f -> _fct_f_virtual ){ 
#line 1211 "../../src/dcl3.c"
error ( (char *)"virtualK", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ,
#line 1211 "../../src/dcl3.c"
(struct ea *)ea0 ) ;
_au1_f -> _fct_f_virtual = 0 ;
}
case 162 : 
#line 1215 "../../src/dcl3.c"
if (fct_void )_au0_this -> _name_n_scope = 25 ;
_au1_can_overload = in_class_dcl ;
break ;
case 97 : 
#line 1219 "../../src/dcl3.c"
_au1_can_overload = 0 ;
break ;
case 70 : 
#line 1223 "../../src/dcl3.c"
if (_au1_class_name && (_au1_f -> _fct_nargs == 1 )){ 
#line 1224 "../../src/dcl3.c"
Ptype _au3_t ;
Pname _au3_an ;

#line 1224 "../../src/dcl3.c"
_au3_t = _au1_f -> _fct_argtype -> _expr__O2.__C2_tp ;
_au3_an = _type_is_cl_obj ( _au3_t ) ;
if ((_au3_an == 0 )&& _type_is_ref ( _au3_t ) ){ 
#line 1227 "../../src/dcl3.c"
_au3_t = (((struct ptr *)_au3_t ))-> _pvtyp_typ ;
rx1 :
#line 1229 "../../src/dcl3.c"
switch (_au3_t -> _node_base ){ 
#line 1230 "../../src/dcl3.c"
case 97 : _au3_t = (((struct basetype *)_au3_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 1230 "../../src/dcl3.c"
goto rx1 ;
case 119 : _au3_an = (((struct basetype *)_au3_t ))-> _basetype_b_name ;
}
}
if (_au3_an && (_au3_an == _au1_class_name ))(((struct classdef *)_au3_an -> _expr__O2.__C2_tp ))-> _classdef_bit_ass = 0 ;
}
else 
#line 1237 "../../src/dcl3.c"
if (_au1_f -> _fct_nargs == 2 ){ 
#line 1238 "../../src/dcl3.c"
Ptype _au3_t ;
Pname _au3_an1 ;

#line 1238 "../../src/dcl3.c"
_au3_t = _au1_f -> _fct_argtype -> _expr__O2.__C2_tp ;

#line 1240 "../../src/dcl3.c"
if (_type_is_ref ( _au3_t ) ){ 
#line 1241 "../../src/dcl3.c"
_au3_t = (((struct ptr *)_au3_t ))-> _pvtyp_typ ;
rx2 :
#line 1243 "../../src/dcl3.c"
switch (_au3_t -> _node_base ){ 
#line 1244 "../../src/dcl3.c"
case 97 : _au3_t = (((struct basetype *)_au3_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 1244 "../../src/dcl3.c"
goto rx2 ;
case 119 : _au3_an1 = (((struct basetype *)_au3_t ))-> _basetype_b_name ;
}
}
_au3_t = _au1_f -> _fct_argtype -> _name_n_list -> _expr__O2.__C2_tp ;
{ Pname _au3_an2 ;

#line 1249 "../../src/dcl3.c"
_au3_an2 = _type_is_cl_obj ( _au3_t ) ;
if ((_au3_an2 == 0 )&& _type_is_ref ( _au3_t ) ){ 
#line 1251 "../../src/dcl3.c"
_au3_t = (((struct ptr *)_au3_t ))-> _pvtyp_typ ;
rx3 :
#line 1253 "../../src/dcl3.c"
switch (_au3_t -> _node_base ){ 
#line 1254 "../../src/dcl3.c"
case 97 : _au3_t = (((struct basetype *)_au3_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 1254 "../../src/dcl3.c"
goto rx3 ;
case 119 : _au3_an2 = (((struct basetype *)_au3_t ))-> _basetype_b_name ;
}
}
if (_au3_an1 && (_au3_an1 == _au3_an2 ))(((struct classdef *)_au3_an1 -> _expr__O2.__C2_tp ))-> _classdef_bit_ass = 0 ;
}
}

#line 1260 "../../src/dcl3.c"
default : 
#line 1261 "../../src/dcl3.c"
_au1_can_overload = 1 ;
}

#line 1264 "../../src/dcl3.c"
switch (_au0_scope ){ 
#line 1265 "../../src/dcl3.c"
case 108 : 
#line 1266 "../../src/dcl3.c"
case 136 : 
#line 1267 "../../src/dcl3.c"
{ Pname _au3_nx ;

#line 1267 "../../src/dcl3.c"
_au3_nx = _table_insert ( gtbl , _au0_this , (unsigned char )0 ) ;
_au0_this -> _expr__O5.__C5_n_table = 0 ;
_au0_this -> _name_n_tbl_list = 0 ;
if (Nold && _type_check ( _au0_this -> _expr__O2.__C2_tp , _au3_nx -> _expr__O2.__C2_tp , (unsigned char )0 ) ){ 
#line 1556 "../../src/dcl3.c"
struct ea _au0__V98 ;

#line 1556 "../../src/dcl3.c"
struct ea _au0__V99 ;

#line 1556 "../../src/dcl3.c"
struct ea _au0__V100 ;

#line 1270 "../../src/dcl3.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"%n has been declared both as%t and as%t", (struct ea *)( ( ((& _au0__V98 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((&
#line 1270 "../../src/dcl3.c"
_au0__V98 )))) ) , (struct ea *)( ( ((& _au0__V99 )-> _ea__O1.__C1_p = ((char *)_au3_nx -> _expr__O2.__C2_tp )), (((& _au0__V99 ))))
#line 1270 "../../src/dcl3.c"
) , (struct ea *)( ( ((& _au0__V100 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _expr__O2.__C2_tp )), (((& _au0__V100 )))) )
#line 1270 "../../src/dcl3.c"
, (struct ea *)ea0 ) ;
} }
}

#line 1274 "../../src/dcl3.c"
{ Pname _au1_nn ;

#line 1274 "../../src/dcl3.c"
_au1_nn = _table_insert ( _au1_etbl , _au0_this , (unsigned char )0 ) ;
_name_assign ( _au1_nn ) ;
_au0_this -> _expr__O5.__C5_n_table = _au1_etbl ;

#line 1278 "../../src/dcl3.c"
if (Nold ){ 
#line 1279 "../../src/dcl3.c"
Pfct _au2_nf ;

#line 1279 "../../src/dcl3.c"
_au2_nf = (((struct fct *)_au1_nn -> _expr__O2.__C2_tp ));

#line 1281 "../../src/dcl3.c"
if ((_au2_nf -> _node_base == 141 )|| (_au1_f -> _node_base == 141 ))
#line 1282 "../../src/dcl3.c"
;
else if (_au2_nf -> _node_base == 76 ){ 
#line 1284 "../../src/dcl3.c"
Pgen _au3_g ;

#line 1284 "../../src/dcl3.c"
_au3_g = (((struct gen *)_au2_nf ));
_au1_nn = _gen_add ( _au3_g , _au0_this , (int )0 ) ;
_au0_this -> _expr__O3.__C3_string = _au1_nn -> _expr__O3.__C3_string ;
if (Nold == 0 ){ 
#line 1288 "../../src/dcl3.c"
if (_au1_f -> _fct_body ){ 
#line 1289 "../../src/dcl3.c"
if (_au0_this -> _name__O6.__C6_n_qualifier ){ 
#line 1290 "../../src/dcl3.c"
{ 
#line 1556 "../../src/dcl3.c"
struct ea _au0__V101 ;

#line 1556 "../../src/dcl3.c"
struct ea _au0__V102 ;

#line 1290 "../../src/dcl3.c"
error ( (char *)"badAL for overloaded %n::%s()", (struct ea *)( ( ((& _au0__V101 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _name__O6.__C6_n_qualifier )), (((& _au0__V101 ))))
#line 1290 "../../src/dcl3.c"
) , (struct ea *)( ( ((& _au0__V102 )-> _ea__O1.__C1_p = ((char *)_au3_g -> _gen_string )), (((& _au0__V102 )))) )
#line 1290 "../../src/dcl3.c"
, (struct ea *)ea0 , (struct ea *)ea0 ) ;
return (struct name *)0 ;
} }
}

#line 1296 "../../src/dcl3.c"
goto thth ;
}
else { 
#line 1299 "../../src/dcl3.c"
if ((_au1_f -> _fct_body == 0 )&& (friend_in_class == 0 )){ 
#line 1556 "../../src/dcl3.c"
struct ea _au0__V103 ;

#line 1299 "../../src/dcl3.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"overloaded%n redeclared", (struct ea *)( ( ((& _au0__V103 )-> _ea__O1.__C1_p = ((char *)_au1_nn )), (((&
#line 1299 "../../src/dcl3.c"
_au0__V103 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }

#line 1302 "../../src/dcl3.c"
_au2_nf = (((struct fct *)_au1_nn -> _expr__O2.__C2_tp ));

#line 1304 "../../src/dcl3.c"
if (_au1_f -> _fct_body && _au2_nf -> _fct_body ){ 
#line 1305 "../../src/dcl3.c"
{ 
#line 1556 "../../src/dcl3.c"
struct ea _au0__V104 ;

#line 1305 "../../src/dcl3.c"
error ( (char *)"two definitions of overloaded%n", (struct ea *)( ( ((& _au0__V104 )-> _ea__O1.__C1_p = ((char *)_au1_nn )), (((& _au0__V104 )))) )
#line 1305 "../../src/dcl3.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
return (struct name *)0 ;
} }

#line 1309 "../../src/dcl3.c"
if (_au1_f -> _fct_body )goto bdbd ;

#line 1311 "../../src/dcl3.c"
goto stst ;
}
else if (_au2_nf -> _node_base != 108 ){ 
#line 1314 "../../src/dcl3.c"
{ 
#line 1556 "../../src/dcl3.c"
struct ea _au0__V105 ;

#line 1556 "../../src/dcl3.c"
struct ea _au0__V106 ;

#line 1314 "../../src/dcl3.c"
error ( (char *)"%n declared both as%t and asF", (struct ea *)( ( ((& _au0__V105 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V105 )))) )
#line 1314 "../../src/dcl3.c"
, (struct ea *)( ( ((& _au0__V106 )-> _ea__O1.__C1_p = ((char *)_au2_nf )), (((& _au0__V106 )))) ) , (struct
#line 1314 "../../src/dcl3.c"
ea *)ea0 , (struct ea *)ea0 ) ;
_au1_f -> _fct_body = 0 ;
} }
else if (_au1_can_overload ){ 
#line 1318 "../../src/dcl3.c"
if (_type_check ( (struct type *)_au2_nf , (struct type *)_au1_f , (unsigned char )76 ) ||
#line 1318 "../../src/dcl3.c"
vrp_equiv ){ 
#line 1319 "../../src/dcl3.c"
if (_au1_f -> _fct_body && _au0_this -> _name__O6.__C6_n_qualifier ){ 
#line 1320 "../../src/dcl3.c"
{ 
#line 1556 "../../src/dcl3.c"
struct ea _au0__V107 ;

#line 1320 "../../src/dcl3.c"
error ( (char *)"badT for%n", (struct ea *)( ( ((& _au0__V107 )-> _ea__O1.__C1_p = ((char *)_au1_nn )), (((& _au0__V107 )))) )
#line 1320 "../../src/dcl3.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
return (struct name *)0 ;
} }
{ Pgen _au4_g ;
Pname _au4_n1 ;
Pname _au4_n2 ;

#line 1323 "../../src/dcl3.c"
_au4_g = (struct gen *)_gen__ctor ( (struct gen *)0 , _au0_this -> _expr__O3.__C3_string ) ;
_au4_n1 = _gen_add ( _au4_g , _au1_nn , in_class_dcl ) ;
_au4_n2 = _gen_add ( _au4_g , _au0_this , (int )0 ) ;
_au1_nn -> _expr__O2.__C2_tp = (((struct type *)_au4_g ));
_au1_nn -> _expr__O3.__C3_string = _au4_g -> _gen_string ;
_au1_nn = _au4_n2 ;
goto thth ;
}
}
if (in_class_dcl ){ 
#line 1333 "../../src/dcl3.c"
{ 
#line 1556 "../../src/dcl3.c"
struct ea _au0__V108 ;

#line 1333 "../../src/dcl3.c"
error ( (char *)"twoDs of%n", (struct ea *)( ( ((& _au0__V108 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V108 )))) )
#line 1333 "../../src/dcl3.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
_au1_f -> _fct_body = 0 ;
return (struct name *)0 ;
} }

#line 1338 "../../src/dcl3.c"
if (_au2_nf -> _fct_body && _au1_f -> _fct_body ){ 
#line 1339 "../../src/dcl3.c"
{ 
#line 1556 "../../src/dcl3.c"
struct ea _au0__V109 ;

#line 1339 "../../src/dcl3.c"
error ( (char *)"two definitions of%n", (struct ea *)( ( ((& _au0__V109 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V109 )))) )
#line 1339 "../../src/dcl3.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
_au1_f -> _fct_body = 0 ;
return (struct name *)0 ;
} }

#line 1344 "../../src/dcl3.c"
if (_au1_f -> _fct_body )goto bdbd ;

#line 1346 "../../src/dcl3.c"
goto stst ;
}
else if (_type_check ( (struct type *)_au2_nf , (struct type *)_au1_f , (unsigned char )0 ) ){ 
#line 1349 "../../src/dcl3.c"
switch (_au0_this -> _name_n_oper ){
#line 1349 "../../src/dcl3.c"

#line 1350 "../../src/dcl3.c"
case 161 : 
#line 1351 "../../src/dcl3.c"
case 162 : 
#line 1352 "../../src/dcl3.c"
_au1_f -> _fct_s_returns = _au2_nf -> _fct_s_returns ;
}
{ 
#line 1556 "../../src/dcl3.c"
struct ea _au0__V110 ;

#line 1556 "../../src/dcl3.c"
struct ea _au0__V111 ;

#line 1556 "../../src/dcl3.c"
struct ea _au0__V112 ;

#line 1354 "../../src/dcl3.c"
error ( (char *)"%nT mismatch:%t and%t", (struct ea *)( ( ((& _au0__V110 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V110 )))) )
#line 1354 "../../src/dcl3.c"
, (struct ea *)( ( ((& _au0__V111 )-> _ea__O1.__C1_p = ((char *)_au2_nf )), (((& _au0__V111 )))) ) , (struct
#line 1354 "../../src/dcl3.c"
ea *)( ( ((& _au0__V112 )-> _ea__O1.__C1_p = ((char *)_au1_f )), (((& _au0__V112 )))) ) , (struct ea *)ea0 ) ;
#line 1354 "../../src/dcl3.c"

#line 1355 "../../src/dcl3.c"
_au1_f -> _fct_body = 0 ;
} }
else if (_au2_nf -> _fct_body && _au1_f -> _fct_body ){ 
#line 1358 "../../src/dcl3.c"
{ 
#line 1556 "../../src/dcl3.c"
struct ea _au0__V113 ;

#line 1358 "../../src/dcl3.c"
error ( (char *)"two definitions of%n", (struct ea *)( ( ((& _au0__V113 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V113 )))) )
#line 1358 "../../src/dcl3.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
_au1_f -> _fct_body = 0 ;
} }
else if (_au1_f -> _fct_body ){ 
#line 1362 "../../src/dcl3.c"
bdbd :
#line 1363 "../../src/dcl3.c"
if (_au1_f -> _fct_nargs_known && _au2_nf -> _fct_nargs_known )merge_init ( _au1_nn , _au1_f , _au2_nf ) ;
#line 1363 "../../src/dcl3.c"

#line 1364 "../../src/dcl3.c"
_au1_f -> _fct_f_virtual = _au2_nf -> _fct_f_virtual ;
_au1_f -> _fct_f_this = _au2_nf -> _fct_f_this ;
_au1_f -> _fct_f_result = _au2_nf -> _fct_f_result ;
_au1_f -> _fct_s_returns = _au2_nf -> _fct_s_returns ;

#line 1369 "../../src/dcl3.c"
_au1_nn -> _expr__O2.__C2_tp = (struct type *)_au1_f ;
if (_au1_f -> _fct_f_inline ){ 
#line 1371 "../../src/dcl3.c"
if (_au2_nf -> _fct_f_inline == 0 ){ 
#line 1372 "../../src/dcl3.c"
if (_au1_nn -> _name_n_used ){ 
#line 1556 "../../src/dcl3.c"
struct ea _au0__V114 ;

#line 1372 "../../src/dcl3.c"
error ( (char *)"%n called before defined as inline", (struct ea *)( ( ((& _au0__V114 )-> _ea__O1.__C1_p = ((char *)_au1_nn )), (((& _au0__V114 )))) )
#line 1372 "../../src/dcl3.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} _au1_f -> _fct_f_inline = (_au2_nf -> _fct_f_inline = 2 );
}
else 
#line 1379 "../../src/dcl3.c"
_au2_nf -> _fct_f_inline = 1 ;
_au1_nn -> _name_n_sto = 31 ;
}
else if (_au2_nf -> _fct_f_inline ){ 
#line 1384 "../../src/dcl3.c"
_au1_f -> _fct_f_inline = 1 ;
}
goto stst2 ;
}
else { 
#line 1389 "../../src/dcl3.c"
_au1_f -> _fct_f_this = _au2_nf -> _fct_f_this ;
_au1_f -> _fct_f_result = _au2_nf -> _fct_f_result ;
_au1_f -> _fct_s_returns = _au2_nf -> _fct_s_returns ;
stst :
#line 1393 "../../src/dcl3.c"
if (_au1_f -> _fct_nargs_known && _au2_nf -> _fct_nargs_known )merge_init ( _au1_nn , _au1_f , _au2_nf ) ;
stst2 :
#line 1395 "../../src/dcl3.c"
if (_au1_f -> _fct_f_inline )_au0_this -> _name_n_sto = 31 ;
if (((_au0_this -> _name_n_sto && (_au1_nn -> _name_n_scope != _au0_this -> _name_n_sto ))&& (friend_in_class == 0 ))&& (_au1_f -> _fct_f_inline == 0 ))
#line 1399 "../../src/dcl3.c"
{ 
#line 1400 "../../src/dcl3.c"
switch (_au0_this ->
#line 1400 "../../src/dcl3.c"
_name_n_sto ){ 
#line 1401 "../../src/dcl3.c"
case 31 : 
#line 1402 "../../src/dcl3.c"
_au1_nn -> _name_n_sto = 31 ;
break ;
case 14 : 
#line 1405 "../../src/dcl3.c"
if ((_au1_nn -> _name_n_scope == 25 )|| (_au1_nn -> _name_n_scope == 0 ))
#line 1407 "../../src/dcl3.c"
break ;
default : 
#line 1409 "../../src/dcl3.c"
{ 
#line 1556 "../../src/dcl3.c"
struct ea _au0__V115 ;

#line 1556 "../../src/dcl3.c"
struct ea _au0__V116 ;

#line 1556 "../../src/dcl3.c"
struct ea _au0__V117 ;

#line 1409 "../../src/dcl3.c"
error ( (char *)"%n both%k and%k", (struct ea *)( ( ((& _au0__V115 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V115 )))) )
#line 1409 "../../src/dcl3.c"
, (struct ea *)( ( ((& _au0__V116 )-> _ea__O1.__C1_i = ((int )_au0_this -> _name_n_sto )), (((& _au0__V116 )))) ) ,
#line 1409 "../../src/dcl3.c"
(struct ea *)( ( ((& _au0__V117 )-> _ea__O1.__C1_i = ((int )_au1_nn -> _name_n_scope )), (((& _au0__V117 )))) ) , (struct
#line 1409 "../../src/dcl3.c"
ea *)ea0 ) ;
} }
}
_au0_this -> _name_n_scope = _au1_nn -> _name_n_scope ;
_au0_this -> _name_n_sto = _au1_nn -> _name_n_sto ;
}
}
else 
#line 1418 "../../src/dcl3.c"
{ 
#line 1419 "../../src/dcl3.c"
if ((_au0_tbl == gtbl )&& _au0_this -> _name_n_oper ){ 
#line 1420 "../../src/dcl3.c"
Pgen _au3_g ;
Pname _au3_n1 ;

#line 1420 "../../src/dcl3.c"
_au3_g = (struct gen *)_gen__ctor ( (struct gen *)0 , _au0_this -> _expr__O3.__C3_string ) ;
_au3_n1 = _gen_add ( _au3_g , _au1_nn , 1 ) ;
_au1_nn -> _expr__O2.__C2_tp = (((struct type *)_au3_g ));
_au1_nn -> _expr__O3.__C3_string = _au3_g -> _gen_string ;
_au0_this -> _expr__O3.__C3_string = _au3_n1 -> _expr__O3.__C3_string ;
_au1_nn = _au3_n1 ;
}
thth :
#line 1428 "../../src/dcl3.c"
_au1_just_made = 1 ;
if (_au1_f -> _fct_f_inline )
#line 1430 "../../src/dcl3.c"
_au1_nn -> _name_n_sto = 31 ;
else if (((_au1_class_name == 0 )&& (_au0_this -> _name_n_sto == 0 ))&& (_au1_f -> _fct_body == 0 ))
#line 1432 "../../src/dcl3.c"
_au1_nn -> _name_n_sto = 14 ;

#line 1434 "../../src/dcl3.c"
if (_au1_class_name && (_au1_etbl != gtbl )){ 
#line 1435 "../../src/dcl3.c"
Pname _au3_cn ;
Pname _au3_tt ;

#line 1435 "../../src/dcl3.c"
_au3_cn = _au1_nn -> _expr__O5.__C5_n_table -> _table_t_name ;
_au3_tt = (struct name *)_name__ctor ( (struct name *)0 , "this") ;
_au3_tt -> _name_n_scope = 136 ;
_au3_tt -> _name_n_sto = 136 ;
_au3_tt -> _expr__O2.__C2_tp = (((struct classdef *)_au1_class_name -> _expr__O2.__C2_tp ))-> _classdef_this_type ;
_au3_tt -> _node_permanent = 1 ;
(((struct fct *)_au1_nn -> _expr__O2.__C2_tp ))-> _fct_f_this = (_au1_f -> _fct_f_this = _au3_tt );
_au3_tt -> _name_n_list = _au1_f -> _fct_argtype ;
}

#line 1445 "../../src/dcl3.c"
if (_au1_f -> _fct_f_result == 0 ){ 
#line 1446 "../../src/dcl3.c"
Pname _au3_rcln ;

#line 1446 "../../src/dcl3.c"
_au3_rcln = _type_is_cl_obj ( _au1_f -> _fct_returns ) ;
if (_au3_rcln && ( (((struct classdef *)_au3_rcln -> _expr__O2.__C2_tp ))-> _classdef_itor ) )make_res ( _au1_f ) ;
}
else if (_au1_f -> _fct_f_this )
#line 1450 "../../src/dcl3.c"
_au1_f -> _fct_f_this -> _name_n_list = _au1_f -> _fct_f_result ;

#line 1452 "../../src/dcl3.c"
if (_au1_f -> _fct_f_virtual ){ 
#line 1453 "../../src/dcl3.c"
switch (_au1_nn -> _name_n_scope ){ 
#line 1454 "../../src/dcl3.c"
default : 
#line 1455 "../../src/dcl3.c"
{ 
#line 1556 "../../src/dcl3.c"
struct ea _au0__V118 ;

#line 1455 "../../src/dcl3.c"
error ( (char *)"nonC virtual%n", (struct ea *)( ( ((& _au0__V118 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((& _au0__V118 )))) )
#line 1455 "../../src/dcl3.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
break ;
case 0 : 
#line 1458 "../../src/dcl3.c"
case 25 : 
#line 1459 "../../src/dcl3.c"
cc -> _dcl_context_cot -> _classdef_virt_count = 1 ;
(((struct fct *)_au1_nn -> _expr__O2.__C2_tp ))-> _fct_f_virtual = 1 ;
break ;
} }
}
}

#line 1469 "../../src/dcl3.c"
switch (_au0_this -> _name_n_oper ){ 
#line 1470 "../../src/dcl3.c"
case 161 : 
#line 1471 "../../src/dcl3.c"
if (_au1_f -> _fct_nargs == 1 ){ 
#line 1472 "../../src/dcl3.c"
Ptype _au3_t ;

#line 1472 "../../src/dcl3.c"
_au3_t = _au1_f -> _fct_argtype -> _expr__O2.__C2_tp ;
clll :
#line 1474 "../../src/dcl3.c"
switch (_au3_t -> _node_base ){ 
#line 1475 "../../src/dcl3.c"
case 97 : 
#line 1476 "../../src/dcl3.c"
_au3_t = (((struct basetype *)_au3_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
goto clll ;
case 158 : 
#line 1479 "../../src/dcl3.c"
_au3_t = (((struct ptr *)_au3_t ))-> _pvtyp_typ ;
cxll :
#line 1481 "../../src/dcl3.c"
switch (_au3_t -> _node_base ){ 
#line 1482 "../../src/dcl3.c"
case 97 : 
#line 1483 "../../src/dcl3.c"
_au3_t = (((struct basetype *)_au3_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
goto cxll ;
case 119 : 
#line 1486 "../../src/dcl3.c"
if (_au1_class_name == (((struct basetype *)_au3_t ))-> _basetype_b_name )
#line 1487 "../../src/dcl3.c"
(((struct classdef *)_au1_class_name -> _expr__O2.__C2_tp ))-> _classdef_itor = _au1_nn ;
}
break ;
case 119 : 
#line 1491 "../../src/dcl3.c"
if (_au1_class_name == (((struct basetype *)_au3_t ))-> _basetype_b_name )
#line 1492 "../../src/dcl3.c"
{ 
#line 1556 "../../src/dcl3.c"
struct ea _au0__V119 ;

#line 1556 "../../src/dcl3.c"
struct ea _au0__V120 ;

#line 1492 "../../src/dcl3.c"
error ( (char *)"impossibleK: %s(%s)", (struct ea *)( ( ((& _au0__V119 )-> _ea__O1.__C1_p = ((char *)_au1_class_name -> _expr__O3.__C3_string )), (((& _au0__V119 ))))
#line 1492 "../../src/dcl3.c"
) , (struct ea *)( ( ((& _au0__V120 )-> _ea__O1.__C1_p = ((char *)_au1_class_name -> _expr__O3.__C3_string )), (((& _au0__V120 )))) )
#line 1492 "../../src/dcl3.c"
, (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}
break ;
case 97 : 
#line 1498 "../../src/dcl3.c"
if (_au1_just_made ){ 
#line 1499 "../../src/dcl3.c"
_au1_nn -> _name_n_list = (((struct classdef *)_au1_class_name -> _expr__O2.__C2_tp ))-> _classdef_conv ;
(((struct classdef *)_au1_class_name -> _expr__O2.__C2_tp ))-> _classdef_conv = _au1_nn ;
}
break ;
case 162 : 
#line 1504 "../../src/dcl3.c"
case 23 : 
#line 1505 "../../src/dcl3.c"
case 9 : 
#line 1506 "../../src/dcl3.c"
case 109 : 
#line 1507 "../../src/dcl3.c"
case 0 : 
#line 1508 "../../src/dcl3.c"
break ;
default : 
#line 1510 "../../src/dcl3.c"
if (_au1_f -> _fct_nargs_known != 1 ){ 
#line 1511 "../../src/dcl3.c"
{ 
#line 1556 "../../src/dcl3.c"
struct ea _au0__V121 ;

#line 1511 "../../src/dcl3.c"
error ( (char *)"ATs must be fully specified for%n", (struct ea *)( ( ((& _au0__V121 )-> _ea__O1.__C1_p = ((char *)_au1_nn )), (((& _au0__V121 )))) )
#line 1511 "../../src/dcl3.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
else if (_au1_class_name == 0 ){ 
#line 1514 "../../src/dcl3.c"
Pname _au3_a ;
switch (_au1_f -> _fct_nargs ){ 
#line 1516 "../../src/dcl3.c"
case 1 : 
#line 1517 "../../src/dcl3.c"
case 2 : 
#line 1518 "../../src/dcl3.c"
for(_au3_a = _au1_f -> _fct_argtype ;_au3_a ;_au3_a = _au3_a -> _name_n_list ) { 
#line 1519 "../../src/dcl3.c"
Ptype _au5_tx ;
#line 1519 "../../src/dcl3.c"
_au5_tx = _au3_a -> _expr__O2.__C2_tp ;
if (_au5_tx -> _node_base == 158 )_au5_tx = (((struct ptr *)_au5_tx ))-> _pvtyp_typ ;
if (_type_is_cl_obj ( _au5_tx ) )goto cok ;
}
{ 
#line 1556 "../../src/dcl3.c"
struct ea _au0__V122 ;

#line 1523 "../../src/dcl3.c"
error ( (char *)"%n must take at least oneCTA", (struct ea *)( ( ((& _au0__V122 )-> _ea__O1.__C1_p = ((char *)_au1_nn )), (((& _au0__V122 )))) )
#line 1523 "../../src/dcl3.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
break ;
default : 
#line 1526 "../../src/dcl3.c"
{ 
#line 1556 "../../src/dcl3.c"
struct ea _au0__V123 ;

#line 1526 "../../src/dcl3.c"
error ( (char *)"%n must take 1 or 2As", (struct ea *)( ( ((& _au0__V123 )-> _ea__O1.__C1_p = ((char *)_au1_nn )), (((& _au0__V123 )))) )
#line 1526 "../../src/dcl3.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} } }
}
else { 
#line 1530 "../../src/dcl3.c"
switch (_au1_f -> _fct_nargs ){ 
#line 1531 "../../src/dcl3.c"
case 0 : 
#line 1532 "../../src/dcl3.c"
case 1 : 
#line 1533 "../../src/dcl3.c"
break ;
default : 
#line 1535 "../../src/dcl3.c"
{ 
#line 1556 "../../src/dcl3.c"
struct ea _au0__V124 ;

#line 1535 "../../src/dcl3.c"
error ( (char *)"%n must take 0 or 1As", (struct ea *)( ( ((& _au0__V124 )-> _ea__O1.__C1_p = ((char *)_au1_nn )), (((& _au0__V124 )))) )
#line 1535 "../../src/dcl3.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}
cok :;
}

#line 1541 "../../src/dcl3.c"
{ int _au1_i ;

#line 1541 "../../src/dcl3.c"
_au1_i = 0 ;

#line 1543 "../../src/dcl3.c"
{ Pname _au1_a ;

#line 1543 "../../src/dcl3.c"
_au1_a = _au1_f -> _fct_argtype ;

#line 1543 "../../src/dcl3.c"
for(;_au1_a ;_au1_a = _au1_a -> _name_n_list ) { 
#line 1544 "../../src/dcl3.c"
if (_au1_a -> _expr__O4.__C4_n_initializer )
#line 1545 "../../src/dcl3.c"
_au1_i = 1 ;
else if (_au1_i )
#line 1547 "../../src/dcl3.c"
{ 
#line 1556 "../../src/dcl3.c"
struct ea _au0__V125 ;

#line 1547 "../../src/dcl3.c"
error ( (char *)"trailingA%n withoutIr", (struct ea *)( ( ((& _au0__V125 )-> _ea__O1.__C1_p = ((char *)_au1_a )), (((& _au0__V125 )))) )
#line 1547 "../../src/dcl3.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }

#line 1554 "../../src/dcl3.c"
if (_au1_f -> _fct_body )_fct_dcl ( _au1_f , _au1_nn ) ;
return _au1_nn ;
}
}
}
}
;

/* the end */
