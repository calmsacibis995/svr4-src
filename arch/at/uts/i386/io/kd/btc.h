/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)at:uts/i386/io/kd/btc.h	1.1"

#ifndef XBTB_H
#define	XBTB_H

/*
 *	Register definitions for Bell Technologies "Blit Express"
 *	Workstation Graphics Engine card.
 */

/* bits in config register */
#define	INVISIBLE		0x40	/* if 1, board is invisible   */
#define	UNIX			0xA0	/* if 1, use UNIX addressing  */
#define	RESET			0x10	/* reset 82786 chip           */
#define	DOS_3			0x08	/* DOS mode: select 64K of 1M */
#define	DOS_2			0x04	/*   ""		""            */
#define	DOS_1			0x02	/*   ""		""            */
#define	DOS_0			0x01	/*   ""		""            */

/* bits in diag register */
#define	LED_2			0x80	/* turn on diagnostic LED 2   */
#define	LED_1			0x40	/* turn on diagnostic LED 1   */
#define	LED_0			0x20	/* turn on diagnostic LED 0   */
#define	UNIX_4			0x10	/* select UNIX base address   */
#define	UNIX_3			0x08	/*	""	""            */
#define	UNIX_2			0x04	/*	""	""            */
#define	UNIX_1			0x02	/*	""	""            */
#define	UNIX_0			0x01	/*	""	""            */

/* size of each portion of memory onboard */
#define	BTB_MEM_SIZE		(1024 * 1024)	/* 1MB RAM onboard    */
#define	BTB_REG_SIZE		128		/* 128 bytes of regs  */

/* special defines for BLT */
#define	BTB_ROMOFF		BTB_MEM_SIZE+0x20000

/* special defines for CBLT */
#define	CBTB_VRAMSIZE		(2048 * 1024)	/* 2MB VRAM (max) */
#define	CBTB_ROMOFF		0xC0000		/* rom offset */
#define	CBTB_REGOFF		0xFFF00		/* 82786 register offset */
#define	CBTB_DACOFF		0xFFF80		/* RAMDAC offset */

#define BTBIOCTL	('x' << 8)
/* ioctl to reset blit board.  Instead of outs in server. */
#define BTBRESET	(BTBIOCTL|0x01)
/* ioctl to return info to pass to KDMAPDISP */
#define BTBSTAT		(BTBIOCTL|0x02)
/* ioctl to turn off ROM on Color Blit */
#define	BTBNOROM	(BTBIOCTL|0x03)

/* btb_stat type */
#define BLTCARD	1
#define CBLTCARD 2	

struct btb_stat {
	int	bs_type;
	int	bs_mapsize;
	int 	bs_memsize;
	int	bs_regoffset;
	int 	bs_romoffset;
	int	bs_ioadx;
	int	bs_pmemadx;
	int	reserved[5];
};
#endif /* XBTB_H */

#ifndef BLTHW_H
#define BLTHW_H

/*
 * Header file for Bell Technologies Blit Express.
 */

/* structures */

typedef struct {
	ushort front[6];		/* first 6 values */
	ushort hsyncstop;		/* monitor parameters */
	ushort hfldstart;
	ushort hfldstop;
	ushort linelength;
	ushort vsyncstop;
	ushort vfldstart;
	ushort vfldstop;
	ushort vframelen;
	ushort descl;			/* descriptor pointer low part */
	ushort desch;			/* descriptor pointer high part */
	ushort middle[8];		/* another 8 values */
	ushort cursorx;			/* cursor x location */
	ushort cursory;			/* cursor y location */
	ushort cursorpat[16];		/* cursor pattern */
} DPCONTROLBLK;

typedef struct {
	ushort lines;			/* lines in strip - 1 */
	ushort linkl;			/* link to next strip low part */
	ushort linkh;			/* link to next strip high part */
	ushort tiles;			/* field flag and tiles in strip - 1 */
} STRIPHEADER;

typedef struct {
	ushort bitmapw;			/* width of bitmap */
	ushort meml;			/* btb mem address low part */
	ushort memh;			/* btb mem address high part */
	ushort bppss;			/* bpp, start and stop fields */
	ushort fetchcnt;		/* fetch count */
	ushort flags;			/* various flags */
} TILEDESC;

typedef struct {
	short dx;			/* delta x */
	short dy;			/* delta y */
	short width;			/* width */
} SCANLINEDATA;

typedef struct {
	short dx;			/* delta x */
	short dy;			/* delta y */
} POLYLINEDATA;

/* misc */

#define DPRDYTO			1000000	/* dp ready wait time out cnt */
#define GPRDYTO			1000000	/* gp ready wait time out cnt */
#define RESETDLY		200	/* delay for reset to complete */
#define	ROMSIZE			0x10000	/* size of bios rom	*/

/* local (on the btb board) and physical (PC bus) memory address conversion */

#define PTOLADX(padx)		(((int)(padx)) - btc_pmem)
#define LTOPADX(ladx)		(((int)(ladx)) + btc_pmem)
#define	RTOPADX(radx)		(((int)(radx)) + btc_rom)
#define LADXLOW(ladx)		(((int)(ladx)) & 0xffff)
#define LADXHIGH(ladx)		((((int)(ladx)) >> 16) & 0x3f)

/* btb board control register */

#define	INVISIBLE		0x40
#define SOFTRESET		0x10
#define DOSMEMMAP		0x00
#define UNIXMEMMAP		0x20	/* rom ON for CBLT */
#define SETCONTROLREG(val)	outb(btc_ioadx, (val) & 0xff)

/* directly addressable internal registers */

#define RREG(adx)		(*((ushort *)(btc_dair+(adx))))
#define WREG(adx, val)		(*((ushort *)(btc_dair+(adx))) = val)
#define WREGB(adx, val)		(*((char *)(btc_dair+(adx))) = val)

#define BIUBASEADX		0x00
#define BIUCTRL			0x04
#define BIUREFRESHCTRL		0x06
#define BIURAMCTRL		0x08
#define BIUDISPLAYPRI		0x0a
#define BIUGPPRI		0x0c
#define BIUEXTPRI		0x0e

#define GPOPCODE		0x20
#define GPLINKL			0x22
#define GPLINKH			0x24
#define GPSTAT			0x26
#define GPIPL			0x28
#define GPIPH			0x2a

#define DPOPCODE		0x40
#define DPMEML			0x42
#define DPMEMH			0x44
#define DPREGID			0x46
#define DPSTAT			0x48
#define DPDEFVIDEO		0x4a

/* dp */

#define ECL			1
#define DPLOADALL		0x0500
#define DPLOADREG		0x0400
#define	CREGID			0x18

/* gp command and heap */

#define GECL			1
#define	GPOLL			0x80
#define HALTINST		(0x0300 | GECL)
#define TRANSPARENT		0x100
#define OPAQUE			0
#define NORMAL			0
#define OP_LINK			0x0200

#endif /* BLTHW_H */

#ifndef BTC_H
#define BTC_H

#define	ASCII_BUFSIZE	(25*80)
#define	ASCII_INIT	0x720

#define	BITMAP		0x00000
#define	ASCII_BUF	0x50000
#define	FONT		0x60000
#define	DP_REG_MAP	0x70000
#define	DESC_PTR	(DP_REG_MAP+sizeof(DPCONTROLBLK))
#define	STACK_PTR	0x7F1F0
#define	GP_START	0x7F200
#define	GP_CHAR		0x7F300
#define	GP_MOVE		0x7F400
#define	LINEOUT		0x7F900
#define	STACK		0x7FFF0

#define	LO(x)	((unsigned short)((x) & 0xFFFF))
#define	HI(x)	((unsigned short)((x) >> 16))

/*   Display Processor opcodes:   */
#define	DP_LOADREG 	0x400
#define	DP_LOAD_REG	0x400
#define	DP_LOADALL	0x500
#define	DP_LOAD_ALL	0x500
#define	DP_DUMPREG	0x600
#define	DP_DUMP_REG	0x600
#define	DP_DUMPALL	0x700
#define	DP_DUMP_ALL	0x700


/*  Graphics Processor opcodes:   */
#define	GP_ABS_MOV	0x4F00
#define	GP_ARC_EXCL	0x6800
#define	GP_ARC_INCL	0x6900
#define	GP_CALL		0x0F00
#define	GP_CIRCLE	0x8E00
#define	GP_DEF_BITMAP	0x1A00
#define	GP_DEF_COLORS	0x3D00
#define	GP_DEF_LOGICAL_OP	0x4100
#define	GP_DEF_TEXTURE_OP	0x0600
#define	GP_LINE		0x5400
#define	GP_LINK		0x0200
#define	GP_LOADREG	0x3400
#define	GP_POINT	0x5300
#define	GP_REL_MOV	0x5200
#define	GP_HALT		0x0301
#define	GP_H_LINE	0xBA00
#define	GP_BIT_BLT	0x6400
#define	GP_BIT_BLT_M	0xAE00
#define	GP_BIT_BLT_E	0xD400
#define	GP_RECT		0x5800
#define	GP_RETURN	0x1700

#endif /* BTC_H */
