	.ident	"@(#)rtld:i386/rtboot.s	1.11"
	.file	"rtboot.s"

/ bootstrap routine for run-time linker
/ we get control from exec which has loaded our text and
/ data into the process' address space and created the process 
/ stack
/
/ on entry, the process stack looks like this:
/
/			# <- %esp
/_______________________#  high addresses
/	strings		#  
/_______________________#
/	0 word		#
/_______________________#
/	Auxiliary	#
/	entries		#
/	...		#
/	(size varies)	#
/_______________________#
/	0 word		#
/_______________________#
/	Environment	#
/	pointers	#
/	...		#
/	(one word each)	#
/_______________________#
/	0 word		#
/_______________________#
/	Argument	# low addresses
/	pointers	#
/	Argc words	#
/_______________________#
/	argc		# 
/_______________________# <- %ebp

/
/ We must calculate the address at which ld.so was loaded,
/ find the addr of the dynamic section of ld.so, of argv[0], and  of
/ the process' environment pointers - and pass the thing to _rt_setup
/ to handle.  We then call _rtld - on return we jump to the entry
/ point for the a.out.

	.text
	.globl	_rt_boot
	.globl	_rt_errout
	.globl	_rt_setup
	.globl	_kill
	.globl	_getpid
	.globl	_write
	.globl	_GLOBAL_OFFSET_TABLE_
	.type	_rt_boot,@function
	.type	_rt_errout,@function
	.align	4
_rt_boot:
				/ get addresses of global offset 
				/ table and ld.so load point
				/ load point is referenced by special 
				/ symbol _base

				/ push in the order: ld_base, ld_dyn, &argc

	pushl	$0		/ clear 2 words for a fake stack frame for sdb
	pushl	$0
	movl	%esp,%ebp

	leal	8(%ebp),%eax	/ get address of argc and push it
	pushl	%eax

	call	.L1		/ only way to get IP into a register
.L1:
	popl	%ebx		/ pop the IP we just "pushed"
	movl	%ebx,%edx	/ save it for later
	addl	$_GLOBAL_OFFSET_TABLE_+[.-.L1],%ebx
	pushl	_DYNAMIC@GOT(%ebx)	/ address of dynamic structure

	subl	$[.L1-_rt_boot],%edx	/ get address of _rt_boot
	andl	$0xffffe000,%edx	/ remove LS bits from address of _rt_boot
	pushl	%edx		/ push ld_base

	call	_rt_setup@PLT	/ _rt_setup(ld_base, _DYNAMIC, &argc)
	addl	$20,%esp

	movl	_rt_do_exit@GOT(%ebx), %edx
	jmp	*%eax 		/ transfer control to a.out
	.size	_rt_boot,.-_rt_boot

/ Print an error message then exit.
/ This routine is necessary since we cannot do relative addressing on the 386.
/ One parameter: message number to print.
	.section	.rodata
	.align	4
.M0:
	.string	"ld.so: internal error: no file descriptor for the a.out\n"
.M1:
	.string	"ld.so: internal error: invalid relocation type\n"
.M2:
	.text
	.align	4
_rt_errout:
	pushl	%ebp
	movl	%esp,%ebp
	call	.L2		/ get the IP
.L2:
	popl	%ebx
	addl	$_GLOBAL_OFFSET_TABLE_+[.-.L2],%ebx

	cmpl	$0,8(%ebp)	/ check arg again message 0
	jne	.L3
	leal	.M0@GOTOFF(%ebx),%eax	/ address of message into %eax
	movl	$[.M1-.M0-1],%ecx	/ length of string
	jmp	.printit

.L3:
	cmpl	$1,8(%ebp)	/ check arg again message 1
	jne	.getout
	leal	.M1@GOTOFF(%ebx),%eax	/ address of message into %eax
	movl	$[.M2-.M1-1],%ecx	/ length of string
.printit:
	pushl	%ecx		/ length of the string - NULL byte
	pushl	%eax		/ address of the string
	pushl	2		/ stderr
	call	_write		/ write(fd, str, length)
				/ fall through on error (what else can we do?)
.getout:
	call	_getpid		/ getpid()

	pushl	%eax
	pushl	$9
	call	_kill		/ kill(pid, signal)
	.size	_rt_errout,.-_rt_errout
