/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/Process.h	1.28"
#ifndef Process_h
#define Process_h

#include	"prioctl.h"
#include	"Itype.h"
#include	"Circlech.h"
#include	"Stmt.h"
#include	"Instr.h"
#include	"Breaklist.h"
#include	"RegAccess.h"
#include	"Rvalue.h"
#include	"Frame.h"
#include	"Seglist.h"
#include	"oslevel.h"

#include	"TYPE.h" /* for Process::SDB_type only */

class	Assoccmds;
struct	EventTable;
class	Symbol;
class	Status;
class	Watchpoint;
class	Core;
enum	Goal1;
enum	Goal2;
enum	WS;

enum Execstate	{
			es_none,
			es_stepping,
			es_running,
			es_stepped,
			es_suspended,
			es_signalled,
			es_breakpoint,
			es_syscallent,
			es_syscallxit,
			es_corefile,
		};

class Process {
	int		ischild;
	Circlech	link;
	int		exec_cnt;
	int		skip_count,go_past_rtl;
	Iaddr		pc;
	Iaddr		lastpc;
	Goal1		goal;
	Goal2		goal2;
	Breakpoint *	latestbkpt;
	int		signo;		// change to latestsig
	int		tsc;		// change to latesttsc
	Iaddr		retaddr;
	Stmt		startstmt;
	Stmt		currstmt;
	Iaddr		start_a_r;
	Seglist		seglist;
	prrun		pr_run;
	Breakpoint	hoppt;
	Breakpoint	destpt;
	Breakpoint	dynpt;
	int		ecount;
	int		verbosity;
	RegAccess	regaccess;
	Core *		core;

	int		textfd;
	int		corefd;

	int		inform_startup();
	int		inform_run();
	int		inform_step();
	int		inform_stepbkpt();

	int		start_run();
	int		start_step();
	int		start_stepbkpt();
	int		restart();

	int		check_stmt_step();
	int		check_stmt_step_over();
	int		check_instr_step();
	int		check_instr_step_over();

	int		respond_to_sig();
	int		respond_to_sus();
	int		respond_to_bkpt();
	int		respond_to_tsc();
	int		respond_to_dynpt();

	int		set( Breakpoint &, Iaddr );
	int		remove( Breakpoint & );
	int		insert_bkpt( Breakpoint * );
	int		lift_bkpt( Breakpoint * );
	char *		bkpt_info( Breakpoint * );
	int		ask( Breakpoint * );

	Frame *		find_frame( const FrameId & );
	int		check_existence(Watchpoint *,Frame *&, Rvalue &, int &);
	int		check_value( Watchpoint *, Frame *, Rvalue &, int & );
	int		report_wpt( Watchpoint *, WS, Rvalue &, int & );
	int		check_watchpoints();
	int		update_wpts();

	int		save_start_info();
	int		find_stmt( Stmt &, Iaddr );
	int		find_cur_src();
	friend class	FrameId;
	friend int	dispose_process( Process * );
public:
	Instr		instr;
	EventTable *	etable;
	Key		key;
	int		grabbed;
	int		pid;
	void *		child_io;
	unsigned long	epoch;
	Status *	status;
	char *		current_srcfile;
	Execstate	state;

	TYPE		SDB_type; /* for libexp.a sdb support. */

			Process( Key, int );
			Process( int, int, int );
			~Process();

	int		use_et( EventTable * );
	int		drop_et();
	int		cleanup_et();

	int		is_child()	{ return ischild; }

	Frame *		topframe();

	int		inform( prstatus & );

	int		run( int, Iaddr, int, int );
	int		stmt_step( int, Iaddr, int, int );
	int		instr_step( int, Iaddr, int, int );
	int		stmt_step_over( int, Iaddr, int, int );
	int		instr_step_over( int, Iaddr, int, int );
	int		resume( int );

	int		setframe( Frame *, int lock );
	Frame *		curframe();
	Frame *		top_frame;
	Frame *		cur_frame;

	int		read( Iaddr, int, char *);
	int		write( Iaddr, void *, int );
	int		read( Iaddr, Stype, Itype & );
	int		write( Iaddr, Stype, const Itype & );

	int		evaluate_expr( char *, char * );
	int		evaluate_addr( char *, char * );

	int		disassemble( Iaddr, int, Iaddr * );

	char *		stateinfo();

	int		is_proto() { return (key.pid == -1) &&
					    (key.fd  == -1) &&
					    (corefd  == -1); }

	Symtab *	find_symtab( Iaddr );
	int		find_source( char *, Symbol & );
	Symbol		find_entry( Iaddr );
	Symbol		find_scope( Iaddr );
	NameEntry *	first_global();
	NameEntry *	next_global();
	Symbol		find_global( char * );
	Symbol		ptr_type( Fund_type );
	Symbol		current_function();
	Symbol		current_scope();
	Symbol		first_file();
	Symbol		next_file();

	int		set_current_stmt( char *, long );
	long		current_line();
	int		show_current_location( int, int );

	int		set_bkpt( Iaddr, Assoccmds * = 0, int = 1 );
	int		remove_bkpt( Iaddr );
	int		display_bkpts();
	int		remove_all_bkpts( int );
	char *		text_nobkpt( Iaddr );

	int		set_wpt( char * );
	int		remove_wpt( char * );
	int		display_wpts();
	int		remove_all_wpts( int );

	int		set_sig_catch( sigset_t );

	Iaddr		pc_value()	{	return pc; }
	Iaddr		getreg( RegRef );
	int		readreg( RegRef, Stype, Itype & );
	int		writereg( RegRef, Stype, Itype & );
	int		display_regs( int );

	int		print_map();

	char *		symbol_name( Symbol );	// get symbol name

	int		in_stack( Iaddr );
	int		in_text( Iaddr );
};

#endif

// end of Process.h
