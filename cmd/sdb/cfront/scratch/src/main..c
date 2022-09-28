/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#line 1 "../../src/main.c"

/* <<cfront 1.2.1 2/16/87>> */
/* < ../../src/main.c */

#ident	"@(#)sdb:cfront/scratch/src/main..c	1.2"
char *_new(); char _delete(); char *_vec_new(); char _vec_delete();

#line 1 "../../src/main.c"

#include <time.h>

#include <ctype.h>

#line 24 "../../src/main.c"
extern char *ctime ();
extern long time ();
long start_time = 0 ;

#line 26 "../../src/main.c"
long stop_time = 0 ;

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

#line 30 "../../src/main.c"
char *prog_name = "<<cfront 1.2.4 8/23/87>>";

#line 32 "../../src/main.c"
char *src_file_name = 0 ;

#line 37 "../../src/main.c"
bit old_fct_accepted ;
bit fct_void ;

#line 40 "../../src/main.c"
char *line_format = "\n# %d \"%s\"\n";

#include <signal.h>

#line 45 "../../src/main.c"
char core_dump ()
#line 46 "../../src/main.c"
{ 
#line 47 "../../src/main.c"
if (error_count ){ 
#line 48 "../../src/main.c"
fprintf ( stderr, (char *)"sorry, cannot recover from previous error\n") ;
}
else 
#line 51 "../../src/main.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'i' , (char *)"bus error (or something nasty like that)", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 )
#line 51 "../../src/main.c"
;
ext ( 99 ) ;
}
;

#line 56 "../../src/main.c"
Plist isf_list ;
Pstmt st_ilist ;
Pstmt st_dlist ;
Ptable sti_tbl ;
Ptable std_tbl ;

#line 62 "../../src/main.c"
int Nspy = 0 ;
int Nfile = 1 ;

#line 63 "../../src/main.c"
int Nline = 0 ;

#line 63 "../../src/main.c"
int Ntoken = 0 ;
int Nfree_store = 0 ;

#line 64 "../../src/main.c"
int Nalloc = 0 ;

#line 64 "../../src/main.c"
int Nfree = 0 ;
int Nname = 0 ;
int Nn = 0 ;

#line 66 "../../src/main.c"
int Nbt = 0 ;

#line 66 "../../src/main.c"
int Nt = 0 ;

#line 66 "../../src/main.c"
int Ne = 0 ;

#line 66 "../../src/main.c"
int Ns = 0 ;

#line 66 "../../src/main.c"
int Nc = 0 ;

#line 66 "../../src/main.c"
int Nstr = 0 ;

#line 66 "../../src/main.c"
int Nl = 0 ;
extern int NFn ;

#line 67 "../../src/main.c"
extern int NFtn ;

#line 67 "../../src/main.c"
extern int NFbt ;

#line 67 "../../src/main.c"
extern int NFpv ;

#line 67 "../../src/main.c"
extern int NFf ;

#line 67 "../../src/main.c"
extern int NFe ;

#line 67 "../../src/main.c"
extern int NFs ;

#line 67 "../../src/main.c"
extern int NFc ;

#line 67 "../../src/main.c"
extern int NFl ;
int vtbl_opt = -1;

#line 73 "../../src/main.c"
int simpl_init ();
int typ_init ();
int syn_init ();
int lex_init ();
int error_init ();
int print_free ();
int read_align ();
int print_align ();

#line 82 "../../src/main.c"
char spy (_au0_s )char *_au0_s ;
{ 
#line 84 "../../src/main.c"
if (_au0_s )fprintf ( stderr, (char *)"%s:\n", _au0_s ) ;
fprintf ( stderr, (char *)"files=%d lines=%d tokens=%d\n", Nfile , Nline , Ntoken ) ;

#line 87 "../../src/main.c"
fprintf ( stderr, (char *)"Names: distinct=%d global=%d type=%d\n", Nname , ( (gtbl -> _table_free_slot - 1 )) , ( (ktbl -> _table_free_slot -
#line 87 "../../src/main.c"
1 )) ) ;
fflush ( stderr) ;
if (start_time && stop_time ){ 
#line 90 "../../src/main.c"
fprintf ( stderr, (char *)"start time: %s", ctime ( & start_time ) ) ;
fprintf ( stderr, (char *)"stop time:  %s", ctime ( & stop_time ) ) ;

#line 93 "../../src/main.c"
fprintf ( stderr, (char *)"real time delay %ld: %d lines per second\n", stop_time - start_time , Nline / (stop_time - start_time )) ;
fflush ( stderr) ;
}
print_free ( ) ;
fflush ( stderr) ;

#line 100 "../../src/main.c"
fprintf ( stderr, (char *)"sizeof: n=%d bt=%d f=%d pv=%d s=%d e=%d c=%d l=%d\n", sizeof (struct name ), sizeof (struct basetype ), sizeof (struct fct ),
#line 100 "../../src/main.c"
sizeof (struct ptr ), sizeof (struct stmt ), sizeof (struct expr ), sizeof (struct classdef ), sizeof (struct
#line 100 "../../src/main.c"
elist )) ;

#line 102 "../../src/main.c"
fprintf ( stderr, (char *)"alloc(): n=%d bt=%d t=%d e=%d s=%d c=%d l=%d str=%d\n", Nn , Nbt , Nt , Ne , Ns , Nc , Nl , Nstr ) ;
#line 102 "../../src/main.c"

#line 104 "../../src/main.c"
fprintf ( stderr, (char *)"free(): n=%d bt=%d t=%d e=%d s=%d c=%d l=%d\n", NFn , NFbt , NFpv + NFf , NFe , NFs , NFc , NFl ) ;
#line 104 "../../src/main.c"

#line 105 "../../src/main.c"
fflush ( stderr) ;
fprintf ( stderr, (char *)"%d errors\n", error_count ) ;
fflush ( stderr) ;
}
;
Pname dcl_list ;

#line 112 "../../src/main.c"
char *st_name ();

#line 114 "../../src/main.c"
char run ()
#line 118 "../../src/main.c"
{ 
#line 119 "../../src/main.c"
Pname _au1_n ;
int _au1_i ;

#line 235 "../../src/main.c"
Pname _au1_m ;

#line 120 "../../src/main.c"
_au1_i = 1 ;

#line 122 "../../src/main.c"
while (_au1_n = syn ( ) ){ 
#line 123 "../../src/main.c"
Pname _au2_nn ;
Pname _au2_nx ;

#line 126 "../../src/main.c"
if (_au1_n == (((struct name *)1 )))continue ;

#line 140 "../../src/main.c"
if (_au1_n -> _name_n_list )_au1_n -> _expr__O2.__C2_tp -> _node_permanent = 1 ;

#line 142 "../../src/main.c"
for(_au2_nn = _au1_n ;_au2_nn ;_au2_nn = _au2_nx ) { 
#line 143 "../../src/main.c"
_au2_nx = _au2_nn -> _name_n_list ;
_au2_nn -> _name_n_list = 0 ;

#line 146 "../../src/main.c"
if (_name_dcl ( _au2_nn , gtbl , (unsigned char )14 ) == 0 )continue ;

#line 148 "../../src/main.c"
if (error_count )continue ;

#line 150 "../../src/main.c"
_name_simpl ( _au2_nn ) ;

#line 153 "../../src/main.c"
{ Pname _au3_dx ;

#line 153 "../../src/main.c"
Pname _au3_d ;

#line 153 "../../src/main.c"
_au3_d = dcl_list ;

#line 153 "../../src/main.c"
for(;_au3_d ;_au3_d = _au3_dx ) { 
#line 154 "../../src/main.c"
_au3_dx = _au3_d -> _name_n_list ;
_name_dcl_print ( _au3_d , (unsigned char )0 ) ;
_name__dtor ( _au3_d , 1) ;
}
dcl_list = 0 ;

#line 160 "../../src/main.c"
if (_au2_nn -> _node_base )_name_dcl_print ( _au2_nn , (unsigned char )0 ) ;

#line 162 "../../src/main.c"
switch (_au2_nn -> _expr__O2.__C2_tp -> _node_base ){ 
#line 163 "../../src/main.c"
default : 
#line 164 "../../src/main.c"
{ Pexpr _au5_i ;

#line 164 "../../src/main.c"
_au5_i = _au2_nn -> _expr__O4.__C4_n_initializer ;
if (_au5_i && (_au5_i != (((struct expr *)1 ))))if (_au5_i && (_au5_i -> _node_permanent == 0 ))_expr_del ( _au5_i ) ;

#line 165 "../../src/main.c"
break ;
}

#line 168 "../../src/main.c"
case 108 : 
#line 169 "../../src/main.c"
{ Pfct _au5_f ;

#line 169 "../../src/main.c"
_au5_f = (((struct fct *)_au2_nn -> _expr__O2.__C2_tp ));
if (_au5_f -> _fct_body && (debug || (_au5_f -> _fct_f_inline == 0 ))){ 
#line 171 "../../src/main.c"
if (_au5_f -> _fct_body && (_au5_f -> _fct_body -> _node_permanent == 0 ))_stmt_del (
#line 171 "../../src/main.c"
(struct stmt *)_au5_f -> _fct_body ) ;
}

#line 174 "../../src/main.c"
break ;
}

#line 177 "../../src/main.c"
case 6 : 
#line 178 "../../src/main.c"
{ Pclass _au5_cl ;
register Pname _au5_p ;
Pname _au5_px ;

#line 178 "../../src/main.c"
_au5_cl = (((struct classdef *)_au2_nn -> _expr__O2.__C2_tp ));

#line 181 "../../src/main.c"
for(_au5_p = _au5_cl -> _classdef_mem_list ;_au5_p ;_au5_p = _au5_px ) { 
#line 182 "../../src/main.c"
_au5_px = _au5_p -> _name_n_list ;
if (_au5_p -> _expr__O2.__C2_tp )
#line 184 "../../src/main.c"
switch (_au5_p -> _expr__O2.__C2_tp -> _node_base ){ 
#line 185 "../../src/main.c"
case 108 : 
#line 186 "../../src/main.c"
{ Pfct _au8_f ;

#line 186 "../../src/main.c"
_au8_f = (((struct fct *)_au5_p -> _expr__O2.__C2_tp ));
if (_au8_f -> _fct_body && (debug || (_au8_f -> _fct_f_inline == 0 ))){ 
#line 188 "../../src/main.c"
if (_au8_f -> _fct_body && (_au8_f -> _fct_body -> _node_permanent == 0 ))_stmt_del (
#line 188 "../../src/main.c"
(struct stmt *)_au8_f -> _fct_body ) ;
_au8_f -> _fct_body = 0 ;
}
}
case 6 : 
#line 193 "../../src/main.c"
case 13 : 
#line 194 "../../src/main.c"
break ;
case 119 : 
#line 196 "../../src/main.c"
case 121 : 
#line 197 "../../src/main.c"
if (_au5_p && (_au5_p -> _node_permanent == 0 ))_name_del ( _au5_p ) ;
break ;
default : 
#line 200 "../../src/main.c"
_name__dtor ( _au5_p , 1) ;
}
else 
#line 203 "../../src/main.c"
_name__dtor ( _au5_p , 1) ;
}
_au5_cl -> _classdef_mem_list = 0 ;
_au5_cl -> _node_permanent = 3 ;
break ;
}
}
if (_au2_nn && (_au2_nn -> _node_permanent == 0 ))_name_del ( _au2_nn ) ;
}
}

#line 212 "../../src/main.c"
lex_clear ( ) ;
}

#line 215 "../../src/main.c"
switch (no_of_undcl ){ 
#line 216 "../../src/main.c"
case 0 : 
#line 217 "../../src/main.c"
break ;
case 1 : 
#line 219 "../../src/main.c"
{ 
#line 331 "../../src/main.c"
struct ea _au0__V10 ;

#line 219 "../../src/main.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"undeclaredF%n called", (struct ea *)( ( ((& _au0__V10 )-> _ea__O1.__C1_p = ((char *)undcl )), (((&
#line 219 "../../src/main.c"
_au0__V10 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
break ;
default : 
#line 222 "../../src/main.c"
{ 
#line 331 "../../src/main.c"
struct ea _au0__V11 ;

#line 331 "../../src/main.c"
struct ea _au0__V12 ;

#line 222 "../../src/main.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"%d undeclaredFs called; for example%n", (struct ea *)( ( ((& _au0__V11 )-> _ea__O1.__C1_i = no_of_undcl ), (((& _au0__V11 ))))
#line 222 "../../src/main.c"
) , (struct ea *)( ( ((& _au0__V12 )-> _ea__O1.__C1_p = ((char *)undcl )), (((& _au0__V12 )))) ) ,
#line 222 "../../src/main.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
} } }

#line 225 "../../src/main.c"
switch (no_of_badcall ){ 
#line 226 "../../src/main.c"
case 0 : 
#line 227 "../../src/main.c"
break ;
case 1 : 
#line 229 "../../src/main.c"
{ 
#line 331 "../../src/main.c"
struct ea _au0__V13 ;

#line 229 "../../src/main.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"%n declaredWoutAs calledWAs", (struct ea *)( ( ((& _au0__V13 )-> _ea__O1.__C1_p = ((char *)badcall )), (((&
#line 229 "../../src/main.c"
_au0__V13 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
break ;
default : 
#line 232 "../../src/main.c"
{ 
#line 331 "../../src/main.c"
struct ea _au0__V14 ;

#line 331 "../../src/main.c"
struct ea _au0__V15 ;

#line 232 "../../src/main.c"
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"%d Fs declaredWoutAs calledWAs; for example%n", (struct ea *)( ( ((& _au0__V14 )-> _ea__O1.__C1_i = no_of_badcall ), (((& _au0__V14 ))))
#line 232 "../../src/main.c"
) , (struct ea *)( ( ((& _au0__V15 )-> _ea__O1.__C1_p = ((char *)badcall )), (((& _au0__V15 )))) ) ,
#line 232 "../../src/main.c"
(struct ea *)ea0 , (struct ea *)ea0 ) ;
} } }

#line 235 "../../src/main.c"
;
if (fct_void == 0 )
#line 237 "../../src/main.c"
for(_au1_m = _table_get_mem ( gtbl , _au1_i = 1 ) ;_au1_m ;_au1_m = _table_get_mem ( gtbl , ++ _au1_i ) ) {
#line 237 "../../src/main.c"

#line 238 "../../src/main.c"
if (((_au1_m -> _node_base == 123 )|| (_au1_m -> _name_n_sto == 14 ))|| (_au1_m -> _name_n_stclass == 13 ))
#line 240 "../../src/main.c"
continue ;

#line 242 "../../src/main.c"
{ Ptype _au2_t ;

#line 242 "../../src/main.c"
_au2_t = _au1_m -> _expr__O2.__C2_tp ;
if (_au2_t == 0 )continue ;
ll :
#line 245 "../../src/main.c"
switch (_au2_t -> _node_base ){ 
#line 246 "../../src/main.c"
case 97 : _au2_t = (((struct basetype *)_au2_t ))-> _basetype_b_name -> _expr__O2.__C2_tp ;

#line 246 "../../src/main.c"
goto ll ;
case 6 : 
#line 248 "../../src/main.c"
case 13 : 
#line 249 "../../src/main.c"
case 119 : 
#line 250 "../../src/main.c"
case 76 : 
#line 251 "../../src/main.c"
case 110 : continue ;
case 108 : 
#line 253 "../../src/main.c"
if ((((struct fct *)_au2_t ))-> _fct_f_inline || ((((struct fct *)_au2_t ))-> _fct_body == 0 ))continue ;
}

#line 256 "../../src/main.c"
if (((_au1_m -> _name_n_addr_taken == 0 )&& (_au1_m -> _name_n_used == 0 ))&& (_type_tconst ( _au1_m -> _expr__O2.__C2_tp ) == 0 ))
#line 259 "../../src/main.c"
if (_au1_m -> _name_n_sto ==
#line 259 "../../src/main.c"
31 )
#line 260 "../../src/main.c"
{ 
#line 331 "../../src/main.c"
struct ea _au0__V16 ;

#line 260 "../../src/main.c"
errorFI_PCloc__PC_RCea__RCea__RCea__RCea___ ( (int )'w' , & _au1_m -> _name_where , (char *)"%n defined but not used", (struct ea *)( ( ((& _au0__V16 )-> _ea__O1.__C1_p =
#line 260 "../../src/main.c"
((char *)_au1_m )), (((& _au0__V16 )))) ) , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
} }
}

#line 265 "../../src/main.c"
{ char *_au1_ctor_name ;

#line 265 "../../src/main.c"
char *_au1_dtor_name ;

#line 265 "../../src/main.c"
_au1_ctor_name = 0 ;

#line 265 "../../src/main.c"
_au1_dtor_name = 0 ;

#line 267 "../../src/main.c"
if (st_ilist ){ 
#line 270 "../../src/main.c"
Pname _au2_n ;
Pfct _au2_f ;

#line 272 "../../src/main.c"
struct block *_au0__Xthis__ctor_block ;

#line 270 "../../src/main.c"
_au2_n = (struct name *)_name__ctor ( (struct name *)0 , st_name ( "_STI") ) ;
_au2_f = (struct fct *)_fct__ctor ( (struct fct *)0 , (struct type *)void_type , (struct name *)0 , (unsigned char )1 ) ;
_au2_n -> _expr__O2.__C2_tp = (struct type *)_au2_f ;
_au2_f -> _fct_body = (struct block *)( (_au0__Xthis__ctor_block = 0 ), ( ( (_au0__Xthis__ctor_block = 0 ), (_au0__Xthis__ctor_block = (struct block *)_stmt__ctor ( ((struct
#line 273 "../../src/main.c"
stmt *)_au0__Xthis__ctor_block ), (unsigned char )116 , st_ilist -> _stmt_where , ((struct stmt *)0 )) )) , ( (_au0__Xthis__ctor_block -> _stmt__O7.__C7_d = ((struct
#line 273 "../../src/main.c"
name *)0 )), ((_au0__Xthis__ctor_block ))) ) ) ;
_au2_f -> _fct_body -> _stmt_s = st_ilist ;
_au2_f -> _fct_body -> _stmt_memtbl = sti_tbl ;
_au2_n -> _name_n_sto = 14 ;
(_name_dcl ( _au2_n , gtbl , (unsigned char )14 ) );
_name_simpl ( _au2_n ) ;
_name_dcl_print ( _au2_n , (unsigned char )0 ) ;
_au1_ctor_name = _au2_n -> _expr__O3.__C3_string ;
}

#line 283 "../../src/main.c"
if (st_dlist ){ 
#line 286 "../../src/main.c"
Pname _au2_n ;
Pfct _au2_f ;

#line 288 "../../src/main.c"
struct block *_au0__Xthis__ctor_block ;

#line 286 "../../src/main.c"
_au2_n = (struct name *)_name__ctor ( (struct name *)0 , st_name ( "_STD") ) ;
_au2_f = (struct fct *)_fct__ctor ( (struct fct *)0 , (struct type *)void_type , (struct name *)0 , (unsigned char )1 ) ;
_au2_n -> _expr__O2.__C2_tp = (struct type *)_au2_f ;
_au2_f -> _fct_body = (struct block *)( (_au0__Xthis__ctor_block = 0 ), ( ( (_au0__Xthis__ctor_block = 0 ), (_au0__Xthis__ctor_block = (struct block *)_stmt__ctor ( ((struct
#line 289 "../../src/main.c"
stmt *)_au0__Xthis__ctor_block ), (unsigned char )116 , st_dlist -> _stmt_where , ((struct stmt *)0 )) )) , ( (_au0__Xthis__ctor_block -> _stmt__O7.__C7_d = ((struct
#line 289 "../../src/main.c"
name *)0 )), ((_au0__Xthis__ctor_block ))) ) ) ;
_au2_f -> _fct_body -> _stmt_s = st_dlist ;
_au2_f -> _fct_body -> _stmt_memtbl = std_tbl ;
_au2_n -> _name_n_sto = 14 ;
(_name_dcl ( _au2_n , gtbl , (unsigned char )14 ) );
_name_simpl ( _au2_n ) ;
_name_dcl_print ( _au2_n , (unsigned char )0 ) ;
_au1_dtor_name = _au2_n -> _expr__O3.__C3_string ;
}

#line 312 "../../src/main.c"
if (debug == 0 ){ 
#line 313 "../../src/main.c"
{ Plist _au2_l ;

#line 313 "../../src/main.c"
_au2_l = isf_list ;

#line 313 "../../src/main.c"
for(;_au2_l ;_au2_l = _au2_l -> _name_list_l ) { 
#line 314 "../../src/main.c"
Pname _au3_n ;
Pfct _au3_f ;

#line 314 "../../src/main.c"
_au3_n = _au2_l -> _name_list_f ;
_au3_f = (((struct fct *)_au3_n -> _expr__O2.__C2_tp ));

#line 317 "../../src/main.c"
if (_au3_f -> _node_base == 76 ){ 
#line 318 "../../src/main.c"
_au3_n = (((struct gen *)_au3_f ))-> _gen_fct_list -> _name_list_f ;
_au3_f = (((struct fct *)_au3_n -> _expr__O2.__C2_tp ));
}

#line 322 "../../src/main.c"
if (_au3_n -> _name_n_addr_taken || (_au3_f -> _fct_f_virtual && (vtbl_opt != 0 ))){ 
#line 323 "../../src/main.c"
_loc_putline ( & _au3_n -> _name_where ) ;
_type_dcl_print ( _au3_n -> _expr__O2.__C2_tp , _au3_n ) ;
}
}
}
}
fprintf ( out_file , (char *)"\n/* the end */\n") ;
}
}
;
bit warn = 1 ;
bit debug ;
char *afile = "";

#line 337 "../../src/main.c"
int no_of_undcl = 0 ;

#line 337 "../../src/main.c"
int no_of_badcall = 0 ;
Pname undcl ;

#line 338 "../../src/main.c"
Pname badcall ;

#line 340 "../../src/main.c"
int main (_au0_argc , _au0_argv )int _au0_argc ;

#line 340 "../../src/main.c"
char *_au0_argv [];
{ _main(); 
#line 344 "../../src/main.c"
{ 
#line 345 "../../src/main.c"
extern char *mktemp ();
register char *_au1_cp ;
short _au1_i ;

#line 451 "../../src/main.c"
typedef char (*ST )();

#line 350 "../../src/main.c"
ST _au1_sick ;

#line 363 "../../src/main.c"
extern char *malloc ();

#line 350 "../../src/main.c"
_au1_sick = (((char (*)())(signal )));
(*_au1_sick )( SIGILL , core_dump ) ;
(*_au1_sick )( SIGIOT , core_dump ) ;
(*_au1_sick )( SIGEMT , core_dump ) ;
(*_au1_sick )( SIGFPE , core_dump ) ;
(*_au1_sick )( SIGBUS , core_dump ) ;
(*_au1_sick )( SIGSEGV , core_dump ) ;

#line 363 "../../src/main.c"
;
(malloc ( (unsigned int )0 ) );

#line 366 "../../src/main.c"
error_init ( ) ;

#line 368 "../../src/main.c"
for(_au1_i = 1 ;_au1_i < _au0_argc ;++ _au1_i ) { 
#line 369 "../../src/main.c"
switch (*(_au1_cp = (_au0_argv [_au1_i ]))){ 
#line 370 "../../src/main.c"
case '+' : 
#line 371 "../../src/main.c"
while (*(++ _au1_cp )){ 
#line 372 "../../src/main.c"
switch
#line 372 "../../src/main.c"
(*_au1_cp ){ 
#line 373 "../../src/main.c"
case 'w' : 
#line 374 "../../src/main.c"
warn = 0 ;
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"+w option will not be supported in future releases", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 375 "../../src/main.c"

#line 376 "../../src/main.c"
break ;
case 'd' : 
#line 378 "../../src/main.c"
debug = 1 ;
errorFI_PC_RCea__RCea__RCea__RCea___ ( (int )'w' , (char *)"+d option will not be supported in future releases", (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 , (struct ea *)ea0 ) ;
#line 379 "../../src/main.c"

#line 380 "../../src/main.c"
break ;
case 'f' : 
#line 382 "../../src/main.c"
src_file_name = (_au1_cp + 1 );
goto xx ;
case 'x' : 
#line 385 "../../src/main.c"
if (read_align ( afile = (_au1_cp + 1 )) ){ 
#line 386 "../../src/main.c"
fprintf ( stderr, (char *)"bad size-table (option +x)") ;
#line 386 "../../src/main.c"

#line 387 "../../src/main.c"
exit ( 999 ) ;
}
goto xx ;
case 'V' : 
#line 391 "../../src/main.c"
fct_void = (old_fct_accepted = 1 );
fprintf ( stderr, (char *)"\nwarning: +V option will not be supported in future releases\n") ;
break ;
case 'e' : 
#line 395 "../../src/main.c"
switch (*(++ _au1_cp )){ 
#line 396 "../../src/main.c"
case '0' : 
#line 397 "../../src/main.c"
case '1' : 
#line 398 "../../src/main.c"
vtbl_opt = ((*_au1_cp )- '0' );
break ;
default : 
#line 401 "../../src/main.c"
fprintf ( stderr, (char *)"bad +e option") ;
exit ( 999 ) ;
}
break ;
case 'S' : 
#line 406 "../../src/main.c"
Nspy ++ ;
break ;
case 'L' : 
#line 409 "../../src/main.c"
line_format = "\n#line %d \"%s\"\n";
break ;
default : 
#line 412 "../../src/main.c"
fprintf ( stderr, (char *)"%s: unexpected option: +%c ignored\n", prog_name , *_au1_cp ) ;
}
}

#line 416 "../../src/main.c"
xx :
#line 417 "../../src/main.c"
break ;
default : 
#line 419 "../../src/main.c"
fprintf ( stderr, (char *)"%s: bad argument \"%s\"\n", prog_name , _au1_cp ) ;
exit ( 999 ) ;
}
}

#line 427 "../../src/main.c"
fprintf ( out_file , (char *)(line_format + 1 ), 1 , src_file_name ? src_file_name : "") ;
fprintf ( out_file , (char *)"\n/* %s */\n", prog_name ) ;
if (src_file_name )fprintf ( out_file , (char *)"/* < %s */\n", src_file_name ) ;

#line 431 "../../src/main.c"
if (Nspy ){ 
#line 432 "../../src/main.c"
start_time = time ( (long *)0 ) ;
print_align ( afile ) ;
}
fflush ( stderr) ;

#line 437 "../../src/main.c"
otbl_init ( ) ;
lex_init ( ) ;
syn_init ( ) ;
typ_init ( ) ;
simpl_init ( ) ;
scan_started = 1 ;
_loc_putline ( & curloc ) ;
run ( ) ;
if (Nspy ){ 
#line 446 "../../src/main.c"
stop_time = time ( (long *)0 ) ;
spy ( src_file_name ) ;
}

#line 450 "../../src/main.c"
exit ( ((0 <= error_count )&& (error_count < 127 ))? error_count : 127 ) ;
}
};

#line 454 "../../src/main.c"
extern char *st_name (_au0_pref )char *_au0_pref ;

#line 459 "../../src/main.c"
{ 
#line 460 "../../src/main.c"
int _au1_prefl ;
int _au1_strl ;

#line 463 "../../src/main.c"
char *_au1_name ;

#line 468 "../../src/main.c"
char *_au1_p ;

#line 460 "../../src/main.c"
_au1_prefl = strlen ( (char *)_au0_pref ) ;
_au1_strl = (_au1_prefl + 2 );
if (src_file_name )_au1_strl += strlen ( (char *)src_file_name ) ;
_au1_name = (((char *)_new ( (long )((sizeof (char ))* _au1_strl )) ));
strcpy ( _au1_name , (char *)_au0_pref ) ;
if (src_file_name )strcpy ( _au1_name + _au1_prefl , (char *)src_file_name ) ;
(_au1_name [_au1_strl - 2 ])= '_' ;
(_au1_name [_au1_strl - 1 ])= 0 ;
_au1_p = _au1_name ;
while (*(++ _au1_p ))if ((! isalpha(*_au1_p))&& (! isdigit(*_au1_p)))(*_au1_p )= '_' ;
return _au1_name ;
}
;

/* the end */
