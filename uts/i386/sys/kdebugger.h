#ident	"@(#)kdebugger.h	1.2	92/02/17	JPB"

/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_KDEBUGGER_H
#define _SYS_KDEBUGGER_H

#ident	"@(#)head.sys:sys/kdebugger.h	1.2"

/*	Used by kernel debuggers */

#include "sys/types.h"
#include "sys/conf.h"

extern void (*cdebugger)();

struct kdebugger {
	void	(*kdb_entry)();
	int	kdb_reserved[5];
	struct kdebugger *kdb_next;
	struct kdebugger *kdb_prev;
};
extern struct kdebugger *debuggers;

/*
 * Each entry in the dbg_io array is a pointer to a structure of
 * type conssw (which contains putchar routine, dev number, and
 * getchar routine).  Generally, the first entry will point to the
 * kernel variable, conssw, so we use whatever is the standard console
 * device.
 */

extern struct conssw *dbg_io[], **cdbg_io;
extern int ndbg_io;
extern unsigned dbg_putc_count;

#define DBG_GETCHAR()	(((*cdbg_io)->ci) ((*cdbg_io)->co_dev))
#define DBG_PUTCHAR(c)	(void) ((*(*cdbg_io)->co) (c, (*cdbg_io)->co_dev), \
				++dbg_putc_count)

/*
 * As a security feature, the kdb_security flag (set by the KDBSECURITY
 * tuneable) is provided.  If it is non-zero, the debugger should ignore
 * attempts to enter from a console key sequence.
 */
extern int kdb_security;

extern char symtable[];
extern int kdb_symsize;

extern char *debugger_init;	/* Optional initial command string */

#define NREGSET	8

extern unsigned long db_st_startpc;
extern unsigned long db_st_startsp;
extern unsigned long db_st_startfp;
extern unsigned long db_st_offset;

#ifdef i386
/* The stack pointer as saved in a register frame is offset from the actual
   value it had at the time of the trap, since the trap pushed several words
   onto the stack.  This symbol provides the proper adjustment factor. */

#define ESP_OFFSET	0x14

/* Some useful 386 opcodes */

#define OPC_INT3	0xCC
#define OPC_PUSHFL	0x9C
#define OPC_CALL_REL	0xE8
#define OPC_CALL_DIR	0x9A
#define OPC_CALL_IND	0xFF
#define OPC_PUSH_EBP	0x55
#define OPC_MOV_RTOM	0x89
#define OPC_ESP_EBP	0xE5
#define OPC_MOV_MTOR	0x8B
#define OPC_EBP_ESP	0xEC

/* This inline function gets the register values needed to start
   a stack trace.  Must be called from the same routine which calls
   db_stacktrace(). */
asm void
db_get_stack()
{
%lab	push_eip
	call	push_eip
push_eip:
	popl	db_st_startpc
	movl	%esp, db_st_startsp
	movl	%ebp, db_st_startfp
}

#endif /* i386 */

#endif /* _SYS_KDEBUGGER_H */
