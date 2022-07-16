/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-exe:elf/elf.c	1.2"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/signal.h"
#include "sys/cred.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/buf.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/mman.h"
#include "sys/kmem.h"
#include "sys/fstyp.h"
#include "sys/acct.h"
#include "sys/sysinfo.h"
#include "sys/reg.h"
#include "sys/var.h"
#include "sys/immu.h"
#include "sys/proc.h"
#include "sys/tty.h"
#include "sys/cmn_err.h"
#include "sys/debug.h"
#include "sys/rf_messg.h"
#include "sys/conf.h"
#include "sys/uio.h"
#include "sys/pathname.h"
#include "sys/systm.h"
#include "sys/elf.h"
#ifdef i386
#include "sys/elf_386.h"
#else
#include "sys/elf_M32.h"
#endif
#include "sys/auxv.h"
#include "sys/exec.h"
#include "sys/prsystm.h"

#include "vm/as.h"
#include "vm/seg.h"

#ifdef WEITEK
#include "sys/weitek.h"
#endif

#ifdef	i386		/* Unfortunate: x.out.h is needed for Xenix support */
#include "sys/x.out.h"
#endif


#ifdef i386
short elfmagic = 0x457f;		/* because of reverse byte ordering */
#else
short elfmagic = 0x7f45;		/* magic number for cunix */
#endif

#ifdef i386
u_long	elf_stackgap = (u_long) (128 * 1024 * 1024);	/* new virtual map */
u_long	elf_stackpgs = (u_long) (288 * 1024);		/* support */
extern int	userstack[];
#endif

#ifndef i386
extern int mau_present;
#endif

STATIC int getelfhead();

int
elfexec(vp, args, level, execsz, ehdp, setid)
struct vnode *vp;
struct uarg *args;
int level;
long *execsz;
exhda_t *ehdp;
int setid;
{
	Elf32_Ehdr	*ehdrp;
	Elf32_Phdr	*phdr;
	caddr_t		phdrbase = 0;
	caddr_t 	base = 0;
	char		*dlnp;
	int		dlnsize, fd, *aux, resid, error;
	long		voffset;
	struct execenv	exenv;
	struct proc *pp = u.u_procp;
	struct vnode	*nvp;
	Elf32_Phdr	*dyphdr = NULL;
	Elf32_Phdr	*stphdr = NULL;
	Elf32_Phdr	*uphdr = NULL;
	Elf32_Phdr	*junk = NULL;
	long		phdrsize;
	int		elfargs[16];
	int		postfixsize = 0;
	register	int i, hcnt;
	int		hasu = 0;
	int		hasdy = 0;
	exhda_t		dehdr;
	struct vattr	vattr;
#ifdef i386
	caddr_t		lowest_addr = (caddr_t) 0xFFFFFFFF;
#endif


	if ((error = getelfhead(&ehdrp, &phdrbase, &phdrsize, ehdp)) != 0)
		return error;

	/* determine aux size now so that stack can be built
	 * in one shot (except actual copyout of aux image)
	 * and still have this code be machine independent.
	 */
	hcnt = ehdrp->e_phnum;
	for (i = 0; i < hcnt; i++) {
		phdr = (Elf32_Phdr *)(phdrbase + (ehdrp->e_phentsize *i));
		switch(phdr->p_type) {
		case PT_INTERP:
			hasdy = 1;
			break;
		case PT_PHDR:
			hasu = 1;
#ifdef i386
			break;
		case PT_LOAD:
				if ((caddr_t)phdr->p_vaddr < lowest_addr)
					lowest_addr = (caddr_t) phdr->p_vaddr;
		break;
#endif
		}
	}
	if (hasdy) {
		args->auxsize = (hasu ? (16 * NBPW) : (10 * NBPW));
	}

#ifdef i386
	/*
	 *	New virtual map support.
	 */
	if (lowest_addr == (caddr_t) 0xFFFFFFFF ||
		(u_long)lowest_addr >= (elf_stackgap + elf_stackpgs))
		u.u_userstack = (elf_stackgap + elf_stackpgs) - sizeof(int);
	else	u.u_userstack = (u_long) userstack;
#endif

	if ((error = remove_proc(args)) != 0)
		return error;

	if ((error = mapelfexec(vp, ehdrp, phdrbase, &uphdr, &dyphdr, &stphdr, 
				&base, &voffset, execsz)) != 0)
			goto bad;

	if (stphdr != NULL){
		/* call coff stuff. */
		if (error = elf_coffshlib(vp, stphdr, execsz, ehdp))
			goto bad;
	}

	if (uphdr != NULL && dyphdr == NULL)
		goto bad;

	if (dyphdr != NULL){

		dlnsize = dyphdr->p_filesz;

		if (dlnsize > MAXPATHLEN || dlnsize <= 0)
			goto bad;

		error = exhd_getmap(ehdp,
			dyphdr->p_offset,
			dyphdr->p_filesz,
			EXHD_NOALIGN,
			(caddr_t) &dlnp);
		if (error)
			goto bad;

		if (dlnp[dlnsize - 1] != '\0')
			goto bad;

		if (error = lookupname(dlnp, UIO_SYSSPACE, FOLLOW,
			NULLVPP, &nvp))
				goto bad;


		aux = elfargs;
		if (uphdr){
			exenv.ex_brkbase = base;  
			exenv.ex_magic = elfmagic;
			exenv.ex_vp = vp;
			setexecenv(&exenv);

			*aux++ = AT_PHDR;	
			*aux++ = (int)uphdr->p_vaddr + voffset;
			*aux++ = AT_PHENT;	
			*aux++ = ehdrp->e_phentsize;	
			*aux++ = AT_PHNUM;	
			*aux++ = ehdrp->e_phnum;	
			*aux++ = AT_ENTRY;	
			*aux++ = ehdrp->e_entry + voffset;	
			postfixsize += 8 * NBPW;
		} else {
			if (error = execopen(&vp, &fd))
				goto bad;

			*aux++ = AT_EXECFD;
			*aux++ = fd;
			postfixsize += (2 * NBPW);
		}

		if ((error = execpermissions(nvp, &vattr, &dehdr, args)) != 0) {
			VN_RELE(nvp);
			goto bad;
		}

		if ((error = getelfhead(&ehdrp, &phdrbase,
				&phdrsize, &dehdr)) != 0){
			exhd_release(&dehdr);
			VN_RELE(nvp);
			goto bad;
		}

		error = mapelfexec(nvp, ehdrp, phdrbase, &junk, &junk,
				&junk, &base, &voffset, execsz);
		exhd_release(&dehdr);
		VN_RELE(nvp);
		if (error)
			goto bad;

		if (junk != NULL)
			goto bad;

		*aux++ = AT_BASE;
#ifdef i386
		*aux++ = voffset & ~(ELF_386_MAXPGSZ - 1);
#else
		*aux++ = voffset & ~(ELF_M32_MAXPGSZ - 1);
#endif
		*aux++ = AT_FLAGS;
#ifdef i386
		*aux++ = EF_I386_NONE;
#else
		*aux++ = mau_present ? EF_M32_MAU : 0;
#endif
		*aux++ = AT_PAGESZ;
		*aux++ = PAGESIZE;
		*aux++ = AT_NULL;
		*aux = 0;
		postfixsize += (8 * NBPW);
		ASSERT(postfixsize == args->auxsize);
		
	}
	
	if (*execsz > btoc(u.u_rlimit[RLIMIT_VMEM].rlim_cur)) {
		error = ENOMEM;
		goto bad;
	}

	/*
	 * Set up the users stack.
	 */
	
	if (postfixsize) {
#ifdef DEBUG
		if (args->auxaddr == (caddr_t) 0)
			cmn_err(CE_PANIC,"elfexec: invalid args auxaddr\n");
#endif
		error = execpoststack(args, elfargs, postfixsize);
		if (error)
			goto bad;
	}
		
	/* 
	 * XXX -- should get rid of this stuff.
	 */
	u.u_exdata.ux_mag = 0413;
	u.u_exdata.ux_entloc  = (caddr_t)(ehdrp->e_entry + voffset);

	/*
	 *	Unfortunate: Xenix support.
	 */
#ifdef i386
	u.u_renv = XE_V5|XE_EXEC|U_IS386|U_ISELF;
#endif

	if (!uphdr){
		exenv.ex_brkbase = base;
		exenv.ex_magic = elfmagic;
		exenv.ex_vp = vp;
		setexecenv(&exenv);
	}

	return 0;

bad:
	if (fd != -1)		/* did we open the a.out yet */
		(void)execclose(fd);

	psignal(pp,SIGKILL);

	if (error == 0)
		error = ENOEXEC;

	return error;
		
}
		

int
getelfhead(ehdrp, phdrbase, phdrsize, ehdp)
Elf32_Ehdr **ehdrp;
register caddr_t *phdrbase;
long *phdrsize;
exhda_t *ehdp;
{
	register Elf32_Ehdr *ehdr;
	int resid, error=0;

	error = exhd_getmap(ehdp, (off_t) 0, sizeof(Elf32_Ehdr),
			EXHD_4BALIGN|EXHD_KEEPMAP,
			(caddr_t)ehdrp);
	if (error)
		goto bad;
	ehdr = *ehdrp;
	/*
   	 * We got here by the first two bytes in ident 
	 */

	if (ehdr->e_ident[EI_MAG2] != ELFMAG2 
	     || ehdr->e_ident[EI_MAG3] != ELFMAG3
	     || ehdr->e_ident[EI_CLASS] != ELFCLASS32
#ifdef i386
	     || ehdr->e_ident[EI_DATA] != ELFDATA2LSB
	     || ehdr->e_machine != EM_386
#else
	     || ehdr->e_ident[EI_DATA] != ELFDATA2MSB
	     || ehdr->e_machine != EM_M32
#endif
	     || (ehdr->e_type != ET_EXEC && ehdr->e_type != ET_DYN) 
#ifndef i386
	     || ((ehdr->e_flags & EF_M32_MAU) && !mau_present)
#endif
	     || ehdr->e_phentsize == 0)
			goto bad;

	*phdrsize = ehdr->e_phnum * ehdr->e_phentsize;
	error = exhd_getmap(ehdp, (off_t)ehdr->e_phoff, *phdrsize,
			EXHD_4BALIGN|EXHD_KEEPMAP,
			(caddr_t) phdrbase);
	if (error)
		goto bad;

	return 0;

bad:
	if (error == 0)
		error = ENOEXEC;

	return error;
}



STATIC int
mapelfexec(vp, ehdr, phdrbase, uphdr, dyphdr, stphdr, base, voffset, execsz)
struct vnode *vp;
Elf32_Ehdr *ehdr;
caddr_t phdrbase;
Elf32_Phdr **uphdr, **dyphdr, **stphdr;
caddr_t *base;
long *voffset;
long *execsz;
{
	struct proc *pp = u.u_procp;
	Elf32_Phdr *phdr;
	int i, prot, error;
	caddr_t addr;
	size_t zfodsz;
	int ptload = 0;
#ifdef i386
	caddr_t	data_base = (caddr_t) 0;
#endif

	if (ehdr->e_type == ET_DYN)
		*voffset = (long)findvaddr(pp);
	else
		*voffset = 0;

	for ( i=0; i < (int)ehdr->e_phnum; i++){
		phdr = (Elf32_Phdr *)(phdrbase + (ehdr->e_phentsize *i));

		switch(phdr->p_type){
		case PT_LOAD:
			if ((*dyphdr != NULL) && (*uphdr == NULL))
				return 0;

			ptload = 1;
			prot = PROT_USER;
			if (phdr->p_flags & PF_R)
				prot |= PROT_READ;
			if (phdr->p_flags & PF_W)
				prot |= PROT_WRITE;
			if (phdr->p_flags & PF_X)
				prot |= PROT_EXEC;

			addr = (caddr_t) phdr->p_vaddr + *voffset;
			zfodsz = (size_t) phdr->p_memsz - phdr->p_filesz;

			if (error = execmap(vp, addr, phdr->p_filesz, 
					     zfodsz, phdr->p_offset, prot))
					goto bad;

#ifdef i386
			if (phdr->p_flags & PF_W) {
				if (addr > *base)
					*base = addr + phdr->p_memsz;

				/* new virtual map support */
				if (data_base == (caddr_t) 0 ||
					addr < data_base)
					u.u_exdata.ux_datorg = data_base = addr;
			}
#else
			if ((phdr->p_flags & PF_W) && addr > *base)
				*base = addr + phdr->p_memsz;
#endif

			*execsz += btoc(phdr->p_memsz);
			break;

		case PT_INTERP:
			if (ptload)
				goto bad;
			*dyphdr = phdr;
			break;

		case PT_SHLIB:
			*stphdr = phdr;
			break;

		case PT_PHDR:
			if (ptload)
				goto bad;
			*uphdr = phdr;
			break;

		case PT_NULL:
		case PT_DYNAMIC:
		case PT_NOTE:	
			break;

		default:
			break;
		}
	}
	return 0;
bad:
	if (error == 0)
		error = ENOEXEC;
	return error;
}


STATIC int
elf_coffshlib(vp, stphdr, execsz, ehdp)
struct vnode *vp;
Elf32_Phdr *stphdr;
long *execsz;
exhda_t *ehdp;
{
	int i, error;
	int dataprot = PROT_ALL;
	int textprot = PROT_ALL & ~PROT_WRITE;
	struct exdata edp, *shlb_dat, *datp;
	u_int shlb_scnsz, shlb_datsz;

	edp.ux_lsize = stphdr->p_filesz;
	edp.ux_loffset = stphdr->p_offset;

	shlb_scnsz = (edp.ux_lsize + NBPW) & (~(NBPW - 1));
	shlb_datsz = shlbinfo.shlbs * sizeof(struct exdata);
	
	shlb_dat = (struct exdata *)kmem_alloc(shlb_datsz, KM_SLEEP);

	if ((error = getcoffshlibs(vp, &edp, shlb_dat, execsz, ehdp)) != 0)
			goto done;

	datp = shlb_dat;

	for (i=0 ; i < edp.ux_nshlibs; i++, datp++){
		if (error = execmap(datp->vp, datp->ux_txtorg,
			    datp->ux_tsize, (off_t)0, datp->ux_toffset,
					textprot)){
			coffexec_err(++datp, edp.ux_nshlibs - i - 1);
			goto done;
		}

		if (error = execmap(datp->vp, datp->ux_datorg,
			   datp->ux_dsize, (off_t)datp->ux_bsize, 
			    datp->ux_doffset, dataprot)){
			coffexec_err(++datp, edp.ux_nshlibs - i - 1);
			goto done;
		}
		VN_RELE(datp->vp);	/* done with this reference */
	}
		
done:
	kmem_free(shlb_dat, shlb_datsz);

	return error;
}

	

#define WR(vp, base, count, offset, rlimit, credp) \
	vn_rdwr(UIO_WRITE, vp, (caddr_t)base, count, offset, UIO_SYSSPACE, \
	0, rlimit, credp, (int *)NULL)

typedef struct {
	Elf32_Word namesz;
	Elf32_Word descsz;
	Elf32_Word type;
	char name[8];
} Elf32_Note;

#define NT_PRSTATUS	1
#define NT_PRFPREG	2
#define NT_PRPSINFO	3

STATIC int
elfnote(vp, offsetp, type, descsz, desc, rlimit, credp)
vnode_t *vp;
off_t *offsetp;
int descsz, type;
caddr_t desc;
struct cred *credp;
rlim_t rlimit;
{
	Elf32_Note note;
	int error;

	bzero((caddr_t)&note, sizeof(note));
	bcopy("CORE", note.name, 4);
	note.type = type;
	note.namesz = 8;
	note.descsz = roundup(descsz, sizeof(Elf32_Word));
	if (error = WR(vp, &note, sizeof(note), *offsetp, rlimit, credp))
		return error;
	*offsetp += sizeof(note);
	if (error = WR(vp, desc, note.descsz, *offsetp, rlimit, credp))
		return error;
	*offsetp += note.descsz;
	return 0;
}

elfcore(vp, pp, credp, rlimit, sig)
	vnode_t *vp;
	proc_t *pp;
	struct cred *credp;
	rlim_t rlimit;
	int sig;
{
	Elf32_Ehdr ehdr;
	Elf32_Phdr *v;
	u_long hdrsz;
	off_t offset, poffset;
	int error, i, nhdrs;
	struct seg *seg;
	prstatus_t prstat;
	fpregset_t fpregs;
	prpsinfo_t psinfo;

	nhdrs = (prnsegs(pp) + 1);
	hdrsz = nhdrs * sizeof(Elf32_Phdr);

	v = (Elf32_Phdr *)kmem_zalloc(hdrsz, KM_SLEEP);

	bzero((caddr_t)&ehdr, sizeof(Elf32_Ehdr));
	ehdr.e_ident[EI_MAG0] = ELFMAG0;
	ehdr.e_ident[EI_MAG1] = ELFMAG1;
	ehdr.e_ident[EI_MAG2] = ELFMAG2;
	ehdr.e_ident[EI_MAG3] = ELFMAG3;
	ehdr.e_ident[EI_CLASS] = ELFCLASS32;
#ifdef i386
	ehdr.e_ident[EI_DATA] = ELFDATA2LSB;
#else
	ehdr.e_ident[EI_DATA] = ELFDATA2MSB;
#endif
	ehdr.e_type = ET_CORE;

#ifdef i386
	ehdr.e_machine = EM_386;
#else
	ehdr.e_machine = EM_M32;
#endif

#ifdef i386
	if (PTOU(pp)->u_fpvalid)
		ehdr.e_flags |= EF_I386_FP;
#ifdef WEITEK
	if (PTOU(pp)->u_weitek == WEITEK_HW)	/* iff process used Weitek board */
		ehdr.e_flags |= EF_I386_WEITEK;
#endif

#else
	if (mau_present)
		ehdr.e_flags |= EF_M32_MAU;
#endif

	ehdr.e_version = EV_CURRENT;
	ehdr.e_phoff = sizeof(Elf32_Ehdr);
	ehdr.e_ehsize = sizeof(Elf32_Ehdr);
	ehdr.e_phentsize = sizeof(Elf32_Phdr);
	ehdr.e_phnum = (unsigned short)nhdrs;
	if (error = WR(vp, &ehdr, sizeof(Elf32_Ehdr), 0, rlimit, credp))
		goto done;

	offset = sizeof(Elf32_Ehdr);
	poffset = sizeof(Elf32_Ehdr) + hdrsz;

	v[0].p_type = PT_NOTE;
	v[0].p_flags = PF_R;
	v[0].p_offset = poffset;
	v[0].p_filesz = (sizeof(Elf32_Note) * 2)
	  + roundup(sizeof(prstatus_t), sizeof(Elf32_Word))
	  + roundup(sizeof(prpsinfo_t), sizeof(Elf32_Word));

#ifdef i386
	if (PTOU(pp)->u_fpvalid
#ifdef WEITEK
		|| PTOU(pp)->u_weitek == WEITEK_HW
#endif
	)
#else
	if (mau_present)
#endif
		v[0].p_filesz += sizeof(Elf32_Note)
		+ roundup(sizeof(fpregset_t), sizeof(Elf32_Word));

	poffset += v[0].p_filesz;

	for (i = 1, seg = pp->p_as->a_segs; i < nhdrs; seg = seg->s_next) {
		caddr_t naddr;
		caddr_t saddr = seg->s_base;
		caddr_t eaddr = seg->s_base + seg->s_size;
		do {
			u_int prot, size;
			prot = as_getprot(pp->p_as, saddr, &naddr);
			size = naddr - saddr;
			v[i].p_type = PT_LOAD;
			v[i].p_vaddr = (Elf32_Word)saddr;
			v[i].p_memsz = size;
			if (prot & PROT_WRITE)
				v[i].p_flags |= PF_W;
			if (prot & PROT_READ)
				v[i].p_flags |= PF_R;
			if (prot & PROT_EXEC)
				v[i].p_flags |= PF_X;
			if ((prot & (PROT_WRITE|PROT_EXEC)) != PROT_EXEC) {
				v[i].p_offset = poffset;
				v[i].p_filesz = size;
				poffset += size;
			}
			saddr = naddr;
			i++;
		} while (naddr < eaddr);
	}

	error = WR(vp, v, hdrsz, offset, rlimit, credp);
	if (error)
		goto done;
	offset += hdrsz;

	prgetstatus(pp, &prstat);

	/* LINTED */
	prstat.pr_cursig = sig;

	error = elfnote(vp, &offset, NT_PRSTATUS, sizeof(prstat), 
	  (caddr_t)&prstat, rlimit, credp);
	if (error)
		goto done;

#ifdef i386
	if (PTOU(pp)->u_fpvalid
#ifdef WEITEK
		|| PTOU(pp)->u_weitek == WEITEK_HW
#endif
	) {
#else
	if (mau_present) {
#endif
		prgetfpregs(pp, &fpregs);
		error = elfnote(vp, &offset, NT_PRFPREG, sizeof(fpregs), 
		  (caddr_t)&fpregs, rlimit, credp);
		if (error)
			goto done;
	}

	prgetpsinfo(pp, &psinfo);
	error = elfnote(vp, &offset, NT_PRPSINFO, sizeof(psinfo), 
	  (caddr_t)&psinfo, rlimit, credp);

	for (i = 1; !error && i < nhdrs; i++) {
		if (v[i].p_filesz == 0)
			continue;
		error = core_seg(pp, vp, v[i].p_offset, (caddr_t)v[i].p_vaddr, 
		  v[i].p_filesz, rlimit, credp);
	}

done:
	kmem_free(v, hdrsz);
	return error;
}
