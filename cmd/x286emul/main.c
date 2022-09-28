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

#ident	"@(#)x286emul:main.c	1.1"

#define	MAIN
#include "vars.h"
#include <sys/sysi86.h>

main( argc, argv, envp )
	int   argc;
	char *argv[];
	char **envp;
{
	unsigned short space_for_the_segment_containing_the_stack[ NBPS/2 ];
	int i;
	char *syst;
	extern char *getenv();

#ifdef TRACE
	/*
	 * When we are built with -DTRACE, setting the environment variable
	 * SYSTRACE=Y enables system call and memory mapping debugging output.
	 */
	if ((syst = getenv("SYSTRACE")) && syst[0]=='Y')
		systrace++;
#endif

#if defined(TRACE) || defined(DEBUG)
	/*
	 * For debugging output, we don't want to disturb any of the program's
	 * conceptions about what file descriptors do what.  So we open a
	 * bunch of them to push our fd well out of normal reach.
	 */
	if (systrace && (dbgfd == stdout) ) {
		int fdsav[100], i;

		for(i=0; i < 100; i++) {
			if ( (fdsav[i] = dup(1)) < 0) {
				break;
			}
		}
		if (i > 0) {
			dbgdesc = fdsav[--i];	/* cache descriptor for Close */
			dbgfd = fdopen(fdsav[i], "r+");
			setbuf(dbgfd, NULL);	/* turn off buffering */
			fprintf(stderr, "x286emul:  using fd %d for output\n", fdsav[i]);
		}
		while(i-- > 0) {
			close(fdsav[i]);
		}
	}
#endif

	Pgmname = argv[0];

	TheStack = (char *)&space_for_the_segment_containing_the_stack[0];
	for ( i = 0; i < NBPS/2; i++ )
		TheStack[i] = 0;

	if ( ! moredsegs( 32 ) ) {
		emprintf( "Out of memory\n" );
		exit(1);
	}

	SetupFloat();
	i286exec( argv[argc-1], argc, argv, envp );
}

/*
 * Some Xenix binaries preceed floating point instructions with an INT
 * instruction.  The Xenix kernel, when it gets the interrupt, emulates
 * the following floating point instructions if there is no floating point
 * hardware.  If there is floating point hardware, it patches out the INT
 * instruction and lets the program proceed.
 *
 * We always patch out the INT instruction, since the MP kernel will deal
 * directly with the floating point instructions if there is no FP hardware
 */


SetupFloat() {
	extern IDTTarget();

	if ( -1 == sysi86( SI86CHIDT, IDTTarget ) ) {
		emprintf( "Warning: IDT modification for FLOAT failed\n" );
		return -1;
	}
}

#define	I_INT		(0xCD)		/* the INT instruction */
#define	I_FWAIT		(0x9B)		/* FWAIT instruction to replace INT */

#define	EMWAIT		(0xF9)		/* Value that indicates that a... */
#define	I_NOP		(0x90)		/* ...NOP instruction is needed */

#define	EMESOVER	(0xF8)		/* Value that indicates that a... */
#define	I_ESOVER	(0x26)		/* ...ES override is needed */

#define	EMSSOVER	(0xFA)		/* Value that indicates that a... */
#define	I_SSOVER	(0x36)		/* ...SS override is needed */

#define	EMESCN		(0x07)
#define	I_ESC		(0xd8)

/* 
 * The arguments to FloatCommon are the registers saved by IDTTarget
 * and the flags and cs:ip saved by the interrupt gate in the IDT.
 * Any changes made to the arguments by FloatCommon will affect
 * the 286 program when IDTTarget restores the saved regsiters
 * and does an iret.
 */
FloatCommon( dxax, spbx, cxsi, dies, dsbp, ip, cs, flags )
	unsigned long dxax, spbx, cxsi, dies, dsbp;
	unsigned long ip, cs, flags;
{
	unsigned char * pint;

	/*
	 * ip is two bytes passed the instruction that caused the problem
	 * We subtract two to find the correct instruction, and so that
	 * IDTTarget will return to the correct place ( remember, changes
	 * to ip here get propogated back to the 286 program! )
	 */
	ip -= 2;
	pint = (unsigned char *)cvtptr( MAKEPTR(cs, ip) );
	if ( pint[0] == I_INT ) {
		char new[3];
		int len;

		new[0] = I_FWAIT;
		len = 1;

		switch ( pint[1] ) {
	case EMWAIT:
			new[len++] = I_NOP;
			break;
	case EMESOVER:
			new[len++] = I_ESOVER;
			goto IF0;
	case EMSSOVER:
			new[len++] = I_SSOVER;
	IF0:
	default:
			new[len] = I_ESC | (pint[len] & EMESCN);
			len++;
		}
		PatchMem( pint, len, new );
	} else {
		emprintf( "Unexpected call to FloatCommon\n" );
		exit(1);
	}
}
