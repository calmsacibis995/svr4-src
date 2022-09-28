/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#line 1 "../../src/norm2.c"

/* <<cfront 1.2.1 2/16/87>> */
/* < ../../src/norm2.c */

#ident	"@(#)sdb:cfront/scratch/src/norm2..c	1.2"
char *_new(); char _delete(); char *_vec_new(); char _vec_delete();

#line 1 "../../src/norm2.c"

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

#line 23 "../../src/norm2.c"

#line 24 "../../src/norm2.c"

#line 25 "../../src/norm2.c"

#line 27 "../../src/norm2.c"
struct fct *_fct__ctor (_au0_this , _au0_t , _au0_arg , _au0_known )
#line 400 "../../src/cfront.h"
struct fct *_au0_this ;

#line 27 "../../src/norm2.c"
Ptype _au0_t ;

#line 27 "../../src/norm2.c"
Pname _au0_arg ;

#line 27 "../../src/norm2.c"
TOK _au0_known ;
{ 
#line 38 "../../src/norm2.c"
register Pname _au1_n ;

#line 28 "../../src/norm2.c"
if (_au0_this == 0 )_au0_this = (struct fct *)_new ( (long )(sizeof (struct fct))) ;
Nt ++ ;
_au0_this -> _node_base = 108 ;
_au0_this -> _fct_nargs_known = _au0_known ;
_au0_this -> _fct_returns = _au0_t ;
_au0_this -> _fct_argtype = _au0_arg ;

#line 36 "../../src/norm2.c"
if ((_au0_arg == 0 )|| (_au0_arg -> _node_base == 140 ))return _au0_this ;

#line 38 "../../src/norm2.c"
;
for(_au1_n = _au0_arg ;_au1_n ;_au1_n = _au1_n -> _name_n_list ) { 
#line 40 "../../src/norm2.c"
switch (_au1_n -> _expr__O2.__C2_tp -> _node_base ){ 
#line 41 "../../src/norm2.c"
case 38 : 
#line 42 "../../src/norm2.c"
_au0_this -> _fct_argtype = 0 ;
#line 42 "../../src/norm2.c"

#line 43 "../../src/norm2.c"
_au0_this -> _fct_nargs = 0 ;
_au0_this -> _fct_nargs_known = 1 ;
if (_au1_n -> _expr__O3.__C3_string )
#line 46 "../../src/norm2.c"
{ 
#line 59 "../../src/norm2.c"
struct ea _au0__V10 ;

#line 46 "../../src/norm2.c"
error ( (char *)"voidFA%n", (struct ea *)( ( ((& _au0__V10 )-> _ea__O1.__C1_p = ((char *)_au1_n )), (((& _au0__V10 )))) )
#line 46 "../../src/norm2.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} else if (_au0_this -> _fct_nargs || _au1_n -> _name_n_list ){ 
#line 48 "../../src/norm2.c"
error ( (char *)"voidFA", (struct ea *)ea0 , (struct ea *)ea0 ,
#line 48 "../../src/norm2.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
_au0_this -> _fct_nargs_known = 0 ;
}
break ;
case 6 : 
#line 53 "../../src/norm2.c"
case 13 : 
#line 54 "../../src/norm2.c"
break ;
default : 
#line 56 "../../src/norm2.c"
_au0_this -> _fct_nargs ++ ;
}
}
return _au0_this ;
}
;

#line 61 "../../src/norm2.c"
Pexpr expr_free ;

#line 63 "../../src/norm2.c"
struct expr *_expr__ctor (_au0_this , _au0_ba , _au0_a , _au0_b )
#line 525 "../../src/cfront.h"
struct expr *_au0_this ;

#line 63 "../../src/norm2.c"
TOK _au0_ba ;

#line 63 "../../src/norm2.c"
Pexpr _au0_a ;

#line 63 "../../src/norm2.c"
Pexpr _au0_b ;
{ 
#line 65 "../../src/norm2.c"
register Pexpr _au1_p ;

#line 67 "../../src/norm2.c"
if (_au0_this )goto ret ;

#line 69 "../../src/norm2.c"
if ((_au1_p = expr_free )== 0 ){ 
#line 70 "../../src/norm2.c"
register Pexpr _au2_q ;

#line 70 "../../src/norm2.c"
_au2_q = (((struct expr *)chunk ( 1 ) ));
for(_au1_p = (expr_free = (& (_au2_q [407])));_au2_q < _au1_p ;_au1_p -- ) _au1_p -> _expr__O3.__C3_e1 = (_au1_p - 1 );
(_au1_p + 1 )-> _expr__O3.__C3_e1 = 0 ;
}
else 
#line 75 "../../src/norm2.c"
expr_free = _au1_p -> _expr__O3.__C3_e1 ;

#line 77 "../../src/norm2.c"
_au0_this = _au1_p ;

#line 79 "../../src/norm2.c"
_au0_this -> _node_permanent = 0 ;
_au0_this -> _expr__O2.__C2_tp = 0 ;
_au0_this -> _expr__O5.__C5_tp2 = 0 ;

#line 83 "../../src/norm2.c"
Ne ++ ;
ret :
#line 85 "../../src/norm2.c"
_au0_this -> _node_base = _au0_ba ;
_au0_this -> _expr__O3.__C3_e1 = _au0_a ;
_au0_this -> _expr__O4.__C4_e2 = _au0_b ;
return _au0_this ;
}
;

#line 90 "../../src/norm2.c"
char _expr__dtor (_au0_this , _au0__free )
#line 525 "../../src/cfront.h"
struct expr *_au0_this ;

#line 96 "../../src/norm2.c"
int _au0__free ;

#line 91 "../../src/norm2.c"
{ if (_au0_this ){ 
#line 92 "../../src/norm2.c"
NFe ++ ;
_au0_this -> _expr__O3.__C3_e1 = expr_free ;
expr_free = _au0_this ;
_au0_this = 0 ;
if (_au0_this )if (_au0__free )_delete ( (char *)_au0_this ) ;
} }
;

#line 98 "../../src/norm2.c"
Pstmt stmt_free ;

#line 100 "../../src/norm2.c"
struct stmt *_stmt__ctor (_au0_this , _au0_ba , _au0_ll , _au0_a )
#line 651 "../../src/cfront.h"
struct stmt *_au0_this ;

#line 100 "../../src/norm2.c"
TOK _au0_ba ;

#line 100 "../../src/norm2.c"
struct loc _au0_ll ;

#line 100 "../../src/norm2.c"
Pstmt _au0_a ;
{ 
#line 102 "../../src/norm2.c"
register Pstmt _au1_p ;

#line 104 "../../src/norm2.c"
if ((_au1_p = stmt_free )== 0 ){ 
#line 105 "../../src/norm2.c"
register Pstmt _au2_q ;

#line 105 "../../src/norm2.c"
_au2_q = (((struct stmt *)chunk ( 1 ) ));
for(_au1_p = (stmt_free = (& (_au2_q [253])));_au2_q < _au1_p ;_au1_p -- ) _au1_p -> _stmt_s_list = (_au1_p - 1 );
(_au1_p + 1 )-> _stmt_s_list = 0 ;
}
else 
#line 110 "../../src/norm2.c"
stmt_free = _au1_p -> _stmt_s_list ;

#line 112 "../../src/norm2.c"
_au0_this = _au1_p ;

#line 114 "../../src/norm2.c"
_au0_this -> _node_permanent = 0 ;
_au0_this -> _stmt__O8.__C8_e = (_au0_this -> _stmt__O7.__C7_e2 = 0 );
_au0_this -> _stmt_memtbl = 0 ;
_au0_this -> _stmt__O9.__C9_else_stmt = 0 ;
_au0_this -> _stmt_s_list = 0 ;

#line 120 "../../src/norm2.c"
Ns ++ ;
_au0_this -> _node_base = _au0_ba ;
_au0_this -> _stmt_where = _au0_ll ;
_au0_this -> _stmt_s = _au0_a ;
return _au0_this ;
}
;

#line 126 "../../src/norm2.c"
char _stmt__dtor (_au0_this , _au0__free )
#line 651 "../../src/cfront.h"
struct stmt *_au0_this ;

#line 132 "../../src/norm2.c"
int _au0__free ;

#line 127 "../../src/norm2.c"
{ if (_au0_this ){ 
#line 128 "../../src/norm2.c"
NFs ++ ;
_au0_this -> _stmt_s_list = stmt_free ;
stmt_free = _au0_this ;
_au0_this = 0 ;
if (_au0_this )if (_au0__free )_delete ( (char *)_au0_this ) ;
} }
;

#line 134 "../../src/norm2.c"
struct classdef *_classdef__ctor (_au0_this , _au0_b )
#line 319 "../../src/cfront.h"
struct classdef *_au0_this ;

#line 134 "../../src/norm2.c"
TOK _au0_b ;
{ if (_au0_this == 0 )_au0_this = (struct classdef *)_new ( (long )(sizeof (struct classdef))) ;
_au0_this -> _node_base = 6 ;
_au0_this -> _classdef_csu = _au0_b ;
_au0_this -> _classdef_memtbl = (struct table *)_table__ctor ( (struct table *)0 , (short )12 , (struct table *)0 , (struct name *)0 ) ;
return _au0_this ;
}
;

#line 141 "../../src/norm2.c"
struct basetype *_basetype__ctor (_au0_this , _au0_b , _au0_n )
#line 362 "../../src/cfront.h"
struct basetype *_au0_this ;

#line 141 "../../src/norm2.c"
TOK _au0_b ;

#line 141 "../../src/norm2.c"
Pname _au0_n ;
{ if (_au0_this == 0 )_au0_this = (struct basetype *)_new ( (long )(sizeof (struct basetype))) ;
Nbt ++ ;
switch (_au0_b ){ 
#line 145 "../../src/norm2.c"
case 0 : break ;
case 35 : _au0_this -> _basetype_b_typedef = 1 ;

#line 146 "../../src/norm2.c"
break ;
case 75 : _au0_this -> _basetype_b_inline = 1 ;

#line 147 "../../src/norm2.c"
break ;
case 77 : _au0_this -> _basetype_b_virtual = 1 ;

#line 148 "../../src/norm2.c"
break ;
case 26 : _au0_this -> _basetype_b_const = 1 ;

#line 149 "../../src/norm2.c"
break ;
case 37 : _au0_this -> _basetype_b_unsigned = 1 ;

#line 150 "../../src/norm2.c"
break ;
case 18 : 
#line 152 "../../src/norm2.c"
case 76 : 
#line 153 "../../src/norm2.c"
case 14 : 
#line 154 "../../src/norm2.c"
case 31 : 
#line 155 "../../src/norm2.c"
case 2 : 
#line 156 "../../src/norm2.c"
case 27 : _au0_this -> _basetype_b_sto =
#line 156 "../../src/norm2.c"
_au0_b ;

#line 156 "../../src/norm2.c"
break ;
case 29 : _au0_this -> _basetype_b_short = 1 ;

#line 157 "../../src/norm2.c"
break ;
case 22 : _au0_this -> _basetype_b_long = 1 ;

#line 158 "../../src/norm2.c"
break ;
case 141 : 
#line 160 "../../src/norm2.c"
case 138 : 
#line 161 "../../src/norm2.c"
case 38 : 
#line 162 "../../src/norm2.c"
case 5 : 
#line 163 "../../src/norm2.c"
case 21 : 
#line 164 "../../src/norm2.c"
case 15 : 
#line 165 "../../src/norm2.c"
case 11 :
#line 165 "../../src/norm2.c"
_au0_this -> _node_base = _au0_b ;

#line 165 "../../src/norm2.c"
break ;
case 97 : 
#line 167 "../../src/norm2.c"
case 119 : 
#line 168 "../../src/norm2.c"
case 121 : 
#line 169 "../../src/norm2.c"
case 114 : 
#line 170 "../../src/norm2.c"
case 1 : 
#line 171 "../../src/norm2.c"
_au0_this -> _node_base = _au0_b ;
_au0_this -> _basetype_b_name = _au0_n ;
break ;
case 171 : 
#line 175 "../../src/norm2.c"
case 170 : 
#line 176 "../../src/norm2.c"
{ 
#line 181 "../../src/norm2.c"
struct ea _au0__V11 ;

#line 176 "../../src/norm2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"\"%k\" not implemented (ignored)", (struct ea *)( ( ((& _au0__V11 )-> _ea__O1.__C1_i = ((int )_au0_b )), (((&
#line 176 "../../src/norm2.c"
_au0__V11 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
break ;
default : 
#line 179 "../../src/norm2.c"
{ 
#line 181 "../../src/norm2.c"
struct ea _au0__V12 ;

#line 179 "../../src/norm2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"badBT:%k", (struct ea *)( ( ((& _au0__V12 )-> _ea__O1.__C1_i = ((int )_au0_b )), (((&
#line 179 "../../src/norm2.c"
_au0__V12 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} } }
return _au0_this ;
}
;

#line 183 "../../src/norm2.c"
Pname name_free ;

#line 185 "../../src/norm2.c"
struct name *_name__ctor (_au0_this , _au0_s )
#line 607 "../../src/cfront.h"
struct name *_au0_this ;

#line 185 "../../src/norm2.c"
char *_au0_s ;
{ 
#line 187 "../../src/norm2.c"
register Pname _au1_p ;

#line 189 "../../src/norm2.c"
if ((_au1_p = name_free )== 0 ){ 
#line 190 "../../src/norm2.c"
register Pname _au2_q ;

#line 190 "../../src/norm2.c"
_au2_q = (((struct name *)chunk ( 1 ) ));
for(_au1_p = (name_free = (& (_au2_q [134])));_au2_q < _au1_p ;_au1_p -- ) _au1_p -> _name_n_tbl_list = (_au1_p - 1 );
(_au1_p + 1 )-> _name_n_tbl_list = 0 ;
}
else 
#line 195 "../../src/norm2.c"
name_free = _au1_p -> _name_n_tbl_list ;

#line 197 "../../src/norm2.c"
( (_au0_this = _au1_p ), (_au0_this = (struct name *)_expr__ctor ( ((struct expr *)_au0_this ), (unsigned char )85 , (struct expr *)0 , (struct
#line 197 "../../src/norm2.c"
expr *)0 ) )) ;

#line 200 "../../src/norm2.c"
Nn ++ ;
_au0_this -> _expr__O3.__C3_string = _au0_s ;
_au0_this -> _name_where = curloc ;
_au0_this -> _name_lex_level = bl_level ;

#line 206 "../../src/norm2.c"
_au0_this -> _expr__O2.__C2_tp = 0 ;
_au0_this -> _expr__O4.__C4_n_initializer = 0 ;
_au0_this -> _expr__O5.__C5_n_table = 0 ;
_au0_this -> _name_n_oper = 0 ;
_au0_this -> _name_n_sto = 0 ;
_au0_this -> _name_n_stclass = 0 ;
_au0_this -> _name_n_scope = 0 ;
_au0_this -> _name_n_union = 0 ;
_au0_this -> _name_n_evaluated = 0 ;
_au0_this -> _name_n_xref = 0 ;
_au0_this -> _name_n_protect = 0 ;
_au0_this -> _name_n_addr_taken = 0 ;
_au0_this -> _name_n_used = 0 ;
_au0_this -> _name_n_assigned_to = 0 ;
_au0_this -> _name_n_val = 0 ;
_au0_this -> _name_n_offset = 0 ;
_au0_this -> _name_n_list = 0 ;
_au0_this -> _name_n_tbl_list = 0 ;
_au0_this -> _name__O6.__C6_n_qualifier = 0 ;
return _au0_this ;
}
;

#line 227 "../../src/norm2.c"
char _name__dtor (_au0_this , _au0__free )
#line 607 "../../src/cfront.h"
struct name *_au0_this ;

#line 234 "../../src/norm2.c"
int _au0__free ;

#line 228 "../../src/norm2.c"
{ if (_au0_this ){ 
#line 229 "../../src/norm2.c"
NFn ++ ;

#line 231 "../../src/norm2.c"
_au0_this -> _name_n_tbl_list = name_free ;
name_free = _au0_this ;
_au0_this = 0 ;
if (_au0_this )_expr__dtor ( (struct expr *)_au0_this , _au0__free ) ;
} }
;
struct nlist *_nlist__ctor (_au0_this , _au0_n )
#line 714 "../../src/cfront.h"
struct nlist *_au0_this ;

#line 237 "../../src/norm2.c"
Pname _au0_n ;
{ 
#line 242 "../../src/norm2.c"
Pname _au1_nn ;

#line 238 "../../src/norm2.c"
if (_au0_this == 0 )_au0_this = (struct nlist *)_new ( (long )(sizeof (struct nlist))) ;

#line 241 "../../src/norm2.c"
_au0_this -> _nlist_head = _au0_n ;
for(_au1_nn = _au0_n ;_au1_nn -> _name_n_list ;_au1_nn = _au1_nn -> _name_n_list ) ;
_au0_this -> _nlist_tail = _au1_nn ;
Nl ++ ;
return _au0_this ;
}
;

#line 247 "../../src/norm2.c"
char _nlist_add_list (_au0_this , _au0_n )
#line 714 "../../src/cfront.h"
struct nlist *_au0_this ;

#line 247 "../../src/norm2.c"
Pname _au0_n ;
{ 
#line 252 "../../src/norm2.c"
Pname _au1_nn ;

#line 249 "../../src/norm2.c"
if (_au0_n -> _expr__O2.__C2_tp && (_au0_n -> _expr__O2.__C2_tp -> _type_defined & 010 ))return ;

#line 251 "../../src/norm2.c"
_au0_this -> _nlist_tail -> _name_n_list = _au0_n ;
for(_au1_nn = _au0_n ;_au1_nn -> _name_n_list ;_au1_nn = _au1_nn -> _name_n_list ) ;
_au0_this -> _nlist_tail = _au1_nn ;
}
;
int NFl = 0 ;

#line 258 "../../src/norm2.c"
extern Pname name_unlist (_au0_l )Pnlist _au0_l ;
{ 
#line 261 "../../src/norm2.c"
Pname _au1_n ;

#line 260 "../../src/norm2.c"
if (_au0_l == 0 )return (struct name *)0 ;
_au1_n = _au0_l -> _nlist_head ;
NFl ++ ;

#line 264 "../../src/norm2.c"
_delete ( (char *)_au0_l ) ;
return _au1_n ;
}
;
extern Pstmt stmt_unlist (_au0_l )Pslist _au0_l ;
{ 
#line 271 "../../src/norm2.c"
Pstmt _au1_s ;

#line 270 "../../src/norm2.c"
if (_au0_l == 0 )return (struct stmt *)0 ;
_au1_s = _au0_l -> _slist_head ;
NFl ++ ;

#line 274 "../../src/norm2.c"
_delete ( (char *)_au0_l ) ;
return _au1_s ;
}
;
extern Pexpr expr_unlist (_au0_l )Pelist _au0_l ;
{ 
#line 281 "../../src/norm2.c"
Pexpr _au1_e ;

#line 280 "../../src/norm2.c"
if (_au0_l == 0 )return (struct expr *)0 ;
_au1_e = _au0_l -> _elist_head ;
NFl ++ ;

#line 284 "../../src/norm2.c"
_delete ( (char *)_au0_l ) ;
return _au1_e ;
}
;
char sig_name (_au0_n )Pname _au0_n ;
{ 
#line 290 "../../src/norm2.c"
static char _static_buf [256];

#line 293 "../../src/norm2.c"
char *_au1_p ;

#line 291 "../../src/norm2.c"
(_static_buf [0 ])= '_' ;
(_static_buf [1 ])= 'O' ;
_au1_p = _type_signature ( _au0_n -> _expr__O2.__C2_tp , _static_buf + 2 ) ;
if (255 < (_au1_p - _static_buf ))errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"sig_name():N buffer overflow", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ,
#line 294 "../../src/norm2.c"
(struct ea *)ea0 ) ;
_au0_n -> _expr__O3.__C3_string = _static_buf ;
_au0_n -> _expr__O2.__C2_tp = 0 ;
}
;
Ptype tok_to_type (_au0_b )TOK _au0_b ;
{ 
#line 301 "../../src/norm2.c"
Ptype _au1_t ;
switch (_au0_b ){ 
#line 303 "../../src/norm2.c"
case 5 : _au1_t = (struct type *)char_type ;

#line 303 "../../src/norm2.c"
break ;
case 29 : _au1_t = (struct type *)short_type ;

#line 304 "../../src/norm2.c"
break ;
case 22 : _au1_t = (struct type *)long_type ;

#line 305 "../../src/norm2.c"
break ;
case 37 : _au1_t = (struct type *)uint_type ;

#line 306 "../../src/norm2.c"
break ;
case 15 : _au1_t = (struct type *)float_type ;

#line 307 "../../src/norm2.c"
break ;
case 11 : _au1_t = (struct type *)double_type ;

#line 308 "../../src/norm2.c"
break ;
case 38 : _au1_t = (struct type *)void_type ;

#line 309 "../../src/norm2.c"
break ;
default : { 
#line 314 "../../src/norm2.c"
struct ea _au0__V13 ;

#line 310 "../../src/norm2.c"
error ( (char *)"illegalK:%k", (struct ea *)( ( ((& _au0__V13 )-> _ea__O1.__C1_i = ((int )_au0_b )), (((& _au0__V13 )))) )
#line 310 "../../src/norm2.c"
, (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
case 21 : _au1_t = (struct type *)int_type ;
} }
return _au1_t ;
}
;
Pbase defa_type ;
Pbase moe_type ;
Pexpr dummy ;
Pexpr zero ;

#line 321 "../../src/norm2.c"
Pclass ccl ;
Plist modified_tn = 0 ;

#line 324 "../../src/norm2.c"
static struct name sta_name_dummy ;
Pname sta_name = (& sta_name_dummy );

#line 327 "../../src/norm2.c"
TOK back ;

#line 329 "../../src/norm2.c"
char memptrdcl (_au0_bn , _au0_tn , _au0_ft , _au0_n )Pname _au0_bn ;

#line 329 "../../src/norm2.c"
Pname _au0_tn ;

#line 329 "../../src/norm2.c"
Ptype _au0_ft ;

#line 329 "../../src/norm2.c"
Pname _au0_n ;
{ 
#line 331 "../../src/norm2.c"
Pptr _au1_p ;

#line 333 "../../src/norm2.c"
Pbase _au1_b ;

#line 335 "../../src/norm2.c"
Pfct _au1_f ;
Ptype _au1_t ;

#line 337 "../../src/norm2.c"
struct ptr *_au0__Xthis__ctor_ptr ;

#line 331 "../../src/norm2.c"
_au1_p = (struct ptr *)( (_au0__Xthis__ctor_ptr = 0 ), ( (_au0__Xthis__ctor_ptr = (struct ptr *)_new ( (long )(sizeof (struct ptr))) ), (
#line 331 "../../src/norm2.c"
(Nt ++ ), ( (_au0__Xthis__ctor_ptr -> _node_base = ((unsigned char )125 )), ( (_au0__Xthis__ctor_ptr -> _pvtyp_typ = ((struct type *)0 )), (
#line 331 "../../src/norm2.c"
(_au0__Xthis__ctor_ptr -> _ptr_rdo = ((char )0 )), ((_au0__Xthis__ctor_ptr ))) ) ) ) ) ) ;
_au1_p -> _ptr_memof = (((struct classdef *)(((struct basetype *)_au0_bn -> _expr__O2.__C2_tp ))-> _basetype_b_name -> _expr__O2.__C2_tp ));
_au1_b = (struct basetype *)_basetype__ctor ( (struct basetype *)0 , (unsigned char )97 , _au0_tn ) ;
_au1_p -> _node_permanent = 1 ;
_au1_f = (((struct fct *)_au0_ft ));

#line 335 "../../src/norm2.c"
_au1_t = _au0_n -> _expr__O2.__C2_tp ;

#line 337 "../../src/norm2.c"
if (_au1_t ){ 
#line 338 "../../src/norm2.c"
_au1_p -> _pvtyp_typ = _au1_t ;
ltlt :
#line 340 "../../src/norm2.c"
switch (_au1_t -> _node_base ){ 
#line 341 "../../src/norm2.c"
case 125 : 
#line 342 "../../src/norm2.c"
case 158 : 
#line 343 "../../src/norm2.c"
case 110 : 
#line 344 "../../src/norm2.c"
if ((((struct ptr *)_au1_t ))-> _pvtyp_typ == 0 ){
#line 344 "../../src/norm2.c"

#line 345 "../../src/norm2.c"
(((struct ptr *)_au1_t ))-> _pvtyp_typ = (struct type *)_au1_b ;
break ;
}
_au1_t = (((struct ptr *)_au1_t ))-> _pvtyp_typ ;
goto ltlt ;
default : 
#line 351 "../../src/norm2.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'s' , (char *)"P toMFT too complicated", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct
#line 351 "../../src/norm2.c"
ea *)ea0 ) ;
}
}
else 
#line 355 "../../src/norm2.c"
_au1_p -> _pvtyp_typ = (struct type *)_au1_b ;
_au1_f -> _fct_returns = (struct type *)_au1_p ;
_au0_n -> _expr__O2.__C2_tp = (struct type *)_au1_f ;
}
;

#line 359 "../../src/norm2.c"
extern char _STI______src_norm2_c_ ()
#line 324 "../../src/norm2.c"
{ _name__ctor ( & sta_name_dummy , (char *)0 ) ;
}
;

#line 359 "../../src/norm2.c"
extern char _STD______src_norm2_c_ ()
#line 324 "../../src/norm2.c"
{ _name__dtor ( & sta_name_dummy , (int )0 ) ;
}
;

/* the end */
