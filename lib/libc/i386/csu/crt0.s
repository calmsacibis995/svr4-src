	.file	"crt0.s"

	.ident	"@(#)libc-i386:libc-i386/csu/crt0.s	1.6"


_m4_define_(`PTRSHIFT', 2)
_m4_define_(`PTRSIZE', 4)

	.globl	_start
	.globl	_mcount
/	.globl	environ

/ C language startup routine.
/ Assume that exec code has cleared the direction flag in the TSS.
/ Assume that %esp is set to the addr after the last word pushed.
/ The stack contains (in order): argc, argv[],envp[],...
/ Assume that all of the segment registers are initialized.

_fgdef_(_start):
/ Allocate a NULL return address and a NULL previous %ebp as if
/ there was a genuine call to _start.
/ sdb stack trace shows _start(argc,argv[0],argv[1],...,envp[0],...)
	subl	$[PTRSIZE+PTRSIZE],%esp
	movl	%esp,%ebp		/ The first stack frame.

/ Calculate the location of the envp array by adding the size of
/ the argv array to the start of the argv array.
	movl	8(%ebp),%eax		/ argc
	leal	[PTRSIZE\*4](%ebp,%eax,4),%edx	/envp
	movl	%edx,environ		/ copy to environ
	pushl	%edx
	leal	[PTRSIZE\*3](%ebp),%edx	/ argv
	pushl	%edx
	pushl	%eax			/ argc
	call	main			/ main(argc,argv,envp)
	addl	$12,%esp		/ let sdb know how many args in call to main()
	pushl	%eax			/	and call exit
	call	exit
	pushl	$0			/ spare word for call address before arg
	movl	$EXIT,%eax		/ if user redefined exit, do the
	lcall	$0x7,$0			/	system call here
	hlt

/ The following is here in case any object module compiled with cc -p
/	was linked into this module.
_fgdef_(_mcount):
	ret
