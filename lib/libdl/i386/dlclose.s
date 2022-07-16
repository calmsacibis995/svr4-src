/ident	"@(#)libdl:i386/dlclose.s	1.1"
/ dlclose calls _dlopen in ld.so

	.globl	dlclose
	.globl	_dlclose

dlclose:
	jmp	_dlclose@PLT
