/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-io:iosubr.c	1.3.1.1"

#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/types.h"
#include "sys/immu.h"
#include "sys/cmn_err.h"
#include "sys/systm.h"
#include "sys/errno.h"
#include "sys/proc.h"
#include "sys/tss.h"
#include "sys/user.h"
#include "sys/debug.h"


/*
 * this routine will enable I/O permissions for the
 * current user according to the input list of I/O 
 * addresses.  Bits corresponding to the I/O addresses
 * will be cleared in the TSS I/O permission bitmap.
 * The input list is terminated by a zero.
 */
enableio(iolist)
register unsigned short	*iolist;
{
	register int iobmcnt, old_cnt, newsz, size_incr;
	unsigned short ionum, highest_io=0;
	register char   *iobm;
	int oldpri;
	unsigned short *sav_iolist;
	struct dscr *ldt_p, *nldt_end_p, *ldt_end_p;
	extern struct seg_desc gdt[];

	iobm = (char *) u.u_tss + (u.u_tss -> t_bitmapbase >> 16);
/* Get highest numbered iolist member */
	for (sav_iolist = iolist; *iolist != 0; iolist++) {
		if(*iolist <= MAXTSSIOADDR)
			continue;
		if(highest_io < *iolist)
			highest_io = *iolist;
	}
/* Grow tss_bitmap size, if needed */
	if(highest_io){
		iobmcnt = ((highest_io + 72) >> 3 ) & ~7;
		if((newsz = (int)((iobm + iobmcnt) - (char *)u.u_tss)) <= u.u_sztss)
			goto done;	/* map was previously grown */
		size_incr = newsz - u.u_sztss;
/* ptr to user LDT table */
		ldt_p = (struct dscr *)u.u_procp->p_ldt; 
/* ptr to end of user LDT table */
		ldt_end_p = ldt_p + u.u_ldtlimit + 1;
		nldt_end_p = (struct dscr *)((char *)ldt_end_p + size_incr);
		if(!segu_expand(btoc((ulong)nldt_end_p -
					(ulong)PTOU(u.u_procp)))){
			return;
		}
/* Move LDT entries */
		while(ldt_end_p > ldt_p)
			*--nldt_end_p = *--ldt_end_p;
/* Point p_ldt to new LDT table location */
		oldpri = splhi();
		u.u_procp->p_ldt = (caddr_t)nldt_end_p;
		setdscrbase(&u.u_ldt_desc, u.u_procp->p_ldt);
		gdt[seltoi(LDTSEL)] = u.u_ldt_desc;
		loadldt(LDTSEL);
/* Fill new tssbitmap space with 1's */
		old_cnt = iobmcnt - size_incr;
		while(old_cnt < iobmcnt)
			iobm[old_cnt++] = 0xff;
		u.u_sztss = newsz;
		setdscrlim(&u.u_tss_desc, u.u_sztss - 1);
		setdscrlim(&gdt[seltoi(UTSSSEL)], u.u_sztss - 1);
		setdscracc1(&gdt[seltoi(UTSSSEL)], TSS3_KACC1);
		loadtr(UTSSSEL);
		splx(oldpri);
	}
done:
/* Update user I/O access permission */
	for (iolist = sav_iolist; *iolist != 0; iolist++) {
		ionum = *iolist;
		iobm[ionum >> 3] &= ~(1 << (ionum & 7));
	}
}

/*
 * this routine will disable I/O permissions for the
 * current user according to the input list of I/O 
 * addresses.  Bits corresponding to the I/O addresses
 * will be set in the TSS I/O permission bitmap.
 * The input list is terminated by a zero.
 */
disableio(iolist)
	register unsigned short	*iolist;
{
	register unsigned short ionum;
	register char   *iobm;

	iobm = (char *) u.u_tss + (u.u_tss -> t_bitmapbase >> 16);
	for (; *iolist != 0; iolist++) {
		ionum = *iolist;
		if (ionum > MAXTSSIOADDR)
			continue;
		iobm[ionum >> 3] |= 1 << (ionum & 7);
	}
}

/* verify that user has I/O address permissions for the 0-terminated
 * list of I/O addresses. Return 0 if one of the addresses is 
 * inaccessible or is greater than the number of I/O addresses configured
 * for the user. Return 1 only if access is possible to all I/O addresses
 * in the list. Used by the KD driver for the EGA_IOPRIVL/VGA_IOPRIVL ioctls.
 */
int
checkio(iolist)
register unsigned short	*iolist;
{
	register unsigned short ionum;
	register char   *iobm;

	iobm = (char *) u.u_tss + (u.u_tss -> t_bitmapbase >> 16);

	for (; *iolist != 0 ; iolist++) {
		ionum = *iolist;
		if (ionum > u.u_sztss)
			return (0);
		if (iobm[ionum >> 3] & (1 << (ionum & 7)))
			return (0);
	}
	return (1);
}

/*
 * This subroutine is a boolean that returns true (1) if
 * the process given by BOTH the pid and procp below is still
 * alive and well. Returns 0 otherwise.
 */

validproc(procp,pid)
proc_t *procp;
pid_t pid;
{
	proc_t *pp;

	/*
	 * find ptr to proc structure for pid.
	 */

	if (procp == (proc_t *)NULL) return (0);

	pp = prfind(pid);
	if (pp != procp)
		return (0);
	if (pp->p_pidp->pid_id != pid)
		return (0);
	if (pp->p_stat == SZOMB)
		return (0);
	return (1);
}
