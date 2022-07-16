.ident	"@(#)libc-i386:sys/readv.s	1.4"

/ gid = readv();
/ returns effective gid

	.file	"readv.s"

	.text

	.set	ERESTART,91

	.globl  _cerror

_fwdef_(`readv'):
	MCOUNT
	movl	$READV,%eax
	lcall	$0x7,$0
	jae	noerror
	cmpb	$ERESTART,%al
	je	readv
	jmp	_cerror
noerror:
	ret
