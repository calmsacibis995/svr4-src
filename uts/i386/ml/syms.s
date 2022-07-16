/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

	.ident	"@(#)kern-ml:syms.s	1.3"

/	Due to mapfile/vuifile compatibility, the usage of
/	this file, and in this particular fashion is
/	kludgy but necessary.

/	The following symbols are defined here for CI5 cplu.
/	CI5 ld will not allow global arbitrary
/	assignments in mapfiles 

	.text
	.globl	stext
	.align	0x1000
	.set	stext, .

	.data
	.globl	sdata
	.align	0x1000
	.set 	sdata, .

	.bss
	.globl	sbss
	.align	0x1000
	.set 	sbss, .

	.section	BKI, "a"
	.align	4
	.value		0x02


	.globl	df_stack
	.set	df_stack, 0xD0000000

#if defined	(MB1)
	.globl	enetmem
	.globl	bootinfo
	.globl	kspt0
	.globl	kpd0
	.globl	bpsinfo
	.globl	lma_start

	.set	Page0,0xD0000000
	.set	enetmem	, Page0 + 0x1600
	.set	bootinfo, Page0 + 0x1700
	.set 	kspt0	, Page0 + 0x2000
	.set	kpd0	, Page0 + 0x3000
	.set	bpsinfo	, Page0 + 0x10000
	.set	lma_start, Page0 + 0x10000
#endif
#if defined	(MB2)
	.globl	bootinfo
	.globl	kspt0
	.globl	kpd0
	.globl	bpsinfo

	.set	Page0,0xD0000000
	.set	bootinfo, Page0 + 0xd000
	.set 	kspt0	, Page0 + 0xe000
	.set	kpd0	, Page0 + 0xf000
	.set	bpsinfo	, Page0 + 0x10000
#endif
#if defined (AT386) || defined (MCA)
	.globl	bootinfo
	.globl	kspt0
	.globl	kpd0

	.set	Page0,0xD0000000
	.set	bootinfo, Page0 + 0x600
	.set 	kspt0	, Page0 + 0x1000
	.set	kpd0	, Page0 + 0x2000
#endif

	.globl	syssegs
	.set	syssegs, 0xd1000000

	.globl	piosegs
	.set	piosegs, 0xd2000000

	.globl	kvsegmap
	.set	kvsegmap, 0xd2400000

	.globl	kvsegu
	.set	kvsegu, 0xe0400000

	.globl	userstack
	.set	userstack,0x7ffffffc

	.globl	u
	.set	u,0xe0000000
