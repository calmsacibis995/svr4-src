/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-ktli:t_kalloc.c	1.1"
#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)t_kalloc.c 1.2 89/01/11 SMI"
#endif

/*
 *  		PROPRIETARY NOTICE (Combined)
 *  
 *  This source code is unpublished proprietary information
 *  constituting, or derived under license from AT&T's Unix(r) System V.
 *  In addition, portions of such source code were derived from Berkeley
 *  4.3 BSD under license from the Regents of the University of
 *  California.
 *  
 *  
 *  
 *  		Copyright Notice 
 *  
 *  Notice of copyright on this source code product does not indicate 
 *  publication.
 *  
 *  	(c) 1986,1987,1988,1989  Sun Microsystems, Inc.
 *  	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *  	          All rights reserved.
 */

/*
 *	Kernel TLI-like function to allocate memory for the
 *	various TLI primitives.
 *
 *	Returns 0 on success or a positive error value.
 *	On success, ptr is set the structure required.
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/stream.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/stropts.h>
#include <sys/tihdr.h>
#include <sys/timod.h>
#include <sys/tiuser.h>
#include <sys/errno.h>
#include <sys/signal.h>
#include <sys/t_kuser.h>
#include <sys/kmem.h>

static void _alloc_buf();

int
t_kalloc(tiptr, struct_type, fields, ptr)
	register TIUSER 	*tiptr;
	register int 		struct_type;
	register int 		fields;
	char			**ptr;
{
	register union structptrs {
		char	*caddr;
		struct t_bind *bind;
		struct t_call *call;
		struct t_discon *dis;
		struct t_optmgmt *opt;
		struct t_kunitdata *udata;
		struct t_uderr *uderr;
		struct t_info *info;
	} p;
	register unsigned	dsize;

	if (ptr == NULL)
		return EINVAL;

	/*
	 * allocate appropriate structure and the specified
	 * fields within each structure.  Initialize the
	 * 'buf' and 'maxlen' fields of each.
	 */
	switch (struct_type) {
	case T_BIND:
		p.bind = (struct t_bind *)
			kmem_zalloc((u_int)sizeof(struct t_bind), KM_SLEEP);
		if (fields & T_ADDR) {
			_alloc_buf(&p.bind->addr, tiptr->tp_info.addr);
		}
		*ptr = ((char *)p.bind);
		return 0;

	case T_CALL:
		p.call = (struct t_call *)
			kmem_zalloc((u_int)sizeof(struct t_call), KM_SLEEP);
		if (fields & T_ADDR) {
			_alloc_buf(&p.call->addr, tiptr->tp_info.addr);
		}
		if (fields & T_OPT) {
			_alloc_buf(&p.call->opt, tiptr->tp_info.options);
		}
		if (fields & T_UDATA) {
			dsize = max(tiptr->tp_info.connect, tiptr->tp_info.discon);
			_alloc_buf(&p.call->opt, dsize);
		}
		*ptr = ((char *)p.call);
		return 0;

	case T_OPTMGMT:
		p.opt = (struct t_optmgmt *)
			kmem_zalloc((u_int)sizeof(struct t_optmgmt), KM_SLEEP);
		if (fields & T_OPT){
			_alloc_buf(&p.opt->opt, tiptr->tp_info.options);
		}
		*ptr = ((char *)p.opt);
		return 0;

	case T_DIS:
		p.dis = (struct t_discon *)
			kmem_zalloc((u_int)sizeof(struct t_discon), KM_SLEEP);
		if (fields & T_UDATA){
			_alloc_buf(&p.dis->udata, tiptr->tp_info.discon);
		}
		*ptr = ((char *)p.dis);
		return 0;

	case T_UNITDATA:
		p.udata = (struct t_kunitdata *)
			kmem_zalloc((u_int)sizeof(struct t_kunitdata), KM_SLEEP);
		if (fields & T_ADDR){
			_alloc_buf(&p.udata->addr, tiptr->tp_info.addr);
		}
		else	p.udata->addr.maxlen = p.udata->addr.len = 0;

		if (fields & T_OPT){
			_alloc_buf(&p.udata->opt, tiptr->tp_info.options);
		}
		else	p.udata->opt.maxlen = p.udata->opt.len = 0;

		if (fields & T_UDATA){
			p.udata->udata.udata_mp = NULL;
			p.udata->udata.buf = NULL;
			p.udata->udata.maxlen = tiptr->tp_info.tsdu;
			p.udata->udata.len = 0;
		}
		else	p.udata->udata.maxlen = p.udata->udata.len = 0;
		*ptr = (char *)p.udata;
		return 0;

	case T_UDERROR:
		p.uderr = (struct t_uderr *)
			kmem_zalloc((u_int)sizeof(struct t_uderr), KM_SLEEP);
		if (fields & T_ADDR){
			_alloc_buf(&p.uderr->addr, tiptr->tp_info.addr);
		}
		if (fields & T_OPT){
			_alloc_buf(&p.uderr->opt, tiptr->tp_info.options);
		}
		*ptr = (char *)p.uderr;
		return 0;

	case T_INFO:
		p.info = (struct t_info *)
			kmem_zalloc((u_int)sizeof(struct t_info), KM_SLEEP);
		*ptr = (char *)p.info;
		return 0;

	default:
		return EINVAL;
	}
}

 
static void
_alloc_buf(buf, n)
register struct netbuf *buf;
register long n;
{
	switch(n)
	{
		case -1:
			buf->buf = (char *)kmem_zalloc((u_int)1024, KM_SLEEP);
			buf->maxlen = 1024;
			buf->len = 0;
			break;

		case 0:
		case -2:
			buf->buf = NULL;
			buf->maxlen = 0;
			buf->len = 0;
			break;

		default:
			buf->buf = (char *)kmem_zalloc((u_int)n, KM_SLEEP);
			buf->maxlen = n;
			buf->len = 0;
			break;
	}
}
