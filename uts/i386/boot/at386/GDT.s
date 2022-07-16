/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

/	Copyright (c) 1987, 1988 Microsoft Corporation
/	  All Rights Reserved

/	This Module contains Proprietary Information of Microsoft
/	Corporation and should be treated as Confidential.

	.file "GDT.s"

	.ident	"@(#)boot:boot/at386/GDT.s	1.1.2.1"

#include "bsymvals.h"

/	----------------------------------------------------
/ 	The GDT for protected mode operation

	.globl	GDTstart
	.globl	datadesc
	.globl	codedesc
	.globl	code16desc
	.globl	initcode
	.globl	initdata
	.globl	initcode16


GDTstart:
nulldesc:			/ offset = 0x0

	.value	0x0	
	.value	0x0	
	.byte	0x0	
	.byte	0x0	
	.byte	0x0	
	.byte	0x0	

flatdesc:			/ offset = 0x08

	.value	0xFFFF		/ segment limit 0..15
	.value	0x0		/ segment base 0..15
	.byte	0x0		/ segment base 16..23
	.byte	0x92		/ flags; A=0, Type=001, DPL=00, P=1
	.byte	0xCF		/ flags; Limit (16..19)=1111, AVL=0, G=1
	.byte	0x0		/ segment base 24..32

datadesc:			/ offset = 0x10

	.value	0xFFFF		/ segment limit 0..15
	.value	0x1000		/ segment base 0..15
	.byte	0x0		/ segment base 16..23; set for 4K
	.byte	0x92		/ flags; A=0, Type=001, DPL=00, P=1
	.byte	0x4F		/ flags; Limit (16..19)=1111, AVL=0, G=0, B=1
	.byte	0x0		/ segment base 24..32

codedesc:			/ offset = 0x18

	.value	0xFFFF		/ segment limit 0..15
	.value	0x1000		/ segment base 0..15
	.byte	0x0		/ segment base 16..23; set for 4k
	.byte	0x9E		/ flags; A=0, Type=111, DPL=00, P=1
	.byte	0x4F		/ flags; Limit (16..19)=1111, AVL=0, G=0, D=1
	.byte	0x0		/ segment base 24..32

code16desc:			/ offset = 0x20

	.value	0xFFFF		/ segment limit 0..15
	.value	0x1000		/ segment base 0..15
	.byte	0x0		/ segment base 16..23; set for 4k
	.byte	0x9E		/ flags; A=0, Type=111, DPL=00, P=1
	.byte	0x0F		/ flags; Limit (16..19)=1111, AVL=0, G=0, D=0
	.byte	0x0		/ segment base 24..32

initdata:			/ offset = 0x28

	.value	0xFFFF		/ segment limit 0..15
	.value	0xA000		/ segment base 0..15
	.byte	0x0		/ segment base 16..23; initially set for 40k
	.byte	0x92		/ flags; A=0, Type=001, DPL=00, P=1
	.byte	0x4F		/ flags; Limit (16..19)=1111, AVL=0, G=0, B=1
	.byte	0x0		/ segment base 24..32

initcode:			/ offset = 0x30

	.value	0xFFFF		/ segment limit 0..15
	.value	0xA000		/ segment base 0..15
	.byte	0x0		/ segment base 16..23; initially set for 40k
	.byte	0x9E		/ flags; A=0, Type=111, DPL=00, P=1
	.byte	0x4F		/ flags; Limit (16..19)=1111, AVL=0, G=0, D=1
	.byte	0x0		/ segment base 24..32

initcode16:			/ offset = 0x38

	.value	0xFFFF		/ segment limit 0..15
	.value	0xA000		/ segment base 0..15
	.byte	0x0		/ segment base 16..23; initially set for 40k
	.byte	0x9E		/ flags; A=0, Type=111, DPL=00, P=1
	.byte	0x0F		/ flags; Limit (16..19)=1111, AVL=0, G=0, D=0
	.byte	0x0		/ segment base 24..32


/	In-memory GDT pointer for the lgdt call

	.globl GDTptr
	.globl gdtlimit
	.globl gdtbase

GDTptr:	
gdtlimit:
	.value	GDTSIZE
gdtbase:
	.long	GDTstart + 0x1000
