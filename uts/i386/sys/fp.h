/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_FP_H
#define _SYS_FP_H

#ident	"@(#)head.sys:sys/fp.h	1.1.2.1"

/*
 * 80287/80387 floating point processor definitions
 */

/*
 * values that go into fp_kind
 */
#define FP_NO   0       /* no fp chip, no emulator (no fp support)      */
#define FP_SW   1       /* no fp chip, using software emulator          */
#define FP_HW   2       /* chip present bit                             */
#define FP_287  2       /* 80287 chip present                           */
#define FP_387  3       /* 80387 chip present                           */

/*
 * masks for 80387 control word
 */
#define FPINV   0x00000001      /* invalid operation                    */
#define FPDNO   0x00000002      /* denormalized operand                 */
#define FPZDIV  0x00000004      /* zero divide                          */
#define FPOVR   0x00000008      /* overflow                             */
#define FPUNR   0x00000010      /* underflow                            */
#define FPPRE   0x00000020      /* precision                            */
#define FPPC    0x00000300      /* precision control                    */
#define FPRC    0x00000C00      /* rounding control                     */
#define FPIC    0x00001000      /* infinity control                     */
#define WFPDE   0x00000080      /* data chain exception                 */

/*
 * precision, rounding, and infinity options in control word
 */
#define FPSIG24 0x00000000      /* 24-bit significand precision (short) */
#define FPSIG53 0x00000200      /* 53-bit significand precision (long)  */
#define FPSIG64 0x00000300      /* 64-bit significand precision (temp)  */
#define FPRTN   0x00000000      /* round to nearest or even             */
#define FPRD    0x00000400      /* round down                           */
#define FPRU    0x00000800      /* round up                             */
#define FPCHOP  0x00000C00      /* chop (truncate toward zero)          */
#define FPP     0x00000000      /* projective infinity                  */
#define FPA     0x00001000      /* affine infinity                      */
#define WFPB17  0x00020000      /* bit 17                               */
#define WFPB24  0x01000000      /* bit 24                               */

/*
 * masks for 80387 status word
 */
#define FPS_ES	0x00000080      /* error summary bit                    */

extern char fp_kind;            /* kind of fp support                   */
extern struct proc *fp_proc;    /* process that owns the fp unit        */

/*
 * values for fp_vers
 */
#define	FP_COFF		1
#define	FP_XOUT		2
/*  Since Elf or Coff Format Emulator behaves/works in the same way.
 */
#define FP_ELF		1

extern char fp_vers;		/* used to indicate how to map u-area	*/

#define EMUL_START	0x15

#endif	/* _SYS_FP_H */
