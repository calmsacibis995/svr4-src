	.file   "i386_data.s"

	.ident	"@(#)libc-i386:libc-i386/gen/i386_data.s	1.4"

/ This file contains
/ the definition of the
/ global symbols errno and _siguhandler
/ 
/ int errno;

	.globl	errno
	.comm	errno,4

	.data
	.align	4
	.globl	_siguhandler
_dgdef2_(_siguhandler,128):	/ used in libos:sigaction.c
	.zero	128 / 32 * 4
