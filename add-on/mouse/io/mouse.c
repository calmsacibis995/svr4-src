/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mouse:io/mouse.c	1.3.1.2"

/*
 *	indirect driver for underlying mouse STREAMS drivers/modules.
 */
#include "sys/types.h"
#include "sys/kmem.h"
#include "sys/param.h"
#include "sys/immu.h"
#include "sys/sysmacros.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/proc.h"
#include "sys/user.h"
#include "sys/conf.h"
#include "sys/tty.h"
#include "sys/stream.h"
#include "sys/strsubr.h"
#include "sys/cred.h"
#include "sys/uio.h"
#include "sys/session.h"
#include "sys/open.h"
#include "sys/vt.h"
#include "sys/cmn_err.h"
#include "sys/mouse.h"
#include "mse.h"

static int	mouse_opening;		/* Set if an open is in progress to
					   eliminate plock race condition */
static struct mse_mon mgr_command;	/* Current command to manager */
static int	mgr_unit = -1;		/* Current mouse unit requesting mgr */

int msedevflag = 0;
extern int	mse_nbus;

MOUSE_UNIT	mse_unit[MAX_MSE_UNIT+1];
int		mse_nunit=0;
MOUSE_STRUCT	*mse_structs;

char	msebusy[MAX_MSE_UNIT+1];
static int	mon_open;		/* Monitor chan is open (exclusive) */
static int	cfg_in_progress;	/* Configuration is in progress */

int mseconfig();

int
mseopen(devp, flag, otyp, cr)
	dev_t *devp;
	int flag;
	int otyp;
	struct cred *cr;
{
	int	n, i, tmp;
	register MOUSE_STRUCT *m;
	struct mount	*mp;
	int	waitcnt = 200000;
	int error=0;
	dev_t sydev;

	if (otyp != OTYP_LYR) {
		switch (getminor(*devp)) {
		case MSE_CLONE:
			break;
		case MSE_MON:
			if (!(error = drv_priv(cr))) {
				if (mon_open)
					return(EBUSY);
				else{
					mon_open = 1;
					return 0;
				}
			}
			return(error);
		case MSE_CFG:
			return 0;
		default:
			return(ENXIO);
		}
	}
	if (error = ws_getctty(&sydev))
		return error;
#ifdef DEBUG
printf("entered mseopen:/dev/mouse\n");
#endif
/* If a configuration is in progress, wait until it's done */
	while (cfg_in_progress)
		sleep(&cfg_in_progress, PZERO + 3);

/* Find the mouse device corresponding to the process's controlling terminal.
 * If there isn't one, fail and return .
 */
	for (n = 0; n < mse_nunit; n++) {
		if (DISP_UNIT(mse_unit[n].map.disp_dev) != DISP_UNIT(sydev))
			continue;
		else
			break;
	}
	if (n == mse_nunit || MSE_UNIT(sydev) >= mse_unit[n].n_vts) {
		return(ENXIO);
	}

/* Make sure there is no other open pending on the mouse */
	while (mouse_opening)
		sleep (&mouse_opening, PZERO + 3);
	mouse_opening = 1;

/* Enforce exclusive open on a virtual mouse */
	if (mgr_unit == n || mse_unit[n].ms[MSE_UNIT(sydev)].isopen == 1) {
		error = EBUSY;
		goto out;
	}

#ifdef DEBUG
printf("mseopen:unit=%d, vt = %d\n",n, MSE_UNIT(sydev));
#endif
	m = &mse_unit[n].ms[MSE_UNIT(sydev)];
#ifdef DEBUG
printf("mseopen:mouse TYPE = %d unit = %d\n",mse_unit[n].map.type, n);
#endif

#ifdef DEBUG
printf("mseopen:calling MSE_MGR\n");
#endif
	if (mse_mgr_cmd(MSE_MGR_OPEN, sydev, n) == -1){
#ifdef DEBUG
printf("mseopen:failed to attach STREAMS mouse module/driver\n");
#endif
		error = mgr_command.errno;
		goto out;
	}
/* Done opening */
	*devp = makedevice(getmajor(*devp), getminor(sydev));
	m->isopen = msebusy[n] = 1;
out:
	mouse_opening = 0;
	wakeup(&mouse_opening);
#ifdef DEBUG
printf("leaving mseopen\n");
#endif
	return error;
}

int
mseclose(dev, flag, otyp, cr)
	dev_t dev;
	int flag;
	int otyp;
	struct cred *cr;
{
	register int	unit, i, tmp, error;
	dev_t sydev;

#ifdef DEBUG
printf("entered mseclose:\n");
#endif
	switch (getminor(dev)) {
	case MSE_MON:
		mon_open = 0;
#ifdef DEBUG
printf("mseclose:MSE_MON\n");
#endif
		return 0;
	case MSE_CLONE:
	case MSE_CFG:
#ifdef DEBUG
printf("mseclose:MSE_CFG or MSE_CLONE\n");
#endif
		return 0;
	}

        if (error = ws_getctty(&sydev))
		return (error);

	for (i = 0; i < mse_nunit; i++) {
		if (DISP_UNIT(sydev) == DISP_UNIT(mse_unit[i].map.disp_dev)){
			unit = i;
			break;
		}
	}
	if(i == mse_nunit){
		for (i = 0; i < mse_nunit; i++) {
			if(mse_unit[i].map.mse_dev == dev){
				unit = i;
				goto got_unit;
			}
		}
#ifdef DEBUG
printf("mseclose:could not match dev\n");
#endif
		return ENODEV;
	}
got_unit:
	mse_unit[unit].ms[MSE_UNIT(dev)].isopen = 0;

#ifdef DEBUG
printf("mseclose:unit=%d, vt#=%d\n",unit , MSE_UNIT(dev));
#endif

/* Return if this was not the only virtual mouse open for this mouse */
	for (i =0;i< mse_unit[unit].n_vts; i++)
		if (mse_unit[unit].ms[i].isopen == 1){
#ifdef DEBUG
printf("mseclose: call mse_mgr_cmd - CLOSE\n");
#endif
			mse_mgr_cmd(MSE_MGR_CLOSE, sydev, unit);
#ifdef DEBUG
printf("leaving mseclose:\n");
#endif
			return mgr_command.errno;
		}

	msebusy[unit] = 0;
	if(mse_unit[unit].map.type == MSERIAL) {
		mse_unit[unit].old = -1;
	}

#ifdef DEBUG
printf("mseclose: call mse_mgr_cmd - LCLOSE\n");
#endif
	mse_mgr_cmd(MSE_MGR_LCLOSE, sydev, unit);
#ifdef DEBUG
printf("leaving mseclose:\n");
#endif
	return mgr_command.errno;
}

int
mseread(dev, uiop, cr)
	dev_t dev;
	struct uio *uiop;
	struct cred *cr;
{
	return (ENXIO);
}

int
msewrite(dev, uiop, cr)
	dev_t dev;
	struct uio *uiop;
	struct cred *cr;
{
	return (ENXIO);
}


int
mseioctl(dev, cmd, arg, mode, cr, rvalp)
	dev_t dev;
	int cmd;
	int arg;
	int mode;
	struct cred *cr;
	int *rvalp;
{
	int error = 0;
	dev_t ttyd;
	register int	i, unit;


	switch(cmd){
		case MOUSEIOCMON:
#ifdef DEBUG
printf("mseioctl: mouseiomon\n");
#endif 
			if (getminor(dev) != MSE_MON) {
				error = EINVAL;
#ifdef DEBUG
printf("mseioctl: mouseiomon - bad minor\n");
#endif 
				break;
			}
			if (copyin(arg, &mgr_command, sizeof(mgr_command))) {
				error = EFAULT;
#ifdef DEBUG
printf("mseioctl: mouseiomon - copyin failed\n");
#endif 
				break;
			}
			mgr_command.cmd |= MGR_WAITING;
			if (mgr_unit != -1) {
				mse_unit[mgr_unit].status = mgr_command.errno;
				wakeup(&mse_unit[mgr_unit].status);
				mgr_unit = -1;
			}
			wakeup(&mgr_command);
			do {
#ifdef DEBUG
printf("mseioctl:MOUSEIOCMON:going to sleep on command\n");
#endif
				if (sleep(&mgr_command, (PZERO + 3)|PCATCH)) {
					if ((mgr_command.cmd & MGR_WAITING) == MGR_WAITING)
						mgr_command.cmd &= ~MGR_WAITING;
#ifdef DEBUG
printf("mseioctl:MOUSEIOCMON:wakeup due to PCATCH\n");
#endif
					return EINTR;
				}
			} while((mgr_command.cmd & MGR_WAITING) == MGR_WAITING);
#ifdef DEBUG
printf("mseioctl:MOUSEIOCMON:waking from sleep on command\n");
#endif
			mgr_command.errno = 0;
			if (copyout(&mgr_command, arg, sizeof(mgr_command))) {
				error = EFAULT;
#ifdef DEBUG
printf("mseioctl: mouseiomon - copyout failed\n");
#endif 
				break;
			}
			break;
	
		case MOUSEISOPEN:
#ifdef 	DEBUG
printf("mseioctl: mouseisopen\n");
#endif 
			if (getminor(dev) != MSE_CFG ) {
				error = EINVAL;
				break;
			}
			for(i=0;i<MAX_MSE_UNIT+1;i++)
				msebusy[i] = 0;
			if(mse_nunit){
				for(unit=0;unit<mse_nunit;unit++){
					if(mse_unit[unit].ms != NULL){
						for (i = mse_unit[unit].n_vts; i-- > 0;)
							if(mse_unit[unit].ms[i].isopen == 1){
								msebusy[unit] = 1;
								break;
							}
					}
				}
			}
			if (copyout(msebusy, arg, sizeof(msebusy))) {
				error = EFAULT;
				break;
			}
			break;
		case MOUSEIOCCONFIG:
#ifdef 	DEBUG
printf(	"mseioctl: mouseioconfig\n");
#endif 
			if (getminor(dev) != MSE_CFG && getminor(dev) != MSE_MON) {
				error = EINVAL;
				break;
			}
			{
				struct mse_cfg	mse_cfg;

			/* Wait for any other configuration to finish */
				while (cfg_in_progress)
					sleep(&cfg_in_progress, PZERO + 3);
	
				/* If any mice are open, we can't configure */
				if (mouse_opening || mgr_unit != -1) {
					error = EBUSY;
					break;
				}

				cfg_in_progress = 1;

				if (copyin(arg, &mse_cfg, sizeof(mse_cfg))) {
					cfg_in_progress = 0;
					wakeup(&cfg_in_progress);
					error = EFAULT;
					break;
				}
				error = mseconfig(mse_cfg.mapping, mse_cfg.count);

				cfg_in_progress = 0;
				wakeup(&cfg_in_progress);
			}
			break;
		default:{
			if (error = ws_getctty(&ttyd))
				return error;

			error = ws_ioctl(ttyd,cmd,arg,mode,cr,rvalp);
			return error;
		}
	}
	return error;
}

int
mseconfig(maptbl, mapcnt)
struct mousemap *maptbl;
unsigned mapcnt;
{
	struct mousemap	map;
	register int	unit;
	register int	bunit;
	register MOUSE_STRUCT	*ms;
	int	mse_nms, errflg = 0;
	int  Ounit, msecnt, oldpri;

#ifdef DEBUG
printf("entered mseconfig - mapcnt=%d\n",mapcnt);
#endif
	oldpri = splstr();
	if(mapcnt < mse_nunit){
		for(unit=mapcnt;unit<mse_nunit;unit++){
			if(mse_unit[unit].ms != NULL && !msebusy[unit]){
				mse_nms = mse_unit[unit].n_vts ;
				kmem_free((caddr_t)mse_unit[unit].ms, mse_nms * sizeof(MOUSE_STRUCT));  /* free memory */
				mse_unit[unit].ms = (MOUSE_STRUCT *) NULL;
				mse_unit[unit].map.mse_dev = 0;
				mse_unit[unit].map.disp_dev = 0;
				mse_unit[unit].map.type = 0;
			}
		}
	}
	msecnt = mse_nunit;
	mse_nunit = 0;
	/* Set up and validate mapping table information */
	for (unit = 0; mapcnt-- > 0 && unit <= MAX_MSE_UNIT;unit++) {
		if (copyin(maptbl++, &map, sizeof(struct mousemap))) {
#ifdef DEBUG
printf("mseconfig:failure \n");
#endif
			splx(oldpri);
			return(EFAULT);
		}
		for(Ounit=0;Ounit<msecnt;Ounit++){
			if((mse_unit[Ounit].map.mse_dev == map.mse_dev) && (mse_unit[Ounit].map.disp_dev == map.disp_dev)){
				if(Ounit != unit){
					if(msebusy[Ounit]){
						msebusy[Ounit] = 0;
						msebusy[unit] = 1;
					}
					mse_unit[unit] = mse_unit[Ounit];
					mse_unit[Ounit].ms = NULL;
					mse_unit[Ounit].map.mse_dev = 0;
					mse_unit[Ounit].map.disp_dev = 0;
					mse_unit[Ounit].map.type = 0;
				}
				goto maploop;
			}
		}
		goto fixmap;
maploop:
		mse_nunit = unit + 1;
		continue;
fixmap:
		if(mse_unit[unit].ms != NULL){	/* free memory */
			mse_nms = mse_unit[unit].n_vts ;
			kmem_free((caddr_t)mse_unit[unit].ms, mse_nms * sizeof(MOUSE_STRUCT));
			mse_unit[unit].ms = (MOUSE_STRUCT *) NULL;
		}

		mse_unit[unit].map = map;
		if (map.type == MSERIAL) {
			mse_unit[unit].old = -1;
		}
		mse_unit[unit].n_vts = 15; /* XXX this should be WS_MAXCHAN */
		if (mse_unit[unit].n_vts < 1){
			errflg = 1;
#ifdef DEBUG
printf("mseconfig:errflag - unit=%d \n",unit);
#endif
		}
		if(errflg){
			errflg = 0;
			mse_unit[unit].map.mse_dev = 0;
			mse_unit[unit].map.disp_dev = 0;
			mse_unit[unit].map.type = 0;
			continue;
		}
		if (mse_unit[unit].n_vts > VTMAX)
			mse_unit[unit].n_vts = VTMAX;

		mse_nms = mse_unit[unit].n_vts ;
		ms = (MOUSE_STRUCT *)kmem_zalloc(mse_nms * sizeof(MOUSE_STRUCT), KM_SLEEP);
		if ( ms == NULL) {
			cmn_err(CE_WARN, "Not enough memory for mouse structure");
			mse_unit[unit].map.mse_dev = 0;
			mse_unit[unit].map.disp_dev = 0;
			mse_unit[unit].map.type = 0;
			splx(oldpri);
			return(ENOMEM);
		}
	
		mse_nunit = unit + 1;
		mse_unit[unit].ms = ms;
	}
	splx(oldpri);
#ifdef DEBUG
printf("leaving mseconfig - nunit= %d\n",mse_nunit);
#endif
	return 0;
}

int
mse_mgr_cmd(cmd, dev, unit)
int	 unit;
dev_t	dev;
{
#ifdef DEBUG
printf("mse_mgr_cmd: entered\n");
#endif
	while ((mgr_command.cmd & MGR_WAITING) != MGR_WAITING) {
#ifdef DEBUG
printf("mse_mgr_cmd: going to sleep, MGR not waiting\n");
#endif
		if (sleep(&mgr_command, (PZERO + 3)|PCATCH)) {
#ifdef DEBUG
printf("mse_mgr_cmd:waking due to PCATCH\n");
#endif
			return -1;
		}
	}
	mse_unit[mgr_unit = unit].status = -1;
	mgr_command.cmd = cmd ;
	mgr_command.dev = dev;
	mgr_command.mdev = mse_unit[unit].map.mse_dev;
	wakeup(&mgr_command);
	do {
		if (sleep(&mse_unit[unit].status, (PZERO + 3)|PCATCH) || u.u_signal[SIGKILL -1] == SIG_IGN ) {
			return -1;
		}
	} while (mse_unit[unit].status == -1);

	if(mse_unit[unit].status == 0)
		return 0;
	else
		return -1;
}
