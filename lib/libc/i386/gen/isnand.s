	.file	"isnand.s"

	.ident	"@(#)libc-i386:libc-i386/gen/isnand.s	1.5"

/	int isnand(srcD)
/	double srcD;
/
/	This routine returns 1 if the argument is a NaN
/		     returns 0 otherwise.

	.set	DMAX_EXP,0x7ff

	.text
	.align	4
/	.def	isnand;	.val	isnand;	.scl	2;	.type	047;	.endef

_fwdef_(`isnand'):
_fwdef_(`isnan'):
	MCOUNT
	movl	8(%esp),%eax
	andl	$0x7ff00000,%eax	/ bits 62-52
	cmpl	$[DMAX_EXP<<20],%eax
	jne	.false

	movl	8(%esp),%eax
	andl	$0x000fffff,%eax	/ bits 51-32
	orl	4(%esp),%eax		/ bits 31-0
	jz	.false			/ all fraction bits are 0
					/ its an infinity
	movl	$1,%eax
	ret
.false:
	movl	$0,%eax
	ret
/	.def	isnand;	.val	.;	.scl	-1;	.endef
