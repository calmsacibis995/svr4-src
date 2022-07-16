/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)kern-io:prf.c	1.3"
/*
 * UNIX Operating System Profiler
 *
 * Sorted Kernel text addresses are written into driver.  At each
 * clock interrupt a binary search locates the counter for the
 * interval containing the captured PC and increments it.
 * The last counter is used to hold the User mode counts.
 */

#include "sys/types.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/param.h"
#include "sys/immu.h"
#include "sys/tss.h"
#include "sys/user.h"
#include "sys/buf.h"
#include "sys/uio.h"
#include "sys/cred.h"

#define PRF_ON	1		/* profiler collecting samples */
#define PRF_VAL	2		/* profiler contains valid text symbols */
#define BPW	4		/* bytes per word */
#define L2BPW	2		/* log2(BPW) */

extern int maxprf;
extern unsigned prfctr[];	/* counters for symbols; last used for User */
extern unsigned prfsym[];	/* text symbols */
extern unsigned  prfstat;	/* state of profiler */
unsigned  prfmax;		/* number of loaded text symbols */

int prfdevflag = 0;

/* ARGSUSED */
int
prfopen(devp, mode, otyp, cr)
	dev_t *devp;
	int mode;
	int otyp;
	struct cred *cr;
{
	return 0;
}

/* ARGSUSED */
int
prfclose(dev, mode, otyp, cr)
	dev_t dev;
	int mode;
	struct cred *cr;
{
	return 0;
}

/* ARGSUSED */
int
prfread(dev, uiop, cr)
	dev_t dev;
	struct uio *uiop;
	struct cred *cr;
{
	unsigned min();
	int error;

	if ((prfstat & PRF_VAL) == 0)
		return ENXIO;
	error = uiomove((caddr_t) prfsym,
	  min(uiop->uio_resid, prfmax * BPW), UIO_READ, uiop);
	if (error == 0)
		error = uiomove((caddr_t) prfctr,
		  min(uiop->uio_resid, (prfmax + 1) * BPW), UIO_READ, uiop);

	return error;
}

/* ARGSUSED */
int
prfwrite(dev, uiop, cr)
	dev_t dev;
	struct uio *uiop;
	struct cred *cr;
{
	register  unsigned  *ip;
	register int error;

	if (uiop->uio_resid > (maxprf * sizeof (int)))
		return ENOSPC;
	if (uiop->uio_resid & (BPW - 1) || uiop->uio_resid < 3 * BPW)
		return E2BIG;
	if (prfstat & PRF_ON)
		return EBUSY;
	for (ip = prfctr; ip != &prfctr[maxprf + 1];)
		*ip++ = 0;
	prfmax = uiop->uio_resid >> L2BPW;
	error = uiomove((caddr_t) prfsym, uiop->uio_resid, UIO_WRITE, uiop);
	for (ip = &prfsym[1]; ip != &prfsym[prfmax]; ip++)
		if (*ip < ip[-1]) {
			error = EINVAL;
			break;
		}
	if (error)
		prfstat = 0;
	else
		prfstat = PRF_VAL;

	return error;
}

/* ARGSUSED */
int
prfioctl(dev, cmd, arg, mode, cr, rvalp)
	dev_t dev;
	int cmd;
	int arg;
	int mode;
	struct cred *cr;
	int *rvalp;
{
	register int error = 0;

	switch (cmd) {
	case 1:
		*rvalp = prfstat;
		break;
	case 2:
		*rvalp = prfmax;
		break;
	case 3:
		if (prfstat & PRF_VAL) {
			prfstat = PRF_VAL | arg & PRF_ON;
			break;
		}
		/* Fall-through */
	default:
		error = EINVAL;
		break;
	}

	return error;
}

caddr_t	func_addr;
extern int	spl0(),
		setpicmasks();

prfintr(pc, cs)
	register  unsigned  pc;
	int  cs;
{
	register  int  h, l, m;

	if (USERMODE(cs))
		prfctr[prfmax]++;
	else {
		/*
		 * Was an spl routine running when clock interrupted?
		 */
		if ((unsigned)spl0 <= pc && pc < (unsigned)setpicmasks) {
			/*
			 * Find out who called the spl function.
			 */
			asm("pushl	%eax");
			asm("movl	(%ebp), %eax");
			asm("movl	(%eax), %eax");
			asm("movl	(%eax), %eax");
			asm("movl	4(%eax), %eax");
			asm("movl	%eax, func_addr");
			asm("popl	%eax");
			pc = (unsigned)func_addr;
		}
		l = 0;
		h = prfmax;
		while ((m = (l + h) / 2) != l)
			if (pc >= prfsym[m])
				l = m;
			else
				h = m;
		prfctr[m]++;
	}
}
