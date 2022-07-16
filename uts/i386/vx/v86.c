/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-vx:v86.c	1.3.1.1"

/*
 * Screen Scan Delay change:
 * LIM 4.0 & EMM changes:
 * Dual-Mode floating Point support:
 * Copyright (c) 1989 Phoenix Technologies Ltd.
 * All Rights Reserved
*/

#ifdef VPIX
#include "sys/sysmacros.h"
#include "sys/inline.h"
#include "sys/param.h"
#include "sys/types.h"
#include "sys/immu.h"
#include "sys/cmn_err.h"
#include "sys/systm.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/proc.h"
#include "sys/reg.h"
#include "sys/tss.h"
#include "sys/disp.h"
#include "sys/fp.h"
#include "sys/user.h"
#include "sys/debug.h"
#include "sys/var.h"
#include "sys/ioctl.h"
#include "sys/conf.h"
#include "vm/as.h"
#include "vm/seg.h"
#include "vm/seg_vpix.h"
#include "vm/faultcode.h"
#include "sys/v86.h"
#include "sys/mman.h"
#include "sys/kmem.h"
#include "sys/bootinfo.h"

#define	PV86	PSLEP		/* Priority for v86 processes */

/* Scheduler definitions */
#include "sys/class.h"
#include "sys/ts.h"

#ifdef DEBUG2
extern mont_t	page_freelock;
#endif

extern struct seg_desc gdt[];           /* GDT table for the system */
extern time_t lbolt;                    /* Ticks from power up */

extern v86_t *v86tab[];			/* ptrs to dynamically-allocated v86_t's */
extern char v86procflag;		/* ldt-switch flag for ttrap.s */
extern char v86_slice_start;		/* Flag for start of v86 time slice */
extern int v86_slice_size;		/* Time slice for v86 processes */
extern int v86timer;			/* Current ticks to go for timer */
extern int num_v86procs;		/* Number of active v86 processes */

extern int donice();

int	v86_set_nice();			/* Change TS proc's nice(2) value */
STATIC char v86_get_nice();		/* Get TS proc's nice(2) value */

STATIC int _v86physmap();

/*  v86syscall() - SI86V86 subsystem call of SYSI86 system call.
**  The v86 system call is itself a subsystem call of sysi86 system
**  call. The subsystem calls of SI86V86 are defined in v86.h and
**  processed here.
*/

int
v86syscall()
{
	register  struct  a
	{
		unsigned long   sysi86cmd;      /* SI86V86 number          */
		unsigned long   v86cmd;         /* V86 subsystem call      */
		union ioctl_arg arg;            /* Access first argument   */
		long     arg2;                  /* Rest of the arguments   */
	} *uap = (struct a *) u.u_ap;
#define         arg1            arg.larg        /* First argument to call  */


	switch (uap->v86cmd)                    /* Proces sub system calls */
	{
		case V86SC_INIT:                /* v86init() system call   */
			return v86init((caddr_t)(uap->arg1));

		case V86SC_SLEEP:               /* v86sleep() system call  */
			return v86sleep();

		case V86SC_MEMFUNC:             /* v86memfunc() sys call   */
			return v86memfunc((caddr_t)(uap->arg1));

		case V86SC_IOPL:                /* v86iopl() system call */
			return v86iopl(uap->arg1);

		default:                        /* Invalid system call     */
			return EINVAL;
	}
}


/*
** v86_get_nice function.  This function looks up the nice(2) value
** of the v86 proc using the time-sharing (TS) scheduler class
** specific process data structure linked into the v86 process's
** proc structure.  If the process has been moved into the real-time (RT)
** or some other scheduling class, -1 is returned, since the nice value
** is not defined for the other classes.
*/

STATIC char
v86_get_nice(p)
proc_t *p;
{
	/* The 'class' array is declared in sys/class.h */
	if (strcmp(class[p->p_cid].cl_name, "TS") == 0)
		return(((tsproc_t *) p->p_clproc)->ts_nice);
	else return(-1);
}


/*
** v86_set_nice routine.  This routine sets the nice value of the
** v86 proc using the scheduler class-independent donice() routine in disp.c.
** If the process is not in the time sharing class (i.e. it has been moved
** into the real-time (RT) or some other scheduling class), nothing is
** done, since the nice value is not defined for the other classes.
** The donice routine uses an increment value rather than an absolute
** nice value, so we convert the passed value by subtracting the current
** nice value from the proc structure by calling v86_get_nice().
*/

int
v86_set_nice(p, val)
proc_t *p;
char val;
{
	return(donice(p, p->p_cred, (int) (val-v86_get_nice(p)),
			(int *) NULL));
}

/*
**  v86init() system call.  Enroll the calling process as a
**  virtual 8086 ECT, initialize its XTSS, create and map in a
**  virtual 8086 segment, and set up a task gate so the ECT can
**  switch modes at the user level.
*/

int
v86init(arg)
caddr_t arg;
{
	v86_t *vp;
	register v86_t **v86pp;	
	register xtss_t *xtp;
	ulong xtsssegsize;
	struct v86parm viparm;
	proc_t *pp = u.u_procp;
	flags_t *flags = (flags_t *)&u.u_ar0[EFL];
	int i, error, size;
	struct	seg	*seg;
	struct	as	*as;
	faultcode_t	fc;

	/* Base vpix segment has 2 holes: from 1M to the XTSS, and after the
	   XTSS to 2M. */
	static struct segvpix_crargs vpix_base_args = {
		2, {{ (addr_t)ptob(V86VIRTSIZE),
			(u_int)XTSSADDR - ptob(V86VIRTSIZE) },
		    { XTSSADDR + ptob(1),
			ptob(2 * V86VIRTSIZE - 1) - (u_int)XTSSADDR }}
	};

#ifdef DEBUG1
	cmn_err(CE_CONT,"v86init: \n"); 
#endif

/*
**  v86init() is available only to the superuser.
*/
	if (!suser(u.u_cred)) {
		cmn_err(CE_WARN, "VP/ix not super user");
		return EPERM; 
	}

	if ((as = pp->p_as) == (struct as *)NULL)
		cmn_err(CE_PANIC, "v86init: no as allocated");

/*
**  We have nowhere to put a virtual 8086 if virtual address 0 is in use.
*/
	if (as_segat(as, 0))
		return ENOMEM;

/*
**  Copy in the argument structure.
*/
	if (copyin(arg, (caddr_t)&viparm, sizeof (struct v86parm)))
		return EFAULT;

/*
**  Old ECTs give a single size which is the size of the XTSS structure
**  including the I/O bitmap.  Accept that size to mean an old XTSS.
**  New ECTs give a separate size for XTSS structure and I/O bitmap.
**  For these, check that the XTSS size matches an xtss_t and check
**  that the I/O bitmap size is non-neagtive and small enough to fit
**  in the same page.
*/
#define OLDXTSSSIZE     301     /* Including bitmap */
#define OLDPARMSIZE	8	/* struct {xtss_t *xtssp; ulong szxtss;} */

	if (viparm.szxtss == OLDXTSSSIZE)
	    xtsssegsize = OLDXTSSSIZE;
	else
	{   if (viparm.magic[0] != V86_MAGIC0 ||
		    viparm.magic[1] != V86_MAGIC1)
	    {
		return EINVAL;
	    }
	    xtsssegsize = viparm.szxtss + viparm.szbitmap;
	    if (viparm.szxtss != (ulong)sizeof (xtss_t) ||
		    xtsssegsize > (ulong)NBPC)
	    {
		return EINVAL;
	    }
	}

/*
**  Find a free slot in the v86 table.
*/

	for (v86pp = v86tab, i = 0; *v86pp != (v86_t *)NULL; v86pp++) {
		if (++i >= NV86TASKS)
			return EAGAIN;
	}

	if ((*v86pp = (v86_t *)kmem_zalloc(sizeof(v86_t), KM_SLEEP)) == NULL) {
#ifdef DEBUG1
		cmn_err(CE_CONT, "v86init(): cannot allocate memory.\n");
#endif
		return ENOMEM;
	}
	vp = *v86pp;	/* actual v86_t */

	vp->vp_oldtr = 0;
	vp->vp_szxtss = xtsssegsize;              /* Set size in V86 struct */
	vp->vp_mem.vmem_cmd = 0;                  /* Start not map or track */
	vp->vp_mem.vmem_physaddr = 0;             /* Start with no address  */
	vp->vp_mem.vmem_membase = 0;              /* Start with no address  */
	vp->vp_mem.vmem_memlen = 0;               /* Start with no length   */
	vp->vp_wakeup = 0;                        /* Reset wakeup flag      */

	/* The time slice stuff is no longer valid in SVR4, but other routines
	** in this file seem to care about these values, so we'll continue to
	** set them, even though the scheduler won't use them. */

	vp->vp_slice_shft = V86_SHIFT_NORM;       /* Reset time slice shift */
	vp->vp_nice = v86_get_nice(pp);	          /* Save nice value */
	vp->vp_hpnice = vp->vp_nice - V86_DELTA_NICE;	/* high prty nice val */
	if (vp->vp_hpnice < 0) vp->vp_hpnice = 0;
	vp->vp_new_nice = -1;
	vp->vp_pri_state = V86_PRI_NORM;	  /* Set normal priority    */

/*
**  Create and map the virtual 86 segment.
*/
	/* Get the one megabyte needed  for  the  v86  process,  and */
	/* another  megabyte  where  the  only  memory  that is used */
	/* should be for the xtss.  Except for the xtss, we will try */
	/* not to allow any addition pages in the second megabyte to */
	/* be allocated (although pages may be mapped  to  pages  in */
	/* the  first  megabyte).  This will allow us to put most of */
	/* the second megabyte back into avail[rs]mem.               */
	
	size = ctob(2*V86VIRTSIZE);
	if (error = as_map(as, (caddr_t)0, size, segvpix_create, &vpix_base_args)) {
	    vp->vp_procp = (proc_t *)NULL;
	    pp->p_v86 = (v86_t *)NULL;
#ifdef DEBUG1
	   cmn_err(CE_CONT,"v86init: as_map failed\n"); 
#endif
	    return error;        
	}

	if ((seg = as_segat(as, (caddr_t)0)) == (struct seg *) NULL)
		cmn_err(CE_PANIC,"v86init: No segment for 0 address space\n");

	/* Need a hook from the segment structure  back  to  the  v86 */
	/* data  structure.   This  presumes  that  v86 segments will */
	/* never be shared.                                          */

	((struct segvpix_data *)seg->s_data)->s_vpix = v86pp - v86tab;


	/* Set up the wrap area equivalence */
	segvpix_range_equiv(seg, V86VIRTSIZE, 0, V86WRAPSIZE);


/*
**  Nail all the pages of the new XTSS.  We mustn't allow them to be
**  paged out as long as this process is SLOAD'ed.  If we're deactivated,
**  the XTSS pages can be un-nailed and swapped.
**
**  Note that these pages are zeroed when they are faulted.  Therefore this code
**  zeros out the new XTSS as a side-effect.
*/

	/*
	 *	Fault in the extended TSS pages and lock them in memory.
	 *	segvpix_faultpage won't do PAGE_RELE - so pages will be
	 *	locked in memory.
	 */
	if ((fc = as_fault(as, (caddr_t)XTSSADDR, vp->vp_szxtss, 
				F_SOFTLOCK, S_WRITE)) != 0) {
#ifdef DEBUG1
	   cmn_err(CE_CONT,"v86init: as_fault failed\n");
#endif
		vp->vp_procp = (proc_t *)NULL;
		pp->p_v86 = (v86_t *)NULL;
		as_unmap(as, seg->s_base, seg->s_size);
		return FC_MAKE_ERR(fc);
	}

	/* Remember the nailed XTSS address for use at interrupt time */
	vp->vp_xtss = (xtss_t *)
		(phystokv (svirtophys((unsigned long)XTSSADDR)));

/*
**  Set up the gdt XTSS segment descriptor,
**  and mark that descriptor BUSY so the ECT can iret into it.
*/
	setdscrbase(&vp->vp_xtss_desc, (uint)vp->vp_xtss);
	setdscrlim(&vp->vp_xtss_desc, xtsssegsize - 1);
	setdscracc1(&vp->vp_xtss_desc, TSS3_KACC1);
	setdscracc2(&vp->vp_xtss_desc, TSS_ACC2);
	gdt[seltoi(XTSSSEL)] = vp->vp_xtss_desc;
	setdscracc1(&gdt[seltoi(XTSSSEL)], TSS3_KBACC1);

/*
**  Copy fields from the regular tss to the xtss, and set the VM flag.
**  Don't have to do suword()'s...a fault "can't happen" because
**  we've nailed the pages.  ANY FIELDS NOT SET HERE HAVE A VALUE OF ZERO.
*/
	xtp = (xtss_t *)XTSSADDR;
	xtp->xt_tss.t_ldt = u.u_tss->t_ldt;
	xtp->xt_tss.t_cr3 = u.u_tss->t_cr3;
	xtp->xt_tss.t_esp0 = u.u_tss->t_esp0;
	xtp->xt_tss.t_esp1 = u.u_tss->t_esp1;
	xtp->xt_tss.t_esp2 = u.u_tss->t_esp2;
	xtp->xt_tss.t_ss0 = u.u_tss->t_ss0;
	xtp->xt_tss.t_ss1 = u.u_tss->t_ss1;
	xtp->xt_tss.t_ss2 = u.u_tss->t_ss2;
	xtp->xt_tss.t_bitmapbase = u.u_tss->t_bitmapbase;
	xtp->xt_tss.t_eflags = PS_IE | PS_VM;   /* LEAVES IOPL == 0 ! */
	xtp->xt_tss.t_cs = V86_RCS;             /* RESET to virtual 8086 */
	xtp->xt_tss.t_eip = V86_RIP;

/*
**   Set up some XTSS fields.
*/
	xtp->xt_magic[0] = V86_MAGIC0;
	xtp->xt_magic[1] = V86_MAGIC1;
	xtp->xt_magicstat = XT_MSTAT_NOPROCESS; /* Do not process virt int */
	xtp->xt_tslice_shft = V86_SHIFT_NORM;	/* Normal time slice shift */
	xtp->xt_timer_bound = V86_TIMER_BOUND;  /* Limit to force ECT in */
	xtp->xt_viurgent = (V86VI_KBD | V86VI_MOUSE);
	xtp->xt_vitoect = ~V86VI_NONE;
	xtp->xt_vp_relcode = VP_RELCODE;

/*
**   The back-link in the UTSS must point to the XTSS
**   so the ECT can start the virtual 8086 with an iret.
**   Also turn on caller's NT bit in the stack flag image.
*/
	u.u_tss->t_link = XTSSSEL;
	flags->fl_nt = 1;

/*
**  Set up the return values in the passed-in argument structure.
*/
	i = splhi();
	vp->vp_procp = pp;
	pp->p_v86 = vp;
	v86procflag = 1;        /* ldt-switch flag for ttrap.s */

	/* Set V86 task floating point state to NOTVALID */
	vp->vp_fpvalid = V86FPV_NOTVALID;

	num_v86procs++;
	splx(i);

	viparm.xtssp = xtp;
	if (xtsssegsize == OLDXTSSSIZE)
		copyout((caddr_t)&viparm, arg, sizeof (struct v86parm));
	else
		copyout((caddr_t)&viparm, arg, OLDPARMSIZE);

#ifdef DEBUG1
	    cmn_err(CE_CONT,"v86init: exiting\n"); 
#endif
	return 0;
}


/*  Sleep until next virtual interrupt.
**  The current dual mode process sleeps on the v86 structure
**  and is woken up at the next virtual interrupt to the dual
**  mode process.
*/

int
v86sleep()
{
	register v86_t *v86p;           /* Current v86 structure */
	register s;                     /* Previous priority */
	xtss_t *xtp;                    /* Pointer to the XTSS */

/* Since the XTSS is nailed in no fault can occur. Hence members of
** the XTSS can be accessed directly though the XTSS is in user space.
*/
#ifdef DEBUG1
	cmn_err(CE_CONT,"In v86sleep\n"); 
#endif
	xtp = (xtss_t *)XTSSADDR;       /* Init to user XTSS address */
	v86p = u.u_procp->p_v86;	/* Get current v86 structure */
	if (!v86p)			/* Must be a dual-mode process */
		return EINVAL;

/* Save (in the kernel structure) the XTSS mask that the user has set
** so that the drivers can use it at interrupt time. If there are any
** psuedorupts that are pending that the process will wakeup on, do
** not go to sleep.
*/

	s = splhi();		/* Critical section */
	if (!(xtp->xt_vimask & xtp->xt_viflag)) {
		v86p->vp_wakeup++;
		sleep((caddr_t)v86p, PV86);
	}
	splx(s);		/* End critical section */

	return 0;
}


/* Process mappings and trackings. */

int
v86memfunc(arg)
caddr_t arg;
{
	struct v86memory vmem;          /* User arguments structure */

/*  Get user arguments into local structure
*/
#ifdef DEBUG1
	cmn_err(CE_CONT,"In v86memfunc\n"); 
#endif
	if (copyin(arg, (caddr_t)&vmem, sizeof (struct v86memory)))
		return EFAULT;

/*
 *  Process map and track requests
 */

	switch (vmem.vmem_cmd)
	{
		case V86MEM_MAP:                /* Map the screen */
		case V86MEM_TRACK:              /* Track the screen */
		case V86MEM_UNTRACK:		/* Untrack the screen */
		case V86MEM_UNMAP:		/* Unmap virt addr */
			return v86scrfunc(&vmem);

		case V86MEM_EMM:		/* Support EMM memory     */
			return v86emmset(&vmem);

		case V86MEM_GROW:		/* Grow EMM memory        */
			return v86emmgrow(&vmem);

		default:
			return EINVAL;		/* Invalid system call */
	}
}


int
v86scrfunc(vmemptr)
struct v86memory *vmemptr;              /* User arguments structure         */
{
	v86_t       *v86p;		/* pointer to the v86 struct        */
	struct	as  *as;		/* process's address space	    */
	struct  seg *seg;		/* vp/ix segment for process	    */
	u_int	    basepage;		/* first screen memory page	    */
	u_int	    npages;		/* number of screen memory pages    */
	u_int	    physpage;		/* physical page to map to (opt.)   */
	int	    err;

#ifdef DEBUG1
	cmn_err(CE_CONT, "In v86scrfunc\n"); 
#endif

/*
 * Validate the request arguments.
 */

	/* Make sure this process is a v86 process. */
	if ((v86p = u.u_procp->p_v86) == NULL)
		return EACCES;

	/* Only V86MEM_MAP uses vmem_physaddr, so ignore it for all else. */
	if (vmemptr->vmem_cmd != V86MEM_MAP)
		vmemptr->vmem_physaddr = 0;

	/* Make sure the arguments are all page-aligned. */
	if (((u_int)vmemptr->vmem_membase & PAGEOFFSET) != 0 ||
	    ((u_int)vmemptr->vmem_memlen & PAGEOFFSET) != 0 ||
	    ((u_int)vmemptr->vmem_physaddr & PAGEOFFSET) != 0)
		return ENXIO;

	/* Convert the arguments to units of pages. */
	basepage = btop(vmemptr->vmem_membase);
	npages = btop(vmemptr->vmem_memlen);
	physpage = btop(vmemptr->vmem_physaddr);

	/* Find the segment corresponding to the base address. */

	if ((as = u.u_procp->p_as) == (struct as *)NULL)
		cmn_err(CE_PANIC, "v86scrfunc: no as allocated");

	if ((seg = as_segat(as, vmemptr->vmem_membase)) == (struct seg *)NULL) {
#ifdef DEBUG1
		cmn_err(CE_CONT, "v86scrfunc: as_segat failed\n");
#endif
		return EACCES;
	}

	/* It must be a seg_vpix segment. */

	if (seg->s_ops != &segvpix_ops) {
#ifdef DEBUG1
		cmn_err(CE_CONT, "v86scrfunc: Not a vpix segment\n");
#endif
		return EACCES;
	}

	/* The desired range, including its tracking equivalence range,
	 * must fall within the segment size. */

	ASSERT(seg->s_base == 0);

	if (ptob(basepage + npages) > seg->s_size ||
	    ptob(basepage + npages + V86VIRTSIZE) > seg->s_size) {
#ifdef DEBUG1
		cmn_err(CE_CONT, "v86scrfunc: Out of range\n");
#endif
		return EACCES;
	}

/*
 * Perform the desired screen memory operation(s).
 */

	switch (vmemptr->vmem_cmd) {

	case V86MEM_MAP:                /* Map the screen */

		/* Map the address range to the physical screen memory. */

		err = _v86physmap(seg, basepage, physpage, npages);
		if (err)
			return err;

		/* Fall through... */

	case V86MEM_TRACK:              /* Track the screen */

		/* Set up an equivalence between this (screen) memory area
		 * and the shadow range used for tracking. */

		err = segvpix_range_equiv(seg, basepage + V86VIRTSIZE,
					       basepage,
					       npages);
		if (err)
			return err;

		/* Lock down the screen pages so we don't lose the dirty
		 * bits, which we need to check periodically in v86scrscan. */

		err = segvpix_lockop(seg, vmemptr->vmem_membase,
					  vmemptr->vmem_memlen,
					  0, MC_LOCK, NULL, 0);
		if (err) {
			segvpix_range_equiv(seg, basepage + V86VIRTSIZE,
						 basepage + V86VIRTSIZE,
						 npages);
			if (vmemptr->vmem_cmd == V86MEM_MAP)
				segvpix_unphys(seg, basepage, npages);
			return err;
		}

		/* NOTE: We depend on the XTSS lock to lock the translations
		 * for the screen pages.  This works because they're in the
		 * same page table and the current hat implements translation
		 * locking at the page table level. */

		ASSERT(ptnum(vmemptr->vmem_membase) == 0);

		/* Remember the screen mapping in vp_mem. */

		v86p->vp_mem = *vmemptr;

		break;

	case V86MEM_UNMAP:		/* Unmap virt addr */

		/* Undo the physical mapping. */

		err = segvpix_unphys(seg, basepage, npages);
		if (err)
			return err;

		/* Fall through... */

	case V86MEM_UNTRACK:		/* Untrack the screen */

		/* Undo the screen tracking equivalence. */

		err = segvpix_range_equiv(seg, basepage + V86VIRTSIZE,
					       basepage + V86VIRTSIZE,
					       npages);
		if (err)
			return err;

		/* Unlock the pages. */

		err = segvpix_lockop(seg, vmemptr->vmem_membase,
					  vmemptr->vmem_memlen,
					  0, MC_UNLOCK, NULL, 0);
		if (err)
			return err;

		/* Screen mapping no longer active. */

		v86p->vp_mem.vmem_memlen = 0;

		break;

	}

	return 0;
}


int
v86emmset (vmemptr)
struct v86memory *vmemptr;              /* User arguments structure         */
{
	v86_t       *v86p;		/* pointer to the v86 struct        */
	struct	as  *as;                /* address space for the process    */
	struct	seg *seg;               /* segment pointer for v86 segment  */
	u_int	    basepage;		/* first memory page for emm window */
	u_int	    npages;		/* number of pages in winsow        */
	u_int	    emmpage;		/* emm page to map window to	    */

#ifdef DEBUG1
	cmn_err(CE_CONT, "In v86emmset\n"); 
#endif

/*
 * Validate the request arguments.
 */

	/* Make sure this process is a v86 process. */
	if ((v86p = u.u_procp->p_v86) == NULL)
		return EACCES;

	/* Make sure the arguments are all page-aligned. */
	if (((u_int)vmemptr->vmem_membase & PAGEOFFSET) != 0 ||
	    ((u_int)vmemptr->vmem_memlen & PAGEOFFSET) != 0 ||
	    ((u_int)vmemptr->vmem_physaddr & PAGEOFFSET) != 0)
		return ENXIO;

	/* Convert the arguments to units of pages. */
	basepage = btop(vmemptr->vmem_membase);
	npages = btop(vmemptr->vmem_memlen);
	emmpage = btop(vmemptr->vmem_physaddr);

	/* Make sure EMM window is not in low memory used with 1M wrap. */
	if (basepage < V86WRAPSIZE)
		return ENXIO;

	/* Find the segment corresponding to the base address. */

	if ((as = u.u_procp->p_as) == (struct as *)NULL)
		cmn_err(CE_PANIC, "v86emmset: no as allocated");

	if ((seg = as_segat(as, vmemptr->vmem_membase)) == (struct seg *)NULL) {
#ifdef DEBUG1
		cmn_err(CE_CONT, "v86emmset: as_segat failed\n");
#endif
		return EACCES;
	}

	/* It must be a seg_vpix segment. */

	if (seg->s_ops != &segvpix_ops) {
#ifdef DEBUG1
		cmn_err(CE_CONT, "v86emmset: Not a vpix segment\n");
#endif
		return EACCES;
	}

	/*
	 * Address validation is done by the LIM 4.0 driver and/or
	 * the ECT.  This is because the LIM 4.0 specification allows,
	 * virtually, any mapping;  the VP/ix environment, however,
	 * imposes some limitation on this.
	 *
	 * Make sure the length is V86EMM_PGSIZE (16K).
	 * Make sure the address being mapped to is on a
	 * page boundary, not in the first two megabytes,
	 * and falls within the vpix segment.
	*/

	ASSERT(seg->s_base == 0);

	if (ptob(npages) != V86EMM_PGSIZE ||
	    ptob(basepage + npages) > seg->s_size ||
	    (emmpage && (ptob(emmpage) < V86EMM_LBASE ||
			 ptob(emmpage + npages) > seg->s_size))) {
#ifdef DEBUG1
		cmn_err(CE_CONT, "v86emmset: Out of range\n");
#endif
		return ENXIO;
	}

/*
 * Set up the actual mapping.
 */

	/* Set up an equivalence for this emm page. */
	return segvpix_range_equiv(seg, basepage,
					emmpage? emmpage : basepage,
					btop(V86EMM_PGSIZE));
}


int
v86emmgrow (vmemptr)
struct v86memory *vmemptr;              /* User arguments structure         */
{
	struct	as  *as;                /* address space for the process    */
	struct	seg *seg;               /* segment pointer for v86 segment  */
	struct	proc	*pp = u.u_procp;
	addr_t      growbound;          /* address that we are growing to   */

#ifdef DEBUG1
	cmn_err(CE_CONT, "In v86emmgrow\n"); 
#endif

/*
 * Validate the request arguments.
 */

	/* Make sure this process is a v86 process. */

	if (u.u_procp->p_v86 == NULL)
		return EACCES;

	/* Make sure the arguments are all page-aligned. */

	growbound = vmemptr->vmem_membase;
	if (((u_int)growbound & PAGEOFFSET) != 0) {
#ifdef DEBUG1
		cmn_err(CE_CONT, "v86emmgrow: invalid growbound address\n");
#endif
		return ENXIO;
	}

	/* Find the segment corresponding to the base address. */

	if ((as = u.u_procp->p_as) == (struct as *)NULL)
		cmn_err(CE_PANIC, "v86emmgrow: no as allocated");

	if ((seg = as_segat(as, 0)) == (struct seg *)NULL) {
#ifdef DEBUG1
		cmn_err(CE_CONT, "v86emmgrow: as_segat failed\n");
#endif
		return EACCES;
	}

	/* It must be a seg_vpix segment. */

	if (seg->s_ops != &segvpix_ops) {
#ifdef DEBUG1
		cmn_err(CE_CONT, "v86emmgrow: Not a vpix segment\n");
#endif
		return EACCES;
	}

	/* The grow boundary (new seg_vpix size) must be between
	 * 2 and 4 megabytes. */

	if ((u_int)growbound < ptob(2 * V86VIRTSIZE) ||
	  		(u_int)growbound > ptob(V86SIZE)) {
#ifdef DEBUG1
		cmn_err(CE_CONT, "v86emmgrow: address not in 2 to 4 MB range\n");
#endif
		return ENXIO;
	}

/*
 * Do the actual grow operation.
 */

	ASSERT(seg->s_base == 0);

	/* If the segment is already big enough, don't do anything. */

	if (growbound <= (addr_t)seg->s_size)
		return 0;

	/* Map in a new vpix segment to cover the new size.
	 * Seg_vpix will coalesce it with the existing segment. */

	if (as_map(as, (addr_t)seg->s_size,
			growbound - (addr_t)seg->s_size,
			segvpix_create, vpix_argsp) != 0) {
#ifdef DEBUG1
		cmn_err(CE_CONT, "v86emmgrow: as_map failed for growth\n");
#endif
		return ENXIO;
	}

	return 0;
}


/* Allow a process to play  with  its  io  privilege  level, */
/* allowing the process to do ins and outs.                  */

int
v86iopl(arg)
long    arg;
{
#ifdef DEBUG1
	cmn_err(CE_CONT,"In v86iopl\n"); 
#endif
	/* Must be privileged to run this system call if giving more */
	/* io privilege.                                             */

	if (((u.u_ar0[EFL] & PS_IOPL) < (arg & PS_IOPL)) && !suser (u.u_cred))
	    return EPERM;

	u.u_ar0[EFL] = (u.u_ar0[EFL] & ~PS_IOPL) | (arg & PS_IOPL);
	return 0;
}


/*
**  Exit processing for a v86 process.
**  If it's being killed in v86 mode, and is the current process,
**  make sure to do an ltr back to UTSSSEL before jumping away
**  to some other process, because we're de-nailing the XTSS.
**  Otherwise, just free up v86 slot and v86 struct memory,
**  clear out v86 ptr in proc table,
**  and de-nail the XTSS.
*/

v86exit(p)
proc_t *p;
{
    v86_t *vp;
    register v86_t **v86pp;
    register struct as *as;
    int i;
	
#ifdef DEBUG1
	  cmn_err(CE_CONT,"In v86exit\n"); 
#endif
    vp = p->p_v86;
    if (vp)                            /* If v86 process */
    {
	if (get_tr() == XTSSSEL)
	    loadtr(UTSSSEL);
	v86procflag = 0;
	p->p_v86 = (v86_t *)NULL;
	--num_v86procs;
	vp->vp_procp = (proc_t *)NULL;

	if ((as = p->p_as) == (struct as *) NULL)
		cmn_err(CE_PANIC,"v86exit: no as allocated\n");

	/* Unlock the XTSS */
	if (as_fault(as, (addr_t)XTSSADDR, vp->vp_szxtss, F_SOFTUNLOCK, S_WRITE) != 0)
		cmn_err(CE_PANIC,"v86exit: SOFT_UNLOCK on X-TSS failed\n");

	for (i=0, v86pp = v86tab; i < NV86TASKS; i++, v86pp++){
		if (vp == *v86pp) {
			*v86pp = (v86_t *)NULL;
			kmem_free((_VOID *)vp, sizeof (v86_t));
		}
	}
    }

}


/*  Virtual timer interrupt for all v86 tasks.
**  The virtual timer interrupt for all v86 tasks is done every
**  "v86timer" ticks, from the clock interrut. This routine sets
**  the timer interrupt bit in the interrupt flags of the V86
**  structure for every valid dual mode process in "v86tab" and
**  the kernel posts it to the ECT at exit to user mode.
*/

v86timerint()
{
	register int i;
	register v86_t **v86pp;

	v86timer = V86TIMER;
	for(i = 0, v86pp = v86tab;  i < NV86TASKS;  i++, v86pp++) {
		if (*v86pp != (v86_t *)NULL)
			v86setint(*v86pp,V86VI_TIMER);
	}
}


/*  Force a virtual interrupt and save signal information.
**  This routine is called from "sendsig" when we are returning
**  to V86 user mode. Since the handler can only be in 386
**  user mode, this routine forces a virtual interrupt to get
**  back into 386 mode and saves the signal number and signal
**  handler, so that the ECT can call its handler directly if
**  necessary. Since the XTSS is nailed no fault can occur
**  accessing fields in the XTSS. The ECT is expected to
**  to zero the "xt_signo" field after it is done processing.
**  NOTE: There is no race condition as we should switch from
**  V86 to 386 mode and process the signals. We will switch
**  back to V86 mode only when the processing is done. This
**  code is executed only if we are going back into V86 mode.
*/

v86sighdlint(hdlr, signo)
int     (* hdlr)();
unsigned int signo;
{
	register xtss_t *xtp;           /* Pointer to XTSS in user space */
	proc_t *p = u.u_procp;          /* Current proc structure */
	register v86_t *v86p;           /* Current v86 structure */
#ifdef DEBUG1
	cmn_err(CE_CONT,"In v86sighdlint\n"); 
#endif
	v86p = p->p_v86;                /* Get current v86 structure */
	xtp = (xtss_t *)XTSSADDR;       /* Init to user XTSS address */

	if (xtp->xt_signo == 0)         /* If no signal being processed */
	{
		xtp->xt_signo = signo;  /* Save signal number */
		xtp->xt_hdlr = hdlr;    /* Save signal handler */
		v86setint(v86p, V86VI_SIGHDL);  /* Force virtual interrupt */
	}
	else                            /* This code should be taken out */
		printf("v86sighdlint: Lost signal.\n");
}


/*  Set the virtual interrupt in V86 structure.
**  The drivers for the various interrupts that need to send a
**  virtual interrupt to a v86 task call this routine at interrupt
**  time to set the virtual interrupt bit(s) in the flags. The kernel
**  at exit to the user process, processes this bit and posts the
**  virtual interrupt to the ECT. Any process that has gone to
**  sleep waiting for one of many psuedorupts is woken up if this
**  pseudorupt is among the ones set in the mask. If the priority
**  of the process has been lowered because of a request from the
**  ECT and this is one of the pseudorupts of interest (given by
**  the mask) then, the priority of the process is brought back
**  to normal.
**
**  NOTE: The mask is actually in the XTSS and the ECT can dyna-
**  mically change it. This mask is copied into the v86 structure
**  and used. Since all code now accesses the XTSS to get the
**  mask the one in the v86 structure can probably be taken out.
*/

v86setint(v86p, intr_bits)
register v86_t *v86p;
unsigned int intr_bits;
{
	int s;                                  /* Old priority             */
	register    proc_t  *p;                 /* Ptr to proc struct       */
	pte_t   *pte;                           /* Ptr to page table        */
	register    xtss_t      *xtp;           /* Ptr to xtss in proc p    */
	register v86_t **v86pp;			/* Loop ptr through v86tab  */
	register int i;

#ifdef DEBUG1
        cmn_err(CE_CONT,"In v86setint\n"); 
#endif
	/* Now we have to loop through the array of v86 processes to verify
	** that we've been called with a valid v86_t pointer.  Why?  Because
	** the serial port drivers have an ioctl flag called DOSMODE that gets
	** set by vpix to tell the driver to call this routine (v86setint)
	** when a char comes into the serial port buffer.  If vpix dies
	** at user level before getting a chance to unset the DOSMODE flag,
	** every subsequent character at that serial port will still generate
	** a v86setint.  The v86_t pointer may therefore be invalid if the
	** process died.  If so, we exit with EINVAL.
	*/
	for (v86pp = v86tab, i = 0; *v86pp != v86p; v86pp++) {
		if (++i >= NV86TASKS) {
#ifdef DEBUG1
			cmn_err(CE_CONT, "v86setint: Bad v86p passed in from \
external event!\n     Process no longer valid.\n");
#endif
			return(EINVAL);
		}
	}

	xtp = v86p->vp_xtss;
	if (xtp == 0)
		return;
	xtp->xt_viflag |= intr_bits;            /* Set virt intr bit */
	p = v86p->vp_procp;                     /* Get the proc struct ptr */
	if (p == 0)                             /* Should never be true */
		return;                         /* But unfortunately it is */

	if (intr_bits & V86VI_TIMER)
		xtp->xt_timer_count++;

	s = splhi();                            /* Critical section */
	if (xtp->xt_viflag & xtp->xt_vimask & xtp->xt_viurgent) {
		v86p->vp_pri_state = V86_PRI_XHI;
		v86p->vp_slice_shft = V86_SHIFT_HI;

		/* Reset nice to 0 (highest priority) for urgent pseudorupts.
		   The actual change will happen in s_trap(). */

		v86p->vp_new_nice = 0;
	}
	/* High priority for other interesting non-timer pseudorupts */
	else if (xtp->xt_viflag & xtp->xt_vimask &
			~(V86VI_TIMER | V86VI_LBOLT)) {
		v86p->vp_pri_state = V86_PRI_HI;
		v86p->vp_slice_shft = V86_SHIFT_HI;

		/* Set nice to the high-priority nice value.
		   The actual change will happen in s_trap(). */

		v86p->vp_new_nice = v86p->vp_hpnice;
	}
	/* This potential wakeup must go after the above tests which
	 * check for SRUN. */
	if ((xtp->xt_vimask & intr_bits) || (xtp->xt_timer_count >=
						xtp->xt_timer_bound)) {
		if (v86p->vp_wakeup) {          /* Wakeup process? */
			v86p->vp_wakeup = 0;    /* Reset wakeup flag */
			wakeprocs((caddr_t)v86p, PRMPT);  /* Wakeup process */
		}
	}
	splx(s);                                /* End critical section */
}


/*  Process virtual interrupts before going into user mode.
**  This routine is called from ttrap.s before exiting to
**  user mode of a dual mode process (V86 mode OR 386 mode).
**  If there are virtual interrupts to be processed and the
**  ECT has enabled processing of virtual interrupts then
**  set the ARPL instruction in the CS:IP of the V86 task
**  and save the opcode at CS:IP in a fixed location in
**  the XTSS.
*/

v86vint(r0ptr, v86flag)
register int *r0ptr;
int v86flag;
{
	register xtss_t *xtp;           /* Pointer to XTSS in user space */
	register s;                     /* Saved priority */
	proc_t *p = u.u_procp;          /* Current proc structure */
	register v86_t *v86p;           /* Current v86 structure */
	caddr_t usrmemp;                /* Pointer to user memory */
	char tslice_shft;               /* Time slice shift requested */
	void    v86scrscan ();          /* Screen memeory scan routine */

#ifdef DEBUG1
	cmn_err(CE_CONT,"In v86vint\n"); 
#endif
	v86p = p->p_v86;                /* Get current v86 structure */

/*  Since the XTSS is nailed no fault can occur. Transfer the virtual
**  interrupts to the XTSS and clear the interrupts in the V86 struct.
*/
	xtp = (xtss_t *)XTSSADDR;       /* Init to user XTSS address */

/*  The following comment is being kept here for historical purposes.
**  It really doesn't apply anymore.  Time slicing of the v86 processes
**  is not manipulated by this module in SVR4.0, since the dispatcher
**  assigns time quanta based on user priority as called for in the dptbl
**  for the given scheduling class.  This comment helps explain what
**  what was going on though in earlier releases:
**
**  The ECT is given control of reducing its time slice through a field
**  in the XTSS. For every v86 process, the time slice is set to an
**  enormous value. While exiting to user mode, this slice is adjusted
**  to either what the ECT requested, or the normal time slice. The
**  priority of a v86 process requesting a very small time slice is
**  reduced, so that it does not run as often. When a pseudorupt of
**  interest occurs or the ECT withdraws its request for a short time
**  slice, the priority is restored.
**
**  NOTE: Timer ticks happening before shift can be lost. Also it assumes
**        that number of ticks happened is less than V86_SLICE.
*/

	s = splhi();                    /* Start of critical section */
	/* If scheduled due to high priority revert to normal.  Pay no
	 * attention to xtss slice shift because the pseudorupt could
	 * result in it changing.  We were probably scheduled as a result
	 * of the pseudorupt that gave us high priority. */
	if (v86p->vp_pri_state == V86_PRI_HI ||
			v86p->vp_pri_state == V86_PRI_XHI) {
		v86p->vp_pri_state = V86_PRI_NORM;
		v86p->vp_slice_shft = 0;

		/* Call donice() to set nice to v86p->vp_nice (recorded */
		/* priority nice value). */

		(void) v86_set_nice(p, v86p->vp_nice);
	}
	/* If the slice shift changed, set the appropriate nice value. */
	else if (xtp->xt_tslice_shft != v86p->vp_slice_shft) {
		xtp->xt_tslice_shft =
			min (V86_SLICE_SHIFT, xtp->xt_tslice_shft);
		if (xtp->xt_tslice_shft) {
		    v86p->vp_pri_state = V86_PRI_LO;
			/* donice() will be called to set the nice value to:
			** 2*NZERO-1-V86_SLICE_SHIFT+xtp->xt_tslice_shft. */

		    (void) v86_set_nice(p, 2 * NZERO - 1 - V86_SLICE_SHIFT
			+ xtp->xt_tslice_shft);
		}
		else {
		    v86p->vp_pri_state = V86_PRI_NORM;
			/* Call donice() to set nice to v86p->vp_nice
			** (recorded priority nice value). */

		    (void) v86_set_nice(p, v86p->vp_nice);
		}
		v86p->vp_slice_shft = xtp->xt_tslice_shft;
	}
	v86_slice_start = 0;

	/*
	 * If both the viflag bit is clear AND the vimask bit for video
	 * dirty bits are set, call v86scrscan(); otherwise
	 * we either have a video psuedorupt pending, or we
	 * do not want a video psuedorupt at this time.
	*/
	if(!(xtp->xt_viflag&V86VI_MEMORY) && (xtp->xt_vimask&V86VI_MEMORY))
		v86scrscan();

	if (xtp->xt_lbolt != lbolt) {
		xtp->xt_viflag |= V86VI_LBOLT;
		xtp->xt_lbolt = lbolt;          /* Ticks from powerup */
	}
	splx(s);                        /* End of critical segment */

/*  If the "xt_magicstat" is set to XT_MSTAT_OPSAVED then the
**  the ECT is still processing a previous virtual interrupt.
**  If the "xt_magicstat" is set to XT_MSTAT_PROCESS by the
**  ECT and there are virtual interrupts to process, or the
**  "xt_timer_count" is greater than or equal to "xt_timer_bound",
**  then set the ARPL instruction in the next instr to be exec-ed
**  in V86 mode. The valid CS:IP is on the stack if we are
**  are going back to V86 user mode, otherwise the valid CS:IP
**  is the one in the XTSS. The byte at the valid CS:IP location
**  for the V86 program is savid in "xt_magictrap" in the XTSS
**  and a ARPL instruction is set in the v86 program at this
**  location.
**  NOTE: We can exit to user mode (V86 or 386) only once as this
**  routine is called just before exit to user mode. If this routine
**  is interrupted we won't reenter it (as we will be in the kernel
**  at the end of interrupt processing) and will just resume where
**  we left off after interrupt processing. Hence there is no race
**  condition.
*/
	if ((xtp->xt_viflag & xtp->xt_vitoect) &&
			(xtp->xt_magicstat == XT_MSTAT_PROCESS))
	{
		xtp->xt_magicstat = XT_MSTAT_OPSAVED;   /* Tell ECT */
		if (v86flag)            /* Going to V86 mode */
		    usrmemp = (caddr_t)(((r0ptr[CS] & 0x0000FFFF) << 4)
				      + (r0ptr[EIP] & 0x0000FFFF));
		else                    /* Going to 386 mode */
		    usrmemp = (caddr_t)(((xtp->xt_tss.t_cs & 0x0000FFFF) << 4)
				      + (xtp->xt_tss.t_eip & 0x0000FFFF));
		xtp->xt_magictrap = fubyte(usrmemp);    /* Save user byte */
		subyte(usrmemp, ARPL);  /* Set ARPL in user program */
	}
}


void
v86scrscan()
{
	v86_t       *v86p;		/* pointer to the v86 struct        */
	struct	as  *as;                /* address space for the process    */
	struct  seg *seg;               /* pointer to vpix segment          */
	int         retval;             /* Do we need to send an interrupt  */

#ifdef DEBUG1
	cmn_err(CE_CONT,"In v86scrscan\n"); 
#endif

	/* Scan the screen memory which is designated by  vp_membase */
	/* to  vp_membase  +  vp_memlen for page modified bits.  For */
	/* each modified bit, set a bit in a word to be  written  to */
	/* the user.  And clear the modified bit for the next scan.  */

	/* This routine is presumed to  be  called  with  interrupts */
	/* disabled.  This is just in case the pager decides to page */
	/* out a page that we are looking at.                        */

	/* Make sure this is a v86 process and it wants tracking.    */

	if ((v86p = u.u_procp->p_v86) == NULL ||
		v86p->vp_mem.vmem_memlen == 0 ||
		(v86p->vp_xtss->xt_viflag & V86VI_MEMORY))
	    return;


	/* Find the segment corresponding to the base address. */

	if ((as = u.u_procp->p_as) == (struct as *)NULL)
		cmn_err(CE_PANIC, "v86scrscan: no as allocated");

	if ((seg = as_segat(as, v86p->vp_mem.vmem_membase)) ==
				(struct seg *)NULL) {
#ifdef DEBUG1
		cmn_err(CE_CONT, "v86scrscan: as_segat failed\n");
#endif
		return;
	}

	/* It must be a seg_vpix segment. */

	if (seg->s_ops != &segvpix_ops) {
#ifdef DEBUG1
		cmn_err(CE_CONT, "v86scrscan: as_segat failed\n");
#endif
		return;
	}

/*
 * Scan the screen memory for modified bits.
 */

	retval = segvpix_modscan(seg, btop(v86p->vp_mem.vmem_membase),
				      btop(v86p->vp_mem.vmem_memlen));

	/* If we had a bit that was modified, send an interrupt. */

	if (retval)
		v86setint(v86p, V86VI_MEMORY);
}


int
v86physmap(begmapaddr, length, begphysaddr)
caddr_t begmapaddr;                     /* address to start mapping at      */
int     length;                         /* byte count to map                */
paddr_t begphysaddr;                    /* physical address to map to       */
{
	v86_t       *v86p;		/* pointer to the v86 struct        */
	struct	as  *as;		/* process's address space	    */
	struct  seg *seg;		/* vp/ix segment for process	    */
	u_int	     npages;		/* number of pages to map	    */

#ifdef DEBUG1
	cmn_err(CE_CONT, "In v86physmap\n");
#endif

/*
 * Validate the request arguments.
 */

	/* Make sure this process is a v86 process. */
	if ((v86p = u.u_procp->p_v86) == NULL)
		return EACCES;

	/* Find the segment corresponding to the base address. */

	if ((as = u.u_procp->p_as) == (struct as *)NULL)
		cmn_err(CE_PANIC, "v86physmap: no as allocated");

	if ((seg = as_segat(as, begmapaddr)) == (struct seg *)NULL) {
#ifdef DEBUG1
		cmn_err(CE_CONT, "v86physmap: as_segat failed\n");
#endif
		return EACCES;
	}

	/* It must be a seg_vpix segment. */

	if (seg->s_ops != &segvpix_ops) {
#ifdef DEBUG1
		cmn_err(CE_CONT, "v86physmap: Not a vpix segment\n");
#endif
		return EACCES;
	}

	/* The desired range must fall within the segment size. */

	ASSERT(seg->s_base == 0);

	if (begmapaddr + length > (addr_t)seg->s_size) {
#ifdef DEBUG1
		cmn_err(CE_CONT, "v86physmap: Out of range\n");
#endif
		return EACCES;
	}

/*
 * Perform the actual mapping.
 */

	npages = btopr(begmapaddr + length) - btop(begmapaddr);

	return _v86physmap(seg, btop(begmapaddr),
				btop(begphysaddr),
				npages);
}

STATIC int
_v86physmap(seg, vpage, ppage, npages)
	struct seg	*seg;
	u_int		vpage;
	u_int		ppage;
	u_int		npages;
{
	paddr_t		seg_start, seg_end;
	u_int		cnt;

	/* Make sure the physical addresses we wish to map are not part
	 * of system memory. */

	for (cnt = 0; cnt < bootinfo.memavailcnt; cnt++) {
		if (bootinfo.memavail[cnt].extent == 0)
			break;
		seg_start = bootinfo.memavail[cnt].base;
		seg_end = seg_start + bootinfo.memavail[cnt].extent - 1;
		if (ptob(ppage) < seg_end &&
		    ptob(ppage + npages) >= seg_start) {
			return EIO;
		}
	}

	/* Seg_vpix does the actual mapping. */

	return segvpix_physmap(seg, vpage, ppage, npages);
}

/* BACKWARD COMPATIBILITY */
mappages(begmapaddr, length, begphysaddr)
caddr_t begmapaddr;                     /* address to start mapping at      */
int     length;                         /* byte count to map                */
paddr_t begphysaddr;                    /* physical address to map to       */
{
	int	err;

	err = v86physmap(begmapaddr, length, begphysaddr);
	if (err)
		u.u_error = err;
}


int
v86unphys(begmapaddr, length)
caddr_t begmapaddr;                     /* address of start of mapping      */
int     length;                         /* length, in bytes, of mapping     */
{
	v86_t       *v86p;		/* pointer to the v86 struct        */
	struct	as  *as;		/* process's address space	    */
	struct  seg *seg;		/* vp/ix segment for process	    */
	u_int	     npages;		/* number of pages to unmap	    */

#ifdef DEBUG1
	cmn_err(CE_CONT, "In v86unphys\n");
#endif

/*
 * Validate the request arguments.
 */

	/* Make sure this process is a v86 process. */
	if ((v86p = u.u_procp->p_v86) == NULL)
		return EACCES;

	/* Find the segment corresponding to the base address. */

	if ((as = u.u_procp->p_as) == (struct as *)NULL)
		cmn_err(CE_PANIC, "v86unphys: no as allocated");

	if ((seg = as_segat(as, begmapaddr)) == (struct seg *)NULL) {
#ifdef DEBUG1
		cmn_err(CE_CONT, "v86unphys: as_segat failed\n");
#endif
		return EACCES;
	}

	/* It must be a seg_vpix segment. */

	if (seg->s_ops != &segvpix_ops) {
#ifdef DEBUG1
		cmn_err(CE_CONT, "v86unphys: Not a vpix segment\n");
#endif
		return EACCES;
	}

	/* The desired range must fall within the segment size. */

	ASSERT(seg->s_base == 0);

	if (begmapaddr + length > (addr_t)seg->s_size) {
#ifdef DEBUG1
		cmn_err(CE_CONT, "v86unphys: Out of range\n");
#endif
		return EACCES;
	}

/*
 * Perform the actual unmapping.
 */

	npages = btopr(begmapaddr + length) - btop(begmapaddr);

	return segvpix_unphys(seg, btop(begmapaddr), npages);
}

/* BACKWARD COMPATIBILITY */
freemappages(begmapaddr, length, dzero)
caddr_t begmapaddr;                     /* address to start mapping at      */
int     length;                         /* byte count to map                */
int     dzero;                          /* should the memory be demand zero */
{
	int	err;

	err = v86unphys(begmapaddr, length);
	if (err)
		u.u_error = err;
}


#else /* VPIX */

/* THESE are 'stubs' for the functions defined in this source file */
#include "sys/errno.h"

int v86setint() { return ENOSYS; }

int v86syscall() { return ENOSYS; }
int v86init() { return ENOSYS; }
int v86exit() { return ENOSYS; }
int v86timerint() { extern int v86timer; v86timer = 0x7fffffff; }
int v86sighdlint() { return ENOSYS; }
int v86vint() { return ENOSYS; }
int core_vpix() { return ENOSYS; }

int v86physmap() { return ENOSYS; }
int v86unphys() { return ENOSYS; }
int mappages() { return ENOSYS; }
int freemappages() { return ENOSYS; }

int v86_set_nice() { return ENOSYS; }

#endif /* VPIX */
