.ident	"@(#)libc-i386:sys/putpmsg.s	1.3"

/ gid = putpmsg();
/ returns effective gid

	.file	"putpmsg.s"

	.text

	.set	ERESTART,91

	.globl  _cerror

_fwdef_(`putpmsg'):
	MCOUNT
	movl	$PUTPMSG,%eax
	lcall	$0x7,$0
	jae	noerror
	cmpb	$ERESTART,%al
	je	putpmsg
	jmp	_cerror
noerror:
	ret
