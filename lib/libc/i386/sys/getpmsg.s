.ident	"@(#)libc-i386:sys/getpmsg.s	1.3"

/ gid = getpmsg();
/ returns effective gid

	.file	"getpmsg.s"

	.text

	.set	ERESTART,91

	.globl  _cerror

_fwdef_(`getpmsg'):
	MCOUNT
	movl	$GETPMSG,%eax
	lcall	$0x7,$0
	jae	noerror
	cmpb	$ERESTART,%al
	je	getpmsg
	jmp	_cerror
noerror:
	ret
