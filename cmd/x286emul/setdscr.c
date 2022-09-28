/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)x286emul:setdscr.c	1.1"

#include <sys/seg.h>
#include <sys/sysi86.h>
#include "vars.h"

setsegdscr(sel, base, lsize, psize, data)
	unsigned int sel;       /* segment selector */
	unsigned int base;      /* segment base address */
	unsigned int lsize;     /* # of byte mapped in LDT */
	unsigned int psize;	/* allocated size */
	int data;               /* data&0xffff -> 0=text, 1=data, 2=no access
						  3=32-bit stk. alias */
				/* data>>16 -> 0=default, 1=data, 2=shr data */
{
	struct ssd ssd;
	int rc;
	int type;
	int i;

#ifdef TRACE
	if (systrace) {
		fprintf( dbgfd,"setsegdscr(sel=0x%2x, base=0x%4x, lsize=0x%4x, psize=0x%4x, type %d, data %d\n",sel,base,lsize,psize,(data>>16)&0xffff,data&0xffff);
	}
#endif
	/* set up the request structure to map the segment */

	type = (data >> 16) & 0xffff;
	data &= 0xffff;
	if ( type == 0 )
		type = data;
	ssd.sel  = sel;
	ssd.bo   = base;
	ssd.ls = (lsize == 0) ? 0 : lsize - 1;
	ssd.acc2 = 0;                   /* 1 byte granularity */
	switch ( data ) {
	case 0:
		ssd.acc1 = UTEXT_ACC1; break;
	case 1:
		ssd.acc1 = UDATA_ACC1; break;
	case 3:
		ssd.acc1 = UDATA_ACC1; ssd.acc2 = TEXT_ACC2; break;
	default:
		ssd.acc1 = 0; break;
	}

	/* do the system call */

#ifdef DEBUG
	fprintf( dbgfd,"setsegdscr calling sysi86(SI86DSCR, &ssd)\n");
#endif
	rc = sysi86(SI86DSCR, &ssd);
#ifdef DEBUG
	fprintf( dbgfd,"sysi86() returned %d\n", rc);
#endif
	if (rc) {
		if ( Nodeath )
			return 0;
		emprintf( "setsegdscr: sysi86(SI86DSCR) failed\n" );
		emprintf( "  sel=%x base=%x {l,p}size=%x,%x data=%x\n",
			sel, base, lsize, psize, data);
		exit(1);
	}


	if (data == 3)		/* 32-bit stack alias */
		return 1;

	i = SELTOIDX(sel);
	if ( data > 1 ) {
		char * sbase;
		sbase = Dsegs[i].base;
		if ( type == 0 && ISMAPPED(sbase) )
			free( Dsegs[i].base );
		Dsegs[i].lsize = 0;
		Dsegs[i].psize = 0;
		Dsegs[i].base = BAD_ADDR;
		Dsegs[i].type = 0;
	} else {
		Dsegs[i].base = (char *)base;
		Dsegs[i].lsize = lsize;
		Dsegs[i].psize = psize;
		Dsegs[i].type = type;
	}
	return 1;
}
