/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_INLINE_H
#define _SYS_INLINE_H

#ident	"@(#)head.sys:sys/inline.h	11.9.5.1"

#if !defined(lint)

asm	void flushtlb()	
{
	movl	%cr3, %eax
	movl	%eax, %cr3
}

asm	int _cr0()
{
	movl	%cr0, %eax
}

asm	int _cr2()
{
	movl	%cr2, %eax
}

asm	int _cr3()
{
	movl	%cr3, %eax
	andl	$0x7FFFFFFF, %eax
}

asm	void _wdr0(x)
{
%reg	x;
	movl    x, %db0
%ureg	x;
	movl    x, %db0
%con	x;
	movl	x,%eax	
	movl    %eax, %db0
%mem	x;
	movl	x,%eax
	movl    %eax, %db0
}

asm	void _wdr1(x)
{
%reg	x;
	movl    x, %db1
%ureg	x;
	movl    x, %db1
%con	x;
	movl	x,%eax
	movl    %eax, %db1
%mem	x;
	movl	x,%eax
	movl    %eax, %db1
}

asm	void _wdr2(x)
{
%reg	x;
	movl    x, %db2
%ureg	x;
	movl    x, %db2
%con	x;
	movl	x,%eax
	movl    %eax, %db2
%mem	x;
	movl	x,%eax
	movl    %eax, %db2
}

asm	void _wdr3(x)
{
%reg	x;
	movl    x, %db3
%ureg	x;
	movl    x, %db3
%con	x;
	movl	x,%eax
	movl    %eax, %db3
%mem	x;
	movl	x,%eax
	movl    %eax, %db3
}

asm	void _wdr6(x)
{
%reg	x;
	movl    x, %db6
%ureg	x;
	movl    x, %db6
%con	x;
	movl	x,%eax
	movl    %eax, %db6
%mem	x;
	movl	x,%eax
	movl    %eax, %db6
}

asm	void _wdr7(x)
{
%reg	x;
	movl    x, %db7
%ureg	x;
	movl    x, %db7
%con	x;
	movl	x,%eax
	movl    %eax, %db7
%mem	x;
	movl	x,%eax
	movl    %eax, %db7
}

asm	int _dr0()
{
	movl	%dr0, %eax
}

asm	int _dr1()
{
	movl	%dr1, %eax
}

asm	int _dr2()
{
	movl	%dr2, %eax
}

asm	int _dr3()
{
	movl	%dr3, %eax
}

asm	int _dr6()
{
	movl	%dr6, %eax
}

asm	int _dr7()
{
	movl	%dr7, %eax
}

asm	void loadtr(x)
{
%reg	x;
	movl    x,%eax
	ltr	%ax
%ureg	x;
	movl    x,%eax
	ltr	%ax
%con	x;
	movl	x,%eax
	ltr	%ax
%mem	x;
	movl	x,%eax
	ltr	%ax
}

asm     void outl(port,val)
{
%reg	port,val;
	movl	port, %edx
	movl	val, %eax
	outl	(%dx)
%reg	port; mem	val;
	movl	port, %edx
	movl    val, %eax
	outl	(%dx)
%mem	port; reg	val;
	movw	port, %dx
	movl	val, %eax
	outl	(%dx)
%mem	port,val;
	movw	port, %dx
	movl    val, %eax
	outl	(%dx)
}

asm	void outw(port,val)
{
%reg	port,val;
	movl	port, %edx
	movl	val, %eax
	data16
	outl	(%dx)
%reg	port; mem	val;
	movl	port, %edx
	movw	val, %ax
	data16
	outl	(%dx)
%mem	port; reg	val;
	movw	port, %dx
	movl	val, %eax
	data16
	outl	(%dx)
%mem	port,val;
	movw	port, %dx
	movw	val, %ax
	data16
	outl	(%dx)
}

asm	void outb(port,val)
{
%reg	port,val;
	movl	port, %edx
	movl	val, %eax
	outb	(%dx)
%reg	port; mem	val;
	movl	port, %edx
	movb	val, %al
	outb	(%dx)
%mem	port; reg	val;
	movw	port, %dx
	movl	val, %eax
	outb	(%dx)
%mem	port,val;
	movw	port, %dx
	movb	val, %al
	outb	(%dx)
}

asm     int inl(port)
{
%reg	port;
	movl	port, %edx
	inl	(%dx)
%mem	port;
	movw	port, %dx
	inl	(%dx)
}

asm	int inw(port)
{
%reg	port;
	subl    %eax, %eax
	movl	port, %edx
	data16
	inl	(%dx)
%mem	port;
	subl    %eax, %eax
	movw	port, %dx
	data16
	inl	(%dx)
}

asm	int inb(port)
{
%reg	port;
	subl    %eax, %eax
	movl	port, %edx
	inb	(%dx)
%mem	port;
	subl    %eax, %eax
	movw	port, %dx
	inb	(%dx)
}

asm     void intr_disable()
{
	pushfl
	cli
}

asm     void intr_restore()
{
	popfl
}

asm     void intr_enable()
{
	popfl
	sti
}

asm int struct_zero(addr, len)
{
%mem	addr; con	len;
	pushl	%edi
	pushl	addr
	movl	len, %ecx
	popl	%edi
	movl	$0, %eax
	rep
	sstob
	popl	%edi
%mem	addr; reg	len;
	pushl	%edi
	pushl	addr
	movl	len, %ecx
	popl	%edi
	movl	$0, %eax
	rep
	sstob
	popl	%edi
%mem	addr, len;
	pushl	%edi
	pushl	addr
	movl	len, %ecx
	popl	%edi
	movl	$0, %eax
	rep
	sstob
	popl	%edi
}

asm void copy_bytes(from, to, count)
{
%mem	from,to; con	count;
	pushl	%esi
	pushl	%edi
	pushl	from
	pushl	to
	movl	count, %ecx
	popl	%edi
	popl	%esi
	rep
	smovb
	popl	%edi
	popl	%esi
%mem	from,to; reg	count;
	pushl	%esi
	pushl	%edi
	pushl	from
	pushl	to
	movl	count, %ecx
	popl	%edi
	popl	%esi
	rep
	smovb
	popl	%edi
	popl	%esi
%mem	from,to,count;
	pushl	%esi
	pushl	%edi
	pushl	from
	pushl	to
	movl	count, %ecx
	popl	%edi
	popl	%esi
	rep
	smovb
	popl	%edi
	popl	%esi
}
#else	/* !defined(lint) */
/*
 *	Very fast byte-at-a-time copy, as opposed to bcopy, which is
 *	longword-at-a-time. For controler boards which can't handle 32 bit accesses.
*/
void copy_bytes(from, to, count)
register caddr_t from, to;
register int count;
{	*to = *from;
	while (--count)
		*(++to) = *(++from);
}

#if defined(__STDC__)

extern	void flushtlb(void);
extern	int _cr0(void);
extern	int _cr2(void);
extern	int _cr3(void);
extern	void _wdr0(ulong);
extern	void _wdr1(ulong);
extern	void _wdr2(ulong);
extern	void _wdr3(ulong);
extern	void _wdr6(ulong);
extern	void _wdr7(ulong);
extern	int _dr0(void);
extern	int _dr1(void);
extern	int _dr2(void);
extern	int _dr3(void);
extern	int _dr6(void);
extern	int _dr7(void);
extern	void loadtr(ulong);
extern  void outl(unsigned, ulong);
extern	void outw(unsigned,ulong);
extern	void outb(unsigned,ulong);
extern  int inl(unsigned);
extern	int inw(unsigned);
extern	int inb(unsigned);
extern  void intr_disable();
extern  void intr_restore();
extern  void intr_enable();
extern  int struct_zero(caddr_t, int);

#else	/* __STDC__ */

extern	void flushtlb();	
extern	int _cr0();
extern	int _cr2();
extern	int _cr3();
extern	void _wdr0();
extern	void _wdr1();
extern	void _wdr2();
extern	void _wdr3();
extern	void _wdr6();
extern	void _wdr7();
extern	int _dr0();
extern	int _dr1();
extern	int _dr2();
extern	int _dr3();
extern	int _dr6();
extern	int _dr7();
extern	void loadtr();
extern  void outl();
extern	void outw();
extern	void outb();
extern  int inl();
extern	int inw();
extern	int inb();
extern  void intr_disable();
extern  void intr_restore();
extern  void intr_enable();
extern  int struct_zero();

#endif	/* __STDC__ */

#endif	/* !defined(lint) */

#endif	/* _SYS_INLINE_H */

#ifdef	KPERF  /* This is for kernel performance tool */
asm	int
get_spl()
{
	movl	ipl, %eax
}
#endif	/* KPERF */
