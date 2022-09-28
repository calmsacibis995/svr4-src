/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#line 1 "../../src/simpl.c"

/* <<cfront 1.2.1 2/16/87>> */
/* < ../../src/simpl.c */

#ident	"@(#)sdb:cfront/scratch/src/simpl..c	1.2"
char *_new(); char _delete(); char *_vec_new(); char _vec_delete();

#line 1 "../../src/simpl.c"

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

#include <ctype.h>

#line 35 "../../src/simpl.c"
Pname make_tmp ();
Pexpr init_tmp ();

#line 38 "../../src/simpl.c"
Pname new_fct ;
Pname del_fct ;
Pname vec_new_fct ;
Pname vec_del_fct ;
Pstmt del_list ;
Pstmt break_del_list ;
Pstmt continue_del_list ;
bit not_inl ;
Pname curr_fct ;
Pexpr init_list ;
Pexpr one ;

#line 50 "../../src/simpl.c"
Pexpr cdvec (_au0_f , _au0_vec , _au0_cl , _au0_cd , _au0_tail )Pname _au0_f ;

#line 50 "../../src/simpl.c"
Pexpr _au0_vec ;

#line 50 "../../src/simpl.c"
Pclass _au0_cl ;

#line 50 "../../src/simpl.c"
Pname _au0_cd ;

#line 50 "../../src/simpl.c"
int _au0_tail ;

#line 54 "../../src/simpl.c"
{ 
#line 55 "../../src/simpl.c"
Pexpr _au1_sz ;

#line 58 "../../src/simpl.c"
Pexpr _au1_esz ;

#line 61 "../../src/simpl.c"
Pexpr _au1_noe ;

#line 66 "../../src/simpl.c"
Pexpr _au1_arg ;

#line 67 "../../src/simpl.c"
struct texpr *_au0__Xthis__ctor_texpr ;

#line 67 "../../src/simpl.c"
struct call *_au0__Xthis__ctor_call ;

#line 55 "../../src/simpl.c"
_au1_sz = (struct expr *)( (_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr = (struct texpr *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_texpr ),
#line 55 "../../src/simpl.c"
((unsigned char )30 ), ((struct expr *)0 ), (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 = ((struct type *)_au0_cl )),
#line 55 "../../src/simpl.c"
((_au0__Xthis__ctor_texpr ))) ) ) ;
_au1_sz -> _expr__O2.__C2_tp = (struct type *)uint_type ;

#line 58 "../../src/simpl.c"
_au1_esz = (struct expr *)( (_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr = (struct texpr *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_texpr ),
#line 58 "../../src/simpl.c"
((unsigned char )30 ), ((struct expr *)0 ), (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 = ((struct type *)_au0_cl )),
#line 58 "../../src/simpl.c"
((_au0__Xthis__ctor_texpr ))) ) ) ;
_au1_esz -> _expr__O2.__C2_tp = (struct type *)int_type ;

#line 61 "../../src/simpl.c"
_au1_noe = (struct expr *)( (_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr = (struct texpr *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_texpr ),
#line 61 "../../src/simpl.c"
((unsigned char )30 ), ((struct expr *)0 ), (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 = _au0_vec -> _expr__O2.__C2_tp ),
#line 61 "../../src/simpl.c"
((_au0__Xthis__ctor_texpr ))) ) ) ;
_au1_noe -> _expr__O2.__C2_tp = (struct type *)int_type ;
_au1_noe = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )51 , _au1_noe , _au1_esz ) ;
_au1_noe -> _expr__O2.__C2_tp = (struct type *)uint_type ;

#line 66 "../../src/simpl.c"
_au1_arg = ((0 <= _au0_tail )? _expr__ctor ( (struct expr *)0 , (unsigned char )140 , zero , (struct expr *)0 ) : (((struct
#line 66 "../../src/simpl.c"
expr *)0 )));

#line 68 "../../src/simpl.c"
_au1_arg = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , (struct expr *)_au0_cd , _au1_arg ) ;
_expr_lval ( (struct expr *)_au0_cd , (unsigned char )112 ) ;

#line 71 "../../src/simpl.c"
_au1_arg = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , _au1_sz , _au1_arg ) ;
_au1_arg = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , _au1_noe , _au1_arg ) ;
_au1_arg = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , _au0_vec , _au1_arg ) ;

#line 75 "../../src/simpl.c"
_au1_arg = (struct expr *)( (_au0__Xthis__ctor_call = 0 ), ( ( (_au0__Xthis__ctor_call = 0 ), (_au0__Xthis__ctor_call = (struct call *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_call ),
#line 75 "../../src/simpl.c"
(unsigned char )109 , ((struct expr *)_au0_f ), _au1_arg ) )) , ((_au0__Xthis__ctor_call ))) ) ;
_au1_arg -> _node_base = 146 ;
_au1_arg -> _expr__O5.__C5_fct_name = _au0_f ;

#line 79 "../../src/simpl.c"
return _au1_arg ;
}
;

#line 83 "../../src/simpl.c"
Pstmt trim_tail (_au0_tt )Pstmt _au0_tt ;

#line 88 "../../src/simpl.c"
{ 
#line 89 "../../src/simpl.c"
Pstmt _au1_tx ;
while (_au0_tt -> _stmt_s_list ){ 
#line 92 "../../src/simpl.c"
switch (_au0_tt -> _node_base ){ 
#line 93 "../../src/simpl.c"
case 166 : 
#line 94 "../../src/simpl.c"
_au1_tx = trim_tail ( _au0_tt -> _stmt__O8.__C8_s2 ) ;
goto txl ;
case 116 : 
#line 97 "../../src/simpl.c"
_au1_tx = trim_tail ( _au0_tt -> _stmt_s ) ;
txl :
#line 100 "../../src/simpl.c"
switch (_au1_tx -> _node_base ){ 
#line 101 "../../src/simpl.c"
case 72 : 
#line 102 "../../src/simpl.c"
break ;
case 7 : 
#line 104 "../../src/simpl.c"
case 3 : 
#line 105 "../../src/simpl.c"
case 19 : 
#line 106 "../../src/simpl.c"
case 28 : 
#line 108 "../../src/simpl.c"
_au0_tt -> _stmt_s_list = 0 ;
default : 
#line 110 "../../src/simpl.c"
return _au1_tx ;
}
default : 
#line 113 "../../src/simpl.c"
_au0_tt = _au0_tt -> _stmt_s_list ;
break ;
case 28 : 
#line 117 "../../src/simpl.c"
_au0_tt -> _stmt_s_list = 0 ;
return _au0_tt ;
}
}

#line 122 "../../src/simpl.c"
switch (_au0_tt -> _node_base ){ 
#line 123 "../../src/simpl.c"
case 166 : return trim_tail ( _au0_tt -> _stmt__O8.__C8_s2 ) ;

#line 125 "../../src/simpl.c"
case 116 : if (_au0_tt -> _stmt_s )return trim_tail ( _au0_tt -> _stmt_s ) ;
default : return _au0_tt ;
}
}
;
char simpl_init ()
#line 131 "../../src/simpl.c"
{ 
#line 132 "../../src/simpl.c"
Pname _au1_nw ;
Pname _au1_dl ;
Pname _au1_vn ;
Pname _au1_vd ;

#line 139 "../../src/simpl.c"
Pname _au1_a ;

#line 161 "../../src/simpl.c"
Pname _au1_al ;

#line 162 "../../src/simpl.c"
struct ival *_au0__Xthis__ctor_ival ;

#line 132 "../../src/simpl.c"
_au1_nw = (struct name *)_name__ctor ( (struct name *)0 , oper_name ( (unsigned char )23 ) ) ;
_au1_dl = (struct name *)_name__ctor ( (struct name *)0 , oper_name ( (unsigned char )9 ) ) ;
_au1_vn = (struct name *)_name__ctor ( (struct name *)0 , "_vec_new") ;
_au1_vd = (struct name *)_name__ctor ( (struct name *)0 , "_vec_delete") ;

#line 137 "../../src/simpl.c"
new_fct = _table_insert ( gtbl , _au1_nw , (unsigned char )0 ) ;
_name__dtor ( _au1_nw , 1) ;
_au1_a = (struct name *)_name__ctor ( (struct name *)0 , (char *)0 ) ;
_au1_a -> _expr__O2.__C2_tp = (struct type *)long_type ;
new_fct -> _expr__O2.__C2_tp = (struct type *)_fct__ctor ( (struct fct *)0 , Pvoid_type , _au1_a , (unsigned char )1 ) ;
new_fct -> _name_n_scope = 14 ;
new_fct -> _node_permanent = 1 ;
new_fct -> _expr__O2.__C2_tp -> _node_permanent = 1 ;
( (new_fct -> _name_n_used ++ )) ;

#line 148 "../../src/simpl.c"
del_fct = _table_insert ( gtbl , _au1_dl , (unsigned char )0 ) ;
_name__dtor ( _au1_dl , 1) ;
_au1_a = (struct name *)_name__ctor ( (struct name *)0 , (char *)0 ) ;
_au1_a -> _expr__O2.__C2_tp = Pvoid_type ;
del_fct -> _expr__O2.__C2_tp = (struct type *)_fct__ctor ( (struct fct *)0 , (struct type *)void_type , _au1_a , (unsigned char )1 ) ;
del_fct -> _name_n_scope = 14 ;
del_fct -> _node_permanent = 1 ;
del_fct -> _expr__O2.__C2_tp -> _node_permanent = 1 ;
( (del_fct -> _name_n_used ++ )) ;

#line 159 "../../src/simpl.c"
_au1_a = (struct name *)_name__ctor ( (struct name *)0 , (char *)0 ) ;
_au1_a -> _expr__O2.__C2_tp = Pvoid_type ;
_au1_al = _au1_a ;
_au1_a = (struct name *)_name__ctor ( (struct name *)0 , (char *)0 ) ;
_au1_a -> _expr__O2.__C2_tp = (struct type *)int_type ;
_au1_a -> _name_n_list = _au1_al ;
_au1_al = _au1_a ;
_au1_a = (struct name *)_name__ctor ( (struct name *)0 , (char *)0 ) ;
_au1_a -> _expr__O2.__C2_tp = (struct type *)int_type ;
_au1_a -> _name_n_list = _au1_al ;
_au1_al = _au1_a ;
_au1_a = (struct name *)_name__ctor ( (struct name *)0 , (char *)0 ) ;
_au1_a -> _expr__O2.__C2_tp = Pvoid_type ;
_au1_a -> _name_n_list = _au1_al ;
_au1_al = _au1_a ;

#line 175 "../../src/simpl.c"
vec_new_fct = _table_insert ( gtbl , _au1_vn , (unsigned char )0 ) ;
_name__dtor ( _au1_vn , 1) ;
vec_new_fct -> _expr__O2.__C2_tp = (struct type *)_fct__ctor ( (struct fct *)0 , Pvoid_type , _au1_al , (unsigned char )1 ) ;
vec_new_fct -> _name_n_scope = 14 ;
vec_new_fct -> _node_permanent = 1 ;
vec_new_fct -> _expr__O2.__C2_tp -> _node_permanent = 1 ;
( (vec_new_fct -> _name_n_used ++ )) ;

#line 184 "../../src/simpl.c"
_au1_a = (struct name *)_name__ctor ( (struct name *)0 , (char *)0 ) ;
_au1_a -> _expr__O2.__C2_tp = (struct type *)int_type ;
_au1_al = _au1_a ;
_au1_a = (struct name *)_name__ctor ( (struct name *)0 , (char *)0 ) ;
_au1_a -> _expr__O2.__C2_tp = Pvoid_type ;
_au1_a -> _name_n_list = _au1_al ;
_au1_al = _au1_a ;
_au1_a = (struct name *)_name__ctor ( (struct name *)0 , (char *)0 ) ;
_au1_a -> _expr__O2.__C2_tp = (struct type *)int_type ;
_au1_a -> _name_n_list = _au1_al ;
_au1_al = _au1_a ;
_au1_a = (struct name *)_name__ctor ( (struct name *)0 , (char *)0 ) ;
_au1_a -> _expr__O2.__C2_tp = (struct type *)int_type ;
_au1_a -> _name_n_list = _au1_al ;
_au1_al = _au1_a ;
_au1_a = (struct name *)_name__ctor ( (struct name *)0 , (char *)0 ) ;
_au1_a -> _expr__O2.__C2_tp = Pvoid_type ;
_au1_a -> _name_n_list = _au1_al ;
_au1_al = _au1_a ;

#line 204 "../../src/simpl.c"
vec_del_fct = _table_insert ( gtbl , _au1_vd , (unsigned char )0 ) ;
_name__dtor ( _au1_vd , 1) ;
vec_del_fct -> _expr__O2.__C2_tp = (struct type *)_fct__ctor ( (struct fct *)0 , (struct type *)void_type , _au1_al , (unsigned char )1 ) ;
vec_del_fct -> _name_n_scope = 14 ;
vec_del_fct -> _node_permanent = 1 ;
vec_del_fct -> _expr__O2.__C2_tp -> _node_permanent = 1 ;
( (vec_del_fct -> _name_n_used ++ )) ;

#line 213 "../../src/simpl.c"
one = (struct expr *)( (_au0__Xthis__ctor_ival = 0 ), ( ( (_au0__Xthis__ctor_ival = 0 ), (_au0__Xthis__ctor_ival = (struct ival *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ival ),
#line 213 "../../src/simpl.c"
(unsigned char )150 , (struct expr *)0 , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ival -> _expr__O3.__C3_i1 = 1 ), ((_au0__Xthis__ctor_ival )))
#line 213 "../../src/simpl.c"
) ) ;
one -> _expr__O2.__C2_tp = (struct type *)int_type ;
one -> _node_permanent = 1 ;

#line 218 "../../src/simpl.c"
fputs ( (char *)"char *_new(); char _delete(); char *_vec_new(); char _vec_delete();\n", out_file ) ;
}
;

#line 222 "../../src/simpl.c"
Ptable scope ;
Pname expand_fn ;
Ptable expand_tbl ;

#line 226 "../../src/simpl.c"
Pname _classdef_has_oper (_au0_this , _au0_op )
#line 319 "../../src/cfront.h"
struct classdef *_au0_this ;

#line 226 "../../src/simpl.c"
TOK _au0_op ;
{ 
#line 228 "../../src/simpl.c"
char *_au1_s ;
Pname _au1_n ;

#line 228 "../../src/simpl.c"
_au1_s = oper_name ( _au0_op ) ;

#line 230 "../../src/simpl.c"
if (_au0_this == 0 ){ 
#line 238 "../../src/simpl.c"
struct ea _au0__V10 ;

#line 230 "../../src/simpl.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"0->has_oper(%s)", (struct ea *)( ( ((& _au0__V10 )-> _ea__O1.__C1_p = ((char *)_au1_s )), (((&
#line 230 "../../src/simpl.c"
_au0__V10 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} _au1_n = _table_lookc ( _au0_this -> _classdef_memtbl , _au1_s , (unsigned char )0 ) ;
if (_au1_n == 0 )return (struct name *)0 ;
switch (_au1_n -> _name_n_scope ){ 
#line 234 "../../src/simpl.c"
case 0 : 
#line 235 "../../src/simpl.c"
case 25 : return _au1_n ;
default : return (struct name *)0 ;
}
}
;
int is_expr (_au0_s )Pstmt _au0_s ;

#line 245 "../../src/simpl.c"
{ 
#line 246 "../../src/simpl.c"
int _au1_i ;
Pstmt _au1_ss ;

#line 246 "../../src/simpl.c"
_au1_i = 0 ;
for(_au1_ss = ((_au0_s -> _node_base == 116 )? _au0_s -> _stmt_s : _au0_s );_au1_ss ;_au1_ss = _au1_ss -> _stmt_s_list ) { 
#line 249 "../../src/simpl.c"
switch (_au1_ss -> _node_base ){ 
#line 250 "../../src/simpl.c"
case
#line 250 "../../src/simpl.c"
116 : 
#line 251 "../../src/simpl.c"
if ((((struct block *)_au1_ss ))-> _stmt_memtbl || (is_expr ( _au1_ss -> _stmt_s ) == 0 ))return (int )0 ;
case 72 : 
#line 253 "../../src/simpl.c"
if (_au1_ss -> _stmt__O8.__C8_e -> _node_base == 168 ){ 
#line 254 "../../src/simpl.c"
Pname _au4_fn ;
Pfct _au4_f ;

#line 254 "../../src/simpl.c"
_au4_fn = _au1_ss -> _stmt__O8.__C8_e -> _expr__O5.__C5_il -> _iline_fct_name ;
_au4_f = (((struct fct *)_au4_fn -> _expr__O2.__C2_tp ));
if (_au4_f -> _fct_f_expr == 0 )return (int )0 ;
}
break ;
case 20 : 
#line 260 "../../src/simpl.c"
if (is_expr ( _au1_ss -> _stmt_s ) == 0 )return (int )0 ;
if (_au1_ss -> _stmt__O9.__C9_else_stmt && (is_expr ( _au1_ss -> _stmt__O9.__C9_else_stmt ) == 0 ))return (int )0 ;
break ;
default : 
#line 264 "../../src/simpl.c"
return (int )0 ;
}
_au1_i ++ ;
}
return _au1_i ;
}
;
int no_of_returns = 0 ;

#line 273 "../../src/simpl.c"
char _name_simpl (_au0_this )
#line 607 "../../src/cfront.h"
struct name *_au0_this ;

#line 274 "../../src/simpl.c"
{ 
#line 276 "../../src/simpl.c"
if (_au0_this -> _node_base == 25 )return ;

#line 278 "../../src/simpl.c"
if (_au0_this -> _expr__O2.__C2_tp == 0 ){ 
#line 371 "../../src/simpl.c"
struct ea _au0__V11 ;

#line 278 "../../src/simpl.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"%n->N::simple(tp==0)", (struct ea *)( ( ((& _au0__V11 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((&
#line 278 "../../src/simpl.c"
_au0__V11 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} 
#line 280 "../../src/simpl.c"
switch (_au0_this -> _expr__O2.__C2_tp -> _node_base ){ 
#line 281 "../../src/simpl.c"
case 0 : 
#line 282 "../../src/simpl.c"
{ 
#line 371 "../../src/simpl.c"
struct ea _au0__V12 ;

#line 282 "../../src/simpl.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"%n->N::simpl(tp->B==0)", (struct ea *)( ( ((& _au0__V12 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((&
#line 282 "../../src/simpl.c"
_au0__V12 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;

#line 284 "../../src/simpl.c"
case 76 : 
#line 285 "../../src/simpl.c"
{ { Plist _au3_gl ;

#line 285 "../../src/simpl.c"
_au3_gl = (((struct gen *)_au0_this -> _expr__O2.__C2_tp ))-> _gen_fct_list ;

#line 285 "../../src/simpl.c"
for(;_au3_gl ;_au3_gl = _au3_gl -> _name_list_l ) _name_simpl ( _au3_gl -> _name_list_f ) ;
break ;
}
}
case 108 : 
#line 290 "../../src/simpl.c"
{ Pfct _au3_f ;
Pname _au3_n ;
Pname _au3_th ;

#line 290 "../../src/simpl.c"
_au3_f = (((struct fct *)_au0_this -> _expr__O2.__C2_tp ));

#line 292 "../../src/simpl.c"
_au3_th = _au3_f -> _fct_f_this ;

#line 294 "../../src/simpl.c"
if (_au3_th ){ 
#line 296 "../../src/simpl.c"
if (2 < _au3_th -> _name_n_used )
#line 297 "../../src/simpl.c"
_au3_th -> _name_n_sto = 27 ;
else _au3_th -> _name_n_sto = 0 ;
if (_au0_this -> _name_n_oper == 161 )_au3_f -> _fct_s_returns = _au3_th -> _expr__O2.__C2_tp ;
}

#line 302 "../../src/simpl.c"
if (_au0_this -> _expr__O2.__C2_tp -> _type_defined & -2)return ;

#line 304 "../../src/simpl.c"
for(_au3_n = (_au3_th ? _au3_th : (_au3_f -> _fct_f_result ? _au3_f -> _fct_f_result : _au3_f -> _fct_argtype ));_au3_n ;_au3_n = _au3_n -> _name_n_list ) _name_simpl ( _au3_n ) ;
#line 304 "../../src/simpl.c"

#line 306 "../../src/simpl.c"
if (_au3_f -> _fct_body ){ 
#line 307 "../../src/simpl.c"
Ptable _au4_oscope ;

#line 307 "../../src/simpl.c"
_au4_oscope = scope ;
scope = _au3_f -> _fct_body -> _stmt_memtbl ;
if (scope == 0 ){ 
#line 371 "../../src/simpl.c"
struct ea _au0__V13 ;

#line 309 "../../src/simpl.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"%n memtbl missing", (struct ea *)( ( ((& _au0__V13 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((&
#line 309 "../../src/simpl.c"
_au0__V13 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} curr_fct = _au0_this ;
_fct_simpl ( _au3_f ) ;
if (_au3_f -> _fct_f_inline && (debug == 0 )){ 
#line 313 "../../src/simpl.c"
if (8<= _au3_f -> _fct_nargs ){ 
#line 314 "../../src/simpl.c"
{ 
#line 371 "../../src/simpl.c"
struct ea _au0__V14 ;

#line 314 "../../src/simpl.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"too manyAs for inline%n (inline ignored)", (struct ea *)( ( ((& _au0__V14 )-> _ea__O1.__C1_p = ((char *)_au0_this )), (((&
#line 314 "../../src/simpl.c"
_au0__V14 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
_au3_f -> _fct_f_inline = 0 ;
scope = _au4_oscope ;
break ;
} }
{ int _au5_i ;

#line 319 "../../src/simpl.c"
_au5_i = 0 ;
for(_au3_n = (_au3_th ? _au3_th : (_au3_f -> _fct_f_result ? _au3_f -> _fct_f_result : _au3_f -> _fct_argtype ));_au3_n ;_au3_n = _au3_n -> _name_n_list ) { 
#line 321 "../../src/simpl.c"
_au3_n -> _node_base =
#line 321 "../../src/simpl.c"
169 ;
_au3_n -> _name_n_val = (_au5_i ++ );
if (_au3_n -> _expr__O5.__C5_n_table != scope ){ 
#line 371 "../../src/simpl.c"
struct ea _au0__V15 ;

#line 371 "../../src/simpl.c"
struct ea _au0__V16 ;

#line 371 "../../src/simpl.c"
struct ea _au0__V17 ;

#line 323 "../../src/simpl.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"aname scope: %s %d %d\n", (struct ea *)( ( ((& _au0__V15 )-> _ea__O1.__C1_p = ((char *)_au3_n -> _expr__O3.__C3_string )),
#line 323 "../../src/simpl.c"
(((& _au0__V15 )))) ) , (struct ea *)( ( ((& _au0__V16 )-> _ea__O1.__C1_p = ((char *)_au3_n -> _expr__O5.__C5_n_table )), (((&
#line 323 "../../src/simpl.c"
_au0__V16 )))) ) , (struct ea *)( ( ((& _au0__V17 )-> _ea__O1.__C1_p = ((char *)scope )), (((& _au0__V17 )))) )
#line 323 "../../src/simpl.c"
, (struct ea *)ea0 ) ;
} }
expand_tbl = (((_au3_f -> _fct_returns -> _node_base != 38 )|| (_au0_this -> _name_n_oper == 161 ))? scope : (((struct table *)0 )));
expand_fn = _au0_this ;
if (expand_tbl ){ 
#line 328 "../../src/simpl.c"
genlab :
#line 331 "../../src/simpl.c"
{ Pexpr _au6_ee ;

#line 331 "../../src/simpl.c"
_au6_ee = (((struct expr *)_stmt_expand ( (struct stmt *)_au3_f -> _fct_body ) ));

#line 333 "../../src/simpl.c"
_au3_f -> _fct_f_expr = ((_au6_ee -> _node_base == 71 )? _au6_ee : _expr__ctor ( (struct expr *)0 , (unsigned char )71 , zero , _au6_ee )
#line 333 "../../src/simpl.c"
);
}
}
else { 
#line 337 "../../src/simpl.c"
if (is_expr ( (struct stmt *)_au3_f -> _fct_body ) ){ 
#line 339 "../../src/simpl.c"
_au3_f -> _fct_s_returns = (struct type *)int_type ;
expand_tbl = scope ;
goto genlab ;
}

#line 345 "../../src/simpl.c"
_au3_f -> _fct_f_expr = 0 ;
_au3_f -> _fct_body = (((struct block *)_stmt_expand ( (struct stmt *)_au3_f -> _fct_body ) ));
}
expand_fn = 0 ;
expand_tbl = 0 ;
}
}

#line 351 "../../src/simpl.c"
scope = _au4_oscope ;
}
break ;
}

#line 356 "../../src/simpl.c"
case 6 : 
#line 357 "../../src/simpl.c"
_classdef_simpl ( ((struct classdef *)_au0_this -> _expr__O2.__C2_tp )) ;
break ;

#line 364 "../../src/simpl.c"
default : 
#line 366 "../../src/simpl.c"
break ;
} }

#line 369 "../../src/simpl.c"
if (_au0_this -> _expr__O4.__C4_n_initializer )_expr_simpl ( _au0_this -> _expr__O4.__C4_n_initializer ) ;
_au0_this -> _expr__O2.__C2_tp -> _type_defined |= 02 ;
}
;
char _fct_simpl (_au0_this )
#line 400 "../../src/cfront.h"
struct fct *_au0_this ;

#line 395 "../../src/simpl.c"
{ 
#line 396 "../../src/simpl.c"
Pexpr _au1_th ;
Ptable _au1_tbl ;
Pstmt _au1_ss ;
Pstmt _au1_tail ;
Pname _au1_cln ;
Pclass _au1_cl ;
Pstmt _au1_dtail ;

#line 505 "../../src/simpl.c"
int _au1_ass_count ;

#line 506 "../../src/simpl.c"
struct ifstmt *_au0__Xthis__ctor_ifstmt ;

#line 396 "../../src/simpl.c"
_au1_th = (struct expr *)_au0_this -> _fct_f_this ;
_au1_tbl = _au0_this -> _fct_body -> _stmt_memtbl ;
_au1_ss = 0 ;

#line 402 "../../src/simpl.c"
_au1_dtail = 0 ;

#line 404 "../../src/simpl.c"
not_inl = (debug || (_au0_this -> _fct_f_inline == 0 ));
del_list = 0 ;
continue_del_list = 0 ;
break_del_list = 0 ;
scope = _au1_tbl ;
if (scope == 0 )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"F::simpl()", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 409 "../../src/simpl.c"
ea *)ea0 ) ;

#line 411 "../../src/simpl.c"
if (_au1_th ){ 
#line 412 "../../src/simpl.c"
Pptr _au2_p ;

#line 412 "../../src/simpl.c"
_au2_p = (((struct ptr *)_au1_th -> _expr__O2.__C2_tp ));
_au1_cln = (((struct basetype *)_au2_p -> _pvtyp_typ ))-> _basetype_b_name ;
_au1_cl = (((struct classdef *)_au1_cln -> _expr__O2.__C2_tp ));
}

#line 417 "../../src/simpl.c"
if (curr_fct -> _name_n_oper == 162 ){ 
#line 418 "../../src/simpl.c"
Pexpr _au2_ee ;
Pstmt _au2_es ;
Pname _au2_bcln ;
Pclass _au2_bcl ;
Pname _au2_d ;

#line 424 "../../src/simpl.c"
Pname _au2_fa ;

#line 426 "../../src/simpl.c"
Pname _au2_free_arg ;

#line 430 "../../src/simpl.c"
Ptable _au2_tbl ;
int _au2_i ;
Pname _au2_m ;

#line 433 "../../src/simpl.c"
struct ifstmt *_au0__Xthis__ctor_ifstmt ;

#line 420 "../../src/simpl.c"
_au2_bcln = _au1_cl -> _classdef_clbase ;

#line 424 "../../src/simpl.c"
_au2_fa = (struct name *)_name__ctor ( (struct name *)0 , "_free") ;
_au2_fa -> _expr__O2.__C2_tp = (struct type *)int_type ;
_au2_free_arg = _name_dcl ( _au2_fa , _au0_this -> _fct_body -> _stmt_memtbl , (unsigned char )136 ) ;
_name__dtor ( _au2_fa , 1) ;
_au0_this -> _fct_f_this -> _name_n_list = _au2_free_arg ;

#line 430 "../../src/simpl.c"
_au2_tbl = _au1_cl -> _classdef_memtbl ;

#line 430 "../../src/simpl.c"
;

#line 430 "../../src/simpl.c"
;

#line 435 "../../src/simpl.c"
for(_au2_m = _table_get_mem ( _au2_tbl , _au2_i = 1 ) ;_au2_m ;_au2_m = _table_get_mem ( _au2_tbl , ++ _au2_i ) ) { 
#line 436 "../../src/simpl.c"
Ptype _au3_t ;
Pname _au3_cn ;
Pclass _au3_cl ;
Pname _au3_dtor ;

#line 436 "../../src/simpl.c"
_au3_t = _au2_m -> _expr__O2.__C2_tp ;

#line 440 "../../src/simpl.c"
if (_au2_m -> _name_n_stclass == 31 )continue ;

#line 442 "../../src/simpl.c"
if (_au3_cn = _type_is_cl_obj ( _au3_t ) ){ 
#line 443 "../../src/simpl.c"
_au3_cl = (((struct classdef *)_au3_cn -> _expr__O2.__C2_tp ));
if (_au3_dtor = ( _table_look ( _au3_cl -> _classdef_memtbl , "_dtor", (unsigned char )0 ) ) ){ 
#line 446 "../../src/simpl.c"
Pexpr _au5_aa ;

#line 447 "../../src/simpl.c"
struct ref *_au0__Xthis__ctor_ref ;

#line 447 "../../src/simpl.c"
struct call *_au0__Xthis__ctor_call ;

#line 447 "../../src/simpl.c"
struct estmt *_au0__Xthis__ctor_estmt ;

#line 446 "../../src/simpl.c"
_au5_aa = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , zero , (struct expr *)0 ) ;
_au2_ee = (struct expr *)( (_au0__Xthis__ctor_ref = 0 ), ( ( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ref ),
#line 447 "../../src/simpl.c"
((unsigned char )44 ), _au1_th , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = _au2_m ), ((_au0__Xthis__ctor_ref ))) )
#line 447 "../../src/simpl.c"
) ;
_au2_ee = (struct expr *)( (_au0__Xthis__ctor_ref = 0 ), ( ( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ref ),
#line 448 "../../src/simpl.c"
((unsigned char )45 ), _au2_ee , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = _au3_dtor ), ((_au0__Xthis__ctor_ref ))) )
#line 448 "../../src/simpl.c"
) ;
_au2_ee = (struct expr *)( (_au0__Xthis__ctor_call = 0 ), ( ( (_au0__Xthis__ctor_call = 0 ), (_au0__Xthis__ctor_call = (struct call *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_call ),
#line 449 "../../src/simpl.c"
(unsigned char )109 , _au2_ee , _au5_aa ) )) , ((_au0__Xthis__ctor_call ))) ) ;
_au2_ee -> _expr__O5.__C5_fct_name = _au3_dtor ;
_au2_ee -> _node_base = 146 ;
_au2_es = (struct stmt *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt = (struct estmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_estmt ),
#line 452 "../../src/simpl.c"
((unsigned char )72 ), curloc , ((struct stmt *)0 )) )) , ( (_au0__Xthis__ctor_estmt -> _stmt__O8.__C8_e = _au2_ee ), ((_au0__Xthis__ctor_estmt ))) )
#line 452 "../../src/simpl.c"
) ;
if (_au1_dtail )
#line 454 "../../src/simpl.c"
_au1_dtail -> _stmt_s_list = _au2_es ;
else 
#line 456 "../../src/simpl.c"
del_list = _au2_es ;
_au1_dtail = _au2_es ;
}
}
else if (cl_obj_vec ){ 
#line 461 "../../src/simpl.c"
_au3_cl = (((struct classdef *)cl_obj_vec -> _expr__O2.__C2_tp ));
if (_au3_dtor = ( _table_look ( _au3_cl -> _classdef_memtbl , "_dtor", (unsigned char )0 ) ) ){ 
#line 463 "../../src/simpl.c"
Pexpr _au5_mm ;

#line 465 "../../src/simpl.c"
Pexpr _au5_ee ;

#line 466 "../../src/simpl.c"
struct estmt *_au0__Xthis__ctor_estmt ;

#line 466 "../../src/simpl.c"
struct ref *_au0__Xthis__ctor_ref ;

#line 463 "../../src/simpl.c"
_au5_mm = (struct expr *)( (_au0__Xthis__ctor_ref = 0 ), ( ( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ref ),
#line 463 "../../src/simpl.c"
((unsigned char )44 ), _au1_th , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = _au2_m ), ((_au0__Xthis__ctor_ref ))) )
#line 463 "../../src/simpl.c"
) ;
_au5_mm -> _expr__O2.__C2_tp = _au2_m -> _expr__O2.__C2_tp ;
_au5_ee = cdvec ( vec_del_fct , _au5_mm , _au3_cl , _au3_dtor , (int )0 ) ;
_au2_es = (struct stmt *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt = (struct estmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_estmt ),
#line 466 "../../src/simpl.c"
((unsigned char )72 ), curloc , ((struct stmt *)0 )) )) , ( (_au0__Xthis__ctor_estmt -> _stmt__O8.__C8_e = _au5_ee ), ((_au0__Xthis__ctor_estmt ))) )
#line 466 "../../src/simpl.c"
) ;
if (_au1_dtail )
#line 468 "../../src/simpl.c"
_au1_dtail -> _stmt_s_list = _au2_es ;
else 
#line 470 "../../src/simpl.c"
del_list = _au2_es ;
_au1_dtail = _au2_es ;
}
}
}

#line 477 "../../src/simpl.c"
if ((_au2_bcln && (_au2_bcl = (((struct classdef *)_au2_bcln -> _expr__O2.__C2_tp ))))&& (_au2_d = ( _table_look ( _au2_bcl -> _classdef_memtbl , "_dtor", (unsigned char
#line 477 "../../src/simpl.c"
)0 ) ) ))
#line 479 "../../src/simpl.c"
{ 
#line 480 "../../src/simpl.c"
Pexpr _au3_aa ;

#line 481 "../../src/simpl.c"
struct ref *_au0__Xthis__ctor_ref ;

#line 481 "../../src/simpl.c"
struct call *_au0__Xthis__ctor_call ;

#line 481 "../../src/simpl.c"
struct estmt *_au0__Xthis__ctor_estmt ;

#line 480 "../../src/simpl.c"
_au3_aa = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , (struct expr *)_au2_free_arg , (struct expr *)0 ) ;
_au2_ee = (struct expr *)( (_au0__Xthis__ctor_ref = 0 ), ( ( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ref ),
#line 481 "../../src/simpl.c"
((unsigned char )44 ), _au1_th , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = _au2_d ), ((_au0__Xthis__ctor_ref ))) )
#line 481 "../../src/simpl.c"
) ;
_au2_ee = (struct expr *)( (_au0__Xthis__ctor_call = 0 ), ( ( (_au0__Xthis__ctor_call = 0 ), (_au0__Xthis__ctor_call = (struct call *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_call ),
#line 482 "../../src/simpl.c"
(unsigned char )109 , _au2_ee , _au3_aa ) )) , ((_au0__Xthis__ctor_call ))) ) ;

#line 484 "../../src/simpl.c"
_au2_ee -> _node_base = 146 ;
_au2_es = (struct stmt *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt = (struct estmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_estmt ),
#line 485 "../../src/simpl.c"
((unsigned char )72 ), curloc , ((struct stmt *)0 )) )) , ( (_au0__Xthis__ctor_estmt -> _stmt__O8.__C8_e = _au2_ee ), ((_au0__Xthis__ctor_estmt ))) )
#line 485 "../../src/simpl.c"
) ;
}
else { 
#line 488 "../../src/simpl.c"
Pexpr _au3_aa ;

#line 489 "../../src/simpl.c"
struct call *_au0__Xthis__ctor_call ;

#line 489 "../../src/simpl.c"
struct estmt *_au0__Xthis__ctor_estmt ;

#line 489 "../../src/simpl.c"
struct ifstmt *_au0__Xthis__ctor_ifstmt ;

#line 488 "../../src/simpl.c"
_au3_aa = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , _au1_th , (struct expr *)0 ) ;
_au2_ee = (struct expr *)( (_au0__Xthis__ctor_call = 0 ), ( ( (_au0__Xthis__ctor_call = 0 ), (_au0__Xthis__ctor_call = (struct call *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_call ),
#line 489 "../../src/simpl.c"
(unsigned char )109 , ((struct expr *)del_fct ), _au3_aa ) )) , ((_au0__Xthis__ctor_call ))) ) ;
_au2_ee -> _expr__O5.__C5_fct_name = del_fct ;
_au2_ee -> _node_base = 146 ;
_au2_es = (struct stmt *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt = (struct estmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_estmt ),
#line 492 "../../src/simpl.c"
((unsigned char )72 ), curloc , ((struct stmt *)0 )) )) , ( (_au0__Xthis__ctor_estmt -> _stmt__O8.__C8_e = _au2_ee ), ((_au0__Xthis__ctor_estmt ))) )
#line 492 "../../src/simpl.c"
) ;
_au2_es = (struct stmt *)( (_au0__Xthis__ctor_ifstmt = 0 ), ( ( (_au0__Xthis__ctor_ifstmt = 0 ), (_au0__Xthis__ctor_ifstmt = (struct ifstmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_ifstmt ),
#line 493 "../../src/simpl.c"
(unsigned char )20 , curloc , _au2_es ) )) , ( (_au0__Xthis__ctor_ifstmt -> _stmt__O8.__C8_e = ((struct expr *)_au2_free_arg )), ( (_au0__Xthis__ctor_ifstmt ->
#line 493 "../../src/simpl.c"
_stmt__O9.__C9_else_stmt = ((struct stmt *)0 )), ((_au0__Xthis__ctor_ifstmt ))) ) ) ) ;
}
( (_au2_free_arg -> _name_n_used ++ )) ;
( ((((struct name *)_au1_th ))-> _name_n_used ++ )) ;
if (_au1_dtail )
#line 498 "../../src/simpl.c"
_au1_dtail -> _stmt_s_list = _au2_es ;
else 
#line 500 "../../src/simpl.c"
del_list = _au2_es ;
del_list = (struct stmt *)( (_au0__Xthis__ctor_ifstmt = 0 ), ( ( (_au0__Xthis__ctor_ifstmt = 0 ), (_au0__Xthis__ctor_ifstmt = (struct ifstmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_ifstmt ),
#line 501 "../../src/simpl.c"
(unsigned char )20 , curloc , del_list ) )) , ( (_au0__Xthis__ctor_ifstmt -> _stmt__O8.__C8_e = _au1_th ), ( (_au0__Xthis__ctor_ifstmt -> _stmt__O9.__C9_else_stmt =
#line 501 "../../src/simpl.c"
((struct stmt *)0 )), ((_au0__Xthis__ctor_ifstmt ))) ) ) ) ;
if (del_list )_stmt_simpl ( del_list ) ;
}

#line 505 "../../src/simpl.c"
;

#line 507 "../../src/simpl.c"
if (curr_fct -> _name_n_oper == 161 ){ 
#line 508 "../../src/simpl.c"
Pexpr _au2_ee ;
Ptable _au2_tbl ;
Pname _au2_m ;
int _au2_i ;

#line 545 "../../src/simpl.c"
Pname _au2_nn ;

#line 509 "../../src/simpl.c"
_au2_tbl = _au1_cl -> _classdef_memtbl ;

#line 517 "../../src/simpl.c"
if (_au0_this -> _fct_b_init ){ 
#line 519 "../../src/simpl.c"
switch (_au0_this -> _fct_b_init -> _node_base ){ 
#line 520 "../../src/simpl.c"
case 70 : 
#line 521 "../../src/simpl.c"
case 71 : 
#line 522 "../../src/simpl.c"
break ;
default : 
#line 524 "../../src/simpl.c"
{ Pcall _au5_cc ;
Pname _au5_bn ;
Pname _au5_tt ;

#line 524 "../../src/simpl.c"
_au5_cc = (((struct call *)_au0_this -> _fct_b_init ));
_au5_bn = _au5_cc -> _expr__O5.__C5_fct_name ;
_au5_tt = (((struct fct *)_au5_bn -> _expr__O2.__C2_tp ))-> _fct_f_this ;
_au1_ass_count = _au5_tt -> _name_n_assigned_to ;
_call_simpl ( _au5_cc ) ;
init_list = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )70 , _au1_th , (struct expr *)_au5_cc ) ;
}
}
}
else { 
#line 534 "../../src/simpl.c"
_au1_ass_count = 0 ;
init_list = 0 ;
}

#line 538 "../../src/simpl.c"
if (_au1_cl -> _classdef_virt_count ){ 
#line 539 "../../src/simpl.c"
Pname _au3_vp ;
Pexpr _au3_vtbl ;

#line 541 "../../src/simpl.c"
struct ref *_au0__Xthis__ctor_ref ;

#line 541 "../../src/simpl.c"
struct text_expr *_au0__Xthis__ctor_text_expr ;

#line 539 "../../src/simpl.c"
_au3_vp = _table_look ( _au1_cl -> _classdef_memtbl , "_vptr", (unsigned char )0 ) ;
_au3_vtbl = (struct expr *)( (_au0__Xthis__ctor_text_expr = 0 ), ( (_au0__Xthis__ctor_text_expr = (struct text_expr *)_new ( (long )(sizeof (struct text_expr))) ), (
#line 540 "../../src/simpl.c"
(_au0__Xthis__ctor_text_expr = (struct text_expr *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_text_expr ), (unsigned char )165 , (struct expr *)0 , (struct expr *)0 ) ), (
#line 540 "../../src/simpl.c"
(_au0__Xthis__ctor_text_expr -> _expr__O3.__C3_string = _au1_cl -> _classdef_string ), ( (_au0__Xthis__ctor_text_expr -> _expr__O4.__C4_string2 = "_vtbl"), ((_au0__Xthis__ctor_text_expr ))) ) ) ) ) ;
#line 540 "../../src/simpl.c"

#line 541 "../../src/simpl.c"
_au2_ee = (struct expr *)( (_au0__Xthis__ctor_ref = 0 ), ( ( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ref ),
#line 541 "../../src/simpl.c"
((unsigned char )44 ), _au1_th , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = _au3_vp ), ((_au0__Xthis__ctor_ref ))) )
#line 541 "../../src/simpl.c"
) ;
_au2_ee = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )70 , _au2_ee , _au3_vtbl ) ;
init_list = (init_list ? _expr__ctor ( (struct expr *)0 , (unsigned char )71 , init_list , _au2_ee ) : _au2_ee );
}
for(_au2_nn = _au0_this -> _fct_f_init ;_au2_nn ;_au2_nn = _au2_nn -> _name_n_list ) { 
#line 546 "../../src/simpl.c"
if (_au2_nn -> _expr__O4.__C4_n_initializer == 0 )continue ;
{ Pname _au3_m ;

#line 547 "../../src/simpl.c"
_au3_m = _table_look ( _au2_tbl , _au2_nn -> _expr__O3.__C3_string , (unsigned char )0 ) ;
if (_au3_m && (_au3_m -> _expr__O5.__C5_n_table == _au2_tbl ))_au3_m -> _expr__O4.__C4_n_initializer = _au2_nn -> _expr__O4.__C4_n_initializer ;
}
}
for(_au2_m = _table_get_mem ( _au2_tbl , _au2_i = 1 ) ;_au2_m ;_au2_m = _table_get_mem ( _au2_tbl , ++ _au2_i ) ) { 
#line 552 "../../src/simpl.c"
Ptype _au3_t ;
Pname _au3_cn ;
Pclass _au3_cl ;
Pname _au3_ctor ;

#line 552 "../../src/simpl.c"
_au3_t = _au2_m -> _expr__O2.__C2_tp ;

#line 557 "../../src/simpl.c"
switch (_au2_m -> _name_n_stclass ){ 
#line 558 "../../src/simpl.c"
case 31 : 
#line 559 "../../src/simpl.c"
case 13 : 
#line 560 "../../src/simpl.c"
continue ;
}
switch (_au3_t -> _node_base ){ 
#line 563 "../../src/simpl.c"
case 108 : 
#line 564 "../../src/simpl.c"
case 76 : 
#line 565 "../../src/simpl.c"
case 6 : 
#line 566 "../../src/simpl.c"
case 13 : 
#line 567 "../../src/simpl.c"
continue ;
}
if (_au2_m -> _node_base == 25 )continue ;

#line 571 "../../src/simpl.c"
if (_au3_cn = _type_is_cl_obj ( _au3_t ) ){ 
#line 572 "../../src/simpl.c"
Pexpr _au4_ee ;

#line 573 "../../src/simpl.c"
struct ref *_au0__Xthis__ctor_ref ;

#line 573 "../../src/simpl.c"
struct call *_au0__Xthis__ctor_call ;

#line 572 "../../src/simpl.c"
_au4_ee = _au2_m -> _expr__O4.__C4_n_initializer ;
_au2_m -> _expr__O4.__C4_n_initializer = 0 ;

#line 575 "../../src/simpl.c"
if (_au4_ee == 0 ){ 
#line 576 "../../src/simpl.c"
_au3_cl = (((struct classdef *)_au3_cn -> _expr__O2.__C2_tp ));
if (_au3_ctor = _classdef_has_ictor ( _au3_cl ) ){ 
#line 578 "../../src/simpl.c"
_au4_ee = (struct expr *)( (_au0__Xthis__ctor_ref = 0 ), ( ( (_au0__Xthis__ctor_ref = 0 ),
#line 578 "../../src/simpl.c"
(_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ref ), ((unsigned char )44 ), _au1_th , (struct expr *)0 ) )) , (
#line 578 "../../src/simpl.c"
(_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = _au2_m ), ((_au0__Xthis__ctor_ref ))) ) ) ;
_au4_ee = (struct expr *)( (_au0__Xthis__ctor_ref = 0 ), ( ( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ref ),
#line 579 "../../src/simpl.c"
((unsigned char )45 ), _au4_ee , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = _au3_ctor ), ((_au0__Xthis__ctor_ref ))) )
#line 579 "../../src/simpl.c"
) ;
_au4_ee = (struct expr *)( (_au0__Xthis__ctor_call = 0 ), ( ( (_au0__Xthis__ctor_call = 0 ), (_au0__Xthis__ctor_call = (struct call *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_call ),
#line 580 "../../src/simpl.c"
(unsigned char )109 , _au4_ee , ((struct expr *)0 )) )) , ((_au0__Xthis__ctor_call ))) ) ;
_au4_ee -> _expr__O5.__C5_fct_name = _au3_ctor ;
_au4_ee -> _node_base = 146 ;
_au4_ee = _expr_typ ( _au4_ee , _au2_tbl ) ;
}
else if (( _table_look ( _au3_cl -> _classdef_memtbl , "_ctor", (unsigned char )0 ) ) ){ 
#line 586 "../../src/simpl.c"
{ 
#line 724 "../../src/simpl.c"
struct
#line 724 "../../src/simpl.c"
ea _au0__V18 ;

#line 724 "../../src/simpl.c"
struct ea _au0__V19 ;

#line 586 "../../src/simpl.c"
error ( (char *)"M%n needsIr (no defaultK forC %s)", (struct ea *)( ( ((& _au0__V18 )-> _ea__O1.__C1_p = ((char *)_au2_m )), (((& _au0__V18 )))) )
#line 586 "../../src/simpl.c"
, (struct ea *)( ( ((& _au0__V19 )-> _ea__O1.__C1_p = ((char *)_au3_cl -> _classdef_string )), (((& _au0__V19 )))) ) ,
#line 586 "../../src/simpl.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}

#line 590 "../../src/simpl.c"
if (_au4_ee ){ 
#line 591 "../../src/simpl.c"
_expr_simpl ( _au4_ee ) ;
if (init_list )
#line 593 "../../src/simpl.c"
init_list = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )71 , init_list , _au4_ee ) ;
else 
#line 595 "../../src/simpl.c"
init_list = _au4_ee ;
}
}
else if (cl_obj_vec ){ 
#line 599 "../../src/simpl.c"
_au3_cl = (((struct classdef *)cl_obj_vec -> _expr__O2.__C2_tp ));
if (_au3_ctor = _classdef_has_ictor ( _au3_cl ) ){ 
#line 602 "../../src/simpl.c"
Pexpr _au5_mm ;

#line 604 "../../src/simpl.c"
Pexpr _au5_ee ;

#line 605 "../../src/simpl.c"
struct ref *_au0__Xthis__ctor_ref ;

#line 602 "../../src/simpl.c"
_au5_mm = (struct expr *)( (_au0__Xthis__ctor_ref = 0 ), ( ( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ref ),
#line 602 "../../src/simpl.c"
((unsigned char )44 ), _au1_th , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = _au2_m ), ((_au0__Xthis__ctor_ref ))) )
#line 602 "../../src/simpl.c"
) ;
_au5_mm -> _expr__O2.__C2_tp = _au2_m -> _expr__O2.__C2_tp ;
_au5_ee = cdvec ( vec_new_fct , _au5_mm , _au3_cl , _au3_ctor , -1) ;
_expr_simpl ( _au5_ee ) ;
if (init_list )
#line 607 "../../src/simpl.c"
init_list = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )71 , init_list , _au5_ee ) ;
else 
#line 609 "../../src/simpl.c"
init_list = _au5_ee ;
}
else if (( _table_look ( _au3_cl -> _classdef_memtbl , "_ctor", (unsigned char )0 ) ) ){ 
#line 612 "../../src/simpl.c"
{ 
#line 724 "../../src/simpl.c"
struct
#line 724 "../../src/simpl.c"
ea _au0__V20 ;

#line 724 "../../src/simpl.c"
struct ea _au0__V21 ;

#line 612 "../../src/simpl.c"
error ( (char *)"M%n[] needsIr (no defaultK forC %s)", (struct ea *)( ( ((& _au0__V20 )-> _ea__O1.__C1_p = ((char *)_au2_m )), (((& _au0__V20 )))) )
#line 612 "../../src/simpl.c"
, (struct ea *)( ( ((& _au0__V21 )-> _ea__O1.__C1_p = ((char *)_au3_cl -> _classdef_string )), (((& _au0__V21 )))) ) ,
#line 612 "../../src/simpl.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}
else if (_au2_m -> _expr__O4.__C4_n_initializer ){ 
#line 618 "../../src/simpl.c"
if (init_list )
#line 619 "../../src/simpl.c"
init_list = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )71 ,
#line 619 "../../src/simpl.c"
init_list , _au2_m -> _expr__O4.__C4_n_initializer ) ;
else 
#line 621 "../../src/simpl.c"
init_list = _au2_m -> _expr__O4.__C4_n_initializer ;
_au2_m -> _expr__O4.__C4_n_initializer = 0 ;
}
else if (_type_is_ref ( _au3_t ) ){ 
#line 625 "../../src/simpl.c"
{ 
#line 724 "../../src/simpl.c"
struct ea _au0__V22 ;

#line 625 "../../src/simpl.c"
error ( (char *)"RM%n needsIr", (struct ea *)( ( ((& _au0__V22 )-> _ea__O1.__C1_p = ((char *)_au2_m )), (((& _au0__V22 )))) )
#line 625 "../../src/simpl.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
else if (_type_tconst ( _au3_t ) && (vec_const == 0 )){ 
#line 628 "../../src/simpl.c"
{ 
#line 724 "../../src/simpl.c"
struct ea _au0__V23 ;

#line 628 "../../src/simpl.c"
error ( (char *)"constM%n needsIr", (struct ea *)( ( ((& _au0__V23 )-> _ea__O1.__C1_p = ((char *)_au2_m )), (((& _au0__V23 )))) )
#line 628 "../../src/simpl.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}
}

#line 633 "../../src/simpl.c"
no_of_returns = 0 ;
_au1_tail = _block_simpl ( _au0_this -> _fct_body ) ;

#line 636 "../../src/simpl.c"
if ((_au0_this -> _fct_returns -> _node_base != 38 )|| _au0_this -> _fct_f_result ){ 
#line 637 "../../src/simpl.c"
if (no_of_returns ){ 
#line 638 "../../src/simpl.c"
Pstmt _au3_tt ;

#line 638 "../../src/simpl.c"
_au3_tt = (((_au1_tail -> _node_base == 28 )|| (_au1_tail -> _node_base == 115 ))? _au1_tail : trim_tail ( _au1_tail ) );

#line 640 "../../src/simpl.c"
switch (_au3_tt -> _node_base ){ 
#line 641 "../../src/simpl.c"
case 28 : 
#line 642 "../../src/simpl.c"
case 19 : 
#line 643 "../../src/simpl.c"
del_list = 0 ;
break ;
case 72 : 
#line 646 "../../src/simpl.c"
switch (_au3_tt -> _stmt__O8.__C8_e -> _node_base ){ 
#line 647 "../../src/simpl.c"
case 168 : 
#line 648 "../../src/simpl.c"
case 146 : 
#line 649 "../../src/simpl.c"
goto chicken ;
}

#line 650 "../../src/simpl.c"
;
default : 
#line 652 "../../src/simpl.c"
if (strcmp ( (char *)curr_fct -> _expr__O3.__C3_string , (char *)"main") )
#line 653 "../../src/simpl.c"
{ 
#line 724 "../../src/simpl.c"
struct ea _au0__V24 ;

#line 653 "../../src/simpl.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"maybe no value returned from%n", (struct ea *)( ( ((& _au0__V24 )-> _ea__O1.__C1_p = ((char *)curr_fct )), (((&
#line 653 "../../src/simpl.c"
_au0__V24 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} case 20 : 
#line 655 "../../src/simpl.c"
case 33 : 
#line 656 "../../src/simpl.c"
case 10 : 
#line 657 "../../src/simpl.c"
case 39 : 
#line 658 "../../src/simpl.c"
case 16 : 
#line 659 "../../src/simpl.c"
case 115 : 
#line 660 "../../src/simpl.c"
chicken :
#line 661 "../../src/simpl.c"
break
#line 661 "../../src/simpl.c"
;
}
}
else { 
#line 665 "../../src/simpl.c"
if (strcmp ( (char *)curr_fct -> _expr__O3.__C3_string , (char *)"main") )
#line 666 "../../src/simpl.c"
{ 
#line 724 "../../src/simpl.c"
struct ea _au0__V25 ;

#line 666 "../../src/simpl.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"no value returned from%n", (struct ea *)( ( ((& _au0__V25 )-> _ea__O1.__C1_p = ((char *)curr_fct )), (((&
#line 666 "../../src/simpl.c"
_au0__V25 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
if (del_list )goto zaq ;
}
else if (del_list ){ 
#line 671 "../../src/simpl.c"
zaq :
#line 672 "../../src/simpl.c"
if (_au1_tail )
#line 673 "../../src/simpl.c"
_au1_tail -> _stmt_s_list = del_list ;
else 
#line 675 "../../src/simpl.c"
_au0_this -> _fct_body -> _stmt_s = del_list ;
_au1_tail = _au1_dtail ;
}

#line 679 "../../src/simpl.c"
if (curr_fct -> _name_n_oper == 162 ){ 
#line 681 "../../src/simpl.c"
_au0_this -> _fct_body -> _stmt_s = (struct stmt *)( (_au0__Xthis__ctor_ifstmt = 0 ), ( ( (_au0__Xthis__ctor_ifstmt =
#line 681 "../../src/simpl.c"
0 ), (_au0__Xthis__ctor_ifstmt = (struct ifstmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_ifstmt ), (unsigned char )20 , _au0_this -> _fct_body -> _stmt_where , _au0_this -> _fct_body ->
#line 681 "../../src/simpl.c"
_stmt_s ) )) , ( (_au0__Xthis__ctor_ifstmt -> _stmt__O8.__C8_e = _au1_th ), ( (_au0__Xthis__ctor_ifstmt -> _stmt__O9.__C9_else_stmt = ((struct stmt *)0 )), ((_au0__Xthis__ctor_ifstmt ))) )
#line 681 "../../src/simpl.c"
) ) ;
}

#line 684 "../../src/simpl.c"
if (curr_fct -> _name_n_oper == 161 ){ 
#line 686 "../../src/simpl.c"
if ((((struct name *)_au1_th ))-> _name_n_assigned_to == 0 ){ 
#line 690 "../../src/simpl.c"
(((struct name *)_au1_th ))-> _name_n_assigned_to = (_au1_ass_count ? _au1_ass_count :
#line 690 "../../src/simpl.c"
111 );
{ Pexpr _au3_sz ;

#line 692 "../../src/simpl.c"
struct texpr *_au0__Xthis__ctor_texpr ;

#line 691 "../../src/simpl.c"
_au3_sz = (struct expr *)( (_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr = (struct texpr *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_texpr ),
#line 691 "../../src/simpl.c"
((unsigned char )30 ), ((struct expr *)0 ), (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 = ((struct type *)_au1_cl )),
#line 691 "../../src/simpl.c"
((_au0__Xthis__ctor_texpr ))) ) ) ;
_au3_sz -> _expr__O2.__C2_tp = (struct type *)uint_type ;
{ Pexpr _au3_ee ;

#line 694 "../../src/simpl.c"
struct call *_au0__Xthis__ctor_call ;

#line 693 "../../src/simpl.c"
_au3_ee = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , _au3_sz , (struct expr *)0 ) ;
_au3_ee = (struct expr *)( (_au0__Xthis__ctor_call = 0 ), ( ( (_au0__Xthis__ctor_call = 0 ), (_au0__Xthis__ctor_call = (struct call *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_call ),
#line 694 "../../src/simpl.c"
(unsigned char )109 , ((struct expr *)new_fct ), _au3_ee ) )) , ((_au0__Xthis__ctor_call ))) ) ;
_au3_ee -> _expr__O5.__C5_fct_name = new_fct ;
_au3_ee -> _node_base = 146 ;
_expr_simpl ( _au3_ee ) ;
_au3_ee = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )70 , _au1_th , _au3_ee ) ;
{ Pstmt _au3_es ;

#line 700 "../../src/simpl.c"
struct estmt *_au0__Xthis__ctor_estmt ;

#line 699 "../../src/simpl.c"
_au3_es = (struct stmt *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt = (struct estmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_estmt ),
#line 699 "../../src/simpl.c"
((unsigned char )72 ), _au0_this -> _fct_body -> _stmt_where , ((struct stmt *)0 )) )) , ( (_au0__Xthis__ctor_estmt -> _stmt__O8.__C8_e = _au3_ee ),
#line 699 "../../src/simpl.c"
((_au0__Xthis__ctor_estmt ))) ) ) ;
_au3_ee = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )62 , _au1_th , zero ) ;
{ struct ifstmt *_au3_ifs ;

#line 702 "../../src/simpl.c"
struct estmt *_au0__Xthis__ctor_estmt ;

#line 702 "../../src/simpl.c"
struct ifstmt *_au0__Xthis__ctor_ifstmt ;

#line 701 "../../src/simpl.c"
_au3_ifs = (struct ifstmt *)( (_au0__Xthis__ctor_ifstmt = 0 ), ( ( (_au0__Xthis__ctor_ifstmt = 0 ), (_au0__Xthis__ctor_ifstmt = (struct ifstmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_ifstmt ),
#line 701 "../../src/simpl.c"
(unsigned char )20 , _au0_this -> _fct_body -> _stmt_where , _au3_es ) )) , ( (_au0__Xthis__ctor_ifstmt -> _stmt__O8.__C8_e = _au3_ee ), (
#line 701 "../../src/simpl.c"
(_au0__Xthis__ctor_ifstmt -> _stmt__O9.__C9_else_stmt = ((struct stmt *)0 )), ((_au0__Xthis__ctor_ifstmt ))) ) ) ) ;

#line 706 "../../src/simpl.c"
if (init_list ){ 
#line 707 "../../src/simpl.c"
_au3_es = (struct stmt *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt = (struct estmt *)_stmt__ctor (
#line 707 "../../src/simpl.c"
((struct stmt *)_au0__Xthis__ctor_estmt ), ((unsigned char )72 ), _au0_this -> _fct_body -> _stmt_where , ((struct stmt *)0 )) )) , ( (_au0__Xthis__ctor_estmt ->
#line 707 "../../src/simpl.c"
_stmt__O8.__C8_e = init_list ), ((_au0__Xthis__ctor_estmt ))) ) ) ;
_au3_es -> _stmt_s_list = _au0_this -> _fct_body -> _stmt_s ;
_au0_this -> _fct_body -> _stmt_s = _au3_es ;
if (_au1_tail == 0 )_au1_tail = _au3_es ;
}
_au3_ifs -> _stmt_s_list = _au0_this -> _fct_body -> _stmt_s ;
_au0_this -> _fct_body -> _stmt_s = (struct stmt *)_au3_ifs ;
if (_au1_tail == 0 )_au1_tail = (struct stmt *)_au3_ifs ;
}
}
}
}
}

#line 717 "../../src/simpl.c"
{ Pstmt _au2_st ;

#line 718 "../../src/simpl.c"
struct estmt *_au0__Xthis__ctor_estmt ;

#line 717 "../../src/simpl.c"
_au2_st = (struct stmt *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt = (struct estmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_estmt ),
#line 717 "../../src/simpl.c"
((unsigned char )28 ), curloc , ((struct stmt *)0 )) )) , ( (_au0__Xthis__ctor_estmt -> _stmt__O8.__C8_e = _au1_th ), ((_au0__Xthis__ctor_estmt ))) )
#line 717 "../../src/simpl.c"
) ;
if (_au1_tail )
#line 719 "../../src/simpl.c"
_au1_tail -> _stmt_s_list = _au2_st ;
else 
#line 721 "../../src/simpl.c"
_au0_this -> _fct_body -> _stmt_s = _au2_st ;
_au1_tail = _au2_st ;
}
}
}
;

#line 726 "../../src/simpl.c"
Pstmt _block_simpl (_au0_this )
#line 700 "../../src/cfront.h"
struct block *_au0_this ;

#line 727 "../../src/simpl.c"
{ 
#line 728 "../../src/simpl.c"
int _au1_i ;
Pname _au1_n ;
Pstmt _au1_ss ;

#line 730 "../../src/simpl.c"
Pstmt _au1_sst ;
Pstmt _au1_dd ;

#line 731 "../../src/simpl.c"
Pstmt _au1_ddt ;
Pstmt _au1_stail ;
Ptable _au1_old_scope ;

#line 730 "../../src/simpl.c"
_au1_ss = 0 ;

#line 730 "../../src/simpl.c"
_au1_sst = 0 ;
_au1_dd = 0 ;

#line 731 "../../src/simpl.c"
_au1_ddt = 0 ;

#line 733 "../../src/simpl.c"
_au1_old_scope = scope ;

#line 735 "../../src/simpl.c"
if (_au0_this -> _stmt__O8.__C8_own_tbl == 0 ){ 
#line 736 "../../src/simpl.c"
_au1_ss = (_au0_this -> _stmt_s ? _stmt_simpl ( _au0_this -> _stmt_s ) : (((struct stmt *)0 )));
return _au1_ss ;
}

#line 740 "../../src/simpl.c"
scope = _au0_this -> _stmt_memtbl ;
if (scope -> _table_init_stat == 0 )scope -> _table_init_stat = 1 ;

#line 743 "../../src/simpl.c"
for(_au1_n = _table_get_mem ( scope , _au1_i = 1 ) ;_au1_n ;_au1_n = _table_get_mem ( scope , ++ _au1_i ) ) { 
#line 744 "../../src/simpl.c"
Pstmt _au2_st ;
Pname _au2_cln ;
Pexpr _au2_in ;

#line 744 "../../src/simpl.c"
_au2_st = 0 ;

#line 746 "../../src/simpl.c"
_au2_in = _au1_n -> _expr__O4.__C4_n_initializer ;

#line 748 "../../src/simpl.c"
if (_au2_in )scope -> _table_init_stat = 2 ;

#line 750 "../../src/simpl.c"
switch (_au1_n -> _name_n_scope ){ 
#line 751 "../../src/simpl.c"
case 136 : 
#line 752 "../../src/simpl.c"
case 0 : 
#line 753 "../../src/simpl.c"
case 25 : 
#line 754 "../../src/simpl.c"
continue ;
}

#line 757 "../../src/simpl.c"
if (_au1_n -> _name_n_stclass == 31 )continue ;

#line 759 "../../src/simpl.c"
if (_au2_in && (_au2_in -> _node_base == 124 ))
#line 760 "../../src/simpl.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"initialization of automatic aggregates", (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 760 "../../src/simpl.c"
ea *)ea0 , (struct ea *)ea0 ) ;

#line 762 "../../src/simpl.c"
if (_au1_n -> _expr__O2.__C2_tp == 0 )continue ;

#line 764 "../../src/simpl.c"
if (_au1_n -> _name_n_evaluated )continue ;

#line 767 "../../src/simpl.c"
{ char *_au3_s ;
register char _au3_c3 ;

#line 767 "../../src/simpl.c"
_au3_s = _au1_n -> _expr__O3.__C3_string ;
_au3_c3 = (_au3_s [3 ]);
if ((((_au3_s [0 ])== '_' )&& ((_au3_s [1 ])== 'D' ))&& isdigit(_au3_c3))continue ;
}
if (_au2_cln = _type_is_cl_obj ( _au1_n -> _expr__O2.__C2_tp ) ){ 
#line 772 "../../src/simpl.c"
Pclass _au3_cl ;
Pname _au3_d ;

#line 774 "../../src/simpl.c"
struct estmt *_au0__Xthis__ctor_estmt ;

#line 772 "../../src/simpl.c"
_au3_cl = (((struct classdef *)_au2_cln -> _expr__O2.__C2_tp ));
_au3_d = ( _table_look ( _au3_cl -> _classdef_memtbl , "_dtor", (unsigned char )0 ) ) ;

#line 775 "../../src/simpl.c"
if (_au3_d ){ 
#line 776 "../../src/simpl.c"
Pref _au4_r ;
Pexpr _au4_ee ;
Pcall _au4_dl ;
Pstmt _au4_dls ;

#line 780 "../../src/simpl.c"
struct ref *_au0__Xthis__ctor_ref ;

#line 780 "../../src/simpl.c"
struct call *_au0__Xthis__ctor_call ;

#line 780 "../../src/simpl.c"
struct estmt *_au0__Xthis__ctor_estmt ;

#line 776 "../../src/simpl.c"
_au4_r = (struct ref *)( (_au0__Xthis__ctor_ref = 0 ), ( ( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ref ),
#line 776 "../../src/simpl.c"
((unsigned char )45 ), ((struct expr *)_au1_n ), (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = _au3_d ), ((_au0__Xthis__ctor_ref )))
#line 776 "../../src/simpl.c"
) ) ;
_au4_ee = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , zero , (struct expr *)0 ) ;
_au4_dl = (struct call *)( (_au0__Xthis__ctor_call = 0 ), ( ( (_au0__Xthis__ctor_call = 0 ), (_au0__Xthis__ctor_call = (struct call *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_call ),
#line 778 "../../src/simpl.c"
(unsigned char )109 , ((struct expr *)_au4_r ), _au4_ee ) )) , ((_au0__Xthis__ctor_call ))) ) ;
_au4_dls = (struct stmt *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt = (struct estmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_estmt ),
#line 779 "../../src/simpl.c"
((unsigned char )72 ), _au1_n -> _name_where , ((struct stmt *)0 )) )) , ( (_au0__Xthis__ctor_estmt -> _stmt__O8.__C8_e = ((struct expr *)_au4_dl )),
#line 779 "../../src/simpl.c"
((_au0__Xthis__ctor_estmt ))) ) ) ;
_au4_dl -> _node_base = 146 ;
_au4_dl -> _expr__O5.__C5_fct_name = _au3_d ;

#line 790 "../../src/simpl.c"
if (_au1_dd ){ 
#line 791 "../../src/simpl.c"
_au4_dls -> _stmt_s_list = _au1_dd ;
_au1_dd = _au4_dls ;
}
else 
#line 795 "../../src/simpl.c"
_au1_ddt = (_au1_dd = _au4_dls );
}

#line 798 "../../src/simpl.c"
if (_au2_in ){ 
#line 799 "../../src/simpl.c"
switch (_au2_in -> _node_base ){ 
#line 800 "../../src/simpl.c"
case 111 : 
#line 801 "../../src/simpl.c"
if (_au2_in -> _expr__O3.__C3_e1 -> _node_base == 146 ){ 
#line 802 "../../src/simpl.c"
Pname _au6_fn ;

#line 803 "../../src/simpl.c"
struct estmt *_au0__Xthis__ctor_estmt ;

#line 802 "../../src/simpl.c"
_au6_fn = _au2_in -> _expr__O3.__C3_e1 -> _expr__O5.__C5_fct_name ;
if ((_au6_fn == 0 )|| (_au6_fn -> _name_n_oper != 161 ))goto ddd ;
_au2_st = (struct stmt *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt = (struct estmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_estmt ),
#line 804 "../../src/simpl.c"
((unsigned char )72 ), _au1_n -> _name_where , ((struct stmt *)0 )) )) , ( (_au0__Xthis__ctor_estmt -> _stmt__O8.__C8_e = _au2_in -> _expr__O3.__C3_e1 ),
#line 804 "../../src/simpl.c"
((_au0__Xthis__ctor_estmt ))) ) ) ;
_au1_n -> _expr__O4.__C4_n_initializer = 0 ;
break ;
}
goto ddd ;
case 147 : 
#line 810 "../../src/simpl.c"
_au2_st = (struct stmt *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt = (struct estmt *)_stmt__ctor (
#line 810 "../../src/simpl.c"
((struct stmt *)_au0__Xthis__ctor_estmt ), ((unsigned char )72 ), _au1_n -> _name_where , ((struct stmt *)0 )) )) , ( (_au0__Xthis__ctor_estmt -> _stmt__O8.__C8_e =
#line 810 "../../src/simpl.c"
_au2_in -> _expr__O3.__C3_e1 ), ((_au0__Xthis__ctor_estmt ))) ) ) ;
_au1_n -> _expr__O4.__C4_n_initializer = 0 ;
break ;
case 70 : 
#line 814 "../../src/simpl.c"
if (_au2_in -> _expr__O3.__C3_e1 == (struct expr *)_au1_n ){ 
#line 815 "../../src/simpl.c"
_au2_st = (struct stmt *)( (_au0__Xthis__ctor_estmt = 0 ), ( (
#line 815 "../../src/simpl.c"
(_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt = (struct estmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_estmt ), ((unsigned char )72 ), _au1_n -> _name_where , ((struct stmt *)0 ))
#line 815 "../../src/simpl.c"
)) , ( (_au0__Xthis__ctor_estmt -> _stmt__O8.__C8_e = _au2_in ), ((_au0__Xthis__ctor_estmt ))) ) ) ;
_au1_n -> _expr__O4.__C4_n_initializer = 0 ;
break ;
}
default : 
#line 820 "../../src/simpl.c"
goto ddd ;
}
}
}
else if (cl_obj_vec ){ 
#line 825 "../../src/simpl.c"
Pclass _au3_cl ;
Pname _au3_d ;
Pname _au3_c ;

#line 825 "../../src/simpl.c"
_au3_cl = (((struct classdef *)cl_obj_vec -> _expr__O2.__C2_tp ));
_au3_d = ( _table_look ( _au3_cl -> _classdef_memtbl , "_dtor", (unsigned char )0 ) ) ;
_au3_c = _classdef_has_ictor ( _au3_cl ) ;

#line 829 "../../src/simpl.c"
_au1_n -> _expr__O4.__C4_n_initializer = 0 ;

#line 831 "../../src/simpl.c"
if (_au3_c ){ 
#line 832 "../../src/simpl.c"
Pexpr _au4_a ;

#line 833 "../../src/simpl.c"
struct estmt *_au0__Xthis__ctor_estmt ;

#line 832 "../../src/simpl.c"
_au4_a = cdvec ( vec_new_fct , (struct expr *)_au1_n , _au3_cl , _au3_c , -1) ;
_au2_st = (struct stmt *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt = (struct estmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_estmt ),
#line 833 "../../src/simpl.c"
((unsigned char )72 ), _au1_n -> _name_where , ((struct stmt *)0 )) )) , ( (_au0__Xthis__ctor_estmt -> _stmt__O8.__C8_e = _au4_a ), ((_au0__Xthis__ctor_estmt )))
#line 833 "../../src/simpl.c"
) ) ;
}

#line 836 "../../src/simpl.c"
if (_au3_d ){ 
#line 837 "../../src/simpl.c"
Pexpr _au4_a ;
Pstmt _au4_dls ;

#line 839 "../../src/simpl.c"
struct estmt *_au0__Xthis__ctor_estmt ;

#line 837 "../../src/simpl.c"
_au4_a = cdvec ( vec_del_fct , (struct expr *)_au1_n , _au3_cl , _au3_d , (int )0 ) ;
_au4_dls = (struct stmt *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt = (struct estmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_estmt ),
#line 838 "../../src/simpl.c"
((unsigned char )72 ), _au1_n -> _name_where , ((struct stmt *)0 )) )) , ( (_au0__Xthis__ctor_estmt -> _stmt__O8.__C8_e = _au4_a ), ((_au0__Xthis__ctor_estmt )))
#line 838 "../../src/simpl.c"
) ) ;

#line 845 "../../src/simpl.c"
if (_au1_dd ){ 
#line 846 "../../src/simpl.c"
_au4_dls -> _stmt_s_list = _au1_dd ;
_au1_dd = _au4_dls ;
}
else 
#line 850 "../../src/simpl.c"
_au1_ddt = (_au1_dd = _au4_dls );
}
}
else 
#line 854 "../../src/simpl.c"
if (_au2_in ){ 
#line 855 "../../src/simpl.c"
switch (_au2_in -> _node_base ){ 
#line 856 "../../src/simpl.c"
case 124 : 
#line 857 "../../src/simpl.c"
switch (_au1_n -> _name_n_scope ){ 
#line 858 "../../src/simpl.c"
case 108 : 
#line 859 "../../src/simpl.c"
case
#line 859 "../../src/simpl.c"
136 : 
#line 860 "../../src/simpl.c"
{ 
#line 927 "../../src/simpl.c"
struct ea _au0__V26 ;

#line 860 "../../src/simpl.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"Ir list for localV%n", (struct ea *)( ( ((& _au0__V26 )-> _ea__O1.__C1_p = ((char *)_au1_n )), (((&
#line 860 "../../src/simpl.c"
_au0__V26 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
break ;
case 81 : 
#line 864 "../../src/simpl.c"
if (_au1_n -> _expr__O2.__C2_tp -> _node_base == 110 )break ;
default : 
#line 866 "../../src/simpl.c"
ddd :
#line 867 "../../src/simpl.c"
{ Pexpr _au5_ee ;

#line 868 "../../src/simpl.c"
struct estmt *_au0__Xthis__ctor_estmt ;

#line 867 "../../src/simpl.c"
_au5_ee = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )70 , (struct expr *)_au1_n , _au2_in ) ;
_au2_st = (struct stmt *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt = (struct estmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_estmt ),
#line 868 "../../src/simpl.c"
((unsigned char )72 ), _au1_n -> _name_where , ((struct stmt *)0 )) )) , ( (_au0__Xthis__ctor_estmt -> _stmt__O8.__C8_e = _au5_ee ), ((_au0__Xthis__ctor_estmt )))
#line 868 "../../src/simpl.c"
) ) ;
_au1_n -> _expr__O4.__C4_n_initializer = 0 ;
}
}
}

#line 874 "../../src/simpl.c"
if (_au2_st ){ 
#line 875 "../../src/simpl.c"
if (_au1_ss )
#line 876 "../../src/simpl.c"
_au1_sst -> _stmt_s_list = _au2_st ;
else 
#line 878 "../../src/simpl.c"
_au1_ss = _au2_st ;
_au1_sst = _au2_st ;
}
}

#line 883 "../../src/simpl.c"
if (_au1_dd ){ 
#line 884 "../../src/simpl.c"
Pstmt _au2_od ;
Pstmt _au2_obd ;
Pstmt _au2_ocd ;

#line 887 "../../src/simpl.c"
struct pair *_au0__Xthis__ctor_pair ;

#line 884 "../../src/simpl.c"
_au2_od = del_list ;
_au2_obd = break_del_list ;
_au2_ocd = continue_del_list ;

#line 888 "../../src/simpl.c"
_stmt_simpl ( _au1_dd ) ;
del_list = (_au2_od ? (((struct stmt *)( (_au0__Xthis__ctor_pair = 0 ), ( ( (_au0__Xthis__ctor_pair = 0 ), (_au0__Xthis__ctor_pair = (struct pair *)_stmt__ctor ( ((struct
#line 889 "../../src/simpl.c"
stmt *)_au0__Xthis__ctor_pair ), (unsigned char )166 , curloc , _au1_dd ) )) , ( (_au0__Xthis__ctor_pair -> _stmt__O8.__C8_s2 = _au2_od ), ((_au0__Xthis__ctor_pair ))) )
#line 889 "../../src/simpl.c"
) )): _au1_dd );
break_del_list = ((break_del_list && _au2_obd )? (((struct stmt *)( (_au0__Xthis__ctor_pair = 0 ), ( ( (_au0__Xthis__ctor_pair = 0 ), (_au0__Xthis__ctor_pair = (struct pair *)_stmt__ctor (
#line 890 "../../src/simpl.c"
((struct stmt *)_au0__Xthis__ctor_pair ), (unsigned char )166 , curloc , _au1_dd ) )) , ( (_au0__Xthis__ctor_pair -> _stmt__O8.__C8_s2 = _au2_obd ), ((_au0__Xthis__ctor_pair )))
#line 890 "../../src/simpl.c"
) ) )): _au1_dd );
continue_del_list = ((continue_del_list && _au2_ocd )? (((struct stmt *)( (_au0__Xthis__ctor_pair = 0 ), ( ( (_au0__Xthis__ctor_pair = 0 ), (_au0__Xthis__ctor_pair = (struct pair *)_stmt__ctor (
#line 891 "../../src/simpl.c"
((struct stmt *)_au0__Xthis__ctor_pair ), (unsigned char )166 , curloc , _au1_dd ) )) , ( (_au0__Xthis__ctor_pair -> _stmt__O8.__C8_s2 = _au2_ocd ), ((_au0__Xthis__ctor_pair )))
#line 891 "../../src/simpl.c"
) ) )): _au1_dd );

#line 893 "../../src/simpl.c"
_au1_stail = (_au0_this -> _stmt_s ? _stmt_simpl ( _au0_this -> _stmt_s ) : (((struct stmt *)0 )));

#line 895 "../../src/simpl.c"
{ Pfct _au2_f ;

#line 895 "../../src/simpl.c"
_au2_f = (((struct fct *)curr_fct -> _expr__O2.__C2_tp ));
if ((((_au0_this != _au2_f -> _fct_body )|| (_au2_f -> _fct_returns -> _node_base == 38 ))|| ((_au2_f -> _fct_returns -> _node_base != 38 )&& (no_of_returns == 0 )))||
#line 896 "../../src/simpl.c"
(strcmp ( (char *)curr_fct -> _expr__O3.__C3_string , (char *)"main") == 0 ))
#line 899 "../../src/simpl.c"
{ 
#line 901 "../../src/simpl.c"
if (_au1_stail ){ 
#line 902 "../../src/simpl.c"
Pstmt _au4_tt ;

#line 902 "../../src/simpl.c"
_au4_tt = (((_au1_stail -> _node_base == 28 )|| (_au1_stail -> _node_base == 115 ))? _au1_stail : trim_tail ( _au1_stail ) );
if (_au4_tt -> _node_base != 28 )_au1_stail -> _stmt_s_list = _au1_dd ;
}
else 
#line 906 "../../src/simpl.c"
_au0_this -> _stmt_s = _au1_dd ;
_au1_stail = _au1_ddt ;
}

#line 910 "../../src/simpl.c"
del_list = _au2_od ;
continue_del_list = _au2_ocd ;
break_del_list = _au2_obd ;
}
}
else _au1_stail = (_au0_this -> _stmt_s ? _stmt_simpl ( _au0_this -> _stmt_s ) : (((struct stmt *)0 )));

#line 917 "../../src/simpl.c"
if (_au1_ss ){ 
#line 918 "../../src/simpl.c"
_stmt_simpl ( _au1_ss ) ;
_au1_sst -> _stmt_s_list = _au0_this -> _stmt_s ;
_au0_this -> _stmt_s = _au1_ss ;
if (_au1_stail == 0 )_au1_stail = _au1_sst ;
}

#line 924 "../../src/simpl.c"
scope = _au1_old_scope ;

#line 926 "../../src/simpl.c"
return _au1_stail ;
}
;

#line 930 "../../src/simpl.c"
char _classdef_simpl (_au0_this )
#line 319 "../../src/cfront.h"
struct classdef *_au0_this ;

#line 931 "../../src/simpl.c"
{ 
#line 932 "../../src/simpl.c"
int _au1_i ;
Pname _au1_m ;
Pclass _au1_oc ;

#line 945 "../../src/simpl.c"
Plist _au1_fl ;

#line 934 "../../src/simpl.c"
_au1_oc = _au0_this -> _classdef_in_class ;

#line 936 "../../src/simpl.c"
_au0_this -> _classdef_in_class = _au0_this ;
for(_au1_m = _table_get_mem ( _au0_this -> _classdef_memtbl , _au1_i = 1 ) ;_au1_m ;_au1_m = _table_get_mem ( _au0_this -> _classdef_memtbl , ++ _au1_i ) ) {
#line 937 "../../src/simpl.c"

#line 938 "../../src/simpl.c"
Pexpr _au2_i ;

#line 938 "../../src/simpl.c"
_au2_i = _au1_m -> _expr__O4.__C4_n_initializer ;
_au1_m -> _expr__O4.__C4_n_initializer = 0 ;
_name_simpl ( _au1_m ) ;
_au1_m -> _expr__O4.__C4_n_initializer = _au2_i ;
}
_au0_this -> _classdef_in_class = _au1_oc ;

#line 945 "../../src/simpl.c"
for(_au1_fl = _au0_this -> _classdef_friend_list ;_au1_fl ;_au1_fl = _au1_fl -> _name_list_l ) { 
#line 946 "../../src/simpl.c"
Pname _au2_p ;

#line 946 "../../src/simpl.c"
_au2_p = _au1_fl -> _name_list_f ;
switch (_au2_p -> _expr__O2.__C2_tp -> _node_base ){ 
#line 948 "../../src/simpl.c"
case 108 : 
#line 949 "../../src/simpl.c"
case 76 : 
#line 950 "../../src/simpl.c"
_name_simpl ( _au2_p ) ;
}
}
}
;
Ptype nstd_type ;
char _expr_simpl (_au0_this )
#line 525 "../../src/cfront.h"
struct expr *_au0_this ;

#line 957 "../../src/simpl.c"
{ 
#line 959 "../../src/simpl.c"
if ((_au0_this == 0 )|| (_au0_this -> _node_permanent == 2 ))return ;

#line 961 "../../src/simpl.c"
switch (_au0_this -> _node_base ){ 
#line 974 "../../src/simpl.c"
case 168 : 
#line 975 "../../src/simpl.c"
return ;
case 145 : 
#line 977 "../../src/simpl.c"
case 112 : 
#line 978 "../../src/simpl.c"
_expr_simpl ( _au0_this -> _expr__O4.__C4_e2 ) ;
switch (_au0_this -> _expr__O4.__C4_e2 -> _node_base ){ 
#line 980 "../../src/simpl.c"
case 45 : 
#line 981 "../../src/simpl.c"
case 44 : 
#line 982 "../../src/simpl.c"
{ Pref _au4_r ;
Pname _au4_m ;

#line 982 "../../src/simpl.c"
_au4_r = (((struct ref *)_au0_this -> _expr__O4.__C4_e2 ));
_au4_m = _au4_r -> _expr__O5.__C5_mem ;
if (_au4_m -> _name_n_stclass == 31 ){ 
#line 985 "../../src/simpl.c"
Pexpr _au5_x ;
delp :
#line 987 "../../src/simpl.c"
_au5_x = _au0_this -> _expr__O4.__C4_e2 ;
_au0_this -> _expr__O4.__C4_e2 = (struct expr *)_au4_m ;
_au4_r -> _expr__O5.__C5_mem = 0 ;
if (_au5_x && (_au5_x -> _node_permanent == 0 ))_expr_del ( _au5_x ) ;
}
else if (_au4_m -> _expr__O2.__C2_tp -> _node_base == 108 ){ 
#line 993 "../../src/simpl.c"
Pfct _au5_f ;

#line 993 "../../src/simpl.c"
_au5_f = (((struct fct *)_au4_m -> _expr__O2.__C2_tp ));
if (_au5_f -> _fct_f_virtual ){ 
#line 995 "../../src/simpl.c"
int _au6_index ;
Pexpr _au6_ie ;

#line 997 "../../src/simpl.c"
struct ival *_au0__Xthis__ctor_ival ;

#line 995 "../../src/simpl.c"
_au6_index = _au5_f -> _fct_f_virtual ;
_au6_ie = (struct expr *)((1 < _au6_index )? ( (_au0__Xthis__ctor_ival = 0 ), ( ( (_au0__Xthis__ctor_ival = 0 ), (_au0__Xthis__ctor_ival = (struct ival *)_expr__ctor (
#line 996 "../../src/simpl.c"
((struct expr *)_au0__Xthis__ctor_ival ), (unsigned char )150 , (struct expr *)0 , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ival -> _expr__O3.__C3_i1 =
#line 996 "../../src/simpl.c"
(_au6_index - 1 )), ((_au0__Xthis__ctor_ival ))) ) ) : (((struct ival *)0 )));
if (_au6_ie )_au6_ie -> _expr__O2.__C2_tp = (struct type *)int_type ;
{ Pname _au6_vp ;

#line 998 "../../src/simpl.c"
_au6_vp = _table_look ( _au4_m -> _expr__O5.__C5_n_table , "_vptr", (unsigned char )0 ) ;
_au4_r -> _expr__O5.__C5_mem = _au6_vp ;
_au0_this -> _node_base = 111 ;
_au0_this -> _expr__O3.__C3_e1 = _au0_this -> _expr__O4.__C4_e2 ;
_au0_this -> _expr__O4.__C4_e2 = _au6_ie ;
}
}
else 
#line 1004 "../../src/simpl.c"
{ 
#line 1005 "../../src/simpl.c"
goto delp ;
}
}
}
}
break ;

#line 1012 "../../src/simpl.c"
default : 
#line 1013 "../../src/simpl.c"
if (_au0_this -> _expr__O3.__C3_e1 )_expr_simpl ( _au0_this -> _expr__O3.__C3_e1 ) ;
if (_au0_this -> _expr__O4.__C4_e2 )_expr_simpl ( _au0_this -> _expr__O4.__C4_e2 ) ;
break ;

#line 1017 "../../src/simpl.c"
case 85 : 
#line 1018 "../../src/simpl.c"
case 144 : 
#line 1019 "../../src/simpl.c"
case 82 : 
#line 1020 "../../src/simpl.c"
case 83 : 
#line 1021 "../../src/simpl.c"
case 84 : 
#line 1022 "../../src/simpl.c"
case 150 : 
#line 1023 "../../src/simpl.c"
case 151 :
#line 1023 "../../src/simpl.c"

#line 1024 "../../src/simpl.c"
case 152 : 
#line 1025 "../../src/simpl.c"
case 81 : 
#line 1026 "../../src/simpl.c"
case 86 : 
#line 1027 "../../src/simpl.c"
case 124 : 
#line 1028 "../../src/simpl.c"
case 30 : 
#line 1029 "../../src/simpl.c"
return ;

#line 1037 "../../src/simpl.c"
case 146 : 
#line 1038 "../../src/simpl.c"
case 109 : 
#line 1039 "../../src/simpl.c"
_call_simpl ( ((struct call *)_au0_this )) ;
break ;

#line 1042 "../../src/simpl.c"
case 23 : 
#line 1043 "../../src/simpl.c"
_expr_simpl_new ( _au0_this ) ;
return ;

#line 1046 "../../src/simpl.c"
case 9 : 
#line 1047 "../../src/simpl.c"
_expr_simpl_delete ( _au0_this ) ;
break ;

#line 1050 "../../src/simpl.c"
case 68 : 
#line 1051 "../../src/simpl.c"
_expr_simpl ( _au0_this -> _expr__O5.__C5_cond ) ;
_expr_simpl ( _au0_this -> _expr__O4.__C4_e2 ) ;

#line 1054 "../../src/simpl.c"
case 113 : 
#line 1055 "../../src/simpl.c"
case 44 : 
#line 1056 "../../src/simpl.c"
_expr_simpl ( _au0_this -> _expr__O3.__C3_e1 ) ;
break ;

#line 1059 "../../src/simpl.c"
case 45 : 
#line 1060 "../../src/simpl.c"
_expr_simpl ( _au0_this -> _expr__O3.__C3_e1 ) ;
switch (_au0_this -> _expr__O3.__C3_e1 -> _node_base ){ 
#line 1062 "../../src/simpl.c"
case 71 : 
#line 1063 "../../src/simpl.c"
case 147 : 
#line 1064 "../../src/simpl.c"
{ 
#line 1065 "../../src/simpl.c"
Pexpr _au4_ex ;

#line 1065 "../../src/simpl.c"
_au4_ex = _au0_this -> _expr__O3.__C3_e1 ;
cfr :
#line 1067 "../../src/simpl.c"
switch (_au4_ex -> _expr__O4.__C4_e2 -> _node_base ){ 
#line 1068 "../../src/simpl.c"
case 85 : 
#line 1069 "../../src/simpl.c"
_au0_this -> _node_base = 44 ;
_au4_ex -> _expr__O4.__C4_e2 = _expr_address ( _au4_ex -> _expr__O4.__C4_e2 ) ;
break ;
case 71 : 
#line 1073 "../../src/simpl.c"
case 147 : 
#line 1074 "../../src/simpl.c"
_au4_ex = _au4_ex -> _expr__O4.__C4_e2 ;
goto cfr ;
}
}
}
break ;

#line 1081 "../../src/simpl.c"
case 70 : 
#line 1082 "../../src/simpl.c"
{ Pfct _au3_f ;
Pexpr _au3_th ;

#line 1082 "../../src/simpl.c"
_au3_f = (((struct fct *)curr_fct -> _expr__O2.__C2_tp ));
_au3_th = (struct expr *)_au3_f -> _fct_f_this ;

#line 1085 "../../src/simpl.c"
if (_au0_this -> _expr__O3.__C3_e1 )_expr_simpl ( _au0_this -> _expr__O3.__C3_e1 ) ;
if (_au0_this -> _expr__O4.__C4_e2 && (_au0_this -> _expr__O4.__C4_e2 -> _node_base == 70 ))nstd_type = _au0_this -> _expr__O4.__C4_e2 -> _expr__O2.__C2_tp ;
if (_au0_this -> _expr__O4.__C4_e2 )_expr_simpl ( _au0_this -> _expr__O4.__C4_e2 ) ;

#line 1089 "../../src/simpl.c"
if (((_au3_th && (_au3_th == _au0_this -> _expr__O3.__C3_e1 ))&& (curr_fct -> _name_n_oper == 161 ))&& init_list ){ 
#line 1091 "../../src/simpl.c"
_au0_this -> _node_base = 71 ;
_au0_this -> _expr__O3.__C3_e1 = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )70 , _au0_this -> _expr__O3.__C3_e1 , _au0_this -> _expr__O4.__C4_e2 ) ;
#line 1092 "../../src/simpl.c"

#line 1093 "../../src/simpl.c"
_au0_this -> _expr__O4.__C4_e2 = init_list ;

#line 1095 "../../src/simpl.c"
if (nstd_type && (nstd_type == _au3_th -> _expr__O2.__C2_tp )){ 
#line 1096 "../../src/simpl.c"
Pexpr _au5_ee ;

#line 1096 "../../src/simpl.c"
_au5_ee = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )71 , _au0_this -> _expr__O3.__C3_e1 , _au0_this -> _expr__O4.__C4_e2 ) ;
_au0_this -> _expr__O3.__C3_e1 = _au5_ee ;

#line 1097 "../../src/simpl.c"
_au0_this -> _expr__O4.__C4_e2 = _au3_th ;
}
}
if (nstd_type )nstd_type = 0 ;
break ;
}
}

#line 1105 "../../src/simpl.c"
if (_au0_this -> _expr__O2.__C2_tp == (struct type *)int_type ){ 
#line 1106 "../../src/simpl.c"
Neval = 0 ;
{ int _au2_i ;

#line 1107 "../../src/simpl.c"
_au2_i = _expr_eval ( _au0_this ) ;
if (Neval == 0 ){ 
#line 1109 "../../src/simpl.c"
_au0_this -> _node_base = 150 ;
_au0_this -> _expr__O3.__C3_i1 = _au2_i ;
}
}
}
}
;

#line 1117 "../../src/simpl.c"
char _call_simpl (_au0_this )
#line 541 "../../src/cfront.h"
struct call *_au0_this ;

#line 1126 "../../src/simpl.c"
{ 
#line 1127 "../../src/simpl.c"
Pname _au1_fn ;
Pfct _au1_f ;

#line 1320 "../../src/simpl.c"
Pexpr _au1_ee ;

#line 1127 "../../src/simpl.c"
_au1_fn = _au0_this -> _expr__O5.__C5_fct_name ;
_au1_f = (_au1_fn ? (((struct fct *)_au1_fn -> _expr__O2.__C2_tp )): (((struct fct *)0 )));

#line 1130 "../../src/simpl.c"
if (_au1_fn == 0 )_expr_simpl ( _au0_this -> _expr__O3.__C3_e1 ) ;

#line 1132 "../../src/simpl.c"
if (_au1_f ){ 
#line 1133 "../../src/simpl.c"
switch (_au1_f -> _node_base ){ 
#line 1134 "../../src/simpl.c"
case 141 : 
#line 1135 "../../src/simpl.c"
return ;
case 108 : 
#line 1137 "../../src/simpl.c"
break ;
case 76 : 
#line 1139 "../../src/simpl.c"
{ Pgen _au4_g ;

#line 1139 "../../src/simpl.c"
_au4_g = (((struct gen *)_au1_f ));
_au0_this -> _expr__O5.__C5_fct_name = (_au1_fn = _au4_g -> _gen_fct_list -> _name_list_f );
_au1_f = (((struct fct *)_au1_fn -> _expr__O2.__C2_tp ));
}
}
}

#line 1146 "../../src/simpl.c"
switch (_au0_this -> _expr__O3.__C3_e1 -> _node_base ){ 
#line 1147 "../../src/simpl.c"
case 173 : 
#line 1148 "../../src/simpl.c"
{ Pexpr _au3_p ;
Pexpr _au3_q ;
Pclass _au3_cl ;
Pfct _au3_f ;

#line 1148 "../../src/simpl.c"
_au3_p = _au0_this -> _expr__O3.__C3_e1 -> _expr__O3.__C3_e1 ;
_au3_q = _au0_this -> _expr__O3.__C3_e1 -> _expr__O4.__C4_e2 ;
_au3_cl = (((struct classdef *)_au0_this -> _expr__O3.__C3_e1 -> _expr__O5.__C5_tp2 ));
_au3_f = (((struct fct *)_type_deref ( _au3_q -> _expr__O2.__C2_tp ) ));

#line 1153 "../../src/simpl.c"
if (_au3_cl -> _classdef_virt_count == 0 ){ 
#line 1155 "../../src/simpl.c"
_au0_this -> _expr__O4.__C4_e2 = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , _au3_p ,
#line 1155 "../../src/simpl.c"
_au0_this -> _expr__O4.__C4_e2 ) ;
_au0_this -> _expr__O3.__C3_e1 = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )111 , _au3_q , (struct expr *)0 ) ;
_au0_this -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp = _type_deref ( _au3_q -> _expr__O2.__C2_tp ) ;
}
else { 
#line 1160 "../../src/simpl.c"
if (_au3_f -> _fct_f_this == 0 ){ 
#line 1161 "../../src/simpl.c"
if (_au3_f -> _fct_memof == 0 )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"memof missing",
#line 1161 "../../src/simpl.c"
(struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
{ Pname _au5_tt ;

#line 1162 "../../src/simpl.c"
_au5_tt = (struct name *)_name__ctor ( (struct name *)0 , "this") ;
_au5_tt -> _name_n_scope = 136 ;
_au5_tt -> _name_n_sto = 136 ;
_au5_tt -> _expr__O2.__C2_tp = _au3_f -> _fct_memof -> _classdef_this_type ;
_au5_tt -> _node_permanent = 1 ;
_au3_f -> _fct_f_this = _au5_tt ;
_au5_tt -> _name_n_list = (_au3_f -> _fct_f_result ? _au3_f -> _fct_f_result : _au3_f -> _fct_argtype );
}
}

#line 1175 "../../src/simpl.c"
if (_expr_not_simple ( _au3_q ) )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"second operand of .* too complicated", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ,
#line 1175 "../../src/simpl.c"
(struct ea *)ea0 ) ;
if (_expr_not_simple ( _au3_p ) )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"first operand of .* too complicated", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ,
#line 1176 "../../src/simpl.c"
(struct ea *)ea0 ) ;

#line 1178 "../../src/simpl.c"
{ Pexpr _au4_c ;
Pexpr _au4_pp ;

#line 1188 "../../src/simpl.c"
Pexpr _au4_ie ;

#line 1190 "../../src/simpl.c"
Pname _au4_vp ;
Pexpr _au4_vc ;

#line 1192 "../../src/simpl.c"
struct texpr *_au0__Xthis__ctor_texpr ;

#line 1192 "../../src/simpl.c"
struct ref *_au0__Xthis__ctor_ref ;

#line 1192 "../../src/simpl.c"
struct ival *_au0__Xthis__ctor_ival ;

#line 1178 "../../src/simpl.c"
_au4_c = (struct expr *)( (_au0__Xthis__ctor_ival = 0 ), ( ( (_au0__Xthis__ctor_ival = 0 ), (_au0__Xthis__ctor_ival = (struct ival *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ival ),
#line 1178 "../../src/simpl.c"
(unsigned char )150 , (struct expr *)0 , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ival -> _expr__O3.__C3_i1 = 127 ), ((_au0__Xthis__ctor_ival )))
#line 1178 "../../src/simpl.c"
) ) ;
_au4_pp = (struct expr *)( (_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr = (struct texpr *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_texpr ),
#line 1179 "../../src/simpl.c"
((unsigned char )113 ), _au3_q , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 = ((struct type *)((SZ_INT == SZ_WPTR )?
#line 1179 "../../src/simpl.c"
int_type : long_type ))), ((_au0__Xthis__ctor_texpr ))) ) ) ;
_au4_c = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )47 , (struct expr *)0 , _au4_c ) ;
_au4_c = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )52 , _au4_pp , _au4_c ) ;

#line 1188 "../../src/simpl.c"
_au4_ie = (struct expr *)( (_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr = (struct texpr *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_texpr ),
#line 1188 "../../src/simpl.c"
((unsigned char )113 ), _au3_q , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 = ((struct type *)int_type )), ((_au0__Xthis__ctor_texpr )))
#line 1188 "../../src/simpl.c"
) ) ;
_au4_ie = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )55 , _au4_ie , one ) ;
_au4_vp = _table_look ( _au3_cl -> _classdef_memtbl , "_vptr", (unsigned char )0 ) ;

#line 1190 "../../src/simpl.c"
_au4_vc = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )111 , (struct expr *)( (_au0__Xthis__ctor_ref = 0 ), ( (
#line 1190 "../../src/simpl.c"
(_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ref ), ((unsigned char )44 ), _au3_p , (struct expr *)0 ) ))
#line 1190 "../../src/simpl.c"
, ( (_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = _au4_vp ), ((_au0__Xthis__ctor_ref ))) ) ) , _au4_ie ) ;

#line 1193 "../../src/simpl.c"
if (_au3_f -> _fct_returns -> _node_base == 38 )
#line 1194 "../../src/simpl.c"
_au4_vc = (struct expr *)( (_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr =
#line 1194 "../../src/simpl.c"
(struct texpr *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_texpr ), ((unsigned char )113 ), _au4_vc , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_texpr ->
#line 1194 "../../src/simpl.c"
_expr__O5.__C5_tp2 = Pfctchar_type ), ((_au0__Xthis__ctor_texpr ))) ) ) ;

#line 1196 "../../src/simpl.c"
_au0_this -> _node_base = 146 ;

#line 1198 "../../src/simpl.c"
_au0_this -> _expr__O3.__C3_e1 = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )68 , _au3_q , _au4_vc ) ;
_au0_this -> _expr__O3.__C3_e1 -> _expr__O5.__C5_cond = _au4_c ;
_au0_this -> _expr__O3.__C3_e1 = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )111 , _au0_this -> _expr__O3.__C3_e1 , (struct expr *)0 ) ;
#line 1200 "../../src/simpl.c"

#line 1201 "../../src/simpl.c"
_au0_this -> _expr__O3.__C3_e1 -> _expr__O5.__C5_tp2 = (((struct type *)_au3_f -> _fct_f_this ));
_au0_this -> _expr__O4.__C4_e2 = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , _au3_p , _au0_this -> _expr__O4.__C4_e2 ) ;
}
}

#line 1204 "../../src/simpl.c"
break ;
}
case 45 : 
#line 1207 "../../src/simpl.c"
case 44 : 
#line 1208 "../../src/simpl.c"
{ Pref _au3_r ;
Pexpr _au3_a1 ;

#line 1208 "../../src/simpl.c"
_au3_r = (((struct ref *)_au0_this -> _expr__O3.__C3_e1 ));
_au3_a1 = _au3_r -> _expr__O3.__C3_e1 ;

#line 1211 "../../src/simpl.c"
if (_au1_f && _au1_f -> _fct_f_virtual ){ 
#line 1212 "../../src/simpl.c"
Pexpr _au4_a11 ;

#line 1256 "../../src/simpl.c"
int _au4_index ;
Pexpr _au4_ie ;
Pname _au4_vp ;
Pexpr _au4_vptr ;
Pexpr _au4_ee ;
Ptype _au4_pft ;

#line 1262 "../../src/simpl.c"
struct ival *_au0__Xthis__ctor_ival ;

#line 1262 "../../src/simpl.c"
struct ref *_au0__Xthis__ctor_ref ;

#line 1262 "../../src/simpl.c"
struct ptr *_au0__Xthis__ctor_ptr ;

#line 1262 "../../src/simpl.c"
struct texpr *_au0__Xthis__ctor_texpr ;

#line 1212 "../../src/simpl.c"
_au4_a11 = 0 ;

#line 1214 "../../src/simpl.c"
switch (_au3_a1 -> _node_base ){ 
#line 1215 "../../src/simpl.c"
case 85 : 
#line 1216 "../../src/simpl.c"
_au4_a11 = _au3_a1 ;
break ;

#line 1219 "../../src/simpl.c"
case 44 : 
#line 1220 "../../src/simpl.c"
case 45 : 
#line 1221 "../../src/simpl.c"
if (_au3_a1 -> _expr__O3.__C3_e1 -> _node_base == 85 )_au4_a11 = _au3_a1 ;
break ;
case 112 : 
#line 1224 "../../src/simpl.c"
case 145 : 
#line 1225 "../../src/simpl.c"
if (_au3_a1 -> _expr__O4.__C4_e2 -> _node_base == 85 )_au4_a11 = _au3_a1 ;
break ;
}

#line 1229 "../../src/simpl.c"
if (_au0_this -> _expr__O3.__C3_e1 -> _node_base == 45 ){ 
#line 1230 "../../src/simpl.c"
if (_au4_a11 )_au4_a11 = _expr_address ( _au4_a11 ) ;
_au3_a1 = _expr_address ( _au3_a1 ) ;
}

#line 1234 "../../src/simpl.c"
if (_au4_a11 == 0 ){ 
#line 1236 "../../src/simpl.c"
Pname _au5_nx ;

#line 1238 "../../src/simpl.c"
Pname _au5_n ;

#line 1240 "../../src/simpl.c"
Pname _au5_cln ;

#line 1248 "../../src/simpl.c"
Pcall _au5_cc ;

#line 1249 "../../src/simpl.c"
struct call *_au0__Xthis__ctor_call ;

#line 1236 "../../src/simpl.c"
_au5_nx = (struct name *)_name__ctor ( (struct name *)0 , make_name ( (unsigned char )'K' ) ) ;
_au5_nx -> _expr__O2.__C2_tp = _au3_a1 -> _expr__O2.__C2_tp ;
_au5_n = _name_dcl ( _au5_nx , scope , (unsigned char )136 ) ;
_name__dtor ( _au5_nx , 1) ;
_au5_cln = _type_is_cl_obj ( _au3_a1 -> _expr__O2.__C2_tp ) ;
if (_au5_cln && ( (((struct classdef *)_au5_cln -> _expr__O2.__C2_tp ))-> _classdef_itor ) )_au5_n -> _name_n_xref = 1 ;
_au5_n -> _name_n_scope = 108 ;
_name_assign ( _au5_n ) ;
_au4_a11 = (struct expr *)_au5_n ;
_au3_a1 = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )70 , (struct expr *)_au5_n , _au3_a1 ) ;
_au3_a1 -> _expr__O2.__C2_tp = _au5_n -> _expr__O2.__C2_tp ;
_expr_simpl ( _au3_a1 ) ;
_au5_cc = (struct call *)( (_au0__Xthis__ctor_call = 0 ), ( ( (_au0__Xthis__ctor_call = 0 ), (_au0__Xthis__ctor_call = (struct call *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_call ),
#line 1248 "../../src/simpl.c"
(unsigned char )109 , ((struct expr *)0 ), ((struct expr *)0 )) )) , ((_au0__Xthis__ctor_call ))) ) ;
(*_au5_cc )= (*_au0_this );
_au0_this -> _node_base = 71 ;
_au0_this -> _expr__O3.__C3_e1 = _au3_a1 ;
_au0_this -> _expr__O4.__C4_e2 = (struct expr *)_au5_cc ;
_au0_this = _au5_cc ;
}
_au0_this -> _expr__O4.__C4_e2 = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , _au4_a11 , _au0_this -> _expr__O4.__C4_e2 ) ;
_au4_index = _au1_f -> _fct_f_virtual ;

#line 1256 "../../src/simpl.c"
_au4_ie = (struct expr *)((1 < _au4_index )? ( (_au0__Xthis__ctor_ival = 0 ), ( ( (_au0__Xthis__ctor_ival = 0 ), (_au0__Xthis__ctor_ival = (struct ival *)_expr__ctor (
#line 1256 "../../src/simpl.c"
((struct expr *)_au0__Xthis__ctor_ival ), (unsigned char )150 , (struct expr *)0 , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ival -> _expr__O3.__C3_i1 =
#line 1256 "../../src/simpl.c"
(_au4_index - 1 )), ((_au0__Xthis__ctor_ival ))) ) ) : (((struct ival *)0 )));

#line 1256 "../../src/simpl.c"
_au4_vp = _table_look ( _au1_fn -> _expr__O5.__C5_n_table , "_vptr", (unsigned char )0 ) ;

#line 1256 "../../src/simpl.c"
_au4_vptr = (struct expr *)( (_au0__Xthis__ctor_ref = 0 ), ( ( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ref ),
#line 1256 "../../src/simpl.c"
((unsigned char )44 ), _au4_a11 , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = _au4_vp ), ((_au0__Xthis__ctor_ref ))) )
#line 1256 "../../src/simpl.c"
) ;

#line 1256 "../../src/simpl.c"
_au4_ee = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )111 , _au4_vptr , _au4_ie ) ;

#line 1256 "../../src/simpl.c"
_au4_pft = (struct type *)( (_au0__Xthis__ctor_ptr = 0 ), ( (_au0__Xthis__ctor_ptr = (struct ptr *)_new ( (long )(sizeof (struct ptr))) ), (
#line 1256 "../../src/simpl.c"
(Nt ++ ), ( (_au0__Xthis__ctor_ptr -> _node_base = ((unsigned char )125 )), ( (_au0__Xthis__ctor_ptr -> _pvtyp_typ = ((struct type *)_au1_f )), (
#line 1256 "../../src/simpl.c"
(_au0__Xthis__ctor_ptr -> _ptr_rdo = ((char )0 )), ((_au0__Xthis__ctor_ptr ))) ) ) ) ) ) ;

#line 1262 "../../src/simpl.c"
_au4_ee = (struct expr *)( (_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr = (struct texpr *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_texpr ),
#line 1262 "../../src/simpl.c"
((unsigned char )113 ), _au4_ee , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 = _au4_pft ), ((_au0__Xthis__ctor_texpr ))) )
#line 1262 "../../src/simpl.c"
) ;
_au4_ee -> _expr__O2.__C2_tp = (((struct type *)_au1_f -> _fct_f_this ));
_au0_this -> _expr__O3.__C3_e1 = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )111 , _au4_ee , (struct expr *)0 ) ;

#line 1266 "../../src/simpl.c"
_au0_this -> _expr__O5.__C5_fct_name = 0 ;
_au1_fn = 0 ;
_expr_simpl ( _au0_this -> _expr__O4.__C4_e2 ) ;
return ;
}
else { 
#line 1273 "../../src/simpl.c"
Ptype _au4_tt ;

#line 1273 "../../src/simpl.c"
_au4_tt = _au3_r -> _expr__O5.__C5_mem -> _expr__O2.__C2_tp ;
llp :
#line 1275 "../../src/simpl.c"
switch (_au4_tt -> _node_base ){ 
#line 1277 "../../src/simpl.c"
case 97 : 
#line 1278 "../../src/simpl.c"
_au4_tt = (((struct basetype *)_au4_tt ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 1278 "../../src/simpl.c"
goto llp ;
case 108 : 
#line 1280 "../../src/simpl.c"
case 76 : 
#line 1281 "../../src/simpl.c"
if (_au3_a1 -> _node_base != 68 ){ 
#line 1282 "../../src/simpl.c"
if (_au0_this -> _expr__O3.__C3_e1 -> _node_base == 45 )_au3_a1 = _expr_address (
#line 1282 "../../src/simpl.c"
_au3_a1 ) ;
_au0_this -> _expr__O4.__C4_e2 = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , _au3_a1 , _au0_this -> _expr__O4.__C4_e2 ) ;
_au0_this -> _expr__O3.__C3_e1 = (struct expr *)_au3_r -> _expr__O5.__C5_mem ;
}
else { 
#line 1287 "../../src/simpl.c"
Pexpr _au6_t0 ;
Pexpr _au6_t1 ;

#line 1287 "../../src/simpl.c"
_au6_t0 = (struct expr *)_expr__ctor ( (struct expr *)0 , _au0_this -> _node_base , (struct expr *)0 , (struct expr *)0 ) ;
_au6_t1 = (struct expr *)_expr__ctor ( (struct expr *)0 , _au0_this -> _expr__O3.__C3_e1 -> _node_base , (struct expr *)0 , (struct expr *)0 ) ;
(*_au6_t0 )= (*(((struct expr *)_au0_this )));
(*_au6_t1 )= (*_au0_this -> _expr__O3.__C3_e1 );
_au6_t0 -> _expr__O3.__C3_e1 = _au6_t1 ;
_au6_t1 -> _expr__O3.__C3_e1 = _au3_a1 -> _expr__O3.__C3_e1 ;
_au3_a1 -> _expr__O3.__C3_e1 = _au6_t0 ;

#line 1295 "../../src/simpl.c"
{ Pexpr _au6_tt0 ;
Pexpr _au6_tt1 ;

#line 1295 "../../src/simpl.c"
_au6_tt0 = (struct expr *)_expr__ctor ( (struct expr *)0 , _au0_this -> _node_base , (struct expr *)0 , (struct expr *)0 ) ;
_au6_tt1 = (struct expr *)_expr__ctor ( (struct expr *)0 , _au0_this -> _expr__O3.__C3_e1 -> _node_base , (struct expr *)0 , (struct expr *)0 ) ;
(*_au6_tt0 )= (*(((struct expr *)_au0_this )));
(*_au6_tt1 )= (*_au0_this -> _expr__O3.__C3_e1 );
_au6_tt0 -> _expr__O3.__C3_e1 = _au6_tt1 ;
_au6_tt1 -> _expr__O3.__C3_e1 = _au3_a1 -> _expr__O4.__C4_e2 ;
_au3_a1 -> _expr__O4.__C4_e2 = _au6_tt0 ;

#line 1303 "../../src/simpl.c"
(*(((struct expr *)_au0_this )))= (*_au3_a1 );
_expr_simpl ( ((struct expr *)_au0_this )) ;
return ;
}
}
}
}
}
}
_expr_simpl ( _au0_this -> _expr__O4.__C4_e2 ) ;

#line 1314 "../../src/simpl.c"
if ((_au0_this -> _expr__O3.__C3_e1 -> _node_base == 85 )&& (_au0_this -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp -> _node_base == 108 )){ 
#line 1316 "../../src/simpl.c"
_au0_this -> _expr__O5.__C5_fct_name = (_au1_fn = (((struct
#line 1316 "../../src/simpl.c"
name *)_au0_this -> _expr__O3.__C3_e1 )));
_au1_f = (((struct fct *)_au1_fn -> _expr__O2.__C2_tp ));
}

#line 1320 "../../src/simpl.c"
;
if ((_au1_fn && _au1_f -> _fct_f_inline )&& (debug == 0 )){ 
#line 1322 "../../src/simpl.c"
_au1_ee = _fct_expand ( _au1_f , _au1_fn , scope , _au0_this -> _expr__O4.__C4_e2 ) ;
#line 1322 "../../src/simpl.c"

#line 1323 "../../src/simpl.c"
if (_au1_ee )
#line 1324 "../../src/simpl.c"
(*(((struct expr *)_au0_this )))= (*_au1_ee );
else 
#line 1327 "../../src/simpl.c"
if (((_au0_this -> _expr__O2.__C2_tp -> _node_base == 97 )&& (((struct basetype *)_au0_this -> _expr__O2.__C2_tp ))-> _basetype_b_name )&& ((((struct basetype *)_au0_this -> _expr__O2.__C2_tp ))-> _basetype_b_name ->
#line 1327 "../../src/simpl.c"
_expr__O2.__C2_tp -> _node_base == 119 ))
#line 1330 "../../src/simpl.c"
{ 
#line 1331 "../../src/simpl.c"
Pexpr _au3_ee1 ;

#line 1331 "../../src/simpl.c"
_au3_ee1 = (struct expr *)_expr__ctor ( (struct expr *)0 , _au0_this -> _node_base , (struct expr *)0 , (struct expr *)0 ) ;
(*_au3_ee1 )= (*(((struct expr *)_au0_this )));
{ Pname _au3_tmp ;

#line 1333 "../../src/simpl.c"
_au3_tmp = make_tmp ( 'T' , _au0_this -> _expr__O2.__C2_tp , scope ) ;
_au1_ee = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )70 , (struct expr *)_au3_tmp , _au3_ee1 ) ;
{ Pexpr _au3_ee2 ;

#line 1335 "../../src/simpl.c"
_au3_ee2 = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )145 , (struct expr *)0 , (struct expr *)_au3_tmp ) ;
_au1_ee = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )71 , _au1_ee , _au3_ee2 ) ;
(*(((struct expr *)_au0_this )))= (*_au1_ee );
}
}
}
}
}
;

#line 1342 "../../src/simpl.c"
char ccheck (_au0_e )Pexpr _au0_e ;

#line 1346 "../../src/simpl.c"
{ 
#line 1347 "../../src/simpl.c"
if (_au0_e )
#line 1348 "../../src/simpl.c"
switch (_au0_e -> _node_base ){ 
#line 1349 "../../src/simpl.c"
case 68 : 
#line 1350 "../../src/simpl.c"
case 66 : 
#line 1351 "../../src/simpl.c"
case 67 : 
#line 1352 "../../src/simpl.c"
{ 
#line 1383 "../../src/simpl.c"
struct ea _au0__V27 ;
#line 1383 "../../src/simpl.c"

#line 1352 "../../src/simpl.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"E too complicated: uses%k and needs temorary ofCW destructor", (struct ea *)( ( ((& _au0__V27 )-> _ea__O1.__C1_i = ((int )_au0_e -> _node_base )),
#line 1352 "../../src/simpl.c"
(((& _au0__V27 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
break ;
case 58 : 
#line 1355 "../../src/simpl.c"
case 59 : 
#line 1356 "../../src/simpl.c"
case 60 : 
#line 1357 "../../src/simpl.c"
case 61 : 
#line 1358 "../../src/simpl.c"
case 62 : 
#line 1359 "../../src/simpl.c"
case 63 : 
#line 1360 "../../src/simpl.c"
case 70 :
#line 1360 "../../src/simpl.c"

#line 1361 "../../src/simpl.c"
case 126 : 
#line 1362 "../../src/simpl.c"
case 127 : 
#line 1363 "../../src/simpl.c"
case 147 : 
#line 1364 "../../src/simpl.c"
case 71 : 
#line 1365 "../../src/simpl.c"
case 54 : 
#line 1366 "../../src/simpl.c"
case 55 : 
#line 1367 "../../src/simpl.c"
case 50 :
#line 1367 "../../src/simpl.c"

#line 1368 "../../src/simpl.c"
case 51 : 
#line 1369 "../../src/simpl.c"
case 65 : 
#line 1370 "../../src/simpl.c"
case 64 : 
#line 1371 "../../src/simpl.c"
case 52 : 
#line 1372 "../../src/simpl.c"
case 146 : 
#line 1373 "../../src/simpl.c"
case 109 : 
#line 1374 "../../src/simpl.c"
case 140 :
#line 1374 "../../src/simpl.c"

#line 1375 "../../src/simpl.c"
ccheck ( _au0_e -> _expr__O3.__C3_e1 ) ;
case 46 : 
#line 1377 "../../src/simpl.c"
case 47 : 
#line 1378 "../../src/simpl.c"
case 113 : 
#line 1379 "../../src/simpl.c"
case 112 : 
#line 1380 "../../src/simpl.c"
case 145 : 
#line 1381 "../../src/simpl.c"
ccheck ( _au0_e -> _expr__O4.__C4_e2 ) ;
#line 1381 "../../src/simpl.c"
} }
}
;

#line 1385 "../../src/simpl.c"
char temp_in_cond (_au0_ee , _au0_ss , _au0_tbl )Pexpr _au0_ee ;

#line 1385 "../../src/simpl.c"
Pstmt _au0_ss ;

#line 1385 "../../src/simpl.c"
Ptable _au0_tbl ;

#line 1389 "../../src/simpl.c"
{ 
#line 1392 "../../src/simpl.c"
Ptype _au1_ct ;

#line 1397 "../../src/simpl.c"
Pname _au1_n ;

#line 1399 "../../src/simpl.c"
Pname _au1_tmp ;

#line 1402 "../../src/simpl.c"
Pexpr _au1_v ;

#line 1406 "../../src/simpl.c"
Pexpr _au1_c ;

#line 1410 "../../src/simpl.c"
Pexpr _au1_ex ;
Pstmt _au1_sx ;

#line 1412 "../../src/simpl.c"
struct texpr *_au0__Xthis__ctor_texpr ;

#line 1390 "../../src/simpl.c"
ccheck ( _au0_ee ) ;
while ((_au0_ee -> _node_base == 71 )|| (_au0_ee -> _node_base == 147 ))_au0_ee = _au0_ee -> _expr__O4.__C4_e2 ;
_au1_ct = _au0_ee -> _expr__O2.__C2_tp ;

#line 1392 "../../src/simpl.c"
_au1_n = (struct name *)_name__ctor ( (struct name *)0 , make_name ( (unsigned char )'Q' ) ) ;

#line 1398 "../../src/simpl.c"
_au1_n -> _expr__O2.__C2_tp = _au1_ct ;
_au1_tmp = _name_dcl ( _au1_n , _au0_tbl , (unsigned char )136 ) ;
_name__dtor ( _au1_n , 1) ;
_au1_tmp -> _name_n_scope = 108 ;
_au1_v = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )0 , (struct expr *)0 , (struct expr *)0 ) ;
(*_au1_v )= (*_au0_ee );
_au1_v = (struct expr *)( (_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr = (struct texpr *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_texpr ),
#line 1404 "../../src/simpl.c"
((unsigned char )113 ), _au1_v , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 = _au1_ct ), ((_au0__Xthis__ctor_texpr ))) )
#line 1404 "../../src/simpl.c"
) ;
_au1_v -> _expr__O2.__C2_tp = _au1_ct ;
_au1_c = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )70 , (struct expr *)_au1_tmp , _au1_v ) ;
_au1_c -> _expr__O2.__C2_tp = _au1_ct ;
_au0_ee -> _node_base = 71 ;
_au0_ee -> _expr__O3.__C3_e1 = _au1_c ;
_au1_ex = 0 ;
for(_au1_sx = _au0_ss ;_au1_sx ;_au1_sx = _au1_sx -> _stmt_s_list ) { 
#line 1412 "../../src/simpl.c"
_au1_ex = (_au1_ex ? _expr__ctor ( (struct expr *)0 , (unsigned char )71 , _au1_ex ,
#line 1412 "../../src/simpl.c"
_au1_sx -> _stmt__O8.__C8_e ) : _au1_sx -> _stmt__O8.__C8_e );
_au1_ex -> _expr__O2.__C2_tp = _au1_sx -> _stmt__O8.__C8_e -> _expr__O2.__C2_tp ;
}
_au0_ee -> _expr__O4.__C4_e2 = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )71 , _au0_ss -> _stmt__O8.__C8_e , (struct expr *)_au1_tmp ) ;
#line 1415 "../../src/simpl.c"

#line 1416 "../../src/simpl.c"
_au0_ee -> _expr__O4.__C4_e2 -> _expr__O2.__C2_tp = _au1_ct ;
}
;
bit not_safe (_au0_e )Pexpr _au0_e ;
{ 
#line 1422 "../../src/simpl.c"
switch (_au0_e -> _node_base ){ 
#line 1423 "../../src/simpl.c"
default : 
#line 1424 "../../src/simpl.c"
return (char )1 ;

#line 1433 "../../src/simpl.c"
case 85 : 
#line 1436 "../../src/simpl.c"
{ Pname _au3_n ;

#line 1436 "../../src/simpl.c"
_au3_n = (((struct name *)_au0_e ));
if ((_au3_n -> _expr__O5.__C5_n_table != gtbl )&& (_au3_n -> _expr__O5.__C5_n_table -> _table_t_name == 0 )){ 
#line 1438 "../../src/simpl.c"
Pname _au4_cn ;

#line 1438 "../../src/simpl.c"
_au4_cn = _type_is_cl_obj ( _au3_n -> _expr__O2.__C2_tp ) ;
if (_au4_cn && ( _table_look ( (((struct classdef *)_au4_cn -> _expr__O2.__C2_tp ))-> _classdef_memtbl , "_dtor", (unsigned char )0 ) ) )return
#line 1439 "../../src/simpl.c"
(char )1 ;
}
}
case 150 : 
#line 1443 "../../src/simpl.c"
case 82 : 
#line 1444 "../../src/simpl.c"
case 84 : 
#line 1445 "../../src/simpl.c"
case 83 : 
#line 1446 "../../src/simpl.c"
case 81 : 
#line 1447 "../../src/simpl.c"
return (char )0 ;
case 46 : 
#line 1449 "../../src/simpl.c"
case 47 : 
#line 1450 "../../src/simpl.c"
case 112 : 
#line 1451 "../../src/simpl.c"
case 145 : 
#line 1452 "../../src/simpl.c"
return not_safe ( _au0_e -> _expr__O4.__C4_e2 ) ;
case 111 : 
#line 1455 "../../src/simpl.c"
{ 
#line 1456 "../../src/simpl.c"
int _au3_i ;

#line 1456 "../../src/simpl.c"
_au3_i = not_safe ( _au0_e -> _expr__O3.__C3_e1 ) ;
if (_au3_i )return (char )_au3_i ;
if (_au0_e -> _expr__O4.__C4_e2 )return not_safe ( _au0_e -> _expr__O4.__C4_e2 ) ;
return (char )0 ;
}
case 71 : 
#line 1462 "../../src/simpl.c"
case 54 : 
#line 1463 "../../src/simpl.c"
case 55 : 
#line 1464 "../../src/simpl.c"
case 50 : 
#line 1465 "../../src/simpl.c"
case 51 : 
#line 1466 "../../src/simpl.c"
case 53 : 
#line 1467 "../../src/simpl.c"
case 70 :
#line 1467 "../../src/simpl.c"

#line 1468 "../../src/simpl.c"
case 126 : 
#line 1469 "../../src/simpl.c"
case 127 : 
#line 1470 "../../src/simpl.c"
case 128 : 
#line 1471 "../../src/simpl.c"
case 129 : 
#line 1472 "../../src/simpl.c"
case 65 : 
#line 1473 "../../src/simpl.c"
case 52 : 
#line 1474 "../../src/simpl.c"
case 67 :
#line 1474 "../../src/simpl.c"

#line 1475 "../../src/simpl.c"
case 66 : 
#line 1476 "../../src/simpl.c"
case 58 : 
#line 1477 "../../src/simpl.c"
case 59 : 
#line 1478 "../../src/simpl.c"
case 60 : 
#line 1479 "../../src/simpl.c"
case 61 : 
#line 1480 "../../src/simpl.c"
case 62 : 
#line 1481 "../../src/simpl.c"
case 63 :
#line 1481 "../../src/simpl.c"

#line 1482 "../../src/simpl.c"
return (char )(not_safe ( _au0_e -> _expr__O3.__C3_e1 ) || not_safe ( _au0_e -> _expr__O4.__C4_e2 ) );
case 68 : 
#line 1484 "../../src/simpl.c"
return (char )((not_safe ( _au0_e -> _expr__O5.__C5_cond ) || not_safe ( _au0_e -> _expr__O3.__C3_e1 ) )|| not_safe ( _au0_e ->
#line 1484 "../../src/simpl.c"
_expr__O4.__C4_e2 ) );
}
}
;

#line 1489 "../../src/simpl.c"
Pexpr curr_expr ;

#line 1493 "../../src/simpl.c"
Pstmt _stmt_simpl (_au0_this )
#line 651 "../../src/cfront.h"
struct stmt *_au0_this ;

#line 1497 "../../src/simpl.c"
{ 
#line 1498 "../../src/simpl.c"
if (_au0_this == 0 )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"0->S::simpl()", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ,
#line 1498 "../../src/simpl.c"
(struct ea *)ea0 ) ;

#line 1501 "../../src/simpl.c"
curr_expr = _au0_this -> _stmt__O8.__C8_e ;

#line 1503 "../../src/simpl.c"
switch (_au0_this -> _node_base ){ 
#line 1504 "../../src/simpl.c"
default : 
#line 1505 "../../src/simpl.c"
{ 
#line 1946 "../../src/simpl.c"
struct ea _au0__V28 ;

#line 1505 "../../src/simpl.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"S::simpl(%k)", (struct ea *)( ( ((& _au0__V28 )-> _ea__O1.__C1_i = ((int )_au0_this -> _node_base )),
#line 1505 "../../src/simpl.c"
(((& _au0__V28 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;

#line 1507 "../../src/simpl.c"
case 1 : 
#line 1508 "../../src/simpl.c"
break ;

#line 1510 "../../src/simpl.c"
case 3 : 
#line 1511 "../../src/simpl.c"
if (break_del_list ){ 
#line 1512 "../../src/simpl.c"
Pstmt _au3_bs ;
Pstmt _au3_dl ;

#line 1514 "../../src/simpl.c"
struct pair *_au0__Xthis__ctor_pair ;

#line 1512 "../../src/simpl.c"
_au3_bs = (struct stmt *)_stmt__ctor ( (struct stmt *)0 , _au0_this -> _node_base , _au0_this -> _stmt_where , (struct stmt *)0 ) ;
_au3_dl = _stmt_copy ( break_del_list ) ;
_au0_this -> _node_base = 116 ;
_au0_this -> _stmt_s = (struct stmt *)( (_au0__Xthis__ctor_pair = 0 ), ( ( (_au0__Xthis__ctor_pair = 0 ), (_au0__Xthis__ctor_pair = (struct pair *)_stmt__ctor ( ((struct
#line 1515 "../../src/simpl.c"
stmt *)_au0__Xthis__ctor_pair ), (unsigned char )166 , _au0_this -> _stmt_where , _au3_dl ) )) , ( (_au0__Xthis__ctor_pair -> _stmt__O8.__C8_s2 = _au3_bs ), ((_au0__Xthis__ctor_pair )))
#line 1515 "../../src/simpl.c"
) ) ;
}
break ;

#line 1519 "../../src/simpl.c"
case 7 : 
#line 1520 "../../src/simpl.c"
if (continue_del_list ){ 
#line 1521 "../../src/simpl.c"
Pstmt _au3_bs ;
Pstmt _au3_dl ;

#line 1523 "../../src/simpl.c"
struct pair *_au0__Xthis__ctor_pair ;

#line 1521 "../../src/simpl.c"
_au3_bs = (struct stmt *)_stmt__ctor ( (struct stmt *)0 , _au0_this -> _node_base , _au0_this -> _stmt_where , (struct stmt *)0 ) ;
_au3_dl = _stmt_copy ( continue_del_list ) ;
_au0_this -> _node_base = 116 ;
_au0_this -> _stmt_s = (struct stmt *)( (_au0__Xthis__ctor_pair = 0 ), ( ( (_au0__Xthis__ctor_pair = 0 ), (_au0__Xthis__ctor_pair = (struct pair *)_stmt__ctor ( ((struct
#line 1524 "../../src/simpl.c"
stmt *)_au0__Xthis__ctor_pair ), (unsigned char )166 , _au0_this -> _stmt_where , _au3_dl ) )) , ( (_au0__Xthis__ctor_pair -> _stmt__O8.__C8_s2 = _au3_bs ), ((_au0__Xthis__ctor_pair )))
#line 1524 "../../src/simpl.c"
) ) ;
}
break ;

#line 1528 "../../src/simpl.c"
case 8 : 
#line 1529 "../../src/simpl.c"
_stmt_simpl ( _au0_this -> _stmt_s ) ;
break ;

#line 1532 "../../src/simpl.c"
case 72 : 
#line 1533 "../../src/simpl.c"
if (_au0_this -> _stmt__O8.__C8_e ){ 
#line 1534 "../../src/simpl.c"
if (_au0_this -> _stmt__O8.__C8_e -> _node_base == 111 )_au0_this -> _stmt__O8.__C8_e = _au0_this -> _stmt__O8.__C8_e -> _expr__O3.__C3_e1 ;
#line 1534 "../../src/simpl.c"

#line 1535 "../../src/simpl.c"
_expr_simpl ( _au0_this -> _stmt__O8.__C8_e ) ;
if (_au0_this -> _stmt__O8.__C8_e -> _node_base == 111 )_au0_this -> _stmt__O8.__C8_e = _au0_this -> _stmt__O8.__C8_e -> _expr__O3.__C3_e1 ;
}
break ;

#line 1540 "../../src/simpl.c"
case 28 : 
#line 1541 "../../src/simpl.c"
{ 
#line 1555 "../../src/simpl.c"
no_of_returns ++ ;
{ Pstmt _au3_dl ;
Pfct _au3_f ;

#line 1558 "../../src/simpl.c"
struct estmt *_au0__Xthis__ctor_estmt ;

#line 1556 "../../src/simpl.c"
_au3_dl = (del_list ? _stmt_copy ( del_list ) : (((struct stmt *)0 )));
_au3_f = (((struct fct *)curr_fct -> _expr__O2.__C2_tp ));

#line 1559 "../../src/simpl.c"
if (_au0_this -> _stmt__O8.__C8_e == 0 )_au0_this -> _stmt__O8.__C8_e = dummy ;
if ((_au0_this -> _stmt__O8.__C8_e == dummy )&& (curr_fct -> _name_n_oper == 161 ))_au0_this -> _stmt__O8.__C8_e = (struct expr *)_au3_f -> _fct_f_this ;

#line 1562 "../../src/simpl.c"
if (_au3_f -> _fct_f_result ){ 
#line 1564 "../../src/simpl.c"
if (_au0_this -> _stmt__O8.__C8_e -> _node_base == 147 )_au0_this -> _stmt__O8.__C8_e = replace_temp ( _au0_this -> _stmt__O8.__C8_e , (struct expr *)_au3_f ->
#line 1564 "../../src/simpl.c"
_fct_f_result ) ;
_expr_simpl ( _au0_this -> _stmt__O8.__C8_e ) ;
{ Pstmt _au4_cs ;

#line 1567 "../../src/simpl.c"
struct pair *_au0__Xthis__ctor_pair ;

#line 1567 "../../src/simpl.c"
struct estmt *_au0__Xthis__ctor_estmt ;

#line 1566 "../../src/simpl.c"
_au4_cs = (struct stmt *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt = (struct estmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_estmt ),
#line 1566 "../../src/simpl.c"
((unsigned char )72 ), _au0_this -> _stmt_where , ((struct stmt *)0 )) )) , ( (_au0__Xthis__ctor_estmt -> _stmt__O8.__C8_e = _au0_this -> _stmt__O8.__C8_e ),
#line 1566 "../../src/simpl.c"
((_au0__Xthis__ctor_estmt ))) ) ) ;
if (_au3_dl )_au4_cs = (struct stmt *)( (_au0__Xthis__ctor_pair = 0 ), ( ( (_au0__Xthis__ctor_pair = 0 ), (_au0__Xthis__ctor_pair = (struct pair *)_stmt__ctor ( ((struct
#line 1567 "../../src/simpl.c"
stmt *)_au0__Xthis__ctor_pair ), (unsigned char )166 , _au0_this -> _stmt_where , _au4_cs ) )) , ( (_au0__Xthis__ctor_pair -> _stmt__O8.__C8_s2 = _au3_dl ), ((_au0__Xthis__ctor_pair )))
#line 1567 "../../src/simpl.c"
) ) ;
_au0_this -> _node_base = 166 ;
_au0_this -> _stmt_s = _au4_cs ;
_au0_this -> _stmt__O8.__C8_s2 = (struct stmt *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt = (struct estmt *)_stmt__ctor ( ((struct
#line 1570 "../../src/simpl.c"
stmt *)_au0__Xthis__ctor_estmt ), ((unsigned char )28 ), _au0_this -> _stmt_where , ((struct stmt *)0 )) )) , ( (_au0__Xthis__ctor_estmt -> _stmt__O8.__C8_e = ((struct
#line 1570 "../../src/simpl.c"
expr *)0 )), ((_au0__Xthis__ctor_estmt ))) ) ) ;
}
}
else 
#line 1576 "../../src/simpl.c"
{ 
#line 1578 "../../src/simpl.c"
_expr_simpl ( _au0_this -> _stmt__O8.__C8_e ) ;
if (_au3_dl ){ 
#line 1580 "../../src/simpl.c"
if ((_au0_this -> _stmt__O8.__C8_e != dummy )&& not_safe ( _au0_this -> _stmt__O8.__C8_e ) ){ 
#line 1582 "../../src/simpl.c"
Ptable _au6_ftbl ;

#line 1584 "../../src/simpl.c"
Pname _au6_r ;

#line 1594 "../../src/simpl.c"
Pexpr _au6_as ;

#line 1596 "../../src/simpl.c"
Pstmt _au6_cs ;

#line 1597 "../../src/simpl.c"
struct estmt *_au0__Xthis__ctor_estmt ;

#line 1597 "../../src/simpl.c"
struct pair *_au0__Xthis__ctor_pair ;

#line 1582 "../../src/simpl.c"
_au6_ftbl = (((struct fct *)curr_fct -> _expr__O2.__C2_tp ))-> _fct_body -> _stmt_memtbl ;

#line 1584 "../../src/simpl.c"
_au6_r = _table_look ( _au6_ftbl , "_result", (unsigned char )0 ) ;
if (_au6_r == 0 ){ 
#line 1586 "../../src/simpl.c"
_au6_r = (struct name *)_name__ctor ( (struct name *)0 , "_result") ;
_au6_r -> _expr__O2.__C2_tp = _au0_this -> _stmt__O7.__C7_ret_tp ;
{ Pname _au7_rn ;

#line 1588 "../../src/simpl.c"
_au7_rn = _name_dcl ( _au6_r , _au6_ftbl , (unsigned char )136 ) ;
_au7_rn -> _name_n_scope = 108 ;
_name_assign ( _au7_rn ) ;
_name__dtor ( _au6_r , 1) ;
_au6_r = _au7_rn ;
}
}

#line 1594 "../../src/simpl.c"
_au6_as = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )70 , (struct expr *)_au6_r , _au0_this -> _stmt__O8.__C8_e ) ;
_au6_as -> _expr__O2.__C2_tp = _au0_this -> _stmt__O7.__C7_ret_tp ;
_au6_cs = (struct stmt *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt = (struct estmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_estmt ),
#line 1596 "../../src/simpl.c"
((unsigned char )72 ), _au0_this -> _stmt_where , ((struct stmt *)0 )) )) , ( (_au0__Xthis__ctor_estmt -> _stmt__O8.__C8_e = _au6_as ), ((_au0__Xthis__ctor_estmt )))
#line 1596 "../../src/simpl.c"
) ) ;
_au6_cs = (struct stmt *)( (_au0__Xthis__ctor_pair = 0 ), ( ( (_au0__Xthis__ctor_pair = 0 ), (_au0__Xthis__ctor_pair = (struct pair *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_pair ),
#line 1597 "../../src/simpl.c"
(unsigned char )166 , _au0_this -> _stmt_where , _au6_cs ) )) , ( (_au0__Xthis__ctor_pair -> _stmt__O8.__C8_s2 = _au3_dl ), ((_au0__Xthis__ctor_pair ))) )
#line 1597 "../../src/simpl.c"
) ;
_au0_this -> _node_base = 166 ;
_au0_this -> _stmt_s = _au6_cs ;
_au0_this -> _stmt__O8.__C8_s2 = (struct stmt *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt = (struct estmt *)_stmt__ctor ( ((struct
#line 1600 "../../src/simpl.c"
stmt *)_au0__Xthis__ctor_estmt ), ((unsigned char )28 ), _au0_this -> _stmt_where , ((struct stmt *)0 )) )) , ( (_au0__Xthis__ctor_estmt -> _stmt__O8.__C8_e = ((struct
#line 1600 "../../src/simpl.c"
expr *)_au6_r )), ((_au0__Xthis__ctor_estmt ))) ) ) ;
_au0_this -> _stmt__O8.__C8_s2 -> _stmt__O7.__C7_ret_tp = _au0_this -> _stmt__O7.__C7_ret_tp ;
}
else { 
#line 1605 "../../src/simpl.c"
_au0_this -> _node_base = 166 ;
_au0_this -> _stmt_s = _au3_dl ;
_au0_this -> _stmt__O8.__C8_s2 = (struct stmt *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt = (struct estmt *)_stmt__ctor ( ((struct
#line 1607 "../../src/simpl.c"
stmt *)_au0__Xthis__ctor_estmt ), ((unsigned char )28 ), _au0_this -> _stmt_where , ((struct stmt *)0 )) )) , ( (_au0__Xthis__ctor_estmt -> _stmt__O8.__C8_e = _au0_this ->
#line 1607 "../../src/simpl.c"
_stmt__O8.__C8_e ), ((_au0__Xthis__ctor_estmt ))) ) ) ;
}
}
}
break ;
}
}
case 39 : 
#line 1615 "../../src/simpl.c"
case 10 : 
#line 1616 "../../src/simpl.c"
_expr_simpl ( _au0_this -> _stmt__O8.__C8_e ) ;
{ Pstmt _au3_obl ;
Pstmt _au3_ocl ;

#line 1617 "../../src/simpl.c"
_au3_obl = break_del_list ;
_au3_ocl = continue_del_list ;
break_del_list = 0 ;
continue_del_list = 0 ;
_stmt_simpl ( _au0_this -> _stmt_s ) ;
break_del_list = _au3_obl ;
continue_del_list = _au3_ocl ;
}
break ;

#line 1627 "../../src/simpl.c"
case 33 : 
#line 1628 "../../src/simpl.c"
_expr_simpl ( _au0_this -> _stmt__O8.__C8_e ) ;
{ Pstmt _au3_obl ;

#line 1629 "../../src/simpl.c"
_au3_obl = break_del_list ;
break_del_list = 0 ;
_stmt_simpl ( _au0_this -> _stmt_s ) ;
break_del_list = _au3_obl ;
}
switch (_au0_this -> _stmt_s -> _node_base ){ 
#line 1635 "../../src/simpl.c"
case 8 : 
#line 1636 "../../src/simpl.c"
case 115 : 
#line 1637 "../../src/simpl.c"
case 4 : 
#line 1638 "../../src/simpl.c"
break ;
case 116 : 
#line 1640 "../../src/simpl.c"
if (_au0_this -> _stmt_s -> _stmt_s )
#line 1641 "../../src/simpl.c"
switch (_au0_this -> _stmt_s -> _stmt_s -> _node_base ){ 
#line 1642 "../../src/simpl.c"
case 3 : 
#line 1643 "../../src/simpl.c"
case 4 :
#line 1643 "../../src/simpl.c"

#line 1644 "../../src/simpl.c"
case 115 : 
#line 1645 "../../src/simpl.c"
case 8 : 
#line 1646 "../../src/simpl.c"
break ;
default : 
#line 1648 "../../src/simpl.c"
goto df ;
}
break ;
default : 
#line 1652 "../../src/simpl.c"
df :
#line 1653 "../../src/simpl.c"
errorFI_PCloc__PC_RCea__RCea__RCea__RCea___ ( (int )'w' , & _au0_this -> _stmt_s -> _stmt_where , (char *)"S not reached: case label missing", (struct ea *)ea0 , (struct
#line 1653 "../../src/simpl.c"
ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
}
break ;

#line 1657 "../../src/simpl.c"
case 4 : 
#line 1658 "../../src/simpl.c"
_expr_simpl ( _au0_this -> _stmt__O8.__C8_e ) ;
_stmt_simpl ( _au0_this -> _stmt_s ) ;
break ;

#line 1662 "../../src/simpl.c"
case 115 : 
#line 1663 "../../src/simpl.c"
if (del_list )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"label in blockW destructors", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ,
#line 1663 "../../src/simpl.c"
(struct ea *)ea0 ) ;
_stmt_simpl ( _au0_this -> _stmt_s ) ;
break ;

#line 1667 "../../src/simpl.c"
case 19 : 
#line 1677 "../../src/simpl.c"
{ 
#line 1678 "../../src/simpl.c"
Pname _au3_n ;

#line 1678 "../../src/simpl.c"
_au3_n = _table_look ( scope , _au0_this -> _stmt__O7.__C7_d -> _expr__O3.__C3_string , (unsigned char )115 ) ;
if (_au3_n == 0 ){ 
#line 1946 "../../src/simpl.c"
struct ea _au0__V29 ;

#line 1679 "../../src/simpl.c"
errorFI_PCloc__PC_RCea__RCea__RCea__RCea___ ( (int )'i' , & _au0_this -> _stmt_where , (char *)"label%n missing", (struct ea *)( ( ((& _au0__V29 )-> _ea__O1.__C1_p =
#line 1679 "../../src/simpl.c"
((char *)_au0_this -> _stmt__O7.__C7_d )), (((& _au0__V29 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 1679 "../../src/simpl.c"
} 
#line 1680 "../../src/simpl.c"
if ((_au3_n -> _name__O6.__C6_n_realscope != scope )&& _au3_n -> _name_n_assigned_to ){ 
#line 1687 "../../src/simpl.c"
Ptable _au4_r ;

#line 1687 "../../src/simpl.c"
_au4_r = 0 ;

#line 1689 "../../src/simpl.c"
{ Ptable _au4_q ;

#line 1689 "../../src/simpl.c"
_au4_q = _au3_n -> _name__O6.__C6_n_realscope ;

#line 1689 "../../src/simpl.c"
for(;_au4_q != gtbl ;_au4_q = _au4_q -> _table_next ) { 
#line 1690 "../../src/simpl.c"
{ Ptable _au5_p ;

#line 1690 "../../src/simpl.c"
_au5_p = scope ;

#line 1690 "../../src/simpl.c"
for(;_au5_p != gtbl ;_au5_p = _au5_p -> _table_next ) { 
#line 1691 "../../src/simpl.c"
if (_au5_p == _au4_q ){ 
#line 1692 "../../src/simpl.c"
_au4_r = _au5_p ;
goto xyzzy ;
}
}
}
}
xyzzy :if (_au4_r == 0 )errorFI_PCloc__PC_RCea__RCea__RCea__RCea___ ( (int )'i' , & _au0_this -> _stmt_where , (char *)"finding root of subtree", (struct ea *)ea0 , (struct ea *)ea0 ,
#line 1698 "../../src/simpl.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;

#line 1711 "../../src/simpl.c"
{ Ptable _au4_p ;

#line 1711 "../../src/simpl.c"
_au4_p = _au3_n -> _name__O6.__C6_n_realscope ;

#line 1711 "../../src/simpl.c"
for(;_au4_p != _au4_r ;_au4_p = _au4_p -> _table_next ) 
#line 1712 "../../src/simpl.c"
if (_au4_p -> _table_init_stat == 2 ){ 
#line 1713 "../../src/simpl.c"
{ 
#line 1946 "../../src/simpl.c"
struct ea _au0__V30 ;

#line 1713 "../../src/simpl.c"
errorFPCloc__PC_RCea__RCea__RCea__RCea___ ( & _au0_this -> _stmt_where , (char *)"goto%n pastDWIr", (struct ea *)( ( ((& _au0__V30 )-> _ea__O1.__C1_p = ((char *)_au0_this ->
#line 1713 "../../src/simpl.c"
_stmt__O7.__C7_d )), (((& _au0__V30 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
goto plugh ;
} }
else if (_au4_p -> _table_init_stat == 0 ){ 
#line 1717 "../../src/simpl.c"
int _au5_i ;
{ Pname _au5_nn ;

#line 1718 "../../src/simpl.c"
_au5_nn = _table_get_mem ( _au4_p , _au5_i = 1 ) ;

#line 1718 "../../src/simpl.c"
for(;_au5_nn ;_au5_nn = _table_get_mem ( _au4_p , ++ _au5_i ) ) 
#line 1719 "../../src/simpl.c"
if (_au5_nn -> _expr__O4.__C4_n_initializer || _au5_nn -> _name_n_evaluated ){ 
#line 1720 "../../src/simpl.c"
{ 
#line 1946 "../../src/simpl.c"
struct ea _au0__V31 ;
#line 1946 "../../src/simpl.c"
struct ea _au0__V32 ;

#line 1720 "../../src/simpl.c"
errorFPCloc__PC_RCea__RCea__RCea__RCea___ ( & _au5_nn -> _name_where , (char *)"goto%n pastId%n", (struct ea *)( ( ((& _au0__V31 )-> _ea__O1.__C1_p = ((char *)_au0_this ->
#line 1720 "../../src/simpl.c"
_stmt__O7.__C7_d )), (((& _au0__V31 )))) ) , (struct ea *)( ( ((& _au0__V32 )-> _ea__O1.__C1_p = ((char *)_au5_nn )), (((&
#line 1720 "../../src/simpl.c"
_au0__V32 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 ) ;
goto plugh ;
} }
}
}

#line 1724 "../../src/simpl.c"
plugh :
#line 1734 "../../src/simpl.c"
{ 
#line 1735 "../../src/simpl.c"
Pstmt _au5_dd ;

#line 1735 "../../src/simpl.c"
Pstmt _au5_ddt ;

#line 1735 "../../src/simpl.c"
_au5_dd = 0 ;

#line 1735 "../../src/simpl.c"
_au5_ddt = 0 ;

#line 1737 "../../src/simpl.c"
{ Ptable _au5_p ;

#line 1737 "../../src/simpl.c"
_au5_p = scope ;

#line 1737 "../../src/simpl.c"
for(;_au5_p != _au4_r ;_au5_p = _au5_p -> _table_next ) { 
#line 1738 "../../src/simpl.c"
int _au6_i ;
{ Pname _au6_n ;

#line 1739 "../../src/simpl.c"
_au6_n = _table_get_mem ( _au5_p , _au6_i = 1 ) ;

#line 1739 "../../src/simpl.c"
for(;_au6_n ;_au6_n = _table_get_mem ( _au5_p , ++ _au6_i ) ) { 
#line 1740 "../../src/simpl.c"
Pname _au7_cln ;
if (_au6_n -> _expr__O2.__C2_tp == 0 )continue ;

#line 1743 "../../src/simpl.c"
if (_au7_cln = _type_is_cl_obj ( _au6_n -> _expr__O2.__C2_tp ) ){ 
#line 1744 "../../src/simpl.c"
Pclass _au8_cl ;
Pname _au8_d ;

#line 1744 "../../src/simpl.c"
_au8_cl = (((struct classdef *)_au7_cln -> _expr__O2.__C2_tp ));
_au8_d = ( _table_look ( _au8_cl -> _classdef_memtbl , "_dtor", (unsigned char )0 ) ) ;

#line 1747 "../../src/simpl.c"
if (_au8_d ){ 
#line 1748 "../../src/simpl.c"
Pref _au9_r ;
Pexpr _au9_ee ;
Pcall _au9_dl ;
Pstmt _au9_dls ;

#line 1752 "../../src/simpl.c"
struct ref *_au0__Xthis__ctor_ref ;

#line 1752 "../../src/simpl.c"
struct call *_au0__Xthis__ctor_call ;

#line 1752 "../../src/simpl.c"
struct estmt *_au0__Xthis__ctor_estmt ;

#line 1748 "../../src/simpl.c"
_au9_r = (struct ref *)( (_au0__Xthis__ctor_ref = 0 ), ( ( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ref ),
#line 1748 "../../src/simpl.c"
((unsigned char )45 ), ((struct expr *)_au6_n ), (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = _au8_d ), ((_au0__Xthis__ctor_ref )))
#line 1748 "../../src/simpl.c"
) ) ;
_au9_ee = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , zero , (struct expr *)0 ) ;
_au9_dl = (struct call *)( (_au0__Xthis__ctor_call = 0 ), ( ( (_au0__Xthis__ctor_call = 0 ), (_au0__Xthis__ctor_call = (struct call *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_call ),
#line 1750 "../../src/simpl.c"
(unsigned char )109 , ((struct expr *)_au9_r ), _au9_ee ) )) , ((_au0__Xthis__ctor_call ))) ) ;
_au9_dls = (struct stmt *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt = (struct estmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_estmt ),
#line 1751 "../../src/simpl.c"
((unsigned char )72 ), _au6_n -> _name_where , ((struct stmt *)0 )) )) , ( (_au0__Xthis__ctor_estmt -> _stmt__O8.__C8_e = ((struct expr *)_au9_dl )),
#line 1751 "../../src/simpl.c"
((_au0__Xthis__ctor_estmt ))) ) ) ;
_au9_dl -> _node_base = 146 ;
_au9_dl -> _expr__O5.__C5_fct_name = _au8_d ;
if (_au5_dd )
#line 1755 "../../src/simpl.c"
_au5_ddt -> _stmt_s_list = _au9_dls ;
else 
#line 1757 "../../src/simpl.c"
_au5_dd = _au9_dls ;
_au5_ddt = _au9_dls ;
}
}
else 
#line 1762 "../../src/simpl.c"
if (cl_obj_vec ){ 
#line 1763 "../../src/simpl.c"
Pclass _au8_cl ;

#line 1765 "../../src/simpl.c"
Pname _au8_d ;

#line 1763 "../../src/simpl.c"
_au8_cl = (((struct classdef *)cl_obj_vec -> _expr__O2.__C2_tp ));

#line 1765 "../../src/simpl.c"
_au8_d = ( _table_look ( _au8_cl -> _classdef_memtbl , "_dtor", (unsigned char )0 ) ) ;

#line 1767 "../../src/simpl.c"
if (_au8_d ){ 
#line 1768 "../../src/simpl.c"
Pexpr _au9_a ;
Pstmt _au9_dls ;

#line 1770 "../../src/simpl.c"
struct estmt *_au0__Xthis__ctor_estmt ;

#line 1768 "../../src/simpl.c"
_au9_a = cdvec ( vec_del_fct , (struct expr *)_au6_n , _au8_cl , _au8_d , (int )0 ) ;
_au9_dls = (struct stmt *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt = (struct estmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_estmt ),
#line 1769 "../../src/simpl.c"
((unsigned char )72 ), _au6_n -> _name_where , ((struct stmt *)0 )) )) , ( (_au0__Xthis__ctor_estmt -> _stmt__O8.__C8_e = _au9_a ), ((_au0__Xthis__ctor_estmt )))
#line 1769 "../../src/simpl.c"
) ) ;
if (_au5_dd )
#line 1771 "../../src/simpl.c"
_au5_ddt -> _stmt_s_list = _au9_dls ;
else 
#line 1773 "../../src/simpl.c"
_au5_dd = _au9_dls ;
_au5_ddt = _au9_dls ;
}
}
}
}
}

#line 1782 "../../src/simpl.c"
if (_au5_dd ){ 
#line 1783 "../../src/simpl.c"
_stmt_simpl ( _au5_dd ) ;
{ Pstmt _au6_bs ;

#line 1784 "../../src/simpl.c"
_au6_bs = (struct stmt *)_stmt__ctor ( (struct stmt *)0 , _au0_this -> _node_base , _au0_this -> _stmt_where , (struct stmt *)0 ) ;
(*_au6_bs )= (*_au0_this );
_au0_this -> _node_base = 166 ;
_au0_this -> _stmt_s = _au5_dd ;
_au0_this -> _stmt__O8.__C8_s2 = _au6_bs ;
}
}
}
}
}
}
}
}

#line 1793 "../../src/simpl.c"
break ;

#line 1795 "../../src/simpl.c"
case 20 : 
#line 1796 "../../src/simpl.c"
_expr_simpl ( _au0_this -> _stmt__O8.__C8_e ) ;
_stmt_simpl ( _au0_this -> _stmt_s ) ;
if (_au0_this -> _stmt__O9.__C9_else_stmt )_stmt_simpl ( _au0_this -> _stmt__O9.__C9_else_stmt ) ;
break ;

#line 1801 "../../src/simpl.c"
case 16 : 
#line 1802 "../../src/simpl.c"
if (_au0_this -> _stmt__O9.__C9_for_init ){ 
#line 1803 "../../src/simpl.c"
_stmt_simpl ( _au0_this -> _stmt__O9.__C9_for_init ) ;
}

#line 1809 "../../src/simpl.c"
if (_au0_this -> _stmt__O8.__C8_e )_expr_simpl ( _au0_this -> _stmt__O8.__C8_e ) ;
if (_au0_this -> _stmt__O7.__C7_e2 ){ 
#line 1811 "../../src/simpl.c"
curr_expr = _au0_this -> _stmt__O7.__C7_e2 ;
_expr_simpl ( _au0_this -> _stmt__O7.__C7_e2 ) ;
if ((_au0_this -> _stmt__O7.__C7_e2 -> _node_base == 168 )&& (_au0_this -> _stmt__O7.__C7_e2 -> _expr__O2.__C2_tp == (struct type *)void_type ))
#line 1814 "../../src/simpl.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"call of inline voidF in forE",
#line 1814 "../../src/simpl.c"
(struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
}
{ Pstmt _au3_obl ;
Pstmt _au3_ocl ;

#line 1816 "../../src/simpl.c"
_au3_obl = break_del_list ;
_au3_ocl = continue_del_list ;
break_del_list = 0 ;
continue_del_list = 0 ;
_stmt_simpl ( _au0_this -> _stmt_s ) ;
break_del_list = _au3_obl ;
continue_del_list = _au3_ocl ;
}
break ;

#line 1826 "../../src/simpl.c"
case 116 : 
#line 1827 "../../src/simpl.c"
_block_simpl ( ((struct block *)_au0_this )) ;
break ;

#line 1830 "../../src/simpl.c"
case 166 : 
#line 1831 "../../src/simpl.c"
break ;
} }

#line 1835 "../../src/simpl.c"
if ((_au0_this -> _node_base != 116 )&& _au0_this -> _stmt_memtbl ){ 
#line 1836 "../../src/simpl.c"
int _au2_i ;
Pstmt _au2_t1 ;
Pstmt _au2_tx ;

#line 1840 "../../src/simpl.c"
Pstmt _au2_ss ;
Pname _au2_cln ;

#line 1837 "../../src/simpl.c"
_au2_t1 = (_au0_this -> _stmt_s_list ? _stmt_simpl ( _au0_this -> _stmt_s_list ) : (((struct stmt *)0 )));
_au2_tx = (_au2_t1 ? _au2_t1 : _au0_this );

#line 1840 "../../src/simpl.c"
_au2_ss = 0 ;

#line 1842 "../../src/simpl.c"
{ Pname _au2_tn ;

#line 1842 "../../src/simpl.c"
_au2_tn = _table_get_mem ( _au0_this -> _stmt_memtbl , _au2_i = 1 ) ;

#line 1842 "../../src/simpl.c"
for(;_au2_tn ;_au2_tn = _table_get_mem ( _au0_this -> _stmt_memtbl , ++ _au2_i ) ) { 
#line 1843 "../../src/simpl.c"
if (_au2_cln = _type_is_cl_obj ( _au2_tn -> _expr__O2.__C2_tp ) ){
#line 1843 "../../src/simpl.c"

#line 1844 "../../src/simpl.c"
Pclass _au4_cl ;
Pname _au4_d ;

#line 1844 "../../src/simpl.c"
_au4_cl = (((struct classdef *)_au2_cln -> _expr__O2.__C2_tp ));
_au4_d = ( _table_look ( _au4_cl -> _classdef_memtbl , "_dtor", (unsigned char )0 ) ) ;
if (_au4_d ){ 
#line 1847 "../../src/simpl.c"
Pref _au5_r ;
Pexpr _au5_ee ;
Pcall _au5_dl ;
Pstmt _au5_dls ;

#line 1851 "../../src/simpl.c"
struct ref *_au0__Xthis__ctor_ref ;

#line 1851 "../../src/simpl.c"
struct call *_au0__Xthis__ctor_call ;

#line 1851 "../../src/simpl.c"
struct estmt *_au0__Xthis__ctor_estmt ;

#line 1847 "../../src/simpl.c"
_au5_r = (struct ref *)( (_au0__Xthis__ctor_ref = 0 ), ( ( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ref ),
#line 1847 "../../src/simpl.c"
((unsigned char )45 ), ((struct expr *)_au2_tn ), (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = _au4_d ), ((_au0__Xthis__ctor_ref )))
#line 1847 "../../src/simpl.c"
) ) ;
_au5_ee = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , zero , (struct expr *)0 ) ;
_au5_dl = (struct call *)( (_au0__Xthis__ctor_call = 0 ), ( ( (_au0__Xthis__ctor_call = 0 ), (_au0__Xthis__ctor_call = (struct call *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_call ),
#line 1849 "../../src/simpl.c"
(unsigned char )109 , ((struct expr *)_au5_r ), _au5_ee ) )) , ((_au0__Xthis__ctor_call ))) ) ;
_au5_dls = (struct stmt *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt = (struct estmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_estmt ),
#line 1850 "../../src/simpl.c"
((unsigned char )72 ), _au2_tn -> _name_where , ((struct stmt *)0 )) )) , ( (_au0__Xthis__ctor_estmt -> _stmt__O8.__C8_e = ((struct expr *)_au5_dl )),
#line 1850 "../../src/simpl.c"
((_au0__Xthis__ctor_estmt ))) ) ) ;
_au5_dl -> _node_base = 146 ;
_au5_dl -> _expr__O5.__C5_fct_name = _au4_d ;
_au5_dls -> _stmt_s_list = _au2_ss ;
_au2_ss = _au5_dls ;
}
}
}

#line 1859 "../../src/simpl.c"
if (_au2_ss ){ 
#line 1860 "../../src/simpl.c"
Pstmt _au3_t2 ;

#line 1860 "../../src/simpl.c"
_au3_t2 = _stmt_simpl ( _au2_ss ) ;
switch (_au0_this -> _node_base ){ 
#line 1862 "../../src/simpl.c"
case 20 : 
#line 1863 "../../src/simpl.c"
case 39 : 
#line 1864 "../../src/simpl.c"
case 10 : 
#line 1865 "../../src/simpl.c"
case 33 : 
#line 1866 "../../src/simpl.c"
temp_in_cond ( _au0_this -> _stmt__O8.__C8_e ,
#line 1866 "../../src/simpl.c"
_au2_ss , _au0_this -> _stmt_memtbl ) ;
break ;

#line 1869 "../../src/simpl.c"
case 166 : 
#line 1870 "../../src/simpl.c"
{ Pstmt _au5_ts ;

#line 1870 "../../src/simpl.c"
_au5_ts = _au0_this -> _stmt__O8.__C8_s2 ;
while (_au5_ts -> _node_base == 166 )_au5_ts = _au5_ts -> _stmt__O8.__C8_s2 ;
if (_au5_ts -> _node_base == 28 ){ 
#line 1873 "../../src/simpl.c"
_au0_this = _au5_ts ;
goto retu ;
}
goto def ;
}
case 28 : 
#line 1879 "../../src/simpl.c"
retu :
#line 1881 "../../src/simpl.c"
{ 
#line 1882 "../../src/simpl.c"
if (_au0_this -> _stmt__O8.__C8_e == 0 ){ 
#line 1884 "../../src/simpl.c"
Pstmt _au6_rs ;

#line 1885 "../../src/simpl.c"
struct estmt *_au0__Xthis__ctor_estmt ;

#line 1884 "../../src/simpl.c"
_au6_rs = (struct stmt *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt = (struct estmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_estmt ),
#line 1884 "../../src/simpl.c"
((unsigned char )28 ), _au0_this -> _stmt_where , ((struct stmt *)0 )) )) , ( (_au0__Xthis__ctor_estmt -> _stmt__O8.__C8_e = ((struct expr *)0 )),
#line 1884 "../../src/simpl.c"
((_au0__Xthis__ctor_estmt ))) ) ) ;
_au6_rs -> _stmt__O9.__C9_empty = _au0_this -> _stmt__O9.__C9_empty ;
_au6_rs -> _stmt__O7.__C7_ret_tp = _au0_this -> _stmt__O7.__C7_ret_tp ;
_au0_this -> _node_base = 166 ;
_au0_this -> _stmt_s = _au2_ss ;
_au0_this -> _stmt__O8.__C8_s2 = _au6_rs ;
return (_au2_t1 ? _au2_t1 : _au6_rs );
}
ccheck ( _au0_this -> _stmt__O8.__C8_e ) ;
{ Pname _au5_cln ;

#line 1893 "../../src/simpl.c"
_au5_cln = _type_is_cl_obj ( _au0_this -> _stmt__O8.__C8_e -> _expr__O2.__C2_tp ) ;

#line 1895 "../../src/simpl.c"
if ((_au5_cln == 0 )|| (_classdef_has_oper ( ((struct classdef *)_au5_cln -> _expr__O2.__C2_tp ), (unsigned char )70 ) == 0 )){ 
#line 1898 "../../src/simpl.c"
Pname _au6_rv ;

#line 1898 "../../src/simpl.c"
_au6_rv = (struct name *)_name__ctor ( (struct name *)0 , "_rresult") ;
_au6_rv -> _expr__O2.__C2_tp = _au0_this -> _stmt__O7.__C7_ret_tp ;
if (_au0_this -> _stmt_memtbl == 0 )_au0_this -> _stmt_memtbl = (struct table *)_table__ctor ( (struct table *)0 , (short )4 , (struct table *)0 , (struct
#line 1900 "../../src/simpl.c"
name *)0 ) ;
{ Pname _au6_n ;

#line 1901 "../../src/simpl.c"
_au6_n = _name_dcl ( _au6_rv , _au0_this -> _stmt_memtbl , (unsigned char )108 ) ;
_au6_n -> _name_n_assigned_to = 1 ;
_name__dtor ( _au6_rv , 1) ;
{ Pstmt _au6_rs ;

#line 1905 "../../src/simpl.c"
struct estmt *_au0__Xthis__ctor_estmt ;

#line 1904 "../../src/simpl.c"
_au6_rs = (struct stmt *)( (_au0__Xthis__ctor_estmt = 0 ), ( ( (_au0__Xthis__ctor_estmt = 0 ), (_au0__Xthis__ctor_estmt = (struct estmt *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_estmt ),
#line 1904 "../../src/simpl.c"
((unsigned char )28 ), _au0_this -> _stmt_where , ((struct stmt *)0 )) )) , ( (_au0__Xthis__ctor_estmt -> _stmt__O8.__C8_e = ((struct expr *)_au6_n )),
#line 1904 "../../src/simpl.c"
((_au0__Xthis__ctor_estmt ))) ) ) ;
_au6_rs -> _stmt__O7.__C7_ret_tp = _au0_this -> _stmt__O7.__C7_ret_tp ;
_au0_this -> _node_base = 72 ;
_au0_this -> _stmt__O8.__C8_e = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )70 , (struct expr *)_au6_n , _au0_this -> _stmt__O8.__C8_e ) ;
#line 1907 "../../src/simpl.c"

#line 1908 "../../src/simpl.c"
_au0_this -> _stmt__O8.__C8_e -> _expr__O2.__C2_tp = _au6_n -> _expr__O2.__C2_tp ;
{ Pstmt _au6_ps ;

#line 1910 "../../src/simpl.c"
struct pair *_au0__Xthis__ctor_pair ;

#line 1909 "../../src/simpl.c"
_au6_ps = (struct stmt *)( (_au0__Xthis__ctor_pair = 0 ), ( ( (_au0__Xthis__ctor_pair = 0 ), (_au0__Xthis__ctor_pair = (struct pair *)_stmt__ctor ( ((struct stmt *)_au0__Xthis__ctor_pair ),
#line 1909 "../../src/simpl.c"
(unsigned char )166 , _au0_this -> _stmt_where , _au2_ss ) )) , ( (_au0__Xthis__ctor_pair -> _stmt__O8.__C8_s2 = _au6_rs ), ((_au0__Xthis__ctor_pair ))) )
#line 1909 "../../src/simpl.c"
) ;
_au6_ps -> _stmt_s_list = _au0_this -> _stmt_s_list ;
_au0_this -> _stmt_s_list = _au6_ps ;
return (_au2_t1 ? _au2_t1 : _au6_rs );
}
}
}
}
}
}

#line 1916 "../../src/simpl.c"
case 16 : 
#line 1917 "../../src/simpl.c"
{ 
#line 1946 "../../src/simpl.c"
struct ea _au0__V33 ;

#line 1946 "../../src/simpl.c"
struct ea _au0__V34 ;

#line 1917 "../../src/simpl.c"
errorFI_PCloc__PC_RCea__RCea__RCea__RCea___ ( (int )'s' , & _au0_this -> _stmt_where , (char *)"E in %kS needs temporary ofC%nW destructor", (struct ea *)( ( ((& _au0__V33 )-> _ea__O1.__C1_i =
#line 1917 "../../src/simpl.c"
((int )_au0_this -> _node_base )), (((& _au0__V33 )))) ) , (struct ea *)( ( ((& _au0__V34 )-> _ea__O1.__C1_p = ((char
#line 1917 "../../src/simpl.c"
*)_au2_cln )), (((& _au0__V34 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 ) ;
break ;

#line 1920 "../../src/simpl.c"
case 72 : 
#line 1921 "../../src/simpl.c"
if (_au2_t1 ){ 
#line 1922 "../../src/simpl.c"
{ Pstmt _au5_ttt ;

#line 1922 "../../src/simpl.c"
Pstmt _au5_tt ;

#line 1922 "../../src/simpl.c"
_au5_tt = _au0_this ;

#line 1922 "../../src/simpl.c"
for(;(_au5_ttt = _au5_tt -> _stmt_s_list )&& (_au5_ttt -> _node_base == 72 );_au5_tt = _au5_ttt ) 
#line 1924 "../../src/simpl.c"
;
_au3_t2 -> _stmt_s_list = _au5_ttt ;
_au5_tt -> _stmt_s_list = _au2_ss ;
return ((_au2_t1 != _au5_tt )? _au2_t1 : _au3_t2 );
}
}

#line 1929 "../../src/simpl.c"
default : 
#line 1930 "../../src/simpl.c"
def :
#line 1932 "../../src/simpl.c"
if (_au0_this -> _stmt__O8.__C8_e )ccheck ( _au0_this -> _stmt__O8.__C8_e ) ;
if (_au2_t1 ){ 
#line 1934 "../../src/simpl.c"
_au3_t2 -> _stmt_s_list = _au0_this -> _stmt_s_list ;
_au0_this -> _stmt_s_list = _au2_ss ;
return _au2_t1 ;
}
_au0_this -> _stmt_s_list = _au2_ss ;
return _au3_t2 ;
} }
}
return (_au2_t1 ? _au2_t1 : _au0_this );
}
}
return (_au0_this -> _stmt_s_list ? _stmt_simpl ( _au0_this -> _stmt_s_list ) : _au0_this );
}
;
Pstmt _stmt_copy (_au0_this )
#line 651 "../../src/cfront.h"
struct stmt *_au0_this ;

#line 1951 "../../src/simpl.c"
{ 
#line 1952 "../../src/simpl.c"
Pstmt _au1_ns ;

#line 1952 "../../src/simpl.c"
_au1_ns = (struct stmt *)_stmt__ctor ( (struct stmt *)0 , (unsigned char )0 , curloc , (struct stmt *)0 ) ;

#line 1954 "../../src/simpl.c"
(*_au1_ns )= (*_au0_this );
if (_au0_this -> _stmt_s )_au1_ns -> _stmt_s = _stmt_copy ( _au0_this -> _stmt_s ) ;
if (_au0_this -> _stmt_s_list )_au1_ns -> _stmt_s_list = _stmt_copy ( _au0_this -> _stmt_s_list ) ;

#line 1958 "../../src/simpl.c"
switch (_au0_this -> _node_base ){ 
#line 1959 "../../src/simpl.c"
case 166 : 
#line 1960 "../../src/simpl.c"
_au1_ns -> _stmt__O8.__C8_s2 = _stmt_copy ( _au0_this -> _stmt__O8.__C8_s2 ) ;
break ;
}

#line 1964 "../../src/simpl.c"
return _au1_ns ;
}
;
char _expr_simpl_new (_au0_this )
#line 525 "../../src/cfront.h"
struct expr *_au0_this ;

#line 1971 "../../src/simpl.c"
{ 
#line 1972 "../../src/simpl.c"
Pname _au1_cln ;
Pname _au1_ctor ;
int _au1_sz ;

#line 1976 "../../src/simpl.c"
Pexpr _au1_var_expr ;
Pexpr _au1_const_expr ;
Ptype _au1_tt ;
Pexpr _au1_arg ;
Pexpr _au1_szof ;

#line 1981 "../../src/simpl.c"
struct texpr *_au0__Xthis__ctor_texpr ;

#line 1981 "../../src/simpl.c"
struct ival *_au0__Xthis__ctor_ival ;

#line 1974 "../../src/simpl.c"
_au1_sz = 1 ;

#line 1976 "../../src/simpl.c"
_au1_var_expr = 0 ;

#line 1978 "../../src/simpl.c"
_au1_tt = _au0_this -> _expr__O5.__C5_tp2 ;

#line 1982 "../../src/simpl.c"
if (_au1_cln = _type_is_cl_obj ( _au1_tt ) ){ 
#line 1983 "../../src/simpl.c"
Pclass _au2_cl ;

#line 1983 "../../src/simpl.c"
_au2_cl = (((struct classdef *)_au1_cln -> _expr__O2.__C2_tp ));
if (_au1_ctor = ( _table_look ( _au2_cl -> _classdef_memtbl , "_ctor", (unsigned char )0 ) ) ){ 
#line 1985 "../../src/simpl.c"
Pexpr _au3_p ;

#line 1994 "../../src/simpl.c"
Pcall _au3_c ;

#line 1995 "../../src/simpl.c"
struct ref *_au0__Xthis__ctor_ref ;

#line 1985 "../../src/simpl.c"
_au3_p = zero ;
if (_au1_ctor -> _expr__O5.__C5_n_table != _au2_cl -> _classdef_memtbl ){ 
#line 1988 "../../src/simpl.c"
Pexpr _au4_ce ;

#line 1989 "../../src/simpl.c"
struct texpr *_au0__Xthis__ctor_texpr ;

#line 1988 "../../src/simpl.c"
_au4_ce = (struct expr *)( (_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr = (struct texpr *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_texpr ),
#line 1988 "../../src/simpl.c"
((unsigned char )30 ), ((struct expr *)0 ), (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 = _au1_tt ), ((_au0__Xthis__ctor_texpr )))
#line 1988 "../../src/simpl.c"
) ) ;
_au4_ce -> _expr__O2.__C2_tp = (struct type *)uint_type ;
_au4_ce = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , _au4_ce , (struct expr *)0 ) ;
_au3_p = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )146 , (struct expr *)new_fct , _au4_ce ) ;
_au3_p -> _expr__O5.__C5_fct_name = new_fct ;
}
_au3_c = (((struct call *)_au0_this -> _expr__O3.__C3_e1 ));
_au3_c -> _expr__O3.__C3_e1 = (struct expr *)( (_au0__Xthis__ctor_ref = 0 ), ( ( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct
#line 1995 "../../src/simpl.c"
expr *)_au0__Xthis__ctor_ref ), ((unsigned char )44 ), _au3_p , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = (((struct name *)_au3_c ->
#line 1995 "../../src/simpl.c"
_expr__O3.__C3_e1 ))), ((_au0__Xthis__ctor_ref ))) ) ) ;

#line 1997 "../../src/simpl.c"
_call_simpl ( _au3_c ) ;
(*_au0_this )= (*(((struct expr *)_au3_c )));
return ;
}
}
else if (cl_obj_vec ){ 
#line 2003 "../../src/simpl.c"
Pclass _au2_cl ;

#line 2003 "../../src/simpl.c"
_au2_cl = (((struct classdef *)cl_obj_vec -> _expr__O2.__C2_tp ));
_au1_ctor = _classdef_has_ictor ( _au2_cl ) ;
if (_au1_ctor == 0 ){ 
#line 2006 "../../src/simpl.c"
if (( _table_look ( _au2_cl -> _classdef_memtbl , "_ctor", (unsigned char )0 ) ) ){
#line 2006 "../../src/simpl.c"

#line 2079 "../../src/simpl.c"
struct ea _au0__V35 ;

#line 2006 "../../src/simpl.c"
error ( (char *)"new %s[], no defaultK", (struct ea *)( ( ((& _au0__V35 )-> _ea__O1.__C1_p = ((char *)_au2_cl -> _classdef_string )), (((& _au0__V35 ))))
#line 2006 "../../src/simpl.c"
) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} cl_obj_vec = 0 ;
}
}

#line 2011 "../../src/simpl.c"
xxx :
#line 2012 "../../src/simpl.c"
switch (_au1_tt -> _node_base ){ 
#line 2013 "../../src/simpl.c"
case 97 : 
#line 2014 "../../src/simpl.c"
_au1_tt = (((struct basetype *)_au1_tt ))-> _basetype_b_name -> _expr__O2.__C2_tp ;
goto xxx ;

#line 2017 "../../src/simpl.c"
default : 
#line 2020 "../../src/simpl.c"
_au1_szof = (struct expr *)( (_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr = (struct texpr *)_expr__ctor (
#line 2020 "../../src/simpl.c"
((struct expr *)_au0__Xthis__ctor_texpr ), ((unsigned char )30 ), ((struct expr *)0 ), (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 =
#line 2020 "../../src/simpl.c"
_au1_tt ), ((_au0__Xthis__ctor_texpr ))) ) ) ;
_au1_szof -> _expr__O2.__C2_tp = (struct type *)uint_type ;
break ;

#line 2024 "../../src/simpl.c"
case 110 : 
#line 2025 "../../src/simpl.c"
{ Pvec _au3_v ;

#line 2025 "../../src/simpl.c"
_au3_v = (((struct vec *)_au1_tt ));
if (_au3_v -> _vec_size )
#line 2027 "../../src/simpl.c"
_au1_sz *= _au3_v -> _vec_size ;
else if (_au3_v -> _vec_dim )
#line 2029 "../../src/simpl.c"
_au1_var_expr = (_au1_var_expr ? _expr__ctor ( (struct expr *)0 , (unsigned char )50 , _au1_var_expr , _au3_v -> _vec_dim )
#line 2029 "../../src/simpl.c"
: _au3_v -> _vec_dim );
else { 
#line 2031 "../../src/simpl.c"
_au1_sz = SZ_WPTR ;
break ;
}
_au1_tt = _au3_v -> _pvtyp_typ ;
goto xxx ;
}
}

#line 2039 "../../src/simpl.c"
if (cl_obj_vec ){ 
#line 2040 "../../src/simpl.c"
_au1_const_expr = (struct expr *)( (_au0__Xthis__ctor_ival = 0 ), ( ( (_au0__Xthis__ctor_ival = 0 ), (_au0__Xthis__ctor_ival = (struct ival *)_expr__ctor (
#line 2040 "../../src/simpl.c"
((struct expr *)_au0__Xthis__ctor_ival ), (unsigned char )150 , (struct expr *)0 , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ival -> _expr__O3.__C3_i1 =
#line 2040 "../../src/simpl.c"
_au1_sz ), ((_au0__Xthis__ctor_ival ))) ) ) ;
{ Pexpr _au2_noe ;

#line 2041 "../../src/simpl.c"
_au2_noe = (_au1_var_expr ? ((_au1_sz != 1 )? _expr__ctor ( (struct expr *)0 , (unsigned char )50 , _au1_const_expr , _au1_var_expr ) : _au1_var_expr ):
#line 2041 "../../src/simpl.c"
_au1_const_expr );

#line 2043 "../../src/simpl.c"
_au1_const_expr = _au1_szof ;
_au1_const_expr -> _expr__O2.__C2_tp = (struct type *)uint_type ;
_au0_this -> _node_base = 109 ;
_au1_arg = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , (struct expr *)_au1_ctor , (struct expr *)0 ) ;

#line 2048 "../../src/simpl.c"
_expr_lval ( (struct expr *)_au1_ctor , (unsigned char )112 ) ;
_au1_arg = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , _au1_const_expr , _au1_arg ) ;
_au1_arg = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , _au2_noe , _au1_arg ) ;
_au1_arg = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , zero , _au1_arg ) ;
_au0_this -> _node_base = 113 ;
_au0_this -> _expr__O5.__C5_tp2 = _au0_this -> _expr__O2.__C2_tp ;
_au0_this -> _expr__O3.__C3_e1 = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )146 , (struct expr *)vec_new_fct , _au1_arg ) ;
_au0_this -> _expr__O3.__C3_e1 -> _expr__O5.__C5_fct_name = vec_new_fct ;
_expr_simpl ( _au0_this ) ;
return ;
}
}

#line 2062 "../../src/simpl.c"
if (_au1_sz == 1 )
#line 2063 "../../src/simpl.c"
_au1_arg = (_au1_var_expr ? _expr__ctor ( (struct expr *)0 , (unsigned char )50 , _au1_szof , _au1_var_expr ) : _au1_szof );
#line 2063 "../../src/simpl.c"
else 
#line 2064 "../../src/simpl.c"
{ 
#line 2065 "../../src/simpl.c"
_au1_const_expr = (struct expr *)( (_au0__Xthis__ctor_ival = 0 ), ( ( (_au0__Xthis__ctor_ival = 0 ), (_au0__Xthis__ctor_ival = (struct ival *)_expr__ctor (
#line 2065 "../../src/simpl.c"
((struct expr *)_au0__Xthis__ctor_ival ), (unsigned char )150 , (struct expr *)0 , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ival -> _expr__O3.__C3_i1 =
#line 2065 "../../src/simpl.c"
_au1_sz ), ((_au0__Xthis__ctor_ival ))) ) ) ;
_au1_const_expr -> _expr__O2.__C2_tp = (struct type *)uint_type ;
_au1_const_expr = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )50 , _au1_const_expr , _au1_szof ) ;
_au1_const_expr -> _expr__O2.__C2_tp = (struct type *)uint_type ;
_au1_arg = (_au1_var_expr ? _expr__ctor ( (struct expr *)0 , (unsigned char )50 , _au1_const_expr , _au1_var_expr ) : _au1_const_expr );
}

#line 2072 "../../src/simpl.c"
_au1_arg -> _expr__O2.__C2_tp = (struct type *)uint_type ;

#line 2074 "../../src/simpl.c"
_au0_this -> _node_base = 113 ;
_au0_this -> _expr__O5.__C5_tp2 = _au0_this -> _expr__O2.__C2_tp ;
_au0_this -> _expr__O3.__C3_e1 = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )146 , (struct expr *)new_fct , (struct expr *)_expr__ctor ( (struct
#line 2076 "../../src/simpl.c"
expr *)0 , (unsigned char )140 , _au1_arg , (struct expr *)0 ) ) ;
_au0_this -> _expr__O3.__C3_e1 -> _expr__O5.__C5_fct_name = new_fct ;
_expr_simpl ( _au0_this ) ;
}
;
char _expr_simpl_delete (_au0_this )
#line 525 "../../src/cfront.h"
struct expr *_au0_this ;

#line 2088 "../../src/simpl.c"
{ 
#line 2089 "../../src/simpl.c"
Ptype _au1_tt ;

#line 2089 "../../src/simpl.c"
_au1_tt = _au0_this -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp ;

#line 2091 "../../src/simpl.c"
ttloop :
#line 2092 "../../src/simpl.c"
switch (_au1_tt -> _node_base ){ 
#line 2093 "../../src/simpl.c"
case 97 : _au1_tt = (((struct basetype *)_au1_tt ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 2093 "../../src/simpl.c"
goto ttloop ;
case 110 : 
#line 2095 "../../src/simpl.c"
case 125 : _au1_tt = (((struct ptr *)_au1_tt ))-> _pvtyp_typ ;

#line 2095 "../../src/simpl.c"
break ;
}

#line 2098 "../../src/simpl.c"
{ Pname _au1_cln ;
Pclass _au1_cl ;
Pname _au1_n ;

#line 2098 "../../src/simpl.c"
_au1_cln = _type_is_cl_obj ( _au1_tt ) ;

#line 2102 "../../src/simpl.c"
if (_au1_cln && (_au1_n = ( _table_look ( (((struct classdef *)_au1_cln -> _expr__O2.__C2_tp ))-> _classdef_memtbl , "_dtor", (unsigned char )0 ) )
#line 2102 "../../src/simpl.c"
)){ 
#line 2103 "../../src/simpl.c"
Pexpr _au2_r ;

#line 2103 "../../src/simpl.c"
_au2_r = _au0_this -> _expr__O3.__C3_e1 ;
if (_au0_this -> _expr__O4.__C4_e2 == 0 ){ 
#line 2105 "../../src/simpl.c"
Pexpr _au3_rrr ;
Pexpr _au3_aaa ;
Pexpr _au3_ee ;

#line 2108 "../../src/simpl.c"
struct ref *_au0__Xthis__ctor_ref ;

#line 2108 "../../src/simpl.c"
struct call *_au0__Xthis__ctor_call ;

#line 2105 "../../src/simpl.c"
_au3_rrr = (struct expr *)( (_au0__Xthis__ctor_ref = 0 ), ( ( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ref ),
#line 2105 "../../src/simpl.c"
((unsigned char )44 ), _au2_r , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = _au1_n ), ((_au0__Xthis__ctor_ref ))) )
#line 2105 "../../src/simpl.c"
) ;
_au3_aaa = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , one , (struct expr *)0 ) ;
_au3_ee = (struct expr *)( (_au0__Xthis__ctor_call = 0 ), ( ( (_au0__Xthis__ctor_call = 0 ), (_au0__Xthis__ctor_call = (struct call *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_call ),
#line 2107 "../../src/simpl.c"
(unsigned char )109 , _au3_rrr , _au3_aaa ) )) , ((_au0__Xthis__ctor_call ))) ) ;
_au3_ee -> _expr__O5.__C5_fct_name = (((struct name *)_au1_n ));
_au3_ee -> _node_base = 146 ;
if ((((struct fct *)_au1_n -> _expr__O2.__C2_tp ))-> _fct_f_virtual ){ 
#line 2111 "../../src/simpl.c"
_au3_ee = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )68 , _au3_ee ,
#line 2111 "../../src/simpl.c"
zero ) ;
_au3_ee -> _expr__O2.__C2_tp = _au3_ee -> _expr__O3.__C3_e1 -> _expr__O2.__C2_tp ;
_au3_ee -> _expr__O5.__C5_cond = _au2_r ;
}
(*_au0_this )= (*_au3_ee );
_expr__dtor ( _au3_ee , 1) ;
_expr_simpl ( _au0_this ) ;
return ;
}
else { 
#line 2121 "../../src/simpl.c"
Pexpr _au3_sz ;

#line 2122 "../../src/simpl.c"
struct texpr *_au0__Xthis__ctor_texpr ;

#line 2121 "../../src/simpl.c"
_au3_sz = (struct expr *)( (_au0__Xthis__ctor_texpr = 0 ), ( ( (_au0__Xthis__ctor_texpr = 0 ), (_au0__Xthis__ctor_texpr = (struct texpr *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_texpr ),
#line 2121 "../../src/simpl.c"
((unsigned char )30 ), ((struct expr *)0 ), (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_texpr -> _expr__O5.__C5_tp2 = _au1_tt ), ((_au0__Xthis__ctor_texpr )))
#line 2121 "../../src/simpl.c"
) ) ;
_au3_sz -> _expr__O2.__C2_tp = (struct type *)uint_type ;
{ Pexpr _au3_arg ;

#line 2123 "../../src/simpl.c"
_au3_arg = one ;
if ((((struct fct *)_au1_n -> _expr__O2.__C2_tp ))-> _fct_f_virtual ){ 
#line 2126 "../../src/simpl.c"
if (_au0_this -> _expr__O3.__C3_e1 -> _node_base != 85 )errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"PE too complicated for delete[]",
#line 2126 "../../src/simpl.c"
(struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
{ Pexpr _au4_a ;

#line 2128 "../../src/simpl.c"
struct ref *_au0__Xthis__ctor_ref ;

#line 2127 "../../src/simpl.c"
_au4_a = (struct expr *)( (_au0__Xthis__ctor_ref = 0 ), ( ( (_au0__Xthis__ctor_ref = 0 ), (_au0__Xthis__ctor_ref = (struct ref *)_expr__ctor ( ((struct expr *)_au0__Xthis__ctor_ref ),
#line 2127 "../../src/simpl.c"
((unsigned char )44 ), _au0_this -> _expr__O3.__C3_e1 , (struct expr *)0 ) )) , ( (_au0__Xthis__ctor_ref -> _expr__O5.__C5_mem = _au1_n ), ((_au0__Xthis__ctor_ref )))
#line 2127 "../../src/simpl.c"
) ) ;
_au4_a = _expr_address ( _au4_a ) ;
_au3_arg = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , _au4_a , _au3_arg ) ;
}
}
else 
#line 2131 "../../src/simpl.c"
{ 
#line 2132 "../../src/simpl.c"
_au3_arg = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , (struct expr *)_au1_n , _au3_arg ) ;
#line 2132 "../../src/simpl.c"

#line 2133 "../../src/simpl.c"
_expr_lval ( (struct expr *)_au1_n , (unsigned char )112 ) ;
}
_au3_arg = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , _au3_sz , _au3_arg ) ;
_au3_arg = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , _au0_this -> _expr__O4.__C4_e2 , _au3_arg ) ;
_au3_arg = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , _au0_this -> _expr__O3.__C3_e1 , _au3_arg ) ;
_au0_this -> _node_base = 146 ;
_au0_this -> _expr__O3.__C3_e1 = (struct expr *)vec_del_fct ;
_au0_this -> _expr__O4.__C4_e2 = _au3_arg ;
_au0_this -> _expr__O5.__C5_fct_name = vec_del_fct ;
}
}
}
else 
#line 2144 "../../src/simpl.c"
if (cl_obj_vec ){ 
#line 2145 "../../src/simpl.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"delete vector of vectors", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ,
#line 2145 "../../src/simpl.c"
(struct ea *)ea0 ) ;
}
else { 
#line 2148 "../../src/simpl.c"
_au0_this -> _node_base = 146 ;
_au0_this -> _expr__O4.__C4_e2 = (struct expr *)_expr__ctor ( (struct expr *)0 , (unsigned char )140 , _au0_this -> _expr__O3.__C3_e1 , (struct expr *)0 ) ;
#line 2149 "../../src/simpl.c"

#line 2150 "../../src/simpl.c"
_au0_this -> _expr__O3.__C3_e1 = (struct expr *)(_au0_this -> _expr__O5.__C5_fct_name = del_fct );
}

#line 2153 "../../src/simpl.c"
_call_simpl ( ((struct call *)_au0_this )) ;
}
}
;

/* the end */
