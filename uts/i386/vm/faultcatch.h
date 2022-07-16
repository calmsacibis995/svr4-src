/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _VM_FAULTCATCH_H
#define _VM_FAULTCATCH_H

#ident	"@(#)kern-vm:faultcatch.h	1.1"

/*
 * This file defines a mechanism for catching kernel page fault errors.
 * Any access to a pageable address should be protected by this mechanism,
 * since the I/O may fail, or (in the case of a user-supplied address)
 * the address may be invalid.
 *
 * Usage:
 *		CATCH_FAULTS(flags)
 *			protected_statement
 *		errno = END_CATCH();
 *
 * The flags define the type of address to protect.  This includes user
 * addresses, seg_u addresses, and seg_map addresses.
 *
 * The value returned by END_CATCH() will be 0 if no fault error occurred,
 * or the errno returned from the fault handler (unless the error occurred
 * on a user address, in which case the fault handler's return value is
 * ignored and EFAULT is returned).
 *
 * Caveats:
 *
 * CATCH_FAULTS should not be used from interrupt routines, or
 * nested within another CATCH_FAULTS.
 *
 * The protected code must not do anything stateful, such as using spl's
 * or setting locks, since it may be aborted in midstream.
 */

#define CATCH_UFAULT		0x0001
#define CATCH_SEGMAP_FAULT	0x0002
#define CATCH_SEGU_FAULT	0x0004
#define CATCH_BUS_TIMEOUT	0x4000
#define CATCH_ALL_FAULTS	0x8000

#define CATCH_KERNEL_FAULTS	(CATCH_SEGMAP_FAULT|CATCH_SEGU_FAULT)

#if !defined(LOCORE)

#include <sys/types.h>

typedef struct fault_catch {
	u_int	fc_flags;
	int	fc_errno;
	void	(*fc_func)();
	label_t	fc_jmp;
} fault_catch_t;

#if defined(_KERNEL)

/* NOTE: Although the implementation of CATCH_FAULTS() uses setjmp/longjmp,
 * the enclosed code MUST NOT do anything stateful, since it could be aborted
 * at any point.  This applies to multiprocessing locks as well, so this
 * particular use of setjmp/longjmp is safe in a multiprocessor context.
 */

#if DEBUG == 1

#include <sys/debug.h>

#define CATCH_FAULTS(flags) \
	if (ASSERT(!servicing_interrupt()), \
	    ASSERT(u.u_fault_catch.fc_flags == 0), \
	    (u.u_fault_catch.fc_errno = 0), \
	    (u.u_fault_catch.fc_flags = (flags)), \
	    setjmp(&u.u_fault_catch.fc_jmp) == 0)
#else
#define CATCH_FAULTS(flags) \
	if ((u.u_fault_catch.fc_errno = 0), \
	    (u.u_fault_catch.fc_flags = (flags)), \
	    setjmp(&u.u_fault_catch.fc_jmp) == 0)
#endif

#define END_CATCH() \
	((u.u_fault_catch.fc_flags = 0), \
	 u.u_fault_catch.fc_errno)

#if defined(__STDC__)
extern void	fc_jmpjmp(void);
#else
extern void	fc_jmpjmp();
#endif	/* __STDC__ */

#endif	/* _KERNEL */

#endif	/* not LOCORE */

#endif	/* _VM_FAULTCATCH_H */
