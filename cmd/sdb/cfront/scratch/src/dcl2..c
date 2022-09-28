/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#line 1 "../../src/dcl2.c"

/* <<cfront 1.2.1 2/16/87>> */
/* < ../../src/dcl2.c */

#ident	"@(#)sdb:cfront/scratch/src/dcl2..c	1.2"
char *_new(); char _delete(); char *_vec_new(); char _vec_delete();

#line 1 "../../src/dcl2.c"

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

#line 20 "../../src/dcl2.c"
extern int tsize ;
char _classdef_dcl (_au0_this , _au0_cname , _au0_tbl )
#line 319 "../../src/cfront.h"
struct classdef *_au0_this ;

#line 21 "../../src/dcl2.c"
Pname _au0_cname ;

#line 21 "../../src/dcl2.c"
Ptable _au0_tbl ;
{ 
#line 23 "../../src/dcl2.c"
int _au1_nmem ;
Pname _au1_p ;
Pptr _au1_cct ;
Pbase _au1_bt ;
Pname _au1_px ;
Ptable _au1_btbl ;
int _au1_bvirt ;
Pclass _au1_bcl ;
int _au1_i ;
int _au1_fct_seen ;
int _au1_static_seen ;
int _au1_local ;
int _au1_scope ;
int _au1_protect ;
Pname _au1_publist ;

#line 39 "../../src/dcl2.c"
int _au1_byte_old ;
int _au1_bit_old ;
int _au1_max_old ;
int _au1_boff ;

#line 44 "../../src/dcl2.c"
int _au1_in_union ;
int _au1_usz ;
int _au1_make_ctor ;
int _au1_make_dtor ;

#line 354 "../../src/dcl2.c"
Pname _au1_pnx ;

#line 355 "../../src/dcl2.c"
struct ptr *_au0__Xthis__ctor_ptr ;

#line 32 "../../src/dcl2.c"
_au1_fct_seen = 0 ;
_au1_static_seen = 0 ;
_au1_local = (_au0_tbl != gtbl );
_au1_scope = 25 ;
_au1_protect = 0 ;
_au1_publist = 0 ;

#line 39 "../../src/dcl2.c"
_au1_byte_old = byte_offset ;
_au1_bit_old = bit_offset ;
_au1_max_old = max_align ;

#line 44 "../../src/dcl2.c"
_au1_in_union = 0 ;

#line 46 "../../src/dcl2.c"
_au1_make_ctor = 0 ;
_au1_make_dtor = 0 ;

#line 50 "../../src/dcl2.c"
if (_au0_this == 0 ){ 
#line 575 "../../src/dcl2.c"
struct ea _au0__V10 ;

#line 50 "../../src/dcl2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"0->Cdef::dcl(%p)", (struct ea *)( ( ((& _au0__V10 )-> _ea__O1.__C1_p = ((char *)_au0_tbl )), (((&
#line 50 "../../src/dcl2.c"
_au0__V10 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} if (_au0_this -> _node_base != 6 ){ 
#line 575 "../../src/dcl2.c"
struct ea _au0__V11 ;

#line 51 "../../src/dcl2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"Cdef::dcl(%d)", (struct ea *)( ( ((& _au0__V11 )-> _ea__O1.__C1_i = ((int )_au0_this -> _node_base )),
#line 51 "../../src/dcl2.c"
(((& _au0__V11 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} if (_au0_cname == 0 )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"unNdC", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ,
#line 52 "../../src/dcl2.c"
(struct ea *)ea0 ) ;
if (_au0_cname -> _expr__O2.__C2_tp != (struct type *)_au0_this )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"badCdef", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 53 "../../src/dcl2.c"
ea *)ea0 , (struct ea *)ea0 ) ;
if (_au0_tbl == 0 ){ 
#line 575 "../../src/dcl2.c"
struct ea _au0__V12 ;

#line 54 "../../src/dcl2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"Cdef::dcl(%n,0)", (struct ea *)( ( ((& _au0__V12 )-> _ea__O1.__C1_p = ((char *)_au0_cname )), (((&
#line 54 "../../src/dcl2.c"
_au0__V12 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} if (_au0_tbl -> _node_base != 142 ){ 
#line 575 "../../src/dcl2.c"
struct ea _au0__V13 ;

#line 575 "../../src/dcl2.c"
struct ea _au0__V14 ;

#line 55 "../../src/dcl2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"Cdef::dcl(%n,tbl=%d)", (struct ea *)( ( ((& _au0__V13 )-> _ea__O1.__C1_p = ((char *)_au0_cname )), (((&
#line 55 "../../src/dcl2.c"
_au0__V13 )))) ) , (struct ea *)( ( ((& _au0__V14 )-> _ea__O1.__C1_i = ((int )_au0_tbl -> _node_base )), (((& _au0__V14 ))))
#line 55 "../../src/dcl2.c"
) , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} 
#line 57 "../../src/dcl2.c"
_au1_nmem = (_name_no_of_names ( _au0_this -> _classdef_mem_list ) + _name_no_of_names ( _au0_this -> _classdef_pubdef ) );

#line 59 "../../src/dcl2.c"
switch (_au0_this -> _classdef_csu ){ 
#line 60 "../../src/dcl2.c"
case 36 : 
#line 61 "../../src/dcl2.c"
case 167 : 
#line 62 "../../src/dcl2.c"
_au1_in_union = 1 ;
if (_au0_this -> _classdef_virt_count )error ( (char *)"virtualF in union", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 63 "../../src/dcl2.c"

#line 64 "../../src/dcl2.c"
break ;
case 6 : 
#line 66 "../../src/dcl2.c"
_au1_scope = 0 ;
if (_au0_this -> _classdef_virt_count == 0 )_au0_this -> _classdef_csu = 32 ;
}

#line 70 "../../src/dcl2.c"
if (_au0_this -> _classdef_clbase ){ 
#line 71 "../../src/dcl2.c"
if (_au0_this -> _classdef_clbase -> _node_base != 123 ){ 
#line 575 "../../src/dcl2.c"
struct ea _au0__V15 ;

#line 71 "../../src/dcl2.c"
error ( (char *)"BC%nU", (struct ea *)( ( ((& _au0__V15 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _classdef_clbase )), (((& _au0__V15 ))))
#line 71 "../../src/dcl2.c"
) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} _au0_this -> _classdef_clbase = (((struct basetype *)_au0_this -> _classdef_clbase -> _expr__O2.__C2_tp ))-> _basetype_b_name ;
_au1_bcl = (((struct classdef *)_au0_this -> _classdef_clbase -> _expr__O2.__C2_tp ));
if ((_au1_bcl -> _type_defined & 3)== 0 ){ 
#line 75 "../../src/dcl2.c"
{ 
#line 575 "../../src/dcl2.c"
struct ea _au0__V16 ;

#line 75 "../../src/dcl2.c"
error ( (char *)"BC%nU", (struct ea *)( ( ((& _au0__V16 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _classdef_clbase )), (((& _au0__V16 ))))
#line 75 "../../src/dcl2.c"
) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
goto nobase ;
} }
_au0_tbl = _au1_bcl -> _classdef_memtbl ;
if (_au0_tbl -> _node_base != 142 ){ 
#line 80 "../../src/dcl2.c"
{ 
#line 575 "../../src/dcl2.c"
struct ea _au0__V17 ;

#line 80 "../../src/dcl2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"badBC table %p", (struct ea *)( ( ((& _au0__V17 )-> _ea__O1.__C1_p = ((char *)_au0_tbl )), (((&
#line 80 "../../src/dcl2.c"
_au0__V17 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
goto nobase ;
} }
_au1_btbl = _au0_tbl ;
_au1_bvirt = _au1_bcl -> _classdef_virt_count ;
if (_au1_bcl -> _classdef_csu == 36 )error ( (char *)"C derived from union", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 )
#line 85 "../../src/dcl2.c"
;
if (_au1_in_union )error ( (char *)"derived union", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
if (_au0_this -> _classdef_pubbase == 0 )_au0_this -> _classdef_csu = 6 ;
_au1_boff = _au1_bcl -> _classdef_real_size ;
max_align = _type_align ( (struct type *)_au1_bcl ) ;
_au0_this -> _classdef_bit_ass = _au1_bcl -> _classdef_bit_ass ;
}
else 
#line 93 "../../src/dcl2.c"
{ 
#line 94 "../../src/dcl2.c"
nobase :
#line 95 "../../src/dcl2.c"
_au1_btbl = 0 ;
_au1_bvirt = 0 ;
_au1_boff = 0 ;
_au0_tbl = gtbl ;
while ((_au0_tbl != gtbl )&& _au0_tbl -> _table_t_name )_au0_tbl = _au0_tbl -> _table_next ;
max_align = AL_STRUCT ;
if (_au0_this -> _classdef_virt_count == 0 )_au0_this -> _classdef_bit_ass = 1 ;
}

#line 104 "../../src/dcl2.c"
( (_au0_this -> _classdef_memtbl -> _table_next = _au0_tbl )) ;
( (_au0_this -> _classdef_memtbl -> _table_t_name = _au0_cname )) ;
if (_au1_nmem )_table_grow ( _au0_this -> _classdef_memtbl , (_au1_nmem <= 2 )? 3 : _au1_nmem ) ;

#line 108 "../../src/dcl2.c"
( (cc ++ ), ((*cc )= (*(cc - 1 )))) ;
cc -> _dcl_context_not = _au0_cname ;
cc -> _dcl_context_cot = _au0_this ;

#line 112 "../../src/dcl2.c"
byte_offset = (_au1_usz = _au1_boff );
bit_offset = 0 ;

#line 115 "../../src/dcl2.c"
_au1_bt = (struct basetype *)_basetype__ctor ( (struct basetype *)0 , (unsigned char )119 , _au0_cname ) ;
_au1_bt -> _basetype_b_table = _au0_this -> _classdef_memtbl ;
_au0_this -> _classdef_this_type = (cc -> _dcl_context_tot = (struct type *)(_au1_cct = (struct ptr *)( (_au0__Xthis__ctor_ptr = 0 ), ( (_au0__Xthis__ctor_ptr = (struct ptr *)_new (
#line 117 "../../src/dcl2.c"
(long )(sizeof (struct ptr))) ), ( (Nt ++ ), ( (_au0__Xthis__ctor_ptr -> _node_base = ((unsigned char )125 )), (
#line 117 "../../src/dcl2.c"
(_au0__Xthis__ctor_ptr -> _pvtyp_typ = ((struct type *)_au1_bt )), ( (_au0__Xthis__ctor_ptr -> _ptr_rdo = ((char )0 )), ((_au0__Xthis__ctor_ptr ))) ) ) ) )
#line 117 "../../src/dcl2.c"
) ));
_au1_cct -> _node_permanent = 1 ;
_au1_bt -> _node_permanent = 1 ;

#line 121 "../../src/dcl2.c"
for(_au1_p = _au0_this -> _classdef_mem_list ;_au1_p ;_au1_p = _au1_px ) { 
#line 122 "../../src/dcl2.c"
Pname _au2_m ;
_au1_px = _au1_p -> _name_n_list ;

#line 125 "../../src/dcl2.c"
switch (_au1_p -> _node_base ){ 
#line 126 "../../src/dcl2.c"
case 25 : 
#line 127 "../../src/dcl2.c"
_au1_scope = 25 ;
_au1_protect = 0 ;
goto prpr ;
case 174 : 
#line 131 "../../src/dcl2.c"
_au1_scope = 0 ;
_au1_protect = 0 ;
_au0_this -> _classdef_csu = 6 ;
goto prpr ;
case 79 : 
#line 136 "../../src/dcl2.c"
_au1_scope = 0 ;
_au1_protect = 79 ;
_au0_this -> _classdef_csu = 6 ;
prpr :
#line 141 "../../src/dcl2.c"
if (_au1_in_union ){ 
#line 575 "../../src/dcl2.c"
struct ea _au0__V18 ;

#line 141 "../../src/dcl2.c"
error ( (char *)"%k label in unionD", (struct ea *)( ( ((& _au0__V18 )-> _ea__O1.__C1_i = ((int )_au1_p -> _node_base )), (((& _au0__V18 ))))
#line 141 "../../src/dcl2.c"
) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} continue ;
}

#line 145 "../../src/dcl2.c"
if (_au1_p -> _node_base == 175 ){ 
#line 146 "../../src/dcl2.c"
_au1_p -> _node_base = 85 ;
if (_au1_scope != 25 ){ 
#line 148 "../../src/dcl2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"visibilityD not in public section", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ,
#line 148 "../../src/dcl2.c"
(struct ea *)ea0 ) ;
continue ;
}
_au1_p -> _name_n_list = _au1_publist ;
_au1_publist = _au1_p ;
continue ;
}

#line 157 "../../src/dcl2.c"
if ((_au1_scope == 0 )|| (_au1_scope == 79 ))_au0_this -> _classdef_csu = 6 ;

#line 159 "../../src/dcl2.c"
if (_au1_p -> _expr__O2.__C2_tp -> _node_base == 108 ){ 
#line 160 "../../src/dcl2.c"
Pfct _au3_f ;
Pblock _au3_b ;

#line 160 "../../src/dcl2.c"
_au3_f = (((struct fct *)_au1_p -> _expr__O2.__C2_tp ));
_au3_b = _au3_f -> _fct_body ;
_au3_f -> _fct_body = 0 ;
switch (_au1_p -> _name_n_sto ){ 
#line 164 "../../src/dcl2.c"
case 2 : 
#line 165 "../../src/dcl2.c"
case 31 : 
#line 166 "../../src/dcl2.c"
case 27 : 
#line 167 "../../src/dcl2.c"
case 14 : 
#line 168 "../../src/dcl2.c"
{ 
#line 575 "../../src/dcl2.c"
struct ea _au0__V19 ;
#line 575 "../../src/dcl2.c"
struct ea _au0__V20 ;

#line 168 "../../src/dcl2.c"
error ( (char *)"M%n cannot be%k", (struct ea *)( ( ((& _au0__V19 )-> _ea__O1.__C1_p = ((char *)_au1_p )), (((& _au0__V19 )))) )
#line 168 "../../src/dcl2.c"
, (struct ea *)( ( ((& _au0__V20 )-> _ea__O1.__C1_i = ((int )_au1_p -> _name_n_sto )), (((& _au0__V20 )))) ) ,
#line 168 "../../src/dcl2.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
_au1_p -> _name_n_sto = 0 ;
} }
_au2_m = _name_dcl ( _au1_p , _au0_this -> _classdef_memtbl , (unsigned char )_au1_scope ) ;
if (_au2_m == 0 )continue ;
if (tsize ){ byte_offset += tsize ;

#line 173 "../../src/dcl2.c"
tsize = 0 ;
}

#line 174 "../../src/dcl2.c"
_au2_m -> _name_n_protect = _au1_protect ;
if (_au3_b ){ 
#line 176 "../../src/dcl2.c"
if (_au2_m -> _expr__O2.__C2_tp -> _type_defined & 3)
#line 177 "../../src/dcl2.c"
{ 
#line 575 "../../src/dcl2.c"
struct ea _au0__V21 ;

#line 177 "../../src/dcl2.c"
error ( (char *)"two definitions of%n", (struct ea *)( ( ((& _au0__V21 )-> _ea__O1.__C1_p = ((char *)_au2_m )), (((& _au0__V21 )))) )
#line 177 "../../src/dcl2.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} else if (_au1_p -> _name_where . _loc_line != _au2_m -> _name_where . _loc_line )
#line 179 "../../src/dcl2.c"
{ 
#line 575 "../../src/dcl2.c"
struct ea _au0__V22 ;

#line 179 "../../src/dcl2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"previously declared%n cannot be defined inCD", (struct ea *)( ( ((& _au0__V22 )-> _ea__O1.__C1_p = ((char *)_au1_p )), (((&
#line 179 "../../src/dcl2.c"
_au0__V22 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} else 
#line 181 "../../src/dcl2.c"
(((struct fct *)_au2_m -> _expr__O2.__C2_tp ))-> _fct_body = _au3_b ;
}
_au1_fct_seen = 1 ;
}
else { 
#line 186 "../../src/dcl2.c"
_au2_m = _name_dcl ( _au1_p , _au0_this -> _classdef_memtbl , (unsigned char )_au1_scope ) ;
_au2_m -> _name_n_protect = _au1_protect ;
if (_au2_m ){ 
#line 189 "../../src/dcl2.c"
if (_au2_m -> _name_n_stclass == 31 ){ 
#line 190 "../../src/dcl2.c"
_au1_static_seen = 1 ;
_au2_m -> _name_n_sto = ((_au0_tbl == gtbl )? 0: 31 );
if (_au2_m -> _expr__O4.__C4_n_initializer ){ 
#line 575 "../../src/dcl2.c"
struct ea _au0__V23 ;

#line 192 "../../src/dcl2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"staticM%nWIr", (struct ea *)( ( ((& _au0__V23 )-> _ea__O1.__C1_p = ((char *)_au2_m )), (((&
#line 192 "../../src/dcl2.c"
_au0__V23 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
if (_au1_in_union ){ 
#line 195 "../../src/dcl2.c"
if (_au1_usz < byte_offset )_au1_usz = byte_offset ;
byte_offset = 0 ;
}
}
}
}

#line 204 "../../src/dcl2.c"
if (_au1_in_union )byte_offset = _au1_usz ;

#line 206 "../../src/dcl2.c"
if (_au0_this -> _classdef_virt_count || _au1_bvirt ){ 
#line 207 "../../src/dcl2.c"
Pname _au2_vp [100];
Pname _au2_nn ;

#line 210 "../../src/dcl2.c"
_au2_nn = ( _table_look ( _au0_this -> _classdef_memtbl , "_ctor", (unsigned char )0 ) ) ;
if ((_au2_nn == 0 )|| (_au2_nn -> _expr__O5.__C5_n_table != _au0_this -> _classdef_memtbl ))_au1_make_ctor = 1 ;

#line 213 "../../src/dcl2.c"
{ 
#line 214 "../../src/dcl2.c"
char _au3_s [128];
sprintf ( _au3_s , (char *)"%s__vtbl", _au0_this -> _classdef_string ) ;
{ Pname _au3_n ;

#line 216 "../../src/dcl2.c"
_au3_n = (struct name *)_name__ctor ( (struct name *)0 , _au3_s ) ;
_au3_n -> _expr__O2.__C2_tp = Pfctvec_type ;
{ Pname _au3_nn ;

#line 218 "../../src/dcl2.c"
_au3_nn = _table_insert ( gtbl , _au3_n , (unsigned char )0 ) ;
( (_au3_nn -> _name_n_used ++ )) ;
}
}
}

#line 222 "../../src/dcl2.c"
if (_au0_this -> _classdef_virt_count = _au1_bvirt )
#line 223 "../../src/dcl2.c"
for(_au1_i = 0 ;_au1_i < _au1_bvirt ;_au1_i ++ ) (_au2_vp [_au1_i ])= (_au1_bcl -> _classdef_virt_init [_au1_i ]);

#line 225 "../../src/dcl2.c"
for(_au2_nn = _table_get_mem ( _au0_this -> _classdef_memtbl , _au1_i = 1 ) ;_au2_nn ;_au2_nn = _table_get_mem ( _au0_this -> _classdef_memtbl , ++ _au1_i ) ) {
#line 225 "../../src/dcl2.c"

#line 226 "../../src/dcl2.c"
switch (_au2_nn -> _expr__O2.__C2_tp -> _node_base ){ 
#line 227 "../../src/dcl2.c"
case 108 : 
#line 228 "../../src/dcl2.c"
{ Pfct _au5_f ;

#line 228 "../../src/dcl2.c"
_au5_f = (((struct fct *)_au2_nn -> _expr__O2.__C2_tp ));
if (_au1_bvirt ){ 
#line 230 "../../src/dcl2.c"
Pname _au6_vn ;

#line 233 "../../src/dcl2.c"
Pfct _au7_vnf ;

#line 230 "../../src/dcl2.c"
_au6_vn = _table_look ( _au1_btbl , _au2_nn -> _expr__O3.__C3_string , (unsigned char )0 ) ;
if (_au6_vn ){ 
#line 232 "../../src/dcl2.c"
if (_au6_vn -> _expr__O5.__C5_n_table == gtbl )goto vvv ;
;
switch (_au6_vn -> _expr__O2.__C2_tp -> _node_base ){ 
#line 235 "../../src/dcl2.c"
case 108 : 
#line 236 "../../src/dcl2.c"
_au7_vnf = (((struct fct *)_au6_vn -> _expr__O2.__C2_tp ));
if (_au7_vnf -> _fct_f_virtual ){ 
#line 238 "../../src/dcl2.c"
if (_type_check ( (struct type *)_au7_vnf , (struct type *)_au5_f , (unsigned char )0 ) ){ 
#line 575 "../../src/dcl2.c"
struct
#line 575 "../../src/dcl2.c"
ea _au0__V24 ;

#line 575 "../../src/dcl2.c"
struct ea _au0__V25 ;

#line 575 "../../src/dcl2.c"
struct ea _au0__V26 ;

#line 238 "../../src/dcl2.c"
error ( (char *)"virtual%nT mismatch:%t and%t", (struct ea *)( ( ((& _au0__V24 )-> _ea__O1.__C1_p = ((char *)_au2_nn )), (((& _au0__V24 )))) )
#line 238 "../../src/dcl2.c"
, (struct ea *)( ( ((& _au0__V25 )-> _ea__O1.__C1_p = ((char *)_au5_f )), (((& _au0__V25 )))) ) , (struct
#line 238 "../../src/dcl2.c"
ea *)( ( ((& _au0__V26 )-> _ea__O1.__C1_p = ((char *)_au7_vnf )), (((& _au0__V26 )))) ) , (struct ea *)ea0 ) ;
#line 238 "../../src/dcl2.c"
} 
#line 239 "../../src/dcl2.c"
_au5_f -> _fct_f_virtual = _au7_vnf -> _fct_f_virtual ;
(_au2_vp [_au5_f -> _fct_f_virtual - 1 ])= _au2_nn ;
}
else 
#line 243 "../../src/dcl2.c"
goto vvv ;
break ;
case 76 : 
#line 246 "../../src/dcl2.c"
{ Pgen _au9_g ;

#line 246 "../../src/dcl2.c"
_au9_g = (((struct gen *)_au6_vn -> _expr__O2.__C2_tp ));
if (_au5_f -> _fct_f_virtual || (((struct fct *)_au9_g -> _gen_fct_list -> _name_list_f -> _expr__O2.__C2_tp ))-> _fct_f_virtual )
#line 249 "../../src/dcl2.c"
{ 
#line 575 "../../src/dcl2.c"
struct ea _au0__V27 ;

#line 249 "../../src/dcl2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"virtual%n overloaded inBC but not in derivedC", (struct ea *)( ( ((& _au0__V27 )-> _ea__O1.__C1_p = ((char *)_au2_nn )), (((&
#line 249 "../../src/dcl2.c"
_au0__V27 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} break ;
}
default : 
#line 253 "../../src/dcl2.c"
goto vvv ;
}
}
else 
#line 257 "../../src/dcl2.c"
goto vvv ;
}
else { 
#line 260 "../../src/dcl2.c"
vvv :
#line 262 "../../src/dcl2.c"
if (_au5_f -> _fct_f_virtual ){ 
#line 263 "../../src/dcl2.c"
_au5_f -> _fct_f_virtual = (++ _au0_this -> _classdef_virt_count );
switch (_au5_f -> _fct_f_virtual ){ 
#line 265 "../../src/dcl2.c"
case 1 : 
#line 266 "../../src/dcl2.c"
{ Pname _au9_vpn ;

#line 266 "../../src/dcl2.c"
_au9_vpn = (struct name *)_name__ctor ( (struct name *)0 , "_vptr") ;
_au9_vpn -> _expr__O2.__C2_tp = Pfctvec_type ;
(_name_dcl ( _au9_vpn , _au0_this -> _classdef_memtbl , (unsigned char )25 ) );
_name__dtor ( _au9_vpn , 1) ;
}
default : 
#line 272 "../../src/dcl2.c"
(_au2_vp [_au5_f -> _fct_f_virtual - 1 ])= _au2_nn ;
}
}
}
break ;
}

#line 279 "../../src/dcl2.c"
case 76 : 
#line 280 "../../src/dcl2.c"
{ Plist _au5_gl ;
Pgen _au5_g ;

#line 281 "../../src/dcl2.c"
_au5_g = (((struct gen *)_au2_nn -> _expr__O2.__C2_tp ));

#line 283 "../../src/dcl2.c"
if (_au1_bvirt ){ 
#line 284 "../../src/dcl2.c"
Pname _au6_vn ;
Pgen _au6_g2 ;
Pfct _au6_f2 ;

#line 284 "../../src/dcl2.c"
_au6_vn = _table_look ( _au1_btbl , _au2_nn -> _expr__O3.__C3_string , (unsigned char )0 ) ;

#line 287 "../../src/dcl2.c"
if (_au6_vn ){ 
#line 289 "../../src/dcl2.c"
if (_au6_vn -> _expr__O5.__C5_n_table == gtbl )goto ovvv ;
switch (_au6_vn -> _expr__O2.__C2_tp -> _node_base ){ 
#line 291 "../../src/dcl2.c"
default : 
#line 292 "../../src/dcl2.c"
goto ovvv ;
case 108 : 
#line 294 "../../src/dcl2.c"
_au6_f2 = (((struct fct *)_au6_vn -> _expr__O2.__C2_tp ));
if (_au6_f2 -> _fct_f_virtual || (((struct fct *)_au5_g -> _gen_fct_list -> _name_list_f -> _expr__O2.__C2_tp ))-> _fct_f_virtual )
#line 297 "../../src/dcl2.c"
{ 
#line 575 "../../src/dcl2.c"
struct ea _au0__V28 ;

#line 297 "../../src/dcl2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"virtual%n overloaded in derivedC but not inBC", (struct ea *)( ( ((& _au0__V28 )-> _ea__O1.__C1_p = ((char *)_au2_nn )), (((&
#line 297 "../../src/dcl2.c"
_au0__V28 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} break ;
case 76 : 
#line 300 "../../src/dcl2.c"
_au6_g2 = (((struct gen *)_au6_vn -> _expr__O2.__C2_tp ));

#line 302 "../../src/dcl2.c"
for(_au5_gl = _au5_g -> _gen_fct_list ;_au5_gl ;_au5_gl = _au5_gl -> _name_list_l ) { 
#line 303 "../../src/dcl2.c"
Pname _au9_fn ;
Pfct _au9_f ;
Pname _au9_vn2 ;

#line 303 "../../src/dcl2.c"
_au9_fn = _au5_gl -> _name_list_f ;
_au9_f = (((struct fct *)_au9_fn -> _expr__O2.__C2_tp ));
_au9_vn2 = _gen_find ( _au6_g2 , _au9_f , (char )0 ) ;

#line 307 "../../src/dcl2.c"
if (_au9_vn2 == 0 ){ 
#line 308 "../../src/dcl2.c"
if (_au9_f -> _fct_f_virtual ){ 
#line 575 "../../src/dcl2.c"
struct ea _au0__V29 ;

#line 308 "../../src/dcl2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"virtual overloaded%n not found inBC", (struct ea *)( ( ((& _au0__V29 )-> _ea__O1.__C1_p = ((char *)_au9_fn )), (((&
#line 308 "../../src/dcl2.c"
_au0__V29 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
else { 
#line 311 "../../src/dcl2.c"
Pfct _au10_vn2f ;

#line 311 "../../src/dcl2.c"
_au10_vn2f = (((struct fct *)_au9_vn2 -> _expr__O2.__C2_tp ));
if (_au10_vn2f -> _fct_f_virtual ){ 
#line 313 "../../src/dcl2.c"
_au9_f -> _fct_f_virtual = _au10_vn2f -> _fct_f_virtual ;
(_au2_vp [_au9_f -> _fct_f_virtual - 1 ])= _au9_fn ;
}
}
}
break ;
}
}
else 
#line 322 "../../src/dcl2.c"
goto ovvv ;
}
else { 
#line 325 "../../src/dcl2.c"
ovvv :
#line 326 "../../src/dcl2.c"
for(_au5_gl = _au5_g -> _gen_fct_list ;_au5_gl ;_au5_gl = _au5_gl -> _name_list_l ) { 
#line 327 "../../src/dcl2.c"
Pname _au7_fn ;
Pfct _au7_f ;

#line 327 "../../src/dcl2.c"
_au7_fn = _au5_gl -> _name_list_f ;
_au7_f = (((struct fct *)_au7_fn -> _expr__O2.__C2_tp ));

#line 331 "../../src/dcl2.c"
if (_au7_f -> _fct_f_virtual ){ 
#line 332 "../../src/dcl2.c"
_au7_f -> _fct_f_virtual = (++ _au0_this -> _classdef_virt_count );
switch (_au7_f -> _fct_f_virtual ){ 
#line 334 "../../src/dcl2.c"
case 1 : 
#line 335 "../../src/dcl2.c"
{ Pname _au10_vpn ;

#line 335 "../../src/dcl2.c"
_au10_vpn = (struct name *)_name__ctor ( (struct name *)0 , "_vptr") ;
_au10_vpn -> _expr__O2.__C2_tp = Pfctvec_type ;
(_name_dcl ( _au10_vpn , _au0_this -> _classdef_memtbl , (unsigned char )0 ) );
_name__dtor ( _au10_vpn , 1) ;
}
default : 
#line 341 "../../src/dcl2.c"
(_au2_vp [_au7_f -> _fct_f_virtual - 1 ])= _au7_fn ;
}
}
}
}
break ;
}
}
}
_au0_this -> _classdef_virt_init = (((struct name **)_new ( (long )((sizeof (struct name *))* _au0_this -> _classdef_virt_count )) ));
for(_au1_i = 0 ;_au1_i < _au0_this -> _classdef_virt_count ;_au1_i ++ ) (_au0_this -> _classdef_virt_init [_au1_i ])= (_au2_vp [_au1_i ]);
}

#line 354 "../../src/dcl2.c"
;
for(_au1_p = _au1_publist ;_au1_p ;_au1_p = _au1_pnx ) { 
#line 356 "../../src/dcl2.c"
char *_au2_qs ;
char *_au2_ms ;
Pname _au2_cx ;
Ptable _au2_ctbl ;
Pname _au2_mx ;

#line 356 "../../src/dcl2.c"
_au2_qs = _au1_p -> _name__O6.__C6_n_qualifier -> _expr__O3.__C3_string ;
_au2_ms = _au1_p -> _expr__O3.__C3_string ;

#line 361 "../../src/dcl2.c"
_au1_pnx = _au1_p -> _name_n_list ;

#line 363 "../../src/dcl2.c"
if (strcmp ( (char *)_au2_ms , (char *)_au2_qs ) == 0 )_au2_ms = "_ctor";

#line 365 "../../src/dcl2.c"
for(_au2_cx = _au0_this -> _classdef_clbase ;_au2_cx ;_au2_cx = (((struct classdef *)_au2_cx -> _expr__O2.__C2_tp ))-> _classdef_clbase ) { 
#line 366 "../../src/dcl2.c"
if (strcmp ( (char *)_au2_cx -> _expr__O3.__C3_string , (char
#line 366 "../../src/dcl2.c"
*)_au2_qs ) == 0 )goto ok ;
}
{ 
#line 575 "../../src/dcl2.c"
struct ea _au0__V30 ;

#line 368 "../../src/dcl2.c"
error ( (char *)"publicQr %s not aBC", (struct ea *)( ( ((& _au0__V30 )-> _ea__O1.__C1_p = ((char *)_au2_qs )), (((& _au0__V30 )))) )
#line 368 "../../src/dcl2.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
continue ;
ok :
#line 371 "../../src/dcl2.c"
_au2_ctbl = (((struct classdef *)_au2_cx -> _expr__O2.__C2_tp ))-> _classdef_memtbl ;
_au2_mx = _table_lookc ( _au2_ctbl , _au2_ms , (unsigned char )0 ) ;

#line 374 "../../src/dcl2.c"
if (Ebase ){ 
#line 375 "../../src/dcl2.c"
if (! _classdef_has_friend ( Ebase , cc -> _dcl_context_nof ) ){ 
#line 575 "../../src/dcl2.c"
struct ea _au0__V31 ;

#line 375 "../../src/dcl2.c"
error ( (char *)"QdMN%n is in privateBC", (struct ea *)( ( ((& _au0__V31 )-> _ea__O1.__C1_p = ((char *)_au1_p )), (((& _au0__V31 )))) )
#line 375 "../../src/dcl2.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
else if (Epriv ){ 
#line 378 "../../src/dcl2.c"
if ((! _classdef_has_friend ( Epriv , cc -> _dcl_context_nof ) )&& (! (_au2_mx -> _name_n_protect &&
#line 378 "../../src/dcl2.c"
_classdef_baseofFPCname___ ( Epriv , cc -> _dcl_context_nof ) )))
#line 379 "../../src/dcl2.c"
{ 
#line 575 "../../src/dcl2.c"
struct ea _au0__V32 ;

#line 379 "../../src/dcl2.c"
error ( (char *)"QdMN%n is private", (struct ea *)( ( ((& _au0__V32 )-> _ea__O1.__C1_p = ((char *)_au1_p )), (((& _au0__V32 )))) )
#line 379 "../../src/dcl2.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }

#line 382 "../../src/dcl2.c"
if (_au2_mx == 0 ){ 
#line 383 "../../src/dcl2.c"
{ 
#line 575 "../../src/dcl2.c"
struct ea _au0__V33 ;

#line 575 "../../src/dcl2.c"
struct ea _au0__V34 ;

#line 383 "../../src/dcl2.c"
error ( (char *)"C%n does not have aM %s", (struct ea *)( ( ((& _au0__V33 )-> _ea__O1.__C1_p = ((char *)_au2_cx )), (((& _au0__V33 )))) )
#line 383 "../../src/dcl2.c"
, (struct ea *)( ( ((& _au0__V34 )-> _ea__O1.__C1_p = ((char *)_au1_p -> _expr__O3.__C3_string )), (((& _au0__V34 )))) ) ,
#line 383 "../../src/dcl2.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
_au1_p -> _expr__O2.__C2_tp = (struct type *)any_type ;
} }
else { 
#line 387 "../../src/dcl2.c"
if (_au2_mx -> _expr__O2.__C2_tp -> _node_base == 76 ){ 
#line 575 "../../src/dcl2.c"
struct ea _au0__V35 ;

#line 387 "../../src/dcl2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"public specification of overloaded%n", (struct ea *)( ( ((& _au0__V35 )-> _ea__O1.__C1_p = ((char *)_au2_mx )), (((&
#line 387 "../../src/dcl2.c"
_au0__V35 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} _au1_p -> _node_base = 25 ;
}

#line 391 "../../src/dcl2.c"
_au1_p -> _name__O6.__C6_n_qualifier = _au2_mx ;
(_table_insert ( _au0_this -> _classdef_memtbl , _au1_p , (unsigned char )0 ) );

#line 394 "../../src/dcl2.c"
if (Nold ){ 
#line 575 "../../src/dcl2.c"
struct ea _au0__V36 ;

#line 394 "../../src/dcl2.c"
error ( (char *)"twoDs of CM%n", (struct ea *)( ( ((& _au0__V36 )-> _ea__O1.__C1_p = ((char *)_au1_p )), (((& _au0__V36 )))) )
#line 394 "../../src/dcl2.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} } }

#line 397 "../../src/dcl2.c"
if (bit_offset )byte_offset += ((bit_offset / BI_IN_BYTE )+ 1 );
_au0_this -> _classdef_real_size = byte_offset ;

#line 400 "../../src/dcl2.c"
if (byte_offset < SZ_STRUCT )byte_offset = SZ_STRUCT ;
{ int _au1_waste ;

#line 401 "../../src/dcl2.c"
_au1_waste = (byte_offset % max_align );
if (_au1_waste )byte_offset += (max_align - _au1_waste );

#line 404 "../../src/dcl2.c"
_au0_this -> _classdef_obj_size = byte_offset ;
_au0_this -> _classdef_obj_align = max_align ;

#line 407 "../../src/dcl2.c"
if (( _table_look ( _au0_this -> _classdef_memtbl , "_dtor", (unsigned char )0 ) ) && (( _table_look ( _au0_this ->
#line 407 "../../src/dcl2.c"
_classdef_memtbl , "_ctor", (unsigned char )0 ) ) == 0 ))
#line 408 "../../src/dcl2.c"
{ 
#line 575 "../../src/dcl2.c"
struct ea _au0__V37 ;

#line 408 "../../src/dcl2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"%s has destructor but noK", (struct ea *)( ( ((& _au0__V37 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _classdef_string )),
#line 408 "../../src/dcl2.c"
(((& _au0__V37 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} 
#line 410 "../../src/dcl2.c"
{ 
#line 411 "../../src/dcl2.c"
Pname _au2_m ;
Pclass _au2_oc ;
Pname _au2_ct ;
Pname _au2_dt ;
int _au2_un ;

#line 412 "../../src/dcl2.c"
_au2_oc = _au0_this -> _classdef_in_class ;
_au2_ct = ( _table_look ( _au0_this -> _classdef_memtbl , "_ctor", (unsigned char )0 ) ) ;
_au2_dt = ( _table_look ( _au0_this -> _classdef_memtbl , "_dtor", (unsigned char )0 ) ) ;
_au2_un = (_au0_this -> _classdef_csu == 36 );

#line 417 "../../src/dcl2.c"
if (_au2_ct && (_au2_ct -> _expr__O5.__C5_n_table != _au0_this -> _classdef_memtbl ))_au2_ct = 0 ;
if (_au2_dt && (_au2_dt -> _expr__O5.__C5_n_table != _au0_this -> _classdef_memtbl ))_au2_dt = 0 ;

#line 420 "../../src/dcl2.c"
if (((_au2_ct == 0 )|| (_au2_dt == 0 ))|| _au2_un )
#line 421 "../../src/dcl2.c"
for(_au2_m = _table_get_mem ( _au0_this -> _classdef_memtbl , _au1_i = 1 ) ;_au2_m ;_au2_m = _table_get_mem ( _au0_this ->
#line 421 "../../src/dcl2.c"
_classdef_memtbl , ++ _au1_i ) ) { 
#line 423 "../../src/dcl2.c"
Ptype _au3_t ;

#line 423 "../../src/dcl2.c"
_au3_t = _au2_m -> _expr__O2.__C2_tp ;
switch (_au3_t -> _node_base ){ 
#line 425 "../../src/dcl2.c"
default : 
#line 426 "../../src/dcl2.c"
if ((_au2_ct == 0 )&& (_au2_m -> _name_n_stclass != 13 )){ 
#line 427 "../../src/dcl2.c"
if (_type_is_ref ( _au3_t )
#line 427 "../../src/dcl2.c"
){ 
#line 575 "../../src/dcl2.c"
struct ea _au0__V38 ;

#line 575 "../../src/dcl2.c"
struct ea _au0__V39 ;

#line 427 "../../src/dcl2.c"
error ( (char *)"R%n inC %sWoutK", (struct ea *)( ( ((& _au0__V38 )-> _ea__O1.__C1_p = ((char *)_au2_m )), (((& _au0__V38 )))) )
#line 427 "../../src/dcl2.c"
, (struct ea *)( ( ((& _au0__V39 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _classdef_string )), (((& _au0__V39 )))) ) ,
#line 427 "../../src/dcl2.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
} if (_type_tconst ( _au3_t ) && (vec_const == 0 )){ 
#line 575 "../../src/dcl2.c"
struct ea _au0__V40 ;

#line 575 "../../src/dcl2.c"
struct ea _au0__V41 ;

#line 428 "../../src/dcl2.c"
error ( (char *)"constant%n inC %sWoutK", (struct ea *)( ( ((& _au0__V40 )-> _ea__O1.__C1_p = ((char *)_au2_m )), (((& _au0__V40 )))) )
#line 428 "../../src/dcl2.c"
, (struct ea *)( ( ((& _au0__V41 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _classdef_string )), (((& _au0__V41 )))) ) ,
#line 428 "../../src/dcl2.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
break ;
case 108 : 
#line 432 "../../src/dcl2.c"
case 76 : 
#line 433 "../../src/dcl2.c"
case 6 : 
#line 434 "../../src/dcl2.c"
case 13 : 
#line 435 "../../src/dcl2.c"
continue ;
case 110 : 
#line 437 "../../src/dcl2.c"
break ;
}
{ Pname _au3_cn ;

#line 439 "../../src/dcl2.c"
_au3_cn = _type_is_cl_obj ( _au3_t ) ;
if (_au3_cn == 0 )_au3_cn = cl_obj_vec ;
if (_au3_cn == 0 )continue ;

#line 443 "../../src/dcl2.c"
{ Pclass _au3_cl ;

#line 443 "../../src/dcl2.c"
_au3_cl = (((struct classdef *)_au3_cn -> _expr__O2.__C2_tp ));
if (_au3_cl -> _classdef_bit_ass == 0 )_au0_this -> _classdef_bit_ass = 0 ;

#line 446 "../../src/dcl2.c"
{ Pname _au3_ctor ;
Pname _au3_dtor ;

#line 446 "../../src/dcl2.c"
_au3_ctor = ( _table_look ( _au3_cl -> _classdef_memtbl , "_ctor", (unsigned char )0 ) ) ;
_au3_dtor = ( _table_look ( _au3_cl -> _classdef_memtbl , "_dtor", (unsigned char )0 ) ) ;

#line 449 "../../src/dcl2.c"
if (_au3_ctor ){ 
#line 450 "../../src/dcl2.c"
if (_au2_m -> _name_n_stclass == 31 )
#line 451 "../../src/dcl2.c"
{ 
#line 575 "../../src/dcl2.c"
struct ea _au0__V42 ;

#line 575 "../../src/dcl2.c"
struct ea _au0__V43 ;

#line 451 "../../src/dcl2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"staticM%n ofC%nWK", (struct ea *)( ( ((& _au0__V42 )-> _ea__O1.__C1_p = ((char *)_au2_m )), (((&
#line 451 "../../src/dcl2.c"
_au0__V42 )))) ) , (struct ea *)( ( ((& _au0__V43 )-> _ea__O1.__C1_p = ((char *)_au3_cn )), (((& _au0__V43 )))) )
#line 451 "../../src/dcl2.c"
, (struct ea *)ea0 , (struct ea *)ea0 ) ;
} else if (_au2_un )
#line 453 "../../src/dcl2.c"
{ 
#line 575 "../../src/dcl2.c"
struct ea _au0__V44 ;

#line 575 "../../src/dcl2.c"
struct ea _au0__V45 ;

#line 453 "../../src/dcl2.c"
error ( (char *)"M%n ofC%nWK in union", (struct ea *)( ( ((& _au0__V44 )-> _ea__O1.__C1_p = ((char *)_au2_m )), (((& _au0__V44 )))) )
#line 453 "../../src/dcl2.c"
, (struct ea *)( ( ((& _au0__V45 )-> _ea__O1.__C1_p = ((char *)_au3_cn )), (((& _au0__V45 )))) ) , (struct
#line 453 "../../src/dcl2.c"
ea *)ea0 , (struct ea *)ea0 ) ;
} else if (_au2_ct == 0 )_au1_make_ctor = 1 ;
}

#line 457 "../../src/dcl2.c"
if (_au3_dtor ){ 
#line 458 "../../src/dcl2.c"
if (_au2_m -> _name_n_stclass == 31 )
#line 459 "../../src/dcl2.c"
{ 
#line 575 "../../src/dcl2.c"
struct ea _au0__V46 ;

#line 575 "../../src/dcl2.c"
struct ea _au0__V47 ;

#line 459 "../../src/dcl2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"staticM%n ofC%nW destructor", (struct ea *)( ( ((& _au0__V46 )-> _ea__O1.__C1_p = ((char *)_au2_m )), (((&
#line 459 "../../src/dcl2.c"
_au0__V46 )))) ) , (struct ea *)( ( ((& _au0__V47 )-> _ea__O1.__C1_p = ((char *)_au3_cn )), (((& _au0__V47 )))) )
#line 459 "../../src/dcl2.c"
, (struct ea *)ea0 , (struct ea *)ea0 ) ;
} else if (_au2_un )
#line 461 "../../src/dcl2.c"
{ 
#line 575 "../../src/dcl2.c"
struct ea _au0__V48 ;

#line 575 "../../src/dcl2.c"
struct ea _au0__V49 ;

#line 461 "../../src/dcl2.c"
error ( (char *)"M%n ofC%nW destructor in union", (struct ea *)( ( ((& _au0__V48 )-> _ea__O1.__C1_p = ((char *)_au2_m )), (((& _au0__V48 )))) )
#line 461 "../../src/dcl2.c"
, (struct ea *)( ( ((& _au0__V49 )-> _ea__O1.__C1_p = ((char *)_au3_cn )), (((& _au0__V49 )))) ) , (struct
#line 461 "../../src/dcl2.c"
ea *)ea0 , (struct ea *)ea0 ) ;
} else if (_au2_dt == 0 )_au1_make_dtor = 1 ;
}
}
}
}
}
}

#line 467 "../../src/dcl2.c"
if (_au1_make_ctor ){ 
#line 468 "../../src/dcl2.c"
Pname _au2_ct ;

#line 468 "../../src/dcl2.c"
_au2_ct = ( _table_look ( _au0_this -> _classdef_memtbl , "_ctor", (unsigned char )0 ) ) ;
if ((_au2_ct == 0 )|| (_au2_ct -> _expr__O5.__C5_n_table != _au0_this -> _classdef_memtbl )){ 
#line 472 "../../src/dcl2.c"
if (_au2_ct && (_classdef_has_ictor ( _au0_this ) == 0 )){ 
#line 575 "../../src/dcl2.c"
struct
#line 575 "../../src/dcl2.c"
ea _au0__V50 ;

#line 575 "../../src/dcl2.c"
struct ea _au0__V51 ;

#line 472 "../../src/dcl2.c"
error ( (char *)"%k %s needs aK", (struct ea *)( ( ((& _au0__V50 )-> _ea__O1.__C1_i = ((int )_au0_this -> _classdef_csu )), (((& _au0__V50 ))))
#line 472 "../../src/dcl2.c"
) , (struct ea *)( ( ((& _au0__V51 )-> _ea__O1.__C1_p = ((char *)_au0_this -> _classdef_string )), (((& _au0__V51 )))) )
#line 472 "../../src/dcl2.c"
, (struct ea *)ea0 , (struct ea *)ea0 ) ;
} { Pname _au3_n ;
Pfct _au3_f ;

#line 473 "../../src/dcl2.c"
_au3_n = (struct name *)_name__ctor ( (struct name *)0 , _au0_this -> _classdef_string ) ;
_au3_f = (struct fct *)_fct__ctor ( (struct fct *)0 , (struct type *)defa_type , (struct name *)0 , (unsigned char )1 ) ;
_au3_n -> _expr__O2.__C2_tp = (struct type *)_au3_f ;
_au3_n -> _name_n_oper = 123 ;
{ Pname _au3_m ;

#line 478 "../../src/dcl2.c"
struct block *_au0__Xthis__ctor_block ;

#line 477 "../../src/dcl2.c"
_au3_m = _name_dcl ( _au3_n , _au0_this -> _classdef_memtbl , (unsigned char )25 ) ;
(((struct fct *)_au3_m -> _expr__O2.__C2_tp ))-> _fct_body = (struct block *)( (_au0__Xthis__ctor_block = 0 ), ( ( (_au0__Xthis__ctor_block = 0 ), (_au0__Xthis__ctor_block = (struct
#line 478 "../../src/dcl2.c"
block *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_block ), (unsigned char )116 , curloc , ((struct stmt *)0 )) )) , ( (_au0__Xthis__ctor_block -> _stmt__O7.__C7_d =
#line 478 "../../src/dcl2.c"
((struct name *)0 )), ((_au0__Xthis__ctor_block ))) ) ) ;
}
}
}
}

#line 482 "../../src/dcl2.c"
if (_au1_make_dtor && (( _table_look ( _au0_this -> _classdef_memtbl , "_dtor", (unsigned char )0 ) ) == 0 )){ 
#line 484 "../../src/dcl2.c"
Pname _au2_n ;
#line 484 "../../src/dcl2.c"

#line 485 "../../src/dcl2.c"
Pfct _au2_f ;

#line 484 "../../src/dcl2.c"
_au2_n = (struct name *)_name__ctor ( (struct name *)0 , _au0_this -> _classdef_string ) ;
_au2_f = (struct fct *)_fct__ctor ( (struct fct *)0 , (struct type *)defa_type , (struct name *)0 , (unsigned char )1 ) ;
_au2_n -> _expr__O2.__C2_tp = (struct type *)_au2_f ;
_au2_n -> _name_n_oper = 162 ;
{ Pname _au2_m ;

#line 489 "../../src/dcl2.c"
struct block *_au0__Xthis__ctor_block ;

#line 488 "../../src/dcl2.c"
_au2_m = _name_dcl ( _au2_n , _au0_this -> _classdef_memtbl , (unsigned char )25 ) ;
(((struct fct *)_au2_m -> _expr__O2.__C2_tp ))-> _fct_body = (struct block *)( (_au0__Xthis__ctor_block = 0 ), ( ( (_au0__Xthis__ctor_block = 0 ), (_au0__Xthis__ctor_block = (struct
#line 489 "../../src/dcl2.c"
block *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_block ), (unsigned char )116 , curloc , ((struct stmt *)0 )) )) , ( (_au0__Xthis__ctor_block -> _stmt__O7.__C7_d =
#line 489 "../../src/dcl2.c"
((struct name *)0 )), ((_au0__Xthis__ctor_block ))) ) ) ;
}
}
_au0_this -> _type_defined |= 01 ;

#line 494 "../../src/dcl2.c"
{ Pname _au1_it ;

#line 494 "../../src/dcl2.c"
_au1_it = ( _au0_this -> _classdef_itor ) ;
for(_au1_p = _table_get_mem ( _au0_this -> _classdef_memtbl , _au1_i = 1 ) ;_au1_p ;_au1_p = _table_get_mem ( _au0_this -> _classdef_memtbl , ++ _au1_i ) ) {
#line 495 "../../src/dcl2.c"

#line 498 "../../src/dcl2.c"
switch (_au1_p -> _expr__O2.__C2_tp -> _node_base ){ 
#line 499 "../../src/dcl2.c"
case 108 : 
#line 500 "../../src/dcl2.c"
{ Pfct _au4_f ;

#line 500 "../../src/dcl2.c"
_au4_f = (((struct fct *)_au1_p -> _expr__O2.__C2_tp ));
if (_au4_f -> _fct_body ){ 
#line 502 "../../src/dcl2.c"
_au4_f -> _fct_f_inline = 1 ;
_au1_p -> _name_n_sto = 31 ;
_fct_dcl ( _au4_f , _au1_p ) ;
}

#line 509 "../../src/dcl2.c"
break ;
}
case 76 : 
#line 512 "../../src/dcl2.c"
{ Pgen _au4_g ;

#line 512 "../../src/dcl2.c"
_au4_g = (((struct gen *)_au1_p -> _expr__O2.__C2_tp ));
{ Plist _au4_gl ;

#line 513 "../../src/dcl2.c"
_au4_gl = _au4_g -> _gen_fct_list ;

#line 513 "../../src/dcl2.c"
for(;_au4_gl ;_au4_gl = _au4_gl -> _name_list_l ) { 
#line 514 "../../src/dcl2.c"
Pname _au5_n ;
Pfct _au5_f ;

#line 514 "../../src/dcl2.c"
_au5_n = _au4_gl -> _name_list_f ;
_au5_f = (((struct fct *)_au5_n -> _expr__O2.__C2_tp ));
if (_au5_f -> _fct_body ){ 
#line 517 "../../src/dcl2.c"
_au5_f -> _fct_f_inline = 1 ;
_au5_n -> _name_n_sto = 31 ;
_fct_dcl ( _au5_f , _au5_n ) ;
}
}
}
}
}
}

#line 530 "../../src/dcl2.c"
{ Plist _au1_fl ;

#line 530 "../../src/dcl2.c"
_au1_fl = _au0_this -> _classdef_friend_list ;

#line 530 "../../src/dcl2.c"
for(;_au1_fl ;_au1_fl = _au1_fl -> _name_list_l ) { 
#line 531 "../../src/dcl2.c"
Pname _au2_p ;

#line 531 "../../src/dcl2.c"
_au2_p = _au1_fl -> _name_list_f ;
switch (_au2_p -> _expr__O2.__C2_tp -> _node_base ){ 
#line 533 "../../src/dcl2.c"
case 108 : 
#line 534 "../../src/dcl2.c"
{ Pfct _au4_f ;

#line 534 "../../src/dcl2.c"
_au4_f = (((struct fct *)_au2_p -> _expr__O2.__C2_tp ));

#line 540 "../../src/dcl2.c"
if (_au4_f -> _fct_body && ((_au4_f -> _type_defined & 3)== 0 ))
#line 541 "../../src/dcl2.c"
{ 
#line 542 "../../src/dcl2.c"
_au4_f -> _fct_f_inline = 1 ;
_au2_p -> _name_n_sto = 31 ;
_fct_dcl ( _au4_f , _au2_p ) ;
}
break ;
}
case 76 : 
#line 549 "../../src/dcl2.c"
{ Pgen _au4_g ;
Plist _au4_gl ;

#line 549 "../../src/dcl2.c"
_au4_g = (((struct gen *)_au2_p -> _expr__O2.__C2_tp ));

#line 551 "../../src/dcl2.c"
for(_au4_gl = _au4_g -> _gen_fct_list ;_au4_gl ;_au4_gl = _au4_gl -> _name_list_l ) { 
#line 552 "../../src/dcl2.c"
Pname _au5_n ;
Pfct _au5_f ;

#line 552 "../../src/dcl2.c"
_au5_n = _au4_gl -> _name_list_f ;
_au5_f = (((struct fct *)_au5_n -> _expr__O2.__C2_tp ));

#line 559 "../../src/dcl2.c"
if (_au5_f -> _fct_body && ((_au5_f -> _type_defined & 3)== 0 ))
#line 560 "../../src/dcl2.c"
{ 
#line 561 "../../src/dcl2.c"
_au5_f -> _fct_f_inline = 1 ;
_au5_n -> _name_n_sto = 31 ;
_fct_dcl ( _au5_f , _au5_n ) ;
}
}
}
}
}

#line 570 "../../src/dcl2.c"
byte_offset = _au1_byte_old ;
bit_offset = _au1_bit_old ;
max_align = _au1_max_old ;

#line 574 "../../src/dcl2.c"
( (cc -- )) ;
}
}
}
}
;

#line 577 "../../src/dcl2.c"
char _enumdef_dcl (_au0_this , _au0__A52 , _au0_tbl )
#line 276 "../../src/cfront.h"
struct enumdef *_au0_this ;

#line 577 "../../src/dcl2.c"
Pname _au0__A52 ;

#line 577 "../../src/dcl2.c"
Ptable _au0_tbl ;
{ 
#line 580 "../../src/dcl2.c"
int _au1_nmem ;
Pname _au1_p ;
Pname _au1_ns ;
Pname _au1_nl ;
int _au1_enum_old ;

#line 590 "../../src/dcl2.c"
Pname _au1_px ;

#line 580 "../../src/dcl2.c"
_au1_nmem = _name_no_of_names ( _au0_this -> _enumdef_mem ) ;

#line 582 "../../src/dcl2.c"
_au1_ns = 0 ;
_au1_nl = 0 ;
_au1_enum_old = enum_count ;
_au0_this -> _enumdef_no_of_enumerators = _au1_nmem ;

#line 587 "../../src/dcl2.c"
enum_count = 0 ;

#line 589 "../../src/dcl2.c"
if (_au0_this == 0 ){ 
#line 626 "../../src/dcl2.c"
struct ea _au0__V53 ;

#line 589 "../../src/dcl2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"0->enumdef::dcl(%p)", (struct ea *)( ( ((& _au0__V53 )-> _ea__O1.__C1_p = ((char *)_au0_tbl )), (((&
#line 589 "../../src/dcl2.c"
_au0__V53 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} ;
for(( (_au1_p = _au0_this -> _enumdef_mem ), (_au0_this -> _enumdef_mem = 0 )) ;_au1_p ;_au1_p = _au1_px ) { 
#line 592 "../../src/dcl2.c"
Pname _au2_nn ;
_au1_px = _au1_p -> _name_n_list ;
if (_au1_p -> _expr__O4.__C4_n_initializer ){ 
#line 595 "../../src/dcl2.c"
Pexpr _au3_i ;

#line 595 "../../src/dcl2.c"
_au3_i = _expr_typ ( _au1_p -> _expr__O4.__C4_n_initializer , _au0_tbl ) ;
Neval = 0 ;
enum_count = _expr_eval ( _au3_i ) ;
if (Neval ){ 
#line 626 "../../src/dcl2.c"
struct ea _au0__V54 ;

#line 598 "../../src/dcl2.c"
error ( (char *)"%s", (struct ea *)( ( ((& _au0__V54 )-> _ea__O1.__C1_p = ((char *)Neval )), (((& _au0__V54 )))) )
#line 598 "../../src/dcl2.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} if (_au3_i && (_au3_i -> _node_permanent == 0 ))_expr_del ( _au3_i ) ;
_au1_p -> _expr__O4.__C4_n_initializer = 0 ;
}
_au1_p -> _name_n_evaluated = 1 ;
_au1_p -> _name_n_val = (enum_count ++ );
_au2_nn = _table_insert ( _au0_tbl , _au1_p , (unsigned char )0 ) ;
if (Nold ){ 
#line 606 "../../src/dcl2.c"
if (_au2_nn -> _name_n_stclass == 13 )
#line 607 "../../src/dcl2.c"
{ 
#line 626 "../../src/dcl2.c"
struct ea _au0__V55 ;

#line 607 "../../src/dcl2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (_au1_p -> _name_n_val != _au2_nn -> _name_n_val )? 0: 119, (char *)"enumerator%n declared twice", (struct ea *)( ( ((& _au0__V55 )->
#line 607 "../../src/dcl2.c"
_ea__O1.__C1_p = ((char *)_au2_nn )), (((& _au0__V55 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 607 "../../src/dcl2.c"
} else 
#line 609 "../../src/dcl2.c"
{ 
#line 626 "../../src/dcl2.c"
struct ea _au0__V56 ;

#line 609 "../../src/dcl2.c"
error ( (char *)"incompatibleDs of%n", (struct ea *)( ( ((& _au0__V56 )-> _ea__O1.__C1_p = ((char *)_au2_nn )), (((& _au0__V56 )))) )
#line 609 "../../src/dcl2.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
else { 
#line 612 "../../src/dcl2.c"
_au2_nn -> _name_n_stclass = 13 ;
if (_au1_ns )
#line 614 "../../src/dcl2.c"
_au1_nl -> _name_n_list = _au2_nn ;
else 
#line 616 "../../src/dcl2.c"
_au1_ns = _au2_nn ;
_au1_nl = _au2_nn ;
}
_name__dtor ( _au1_p , 1) ;
}

#line 622 "../../src/dcl2.c"
_au0_this -> _enumdef_mem = _au1_ns ;

#line 624 "../../src/dcl2.c"
enum_count = _au1_enum_old ;
_au0_this -> _type_defined |= 01 ;
}
;
Pstmt curr_loop ;
Pstmt curr_switch ;
Pblock curr_block ;

#line 632 "../../src/dcl2.c"
char _stmt_reached (_au0_this )
#line 651 "../../src/cfront.h"
struct stmt *_au0_this ;

#line 633 "../../src/dcl2.c"
{ 
#line 634 "../../src/dcl2.c"
register Pstmt _au1_ss ;

#line 634 "../../src/dcl2.c"
_au1_ss = _au0_this -> _stmt_s_list ;

#line 636 "../../src/dcl2.c"
if (_au1_ss == 0 )return ;

#line 638 "../../src/dcl2.c"
switch (_au1_ss -> _node_base ){ 
#line 639 "../../src/dcl2.c"
case 115 : 
#line 640 "../../src/dcl2.c"
case 4 : 
#line 641 "../../src/dcl2.c"
case 8 : 
#line 642 "../../src/dcl2.c"
break ;
default : 
#line 644 "../../src/dcl2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"S not reached", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 644 "../../src/dcl2.c"
ea *)ea0 ) ;
for(;_au1_ss ;_au1_ss = _au1_ss -> _stmt_s_list ) { 
#line 646 "../../src/dcl2.c"
switch (_au1_ss -> _node_base ){ 
#line 647 "../../src/dcl2.c"
case 115 : 
#line 648 "../../src/dcl2.c"
case 4 : 
#line 649 "../../src/dcl2.c"
case 8 : 
#line 650 "../../src/dcl2.c"
_au0_this ->
#line 650 "../../src/dcl2.c"
_stmt_s_list = _au1_ss ;
return ;
case 118 : 
#line 654 "../../src/dcl2.c"
case 20 : 
#line 655 "../../src/dcl2.c"
case 10 : 
#line 656 "../../src/dcl2.c"
case 39 : 
#line 657 "../../src/dcl2.c"
case 33 : 
#line 658 "../../src/dcl2.c"
case 16 : 
#line 659 "../../src/dcl2.c"
case 116 :
#line 659 "../../src/dcl2.c"

#line 660 "../../src/dcl2.c"
_au0_this -> _stmt_s_list = _au1_ss ;
return ;
}
}
_au0_this -> _stmt_s_list = 0 ;
}
}
;
bit arg_err_suppress ;

#line 670 "../../src/dcl2.c"
extern Pexpr check_cond (_au0_e , _au0_b , _au0_tbl )Pexpr _au0_e ;

#line 670 "../../src/dcl2.c"
TOK _au0_b ;

#line 670 "../../src/dcl2.c"
Ptable _au0_tbl ;
{ 
#line 673 "../../src/dcl2.c"
Pname _au1_cn ;
if (_au1_cn = _type_is_cl_obj ( _au0_e -> _expr__O2.__C2_tp ) ){ 
#line 675 "../../src/dcl2.c"
Pclass _au2_cl ;
int _au2_i ;
Pname _au2_found ;
Pname _au2_on ;

#line 675 "../../src/dcl2.c"
_au2_cl = (((struct classdef *)_au1_cn -> _expr__O2.__C2_tp ));
_au2_i = 0 ;
_au2_found = 0 ;
for(_au2_on = _au2_cl -> _classdef_conv ;_au2_on ;_au2_on = _au2_on -> _name_n_list ) { 
#line 679 "../../src/dcl2.c"
Pfct _au3_f ;
Ptype _au3_t ;

#line 679 "../../src/dcl2.c"
_au3_f = (((struct fct *)_au2_on -> _expr__O2.__C2_tp ));
_au3_t = _au3_f -> _fct_returns ;
xx :
#line 682 "../../src/dcl2.c"
switch (_au3_t -> _node_base ){ 
#line 683 "../../src/dcl2.c"
case 97 : 
#line 684 "../../src/dcl2.c"
_au3_t = (((struct basetype *)_au3_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
goto xx ;
case 15 : 
#line 687 "../../src/dcl2.c"
case 11 : 
#line 688 "../../src/dcl2.c"
case 125 : 
#line 689 "../../src/dcl2.c"
if (_au0_b == 111 )break ;
case 5 : 
#line 691 "../../src/dcl2.c"
case 29 : 
#line 692 "../../src/dcl2.c"
case 21 : 
#line 693 "../../src/dcl2.c"
case 22 : 
#line 694 "../../src/dcl2.c"
case 121 : 
#line 695 "../../src/dcl2.c"
_au2_i ++ ;
_au2_found = _au2_on ;
}
}
switch (_au2_i ){ 
#line 700 "../../src/dcl2.c"
case 0 : 
#line 701 "../../src/dcl2.c"
{ 
#line 722 "../../src/dcl2.c"
struct ea _au0__V57 ;

#line 722 "../../src/dcl2.c"
struct ea _au0__V58 ;

#line 701 "../../src/dcl2.c"
error ( (char *)"%nO in%kE", (struct ea *)( ( ((& _au0__V57 )-> _ea__O1.__C1_p = ((char *)_au1_cn )), (((& _au0__V57 )))) )
#line 701 "../../src/dcl2.c"
, (struct ea *)( ( ((& _au0__V58 )-> _ea__O1.__C1_i = ((int )_au0_b )), (((& _au0__V58 )))) ) , (struct
#line 701 "../../src/dcl2.c"
ea *)ea0 , (struct ea *)ea0 ) ;
return _au0_e ;
case 1 : 
#line 704 "../../src/dcl2.c"
{ 
#line 706 "../../src/dcl2.c"
Pclass _au4_cl ;
Pref _au4_r ;

#line 708 "../../src/dcl2.c"
struct ref *_au0__Xthis__ctor_ref ;

#line 706 "../../src/dcl2.c"
_au4_cl = (((struct classdef *)_au1_cn -> _expr__O2.__C2_tp ));
_au4_r = (struct ref *)( (_au0__Xthis__ctor_ref = 0 ), ( ( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ref ),
#line 707 "../../src/dcl2.c"
((unsigned char )45 ), _au0_e , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = _au2_found ), ((_au0__Xthis__ctor_ref ))) )
#line 707 "../../src/dcl2.c"
) ;
_au4_r -> _expr__O2.__C2_tp = _au2_found -> _expr__O2.__C2_tp ;
{ Pexpr _au4_c ;

#line 709 "../../src/dcl2.c"
_au4_c = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )146 , (struct expr *)_au4_r , (struct expr *)0 ) ;
_au4_c -> _expr__O5.__C5_fct_name = _au2_found ;

#line 712 "../../src/dcl2.c"
return _expr_typ ( _au4_c , _au0_tbl ) ;
}
}

#line 714 "../../src/dcl2.c"
default : 
#line 715 "../../src/dcl2.c"
{ 
#line 722 "../../src/dcl2.c"
struct ea _au0__V59 ;

#line 722 "../../src/dcl2.c"
struct ea _au0__V60 ;

#line 722 "../../src/dcl2.c"
struct ea _au0__V61 ;

#line 715 "../../src/dcl2.c"
error ( (char *)"%d possible conversions for%nO in%kE", (struct ea *)( ( ((& _au0__V59 )-> _ea__O1.__C1_i = _au2_i ), (((& _au0__V59 )))) ) ,
#line 715 "../../src/dcl2.c"
(struct ea *)( ( ((& _au0__V60 )-> _ea__O1.__C1_p = ((char *)_au1_cn )), (((& _au0__V60 )))) ) , (struct ea *)(
#line 715 "../../src/dcl2.c"
( ((& _au0__V61 )-> _ea__O1.__C1_i = ((int )_au0_b )), (((& _au0__V61 )))) ) , (struct ea *)ea0 ) ;
return _au0_e ;
} } }
}

#line 720 "../../src/dcl2.c"
if (( _type_kind ( _au0_e -> _expr__O2.__C2_tp , _au0_b , (unsigned char )'P' ) ) == 108 ){
#line 720 "../../src/dcl2.c"

#line 722 "../../src/dcl2.c"
struct ea _au0__V62 ;

#line 720 "../../src/dcl2.c"
error ( (char *)"%k(F)", (struct ea *)( ( ((& _au0__V62 )-> _ea__O1.__C1_i = ((int )_au0_b )), (((& _au0__V62 )))) )
#line 720 "../../src/dcl2.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} return _au0_e ;
}
;
char _stmt_dcl (_au0_this )
#line 651 "../../src/cfront.h"
struct stmt *_au0_this ;

#line 728 "../../src/dcl2.c"
{ 
#line 729 "../../src/dcl2.c"
Pstmt _au1_ss ;
Pname _au1_n ;
Pname _au1_nn ;
Pstmt _au1_ostmt ;

#line 732 "../../src/dcl2.c"
_au1_ostmt = Cstmt ;

#line 734 "../../src/dcl2.c"
for(_au1_ss = _au0_this ;_au1_ss ;_au1_ss = _au1_ss -> _stmt_s_list ) { 
#line 735 "../../src/dcl2.c"
Pstmt _au2_old_loop ;

#line 735 "../../src/dcl2.c"
Pstmt _au2_old_switch ;

#line 737 "../../src/dcl2.c"
Ptable _au2_tbl ;

#line 736 "../../src/dcl2.c"
Cstmt = _au1_ss ;
_au2_tbl = curr_block -> _stmt_memtbl ;

#line 739 "../../src/dcl2.c"
switch (_au1_ss -> _node_base ){ 
#line 740 "../../src/dcl2.c"
case 3 : 
#line 741 "../../src/dcl2.c"
if ((curr_loop == 0 )&& (curr_switch == 0 ))
#line 742 "../../src/dcl2.c"
{ 
#line 1348 "../../src/dcl2.c"
struct ea _au0__V63 ;

#line 742 "../../src/dcl2.c"
error ( (char *)"%k not in loop or switch", (struct ea *)( ( ((& _au0__V63 )-> _ea__O1.__C1_i = 3 ), (((& _au0__V63 )))) ) ,
#line 742 "../../src/dcl2.c"
(struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} _stmt_reached ( _au1_ss ) ;
break ;

#line 746 "../../src/dcl2.c"
case 7 : 
#line 747 "../../src/dcl2.c"
if (curr_loop == 0 ){ 
#line 1348 "../../src/dcl2.c"
struct ea _au0__V64 ;

#line 747 "../../src/dcl2.c"
error ( (char *)"%k not in loop", (struct ea *)( ( ((& _au0__V64 )-> _ea__O1.__C1_i = 7 ), (((& _au0__V64 )))) ) ,
#line 747 "../../src/dcl2.c"
(struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} _stmt_reached ( _au1_ss ) ;
break ;

#line 751 "../../src/dcl2.c"
case 8 : 
#line 752 "../../src/dcl2.c"
if (curr_switch == 0 ){ 
#line 753 "../../src/dcl2.c"
error ( (char *)"default not in switch", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ,
#line 753 "../../src/dcl2.c"
(struct ea *)ea0 ) ;
break ;
}
if (curr_switch -> _stmt__O7.__C7_has_default )error ( (char *)"two defaults in switch", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 756 "../../src/dcl2.c"

#line 757 "../../src/dcl2.c"
curr_switch -> _stmt__O7.__C7_has_default = _au1_ss ;
_au1_ss -> _stmt_s -> _stmt_s_list = _au1_ss -> _stmt_s_list ;
_au1_ss -> _stmt_s_list = 0 ;
_stmt_dcl ( _au1_ss -> _stmt_s ) ;
break ;

#line 763 "../../src/dcl2.c"
case 72 : 
#line 764 "../../src/dcl2.c"
{ TOK _au4_b ;

#line 764 "../../src/dcl2.c"
_au4_b = _au1_ss -> _stmt__O8.__C8_e -> _node_base ;
switch (_au4_b ){ 
#line 766 "../../src/dcl2.c"
case 144 : 
#line 767 "../../src/dcl2.c"
_au1_ss -> _stmt__O8.__C8_e = 0 ;
break ;

#line 773 "../../src/dcl2.c"
case 62 : 
#line 774 "../../src/dcl2.c"
case 63 : 
#line 775 "../../src/dcl2.c"
case 54 : 
#line 776 "../../src/dcl2.c"
case 55 : 
#line 777 "../../src/dcl2.c"
case 44 : 
#line 778 "../../src/dcl2.c"
case 45 : 
#line 779 "../../src/dcl2.c"
case 50 :
#line 779 "../../src/dcl2.c"

#line 780 "../../src/dcl2.c"
case 51 : 
#line 781 "../../src/dcl2.c"
case 112 : 
#line 782 "../../src/dcl2.c"
case 52 : 
#line 783 "../../src/dcl2.c"
case 65 : 
#line 784 "../../src/dcl2.c"
case 64 : 
#line 785 "../../src/dcl2.c"
case 111 : 
#line 786 "../../src/dcl2.c"
case 66 :
#line 786 "../../src/dcl2.c"

#line 787 "../../src/dcl2.c"
case 67 : 
#line 788 "../../src/dcl2.c"
case 85 : 
#line 789 "../../src/dcl2.c"
if (_au1_ss -> _stmt__O8.__C8_e -> _expr__O2.__C2_tp )break ;
_au1_ss -> _stmt__O8.__C8_e = _expr_typ ( _au1_ss -> _stmt__O8.__C8_e , _au2_tbl ) ;
if (_au1_ss -> _stmt__O8.__C8_e -> _node_base == 109 )break ;
if (_au1_ss -> _stmt__O8.__C8_e -> _expr__O2.__C2_tp -> _node_base != 38 ){ 
#line 1348 "../../src/dcl2.c"
struct ea _au0__V65 ;

#line 792 "../../src/dcl2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"result of%kE not used", (struct ea *)( ( ((& _au0__V65 )-> _ea__O1.__C1_i = ((int )_au4_b )), (((&
#line 792 "../../src/dcl2.c"
_au0__V65 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} break ;
default : 
#line 795 "../../src/dcl2.c"
_au1_ss -> _stmt__O8.__C8_e = _expr_typ ( _au1_ss -> _stmt__O8.__C8_e , _au2_tbl ) ;
}
break ;
}
case 28 : 
#line 800 "../../src/dcl2.c"
{ Pname _au4_fn ;
Pfct _au4_f ;
Ptype _au4_rt ;
Pexpr _au4_v ;

#line 800 "../../src/dcl2.c"
_au4_fn = cc -> _dcl_context_nof ;
_au4_f = (((struct fct *)_au4_fn -> _expr__O2.__C2_tp ));
_au4_rt = _au4_f -> _fct_returns ;
_au4_v = _au1_ss -> _stmt__O8.__C8_e ;
if (_au4_v != dummy ){ 
#line 805 "../../src/dcl2.c"
if (_au4_rt -> _node_base == 38 ){ 
#line 806 "../../src/dcl2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"unexpected return value", (struct ea *)ea0 ,
#line 806 "../../src/dcl2.c"
(struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;

#line 808 "../../src/dcl2.c"
_au1_ss -> _stmt__O8.__C8_e = dummy ;
}
else { 
#line 811 "../../src/dcl2.c"
_au4_v = _expr_typ ( _au4_v , _au2_tbl ) ;
lx :
#line 813 "../../src/dcl2.c"
switch (_au4_rt -> _node_base ){ 
#line 814 "../../src/dcl2.c"
case 97 : 
#line 815 "../../src/dcl2.c"
_au4_rt = (((struct basetype *)_au4_rt ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
goto lx ;
case 158 : 
#line 818 "../../src/dcl2.c"
if ((_expr_lval ( _au4_v , (unsigned char )0 ) == 0 )&& (_type_tconst ( _au4_v -> _expr__O2.__C2_tp ) ==
#line 818 "../../src/dcl2.c"
0 ))
#line 820 "../../src/dcl2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"R to non-lvalue returned", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 820 "../../src/dcl2.c"
else 
#line 821 "../../src/dcl2.c"
if ((_au4_v -> _node_base == 85 )&& ((((struct name *)_au4_v ))-> _name_n_scope == 108 ))
#line 823 "../../src/dcl2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"R to localV returned", (struct
#line 823 "../../src/dcl2.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
_au4_v = ref_init ( ((struct ptr *)_au4_rt ), _au4_v , _au2_tbl ) ;

#line 826 "../../src/dcl2.c"
case 141 : 
#line 827 "../../src/dcl2.c"
break ;
case 119 : 
#line 831 "../../src/dcl2.c"
if (_au4_v -> _node_base == 111 ){ 
#line 832 "../../src/dcl2.c"
Pexpr _au8_v1 ;

#line 832 "../../src/dcl2.c"
_au8_v1 = _au4_v -> _expr__O3.__C3_e1 ;
if (_au8_v1 -> _node_base == 113 ){ 
#line 834 "../../src/dcl2.c"
Pexpr _au9_v2 ;

#line 834 "../../src/dcl2.c"
_au9_v2 = _au8_v1 -> _expr__O3.__C3_e1 ;
if (_au9_v2 -> _node_base == 147 ){ 
#line 836 "../../src/dcl2.c"
Pexpr _au10_v3 ;

#line 836 "../../src/dcl2.c"
_au10_v3 = _au9_v2 -> _expr__O4.__C4_e2 ;
_au9_v2 -> _expr__O4.__C4_e2 = _au4_v ;
_au9_v2 -> _expr__O2.__C2_tp = _au4_v -> _expr__O2.__C2_tp ;
_au4_v = _au9_v2 ;
_au8_v1 -> _expr__O3.__C3_e1 = _au10_v3 ;
}
}
}
if (_au4_f -> _fct_f_result ){ 
#line 845 "../../src/dcl2.c"
if ((_type_check ( _au4_rt , _au4_v -> _expr__O2.__C2_tp , (unsigned char )70 ) == 0 )&& (_au4_v ->
#line 845 "../../src/dcl2.c"
_node_base == 147 ))
#line 846 "../../src/dcl2.c"
_au4_v = replace_temp ( _au4_v , (struct expr *)_au4_f -> _fct_f_result ) ;
else { 
#line 848 "../../src/dcl2.c"
_au4_v = class_init ( _expr_contents ( (struct expr *)_au4_f -> _fct_f_result ) , _au4_rt , _au4_v , _au2_tbl ) ;
{ Pname _au9_rcn ;

#line 849 "../../src/dcl2.c"
_au9_rcn = _type_is_cl_obj ( _au4_rt ) ;
if (( (((struct classdef *)_au9_rcn -> _expr__O2.__C2_tp ))-> _classdef_itor ) == 0 ){ 
#line 852 "../../src/dcl2.c"
_au4_v -> _expr__O2.__C2_tp = _au4_rt ;
_au4_v = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )70 , _expr_contents ( (struct expr *)_au4_f -> _fct_f_result ) , _au4_v )
#line 853 "../../src/dcl2.c"
;
_au4_v -> _expr__O2.__C2_tp = _au4_rt ;
}
}
}
}
else _au4_v = class_init ( (struct expr *)0 , _au4_rt , _au4_v , _au2_tbl ) ;
break ;
case 125 : 
#line 862 "../../src/dcl2.c"
_au4_v = ptr_init ( ((struct ptr *)_au4_rt ), _au4_v , _au2_tbl ) ;
goto def ;
case 21 : 
#line 865 "../../src/dcl2.c"
case 5 : 
#line 866 "../../src/dcl2.c"
case 22 : 
#line 867 "../../src/dcl2.c"
case 29 : 
#line 868 "../../src/dcl2.c"
if (((((struct basetype *)_au4_rt ))-> _basetype_b_unsigned && (_au4_v -> _node_base ==
#line 868 "../../src/dcl2.c"
107 ))&& (_au4_v -> _expr__O4.__C4_e2 -> _node_base == 82 ))
#line 871 "../../src/dcl2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"negative retured fromF returning unsigned", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 871 "../../src/dcl2.c"
ea *)ea0 , (struct ea *)ea0 ) ;
default : 
#line 873 "../../src/dcl2.c"
def :
#line 874 "../../src/dcl2.c"
if (_type_check ( _au4_rt , _au4_v -> _expr__O2.__C2_tp , (unsigned char )70 ) ){ 
#line 875 "../../src/dcl2.c"
Pexpr _au8_x ;

#line 875 "../../src/dcl2.c"
_au8_x = try_to_coerce ( _au4_rt , _au4_v , "return value", _au2_tbl ) ;
if (_au8_x )
#line 877 "../../src/dcl2.c"
_au4_v = _au8_x ;
else 
#line 879 "../../src/dcl2.c"
{ 
#line 1348 "../../src/dcl2.c"
struct ea _au0__V66 ;

#line 1348 "../../src/dcl2.c"
struct ea _au0__V67 ;

#line 1348 "../../src/dcl2.c"
struct ea _au0__V68 ;

#line 879 "../../src/dcl2.c"
error ( (char *)"bad return valueT for%n:%t (%tX)", (struct ea *)( ( ((& _au0__V66 )-> _ea__O1.__C1_p = ((char *)_au4_fn )), (((& _au0__V66 )))) )
#line 879 "../../src/dcl2.c"
, (struct ea *)( ( ((& _au0__V67 )-> _ea__O1.__C1_p = ((char *)_au4_v -> _expr__O2.__C2_tp )), (((& _au0__V67 )))) ) ,
#line 879 "../../src/dcl2.c"
(struct ea *)( ( ((& _au0__V68 )-> _ea__O1.__C1_p = ((char *)_au4_rt )), (((& _au0__V68 )))) ) , (struct ea *)ea0 )
#line 879 "../../src/dcl2.c"
;
} }
}
_au1_ss -> _stmt__O7.__C7_ret_tp = _au4_rt ;
_au1_ss -> _stmt__O8.__C8_e = _au4_v ;
}
}
else { 
#line 887 "../../src/dcl2.c"
if (_au4_rt -> _node_base != 38 )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"return valueX", (struct ea *)ea0 , (struct ea *)ea0 ,
#line 887 "../../src/dcl2.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
}
_stmt_reached ( _au1_ss ) ;
break ;
}

#line 893 "../../src/dcl2.c"
case 10 : 
#line 894 "../../src/dcl2.c"
inline_restr |= 8 ;
_au2_old_loop = curr_loop ;
curr_loop = _au1_ss ;
if (_au1_ss -> _stmt_s -> _node_base == 118 )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"D as onlyS in do-loop", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 897 "../../src/dcl2.c"
ea *)ea0 , (struct ea *)ea0 ) ;
_stmt_dcl ( _au1_ss -> _stmt_s ) ;
_au1_ss -> _stmt__O8.__C8_e = _expr_typ ( _au1_ss -> _stmt__O8.__C8_e , _au2_tbl ) ;
_au1_ss -> _stmt__O8.__C8_e = check_cond ( _au1_ss -> _stmt__O8.__C8_e , (unsigned char )10 , _au2_tbl ) ;
curr_loop = _au2_old_loop ;
break ;

#line 904 "../../src/dcl2.c"
case 39 : 
#line 905 "../../src/dcl2.c"
inline_restr |= 8 ;
_au2_old_loop = curr_loop ;
curr_loop = _au1_ss ;
_au1_ss -> _stmt__O8.__C8_e = _expr_typ ( _au1_ss -> _stmt__O8.__C8_e , _au2_tbl ) ;
_au1_ss -> _stmt__O8.__C8_e = check_cond ( _au1_ss -> _stmt__O8.__C8_e , (unsigned char )39 , _au2_tbl ) ;
if (_au1_ss -> _stmt_s -> _node_base == 118 )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"D as onlyS in while-loop", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 910 "../../src/dcl2.c"
ea *)ea0 , (struct ea *)ea0 ) ;
_stmt_dcl ( _au1_ss -> _stmt_s ) ;
curr_loop = _au2_old_loop ;
break ;

#line 915 "../../src/dcl2.c"
case 33 : 
#line 916 "../../src/dcl2.c"
{ int _au4_ne ;

#line 916 "../../src/dcl2.c"
_au4_ne = 0 ;
inline_restr |= 4 ;
_au2_old_switch = curr_switch ;
curr_switch = _au1_ss ;
_au1_ss -> _stmt__O8.__C8_e = _expr_typ ( _au1_ss -> _stmt__O8.__C8_e , _au2_tbl ) ;
_au1_ss -> _stmt__O8.__C8_e = check_cond ( _au1_ss -> _stmt__O8.__C8_e , (unsigned char )33 , _au2_tbl ) ;
{ Ptype _au5_tt ;

#line 922 "../../src/dcl2.c"
_au5_tt = _au1_ss -> _stmt__O8.__C8_e -> _expr__O2.__C2_tp ;
sii :
#line 924 "../../src/dcl2.c"
switch (_au5_tt -> _node_base ){ 
#line 925 "../../src/dcl2.c"
case 97 : 
#line 926 "../../src/dcl2.c"
_au5_tt = (((struct basetype *)_au5_tt ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 926 "../../src/dcl2.c"
goto sii ;
case 121 : 
#line 928 "../../src/dcl2.c"
_au4_ne = (((struct enumdef *)(((struct basetype *)_au5_tt ))-> _basetype_b_name -> _expr__O2.__C2_tp ))-> _enumdef_no_of_enumerators ;
case 138 : 
#line 930 "../../src/dcl2.c"
case 141 : 
#line 931 "../../src/dcl2.c"
case 5 : 
#line 932 "../../src/dcl2.c"
case 29 : 
#line 933 "../../src/dcl2.c"
case 21 : 
#line 934 "../../src/dcl2.c"
case 22 : 
#line 935 "../../src/dcl2.c"
case 114 :
#line 935 "../../src/dcl2.c"

#line 936 "../../src/dcl2.c"
break ;
default : 
#line 938 "../../src/dcl2.c"
{ 
#line 1348 "../../src/dcl2.c"
struct ea _au0__V69 ;

#line 938 "../../src/dcl2.c"
error ( (char *)"%t switchE must be converted to int", (struct ea *)( ( ((& _au0__V69 )-> _ea__O1.__C1_p = ((char *)_au1_ss -> _stmt__O8.__C8_e -> _expr__O2.__C2_tp )), (((&
#line 938 "../../src/dcl2.c"
_au0__V69 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}
_stmt_dcl ( _au1_ss -> _stmt_s ) ;
if (_au4_ne ){ 
#line 945 "../../src/dcl2.c"
int _au5_i ;
Pstmt _au5_cs ;

#line 945 "../../src/dcl2.c"
_au5_i = 0 ;

#line 947 "../../src/dcl2.c"
for(_au5_cs = _au1_ss -> _stmt__O9.__C9_case_list ;_au5_cs ;_au5_cs = _au5_cs -> _stmt__O9.__C9_case_list ) _au5_i ++ ;
if (_au5_i && (_au5_i != _au4_ne )){ 
#line 949 "../../src/dcl2.c"
if (_au4_ne < _au5_i ){ 
#line 950 "../../src/dcl2.c"
ee :{ 
#line 1348 "../../src/dcl2.c"
struct ea _au0__V70 ;

#line 1348 "../../src/dcl2.c"
struct ea _au0__V71 ;

#line 1348 "../../src/dcl2.c"
struct ea _au0__V72 ;

#line 950 "../../src/dcl2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"switch (%t)W %d cases (%d enumerators)", (struct ea *)( ( ((& _au0__V70 )-> _ea__O1.__C1_p = ((char *)_au1_ss -> _stmt__O8.__C8_e ->
#line 950 "../../src/dcl2.c"
_expr__O2.__C2_tp )), (((& _au0__V70 )))) ) , (struct ea *)( ( ((& _au0__V71 )-> _ea__O1.__C1_i = _au5_i ), (((& _au0__V71 ))))
#line 950 "../../src/dcl2.c"
) , (struct ea *)( ( ((& _au0__V72 )-> _ea__O1.__C1_i = _au4_ne ), (((& _au0__V72 )))) ) , (struct
#line 950 "../../src/dcl2.c"
ea *)ea0 ) ;
} }
else { 
#line 953 "../../src/dcl2.c"
switch (_au4_ne - _au5_i ){ 
#line 954 "../../src/dcl2.c"
case 1 : if (3 < _au4_ne )goto ee ;
case 2 : if (7 < _au4_ne )goto ee ;
case 3 : if (23 < _au4_ne )goto ee ;
case 4 : if (60 < _au4_ne )goto ee ;
case 5 : if (99 < _au4_ne )goto ee ;
}
}
}
}
curr_switch = _au2_old_switch ;
break ;
}
case 4 : 
#line 967 "../../src/dcl2.c"
if (curr_switch == 0 ){ 
#line 968 "../../src/dcl2.c"
error ( (char *)"case not in switch", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ,
#line 968 "../../src/dcl2.c"
(struct ea *)ea0 ) ;
break ;
}
_au1_ss -> _stmt__O8.__C8_e = _expr_typ ( _au1_ss -> _stmt__O8.__C8_e , _au2_tbl ) ;
( _type_kind ( _au1_ss -> _stmt__O8.__C8_e -> _expr__O2.__C2_tp , ((unsigned char )4 ), (unsigned char )'P' ) ) ;
{ Ptype _au4_tt ;

#line 973 "../../src/dcl2.c"
_au4_tt = _au1_ss -> _stmt__O8.__C8_e -> _expr__O2.__C2_tp ;
iii :
#line 975 "../../src/dcl2.c"
switch (_au4_tt -> _node_base ){ 
#line 976 "../../src/dcl2.c"
case 97 : 
#line 977 "../../src/dcl2.c"
_au4_tt = (((struct basetype *)_au4_tt ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 977 "../../src/dcl2.c"
goto iii ;
case 138 : 
#line 979 "../../src/dcl2.c"
case 141 : 
#line 980 "../../src/dcl2.c"
case 5 : 
#line 981 "../../src/dcl2.c"
case 29 : 
#line 982 "../../src/dcl2.c"
case 21 : 
#line 983 "../../src/dcl2.c"
case 22 : 
#line 984 "../../src/dcl2.c"
break ;
#line 984 "../../src/dcl2.c"

#line 985 "../../src/dcl2.c"
default : 
#line 986 "../../src/dcl2.c"
{ 
#line 1348 "../../src/dcl2.c"
struct ea _au0__V73 ;

#line 986 "../../src/dcl2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"%t caseE", (struct ea *)( ( ((& _au0__V73 )-> _ea__O1.__C1_p = ((char *)_au1_ss -> _stmt__O8.__C8_e ->
#line 986 "../../src/dcl2.c"
_expr__O2.__C2_tp )), (((& _au0__V73 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}
{ 
#line 990 "../../src/dcl2.c"
Neval = 0 ;
{ int _au4_i ;

#line 991 "../../src/dcl2.c"
_au4_i = _expr_eval ( _au1_ss -> _stmt__O8.__C8_e ) ;
if (Neval == 0 ){ 
#line 993 "../../src/dcl2.c"
Pstmt _au5_cs ;
for(_au5_cs = curr_switch -> _stmt__O9.__C9_case_list ;_au5_cs ;_au5_cs = _au5_cs -> _stmt__O9.__C9_case_list ) { 
#line 995 "../../src/dcl2.c"
if (_au5_cs -> _stmt__O7.__C7_case_value == _au4_i ){ 
#line 1348 "../../src/dcl2.c"
struct ea _au0__V74 ;

#line 995 "../../src/dcl2.c"
error ( (char *)"case %d used twice in switch", (struct ea *)( ( ((& _au0__V74 )-> _ea__O1.__C1_i = _au4_i ), (((& _au0__V74 )))) ) ,
#line 995 "../../src/dcl2.c"
(struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
_au1_ss -> _stmt__O7.__C7_case_value = _au4_i ;
_au1_ss -> _stmt__O9.__C9_case_list = curr_switch -> _stmt__O9.__C9_case_list ;
curr_switch -> _stmt__O9.__C9_case_list = _au1_ss ;
}
else 
#line 1002 "../../src/dcl2.c"
{ 
#line 1348 "../../src/dcl2.c"
struct ea _au0__V75 ;

#line 1002 "../../src/dcl2.c"
error ( (char *)"bad case label: %s", (struct ea *)( ( ((& _au0__V75 )-> _ea__O1.__C1_p = ((char *)Neval )), (((& _au0__V75 )))) )
#line 1002 "../../src/dcl2.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}

#line 1004 "../../src/dcl2.c"
if (_au1_ss -> _stmt_s -> _stmt_s_list ){ 
#line 1348 "../../src/dcl2.c"
struct ea _au0__V76 ;

#line 1004 "../../src/dcl2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"case%k", (struct ea *)( ( ((& _au0__V76 )-> _ea__O1.__C1_i = ((int )_au1_ss -> _stmt_s ->
#line 1004 "../../src/dcl2.c"
_stmt_s_list -> _node_base )), (((& _au0__V76 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} _au1_ss -> _stmt_s -> _stmt_s_list = _au1_ss -> _stmt_s_list ;
_au1_ss -> _stmt_s_list = 0 ;
_stmt_dcl ( _au1_ss -> _stmt_s ) ;
break ;

#line 1010 "../../src/dcl2.c"
case 19 : 
#line 1011 "../../src/dcl2.c"
inline_restr |= 2 ;
_stmt_reached ( _au1_ss ) ;
case 115 : 
#line 1017 "../../src/dcl2.c"
_au1_n = _au1_ss -> _stmt__O7.__C7_d ;
_au1_nn = _table_insert ( cc -> _dcl_context_ftbl , _au1_n , (unsigned char )115 ) ;

#line 1024 "../../src/dcl2.c"
if (_au1_ss -> _node_base == 115 ){ 
#line 1025 "../../src/dcl2.c"
_au1_nn -> _name__O6.__C6_n_realscope = curr_block -> _stmt_memtbl ;
inline_restr |= 1 ;
}

#line 1029 "../../src/dcl2.c"
if (Nold ){ 
#line 1030 "../../src/dcl2.c"
if (_au1_ss -> _node_base == 115 ){ 
#line 1031 "../../src/dcl2.c"
if (_au1_nn -> _expr__O4.__C4_n_initializer ){ 
#line 1348 "../../src/dcl2.c"
struct ea _au0__V77 ;

#line 1031 "../../src/dcl2.c"
error ( (char *)"twoDs of label%n", (struct ea *)( ( ((& _au0__V77 )-> _ea__O1.__C1_p = ((char *)_au1_n )), (((& _au0__V77 )))) )
#line 1031 "../../src/dcl2.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} _au1_nn -> _expr__O4.__C4_n_initializer = (((struct expr *)1 ));
}
if (_au1_n != _au1_nn )_au1_ss -> _stmt__O7.__C7_d = _au1_nn ;
}
else { 
#line 1037 "../../src/dcl2.c"
if (_au1_ss -> _node_base == 115 )_au1_nn -> _expr__O4.__C4_n_initializer = (((struct expr *)1 ));
_au1_nn -> _name_where = _au1_ss -> _stmt_where ;
}
if (_au1_ss -> _node_base == 19 )
#line 1041 "../../src/dcl2.c"
( (_au1_nn -> _name_n_used ++ )) ;
else { 
#line 1043 "../../src/dcl2.c"
if (_au1_ss -> _stmt_s -> _stmt_s_list ){ 
#line 1348 "../../src/dcl2.c"
struct ea _au0__V78 ;

#line 1043 "../../src/dcl2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"label%k", (struct ea *)( ( ((& _au0__V78 )-> _ea__O1.__C1_i = ((int )_au1_ss -> _stmt_s ->
#line 1043 "../../src/dcl2.c"
_stmt_s_list -> _node_base )), (((& _au0__V78 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} _au1_ss -> _stmt_s -> _stmt_s_list = _au1_ss -> _stmt_s_list ;
_au1_ss -> _stmt_s_list = 0 ;
_name_assign ( _au1_nn ) ;
}
if (_au1_ss -> _stmt_s )_stmt_dcl ( _au1_ss -> _stmt_s ) ;
break ;

#line 1051 "../../src/dcl2.c"
case 20 : 
#line 1052 "../../src/dcl2.c"
{ Pexpr _au4_ee ;

#line 1052 "../../src/dcl2.c"
_au4_ee = _expr_typ ( _au1_ss -> _stmt__O8.__C8_e , _au2_tbl ) ;
if (_au4_ee -> _node_base == 70 ){ 
#line 1054 "../../src/dcl2.c"
Neval = 0 ;
(_expr_eval ( _au4_ee -> _expr__O4.__C4_e2 ) );
if (Neval == 0 )
#line 1057 "../../src/dcl2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"constant assignment in condition", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 1057 "../../src/dcl2.c"
ea *)ea0 ) ;
}
_au1_ss -> _stmt__O8.__C8_e = (_au4_ee = check_cond ( _au4_ee , (unsigned char )20 , _au2_tbl ) );

#line 1061 "../../src/dcl2.c"
switch (_au4_ee -> _expr__O2.__C2_tp -> _node_base ){ 
#line 1062 "../../src/dcl2.c"
case 21 : 
#line 1063 "../../src/dcl2.c"
case 138 : 
#line 1064 "../../src/dcl2.c"
{ int _au6_i ;
Neval = 0 ;
_au6_i = _expr_eval ( _au4_ee ) ;

#line 1068 "../../src/dcl2.c"
if (Neval == 0 ){ 
#line 1069 "../../src/dcl2.c"
Pstmt _au7_sl ;

#line 1069 "../../src/dcl2.c"
_au7_sl = _au1_ss -> _stmt_s_list ;
if (_au6_i ){ 
#line 1071 "../../src/dcl2.c"
if (_au1_ss -> _stmt__O9.__C9_else_stmt && (_au1_ss -> _stmt__O9.__C9_else_stmt -> _node_permanent == 0 ))_stmt_del ( _au1_ss -> _stmt__O9.__C9_else_stmt ) ;
_stmt_dcl ( _au1_ss -> _stmt_s ) ;
(*_au1_ss )= (*_au1_ss -> _stmt_s );
}
else { 
#line 1076 "../../src/dcl2.c"
if (_au1_ss -> _stmt_s && (_au1_ss -> _stmt_s -> _node_permanent == 0 ))_stmt_del ( _au1_ss -> _stmt_s ) ;
if (_au1_ss -> _stmt__O9.__C9_else_stmt ){ 
#line 1078 "../../src/dcl2.c"
_stmt_dcl ( _au1_ss -> _stmt__O9.__C9_else_stmt ) ;
(*_au1_ss )= (*_au1_ss -> _stmt__O9.__C9_else_stmt );
}
else { 
#line 1082 "../../src/dcl2.c"
_au1_ss -> _node_base = 72 ;
_au1_ss -> _stmt__O8.__C8_e = dummy ;
_au1_ss -> _stmt_s = 0 ;
}
}
_au1_ss -> _stmt_s_list = _au7_sl ;
continue ;
}
}
}
_stmt_dcl ( _au1_ss -> _stmt_s ) ;
if (_au1_ss -> _stmt__O9.__C9_else_stmt )_stmt_dcl ( _au1_ss -> _stmt__O9.__C9_else_stmt ) ;
break ;
}
case 16 : 
#line 1097 "../../src/dcl2.c"
inline_restr |= 8 ;
_au2_old_loop = curr_loop ;
curr_loop = _au1_ss ;
if (_au1_ss -> _stmt__O9.__C9_for_init ){ 
#line 1101 "../../src/dcl2.c"
Pstmt _au4_fi ;

#line 1101 "../../src/dcl2.c"
_au4_fi = _au1_ss -> _stmt__O9.__C9_for_init ;
switch (_au4_fi -> _node_base ){ 
#line 1103 "../../src/dcl2.c"
case 72 : 
#line 1104 "../../src/dcl2.c"
if (_au4_fi -> _stmt__O8.__C8_e == dummy ){ 
#line 1105 "../../src/dcl2.c"
_au1_ss -> _stmt__O9.__C9_for_init = 0 ;
break ;
}
default : 
#line 1109 "../../src/dcl2.c"
_stmt_dcl ( _au4_fi ) ;
break ;
case 118 : 
#line 1112 "../../src/dcl2.c"
_stmt_dcl ( _au4_fi ) ;

#line 1114 "../../src/dcl2.c"
switch (_au4_fi -> _node_base ){ 
#line 1115 "../../src/dcl2.c"
case 116 : 
#line 1116 "../../src/dcl2.c"
{ 
#line 1121 "../../src/dcl2.c"
Pstmt _au7_tmp ;

#line 1121 "../../src/dcl2.c"
_au7_tmp = (struct stmt *)_stmt__ctor ( (struct stmt *)0 , (unsigned char )72 , curloc , (struct stmt *)0 ) ;
(*_au7_tmp )= (*_au1_ss );
_au7_tmp -> _stmt__O9.__C9_for_init = 0 ;
(*_au1_ss )= (*_au4_fi );
if (_au1_ss -> _stmt_s )
#line 1126 "../../src/dcl2.c"
_au1_ss -> _stmt_s -> _stmt_s_list = _au7_tmp ;
else 
#line 1128 "../../src/dcl2.c"
_au1_ss -> _stmt_s = _au7_tmp ;
curr_block = (((struct block *)_au1_ss ));
_au2_tbl = curr_block -> _stmt_memtbl ;
Cstmt = (_au1_ss = _au7_tmp );
break ;
}
}
}
}
if (_au1_ss -> _stmt__O8.__C8_e == dummy )
#line 1138 "../../src/dcl2.c"
_au1_ss -> _stmt__O8.__C8_e = 0 ;
else { 
#line 1140 "../../src/dcl2.c"
_au1_ss -> _stmt__O8.__C8_e = _expr_typ ( _au1_ss -> _stmt__O8.__C8_e , _au2_tbl ) ;
_au1_ss -> _stmt__O8.__C8_e = check_cond ( _au1_ss -> _stmt__O8.__C8_e , (unsigned char )16 , _au2_tbl ) ;
}
if (_au1_ss -> _stmt_s -> _node_base == 118 )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"D as onlyS in for-loop", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 1143 "../../src/dcl2.c"
ea *)ea0 , (struct ea *)ea0 ) ;
_stmt_dcl ( _au1_ss -> _stmt_s ) ;
_au1_ss -> _stmt__O7.__C7_e2 = ((_au1_ss -> _stmt__O7.__C7_e2 == dummy )? (((struct expr *)0 )): _expr_typ ( _au1_ss -> _stmt__O7.__C7_e2 , _au2_tbl ) );
curr_loop = _au2_old_loop ;
break ;

#line 1149 "../../src/dcl2.c"
case 118 : 
#line 1150 "../../src/dcl2.c"
{ 
#line 1154 "../../src/dcl2.c"
int _au4_non_trivial ;
int _au4_count ;
Pname _au4_tail ;

#line 1154 "../../src/dcl2.c"
_au4_non_trivial = 0 ;
_au4_count = 0 ;
_au4_tail = _au1_ss -> _stmt__O7.__C7_d ;
{ Pname _au4_nn ;

#line 1157 "../../src/dcl2.c"
_au4_nn = _au4_tail ;

#line 1157 "../../src/dcl2.c"
for(;_au4_nn ;_au4_nn = _au4_nn -> _name_n_list ) { 
#line 1160 "../../src/dcl2.c"
_au4_count ++ ;

#line 1162 "../../src/dcl2.c"
if (_au4_nn -> _name_n_list )_au4_tail = _au4_nn -> _name_n_list ;
{ Pname _au5_n ;

#line 1163 "../../src/dcl2.c"
_au5_n = _table_look ( _au2_tbl , _au4_nn -> _expr__O3.__C3_string , (unsigned char )0 ) ;
if (_au5_n && (_au5_n -> _expr__O5.__C5_n_table == _au2_tbl ))_au4_non_trivial = 2 ;
if (_au4_non_trivial == 2 )continue ;
if ((((_au4_nn -> _name_n_sto == 31 )&& (_au4_nn -> _expr__O2.__C2_tp -> _node_base != 108 ))|| _type_is_ref ( _au4_nn -> _expr__O2.__C2_tp ) )|| (_type_tconst ( _au4_nn ->
#line 1166 "../../src/dcl2.c"
_expr__O2.__C2_tp ) && (fct_const == 0 )))
#line 1168 "../../src/dcl2.c"
{ 
#line 1169 "../../src/dcl2.c"
_au4_non_trivial = 2 ;
continue ;
}
{ Pexpr _au5_in ;

#line 1172 "../../src/dcl2.c"
_au5_in = _au4_nn -> _expr__O4.__C4_n_initializer ;
if (_au5_in )
#line 1174 "../../src/dcl2.c"
switch (_au5_in -> _node_base ){ 
#line 1175 "../../src/dcl2.c"
case 124 : 
#line 1176 "../../src/dcl2.c"
case 81 : 
#line 1177 "../../src/dcl2.c"
_au4_non_trivial = 2 ;
continue ;
default : 
#line 1180 "../../src/dcl2.c"
_au4_non_trivial = 1 ;
}
{ Pname _au5_cln ;

#line 1182 "../../src/dcl2.c"
_au5_cln = _type_is_cl_obj ( _au4_nn -> _expr__O2.__C2_tp ) ;
if (_au5_cln == 0 )_au5_cln = cl_obj_vec ;
if (_au5_cln == 0 )continue ;
if (( _table_look ( (((struct classdef *)_au5_cln -> _expr__O2.__C2_tp ))-> _classdef_memtbl , "_ctor", (unsigned char )0 ) ) ){ 
#line 1186 "../../src/dcl2.c"
_au4_non_trivial =
#line 1186 "../../src/dcl2.c"
2 ;
continue ;
}
if (( _table_look ( (((struct classdef *)_au5_cln -> _expr__O2.__C2_tp ))-> _classdef_memtbl , "_dtor", (unsigned char )0 ) ) )_au4_non_trivial = 2 ;
#line 1189 "../../src/dcl2.c"
}
}
}
}

#line 1192 "../../src/dcl2.c"
while (_au1_ss -> _stmt_s_list && (_au1_ss -> _stmt_s_list -> _node_base == 118 )){ 
#line 1193 "../../src/dcl2.c"
Pstmt _au5_sx ;

#line 1193 "../../src/dcl2.c"
_au5_sx = _au1_ss -> _stmt_s_list ;
_au4_tail = (_au4_tail -> _name_n_list = _au5_sx -> _stmt__O7.__C7_d );
for(_au4_nn = _au5_sx -> _stmt__O7.__C7_d ;_au4_nn ;_au4_nn = _au4_nn -> _name_n_list ) { 
#line 1198 "../../src/dcl2.c"
_au4_count ++ ;
if (_au4_nn -> _name_n_list )_au4_tail = _au4_nn -> _name_n_list ;
{ Pname _au6_n ;

#line 1200 "../../src/dcl2.c"
_au6_n = _table_look ( _au2_tbl , _au4_nn -> _expr__O3.__C3_string , (unsigned char )0 ) ;
if (_au6_n && (_au6_n -> _expr__O5.__C5_n_table == _au2_tbl ))_au4_non_trivial = 2 ;
if (_au4_non_trivial == 2 )continue ;
if ((((_au4_nn -> _name_n_sto == 31 )&& (_au4_nn -> _expr__O2.__C2_tp -> _node_base != 108 ))|| _type_is_ref ( _au4_nn -> _expr__O2.__C2_tp ) )|| (_type_tconst ( _au4_nn ->
#line 1203 "../../src/dcl2.c"
_expr__O2.__C2_tp ) && (fct_const == 0 )))
#line 1205 "../../src/dcl2.c"
{ 
#line 1206 "../../src/dcl2.c"
_au4_non_trivial = 2 ;
continue ;
}
{ Pexpr _au6_in ;

#line 1209 "../../src/dcl2.c"
_au6_in = _au4_nn -> _expr__O4.__C4_n_initializer ;
if (_au6_in )
#line 1211 "../../src/dcl2.c"
switch (_au6_in -> _node_base ){ 
#line 1212 "../../src/dcl2.c"
case 124 : 
#line 1213 "../../src/dcl2.c"
case 81 : 
#line 1214 "../../src/dcl2.c"
_au4_non_trivial = 2 ;
continue ;
}
_au4_non_trivial = 1 ;
{ Pname _au6_cln ;

#line 1218 "../../src/dcl2.c"
_au6_cln = _type_is_cl_obj ( _au4_nn -> _expr__O2.__C2_tp ) ;
if (_au6_cln == 0 )_au6_cln = cl_obj_vec ;
if (_au6_cln == 0 )continue ;
if (( _table_look ( (((struct classdef *)_au6_cln -> _expr__O2.__C2_tp ))-> _classdef_memtbl , "_ctor", (unsigned char )0 ) ) ){ 
#line 1222 "../../src/dcl2.c"
_au4_non_trivial =
#line 1222 "../../src/dcl2.c"
2 ;
continue ;
}
if (( _table_look ( (((struct classdef *)_au6_cln -> _expr__O2.__C2_tp ))-> _classdef_memtbl , "_dtor", (unsigned char )0 ) ) )continue ;
#line 1225 "../../src/dcl2.c"
}
}
}
}

#line 1227 "../../src/dcl2.c"
_au1_ss -> _stmt_s_list = _au5_sx -> _stmt_s_list ;
}

#line 1230 "../../src/dcl2.c"
{ Pstmt _au4_next_st ;

#line 1230 "../../src/dcl2.c"
_au4_next_st = _au1_ss -> _stmt_s_list ;

#line 1232 "../../src/dcl2.c"
if ((_au4_non_trivial == 2 )|| ((_au4_non_trivial == 1 )&& ((curr_block -> _stmt__O8.__C8_own_tbl == 0 )|| (inline_restr & 3 ))))
#line 1237 "../../src/dcl2.c"
{ 
#line 1238 "../../src/dcl2.c"
if (curr_switch && (_au4_non_trivial == 2 )){
#line 1238 "../../src/dcl2.c"

#line 1239 "../../src/dcl2.c"
Pstmt _au6_cs ;
Pstmt _au6_ds ;
Pstmt _au6_bl ;

#line 1239 "../../src/dcl2.c"
_au6_cs = curr_switch -> _stmt__O9.__C9_case_list ;
_au6_ds = curr_switch -> _stmt__O7.__C7_has_default ;

#line 1242 "../../src/dcl2.c"
if (_au6_cs == 0 )
#line 1243 "../../src/dcl2.c"
_au6_bl = _au6_ds ;
else if (_au6_ds == 0 )
#line 1245 "../../src/dcl2.c"
_au6_bl = _au6_cs ;
else if (_au6_cs -> _stmt_where . _loc_line < _au6_ds -> _stmt_where . _loc_line )
#line 1247 "../../src/dcl2.c"
_au6_bl = _au6_ds ;
else 
#line 1249 "../../src/dcl2.c"
_au6_bl = _au6_cs ;

#line 1251 "../../src/dcl2.c"
if (((_au6_bl == 0 )|| (_au6_bl -> _stmt_s -> _node_base != 116 ))&& (curr_switch -> _stmt_s -> _stmt_memtbl == _au2_tbl ))
#line 1252 "../../src/dcl2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char
#line 1252 "../../src/dcl2.c"
*)"non trivialD in switchS (try enclosing it in a block)", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
}

#line 1260 "../../src/dcl2.c"
_au1_ss -> _node_base = 116 ;

#line 1265 "../../src/dcl2.c"
for(_au4_nn = _au1_ss -> _stmt__O7.__C7_d ;_au4_nn ;_au4_nn = _au4_nn -> _name_n_list ) { 
#line 1266 "../../src/dcl2.c"
Pname _au6_n ;
if ((curr_block -> _stmt__O8.__C8_own_tbl && (_au6_n = _table_look ( curr_block -> _stmt_memtbl , _au4_nn -> _expr__O3.__C3_string , (unsigned char )0 ) ))&& (_au6_n ->
#line 1267 "../../src/dcl2.c"
_expr__O5.__C5_n_table -> _table_real_block == curr_block -> _stmt_memtbl -> _table_real_block ))
#line 1270 "../../src/dcl2.c"
{ 
#line 1348 "../../src/dcl2.c"
struct ea _au0__V79 ;

#line 1270 "../../src/dcl2.c"
error ( (char *)"twoDs of%n", (struct ea *)( ( ((& _au0__V79 )-> _ea__O1.__C1_p = ((char *)_au6_n )), (((& _au0__V79 )))) )
#line 1270 "../../src/dcl2.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }

#line 1276 "../../src/dcl2.c"
_au1_ss -> _stmt_s = _au4_next_st ;
_au1_ss -> _stmt_s_list = 0 ;

#line 1282 "../../src/dcl2.c"
_au1_ss -> _stmt_memtbl = (struct table *)_table__ctor ( (struct table *)0 , (short )(_au4_count + 4 ), _au2_tbl , (struct name *)0 ) ;
_au1_ss -> _stmt_memtbl -> _table_real_block = curr_block -> _stmt_memtbl -> _table_real_block ;

#line 1285 "../../src/dcl2.c"
_block_dcl ( ((struct block *)_au1_ss ), _au1_ss -> _stmt_memtbl ) ;
}
else { 
#line 1293 "../../src/dcl2.c"
Pstmt _au5_sss ;

#line 1293 "../../src/dcl2.c"
_au5_sss = _au1_ss ;
for(_au4_nn = _au1_ss -> _stmt__O7.__C7_d ;_au4_nn ;_au4_nn = _au4_nn -> _name_n_list ) { 
#line 1295 "../../src/dcl2.c"
Pname _au6_n ;

#line 1295 "../../src/dcl2.c"
_au6_n = _name_dcl ( _au4_nn , _au2_tbl , (unsigned char )108 ) ;

#line 1297 "../../src/dcl2.c"
if (_au6_n == 0 )continue ;
{ Pexpr _au6_in ;

#line 1299 "../../src/dcl2.c"
struct estmt *_au0__Xthis__ctor_estmt ;

#line 1298 "../../src/dcl2.c"
_au6_in = _au6_n -> _expr__O4.__C4_n_initializer ;
_au6_n -> _expr__O4.__C4_n_initializer = 0 ;
if (_au1_ss ){ 
#line 1301 "../../src/dcl2.c"
_au5_sss -> _node_base = 72 ;
_au1_ss = 0 ;
}
else 
#line 1305 "../../src/dcl2.c"
_au5_sss = (_au5_sss -> _stmt_s_list = (struct stmt *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt = (struct
#line 1305 "../../src/dcl2.c"
estmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_estmt ), ((unsigned char )72 ), _au5_sss -> _stmt_where , ((struct stmt *)0 )) )) , ( (_au0__Xthis__ctor_estmt ->
#line 1305 "../../src/dcl2.c"
_stmt__O8.__C8_e = ((struct expr *)0 )), ((_au0__Xthis__ctor_estmt ))) ) ) );
if (_au6_in ){ 
#line 1307 "../../src/dcl2.c"
switch (_au6_in -> _node_base ){ 
#line 1308 "../../src/dcl2.c"
case 146 : 
#line 1309 "../../src/dcl2.c"
{ 
#line 1310 "../../src/dcl2.c"
Pname _au9_fn ;

#line 1310 "../../src/dcl2.c"
_au9_fn = _au6_in -> _expr__O5.__C5_fct_name ;
if (_au9_fn && (_au9_fn -> _name_n_oper == 161 ))break ;
}
default : 
#line 1314 "../../src/dcl2.c"
_au6_in = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )70 , (struct expr *)_au6_n , _au6_in ) ;
#line 1314 "../../src/dcl2.c"
}

#line 1316 "../../src/dcl2.c"
_au5_sss -> _stmt__O8.__C8_e = _expr_typ ( _au6_in , _au2_tbl ) ;
}
else 
#line 1319 "../../src/dcl2.c"
_au5_sss -> _stmt__O8.__C8_e = dummy ;
}
}

#line 1321 "../../src/dcl2.c"
_au1_ss = _au5_sss ;
_au1_ss -> _stmt_s_list = _au4_next_st ;
}
break ;
}
}
}

#line 1327 "../../src/dcl2.c"
case 116 : 
#line 1328 "../../src/dcl2.c"
_block_dcl ( ((struct block *)_au1_ss ), _au2_tbl ) ;
break ;

#line 1331 "../../src/dcl2.c"
case 1 : 
#line 1333 "../../src/dcl2.c"
{ 
#line 1334 "../../src/dcl2.c"
char *_au4_s ;
int _au4_ll ;
char *_au4_s2 ;

#line 1334 "../../src/dcl2.c"
_au4_s = (((char *)_au1_ss -> _stmt__O8.__C8_e ));
_au4_ll = strlen ( (char *)_au4_s ) ;
_au4_s2 = (((char *)_new ( (long )((sizeof (char ))* (_au4_ll + 1 ))) ));
strcpy ( _au4_s2 , (char *)_au4_s ) ;
_au1_ss -> _stmt__O8.__C8_e = (((struct expr *)_au4_s2 ));
break ;
}

#line 1342 "../../src/dcl2.c"
default : 
#line 1343 "../../src/dcl2.c"
{ 
#line 1348 "../../src/dcl2.c"
struct ea _au0__V80 ;

#line 1348 "../../src/dcl2.c"
struct ea _au0__V81 ;

#line 1343 "../../src/dcl2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"badS(%p %d)", (struct ea *)( ( ((& _au0__V80 )-> _ea__O1.__C1_p = ((char *)_au1_ss )), (((&
#line 1343 "../../src/dcl2.c"
_au0__V80 )))) ) , (struct ea *)( ( ((& _au0__V81 )-> _ea__O1.__C1_i = ((int )_au1_ss -> _node_base )), (((& _au0__V81 ))))
#line 1343 "../../src/dcl2.c"
) , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}

#line 1347 "../../src/dcl2.c"
Cstmt = _au1_ostmt ;
}
;
extern int in_class_dcl ;
char _block_dcl (_au0_this , _au0_tbl )
#line 700 "../../src/cfront.h"
struct block *_au0_this ;

#line 1351 "../../src/dcl2.c"
Ptable _au0_tbl ;

#line 1357 "../../src/dcl2.c"
{ 
#line 1358 "../../src/dcl2.c"
int _au1_bit_old ;
int _au1_byte_old ;
int _au1_max_old ;
Pblock _au1_block_old ;

#line 1380 "../../src/dcl2.c"
Pname _au2_nx ;

#line 1358 "../../src/dcl2.c"
_au1_bit_old = bit_offset ;
_au1_byte_old = byte_offset ;
_au1_max_old = max_align ;
_au1_block_old = curr_block ;

#line 1363 "../../src/dcl2.c"
if (_au0_this -> _node_base != 116 ){ 
#line 1462 "../../src/dcl2.c"
struct ea _au0__V82 ;

#line 1363 "../../src/dcl2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"block::dcl(%d)", (struct ea *)( ( ((& _au0__V82 )-> _ea__O1.__C1_i = ((int )_au0_this -> _node_base )),
#line 1363 "../../src/dcl2.c"
(((& _au0__V82 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} 
#line 1365 "../../src/dcl2.c"
curr_block = _au0_this ;

#line 1367 "../../src/dcl2.c"
if (_au0_this -> _stmt__O7.__C7_d ){ 
#line 1368 "../../src/dcl2.c"
_au0_this -> _stmt__O8.__C8_own_tbl = 1 ;
if (_au0_this -> _stmt_memtbl == 0 ){ 
#line 1370 "../../src/dcl2.c"
int _au3_nmem ;

#line 1370 "../../src/dcl2.c"
_au3_nmem = (_name_no_of_names ( _au0_this -> _stmt__O7.__C7_d ) + 4 );
_au0_this -> _stmt_memtbl = (struct table *)_table__ctor ( (struct table *)0 , (short )_au3_nmem , _au0_tbl , (struct name *)0 ) ;
_au0_this -> _stmt_memtbl -> _table_real_block = (struct stmt *)_au0_this ;
}
else 
#line 1378 "../../src/dcl2.c"
if (_au0_this -> _stmt_memtbl != _au0_tbl )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"block::dcl(?)", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 1378 "../../src/dcl2.c"
ea *)ea0 , (struct ea *)ea0 ) ;

#line 1380 "../../src/dcl2.c"
;
{ Pname _au2_n ;

#line 1381 "../../src/dcl2.c"
_au2_n = _au0_this -> _stmt__O7.__C7_d ;

#line 1381 "../../src/dcl2.c"
for(;_au2_n ;_au2_n = _au2_nx ) { 
#line 1382 "../../src/dcl2.c"
_au2_nx = _au2_n -> _name_n_list ;
_name_dcl ( _au2_n , _au0_this -> _stmt_memtbl , (unsigned char )108 ) ;
switch (_au2_n -> _expr__O2.__C2_tp -> _node_base ){ 
#line 1385 "../../src/dcl2.c"
case 6 : 
#line 1386 "../../src/dcl2.c"
case 167 : 
#line 1387 "../../src/dcl2.c"
case 13 : 
#line 1388 "../../src/dcl2.c"
break ;
default : 
#line 1390 "../../src/dcl2.c"
_name__dtor ( _au2_n , 1) ;
}
}
}
}
else _au0_this -> _stmt_memtbl = _au0_tbl ;

#line 1397 "../../src/dcl2.c"
if (_au0_this -> _stmt_s ){ 
#line 1398 "../../src/dcl2.c"
Pname _au2_odcl ;
Pname _au2_m ;
int _au2_i ;

#line 1398 "../../src/dcl2.c"
_au2_odcl = Cdcl ;

#line 1402 "../../src/dcl2.c"
_stmt_dcl ( _au0_this -> _stmt_s ) ;

#line 1404 "../../src/dcl2.c"
if (_au0_this -> _stmt__O8.__C8_own_tbl )
#line 1405 "../../src/dcl2.c"
for(_au2_m = _table_get_mem ( _au0_this -> _stmt_memtbl , _au2_i = 1 ) ;_au2_m ;_au2_m = _table_get_mem ( _au0_this -> _stmt_memtbl , ++ _au2_i )
#line 1405 "../../src/dcl2.c"
) { 
#line 1406 "../../src/dcl2.c"
Ptype _au3_t ;

#line 1406 "../../src/dcl2.c"
_au3_t = _au2_m -> _expr__O2.__C2_tp ;

#line 1408 "../../src/dcl2.c"
if (in_class_dcl )_au2_m -> _name_lex_level -= 1 ;
if (_au3_t == 0 ){ 
#line 1410 "../../src/dcl2.c"
if (_au2_m -> _name_n_assigned_to == 0 )
#line 1411 "../../src/dcl2.c"
{ 
#line 1462 "../../src/dcl2.c"
struct ea _au0__V83 ;

#line 1411 "../../src/dcl2.c"
error ( (char *)"label %sU", (struct ea *)( ( ((& _au0__V83 )-> _ea__O1.__C1_p = ((char *)_au2_m -> _expr__O3.__C3_string )), (((& _au0__V83 ))))
#line 1411 "../../src/dcl2.c"
) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} if (_au2_m -> _name_n_used == 0 )
#line 1413 "../../src/dcl2.c"
{ 
#line 1462 "../../src/dcl2.c"
struct ea _au0__V84 ;

#line 1413 "../../src/dcl2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"label %s not used", (struct ea *)( ( ((& _au0__V84 )-> _ea__O1.__C1_p = ((char *)_au2_m -> _expr__O3.__C3_string )),
#line 1413 "../../src/dcl2.c"
(((& _au0__V84 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} continue ;
}
ll :
#line 1417 "../../src/dcl2.c"
switch (_au3_t -> _node_base ){ 
#line 1418 "../../src/dcl2.c"
case 97 : _au3_t = (((struct basetype *)_au3_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 1418 "../../src/dcl2.c"
goto ll ;
case 6 : 
#line 1420 "../../src/dcl2.c"
case 13 : 
#line 1421 "../../src/dcl2.c"
case 108 : 
#line 1422 "../../src/dcl2.c"
case 110 : continue ;
}

#line 1425 "../../src/dcl2.c"
if (_au2_m -> _name_n_addr_taken == 0 ){ 
#line 1426 "../../src/dcl2.c"
if (_au2_m -> _name_n_used ){ 
#line 1427 "../../src/dcl2.c"
if (_au2_m -> _name_n_assigned_to ){ }
else 
#line 1429 "../../src/dcl2.c"
{ 
#line 1430 "../../src/dcl2.c"
switch (_au2_m ->
#line 1430 "../../src/dcl2.c"
_name_n_scope ){ 
#line 1431 "../../src/dcl2.c"
case 108 : 
#line 1432 "../../src/dcl2.c"
Cdcl = _au2_m ;
{ 
#line 1462 "../../src/dcl2.c"
struct ea _au0__V85 ;

#line 1433 "../../src/dcl2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"%n used but not set", (struct ea *)( ( ((& _au0__V85 )-> _ea__O1.__C1_p = ((char *)_au2_m )), (((&
#line 1433 "../../src/dcl2.c"
_au0__V85 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}
}
else { 
#line 1438 "../../src/dcl2.c"
if (_au2_m -> _name_n_assigned_to ){ }
else 
#line 1440 "../../src/dcl2.c"
{ 
#line 1441 "../../src/dcl2.c"
switch (_au2_m -> _name_n_scope ){ 
#line 1442 "../../src/dcl2.c"
case 136 : 
#line 1443 "../../src/dcl2.c"
if
#line 1443 "../../src/dcl2.c"
(((_au2_m -> _expr__O3.__C3_string [0 ])== '_' )&& ((_au2_m -> _expr__O3.__C3_string [1 ])== 'A' ))break ;
case 108 : 
#line 1445 "../../src/dcl2.c"
Cdcl = _au2_m ;
{ 
#line 1462 "../../src/dcl2.c"
struct ea _au0__V86 ;

#line 1446 "../../src/dcl2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"%n not used", (struct ea *)( ( ((& _au0__V86 )-> _ea__O1.__C1_p = ((char *)_au2_m )), (((&
#line 1446 "../../src/dcl2.c"
_au0__V86 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}
}
}
}
Cdcl = _au2_odcl ;
}

#line 1455 "../../src/dcl2.c"
_au0_this -> _stmt__O7.__C7_d = 0 ;

#line 1457 "../../src/dcl2.c"
if (bit_offset )byte_offset += SZ_WORD ;
if (stack_size < byte_offset )stack_size = byte_offset ;
bit_offset = _au1_bit_old ;
byte_offset = _au1_byte_old ;
curr_block = _au1_block_old ;
}
;
char _name_field_align (_au0_this )
#line 607 "../../src/cfront.h"
struct name *_au0_this ;

#line 1466 "../../src/dcl2.c"
{ 
#line 1467 "../../src/dcl2.c"
Pbase _au1_fld ;

#line 1469 "../../src/dcl2.c"
int _au1_a ;

#line 1490 "../../src/dcl2.c"
int _au1_x ;

#line 1467 "../../src/dcl2.c"
_au1_fld = (((struct basetype *)_au0_this -> _expr__O2.__C2_tp ));

#line 1469 "../../src/dcl2.c"
_au1_a = (F_SENSITIVE ? _type_align ( _au1_fld -> _basetype_b_fieldtype ) : SZ_WORD );
if (max_align < _au1_a )max_align = _au1_a ;

#line 1472 "../../src/dcl2.c"
if (_au1_fld -> _basetype_b_bits == 0 ){ 
#line 1473 "../../src/dcl2.c"
int _au2_b ;
if (bit_offset )
#line 1475 "../../src/dcl2.c"
_au1_fld -> _basetype_b_bits = (BI_IN_WORD - bit_offset );
else if (_au2_b = (byte_offset % SZ_WORD ))
#line 1477 "../../src/dcl2.c"
_au1_fld -> _basetype_b_bits = (_au2_b * BI_IN_BYTE );
else 
#line 1479 "../../src/dcl2.c"
_au1_fld -> _basetype_b_bits = BI_IN_WORD ;
if (max_align < SZ_WORD )max_align = SZ_WORD ;
}
else if (bit_offset == 0 ){ 
#line 1483 "../../src/dcl2.c"
int _au2_b ;

#line 1483 "../../src/dcl2.c"
_au2_b = (byte_offset % SZ_WORD );
if (_au2_b ){ 
#line 1485 "../../src/dcl2.c"
byte_offset -= _au2_b ;
bit_offset = (_au2_b * BI_IN_BYTE );
}
}

#line 1490 "../../src/dcl2.c"
_au1_x = (bit_offset += _au1_fld -> _basetype_b_bits );
if (BI_IN_WORD < _au1_x ){ 
#line 1492 "../../src/dcl2.c"
_au1_fld -> _basetype_b_offset = 0 ;
byte_offset += SZ_WORD ;
bit_offset = _au1_fld -> _basetype_b_bits ;
}
else { 
#line 1497 "../../src/dcl2.c"
_au1_fld -> _basetype_b_offset = bit_offset ;
if (BI_IN_WORD == _au1_x ){ 
#line 1499 "../../src/dcl2.c"
bit_offset = 0 ;
byte_offset += SZ_WORD ;
}
else 
#line 1503 "../../src/dcl2.c"
bit_offset = _au1_x ;
}
_au0_this -> _name_n_offset = byte_offset ;
}
;

/* the end */
