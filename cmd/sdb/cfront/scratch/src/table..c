/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#line 1 "../../src/table.c"

/* <<cfront 1.2.1 2/16/87>> */
/* < ../../src/table.c */

#ident	"@(#)sdb:cfront/scratch/src/table..c	1.2"
char *_new(); char _delete(); char *_vec_new(); char _vec_delete();

#line 1 "../../src/table.c"

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

#line 19 "../../src/table.c"
char *keys [256];

#line 29 "../../src/table.c"
struct table *_table__ctor (_au0_this , _au0_sz , _au0_nx , _au0_n )
#line 207 "../../src/cfront.h"
struct table *_au0_this ;

#line 29 "../../src/table.c"
short _au0_sz ;

#line 29 "../../src/table.c"
Ptable _au0_nx ;

#line 29 "../../src/table.c"
Pname _au0_n ;

#line 40 "../../src/table.c"
{ if (_au0_this == 0 )_au0_this = (struct table *)_new ( (long )(sizeof (struct table))) ;
_au0_this -> _node_base = 142 ;
_au0_this -> _table_t_name = _au0_n ;
_au0_this -> _table_size = (_au0_sz = ((_au0_sz <= 0 )? 2 : (_au0_sz + 1 )));

#line 45 "../../src/table.c"
_au0_this -> _table_entries = (((struct name **)_new ( (long )((sizeof (struct name *))* _au0_sz )) ));
_au0_this -> _table_hashsize = (_au0_sz = ((_au0_sz * 3 )/ 2 ));
_au0_this -> _table_hashtbl = (((short *)_new ( (long )((sizeof (short ))* _au0_sz )) ));
_au0_this -> _table_next = _au0_nx ;
_au0_this -> _table_free_slot = 1 ;
return _au0_this ;
}
;
Pname _table_look (_au0_this , _au0_s , _au0_k )
#line 207 "../../src/cfront.h"
struct table *_au0_this ;

#line 53 "../../src/table.c"
char *_au0_s ;

#line 53 "../../src/table.c"
TOK _au0_k ;

#line 58 "../../src/table.c"
{ 
#line 59 "../../src/table.c"
Ptable _au1_t ;
register char *_au1_p ;
register char *_au1_q ;
register int _au1_i ;
Pname _au1_n ;
int _au1_rr ;

#line 72 "../../src/table.c"
_au1_p = _au0_s ;
_au1_i = 0 ;
while (*_au1_p )_au1_i += (_au1_i + (*(_au1_p ++ )));
_au1_rr = ((0 <= _au1_i )? _au1_i : (- _au1_i ));

#line 77 "../../src/table.c"
for(_au1_t = _au0_this ;_au1_t ;_au1_t = _au1_t -> _table_next ) { 
#line 79 "../../src/table.c"
Pname *_au2_np ;
int _au2_mx ;
short *_au2_hash ;
int _au2_firsti ;

#line 79 "../../src/table.c"
_au2_np = _au1_t -> _table_entries ;
_au2_mx = _au1_t -> _table_hashsize ;
_au2_hash = _au1_t -> _table_hashtbl ;
_au2_firsti = (_au1_i = (_au1_rr % _au2_mx ));

#line 84 "../../src/table.c"
do { 
#line 85 "../../src/table.c"
if ((_au2_hash [_au1_i ])== 0 )goto not_found ;
_au1_n = (_au2_np [_au2_hash [_au1_i ]]);
if (_au1_n == 0 )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"hashed lookup", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 87 "../../src/table.c"
ea *)ea0 ) ;
_au1_p = _au1_n -> _expr__O3.__C3_string ;
_au1_q = _au0_s ;
while ((*_au1_p )&& (*_au1_q ))
#line 91 "../../src/table.c"
if ((*(_au1_p ++ ))!= (*(_au1_q ++ )))goto nxt ;
if ((*_au1_p )== (*_au1_q ))goto found ;
nxt :
#line 94 "../../src/table.c"
if (_au2_mx <= (++ _au1_i ))_au1_i = 0 ;
}
while (_au1_i != _au2_firsti );
found :
#line 98 "../../src/table.c"
for(;_au1_n ;_au1_n = _au1_n -> _name_n_tbl_list ) { 
#line 99 "../../src/table.c"
if (_au1_n -> _node_n_key == _au0_k )return _au1_n ;
}

#line 102 "../../src/table.c"
not_found :;
}

#line 105 "../../src/table.c"
return (struct name *)0 ;
}
;
bit Nold ;

#line 110 "../../src/table.c"
Pname _table_insert (_au0_this , _au0_nx , _au0_k )
#line 207 "../../src/cfront.h"
struct table *_au0_this ;

#line 110 "../../src/table.c"
Pname _au0_nx ;

#line 110 "../../src/table.c"
TOK _au0_k ;

#line 116 "../../src/table.c"
{ 
#line 117 "../../src/table.c"
register char *_au1_p ;
register int _au1_i ;
Pname _au1_n ;
Pname *_au1_np ;
Pname *_au1_link ;
int _au1_firsti ;
int _au1_mx ;
short *_au1_hash ;
char *_au1_s ;

#line 120 "../../src/table.c"
_au1_np = _au0_this -> _table_entries ;

#line 123 "../../src/table.c"
_au1_mx = _au0_this -> _table_hashsize ;
_au1_hash = _au0_this -> _table_hashtbl ;
_au1_s = _au0_nx -> _expr__O3.__C3_string ;

#line 127 "../../src/table.c"
if (_au1_s == 0 ){ 
#line 194 "../../src/table.c"
struct ea _au0__V10 ;

#line 194 "../../src/table.c"
struct ea _au0__V11 ;

#line 127 "../../src/table.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"%d->insert(0,%d)", (struct ea *)( ( ((& _au0__V10 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((&
#line 127 "../../src/table.c"
_au0__V10 )))) ) , (struct ea *)( ( ((& _au0__V11 )-> _ea__O1.__C1_i = ((int )_au0_k )), (((& _au0__V11 )))) )
#line 127 "../../src/table.c"
, (struct ea *)ea0 , (struct ea *)ea0 ) ;
} _au0_nx -> _node_n_key = _au0_k ;
if (_au0_nx -> _name_n_tbl_list || _au0_nx -> _expr__O5.__C5_n_table ){ 
#line 194 "../../src/table.c"
struct ea _au0__V12 ;

#line 129 "../../src/table.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"%n in two tables", (struct ea *)( ( ((& _au0__V12 )-> _ea__O1.__C1_p = ((char *)_au0_nx )), (((&
#line 129 "../../src/table.c"
_au0__V12 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} 
#line 132 "../../src/table.c"
_au1_p = _au1_s ;
_au1_i = 0 ;
while (*_au1_p )_au1_i += (_au1_i + (*(_au1_p ++ )));
if (_au1_i < 0 )_au1_i = (- _au1_i );
_au1_firsti = (_au1_i = (_au1_i % _au1_mx ));

#line 138 "../../src/table.c"
do { 
#line 139 "../../src/table.c"
if ((_au1_hash [_au1_i ])== 0 ){ 
#line 140 "../../src/table.c"
(_au1_hash [_au1_i ])= _au0_this -> _table_free_slot ;
goto add_np ;
}
_au1_n = (_au1_np [_au1_hash [_au1_i ]]);
if (_au1_n == 0 )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"hashed lookup", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 144 "../../src/table.c"
ea *)ea0 ) ;
if (strcmp ( (char *)_au1_n -> _expr__O3.__C3_string , (char *)_au1_s ) == 0 )goto found ;

#line 153 "../../src/table.c"
if (_au1_mx <= (++ _au1_i ))_au1_i = 0 ;
}
while (_au1_i != _au1_firsti );
error ( (char *)"N table full", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;

#line 158 "../../src/table.c"
found :
#line 161 "../../src/table.c"
for(;;) { 
#line 162 "../../src/table.c"
if (_au1_n -> _node_n_key == _au0_k ){ Nold = 1 ;

#line 162 "../../src/table.c"
return _au1_n ;
}
if (_au1_n -> _name_n_tbl_list )
#line 165 "../../src/table.c"
_au1_n = _au1_n -> _name_n_tbl_list ;
else { 
#line 167 "../../src/table.c"
_au1_link = (& _au1_n -> _name_n_tbl_list );
goto re_allocate ;
}
}

#line 172 "../../src/table.c"
add_np :
#line 173 "../../src/table.c"
if (_au0_this -> _table_size <= _au0_this -> _table_free_slot ){ 
#line 174 "../../src/table.c"
_table_grow ( _au0_this , 2 * _au0_this -> _table_size ) ;
return _table_insert ( _au0_this , _au0_nx , _au0_k ) ;
}

#line 178 "../../src/table.c"
_au1_link = (& (_au1_np [_au0_this -> _table_free_slot ++ ]));

#line 180 "../../src/table.c"
re_allocate :
#line 181 "../../src/table.c"
{ 
#line 182 "../../src/table.c"
Pname _au2_nw ;

#line 182 "../../src/table.c"
_au2_nw = (struct name *)_name__ctor ( (struct name *)0 , (char *)0 ) ;
(*_au2_nw )= (*_au0_nx );
{ char *_au2_ps ;

#line 184 "../../src/table.c"
_au2_ps = (((char *)_new ( (long )((sizeof (char ))* (strlen ( (char *)_au1_s ) + 1 ))) ));
strcpy ( _au2_ps , (char *)_au1_s ) ;
Nstr ++ ;
_au2_nw -> _expr__O3.__C3_string = _au2_ps ;
_au2_nw -> _expr__O5.__C5_n_table = _au0_this ;
(*_au1_link )= _au2_nw ;
Nold = 0 ;
Nname ++ ;
return _au2_nw ;
}
}
}
;

#line 196 "../../src/table.c"
char _table_grow (_au0_this , _au0_g )
#line 207 "../../src/cfront.h"
struct table *_au0_this ;

#line 196 "../../src/table.c"
int _au0_g ;
{ 
#line 198 "../../src/table.c"
short *_au1_hash ;
register int _au1_j ;
int _au1_mx ;
register Pname *_au1_np ;
Pname _au1_n ;

#line 204 "../../src/table.c"
if (_au0_g <= _au0_this -> _table_free_slot ){ 
#line 253 "../../src/table.c"
struct ea _au0__V13 ;

#line 253 "../../src/table.c"
struct ea _au0__V14 ;

#line 204 "../../src/table.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"table.grow(%d,%d)", (struct ea *)( ( ((& _au0__V13 )-> _ea__O1.__C1_i = _au0_g ), (((& _au0__V13 ))))
#line 204 "../../src/table.c"
) , (struct ea *)( ( ((& _au0__V14 )-> _ea__O1.__C1_i = ((int )_au0_this -> _table_free_slot )), (((& _au0__V14 )))) )
#line 204 "../../src/table.c"
, (struct ea *)ea0 , (struct ea *)ea0 ) ;
} if (_au0_g <= _au0_this -> _table_size )return ;

#line 207 "../../src/table.c"
_au0_this -> _table_size = (_au1_mx = (_au0_g + 1 ));

#line 209 "../../src/table.c"
_au1_np = (((struct name **)_new ( (long )((sizeof (struct name *))* _au1_mx )) ));
for(_au1_j = 0 ;_au1_j < _au0_this -> _table_free_slot ;_au1_j ++ ) (_au1_np [_au1_j ])= (_au0_this -> _table_entries [_au1_j ]);
_delete ( (char *)_au0_this -> _table_entries ) ;
_au0_this -> _table_entries = _au1_np ;

#line 214 "../../src/table.c"
_delete ( (char *)_au0_this -> _table_hashtbl ) ;
_au0_this -> _table_hashsize = (_au1_mx = ((_au0_g * 3 )/ 2 ));
_au1_hash = (_au0_this -> _table_hashtbl = (((short *)_new ( (long )((sizeof (short ))* _au1_mx )) )));

#line 218 "../../src/table.c"
for(_au1_j = 1 ;_au1_j < _au0_this -> _table_free_slot ;_au1_j ++ ) { 
#line 219 "../../src/table.c"
char *_au2_s ;
register char *_au2_p ;
char *_au2_q ;
register int _au2_i ;
int _au2_firsti ;

#line 219 "../../src/table.c"
_au2_s = (_au1_np [_au1_j ])-> _expr__O3.__C3_string ;

#line 225 "../../src/table.c"
_au2_p = _au2_s ;
_au2_i = 0 ;
while (*_au2_p )_au2_i += (_au2_i + (*(_au2_p ++ )));
if (_au2_i < 0 )_au2_i = (- _au2_i );
_au2_firsti = (_au2_i = (_au2_i % _au1_mx ));

#line 231 "../../src/table.c"
do { 
#line 232 "../../src/table.c"
if ((_au1_hash [_au2_i ])== 0 ){ 
#line 233 "../../src/table.c"
(_au1_hash [_au2_i ])= _au1_j ;
goto add_np ;
}
_au1_n = (_au1_np [_au1_hash [_au2_i ]]);
if (_au1_n == 0 )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"hashed lookup", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 237 "../../src/table.c"
ea *)ea0 ) ;
_au2_p = _au1_n -> _expr__O3.__C3_string ;
_au2_q = _au2_s ;
while ((*_au2_p )&& (*_au2_q ))if ((*(_au2_p ++ ))!= (*(_au2_q ++ )))goto nxt ;
if ((*_au2_p )== (*_au2_q ))goto found ;
nxt :
#line 243 "../../src/table.c"
if (_au1_mx <= (++ _au2_i ))_au2_i = 0 ;
}
while (_au2_i != _au2_firsti );
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"rehash??", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 246 "../../src/table.c"

#line 248 "../../src/table.c"
found :
#line 249 "../../src/table.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"rehash failed", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 249 "../../src/table.c"

#line 251 "../../src/table.c"
add_np :;
}
}
;
Pclass Ebase ;
Pclass Epriv ;

#line 258 "../../src/table.c"
Pname _table_lookc (_au0_this , _au0_s , _au0__A15 )
#line 207 "../../src/cfront.h"
struct table *_au0_this ;

#line 258 "../../src/table.c"
char *_au0_s ;

#line 258 "../../src/table.c"
TOK _au0__A15 ;

#line 265 "../../src/table.c"
{ 
#line 266 "../../src/table.c"
Ptable _au1_t ;
register char *_au1_p ;
register char *_au1_q ;
register int _au1_i ;
Pname _au1_n ;
int _au1_rr ;

#line 277 "../../src/table.c"
Ebase = 0 ;
Epriv = 0 ;

#line 282 "../../src/table.c"
_au1_p = _au0_s ;
_au1_i = 0 ;
while (*_au1_p )_au1_i += (_au1_i + (*(_au1_p ++ )));
_au1_rr = ((0 <= _au1_i )? _au1_i : (- _au1_i ));

#line 287 "../../src/table.c"
for(_au1_t = _au0_this ;_au1_t ;_au1_t = _au1_t -> _table_next ) { 
#line 289 "../../src/table.c"
Pname *_au2_np ;
int _au2_mx ;
short *_au2_hash ;
int _au2_firsti ;
Pname _au2_tname ;

#line 289 "../../src/table.c"
_au2_np = _au1_t -> _table_entries ;
_au2_mx = _au1_t -> _table_hashsize ;
_au2_hash = _au1_t -> _table_hashtbl ;
_au2_firsti = (_au1_i = (_au1_rr % _au2_mx ));
_au2_tname = _au1_t -> _table_t_name ;

#line 295 "../../src/table.c"
do { 
#line 296 "../../src/table.c"
if ((_au2_hash [_au1_i ])== 0 )goto not_found ;
_au1_n = (_au2_np [_au2_hash [_au1_i ]]);
if (_au1_n == 0 )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"hashed lookup", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 298 "../../src/table.c"
ea *)ea0 ) ;
_au1_p = _au1_n -> _expr__O3.__C3_string ;
_au1_q = _au0_s ;
while ((*_au1_p )&& (*_au1_q ))
#line 302 "../../src/table.c"
if ((*(_au1_p ++ ))!= (*(_au1_q ++ )))goto nxt ;
if ((*_au1_p )== (*_au1_q ))goto found ;
nxt :
#line 305 "../../src/table.c"
if (_au2_mx <= (++ _au1_i ))_au1_i = 0 ;
}
while (_au1_i != _au2_firsti );
found :
#line 309 "../../src/table.c"
do { 
#line 310 "../../src/table.c"
if (_au1_n -> _node_n_key == 0 ){ 
#line 311 "../../src/table.c"
if (_au2_tname ){ 
#line 312 "../../src/table.c"
if (_au1_n -> _node_base == 25 )
#line 313 "../../src/table.c"
_au1_n = _au1_n -> _name__O6.__C6_n_qualifier ;
#line 313 "../../src/table.c"
else 
#line 314 "../../src/table.c"
if (_au1_n -> _name_n_scope == 0 )
#line 315 "../../src/table.c"
Epriv = (((struct classdef *)_au2_tname -> _expr__O2.__C2_tp ));
}
return _au1_n ;
}
}
while (_au1_n = _au1_n -> _name_n_tbl_list );

#line 322 "../../src/table.c"
not_found :
#line 323 "../../src/table.c"
if (_au2_tname ){ 
#line 324 "../../src/table.c"
Pclass _au3_cl ;

#line 324 "../../src/table.c"
_au3_cl = (((struct classdef *)_au2_tname -> _expr__O2.__C2_tp ));
if ((_au3_cl && _au3_cl -> _classdef_clbase )&& (_au3_cl -> _classdef_pubbase == 0 ))Ebase = (((struct classdef *)_au3_cl -> _classdef_clbase -> _expr__O2.__C2_tp ));
}
}

#line 329 "../../src/table.c"
Ebase = (Epriv = 0 );
return (struct name *)0 ;
}
;

#line 334 "../../src/table.c"
Pname _table_get_mem (_au0_this , _au0_i )
#line 207 "../../src/cfront.h"
struct table *_au0_this ;

#line 334 "../../src/table.c"
int _au0_i ;

#line 338 "../../src/table.c"
{ 
#line 339 "../../src/table.c"
return (((_au0_i <= 0 )|| (_au0_this -> _table_free_slot <= _au0_i ))? (((struct name *)0 )): (_au0_this -> _table_entries [_au0_i ]));
}
;
extern char new_key (_au0_s , _au0_toknum , _au0_yyclass )char *_au0_s ;

#line 342 "../../src/table.c"
TOK _au0_toknum ;

#line 342 "../../src/table.c"
TOK _au0_yyclass ;

#line 348 "../../src/table.c"
{ 
#line 349 "../../src/table.c"
Pname _au1_n ;
Pname _au1_nn ;

#line 349 "../../src/table.c"
_au1_n = (struct name *)_name__ctor ( (struct name *)0 , _au0_s ) ;
_au1_nn = _table_insert ( ktbl , _au1_n , (unsigned char )0 ) ;

#line 352 "../../src/table.c"
_au1_nn -> _node_base = _au0_toknum ;
_au1_nn -> _expr__O2.__C2_syn_class = (_au0_yyclass ? _au0_yyclass : _au0_toknum );
(keys [(_au0_toknum == 143 )? _au0_yyclass : _au0_toknum ])= _au0_s ;
_name__dtor ( _au1_n , 1) ;
}
;

#line 359 "../../src/table.c"
Pexpr _table_find_name (_au0_this , _au0_n , _au0_f , _au0__A16 )
#line 207 "../../src/cfront.h"
struct table *_au0_this ;

#line 359 "../../src/table.c"
register Pname _au0_n ;

#line 359 "../../src/table.c"
bit _au0_f ;

#line 359 "../../src/table.c"
Pexpr _au0__A16 ;

#line 367 "../../src/table.c"
{ 
#line 368 "../../src/table.c"
Pname _au1_q ;
register Pname _au1_qn ;
register Pname _au1_nn ;
Pclass _au1_cl ;

#line 368 "../../src/table.c"
_au1_q = _au0_n -> _name__O6.__C6_n_qualifier ;
_au1_qn = 0 ;

#line 373 "../../src/table.c"
if (_au0_n -> _expr__O5.__C5_n_table ){ 
#line 374 "../../src/table.c"
_au1_nn = _au0_n ;
_au0_n = 0 ;
if (_au0_f == 3 )_au0_f = 0 ;
goto xx ;
}

#line 380 "../../src/table.c"
if (_au1_q ){ 
#line 381 "../../src/table.c"
Ptable _au2_tbl ;

#line 383 "../../src/table.c"
if (_au1_q == sta_name )
#line 384 "../../src/table.c"
_au2_tbl = gtbl ;
else { 
#line 386 "../../src/table.c"
Ptype _au3_t ;

#line 386 "../../src/table.c"
_au3_t = (struct type *)(((struct classdef *)_au1_q -> _expr__O2.__C2_tp ));
if (_au3_t == 0 ){ 
#line 664 "../../src/table.c"
struct ea _au0__V17 ;

#line 387 "../../src/table.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"Qr%n'sT missing", (struct ea *)( ( ((& _au0__V17 )-> _ea__O1.__C1_p = ((char *)_au1_q )), (((&
#line 387 "../../src/table.c"
_au0__V17 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} 
#line 389 "../../src/table.c"
if (_au1_q -> _node_base == 123 ){ 
#line 390 "../../src/table.c"
if (_au3_t -> _node_base != 119 ){ 
#line 391 "../../src/table.c"
{ 
#line 664 "../../src/table.c"
struct ea _au0__V18 ;

#line 664 "../../src/table.c"
struct ea _au0__V19 ;

#line 391 "../../src/table.c"
error ( (char *)"badT%k forQr%n", (struct ea *)( ( ((& _au0__V18 )-> _ea__O1.__C1_i = ((int )_au3_t -> _node_base )), (((& _au0__V18 ))))
#line 391 "../../src/table.c"
) , (struct ea *)( ( ((& _au0__V19 )-> _ea__O1.__C1_p = ((char *)_au1_q )), (((& _au0__V19 )))) ) ,
#line 391 "../../src/table.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
goto nq ;
} }
_au3_t = (((struct basetype *)_au3_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
}
if (_au3_t -> _node_base != 6 ){ 
#line 397 "../../src/table.c"
{ 
#line 664 "../../src/table.c"
struct ea _au0__V20 ;

#line 664 "../../src/table.c"
struct ea _au0__V21 ;

#line 397 "../../src/table.c"
error ( (char *)"badQr%n(%k)", (struct ea *)( ( ((& _au0__V20 )-> _ea__O1.__C1_p = ((char *)_au1_q )), (((& _au0__V20 )))) )
#line 397 "../../src/table.c"
, (struct ea *)( ( ((& _au0__V21 )-> _ea__O1.__C1_i = ((int )_au3_t -> _node_base )), (((& _au0__V21 )))) ) ,
#line 397 "../../src/table.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
goto nq ;
} }
_au1_cl = (((struct classdef *)_au3_t ));
_au2_tbl = _au1_cl -> _classdef_memtbl ;
}

#line 404 "../../src/table.c"
_au1_qn = _table_look ( _au2_tbl , _au0_n -> _expr__O3.__C3_string , (unsigned char )0 ) ;

#line 406 "../../src/table.c"
if (_au1_qn == 0 ){ 
#line 407 "../../src/table.c"
_au0_n -> _name__O6.__C6_n_qualifier = 0 ;
_au1_nn = 0 ;
goto def ;
}

#line 412 "../../src/table.c"
if (_au1_q == sta_name ){ 
#line 413 "../../src/table.c"
( (_au1_qn -> _name_n_used ++ )) ;
_name__dtor ( _au0_n , 1) ;
return (struct expr *)_au1_qn ;
}
}
else 
#line 421 "../../src/table.c"
if (_au0_f == 3 )_au0_f = 0 ;

#line 423 "../../src/table.c"
nq :
#line 424 "../../src/table.c"
if (cc -> _dcl_context_tot ){ 
#line 425 "../../src/table.c"
{ { Ptable _au3_tbl ;

#line 425 "../../src/table.c"
_au3_tbl = _au0_this ;

#line 425 "../../src/table.c"
for(;;) { 
#line 427 "../../src/table.c"
_au1_nn = _table_lookc ( _au3_tbl , _au0_n -> _expr__O3.__C3_string , (unsigned char )0 ) ;

#line 429 "../../src/table.c"
if (_au1_nn == 0 )goto qq ;

#line 431 "../../src/table.c"
switch (_au1_nn -> _name_n_scope ){ 
#line 432 "../../src/table.c"
case 0 : 
#line 433 "../../src/table.c"
case 25 : 
#line 434 "../../src/table.c"
if (_au1_nn -> _name_n_stclass == 13 )break ;

#line 436 "../../src/table.c"
if (_au1_nn -> _expr__O2.__C2_tp -> _node_base == 76 )break ;

#line 438 "../../src/table.c"
if (((Ebase && cc -> _dcl_context_cot -> _classdef_clbase )&& (Ebase != (struct classdef *)cc -> _dcl_context_cot -> _classdef_clbase -> _expr__O2.__C2_tp ))&& (! _classdef_has_friend ( Ebase ,
#line 438 "../../src/table.c"
cc -> _dcl_context_nof ) ))
#line 442 "../../src/table.c"
{ 
#line 664 "../../src/table.c"
struct ea _au0__V22 ;

#line 442 "../../src/table.c"
error ( (char *)"%n is from a privateBC", (struct ea *)( ( ((& _au0__V22 )-> _ea__O1.__C1_p = ((char *)_au0_n )), (((& _au0__V22 )))) )
#line 442 "../../src/table.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} 
#line 444 "../../src/table.c"
if (((Epriv && (Epriv != cc -> _dcl_context_cot ))&& (! _classdef_has_friend ( Epriv , cc -> _dcl_context_nof ) ))&& (! (_au1_nn ->
#line 444 "../../src/table.c"
_name_n_protect && _classdef_baseofFPCname___ ( Epriv , cc -> _dcl_context_nof ) )))
#line 448 "../../src/table.c"
{ 
#line 664 "../../src/table.c"
struct ea _au0__V23 ;

#line 664 "../../src/table.c"
struct ea _au0__V24 ;

#line 448 "../../src/table.c"
error ( (char *)"%n is %s", (struct ea *)( ( ((& _au0__V23 )-> _ea__O1.__C1_p = ((char *)_au0_n )), (((& _au0__V23 )))) )
#line 448 "../../src/table.c"
, (struct ea *)( ( ((& _au0__V24 )-> _ea__O1.__C1_p = ((char *)(_au1_nn -> _name_n_protect ? "protected": "private"))), (((& _au0__V24 ))))
#line 448 "../../src/table.c"
) , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }

#line 451 "../../src/table.c"
if ((_au1_qn == 0 )|| (_au1_qn == _au1_nn ))break ;

#line 453 "../../src/table.c"
if ((_au3_tbl = _au3_tbl -> _table_next )== 0 ){ 
#line 454 "../../src/table.c"
if ((_au1_qn -> _name_n_scope == 25 )|| _classdef_has_friend ( _au1_cl , cc -> _dcl_context_nof ) )
#line 458 "../../src/table.c"
{
#line 458 "../../src/table.c"

#line 459 "../../src/table.c"
_au1_nn = _au1_qn ;
break ;
}
else { 
#line 463 "../../src/table.c"
if (_au0_f != 3 ){ 
#line 464 "../../src/table.c"
{ 
#line 664 "../../src/table.c"
struct ea _au0__V25 ;

#line 464 "../../src/table.c"
error ( (char *)"QdN%n not in scope", (struct ea *)( ( ((& _au0__V25 )-> _ea__O1.__C1_p = ((char *)_au0_n )), (((& _au0__V25 )))) )
#line 464 "../../src/table.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
goto def ;
} }
break ;
}
}
}
}
}

#line 472 "../../src/table.c"
xx :
#line 474 "../../src/table.c"
if (_au1_nn == 0 )goto def ;
( (_au1_nn -> _name_n_used ++ )) ;
if (2 <= _au0_f ){ 
#line 477 "../../src/table.c"
if (_au1_qn && (_au1_nn -> _name_n_stclass == 0 ))
#line 478 "../../src/table.c"
switch (_au1_nn -> _name_n_scope ){ 
#line 479 "../../src/table.c"
case 0 : 
#line 480 "../../src/table.c"
case 25 :
#line 480 "../../src/table.c"

#line 481 "../../src/table.c"
switch (_au1_qn -> _expr__O2.__C2_tp -> _node_base ){ 
#line 482 "../../src/table.c"
case 108 : 
#line 483 "../../src/table.c"
case 76 : 
#line 484 "../../src/table.c"
if (_au0_f == 3 )return (struct expr *)_au1_qn ;
(*_au0_n )= (*_au1_qn );
_au0_n -> _name__O6.__C6_n_qualifier = _au1_q ;
return (struct expr *)_au0_n ;
}
}
if (_au1_nn -> _expr__O5.__C5_n_table == gtbl ){ 
#line 664 "../../src/table.c"
struct ea _au0__V26 ;

#line 490 "../../src/table.c"
error ( (char *)"M%n not found", (struct ea *)( ( ((& _au0__V26 )-> _ea__O1.__C1_p = ((char *)_au0_n )), (((& _au0__V26 )))) )
#line 490 "../../src/table.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} if (_au0_n )_name__dtor ( _au0_n , 1) ;
return (struct expr *)_au1_nn ;
}

#line 495 "../../src/table.c"
switch (_au1_nn -> _name_n_scope ){ 
#line 496 "../../src/table.c"
case 0 : 
#line 497 "../../src/table.c"
case 25 : 
#line 499 "../../src/table.c"
switch (_au1_nn -> _name_n_stclass ){ 
#line 500 "../../src/table.c"
case 0 : 
#line 501 "../../src/table.c"
if (_au1_qn ){
#line 501 "../../src/table.c"

#line 502 "../../src/table.c"
switch (_au1_qn -> _expr__O2.__C2_tp -> _node_base ){ 
#line 503 "../../src/table.c"
case 108 : 
#line 504 "../../src/table.c"
case 76 : 
#line 505 "../../src/table.c"
(*_au0_n )= (*_au1_qn );
_au0_n -> _name__O6.__C6_n_qualifier = _au1_q ;
_au1_nn = _au0_n ;
_au0_n = 0 ;
}
}

#line 512 "../../src/table.c"
if (cc -> _dcl_context_c_this == 0 ){ 
#line 513 "../../src/table.c"
switch (_au1_nn -> _name_n_oper ){ 
#line 514 "../../src/table.c"
case 161 : 
#line 515 "../../src/table.c"
case 162 : 
#line 516 "../../src/table.c"
break ;
default : 
#line 518 "../../src/table.c"
{ 
#line 664 "../../src/table.c"
struct ea _au0__V27 ;

#line 518 "../../src/table.c"
error ( (char *)"%n cannot be used here", (struct ea *)( ( ((& _au0__V27 )-> _ea__O1.__C1_p = ((char *)_au1_nn )), (((& _au0__V27 )))) )
#line 518 "../../src/table.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
return (struct expr *)_au1_nn ;
} }
}

#line 523 "../../src/table.c"
if (_au0_n )_name__dtor ( _au0_n , 1) ;
{ Pref _au5_r ;

#line 525 "../../src/table.c"
struct ref *_au0__Xthis__ctor_ref ;

#line 524 "../../src/table.c"
_au5_r = (struct ref *)( (_au0__Xthis__ctor_ref = 0 ), ( ( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ref ),
#line 524 "../../src/table.c"
((unsigned char )44 ), ((struct expr *)cc -> _dcl_context_c_this ), (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = _au1_nn ),
#line 524 "../../src/table.c"
((_au0__Xthis__ctor_ref ))) ) ) ;
( (cc -> _dcl_context_c_this -> _name_n_used ++ )) ;
_au5_r -> _expr__O2.__C2_tp = _au1_nn -> _expr__O2.__C2_tp ;
return (struct expr *)_au5_r ;
}
}
default : 
#line 531 "../../src/table.c"
if (_au0_n )_name__dtor ( _au0_n , 1) ;
return (struct expr *)_au1_nn ;
}
}
qq :
#line 537 "../../src/table.c"
if (_au1_qn ){ 
#line 544 "../../src/table.c"
if ((_au1_qn -> _name_n_scope == 0 )&& (! _classdef_has_friend ( _au1_cl , cc -> _dcl_context_nof ) )){ 
#line 545 "../../src/table.c"
{ 
#line 664 "../../src/table.c"
struct
#line 664 "../../src/table.c"
ea _au0__V28 ;

#line 545 "../../src/table.c"
error ( (char *)"%n is private", (struct ea *)( ( ((& _au0__V28 )-> _ea__O1.__C1_p = ((char *)_au1_qn )), (((& _au0__V28 )))) )
#line 545 "../../src/table.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
if (_au0_n )_name__dtor ( _au0_n , 1) ;
return (struct expr *)_au1_qn ;
} }

#line 550 "../../src/table.c"
switch (_au1_qn -> _name_n_stclass ){ 
#line 551 "../../src/table.c"
case 31 : 
#line 552 "../../src/table.c"
break ;
default : 
#line 554 "../../src/table.c"
switch (_au1_qn -> _expr__O2.__C2_tp -> _node_base ){ 
#line 555 "../../src/table.c"
case 108 : 
#line 556 "../../src/table.c"
case 76 : 
#line 557 "../../src/table.c"
if (_au0_f == 1 ){ 
#line 664 "../../src/table.c"
struct
#line 664 "../../src/table.c"
ea _au0__V29 ;

#line 557 "../../src/table.c"
error ( (char *)"O missing for%n", (struct ea *)( ( ((& _au0__V29 )-> _ea__O1.__C1_p = ((char *)_au1_qn )), (((& _au0__V29 )))) )
#line 557 "../../src/table.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} if (_au0_f == 3 )return (struct expr *)_au1_qn ;
(*_au0_n )= (*_au1_qn );
_au0_n -> _name__O6.__C6_n_qualifier = _au1_q ;
return (struct expr *)_au0_n ;
default : 
#line 563 "../../src/table.c"
if (_au0_f < 2 ){ 
#line 664 "../../src/table.c"
struct ea _au0__V30 ;

#line 563 "../../src/table.c"
error ( (char *)"O missing for%n", (struct ea *)( ( ((& _au0__V30 )-> _ea__O1.__C1_p = ((char *)_au1_qn )), (((& _au0__V30 )))) )
#line 563 "../../src/table.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}

#line 567 "../../src/table.c"
if (_au0_n )_name__dtor ( _au0_n , 1) ;
return (struct expr *)_au1_qn ;
}

#line 571 "../../src/table.c"
if (_au1_nn = _table_lookc ( _au0_this , _au0_n -> _expr__O3.__C3_string , (unsigned char )0 ) ){ 
#line 572 "../../src/table.c"
switch (_au1_nn -> _name_n_scope ){ 
#line 573 "../../src/table.c"
case
#line 573 "../../src/table.c"
0 : 
#line 574 "../../src/table.c"
case 25 : 
#line 575 "../../src/table.c"
if (_au1_nn -> _name_n_stclass == 13 )break ;

#line 577 "../../src/table.c"
if (_au1_nn -> _expr__O2.__C2_tp -> _node_base == 76 )break ;

#line 579 "../../src/table.c"
if (Ebase && (! _classdef_has_friend ( Ebase , cc -> _dcl_context_nof ) ))
#line 580 "../../src/table.c"
{ 
#line 664 "../../src/table.c"
struct ea _au0__V31 ;

#line 580 "../../src/table.c"
error ( (char *)"%n is from privateBC", (struct ea *)( ( ((& _au0__V31 )-> _ea__O1.__C1_p = ((char *)_au0_n )), (((& _au0__V31 )))) )
#line 580 "../../src/table.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} 
#line 582 "../../src/table.c"
if ((Epriv && (! _classdef_has_friend ( Epriv , cc -> _dcl_context_nof ) ))&& (! (_au1_nn -> _name_n_protect && _classdef_baseofFPCname___ ( Epriv ,
#line 582 "../../src/table.c"
cc -> _dcl_context_nof ) )))
#line 585 "../../src/table.c"
{ 
#line 664 "../../src/table.c"
struct ea _au0__V32 ;

#line 664 "../../src/table.c"
struct ea _au0__V33 ;

#line 585 "../../src/table.c"
error ( (char *)"%n is %s", (struct ea *)( ( ((& _au0__V32 )-> _ea__O1.__C1_p = ((char *)_au0_n )), (((& _au0__V32 )))) )
#line 585 "../../src/table.c"
, (struct ea *)( ( ((& _au0__V33 )-> _ea__O1.__C1_p = ((char *)(_au1_nn -> _name_n_protect ? "protected": "private"))), (((& _au0__V33 ))))
#line 585 "../../src/table.c"
) , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}

#line 589 "../../src/table.c"
if (_au1_nn ){ 
#line 591 "../../src/table.c"
if ((_au0_f == 2 )&& (_au1_nn -> _expr__O5.__C5_n_table == gtbl )){ 
#line 664 "../../src/table.c"
struct ea _au0__V34 ;

#line 591 "../../src/table.c"
error ( (char *)"M%n not found", (struct ea *)( ( ((& _au0__V34 )-> _ea__O1.__C1_p = ((char *)_au0_n )), (((& _au0__V34 )))) )
#line 591 "../../src/table.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} ( (_au1_nn -> _name_n_used ++ )) ;
if (_au0_n )_name__dtor ( _au0_n , 1) ;
return (struct expr *)_au1_nn ;
}

#line 597 "../../src/table.c"
def :
#line 599 "../../src/table.c"
_au0_n -> _name__O6.__C6_n_qualifier = 0 ;
if (_au0_f == 1 ){ 
#line 601 "../../src/table.c"
if (_au0_n -> _expr__O2.__C2_tp )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"find_name(fct_type?)", (struct ea *)ea0 , (struct ea *)ea0 ,
#line 601 "../../src/table.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
_au0_n -> _expr__O2.__C2_tp = (struct type *)_fct__ctor ( (struct fct *)0 , (struct type *)defa_type , (struct name *)0 , (unsigned char )0 ) ;
#line 602 "../../src/table.c"

#line 603 "../../src/table.c"
_au0_n -> _name_n_sto = 14 ;
}
else 
#line 639 "../../src/table.c"
{ 
#line 640 "../../src/table.c"
_au0_n -> _expr__O2.__C2_tp = (struct type *)any_type ;
if (_au0_this != any_tbl )
#line 642 "../../src/table.c"
if ((cc -> _dcl_context_not && (! strcmp ( (char *)_au0_n -> _expr__O3.__C3_string , (char *)cc -> _dcl_context_not -> _expr__O3.__C3_string )
#line 642 "../../src/table.c"
))&& ((cc -> _dcl_context_cot -> _type_defined & 3)== 0 ))
#line 645 "../../src/table.c"
{ 
#line 664 "../../src/table.c"
struct ea _au0__V35 ;

#line 645 "../../src/table.c"
error ( (char *)"C%n isU", (struct ea *)( ( ((& _au0__V35 )-> _ea__O1.__C1_p = ((char *)cc -> _dcl_context_not )), (((& _au0__V35 ))))
#line 645 "../../src/table.c"
) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} else 
#line 647 "../../src/table.c"
{ 
#line 664 "../../src/table.c"
struct ea _au0__V36 ;

#line 647 "../../src/table.c"
error ( (char *)"%n isU", (struct ea *)( ( ((& _au0__V36 )-> _ea__O1.__C1_p = ((char *)_au0_n )), (((& _au0__V36 )))) )
#line 647 "../../src/table.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }

#line 650 "../../src/table.c"
_au1_nn = _name_dcl ( _au0_n , gtbl , (unsigned char )14 ) ;
_au1_nn -> _name_n_list = 0 ;
( (_au1_nn -> _name_n_used ++ )) ;
( (_au1_nn -> _name_n_used ++ )) ;
if (_au0_n )_name__dtor ( _au0_n , 1) ;

#line 656 "../../src/table.c"
if (_au0_f == 1 )
#line 657 "../../src/table.c"
if (fct_void ){ 
#line 658 "../../src/table.c"
if ((no_of_undcl ++ )== 0 )undcl = _au1_nn ;
}
else 
#line 661 "../../src/table.c"
{ 
#line 664 "../../src/table.c"
struct ea _au0__V37 ;

#line 661 "../../src/table.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"undeclaredF%n called", (struct ea *)( ( ((& _au0__V37 )-> _ea__O1.__C1_p = ((char *)_au1_nn )), (((&
#line 661 "../../src/table.c"
_au0__V37 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} 
#line 663 "../../src/table.c"
return (struct expr *)_au1_nn ;
}
;

/* the end */
