/ident	"@(#)libdl:i386/dlopen.s	1.1"
/ dlopen calls _dlopen in ld.so

	.globl	dlopen
	.globl	_dlopen

dlopen:
	jmp	_dlopen@PLT
