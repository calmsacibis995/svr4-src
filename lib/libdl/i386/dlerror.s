/ident	"@(#)libdl:i386/dlerror.s	1.1"
/ dlerror calls _dlerror in ld.so

	.globl	dlerror
	.globl	_dlerror

dlerror:
	jmp	_dlerror@PLT
