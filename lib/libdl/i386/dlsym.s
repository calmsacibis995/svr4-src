/ident	"@(#)libdl:i386/dlsym.s	1.1"
/ dlsym calls _dlsym in ld.so

	.globl	dlsym
	.globl	_dlsym

dlsym:
	jmp	_dlsym@PLT
