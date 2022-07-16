.ident	"@(#)libc-i386:sys/mmap.s	1.1"

/ gid = mmap();
/ returns effective gid

	.file	"mmap.s"

	.text

	.globl  _cerror

_fwdef_(`mmap'):
	MCOUNT
	movl	$MMAP,%eax
	lcall	$0x7,$0
	jc	_cerror
	ret
