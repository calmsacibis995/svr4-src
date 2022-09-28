#ident	"@(#)sdb:libexecon/common/Process.io.C	1.5.1.17"
#include	"Expr.h"
#include	"Interface.h"
#include	"Process.h"
#include	"Segment.h"
#include	<osfcn.h>

int
Process::evaluate_expr( char * estring, char * fmt )
{
	Expr	expr( estring );
	Frame *	frame;
	Iaddr	pc;

	if ( (frame = curframe()) == 0 )
	{
		printe("no current frame for expression evaluation.\n");
		return 0;
	}
	pc = frame->getreg( REG_PC );
	if ( expr.print( fmt, this, pc, frame ) == 0 )
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

int
Process::evaluate_addr( char * estring, char * fmt )
{
	Expr	expr( estring );
	Frame *	frame;
	Iaddr	pc;

	if ( (frame = curframe()) == 0 )
	{
		printe("no current frame for expression evaluation.\n");
		return 0;
	}
	pc = frame->getreg( REG_PC );
	if ( expr.printaddr( fmt, this, pc, frame ) == 0 ) {
		return 0;
	}
	else
	{
		return 1;
	}
}

int
Process::read( Iaddr addr, Stype stype, Itype & itype )
{
	Segment *	seg;

	if ( state != es_corefile )
	{
		return ::get_bytes( key, addr, &itype, stype_size(stype) );
	}
	else if ( (seg = seglist.find_segment( addr )) == 0 )
	{
		return 0;
	}
	else
	{
		return seg->read( addr, stype, itype );
	}
}

int
Process::write( Iaddr addr, Stype stype, const Itype & itype )
{
	Segment *	seg;

	if ( state != es_corefile )
	{
		return ::put_bytes( key, addr, (void *)&itype, stype_size(stype) );
	}
	else if ( (seg = seglist.find_segment( addr )) == 0 )
	{
		return 0;
	}
	else
	{
		return seg->write( addr, stype, itype );
	}
}

int
Process::read( Iaddr addr, int len, char * buf )
{
	Segment *	seg;

	if ( state != es_corefile )
	{
		return ::get_bytes( key, addr, buf, len );
	}
	else if ( (seg = seglist.find_segment( addr )) == 0 )
	{
		return 0;
	}
	else
	{
		return seg->read( addr, buf, len );
	}
}

int
Process::write( Iaddr addr, void * buf, int len )
{
	Segment *	seg;

	if ( state != es_corefile )
	{
		return ::put_bytes( key, addr, buf, len );
	}
	else if ( (seg = seglist.find_segment( addr )) == 0 )
	{
		return 0;
	}
	else
	{
		return seg->write( addr, buf, len );
	}
}

int
Process::disassemble( Iaddr addr, int symbolic, Iaddr * next )
{
	char *	s;

	if ( (s = instr.deasm( addr, symbolic )) == 0 )
	{
		if ( next) 
			*next = addr + 1;
		return 0;
	}
	else
	{
		printx( "%#x %s\n", addr, s );
		if ( next) 
			*next = instr.next_instr(addr);
		return 1;
	}
}

char *
Process::stateinfo()
{
	static char	buf[100];

	switch( state )
	{
		case es_none:
			::sprintf( buf, "NO STATE for process %d",key.pid);
			break;
		case es_stepping:
			::sprintf( buf, "STEPPING process %d",key.pid);
			break;
		case es_running:
			::sprintf( buf, "RUNNING process %d",key.pid);
			break;
		case es_stepped:
			::sprintf( buf, "STEPPED process %d",key.pid);
			break;
		case es_suspended:
			::sprintf( buf, "SUSPENDED process %d",key.pid);
			break;
		case es_signalled:
			::sprintf( buf, "SIGNALED %d process %d",signo,key.pid);
			break;
		case es_breakpoint:
			::sprintf( buf, "BREAKPOINT process %d",key.pid);
			break;
		case es_syscallent:
			::sprintf( buf, "SYSTEM CALL ENTRY %d process %d",tsc,
				key.pid);
			break;
		case es_syscallxit:
			::sprintf( buf, "SYSTEM CALL EXIT %d process %d",tsc,
				key.pid);
			break;
		case es_corefile:
			::sprintf( buf, "CORE FILE" );
			break;
		default:
			::sprintf( buf, "%s %s", "internal error : BROKEN",
					"state %d for process %d",state,key.pid);
			break;
	}
	return buf;
}

Iaddr
Process::getreg( RegRef regref )
{
	return regaccess.getreg( regref );
}

int
Process::readreg( RegRef regref, Stype stype, Itype & itype )
{
	return regaccess.readreg( regref, stype, itype );
}

int
Process::writereg( RegRef regref, Stype stype, Itype & itype )
{
	if ( regaccess.writereg( regref, stype, itype ) == 0 )
	{
		return 0;
	}
	else if ( regref == REG_PC )
	{
		pc = itype.iaddr;
		state = es_suspended;
	}
	return 1;
}

int
Process::display_regs( int num_per_line )
{
	return regaccess.display_regs( num_per_line );
}

int
Process::setframe(Frame *frame, int )
{
	if ( frame != 0 && frame->valid() ) {
	    cur_frame = frame;
	    return 0;
	}
	printe("faulire (?) of Process::setframe()\n");
	return -1;
}

Frame *
Process::curframe()
{
	if ( !cur_frame || !cur_frame->valid() )
		cur_frame = topframe();

	return cur_frame;
}

Frame *
Process::topframe()
{
	if ( !top_frame || !top_frame->valid() ) {	// if out of date
		register Frame *n = top_frame ? (Frame *)top_frame->next() : 0;
		while ( top_frame ) {		// delete all frames
			delete top_frame;
			if ( n == top_frame )
				break;
			top_frame = n;
			n = top_frame ? (Frame *)top_frame->next() : 0;
		}
		top_frame = new Frame(this);	// create new top frame
		cur_frame = top_frame;
	}
	return top_frame;
}

int
Process::print_map()
{
	return seglist.print_map();
}


int
Process::show_current_location( int srclevel, int condition )
{
	if ( !condition )
	{
		return 1;
	}
	else
	{
		return show_location( this, pc, srclevel );
	}
}
//
// is addr in a text segment
//
int
Process::in_text( Iaddr addr)
{
        return seglist.in_text(addr);
}

//
// is addr in stack segment
//
int
Process::in_stack( Iaddr addr)
{
        return seglist.in_stack(addr);
}
