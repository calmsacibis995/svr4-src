#ident	"@(#)sdb:libexecon/i386/Reg1.C	1.4"

// Reg1.C -- register names and attributes, machine specific data (i386)

#include "Reg.h"
#include "Itype.h"

RegAttrs regs[] = {
//
//	ref		name		size		flags	stype	offset
//
	REG_EAX,	"%eax",		4,		0,	Suint4,	EAX,
	REG_ECX,	"%ecx",		4,		0,	Suint4, ECX,
	REG_EDX,	"%edx",		4,		0,      Suint4, EDX,
	REG_EBX,	"%ebx",		4,		0,	Suint4, EBX,
//
//	Stack registers
//
	REG_ESP,	"%esp",		4,		0,	Suint4, UESP,
	REG_EBP,	"%ebp",		4,		0,	Suint4, EBP,
	REG_ESI,	"%esi",		4,		0,	Suint4, ESI,
	REG_EDI,	"%edi",		4,		0,	Suint4, EDI,
//
//	Instruction Pointer register
//
	REG_EIP,	"%eip",		4,		0,	Suint4, EIP,
//
//	Flags register
//
	REG_EFLAGS,	"%eflags",	4,		0,	Suint4, EFL,
	REG_TRAPNO,	"%trapno",	4,		0,	Suint4, TRAPNO,
//
//	floating point stack
//
	FP_STACK,	"ST(0)",	0,		1,	Suint4, 0,
	FP_STACK,	"ST(1)",	0,		1,	Suint4, 0,
	FP_STACK,	"ST(2)",	0,		1,	Suint4, 0,
	FP_STACK,	"ST(3)",	0,		1,	Suint4, 0,
	FP_STACK,	"ST(4)",	0,		1,	Suint4, 0,
	FP_STACK,	"ST(5)",	0,		1,	Suint4, 0,
	FP_STACK,	"ST(6)",	0,		1,	Suint4, 0,
	FP_STACK,	"ST(7)",	0,		1,	Suint4, 0,
//
// end marker
//
	REG_UNK,	0,		0,		0,	0,	0
};
