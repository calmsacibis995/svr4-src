/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ident	"@(#)i286emu:gethead.c	1.1"

#include <stdio.h>
#include "i286sys/exec.h"
#include "i286sys/param.h"

#undef  MAIN

#include "vars.h"

#define vtoi(X) (((X)>>19)&0x1FFF)      /* vaddr to ldt index */

/*
 * Read the a.out headers.  There must be .text, .data and .bss sections.
 * A small model program has one of each.
 * A large model program has at least one of each.
 */

gethead(fd, exec_data)
	int fd;
	register struct exdata *exec_data;
{
	struct filehdr filhdr;          /* COFF file header buffer */
	struct aouthdr aouthdr;         /* "optional" unix header buffer */
	struct scnhdr  scnhdr;          /* section header buffer */
	struct xscn *xsp;               /* section structure pointer */
	struct xscn *dataxsp;           /* last data section pointer */
	struct xscn *bssxsp;            /* last bss section pointer */

	short   n;                      /* count index */
	unsigned short  scns = 0;               /* text, data & bss section flags */
	unsigned short  nscns = 0;              /* number of sections */

	int count;

	/*
	 * assume file has been checked for execute permission
	 */

	/*
	 * First, read the file header and check it out
	 */

	seekto( fd, 0L );
	readfrom( fd, &filhdr, sizeof filhdr);
dprintf( "filhdr.f_magic == %d, filhdr.f_nscns == %d\n",
	filhdr.f_magic, filhdr.f_nscns );

	if ( (filhdr.f_magic != I286SMAGIC && filhdr.f_magic != I286LMAGIC)
	    || (filhdr.f_nscns > XNSCNMAX)) {
		emprintf(  "nonexistant or bad file header\n" );
		exit(1);
	}
	exec_data->x_fmagic = filhdr.f_magic;

	/*
	 * Next, read the "optional" unix header, which must be there.
	 */

	if (filhdr.f_opthdr != sizeof(aouthdr)) {
		emprintf( "no unix header in file\n");
		exit(1);
	}

	readfrom( fd, &aouthdr, sizeof aouthdr );
	exec_data->x_omagic = aouthdr.o_magic;
	exec_data->x_entloc = aouthdr.o_entloc;
	exec_data->x_tstart = aouthdr.o_tstart;
	exec_data->x_dstart = aouthdr.o_dstart;

	dprintf(
	  "execdata = fmagic=%x omagic=%x entloc=%lx tstart=%lx dstart=%lx  ",
		  exec_data->x_fmagic,exec_data->x_omagic,exec_data->x_entloc,
		  exec_data->x_tstart,exec_data->x_dstart);

	/*
	 * Next, read the section headers; there had better be at
	 * least three: .text, .data and .bss. The shared library
	 * section is optional; initialize the number needed to 0.
	 */

	exec_data->x_ntscns = 0;
	exec_data->x_nshlibs = 0;
	exec_data->x_lsize = 0;

	seekto( fd, (long)sizeof(filhdr) + filhdr.f_opthdr );
	dprintf("filhdr.f_nscns=%x  ", filhdr.f_nscns);

	for (xsp = exec_data->x_scn, n = filhdr.f_nscns; --n >= 0; ) {

		readfrom( fd, &scnhdr, sizeof scnhdr );

		switch ((short) scnhdr.s_flags) {
		    case STYP_TEXT:
			exec_data->x_ntscns++;  /* incr text scn cnt */
			break;  /* switch */
		    case STYP_DATA:
			if ( vtoi(scnhdr.s_vaddr) < firstds )
				firstds = vtoi(scnhdr.s_vaddr);
			if ( vtoi(scnhdr.s_vaddr) > lastds )
				lastds = vtoi(scnhdr.s_vaddr);
			dataxsp = xsp;
			break;  /* switch */
		    case STYP_BSS:
			if ( vtoi(scnhdr.s_vaddr) < firstds )
				firstds = vtoi(scnhdr.s_vaddr);
			if ( vtoi(scnhdr.s_vaddr) > lastds )
				lastds = vtoi(scnhdr.s_vaddr);
			bssxsp = xsp;
			break;  /* switch */
		    case STYP_LIB:
			exec_data->x_nshlibs = scnhdr.s_paddr;
			exec_data->x_lsize = scnhdr.s_size;
			exec_data->x_loffset = scnhdr.s_scnptr;
			continue; /* for loop */

		    default:    /* ignore other section types */
			continue; /* for loop */
		}

		/* common text, data and bss processing */
		if (scnhdr.s_size > NBPS) {     /* check segment size */
			emprintf("section too big\n");
			exit(1);
		}
		nscns++;
		scns |= (xsp->xs_flags = (short) scnhdr.s_flags);
		xsp->xs_vaddr = scnhdr.s_vaddr;
		xsp->xs_offset = scnhdr.s_scnptr;
		xsp->xs_size = scnhdr.s_size;
		xsp++;
	}

	exec_data->x_nscns = nscns;
	dprintf("x_nscns=%x  ", exec_data->x_nscns);

	/* check that text, data and bss sections were found */
	if (scns != (STYP_TEXT|STYP_DATA|STYP_BSS)) {
		emprintf("missing TEXT, DATA, or BSS\n");
		exit(1);
	}

	/* check small model data and bss sections for consistency */
	if (filhdr.f_magic == I286SMAGIC
	    && (nscns != exec_data->x_ntscns + 2
		|| dataxsp->xs_vaddr != exec_data->x_dstart
		|| hiword(dataxsp->xs_vaddr) != hiword(bssxsp->xs_vaddr)
		|| dataxsp->xs_vaddr + dataxsp->xs_size != bssxsp->xs_vaddr
		|| (bssxsp->xs_vaddr & SOFFMASK) + bssxsp->xs_size > NBPS)
	   ) {
		emprintf("Error in header:\n");
emprintf(  "nscns %d, ntscns %d\n", nscns, exec_data->x_ntscns );
emprintf(  "dataxsp->xs_vaddr %x, exec_data->x_dstart %x\n",
	 dataxsp->xs_vaddr, exec_data->x_dstart );
emprintf(  "datasel %x, bsssel %x\n",
		hiword(dataxsp->xs_vaddr), hiword(bssxsp->xs_vaddr) );
emprintf(  "dvaddr %x, dsize %x, bvaddr %x\n",
		dataxsp->xs_vaddr, dataxsp->xs_size, bssxsp->xs_vaddr );
emprintf(  "bvaddr %x, SOFFMASK %xm, bsize %x\n",
		bssxsp->xs_vaddr, SOFFMASK, bssxsp->xs_size );
		exit(1);
	}

	/*
 	 * Check total memory requirements (in clicks) for a new process
	 * against the available memory or upper limit of memory allowed.
	 */

	exec_data->x_ip = 0;
}
