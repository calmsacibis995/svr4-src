/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-dsp:sysclass.c	1.3"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/tss.h"
#include "sys/user.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/var.h"
#include "sys/errno.h"
#include "sys/cmn_err.h"
#include "sys/proc.h"
#include "sys/debug.h"
#include "sys/inline.h"
#include "sys/disp.h"
#include "sys/class.h"


/*
 * Class specific code for the sys class. There are no
 * class specific data structures associated with
 * the sys class and the scheduling policy is trivially
 * simple. There is no time slicing and priority is
 * only changed when the process requests this through the
 * disp argument to sleep().
 */

void		sys_init();
STATIC int	sys_fork(), sys_nosys();
STATIC void	sys_forkret(), sys_nullsys(), sys_preempt(), sys_setrun();
STATIC void	sys_sleep(), sys_wakeup();

STATIC struct classfuncs sys_classfuncs = {
	sys_nosys,
	sys_nosys,
	sys_nullsys,
	sys_fork,
	sys_forkret,
	sys_nosys,
	sys_nullsys,
	sys_nullsys,
	sys_nosys,
	sys_nosys,
	sys_nosys,
	sys_preempt,
	sys_nosys,
	sys_setrun,
	sys_sleep,
	sys_nullsys,
	sys_nullsys,
	sys_nullsys,
	sys_nullsys,
	sys_nullsys,
	sys_wakeup,
	sys_nosys,
	sys_nosys,
	sys_nosys,
	sys_nosys,
	sys_nosys,
	sys_nosys,
	sys_nosys,
	sys_nosys,
	sys_nosys,
	sys_nosys,
	sys_nosys
};


/* ARGSUSED */
void
sys_init(cid, clparmsz, clfuncspp, maxglobprip)
id_t		cid;
int		clparmsz;
classfuncs_t	**clfuncspp;
int		*maxglobprip;
{
	*clfuncspp = &sys_classfuncs;

	if (v.v_maxsyspri < PSLEP) {
		cmn_err(CE_WARN, "Max system class priority must be >= %d, configured value is %d\n- resetting v.v_maxsyspri to %d\n", PSLEP, v.v_maxsyspri, PSLEP);
		v.v_maxsyspri = PSLEP;
	}
	*maxglobprip = v.v_maxsyspri;
}


/* ARGSUSED */
STATIC int
sys_fork(pprocp, cprocp, cpstatp, cpprip, cpflagp, cpcredpp, procpp)
proc_t	*pprocp;
proc_t	*cprocp;
char	*cpstatp;
short	*cpprip;
uint	*cpflagp;
struct cred	**cpcredpp;
proc_t	**procpp;
{
	/*
	 * No class specific data structure so make the proc's
	 * class specific data pointer point back to the
	 * generic proc structure.
	 */
	*procpp = cprocp;
	return(0);
}


/* ARGSUSED */
STATIC void
sys_forkret(cprocp, pprocp)
proc_t	*cprocp;
proc_t	*pprocp;
{
	setbackdq(cprocp);
}


STATIC int
sys_nosys()
{
	return(ENOSYS);
}


STATIC void
sys_nullsys()
{
}


STATIC void
sys_preempt(pp)
proc_t	*pp;
{
	setfrontdq(pp);
}


STATIC void
sys_setrun(pp)
proc_t	*pp;
{
	setbackdq(pp);
}


/* ARGSUSED */
STATIC void
sys_sleep(pp, chan, disp)
proc_t	*pp;
caddr_t	chan;
int	disp;
{
	register int	tmpdisp;

	tmpdisp = disp & PMASK;
	pp->p_pri = v.v_maxsyspri - tmpdisp;
}


/* ARGSUSED */
STATIC void
sys_wakeup(pp, preemptflg)
proc_t	*pp;
int	preemptflg;
{
	setbackdq(pp);
}
