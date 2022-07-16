	.file	"pcrt1.s"
	.ident	"@(#)libc-i386:csu/pcrt1.s	1.16"


_m4_define_(`PTRSHIFT', 2)
_m4_define_(`PTRSIZE', 4)

	.globl	_start
	.globl	_mcount
/	.globl	environ
	.globl	_CAproc

/ global entities defined elsewhere but used here
	.globl	main
	.globl	__fpstart
	.globl	exit
	_wdecl_(_cleanup)
	_wdecl_(_DYNAMIC)
	.globl	_CAnewdump
	.globl	_exithandle

/ Special exit for profiling
/ exit(code)
_fgdef_(exit):
	call	_CAnewdump
	call	_exithandle
	movl	$EXIT,%eax
	lcall	$0x7,$0
	hlt

/ C language startup routine.
/ Assume that exec code has cleared the direction flag in the TSS.
/ Assume that %esp is set to the addr after the last word pushed.
/ The stack contains (in order): argc, argv[],envp[],...
/ Assume that all of the segment registers are initialized.

_fgdef_(_start):
/ Allocate a NULL return address and a NULL previous %ebp as if
/ there was a genuine call to _start.
/ sdb stack trace shows _start(argc,argv[0],argv[1],...,envp[0],...)
	pushl	$0
	pushl	$0
	movl	%esp,%ebp		/ The first stack frame.
	pushl	%edx			/ Save _rt_do_exit

	movl	$_cleanup,%eax
  	testl	%eax,%eax
	jz	.L0
	pushl	$_cleanup
	call	atexit
	addl	$4,%esp
.L0:
        movl	$_DYNAMIC,%eax
	testl	%eax,%eax
	jz	.L1
	call	atexit
.L1:
	pushl	$_fini
	call	atexit

/ Calculate the location of the envp array by adding the size of
/ the argv array to the start of the argv array.
	movl	8(%ebp),%eax		/ argc
	leal	[PTRSIZE\*4](%ebp,%eax,4),%edx	/envp
	movl	%edx,environ		/ copy to environ
	pushl	%edx
	leal	[PTRSIZE\*3](%ebp),%edx	/ argv
	pushl	%edx
	pushl	%eax			/ argc
	movl	(%edx),%eax		/ store process name for CA use
	movl	%eax,_CAproc
	call	_init
	call	__fpstart
	call	main			/ main(argc,argv,envp)
	addl	$12,%esp		/ let sdb know how many args in call to main()
	pushl	%eax			/	and call exit
	call	exit
	pushl	$0			/ Spare word for retaddr before arg
	movl	$EXIT,%eax		/ if user redefined exit, do the
	lcall	$0x7,$0			/	system call here
	hlt

/ The following is here in case any object module compiled with cc -p
/	was linked into this module.
_fgdef_(_mcount):
	ret

	.data
	.align	4
	.globl	_prof_dynamic_ptr	/ see lprof:soqueue.c
_prof_dynamic_ptr:
	.long	_DYNAMIC
_dgdef_(_CAproc):
	.long	0
		
	.globl  __longdouble_used
_dgdef_(__longdouble_used):
	.long	0
