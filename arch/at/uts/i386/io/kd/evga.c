/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 *
 *	Copyright (c) 1989 by Interactive Systems Corporation. 
 *	All rights reserved.  Contains confidential information and
 *	trade secrets proprietary to
 *
 *		Interactive Systems Corporation
 *		2401 Colorado Avenue
 *		Santa Monica, California  90404
 */

#ident	"@(#)at:uts/i386/io/kd/evga.c	1.1.2.2"

#ifdef EVGA

#include "sys/types.h"
#include "sys/errno.h"
#include "sys/cmn_err.h"
#include "sys/at_ansi.h"
#include "sys/kd.h"
#include "sys/proc.h"
#include "sys/stream.h"
#include "sys/termios.h"
#include "sys/strtty.h"
#include "sys/stropts.h"
#include "sys/xque.h"
#include "sys/ws/ws.h"
#include "sys/vid.h"
#include "sys/vdc.h"
#include "sys/inline.h"

static int
no_ext()
{
}

int video7_init();
int video7_rest();
int tseng_init();
int tseng_rest();
int ati_init();
int ati_rest();
int pvga1024_init();
int pvga1024_rest();

extern struct b_param kd_inittab[];

struct	at_disp_info	disp_info[] = {	/* display info for support adapters */
	EVGA_VGA, VT_VGA, 1, 640, 480, 4, 16, 256*1024, 64*1024,
	80, no_ext, no_ext, &(kd_inittab[VT_VGA+STEVGA]),

	EVGA_VEGA, VT_VEGA720, 1, 720, 540, 4, 16, 256*1024, 64*1024,
	90, video7_init, video7_rest, &(kd_inittab[VT_VEGA720+STEVGA]),

	EVGA_VEGA, VT_VEGA800, 1, 800, 600, 4, 16, 256*1024, 64*1024,
	100, video7_init, video7_rest, &(kd_inittab[VT_VEGA800+STEVGA]),

	EVGA_STBGA, VT_TSL8005_16, 1, 800, 560, 4, 16, 256*1024, 64*1024,
	100, tseng_init, tseng_rest, &(kd_inittab[VT_TSL8005_16+STEVGA]),

	EVGA_STBGA, VT_TSL8006_16, 1, 800, 600, 4, 16, 256*1024, 64*1024,
	100, tseng_init, tseng_rest, &(kd_inittab[VT_TSL8006_16+STEVGA]),

	EVGA_STBGA, VT_TSL960, 1, 960, 720, 4, 16, 512*1024, 128*1024,
	120, tseng_init, tseng_rest, &(kd_inittab[VT_TSL960+STEVGA]),

	EVGA_STBGA, VT_TSL1024, 1, 1024, 768, 4, 16, 512*1024, 128*1024,
	128, tseng_init, tseng_rest, &(kd_inittab[VT_TSL1024+STEVGA]),

	EVGA_SIGMAH, VT_TSL8005_16, 1, 800, 560, 4, 16, 256*1024, 64*1024,
	100, tseng_init, tseng_rest, &(kd_inittab[VT_TSL8005_16+STEVGA]),

	EVGA_SIGMAH, VT_TSL8006_16, 1, 800, 600, 4, 16, 256*1024, 64*1024,
	100, tseng_init, tseng_rest, &(kd_inittab[VT_TSL8006_16+STEVGA]),

	EVGA_PVGA1A, VT_PVGA1A, 1, 800, 600, 4, 16, 256*1024, 64*1024,
	100, no_ext, no_ext, &(kd_inittab[VT_PVGA1A+STEVGA]),

	EVGA_DELL, VT_DELL7, 1, 720, 540, 4, 16, 256*1024, 64*1024,
	90, video7_init, video7_rest, &(kd_inittab[VT_DELL7+STEVGA]),

	EVGA_DELL, VT_DELL8, 1, 800, 600, 4, 16, 256*1024, 64*1024,
	100, video7_init, video7_rest, &(kd_inittab[VT_DELL8+STEVGA]),

	EVGA_VRAM, VT_V7VRAM6, 1, 640, 480, 4, 16, 256*1024, 64*1024,
	80, video7_init, video7_rest, &(kd_inittab[VT_V7VRAM6+STEVGA]),

	EVGA_VRAM, VT_V7VRAM7, 1, 720, 540, 4, 16, 256*1024, 64*1024,
	90, video7_init, video7_rest, &(kd_inittab[VT_V7VRAM7+STEVGA]),

	EVGA_VRAM, VT_V7VRAM8, 1, 800, 600, 4, 16, 256*1024, 64*1024,
	100, video7_init, video7_rest, &(kd_inittab[VT_V7VRAM8+STEVGA]),

	EVGA_VRAM, VT_V7VRAM1_2, 1, 1024, 768, 1, 2, 256*1024, 128*1024,
	128, video7_init,video7_rest,&(kd_inittab[VT_V7VRAM1_2+STEVGA]),

	EVGA_VRAM, VT_V7VRAM1_4, 1, 1024, 768, 2, 4, 256*1024, 128*1024,
	128, video7_init,video7_rest,&(kd_inittab[VT_V7VRAM1_4+STEVGA]),

	EVGA_VRAM, VT_V7VRAM1_16, 1, 1024, 768, 4, 16, 512*1024, 128*1024,
	128, video7_init, video7_rest, &(kd_inittab[VT_V7VRAM1_16+STEVGA]),

	EVGA_ORVGA, /*VT_ORVGA8*/ VT_TSL8006_16, 1, 800, 600, 4, 16, 256*1024, 64*1024,
	100, tseng_init, tseng_rest, &(kd_inittab[VT_TSL8006_16+STEVGA]),

	EVGA_ORVGA, VT_TSL1024, 1, 1024, 768, 4, 16, 512*1024, 128*1024,
	128, tseng_init, tseng_rest, &(kd_inittab[VT_TSL1024+STEVGA]),

	EVGA_ORVGAni, VT_TSL1024ni, 1, 1024, 768, 4, 16, 512*1024, 128*1024,
	128, tseng_init, tseng_rest, &(kd_inittab[VT_TSL1024ni+STEVGA]),

	EVGA_TVGA, VT_TSL8005_16, 1, 800, 560, 4, 16, 256*1024, 64*1024,
	100, tseng_init, tseng_rest, &(kd_inittab[VT_TSL8005_16+STEVGA]),

	EVGA_TVGA, VT_TSL8006_16, 1, 800, 600, 4, 16, 256*1024, 64*1024,
	100, tseng_init, tseng_rest, &(kd_inittab[VT_TSL8006_16+STEVGA]),

	EVGA_TVGA, VT_TSL1024, 1, 1024, 768, 4, 16, 512*1024, 128*1024,
	128, tseng_init, tseng_rest, &(kd_inittab[VT_TSL1024+STEVGA]),

	EVGA_TVGAni, VT_TSL1024ni, 1, 1024, 768, 4, 16, 512*1024, 128*1024,
	128, tseng_init, tseng_rest, &(kd_inittab[VT_TSL1024ni+STEVGA]),

	EVGA_GVGA, VT_GVGA8_6, 1, 0 /*800*/, 0 /*600*/, 4, 16, 256*1024, 64*1024,
	100, tseng_init, tseng_rest, &(kd_inittab[VT_GVGA8_6+STEVGA]),

	EVGA_GVGA, VT_TSL1024, 1, 1024, 768, 4, 16, 512*1024, 128*1024,
	128, tseng_init, tseng_rest, &(kd_inittab[VT_TSL1024+STEVGA]),

	EVGA_EGA, VT_EGA, 0, 640, 350, 4, 16, 128*1024, 32*1024,
	80, no_ext, no_ext, &(kd_inittab[VT_EGA+STEVGA]),

	EVGA_PEGA, VT_PEGA, 0, 640, 480, 4, 16, 256*1024, 64*1024,
	80, no_ext, no_ext, &(kd_inittab[VT_PEGA+STEVGA]),

	EVGA_GEGA, VT_GENEGA_6, 0, 640, 480, 4, 16, 256*1024, 64*1024,
	80, no_ext, no_ext, &(kd_inittab[VT_GENEGA_6+STEVGA]),

	EVGA_GEGA, VT_GENEGA_8, 0, 800, 600, 4, 16, 256*1024, 64*1024,
	100, no_ext, no_ext, &(kd_inittab[VT_GENEGA_8+STEVGA]),

	EVGA_FASTWRITE, VT_V7FW6, 1, 640, 480, 4, 16, 256*1024, 64*1024,
	80, video7_init, video7_rest, &(kd_inittab[VT_V7VRAM6+STEVGA]),

	EVGA_FASTWRITE, VT_V7FW7, 1, 720, 540, 4, 16, 256*1024, 64*1024,
	90, video7_init, video7_rest, &(kd_inittab[VT_V7VRAM7+STEVGA]),

	EVGA_FASTWRITE, VT_V7FW8, 1, 800, 600, 4, 16, 256*1024, 64*1024,
	100, video7_init, video7_rest, &(kd_inittab[VT_V7VRAM8+STEVGA]),

	EVGA_FASTWRITE, VT_V7FW1_2, 1, 1024, 768, 1, 2, 256*1024, 128*1024,
	128, video7_init,video7_rest,&(kd_inittab[VT_V7VRAM1_2+STEVGA]),

	EVGA_FASTWRITE, VT_V7FW1_4, 1, 1024, 768, 2, 4, 256*1024, 128*1024,
	128, video7_init,video7_rest,&(kd_inittab[VT_V7VRAM1_4+STEVGA]),

	EVGA_WON, VT_VGAWON, 1, 800, 600, 4, 16, 256*1024, 64*1024,
	100, ati_init, ati_rest, &(kd_inittab[VT_VGAWON+STEVGA]),

	EVGA_PVGA1024, VT_PVGA1024, 1, 1024, 768, 4, 16, 512*1024, 128*1024,
	128, pvga1024_init, pvga1024_rest, &(kd_inittab[VT_PVGA1024+STEVGA]),

	EVGA_PVGA1024, VT_PVGA1024_8, 1, 800, 600, 4, 16, 256*1024, 64*1024,
	100, no_ext, no_ext, &(kd_inittab[VT_PVGA1024_8+STEVGA]),
};

int	evga_num_disp = (sizeof(disp_info) / sizeof(struct at_disp_info));
static	int	max_planes;		/* maximum number of planes available */

int	evga_inited = 0;
int	cur_mode_is_evga = 0;
unsigned long	evga_type;

unchar	saved_misc_out;		 	/* need to save and restore this */
extern struct vdc_info	Vdc;
extern wstation_t	Kdws;
extern struct reginfo kd_regtab[];
extern struct modeinfo kd_modeinfo[];

#define EBSE	EGA_BASE
#define GRAPH	0
#define TBLE	KD_TBLE

/*
 * Evga_init sees if the requested board is supported. It also
 * sets up Vdc for the board, so applications can find out if an
 * EVGA board is present via the KDVDCTYPE ioctl.
 *
 *	Returns:
 *		0 if successful, non-zero otherwise.
 */
evga_init(arg)
int arg;
{
	int	i;
	struct	at_disp_info *disp;

	if (copyin(arg, &evga_type, sizeof(evga_type)) < 0) {
		return(EFAULT);
	}

	for (i = 0, disp = disp_info; i < evga_num_disp; i++, disp++) {
		if (evga_type == disp->type) {

			/* evga_inited allows evga mode set ioctls 
			 * to be accepted.
			 */
			evga_inited = 1;
			break;
		}
	}

	if (! evga_inited) {		/* No match */
		return(EINVAL);
	} else {

		/* Reset some state */

		Vdc.v_type = VEVGA;
		Vdc.v_info.cntlr = KD_EVGA;
	
		if (disp_info[i].is_vga && 
		((inb(MISC_OUT_READ) & IO_ADDR_SEL) == 0)) {

			/* Assume all evga are multi */
			Vdc.v_info.dsply = KD_MULTI_M;
		}
		else	{

			/* Assume all evga are multi */
			Vdc.v_info.dsply = KD_MULTI_C;
		}
	}

	return(0);
}

evga_ext_rest(oldmode) 
unchar	oldmode;
{
	int i;

	if (evga_inited) {
	    if (oldmode > ENDNONEVGAMODE) {
		/* If previous mode was an evga mode, restore extended
	         * registers and misc register.
		 */
		out_reg(&kd_regtab[I_SEQ], 0, SEQ_RST);
		outb(MISC_OUT, saved_misc_out);
		out_reg(&kd_regtab[I_SEQ], 0, SEQ_RUN);

		i = oldmode - (ENDNONEVGAMODE + 1);
		(*disp_info[i].ext_rest)(disp_info[i].vt_type, oldmode);


	    } 
	    /* Save misc register in case new mode is evga.
	     */

	    saved_misc_out = inb(MISC_OUT_READ);
	}
}

evga_ext_init(newmode) 
unchar	newmode;
{
	int i;

	if (evga_inited) {
	    if (newmode > ENDNONEVGAMODE) {

		/* Save extended register values and initialize 
		 * extended registers that the new mode uses to
		 * the appropriate values.
		 */

		i = newmode - (ENDNONEVGAMODE + 1);

		(*disp_info[i].ext_init)(disp_info[i].vt_type, newmode);

		/* Reset dsply. The rest of Vdc was set in evga_init
		 * and doesn't change. Update m_color. v_regaddr and
		 * v_undattr get set in kdv_rst depending on m_color;
		 * don't need to set them here.
		 */

		if (disp_info[i].is_vga && 
		((inb(MISC_OUT_READ) & IO_ADDR_SEL) == 0)) { 
			disp_info[i].regs->miscreg &= ~IO_ADDR_SEL;
			kd_modeinfo[newmode].m_color = 0;

			/* Assume all evga are multi */
			Vdc.v_info.dsply = KD_MULTI_M;
		}
		else	{
			kd_modeinfo[newmode].m_color = 1;

			/* Assume all evga are multi */
			Vdc.v_info.dsply = KD_MULTI_C;
		}

		/* new (current) mode is evga.
		 * Need to remember this because when a KDSETMODE
		 * is done the current mode index (v_cvmode) is overwritten
		 * before kdvm_ioctl is called. This is needed so
		 * that extended registers may be restored correctly.
		 */
		cur_mode_is_evga = newmode;
	    } else {
		/* new (current) mode is not evga. */
		cur_mode_is_evga = 0;
	    }
	}
}



static unchar bandwidth;
static unchar clock;
static unchar clock_ext;
static unchar timing;
static unchar exten;
static unchar compat;
static unchar bank_sel;
/*
 *	video7_init(mode)	-- initialize a Video Seven Vega VGA board to
 *				one of it's "extended" modes.  This takes care
 *				of non-standard VGA registers.
 *
 *	Input:
 *		int	mode	-- display mode being used
 */
video7_init(mode, mode_info_index)
int mode;
unchar mode_info_index;
{
	static int inited = 0;

	out_reg(&kd_regtab[I_SEQ], 0, SEQ_RST);		/* reset sequencer */
	out_reg(&kd_regtab[I_SEQ], 6, 0xea);		/* enable extensions */

	if (!inited) {
		inited = 1;

		switch(mode) {
		case VT_VEGA720:
		case VT_VEGA800:
			/*
			 * We write a 0 to SEQ reg 0x80 because of
			 * of a bug in one chip rev that makes this
			 * sometimes necessary.  
			 */
			out_reg(&kd_regtab[I_SEQ], 0x80, 0);

			in_reg(&kd_regtab[I_SEQ], 0x86, bandwidth);
			in_reg(&kd_regtab[I_SEQ], 0xa4, clock);
			break;

		case VT_V7VRAM6:
		case VT_V7VRAM7:
		case VT_V7VRAM8:
		case VT_V7VRAM1_2:
		case VT_V7VRAM1_4:
		case VT_V7VRAM1_16:
		case VT_V7FW6:
		case VT_V7FW7:
		case VT_V7FW8:
		case VT_V7FW1_2:
		case VT_V7FW1_4:
		case VT_DELL7:
		case VT_DELL8:
			in_reg(&kd_regtab[I_SEQ], 0xfd, timing);
			in_reg(&kd_regtab[I_SEQ], 0xa4, clock);
			in_reg(&kd_regtab[I_SEQ], 0xf8, clock_ext);
			in_reg(&kd_regtab[I_SEQ], 0xfc, compat);
			in_reg(&kd_regtab[I_SEQ], 0xff, exten);
			in_reg(&kd_regtab[I_SEQ], 0xf6, bank_sel);
			break;
		}
	}

	switch(mode) {
	case VT_VEGA720:
	case VT_VEGA800:
		out_reg(&kd_regtab[I_SEQ], 0x86, 0x30);	/* set bandwidth */
		out_reg(&kd_regtab[I_SEQ], 0xa4, 0x0c);	/* set clock */
		break;

	case VT_V7FW6:
	case VT_V7VRAM6:
		break;

	case VT_V7VRAM7:
	case VT_V7FW7:
	case VT_DELL7:
		out_reg(&kd_regtab[I_SEQ], 0xfd, 0x00);	/* set timing */
		out_reg(&kd_regtab[I_SEQ], 0xa4, 0x10);	/* set clock */
		break;

	case VT_V7FW8:
	case VT_DELL8:
		out_reg(&kd_regtab[I_SEQ], 0xfd, 0x30);	/* set timing */
		out_reg(&kd_regtab[I_SEQ], 0xa4, 0x10);	/* set clock */
		out_reg(&kd_regtab[I_SEQ], 0xf8, 0x10);	/* set ext. clock */
		break;

	case VT_V7VRAM8:
		out_reg(&kd_regtab[I_SEQ], 0xfd, 0x90);	/* set timing */
		out_reg(&kd_regtab[I_SEQ], 0xa4, 0x10);	/* set clock */
		out_reg(&kd_regtab[I_SEQ], 0xf8, 0x10);	/* set ext. clock */
		break;
		
	case VT_V7FW1_2:
	case VT_V7FW1_4:
		out_reg(&kd_regtab[I_SEQ], 0xfd, 0x30);	/* set timing */
		out_reg(&kd_regtab[I_SEQ], 0xa4, 0x10);	/* set clock */
		out_reg(&kd_regtab[I_SEQ], 0xfc, 0x18);	/* set compat */
		break;

	case VT_V7VRAM1_2:
	case VT_V7VRAM1_4:
		out_reg(&kd_regtab[I_SEQ], 0xfd, 0xa0);	/* set timing */
		out_reg(&kd_regtab[I_SEQ], 0xa4, 0x10);	/* set clock */
		out_reg(&kd_regtab[I_SEQ], 0xfc, 0x18);	/* set compat */
		break;

	case VT_V7VRAM1_16:
		out_reg(&kd_regtab[I_SEQ], 0xfd, 0xa0);	/* set timing */
		out_reg(&kd_regtab[I_SEQ], 0xff, 0x10);	/* set extensions */
		out_reg(&kd_regtab[I_SEQ], 0xa4, 0x10);	/* set clock */
		out_reg(&kd_regtab[I_SEQ], 0xf6, 0xc0);	/* set bank sel */
		break;
	}

	out_reg(&kd_regtab[I_SEQ], 0, SEQ_RUN);		/* start sequencer */
}
	

/*
 *	video7_rest(mode)	-- restore a Video Seven Vega VGA board from
 *				one of it's "extended" modes.  This takes care
 *				of non-standard VGA registers.
 *
 *	Input:
 *		int	mode	-- display mode being used
 */
video7_rest(mode, mode_info_index)
int mode;
unchar mode_info_index;
{
	out_reg(&kd_regtab[I_SEQ], 0, SEQ_RST);		/* reset sequencer */
	out_reg(&kd_regtab[I_SEQ], 6, 0xea);		/* enable extensions */

	switch(mode) {
	case VT_VEGA720:
	case VT_VEGA800:
		out_reg(&kd_regtab[I_SEQ], 0x86, bandwidth);/* set bandwidth */
		out_reg(&kd_regtab[I_SEQ], 0xa4, clock);	/* set clock */
		break;

	case VT_V7VRAM6:
	case VT_V7VRAM7:
	case VT_V7VRAM8:
	case VT_V7VRAM1_2:
	case VT_V7VRAM1_4:
	case VT_V7VRAM1_16:
	case VT_V7FW6:
	case VT_V7FW7:
	case VT_V7FW8:
	case VT_V7FW1_2:
	case VT_V7FW1_4:
	case VT_DELL7:
	case VT_DELL8:
		out_reg(&kd_regtab[I_SEQ], 0xfd, timing);	/* set timing */
		out_reg(&kd_regtab[I_SEQ], 0xa4, clock);	/* set clock */
		out_reg(&kd_regtab[I_SEQ], 0xf8, clock_ext);/* set ext. clock */
		out_reg(&kd_regtab[I_SEQ], 0xfc, compat);	/* set compat */
		out_reg(&kd_regtab[I_SEQ], 0xff, exten);	/* set extensions */
		out_reg(&kd_regtab[I_SEQ], 0xf6, bank_sel);/* set bank sel */
	}

	out_reg(&kd_regtab[I_SEQ], 6, 0xae);		/* disable extensions */
	out_reg(&kd_regtab[I_SEQ], 0, SEQ_RUN);		/* start sequencer */
}


static unchar tseng_seq_aux;
static unchar tseng_crt_misc;
static unchar tseng_attr_misc;
static unchar tseng_gdc_select;
static unchar sigma_digital;
/*
 *	tseng_init(mode)	-- initialize a Tseng Labs VGA board to one
 *				of it's "extended" modes.  This takes care
 *				of non-standard VGA registers.
 *
 *	Input:
 *		int	mode	-- display mode being used
 */
tseng_init(mode, mode_info_index)
int mode;
unchar mode_info_index;
{
	static int inited = 0;
	int color;
	int i;
	ushort regaddr;
	unchar misc_out;

	if ((inb(MISC_OUT_READ) & IO_ADDR_SEL) == 0) {
		color = 0;
		regaddr = MONO_REGBASE;
	} else {
		color = 1;
		regaddr = COLOR_REGBASE;
	}

	out_reg(&kd_regtab[I_SEQ], 0, SEQ_RST);		/* reset sequencer */
	if (!inited) {
		tseng_gdc_select = inb(0x3cd);
		in_reg(&kd_regtab[I_SEQ], 0x7, tseng_seq_aux);
		(void)inb(regaddr + IN_STAT_1); /* init flip-flop */
		in_reg(&kd_regtab[I_ATTR+1], 0x16, tseng_attr_misc);
		outb(kd_regtab[I_ATTR].ri_address, PALETTE_ENABLE);
		if (color) {
			in_reg(&kd_regtab[I_EGACOLOR], 0x25, tseng_crt_misc);
		}
		else {
			in_reg(&kd_regtab[I_EGAMONO], 0x25, tseng_crt_misc);
		}
		inited = 1;
	}

	switch (mode) {
	case VT_TSL1024:
		outb(0x3cd, 0);				/* 128k segment */


		if (color) {
			out_reg(&kd_regtab[I_EGACOLOR], 0x25, 0x80);
		}
		else {
			out_reg(&kd_regtab[I_EGAMONO], 0x25, 0x80);
		}
		/* FALL THROUGH */

	case VT_TSL960:
		out_reg(&kd_regtab[I_SEQ], 0x7, 0xc8);
		(void)inb(regaddr + IN_STAT_1); /* init flip-flop */
		out_reg(&kd_regtab[I_ATTR], 0x16, 0x10);

		out_reg(&kd_regtab[I_SEQ], 0, SEQ_RUN);	/* start sequencer */
		return;

	case VT_TSL1024ni:
		outb(0x3cd, 0);				/* 128k segment */
		out_reg(&kd_regtab[I_SEQ], 0x7, 0xe8);
		(void)inb(regaddr + IN_STAT_1); /* init flip-flop */
		out_reg(&kd_regtab[I_ATTR], 0x16, 0x10);
		break;

	case VT_ORVGA8:
		out_reg(&kd_regtab[I_SEQ], 0x7, 0xa8);
		break;

	case VT_SIGMAH:
		out_reg(&kd_regtab[I_SEQ], 0x7, 0xc8);
		break;

	case VT_GVGA8_6:
		out_reg(&kd_regtab[I_SEQ], 0x7, 0x88);
		break;

	default:
		out_reg(&kd_regtab[I_SEQ], 0x7, 0x88);
		out_reg(&kd_regtab[I_SEQ], 0, SEQ_RUN);	/* start sequencer */
		return;
	}

	i = mode_info_index - (ENDNONEVGAMODE + 1);

	/* 
	 * set external clock
	 */
	misc_out = inb(MISC_OUT_READ);
	outb(MISC_OUT, misc_out & ~IO_ADDR_SEL);
	outb(0x3bf, 0x02);
	outb(MISC_OUT, misc_out);
	out_reg(&kd_regtab[I_SEQ], 0, SEQ_RUN);	/* start sequencer */
	out_reg(&kd_regtab[I_SEQ], 0, SEQ_RST);	/* reset sequencer */
	if (color) {
		outb(0x3d8, 0xa0);
		in_reg(&kd_regtab[I_EGACOLOR], 0x24, sigma_digital);
		out_reg(&kd_regtab[I_EGACOLOR], 0x24, sigma_digital|0x02);
	}
	else {
		outb(0x3b8, 0xa0);
		in_reg(&kd_regtab[I_EGAMONO], 0x24, sigma_digital);
		out_reg(&kd_regtab[I_EGAMONO], 0x24, sigma_digital|0x02);
	}
	out_reg(&kd_regtab[I_SEQ], 0, SEQ_RUN);		/* start sequencer */
}
	

/*
 *	tseng_rest(mode)  	-- restore a Tseng Labs VGA board from one
 *			  	of it's "extended" modes.  This takes care
 *			  	of non-standard VGA registers.
 *
 *	Input:
 *		int	mode	-- display mode being used
 */
tseng_rest(mode, mode_info_index)
int mode;
unchar mode_info_index;
{
	int color;
	int i;
	ushort regaddr;
	unchar misc_out;

	if ((inb(MISC_OUT_READ) & IO_ADDR_SEL) == 0) {
		color = 0;
		regaddr = MONO_REGBASE;
	} else {
		color = 1;
		regaddr = COLOR_REGBASE;
	}

	out_reg(&kd_regtab[I_SEQ], 0, SEQ_RST);		/* reset sequencer */
	out_reg(&kd_regtab[I_SEQ], 0x7, tseng_seq_aux);
	outb(0x3cd, tseng_gdc_select);			/* 128k segment */
	(void)inb(regaddr + IN_STAT_1); /* init flip-flop */
	out_reg(&kd_regtab[I_ATTR], 0x16, tseng_attr_misc);
	outb(kd_regtab[I_ATTR].ri_address, PALETTE_ENABLE);
	if (color) {
		out_reg(&kd_regtab[I_EGACOLOR], 0x25, tseng_crt_misc);
	}
	else {
		out_reg(&kd_regtab[I_EGAMONO], 0x25, tseng_crt_misc);
	}

	if ((mode == VT_SIGMAH) || (mode == VT_TSL1024ni) ||
	    (mode == VT_ORVGA8) || (mode == VT_GVGA8_6)) {

		i = mode_info_index - (ENDNONEVGAMODE + 1);
		misc_out = inb(MISC_OUT_READ);
		outb(MISC_OUT, misc_out & ~IO_ADDR_SEL);
		outb(0x3bf, 0x02);
		outb(MISC_OUT, misc_out);
		if (color) {
			outb(0x3d8, 0xa0);
			out_reg(&kd_regtab[I_EGACOLOR], 0x24, sigma_digital);
		}
		else {
			outb(0x3b8, 0xa0);
			out_reg(&kd_regtab[I_EGAMONO], 0x24, sigma_digital);
		}
	}
	out_reg(&kd_regtab[I_SEQ], 0, SEQ_RUN);		/* start sequencer */
}




static unchar	ati0;
static unchar	ati2;
static unchar	ati8;
/*
 *	ati_init(mode)	-- initialize an ATI VGA Wonder board into
 *			one of it's "extended" modes.  This takes care
 *			of non-standard VGA registers.
 *
 *	Input:
 *		int	mode	-- display mode being used
 */
ati_init(mode, mode_info_index)
int mode;
unchar mode_info_index;
{
	static int inited = 0;

	out_reg(&kd_regtab[I_SEQ], 0, SEQ_RST);		/* reset sequencer */


	if (!inited) {
		outb(0x1ce, 0xb8);
		ati8 = inb(0x1cf);

		outb(0x1ce, 0xb2);
		ati2 = inb(0x1cf);

		outb(0x1ce, 0xb0);
		ati0 = inb(0x1cf);
		inited = 1;
	}

	outb(0x1ce, 0xb2);
	outb(0x1cf, ati2 | 0x40);

	outb(0x1ce, 0xb0);
	outb(0x1cf, 0x08);

	outb(0x1ce, 0xb8);
	outb(0x1cf, ati8 & 0x3f);

	out_reg(&kd_regtab[I_SEQ], 0, SEQ_RUN);		/* start sequencer */
}
	
/*
 *	ati_rest(mode)	-- restore an ATI VGA Wonder board from
 *			one of it's "extended" modes.  This takes care
 *			of non-standard VGA registers.
 *
 *	Input:
 *		int	mode	-- display mode being used
 */
ati_rest(mode, mode_info_index)
int mode;
unchar mode_info_index;
{

	out_reg(&kd_regtab[I_SEQ], 0, SEQ_RST);		/* reset sequencer */

	outb(0x1ce, 0xb8);
	outb(0x1cf, ati8);

	outb(0x1ce, 0xb2);
	outb(0x1cf, ati2);

	outb(0x1ce, 0xb0);
	outb(0x1cf, ati0);

	out_reg(&kd_regtab[I_SEQ], 0, SEQ_RUN);		/* start sequencer */
}




static unchar	pr1;
static unchar	pr2;
static unchar	pr4;
static unchar	pr5;

static unchar	pr11;
static unchar	pr13;
static unchar	pr14;
static unchar	pr15;
static unchar	pr16;

/*
 *	pvga1024_init(mode)	-- initialize a Western Digital Paradise
 *			VGA 1024 board into one of it's "extended" modes.
 *			This takes care of non-standard VGA registers.
 *                      (1024x768)
 *
 *	Input:
 *		int	mode	-- display mode being used
 */
pvga1024_init(mode, mode_info_index)
int mode;
unchar mode_info_index;
{
	static int inited = 0;
	struct reginfo *pvga_ptr;


	if ((inb(MISC_OUT_READ) & IO_ADDR_SEL) == 0) {
		pvga_ptr = &kd_regtab[I_EGAMONO];
	} else {
		pvga_ptr = &kd_regtab[I_EGACOLOR];
	}

	out_reg(&kd_regtab[I_SEQ], 0, SEQ_RST);		/* reset sequencer */
	if (!inited) {

		in_reg(&kd_regtab[I_GRAPH], 0x0b, pr1);
		in_reg(&kd_regtab[I_GRAPH], 0x0c, pr2);
		in_reg(&kd_regtab[I_GRAPH], 0x0e, pr4);
		in_reg(&kd_regtab[I_GRAPH], 0x0f, pr5);

		out_reg(pvga_ptr, 0x29, 0x85); /* unlock PR10-PR17 */
		in_reg(pvga_ptr, 0x2a, pr11);
		in_reg(pvga_ptr, 0x2c, pr13);
		in_reg(pvga_ptr, 0x2d, pr14);
		in_reg(pvga_ptr, 0x2e, pr15);
		in_reg(pvga_ptr, 0x2f, pr16);

		inited = 1;
	}

	out_reg(&kd_regtab[I_GRAPH], 0x0f, 0x5);   /* write enable PR0-PR4 */
	out_reg(&kd_regtab[I_GRAPH], 0x0b, pr1 | 0x80);
	out_reg(&kd_regtab[I_GRAPH], 0x0c, 0x0);
	out_reg(&kd_regtab[I_GRAPH], 0x0e, pr4 | 0x80);

	out_reg(pvga_ptr, 0x29, 0x85); /* unlock PR10-PR17 */
	out_reg(pvga_ptr, 0x2a, 0x00);
	out_reg(pvga_ptr, 0x2c, 0x34);
	out_reg(pvga_ptr, 0x2d, 0x2a);
	out_reg(pvga_ptr, 0x2e, 0x1b);
	out_reg(pvga_ptr, 0x2f, 0x00);

	out_reg(&kd_regtab[I_SEQ], 0, SEQ_RUN);		/* start sequencer */
}
	
/*
 *	pvga1024_rest(mode)	-- initialize a Western Digital Paradise
 *			VGA 1024 board into one of it's "extended" modes.
 *			This takes care of non-standard VGA registers.
 *                      (1024x768)
 *
 *	Input:
 *		int	mode	-- display mode being used
 */
pvga1024_rest(mode, mode_info_index)
int mode;
unchar mode_info_index;
{
	struct reginfo *pvga_ptr;

	if ((inb(MISC_OUT_READ) & IO_ADDR_SEL) == 0) {
		pvga_ptr = &kd_regtab[I_EGAMONO];
	} else {
		pvga_ptr = &kd_regtab[I_EGACOLOR];
	}

	out_reg(&kd_regtab[I_SEQ], 0, SEQ_RST);		/* reset sequencer */

	out_reg(&kd_regtab[I_GRAPH], 0x0f, 0x5);  /* write enable PR0-PR4 */
	out_reg(&kd_regtab[I_GRAPH], 0x0b, pr1);
	out_reg(&kd_regtab[I_GRAPH], 0x0c, pr2);
	out_reg(&kd_regtab[I_GRAPH], 0x0e, pr4);

	/* restore lock state of PR0 - PR4 */
	out_reg(&kd_regtab[I_GRAPH], 0x0f, pr5); 

	out_reg(pvga_ptr, 0x29, 0x85); /* unlock PR10-PR17 */
	out_reg(pvga_ptr, 0x2a, pr11);
	out_reg(pvga_ptr, 0x2c, pr13);
	out_reg(pvga_ptr, 0x2d, pr14);
	out_reg(pvga_ptr, 0x2e, pr15);
	out_reg(pvga_ptr, 0x2f, pr16);

	out_reg(&kd_regtab[I_SEQ], 0, SEQ_RUN);		/* start sequencer */
}

#endif	/*EVGA*/
