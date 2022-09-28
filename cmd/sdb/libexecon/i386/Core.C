#ident	"@(#)sdb:libexecon/i386/Core.C	1.1.1.6"
// Core.C -- provides access to core files,
// both old and new (ELF) format
//
// If old format, fake new format data as best we can.

#include "Core.h"
#include <libelf.h>
#include <memory.h>
#include "Interface.h"
#include "Machine.h"
#include "Reg1.h"

// following are for COFF core files ONLY
#include "SectHdr.h"
#include <sys/fs/s5dir.h>
#include <sys/signal.h>
#include <sys/user.h>
#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/reg.h>
#include <sys/utsname.h>


extern int debugflag;
struct CoreData {
	int		 is_coff;	// have to fake it
	char		*notes;		// copy of NOTE segment
	Elf32_Phdr	*phdr;		// copy of program header array
	int		 numphdr;	// how many phdrs?
	long		 statseek;	// seek addr of status struct
	long		 fpseek;	// seek addr of fpregs
	prstatus_t	*status;	// points into notes
	fpregset_t	*fpregs;	// ditto, or 0

	CoreData()	{ memset( (char *)this, 0, sizeof(CoreData) ); }
};

static void fake_ELF_core( CoreData *, int corefd );

Core::Core( int corefd )
{
	data = new CoreData;
	long magic = 0;
	if ( corefd < 0 )
		return;			// no core file
	::lseek(corefd, 0, 0);
	::read(corefd, &magic, sizeof magic);
	::lseek(corefd, 0, 0);
	if ( magic == ELFMAGIC ) {	// DEL E L F == ELF file
		Elf_Cmd		 cmd;
		Elf 		*elf;
		Elf32_Ehdr 	*ehdrp;
		Elf32_Phdr	*phdrp;
		int		 phnum;
	
		elf_version( EV_CURRENT );
	
		cmd = ELF_C_READ;
		if ( ( elf = elf_begin( corefd, cmd, 0 )) == 0 ) {
			printe("elf_begin() failed!\n");
			return;
		} else if ( ( ehdrp = elf32_getehdr(elf)) == 0 ) {
			printe("elf32_getehdr() failed!\n");
			return;
		}
	
		data->numphdr = phnum = ehdrp->e_phnum;
	
		if ( !phnum ) {
			printe("no program headers!\n");
			printe("phnum = %d\n", phnum );
out:		//	elf_unmap(0)( ehdrp, sizeof(Elf32_Ehdr) );
			return;
		}
	
		data->phdr = phdrp = elf32_getphdr(elf);
		while ( phnum-- > 0 ) {
			if ( phdrp->p_type == PT_NOTE ) {
				int size = phdrp->p_filesz;
				data->notes = new char[ size ];
				::lseek(corefd, phdrp->p_offset, 0);
				if ( ::read(corefd, data->notes, size) !=
								    size ) {
					printe("can't read NOTES segment!\n");
					goto out;
				}
				int namesz, descsz, type;
				char *p = data->notes;
				while ( size > 0 ) {
					namesz = *(int *)p; p += sizeof(int);
					descsz = *(int *)p; p += sizeof(int);
					type   = *(int *)p; p += sizeof(int);
					size -= 3 * sizeof(int) +
							namesz + descsz;
					p += namesz;
					switch( type ) {
					default:
						printe(
						"unknown type %d, size = %d\n",
							type, descsz);
						break;
					case 1:
						data->status = (prstatus_t *)p;
						data->statseek =
							p - data->notes +
							phdrp->p_offset;
						break;
					case 2:
						data->fpregs = (fpregset_t *)p;
						data->fpseek =
							p - data->notes +
							phdrp->p_offset;
						break;
					case 3:			// psinfo
						break;
					}
					p += descsz;
				}
				if ( !data->status )
					printe("no prstatus struct!\n");
			}
			phdrp++;
		}
		goto out;
	} else {			// old style
		fake_ELF_core( data, corefd );
	}
}

Core::~Core()
{
//	if ( data->phdr )
//		elf_unmap(0)( data->phdr, sizeof(Elf32_Phdr) );
	delete data->notes;
	delete data;
}

int
Core::numsegments()
{
	return data->numphdr;
}

Elf32_Phdr *
Core::segment( int which )
{
	if ( data->phdr && which >= 0 && which < data->numphdr )
		return data->phdr + which;
	else
		return 0;
}

prstatus_t *
Core::getstatus()
{
	return data->status;
}

fpregset_t *
Core::getfpregs()
{
	return data->fpregs;
}

long
Core::statusbase()
{
	return data->statseek;
}

long
Core::fpregbase()
{
	return data->fpseek;
}

static void
fake_ELF_core( register CoreData *d, int corefd )
{
	user_t u;
	int fpvalid;

	if ( ::read(corefd, (char *)&u, sizeof(user_t) ) != sizeof(user_t) ) {
		printe("can't get core\n");
		return;
	}


	d->numphdr = 3;		//  DATA, STACK, UBLOCK

	register Elf32_Phdr *p;

	p = d->phdr = (Elf32_Phdr *) new char [ 3 * sizeof(Elf32_Phdr) ];

	p->p_type	= PT_LOAD;	// DATA segment
	p->p_offset	= ctob(USIZE);
	p->p_vaddr	= (Elf32_Addr) u.u_exdata.ux_datorg;
	p->p_paddr	= 0;
	p->p_filesz	= ctob(u.u_dsize);
	p->p_memsz	= ctob(u.u_dsize);
	p->p_flags	= (PF_R|PF_W);
	p->p_align	= 0;

	++p;
	p->p_type	= PT_LOAD;	// STACK segment
	p->p_offset	= ctob(USIZE) + ctob(u.u_dsize);
	p->p_vaddr	= u.u_sub;
	p->p_paddr	= 0;
	p->p_filesz	= ctob(u.u_ssize);
	p->p_memsz	= ctob(u.u_ssize);
	p->p_flags	= (PF_R|PF_W);
	p->p_align	= 0;

	++p;
	p->p_type	= PT_LOAD;	// UBLOCK
	p->p_offset	= 0;
	p->p_vaddr	= UVUBLK;
	p->p_paddr	= 0;
	p->p_filesz	= ctob(USIZE);
	p->p_memsz	= ctob(USIZE);
	p->p_flags	= (PF_R);
	p->p_align	= 0;

	register prstatus_t *pr;

	pr = d->status = (prstatus_t *) new char [ sizeof(prstatus_t) ];

	memset( (char *)pr, sizeof(prstatus_t), 0 );

	int *ar0 = (int *)((long)&u + (long)u.u_ar0 - UVUBLK);

	pr->pr_reg[EAX] = ar0[EAX];
	pr->pr_reg[EBX] = ar0[EBX];
	pr->pr_reg[ECX] = ar0[ECX];
	pr->pr_reg[EDX] = ar0[EDX];
	pr->pr_reg[ESI] = ar0[ESI];
	pr->pr_reg[EDI] = ar0[EDI];
	pr->pr_reg[UESP] = ar0[UESP];
	pr->pr_reg[EBP] = ar0[EBP];
	pr->pr_reg[EIP] = ar0[EIP];
	pr->pr_reg[EFL] = ar0[EFL];
	pr->pr_reg[TRAPNO] = ar0[TRAPNO];

	fpvalid = ((struct user_t *) &u) -> u_fpvalid;
	if (fpvalid) {
		d->fpregs = (fpregset_t *) new char [ sizeof(fpregset_t) ];
		::memcpy( (char*) d->fpregs, 
			  (char*) &(((struct user_t *)&u)->u_fps.u_fpstate) ,
			  sizeof(fpregset_t) );
	}
	else
		d->fpregs = 0;

	

}
