/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

	.file	"round.s"

	.ident	"@(#)kern-fp:round.s	1.1"

/$tt("80387	emulator	+ + + r o u n d + + + ")
/ **********************************************************************
/
/			r o u n d . m o d
/			=================
/
/	===============================================================
/               intel corporation proprietary information
/    this software  is  supplied  under  the  terms  of  a  license
/    agreement  or non-disclosure agreement  with intel corporation
/    and  may not be copied nor disclosed except in accordance with
/    the terms of that agreement.                                  
/	===============================================================
/
/	function:
/		implements all rounding modes for all precisions
/
/	public procedures:
/		round			special_round_test
/		directed_round_test
/
/	internal procedures:
/		prec16_case		prec24_case
/		prec32_case		prec53_case
/		prec64_case
/
/ ***************************************************************************
/
/...october 31, 1986...
/
/	.file	"a_mro"
/
/$nolist
#include	"e80387.h"
/$list
/...declare status register segment...
/
	.data	/a_msr	segment	rw	public
/	extrn	%gs:sr_flags
/a_msr	ends
/
/	assume	%ds:a_msr
/
	.text	/a_med	segment	er	public
/
/	extrn	get_rnd_control
/
	.globl	round
	.globl	directed_round_test
	.globl	special_round_test
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			prec16_case:
/			""""""""""""
/	function:
/		 sets up for rounding to 16 bits.
/
/	inputs:
/		assumes edi points to operand record.
/
/	outputs:
/		sets: dl if round or sticky bit set
/		      dh if guard bit set
/		      ah if lsb set
/		      cl to number of rounded bytes
/		      ch to pattern to add to low byte for rounding
/		and clears the guard, round, and sticky bytes.
/		doesn't affect al.
/
/	data accessed:
/
/	data changed:
/
/	procecures called:
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
prec16_case:	/proc			; initially, assume all guard,
	xchg	frac80+4(%ebp,%edi),%ecx/ round, and sticky info zero
	and	%ecx,%ecx	/ examine and clear grst word
	jns	get_round_sticky_16
	notb	%dh	/ et guard byte
get_round_sticky_16:
	and	$0x7fffffff,%ecx
	or	frac80(%ebp,%edi),%ecx
	orw	(%ebp,%edi),%cx
	jz	get_lsb_16
	notb	%dl								/ set round-sticky byte
get_lsb_16:
	xorb	%ah,%ah			/ initially, assume lsb is zero
	testb	$0x01,frac80+8(%ebp,%edi)
	jz	set_rnd_info_16
	notb	%ah				/ set lsb byte
	mov	$0,frac80(%ebp,%edi)
	movw	$0,(%ebp,%edi)
set_rnd_info_16:
	mov	$0x0102,%ecx			/ the rounded result has 2 bytes
	ret				/ the pattern to add is 01h
/prec16_case	endp
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			prec24_case:
/			""""""""""""
/	function:
/		sets up for rounding to 24 bits.
/
/	inputs:
/		assumes edi points to operand record.
/
/	outputs:
/		sets: dl if round or sticky bit set
/		      dh if guard bit set
/		      ah if lsb set
/		      cl to number of rounded bytes
/		      ch to pattern to add to low byte for rounding
/		and clears the guard, round, and sticky bytes.
/		doesn't affect al.
/
/	data accessed:
/
/	data changed:
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
prec24_case:	/proc			; initially, assume all guard,
	xchgb	frac32(%ebp,%edi),%cl	/ round, and sticky info zero
	andb	%cl,%cl			/ examine and clear grst word
	jns	get_round_sticky_24
	notb	%dh								/ set guard byte
get_round_sticky_24:
	andb	$0x7f,%cl
	or	frac64(%ebp,%edi),%ecx
	or	(%ebp,%edi),%ecx
	jz	get_lsb_24
	notb	%dl								/ set round-sticky byte
get_lsb_24:
	xorb	%ah,%ah			/ initially, assume lsb is zero
	testb	$0x01,frac32+1(%ebp,%edi)
	jz	clear_sticky_bytes_24
	notb	%ah				/ set lsb byte
clear_sticky_bytes_24:
	mov	$0,frac64(%ebp,%edi)
	mov	$0,(%ebp,%edi)
	mov	$0x0103,%ecx			/ the rounded result has 3 bytes
	ret					/ the pattern to add is 01h
/prec24_case	endp
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			prec32_case:
/			""""""""""""
/	function:
/		sets up for rounding to 32 bits.
/
/	inputs:
/		assumes edi points to operand record.
/
/	outputs:
/		sets: dl if round or sticky bit set
/		      dh if guard bit set
/		      ah if lsb set
/		      cl to number of rounded bytes
/		      ch to pattern to add to low byte for rounding
/		and clears the guard, round, and sticky bytes.
/		doesn't affect al.
/
/	data accessed:
/
/	data changed:
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
prec32_case:	/proc			; initially, assume all guard,
	xchg	frac64(%ebp,%edi),%ecx	/ round, and sticky info zero
	and	%ecx,%ecx
	jns	get_round_sticky_32
	notb	%dh			/ set guard byte
get_round_sticky_32:
	and	$0x7fffffff,%ecx
	or	(%ebp,%edi),%ecx
	jz	get_lsb_32
	notb	%dl			/ set round-sticky byte
get_lsb_32:
	xorb	%ah,%ah	/ initially, assume lsb is zero
	testb	$0x01,frac32(%ebp,%edi)
	jz	clear_sticky_bytes_32
not_ah_clear_32:
	notb	%ah			/ set lsb byte
clear_sticky_bytes_32:
	mov	$0,(%ebp,%edi)
	mov	$0x0104,%ecx/ the rounded result has 4 bytes
	ret					/ the pattern to add is 01h
/prec32_case	endp
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			prec53_case:
/			"""""""""""
/	function:
/		sets up for rounding to 53 bits.
/
/	inputs:
/		assumes edi points to operand record.
/
/	outputs:
/		sets: dl if round or sticky bit set
/		      dh if guard bit set
/		      ah if lsb set
/		      cl to number of rounded bytes
/		      ch to pattern to add to low byte for rounding
/		and clears the guard, round, and sticky bytes.
/		doesn't affect al.
/
/	data accessed:
/
/	data changed:
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
prec53_case:	/proc			; initially, assume all guard,
	mov	frac64(%ebp,%edi),%ecx	/ round, and sticky info zero
	testb	$0x04,%ch	/ examine guard bit
	jz	get_round_sticky_53
	notb	%dh			/ set guard byte
get_round_sticky_53:
	and	$0x03ff,%ecx
	or	(%ebp,%edi),%ecx
	jz	get_lsb_53
	notb	%dl			/ set round-sticky byte
get_lsb_53:
	xorb	%ah,%ah	/ initially, assume lsb is zero
	testb	$0x08,frac64+1(%ebp,%edi)
	jz	clear_grst_bits_53
	notb	%ah			/ set lsb byte
clear_grst_bits_53:
	mov	$0,(%ebp,%edi)
	andw	$0x0f800,frac64(%ebp,%edi)
	mov	$0x0807,%ecx/ the rounded result has 7 bytes
	ret					/ the pattern to add is 08h
/prec53_case	endp
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			prec64_case:
/			""""""""""""
/	function:
/		sets up for rounding to 64 bits.
/
/	inputs:
/		assumes edi points to operand record.
/
/	outputs:
/		sets: dl if round or sticky bit set
/		      dh if guard bit set
/		      ah if lsb set
/		      cl to number of rounded bytes
/		      ch to pattern to add to low byte for rounding
/		and clears the guard, round, and sticky bytes.
/		doesn't affect al.
/
/	data accessed:
/
/	data changed:
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
prec64_case:	/proc			; initially assume all guard,
	xchg	(%ebp,%edi),%ecx	/ round, and sticky info zero
	and	%ecx,%ecx		/ examine and clear grst word
	jns	get_round_sticky_64
	notb	%dh				/ set guard byte
get_round_sticky_64:
	and	$0x7fffffff,%ecx
	jz	get_lsb_64
	notb	%dl								/ set round-sticky byte
get_lsb_64:
	xorb	%ah,%ah			/ initially, assume lsb is zero
	testb	$0x01,frac64(%ebp,%edi)
	jz	set_round_info_64
	notb	%ah				/ set lsb byte
set_round_info_64:
	mov	$0x0108,%ecx			/ rounded result has 8 bytes
common_return:
	ret					/ the pattern to add is 01h
/prec64_case	endp
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			round:
/			"""""
/	function:
/		implements rounding.
/
/	inputs:
/		assumes: edi points to record to round, 
/		          dl contains rounding precision,
/			al is true if second rounding, false if first.
/
/	outputs:
/		on return, al is true if there was an high-bit carry-out
/		as a result of incrementing the significand to do the rounding.
/
/	data accessed:
/		added_one
/
/	data changed:
/		added_one
/
/	procedures called:
/		prec16_case			prec24_case
/		prec32_case			prec53_case
/		prec64_case			get_rnd_control
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
precn_case:	
	.long	prec24_case,prec32_case,prec53_case,prec64_case,prec16_case
/
rnd_case:	
	.long	case_rnd_to_even,case_rnd_down,case_rnd_up,case_rnd_to_zero
/
round:	/proc				; access unpacked status info
	xorb	%dh,%dh				/ set to zero the following:
	movswl	%dx,%esi			/ cl to num bytes in rnd frac
	shl	$2,%esi				/ ch to pattern to add to low
	xorw	%dx,%dx				/ byte in rounding
	xor	%ecx,%ecx			/ dl if round or sticky set
/ 		mov	[ebp+edi],dx		;*** zeroing low word
	call	*%cs:precn_case(%esi)	/ dh if grd set, ah if lsb set
	andw	%dx,%dx				/ if dl and dh both zero, then
	jnz	inexact_case		/ result is exact
	cmpb	true,%al
	movb	%dl,%al				/ no carry out from rounding
	jne	exact_1st_rnd
	movb	%al,rnd2_inexact
	ret
exact_1st_rnd:
	movw	%dx,rnd_history
	movb	%al,added_one	/ clear flag, no overflow
	ret
/
inexact_case:
	cmpb	true,%al
	jne	inexact_1st_rnd
	movb	%al,rnd2_inexact
	jmp	get_rnd_case
inexact_1st_rnd:
	movw	true,rnd_history/ makes rnd1_inexact true, rnd2_inexact false
get_rnd_case:
	push	%eax				/ save lsb/2nd rounding flags
	call	get_rnd_control	/ round different rnd_control
	movzbl	%al,%ebx		/ cases separately
	pop	%eax			/ reload lsb/2nd rounding flags
	jmp	*%cs:rnd_case(%ebx)
case_rnd_up:
	notb	%bh			/if + increment, else truncate
case_rnd_down:
	cmpb	%bh,sign(%ebp,%edi) /increment if neg,
	jne	do_increment		/ truncate if pos
	jmp	case_rnd_to_zero
case_rnd_to_even:
	movb	%dl,%bh				/ save round-sticky info in bh
	orb	%ah,%dl				/ if guard and (round or sticky
	cmpb	true,%al			/ or lsb), set dx to round up
	jne	do_add_				/ if 2nd round, if ((lsb=added)
/		mov	bl,	added_one	; and not (round or sticky)
/		cmp	al, rnd1_inexact	; and ((added=1) or last round
/		jne	p_error_0		; inexact)), round in opposite
/		mov	bl,	al	; al is true (0ffh), hence so is bl
/ the preceding four instructions are equivalent to the next one
	movb	rnd1_inexact,%bl
p_error_0:
	cmpw	$0x00ff,%bx		/ unless first round was inexact and
	jne	do_add_			/ current round-sticky bits are clear,
					/ decide how to round as for first round
	cmpb	added_one,%ah	/ else, dh (guard) must be true here because
				/ exact second rounds were ruled out earlier
	jne	do_add_			/ branch if ah <> added_one
	incw	%dx			/ here, dh = guard, dl = lsb
	jz	case_rnd_to_zero	/ truncate
	jmp	do_increment
do_add_:
	incw	%dx
	jz	do_increment
case_rnd_to_zero:
	movb	false,%al			/ clear flag, indicate nothing
	movb	%al,added_one		/ added in round (al = false)
/		and	%gs:sr_flags,not a_mask	; clear the a-bit 			---> defer
	jmp	set_inexact
do_increment:
/		or	%gs:sr_flags,a_mask		; set the a-bit				---> defer
	movb	true,added_one		/ set flag for 1 added in round
	movb	%ch,%dl				/ put pattern to add in dl
	xorb	%ch,%ch				/ ecx = num bytes in rounded res
	mov	$12,%ebx
	sub	%ecx,%ebx			/ ebx =  num of low byte
	add	%ebx,%edi			/ edi = true offset
	xorb	%al,%al
	addb	%dl,(%ebp,%edi)	/ add pattern to low byte and
	inc	%edi					/ propogate carries
	dec	%ecx
inc_loop:
	adcb	%al,(%ebp,%edi)
	inc	%edi
	loop	inc_loop
	sbbb	%al,%al				/ set al if carry set
	sub	$12,%edi			/ restore edi to entry value
set_inexact:
/	jmp	set_p_error				---> defer
	ret
/round	endp
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			special_round_test:
/			""""""""""""""""""
/	function:
/		test for rounding toward zero.
/
/	inputs:
/		assumes sign to be in al upon entry.
/
/	outputs:
/		returns true (in al) and clears zf if:
/		  (rnd_down and sign is positive) or
/		  (rnd_up and sign is negative);
/       else returns false (in al) and sets zf.
/
/	data accessed:
/
/	data changed:
/
/	procedures called:
/		get_rnd_control
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
special_round_test:	/proc
	movb	%al,%ah				/ save sign
	call	get_rnd_control			/get rounding mode
	cmpb	rnd_down,%al
	jne	round_up_			/ branch if not down
	andb	%ah,%ah
	jz	is_true				/ true if +rnd_down
is_false:
	xorb	%al,%al				/ sets zf.
	ret
round_up_:
	cmpb	rnd_up,%al
	jne	is_false			/ branch if not up
	andb	%ah,%ah
	jz	is_false			/ branch if +rnd_up
is_true:
	orb	true,%al				/ clears zf.
exit_round_test:
	ret
/special_round_test	endp
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			directed_round_test:
/			"""""""""""""""""""
/	function:
/		test for round control (up or down).
/
/	inputs:
/		none
/
/	outputs:
/		returns true (in al) and clears zf if the round control 
/		is rnd_up or rnd_down.  otherwise, returns false and sets zf.
/
/	data accessed:
/
/	data changed:
/
/	procedures called:
/		get_rnd_conrol
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
directed_round_test:	/proc
	call	get_rnd_control
	cmpb	rnd_down,%al
	je	is_true
	cmpb	rnd_up,%al
	je	is_true
	jmp	is_false
/directed_round_test	endp
/
/a_med	ends
/
/	end
