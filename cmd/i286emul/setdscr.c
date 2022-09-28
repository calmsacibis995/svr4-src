/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)i286emu:setdscr.c	1.1"

#include <sys/seg.h>
#include <sys/sysi86.h>
#include <stdio.h>
#include "vars.h"

#define SYSCALLSEL      89      /* 286 system call call gate selector */

setsegdscr(sel, base, size, data)
	unsigned int sel;       /* segment selector */
	unsigned int base;      /* segment base address */
	unsigned int size;      /* segment size */
	int data;               /* 0=text, 1=data, 2=no access
				   3=32-bit stk. alias */
{
	struct ssd ssd;
	int rc;

	/* set up the request structure to map the segment */

	ssd.sel  = sel;
	ssd.bo   = base;
	ssd.ls   = size - 1;
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

	dprintf("setsegdscr calling sysi86(SI86DSCR, &ssd)\n");
	rc = sysi86(SI86DSCR, &ssd);
	dprintf("sysi86() returned %d\n", rc);
	if (rc) {
		if ( nodeath )
			return 0;
		emprintf( "setsegdscr: sysi86(SI86DSCR) failed\n" );
		emprintf(
			"  sel=%x base=%x size=%x data=%x\n",
			sel, base, size, data);
		exit(1);
	}

	if (data == 3)		/* 32-bit stack alias */
		return 1;

	size = ctob(btoc(size));
#define IND(sel) (((sel)>>3)&0x1fff)
	if ( data > 1 ) {
		if ( IND(sel) <= lastds )
			free( dsegs[IND(sel)].base );
		dsegs[IND(sel)].size = 0;
		dsegs[IND(sel)].base = BAD_ADDR;
	} else {
		dsegs[IND(sel)].base = (char *)base;
		dsegs[IND(sel)].size = size;
	}
	return 1;
}


setcallgate()
{
	struct ssd ssd;
	int rc;
	extern unsigned int syscalla;   /* system call interface entry */

	/* set up the request structure */

	ssd.sel  = (SYSCALLSEL << 3) | 3;       /* 286 syscall selector */
	ssd.bo   = (int)&syscalla;              /* system call interface */
	ssd.ls   = USER_CS;                     /* 386 user CS selector */
	ssd.acc1 = GATE_UACC | GATE_386CALL;    /* 386 call gate */
	ssd.acc2 = 0;                           /* zero word count */

	/* do the system call */

	dprintf("setcallgate calling sysi86(SI86DSCR, &ssd)\n");
	rc = sysi86(SI86DSCR, &ssd);
	dprintf("sysi86() returned %d\n", rc);
	if (rc) {
		emprintf( "setsegdscr: sysi86(SI86DSCR) failed\n");
		exit(1);
	}
}
