/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)i286emu:exec.c	1.1"

#include <stdio.h>
#include <fcntl.h>
#include <sys/seg.h>
#include <sys/sysi86.h>
#include "vars.h"
#include <sys/stat.h>
#include "i286sys/exec.h"

extern int errno;

#define	vtosel(a) (((a)>>16)&0xFFFF)
#define vtoselx(a) (((a)>>19)&0x1FFF)

i286exec( i286prog, argc, argv, envp )
	char * i286prog;
	int argc;
	char **argv, **envp;
{
	char    *vwinadr;       /* KVA of window into args and shlib buffer */
	unsigned short  vwinsz;         /* size of window in clicks */
	unsigned short  shlb_scnsz;     /* size of shlib section buffer */
	unsigned short  shlb_datsz;     /* size of shared lib exdata in buffer */
	unsigned short  *shlb_buf;      /* addr of shlib section buffer */
	struct  exdata *shlb_dat;       /* addr of shared lib exdata */

	char    *userstack;     /* VA of high end of user stack */
	int     newsp;          /* initial user stack pointer */

	short   i;              /* index */
	struct exdata exdata;   /* buffer for primary executable file data */
	unsigned short  largemodel;     /* large model flag */

	int aoutfd;             /* file desc. for a.out file */
	int rc;

	aoutfd = open( i286prog, O_RDONLY );
	if ( aoutfd < 0 ) {
		emprintf(  "Can't open %s\n", i286prog );
		exit(1);
	}

	/* Turn off special read access now that we have a valid file dscr. */
	dprintf("i286exec calling sysi86(SI86EMULRDA, 0)\n");
	rc = sysi86(SI86EMULRDA, 0);
	dprintf("sysi86() returned %d\n", rc);
	if (rc) {
		emprintf( "i286exec: sysi86(SI86EMULRDA) failed\n");
		exit(1);
	}

	firstds = 0x7FFF;
	lastds = 0;
	gethead( aoutfd, &exdata );

	/*
	 * Check the magic number from "optional" unix header
	 *
	 *  410 is RO shared text
	 *  443 is a shared library     ??? 440 ???
	 */
	switch (exdata.x_omagic) {
	    case 0410:
		break;
	    default:
		emprintf(  "bad magic number\n" );
		exit(1);
		break;
	}

	/*
	 *      Allocate memory to read the shared library
	 *      section, and the amount of memory needed to store the
	 *      exdata for each a.out.
	 *
	 */

#define NBPW    2
	shlb_scnsz = (exdata.x_lsize + NBPW) & (~(NBPW - 1));
	shlb_datsz = exdata.x_nshlibs * sizeof(struct exdata);
	vwinsz = ctob(btoc((long)shlb_scnsz + (long)shlb_datsz));
dprintf(
"exece: x_lsize=%x x_nshlibs=%x shlb_scnsz=%x shlb_datsz=%x vwinsz=%x  ",
exdata.x_lsize,exdata.x_nshlibs,shlb_scnsz,shlb_datsz,vwinsz);

	/* get the memory */

	vwinadr = getmem( vwinsz );

	/*	Locate and verify any needed shared libraries.
	 */

	if (exdata.x_nshlibs) {
		emprintf(  "shlib: not yet!\n" );
		exit(1);
#ifdef NOTDEF
		shlb_buf = (unsigned short *) (vwinadr);
		shlb_dat = (struct exdata *)((char *) shlb_buf + shlb_scnsz);

		if (getshlibs(&exdata, shlb_buf, shlb_dat)) {
			DEBUG5((printf("exece(): getshlibs failed  ")));
			goto done;
		}
#endif
	}

	uu_model = (exdata.x_fmagic == I286LMAGIC)
		     ? U_MOD_LARGE : U_MOD_SMALL;
dprintf( "model is %s\n", uu_model == U_MOD_LARGE ? "LARGE" : "SMALL" );

	/*
	 * load any shared libraries that are needed
	 */

	for (i = exdata.x_nshlibs; --i >= 0; shlb_dat++) {
		emprintf(  "shlib load: not yet\n" );
		exit(1);
#ifdef  NOTDEF
		if (getxfile(shlb_dat, PT_LIBTXT, PT_LIBDAT)) {
			exec_err(&exdata, ++shlb_dat, i);
			goto kill;
		}
#endif
	}

	/*
	 * load the primary executable file's text and data.
	 */

	getxfile(aoutfd,&exdata);

	/*
	 *	Set up the user's stack.
	 */
dprintf( "stack at 386 %x\n", stackbase );
	newsp = makestack( (char *)stackbase + (uu_model ? 65536 : smssize),
			argc, argv, envp );

	close( aoutfd );
	free( vwinadr );

dprintf( "firstds = 0x%x\n", firstds );

	if ( uu_model )
		stacksegsel = 0x4f;
	else
		stacksegsel = firstds * 8 + 7;

	setsegdscr( stacksegsel, stackbase, 65536, 1 );

	/*
	 * The 32-bit stack alias (USER_FPSTK) must be
	 * setup for use by the x.out floating point
	 * emulator.
	 *
	 * WARNING!!!  The value 16 used in the following
	 * call to setsegdscr() is the number of 4K pages
	 * needed for the stack (e.g. btoc( 65536 )).  The
	 * system macro btoc() could not be used as btoc()
	 * has been defined in ./vars.h to mean something
	 * else.
	 */
	setsegdscr( USER_FPSTK, stackbase, 16, 3 );

	if ( ! uu_model ) {
		int index;
		char * ip;
		/*
		 * patch text at 0 for small model signal handling
		 */
		index = ((int)exdata.x_entloc >> 19) & 0x1FFF;
		ip = dsegs[index].base;
		ip[3] = 0xcb;		/* long return (286) */
	}

	run286( newsp & 0xFFFF, (char *)exdata.x_entloc );

	return newsp;
}

getxfile(fd,exec_data)
	int fd;
	struct exdata *exec_data;
{
	struct xscn *dataxsp;   /* data section structure pointer */
	struct xscn *bssxsp;    /* bss section structure pointer */
	struct xscn *xsp;       /* section structure pointer */
	short n;                /* count index */

	/*
	 *      Load text
	 */

	xalloc(fd,exec_data);

	/*
	 *      Load data regions.
	 *
	 *      Small model is treated as a special case
	 *      because the single data region contains the stack
	 *      and the data and bss sections.
	 *
	 *      In large model, each section is a separate region.
	 */

	switch (uu_model) {

	    case (U_MOD_SMALL):

		/* NOTE: small model data and bss section consistency
		   checking was done in gethead() */

		/* find the data and bss sections */
		for (xsp = exec_data->x_scn, n = exec_data->x_nscns;
		     --n >= 0; xsp++ ) {
			switch (xsp->xs_flags) {
			    case STYP_TEXT:
				break;
			    case STYP_DATA:
				dataxsp = xsp;
				break;
			    case STYP_BSS:
				bssxsp = xsp;
				break;
			    default:
				emprintf(  "small model bad section type");
				exit(1);
			}
		}
		getregion(fd,exec_data, dataxsp);
		smdsize = ctob(btoc(smssize + dataxsp->xs_size + bssxsp->xs_size));
		break;

	    case (U_MOD_LARGE):

		/* find the data and bss sections */
		for (xsp = exec_data->x_scn, n = exec_data->x_nscns;
		     --n >= 0; xsp++ )
			switch (xsp->xs_flags) {
			    case STYP_TEXT:
			    case STYP_LIB:
				break;
			    case STYP_DATA:
			    case STYP_BSS:
				getregion(fd,exec_data, xsp);
				break;
			    default:
				emprintf("large model wild section type");
				exit(1);
			}
	}
}


/*
 * Get a data or bss region.
 * Inode is locked at entry and at normal exit, and is unlocked on error.
 */

getregion(fd,exec_data, xsp)
	int fd;
	struct exdata *exec_data;       /* executable file data */
	struct xscn *xsp;               /* executable file section data */
{
	char * database;                /* where data starts */

	if ( uu_model ==  U_MOD_SMALL ) {
		smssize = xsp->xs_vaddr & 0xFFFF;
		database = (char *)stackbase + smssize;
dprintf( "small model stack is 0x%x bytes\n", smssize );
		seekto( fd, xsp->xs_offset );
		readfrom( fd, database, xsp->xs_size );
	} else {
		int segsize;
		segsize = ctob(btoc(xsp->xs_size));
		database = getmem( segsize );
dprintf( "large model data region of %d bytes\n", xsp->xs_size );
		if (xsp->xs_flags != STYP_BSS) {
			seekto( fd, xsp->xs_offset );
			readfrom( fd, database, xsp->xs_size );
		}
		setsegdscr( vtosel(xsp->xs_vaddr),database, segsize, 1 );
	}
}

#ifdef NOTDEF
/*
 * getshlibs - reads the shared library section in the primary executable
 * file into the buffer area pointed to by bp.  This section contains the
 * path names of the shared libraries.
 * Each shared library file is opened and gethead called to read its
 * header information into an exdata struct in the dat_start array.
 */

getshlibs(exec_data, bp, dat_start)
	struct exdata *exec_data;
	unsigned long *bp;
	struct exdata *dat_start;
{
	struct inode *ip;       /* pointer to inode */
	struct exdata *dat;     /* pointer to current exdata struct */
	unsigned short nlibs;           /* number of shared libraries */
	unsigned short n;               /* library count */
	unsigned short rem_wrds;        /* words remaining */

	DEBUG5((printf("getshlibs(exec_data=%lx bp=%lx dat_start=%lx)  ",
		       exec_data, bp, dat_start)));

	/* initialization */
	ip = exec_data->x_ip;
	dat = dat_start;
	nlibs = exec_data->x_nshlibs;
	n = 0;

	/* read shared lib section */
	u.u_base = (caddr_t)bp;
	u.u_count = exec_data->x_lsize ;
	u.u_offset = exec_data->x_loffset ;
	u.u_segflg = 1;
	DEBUG9((printf("u_offset=%lx u_count=%x   ",
			u.u_offset, u.u_count)));

	FS_READI(ip);

	if (u.u_count != 0) {
		DEBUG5((printf("getshlibs: error #1  ")));
		iput(ip);
		return(-1);
	}

	ip->i_flag |= ITEXT;
	prele(ip);

	while ((bp < (unsigned long *)dat) && (n < nlibs)) {

		/* Check the validity of the shared lib entry. */
		/* bp[0] = size of entry in words */
		/* bp[1] = offset of pathname in words */
		DEBUG9((printf("bp[0]=%lx bp[1]=%lx   ",
			bp[0], bp[1])));

		if ((bp[0] > (rem_wrds = (unsigned long *)dat_start - bp)) ||
		    (bp[1] > rem_wrds) || (bp[0] < 3)) {
			DEBUG5((printf("getshlibs: error #2  ")));
			u.u_error = ELIBSCN;
			goto bad;
		}

		/* Locate the shared lib and get its header info.  */

/*              u.u_syscall = DUEXEC;
  */
		u.u_dirp = (caddr_t)(bp + bp[1]);
		DEBUG9((printf("pathname='%s'  ", u.u_dirp)));
		bp += bp[0];

		if ((ip = namei(spath, NULL)) == NULL) {
			DEBUG5((printf("getshlibs: error #3  ")));
			u.u_error = ELIBACC;
			goto bad;
		}

		if (gethead(ip, dat)) {
			if (u.u_error == EACCES)
				u.u_error = ELIBACC;
			else if (u.u_error != ENOMEM)
				u.u_error = ELIBBAD;
			DEBUG5((printf("getshlibs: error #4  ")));
			goto bad;
		}

		if (dat->x_omagic != 0443) {
			DEBUG5((printf("getshlibs: error #5  ")));
			u.u_error = ELIBBAD;
			iput(ip);
			goto bad;
		}

		ip->i_flag |= ITEXT;
		prele(ip);

		++dat;
		++n;
	}

	if (n != nlibs) {
		DEBUG5((printf("getshlibs: error #6  ")));
		u.u_error = ELIBSCN;
		goto bad;
	}

	return(0);
bad:
	DEBUG5((printf("getshlibs: u_error=%x  ", u.u_error)));
	exec_err(exec_data, dat_start, n);
	return(-1);
}
#endif

#define WORD(X) (*((unsigned short *)(X)))
#define LONG(X) (*((unsigned long *)(X)))

makestack( end, count, ap, ep )
	char * end;
	int count;
	char **ap, **ep;
{
	int na;         /* # of arg strings */
	int ne;         /* # of env strings */
	int nc;         /* # chars in strings */

	int sb;         /* where strings start in stack */
	int pb;         /* where pointers go */

	int i;          /* generic index */

	int sp286;      /* 286 stack offset */

dprintf( "end of stack at 386 %x\n", end );

	nc = 0;
	for ( ne = 0; ep[ne] != 0; ne++ ) {
		nc += strlen(ep[ne]) + 1;
	}
	for ( na = 0; ap[na] != 0; na++ ) {
		nc += strlen(ap[na]) + 1;
	}
	if ( na < 2 ) {
		emprintf(  "bad arg count\n" );
		exit(1);
	}
	--na;
	nc -= strlen(ap[na]) + 1;
/*        ap[0] = ap[na];       */
dprintf( "new first arg <%s>\n", ap[0] );
	ap[na] = 0;

	sb = (unsigned)end - 2 - nc;
	sb &= ~1;               /* make sure it is even */

	pb = sb - 6 - (na + ne)*2;
	if ( uu_model )
		pb -= ( na + ne + 2 ) * 2;


dprintf( "end of stack at 386 %x, pb now %x, sb now %x\n", end, pb, sb );
	sp286 = (unsigned)pb - (unsigned)stackbase;
dprintf( "286 stack offset for argc: %x\n", sp286 );

	pb = wap( pb, na );     /* argc */
	for ( i = 0; ap[i]; i++ ) {
		int off286;

		off286 = (unsigned)sb - (unsigned)stackbase;
		pb = wap( pb, off286 );
		if ( uu_model )
			pb = wap( pb, 0x4f );
		sb = sap( sb, ap[i] );
	}
	pb = wap( pb, 0 );
	if ( uu_model )
		pb = wap( pb, 0 );


	for ( i = 0; ep[i]; i++ ) {
		int off286;

		off286 = (unsigned)sb - (unsigned)stackbase;
		pb = wap( pb, off286 );
		if ( uu_model )
			pb = wap( pb, 0x4f );
		sb = sap( sb, ep[i] );
	}
	pb = wap( pb, 0 );
	if ( uu_model )
		pb = wap( pb, 0 );

	return sp286;
}

sap( dest, src )
	int dest;
	char * src;
{
dprintf( "put <%s> at %x ( 286 %x )\n", src, dest, dest-(unsigned)stackbase );
	while ( *(char *)(dest++) = *src++ )
		;
	return dest;
}

wap( dest, value )
	int dest;
	int value;
{
dprintf( "put %x at %x ( 286 %x )\n", value, dest, dest-(unsigned)stackbase );
	*(unsigned short *)dest = value & 0xFFFF;
	return dest + 2;
}

