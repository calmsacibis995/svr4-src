/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ident	"@(#)i286emu:text.c	1.1"

#undef MAIN
#include "vars.h"
#include "i286sys/exec.h"

#define	vtosel(a) (((a)>>16)&0xFFFF)

xalloc(fd,exec_data)
	int fd;
	struct exdata *exec_data;
{
	struct xscn     *xsp;           /* section structure pointer */
	long            offset;         /* section offset in file */
	long            base;           /* section base virtual addr */
	long            size;           /* section size in bytes */
	unsigned short          tcnt;           /* text region count */
	short           n;              /* count index */
	char            * tp;           /* place to put text */

	tcnt = 0;               /* text region count */

	/*      Go through the file sections,
	 *      reading and mapping each one
	 */

	for (xsp = exec_data->x_scn, n = exec_data->x_nscns;
	     --n >= 0; xsp++) {

		/* skip over non-text sections */
		if (xsp->xs_flags != STYP_TEXT)
			continue;

dprintf("xs_vaddr=%lx xs_offset=%lx xsp->xs_size=%lx\n",
xsp->xs_vaddr, xsp->xs_offset, xsp->xs_size);

		tp = getmem( xsp->xs_size );
		seekto( fd, xsp->xs_offset );
		readfrom( fd, tp, xsp->xs_size );
		setsegdscr( vtosel(xsp->xs_vaddr), tp, xsp->xs_size, 0 );
		tcnt++;         /* increment text region count */
	}
dprintf("xalloc: tcnt=%x new text regions mapped\n", tcnt);
}
