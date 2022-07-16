	.file	"fpstart.s"
	.ident	"@(#)libc-i386:fp/fpstart.s	1.2"

/	__fpstart - glue routine for floating point
/	startup.
/	Called only from crt1.o, this routine's sole
/	purpose is to delay binding _fpstart until
/	it is known whether _fp_hw is needed in this
/	program.

	.globl	__fpstart
	.globl	_fpstart
_fgdef_(__fpstart):
	jmp	_fpstart

