/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988, 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/io/ots/iTLIutils.c	1.3"

/*
** ABSTRACT:	Generic TPI routines for SV-ots driver.
**
** MODIFICATIONS:
*/

#include "sys/ots.h"

extern ulong ots_stat[OTS_SCNT];
extern endpoint ots_endpoints[];
extern int ots_debug;
extern ushort ots_where;


/* FUNCTION:			iTLI_ioctl_check()
 *
 * ABSTRACT:	Process ioctl requests on stream
 *
 * NOTE:  Currently, requests received on streams established through the
 *	administration device (/dev/ots-00) are allowed.  This restriction
 *	may have to be relaxed if we later add support for non-TPI messages
 *	on streams.
 *
 * CALLED FROM:	iTLI_check_msg()
 */
iTLI_ioctl_check(ep, mptr, wr_q)
endpoint	*ep;
mblk_t		*mptr;
queue_t		*wr_q;
{
	struct iocblk *ioc;
	int i;

	DEBUGC('Z');
	DEBUGP(DEB_CALL,(CE_CONT, "iTLI_ioctl_check()\n"));
	ioc = (struct iocblk *)mptr->b_rptr;

	/* First make sure this is the administrator entry */

	if (ep != &ots_endpoints[0])
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "ioctl_check: wrong ep\n"));
		mptr->b_datap->db_type = M_IOCNAK;
		ioc->ioc_count = 0;
		ioc->ioc_error = EACCES;
		qreply(wr_q, mptr);
		return;
	}

	switch(ioc->ioc_cmd)
	{
	case 1:
		DEBUGP(DEB_FULL,(CE_CONT, "iTLI_ioctl_check: stats requested\n"));
		if (  (mptr->b_cont == NULL)
		    ||((mptr->b_cont->b_datap->db_lim
			  - mptr->b_cont->b_datap->db_base)
			  < sizeof(ots_stat)
		      )
		   )
		{
			mptr->b_datap->db_type = M_IOCNAK;
			ioc->ioc_count = 0;
			ioc->ioc_error = EINVAL;
		}
		else
		{
			mptr->b_datap->db_type = M_IOCACK;
			mptr->b_cont->b_rptr = mptr->b_cont->b_datap->db_base;
			mptr->b_cont->b_wptr = mptr->b_cont->b_rptr
						+ sizeof(ots_stat);
			bcopy((char *)ots_stat, (char *)mptr->b_cont->b_rptr,
						sizeof(ots_stat));
			ioc->ioc_count = sizeof(ots_stat);
			ioc->ioc_error = 0;
		}
		break;
#ifdef DEBUG
	case 2:
		if (mptr->b_cont == NULL)
		{
			mptr->b_datap->db_type = M_IOCNAK;
			ioc->ioc_count = 0;
			ioc->ioc_error = EINVAL;
		}
		else
		{
			DEBUGP(DEB_CALL,(CE_CONT, "iTLI_ioctl_check: set debug level %d\n",
				*(int *)mptr->b_cont->b_rptr));
			*(char *)&i = *mptr->b_cont->b_rptr;
			switch (i)
			{
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
				ots_debug = i;
				break;
			case 10:
				ots_where = PRW_CONS | PRW_BUF;
				break;
			case 11:
				ots_where = PRW_CONS;
				break;
			case 12:
				ots_where = PRW_BUF;
				break;
			default:
				break;
			}
			mptr->b_datap->db_type = M_IOCACK;
			ioc->ioc_count = 0;
			ioc->ioc_error = 0;
		}
		break;
	case 4:
		DEBUGP(DEB_CALL,(CE_CONT, "iTLI_ioctl_check: reseting M_debugtxt\n"));
		for(i=0;i<10001;i++)
			M_debugtxt[i] = 0;
		M_debugptr = M_debugtxt;
		mptr->b_datap->db_type = M_IOCACK;
		ioc->ioc_count = 0;
		ioc->ioc_error = 0;
		break;
#endif
	case 6:		
		DEBUGP(DEB_CALL,(CE_CONT, "iTLI_ioctl_check: full reset\n"));
		iTLI_reset();
		mptr->b_datap->db_type = M_IOCACK;
		ioc->ioc_count = 0;
		ioc->ioc_error = 0;
		break;
	default:
		DEBUGP(DEB_ERROR,(CE_CONT, "iMB2ioctl_check: unknown ioctl request %x\n", ioc->ioc_cmd));
		/*
		 * No other type of ioctl should ever get here.
		 */
		mptr->b_datap->db_type = M_IOCNAK;
		ioc->ioc_count = 0;
		ioc->ioc_error = EACCES;
		break;
	}
	qreply(wr_q, mptr);
	DEBUGP(DEB_CALL,(CE_CONT, "iMB2_ioctl_check => NULL\n"));
}


/* FUNCTION:		M_getmptr()
 *
 * ABSTRACT:	Allocate critical STREAMS buffer on endpoint
 *
 *	If the STREAMs buffer can't be allocated, a fatal error is
 *	registered on the endpoint.
 */
mblk_t *
M_getmptr(size, pri, ep)
int	size;
int	pri;
endpoint *ep;
{
	mblk_t	*mptr;

	DEBUGC('{');
	if ((mptr = allocb(size, (uint)pri)) == NULL)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "getmptr: alloc failed\n"));
		ots_stat[ST_ALFA]++;
		iTLI_pferr(ep, ENOSR);
	}
	else	/* flag buffer as allocated by driver */
	{
		DEBUGP(DEB_STOP,(CE_CONT, "getmptr => %x\n", mptr));
		*(ushort *)mptr->b_wptr = 0xffff;
		*(ushort *)(mptr->b_wptr+2) = size;
	}
	return(mptr);
}


/* FUNCTION:		M_growmptr()
 *
 * ABSTRACT:	Ensure buffer large enough to hold response
 *
 *	If not, a larger buffer is allocated.
 */
mblk_t *
M_growmptr(size, pri, ep, mptr)
int	size;
int	pri;
endpoint *ep;
mblk_t *mptr;
{
	mblk_t *tmp;

	DEBUGC('}');
	if ((mptr->b_datap->db_lim - mptr->b_datap->db_base) < size)
	{
		DEBUGP(DEB_STOP,(CE_CONT, "growmptr: %x not big enough\n", mptr));
		tmp = M_getmptr(size, pri, ep);
		if (tmp == NULL)
		{
			DEBUGP(DEB_STOP,(CE_CONT, "growmptr: => NULL (getmpr <= NULL)\n"));
			freemsg(mptr);
			return(NULL);
		}
		tmp->b_datap->db_type = mptr->b_datap->db_type;
		((union T_primitives *)tmp->b_datap->db_base)->type =
			((union T_primitives *)mptr->b_datap->db_base)->type;
		freemsg(mptr);
		DEBUGP(DEB_STOP,(CE_CONT, "growmptr => %x\n",tmp));
		return(tmp);
	}
	else
		return(mptr);
}
