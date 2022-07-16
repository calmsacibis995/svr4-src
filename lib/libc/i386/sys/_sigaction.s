
.ident	"@(#)libc-i386:sys/_sigaction.s	1.1"

/ SYS library -- _sigaction
/ error = _sigaction(sig, act, oact);

	
	.file "_sigaction.s"
	
	.text

	.globl	__sigaction
	.globl	_cerror

_fgdef_(__sigaction):
	_prologue_
	MCOUNT
	movl	$SIGACTION,%eax
	movl	_daref_(_sigreturn),%edx
	_epilogue_
	lcall	$0x7,$0
	jc	_cerror
	ret


/ The following is used as a return address from user level interrupt
/ catching routines to clear the args from the interrupt routine call.
/ Then it enters the system to do the interrupt return to restore the 
/ stack to the proper state. Its address sent to kernel only on signal
/ sigset.
/ The signal handling is done in the kernel because the floating point
/ emulation is done there also.
/
_sigreturn:
	addl	$4,%esp		/ remove args to user interrupt routine
	lcall	$0xF,$0		/ return to kernel to return to user
