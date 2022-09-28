/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:i386/expand2.h	1.2"

typedef enum
{
        IT_UJMP,         /* unconditional jump */
        IT_CJMP,         /* conditional jump */
	IT_LOOP,	/* loop */
        IT_LABEL,       /* label definition */
        NITYPE
} ITYPE;

#define NITYPES	3	/* number of sdi types */

#define CJMP 0
#define UJMP 1
#define LOOPI 2

typedef struct
{
        unsigned int    it_binc,        /* byte->half increment */
                        it_hinc;        /* half->word increment */
} ITINFO;

extern ITINFO   itinfo[];


/*      The sizes below are for span-dependent optimizations.
 *
 */

#define INSTR_BSZ	 3
#define UJMP_BSZ         2
#define UJMP_WSZ         6
#define CJMP_BSZ         2
#define CJMP_WSZ         6
#define LOOP_BSZ	 2
#define LOOP_WSZ	10
#define SPAN_BLO        (-128)
#define SPAN_BHI        (127)
#define SPAN_HLO        (-32768L)
#define SPAN_HHI        32767L
