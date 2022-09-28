#ident	"@(#)sdb:libexecon/common/Process.lx.C	1.14"
#include	"Process.h"
#include	"EventTable.h"
#include	"Flags.h"
#include	"Symtab.h"
#include	"Source.h"
#include	"Object.h"
#include	"Interface.h"
#include	"Assoccmds.h"
#include	<osfcn.h>

int
Process::respond_to_sus()
{
	int	fdobj;

	switch ( exec_cnt )
	{
		case 2:
			--exec_cnt;
			return start_run();
		case 1:
			--exec_cnt;
			seglist.build( key );
			fdobj = ::open_object( key, pc );
			use_et( ::find_et(fdobj) );
			::close( fdobj );
			return 1;
		default:
			check_watchpoints();
			find_cur_src();
			return show_current_location( 1, verbosity >= 0 );
	}
}

int
Process::respond_to_sig()
{
	Assoccmds *	a;

	if ( etable == 0 )
	{
		printf("Process::respond_to_sig() called with zero etable.\n");
		return 0;
	}
	else if ( etable->siglist.ignored(signo) )
	{
		return restart();
	}
	else if ( (a = etable->siglist.assoccmds(signo)) != 0 )
	{
		check_watchpoints();
		find_cur_src();
		return queue_assoc(a);
	}
	else
	{
		check_watchpoints();
		find_cur_src();
		return show_current_location( 1, verbosity >= 0 );
	}
}

int
Process::respond_to_tsc()
{
	Assoccmds *	a;
	int		exit = ( state == es_syscallxit) ? 1 : 0;

	check_watchpoints();
	if ( (a = etable->tsclist.assoccmds(tsc,exit)) != 0 )
	{
		find_cur_src();
		return queue_assoc(a);
	}
	else
	{
		find_cur_src();
		return show_current_location( 1, verbosity >= 0 );
	}
}

int
Process::respond_to_bkpt()
{
	Assoccmds *	a;

	if ( --skip_count > 0 )
	{
		return restart();
	}
	else if ( (a = latestbkpt->assoccmds()) != 0 )
	{
		check_watchpoints();
		find_cur_src();
		if ( ANNOUNCE( latestbkpt->_flags ) )
		{
			show_current_location( 1, verbosity >= 0 );
		}
		return queue_assoc(a);
	}
	else
	{
		check_watchpoints();
		find_cur_src();
		if ( ANNOUNCE( latestbkpt->_flags ) )
		{
			show_current_location( 1, verbosity >= 0 );
		}
		return 1;
	}
}

int
Process::respond_to_dynpt()
{
	int	fdobj;

	if ( !seglist.buildable( key ) )
	{
		return start_stepbkpt();
	}
	else if ( go_past_rtl )
	{
		go_past_rtl = 0;
		seglist.build( key );
		// fdobj = ::open_object( key, pc );
		fdobj = ::open_object( key, PROCKLUDGE ); //til /proc is fixed
		use_et( ::find_et(fdobj) );
		::close( fdobj );
		return 1;
	}
	else
	{
		seglist.build( key );
		return start_stepbkpt();
	}
}

int
Process::save_start_info()
{
	start_a_r = regaccess.top_a_r();
	return find_stmt(startstmt,pc);
}

int
Process::find_stmt( Stmt & stmt, Iaddr pc )
{
	Symtab *	symtab;
	Symbol		symbol;
	Source		source;

	if ( (symtab = seglist.find_symtab(key,pc)) == 0 )
	{
		stmt.unknown();
		return 1;
	}
	else if ( symtab->find_source(pc,symbol) == 0 )
	{
		stmt.unknown();
		return 1;
	}
	else if ( symbol.source(source) == 0 )
	{
		stmt.unknown();
		printe("internal error: ");
		printe("Process::find_stmt() for process %d\n", key.pid );
		return 0;
	}
	else
	{
		source.pc_to_stmt(pc,stmt.line);
		return 1;
	}
}

int
Process::find_cur_src()
{
	Symtab *	symtab;
	Symbol		symbol;
	Source		source;

	if ( (symtab = seglist.find_symtab(key,pc)) == 0 )
	{
		current_srcfile = 0;
		return 1;
	}
	else if ( symtab->find_source(pc,symbol) == 0 )
	{
		current_srcfile = 0;
		return 1;
	}
	else if ( symbol.source( source ) == 0 )
	{
		current_srcfile = symbol.name();
		return 1;
	}
	else
	{
		current_srcfile = symbol.name();
		source.pc_to_stmt( pc, currstmt.line );
		return 1;
	}
}
