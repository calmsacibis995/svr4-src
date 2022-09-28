/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:i386/instab.h	1.3"
/* common definitions for the generation of instructions */
/* NOTE: symbols.h must be included before this file */
#include "expr.h"

#define DBITON	0x0200
#define DBITOFF	0
#define SEGPFX	0x26
#define OSP 0x66	/* operand size prefix */
#define OP_EXT	0x0f	/* opcode extension byte */

#define EBX	0x03
#define EBP	0x05
#define ESI	0x06
#define EDI	0x07

#define BX	0x07
#define BP	0x06
#define SI	0x04
#define DI	0x05

#define REGMOD	0x03
#define NODISP	0x00
#define DISP8	0x01
#define DISP16	0x02
#define DISP32	0x02
#define EA16	0x06
#define EA32	0x05

#define BT	4
#define BTS	5
#define BTR	6
#define BTC	7

#define EXADMD	0x01	/* external address mode */
#define EDSPMD	0x02	/* displacement mode with expression */
#define DSPMD	0x04	/* displacement mode */
#define IMMD	0x08	/* 16 bit immediate mode */
#define EXPRMASK	0x03	/* mask for presence of an expression */

#define SIB_PRESENT 0x10	/* sib byte present */

#define AREG16MD	0x10	/* accumulator (16 bit) mode */
#define AREG32MD	0x10	/* accumulator (32 bit) mode */
#define AREG8MD		0x20	/* accumulator (8 bit) mode */
#define REG16MD		0x40	/* register (16 bit) mode */
#define REG32MD		0x40	/* register (32 bit) mode */
#define REG8MD		0x80	/* register (8 bit) mode */
#define	AREGMASK	0x30
#define REGMASK		0xf0

#define NORELTYPE	0x00
#define HI12TYPE	0x40
#define LO32TYPE	0x80	/* jn 11-18-85 */
#define X86TYPE		0xf0	/* jn 11-18-85, changed from 0x70 to 0xf0 */

#define INSTRB	1
#define INSTRW	2
#define RINST	3
#define SINST	4
#define EINST	5
#define AINST	6
#define IAINST	7
#define ININST	8
#define GREG16	9
#define	GREG8	10
#define SEGREG	11
#define PSEUDO	12
#define GREG32	13
#define INSTRL	14
#define MREG    15

#define AREGOPCD	0

/*
 * The numbers to define the registers in the sib byte are
 * different from the register definitions in the mod r/m byte
 */

#define SIB_EAX	0
#define SIB_ECX 1
#define SIB_EDX 2
#define SIB_EBX 3
#define SIB_EBP 5
#define SIB_ESI 6
#define SIB_EDI 7
#define SIB_ESP_BASE 4

#define ESC_TO_SIB 4
#define NO_IDX_REG 4
#define NO_BASE_REG 5




union sib_byte_tag {
  struct sibtag {
#if uts || sparc || u3b2 || u3b5 || u3b15
    unsigned dummy : 8 ;
    unsigned ss : 2 ;
    unsigned index : 3 ;
    unsigned base : 3 ;
#else
    unsigned base : 3 ;
    unsigned index : 3 ;
    unsigned ss : 2 ;
    unsigned dummy : 8 ;
#endif
  } sib ;
  unsigned short sib_byte ;
} ;


union addrmdtag {
	struct addrtag {
#if uts || sparc || u3b2 || u3b5 || u3b15
		unsigned dummy : 8;
		unsigned mod : 2;
		unsigned reg : 3;
		unsigned rm : 3;
#else
		unsigned rm : 3;
		unsigned reg : 3;
		unsigned mod : 2;
		unsigned dummy : 8;
#endif
	} addrmd;
	unsigned short addrmdfld;
};

typedef struct {
	short reltype;
	EXPR_MEMBERS
} rexpr;

typedef struct {
	BYTE admode;
	BYTE adreg;
	rexpr adexpr;
} addrmode;

#if i386
/*
 * 	80287 declarations
 */

#define FSTACK 0x33

/*
 *	type to return float/double/temp long/llong and bcd in
 */

typedef union {
		unsigned short	fvala[5] ;
		unsigned long   fvalla[3];
		unsigned long	fvall ;
		float		fvalf ;
		double		fvald ;
	} floatval ;

#endif

