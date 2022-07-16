
/ Determine the sign of a double-long number.

	.ident	"@(#)libc-i386:gen/lsign.s	1.2"
	.file	"lsign.s"
	.text

_fwdef_(`lsign'):

	MCOUNT

	movl	8(%esp),%eax
	roll	%eax
	andl	$1,%eax

	ret
