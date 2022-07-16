	.ident	"@(#)rtld:i386/rtbinder.s	1.3"
	.file	"rtbinder.s"

/ we got here because a call to a function resolved to
/ a procedure linkage table entry - that entry did a JMP
/ to the first PLT entry, which in turn did a JMP to _rtbinder
/
/ the stack at this point looks like:
/ 	PC of return from call to foo
/	offset of relocation entry
/	addr of link_map entry for this reloc
/ %esp->
/

	.text
	.globl	_binder
	.globl	_rtbinder
	.type	_rtbinder,@function
	.align	4
_rtbinder:
	call	_binder@PLT	/ transfer control to rtld
				/ rtld returns address of function definition
	addl	$8,%esp		/ fix stack
	jmp	*%eax		/ transfer to the function
