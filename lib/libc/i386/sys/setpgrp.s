.ident	"@(#)libc-i386:libc-i386/sys/setpgrp.s	1.4"

	.file	"setpgrp.s"

	.text

	.globl	_cerror

_fwdef_(`setpgrp'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	%esp,%ecx
	pushl	$1
	pushl	%eax		/ slot for ret addr.
	movl	$SETPGRP,%eax
	lcall	$0x7,$0
	movl	%ecx,%esp
	jc	_cerror
	ret

_fwdef_(`getpgrp'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	%esp,%ecx
	pushl	$0
	pushl	%eax		/ slot for ret addr.
	movl	$SETPGRP,%eax
	lcall	$0x7,$0
	movl	%ecx,%esp
	jc	_cerror
	ret
