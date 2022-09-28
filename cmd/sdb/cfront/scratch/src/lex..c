/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#line 1 "../../src/lex.c"

/* <<cfront 1.2.1 2/16/87>> */
/* < ../../src/lex.c */

#ident	"@(#)sdb:cfront/scratch/src/lex..c	1.2"
char *_new(); char _delete(); char *_vec_new(); char _vec_delete();

#line 1 "../../src/lex.c"

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

#line 78 "../../src/lex.c"
static char inbuf [49152];
char *txtmax = (& (inbuf [49151]));
char *txtstart = 0 ;
char *txtfree = 0 ;

#line 86 "../../src/lex.c"
static char *file_name [508];

#line 89 "../../src/lex.c"
static short file_stack [127];
int curr_file = 0 ;

#line 93 "../../src/lex.c"
struct loc curloc ;
FILE *out_file = stdout;
FILE *in_file = stdin;
Ptable ktbl ;
int br_level = 0 ;
int bl_level = 0 ;

#line 112 "../../src/lex.c"
static short lxmask [129];

#line 114 "../../src/lex.c"
int saved = 0 ;
extern int lxtitle ();

#line 118 "../../src/lex.c"

#line 119 "../../src/lex.c"

#line 120 "../../src/lex.c"

#line 121 "../../src/lex.c"

#line 131 "../../src/lex.c"
extern char ktbl_init ()
#line 136 "../../src/lex.c"
{ 
#line 137 "../../src/lex.c"
ktbl = (struct table *)_table__ctor ( (struct table *)0 , (short )123 , (struct table *)0 , (struct name *)0 )
#line 137 "../../src/lex.c"
;

#line 139 "../../src/lex.c"
new_key ( "asm", (unsigned char )1 , (unsigned char )0 ) ;
new_key ( "auto", (unsigned char )2 , (unsigned char )97 ) ;
new_key ( "break", (unsigned char )143 , (unsigned char )3 ) ;
new_key ( "case", (unsigned char )143 , (unsigned char )4 ) ;
new_key ( "continue", (unsigned char )143 , (unsigned char )7 ) ;
new_key ( "char", (unsigned char )5 , (unsigned char )97 ) ;
new_key ( "do", (unsigned char )143 , (unsigned char )10 ) ;
new_key ( "double", (unsigned char )11 , (unsigned char )97 ) ;
new_key ( "default", (unsigned char )143 , (unsigned char )8 ) ;
new_key ( "enum", (unsigned char )13 , (unsigned char )0 ) ;
new_key ( "else", (unsigned char )143 , (unsigned char )12 ) ;
new_key ( "extern", (unsigned char )14 , (unsigned char )97 ) ;
new_key ( "float", (unsigned char )15 , (unsigned char )97 ) ;
new_key ( "for", (unsigned char )143 , (unsigned char )16 ) ;

#line 154 "../../src/lex.c"
new_key ( "goto", (unsigned char )143 , (unsigned char )19 ) ;
new_key ( "if", (unsigned char )143 , (unsigned char )20 ) ;
new_key ( "int", (unsigned char )21 , (unsigned char )97 ) ;
new_key ( "long", (unsigned char )22 , (unsigned char )97 ) ;
new_key ( "return", (unsigned char )143 , (unsigned char )28 ) ;
new_key ( "register", (unsigned char )27 , (unsigned char )97 ) ;
new_key ( "static", (unsigned char )31 , (unsigned char )97 ) ;
new_key ( "struct", (unsigned char )32 , (unsigned char )156 ) ;
new_key ( "sizeof", (unsigned char )30 , (unsigned char )0 ) ;
new_key ( "short", (unsigned char )29 , (unsigned char )97 ) ;
new_key ( "switch", (unsigned char )143 , (unsigned char )33 ) ;
new_key ( "typedef", (unsigned char )35 , (unsigned char )97 ) ;
new_key ( "unsigned", (unsigned char )37 , (unsigned char )97 ) ;
new_key ( "union", (unsigned char )36 , (unsigned char )156 ) ;
new_key ( "void", (unsigned char )38 , (unsigned char )97 ) ;
new_key ( "while", (unsigned char )143 , (unsigned char )39 ) ;

#line 171 "../../src/lex.c"
new_key ( "class", (unsigned char )6 , (unsigned char )156 ) ;
new_key ( "const", (unsigned char )26 , (unsigned char )97 ) ;
new_key ( "delete", (unsigned char )143 , (unsigned char )9 ) ;
new_key ( "friend", (unsigned char )18 , (unsigned char )97 ) ;
new_key ( "inline", (unsigned char )75 , (unsigned char )97 ) ;
new_key ( "new", (unsigned char )23 , (unsigned char )0 ) ;
new_key ( "operator", (unsigned char )24 , (unsigned char )0 ) ;
new_key ( "overload", (unsigned char )76 , (unsigned char )97 ) ;
new_key ( "private", (unsigned char )174 , (unsigned char )175 ) ;
new_key ( "protected", (unsigned char )79 , (unsigned char )175 ) ;
new_key ( "public", (unsigned char )25 , (unsigned char )175 ) ;
new_key ( "signed", (unsigned char )171 , (unsigned char )97 ) ;
new_key ( "this", (unsigned char )34 , (unsigned char )0 ) ;
new_key ( "virtual", (unsigned char )77 , (unsigned char )97 ) ;
new_key ( "volatile", (unsigned char )170 , (unsigned char )97 ) ;
}
;

#line 196 "../../src/lex.c"
extern char *src_file_name ;
extern char *line_format ;
struct loc last_line ;

#line 200 "../../src/lex.c"
char _loc_putline (_au0_this )
#line 76 "../../src/cfront.h"
struct loc *_au0_this ;

#line 201 "../../src/lex.c"
{ 
#line 202 "../../src/lex.c"
if ((_au0_this -> _loc_file == 0 )&& (_au0_this -> _loc_line == 0 ))return ;
if ((0 <= _au0_this -> _loc_file )&& (_au0_this -> _loc_file < 127 )){ 
#line 204 "../../src/lex.c"
char *_au2_f ;

#line 204 "../../src/lex.c"
_au2_f = (file_name [_au0_this -> _loc_file ]);
if (_au2_f == 0 )_au2_f = (src_file_name ? src_file_name : "");
fprintf ( out_file , (char *)line_format , _au0_this -> _loc_line , _au2_f ) ;
last_line = (*_au0_this );
}
}
;
char _loc_put (_au0_this , _au0_p )
#line 76 "../../src/cfront.h"
struct loc *_au0_this ;

#line 211 "../../src/lex.c"
FILE *_au0_p ;
{ 
#line 213 "../../src/lex.c"
if ((0 <= _au0_this -> _loc_file )&& (_au0_this -> _loc_file < 127 )){ 
#line 214 "../../src/lex.c"
char *_au2_f ;

#line 214 "../../src/lex.c"
_au2_f = (file_name [_au0_this -> _loc_file ]);
if (_au2_f == 0 )_au2_f = (src_file_name ? src_file_name : "");
fprintf ( _au0_p , (char *)"\"%s\", line %d: ", _au2_f , _au0_this -> _loc_line ) ;
}
}
;
char lxenter (_au0_s , _au0_m )register char *_au0_s ;

#line 220 "../../src/lex.c"
short _au0_m ;

#line 222 "../../src/lex.c"
{ 
#line 223 "../../src/lex.c"
register int _au1_c ;

#line 225 "../../src/lex.c"
while (_au1_c = (*(_au0_s ++ )))(lxmask [_au1_c + 1 ])|= _au0_m ;
}
;

#line 230 "../../src/lex.c"
char lxget (_au0_c , _au0_m )register int _au0_c ;

#line 230 "../../src/lex.c"
register int _au0_m ;

#line 236 "../../src/lex.c"
{ 
#line 237 "../../src/lex.c"
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct ea *)ea0 ,
#line 237 "../../src/lex.c"
(struct ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= _au0_c )));
while (( (_au0_c = getc(in_file)),
#line 238 "../../src/lex.c"
((lxmask [_au0_c + 1 ])& _au0_m )) )(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 ,
#line 238 "../../src/lex.c"
(struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= _au0_c )));
ungetc ( _au0_c , in_file ) ;
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 240 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= '\0' )));
}
;
struct LXDOPE {	/* sizeof LXDOPE == 6 */

#line 244 "../../src/lex.c"
short _LXDOPE_lxch ;
short _LXDOPE_lxact ;
TOK _LXDOPE_lxtok ;
};
struct LXDOPE lxdope [34]= { '$' , 0 , 0 , '_' , 1 , 0 , '0' , 2 , 0 , ' ' , 10 ,
#line 247 "../../src/lex.c"
0 , '\n' , 11 , 0 , '"' , 4 , 0 , '\'' , 5 , 0 , '`' , 6 , 0 , '(' ,
#line 247 "../../src/lex.c"
14 , 40 , ')' , 15 , 41 , '{' , 12 , 73 , '}' , 13 , 74 , '[' , 3 , 42 ,
#line 247 "../../src/lex.c"
']' , 3 , 43 , '*' , 26 , 50 , '?' , 3 , 68 , ':' , 28 , 69 , '+' , 27 ,
#line 247 "../../src/lex.c"
54 , '-' , 25 , 55 , '/' , 7 , 51 , '%' , 23 , 53 , '&' , 22 , 52 , '|' ,
#line 247 "../../src/lex.c"
21 , 65 , '^' , 20 , 64 , '!' , 24 , 46 , '~' , 3 , 47 , ',' , 3 , 71 ,
#line 247 "../../src/lex.c"
';' , 3 , 72 , '.' , 8 , 45 , '<' , 18 , 58 , '>' , 19 , 60 , '=' , 17 ,
#line 247 "../../src/lex.c"
70 , '#' , 29 , 0 ,  EOF, 16 , 0 } ;

#line 289 "../../src/lex.c"
static struct LXDOPE *lxcp [129];

#line 291 "../../src/lex.c"
extern char lex_init ();

#line 293 "../../src/lex.c"
extern char lex_init ()
#line 294 "../../src/lex.c"
{ 
#line 295 "../../src/lex.c"
register struct LXDOPE *_au1_p ;
register int _au1_i ;
register char *_au1_cp ;

#line 301 "../../src/lex.c"
for(_au1_i = 0 ;_au1_i <= 128 ;_au1_i ++ ) (lxmask [_au1_i ])= 0 ;

#line 306 "../../src/lex.c"
lxenter ( "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_", (short )01 ) ;

#line 308 "../../src/lex.c"
lxenter ( "0123456789", (short )02 ) ;
lxenter ( "0123456789abcdefABCDEF", (short )010 ) ;

#line 311 "../../src/lex.c"
lxenter ( " \t\r\b\f\013", (short )020 ) ;
(lxmask [47])|= 040 ;

#line 318 "../../src/lex.c"
for(_au1_i = 0 ;_au1_i <= 128 ;++ _au1_i ) (lxcp [_au1_i ])= lxdope ;

#line 322 "../../src/lex.c"
for(_au1_p = lxdope ;;++ _au1_p ) { 
#line 323 "../../src/lex.c"
(lxcp [_au1_p -> _LXDOPE_lxch + 1 ])= _au1_p ;
if (_au1_p -> _LXDOPE_lxch < 0 )break ;
}

#line 330 "../../src/lex.c"
_au1_cp = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
while (*_au1_cp )(lxcp [(*(_au1_cp ++ ))+ 1 ])= (& (lxdope [1 ]));
_au1_cp = "123456789";
while (*_au1_cp )(lxcp [(*(_au1_cp ++ ))+ 1 ])= (& (lxdope [2 ]));
_au1_cp = "\t\b\r\f\013";
while (*_au1_cp )(lxcp [(*(_au1_cp ++ ))+ 1 ])= (& (lxdope [3 ]));

#line 337 "../../src/lex.c"
(file_name [0 ])= src_file_name ;
curloc . _loc_file = 0 ;
curloc . _loc_line = 1 ;

#line 341 "../../src/lex.c"
ktbl_init ( ) ;

#line 343 "../../src/lex.c"
lex_clear ( ) ;

#line 345 "../../src/lex.c"
saved = lxtitle ( ) ;
}
;
extern char lex_clear ()
#line 349 "../../src/lex.c"
{ 
#line 350 "../../src/lex.c"
txtstart = (txtfree = inbuf );
}
;
int int_val (_au0_hex )char _au0_hex ;
{ 
#line 355 "../../src/lex.c"
switch (_au0_hex ){ 
#line 356 "../../src/lex.c"
case '0' : case '1' : case '2' : case '3' : case '4' : 
#line 357 "../../src/lex.c"
case
#line 357 "../../src/lex.c"
'5' : case '6' : case '7' : case '8' : case '9' : 
#line 358 "../../src/lex.c"
return (_au0_hex - '0' );
case 'a' : case 'b' : case 'c' : case 'd' : case 'e' : case 'f' : 
#line 360 "../../src/lex.c"
return ((_au0_hex -
#line 360 "../../src/lex.c"
'a' )+ 10 );
case 'A' : case 'B' : case 'C' : case 'D' : case 'E' : case 'F' : 
#line 362 "../../src/lex.c"
return ((_au0_hex -
#line 362 "../../src/lex.c"
'A' )+ 10 );
}
}
;
char hex_to_oct ()
#line 371 "../../src/lex.c"
{ 
#line 372 "../../src/lex.c"
int _au1_i ;
int _au1_c ;

#line 372 "../../src/lex.c"
_au1_i = 0 ;

#line 374 "../../src/lex.c"
_au1_c = getc(in_file);
if ((lxmask [_au1_c + 1 ])& 010 ){ 
#line 376 "../../src/lex.c"
_au1_i = int_val ( (char )_au1_c ) ;
_au1_c = getc(in_file);
if ((lxmask [_au1_c + 1 ])& 010 ){ 
#line 379 "../../src/lex.c"
_au1_i = ((_au1_i << 4 )+ int_val ( (char )_au1_c ) );
_au1_c = getc(in_file);
if ((lxmask [_au1_c + 1 ])& 010 ){ 
#line 382 "../../src/lex.c"
_au1_i = ((_au1_i << 4 )+ int_val ( (char )_au1_c ) );
}
else 
#line 385 "../../src/lex.c"
ungetc ( _au1_c , in_file ) ;
}
else 
#line 388 "../../src/lex.c"
ungetc ( _au1_c , in_file ) ;
}
else { 
#line 391 "../../src/lex.c"
error ( (char *)"hexadecimal digitX after \\x", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 391 "../../src/lex.c"

#line 392 "../../src/lex.c"
ungetc ( _au1_c , in_file ) ;
}

#line 395 "../../src/lex.c"
if (0777 < _au1_i )error ( (char *)"hexadecimal constant too large after \\x", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 395 "../../src/lex.c"

#line 397 "../../src/lex.c"
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 397 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= ('0' + (_au1_i >> 6 )))));
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 398 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= ('0' + ((_au1_i & 070 )>> 3 )))));
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 399 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= ('0' + (_au1_i & 7 )))));
}
;

#line 403 "../../src/lex.c"
char *chconst ()
#line 407 "../../src/lex.c"
{ 
#line 408 "../../src/lex.c"
register int _au1_c ;
int _au1_nch ;

#line 409 "../../src/lex.c"
_au1_nch = 0 ;

#line 411 "../../src/lex.c"
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 411 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= '\'' )));

#line 413 "../../src/lex.c"
for(;;) { 
#line 414 "../../src/lex.c"
if (SZ_INT < (_au1_nch ++ )){ 
#line 415 "../../src/lex.c"
error ( (char *)"char constant too long", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 415 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 ) ;
goto ex ;
}

#line 419 "../../src/lex.c"
switch (_au1_c = getc(in_file)){ 
#line 420 "../../src/lex.c"
case
#line 420 "../../src/lex.c"
'\'' : 
#line 421 "../../src/lex.c"
goto ex ;
case EOF: 
#line 423 "../../src/lex.c"
error ( (char *)"eof in char constant", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 423 "../../src/lex.c"

#line 424 "../../src/lex.c"
goto ex ;
case '\n' : 
#line 426 "../../src/lex.c"
error ( (char *)"newline in char constant", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 426 "../../src/lex.c"

#line 427 "../../src/lex.c"
goto ex ;
case '\\' : 
#line 429 "../../src/lex.c"
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct
#line 429 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= _au1_c )));
switch (_au1_c = getc(in_file)){ 
#line 431 "../../src/lex.c"
case
#line 431 "../../src/lex.c"
'\n' : 
#line 432 "../../src/lex.c"
++ curloc . _loc_line ;
default : 
#line 434 "../../src/lex.c"
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct
#line 434 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= _au1_c )));
break ;
case '0' : case '1' : case '2' : case '3' : case '4' : 
#line 437 "../../src/lex.c"
case '5' : case '6' :
#line 437 "../../src/lex.c"
case '7' : 
#line 438 "../../src/lex.c"
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct
#line 438 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= _au1_c )));
_au1_c = getc(in_file);
if (((lxmask [_au1_c + 1 ])& 02 )&& (_au1_c < '8' )){ 
#line 441 "../../src/lex.c"
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char
#line 441 "../../src/lex.c"
*)"input buffer overflow", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++
#line 441 "../../src/lex.c"
))= _au1_c )));
_au1_c = getc(in_file);
if (((lxmask [_au1_c + 1 ])& 02 )&& (_au1_c < '8' ))
#line 444 "../../src/lex.c"
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow",
#line 444 "../../src/lex.c"
(struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))=
#line 444 "../../src/lex.c"
_au1_c )));
else 
#line 446 "../../src/lex.c"
ungetc ( _au1_c , in_file ) ;
}
else 
#line 449 "../../src/lex.c"
ungetc ( _au1_c , in_file ) ;
break ;
case 'x' : 
#line 452 "../../src/lex.c"
hex_to_oct ( ) ;
break ;
}

#line 454 "../../src/lex.c"
;
break ;
default : 
#line 457 "../../src/lex.c"
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct
#line 457 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= _au1_c )));
}
}
ex :
#line 461 "../../src/lex.c"
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 461 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= '\'' )));
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 462 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= '\0' )));
return txtstart ;
}
;
char lxcom ()
#line 468 "../../src/lex.c"
{ 
#line 469 "../../src/lex.c"
register int _au1_c ;

#line 471 "../../src/lex.c"
for(;;) 
#line 472 "../../src/lex.c"
switch (_au1_c = getc(in_file)){
#line 472 "../../src/lex.c"

#line 473 "../../src/lex.c"
case EOF: 
#line 474 "../../src/lex.c"
error ( (char *)"eof in comment", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 474 "../../src/lex.c"

#line 475 "../../src/lex.c"
return ;
case '\n' : 
#line 477 "../../src/lex.c"
curloc . _loc_line ++ ;
Nline ++ ;
break ;
case '*' : 
#line 481 "../../src/lex.c"
if ((_au1_c = getc(in_file))== '/' )return ;
ungetc ( _au1_c , in_file ) ;
break ;
case '/' : 
#line 485 "../../src/lex.c"
if ((_au1_c = getc(in_file))== '*' )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"``/*'' in comment", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 )
#line 485 "../../src/lex.c"
;
ungetc ( _au1_c , in_file ) ;
break ;
}
}
;

#line 492 "../../src/lex.c"
char linecom ()
#line 494 "../../src/lex.c"
{ 
#line 495 "../../src/lex.c"
register int _au1_c ;

#line 497 "../../src/lex.c"
for(;;) 
#line 498 "../../src/lex.c"
switch (_au1_c = getc(in_file)){
#line 498 "../../src/lex.c"

#line 499 "../../src/lex.c"
case EOF: 
#line 500 "../../src/lex.c"
error ( (char *)"eof in comment", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 500 "../../src/lex.c"

#line 501 "../../src/lex.c"
return ;
case '\n' : 
#line 503 "../../src/lex.c"
curloc . _loc_line ++ ;
Nline ++ ;
saved = lxtitle ( ) ;
return ;
}
}
;

#line 511 "../../src/lex.c"
extern char tlex ()
#line 512 "../../src/lex.c"
{ 
#line 513 "../../src/lex.c"
TOK _au1_ret ;
Pname _au1_n ;

#line 516 "../../src/lex.c"
Ntoken ++ ;

#line 518 "../../src/lex.c"
for(;;) { 
#line 519 "../../src/lex.c"
register int _au2_lxchar ;
register struct LXDOPE *_au2_p ;

#line 521 "../../src/lex.c"
union _C10 _au1__Xy_rtFUI___global ;

#line 521 "../../src/lex.c"
union _C10 _au1__Xy_rt_global ;

#line 521 "../../src/lex.c"
char *_au0__Xx_rt_global ;

#line 521 "../../src/lex.c"
union _C10 _au1__Xy_rtFCloc____global ;

#line 522 "../../src/lex.c"
txtstart = txtfree ;

#line 524 "../../src/lex.c"
if (saved ){ 
#line 525 "../../src/lex.c"
_au2_lxchar = saved ;
saved = 0 ;
}
else 
#line 529 "../../src/lex.c"
_au2_lxchar = getc(in_file);

#line 531 "../../src/lex.c"
switch ((_au2_p = (lxcp [_au2_lxchar + 1 ]))-> _LXDOPE_lxact ){ 
#line 533 "../../src/lex.c"
case 3 : 
#line 534 "../../src/lex.c"
{ addtok ( _au2_p -> _LXDOPE_lxtok , ( (_au1__Xy_rtFUI___global . __C10_t =
#line 534 "../../src/lex.c"
_au2_p -> _LXDOPE_lxtok ), _au1__Xy_rtFUI___global ) ) ;

#line 534 "../../src/lex.c"
return ;
}

#line 534 "../../src/lex.c"
;

#line 536 "../../src/lex.c"
case 16 : 
#line 537 "../../src/lex.c"
if (br_level || bl_level )
#line 538 "../../src/lex.c"
{ 
#line 991 "../../src/lex.c"
struct ea _au0__V11 ;

#line 538 "../../src/lex.c"
error ( (char *)"'%s' missing at end of input", (struct ea *)( ( ((& _au0__V11 )-> _ea__O1.__C1_p = ((char *)(bl_level ? "}": ")"))), (((&
#line 538 "../../src/lex.c"
_au0__V11 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} { addtok ( (unsigned char )0 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )0 )), _au1__Xy_rtFUI___global ) )
#line 539 "../../src/lex.c"
;

#line 539 "../../src/lex.c"
return ;
}

#line 539 "../../src/lex.c"
;

#line 541 "../../src/lex.c"
case 29 : 
#line 547 "../../src/lex.c"
ungetc ( (int )'#' , in_file ) ;
saved = lxtitle ( ) ;
continue ;

#line 551 "../../src/lex.c"
case 0 : 
#line 552 "../../src/lex.c"
if ((' ' <= _au2_lxchar )&& (_au2_lxchar <= '~' ))
#line 553 "../../src/lex.c"
{ 
#line 991 "../../src/lex.c"
struct ea _au0__V12 ;

#line 553 "../../src/lex.c"
error ( (char *)"illegal character '%c' (ignored)", (struct ea *)( ( ((& _au0__V12 )-> _ea__O1.__C1_i = _au2_lxchar ), (((& _au0__V12 )))) ) ,
#line 553 "../../src/lex.c"
(struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} else 
#line 555 "../../src/lex.c"
{ 
#line 991 "../../src/lex.c"
struct ea _au0__V13 ;

#line 555 "../../src/lex.c"
error ( (char *)"illegal character '0%o' (ignored)", (struct ea *)( ( ((& _au0__V13 )-> _ea__O1.__C1_i = _au2_lxchar ), (((& _au0__V13 )))) ) ,
#line 555 "../../src/lex.c"
(struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} continue ;

#line 558 "../../src/lex.c"
case 1 : 
#line 559 "../../src/lex.c"
lxget ( _au2_lxchar , 3) ;

#line 561 "../../src/lex.c"
if (_au1_n = _table_look ( ktbl , txtstart , (unsigned char )0 ) ){ 
#line 562 "../../src/lex.c"
TOK _au4_x ;

#line 563 "../../src/lex.c"
union _C10 _au1__Xy_rtFPV___global ;

#line 563 "../../src/lex.c"
union _C10 _au1__Xy_rtFCloc____global ;

#line 563 "../../src/lex.c"
union _C10 _au1__Xy_rtFUI___global ;
txtfree = txtstart ;

#line 564 "../../src/lex.c"
switch (_au4_x = _au1_n -> _node_base ){ 
#line 565 "../../src/lex.c"
case 123 : 
#line 566 "../../src/lex.c"
{ addtok ( (unsigned char )123 , ( (_au1__Xy_rtFPV___global . __C10_pn =
#line 566 "../../src/lex.c"
(((struct name *)((char *)(((struct node *)_au1_n )))))), _au1__Xy_rtFPV___global ) ) ;

#line 566 "../../src/lex.c"
return ;
}

#line 566 "../../src/lex.c"
;
break ;
case 143 : 
#line 569 "../../src/lex.c"
{ addtok ( (unsigned char )_au1_n -> _expr__O2.__C2_syn_class , ( (_au1__Xy_rtFCloc____global . __C10_l = curloc ), _au1__Xy_rtFCloc____global ) )
#line 569 "../../src/lex.c"
;

#line 569 "../../src/lex.c"
return ;
}

#line 569 "../../src/lex.c"
;
default : 
#line 581 "../../src/lex.c"
{ addtok ( (unsigned char )_au1_n -> _expr__O2.__C2_syn_class , ( (_au1__Xy_rtFUI___global . __C10_t = _au4_x ), _au1__Xy_rtFUI___global ) )
#line 581 "../../src/lex.c"
;

#line 581 "../../src/lex.c"
return ;
}

#line 581 "../../src/lex.c"
;
}
}
else 
#line 585 "../../src/lex.c"
{ addtok ( (unsigned char )80 , ( (_au1__Xy_rt_global . __C10_s = txtstart ), _au1__Xy_rt_global ) ) ;

#line 585 "../../src/lex.c"
return ;
}

#line 585 "../../src/lex.c"
;

#line 587 "../../src/lex.c"
case 2 : 
#line 589 "../../src/lex.c"
_au1_ret = 82 ;

#line 591 "../../src/lex.c"
if (_au2_lxchar == '0' ){ 
#line 592 "../../src/lex.c"
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 ,
#line 592 "../../src/lex.c"
(struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= '0' )));
switch (_au2_lxchar = getc(in_file)){ 
#line 594 "../../src/lex.c"
case
#line 594 "../../src/lex.c"
'l' : 
#line 595 "../../src/lex.c"
case 'L' : 
#line 596 "../../src/lex.c"
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 ,
#line 596 "../../src/lex.c"
(struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= 'L' )));
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 597 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= 0 )));
{ addtok ( (unsigned char )82 , ( (_au1__Xy_rt_global . __C10_s = txtstart ), _au1__Xy_rt_global ) ) ;

#line 598 "../../src/lex.c"
return ;
}

#line 598 "../../src/lex.c"
;
case 'e' : 
#line 600 "../../src/lex.c"
case 'E' : 
#line 603 "../../src/lex.c"
goto getfp2 ;
case 'x' : 
#line 605 "../../src/lex.c"
case 'X' : 
#line 606 "../../src/lex.c"
lxget ( (int )'X' , 010 ) ;
if ((txtfree - txtstart )< 4 )
#line 608 "../../src/lex.c"
error ( (char *)"hexadecimal digitX after \"0x\"", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 )
#line 608 "../../src/lex.c"
;
switch (_au2_lxchar = getc(in_file)){ 
#line 610 "../../src/lex.c"
case
#line 610 "../../src/lex.c"
'l' : 
#line 611 "../../src/lex.c"
case 'L' : 
#line 612 "../../src/lex.c"
txtfree -- ;
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 613 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= 'L' )));
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 614 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= 0 )));
break ;
default : 
#line 617 "../../src/lex.c"
saved = _au2_lxchar ;
}
{ addtok ( (unsigned char )82 , ( (_au1__Xy_rt_global . __C10_s = txtstart ), _au1__Xy_rt_global ) ) ;

#line 619 "../../src/lex.c"
return ;
}

#line 619 "../../src/lex.c"
;
case '8' : 
#line 621 "../../src/lex.c"
case '9' : 
#line 622 "../../src/lex.c"
{ 
#line 991 "../../src/lex.c"
struct ea _au0__V14 ;

#line 622 "../../src/lex.c"
error ( (char *)"%c used as octal digit", (struct ea *)( ( ((& _au0__V14 )-> _ea__O1.__C1_i = _au2_lxchar ), (((& _au0__V14 )))) ) ,
#line 622 "../../src/lex.c"
(struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
case '0' : 
#line 624 "../../src/lex.c"
case '1' : 
#line 625 "../../src/lex.c"
case '2' : 
#line 626 "../../src/lex.c"
case '3' : 
#line 627 "../../src/lex.c"
case '4' : 
#line 628 "../../src/lex.c"
case '5' : 
#line 629 "../../src/lex.c"
case '6' :
#line 629 "../../src/lex.c"

#line 630 "../../src/lex.c"
case '7' : 
#line 631 "../../src/lex.c"
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct
#line 631 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= _au2_lxchar )));
ox :
#line 633 "../../src/lex.c"
switch (_au2_lxchar = getc(in_file)){ 
#line 634 "../../src/lex.c"
case
#line 634 "../../src/lex.c"
'8' : 
#line 635 "../../src/lex.c"
case '9' : 
#line 636 "../../src/lex.c"
{ 
#line 991 "../../src/lex.c"
struct ea _au0__V15 ;

#line 636 "../../src/lex.c"
error ( (char *)"%c used as octal digit", (struct ea *)( ( ((& _au0__V15 )-> _ea__O1.__C1_i = _au2_lxchar ), (((& _au0__V15 )))) ) ,
#line 636 "../../src/lex.c"
(struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
case '0' : 
#line 638 "../../src/lex.c"
case '1' : 
#line 639 "../../src/lex.c"
case '2' : 
#line 640 "../../src/lex.c"
case '3' : 
#line 641 "../../src/lex.c"
case '4' : 
#line 642 "../../src/lex.c"
case '5' : 
#line 643 "../../src/lex.c"
case '6' :
#line 643 "../../src/lex.c"

#line 644 "../../src/lex.c"
case '7' : 
#line 645 "../../src/lex.c"
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct
#line 645 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= _au2_lxchar )));
goto ox ;
case 'l' : 
#line 648 "../../src/lex.c"
case 'L' : 
#line 649 "../../src/lex.c"
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct
#line 649 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= 'L' )));
#line 649 "../../src/lex.c"

#line 650 "../../src/lex.c"
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 650 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= 0 )));
break ;
default : 
#line 653 "../../src/lex.c"
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct
#line 653 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= 0 )));
saved = _au2_lxchar ;
} }
{ addtok ( (unsigned char )82 , ( (_au1__Xy_rt_global . __C10_s = txtstart ), _au1__Xy_rt_global ) ) ;

#line 656 "../../src/lex.c"
return ;
}

#line 656 "../../src/lex.c"
;
case '.' : 
#line 658 "../../src/lex.c"
lxget ( (int )'.' , 02 ) ;
goto getfp ;
default : 
#line 661 "../../src/lex.c"
saved = _au2_lxchar ;
{ addtok ( (unsigned char )86 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )0 )), _au1__Xy_rtFUI___global ) ) ;
#line 662 "../../src/lex.c"
return ;
}

#line 662 "../../src/lex.c"
;
} }
}
else 
#line 666 "../../src/lex.c"
lxget ( _au2_lxchar , 02 ) ;

#line 668 "../../src/lex.c"
if ((_au2_lxchar = getc(in_file))== '.' ){
#line 668 "../../src/lex.c"

#line 669 "../../src/lex.c"
txtfree -- ;
lxget ( (int )'.' , 02 ) ;
getfp :
#line 672 "../../src/lex.c"
_au1_ret = 83 ;
_au2_lxchar = getc(in_file);
}

#line 674 "../../src/lex.c"
;

#line 676 "../../src/lex.c"
switch (_au2_lxchar ){ 
#line 677 "../../src/lex.c"
case 'e' : 
#line 678 "../../src/lex.c"
case 'E' : 
#line 679 "../../src/lex.c"
txtfree -- ;
switch (_au2_lxchar = getc(in_file)){ 
#line 681 "../../src/lex.c"
case
#line 681 "../../src/lex.c"
'-' : 
#line 682 "../../src/lex.c"
case '+' : 
#line 683 "../../src/lex.c"
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 ,
#line 683 "../../src/lex.c"
(struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= 'e' )));
break ;
default : 
#line 686 "../../src/lex.c"
ungetc ( _au2_lxchar , in_file ) ;
_au2_lxchar = 'e' ;
}

#line 688 "../../src/lex.c"
;
getfp2 :
#line 690 "../../src/lex.c"
lxget ( _au2_lxchar , 02 ) ;
_au1_ret = 83 ;
break ;
case 'l' : 
#line 694 "../../src/lex.c"
case 'L' : 
#line 695 "../../src/lex.c"
case 'u' : 
#line 696 "../../src/lex.c"
case 'U' : 
#line 697 "../../src/lex.c"
if (_au1_ret == 83 )
#line 698 "../../src/lex.c"
{ 
#line 991 "../../src/lex.c"
struct ea _au0__V16 ;

#line 698 "../../src/lex.c"
error ( (char *)"illegal suffix after floating point constant: %c", (struct ea *)( ( ((& _au0__V16 )-> _ea__O1.__C1_i = _au2_lxchar ), (((& _au0__V16 )))) ) ,
#line 698 "../../src/lex.c"
(struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} else { 
#line 700 "../../src/lex.c"
txtfree -- ;
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 701 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= _au2_lxchar )));
}
switch (_au2_lxchar = getc(in_file)){ 
#line 704 "../../src/lex.c"
case
#line 704 "../../src/lex.c"
'l' : 
#line 705 "../../src/lex.c"
case 'L' : 
#line 706 "../../src/lex.c"
case 'u' : 
#line 707 "../../src/lex.c"
case 'U' : 
#line 708 "../../src/lex.c"
if (_au1_ret == 83 )
#line 709 "../../src/lex.c"
{ 
#line 991 "../../src/lex.c"
struct ea _au0__V17 ;

#line 709 "../../src/lex.c"
error ( (char *)"illegal suffix after floating point constant: %c", (struct ea *)( ( ((& _au0__V17 )-> _ea__O1.__C1_i = _au2_lxchar ), (((& _au0__V17 )))) ) ,
#line 709 "../../src/lex.c"
(struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} else (txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct
#line 710 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= _au2_lxchar )));
break ;
default : 
#line 713 "../../src/lex.c"
saved = _au2_lxchar ;
}
break ;
default : 
#line 717 "../../src/lex.c"
saved = _au2_lxchar ;
}

#line 718 "../../src/lex.c"
;

#line 720 "../../src/lex.c"
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 720 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= 0 )));
{ addtok ( _au1_ret , ( (_au1__Xy_rt_global . __C10_s = txtstart ), _au1__Xy_rt_global ) ) ;

#line 721 "../../src/lex.c"
return ;
}

#line 721 "../../src/lex.c"
;

#line 723 "../../src/lex.c"
case 8 : 
#line 724 "../../src/lex.c"
if ((_au2_lxchar = getc(in_file))== '.' ){ 
#line 725 "../../src/lex.c"
if ((_au2_lxchar = getc(in_file))!= '.' ){ 
#line 726 "../../src/lex.c"
error ( (char *)"token .. ?", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 726 "../../src/lex.c"

#line 727 "../../src/lex.c"
saved = _au2_lxchar ;
}
{ addtok ( (unsigned char )155 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )0 )), _au1__Xy_rtFUI___global ) ) ;
#line 729 "../../src/lex.c"
return ;
}

#line 729 "../../src/lex.c"
;
}
if ((lxmask [_au2_lxchar + 1 ])& 02 ){ 
#line 732 "../../src/lex.c"
ungetc ( _au2_lxchar , in_file ) ;
lxget ( (int )'.' , 02 ) ;
goto getfp ;
}
saved = _au2_lxchar ;
{ addtok ( (unsigned char )45 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )0 )), _au1__Xy_rtFUI___global ) ) ;
#line 737 "../../src/lex.c"
return ;
}

#line 737 "../../src/lex.c"
;

#line 739 "../../src/lex.c"
case 4 : 
#line 741 "../../src/lex.c"
for(;;) 
#line 742 "../../src/lex.c"
switch (_au2_lxchar = getc(in_file)){ 
#line 743 "../../src/lex.c"
case '\\' : 
#line 744 "../../src/lex.c"
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct
#line 744 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= '\\' )));
#line 744 "../../src/lex.c"

#line 745 "../../src/lex.c"
switch (_au2_lxchar = getc(in_file)){ 
#line 746 "../../src/lex.c"
case
#line 746 "../../src/lex.c"
'\n' : 
#line 747 "../../src/lex.c"
++ curloc . _loc_line ;
default : 
#line 749 "../../src/lex.c"
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct
#line 749 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= _au2_lxchar )));
break ;
case 'x' : 
#line 752 "../../src/lex.c"
hex_to_oct ( ) ;
break ;
}

#line 754 "../../src/lex.c"
;
break ;
case '"' : 
#line 757 "../../src/lex.c"
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct
#line 757 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= 0 )));
{ addtok ( (unsigned char )81 , ( (_au1__Xy_rt_global . __C10_s = txtstart ), _au1__Xy_rt_global ) ) ;

#line 758 "../../src/lex.c"
return ;
}

#line 758 "../../src/lex.c"
;
case '\n' : 
#line 760 "../../src/lex.c"
error ( (char *)"newline in string", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 760 "../../src/lex.c"

#line 761 "../../src/lex.c"
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 761 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= 0 )));
{ addtok ( (unsigned char )81 , ( (_au1__Xy_rt_global . __C10_s = txtstart ), _au1__Xy_rt_global ) ) ;

#line 762 "../../src/lex.c"
return ;
}

#line 762 "../../src/lex.c"
;
case EOF: 
#line 764 "../../src/lex.c"
error ( (char *)"eof in string", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 764 "../../src/lex.c"

#line 765 "../../src/lex.c"
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 765 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= 0 )));
{ addtok ( (unsigned char )81 , ( (_au1__Xy_rt_global . __C10_s = txtstart ), _au1__Xy_rt_global ) ) ;

#line 766 "../../src/lex.c"
return ;
}

#line 766 "../../src/lex.c"
;
default : 
#line 768 "../../src/lex.c"
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct
#line 768 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= _au2_lxchar )));
}

#line 771 "../../src/lex.c"
case 5 : 
#line 773 "../../src/lex.c"
{ addtok ( (unsigned char )84 , ( (_au0__Xx_rt_global = chconst ( ) ), ( (_au1__Xy_rt_global .
#line 773 "../../src/lex.c"
__C10_s = _au0__Xx_rt_global ), _au1__Xy_rt_global ) ) ) ;

#line 773 "../../src/lex.c"
return ;
}

#line 773 "../../src/lex.c"
;

#line 775 "../../src/lex.c"
case 6 : 
#line 776 "../../src/lex.c"
{ 
#line 777 "../../src/lex.c"
register int _au4_i ;
int _au4_j ;

#line 779 "../../src/lex.c"
union _C10 _au1__Xy_rt_global ;

#line 780 "../../src/lex.c"
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 780 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= '`' )));

#line 782 "../../src/lex.c"
for(_au4_i = 0 ;_au4_i < 7 ;++ _au4_i ) { 
#line 783 "../../src/lex.c"
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow",
#line 783 "../../src/lex.c"
(struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))=
#line 783 "../../src/lex.c"
(_au4_j = getc(in_file)))));
if (_au4_j == '`' )break ;
}
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 786 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= 0 )));
if (6 < _au4_i )
#line 788 "../../src/lex.c"
error ( (char *)"bcd constant exceeds 6 characters", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 788 "../../src/lex.c"

#line 789 "../../src/lex.c"
{ addtok ( (unsigned char )84 , ( (_au1__Xy_rt_global . __C10_s = txtstart ), _au1__Xy_rt_global ) ) ;

#line 789 "../../src/lex.c"
return ;
}

#line 789 "../../src/lex.c"
;
}

#line 792 "../../src/lex.c"
case 7 : 
#line 793 "../../src/lex.c"
switch (_au2_lxchar = getc(in_file)){ 
#line 794 "../../src/lex.c"
case '*' : 
#line 795 "../../src/lex.c"
lxcom ( ) ;
break ;
case '/' : 
#line 798 "../../src/lex.c"
linecom ( ) ;
break ;
case '=' : 
#line 801 "../../src/lex.c"
{ addtok ( (unsigned char )90 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )129 )), _au1__Xy_rtFUI___global )
#line 801 "../../src/lex.c"
) ;

#line 801 "../../src/lex.c"
return ;
}

#line 801 "../../src/lex.c"
;
default : 
#line 803 "../../src/lex.c"
saved = _au2_lxchar ;
{ addtok ( (unsigned char )93 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )51 )), _au1__Xy_rtFUI___global ) ) ;
#line 804 "../../src/lex.c"
return ;
}

#line 804 "../../src/lex.c"
;
}

#line 807 "../../src/lex.c"
case 10 : 
#line 808 "../../src/lex.c"
continue ;

#line 810 "../../src/lex.c"
case 11 : 
#line 811 "../../src/lex.c"
++ curloc . _loc_line ;
Nline ++ ;
saved = lxtitle ( ) ;
continue ;

#line 816 "../../src/lex.c"
case 12 : 
#line 820 "../../src/lex.c"
if (50 <= (bl_level ++ )){ 
#line 821 "../../src/lex.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"blocks too deeply nested", (struct ea *)ea0 , (struct
#line 821 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
ext ( 3 ) ;
}
{ addtok ( (unsigned char )73 , ( (_au1__Xy_rtFCloc____global . __C10_l = curloc ), _au1__Xy_rtFCloc____global ) ) ;

#line 824 "../../src/lex.c"
return ;
}

#line 824 "../../src/lex.c"
;

#line 826 "../../src/lex.c"
case 13 : 
#line 830 "../../src/lex.c"
if ((bl_level -- )<= 0 ){ 
#line 831 "../../src/lex.c"
error ( (char *)"unexpected '}'", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 831 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 ) ;
bl_level = 0 ;
}
{ addtok ( (unsigned char )74 , ( (_au1__Xy_rtFCloc____global . __C10_l = curloc ), _au1__Xy_rtFCloc____global ) ) ;

#line 834 "../../src/lex.c"
return ;
}

#line 834 "../../src/lex.c"
;

#line 836 "../../src/lex.c"
case 14 : 
#line 843 "../../src/lex.c"
br_level ++ ;
{ addtok ( (unsigned char )40 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )0 )), _au1__Xy_rtFUI___global ) ) ;
#line 844 "../../src/lex.c"
return ;
}

#line 844 "../../src/lex.c"
;

#line 846 "../../src/lex.c"
case 15 : 
#line 847 "../../src/lex.c"
if ((br_level -- )<= 0 ){ 
#line 848 "../../src/lex.c"
error ( (char *)"unexpected ')'", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 848 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 ) ;
br_level = 0 ;
}
{ addtok ( (unsigned char )41 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )0 )), _au1__Xy_rtFUI___global ) ) ;
#line 851 "../../src/lex.c"
return ;
}

#line 851 "../../src/lex.c"
;

#line 853 "../../src/lex.c"
case 17 : 
#line 854 "../../src/lex.c"
switch (_au2_lxchar = getc(in_file)){ 
#line 855 "../../src/lex.c"
case '=' : 
#line 856 "../../src/lex.c"
{ addtok ( (unsigned char )92 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )62 )),
#line 856 "../../src/lex.c"
_au1__Xy_rtFUI___global ) ) ;

#line 856 "../../src/lex.c"
return ;
}

#line 856 "../../src/lex.c"
;
default : 
#line 858 "../../src/lex.c"
saved = _au2_lxchar ;
{ addtok ( (unsigned char )70 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )70 )), _au1__Xy_rtFUI___global ) ) ;
#line 859 "../../src/lex.c"
return ;
}

#line 859 "../../src/lex.c"
;
}

#line 862 "../../src/lex.c"
case 28 : 
#line 863 "../../src/lex.c"
switch (_au2_lxchar = getc(in_file)){ 
#line 864 "../../src/lex.c"
case ':' : 
#line 865 "../../src/lex.c"
{ addtok ( (unsigned char )160 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )0 )),
#line 865 "../../src/lex.c"
_au1__Xy_rtFUI___global ) ) ;

#line 865 "../../src/lex.c"
return ;
}

#line 865 "../../src/lex.c"
;
case '=' : 
#line 867 "../../src/lex.c"
error ( (char *)"':=' is not a c++ operator", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 867 "../../src/lex.c"

#line 868 "../../src/lex.c"
{ addtok ( (unsigned char )70 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )70 )), _au1__Xy_rtFUI___global ) ) ;
#line 868 "../../src/lex.c"
return ;
}

#line 868 "../../src/lex.c"
;
default : 
#line 870 "../../src/lex.c"
saved = _au2_lxchar ;
{ addtok ( (unsigned char )69 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )69 )), _au1__Xy_rtFUI___global ) ) ;
#line 871 "../../src/lex.c"
return ;
}

#line 871 "../../src/lex.c"
;
}
case 24 : 
#line 874 "../../src/lex.c"
switch (_au2_lxchar = getc(in_file)){ 
#line 875 "../../src/lex.c"
case '=' : 
#line 876 "../../src/lex.c"
{ addtok ( (unsigned char )92 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )63 )),
#line 876 "../../src/lex.c"
_au1__Xy_rtFUI___global ) ) ;

#line 876 "../../src/lex.c"
return ;
}

#line 876 "../../src/lex.c"
;
default : 
#line 878 "../../src/lex.c"
saved = _au2_lxchar ;
{ addtok ( (unsigned char )46 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )46 )), _au1__Xy_rtFUI___global ) ) ;
#line 879 "../../src/lex.c"
return ;
}

#line 879 "../../src/lex.c"
;
}
case 19 : 
#line 882 "../../src/lex.c"
switch (_au2_lxchar = getc(in_file)){ 
#line 883 "../../src/lex.c"
case '>' : 
#line 884 "../../src/lex.c"
switch (_au2_lxchar = getc(in_file)){ 
#line 885 "../../src/lex.c"
case '=' : 
#line 886 "../../src/lex.c"
{ addtok ( (unsigned char )90 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char
#line 886 "../../src/lex.c"
)135 )), _au1__Xy_rtFUI___global ) ) ;

#line 886 "../../src/lex.c"
return ;
}

#line 886 "../../src/lex.c"
;
break ;
default : 
#line 889 "../../src/lex.c"
saved = _au2_lxchar ;
{ addtok ( (unsigned char )94 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )57 )), _au1__Xy_rtFUI___global ) ) ;
#line 890 "../../src/lex.c"
return ;
}

#line 890 "../../src/lex.c"
;
}
case '=' : 
#line 893 "../../src/lex.c"
{ addtok ( (unsigned char )91 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )61 )), _au1__Xy_rtFUI___global )
#line 893 "../../src/lex.c"
) ;

#line 893 "../../src/lex.c"
return ;
}

#line 893 "../../src/lex.c"
;
default : 
#line 895 "../../src/lex.c"
saved = _au2_lxchar ;
{ addtok ( (unsigned char )91 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )60 )), _au1__Xy_rtFUI___global ) ) ;
#line 896 "../../src/lex.c"
return ;
}

#line 896 "../../src/lex.c"
;
}
case 18 : 
#line 899 "../../src/lex.c"
switch (_au2_lxchar = getc(in_file)){ 
#line 900 "../../src/lex.c"
case '<' : 
#line 901 "../../src/lex.c"
switch (_au2_lxchar = getc(in_file)){ 
#line 902 "../../src/lex.c"
case '=' : 
#line 903 "../../src/lex.c"
{ addtok ( (unsigned char )90 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char
#line 903 "../../src/lex.c"
)134 )), _au1__Xy_rtFUI___global ) ) ;

#line 903 "../../src/lex.c"
return ;
}

#line 903 "../../src/lex.c"
;
default : 
#line 905 "../../src/lex.c"
saved = _au2_lxchar ;
{ addtok ( (unsigned char )94 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )56 )), _au1__Xy_rtFUI___global ) ) ;
#line 906 "../../src/lex.c"
return ;
}

#line 906 "../../src/lex.c"
;
}
case '=' : 
#line 909 "../../src/lex.c"
{ addtok ( (unsigned char )91 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )59 )), _au1__Xy_rtFUI___global )
#line 909 "../../src/lex.c"
) ;

#line 909 "../../src/lex.c"
return ;
}

#line 909 "../../src/lex.c"
;
default : 
#line 911 "../../src/lex.c"
saved = _au2_lxchar ;
{ addtok ( (unsigned char )91 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )58 )), _au1__Xy_rtFUI___global ) ) ;
#line 912 "../../src/lex.c"
return ;
}

#line 912 "../../src/lex.c"
;
}
case 22 : 
#line 915 "../../src/lex.c"
switch (_au2_lxchar = getc(in_file)){ 
#line 916 "../../src/lex.c"
case '&' : 
#line 917 "../../src/lex.c"
{ addtok ( (unsigned char )66 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )66 )),
#line 917 "../../src/lex.c"
_au1__Xy_rtFUI___global ) ) ;

#line 917 "../../src/lex.c"
return ;
}

#line 917 "../../src/lex.c"
;
case '=' : 
#line 919 "../../src/lex.c"
{ addtok ( (unsigned char )90 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )131 )), _au1__Xy_rtFUI___global )
#line 919 "../../src/lex.c"
) ;

#line 919 "../../src/lex.c"
return ;
}

#line 919 "../../src/lex.c"
;
default : 
#line 921 "../../src/lex.c"
saved = _au2_lxchar ;
{ addtok ( (unsigned char )52 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )52 )), _au1__Xy_rtFUI___global ) ) ;
#line 922 "../../src/lex.c"
return ;
}

#line 922 "../../src/lex.c"
;
}
case 21 : 
#line 925 "../../src/lex.c"
switch (_au2_lxchar = getc(in_file)){ 
#line 926 "../../src/lex.c"
case '|' : 
#line 927 "../../src/lex.c"
{ addtok ( (unsigned char )67 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )67 )),
#line 927 "../../src/lex.c"
_au1__Xy_rtFUI___global ) ) ;

#line 927 "../../src/lex.c"
return ;
}

#line 927 "../../src/lex.c"
;
case '=' : 
#line 929 "../../src/lex.c"
{ addtok ( (unsigned char )90 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )132 )), _au1__Xy_rtFUI___global )
#line 929 "../../src/lex.c"
) ;

#line 929 "../../src/lex.c"
return ;
}

#line 929 "../../src/lex.c"
;
default : 
#line 931 "../../src/lex.c"
saved = _au2_lxchar ;
{ addtok ( (unsigned char )65 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )65 )), _au1__Xy_rtFUI___global ) ) ;
#line 932 "../../src/lex.c"
return ;
}

#line 932 "../../src/lex.c"
;
}
case 20 : 
#line 935 "../../src/lex.c"
switch (_au2_lxchar = getc(in_file)){ 
#line 936 "../../src/lex.c"
case '=' : 
#line 937 "../../src/lex.c"
{ addtok ( (unsigned char )90 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )133 )),
#line 937 "../../src/lex.c"
_au1__Xy_rtFUI___global ) ) ;

#line 937 "../../src/lex.c"
return ;
}

#line 937 "../../src/lex.c"
;
default : 
#line 939 "../../src/lex.c"
saved = _au2_lxchar ;
{ addtok ( (unsigned char )64 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )64 )), _au1__Xy_rtFUI___global ) ) ;
#line 940 "../../src/lex.c"
return ;
}

#line 940 "../../src/lex.c"
;
}
case 27 : 
#line 943 "../../src/lex.c"
switch (_au2_lxchar = getc(in_file)){ 
#line 944 "../../src/lex.c"
case '=' : 
#line 945 "../../src/lex.c"
{ addtok ( (unsigned char )90 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )126 )),
#line 945 "../../src/lex.c"
_au1__Xy_rtFUI___global ) ) ;

#line 945 "../../src/lex.c"
return ;
}

#line 945 "../../src/lex.c"
;
case '+' : 
#line 947 "../../src/lex.c"
{ addtok ( (unsigned char )95 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )48 )), _au1__Xy_rtFUI___global )
#line 947 "../../src/lex.c"
) ;

#line 947 "../../src/lex.c"
return ;
}

#line 947 "../../src/lex.c"
;
default : 
#line 949 "../../src/lex.c"
saved = _au2_lxchar ;
{ addtok ( (unsigned char )54 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )54 )), _au1__Xy_rtFUI___global ) ) ;
#line 950 "../../src/lex.c"
return ;
}

#line 950 "../../src/lex.c"
;
}
case 25 : 
#line 953 "../../src/lex.c"
switch (_au2_lxchar = getc(in_file)){ 
#line 954 "../../src/lex.c"
case '=' : 
#line 955 "../../src/lex.c"
{ addtok ( (unsigned char )90 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )127 )),
#line 955 "../../src/lex.c"
_au1__Xy_rtFUI___global ) ) ;

#line 955 "../../src/lex.c"
return ;
}

#line 955 "../../src/lex.c"
;
case '-' : 
#line 957 "../../src/lex.c"
{ addtok ( (unsigned char )95 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )49 )), _au1__Xy_rtFUI___global )
#line 957 "../../src/lex.c"
) ;

#line 957 "../../src/lex.c"
return ;
}

#line 957 "../../src/lex.c"
;
case '>' : 
#line 959 "../../src/lex.c"
{ addtok ( (unsigned char )44 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )44 )), _au1__Xy_rtFUI___global )
#line 959 "../../src/lex.c"
) ;

#line 959 "../../src/lex.c"
return ;
}

#line 959 "../../src/lex.c"
;
default : 
#line 961 "../../src/lex.c"
saved = _au2_lxchar ;
{ addtok ( (unsigned char )55 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )55 )), _au1__Xy_rtFUI___global ) ) ;
#line 962 "../../src/lex.c"
return ;
}

#line 962 "../../src/lex.c"
;
}
case 26 : 
#line 965 "../../src/lex.c"
switch (_au2_lxchar = getc(in_file)){ 
#line 966 "../../src/lex.c"
case '=' : 
#line 967 "../../src/lex.c"
{ addtok ( (unsigned char )90 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )128 )),
#line 967 "../../src/lex.c"
_au1__Xy_rtFUI___global ) ) ;

#line 967 "../../src/lex.c"
return ;
}

#line 967 "../../src/lex.c"
;
case '/' : 
#line 969 "../../src/lex.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"*/ not as end of comment", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 969 "../../src/lex.c"
ea *)ea0 ) ;
default : 
#line 971 "../../src/lex.c"
saved = _au2_lxchar ;
{ addtok ( (unsigned char )50 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )50 )), _au1__Xy_rtFUI___global ) ) ;
#line 972 "../../src/lex.c"
return ;
}

#line 972 "../../src/lex.c"
;
}
case 23 : 
#line 975 "../../src/lex.c"
switch (_au2_lxchar = getc(in_file)){ 
#line 976 "../../src/lex.c"
case '=' : 
#line 977 "../../src/lex.c"
{ addtok ( (unsigned char )90 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )130 )),
#line 977 "../../src/lex.c"
_au1__Xy_rtFUI___global ) ) ;

#line 977 "../../src/lex.c"
return ;
}

#line 977 "../../src/lex.c"
;
default : 
#line 979 "../../src/lex.c"
saved = _au2_lxchar ;
{ addtok ( (unsigned char )93 , ( (_au1__Xy_rtFUI___global . __C10_t = ((unsigned char )53 )), _au1__Xy_rtFUI___global ) ) ;
#line 980 "../../src/lex.c"
return ;
}

#line 980 "../../src/lex.c"
;
}
default : 
#line 983 "../../src/lex.c"
{ 
#line 991 "../../src/lex.c"
struct ea _au0__V18 ;

#line 991 "../../src/lex.c"
struct ea _au0__V19 ;

#line 983 "../../src/lex.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"lex act==%d getc()->%d", (struct ea *)( ( ((& _au0__V18 )-> _ea__O1.__C1_p = ((char *)_au2_p )), (((&
#line 983 "../../src/lex.c"
_au0__V18 )))) ) , (struct ea *)( ( ((& _au0__V19 )-> _ea__O1.__C1_i = _au2_lxchar ), (((& _au0__V19 )))) ) ,
#line 983 "../../src/lex.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
} }

#line 987 "../../src/lex.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"lex, main switch", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 )
#line 987 "../../src/lex.c"
;
}
}
;

#line 993 "../../src/lex.c"
extern int lxtitle ()
#line 997 "../../src/lex.c"
{ 
#line 998 "../../src/lex.c"
register int _au1_c ;

#line 1026 "../../src/lex.c"
char *_au5_fn ;

#line 1000 "../../src/lex.c"
for(;;) 
#line 1001 "../../src/lex.c"
switch (_au1_c = getc(in_file)){
#line 1001 "../../src/lex.c"

#line 1002 "../../src/lex.c"
default : 
#line 1003 "../../src/lex.c"
return _au1_c ;

#line 1006 "../../src/lex.c"
case '\n' : 
#line 1007 "../../src/lex.c"
curloc . _loc_line ++ ;
Nline ++ ;
break ;
ll :
#line 1011 "../../src/lex.c"
break ;
case '#' : 
#line 1013 "../../src/lex.c"
curloc . _loc_line = 0 ;
for(;;) 
#line 1015 "../../src/lex.c"
switch (_au1_c = getc(in_file)){
#line 1015 "../../src/lex.c"

#line 1016 "../../src/lex.c"
case '"' : 
#line 1017 "../../src/lex.c"
txtstart = txtfree ;
for(;;) 
#line 1019 "../../src/lex.c"
switch (_au1_c = getc(in_file)){
#line 1019 "../../src/lex.c"

#line 1020 "../../src/lex.c"
case '"' : 
#line 1021 "../../src/lex.c"
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct
#line 1021 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= '\0' )));
while ((_au1_c = getc(in_file))!= '\n' );
#line 1022 "../../src/lex.c"

#line 1023 "../../src/lex.c"
if (*txtstart ){ 
#line 1024 "../../src/lex.c"
if (curr_file == 0 )goto push ;

#line 1026 "../../src/lex.c"
;

#line 1028 "../../src/lex.c"
if ((_au5_fn = (file_name [file_stack [curr_file ]]))&& (strcmp ( (char *)txtstart , (char *)_au5_fn ) == 0 ))
#line 1029 "../../src/lex.c"
{ }
else 
#line 1032 "../../src/lex.c"
if ((_au5_fn = (file_name [file_stack [curr_file -
#line 1032 "../../src/lex.c"
1 ]]))&& (strcmp ( (char *)txtstart , (char *)_au5_fn ) == 0 ))
#line 1033 "../../src/lex.c"
{ 
#line 1035 "../../src/lex.c"
curr_file -- ;
}
else { 
#line 1038 "../../src/lex.c"
push :
#line 1039 "../../src/lex.c"
if (508< (Nfile ++ ))errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"fileN buffer overflow", (struct ea *)ea0 , (struct ea *)ea0 ,
#line 1039 "../../src/lex.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
if (127 < (curr_file ++ ))errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"fileN stack overflow", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ,
#line 1040 "../../src/lex.c"
(struct ea *)ea0 ) ;
(file_stack [curr_file ])= Nfile ;

#line 1043 "../../src/lex.c"
{ char *_au6_p ;

#line 1043 "../../src/lex.c"
_au6_p = (((char *)_new ( (long )((sizeof (char ))* (txtfree - txtstart ))) ));
(strcpy ( _au6_p , (char *)txtstart ) );
(file_name [Nfile ])= _au6_p ;
Nstr ++ ;
}
}
}
else 
#line 1049 "../../src/lex.c"
{ 
#line 1050 "../../src/lex.c"
curr_file = 0 ;
}
txtfree = txtstart ;
curloc . _loc_file = (file_stack [curr_file ]);
goto ll ;
case '\n' : 
#line 1056 "../../src/lex.c"
error ( (char *)"unexpected end of line on '# line'", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 1056 "../../src/lex.c"

#line 1057 "../../src/lex.c"
default : 
#line 1058 "../../src/lex.c"
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct
#line 1058 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= _au1_c )));
}
case ' ' : 
#line 1061 "../../src/lex.c"
break ;
case '0' : 
#line 1063 "../../src/lex.c"
case '1' : 
#line 1064 "../../src/lex.c"
case '2' : 
#line 1065 "../../src/lex.c"
case '3' : 
#line 1066 "../../src/lex.c"
case '4' : 
#line 1067 "../../src/lex.c"
case '5' : 
#line 1068 "../../src/lex.c"
case '6' :
#line 1068 "../../src/lex.c"

#line 1069 "../../src/lex.c"
case '7' : 
#line 1070 "../../src/lex.c"
case '8' : 
#line 1071 "../../src/lex.c"
case '9' : 
#line 1072 "../../src/lex.c"
curloc . _loc_line = (((curloc . _loc_line * 10 )+ _au1_c )- '0' );
break ;
default : 
#line 1075 "../../src/lex.c"
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct
#line 1075 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= '#' )));
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 1076 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= _au1_c )));
while ((_au1_c = getc(in_file))!= '\n' )(txtmax <=
#line 1077 "../../src/lex.c"
txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ,
#line 1077 "../../src/lex.c"
(struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= _au1_c )));
(txtmax <= txtfree )? (((int )( errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"input buffer overflow", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 1078 "../../src/lex.c"
ea *)ea0 , (struct ea *)ea0 ) , 0 ) )): (((int )((*(txtfree ++ ))= '\0' )));
fprintf ( out_file , (char *)"\n%s\n", txtstart ) ;
txtstart = txtfree ;
curloc . _loc_line ++ ;
Nline ++ ;
goto ll ;
case '\n' : 
#line 1085 "../../src/lex.c"
_loc_putline ( & curloc ) ;
goto ll ;
}
}
}
;

/* the end */
