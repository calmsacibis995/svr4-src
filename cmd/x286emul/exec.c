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

#ident	"@(#)x286emul:exec.c	1.1"

/* 
 * Here we imitate the Xenix kernel's exec system call program loading.
 * The x.out file is mapped twice, once for executable code and once for
 * data.  The first data segment contains the stack in a Xenix program.
 * This segment is copied into a 64k array on the 386 stack, so the 286
 * stack is actually contined in the middle of the 386 stack.
 */

#include "vars.h"
#include "sysent.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysi86.h>
#include <sys/immu.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/seg.h>

#define MAP_PRIVATE 2			/* New SVR4.0 Mem Mgmt  */
extern unsigned char JamArea[127];
int Jam_cs, Sig_cal;		/* selector:offset of 286 jam sig_cal vector */

i286exec( prog, argc, argv, envp )
	char * prog;		/* 286 program */
	int    argc;		/* how many arguments */
	char * argv[];		/* the argument strings */
	char * envp[];		/* the environment strings */
{
	long	MapFile();	/* function to map file */
	int	sp286;		/* 286 stack pointer from makestack() */
	int	firstText;	/* index of first test segment */
	int	i,j;		/* for loops */
	void	PatchJam();
	long	dzerosz = 0;	/* for demand zero fill data */
	long	lastpos = 0;	/* index of data seg whose filpos is greatest */

	My_PID = getpid();	/* our PID for use at fork time */

	InitSignals();		/* handle any inherited signals */

	Stacksize = 0x4000;	/* stack size when not mentioned in header */
				/* Was 0x1000; changed in SVR4.0 */

		/* map whole file for text */
	Tfsize = MapFile( prog, &Tfile, 0, 0, 0 );
	if ( Tfsize <= 0 ) {
		emprintf( "Can't map file %s\n", prog );
		exit(1);
	}

		/* setup pointers to x.out structs */
	X = (struct xexec *)Tfile;
	Xe = (struct xext *)(Tfile + sizeof(*X));
	Xs = (struct xseg *)(Tfile + Xe->xe_segpos);
	if ( Xe->xe_segsize % sizeof(*Xs) != 0 ) {
		emprintf( "I don't understand the segment table\n" );
		exit(1);
	}
	Xnumsegs = Xe->xe_segsize / sizeof(*Xs);
#ifdef DEBUG
	fprintf( dbgfd, "Xnumsegs = %d\n", Xnumsegs);
#endif

#if 0
		/* find last data segment */
	for ( i = Xnumsegs - 1; i >= 0; i-- ) {
		if ( (Xs+i)->xs_type == XS_TDATA ) break;
	}
#endif
		/* Find position of data segment whose filpos is greatest,
		 * and whose physical size != 0.
		 * Keep track of virtual sizes for those data segments whose
		 * physical size == 0.
		 */
	for ( i = Xnumsegs - 1; i >= 0; i-- ) {
		if ( (Xs+i)->xs_type == XS_TDATA ) {
			if (Lastdseg == 0) {
				Lastdseg = (Xs+i)->xs_seg;
				lastpos = i;
			}
			if ((Xs+i)->xs_psize == 0)
				dzerosz += (Xs+i)->xs_vsize;
			else
			if ((Xs+lastpos)->xs_filpos < (Xs+i)->xs_filpos) {
				lastpos = i;
			}
		}
	}

#ifdef DEBUG
	fprintf( dbgfd, "last data segment is 0x%x\n", Lastdseg);
	fprintf( dbgfd, "data segment with greatest filpos is index %d\n", 
			lastpos);
#endif
#if 0
		/* map data as a PRIVATE segment */
	if (i >= 0) {
		int fsize = (Xs+i)->xs_filpos + (Xs+i)->xs_psize;
		int rsize = (Xs+i)->xs_filpos + NBPS;

		Dfsize = MapFile( prog, &Dfile, fsize, rsize, MAP_PRIVATE );
		Lastdseg = (Xs+i)->xs_seg;
	}
#endif
		/* Map data as a PRIVATE segment.  The physical size
		 * of mapped in data is fsize (filpos of data segment
		 * furthest from start of executable file, plus physical
		 * size of that data segment in the exe file).
	 	 * The virtual size is rsize.  It is fsize plus the
		 * bss (demand zero fill) data in that data segment plus
		 * the virtual size of all data segments that are
		 * totally implicit bss (psize = 0, vsize > 0).
		 */
	if (Lastdseg != 0) {
		int fsize = (Xs+lastpos)->xs_filpos + (Xs+lastpos)->xs_psize;
		int rsize = (Xs+lastpos)->xs_filpos + (Xs+lastpos)->xs_vsize 
				+ dzerosz + NBPS;

		Dfsize = MapFile( prog, &Dfile, fsize, rsize, MAP_PRIVATE );
	}

	/* Turn off special read access now that we have mapped the file */
	if (sysi86(SI86EMULRDA, 0) == -1) {
		emprintf("sysi86(SI86EMULRDA) failed\n");
		exit(1);
	}

	SetupSysent();

		/* tell MP kernel what kind of Xenix to emulate */
	if ( ( (X->x_renv & XE_VERS) == XE_OSV) &&
				(Xe->xe_osvers == XE_OSXV5) ) {
			/* sys V x.out */
		BADVISE_flags = SI86B_XOUT;
	} else {
			/* before sys V */
		BADVISE_flags = SI86B_XOUT | SI86B_PRE_SV;
	}
	sysi86(SI86BADVISE, SI86B_SET | BADVISE_flags);

	if ( X->x_renv & XE_FS )	/* stack size provided in x.out */
		Stacksize = Xe->xe_stksize;

	if ( X->x_renv & XE_LTEXT )	/* large model text */
		Ltext = 1;
	if ( X->x_renv & XE_LDATA )	/* large model data */
		Ldata = 1;

	/*
	 * Map in all of the segments.  Use Stacksel and firstText to determine
	 * if we are working on the first data segment, which contains the
	 * stack, and if this is an impure binary, which has no explicit text
	 * segment entry in the x.out segment table (respectively).  Stacksel
	 * is a selector and firstText is an index.
	 */
	Stacksel = 0;
	firstText = -1;

	for ( i = 0; i < Xnumsegs; i++ ) {
		struct xseg * xp;

		
		if (((Xs+i)->xs_attr & XS_AMEM) == 0)
			continue;	/* skip if not memory image */

		switch( (xp = Xs+i)->xs_type ) {

		case XS_TTEXT:
			if (firstText < 0) firstText = i;
			validseg(xp->xs_seg);
			setsegdscr(xp->xs_seg, Tfile+xp->xs_filpos,
				xp->xs_vsize, xp->xs_vsize, 0 );
			break;

		case XS_TDATA:
			validseg(xp->xs_seg);
			if ( Stacksel == 0 ) {	/* first data seg has stack */
				copymem( Dfile+xp->xs_filpos, TheStack,
					xp->xs_psize );
				setsegdscr( xp->xs_seg, TheStack,
					xp->xs_vsize + Stacksize, NBPS, 1 );
				Stackbase = xp->xs_vsize;
				Stacksel = xp->xs_seg;

				/*
				 * The 32-bit stack alias (USER_FPSTK) must be
				 * setup for use by the x.out floating point
				 * emulator.
				 */
				setsegdscr( USER_FPSTK, TheStack,
					btoc( Stackbase + Stacksize ), 0, 3 );

				/*
				 * Impure binaries don't have a text segment.
				 * We must map this data segment into the
				 * segment implied by the entry point address.
				 */
				if (firstText < 0) {
					if (Dsegs[SELTOIDX(Xe->xe_eseg)].base
							== BAD_ADDR) {
						setsegdscr(Xe->xe_eseg,
							Tfile+xp->xs_filpos,
							xp->xs_vsize,
							xp->xs_vsize, 0 );
					}
				}
				break;
			}
			/*
			 * segment is not the stack
			 */
			if ( xp->xs_psize != xp->xs_vsize  ) {
				char *mem;

				if ( !(mem = getmem(NBPS)) ) {
					emprintf("Out of memory!\n");
					exit(1);
				}
				copymem(Dfile+xp->xs_filpos, mem, xp->xs_psize);
				setsegdscr(xp->xs_seg, mem, xp->xs_vsize, NBPS, 1);
			} else {
				setsegdscr(xp->xs_seg, Dfile+xp->xs_filpos,
					xp->xs_vsize, xp->xs_vsize, 1 );
			}
			break;

		default:
			/* ignore things that are not text or data */
			break;
		}
	}

	/*
	 * Put signal return values into easy to use globals.
	 * PatchJam() will finish setting Sig_cal by adding
	 * correct offset into jam area.
	 */
	Jam_cs = Xe->xe_eseg;
	Sig_cal = X->x_entry;

	/* install the jam area on top of the entry point */
	PatchJam(Dsegs[SELTOIDX(Xe->xe_eseg)].base+X->x_entry);
	PatchMem(Dsegs[SELTOIDX(Xe->xe_eseg)].base+X->x_entry,
		sizeof(JamArea), JamArea);

	sp286 = makestack( TheStack + Stackbase + Stacksize, argc, argv, envp );
	run286( sp286 & 0xFFFF, MAKEPTR(Xe->xe_eseg, X->x_entry) );
}


long MapFile( name, base, fsize, rsize, segtype )
	char * name, ** base;
	int fsize, rsize, segtype;
{
	struct stat s;
	struct mmf m;

	if ( stat( name, &s ) < 0 ) {
		emprintf( "Stat of %s failed\n", name );
		return -1;
	}
	if ( s.st_size == 0 )
		return 0;

	m.mf_filename = name;
	m.mf_flags = segtype;
	m.mf_filesz = fsize ? fsize : s.st_size;
	m.mf_regsz = rsize ? rsize : m.mf_filesz;

#ifdef DEBUG
	fprintf( dbgfd, "MapFile(fsize 0x%x, rsize 0x%x)\n",
			m.mf_filesz, m.mf_regsz);
#endif
	if ( (int)(*base = (char *)sysi86(SI86SHFIL, &m)) == -1 ) {
		emprintf("Can't map %s with segment flag 0x%x\n", name, segtype);
		perror("x286emul");
		return -1;
	}
#ifdef DEBUG
	fprintf( dbgfd, "MapFile: base= 0x%x\n", *base);
#endif
	return m.mf_regsz;
}

validseg( seg )
	int seg;
{
	int i = SELTOIDX(seg);
	if (i >= Numdsegs) moredsegs(i-Numdsegs+1);
}

/*
 * Jam area for Xenix 286 applications
 */

unsigned char JamArea[127] = {
/*	-- Jam Code --				  -- Jam Binary --	*/
/*		jmp	.+128		*/	0xeb,0x7e,
/*__syscal:	jmp	sys_cal		*/	0xeb,0x11,
/*__stkgro:	jmp	stk_grow	*/	0xeb,0x0c,
/*__stkset:	jmp	stk_set		*/	0xeb,0x0a,
/*__sigcal:	jmp	sig_call	*/	0xeb,0x14,
/*		.value	0,0,0,0		*/	0,0,0,0,0,0,0,0,
/*stk_set:				*/
/*stk_grow:				*/
/*		mov	$2088,%ax	*/	0xb8,0x28,0x08,
/*sys_cal:				*/
/*		data32			*/
/*		lcall	$0x17,$XXXcalla	*/	0x66,0x9a,0,0,0,0,0x17,0x00,

/*;This point is at offset 23 from start of JamArea ------^		*/
#define SE	23

/*		RET			*/	0,
#define RET1	29

/*sig_call:				*/
/*		data32			*/
/*		lcall	$0x17,$restore	*/	0x66,0x9a,0,0,0,0,0x17,0x00,

/*;This point is at offset 32 from start of JamArea ------^		*/
#define SR	32

/*		RET			*/	0,
#define RET2	38

};

/*
 * Emulator/286 program interfaces
 */
int smallcalla(), largecalla(), restore();
#define BYTE(x,i)	((((unsigned long) x) >> (i<<3)) & 0xff)
#define SYSENT(i)	(Ldata ? BYTE(largecalla, i) : BYTE(smallcalla, i))
#define SIGRET(i)	BYTE(restore, i)

void PatchJam(actualJam)
	char * actualJam;
{
	Sig_cal += SR-2;	/* add sig_cal offset into jam */

		/*
		 * Install system call entry point into the jam area.  The
		 * entry point varies between small and large model because
		 * the system call parameter passing varies between the two.
		 */
	JamArea[SE] = SYSENT(0);
	JamArea[SE+1] = SYSENT(1);
	JamArea[SE+2] = SYSENT(2);
	JamArea[SE+3] = SYSENT(3);
		/* Install the signal return handler's address */
	JamArea[SR] = SIGRET(0);
	JamArea[SR+1] = SIGRET(1);
	JamArea[SR+2] = SIGRET(2);
	JamArea[SR+3] = SIGRET(3);
		/*
		 * Fix the return sequences to match the x.out's model.  The
		 * correct return instruction is available in the x.out as
		 * the last byte of the 128 byte jam area.
		 */
	JamArea[RET1] = JamArea[RET2] = actualJam[127];
}

/*
 * Create the argv and envp structs for the 286 program.
 */
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

#ifdef DEBUG
	fprintf( dbgfd, "end of stack at 386 %x\n", end );
#endif

	nc = 0;
	for ( ne = 0; ep[ne] != 0; ne++ ) {
		nc += strlen(ep[ne]) + 1;
	}

#if 0
	if ( na < 2 ) {
		emprintf(  "bad arg count\n" );
		exit(1);
	}
#endif

	for ( na = 0; ap[na+1] != 0; na++ ) {
		nc += strlen(ap[na]) + 1;
	}
	if ( na < 1 ) {
		emprintf(  "bad arg count\n" );
		exit(1);
	}
	ap[na] = 0;

	sb = (unsigned)end - 2 - nc;
	sb &= ~1;               /* make sure it is even */

	pb = sb - 6 - (na + ne)*2;
	if ( Ldata )
		pb -= ( na + ne + 2 ) * 2;


#ifdef DEBUG
	fprintf( dbgfd, "end of stack at 386 %x, pb now %x, sb now %x\n", end, pb, sb );
#endif
	sp286 = (unsigned)pb - (unsigned)TheStack;
#ifdef DEBUG
	fprintf( dbgfd, "286 stack offset for argc: %x\n", sp286 );
#endif

	pb = wap( pb, na );     /* argc */
	for ( i = 0; ap[i]; i++ ) {
		int off286;

		off286 = (unsigned)sb - (unsigned)TheStack;
		pb = wap( pb, off286 );
		if ( Ldata )
			pb = wap( pb, Stacksel );
		sb = sap( sb, ap[i] );
	}
	pb = wap( pb, 0 );
	if ( Ldata )
		pb = wap( pb, 0 );


	for ( i = 0; ep[i]; i++ ) {
		int off286;

		off286 = (unsigned)sb - (unsigned)TheStack;
		pb = wap( pb, off286 );
		if ( Ldata )
			pb = wap( pb, Stacksel );
		sb = sap( sb, ep[i] );
	}
	pb = wap( pb, 0 );
	if ( Ldata )
		pb = wap( pb, 0 );

	return sp286;
}

sap( dest, src )
	int dest;
	char * src;
{
#ifdef DEBUG
	fprintf( dbgfd, "put <%s> at %x ( 286 %x )\n", src, dest, dest-(unsigned)TheStack );
#endif
	while ( *(char *)(dest++) = *src++ )
		;
	return dest;
}

wap( dest, value )
	int dest;
	int value;
{
#ifdef DEBUG
	fprintf( dbgfd, "put %x at %x ( 286 %x )\n", value, dest, dest-(unsigned)TheStack );
#endif
	*(unsigned short *)dest = value & 0xFFFF;
	return dest + 2;
}
