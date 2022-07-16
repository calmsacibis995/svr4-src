/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-ktli:t_kfree.c	1.1"
#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)t_kfree.c 1.1 88/12/12 SMI"
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
 *	Free the specified kernel tli data structure.
 *
 *	Returns:
 *		0 on success or
 *	        positive error code.
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
#include <sys/t_kuser.h>


/*ARGSUSED*/
int
t_kfree(tiptr, ptr, struct_type)
	register TIUSER			*tiptr;
	register char			*ptr;
	register int			struct_type;
{
	union structptrs {
		struct t_bind		*bind;
		struct t_call		*call;
		struct t_discon		*dis;
		struct t_optmgmt	*opt;
		struct t_kunitdata	*udata;
		struct t_uderr		*uderr;
	} p;
	int				error;

	error = 0;

	/*
	 * Free all the buffers associated with the appropriate
	 * fields of each structure.
	 */

	switch (struct_type) {

	case T_BIND:
		/* LINTED pointer alignment */
		p.bind = (struct t_bind *)ptr;
		if (p.bind->addr.buf != NULL)
			kmem_free(p.bind->addr.buf, (u_int)p.bind->addr.maxlen);
		kmem_free(ptr, (u_int)sizeof(struct t_bind));
		break;

	case T_CALL:
		/* LINTED pointer alignment */
		p.call = (struct t_call *)ptr;
		if (p.call->addr.buf != NULL)
			kmem_free(p.call->addr.buf, (u_int)p.call->addr.maxlen);
		if (p.call->opt.buf != NULL)
			kmem_free(p.call->opt.buf, (u_int)p.call->opt.maxlen);
		if (p.call->udata.buf != NULL)
			kmem_free(p.call->udata.buf, (u_int)p.call->udata.maxlen);
		kmem_free(ptr, (u_int)sizeof(struct t_call));
		break;

	case T_OPTMGMT:
		/* LINTED pointer alignment */
		p.opt = (struct t_optmgmt *)ptr;
		if (p.opt->opt.buf != NULL)
			kmem_free(p.opt->opt.buf, (u_int)p.opt->opt.maxlen);
		kmem_free(ptr, (u_int)sizeof(struct t_optmgmt));
		break;

	case T_DIS:
		/* LINTED pointer alignment */
		p.dis = (struct t_discon *)ptr;
		if (p.dis->udata.buf != NULL)
			kmem_free(p.dis->udata.buf, (u_int)p.dis->udata.maxlen);
		kmem_free(ptr, (u_int)sizeof(struct t_discon));
		break;

	case T_UNITDATA:
		/* LINTED pointer alignment */
		p.udata = (struct t_kunitdata *)ptr;

		if (p.udata->udata.udata_mp) {
		KTLILOG(2, "t_kfree: freeing mblk_t %x, ",
						p.udata->udata.udata_mp);
		KTLILOG(2, "ref %d\n",
				 p.udata->udata.udata_mp->b_datap->db_ref);
			freemsg(p.udata->udata.udata_mp);
		}
		if (p.udata->opt.buf != NULL)
			kmem_free(p.udata->opt.buf, (u_int)p.udata->opt.maxlen);
		if (p.udata->addr.buf != NULL) {
		KTLILOG(2, "t_kfree: freeing address %x, ",
						p.udata->addr.buf);
		KTLILOG(2, "len %d\n",
						p.udata->addr.maxlen);
			kmem_free(p.udata->addr.buf, (u_int)p.udata->addr.maxlen);
		}
		KTLILOG(2, "t_kfree: freeing t_kunitdata\n", 0);
		kmem_free(ptr, (u_int)sizeof(struct t_kunitdata));
		break;

	case T_UDERROR:
		/* LINTED pointer alignment */
		p.uderr = (struct t_uderr *)ptr;
		if (p.uderr->addr.buf != NULL)
			kmem_free(p.uderr->addr.buf, (u_int)p.uderr->addr.maxlen);
		if (p.uderr->opt.buf != NULL)
			kmem_free(p.uderr->opt.buf, (u_int)p.uderr->opt.maxlen);
		kmem_free(ptr, (u_int)sizeof(struct t_uderr));
		break;

	case T_INFO:
		break;

	default:
		error = EINVAL;
		break;
	}
	
	return error;
}
/******************************************************************************/


