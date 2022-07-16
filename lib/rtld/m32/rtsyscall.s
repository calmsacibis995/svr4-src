# dynamic linker system calls
.ident	"@(#)rtld:m32/rtsyscall.s	1.3"

	.set	__sys3b,50*8

	.globl	_cerror
	.globl	_sys3b


# file = sys3b(cmd,arg1[,arg2])

# file == -1 means error

_sys3b:
	MOVW	&4,%r0
	MOVW	&__sys3b,%r1
	GATE
	jgeu 	.noerror2
	jmp 	_cerror

.noerror1:
	CLRW	%r0
	RET

.noerror2:
	RET
