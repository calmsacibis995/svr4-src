#ident	"@(#)sdb:libexecon/common/Process.hx.C	1.21"
#include	"Process.h"
#include	"EventTable.h"
#include	"Object.h"
#include	"Monitor.h"
#include	"Interface.h"
#include	"Assoccmds.h"
#include	"Machine.h"
#include	"Core.h"
#include	<osfcn.h>
#include	<fcntl.h>

enum Goal1	{
			pg_run,
			pg_stmt_step,
			pg_instr_step,
			pg_stmt_step_over,
			pg_instr_step_over,
		};

enum Goal2	{
			sg_run,
			sg_step,
			sg_stepbkpt,
		};

extern char executable_name[];

Process *	current_process = 0;
int		interrupted = 0;

static Circlech	subjectlist;

Process::Process( Key k, int isc ) : link(this),instr(this),seglist(this)
{
	int fd;

	key = k;
	ischild = isc;
	grabbed = !ischild;
	exec_cnt = isc ? EXECCNT : 1;
	if ( ischild )
		fd = ::open( executable_name, O_RDONLY );
	else
//		fd = open_object(key, 0);
		fd = open_object(key, PROCKLUDGE); // until /proc is fixed
	seglist.setup( fd, go_past_rtl );
	if (!ischild)
		go_past_rtl = 0;
	state = es_none;
	status = 0;
	goal = pg_run;
	goal2 = sg_run;
	pr_run.pr_flags = PRSTRACE;
	premptyset(&(pr_run.pr_trace))
	textfd = -1;
	corefd = -1;
	core = 0;
	etable = 0;
	epoch = 0;
	regaccess.setup_live( key );
	top_frame = new Frame(this);
	cur_frame = top_frame;
	pid = key.pid;
	current_srcfile = 0;
	if ( !isc ) {
		prstatus prstat;
		::get_status( key, prstat );
		regaccess.update( prstat );
		seglist.build( key );
		EventTable *et = ::find_et(fd);
		if ( !et ) et = new EventTable;
		use_et( et );
	}
}

Process::Process( int tfd, int cfd, int x ):link(this),instr(this),seglist(this)
{
	key.pid = -1;
	key.fd = -1;
	ischild = 0;
	status = 0;
	etable = 0;
	epoch = 0;
	grabbed = 0;
	state = es_corefile;
	textfd = tfd;
	corefd = cfd;
	core = 0;
	pc = 0;
	current_srcfile = 0;
	if ( corefd != -1 )
	{
		core = new Core( cfd );
		regaccess.setup_core( corefd, core );
		regaccess.update( *(core->getstatus()) );
		pc = regaccess.getreg( REG_PC );
		find_cur_src();
	}
	seglist.readproto( textfd, corefd, core, x );
	subjectlist.add(&link);
	top_frame = new Frame(this);
	cur_frame = top_frame;
	pid = -1;
	use_et( ::find_et(textfd) );
	if ( etable->object ) {
		delete etable->object->protop;
		etable->object->protop = this;
	}
}

Process::~Process()
{
	key = dispose_key( key );
	link.remove();
	::close( textfd );
	::close( corefd );
	delete core;
}

int
Process::run( int clearsig, Iaddr destaddr, int count, int talk_ctl )
{
	if ( clearsig ) signo = 0;
	if ( (state == es_running) || (state == es_stepping) )
	{
		printe("Can't run process %d, it's already executing.\n",key.pid);
		return 0;
	}
	else if ( state == es_corefile )
	{
		printe("Can't run process, it's a core file.\n");
		return 0;
	}
	else if ( remove( hoppt ) == 0 )
	{
		printe("internal error: ");
		printe("hoppt removal failure at %#x\n", pc);
		return 0;
	}
	else if ( set(destpt,destaddr) == 0 )
	{
		printe("Could not set stop point for process %d.\n",key.pid);
		return 0;
	}
	else if ( state == es_breakpoint )
	{
		skip_count = count;
		lastpc = pc;
		retaddr = 0;
		verbosity = talk_ctl;
		goal = pg_run;
		return update_wpts() && start_stepbkpt();
	}
	else
	{
		skip_count = count;
		lastpc = pc;
		retaddr = 0;
		verbosity = talk_ctl;
		goal = pg_run;
		return update_wpts() && start_run();
	}
}

int
Process::stmt_step( int clearsig, Iaddr destaddr, int cnt, int talk_ctl )
{
	if ( clearsig ) signo = 0;
	if ( (state == es_running) || (state == es_stepping) )
	{
		printe("Can't step process %d, it's already executing.\n",key.pid);
		return 0;
	}
	else if ( state == es_corefile )
	{
		printe("Can't step process, it's a core file.\n");
		return 0;
	}
	else if ( remove( hoppt ) == 0 )
	{
		printe("internal error: ");
		printe("hoppt removal failure at %#x\n", pc);
		return 0;
	}
	else if ( set(destpt,destaddr) == 0 )
	{
		printe("Could not set stop point for process %d.\n",key.pid);
		return 0;
	}
	else if ( state == es_breakpoint )
	{
		skip_count = 0;
		lastpc = pc;
		retaddr = 0;
		ecount = cnt;
		verbosity = talk_ctl;
		goal = pg_stmt_step;
		return update_wpts() && save_start_info() && start_stepbkpt();
	}
	else if ( instr.is_bkpt( pc ) )
	{
		skip_count = 0;
		lastpc = pc;
		retaddr = 0;
		ecount = cnt;
		verbosity = talk_ctl;
		goal = pg_stmt_step;
		return update_wpts() && save_start_info() && start_run();
	}
	else
	{
		skip_count = 0;
		lastpc = pc;
		retaddr = 0;
		ecount = cnt;
		verbosity = talk_ctl;
		goal = pg_stmt_step;
		retaddr = instr.retaddr( pc );
		return update_wpts() && save_start_info() && start_step();
	}
}

int
Process::instr_step( int clearsig, Iaddr destaddr, int cnt, int talk_ctl )
{
	if ( clearsig ) signo = 0;
	if ( (state == es_running) || (state == es_stepping) )
	{
		printe("Can't step process %d, it's already executing.\n",key.pid);
		return 0;
	}
	else if ( state == es_corefile )
	{
		printe("Can't step process, it's a core file.\n");
		return 0;
	}
	else if ( remove( hoppt ) == 0 )
	{
		printe("internal error: ");
		printe("hoppt removal failure at %#x\n", pc);
		return 0;
	}
	else if ( set(destpt,destaddr) == 0 )
	{
		printe("Could not set stop point for process %d.\n",key.pid);
		return 0;
	}
	else if ( state == es_breakpoint )
	{
		skip_count = 0;
		lastpc = pc;
		retaddr = 0;
		ecount = cnt;
		verbosity = talk_ctl;
		goal = pg_instr_step;
		return update_wpts() && start_stepbkpt();
	}
	else if ( instr.is_bkpt( pc ) )
	{
		skip_count = 0;
		lastpc = pc;
		retaddr = 0;
		ecount = cnt;
		verbosity = talk_ctl;
		goal = pg_instr_step;
		return update_wpts() && start_run();
	}
	else
	{
		skip_count = 0;
		lastpc = pc;
		retaddr = 0;
		ecount = cnt;
		verbosity = talk_ctl;
		goal = pg_instr_step;
		return update_wpts() && start_step();
	}
}

int
Process::stmt_step_over( int clearsig, Iaddr destaddr, int cnt, int talk_ctl )
{
	if ( clearsig ) signo = 0;
	if ( (state == es_running) || (state == es_stepping) )
	{
		printe("Can't step process %d, it's already executing.\n",key.pid);
		return 0;
	}
	else if ( state == es_corefile )
	{
		printe("Can't step process, it's a core file.\n");
		return 0;
	}
	else if ( remove( hoppt ) == 0 )
	{
		printe("internal error: ");
		printe("hoppt removal failure at %#x\n", pc);
		return 0;
	}
	else if ( set(destpt,destaddr) == 0 )
	{
		printe("Could not set stop point for process %d.\n",key.pid);
		return 0;
	}
	else if ( state == es_breakpoint )
	{
		skip_count = 0;
		lastpc = pc;
		retaddr = 0;
		ecount = cnt;
		verbosity = talk_ctl;
		goal = pg_stmt_step_over;
		return update_wpts() && save_start_info() && start_stepbkpt();
	}
	else if ( instr.is_bkpt( pc ) )
	{
		skip_count = 0;
		lastpc = pc;
		retaddr = 0;
		ecount = cnt;
		verbosity = talk_ctl;
		goal = pg_stmt_step_over;
		return update_wpts() && save_start_info() && start_run();
	}
	else
	{
		skip_count = 0;
		lastpc = pc;
		ecount = cnt;
		verbosity = talk_ctl;
		goal = pg_stmt_step_over;
		retaddr = instr.retaddr( pc );
		return update_wpts() && save_start_info() && start_step();
	}
}

int
Process::instr_step_over( int clearsig, Iaddr destaddr, int cnt, int talk_ctl )
{
	if ( clearsig ) signo = 0;
	if ( (state == es_running) || (state == es_stepping) )
	{
		printe("Can't step process %d, it's already executing.\n",key.pid);
		return 0;
	}
	else if ( state == es_corefile )
	{
		printe("Can't step process, it's a core file.\n");
		return 0;
	}
	else if ( remove( hoppt ) == 0 )
	{
		printe("internal error: ");
		printe("hoppt removal failure at %#x\n", pc);
		return 0;
	}
	else if ( set(destpt,destaddr) == 0 )
	{
		printe("Could not set stop point for process %d.\n",key.pid);
		return 0;
	}
	else if ( state == es_breakpoint )
	{
		skip_count = 0;
		lastpc = pc;
		retaddr = 0;
		ecount = cnt;
		verbosity = talk_ctl;
		goal = pg_instr_step_over;
		return update_wpts() && start_stepbkpt();
	}
	else if ( instr.is_bkpt( pc ) )
	{
		skip_count = 0;
		lastpc = pc;
		retaddr = 0;
		ecount = cnt;
		verbosity = talk_ctl;
		goal = pg_instr_step_over;
		return update_wpts() && start_run();
	}
	else
	{
		skip_count = 0;
		lastpc = pc;
		ecount = cnt;
		verbosity = talk_ctl;
		goal = pg_instr_step_over;
		retaddr = instr.retaddr( pc );
		return update_wpts() && start_step();
	}
}

int
Process::start_run()
{
	if ( ::run_proc( key, pr_run, signo ) == 0 )
	{
		printe("Couldn't restart execution of process %d\n", key.pid);
		return 0;
	}
	else
	{
		++epoch;
		state = es_running;
		goal2 = sg_run;
		return e_monitor.track(this);
	}
}

int
Process::start_step()
{
	if ( ::step_proc( key, pr_run, signo ) == 0 )
	{
		printe("Couldn't start step of process %d\n", key.pid);
		return 0;
	}
	else
	{
		++epoch;
		state = es_stepping;
		goal2 = sg_step;
		return e_monitor.track(this);
	}
}

int
Process::start_stepbkpt()
{
	if ( lift_bkpt(latestbkpt) == 0 )
	{
		printe("Couldn't lift breakpoint in process %d\n", key.pid );
		return 0;
	}
	retaddr = instr.retaddr( pc );
	if ( ::step_proc( key, pr_run, signo ) == 0 )
	{
		printe("Couldn't start step of process %d\n", key.pid);
		return 0;
	}
	else
	{
		++epoch;
		state = es_stepping;
		goal2 = sg_stepbkpt;
		return e_monitor.track(this);
	}
}

int
Process::restart()
{
	if ( state == es_breakpoint)
	{
		return start_stepbkpt();
	}
	else if ( goal == pg_run )
	{
		return start_run();
	}
	else if ( remove( hoppt ) == 0 )
	{
		printe("internal error: ");
		printe("hoppt removal failure at %#x\n", pc);
		return 0;
	}
	else if ( instr.is_bkpt( pc ) )
	{
		return start_run();
	}
	else
	{
		retaddr = instr.retaddr( pc );
		return start_step();
	}
}

int
Process::inform( prstatus & prstat )
{
	regaccess.update( prstat );
	seglist.update_stack( key );
	pc = regaccess.getreg( REG_PC );
	if ( (prstat.pr_why == PR_REQUESTED) && (etable == 0) )
	{
		state = es_suspended;
		signo = SIGTRAP;	// it's a lie !!
		return inform_startup();
	}
	else if (( prstat.pr_why == PR_REQUESTED) || (grabbed && exec_cnt))
	{
		state = es_suspended;
		if (grabbed && exec_cnt)
			exec_cnt -= 1;
		return respond_to_sus();
	}
	else if ( prstat.pr_why == PR_SYSENTRY )
	{
		state = es_syscallent;
		tsc = prstat.pr_what;
		return respond_to_tsc();
	}
	else if ( (prstat.pr_why == PR_SYSEXIT) && (etable == 0) )
	{
		state = es_syscallxit;
		tsc = prstat.pr_what;
		signo = SIGTRAP;	// it's a lie !!
		return inform_startup();
	}
	else if ( prstat.pr_why == PR_SYSEXIT )
	{
		state = es_syscallxit;
		tsc = prstat.pr_what;
		return respond_to_tsc();
	}
	else if ( prstat.pr_why != PR_SIGNALLED )
	{
		printe("internal error: ");
		printe("can't tell why process %d stopped\n", key.pid );
		return 0;
	}
	else if ( etable == 0 )
	{
		signo = prstat.pr_what;
		return inform_startup();
	}
	else if ( goal2 == sg_run )
	{
		signo = prstat.pr_what;
		return inform_run();
	}
	else if ( goal2 == sg_step)
	{
		signo = prstat.pr_what;
		return inform_step();
	}
	else if ( goal2 == sg_stepbkpt )
	{
		signo = prstat.pr_what;
		return inform_stepbkpt();
	}
	else
	{
		state = es_signalled;
		signo = prstat.pr_what;
		return respond_to_sig();
	}
}

int
Process::inform_startup()
{
	Iaddr	addr;
	int	fdobj;

	//
	// if a breakpoint was executed, pc might need adjustment.
	// ( 386 leaves the pc one instruction past the bkpt. )
	//
	pc = instr.adjust_pc();
	if ( pc == dynpt.addr() )
	{
		signo = 0;
		state = es_breakpoint;
		latestbkpt = &dynpt;
		return respond_to_dynpt();
	}
	else if ( signo != SIGTRAP )
	{
		state = es_signalled;
		return restart();
	}
	else if ( SYSCALL_FAILED() )
	{
		// exec() failed; shell is doing path search, continue trying
		signo = 0;
		state = es_suspended;
		return restart();
	}
	else if ( --exec_cnt > 0 )
	{
		// exec() succeeded, but we're not to the subject process yet
		signo = 0;
		state = es_suspended;
		return restart();
	}
	else if ( go_past_rtl && (dynpt.addr() == 0) )
	{
		signo = 0;
		state = es_suspended;
		addr = seglist.rtl_addr( key );
		set( dynpt, addr );
		return restart();
	}
	else if ( dynpt.addr() != 0 )
	{
		signo = 0;
		state = es_stepped;
		insert_bkpt( latestbkpt );
		return restart();
	}
	else
	{
		signo = 0;
		state = es_suspended;
		seglist.build( key );
		fdobj = ::open_object( key, 0 );
		fdobj = ::open_object( key, PROCKLUDGE ); // until /proc fixed
		use_et( ::find_et( fdobj ) );
		::close( fdobj );
		return 1;
	}
}

int
Process::inform_run()
{
	if ( signo != BKPTSIG )
	{
		state = es_signalled;
		return respond_to_sig();
	}
	//
	// if stopped at a breakpoint
	// pc might be pointing at the next instruction. adjust it.
	//
	pc = instr.adjust_pc();
	if ( pc == hoppt.addr() )
	{
		signo = 0;
		remove( hoppt );
		retaddr = 0;
		state = es_stepped;
		switch ( goal )
		{
			case pg_run:
				printe("internal error:  weird goal in ");
				printe("Process::inform_run()\n");
				return 0;
			case pg_stmt_step:
				return check_stmt_step();
			case pg_instr_step:
				printe("internal error:  weird goal in ");
				printe("Process::inform_run()\n");
				return 0;
			case pg_stmt_step_over:
				return check_stmt_step_over();
			case pg_instr_step_over:
				return check_instr_step_over();
			default:
				printe("internal error:  weird goal in ");
				printe("Process::inform_run()\n");
				return 0;
		}
	}
	else if ( pc == destpt.addr() )
	{
		signo = 0;
		remove( destpt );
		state = es_suspended;
		check_watchpoints();
		find_cur_src();
		return show_current_location( 1, verbosity >= 0 );
	}
	else if ( (latestbkpt = etable->breaklist.lookup(pc)) != 0 )
	{
		signo = 0;
		state = es_breakpoint;
		return respond_to_bkpt();
	}
	else if ( pc == dynpt.addr() )
	{
		signo = 0;
		state = es_breakpoint;
		latestbkpt = &dynpt;
		return respond_to_dynpt();
	}
	else
	{
		state = es_signalled;
		signo = BKPTSIG;
		return respond_to_sig();
	}
}

int
Process::inform_step()
{
	if ( signo != SIGTRAP )
	{
		state = es_signalled;
		return respond_to_sig();
	}
	state = es_stepped;
	signo = 0;
	switch ( goal )
	{
		case pg_run:
			printe("internal error:  weird goal in ");
			printe("Process::inform_step()\n");
			return 0;
		case pg_stmt_step:
			return check_stmt_step();
		case pg_instr_step:
			return check_instr_step();
		case pg_stmt_step_over:
			return check_stmt_step_over();
		case pg_instr_step_over:
			return check_instr_step_over();
		default:
			printe("internal error:  weird goal in ");
			printe("Process::inform_step()\n");
			return 0;
	}
}

int
Process::inform_stepbkpt()
{
	insert_bkpt( latestbkpt );
	if ( signo != SIGTRAP )
	{
		state = es_signalled;
		return respond_to_sig();
	}
	signo = 0;
	state = es_stepped;
	switch ( goal )
	{
		case pg_run:
			return restart();
		case pg_stmt_step:
			return check_stmt_step();
		case pg_instr_step:
			return check_instr_step();
		case pg_stmt_step_over:
			return check_stmt_step_over();
		case pg_instr_step_over:
			return check_instr_step_over();
		default:
			printe("internal error:  weird goal in ");
			printe("Process::inform_stepbkpt()\n");
			return 0;
	}
}

int
Process::check_stmt_step()
{
	Iaddr	addr;

	if ( (destpt.addr() == pc) || interrupted )
	{
		remove( destpt );
		check_watchpoints();
		find_cur_src();
		return show_current_location( 1, verbosity >= 0 );
	}
	else if ( find_stmt(currstmt,pc) == 0 )
	{
		return 0;
	}
	else if ( currstmt.is_unknown() && (retaddr != 0 ) )
	{
		if ( (addr = instr.jmp_target( pc )) == 0 ) addr = retaddr;
		set( hoppt, addr );
		return start_run();
	}
	else if ( currstmt.is_unknown() )
	{
		return restart();
	}
//	else if ((currstmt == startstmt) && (regaccess.top_a_r() == start_a_r))
	else if ( currstmt == startstmt )
	{
		show_current_location( 0, verbosity >= 2 );
		return restart();
	}
	else if ( check_watchpoints() != 1 )
	{
		remove( destpt );
		find_cur_src();
		return show_current_location( 1, verbosity >= 0 );
	}
	else if ( destpt.addr() != 0 )
	{
		show_current_location( 1, verbosity >= 1 );
		lastpc = pc;
		return save_start_info() && restart();
	}
	else if ( ecount == -1 )
	{
		show_current_location( 1, verbosity >= 1 );
		lastpc = pc;
		return save_start_info() && restart();
	}
	else if ( --ecount > 0 )
	{
		show_current_location( 1, verbosity >= 1 );
		lastpc = pc;
		return save_start_info() && restart();
	}
	else
	{
		find_cur_src();
		return show_current_location( 1, verbosity >= 0 );
	}
}

int
Process::check_stmt_step_over()
{
	if ( (destpt.addr() == pc) || interrupted )
	{
		remove( destpt );
		check_watchpoints();
		find_cur_src();
		return show_current_location( 1, verbosity >= 0 );
	}
	else if ( retaddr != 0 )
	{
		set( hoppt, retaddr );
		return start_run();
	}
	else if ( find_stmt(currstmt,pc) == 0 )
	{
		return 0;
	}
	else if ( currstmt.is_unknown() )
	{
		return restart();
	}
//	else if ((currstmt == startstmt) && (regaccess.top_a_r() == start_a_r))
	else if ( currstmt == startstmt )
	{
		show_current_location( 0, verbosity >= 2 );
		return restart();
	}
	else if ( check_watchpoints() != 1 )
	{
		remove( destpt );
		find_cur_src();
		return show_current_location( 1, verbosity >= 0 );
	}
	else if ( destpt.addr() != 0 )
	{
		show_current_location( 1, verbosity >= 1 );
		lastpc = pc;
		return save_start_info() && restart();
	}
	else if ( ecount == -1 )
	{
		show_current_location( 1, verbosity >= 1 );
		lastpc = pc;
		return save_start_info() && restart();
	}
	else if ( --ecount > 0 )
	{
		show_current_location( 1, verbosity >= 1 );
		lastpc = pc;
		return save_start_info() && restart();
	}
	else
	{
		find_cur_src();
		return show_current_location( 1, verbosity >= 0 );
	}
}

int
Process::check_instr_step()
{
	if ( (destpt.addr() == pc) || interrupted )
	{
		remove( destpt );
		check_watchpoints();
		find_cur_src();
		return show_current_location( 0, verbosity >= 0 );
	}
	else if ( check_watchpoints() != 1 )
	{
		remove( destpt );
		find_cur_src();
		return show_current_location( 0, verbosity >= 0 );
	}
	else if ( destpt.addr() != 0 )
	{
		show_current_location( 0, verbosity >= 2 );
		lastpc = pc;
		return restart();
	}
	else if ( ecount == -1 )
	{
		show_current_location( 0, verbosity >= 2 );
		lastpc = pc;
		return restart();
	}
	else if ( --ecount > 0 )
	{
		show_current_location( 0, verbosity >= 2 );
		lastpc = pc;
		return restart();
	}
	else
	{
		find_cur_src();
		return show_current_location( 0, verbosity >= 0 );
	}
}

int
Process::check_instr_step_over()
{
	if ( (destpt.addr() == pc) || interrupted )
	{
		remove( destpt );
		check_watchpoints();
		find_cur_src();
		return show_current_location( 0, verbosity >= 0 );
	}
	else if ( (retaddr != 0) && (set( hoppt, retaddr ) != 0) )
	{
		return start_run();
	}
	else if ( check_watchpoints() != 1 )
	{
		remove( destpt );
		find_cur_src();
		return show_current_location( 0, verbosity >= 0 );
	}
	else if ( destpt.addr() != 0 )
	{
		show_current_location( 0, verbosity >= 2 );
		lastpc = pc;
		return restart();
	}
	else if ( ecount == -1 )
	{
		show_current_location( 0, verbosity >= 2 );
		lastpc = pc;
		return restart();
	}
	else if ( --ecount > 0 )
	{
		show_current_location( 0, verbosity >= 2 );
		lastpc = pc;
		return restart();
	}
	else
	{
		find_cur_src();
		return show_current_location( 0, verbosity >= 0 );
	}
}

int
Process::resume( int clearsig )
{
	if ( clearsig ) signo = 0;
	if ( (state == es_running) || (state == es_stepping) )
	{
		printe("Can't resume process %d, already executing.\n",key.pid);
		return 0;
	}
	else if ( state == es_corefile )
	{
		printe("Can't resume process, it's a core file.\n");
		return 0;
	}
	else
	{
		return update_wpts() && restart();
	}
}

Process *
find_process( int pid )
{
	Circlech *	x;
	Process *	p;

	for ( x = subjectlist.next() ; x != &subjectlist ; x = x->next() )
	{
		p = (Process*) x->item;
		if ( p->key.pid == pid )
		{
			return p;
		}
	}
	return 0;
}
