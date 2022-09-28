#ident	"@(#)sdb:libsymbol/common/Elfbuild.C	1.7"
#include	"Elfbuild.h"
#include	"Locdesc.h"
#include	"Syminfo.h"
#include	"Tag.h"
#include	<string.h>
#include	<libelf.h>
#include	<malloc.h>
#include	<osfcn.h>
#include	<sys/types.h>
//#include	<sys/stat.h>

#define equal(x,y)	(strcmp((x),(y))==0)

Elfbuild::Elfbuild( int filedes )
{
	fd = filedes;
	symptr = 0;
	strptr = 0;
	losym = 0;
	hisym = -1;
	lostr = 0;
	histr = -1;
}

long
Elfbuild::first_symbol()
{
	Elf_Cmd		cmd;
	Elf *		elf;
	Elf32_Ehdr *	ehdr;
	Elf32_Half	ndx;
	Elf_Scn *	scn;
	Elf32_Shdr *	shdrp;
	char *		name;
	int		found_sym, found_str;
	long		first;
	long		symsize, strsize;

	first = losym;
	if ( symptr == 0 )
	{
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
			else if ((name=elf_strptr(elf,ndx,
					(size_t)shdrp->sh_name)) == 0)
			{
				break;
			}
			else if ( equal( name, ".symtab" ) )
			{
				losym = shdrp->sh_offset;
				symsize = shdrp->sh_size;
				hisym = losym + symsize - sizeof(Elf32_Sym) + 1;
				found_sym = 1;
			}
			else if ( equal( name, ".strtab" ) )
			{
				lostr = shdrp->sh_offset;
				strsize = shdrp->sh_size;
				histr = lostr + strsize;
				found_str = 1;
			}
		}
		elf_end( elf );
		if ( ! (found_sym && found_str) )
		{
			return 0;
		}
		symptr = ::malloc( symsize );
		strptr = ::malloc( strsize );
		if ( ::lseek( fd, losym, 0 ) < 0 )
		{
			first = 0;
		}
		else if ( ::read( fd, symptr, symsize ) != symsize )
		{
			first = 0;
		}
		else if ( ::lseek( fd, lostr, 0 ) < 0 )
		{
			first = 0;
		}
		else if ( ::read( fd, strptr, strsize ) != strsize )
		{
			first = 0;
		}
		else
		{
			first = losym;
		}
	}
	return first;
}

int
Elfbuild::get_syminfo( long offset, Syminfo & syminfo )
{
	Elf32_Sym *	sym;
	long		size;

	if ( offset < losym )
	{
		return 0;
	}
	else if ( offset >= hisym )
	{
		return 0;
	}
	sym = (Elf32_Sym*)(symptr + offset - losym );
	syminfo.name = sym->st_name;
	syminfo.lo = sym->st_value;
	size = sym->st_size;
	syminfo.hi = syminfo.lo + size;
	switch ( ELF32_ST_BIND(sym->st_info) )
	{
		case STB_LOCAL:		syminfo.bind = sb_local;	break;
		case STB_GLOBAL:	syminfo.bind = sb_global;	break;
		case STB_WEAK:		syminfo.bind = sb_weak;		break;
		default:		syminfo.bind = sb_weak;		break;
	}
	syminfo.resolved = ( sym->st_shndx != 0 );
	switch ( ELF32_ST_TYPE(sym->st_info) )
	{
		case STT_NOTYPE:	syminfo.type = st_func;		break;
		case STT_OBJECT:	syminfo.type = st_object;	break;
		case STT_FUNC:		syminfo.type = st_func;		break;
		case STT_SECTION:	syminfo.type = st_section;	break;
		case STT_FILE:		syminfo.type = st_file;		break;
		default:		syminfo.type = st_none;		break;
	}
	syminfo.sibling = offset + sizeof(Elf32_Sym);
	syminfo.child = 0;
	return 1;
}

Attribute *
Elfbuild::build_record( long offset )
{
	Attribute *	attribute;
	Syminfo		syminfo;
	Locdesc		locdesc;
	Attr_value	value;
	long		len;

	if ( get_syminfo( offset, syminfo ) == 0 )
	{
		return 0;
	}
	else if ( syminfo.type == st_object )
	{
		fetalrec.add_attr( an_tag, af_tag, t_variable );
		fetalrec.add_attr( an_name, af_stringndx,
						get_name(syminfo.name) );
		locdesc.clear().addr(syminfo.lo);
		len = locdesc.size();
		value.ptr = ::malloc( len );
		::memcpy( (char*)value.ptr, (char*)locdesc.addrexp(), len );
		fetalrec.add_attr( an_location, af_locdesc, value );
		attribute = fetalrec.put_record();
	}
	else if ( syminfo.type == st_func )
	{
		fetalrec.add_attr( an_tag, af_tag, t_entry );
		fetalrec.add_attr( an_name, af_stringndx,
						get_name(syminfo.name) );
		fetalrec.add_attr( an_lopc, af_addr, syminfo.lo );
		fetalrec.add_attr( an_hipc, af_addr, syminfo.hi );
		attribute = fetalrec.put_record();
	}
	else
	{
		attribute = 0;
	}
	return attribute;
}

Attribute *
Elfbuild::make_record( long offset )
{
	Attribute *	attribute;

	if ( offset < losym )
	{
		return 0;
	}
	else if ( offset >= hisym )
	{
		return 0;
	}
	else if ( reflist.lookup( offset, attribute) )
	{
		return attribute;
	}
	else if ( (attribute = build_record( offset )) == 0 )
	{
		return 0;
	}
	else
	{
		reflist.add( offset, attribute );
		return attribute;
	}
}

char *
Elfbuild::get_name( long offset )
{
	if ( offset == 0 )
		return 0;
	else
		return strptr + offset;
}
