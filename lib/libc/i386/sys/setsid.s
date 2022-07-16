.ident	"@(#)libc-i386:sys/setsid.s	1.1"

/ C library -- setsid, setpgid, getsid, getpgid

	.file	"setsid.s"

	.text

	.globl  _cerror

_fwdef_(`getsid'):
	popl	%edx
	pushl	$2
	pushl	%edx
	jmp	pgrp

_fwdef_(`setsid'):
	popl	%edx
	pushl	$3
	pushl	%edx
	jmp	pgrp

_fwdef_(`getpgid'):
	popl	%edx
	pushl	$4
	pushl	%edx
	jmp	pgrp

	
_fwdef_(`setpgid'):
	popl	%edx
	pushl	$5
	pushl	%edx
	jmp	pgrp

pgrp:
	movl	$SETSID,%eax
	lcall	$7,$0
	popl	%edx
	movl	%edx,0(%esp)	/ Remove extra word
	jc	_cerror
	ret

