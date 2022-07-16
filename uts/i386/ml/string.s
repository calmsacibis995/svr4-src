/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

	.ident	"@(#)kern-ml:string.s	1.3"

/ String functions copied from the C library.
/
/ Fast assembler language version of the following C-program for
/			strcmp
/ which represents the "standard" for the C-library.

/	/*
/	 * Compare strings:  s1>s2: >0  s1==s2: 0  s1<s2: <0
/	 */
/
/	int
/	strcmp(s1, s2)
/	register char *s1, *s2;
/	{
/
/		if(s1 == s2)
/			return(0);
/		while(*s1 == *s2++)
/			if(*s1++ == '\0')
/				return(0);
/		return(*s1 - *--s2);
/	}


	.globl	strcmp
	.align	4

strcmp:
	pushl	%edi
	pushl	%esi
	movl	12(%esp),%esi	/ %edi = address of string 1
	movl	16(%esp),%edi	/ %esi = address of string 2
	cmpl	%esi,%edi	/ s1 == s2 ?
	je	.equal		/ yes
.loop:				/ Iterate for cache performance
	slodb ; scab
	jne	.notequal
	testb	%al,%al
	je	.equal

	slodb ; scab
	jne	.notequal
	testb	%al,%al
	je	.equal

	slodb ; scab
	jne	.notequal
	testb	%al,%al
	je	.equal

	slodb ; scab
	jne	.notequal
	testb	%al,%al
	je	.equal

	slodb ; scab
	jne	.notequal
	testb	%al,%al
	je	.equal

	slodb ; scab
	jne	.notequal
	testb	%al,%al
	je	.equal

	slodb ; scab
	jne	.notequal
	testb	%al,%al
	je	.equal

	slodb ; scab
	jne	.notequal
	testb	%al,%al
	jne	.loop

.equal:
	xorl	%eax,%eax
	popl	%esi
	popl	%edi		/ restore registers
	ret

.notequal:
	movzbl	%al,%eax
	movzbl	-1(%edi),%edx
	subl	%edx,%eax
	popl	%esi
	popl	%edi		/ restore registers
	ret


/ Fast assembler language version of the following C-program
/			strlen
/ which represents the "standard" for the C-library.
/
/ Given string s, return length (not including the terminating null).

/	strlen(s)
/	register char	*s;
/	{
/		register n;
/	
/		n = 0;
/		while (*s++)
/			n++;
/		return(n);
/	}


	.globl	strlen
	.align	4

strlen:
	pushl	%edi		/ save register variables

	movl	8(%esp),%edi	/ string address
	xorl	%eax,%eax	/ %al = 0
	movl	$-1,%ecx	/ Start count backward from -1.
	repnz ; scab
	incl	%ecx		/ Chip pre-decrements.
	movl	%ecx,%eax	/ %eax = return values
	notl	%eax		/ Twos complement arith. rule.

	popl	%edi		/ restore register variables
	ret

/ Fast assembler language version of the following C-program
/			strcpy
/ which represents the "standard" for the C-library.
/
/ Copy string s2 to s1.  s1 must be large enough. Return s1.
/
/	char	*
/	strcpy(s1, s2)
/	register char	*s1, *s2;
/	{
/		register char	*os1;
/	
/		os1 = s1;
/		while (*s1++ = *s2++)
/			;
/		return(os1);
/	}


	.globl	strcpy
	.align	4

strcpy:
	pushl	%edi		/ save register variables
	movl	%esi,%edx

	movl	12(%esp),%edi	/ %edi = source string address
	xorl	%eax,%eax	/ %al = 0 (search for 0)
	movl	$-1,%ecx	/ length to look: lots
	repnz ; scab

	notl	%ecx		/ %ecx = length to move
	movl	12(%esp),%esi	/ %esi = source string address
	movl	8(%esp),%edi	/ %edi = destination string address
	movl	%ecx,%eax	/ %eax = length to move
	shrl	$2,%ecx		/ %ecx = words to move
	rep ; smovl

	movl	%eax,%ecx	/ %ecx = length to move
	andl	$3,%ecx		/ %ecx = leftover bytes to move
	rep ; smovb

	movl	8(%esp),%eax	/ %eax = returned dest string addr
	movl	%edx,%esi	/ restore register variables
	popl	%edi
	ret

/
/ Fast assembler version of `strcat'.
/

	.globl	strcat
	.align	4

strcat:
	pushl	%esi
	pushl	%edi

	movl	16(%esp), %esi	/ get source address
	movl	%esi, %edi	/ save for later
	xorl	%eax, %eax	/ search \0 char
	movl	$-1, %ecx	/ in many chars
	repnz ;	scab

	notl	%ecx		/ number to copy
	movl	%ecx, %edx	/ save for copy
	
	movl	12(%esp),%edi	/ get destination end address
	movl	$-1, %ecx	/ search for many
	repnz ; scab

	decl	%edi		/ backup 1 byte

	movl	%edx, %ecx	/ bytes to copy
	shrl	$2, %ecx	/ double to copy
	rep ;	smovl

	movl	%edx, %ecx	/ bytes to copy
	andl	$3, %ecx	/ mod 4
	rep ;	smovb

	movl	12(%esp), %eax

	popl	%edi
	popl	%esi
	ret

/
/ Fast assembler version of `strncat'.
/

	.globl	strncat
	.align	4

strncat:
	pushl	%esi
	pushl	%edi

	movl	20(%esp), %ecx	/ max number to copy
	jcxz	strncat_ret

	movl	%ecx, %edx
	movl	16(%esp), %esi	/ get source address
	movl	%esi, %edi	/ save for later
	xorl	%eax, %eax	/ search \0 char
	repnz ;	scab

	jne	strncat_no_null
	incl	%ecx
strncat_no_null:

	subl	%ecx, %edx	/ bytes to copy

	movl	12(%esp),%edi	/ get destination end address
	movl	$-1, %ecx	/ search for many
	repnz ; scab

	decl	%edi		/ backup 1 byte

	movl	%edx, %ecx	/ copy bytes - most suffixes are not very long;
	rep ;	smovb		/   no worth setup for longword moves

	sstob			/ null byte terminator

strncat_ret:
	movl	12(%esp), %eax

	popl	%edi
	popl	%esi
	ret
