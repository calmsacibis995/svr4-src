	.file	"fpstart0.s"
	.ident	"@(#)libc-i386:fp/fpstart0.s	1.2"
/
/	_fpstart - floating point startup code.
/	This version of _fpstart is used if
/	no reference is made to globals _fp_hw
/ 	or _flt_rounds.
/	We set precision control to temp real format
/	and all exception masks on. OS sets rounding
/	mode to round to nearest
/
	.text
	.align	4
	.globl	_fpstart
_fgdef_(_fpstart):
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%eax
	movl	$0,-4(%ebp)
	fstcw	-4(%ebp)
	orl	$0x33f,-4(%ebp)
	fldcw	-4(%ebp)
	leave	
	ret	
	.align	4
	.text
