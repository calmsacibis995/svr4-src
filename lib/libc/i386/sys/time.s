.ident	"@(#)libc-i386:libc-i386/sys/time.s	1.4"


	.file	"time.s"

	.text

_fwdef_(`time'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$TIME,%eax
	lcall	$0x7,$0
	movl	4(%esp),%edx
	testl	%edx,%edx
	jz	.nostore
	movl	%eax,(%edx)
.nostore:
	ret
