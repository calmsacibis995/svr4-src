/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)kern-os:main.c	1.3.4.2"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/signal.h"
#include "sys/tss.h"
#include "sys/cred.h"
#include "sys/user.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/seg.h"
#include "sys/proc.h"
#include "sys/time.h"
#include "sys/file.h"
#include "sys/evecb.h"
#include "sys/hrtcntl.h"
#include "sys/hrtsys.h"
#include "sys/priocntl.h"
#include "sys/procset.h"
#include "sys/events.h"
#include "sys/evsys.h"
#include "sys/asyncsys.h"
#include "sys/var.h"
#include "sys/debug.h"
#include "sys/utsname.h"
#include "sys/conf.h"
#include "sys/utsname.h"
#include "sys/cmn_err.h"
#include "sys/bootinfo.h"
#include "sys/sysi86.h"
#include "sys/inline.h"
#include "sys/x.out.h"
#ifdef WEITEK
#include "sys/weitek.h"
#endif
#include "sys/fp.h"
#include "vm/as.h"
#include "vm/seg_vn.h"
#include "vm/seg_dummy.h"
#include "vm/bootconf.h"
#ifdef AT386
#include "vm/page.h"
#include "sys/dmaable.h"	/* >16 Mb support */
#endif

/* well known processes */
proc_t *proc_sched;		/* memory scheduler */
proc_t *proc_init;		/* init */
proc_t *proc_pageout;		/* pageout daemon */
proc_t *proc_bdflush;		/* buffer cache flush daemon */

int	physmem;	/* Physical memory size in clicks.	*/
int	maxmem;		/* Maximum available memory in clicks.	*/
int	freemem;	/* Current available memory in clicks.	*/
vnode_t	*rootdir;
long	dudebug;
int 	nodevflag = D_OLD; /* If an old driver, devsw flag entry points here */

extern int	icode[];
extern int	szicode;
extern int	userstack[];
extern int	sanity_clk;
int		eisa_bus = 0;
unsigned int	eisa_brd_id = 0;

extern int	checksumOK;

/* workarounds for 80386 B1 stepping bugs */
extern int	do386b1_x87;	/* used for errata #10 (387 section) */
extern paddr_t	fp387cr3;	/* (errata #21) */

extern struct tss386 ktss, dftss;


/*
 * Initialization code.
 * fork - process 0 to schedule
 *      - process 1 execute bootstrap
 *
 * loop at low address in user mode -- /sbin/init
 * cannot be executed.
 */

main(argc, argv)
int	argc;
char	*argv[];
{
#ifdef	EISA
	extern int eisa_enable;
#endif	/* EISA */
	register int	(**initptr)();
	struct		vnode *swapvp;
	extern struct vnode *makespecvp();
	register int    i;
	extern int	(*io_init[])();
	extern int	(*init_tbl[])();
	extern int	(*io_start[])();
	extern int	sched();
	extern int	pageout();
	extern int	fsflush();
	extern int	kmem_freepool();
	extern short	prt_flag;
	extern short	old_prt_flag;
	int		error = 0;
	char		id;
	unsigned char	byte;
	char		portval = (char)0xFF;
	int		eisa_port = 0xC80;
	int		eisa_id_port = 0xC82;

	inituname();
 	cred_init();
 	dnlc_init();

 	/*
 	 * Set up credentials.
 	 */

 	u.u_cred = crget();

	/* Now finish the 80386 B1 stepping (errata #21) workaround */
	u.u_tss->t_cr3 |= fp387cr3;
	ktss.t_cr3 |= fp387cr3;
	dftss.t_cr3 |= fp387cr3;
	prt_where = PRW_CONS;

	/* set eisa_bus variable before driver inits */
	if ((bootinfo.id[0] == 'I') && (bootinfo.id[1] == 'D') &&
           (bootinfo.id[2] == 'N') && (bootinfo.id[3] == 'O'))
		id = bootinfo.id[4];
#ifdef ATT_EISA
	if ((id == C2) || (id == C3) || (id == C4))
		eisa_bus = 0;
	else {
              	outb(eisa_port, portval); 
		portval = inb(eisa_port);
		if (portval == (char)0xFF)
                        eisa_bus = 0;
		else {
                    	eisa_bus = 1;
			byte = inb(eisa_id_port); /* grab 0xc82 */
			eisa_brd_id = byte;
			byte = inb(eisa_id_port + 1); /* get 0xc83 */
			eisa_brd_id += byte << 8;
		}
	}
#else
	eisa_bus = 0;
#endif

	picinit();      /* initialize PICs, but do not enable interrupts */

	/*
	 * general hook for things which need to run prior to turning
	 * on interrupts.
	 */
	oem_pre_clk_init();

	clkstart();

	/* The following messages are stored only in putbuf; they will be
	** displayed on the console when flush_putbuf() is called below.
	*/
	cmn_err(CE_CONT, "^\n");	/* Need a newline for alternate console */
	cmn_err(CE_CONT, "total real memory        = %d\n", ctob(physmem));
	cmn_err(CE_CONT, "total available memory   = %d\n\n", ctob(freemem));

#ifdef	EISA
	cmn_err(CE_CONT,
		"%s bus\n\n", eisa_enable ? "EISA" : "AT386");
#endif	/* EISA */


	cmn_err(CE_CONT, "AT&T UNIX System V/386 Release %s Version %s\n",
					utsname.release, utsname.version);
	cmn_err(CE_CONT, "\nCopyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T\n");
	cmn_err(CE_CONT, "Copyright (c) 1990 UNIX System Laboratories, Inc.\n");
	cmn_err(CE_CONT, "Copyright (c) 1987, 1988 Microsoft Corp.\n");
#if defined (MB1) || defined (MB2) || defined  (MB2AT)
	cmn_err(CE_CONT, "Copyright (c) 1986, 1987, 1988, 1989, 1990 Intel Corp.\n");
#endif /* MB1 || MB2 || MB2AT */
	cmn_err(CE_CONT, "All Rights Reserved\n\n");

	/* was the bootinfo structure OK when recieved from boot? */

	if ( checksumOK != 1 )
		cmn_err(CE_CONT, "Bootinfo checksum incorrect.\n\n");

	/* Assert: (kernel stack + floating point stuff) is equal to 1 page.
	 */

	if (((char *)&u + NBPP) != (char *) &u.u_tss)
		cmn_err(CE_PANIC,"main: Invalid Kernel Stack Size in U block\n");

	/*
	 * Handle arguments to main().
	 */

	for (i = 1; i < argc; i++) {
		if (strncmp(argv[i], "AVAILKMEM=", 10) == 0) {
			extern int	availkmem;

			(void) bs_lexnum(argv[i] + 10, &availkmem);
		}
	}

 	/*
 	 * Call all system initialization functions.
 	 */

 	for (initptr= &io_init[0]; *initptr; initptr++) {
		(**initptr)();
	}

	spl0();         /* enable interrupts */

        if (eisa_bus && sanity_clk)
		sanity_init();	/* start up sanity clock */
	prt_flag = 1;	/* it's OK to use spl's and wakeup now */
	old_prt_flag = 1;
	flush_putbuf();

 	for (initptr= &init_tbl[0]; *initptr; initptr++) 
		(**initptr)();
 	for (initptr= &io_start[0]; *initptr; initptr++) 
		(**initptr)();

#ifdef AT386	/* > 16 Mb DMA support */
	if (dma_check_on)
		setup_dma_strategies();
#endif /* AT386 */

#ifdef DEBUG
	/*
	 *  Debug only - print the page mapping structure contents for
	 *  non contiguous memory chunks.
	*/
	print_page_map_table();
#endif
        /*
         * Set the scan rate and other parameters of the paging subsystem.
 	 */

 	setupclock();

 	u.u_error = 0;		/* XXX kludge for SCSI driver */
 	vfs_mountroot();	/* Mount the root file system */
 
	/*
	 *  pull in the floating point emulator.
	 */

	fpeinit();

	u.u_start = hrestime.tv_sec;

	/*
	 * We need to revisit this after prototype
	 * since we now support ELF in addition to 
	 * 386 3.2 objects - XXX
	 */

	u.u_renv = XE_V5|U_ISCOFF|U_IS386;	/* 386 COFF Sys5 */

 	/*
	 * XXX - swapconf() not yet supported for B6.
	 *	 Need to have id changes to support K13 cunix changes for
	 *	 swap handling.
	 *	 For now we go back to B5/K12 feature.
	 *
	 * This call to swapconf must come after 
	 * root has been mounted.
	 */

	if (swapdev != NODEV) {
		swapfile.bo_size = nswap;
		swapconf();
	}

	/*
	 * Initialize file descriptor info in uarea.
	 * NB:  getf() in fio.c expects u.u_nofiles >= NFPCHUNK
	 */
	u.u_nofiles = NFPCHUNK;

        schedpaging();
 
	/*
 	 * Make init process; enter scheduling loop with system process.
	 */

	spl0();

 	if (newproc(NP_INIT, NULL, &error)) {
		register struct dscr *ldt, *ldta;
		register struct gate_desc *gldt;
		extern struct gate_desc scall_dscr, sigret_dscr;

 		register proc_t *p = u.u_procp;
		proc_init = p;

		/* 
		 * we will start the user level init
		 * clear the special flags set to get
		 * past the first context switch 
		 */
		p->p_flag &= ~(SSYS | SLOCK);  
 		p->p_cstime = p->p_stime = p->p_cutime = p->p_utime = 0;

 		/*
 		 * Set up the text region to do an exec
 		 * of /sbin/init.  The "icode" is in misc.s.
 		 */

 		/*
 		 * Allocate user address space.
 		 */

 		p->p_as = as_alloc();
		if (p->p_as == NULL) {
			cmn_err(CE_PANIC,"main: as_alloc returned null\n");
		}

 		/*
 		 * Make a text segment for icode
 		 */

 		(void) as_map(p->p_as, UVTEXT,
 		    szicode, segvn_create, zfod_argsp);

 		if (copyout((caddr_t)icode, (caddr_t)(UVTEXT), szicode))
			cmn_err(CE_PANIC, "main - copyout of icode failed");

 		/*
 		 * Allocate a stack segment
 		 */

 		(void) as_map(p->p_as, userstack,
 		    ctob(SSIZE), segvn_create, zfod_argsp);

		/*
		 * 80386 B1 Errata #10 -- reserve page at 0x80000000
		 * to prevent bug from occuring.
		 */

		if (do386b1_x87) {
			(void) as_map(p->p_as, 0x80000000, ctob(1),
					segdummy_create, NULL);
		}

		/*
		 * set up LDT. We use gldt to set up the syscall
		 * and sigret call gates, and ldt/ldta for the
		 * code/data segments.
		 */
		gldt = (struct gate_desc *)u.u_procp->p_ldt;
		gldt[seltoi(USER_SCALL)] = scall_dscr;
		gldt[seltoi(USER_SIGCALL)] = sigret_dscr;

		ldt = (struct dscr *)u.u_procp->p_ldt;
		ldt += seltoi(USER_CS);
		ldt->a_base0015 = 0;
		ldt->a_base1623 = 0;
		ldt->a_base2431 = 0;
		ldt->a_lim0015 = (ushort)btoct(MAXUVADR-1);
		ldt->a_lim1619 = ((unsigned char)(btoct(MAXUVADR-1) >> 16)) & 0x0F;
		ldt->a_acc0007 = UTEXT_ACC1;
		ldt->a_acc0811 = TEXT_ACC2;

		ldta = (struct dscr *)u.u_procp->p_ldt;
		ldta += seltoi(USER_DS);
		*ldta = *ldt;
		ldta->a_acc0007 = UDATA_ACC1;
#ifdef WEITEK
		ldta->a_lim0015 = (ushort)btoct(WEITEK_MAXADDR);
		ldta->a_lim1619 = ((unsigned char)(btoct(WEITEK_MAXADDR) >> 16)) & 0x0F;
#endif
		/*
		 * set up LDT entries for floating point emulation.
		 * 2 entries: one for a 32-bit alias to the user's stack,
		 *   and one for a window into the fp save area in the
		 *   user structure.
		 */
		ldt = (struct dscr *)u.u_procp->p_ldt;
		ldt += seltoi(USER_FP);
/* #ifdef XXX - MS_EMULATOR */
		if (fp_vers == FP_COFF)
		{
/* #endif XXX - MS_EMULATOR */
			setdscrbase(ldt, &u.u_fpvalid);
			i = (int)(&u.u_fps) - (int)(&u.u_fpvalid) +
							sizeof(u.u_fps);
/* #ifdef XXX - MS_EMULATOR */
		}
		else
		{
			setdscrbase(ldt, &u.u_fps);
			i = sizeof(u.u_fps);
		}
/* #endif XXX - MS_EMULATOR */
#ifdef WEITEK
		i += sizeof(u.u_weitek_reg);
#endif
		setdscrlim(ldt, i);
		ldt->a_acc0007 = UDATA_ACC1;
		ldt->a_acc0811 = DATA_ACC2_S;

		ldt = (struct dscr *)u.u_procp->p_ldt;
		ldt += seltoi(USER_FPSTK);
		*ldt = *ldta;

 		return UVTEXT;
	}

 	if (newproc(NP_SYSPROC, NULL, &error)) {
		register proc_t *p = u.u_procp;
		proc_pageout = p;
		p->p_cstime = p->p_stime = p->p_cutime = p->p_utime = 0;
		bcopy("pageout", u.u_psargs, 8);
 		bcopy("pageout", u.u_comm, 7);
		pageout();
		cmn_err(CE_PANIC, "main: return from pageout()");
	}

 	if (newproc(NP_SYSPROC, NULL, &error)) {
		register proc_t *p = u.u_procp;
		proc_bdflush = p;
		p->p_cstime = p->p_stime = p->p_cutime = p->p_utime = 0;
		bcopy("fsflush", u.u_psargs, 8);
		bcopy("fsflush", u.u_comm, 7);
		fsflush();
		cmn_err(CE_PANIC, "main: return from fsflush()");
	}
 
	if (aio_config() && aiodmn_spawn() != 0) {
		aiodaemon();
		cmn_err(CE_PANIC, "main: return from aiodaemon()");
	}

 	if (newproc(NP_SYSPROC, NULL, &error)) {
 		/*
 		 * use "kmdaemon" rather than "kmem_freepool"
 		 * will be more intelligble for ps
 		 */
 		u.u_procp->p_cstime = u.u_procp->p_stime = 0;
		u.u_procp->p_cutime = u.u_procp->p_utime = 0;
 		bcopy("kmdaemon", u.u_psargs, 10);
 		bcopy("kmdaemon", u.u_comm, 9);
		kmem_freepool();
		cmn_err(CE_PANIC, "main: return from kmem_freepool()");
 	}
 
	pid_setmin();

	bcopy("sched", u.u_psargs, 6);
	bcopy("sched", u.u_comm, 5);

 	return (int)sched;
}
