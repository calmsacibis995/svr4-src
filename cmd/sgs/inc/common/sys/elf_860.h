/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)sgs-inc:common/sys/elf_860.h	1.1"

#ifndef _SYS_ELF_I860_H
#define _SYS_ELF_I860_H

#define R_860_32	0x01
#define R_860_COPY	0x02
#define R_860_GLOB_DAT	0x03
#define R_860_HAGOT	0x90
#define R_860_HAGOTOFF	0xA0
#define R_860_HAPC	0xB0
#define R_860_HIGH	0xC0
#define R_860_HIGHADJ	0x80
#define R_860_HIGOT	0xD0
#define R_860_HIGOTOFF	0xE0
#define R_860_JMP_SLOT	0x04
#define R_860_LOGOT0	0x50
#define R_860_LOGOT1	0x54
#define R_860_LOGOTOFF0	0x60
#define R_860_LOGOTOFF1	0x64
#define R_860_LOGOTOFF2	0x68
#define R_860_LOGOTOFF3	0x6C
#define R_860_LOPC	0x70
#define R_860_LOW0	0x40
#define R_860_LOW1	0x44
#define R_860_LOW2	0x48
#define R_860_LOW3	0x4C
#define R_860_NONE	0x00
#define R_860_PC16	0x32
#define R_860_PC26	0x30
#define R_860_PLT26	0x31
#define R_860_RELATIVE	0x05
#define R_860_SPGOT0	0x52
#define R_860_SPGOT1	0x56
#define R_860_SPGOTOFF0 0x62
#define R_860_SPGOTOFF1	0x66
#define R_860_SPLIT0	0x42
#define R_860_SPLIT1	0x46
#define R_860_SPLIT2	0x4A
#endif
