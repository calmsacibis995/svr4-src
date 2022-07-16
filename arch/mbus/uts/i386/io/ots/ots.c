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

#ident	"@(#)mbus:uts/i386/io/ots/ots.c	1.3"

#ifndef lint
static char ots_copyright[] = "Copyright 1988, 1989 Intel Corp. 463048";
#endif

/*
** ABSTRACT:	Open and close routines for the SV-ots driver
**
**	These routines run within the context of the calling task
**
** MODIFICATIONS:
*/

#include "sys/ots.h"
#include <sys/immu.h>
#include <sys/region.h>
#include <sys/proc.h>
#include <sys/file.h>
#include <sys/debug.h>

/*
 * These variables must not be commented out if any driver module
 *  has DEBUG turned on.
 */
char M_debugtxt[10001];
char *M_debugptr = M_debugtxt;

/*
 * SV-ots debug level global variable:
 *	0 = DEB_NONE - show nothing
 *	1 = DEB_ERROR - show most error conditions
 *	2 = DEB_CALL - show only call entries and returns
 *	3 = DEB_FULL - show everything
 */
int ots_debug = 1;
ushort ots_where = PRW_CONS | PRW_BUF;
ushort sav_prt_where;

ulong ots_stat[OTS_SCNT] = {0};		/* statistics array */
ushort ots_resetting;			/* driver reset in progress */
#ifdef V_3
ushort otsdg_major;			/* major number of datagram driver */
#else
major_t otsdg_major;			/* major number of datagram driver */
#endif

int otsdevflag = 0;		/* V4.0 style driver */

/*
 * The following are standard STREAMS data structures.
 */
int otsopen(), otsclose();
extern int iTLIrsrv(), iTLIwput(), iTLIwsrv();

struct module_info 
	ots_winfo = {OTS_NUMBER, OTS_NAME,
		OTS_STRMIN, OTS_STRMAX, OTS_DOWN_HIWAT, OTS_DOWN_LOWAT},
	ots_rinfo = {OTS_NUMBER, OTS_NAME,
		OTS_STRMIN, OTS_STRMAX, OTS_UP_HIWAT, OTS_UP_LOWAT};

struct module_stat otsmod_stat = {0, 0, 0, 0, 0,
					(char *)ots_stat, OTS_SCNT};

struct qinit
	otswinit = {iTLIwput, iTLIwsrv,  NULL,  NULL, NULL,
			&ots_winfo, &otsmod_stat},
	otsrinit = {NULL, iTLIrsrv,  otsopen,  otsclose, NULL,
			&ots_rinfo, &otsmod_stat};

struct streamtab otsinfo = {&otsrinit, &otswinit, NULL, NULL};

/*
 * These varaibles are defined in the SV-ots driver space.c file
 */
extern endpoint ots_endpoints[];
extern caddr_t ots_address;
extern struct otscfg otscfg;


/* FUNCTION:			otsopen()
 *
 * ABSTRACT:	Called when the driver is opened
 *
 *	There are two ways the driver can be opened:
 *
 *		minor(rdev) = n, sflag = 0
 *			Indicates that a user is trying to open endpoint 'n'.
 *		minor(rdev) = 0, slfag = CLONEOPEN  
 *			Indicates that a user is trying to allocate a new
 *                      endpoint.
 *
 *	In the latter case, the driver may be called directly from clone
 *	when the user is trying to establish a virtual circuit endpoint
 *	or indirectly through the otsdg driver for datagram endpoints.
 *
 *	SV-ots administration programs access the driver directly through
 *	minor device '0'.
 *
 * RETURNS:	OPENFAIL - couldn't allocate endpoint
 *		0 - open sucessful
 */
#ifdef V_3
int
otsopen(rd_q, rdev, oflag, sflag)
queue_t *rd_q;
int rdev, oflag, sflag;
#else
int
otsopen(rd_q, rdev, oflag, sflag,credp)
queue_t *rd_q;
dev_t	*rdev;				/* This is pointer to device name */
int oflag, sflag;
struct cred *credp;
#endif
{
	endpoint *ep;			/* opened endpoint */
	struct stroptions *soptr;	/* used to set streams parameters */
	mblk_t *mptr;			/* message containing stream params */
#ifdef V_3
	ushort minor_dev;		/* index for searching ep table */
#else
	minor_t minor_dev;		/* index for searching ep table */
#endif
	ushort start, end;		/* bounds for free endpoint search */

	DEBUGC('t');
#ifdef V_3
	DEBUGP(DEB_CALL,(CE_CONT, "otsopen(dev = %d)\n", rdev));
#else
	DEBUGP(DEB_CALL,(CE_CONT, "otsopen(dev = %d)\n", *rdev));
#endif

	if (ots_resetting == TRUE)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "otsopen EBUSY: driver reset in progress"));
#ifdef V_3
		u.u_error = EBUSY;
		return(OPENFAIL);
#else
		return(EBUSY);
#endif
	}
	else if (oflag & ~(FREAD|FWRITE|FNDELAY|FEXCL))
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "otsopen EINVAL: oflag = %x\n", oflag));
#ifdef V_3
		u.u_error = EINVAL;
		return(OPENFAIL);
#else
		return(EINVAL);
#endif
	}
	else if (ots_stat[ST_CURO] >= otscfg.n_endpoints)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "otsopen ENOSPC: exceeded max endpoints\n"));
#ifdef V_3
		u.u_error = ENOSPC;
		return(OPENFAIL);
#else
		return(ENOSPC);
#endif
	}

#ifdef V_3
	minor_dev = minor(rdev);
#else
	minor_dev = getminor(*rdev);
#endif

	if (  (sflag == CLONEOPEN)
	    &&(minor_dev == 0)
	   )
	{
		/*
		 * Find a free endpoint from the location in the endpoint
		 * array for reserved for the requested service type.
		 *
		 * VC endpoints are allocated first, DG endpoints after.
		 */
		DEBUGP(DEB_FULL,(CE_CONT, "otsopen: finding free endpoint\n"));
#ifdef V_3
		if (otsdg_major == major(rdev))
#else
		if (otsdg_major == getmajor(*rdev))
#endif
		{
			start = otscfg.n_vcs + 1;
			end = otscfg.n_endpoints;
		}
		else
		{
			start = 1;
			end = otscfg.n_vcs + 1;
		}

		for (minor_dev = start; minor_dev < end; minor_dev++)
			if (ots_endpoints[minor_dev].str_state == C_IDLE)
				break;
		if (minor_dev == end)
		{
#ifdef V_3
			u.u_error = ENOSPC;
			return(OPENFAIL);
#else
			return(ENOSPC);
#endif
		}
#ifndef V_3
		/*
		 *	Make new device in *rdev.
		 */
		*rdev = makedevice(getmajor(*rdev),minor_dev);
#endif
	}
	else if (  (sflag != 0)
		 ||(minor_dev >= otscfg.n_endpoints)
		)
	{
		/*
		 * Some illegal minor minor_device number:
		 * Perhaps an out-of-range minor device was opened.
		 */
		DEBUGP(DEB_ERROR,(CE_CONT, "otsopen ECHRNG: sflag = %x, minor_dev = %x\n",
				 sflag, minor_dev));
#ifdef V_3
		u.u_error = ECHRNG;
		return(OPENFAIL);
#else
		return(ECHRNG);
#endif
	}
	ep = &ots_endpoints[minor_dev];
	DEBUGP(DEB_FULL,(CE_CONT, "otsopen: ep = %x, minor_dev = %x\n", ep, minor_dev));
	/*
	 * Detect multiple opens on the same STREAM.
	 */
	if (ep->str_state & C_OPEN)
	{
		/*
		 * The STREAM is already completely opened.
		 */
		DEBUGP(DEB_FULL,(CE_CONT, "otsopen: stream already opened\n"));
		DEBUGP(DEB_CALL,(CE_CONT, "otsopen => %d\n", minor_dev));
		if (  (ep->exclude)
		    ||(oflag & FEXCL)
		   )
		{
			DEBUGP(DEB_ERROR,(CE_CONT, "otsopen: Re_open of exclusive dev\n"));
			DEBUGP(DEB_CALL,(CE_CONT, "otsopen => EACCES\n"));
#ifdef V_3
			u.u_error = EACCES;
			return(OPENFAIL);
#else
			return(EACCES);
#endif
		}
	}
	else	/* new endpoint, clear and initialize */
	{
		bzero((char *)ep, sizeof(endpoint));
		/*
		 * Set the streamhead's configuration options for the
		 * READ queue to be consistent with those used by ots.
		 */
		if((mptr = allocb(sizeof(struct stroptions), BPRI_MED)) == NULL)
		{
			ots_stat[ST_ALFA]++;
#ifdef V_3
			u.u_error = ENOSR;
			return(OPENFAIL);
#else
			return(ENOSR);
#endif
		}
		mptr->b_datap->db_type = M_SETOPTS;
		soptr = (struct stroptions *) mptr->b_rptr;
		soptr->so_flags   = SO_MINPSZ | SO_MAXPSZ | SO_HIWAT | SO_LOWAT;
		soptr->so_readopt = 0;
		soptr->so_wroff   = 0;
		soptr->so_minpsz  = OTS_STRMIN;
		soptr->so_maxpsz  = OTS_STRMAX;
		soptr->so_hiwat   = otscfg.sh_hiwat;
		soptr->so_lowat   = otscfg.sh_lowat;
		mptr->b_wptr = mptr->b_rptr + sizeof(struct stroptions);
		putnext(rd_q, mptr);

		ep->tli_state = TS_UNBND;
		ep->str_state = C_OPEN;
		ep->addr.length = 0;
		ep->addr.data = (char *)&ots_address + (otscfg.addr_size * minor_dev);
		ep->rd_q = rd_q;
		rd_q->q_ptr = (WR(rd_q))->q_ptr = (caddr_t)ep;

		if (minor_dev <= otscfg.n_vcs)
			ep->options = otscfg.vc_defaults;
		else
			ep->options = otscfg.dg_defaults;
		if (oflag & FEXCL)
			ep->exclude = TRUE;

		ots_stat[ST_CURO]++;
		ots_stat[ST_TOTO]++;
		DEBUGP(DEB_FULL,(CE_CONT, "ots: opened minor device #%x\n", minor_dev));
		DEBUGP(DEB_CALL,(CE_CONT, "otsopen => %d\n", minor_dev));
	}
#ifdef V_3
	return(minor_dev);
#else
	return(0);
#endif
}

/* FUNCTION:			otsclose()
 *
 * ABSTRACT:	Called whenever a user closes the device.
 *
 *	After waiting for output (if any) to drain, this routine marks the
 *	endpoint for deletion and calls iTLI_abort() to free any resources
 *	associated with the endpoint.  This marking is via the C_DISCON flag.
 *	The freeing of resources is done by lower layer routines which may
 *	operate asynchronous to the process doing the close.  Those tasks
 *	are responsible for resetting the endpoint to the idle state.
 */
int
otsclose(rd_q)
queue_t *rd_q;
{
	endpoint *ep;
	int pri;
	void ots_ignore_drain();

	DEBUGC('u');
	DEBUGP(DEB_CALL,(CE_CONT, "otsclose()\n"));
	ep = (endpoint *)rd_q->q_ptr;

	/*
	 * wait until output drains; allow 50 msecs.
	 */
	while (ep->nbr_datarq)
	{
		ep->str_state |= C_DRAIN;
		timeout(ots_ignore_drain, ep, 5);
		sleep((caddr_t)ep, STOPRI|PCATCH);
		if (issig(JUSTLOOKING))
			break;
	}

	ep->str_state |= C_CLOSE;
	ep->exclude = FALSE;
	pri = SPL();
	iTLI_abort(ep);
	splx(pri);
	ep->tli_state = 0;

	/*
	 * disassociate endpoint from queue
	 */
	rd_q->q_ptr = (WR(rd_q))->q_ptr = (caddr_t)NULL;
	ep->rd_q = NULL;

	ots_stat[ST_CURO]--;
	DEBUGP(DEB_CALL,(CE_CONT, "otsclose => NULL\n"));
	return (0);
}


/* FUNCTION:		ots_ignore_drain()
 *
 * ABSTRACT:	Break close out of wait loop when output just won't drain
 */
void
ots_ignore_drain(ep)

endpoint *ep;
{
	DEBUGP(DEB_CALL,(CE_CONT, "ots_ignore_drain: ep=%x nbr=%x\n",ep,ep->nbr_datarq));
	if (  (ep->str_state & C_DRAIN)
	    &&(ep->nbr_datarq)
	   )
	{
		ep->nbr_datarq = 0;
		wakeup(ep);
	}
}
