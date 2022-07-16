/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_REGSET_H
#define _SYS_REGSET_H

#ident	"@(#)head.sys:sys/regset.h	1.1.5.1"

/* General register access (3B2) */

typedef int greg_t;
#ifdef u3b2
#define NGREG   16
#endif

#ifdef i386
#define NGREG   19
#endif

typedef	greg_t	gregset_t[NGREG];

#ifdef u3b2
#define	R_R0	0
#define	R_R1	1
#define	R_R2	2
#define	R_R3	3
#define	R_R4	4
#define	R_R5	5
#define	R_R6	6
#define	R_R7	7
#define	R_R8	8
#define	R_FP	9
#define	R_AP	10
#define	R_PS	11
#define	R_SP	12
#define	R_PCBP	13
#define	R_ISP	14
#define	R_PC	15

/* Floating-point register access (3B2 MAU) */

typedef	struct fpregset {
	int	f_asr;
	int	f_dr[3];
	int	f_fpregs[4][3];
} fpregset_t;
#endif  /* u3b2 */

#ifdef i386
/*
 * The following picks up the defines for '386 registers.
 *
 */

#include <sys/reg.h>

typedef struct fpregset {
    union {
        struct fpchip_state     /* fp extension state */
        {
            int state[27];  /* 287/387 saved state */
            int status;     /* status word saved at exception */
        } fpchip_state;
        struct fp_emul_space        /* for emulator(s) */
        {
            char    fp_emul[246];
            char    fp_epad[2];
        } fp_emul_space;
        int f_fpregs[62];       /* union of the above */
    } fp_reg_set;
    long    f_wregs[33];            /* saved weitek state */
} fpregset_t;

#define NDEBUGREG	8

typedef struct dbregset {
	unsigned	debugreg[NDEBUGREG];
} dbregset_t;

#endif  /* i386 */


#endif	/* _SYS_REGSET_H */
