	.file	"sigsetjmp.s"

	.ident	"@(#)libc-i386:gen/sigsetjmp.s	1.4"

/	Sigsetjmp() is implemented in assembly language because it needs
/	to have direct control over register use.

_m4_define_(`uc_mcontext',36)
_m4_define_(`UC_ALL',0x1F)
_m4_define_(`UC_SIGMASK',0x01)
_m4_define_(`EAX',11)
_m4_define_(`UESP',17)
_m4_define_(`EIP',14)

/ int sigsetjmp(sigjmp_buf env,int savemask)
/
_fwdef_(`sigsetjmp'):
	MCOUNT			/ subroutine entry counter if profiling

	movl	4(%esp),%eax	/ ucp = (ucontext_t *)env;

	movl	$UC_ALL,(%eax)	/ ucp->uc_flags = UC_ALL;

	pushl	%eax	/ ucp
	pushl	$0	/ GETCONTEXT
	pushl	%eax	/ dummy return addr
	movl	$UCONTEXT,%eax
	lcall	$0x7,$0		/ __getcontext(ucp);
	addl	$0xC,%esp

	movl	4(%esp),%eax

	cmpl	$0,8(%esp)	/ if (!savemask)
	jnz	.mask
	andl	$-1!UC_SIGMASK,(%eax)	/  ucp->uc_flags &= ~UC_SIGMASK;
.mask:

	/ cpup = (greg_t *)&ucp->uc_mcontext.gregs;
	leal	[uc_mcontext](%eax),%edx

	movl	$1,EAX\*4(%edx)	/ cpup[ EAX ] = 1;

	movl	0(%esp),%eax	/ set cpup[ EIP ] to caller's EIP
	movl	%eax,EIP\*4(%edx)

	leal	4(%esp),%eax	/ set cpup[ UESP ] to caller's ESP
	movl	%eax,UESP\*4(%edx)

	xorl	%eax,%eax
	ret
