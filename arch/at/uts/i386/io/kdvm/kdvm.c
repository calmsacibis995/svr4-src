/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)at:uts/i386/io/kdvm/kdvm.c	1.2.1.1"

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/immu.h"
#include "sys/proc.h"	
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/user.h"
#include "sys/inline.h"
#include "sys/kmem.h"
#include "sys/cmn_err.h"
#include "sys/vt.h"
#include "sys/at_ansi.h"
#include "sys/uio.h"
#include "sys/kd.h"
#include "sys/xque.h"
#include "sys/stream.h"
#include "sys/termios.h"
#include "sys/strtty.h"
#include "sys/stropts.h"
#include "sys/ws/ws.h"
#include "sys/ws/chan.h"
#include "sys/vid.h"
#include "sys/vdc.h"
#include "sys/cred.h"
#include "vm/as.h"
#include "vm/seg.h"
#include "vm/seg_objs.h"
#include "sys/mman.h"
#include "sys/ddi.h"

#define CH_MAPMX	10

extern wstation_t	Kdws;
extern struct vdc_info	Vdc;

extern int	kdvmemcnt;
extern struct kd_range	kdvmemtab[];
extern caddr_t	zfod_argsp;

channel_t	*ws_getchan(),
		*ws_activechan();

int	segvn_create();

struct kd_range	kd_basevmem[MKDIOADDR] = {
{ 0xa0000, 0xc0000 },
#ifdef	EVC
{ (unsigned long)0xD0000000, (unsigned long)0xD00FFFFF },
#endif	/*EVC*/
};

int kdvm_devflag = 0; /* we are new-style SVR4 driver */

#ifdef EVGA
extern int evga_inited;  
extern int new_mode_is_evga;
extern int evga_mode;
extern int evga_num_disp;
extern struct at_disp_info disp_info[];
extern unsigned long evga_type;
#endif /* EVGA */
/*
 *
 */

kdvm_open(devp, flag, otype, crp)
dev_t	*devp;
int	flag,
	otype;
struct cred	*crp;
{
	return(0);
}

/*
 *
 */

kdvm_close(dev, flag, otype, crp)
dev_t	dev;
int	flag,
	otype;
struct cred	*crp;
{
	return(0);
}

/*
 *
 */

kdvm_ioctl(dev, cmd, arg, mode, crp, rvalp)
dev_t	dev;
int	cmd,
	arg,
	mode;
struct cred	*crp;
int	*rvalp;
{
	channel_t	*chp;
	termstate_t	*tsp;
	vidstate_t	*vp;
	int	indx, cnt, rv = 0, hit = 1, oldpri;
	struct proc 	*procp;

	indx = getminor(dev);
	if ((chp = ws_getchan(&Kdws, indx)) == (channel_t *)NULL) {
		cmn_err(CE_WARN, "kdvm_ioctl: can't find channel");
		return(ENXIO);
	}
	tsp = &chp->ch_tstate;
	vp = &chp->ch_vstate;
	switch (cmd) {
	case KDSBORDER:	/* set border in ega color text mode */
		rv = kdv_sborder(chp, arg);
		break;
	case KDDISPTYPE: /* return display information to user */
		rv = kdv_disptype(chp, arg);
		break;
	case KDVDCTYPE:	/* return VDC controller/display information */
		rv = vdc_disptype(chp, arg);
		break;
	case KIOCSOUND:
		if (chp == ws_activechan(&Kdws))
			kdkb_sound(arg);
		rv = 0;
		break;
	case KDSETMODE:
		/* Here mode pertains to graphics or text. */
		switch ((int)arg) {
		case KD_TEXT0:
		case KD_TEXT1:
			if (!CHNFLAG(chp, CHN_XMAP) && CHNFLAG(chp, CHN_UMAP)) {
				rv = EIO;
				break;
			}
			if (chp->ch_dmode == arg)
				break;
			chp->ch_dmode = arg;
			if (arg == KD_TEXT0)
				kdv_textmode(chp);
			else
				kdv_text1(chp);
			break;
		case KD_GRAPHICS:
			if (chp->ch_dmode == KD_GRAPHICS)
				break;
			oldpri = splstr();
			kdv_scrxfer(chp, KD_SCRTOBUF);
			chp->ch_dmode = KD_GRAPHICS;
			/*
 			 * If start address has changed, we must re-zero
			 * the screen buffer so as to not confuse VP/ix.
			 */
			if (tsp->t_origin) {
				tsp->t_cursor -= tsp->t_origin;
				tsp->t_origin = 0;
				kdsetbase(chp, tsp);
				kdsetcursor(chp, tsp);
				kdv_scrxfer(chp, KD_BUFTOSCR);
			}
			splx(oldpri);
		}
		break;
	case KDGETMODE:
		if (copyout(&chp->ch_dmode, arg, sizeof(chp->ch_dmode)) < 0)
			rv = EFAULT;
		break;
	case KDQUEMODE:	/* enable/disable special queue mode */
		rv = ws_queuemode(chp, arg);
		break;
	case KDMKTONE:
		if (chp == ws_activechan(&Kdws))
			kdkb_mktone(chp, arg);
		break;
	case KDMAPDISP:
		rv = kdvm_mapdisp(chp, &Kdws.w_map, arg);
		break;
	case KDUNMAPDISP:
		drv_getparm(UPROCP, &procp);
		if (Kdws.w_map.m_procp != procp) {
			rv = EACCES;
			break;
		}
		if (!kdvm_unmapdisp(chp, &Kdws.w_map)) {
			rv = EFAULT;
			break;
		}
		break;
	case KDENABIO:
		enableio(chp->ch_vstate.v_ioaddrs);
		break;
	case KDDISABIO:
		disableio(chp->ch_vstate.v_ioaddrs);
		break;
	case KDADDIO:
		if (rv = drv_priv(crp))
			return(rv);
		if ((ushort)arg > MAXTSSIOADDR)
			return(ENXIO);
		for (indx = 0; indx < MKDIOADDR; indx++) {
			if (!vp->v_ioaddrs[indx]) {
				vp->v_ioaddrs[indx] = (ushort)arg;
				break;
			}
		}
		if (indx == MKDIOADDR)
			return(EIO);
		break;
	case KDDELIO:
		if (rv = drv_priv(crp))
			return(rv);
		for (indx = 0; indx < MKDIOADDR; indx++) {
			if (vp->v_ioaddrs[indx] != (ushort)arg)
				continue;
			for (cnt = indx; cnt < (MKDIOADDR - 1); cnt++)
				vp->v_ioaddrs[cnt] = vp->v_ioaddrs[cnt + 1];
			vp->v_ioaddrs[cnt] = (ushort)0;
			break;
		}
		break;
	case WS_PIO_ROMFONT: {
		unsigned int numchar, size;
		caddr_t newbuf;
		if (copyin((caddr_t) arg, &numchar, sizeof(numchar)) == -1)
			return (EFAULT);
		if (numchar > MAX_ROM_CHAR)
			return (EINVAL);
		if (numchar == 0)
			return (kdv_release_fontmap());
		size = sizeof(numchar) + numchar*sizeof(struct char_def);
		newbuf = (caddr_t) kmem_alloc(size,KM_SLEEP);
		if (newbuf == NULL)
			return (ENOMEM);
		if (copyin((caddr_t) arg, newbuf, size) == -1)
			return (EFAULT);
		rv = kdv_modromfont(newbuf,numchar);
		break;
	}
#ifdef EVGA
	case KDEVGA:
		if ( ! DTYPE(Kdws, KD_VGA)) {
			/* card has already been successfully 
			 * identified; can't be evga.
			 */
			rv = EINVAL;
		}
		else	{
			/* Card was a vanilla-type VGA, could
			 * be an evga.
			 */
			rv = evga_init(arg);
		}
		break;
#endif /* EVGA */
	default:
		/* Mode change hits the default case. Cmd is new mode
		 * or'ed with MODESWITCH.
		 */
		switch (cmd & 0xffffff00) {
		case VTIOC:	/* VT ioctl */
			rv = kdvt_ioctl(chp, cmd, arg, crp, rvalp);
			break;
		case MODESWITCH:	/* UNIX mode switch ioctl */
			rv = kdvm_modeioctl(chp, cmd, crp);
			break;
#ifdef EVGA
		case EVGAIOC:	/* evga mode switch ioctl */
		    	if (evga_inited) {
				rv = evga_modeioctl(chp, cmd, crp);
		    	}
		    	else {
				rv = ENXIO;
		    	}
			break;
#endif
		default:
			rv = kdvm_xenixioctl(chp, cmd, arg, crp, rvalp, &hit);
			if (hit == 1)	/* Xenix ioctl */
				break;
		}
		break;
	}
	return(rv);
}

/*
 *
 */

kdvm_modeioctl(chp, cmd, crp)
channel_t	*chp;
int	cmd;
struct cred	*crp;
{
    int	rv = 0;
    channel_t *achp;
    struct proc *procp;
#ifdef EVGA
    int generic_mode;
#endif

    cmd &= ~O_MODESWITCH;
    cmd |= MODESWITCH;

#ifdef EVGA
	if (evga_inited) {
		generic_mode = 0;
	}
#endif

	/* if kd has been intialized for evga, then DTYPE is still KD_VGA so
	 * modes that require only DTYPE of KD_VGA should succeed. Requests
	 * for modes that require other DTYPEs or have additional requirements
	 * may fail.
	 */

	switch (cmd) {
	/* have to check for Xenix modes */
	case SW_ATT640:
		if (!(VTYPE(V750) && VSWITCH(ATTDISPLAY)) && !VTYPE(V600 | CAS2))
			rv = ENXIO;
		break;
	case SW_ENHB80x43:
	case SW_ENHC80x43:
		if (!DTYPE(Kdws, KD_EGA))
			rv = ENXIO;
		cmd -= OFFSET_80x43;
		break;
	case SW_VGAB40x25:
	case SW_VGAB80x25:
	case SW_VGAC40x25:
	case SW_VGAC80x25:
	case SW_VGAMONO80x25:
	case SW_VGA640x480C:
	case SW_VGA640x480E:
	case SW_VGA320x200:
		if (!DTYPE(Kdws, KD_VGA))
			rv = ENXIO;
		break;
	case SW_MCAMODE:
		if (!(DTYPE(Kdws, KD_MONO) || DTYPE(Kdws, KD_HERCULES)))
			rv = ENXIO;
		break;
	case SW_VDC640x400V:
		if (!VTYPE(V600))
			rv = ENXIO;
		break;
	case SW_VDC800x600E:
		if (!VTYPE(V600 | CAS2) || (Vdc.v_info.dsply != KD_MULTI_M && Vdc.v_info.dsply != KD_MULTI_C)) {	/* not VDC-600 or CAS-2 ? */
#ifdef EVGA
			if (evga_inited) {
				generic_mode = SW_GEN_800x600;
			} else {
#endif 
				rv = ENXIO;
#ifdef EVGA
			}
#endif
		}
		break;

	case SW_CG320_D:
	case SW_CG640_E:
	case SW_ENH_MONOAPA2:
	case SW_VGAMONOAPA:
	case SW_VGA_CG640:
	case SW_ENH_CG640:
	case SW_ENHB40x25:
	case SW_ENHC40x25:
	case SW_ENHB80x25:
	case SW_ENHC80x25:
	case SW_EGAMONO80x25:
		if (!(DTYPE(Kdws, KD_EGA) || DTYPE(Kdws, KD_VGA)))
			rv = ENXIO;
		break;
	case SW_CG640x350:
	case SW_EGAMONOAPA:
		if (!(DTYPE(Kdws, KD_EGA) || DTYPE(Kdws, KD_VGA)))
			rv = ENXIO;
		/* for all VGA and the VDC 750, switch from F to F*
		 * since we know we have enough memory
		 * For other EGA cards that we can't identify, keep your
		 * fingers crossed and hope that mode F works
		 */
		if (VTYPE(V750) || DTYPE(Kdws,KD_VGA))
			cmd += 2;
		break;
	case SW_B40x25:
	case SW_C40x25:
	case SW_B80x25:
	case SW_C80x25:
	case SW_BG320:
	case SW_CG320:
	case SW_BG640:
		if (DTYPE(Kdws, KD_MONO) || DTYPE(Kdws, KD_HERCULES))
			rv = ENXIO;
		break;
#ifdef	EVC
	case SW_EVC1024x768E:
		if (!VTYPE(VEVC) || (Vdc.v_info.dsply != KD_MULTI_M &&
Vdc.v_info.dsply != KD_MULTI_C)) { /* EVC-1 with hi-res monitor only */
#ifdef EVGA
			if (evga_inited) {
				generic_mode = SW_GEN_1024x768;
			} else {
#endif 
				rv = ENXIO;
#ifdef EVGA
			}
#endif
		}
		break;
	case SW_EVC1024x768D:
		if (!VTYPE(VEVC) || (Vdc.v_info.dsply != KD_MULTI_M &&
Vdc.v_info.dsply != KD_MULTI_C)) /* EVC-1 with hi-res monitor only */
                        rv = ENXIO;
		break;
	case SW_EVC640x480V:
		if (!VTYPE(VEVC))
                        rv = ENXIO;
		break;
#endif	/*EVC*/

#ifdef  EVGA
	/* temporary kludge for X server */

	case TEMPEVC1024x768E:
		if (evga_inited) {
			generic_mode = SW_GEN_1024x768;
		} else {
			rv = ENXIO;
		}
		break;
#endif /*EVGA*/

	default:
		rv = ENXIO;
		break;
	}

#ifdef EVGA
	if (evga_inited && generic_mode) {
		rv = evga_modeioctl(chp, generic_mode, crp);
		return(rv);
	}
#endif

    if (!rv) {
	achp = ws_activechan(&Kdws);
	if (!achp) {
		cmn_err(CE_WARN,"kdvm_modeioctl: Could not find active channel");
		return (ENXIO);
	}
	ws_mapavail(achp,&Kdws.w_map);
	drv_getparm (UPROCP,&procp);
	if ( (Kdws.w_map.m_procp != procp) && (!CHNFLAG(chp, CHN_XMAP) && CHNFLAG(chp, CHN_UMAP)))
		return(EIO);

	if (cmd == SW_MCAMODE) {
		/* Use bogus mode to force reset */
		chp->ch_vstate.v_dvmode = 0xff;
		kdv_setmode(chp, DM_EGAMONO80x25);
	} else
		kdv_setmode(chp, (unchar)(cmd & KDMODEMASK));
    }
    return(rv);
}

#ifdef EVGA

evga_modeioctl(chp, cmd, crp)
channel_t	*chp;
int	cmd;
struct cred	*crp;
{
    int	rv = 0;
    int i, gen_mode;
    int color, x, y;
    int newmode;
    channel_t *achp;
    struct proc *procp;
    struct at_disp_info *disp;

	gen_mode = (cmd & EVGAMODEMASK);

	color = 16; /* for most modes */

	switch (gen_mode) {

	case GEN_640x350:
		x = 640; 
		y = 350;
		break;
	case GEN_640x480:
		x = 640; 
		y = 480;
		break;
	case GEN_720x540:
		x = 720; 
		y = 540;
		break;
	case GEN_800x560:
		x = 800; 
		y = 560;
		break;
	case GEN_800x600:
		x = 800; 
		y = 600;
		break;
	case GEN_960x720:
		x = 960; 
		y = 720;
		break;
	case GEN_1024x768:
	case GEN_1024x768x2:
	case GEN_1024x768x4:
		x = 1024; 
		y = 768;
		switch (gen_mode) {
		case GEN_1024x768x2:
			color = 2;
			break;
		case GEN_1024x768x4:
			color = 4;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	/* Look for a match in disp_info, given the board type 	*/
	/* present, the resolution requested, and the number of */
	/* (implicit) colors requested.       			*/

	for (i = 0, disp = disp_info; i < evga_num_disp; i++, disp++) {
		if ((evga_type == disp->type) &&
		   (x == disp->xpix) && (y == disp->ypix) &&
		   (color == disp->colors)) { 
			/* Found a match */
			break;
		}
	}
	if (i >= evga_num_disp) { /* failure */

		rv = ENXIO;             
	}
		
    	if (!rv) {
		achp = ws_activechan(&Kdws);
		if (!achp) {
			cmn_err(CE_WARN,"evga_modeioctl: Could not find active channel");
			return (ENXIO);
		}
		ws_mapavail(achp,&Kdws.w_map);
		drv_getparm (UPROCP,&procp);
		if ( (Kdws.w_map.m_procp != procp) && (!CHNFLAG(chp, CHN_XMAP) && CHNFLAG(chp, CHN_UMAP))) {
			return(EIO);
		}

		/* Convert relative offset (from beginning of evga modes)
	 	* to absolute offset into kd_modeinfo.
	 	*/
		newmode = i + ENDNONEVGAMODE + 1;

		kdv_setmode(chp, (unchar)newmode);
    	}
    	return(rv);
}
#endif /* EVGA */

/*
 *
 */

kdvm_xenixioctl(chp, cmd, arg, crp, rvalp, ioc_hit)
channel_t	*chp;
int	cmd,
	arg;
struct cred	*crp;
int	*rvalp,
	*ioc_hit;
{
	int	tmp;
	int	rv = 0;

	switch (cmd) {
	case GIO_COLOR:
		*rvalp = kdv_colordisp();
		break;
	case CONS_CURRENT:
		*rvalp = kdv_xenixctlr();
		break;
	case MCA_GET:
	case CGA_GET:
	case EGA_GET:
	case VGA_GET:
	case CONS_GET:
		rv = kdv_xenixmode(chp, cmd, rvalp);
		break;
	case MAPMONO:
	case MAPCGA:
	case MAPEGA:
	case MAPVGA:
	case MAPSPECIAL:
	case MAPCONS:
		rv = kdvm_xenixmap(chp, cmd, rvalp);
		break;
	case MCAIO:
	case CGAIO:
	case EGAIO:
	case VGAIO:
	case CONSIO:
		rv = kdvm_xenixdoio(chp, cmd, arg);
		break;
	case SWAPMONO:
		if (DTYPE(Kdws, KD_MONO) || DTYPE(Kdws, KD_HERCULES))
			break;
		if (!DTYPE(Kdws, KD_EGA) && !DTYPE(Kdws, KD_VGA)) {
			rv = EINVAL;
			break;
		}
		switch (chp->ch_vstate.v_cvmode) {
		case DM_EGAMONO80x25:
		case DM_EGAMONOAPA:
		case DM_ENHMONOAPA2:
			break;
		default:
			rv = EINVAL;
		}
		break;
	case SWAPCGA:
		if (DTYPE(Kdws, KD_CGA))
			break;
		if (!DTYPE(Kdws, KD_EGA) && !DTYPE(Kdws, KD_VGA)) {
			rv = EINVAL;
			break;
		}
		switch (chp->ch_vstate.v_cvmode) {
		case DM_B40x25:
		case DM_C40x25:
		case DM_B80x25:
		case DM_C80x25:
		case DM_BG320:
		case DM_CG320:
		case DM_BG640:
		case DM_CG320_D:
		case DM_CG640_E:
			break;
		default:
			rv = EINVAL;
		}
		break;
	case SWAPEGA:
		if (!DTYPE(Kdws, KD_EGA) && !DTYPE(Kdws, KD_VGA)) {
			rv = EINVAL;	
			break;
		}
		/* fail for any VGA mode or non-standard EGA mode */
		if (chp->ch_vstate.v_cvmode >= DM_VGA_C40x25) 
			rv = EINVAL;
		break;
	case SWAPVGA:
		if (!DTYPE(Kdws, KD_VGA)) {
			rv = EINVAL;	
			break;
		}
		/* fail for these non-standard VGA modes */
		switch (chp->ch_vstate.v_cvmode) {
		case DM_ENH_CGA:
		case DM_ATT_640:
		case DM_VDC800x600E:
		case DM_VDC640x400V:
			rv = EINVAL;
			break;
		default:
			break;
		}
		break;

	case PIO_FONT8x8:
		tmp = FONT8x8;
		goto piofont;

	case PIO_FONT8x14:
		tmp = FONT8x14;
		goto piofont;

	case PIO_FONT8x16:
		tmp = FONT8x16;
piofont:
		if (!DTYPE(Kdws, KD_EGA) && !DTYPE(Kdws, KD_VGA))
			return(EINVAL);
		rv = kdv_setxenixfont(chp,tmp,arg);
		break;
	
	case GIO_FONT8x8:
		tmp = FONT8x8;
		goto giofont;

	case GIO_FONT8x14:
		tmp = FONT8x14;
		goto giofont;

	case GIO_FONT8x16:
		tmp = FONT8x16;
giofont:
		if (!DTYPE(Kdws, KD_EGA) && !DTYPE(Kdws, KD_VGA))
			return(EINVAL);
		rv = kdv_getxenixfont(chp, tmp, arg);
		break;

	case KDDISPINFO: {
		struct kd_dispinfo dinfo;

		switch (Kdws.w_vstate.v_type) {

		case KD_MONO:
		case KD_HERCULES:
			dinfo.vaddr = dinfo.physaddr = (caddr_t) MONO_BASE;
			dinfo.size = MONO_SIZE;
			break;
		case KD_CGA:
			dinfo.vaddr = dinfo.physaddr = (caddr_t) COLOR_BASE;
			dinfo.size = COLOR_SIZE;
			break;
		case KD_VGA:
		case KD_EGA: /* Assume as we do for MONOAPA2 that the EGA card
			      * has 128K of RAM. Still have the fingers crossed
			      * Not an issue for VGA -- we always have 128K */
			dinfo.vaddr = dinfo.physaddr = EGA_BASE;
			dinfo.size = EGA_LGSIZE;
			break;
		default:
			return (EINVAL);
		}

		if (copyout(&dinfo, arg, sizeof(struct kd_dispinfo)) < 0)
			rv = EFAULT;
		break;
	}

	case CONS_GETINFO: {
		struct vid_info vinfo;
		termstate_t *tsp;
		
		tsp = &chp->ch_tstate;
		vinfo.size = sizeof(struct vid_info);
		vinfo.m_num = chp->ch_id;
		vinfo.mv_row = tsp->t_row;
		vinfo.mv_col = tsp->t_col;
		vinfo.mv_rsz = tsp->t_rows;
		vinfo.mv_csz = tsp->t_cols;
		vinfo.mv_norm.fore = tsp->t_curattr & 0x07;
		vinfo.mv_norm.back = (tsp->t_curattr & 0x70) >> 4;
		vinfo.mv_rev.fore = 0; /* reverse always black on white */
		vinfo.mv_rev.back = 7;
		vinfo.mv_grfc.fore = 7; /* match graphics with background info */
		vinfo.mv_grfc.back = 0;
		vinfo.mv_ovscan =  chp->ch_vstate.v_border;
		vinfo.mk_keylock = chp->ch_kbstate.kb_state / CAPS_LOCK;
		if (copyout(&vinfo, arg, sizeof(struct vid_info)) < 0)
			return (EFAULT);

		break;
	}

	case CONS_6845INFO: {

		struct m6845_info minfo;

		minfo.size = sizeof(struct m6845_info);
		minfo.cursor_type = chp->ch_tstate.t_curtyp;
		minfo.screen_top = chp->ch_tstate.t_origin;
		if (copyout(&minfo, arg, sizeof(struct m6845_info)) < 0)
			return (EFAULT);

		break;
	}

	case EGA_IOPRIVL:
		if (!DTYPE(Kdws,KD_EGA))
			return(EINVAL);
		goto check;

	case VGA_IOPRIVL:
		if (!DTYPE(Kdws,KD_VGA))
			return(EINVAL);

check:		*rvalp = checkio(chp->ch_vstate.v_ioaddrs);
		break;

		
	case PGAIO:
	case PGAMODE:
	case MAPPGA:
	case PGA_GET:
	case SWAPPGA:
	case CGAMODE:
	case EGAMODE:
	case VGAMODE:
	case MCAMODE:
	case KIOCDOSMODE:
	case KIOCNONDOSMODE:
	case SPECIAL_IOPRIVL:
	case INTERNAL_VID:
	case EXTERNAL_VID:
		rv = EINVAL;
		break;
	default:
		*ioc_hit = 0;
		break;
	}
	return(rv);
}

/*
 *
 */

kdvm_mapdisp(chp, map_p, arg)
channel_t	*chp;
struct map_info	*map_p;
int	arg;
{
	struct kd_memloc	memloc;
	struct proc	*procp;

	if (chp != ws_activechan(&Kdws) || chp->ch_dmode != KD_GRAPHICS)
		return(EACCES);
	ws_mapavail(chp, map_p);
	if (copyin(arg, &memloc, sizeof(memloc)) < 0)
		return(EFAULT);
	drv_getparm(UPROCP, &procp);
	if (map_p->m_procp && map_p->m_procp != procp || map_p->m_cnt == CH_MAPMX)
		return(EBUSY);
	if (!kdvm_map(procp, chp, map_p, &memloc))
		return(EFAULT);
	return(0);
}

/*
 *
 */

int
kdvm_mapfunc(memp, off, prot)
struct kd_memloc	*memp;
register off_t		off;
int			prot;
{
 	if (off >= memp->length)
		return(-1);
	return(btop(memp->physaddr + off));
}


/*
 *
 */

kdvm_map(procp, chp, map_p, memp)
struct proc	*procp;
channel_t	*chp;
struct map_info	*map_p;
struct kd_memloc	*memp;
{
	register ulong	start, end;
	int	cnt;
	struct as	*as;
	struct segobjs_crargs	a;

	if (memp->length <= 0)
		return(0);
	/* check requested physical range for validity */
	start = (ulong)memp->physaddr;
	end = (ulong)memp->physaddr + memp->length;
	for (cnt = 0; cnt < kdvmemcnt + 1; cnt++) {
		if (start >= kd_basevmem[cnt].start && end <= kd_basevmem[cnt].end)
			break;
	}
	if (cnt == kdvmemcnt + 1)
		return(0);
	/* first free any pages user has in requested range */
	map_p->m_addr[map_p->m_cnt] = *memp;	/* struct copy */
	/* the address space must have been created by now; if not, PANIC */
	if ((as = procp->p_as) == NULL)
		cmn_err(CE_PANIC, "kdvm: no as allocated");
	/* first free any pages user has in requested range */
	if (as_unmap(as, memp->vaddr, memp->length))
		return(0);
	/* map in the virtual memory as a set of device pages. */
	a.mapfunc = kdvm_mapfunc;
	a.offset = 0;
	a.arg = (caddr_t)&map_p->m_addr[map_p->m_cnt];
	a.prot = PROT_USER|PROT_READ|PROT_WRITE;
	a.maxprot = PROT_ALL;
	if (as_map(as, memp->vaddr, memp->length, segobjs_create,
&a))
		return(0);
	if (memp->ioflg)
		enableio(chp->ch_vstate.v_ioaddrs);
	if (!map_p->m_cnt) {
		map_p->m_procp = procp;
		/* use procp so we don't have bad alignment on map_p->m_pid) */
		drv_getparm(PPID, &procp);
		map_p->m_pid = (pid_t) procp;
		map_p->m_chan = chp->ch_id;
		chp->ch_flags |= CHN_UMAP;
	}
	map_p->m_cnt++;
	return(1);
}

/*
 *
 */

kdvm_unmapdisp(chp, map_p)
channel_t	*chp;
struct map_info	*map_p;
{
	int	cnt;

	chp->ch_vstate.v_font = 0;	/* font munged */
	for (cnt = 0; cnt < map_p->m_cnt; cnt++) {
		as_unmap(map_p->m_procp->p_as,map_p->m_addr[cnt].vaddr,
			 map_p->m_addr[cnt].length);
		if (map_p->m_addr[cnt].ioflg)
			disableio(chp->ch_vstate.v_ioaddrs);
	}
	map_p->m_procp = (struct proc *)0;
	map_p->m_pid = (pid_t) 0;
	chp->ch_flags &= ~CHN_MAPPED;
	map_p->m_cnt = 0;
	map_p->m_chan = 0;
	return(1);
}

/*
 *
 */

kdvm_xenixmap(chp, cmd, rvalp)
channel_t	*chp;
int	cmd,
	*rvalp;
{
	struct as	*as;
	struct map_info	*map_p = &Kdws.w_map;
	struct kd_memloc	memloc;
	vidstate_t	*vp = &chp->ch_vstate;
	struct proc	*procp;

	switch (cmd) {
	case MAPMONO:
		if (!DTYPE(Kdws, KD_MONO) && !DTYPE(Kdws, KD_HERCULES))
			return(EINVAL);
		break;
	case MAPCGA:
		if (!DTYPE(Kdws, KD_CGA))
			return(EINVAL);
		break;
	case MAPEGA:
		if (!DTYPE(Kdws, KD_EGA) && !DTYPE(Kdws, KD_VGA))
			return(EINVAL);
		break;
	case MAPVGA:
		if (!DTYPE(Kdws, KD_VGA))
			return(EINVAL);
		break;
	case MAPSPECIAL:
		return (EINVAL);
	default:
		break;
	}
	if (chp != ws_activechan(&Kdws))
		return(EACCES);
	ws_mapavail(chp, map_p);
	drv_getparm(UPROCP, &procp);
	if (map_p->m_procp && map_p->m_procp != procp || map_p->m_cnt == CH_MAPMX)
		return(EBUSY);

	/*
	 * find the physical address and size of screen memory,
	 * and the virtual address to map it to.
	 */
	memloc.physaddr = (char *)WSCMODE(vp)->m_base;
	memloc.length = WSCMODE(vp)->m_size;
	(void)map_addr(&memloc.vaddr, memloc.length, 0, 0);
	if (!memloc.vaddr)
		return(EFAULT);
	memloc.ioflg = 0;
	if (!kdvm_map(procp, chp, map_p, &memloc))
		return(EFAULT);
	chp->ch_flags |= CHN_XMAP;
	*rvalp = (int)memloc.vaddr;
	return(0);
}

/*
 *
 */

kdvm_xenixdoio(chp, cmd, arg)
channel_t	*chp;
int	cmd,
	arg;
{
	struct port_io_arg	portio;
	int	cnt, indone = 0;

	switch (cmd) {
	case MCAIO:
		if (!DTYPE(Kdws, KD_MONO) && !DTYPE(Kdws, KD_HERCULES))
			return(EINVAL);
		break;
	case CGAIO:
		if (!DTYPE(Kdws, KD_CGA))
			return(EINVAL);
		break;
	case EGAIO:
		if (!DTYPE(Kdws, KD_EGA) && !DTYPE(Kdws, KD_VGA))
			return(EINVAL);
		break;
	case VGAIO:
		if (!DTYPE(Kdws, KD_VGA))
			return(EINVAL);
		break;
	case CONSIO:
		break;
	default:
		return(EINVAL);
	}
	if (copyin(arg, &portio, sizeof(struct port_io_arg)) < 0)
		return(EFAULT);
	for (cnt = 0; cnt < 4 && portio.args[cnt].port; cnt++) {
		if (!ws_ck_kd_port(&chp->ch_vstate, portio.args[cnt].port))
			return(EINVAL);
		switch (portio.args[cnt].dir) {
		case IN_ON_PORT:
			portio.args[cnt].data = inb(portio.args[cnt].port);
			indone++;
			break;
		case OUT_ON_PORT:
			outb(portio.args[cnt].port, portio.args[cnt].data);
			break;
		default:
			return(EINVAL);
		}
	}
	if (indone && copyout(&portio, arg, sizeof(portio)) < 0)
		return(EFAULT);
	return(0);
}
