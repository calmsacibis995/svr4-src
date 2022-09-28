#ident	"@(#)sdb:libexecon/i386/Seglist.C	1.9"

#include	"Process.h"
#include	"Core.h"
#include	"Seglist.h"
#include	"Symtab.h"
#include	"Object.h"
#include	"Segment.h"
#include	"SectHdr.h"
#include	"oslevel.h"
#include	"prioctl.h"
#include	"Interface.h"
#include	"Machine.h"
#include	<libelf.h>
#include	<link.h>
#include <osfcn.h>
#include <malloc.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/reg.h>
#include <sys/signal.h>
#include <sys/fs/s5dir.h>
#include <sys/user.h>
#include <sys/sysmacros.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/auxv.h>
extern char	executable_name[];

#define equal(x,y)	(strcmp((x),(y))==0)

class Symnode: public Link {
public:
	Symtab		sym;
	char *		pathname;
			Symnode( char *, Iaddr );
			~Symnode();
	int		get_symtable( Key );
	Iaddr		brtbl_lo;	// static shared lib branch table 
	Iaddr		brtbl_hi;
	Symnode *	next()	{ return (Symnode*)Link::next();	}
};

Symnode::Symnode( char * s, Iaddr ss_base )
{
	int	len;

	pathname = 0;
	if ( s != 0 )
	{
		len = ::strlen(s) + 1;
		pathname = ::malloc( len );
		::memcpy( pathname, s, len );
	}
	sym.symtable = 0;
	sym.ss_base = ss_base;
	brtbl_lo = 0;
	brtbl_hi = 0;
}

Symnode::~Symnode()
{
	if ( pathname != 0 )
	{
		::free( pathname );
	}
}

int
Symnode::get_symtable( Key key )
{
	int	fd;

	if ( sym.symtable != 0 )
	{
		return 1;
	}
	else if ( pathname && *pathname )
	{
		fd = ::open( pathname, O_RDONLY );
		sym.symtable = ::get_symtable( fd );
		::close( fd );
		return 1;
	}
	else
	{
		fd = ::open_object( key, 0 );
		sym.symtable = ::get_symtable( fd );
		::close( fd );
		return 1;
	}
}

Seglist::Seglist( Process * process):(1),symlist(1)
{
	mru_segment = 0;
	segment_global = 0;
	r_debug_addr = 0;
	proc = process;
	stack_lo = 0;
	stack_hi = 0;
}

Seglist::~Seglist()
{
	Segment *	seg;
	Segment *	nxt;

	for ( seg = next(); seg != 0; seg = nxt )
	{
		nxt = seg->next();
		delete seg;
	}
}

static Iaddr
find_argp( Process *proc)
{
	// can't just use class Frame, because don't have symbol table yet
	Iaddr	eip, ebp, esp, curebp;
	Itype	itype;

	esp = proc->getreg( REG_ESP );
	eip = proc->getreg( REG_EIP );
	ebp = proc->getreg( REG_EBP );


	if (!ebp || !proc->grabbed)
		return esp;

	for (;;) 
	{
		curebp = ebp;
		if ( eip == 0) 
			break;
		proc->read( curebp, Saddr, itype );
		if ( itype.iaddr ) 
		{
			ebp = itype.iaddr;
			proc->read( curebp+4, Saddr, itype );
			eip = itype.iaddr;
		} 
		else 
			break;
	}


	return (curebp + 8);
}

Iaddr
Seglist::rtld_base(Key key)
{
	if ( rtld_addr != 0 )
		return rtld_addr;

	Iaddr base = 0;
	Iaddr argp = find_argp(proc); // arg ptr

#ifdef PTRACE
	argp += 4;	/* adjust to get address of argc */
#endif

	long argc;
	if ( ::get_bytes( key, argp, &argc, 4 ) != 4 ) {
		printe("get_bytes( key, %#x, &argc, 4 ) failed\n", argp);
		return 0;
	}

	argp += 4 * (1 + argc + 1); /* skip argc, argv */

	/* argp now points at beginning of envp array (null terminated) */
	long envp;
	int i = 0;
	do {		/* find NULL at end of envp array */
		if ( ::get_bytes( key, argp, &envp, 4 ) != 4 ) {
			printe("get_bytes(key, %#x, &envp, 4) failed\n", argp);
			return 0;
		}
		argp += 4;
		i++;
	} while ( envp != 0 );

	auxv_t auxv;
	do {
		if ( ::get_bytes( key, argp, &auxv, sizeof auxv ) !=
							sizeof auxv ) {
			printe("::get_bytes( key, %#x, &auxv, %d ) failed\n",
				argp, sizeof auxv);
			return 0;
		}
		if ( auxv.a_type == AT_BASE ) {
			base = auxv.a_un.a_val;
			break;
		}
		argp += sizeof auxv;
	} while ( auxv.a_type != AT_NULL );

	rtld_addr = base;

	return base;
}

//
// build segments from a.out and static shared libs if there are any
//
int
Seglist::build_static( Key key )
{
	Segment *	seg;
	int 		fd;

	if ( (fd = ::open_object(key,0)) == -1 )
	{
		return 0;
	}
	else if ( add( fd, key, executable_name, 0, 0L, 0 ) == 0 )
	{
		return 0;
	}
	else if ( add_static_shlib( fd, key, 0 ) == 0 )
	{
		return 0;
	}
	else
	{
		seg = new Segment( key, executable_name, stack_lo,
					stack_hi - stack_lo, stack_lo, 0, 0);
		prepend( seg);
		return 1;
	}
}

//
// is addr in stack segment
//
int
Seglist::in_stack( Iaddr addr )
{
	if ( addr == 0 )
		return 0;
	if ( (stack_lo == 0) && (stack_hi == 0) )
		return ( !in_text(addr) );
	else
		return ( (stack_lo <= addr) && (addr <= stack_hi) );
}

//
// update stack segment boundaries
//
void
Seglist::update_stack(Key key)
{
	if (::update_stack(key, stack_lo, stack_hi) == 0)
		printe("cannot get stack adresses");
}

//
// get static shared lib branch table addresses
//
int
Seglist::get_brtbl( Key key, char * name )
{
	Symnode *	symnode;
	Symbol		symbol;

	symnode = (Symnode*)symlist.next();
	while ( symnode != 0) {
		if equal(symnode->pathname, name)
			break;
		symnode = symnode->next();
	}
	symnode->get_symtable( key );
	symnode->sym.find_source("branchtab", symbol);
	if ( symbol.isnull() ) {
		printe("cannot find branch table in %s\n",name);
		return 0;
	}
	symnode->brtbl_lo = symbol.pc(an_lopc);
	symnode->brtbl_hi = symbol.pc(an_hipc);
	return 1;
}

int
Seglist::build( Key key )
{
	update_stack(key);
	if ( build_static( key ) == 0 )
	{
		return 0;
	}
	else
	{
		return build_dynamic( key );
	}
}

int
Seglist::add_static_shlib( int textfd, Key key, int selection )
{
	long		addr, size, seekpt;
	int		i;
	SectHdr		secthdr(textfd);
	char		** stsl_names;
	Key		slkey;
	int		fd;

	if (secthdr.getsect( ".lib", addr, size, seekpt ) == 0)
	{
		return 1;
	}
	else if ( (stsl_names = secthdr.get_stsl_names(seekpt, size)) == 0 )
	{
		return 0;
	}
	else
	{
		for ( i = 0; stsl_names[i] != 0; ++i )
		{
			fd = ::open(stsl_names[i], O_RDONLY);
			if ( key.pid == -1 )
			{
				slkey.pid = -1;
				slkey.fd = fd;
			}
			else
			{
				slkey = key;
			}
			add( fd, slkey, stsl_names[i], selection, 0, 0 );
			get_brtbl( slkey, stsl_names[i] );
		}
		return 1;
	}
}
//
// read proto
// get segments from a.out, static shared libs ( if any )
// and core ( if exists. )
//
int
Seglist::readproto( int textfd, int corefd, Core * core, int flatmap )
{
	Segment *	seg;
	Key		textkey,datakey;
	SectHdr		secthdr(textfd);
	int		i;
	Elf32_Phdr *	phdr;
	struct stat	buf;
	long		size;

	if ( textfd < 0 )
	{
		return 0;
	}
	else if ( flatmap )
	{
		textkey.pid = -1;
		textkey.fd = textfd;
		fstat( textfd, &buf );
		size = (long) buf.st_size;
		seg = new Segment(textkey, executable_name, 0, size, 0, 0, 0);
		prepend(seg);
		return 1;
	}
	else if ( corefd < 0 )	// no core file
	{
		textkey.pid = -1;
		textkey.fd = textfd;
		add( textfd, textkey, executable_name, 0, 0, 0 );
		add_static_shlib( textfd, textkey, 0 );
		return 1;
	}
	else if ( !core )
	{
		printe("can't get core file\n");
		return 0;
	}
	else 			// core file exists
	{
		textkey.pid = -1;
		textkey.fd = textfd;
		add( textfd, textkey, executable_name, 1, 0, 0 );
		add_static_shlib( textfd, textkey, 1 );
		datakey.pid = -1;
		datakey.fd = corefd;
		i = 0;
		for ( phdr = core->segment(i) ; phdr ;
				phdr = core->segment(++i) )
		{
			// in memory, dumped
			if ( phdr->p_memsz && phdr->p_filesz )
			{
				seg = new Segment( datakey, 0, phdr->p_vaddr,
					phdr->p_filesz, phdr->p_offset,
					phdr->p_vaddr,0);
				prepend( seg );
			}
		}
		return add_dynamic_text( textfd );
	}
}

int
Seglist::add_dynamic_text( int textfd )
{
	SectHdr		secthdr( textfd );
	long		addr, size, seekpt;
	int		fd;
	Segment *	seg;
	Key		datakey;

	//
	// get symtabs for dynamic libraries
	//
	// get addr of _DYNAMIC
	//
	if ( !secthdr.getsect( ".dynamic", addr, size, seekpt ) ) {
		return 1;	// not dynamically linked
	}
	//
	// walk list, find DT_DEBUG entry
	//
	Elf32_Dyn dyn;

	if ( (seg = find_segment(addr)) == 0 ) {
		printe("can't find memory segment for _DYNAMIC (%#x)!\n", addr);
		return 0;
	}
	do {
		if ( seg->read( addr, &dyn, sizeof dyn ) != sizeof dyn ) {
			printe("Segment::read( %#x, &dyn, %d ) failed\n",
					addr, sizeof dyn);
			return 0;
		}
		if ( dyn.d_tag == DT_DEBUG ) {
			break;
		}
		addr += sizeof dyn;
	} while ( dyn.d_tag > 0 && dyn.d_tag <= DT_MAXPOSTAGS );
	//
	// get r_debug struct
	//
	addr = dyn.d_un.d_val;
	r_debug debug;
	if ( addr == 0 ) {
		return 1;
	}
	if ( (seg = find_segment(addr)) == 0 ) {
		printe("can't find memory segment for r_debug (%#x)!\n", addr);
		return 0;
	}
	if ( seg->read( addr, &debug, sizeof debug ) != sizeof debug ) {
		printe("Segment::read( %#x, &debug, %d ) failed\n",
				addr, sizeof debug);
		return 0;
	}
	addr = (Iaddr)debug.r_map;
	//
	// for each map entry
	//
	link_map map;
	while (addr) {
		if ( (seg = find_segment(addr)) == 0 ) {
			printe("can't find memory segment for r_map (%#x)!\n", addr);
			return 0;
		}
		if ( seg->read( addr, &map, sizeof map ) != sizeof map ) {
			printe("Segment::read( %#x, &map, %d ) failed\n",
					addr, sizeof map);
			return 0;
		}
		addr = (Iaddr)map.l_next;
		Iaddr nameaddr = (Iaddr)map.l_name;
		char name[1024];
		if ( nameaddr ) {
			if ( (seg = find_segment(nameaddr)) == 0 ) {
				printe(
				"can't find memory segment for name (%#x)!\n",
					nameaddr);
				return 0;
			}
			if ( seg->read( nameaddr, name, sizeof name ) <= 0 ) {
				printe(
				"Segment::read( %#x, name, %d ) failed\n",
						nameaddr, sizeof name);
				return 0;
			}
		} 
		else				 	// no name == a.out
			strcpy( name, executable_name );
		
		if ( nameaddr != 0 )
		{
			fd = ::open ( name, O_RDONLY );
			datakey.pid = -1;
			datakey.fd = fd;
			add( fd, datakey, name, 1, map.l_addr, 0 );
		}
	}
	return 1;
}

Segment *
Seglist::find_segment( Iaddr addr )
{
	Segment *	seg;

	if ( ( mru_segment != 0 ) && ( mru_segment->loaddr <= addr ) &&
		( mru_segment->hiaddr > addr ) )
	{
		return mru_segment;
	}
	for ( seg = next() ; seg != 0 ; seg = seg->next() )
	{
		if ( (addr >= seg->loaddr) && ( addr < seg->hiaddr) )
		{
			mru_segment = seg;
			return seg;
		}
	}
	return 0;
}

Symtab *
Seglist::find_symtab( Key key, Iaddr addr )
{
	Segment *	seg;

	for ( seg = next(); seg != 0 ; seg = seg->next() )
	{
		if ( (addr >= seg->loaddr) && ( addr < seg->hiaddr) )
		{
			seg->get_symtable( key );
			return &seg->sym;
		}
	}
	return 0;
}

int
Seglist::find_source( Key key, char * name, Symbol & symbol )
{
	Segment *	seg;

	for ( seg = next(); seg != 0; seg = seg->next() )
	{
		seg->get_symtable( key );
		if ( seg->sym.find_source( name, symbol ) != 0 )
		{
			return 1;
		}
	}
	return 0;
}

NameEntry *
Seglist::first_global( Key key )
{
	Symnode *	symnode;
	NameEntry *	first;

	symnode = (Symnode *) symlist.next();
	while ( symnode != 0 )
	{
		symnode->get_symtable( key );
		if ( (first = symnode->sym.first_global()) != 0 )
		{
			segment_global = symnode;
			return first;
		}
		else
		{
			symnode = symnode->next();
		}
	}
	segment_global = 0;
	return 0;
}

NameEntry *
Seglist::next_global( Key key )
{
	Symnode *	symnode;
	NameEntry *	next;

	symnode = segment_global;
	if ( symnode == 0 )
	{
		return 0;
	}
	symnode->get_symtable( key );
	if ( (next = symnode->sym.next_global()) != 0 )
	{
		return next;
	}
	symnode = symnode->next();
	while ( symnode != 0 )
	{
		next = 0;
		symnode->get_symtable( key );
		if ( (next = symnode->sym.first_global()) != 0 )
		{
			segment_global = symnode;
			return next;
		}
		else
		{
			symnode = symnode->next();
		}
	}
	segment_global = 0;
	return 0;
}

Symbol
Seglist::first_file( Key key )
{
	Symnode *	symnode;
	Symbol		first;

	symnode = (Symnode *) symlist.next();
	while ( symnode != 0 )
	{
		symnode->get_symtable( key );
		first = symnode->sym.first_symbol();
		if ( !first.isnull() )
		{
			segment_file = symnode;
			current_file = first;
			return first;
		}
		else
		{
			symnode = symnode->next();
		}
	}
	segment_file = 0;
	current_file.null();
	return first;
}

Symbol
Seglist::next_file( Key key )
{
	Symnode *	symnode;
	Symbol		next;

	symnode = segment_file;
	if ( symnode == 0 )
	{
		return next;
	}
	next = current_file.arc( an_sibling );
	if ( !next.isnull() )
	{
		current_file = next;
		return next;
	}
	symnode = symnode->next();
	while ( symnode != 0 )
	{
		symnode->get_symtable( key );
		next = symnode->sym.first_symbol();
		if ( !next.isnull() )
		{
			segment_file = symnode;
			current_file = next;
			return next;
		}
		else
		{
			symnode = symnode->next();
		}
	}
	segment_file = 0;
	current_file.null();
	return next;
}

Symbol
Seglist::find_global( Key key, char * name )
{
	Symnode *	symnode;
	Symbol		symbol;

	symnode = (Symnode*)symlist.next();
	while ( symnode != 0 )
	{
		symnode->get_symtable( key );
		symbol = symnode->sym.find_global(name);
		if ( symbol.isnull() )
		{
			symnode = symnode->next();
		}
		else
		{
			if ( symnode->brtbl_lo > 0 ) {
				//
				// static shared library
				//
				
				Iaddr addr = symbol.pc(an_lopc);
				if ( (addr >= symnode->brtbl_lo) &&
				    ( addr <= symnode->brtbl_hi) ) {
					addr = proc->instr.brtbl2fcn(addr);
					symbol = symnode->sym.find_entry(addr);
				}
			}
			return symbol;
		}
	}
	return symbol;
}

Iaddr
Seglist::find_r_debug( char * name, Key key )
{
	int		fd = ::open( name, O_RDONLY );
	SectHdr		secthdr( fd );
	long		addr;

	if ( secthdr.find_symbol( "_r_debug", addr ) == 0 )
	{
		addr = 0;
	}
	else
	{
		addr += rtld_base(key);
	}
	::close( fd );
	return addr;
}

int
Seglist::add( int fd, Key key, char * name, int text_only, Iaddr base, Segment * segment )
{
	Iaddr		addr;
	long		size;
	long		offset;
	Segment *	seg;
	int		shared;
	Iaddr		ss_base;
	Seginfo *	seginfo;
	int		i, count;
	SectHdr		secthdr( fd );

	if ( fd == -1 )
	{
		printe("cannot open %s\n",name);
		return 0;
	}
	else if ( (seginfo = secthdr.get_seginfo( count, shared )) == 0 )
	{
		return 0;
	}
	else if ( shared )
	{
		ss_base = base;
	}
	else
	{
		ss_base = 0;
	}
	for ( i = 0; i < count ; ++i )
	{
		size = seginfo[i].mem_size;
		addr = seginfo[i].vaddr;
		offset = seginfo[i].offset;
		if ( text_only && seginfo[i].writable ) 
		{
			continue;
		}
		else if ( shared )
		{
			addr += base;
		}
		if ( seginfo[i].loaded )
		{
			//
			// if segment is not writable, assume it's text
			//
			seg = new Segment(key, name, addr, size, offset, ss_base,
						!seginfo[i].writable);
			segment ? segment->prepend( seg ) : prepend(seg);
		}
	}
	add_symnode( name, base );
	return 1;
}

int
Seglist::add_symnode( char * name, Iaddr base )
{
	int		found;
	Symnode *	symnode;

	found = 0;
	symnode = ( Symnode *)symlist.next();
	while ( symnode != 0 )
	{
		if ( (name == 0) && ( symnode->pathname == 0 ) )
		{
			found = 1;
			break;
		}
		else if ( name == 0 )
		{
			symnode = symnode->next();
		}
		else if ( symnode->pathname == 0 )
		{
			symnode = symnode->next();
		}
		else if ( equal( name, symnode->pathname ) )
		{
			found = 1;
			break;
		}
		else
		{
			symnode = symnode->next();
		}
	}
	if ( !found )
	{
		symnode = new Symnode( name, base );
		symlist.prepend( symnode );
	}
	return 1;
}

//
// check if any of the symnodes has a non zero branch table address,
// which means that there is a static shared library
//
int
Seglist::has_stsl()
{
	Symnode * symnode;

	symnode = (Symnode*)symlist.next();
	while ( symnode != 0 )
	{
		if ( symnode->brtbl_lo != 0 )
			return 1;
		symnode = symnode->next();
	}
	return 0;
}

int
Seglist::build_dynamic( Key key )
{
	Segment *	seg;
	Segment *	seg2;
	long		offset;
	int		len;
	link_map	map_node;
	int		found;
	char		buf[201];
	r_debug		r;
	int		fd;

	if ( r_debug_addr == 0 )
	{
		r_debug_addr = find_r_debug( ld_so, key );
	}
	if ( r_debug_addr == 0 )
	{
		return 1;
	}
	buf[200] = '\0';
	seg = next();
	while ( (seg != 0 ) && (seg->sym.ss_base == 0) )
	{
		seg = seg->next();
	}
	len = sizeof( r_debug );
	if ( ::get_bytes( key, r_debug_addr, &r, len ) != len )
	{
		return 0;
	}
	len = sizeof( link_map );
	offset = (long) r.r_map;
	//
	// traverse the link_map to look for dynamic shared objects
	//
	while ( offset != 0 )
	{
		if ( ::get_bytes( key, offset, &map_node, len ) != len )
		{
			return 0;
		}
		else if ( map_node.l_name == 0 )
		{
			//
			// l_name == 0 for the a.out
			//
			offset = (long) map_node.l_next;
			continue;
		}
		else if ( ::get_bytes( key, (long)map_node.l_name, buf, 200 ) < 1 )
		{
			return 0;
		}
		while ( (seg != 0) && !equal( seg->pathname, buf ) )
		{
			seg2 = seg->next();
			delete seg;
			seg = seg2;
		}
		found = 0;
		while ( (seg != 0) && equal( seg->pathname, buf ) )
		{
			found = 1;
			seg = seg->next();
		}
		if ( !found )
		{
			fd = ::open( buf, O_RDONLY );
			add( fd, key, buf, 0, map_node.l_addr, seg );
		}
		offset = (long) map_node.l_next;
	}
	while ( seg != 0 )
	{
		seg2 = seg->next();
		delete seg;
		seg = seg2;
	}
	return 1;
}

int
Seglist::setup( int fd, int & rtl_used )	
{
	SectHdr	secthdr(fd);
	long	addr;
	long	size,seekpt;
	int	result;

	rtl_used = 0;
	r_debug_addr = 0;
	rtld_addr = 0;
	if ( fd == -1 )
	{
		result = 0;
	}
	else if ( secthdr.getsect(".dynamic",addr,size,seekpt) == 0 )
	{
		result = 0;
	}
	else if ( secthdr.getsect(".interp",addr,size,seekpt) == 0 )
	{
		result = 0;
	}
	else if ( ::lseek( fd, seekpt, 0 ) < 0 )
	{
		result = 0;
	}
	else if ( ::read( fd, ld_so, 128 ) != 128 )
	{
		result = 0;
	}
	else
	{
//		extern char *getenv(char*);
//		char *ld_so = getenv("LD_SO");
//		if ( !ld_so || !*ld_so ) ld_so = "/ld.so";
//////	can't find base addr until after successful exec(), must delay
//////		r_debug_addr = find_r_debug( buf );
		rtl_used = 1;
		result = 1;
	}
	::close( fd );
	return result;
}

Iaddr
Seglist::rtl_addr( Key key )
{
	r_debug	r;

	if ( r_debug_addr == 0 )
	{
		r_debug_addr = find_r_debug( ld_so, key );
	}
	if ( r_debug_addr == 0 )
	{
		return 0;
	}
	else if ( ::get_bytes( key, r_debug_addr, &r, sizeof(r) ) != sizeof(r) )
	{
		return 0;
	}
	else
	{
		return r.r_brk + rtld_base(key);
	}
}

int
Seglist::print_map()
{
	Segment *	seg;

	if ( (seg = next()) != 0 )
	{
		printx(" ADDRESS\t   SIZE\t\tOBJECT NAME\n");
	}
	while ( seg != 0 )
	{
		printx("%#.8x\t%#.8x",seg->loaddr, seg->hiaddr - seg->loaddr);
		if ( seg->pathname != 0 )
			printx("\t%s\n",seg->pathname);
		else
			printx("\n");
		seg = seg->next();
	}
	return 1;
}

int
Seglist::buildable( Key key )
{
	int		len;
	r_debug		r;

	len = sizeof( r_debug );
	if ( r_debug_addr == 0 )
	{
		r_debug_addr = find_r_debug( ld_so, key );
	}
	if ( r_debug_addr == 0 )
	{
		return 0;
	}
	else if ( ::get_bytes( key, r_debug_addr, &r, len ) != len )
	{
		return 0;
	}
	else
	{
		return r.r_state == 0; // really RT_CONSISTENT
	}
}

int
Seglist::ptr_type( Key key, Fund_type ft, Symbol &sym )
{
	Symnode *	symnode;

	symnode = (Symnode *) symlist.next();
	if ( symnode != 0 )
	{
		symnode->get_symtable( key );
		sym = symnode->sym.ptr_type( ft );
		return !sym.isnull();
	}
	return 0;
}
//
// is addr in one of the text segments
//
int
Seglist::in_text( Iaddr addr )
{
	Segment *	seg;

	if ( addr == 0 )
		return 0;
	for ( seg = next() ; seg != 0 ; seg = seg->next() ) {
		if ( seg->is_text && (addr >= seg->loaddr) && ( addr <= seg->hiaddr) )
			return 1;
	}
	return 0;
}
