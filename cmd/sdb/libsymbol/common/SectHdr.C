#ident	"@(#)sdb:libsymbol/common/SectHdr.C	1.20"

#include	"Interface.h"
#include	"SectHdr.h"
#include	<libelf.h>
#include	<a.out.h>
#include	<stdio.h>
#include	<malloc.h>
#include	<string.h>
#include	<osfcn.h>
#include 	"Machine.h"

#define equal(x,y)	(strcmp((x),(y))==0)

SectHdr::SectHdr( int filedes )
{
	seginfo = 0;
	fd = filedes;
}

#define SHLBMAX 10

static char *stsl_names[SHLBMAX];

//
// read .lib section, populate stsl_names with static shared libs pathnames.
//

char **
SectHdr::get_stsl_names(long seekpt, long size)
{
	char *slptr, *slbuf;
	int  i, *slinc;



	if ( (slbuf = ::calloc(size, sizeof(char))) == 0 ) {
		printe("cannont malloc\n");
		return 0;
	}
	if ( lseek(fd,seekpt,0) != seekpt ) {
		printe("cannot lseek to  .lib section\n");
		return 0;
	}
	if ( read(fd,slbuf,size) < size ) {
		printe("cannot read .lib section\n");
		return 0;
	}
	slptr = slbuf;
	i = 0;
	while (slptr < slbuf + size) {
		slinc = (int *) slptr;
		if ( (stsl_names[i] = 
			(char*) malloc( (*slinc) * sizeof(int) + 1)) == 0 ) {
			printe("cannot malloc\n");
			return 0;
		}	
		(void) ::strcpy(stsl_names[i++], slptr + 2 * sizeof(int));
		slptr += (*slinc) * sizeof(int);	/* path for next lib */
	}

	return stsl_names;
}
int
SectHdr::get_elf_sect( char *sname, long & addr, long & size, long & seekpt )
{
	Elf_Cmd		cmd;
	Elf *		elf;
	Elf32_Ehdr *	ehdr;
	Elf32_Half	ndx;
	Elf_Scn	*	scn;
	Elf32_Shdr *	shdrp;
	char *		name;
	int		result;

	elf_version( EV_CURRENT );
	::lseek( fd, 0L, 0 );
	cmd = ELF_C_READ;
	if ( (elf = elf_begin( fd, cmd, 0 )) == 0 )
	{
		return 0;
	}
	else if ( (ehdr = elf32_getehdr(elf)) == 0 )
	{
		elf_end(elf);
		return 0;
	}
	ndx = ehdr->e_shstrndx;
	scn = 0;
	addr = 0;
	size = 0;
	seekpt = 0;
	result = 0;
	while ( (scn = elf_nextscn(elf,scn)) != 0 )
	{
		if ( (shdrp = elf32_getshdr(scn)) == 0 )
		{
			break;
		}
		else if ((name=elf_strptr(elf,ndx,(size_t)shdrp->sh_name)) == 0)
		{
			break;
		}
		else if ( equal( sname, name ) )
		{
			addr = shdrp->sh_addr;
			size = shdrp->sh_size;
			seekpt = shdrp->sh_offset;
			result = 1;
			break;
		}
	}
	elf_end(elf);
	return result;
}

int
SectHdr::get_coff_sect( char *sname, long & addr, long & size, long & seekpt )
{
	SCNHDR *	shdrp;
	unsigned int	i;
	long		len;
	char *		sectbuf;
	struct filehdr	header;

	::lseek(fd,0L,0);
	::read(fd,(char*)&header,FILHSZ);
	len = header.f_nscns * sizeof(SCNHDR);
	sectbuf = ::malloc( len );
	::lseek( fd, FILHSZ+header.f_opthdr, 0 );
	::read( fd, (char*)sectbuf, len );
	shdrp = (SCNHDR*)sectbuf;
	for (i = 0 ; i < header.f_nscns ; i++)
	{
		if ( equal(sname,shdrp[i].s_name) )
		{
			addr = shdrp[i].s_vaddr;
			size = shdrp[i].s_size;
			seekpt = shdrp[i].s_scnptr;
			::free( sectbuf );
			return 1;
		}
	}
	addr = 0;
	size = 0;
	seekpt = 0;
	::free( sectbuf );
	return 0;
}

int
SectHdr::getsect( char *sname, long & addr, long & size, long & seekpt )
{
	union {
		long		lmagic;
		short		smagic;
	} magic;

	::lseek( fd, 0L, 0 );
	::read( fd, (char*)&magic, sizeof(magic) );
	if ( magic.lmagic == ELFMAGIC )
	{
		return get_elf_sect( sname, addr, size, seekpt );
	}
	else if ( magic.smagic == COFFMAGIC )
	{
		return get_coff_sect( sname, addr, size, seekpt );
	}
	else
	{
		return 0;
	}
}

SectHdr::~SectHdr()
{
	if ( seginfo != 0 )
	{
		::free( (char*) seginfo );
	}
}

Seginfo *
SectHdr::get_elf_seginfo( int & count, int & shared )
{
	Elf_Cmd		cmd;
	Elf *		elf;
	Elf32_Ehdr *	ehdr;
	Elf32_Phdr *	phdr;
	int		i;

	elf_version( EV_CURRENT );
	::lseek( fd, 0L, 0 );
	cmd = ELF_C_READ;
	if ( (elf = elf_begin( fd, cmd, 0 )) == 0 )
	{
		return 0;
	}
	else if ( (ehdr = elf32_getehdr(elf)) == 0 )
	{
		elf_end(elf);
		return 0;
	}
	else if ( (phdr = elf32_getphdr(elf)) == 0 )
	{
		elf_end(elf);
		return 0;
	}
	else
	{
		count = ehdr->e_phnum;
		shared = (ehdr->e_type == ET_DYN);
		if ( seginfo != 0 )
		{
			::free( (char*) seginfo );
		}
		seginfo = (Seginfo*) ::malloc( count * sizeof(Seginfo) );
		for ( i = 0 ; i < count ; ++i )
		{
			seginfo[i].offset = phdr[i].p_offset;
			seginfo[i].vaddr = phdr[i].p_vaddr;
			seginfo[i].mem_size = phdr[i].p_memsz;
			seginfo[i].file_size = phdr[i].p_filesz;
			seginfo[i].loaded = (phdr[i].p_type == PT_LOAD );
			seginfo[i].executable = phdr[i].p_flags & PF_X;
			seginfo[i].writable = phdr[i].p_flags & PF_W;
		}
		elf_end(elf);
		return seginfo;
	}
}

Seginfo *
SectHdr::get_coff_seginfo( int & count, int & shared )
{
	long		len;
	SCNHDR *	shdrp;
	int		i;
	struct filehdr	header;
	char *		sectbuf;

	if ( ::lseek( fd, 0L, 0 ) == -1 )
	{
		return 0;
	}
	else if ( ::read( fd, (char*) &header, FILHSZ ) != FILHSZ )
	{
		return 0;
	}
	else if ( ::lseek( fd, FILHSZ + header.f_opthdr, 0 ) == -1 )
	{
		return 0;
	}
	len = header.f_nscns * sizeof(SCNHDR);
	sectbuf = ::malloc( len );
	if ( ::read( fd, (char*) sectbuf, len ) != len )
	{
		::free( sectbuf );
		return 0;
	}
	shdrp = (SCNHDR*) sectbuf;
	shared = 0;
	count = header.f_nscns;
	seginfo = (Seginfo*) ::malloc( count * sizeof(Seginfo) );
	for ( i = 0 ; i < count ; i++ )
	{
		seginfo[i].offset = shdrp[i].s_scnptr;
		seginfo[i].vaddr = shdrp[i].s_vaddr;
		seginfo[i].mem_size = shdrp[i].s_size;
		seginfo[i].file_size = shdrp[i].s_size;
		seginfo[i].loaded = equal(shdrp[i].s_name,".text") || equal(shdrp[i].s_name,".data");
		seginfo[i].executable = ((shdrp[i].s_flags & 0xf) == STYP_REG);
		seginfo[i].writable = !equal(shdrp[i].s_name,".text");
	}
	::free( sectbuf );
	return seginfo;
}

Seginfo *
SectHdr::get_seginfo( int & count, int & shared )
{
	union {
		long		lmagic;
		short		smagic;
	} magic;

	::lseek( fd, 0L, 0 );
	::read( fd, (char*)&magic, sizeof(magic) );
	if ( magic.lmagic == ELFMAGIC )
	{
		return get_elf_seginfo( count, shared);
	}
	else if ( magic.smagic == COFFMAGIC )
	{
		return get_coff_seginfo( count, shared);
	}
	else
	{
		return 0;
	}
}

int
SectHdr::find_symbol( char * symbol_name, long & addr )
{
	Elf_Cmd		cmd;
	Elf *		elf;
	Elf32_Ehdr *	ehdr;
	Elf32_Half	ndx;
	Elf_Scn	*	scn;
	Elf32_Shdr *	shdrp;
	char *		name;
	int		found_sym, found_str, result;
	int		i, count;
	long		symseekpt, symsize;
	long		strseekpt, strsize;
	Elf32_Sym *	symtab;
	char *		strtab;
	long		magic;

	::lseek( fd, 0L, 0 );
	::read( fd, (char*)&magic, sizeof(magic) );
	if ( magic != ELFMAGIC )
	{
		return 0;
	}
	elf_version( EV_CURRENT );
	::lseek( fd, 0L, 0 );
	cmd = ELF_C_READ;
	if ( (elf = elf_begin( fd, cmd, 0 )) == 0 )
	{
		return 0;
	}
	else if ( (ehdr = elf32_getehdr(elf)) == 0 )
	{
		elf_end(elf);
		return 0;
	}
	ndx = ehdr->e_shstrndx;
	scn = 0;
	found_sym = 0;
	found_str = 0;
	while ( (scn = elf_nextscn(elf,scn)) != 0 )
	{
		if ( (shdrp = elf32_getshdr(scn)) == 0 )
		{
			break;
		}
		else if ((name=elf_strptr(elf,ndx,(size_t)shdrp->sh_name)) == 0)
		{
			break;
		}
		else if ( equal( name, ".symtab" ) )
		{
			symsize = shdrp->sh_size;
			symseekpt = shdrp->sh_offset;
			found_sym = 1;
		}
		else if ( equal( name, ".strtab" ) )
		{
			strsize = shdrp->sh_size;
			strseekpt = shdrp->sh_offset;
			found_str = 1;
		}
	}
	if ( ! (found_sym && found_str) )
	{
		elf_end( elf );
		return 0;
	}
	symtab = (Elf32_Sym*) ::malloc( symsize );
	strtab = ::malloc( strsize );
	if ( ::lseek( fd, symseekpt, 0 ) < 0 )
	{
		result = 0;
	}
	else if ( ::read( fd, (char*)symtab, symsize ) != symsize )
	{
		result = 0;
	}
	else if ( ::lseek( fd, strseekpt, 0 ) < 0 )
	{
		result = 0;
	}
	else if ( ::read( fd, strtab, strsize ) != strsize )
	{
		result = 0;
	}
	else
	{
		result = 0;
		count = symsize/ sizeof(Elf32_Sym);
		for ( i = 0 ; i < count ; ++i )
		{
			if ( equal( symbol_name, strtab+symtab[i].st_name ) )
			{
				addr = symtab[i].st_value;
				result = 1;
				break;
			}
		}
	}
	::free( (char*)symtab );
	::free( strtab );
	elf_end(elf);
	return result;
}

int
SectHdr::has_debug_info()
{
	union {
		long	lmagic;
		short	smagic;
	}		magic;
	long		addr, size, seekpt;
	SCNHDR *	shdrp;
	unsigned int	i;
	long		len;
	char *		sectbuf;
	struct filehdr	header;
	int		result;

	::lseek( fd, 0L, 0 );
	::read( fd, (char*)&magic, sizeof(magic) );
	if ( magic.lmagic == ELFMAGIC )
	{
		return get_elf_sect( ".debug", addr, size, seekpt );
	}
	else if ( magic.smagic != COFFMAGIC )
	{
		return 0;
	}
	::lseek(fd,0L,0);
	::read(fd,(char*)&header,FILHSZ);
	len = header.f_nscns * sizeof(SCNHDR);
	sectbuf = ::malloc( len );
	::lseek( fd, FILHSZ+header.f_opthdr, 0 );
	::read( fd, (char*)sectbuf, len );
	shdrp = (SCNHDR*)sectbuf;
	for (i = 0 ; i < header.f_nscns ; i++)
	{
		if ( equal(".text",shdrp[i].s_name) )
		{
			result = (shdrp[i].s_lnnoptr != 0);
			::free( sectbuf );
			return result;
		}
	}
	::free( sectbuf );
	return 0;
}
