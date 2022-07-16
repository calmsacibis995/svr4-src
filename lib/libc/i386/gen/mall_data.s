	.file   "mall_data.s"

	.ident	"@(#)libc-i386:libc-i386/gen/mall_data.s	1.3"

/ This file contains
/ the definition of the
/ imported beginning of the malloc arena _allocs
/ 
/ union store *allocs[2] = { 0, 0 } ; /* if it were possible */

	.globl	_allocs

	.data
	.align	4
_dgdef_(_allocs):	
	.long	0
	.long	0
