#ident	"@(#)sdb:libexecon/common/Process.ev.C	1.22.2.1"
#include	"Process.h"
#include	"EventTable.h"
#include	"Symtab.h"
#include	"Source.h"
#include	"Flags.h"
#include	"Interface.h"
#include	"Assoccmds.h"
#include	"Expr.h"
#include	"Place.h"
#include	"Rvalue.h"
#include	"Vector.h"
#include	<malloc.h>
#include	<osfcn.h>
#include	<string.h>
#ifdef i386
#include	"DbregAccess.h"
#endif
int
Process::set( Breakpoint & b, Iaddr addr )
{
	if ( IS_INSERTED( b._flags ) )
	{
		lift_bkpt( &b );
	}
	if ( addr == 0 )
	{
		NOT_INSERTED( b._flags );
		return 1;
	}
	else
	{
		b._addr = addr;
		return insert_bkpt( &b );
	}
}

int
Process::remove( Breakpoint & b )
{
	if ( IS_INSERTED( b._flags ) )
	{
		lift_bkpt( &b );
	}
	b._addr = 0;
	return 1;
}

int
Process::use_et( EventTable * e )
{
	Breakpoint *	b;

	if (e == 0)
	{
		printe("internal error: ");
		printe("zero argument to Process::use_et()\n");
		return 0;
	}
	else if ( etable != 0 )
	{
		printe("internal error: ");
		printe("nonzero etable in Process::use_et()\n");
		return 0;
	}
	else if ( e->process != 0 )
	{
		printe("internal error: ");
		printe("event table already applied\n");
		return 0;
	}
	else if ( state == es_corefile )
	{
		etable = e;
		e->process = this;
		return 1;
	}
	else if ( ::catch_sigs(key,e->siglist.sigset(),pr_run) == 0 )
	{
		printe("internal error: ");
		printe("failure to catch signals\n");
		return 0;
	}
	else
	{
		etable = e;
		e->process = this;
		b = (Breakpoint*) etable->breaklist.tfirst();
		while ( b != 0 )
		{
			if ( insert_bkpt(b) == 0 )
			{
				printe("internal error: failure to insert ");
				printe("breakpoint in Process::use_et()\n");
				return 0;
			}
			b = (Breakpoint*)b->next();
		}
		return 1;
	}
}

int
Process::drop_et()
{
	Breakpoint *	b;
	Watchpoint *	w;
	Watchpoint *	next;
	FrameId		fid;
	sigset_t	no_sigs;

	premptyset( &no_sigs );
	if (etable == 0)
	{
		return 1;
	}
	else if ( state == es_corefile )
	{
		etable = ::dispose_et( etable );
		return 1;
	}
	else if ( ::catch_sigs(key,no_sigs,pr_run) == 0 )
	{
		printe("internal error: ");
		printe("failure to stop catching signals\n");
		return 0;
	}
	else
	{
		b = (Breakpoint*) etable->breaklist.tfirst();
		while ( b != 0 )
		{
			if ( IS_INSERTED(b->_flags) && (lift_bkpt(b) == 0 ) )
			{
				printe("internal error: failure to lift ");
				printe("breakpoint in Process::drop_et()\n");
				return 0;
			}
			b = (Breakpoint*)b->next();
		}
		w = (Watchpoint*) etable->watchlist.tfirst();
		while ( w != 0 )
		{
			next = (Watchpoint*) w->next();
			fid = w->frameid;
			if ( fid.isnull() )
			{
				w = next;
			}
			else if ( etable->watchlist.tdelete( *((Avlnode*)w) ) == 0 )
			{
				printe("internal error: ");
				printe("failure to delete watchpoint ");
				printe("in Process::drop_et\n");
				return 0;
			}
			else
			{
				w = next;
			}
		}
		etable = ::dispose_et( etable );
		if ( state == es_breakpoint )
			state = es_suspended;
		return 1;
	}
}

int
Process::cleanup_et()
{
	Breakpoint *	b;
	Watchpoint *	w;
	Watchpoint *	next;
	FrameId		fid;

	if ( etable == 0 )
	{
		return 1;
	}
	else if ( state == es_corefile )
	{
		etable = ::dispose_et( etable );
		return 1;
	}
	else
	{
		b = (Breakpoint*) etable->breaklist.tfirst();
		while ( b != 0 )
		{
			NOT_INSERTED(b->_flags);
			b = (Breakpoint*)b->next();
		}
		w = (Watchpoint*) etable->watchlist.tfirst();
		while ( w != 0 )
		{
			next = (Watchpoint*) w->next();
			fid = w->frameid;
			if ( fid.isnull() )
			{
				w = next;
			}
			else if ( etable->watchlist.tdelete( *w ) == 0 )
			{
				printe("internal error: ");
				printe("failure to delete watchpoint ");
				printe("in Process::drop_et\n");
				return 0;
			}
			else
			{
				w = next;
			}
		}
		etable = ::dispose_et( etable );
		return 1;
	}
}

int
Process::insert_bkpt( Breakpoint * b )
{
	if ( b == 0 )
	{
		printe("internal error: ");
		printe("null pointer to Process::insert_bkpt()\n");
		return 0;
	}
	else if ( IS_INSERTED(b->_flags) )
	{
		printe("internal error: breakpoint already inserted ");
		printe("in Process::insert_bkpt()\n");
		return 0;
	}
	else if ( IS_DISABLED(b->_flags) )
	{
		printe("internal error: ");
		printe("breakpoint is disabled in Process::insert_bkpt()\n");
		return 0;
	}
	else if ( ::get_bytes( key, b->addr(), b->oldtext(), BKPTSIZE) == 0 )
	{
		printe("internal error: could not read original text ");
		printe("in Process::insert_bkpt()\n");
		return 0;
	}
	else if ( ::put_bytes( key, b->addr(), BKPTTEXT, BKPTSIZE) == 0 )
	{
		printe("internal error: could not write breakpoint opcode ");
		printe("in Process::insert_bkpt()\n");
		return 0;
	}
	else
	{
		INSERTED( b->_flags );
		return 1;
	}
}

int
Process::lift_bkpt( Breakpoint * b )
{
	if ( b == 0 )
	{
		printe("internal error: ");
		printe("null pointer to Process::lift_bkpt()\n");
		return 0;
	}
	else if ( !IS_INSERTED(b->_flags) )
	{
		printe("internal error: ");
		printe("breakpoint is not inserted in Process::lift_bkpt()\n");
		return 0;
	}
	else if ( ::put_bytes( key, b->addr(), b->oldtext(), BKPTSIZE) == 0 )
	{
		printe("internal error: could not write original text ");
		printe("in Process::lift_bkpt()\n");
		return 0;
	}
	else
	{
		NOT_INSERTED( b->_flags );
		return 1;
	}
}

static char *	buf;
static int	slen = 0;

static char *
concat( char * s1, char * s2 )
{
	int	len;

	len = ::strlen(s1) + ::strlen(s2) + 1;
	if ( slen == 0 )
	{
		buf = ::malloc(len);
		slen = len;
	}
	else if ( len > slen )
	{
		::free(buf);
		buf = ::malloc(len);
		slen = len;
	}
	::strcpy(buf,s1);
	::strcat(buf,s2);
	return buf;
}

enum WS	{
		ws_failure = 0,
		ws_deleted,
		ws_unchanged,
		ws_changed,
	};

Frame *
Process::find_frame( const FrameId & frameid )
{
	Frame *		frame;

	frame = topframe();
	while ( frame != 0 )
	{
		if ( frameid == frame->id() )
		{
			return frame;
		}
		frame = frame->caller();
	}
	return 0;
}

int
Process::report_wpt( Watchpoint * w, WS effect, Rvalue & rvalue, int & no_loc )
{
	char *	s;

	if ( no_loc )
	{
		printx("Watchpoints since: \n");
		show_location( this, lastpc, 1 );
		no_loc = 0;
	}
	switch ( effect )
	{
		case ws_failure:
			s = concat("internal error: ",w->expr.string());
			printx("%s\n",s);
			return 0;
		case ws_deleted:
			s = concat("ceased to exist: ",w->expr.string());
			printx("%s\n",s);
			return 1;
		case ws_unchanged:
			s = concat("unchanged: ",w->expr.string());
			rvalue.print(s);
			printx("\n");
			return 1;
		case ws_changed:
			s = concat("changed: ",w->expr.string());
			rvalue.print(s);
			printx("\n");
			return 1;
		default:
			printe("internal error: ");
			printe("weird effect %d in Process::report_wpt()\n",effect);
			return 0;
	}
}

int
Process::check_existence(Watchpoint* w, Frame*& f, Rvalue& rvalue, int& no_loc )
{
	FrameId &	fid = w->frameid;
	Iaddr		fpc;

//	if ( w->frameid.isnull() )
	if ( fid.isnull() )
	{
		f = curframe();
		return ws_unchanged;
	}
	else if ( (f = find_frame(w->frameid)) != 0 )
	{
		fpc = f->getreg( REG_PC );
	}
	if ( (f != 0 ) && (w->lopc <= fpc) && (fpc < w->hipc))
	{
		return ws_unchanged;
	}
	else if ( report_wpt( w, ws_deleted, rvalue, no_loc ) == 0 )
	{
		return 0;
	}
	else if ( etable->watchlist.tdelete( *w ) == 0 )
	{
		printe("internal error: ");
		printe("did not delete wpt in Process::check_existence\n");
		return 0;
	}
	else
	{
		return ws_deleted;
	}
}

int
Process::check_value( Watchpoint * w, Frame * f, Rvalue & rvalue, int & no_loc )
{
	if ( f == 0 )
	{
		printe("internal error: ");
		printe("null frame pointer to Process::check_value()\n");
		return 0;
	}
	else if ( w == 0 )
	{
		printe("internal error: ");
		printe("null watchpoint pointer to Process::check_value()\n");
		return 0;
	}
	else if ( w->expr.eval(EV_RHS,this,f->getreg(REG_PC),f) == 0 )
	{
		printe("w->expr.eval() failure in Process::check_value()\n");
		return 0;
	}
	else if ( w->expr.rvalue( rvalue ) == 0 )
	{
		printe("w->expr.rvalue() failure in Process::check_value()\n");
		return 0;
	}
	else if ( w->rvalue == rvalue )
	{
		return ws_unchanged;
	}
	else if ( report_wpt( w, ws_changed, rvalue, no_loc ) == 0 )
	{
		return 0;
	}
	else
	{
		return ws_changed;
	}
}

int
Process::check_watchpoints()
{
	Watchpoint *	w;
	Watchpoint *	next;
	Frame *		f;
	Rvalue		rvalue;
	int		result, no_loc;
	WS		effect;

#ifdef i386
	extern DbregAccess *proc_dbreg;

	if(proc_dbreg!=NULL){
	  if(proc_dbreg->interpret_status()==1){ // interpret watchpoint status
	    // a watchpoint has been hit.  We must return !=1 to stop process
	    return(0);
          }
        }
#endif

	if ( etable == 0 )
	{
		printe("internal error: ");
		printe("null event table in Process::check_watchpoints()\n");
		return 0;
	}
	no_loc = 1;
	result = 1;
	w = (Watchpoint*)etable->watchlist.tfirst();
	while ( w != 0 )
	{
		next = (Watchpoint*) w->next();
		if ( (effect = check_existence(w,f,rvalue,no_loc) ) == 0 )
		{
			return 0;
		}
		else if ( effect == ws_deleted )
		{
			result = 2;
			w = next;
		}
		else if ( !IS_ENABLED(w->flags) )
		{
			w = next;
		}
		else if ( (effect = check_value(w,f,rvalue,no_loc)) == 0 )
		{
			return 0;
		}
		else if ( effect == ws_changed )
		{
			result = 2;
			w = next;
		}
		else
		{
			w = next;
		}
	}
	return result;
}

int
Process::update_wpts()
{
	Watchpoint *	w;
	Watchpoint *	next;
	Frame *		f;
	FrameId		fid;

	if ( etable == 0 )
	{
		return 1;
	}
	w = (Watchpoint *)etable->watchlist.tfirst();
	while ( w != 0 )
	{
		next = (Watchpoint *)w->next();
		fid = w->frameid;
		if ( fid.isnull() )
		{
			f = curframe();
		}
		else
		{
			f = find_frame( w->frameid );
		}

		if ( f == 0 )
		{
			printe("internal error: ");
			printe("watchpoint in Process::update_watchpoints()\n");
			return 0;
		}
		else if ( w->set_value( f ) == 0 )
		{
			printe("internal error: ");
			printe("could not set watchpoint\n");
			return 0;
		}
		else
		{
			w = next;
		}
	}
	return 1;
}

int
Process::set_bkpt( Iaddr addr, Assoccmds * a, int announce )
{
	Breakpoint *	b;
	int		hpthere,dpthere;

	if ( etable == 0 )
	{
		printe("internal error: ");
		printe("null event table in Process::set_bkpt()\n");
		return 0;
	}
	else if ( seglist.find_segment( addr ) == 0 )
	{
		printe("Invalid address for breakpoint: %#x.\n", addr );
		return 0;
	}
	else if ( etable->breaklist.lookup( addr ) != 0 )
	{
		printe("Breakpoint already exists at address %#x\n", addr );
		return 0;
	}
	else if ( (b = etable->breaklist.add( addr, a, announce )) == 0 )
	{
		printe("internal error: ");
		printe("storage failure bkpt at %#x\n", addr );
		return 0;
	}
	else if ( state == es_corefile )
	{
		printx("%#x   %s\n", addr, bkpt_info( b ) );
		return 1;
	}
	else if ( (hpthere = (addr == hoppt.addr())) && (remove( hoppt )==0) )
	{
		printe("internal error: ");
		printe("hoppt removal failure at addr of bkpt %#x\n", addr );
		return 0;
	}
	else if ( (dpthere = (addr == destpt.addr())) && (remove( hoppt )==0) )
	{
		printe("internal error: ");
		printe("destpt removal failure at addr of bkpt %#x\n", addr );
		return 0;
	}
	else if ( insert_bkpt(b) == 0 )
	{
		printe("internal error: ");
		printe("insert failure bkpt at %#x\n", addr );
		return 0;
	}
	else if ( dpthere && (set(destpt,addr) == 0) )
	{
		printe("internal error: ");
		printe("destpt replacement failure at bkpt %#x\n", addr );
		return 0;
	}
	else if ( hpthere && (set(hoppt,addr) == 0) )
	{
		printe("internal error: ");
		printe("hoppt replacement failure at bkpt %#x\n", addr );
		return 0;
	}
	else
	{
		printx("%#x   %s\n", addr, bkpt_info( b ) );
		return 1;
	}
}

int
Process::remove_bkpt( Iaddr addr )
{
	Breakpoint *	b;
	int		dpthere,hpthere;

	if ( etable == 0 )
	{
		printe("internal error: ");
		printe("null event table in Process::remove_bkpt()\n");
		return 0;
	}
	else if ( (b = etable->breaklist.lookup( addr )) == 0 )
	{
		printe("No breakpoint exists at address %#x\n", addr );
		return 0;
	}
	else if ( (hpthere = (addr == hoppt.addr())) && (remove( hoppt )==0) )
	{
		printe("internal error: ");
		printe("hoppt removal failure at addr of bkpt %#x\n", addr );
		return 0;
	}
	else if ( (dpthere = (addr == destpt.addr())) && (remove( destpt )==0) )
	{
		printe("internal error: ");
		printe("destpt removal failure at addr of bkpt %#x\n", addr );
		return 0;
	}
	else if ( (state != es_corefile) && (IS_ENABLED(b->_flags)) &&
		  (lift_bkpt(b) == 0) )
	{
		printe("internal error: ");
		printe("lift failure bkpt at %#x\n", addr );
		return 0;
	}

	if ( (state == es_breakpoint) && (pc == b->addr()) )
	{
		state = es_suspended;
	}


	if ( dpthere && (set(destpt,addr) == 0) )
	{
		printe("internal error: ");
		printe("destpt replacement failure at bkpt %#x\n", addr );
		return 0;
	}
	else if ( hpthere && (set(hoppt,addr) == 0) )
	{
		printe("internal error: ");
		printe("hoppt replacement failure at bkpt %#x\n", addr );
		return 0;
	}
	else if ( etable->breaklist.remove( addr ) == 0 )
	{
		printe("internal error: ");
		printe("deletion failure bkpt at %#x\n", addr );
		return 0;
	}
	else
	{
		return 1;
	}
}

char *
Process::bkpt_info( Breakpoint * b )
{
	static char	buffer[200];
	Iaddr		addr;
	Symtab *	symtab;
	Symbol		symbol,symbol2;
	long		offset, line;
	Source		source;

	buffer[0] = '\0';
	if ( b == 0 )
	{
		return 0;
	}
	else if ( etable == 0 )
	{
		return 0;
	}
	addr = b->addr();

	if ( (symtab = find_symtab(addr)) == 0 )
	{
		return buffer;
	}
	else if ( symtab->find_source( addr, symbol ) == 0 )
	{
		symbol = symtab->find_entry( addr );
		offset = addr - symbol.pc( an_lopc );
		if ( symbol.isnull() )
			return buffer;
		else if ( offset != 0 )
			sprintf( buffer, "%s+%#x",symbol.name(),offset);
		else
			sprintf( buffer, "%s",symbol.name());
	}
	else if ( symbol.source( source ) == 0 )
	{
		symbol = symtab->find_entry( addr );
		offset = addr - symbol.pc( an_lopc );
		if ( symbol.isnull() )
			return buffer;
		else if ( offset != 0 )
			sprintf( buffer, "%s+%#x",symbol.name(),offset);
		else
			sprintf( buffer, "%s",symbol.name());
	}
	else
	{
		source.pc_to_stmt( addr, line );
		if ( line != 0 )
		{
			symbol2 = symtab->find_entry( addr );
			offset = addr - symbol2.pc( an_lopc );
			if ( offset != 0 )
				sprintf( buffer, "%s:%d\t%s+%#x",
					symbol.name(), line,
					symbol2.name(), offset);
			else
				sprintf( buffer, "%s:%d\t%s", symbol.name(),
					line, symbol2.name() );
		}
		else
		{
			symbol = symtab->find_entry( addr );
			offset = addr - symbol.pc( an_lopc );
			if ( offset != 0 )
				sprintf(buffer,"%s+%#x",symbol.name(),offset);
			else
				sprintf(buffer,"%s",symbol.name());
		}
	}
	return buffer;
}

int
Process::ask( Breakpoint * b )
{
	Iaddr		addr;
	char		ans;
	char		more;

	addr = b->addr();
	printx("%#x   %s?",addr, bkpt_info( b ) );
	if ( fread(&ans, 1, 1, stdin) != 1 )
	{
		printe("invalid input; not deleting breakpoint\n");
		return 0;
	}
	while( fread(&more, 1, 1, stdin) == 1 )
		if ( more == '\n' )
			break;
	return ans == 'y' || ans == 'd';
}

int
Process::remove_all_bkpts( int asking )
{
	Breakpoint	*b;
	Iaddr		addr;
	Vector		vector;
	int		count, i;
	Iaddr *		x;

	if ( etable == 0 )
	{
		return 1;
	}
	if ( asking && !isatty(fileno(stdin)) ) {
		printe("warning: cannot interactively delete breakpoints,\n");
		printe("stdin is not a tty; no breakpoints removed\n");
		return 1;
	}
	b = (Breakpoint *) etable->breaklist.tfirst();
	count = 0;
	while ( b != 0 )
	{
		if ( !asking || ask(b) )
		{
			addr = b->addr();
			vector.add( &addr, sizeof(addr) );
			++count;
		}
		b = (Breakpoint*) b->next();
	}
	x = (Iaddr*) vector.ptr();
	for ( i = 0 ; i < count ; i++ )
	{
		addr = *x;
		if ( remove_bkpt( addr ) == 0 )
		{
			return 0;
		}
		++x;
	}
	return 1;
}

int
Process::remove_all_wpts( int )
{
	Watchpoint	*w, *next;

	if ( etable == 0 )
	{
		return 1;
	}
	w = (Watchpoint*) etable->watchlist.tfirst();
	while ( w != 0 )
	{
		next = (Watchpoint *) w->next();
		if ( etable->watchlist.tdelete( *((Avlnode*)w) ) == 0 )
		{
			printe("internal error: ");
			printe("failure to delete watchpoint ");
			printe("in Process::remove_all_wpts\n");
			return 0;
		}
		else
		{
			w = next;
		}
	}
	return 1;
}

int
Process::display_bkpts()
{
	Breakpoint *	b;
	Iaddr		addr;
	char *		s;

	if ( etable == 0 )
	{
		printe("internal error: ");
		printe("null event table in Process::display_bkpts()\n");
		return 0;
	}

	b = (Breakpoint *) etable->breaklist.tfirst();
	while ( b != 0 )
	{
		addr = b->addr();
		if ( b->assoccmds() != 0 )
			s = b->assoccmds()->string();
		else
			s = 0;
		if ( (s != 0) && (s[0] != '\0') )
			printx("%#x   %s\t%s", addr, bkpt_info( b ), s );
		else
			printx("%#x   %s\n", addr, bkpt_info( b ) );
		b = (Breakpoint*) b->next();
	}
	return 1;
}

int
Process::set_wpt( char * estring )
{
	Watchpoint *	w;
	Place		lvalue;
	Expr		expr( estring );
	Frame *		f;

	f = curframe();
	if ( etable == 0 )
	{
		printe("internal error: ");
		printe("null event table in Process::set_wpt()\n");
		return 0;
	}
	else if ( expr.eval( EV_RHS, this, f->getreg(REG_PC), f) == 0 )
	{
		printe("invalid expression; no watchpoint set.\n");
		return 0;
	}
	else if ( etable->watchlist.lookup( expr ) != 0 )
	{
		printe("Watchpoint on %s already exists.\n", estring);
		return 0;
	}
	else if ( (w = etable->watchlist.add( expr )) == 0 )
	{
		printe("internal error: ");
		printe("storage failure of watchpoint\n");
		return 0;
	}
	else if ( w->set_value( f ) == 0 )
	{
		printe("internal error: ");
		printe("could not set value of watchpoint.\n");
		return 0;
	}
	else
	{
		return 1;
	}
}

int
Process::remove_wpt( char * estring )
{
	Place		lvalue;
	Expr		expr( estring );
	Frame *		f;

	f = curframe();
	if ( etable == 0 )
	{
		printe("internal error: ");
		printe("null event table in Process::remove_wpt()\n");
		return 0;
	}
	else if ( expr.eval( EV_LHS, this, f->getreg(REG_PC), f) == 0 )
	{
		printe("invalid expression; no watchpoint altered.\n");
		return 0;
	}
	else if ( etable->watchlist.lookup( expr ) == 0 )
	{
		printe("No watchpoint on %s exists.\n", estring);
		return 0;
	}
	else if ( etable->watchlist.remove( expr ) == 0 )
	{
		printe("internal error: ");
		printe("removal failure of watchpoint\n");
		return 0;
	}
	else
	{
		return 1;
	}
}

int
Process::set_sig_catch( sigset_t sigset )
{
	if ( ::catch_sigs( key, sigset, pr_run ) == 0 )
	{
		return 0;
	}
	else if ( etable == 0 )
	{
		return 1;
	}
	else
	{
		return etable->siglist.set_sigset( sigset );
	}
}

char *
Process::text_nobkpt( Iaddr addr )
{
	char *		s;
	Breakpoint *	b;

	s = 0;
	if ( addr == hoppt.addr() )
	{
		s = hoppt.oldtext();
	}
	if ( addr == destpt.addr() )
	{
		s = destpt.oldtext();
	}
	if ( (etable != 0 ) && ((b = etable->breaklist.lookup(addr)) != 0) )
	{
		s = b->oldtext();
	}
	if ( addr == dynpt.addr() )
	{
		s = destpt.oldtext();
	}
	return s;
}
