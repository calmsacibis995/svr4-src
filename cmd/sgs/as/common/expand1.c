/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:common/expand1.c	1.7"

#include <stdio.h>
#include "systems.h"
#include "symbols.h"
#include "gendefs.h"
#include <memory.h>
#include "section.h"

/* 	This file contains functions that implement span dependent
 *	instruction optimization.  
 */

#if i386			/* to be added in for span dependent optimization to
				 * account for the PC being at the next instruction
				 * Note: sdihalf() is not used for the 386 which
				 * only allows byte or word displacements */
#define PC_OFFSET	2	 
#else				
#define PC_OFFSET 	0
#endif

extern VOID *malloc();
extern symbol *lookup();


extern ITINFO		itinfo[];
extern symbol		*dot;
extern long		newdot;
extern char		*strtab;


static SDIBOX	*newsdi();

static int	sdibyte();
static int	sdihalf();


SDIBOX	*Sdibox;
SDI	*Sdi;

static int	curscn;

int uniq=0;

void
deflab(sym)
	register symbol	*sym;
{
	register SDIBOX	*tail;
	register SDI	*sdp;

	/*	if no sdi's in this section yet, label value
	 *	can't change and thus doesn't need to go on list
	 */

	if ((tail = sectab[sym->sectnum].Sditail) == 0)
		return;
	if ((sdp = tail->b_end) >= &tail->b_buf[NSDI])
	{
		sectab[sym->sectnum].Sditail = tail = newsdi(tail);
		sdp = tail->b_end;
	}
	sdp->sd_itype = IT_LABEL;
	sdp->sd_flags = SDF_DONE;
	sdp->sd_sym = sym;
	++tail->b_end;
	return;
}


/* expand
 *	fix sdi lengths.  instructions that reference themselves
 *
 *		L1:	jmp	L1
 *
 *	are "backward" jumps, because the label is defined before
 *	the operand is used.
 *
 *	bottom loop mirrors top, except it must distinguish forward
 *	and backward references to know if the addresses have been
 *	updated yet in this pass.
 */

static void
expand(head)
	SDIBOX		*head;
{
	register long	delta;
	register SDI	*sdp;
	SDI		*end;
	register SDIBOX	*hd;

top:
	delta = 0;
	hd = head;
	do
	{
		end = hd->b_end;
		sdp = &hd->b_buf[0];
		do
		{
			if (sdp->sd_flags & SDF_DONE)
				continue;
			if (sdp->sd_flags & SDF_BYTE)
			{
				if ((delta = sdibyte(sdp)) != 0)
					goto bot;
				continue;
			}
			if ((delta = sdihalf(sdp)) != 0)
				goto bot;
		} while (++sdp < end);
	} while ((hd = hd->b_link) != 0);
	return;
/*NOTREACHED*/
unreached_label:;   /* do not generate compiler warning */
	do
	{
		end = hd->b_end;
		sdp = &hd->b_buf[0];
		do
		{
			if (sdp->sd_itype == IT_LABEL)
			{
				sdp->sd_sym->value += delta;
				continue;
			}
			if (sdp->sd_flags & SDF_DONE)
			{
				sdp->sd_addr += delta;
				continue;
			}
			if (sdp->sd_flags & SDF_BYTE)
			{
				if (sdp->sd_flags & SDF_BACK)
				{
					sdp->sd_addr += delta;
					delta += sdibyte(sdp);
				}
				else
					sdp->sd_addr += delta += sdibyte(sdp);
				continue;
			}
			if (sdp->sd_flags & SDF_BACK)
			{
				sdp->sd_addr += delta;
				delta += sdihalf(sdp);
			}
			else
				sdp->sd_addr += delta += sdihalf(sdp);
bot:;
		} while (++sdp < end);
	} while ((hd = hd->b_link) != 0);
	newdot += delta;
	goto top;
}


/* fixsyms
 *	expand the sdi list as needed.  Use cgsect()
 *	because it uses and sets newdot, and it updates
 *	the section size.  Because cgsect() is used later,
 *	we must preserve the correctness of newdot.
 */

void
fixsyms()
{
	int	j;
	extern void cgsect();

	j = dot->sectnum;
	for (curscn = 0; curscn < NSECTIONS; ++curscn)
		if (sectab[curscn].Sdihead)
		{
			cgsect(curscn);
			expand(sectab[curscn].Sdihead);
		}
	cgsect(j);
}


static SDIBOX *
newsdi(old)
	SDIBOX	*old;
{
	extern void aerror();
	SDIBOX	*new;

	if ((new = (SDIBOX *)malloc(sizeof(*new))) == 0)
		aerror("out of sdi memory");
	new->b_link = 0;
	new->b_end = &new->b_buf[0];
	if (old)
		old->b_link = new;
	return new;
}


static int
sdibyte(sdp)
	register SDI	*sdp;
{
	register long	v;
	register symbol	*sym;
	register ITINFO	*ip;

	if ((sym = sdp->sd_sym)->sectnum != curscn)
	{
		sdp->sd_flags |= SDF_DONE;
		sdp->sd_flags &= ~(SDF_BYTE | SDF_HALF);
		ip = &itinfo[sdp->sd_itype];
		return ip->it_binc + ip->it_hinc;
	}
	if (sym == dot)
		v = sdp->sd_addr + sdp->sd_off - (sdp->sd_addr + PC_OFFSET);
		/* symbol value is equal to sdp->sd_addr */
	else
		v = sym->value + sdp->sd_off - (sdp->sd_addr + PC_OFFSET);

	if (v < SPAN_BLO || v > SPAN_BHI)
	{
		sdp->sd_flags &= ~SDF_BYTE;
		return itinfo[sdp->sd_itype].it_binc;
	}
	return 0;
}


static int
sdihalf(sdp)
	register SDI	*sdp;
{
	register long	v;
	register symbol	*sym;
	

	sym = sdp->sd_sym;
	if (sym == dot)
		v = sdp->sd_off;
	else
		v = sym->value + sdp->sd_off - sdp->sd_addr;

	if (sym->sectnum != curscn ||  v < SPAN_HLO ||  v > SPAN_HHI)
	{
		sdp->sd_flags |= SDF_DONE;
		sdp->sd_flags &= ~(SDF_BYTE | SDF_HALF);
		return itinfo[sdp->sd_itype].it_hinc;
	}
	return 0;
}


SDI *
shortsdi(sym,off)
	register symbol	*sym;
	long		off;
{
	char		flags;
	register SDI	*sdp;
	SDIBOX		*tail;
	register symbol	*s;	

	if (sym == 0 || !(sectab[dot->sectnum].flags & SHF_EXECINSTR))
		return 0;
	if (UNDEF_SYM(sym))
		flags = SDF_BYTE | SDF_HALF;
	else if (!(sectab[dot->sectnum].flags & SHF_EXECINSTR) || sym->sectnum != dot->sectnum)
		return 0;
	else
	{
		if (off < MIN16 || off > MAX16)
			return 0;
		flags = SDF_BACK | SDF_BYTE | SDF_HALF;
	}
	if ((tail = sectab[dot->sectnum].Sditail) == 0)
	{
		tail = newsdi((SDIBOX *)0);
		sectab[dot->sectnum].Sdihead = sectab[dot->sectnum].Sditail = tail;
	}
	if (sym == dot) {
		char newlab[20];
		(void) sprintf(newlab,".DOT#%d",uniq++);
		s = lookup(newlab,INSTALL);
		s->value = dot->value;
		s->sectnum = dot->sectnum;
		s->binding = STB_LOCAL;
		s->flags &= ~GO_IN_SYMTAB;
		deflab(s);
	}
	if ((sdp = tail->b_end) >= &tail->b_buf[NSDI])
	{
		sectab[dot->sectnum].Sditail = tail = newsdi(tail);
		sdp = tail->b_end;
	}
	if (sym == dot)
		sdp->sd_sym = s;
	else
		sdp->sd_sym = sym;
	sdp->sd_addr = dot->value;
	sdp->sd_off = (short) off;
	sdp->sd_flags = flags;
	++tail->b_end;
	return sdp;
}
