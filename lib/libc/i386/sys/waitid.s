.ident	"@(#)libc-i386:sys/waitid.s	1.1"

/ C library -- waitid

/ error = waitid(idtype,id,&info,options)

	.file	"waitid.s"
	
	.text

	.set	ERESTART,91

	.globl  _cerror

_fwdef_(`waitid'):
	MCOUNT
	movl	$WAITID,%eax
	lcall	$0x7,$0
	jae 	noerror		/ all OK - normal return
	cmpb	$ERESTART,%al	/  else, if ERRESTART
	je	waitid		/    then loop
	jmp 	_cerror		/  otherwize, error

noerror:
	ret
