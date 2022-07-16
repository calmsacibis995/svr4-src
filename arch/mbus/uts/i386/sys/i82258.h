/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_I82258_H
#define _SYS_I82258_H

/*
 *	INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *	This software is supplied under the terms of a license agreement
 *	or nondisclosure agreement with Intel Corporation and may not be
 *	copied nor disclosed except in accordance with the terms of that
 *	agreement.
 *
 *	Copyright 1987, 1988, 1989  Intel Corporation
*/

#ident	"@(#)mbus:uts/i386/sys/i82258.h	1.3.2.1"

/*
 * addresses for the 82258 D258 controller chip
 *
 */

/*
 * the following offsets must be added to the 82258 base address 
 */
#define D258_GCR     0x00
#define D258_SCR     0x02
#define D258_GSR     0x04
#define D258_GMR     0x08
#define D258_GBR     0x0A
#define D258_GDR     0x0C

/*
 * for the following functions the value returned is the
 * i/o address for channel "x" of the specified 82258 register
 */

#define D258_ADDR	0x40

#define D258_CSR(x)      ((x*D258_ADDR) + 0x10)
#define D258_DAR(x)      ((x*D258_ADDR) + 0x12)
#define D258_MASKR(x)    ((x*D258_ADDR) + 0x14)
#define D258_COMPR(x)    ((x*D258_ADDR) + 0x16)
#define D258_CPRL(x)     ((x*D258_ADDR) + 0x20)
#define D258_CPRH(x)     ((x*D258_ADDR) + 0x22)
#define D258_SPRL(x)     ((x*D258_ADDR) + 0x24)
#define D258_SPRH(x)     ((x*D258_ADDR) + 0x26)
#define D258_DPRL(x)     ((x*D258_ADDR) + 0x28)
#define D258_DPRH(x)     ((x*D258_ADDR) + 0x2a)
#define D258_TTPRL(x)    ((x*D258_ADDR) + 0x2c)
#define D258_TTPRH(x)    ((x*D258_ADDR) + 0x2e)
#define D258_LPRL(x)     ((x*D258_ADDR) + 0x30)
#define D258_LPRH(x)     ((x*D258_ADDR) + 0x32)
#define D258_BCRL(x)     ((x*D258_ADDR) + 0x38)
#define D258_BCRH(x)     ((x*D258_ADDR) + 0x3a)
#define D258_CCRL(x)     ((x*D258_ADDR) + 0x3c)
#define D258_CCRH(x)     ((x*D258_ADDR) + 0x3e)

#define START_CH(x)      ((0x10<<x)+2)
#define STOP_CH(x)       ((0x10<<x)+4)

struct d258_ccb		/* control block structure for 82258 */
{
	unsigned short	res;		/* reserved -- padding */
	unsigned short	command;	/* 82258 command */
	unsigned long	src_ptr;	/* 82258 source pointer */
	unsigned long	dst_ptr;	/* 82258 destination pointer */
	unsigned long	count;		/* transfer byte count */
	unsigned short	status;		/* status byte filled in by 82258 */
	unsigned char	stop[12];	/* unconditional stop */
};


#define D258_TYPE_IO		0x0
#define D258_TYPE_MEM   	0x1	
#define D258_STEP_HOLD		0x0
#define D258_STEP_DEC		0x2
#define D258_STEP_INC		0x4
#define D258_WB_8		0x0
#define D258_WB_16		0x8

/*
 * 82258 DMAC channel assignments
 */

#define MAX_CHAN	4	/* maximum number of 82258 channels */
#define D258_CHAN0	0	/* dma channel 0 */
#define D258_CHAN1	1	/* dma channel 1 */
#define D258_CHAN2	2	/* dma channel for solicited input */
#define D258_CHAN3	3	/* dma channel for solicited output */

/*
 * Base value definitions
 */
#ifdef MB2AT
#define MPC_BASE 0x400		/* only for the 386/PC16 board */
#else
#define MPC_BASE 0x0		/* only for 386/1xx board */
#endif


#define D258_IN1C_CMD   0x4600	
#define D258_IN2C_CMD   0x4200	
#define D258_OUT1C_CMD  0x8600
#define D258_OUT2C_CMD  0x8600

#define D258_SBX_CMD	   	0x0
#define D258_CHAN0_INT_MASK	0x4
#define D258_CHAN1_INT_MASK	0x40

#define DST_SHIFT		4

#define D258_GMR_1CYCLE 0x0010	/* 1 cycle mode */
#define D258_GMR_2CYCLE 0x0000	/* 2 cycle mode */


#define D258_BURST_ALGN 8	/* alignment requirement for blast mode */
#define D258_FLYBY_ALGN 4	/* alignment requirement for fly-by */

#define D258_BURST_SHIFT 	2
#define D258_FLYBY_SHIFT 	1
#define D258_2CYCLE_SHIFT 	0

#define D258_BURST_MODE   0x0
#define D258_FLYBY_MODE   0x1
#define D258_2CYCLE_MODE  0x2

#define BTODW 0x2	/* LOG(NBPW) = LOG(4) = 2 */

/*
 * Function definitions
 */
#ifdef __STDC__
/*
 * the prototype definitions permit only word aligned params. All char's and
 * shorts have been changed to ints
 */
extern void d258_init();
extern unsigned char d258_get_best_mode (struct dma_buf *);
extern int d258_dma_prog (struct dma_cb *, int);
extern int d258_dma_start (struct dma_cb *, int);
extern int d258_dma_stop (int);
extern int d258_intr (int);
extern int d258_dma_enable (int);
extern int d258_dma_disable (int);
extern int d258_get_chan_stat(struct dma_stat *, int);
extern void d258_restore_dmabuf (struct dma_cb *, unsigned int);
extern void d258_fixup_dmabuf (struct dma_cb *, unsigned int);
extern unsigned short d258_mk_cmd (unsigned int, unsigned int, unsigned int);
#else
extern void d258_init();
extern unsigned char d258_get_best_mode ();
extern int d258_dma_prog ();
extern int d258_dma_start ();
extern int d258_dma_stop ();
extern int d258_intr ();
extern int d258_dma_enable ();
extern int d258_dma_disable ();
extern int d258_get_chan_stat();
extern void d258_fixup ();
extern unsigned short d258_mk_cmd ();
extern void d258_restore_dmabuf ();
extern void d258_fixup_dmabuf ();
#endif

#endif	/* _SYS_I82258_H */
