.ident	"@(#)libc-i386:sys/_signal.s	1.1"

	.file	"signal.s"

	.text

/ Libc signal functions:
/ signal(sig, func);
/ sigset(sig,func);
/ sighold(sig);
/ sigrelse(sig);
/ sigpause(sig);
/ sigignore(sig);
/ int	sig;
/ void	(*func)();

	.set 	SIGDEFER,0x100
	.set 	SIGHOLD,0x200
	.set 	SIGRELSE,0x400
	.set 	SIGIGNORE,0x800
	.set 	SIGPAUSE,0x1000

	.set	sig_hold,2	/ Can't use header def because of cast

	.globl	__signal
	.globl	_cerror
	.globl	_siguhandler
	.globl	_sigacthandler

_fgdef_(__signal):
	_prologue_
	MCOUNT
	movl	_esp_(4),%ecx		/ signo param
	jmp	.sig1

_fwdef_(`sigset'):
	_prologue_
	MCOUNT
	movl	_esp_(4),%ecx		/ signo param
	orl	$SIGDEFER,_esp_(4)	/ first param after ret addr (signo)

.sig1:					/ common to signal and sigset
	movl	$SIGNAL,%eax
	movl	_daref_(_sigreturn),%edx
_m4_ifdef_(`DSHLIB',
`	pushl	_esp_(8)		/ handler param
	pushl	_esp_(8)		/ signo param
	subl	$4,%esp			/ where return address would be.
')
	lcall	$0x7,$0
	jc	.sig1_error
_m4_ifdef_(`DSHLIB',
`	addl	$12,%esp
')
	cmpl	_daref_(_sigacthandler),%eax
	jne	.not_sigact
_m4_ifdef_(`DSHLIB',
`	movl	_daref_(_siguhandler),%eax
	movl	(%eax,%ecx,4),%eax
',
`	movl	_siguhandler(,%ecx,4),%eax
')
.not_sigact:
	orl	%eax,%eax		/ clear carry
	_epilogue_
	ret
.sig1_error:
_m4_ifdef_(`DSHLIB',
`	addl	$12,%esp
')
	_epilogue_
	jmp	_cerror

_fwdef_(`sigignore'):
	MCOUNT
	orl	$SIGIGNORE,4(%esp)	/ first param after ret addr
	movl	$SIGNAL,%eax
	jmp	.common

_fwdef_(`sigpause'):
	MCOUNT
	orl	$SIGPAUSE,4(%esp)	/ first param after ret addr
	movl	$SIGNAL,%eax
	jmp	.common

_fwdef_(`sigrelse'):
	MCOUNT
	orl	$SIGRELSE,4(%esp)	/ first param after ret addr
	movl	$SIGNAL,%eax
	jmp	.common
	
_fwdef_(`sighold'):
	MCOUNT
	orl	$SIGHOLD,4(%esp)	/ first param after ret addr
	movl	$SIGNAL,%eax

.common:				/ actually do the call
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
