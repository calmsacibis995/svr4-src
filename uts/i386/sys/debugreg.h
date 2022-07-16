/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_DEBUGREG_H
#define _SYS_DEBUGREG_H

#ident	"@(#)head.sys:sys/debugreg.h	1.1.2.1"

/*
** Specify masks for accessing the i386 debug registers.
*/

/*
** The debug registers are found in an array (debugreg) in the u block.
** On the i386, there are 4 registers to specify linear addresses.
** dr4 and dr5 are reserved.
*/
#define DR_FIRSTADDR 0  /* u.u_debugreg[DR_FIRSTADDR] */
#define DR_LASTADDR 3   /* u.u_debugreg[DR_LASTADDR]  */

/*
** The debug status is found in dr6 after a debug trap.
*/
#define DR_STATUS 6           /* u.u_debugreg[DR_STATUS]     */
#define DR_TRAP0 0x1          /* Trap from debug register #0 */
#define DR_TRAP1 0x2          /* Trap from debug register #1 */
#define DR_TRAP2 0x4          /* Trap from debug register #2 */
#define DR_TRAP3 0x8          /* Trap from debug register #3 */
#define DR_ICEALSO   0x2000   /* Flag bit reserved for the in-circuit-emulator*/
#define DR_SINGLESTEP 0x4000  /* Trap resulting from the single-step flag */
#define DR_TASKSWITCH 0x8000  /* Trap resulting from a task-switch */

/*
** dr7 controls the rest of the debug registers.
** use shifts and masks because arrays of fields tend to get aligned.
** For example,
**    dr7 & DR_LOCAL_ENABLE_MASK
**    dr7 >> (DR_LOCAL_ENABLE_SHIFT + r# * DR_ENABLE_SIZE) & 0x1
**    dr7 >> (DR_CONTROL_SHIFT + r# * DR_CONTROL_SIZE) & DR_RW_MASK
** Note that the GLOBAL bits below and always turned off by the kernel.
*/
#define DR_CONTROL 7               /* u.u_debugreg[DR_CONTROL] */
#define DR_LOCAL_ENABLE_MASK 0x55  /* Enable all 4 regs for ldt addresses   */
#define DR_GLOBAL_ENABLE_MASK 0xAA /* Enable all 4 regs for gdt addresses   */
#define DR_CONTROL_RESERVED 0xFC00 /* Bits reserved by Intel                */
#define DR_LOCAL_SLOWDOWN 0x100    /* Slow the pipeline for ldt addresses   */
#define DR_GLOBAL_SLOWDOWN 0x200   /* Slow the pipeline for gdt addresses   */

#define DR_LOCAL_ENABLE_SHIFT 0    /* Additional shift to the local enable  */
#define DR_GLOBAL_ENABLE_SHIFT 1   /* Additional shift to the global enable */
#define DR_ENABLE_SIZE 2           /* There are 2 enable bits per register  */

#define DR_CONTROL_SHIFT 16        /* Shift to get to register control bits */
#define DR_CONTROL_SIZE 4          /* There are 4 control bits per register */
#define DR_RW_MASK 0x3             /* Two of these bits specify r/w access  */
#define DR_RW_EXECUTE 0x0          /* Settings for the read/write mask      */
#define DR_RW_WRITE 0x1
#define DR_RW_READ 0x3
#define DR_LEN_MASK 0xC            /* Two of these bits specify data length */
#define DR_LEN_1 0x0               /* Settings for data length              */
#define DR_LEN_2 0x4
#define DR_LEN_4 0xC

#endif	/* _SYS_DEBUGREG_H */
