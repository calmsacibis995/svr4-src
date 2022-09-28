/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:i386/gendefs.h	1.3"

/* general definitions used through-out the assembler */

#include <elf.h>

#define  LESS    -1
#define  EQUAL    0
#define  GREATER  1

#define  NO       0
#define  YES      1

#define NCPS	8	/* number of characters per symbol */
#define BITSPBY	8
#define BITSPOW	8
#define OUTWTYPE	char
#define	OUT(a,b)	putc(a,b)

#define NBPW	16

#define MIN16	(-32768L)
#define MAX16	32767L
#define MIN32	((long)0x80000000L)
#define MAX32	((long)0x7fffffffL)

#define SCTALIGN 4L	/* byte alignment for sections */
#define TXTFILL	0x90L
#define FILL 0L
#define NULLVAL 0L
#define NULLSYM ((symbol *)NULL)

/* constants used in testing and flag parsing */

#define TESTVAL	-2
#define NSECTIONS 9
#define NFILES	NSECTIONS+6

#if AR16WR
#define BITSPW 16
#else
#define BITSPW 32
#endif

#define GET_E_FLAGS 0

#define E_BYTE_ORDER ELFDATA2LSB
#define E_MACHINE EM_386



/* index of action routines in modes array */

typedef enum
{
	NOACTION,
	DEFINE,
	SETVAL,
	RELGOT32,
	RELPLT32,
	RELGOTOFF,
	SETSCL,
	ENDEF,
	DOTZERO,
	NEWSTMT,
	RESABS,
	RELPC8,
	CJMPOPT,
	UJMPOPT,
	LOOPOPT,
	SWAPB,
	LONGREL,
	PACK8,
	PACK16,
	DUMPBITS,
	HI12BITS,
	SWAPWB,
	LO32BITS,
	GOTOFF
 } ACTION;
