	.ident	"@(#)libc-i386:csu/pcrtn.s	1.1"
	.file	"pcrtn.s"
/
/ This code provides the end to the _init and _fini functions which are 
/ used C++ static constructors and desctuctors.  This file is
/ included by cc as the last component of the ld command line
/
/ Note: The instruction "call Ln" is used to push the current value
/ of the PC.  (Which is used to find the location of the shared object.)
/
	.section	.init
	call	L1
L1:
	call	_CAstartSO
	addl	$4,%esp
	ret	$0
/
	.section	.fini
	call	L2
L2:
	call	_CAstartSO
	addl	$4,%esp
	ret	$0
