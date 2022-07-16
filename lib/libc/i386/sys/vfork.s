.ident	"@(#)libc-i386:sys/vfork.s	1.3"

/ OS library -- vfork

	.file	"vfork.s"
	
	.text

	.globl	_cerror

/# The child of vfork() will execute in the parent's address space,
/# thereby changing the stack before the parent runs again.
/# Therefore, we cannot execute the RET instruction to return
/# to the caller after the GATE.  Instead, we remember the %pc
/# in %r2 and pop the stack by executing RET before executing GATE.
/# On return from the GATE, we just jump to the address in %r2.
/
/# This works only because there are no arguments to vfork(),
/# %r2 is treated as a scratch register by the compilation system,
/# and the operating system preserves the value of %r2.
/
/# Pity the poor debugger developer who has to deal with this kludge.
/
/	.set	sys_vfork,119
/	.text
/vfork:
/	movw	-8(%sp),%r2		# remember old %pc in %r2
/	movaw	.L0,-8(%sp)		# arrange for RET to return here
/	RET
/.L0:
/	movw	&4,%r0			# call the operating system
/	movw	&sys_vfork*8,%r1
/	GATE
/	jgeu	.L2
/
/# reconstruct the stack before jumping to _cerror
/	call	&0,.L1
/.L1:	movw	%r2,-8(%sp)
/	jmp	_cerror
/
/# %r1 is zero if we are the parent, non-zero if we are the child.
/.L2:	cmpw	%r1,&0
/	je	.L3
/	movw	&0,%r0		# zero the return value in the child
/.L3:	jmp	0(%r2)		# jump back to the caller
	

_m4_ifdef_(`ABI',`
	.globl	vfork
_fgdef_(vfork):
',`
_m4_ifdef_(`DSHLIB',`
	.globl	vfork
_fgdef_(vfork):
',`
_fwdef_(`vfork'):
')
')
	MCOUNT
	movl	0(%esp),%ecx		/ save %eip in %ecx
	leal	_sref_(.L0),%eax	/ arrange for RET to return here
	movl	%eax,0(%esp)
	ret

.L0:
	movl	$VFORK,%eax
	lcall	$0x7,$0
	jae 	.L2

/ reconstruct stack before jumping to _cerror
	call	.L1
.L1:	movl	%ecx,0(%esp)
	jmp	_cerror

/ %edx is zero if we are the parent, non-zero if we are the child.
.L2:	cmpl	$0,%edx
	je	.L3
	movl	$0,%eax		/ zero the return value in the child
.L3:	jmp	*%ecx		/ jump back to the caller
