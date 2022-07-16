/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mbus:uts/i386/io/console.c	1.3"

#ifndef lint
static char clock_copyright[] = "Copyright 1989 Intel Corp. 464463";
#endif

/*
 *	Indirect driver for console tty (/dev/console).
 */
#include "sys/types.h"
#include "sys/param.h"
#include "sys/cred.h"
#include "sys/cmn_err.h"
#include "sys/stream.h"
#include "sys/conf.h"
struct conssw conssw;

struct streamtab *cons_strtab;
int consdevflag = 0;		/* SVR4.0 requirement */

/*
 *	Devices which can be consoles must call consregister from their XXinit()
 *	routines if they discover that they can provide console service.
 *	Start time is too late because main() will try to do output before then.
*/
consregister(ci, co, dev)
int (*ci)();
int (*co)();
dev_t dev;
{
	conssw.ci = ci;
	conssw.co = co;
	conssw.co_dev = dev;
}

int
consopen(q, devp, flag, sflag, crp)
queue_t *q;
dev_t *devp;
int flag;
int sflag;
struct cred *crp;
{
	if (!cons_strtab)
		cons_strtab = cdevsw[getmajor(conssw.co_dev)].d_str;
	if (!cons_strtab)
		cmn_err(CE_PANIC, "consopen: 0x%x is not a STREAMS device!\n", *devp);
	return((*cons_strtab->st_rdinit->qi_qopen)
				(q, &conssw.co_dev, flag, sflag, crp));
}

consrput(q, bp)
queue_t *q;
mblk_t bp;
{
	(*cons_strtab->st_rdinit->qi_putp)(q, bp);
}

conswput(q, bp)
queue_t *q;
mblk_t bp;
{
	(*cons_strtab->st_wrinit->qi_putp)(q, bp);
}

consrsrv(q)
queue_t *q;
{
	(*cons_strtab->st_rdinit->qi_srvp)(q);
}

conswsrv(q)
queue_t *q;
{
	(*cons_strtab->st_wrinit->qi_srvp)(q);
}

consclose(q, flag, crp)
queue_t *q;
int flag;
struct cred *crp;
{
	return((*cons_strtab->st_rdinit->qi_qclose)(q, flag, crp));
}

/*
 *	This information is essentially a copy of the same info for iasy.
*/
struct module_info cons_info = {
	'cons', "cons", 0, INFPSZ, 512, 256 };
static struct qinit cons_rint = {
	consrput, consrsrv, consopen, consclose, NULL, &cons_info, NULL};
static struct qinit cons_wint = {
	conswput, conswsrv, consopen, consclose, NULL, &cons_info, NULL};
struct streamtab consinfo = {
	&cons_rint, &cons_wint, NULL, NULL};
