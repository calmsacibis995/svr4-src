/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:libexp/common/Expr_priv.h	1.7"

// Expr_priv.h -- private definitions for the Expr class

class NameEntry;
class Rvalue;
enum Fund_type;

#include "Symbol.h"
#include "Label.h"
#include "SDBinfo.h"
#include "Type.h"
#include "Tag.h"
#include "Reg.h"
#include "Process.h"
#include "utility.h"
#include "Interface.h"
#include "format.h"
#include "Reg.h"

extern int parse(char *, SDBinfo **);

extern int convert( Rvalue &rval, Fund_type from, Fund_type to );

extern double fetch_double( unsigned char *, int size );
extern int convert_to_fp( unsigned char *, int size, Stype *, char *result );

extern Rvalue *package_itype( Stype, Itype * );
extern void runnit( Process *, Iaddr target, Execstate oldstate );
extern void writereg ( Process *, RegRef, Iaddr val );
extern unsigned char *rev ( Iaddr );
extern Iaddr round ( Iaddr, int align );
extern int need_stack( Rvalue *, SDBinfo_kind );
#ifdef m88k
extern Rvalue *get_ret( Process *process, TYPE *ret, Iaddr stunaddr );
#else
extern Rvalue *get_ret( Process *process, TYPE *ret );
#endif
extern TYPE return_type( Symbol *func, char *fmt );
extern int user_type( TYPE *, Symbol * );
extern void widen( Rvalue *, TYPE * );

extern int debugflag;

// bits in debugflag for special subsystems:
#define	DBG_SYMSTREAM	0x00002
#define DBG_EVALUATE	0x00004
#define DBG_RESOLVE	0x00008
#define DBG_MATCH	0x00010
#define DBG_ERRORS	0x00020
#define DBG_GETREG	0x00040
#define DBG_DECOMP	0x00080
#define DBG_SYMDUMP	0x00100
#define DBG_RVAL	0x00200
#define DBG_TYPES	0x00400
#define DBG_BOUNDS	0x00800
#define DBG_COMPLETE	0x01000
#define DBG_ADDR	0x02000
#define DBG_ASSIGN	0x04000
#define DBG_SDBINFO	0x08000
#define DBG_CALL	0x10000
#define DBG_PARSE	0x20000
#define DBG_ENUM	0x40000

// values for evaluate() mode
#define sUNINIT		0		// uninitialized
#define sARG		1		// produce one rvalue from Symbol
#define sVAL		2		// produce one rvalue from etree
#define sLHS		3		// produce one lvalue from etree
#define sPRINT_VAL	4		// print value(s)
#define sPRINT_ADDR	5		// print address(es)
#define sPRINT_RESULT	6		// print return value from function call
#define sWATCH		7		// produce one rvalue from string

#define level_unspec	(-1)

// SymbolStream state
enum SSState {
	SSinit,
	SSdoing_locals,
	SSdoing_files,
	SSdoing_globals,
	SSnomore
};

class SymbolStream {		// private class
    char	*pattern;
    int		 has_wildcards;
    char	*procedure;
    int		 level;
    int		 cur_level;
    Process	*process;
    Frame	*_frame;
    int		 do_locals;
    int		 do_globals;
    SSState	 state;
    Symbol	 current_file;
    Symbol	 scope_file;
    NameEntry	*current_global;
    Symbol	 current_local;
    Symbol	 func;
    Symbol	 block;
    Symbol	 scope;
    Label	 lab;
    int		 global_count;
    int		 first_time;
    int		 match(char *funcname);
public:
    SymbolStream( SymbolStream& );
    SymbolStream() { pattern = 0; has_wildcards = 0; procedure = 0;
			level = level_unspec; cur_level = 0;
			process = 0; _frame = 0; do_locals = 0;
			do_globals = 0; state = 0;
			current_file.null(); scope_file.null();
			current_global = 0; current_local.null();
			func.null(); block.null(); scope.null();
			lab.clear(); global_count = 0; first_time = 1;}
    void init( char *pattern, char *proc, int level,
				Process *, Frame *, int global_only );
    Symbol next();
    char *label()	{ return lab.str(); }
    Frame *frame()	{ return _frame; }
    int	found_local;
    int globalcount()	{ return global_count; }
};

class Decomp {		// private class
	SDBinfo		*node;
	int		 prev_node_is_integer;
	int		 first_time;
	int		 isfield;
	int		 isindex;
	int		 index;
	int		 span;
	Symbol		 field;
	TYPE		 type;
	Decomp		*next_decomp;	// chain
	int		 completable;	// do we want automatic completion?
	void		 complete(TYPE *);
public:
	Decomp( SDBinfo *, TYPE *, int should_complete, int pnii = 0 );
	~Decomp();
	int next();			// returns 0 for failure
	SDBinfo *pos()	{ return node; }
	int is_field()	{ return isfield; }
	int is_index()	{ return isindex; }
	int cur_index()	{ return isindex ? index : 0; }
	char *cur_name(){ return isfield ? field.name() : 0; }
	Symbol fieldsym() { return field; }
	TYPE cur_type()	{ return type; }
	long cur_offset(Process *, Frame *);
	Decomp *nextdecomp() { return next_decomp; }
};
