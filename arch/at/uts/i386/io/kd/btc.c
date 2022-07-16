/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)at:uts/i386/io/kd/btc.c	1.1"

#ifdef BLTCONS
/*
 * kd driver code for support of BLIT consoles
 */
#include <sys/types.h>
typedef	unsigned char uchar;
#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/immu.h>
#include <sys/at_ansi.h>
#include <sys/kd.h>
#include <sys/inline.h>
#include "btc.h"

#define	CLRCHAR	(NORM << 8 | ' ')

	/* defined in xbtb/space.c */
extern	int xbtb_pmemadx, xbtb_ioadx;

	/* local */
int	btc_ioadx;		/* io address */
int	btc_dairadx;		/* physical address of blit registers */
int	btc_pmem;		/* virtual address of blit memory */
int	btc_dair;		/* virtual address of blit registers */
int	btc_rom;		/* virtual address of blit roms */
int	btc_type;		/* blit card type */
caddr_t	btcbuf;			/* physical address of ascii buffer */
ushort *btcpbuf;		/* virtual address of ascii buffer */
int	btc_dosmode;		/* if !=0, i/o address for page selection */

#define	RPOFF	0x20		/* offset of rom header in DOS mode rom */

#define RHMAGIC	0x65877856

struct romhdr {
	long r_magic;
	ushort dpoff, dplen;	/* dp block (struct d) */
	ushort fontoff, flen;	/* font */
	ushort font_w;		/* font width */
	ushort font_h;		/* font height */
	ushort nchars;		/* # of chars in font */
	ushort xstart, ystart;	/* co-ords of upper left of screen */
	ushort h_dots;		/* width of bitmap */
	ushort v_lines;		/* height of bitmap */
	ushort xcfix, ycfix;	/* fixup values for cursor positioning */
	ushort bpp;		/* bits per pixel */
	ushort reserved[25];	/* reserved for future use */
	char msg[1];		/* message string */
};
struct romhdr *rhp;		/* rom header struct */

struct bltd {
	DPCONTROLBLK d;
	STRIPHEADER s;
	TILEDESC t;
};

struct cbltd {
	DPCONTROLBLK d;
	STRIPHEADER s1;
	TILEDESC t1,t2;
	STRIPHEADER s2;
	TILEDESC t3, t4;
};

int btcpresent = 0;
struct { short x,y; } btc_cursor;

/*
 * called from kd and btb/driver.c
 */
btcinit()
{
	int i;
	register ushort *p, *plim;
	static ushort gp_list [] = {
GP_LOADREG, LO(STACK_PTR), HI(STACK_PTR), 0x10C,	/* load the Stack Pointer */
GP_DEF_BITMAP, LO(BITMAP), HI(BITMAP), 1663, 1199, 1,	/* define the bitmap */
GP_DEF_LOGICAL_OP, 0xFFFF, 0x0000,  	/* replace destination with "0" */
GP_BIT_BLT, 0, 0, 1663, 1199,		/* clear the bitmap */
GP_DEF_TEXTURE_OP, 0x0FFFF,		/* solid texture */
GP_DEF_COLORS, 0xFFFF, 0x0000,
GP_HALT
};

	/* GP stack pointer initializer */
	static ushort gsp[] = {
	LO(STACK), HI(STACK)
};
	int mapsize, romoff, regoff;

	if (!xbtb_pmemadx) return;	/* no blit configured */

	btc_dosmode = 0;
	if (xbtb_pmemadx < 0x100000)
	{
		struct romhdr *rp;
		/* card configured in DOS mode */
		btc_dosmode = xbtb_ioadx;
		btc_dair = phystokv(btc_dairadx = 0xC4400);
		btc_pmem = phystokv(xbtb_pmemadx);
		btc_rom = phystokv(0xCC000);
		rp = (struct romhdr *)RTOPADX(RPOFF);
#ifdef DEBUG
		printf("btc_rom %x btc_pmem %x btc_dair %x\n",
			btc_rom, btc_pmem, btc_dair);
#endif
		/* look for BLT */
		btc_ioadx = xbtb_ioadx;

	        outb(btc_ioadx, DOSMEMMAP);
	        for (i = 0; i < RESETDLY; i++) {
	        }
		outb(btc_ioadx, SOFTRESET);
		for (i = 0; i < RESETDLY; i++) {
		}
		outb(btc_ioadx, DOSMEMMAP);
		for (i = 0; i < RESETDLY; i++) {
		}

		if (rp->r_magic != RHMAGIC)
		{
			/* look for ROM at alternate location */
			btc_rom = phystokv(0xC0000);
			rp = (struct romhdr *)RTOPADX(RPOFF);
		}
		if (rp->r_magic != RHMAGIC)
		{
			/* look for ROM at original location */
			btc_rom = phystokv(0xCC000);
			rp = (struct romhdr *)RTOPADX(RPOFF);
			/* ROM not found */
			/* look for CBLT */
			btc_ioadx++;
		        outb(btc_ioadx, DOSMEMMAP);
		        for (i = 0; i < RESETDLY; i++) {
		        }
			outb(btc_ioadx, SOFTRESET);
			for (i = 0; i < RESETDLY; i++) {
			}
			outb(btc_ioadx, DOSMEMMAP);
			for (i = 0; i < RESETDLY; i++) {
			}

			/* ROM still not found */
			if (rp->r_magic != RHMAGIC)
			{
				/* look for ROM at alternate location */
				btc_rom = phystokv(0xCC000);
				rp = (struct romhdr *)RTOPADX(RPOFF);
			}
			if (rp->r_magic != RHMAGIC)
				return 0;
		}
		if ((rp->bpp > 1) && (rp->h_dots > 800))
		{
			btc_type = CBLTCARD;
			btc_ioadx = xbtb_ioadx+1;
		}
		else
		{
			btc_type = BLTCARD;
			btc_ioadx = xbtb_ioadx;
		}
	}
	else if (xbtb_pmemadx & 0xFFFFF)	/* regular blit */
	{
		btc_type = BLTCARD;
		if (!btc_ioadx) btc_ioadx = xbtb_ioadx;
		if (!btc_dairadx) btc_dairadx = xbtb_pmemadx+BTB_MEM_SIZE;
		mapsize = BTB_MEM_SIZE+NBPP;
		romoff = BTB_ROMOFF;
		regoff = BTB_MEM_SIZE;
	}
	else				/* color blit */
	{
		btc_type = CBLTCARD;
		if (!btc_ioadx) btc_ioadx = xbtb_ioadx+1;
		if (!btc_dairadx) btc_dairadx = xbtb_pmemadx+CBTB_REGOFF;
		mapsize = BTB_MEM_SIZE;
		romoff = CBTB_ROMOFF;
		regoff = CBTB_REGOFF;
	}
	if (!btc_dosmode)
	{
		/* reset 786 */
	        outb(btc_ioadx, UNIXMEMMAP);
	        for (i = 0; i < RESETDLY; i++) {
	        }
		outb(btc_ioadx, SOFTRESET);
		for (i = 0; i < RESETDLY; i++) {
		}
		outb(btc_ioadx, UNIXMEMMAP);
		for (i = 0; i < RESETDLY; i++) {
		}
	}
	/* set up PC bus memory pointers */
	if (!btc_pmem)
		btc_pmem = sptalloc(btoc(mapsize), PG_RW|PG_V, pfnum(xbtb_pmemadx), 0);
	if (!btc_dair)
		btc_dair = btc_pmem+regoff;
	if (!btc_rom)
	{
		if (romoff > mapsize)
			btc_rom = sptalloc(btoc(ROMSIZE), PG_V, pfnum(xbtb_pmemadx+romoff), 0);
		else
			btc_rom = btc_pmem+romoff;
#ifdef DEBUG
		printf("btc_rom %x btc_pmem %x btc_dair %x\n",
			btc_rom, btc_pmem, btc_dair);
#endif
	}
	if (!rhp)
	{
		/* look in original spot */
		rhp = (struct romhdr *)btc_rom;
		if (btc_dosmode || (rhp->r_magic != RHMAGIC))
			rhp = (struct romhdr *)RTOPADX(RPOFF);

		if (rhp->r_magic != RHMAGIC)
		{
			rhp = 0;
			return 0;
		}
		p = &gp_list[5];
		if (btc_type == BLTCARD)
		{
			*p++ = 0;
			*p++ = 0;
			*p++ = rhp->h_dots-1;
		}
		else
		{
			*p++ = LO(BTB_MEM_SIZE);
			*p++ = HI(BTB_MEM_SIZE);
			*p++ = (rhp->h_dots > 1024) ? 2048-1 : 1024-1;
		}
		*p++ = rhp->v_lines-1;
		*p = rhp->bpp;
		p = &gp_list[16];
		*p++ = rhp->h_dots-1;
		*p = rhp->v_lines-1;
	}

	if (check_sig())
		return 0;


	initbiu();

	if (btc_dosmode)
	{
		i = ASCII_BUF & 0xFFFF;
		btcbuf = (caddr_t)(xbtb_pmemadx + i);
		btcpbuf = plim = p = (ushort *)LTOPADX(i);
		outb(btc_dosmode, ASCII_BUF >> 16);
	}
	else
	{
		btcbuf = (caddr_t)(xbtb_pmemadx + ASCII_BUF);
		btcpbuf = plim = p = (ushort *)LTOPADX(ASCII_BUF);
	}
	plim += ASCII_BUFSIZE;
	while (p < plim)
		*p++ = ASCII_INIT;


	cpytol(gsp, STACK_PTR, sizeof(gsp));

	cpytol(RTOPADX(rhp->fontoff), FONT, rhp->flen);

	initdp();

	dogp(gp_list, sizeof(gp_list));

	if (btc_type == CBLTCARD)
		bcvidinit();

	return 1;
}

#ifdef KDDEBUG
btcstuff()
{
	if (rhp)
	{
		printf(
"font_w %d font_h %d nchars %d\nxstart %d ystart %d xfix %d yfix %d\n",
			rhp->font_w, rhp->font_h, rhp->nchars,
			rhp->xstart, rhp->ystart, rhp->xcfix, rhp->ycfix);
		printf("h_dots %d v_lines %d bpp %d\n%s\n",
			rhp->h_dots, rhp->v_lines, rhp->bpp,
			rhp->msg);
	}
	else printf("r_magic %x\n", *(long *)btc_rom);
}
#endif

btcreset()
{
	int i;

	if (!btcpresent)
		return;

	outb(btc_ioadx, SOFTRESET);
	for (i = 0; i < RESETDLY; i++) {
	}
	outb(btc_ioadx, DOSMEMMAP|INVISIBLE);
}
#ifdef HWCURSOR
static
bltcursor(line, col)	/* regular blit cursor routine */
{
	register DPCONTROLBLK *dp = (DPCONTROLBLK *)LTOPADX(DP_REG_MAP);
	static xmul = 0;

	if (!xmul) xmul = (rhp->bpp == 1) ? 2 : 8;

	dp->cursorx = (col * xmul) + rhp->xcfix;
	dp->cursory = (line * rhp->font_h) + rhp->ycfix;

	/* set up dp address */
	WREG(DPMEML, LADXLOW(DP_REG_MAP+CREGID*2));
	WREG(DPMEMH, LADXHIGH(DP_REG_MAP+CREGID*2));

	/* set cursor position register id */
	WREG(DPREGID, CREGID);

	/* load opcode update cursor */
	WREG(DPOPCODE, DPLOADREG);
}
#endif

btcursor(line, col)	/* blit cursor entry */
{
	register ushort *p;
	ushort gp_list[32];
#ifdef HWCURSOR
	if (btc_type == BLTCARD)
	{
		bltcursor(line, col);
		return;
	}
#endif
	if ((btc_cursor.y == line) && (btc_cursor.x == col))
		return;

	/* color blit cursor */
	p = gp_list;
	*p++ = GP_DEF_LOGICAL_OP; *p++ = 0xFFFF; *p++ = 0xC;
	if (btc_cursor.y >= 0)	/* turn off old cursor */
	{
		*p++ = GP_ABS_MOV;
		*p++ = btc_cursor.x * rhp->font_w + rhp->xstart;
		*p++ = btc_cursor.y * rhp->font_h + rhp->ystart;
		*p++ = GP_BIT_BLT;
		*p++ = 0;
		*p++ = 0;
		*p++ = rhp->font_w-1;
		*p++ = rhp->font_h-1;
	}
	if ((line >= 0) && (line < 25) &&
	    (col >= 0) && (col < 80))
	{
		*p++ = GP_ABS_MOV;
		*p++ = col * rhp->font_w + rhp->xstart;	/* set destination x */
		*p++ = line * rhp->font_h + rhp->ystart;/* set destination y */
		*p++ = GP_BIT_BLT;
		*p++ = 0; *p++ = 0;			/* x,y */
		*p++ = rhp->font_w-1; *p++ = rhp->font_h-1;	/* dx,dy */
		btc_cursor.x = col;
		btc_cursor.y = line;
	}
	else btc_cursor.y = -1;

	*p++ = GP_HALT;

	dogp(gp_list, (int)p - (int)gp_list);
}

btcmoveit(from, to, count)
ushort from, to;
int count;
{
	short srcx, srcy, destx, desty, limit, dx, dy;
	ushort gp_list[64];
	register ushort *p;

	srcx = from % 80;
	srcy = from / 80;
	destx = to % 80;
	desty = to / 80;
	limit = from+count-1;
	dx = (limit % 80) - srcx + 1;
	dy = (limit / 80) - srcy + 1;
	p = gp_list;
#ifdef HWCURSOR
	if (btc_type == CBLTCARD)
	{
#endif
	    if (btc_cursor.y >= 0)	/* turn off old cursor */
	    {
		*p++ = GP_DEF_LOGICAL_OP; *p++ = 0xFFFF; *p++ = 0xC;
		*p++ = GP_ABS_MOV;
		*p++ = btc_cursor.x * rhp->font_w + rhp->xstart;
		*p++ = btc_cursor.y * rhp->font_h + rhp->ystart;
		*p++ = GP_BIT_BLT;
		*p++ = 0;
		*p++ = 0;
		*p++ = rhp->font_w-1;
		*p++ = rhp->font_h-1;
	    }
#ifdef HWCURSOR
	}
#endif
	*p++ = GP_ABS_MOV;
	*p++ = destx * rhp->font_w + rhp->xstart;	/* set destination x */
	*p++ = desty * rhp->font_h + rhp->ystart;	/* set destination y */
	*p++ = GP_DEF_LOGICAL_OP; *p++ = 0xFFFF; *p++ = 0x5;
	*p++ = GP_BIT_BLT;
	*p++ = srcx * rhp->font_w + rhp->xstart;	/* set source x */
	*p++ = srcy * rhp->font_h + rhp->ystart;	/* set source y */
	*p++ = dx * rhp->font_w;		/* set source dx */
	*p++ = dy * rhp->font_h;		/* set source dy */
#ifdef HWCURSOR
	if (btc_type == CBLTCARD)
	{
#endif
	    if (btc_cursor.y >= 0)	/* turn cursor back on */
	    {
		*p++ = GP_DEF_LOGICAL_OP; *p++ = 0xFFFF; *p++ = 0xC;
		*p++ = GP_ABS_MOV;
		*p++ = btc_cursor.x * rhp->font_w + rhp->xstart;
		*p++ = btc_cursor.y * rhp->font_h + rhp->ystart;
		*p++ = GP_BIT_BLT;
		*p++ = 0;
		*p++ = 0;
		*p++ = rhp->font_w-1;
		*p++ = rhp->font_h-1;
	    }
#ifdef HWCURSOR
	}
#endif

	*p++ = GP_HALT;
	dogp(gp_list, (int)p - (int)gp_list);
}

btcupdate(srcp, limit)		/* update screen with new buffer contents */
register ushort *srcp;		/* called from kdscrxfer */
register int limit;
{
	static ushort gp_list [] = {
GP_DEF_BITMAP, LO(BITMAP), HI(BITMAP), 1663, 1199, 1,	/* define the bitmap */
GP_DEF_LOGICAL_OP, 0xFFFF, 0x0000,  	/* replace destination with "0" */
GP_BIT_BLT, 0, 0, 1663, 1199,		/* clear the bitmap */
GP_DEF_TEXTURE_OP, 0x0FFFF,		/* solid texture */
GP_DEF_COLORS, 0xFFFF, 0x0000,
GP_DEF_LOGICAL_OP, 0xFFFF, 5,
GP_HALT
};
	register i;
	register ushort *destp;

	destp = &gp_list[1];
	if (btc_type == CBLTCARD)
	{
		if (!btc_dosmode)
			/* turn on ROM */
			outb(btc_ioadx, UNIXMEMMAP);
		*destp++ = LO(BTB_MEM_SIZE);
		*destp++ = HI(BTB_MEM_SIZE);
		*destp++ = (rhp->h_dots > 1024) ? 2048-1 : 1024-1;
	}
	else
	{
		*destp++ = 0;
		*destp++ = 0;
		*destp++ = rhp->h_dots-1;
	}
	*destp++ = rhp->v_lines-1;
	*destp++ = rhp->bpp;
	destp += 6;
	*destp++ = rhp->h_dots-1;
	*destp = rhp->v_lines-1;

	/* copy font back */
	cpytol(RTOPADX(rhp->fontoff), FONT, rhp->flen);

	/* init bitmap */
	dogp(gp_list, sizeof(gp_list));
	waitgp();

	/* turn display back on */
	initdp();

	destp = btcpbuf;
	for (i=0; i<limit; i++, srcp++, destp++)
	{
		if ((*destp = *srcp) != CLRCHAR)
			btchar(*srcp, i, 1);
	}
}

static
btclear(start, count)
ushort start;
register int count;
{
	short x, y, dx, dy;
	static ushort gp_list[] = {
GP_DEF_LOGICAL_OP, 0xFFFF, 0,		/* set to all 0 */
GP_ABS_MOV, 0, 0,			/* x, y */
GP_BIT_BLT, 0, 0, 0, 0,			/* x, y, dx, dy */
GP_HALT
	};
	register ushort *p, *limit;

#ifndef HWCURSOR
	short curpos;
#endif
	if (count <= 1) return;
#ifndef HWCURSOR
	curpos = btc_cursor.y * 80 + btc_cursor.x;
	if ((curpos < start) || (curpos >= (start+count)))
		curpos = -1;
#endif
	p = btcpbuf+start;
	limit = p+count;
	do *p++ = CLRCHAR;
	while (p < limit);

	while (count > 0)
	{
		x = start % 80;
		y = start / 80;
		if (x)
		{
			dx = (80 - x);
			if (count < dx) dx = count;
			dy = 1;
			count -= dx;
			start += dx;
		}
		else if (count > 80)
		{
			dy = count / 80;
			dx = dy * 80;
			count -= dx;
			start += dx;
			dx = 80;
		}
		else
		{
			dx = count;
			dy = 1;
			count = 0;
		}
		p = &gp_list[4];
		*p++ = x * rhp->font_w + rhp->xstart;	/* set destination x */
		*p = y * rhp->font_h + rhp->ystart;	/* set destination y */
		p = &gp_list[9];
		*p++ = dx * rhp->font_w - 1;	/* set width */
		*p = dy * rhp->font_h - 1;	/* set height */
		dogp(gp_list, sizeof(gp_list));
	}
#ifndef HWCURSOR
	if (curpos < 0) return;
	btc_cursor.y = -1;
	btcursor(curpos/80, btc_cursor.x);
#endif
}

btchar(srcchar, dest, count)
ushort srcchar, dest;
int count;
{
	static short ulin = 0;
	static short bold;
	static ushort gp_buf[128] = {
GP_DEF_LOGICAL_OP, 0xFFFF, 5,
GP_ABS_MOV, 0, 0,					/* x, y */
GP_BIT_BLT_E, LO(FONT), HI(FONT), 0, 0, 0, 0, 0, 0,	/* w, h, x, y, dx, dy */
GP_HALT
	};
	register ushort *p;
	ushort row, col, attr; 

	if ((srcchar == CLRCHAR) && (count > 1))
	{
		btclear(dest, count);
		return;
	}
	if (!ulin)
	{
		p = &gp_buf[9];
		*p++ = (rhp->font_w * rhp->nchars)-1;
		*p = rhp->font_h;
		p = &gp_buf[13];
		*p++ = rhp->font_w-1;
		*p = rhp->font_h-1;
		ulin = (rhp->bpp == 1) ? 3 : 2;
		bold = (rhp->bpp == 1) ? 2 : 1;
	}
	attr = srcchar >> 8;
	while (count-- > 0)
	{
		btcpbuf[dest] = srcchar;
		col = (dest % 80);
		row = (dest / 80);
#ifdef HWCURSOR
		if (btc_type == CBLTCARD)
		{
#endif
			if ((row == btc_cursor.y) && (col == btc_cursor.x))
				btc_cursor.y = -1;
#ifdef HWCURSOR
		}
#endif
		col = col * rhp->font_w + rhp->xstart;
		row = row * rhp->font_h + rhp->ystart;
		p = &gp_buf[4];
		*p++ = col;			/* set destination x */
		*p = row;			/* set destination y */
		p = &gp_buf[11];
		*p = (srcchar & 0xFF) < rhp->nchars ?
			(srcchar & 0xFF) :
			'.';
		*p *= rhp->font_w;
		p = &gp_buf[15];
		if (attr & BRIGHT)
		{
			*p++ = GP_ABS_MOV;
			*p++ = col+bold;
			*p++ = row;
			*p++ = GP_DEF_LOGICAL_OP;
			*p++ = 0xFFFF;
			*p++ = 0x7;
			*p++ = GP_BIT_BLT;
			*p++ = col;
			*p++ = row,
			*p++ = rhp->font_w-1;
			*p++ = rhp->font_h-1;	/* copy the char cell */
		}
		if ((attr & NORM) == UNDERLINE)
		{
			*p++ = GP_ABS_MOV;
			*p++ = col;
			*p++ = row+rhp->font_h-ulin;
			*p++ = GP_DEF_LOGICAL_OP;
			*p++ = 0xFFFF;
			*p++ = 0x5;		/* reset the logical op */
			*p++ = GP_LINE;
			*p++ = rhp->font_w;
			*p++ = 0;
		}
		if ((attr & REVERSE) == REVERSE)
		{
			*p++ = GP_ABS_MOV;
			*p++ = col;
			*p++ = row;
			*p++ = GP_DEF_LOGICAL_OP;
			*p++ = 0xFFFF;
			*p++ = 0xC;
			*p++ = GP_BIT_BLT;
			*p++ = 0;
			*p++ = 0;
			*p++ = rhp->font_w-1;
			*p++ = rhp->font_h-1;	/* complement the char cell */
		}
		*p++ = GP_HALT;
		dogp(gp_buf, (int)p - (int)gp_buf);
		dest++;
	}
}

#define	BIOSMAGIC 0xAA55
#define	BIOSTART  0xC000
#define	ROMSIG	7	/* offset of signature string from start of BIOS ROM */

static
check_sig()
{
	static char sig[] = "Bell Tech BLIT";
	register biostart = ((int)rhp == btc_rom) ? BIOSTART : 0;
	register i;

	/* check for bios rom */
#ifdef DEBUG
	printf("BIOSTART: %x *BIOSTART %x\n",
		RTOPADX(biostart), *(ushort *)RTOPADX(biostart));
#endif
	if (*(ushort *)RTOPADX(biostart) != BIOSMAGIC)
	{
#ifdef DEBUG
		printf("btcinit: Blit roms not found\n");
#endif
		return 1;
	}

	/* check for Bell Tech signature */
	for (i=0; sig[i]; i++)
	{
		if (sig[i] == *(char *)RTOPADX(biostart+ROMSIG+i)) continue;
#ifdef DEBUG
		printf("btcinit: Bad signature in Blit roms\n");
#endif
		return 1;
	}
#ifdef DEBUG
	printf("Blit roms found\n");
#endif
	btcpresent = 1;
	return 0;
}
/*
 * Initialize the bus interface unit.
 */
static
initbiu()
{
	int i;
	if (btc_type == CBLTCARD)
	{
	    if (btc_dosmode)
	    {
		/* set base address for dair to 4400 */
		WREGB(BIUBASEADX, 0x10);
		WREGB(BIUBASEADX + 1, 0x01);
	    }
	    else
	    {
		/* must start in DOS mode */
	        outb(btc_ioadx, DOSMEMMAP);
	        for (i = 0; i < RESETDLY; i++);
/*
 *		WREGB(BIUBASEADX, 0xfd);
 *		WREGB(BIUBASEADX + 1, 0x3f);
 */
		/* for now, must set reloc reg in DOS mode */
		*(char *)phystokv(0xc4400) = 0xfd;
		*(char *)phystokv(0xc4401) = 0x3f;

		/* now change to UNIX mode */

		outb(btc_ioadx, UNIXMEMMAP);
	    }

		/* set 16 bit access mode */
		WREGB(BIUCTRL, 0x50);
		WREGB(BIUCTRL + 1, 0x00);

		/* set other biu registers */
		WREG(BIUREFRESHCTRL, 0x0012);
		WREG(BIURAMCTRL, 0x004d);
		WREG(BIUDISPLAYPRI, 0x003f);
		WREG(BIUGPPRI, 0x0009);
		WREG(BIUEXTPRI, 0x0028);
		WREG(BIUCTRL, 0x0052);
	}
	else
	{
	    if (btc_dosmode)
	    {
		/* set base address for dair to 4400 */
		WREGB(BIUBASEADX, 0x10);
		WREGB(BIUBASEADX + 1, 0x01);
	    }
	    else
	    {
		/* set base address for dair to 0 */
		WREGB(BIUBASEADX, 0);
		WREGB(BIUBASEADX + 1, 0);
	    }

		/* set 16 bit access mode */
		WREGB(BIUCTRL, 0x10);
		WREGB(BIUCTRL + 1, 0x00);

		/* set other biu registers */
		WREG(BIUREFRESHCTRL, 0x0018);
		WREG(BIURAMCTRL, 0x001d);
		WREG(BIUDISPLAYPRI, 0x003f);
		WREG(BIUGPPRI, 0x0009);
		WREG(BIUEXTPRI, 0x0028);
		WREG(BIUCTRL, 0x0012);
	}
	WREG(GPOPCODE, (OP_LINK|GECL));
}

static
initdp()
{
	struct bltd *bdp;
	struct cbltd *cdp;

	cpytol(RTOPADX(rhp->dpoff), DP_REG_MAP, rhp->dplen);
	if (btc_dosmode)
		outb(btc_dosmode, DP_REG_MAP >> 16);
	if (btc_type == BLTCARD)
	{
		if (btc_dosmode)
			bdp = (struct bltd *)LTOPADX(DP_REG_MAP & 0xFFFF);
		else
			bdp = (struct bltd *)LTOPADX(DP_REG_MAP);
#ifdef HWCURSOR
		bdp->d.cursorx = rhp->xcfix;
		bdp->d.cursory = 24*rhp->font_h+rhp->ycfix;
#else
		btc_cursor.y = -1;
		bdp->d.front[0] = 1;
#endif
		bdp->d.descl = bdp->s.linkl = LO(DESC_PTR);
		bdp->d.desch = bdp->s.linkh = HI(DESC_PTR);
		bdp->t.meml = 0;
		bdp->t.memh = 0;
	}
	else
	{
		if (btc_dosmode)
			cdp = (struct cbltd *)LTOPADX(DP_REG_MAP & 0xFFFF);
		else
			cdp = (struct cbltd *)LTOPADX(DP_REG_MAP);
		cdp->d.descl = LO(DESC_PTR);
		cdp->d.desch = HI(DESC_PTR);
#define	STRIP_LEN 32			/* 32 words to the next strip */
		cdp->s1.linkl = LO(DESC_PTR+STRIP_LEN);
		cdp->s1.linkh = HI(DESC_PTR+STRIP_LEN);

		cdp->t1.meml = LO(BTB_MEM_SIZE);/* bitmap memory address */
		cdp->t1.memh = HI(BTB_MEM_SIZE);

		cdp->s2.linkl = LO(DESC_PTR);
		cdp->s2.linkh = HI(DESC_PTR);

		cdp->t3.meml = LO(BTB_MEM_SIZE+2*cdp->t3.bitmapw);
		cdp->t3.memh = HI(BTB_MEM_SIZE);
		btc_cursor.y = -1;
	}
	/* set up dp address */
	WREG(DPMEML, DP_REG_MAP & 0xFFFF);
	WREG(DPMEMH, DP_REG_MAP >> 16);
  
	/* set blanking video */
	WREG(DPDEFVIDEO, 0);

	/* load opcode to start dp */
	WREG(DPOPCODE, DPLOADALL);

	if (btc_dosmode)
		outb(btc_dosmode, ASCII_BUF >> 16);
}

static
dogp(bufp, len)
short *bufp;
int len;
{
	waitgp();		/* wait for it */
	cpytol(bufp, GP_START, len);
	WREG(GPLINKL, LADXLOW(GP_START));
	WREG(GPLINKH, LADXHIGH(GP_START));
	WREG(GPOPCODE, OP_LINK);
}
/*
 * If a command is executing in the gp, wait for it to finish.
 */
static
waitgp()
{
	int i, s;

	for (i = 0; ((s = RREG(GPSTAT)) & GPOLL) == 0 && i < GPRDYTO; i++) {
	}
}
/*
 * Copy <src> in UNIX memory space to <dest> in btb memory space for
 * <len> bytes.
 */
static
cpytol(src, dest, len)
char *src;
int dest;
int len;
{

#ifdef DEBUG
	printf("cpytol(0x%x,0x%x,0x%x) -> 0x%x",
		src, dest, len, LTOPADX(dest));
#endif

	if (btc_dosmode)
	{
		register page, n;

		page = dest >> 16;
		while (len > 0)
		{
			dest &= 0xFFFF;
			n = 0x10000 - dest;
			if (len < n) n = len;
			outb(btc_dosmode, page++);
			bcopy(src, LTOPADX(dest), n);
			src += n;
			dest += n;
			len -= n;
		}
		outb(btc_dosmode, ASCII_BUF >> 16);
	}
	bcopy(src, LTOPADX(dest), len);  /* copy data */

#ifdef DEBUG
	printf(" finished\n");
#endif
}

/*
 * this routine is in a seperate module
 * only because it must be compiled WITHOUT OPTIMIZATION
 */
extern int btc_dair;

/*	Addresses for the Brooktree BT458 RAMDAC */
#define DAC_ADDR	0
#define DAC_RAMDAC	2
#define DAC_CNTRL	4

#define DAC_COLOR	0
#define DAC_READ	4
#define DAC_BLINK	5
#define DAC_CMD		6

/*
 * Initialize pseudo color map.
 */
bcvidinit()
{
	volatile char *dac_addr;
	volatile char *dac_ramdac;
	volatile char *dac_cntrl;
	int i;


	/* White = 255; black = 0x0 */ 
	/* set blink mode, command mode, etc */ 
	dac_addr	= (char *) btc_dair+128;
	dac_cntrl	= dac_addr + DAC_CNTRL;
	dac_ramdac	= dac_addr + DAC_RAMDAC;

	*dac_addr = DAC_READ;
	*dac_cntrl = 0xff;		/* turn on all bits in pixel */
	*dac_addr = DAC_BLINK;
	*dac_cntrl = 0x00;		/* disable blink */
	*dac_addr = DAC_CMD;
	*dac_cntrl = 0x40;		/* 4to1 mux, color pallete RAM */
							/* don't care blink rate */
							/* disable blinking, disable overlay */
	/* initialize ramdac to shades of grey */
	*dac_addr = DAC_COLOR;
	*dac_ramdac = (char) 0;	/* set color 0 to black */
	*dac_ramdac = (char) 0;
	*dac_ramdac = (char) 0;
	*dac_ramdac = (char) 0xff;	/* set color 1 to white */
	*dac_ramdac = (char) 0xff;
	*dac_ramdac = (char) 0xff;
	for ( i = 2; i < 254; i++ ) {
			*dac_ramdac =  (char) 0;	/* red */
			*dac_ramdac =  (char) 0;	/* green */
			*dac_ramdac =  (char) 0;	/* blue */
	}
	*dac_ramdac = (char) 0xff;	/* set color 255 to white */
	*dac_ramdac = (char) 0xff;
	*dac_ramdac = (char) 0xff;
}
#endif /* BLTCONS */
