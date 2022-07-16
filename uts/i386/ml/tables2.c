/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)kern-ml:tables2.c	1.3.1.1"

#include	"sys/types.h"
#include	"sys/immu.h"
#include	"sys/tss.h"
#include	"sys/seg.h"
#include	"sys/signal.h"
#include	"sys/fs/s5dir.h"
#include	"sys/user.h"
#ifdef VPIX
#include        "sys/param.h"
#include        "sys/sysmacros.h"
#include	"sys/proc.h"
#include        "sys/v86.h"
#endif

extern pte_t kpd0[];
extern char df_stack;	/* top of stack for double fault handler */

/*
 *	386 Interrupt Descriptor Table
 */
extern div0trap(), dbgtrap(), nmiint(), brktrap(), ovflotrap(), boundstrap();
extern invoptrap(), ndptrap0(), ndptrap2(),ndptrap3(),ndptrap4(), ndptrap(), syserrtrap(), invaltrap(), invtsstrap();
extern segnptrap(), stktrap(), gptrap(), pftrap(), ndperr();
extern overrun(), resvtrap();
extern vstart(), sys_call(), sig_clean(), cmnint();
#ifdef VPIX
extern v86gptrap();
#endif

/* And about 10 million interrupt handlers.....
 */
extern ivctM0(), ivctM1(), ivctM2(), ivctM3();
extern ivctM4(), ivctM5(), ivctM6(), ivctM7();
extern ivctM0S0(), ivctM0S1(), ivctM0S2(), ivctM0S3();
extern ivctM0S4(), ivctM0S5(), ivctM0S6(), ivctM0S7();
extern ivctM1S0(), ivctM1S1(), ivctM1S2(), ivctM1S3();
extern ivctM1S4(), ivctM1S5(), ivctM1S6(), ivctM1S7();
extern ivctM2S0(), ivctM2S1(), ivctM2S2(), ivctM2S3();
extern ivctM2S4(), ivctM2S5(), ivctM2S6(), ivctM2S7();
extern ivctM3S0(), ivctM3S1(), ivctM3S2(), ivctM3S3();
extern ivctM3S4(), ivctM3S5(), ivctM3S6(), ivctM3S7();
extern ivctM4S0(), ivctM4S1(), ivctM4S2(), ivctM4S3();
extern ivctM4S4(), ivctM4S5(), ivctM4S6(), ivctM4S7();
extern ivctM5S0(), ivctM5S1(), ivctM5S2(), ivctM5S3();
extern ivctM5S4(), ivctM5S5(), ivctM5S6(), ivctM5S7();
extern ivctM6S0(), ivctM6S1(), ivctM6S2(), ivctM6S3();
extern ivctM6S4(), ivctM6S5(), ivctM6S6(), ivctM6S7();
extern ivctM7S0(), ivctM7S1(), ivctM7S2(), ivctM7S3();
extern ivctM7S4(), ivctM7S5(), ivctM7S6(), ivctM7S7();

/*extern debugtrap();*/

extern struct tss386 ktss, ltss, dftss;

struct gate_desc idt[IDTSZ] = {
			MKKTRPG(div0trap),	/* 000 */
			MKKTRPG(dbgtrap), 	/* 001 */
			MKINTG(nmiint),		/* 002 */
			MKUTRPG(brktrap), 	/* 003 */
			MKUTRPG(ovflotrap),	/* 004 */
			MKKTRPG(boundstrap),	/* 005 */
			MKKTRPG(invoptrap),	/* 006 */
			MKKTRPG(ndptrap0),	/* 007 */
			MKGATE(0, DFTSSSEL, GATE_KACC|GATE_TSS), /* 008 */
			MKKTRPG(overrun),	/* 009 */
			MKKTRPG(invtsstrap),	/* 010 */
			MKKTRPG(segnptrap),	/* 011 */
			MKKTRPG(stktrap),	/* 012 */
			MKKTRPG(gptrap),	/* 013 */
			MKKTRPG(pftrap),	/* 014 */
			MKKTRPG(resvtrap),	/* 015 */
			MKUTRPG(ndperr),	/* 016 */
			MKKTRPG(invaltrap),	/* 017 */
			MKKTRPG(invaltrap),	/* 018 */
			MKKTRPG(invaltrap),	/* 019 */
			MKKTRPG(invaltrap),	/* 020 */
			MKKTRPG(invaltrap),	/* 021 */
			MKKTRPG(invaltrap),	/* 022 */
			MKKTRPG(invaltrap),	/* 023 */
			MKKTRPG(invaltrap),	/* 024 */
			MKKTRPG(invaltrap),	/* 025 */
			MKKTRPG(invaltrap),	/* 026 */
			MKKTRPG(invaltrap),	/* 027 */
			MKKTRPG(invaltrap),	/* 028 */
			MKKTRPG(invaltrap),	/* 029 */
			MKKTRPG(invaltrap),	/* 030 */
			MKKTRPG(invaltrap),	/* 031 */
			MKUTRPG(ndptrap2),	/* 032 */ /* for fp emul */
			MKKTRPG(invaltrap),	/* 033 */
			MKKTRPG(invaltrap),	/* 034 */
			MKKTRPG(invaltrap),	/* 035 */
			MKKTRPG(invaltrap),	/* 036 */
			MKKTRPG(invaltrap),	/* 037 */
			MKKTRPG(invaltrap),	/* 038 */
			MKKTRPG(invaltrap),	/* 039 */
			MKKTRPG(invaltrap),	/* 040 */
			MKKTRPG(invaltrap),	/* 041 */
			MKKTRPG(invaltrap),	/* 042 */
			MKKTRPG(invaltrap),	/* 043 */
			MKKTRPG(invaltrap),	/* 044 */
			MKKTRPG(invaltrap),	/* 045 */
			MKKTRPG(invaltrap),	/* 046 */
			MKKTRPG(invaltrap),	/* 047 */
			MKKTRPG(invaltrap),	/* 048 */
			MKKTRPG(invaltrap),	/* 049 */
			MKKTRPG(invaltrap),	/* 050 */
			MKKTRPG(invaltrap),	/* 051 */
			MKKTRPG(invaltrap),	/* 052 */
			MKKTRPG(invaltrap),	/* 053 */
			MKKTRPG(invaltrap),	/* 054 */
			MKKTRPG(invaltrap),	/* 055 */
			MKKTRPG(invaltrap),	/* 056 */
			MKKTRPG(invaltrap),	/* 057 */
			MKKTRPG(invaltrap),	/* 058 */
			MKKTRPG(invaltrap),	/* 059 */
			MKKTRPG(invaltrap),	/* 060 */
			MKKTRPG(invaltrap),	/* 061 */
			MKKTRPG(invaltrap),	/* 062 */
			MKKTRPG(invaltrap),	/* 063 */
			MKINTG(ivctM0),		/* 064 */
			MKINTG(ivctM1),		/* 065 */
			MKINTG(ivctM2),		/* 066 */
			MKINTG(ivctM3),		/* 067 */
			MKINTG(ivctM4),		/* 068 */
			MKINTG(ivctM5),		/* 069 */
			MKINTG(ivctM6),		/* 070 */
			MKINTG(ivctM7),		/* 071 */
			MKINTG(ivctM0S0),	/* 072 */
			MKINTG(ivctM0S1),	/* 073 */
			MKINTG(ivctM0S2),	/* 074 */
			MKINTG(ivctM0S3),	/* 075 */
			MKINTG(ivctM0S4),	/* 076 */
			MKINTG(ivctM0S5),	/* 077 */
			MKINTG(ivctM0S6),	/* 078 */
			MKINTG(ivctM0S7),	/* 079 */
			MKINTG(ivctM1S0),	/* 080 */
			MKINTG(ivctM1S1),	/* 081 */
			MKINTG(ivctM1S2),	/* 082 */
			MKINTG(ivctM1S3),	/* 083 */
			MKINTG(ivctM1S4),	/* 084 */
			MKINTG(ivctM1S5),	/* 085 */
			MKINTG(ivctM1S6),	/* 086 */
			MKINTG(ivctM1S7),	/* 087 */
			MKINTG(ivctM2S0),	/* 088 */
			MKINTG(ivctM2S1),	/* 089 */
			MKINTG(ivctM2S2),	/* 090 */
			MKINTG(ivctM2S3),	/* 091 */
			MKINTG(ivctM2S4),	/* 092 */
			MKINTG(ivctM2S5),	/* 093 */
			MKINTG(ivctM2S6),	/* 094 */
			MKINTG(ivctM2S7),	/* 095 */
			MKINTG(ivctM3S0),	/* 096 */
			MKINTG(ivctM3S1),	/* 097 */
			MKINTG(ivctM3S2),	/* 098 */
			MKINTG(ivctM3S3),	/* 099 */
			MKINTG(ivctM3S4),	/* 100 */
			MKINTG(ivctM3S5),	/* 101 */
			MKINTG(ivctM3S6),	/* 102 */
			MKINTG(ivctM3S7),	/* 103 */
			MKINTG(ivctM4S0),	/* 104 */
			MKINTG(ivctM4S1),	/* 105 */
			MKINTG(ivctM4S2),	/* 106 */
			MKINTG(ivctM4S3),	/* 107 */
			MKINTG(ivctM4S4),	/* 108 */
			MKINTG(ivctM4S5),	/* 109 */
			MKINTG(ivctM4S6),	/* 110 */
			MKINTG(ivctM4S7),	/* 111 */
			MKINTG(ivctM5S0),	/* 112 */
			MKINTG(ivctM5S1),	/* 113 */
			MKINTG(ivctM5S2),	/* 114 */
			MKINTG(ivctM5S3),	/* 115 */
			MKINTG(ivctM5S4),	/* 116 */
			MKINTG(ivctM5S5),	/* 117 */
			MKINTG(ivctM5S6),	/* 118 */
			MKINTG(ivctM5S7),	/* 119 */
			MKINTG(ivctM6S0),	/* 120 */
			MKINTG(ivctM6S1),	/* 121 */
			MKINTG(ivctM6S2),	/* 122 */
			MKINTG(ivctM6S3),	/* 123 */
			MKINTG(ivctM6S4),	/* 124 */
			MKINTG(ivctM6S5),	/* 125 */
			MKINTG(ivctM6S6),	/* 126 */
			MKINTG(ivctM6S7),	/* 127 */
			MKINTG(ivctM7S0),	/* 128 */
			MKINTG(ivctM7S1),	/* 129 */
			MKINTG(ivctM7S2),	/* 130 */
			MKINTG(ivctM7S3),	/* 131 */
			MKINTG(ivctM7S4),	/* 132 */
			MKINTG(ivctM7S5),	/* 133 */
			MKINTG(ivctM7S6),	/* 134 */
			MKINTG(ivctM7S7),	/* 135 */
			MKKTRPG(invaltrap),	/* 136 */
			MKKTRPG(invaltrap),	/* 137 */
			MKKTRPG(invaltrap),	/* 138 */
			MKKTRPG(invaltrap),	/* 139 */
			MKKTRPG(invaltrap),	/* 140 */
			MKKTRPG(invaltrap),	/* 141 */
			MKKTRPG(invaltrap),	/* 142 */
			MKKTRPG(invaltrap),	/* 143 */
			MKKTRPG(invaltrap),	/* 144 */
			MKKTRPG(invaltrap),	/* 145 */
			MKKTRPG(invaltrap),	/* 146 */
			MKKTRPG(invaltrap),	/* 147 */
			MKKTRPG(invaltrap),	/* 148 */
			MKKTRPG(invaltrap),	/* 149 */
			MKKTRPG(invaltrap),	/* 150 */
			MKKTRPG(invaltrap),	/* 151 */
			MKKTRPG(invaltrap),	/* 152 */
			MKKTRPG(invaltrap),	/* 153 */
			MKKTRPG(invaltrap),	/* 154 */
			MKKTRPG(invaltrap),	/* 155 */
			MKKTRPG(invaltrap),	/* 156 */
			MKKTRPG(invaltrap),	/* 157 */
			MKKTRPG(invaltrap),	/* 158 */
			MKKTRPG(invaltrap),	/* 159 */
			MKKTRPG(invaltrap),	/* 160 */
			MKKTRPG(invaltrap),	/* 161 */
			MKKTRPG(invaltrap),	/* 162 */
			MKKTRPG(invaltrap),	/* 163 */
			MKKTRPG(invaltrap),	/* 164 */
			MKKTRPG(invaltrap),	/* 165 */
			MKKTRPG(invaltrap),	/* 166 */
			MKKTRPG(invaltrap),	/* 167 */
			MKKTRPG(invaltrap),	/* 168 */
			MKKTRPG(invaltrap),	/* 169 */
			MKKTRPG(invaltrap),	/* 170 */
			MKKTRPG(invaltrap),	/* 171 */
			MKKTRPG(invaltrap),	/* 172 */
			MKKTRPG(invaltrap),	/* 173 */
			MKKTRPG(invaltrap),	/* 174 */
			MKKTRPG(invaltrap),	/* 175 */
			MKKTRPG(invaltrap),	/* 176 */
			MKKTRPG(invaltrap),	/* 177 */
			MKKTRPG(invaltrap),	/* 178 */
			MKKTRPG(invaltrap),	/* 179 */
			MKKTRPG(invaltrap),	/* 180 */
			MKKTRPG(invaltrap),	/* 181 */
			MKKTRPG(invaltrap),	/* 182 */
			MKKTRPG(invaltrap),	/* 183 */
			MKKTRPG(invaltrap),	/* 184 */
			MKKTRPG(invaltrap),	/* 185 */
			MKKTRPG(invaltrap),	/* 186 */
			MKKTRPG(invaltrap),	/* 187 */
			MKKTRPG(invaltrap),	/* 188 */
			MKKTRPG(invaltrap),	/* 189 */
			MKKTRPG(invaltrap),	/* 190 */
			MKKTRPG(invaltrap),	/* 191 */
			MKKTRPG(invaltrap),	/* 192 */
			MKKTRPG(invaltrap),	/* 193 */
			MKKTRPG(invaltrap),	/* 194 */
			MKKTRPG(invaltrap),	/* 195 */
			MKKTRPG(invaltrap),	/* 196 */
			MKKTRPG(invaltrap),	/* 197 */
			MKKTRPG(invaltrap),	/* 198 */
			MKKTRPG(invaltrap),	/* 199 */
			MKKTRPG(invaltrap),	/* 200 */
			MKKTRPG(invaltrap),	/* 201 */
			MKKTRPG(invaltrap),	/* 202 */
			MKKTRPG(invaltrap),	/* 203 */
			MKKTRPG(invaltrap),	/* 204 */
			MKKTRPG(invaltrap),	/* 205 */
			MKKTRPG(invaltrap),	/* 206 */
			MKKTRPG(invaltrap),	/* 207 */
			MKKTRPG(invaltrap),	/* 208 */
			MKKTRPG(invaltrap),	/* 209 */
			MKKTRPG(invaltrap),	/* 210 */
			MKKTRPG(invaltrap),	/* 211 */
			MKKTRPG(invaltrap),	/* 212 */
			MKKTRPG(invaltrap),	/* 213 */
			MKKTRPG(invaltrap),	/* 214 */
			MKKTRPG(invaltrap),	/* 215 */
			MKKTRPG(invaltrap),	/* 216 */
			MKKTRPG(invaltrap),	/* 217 */
			MKKTRPG(invaltrap),	/* 218 */
			MKKTRPG(invaltrap),	/* 219 */
			MKKTRPG(invaltrap),	/* 220 */
			MKKTRPG(invaltrap),	/* 221 */
			MKKTRPG(invaltrap),	/* 222 */
			MKKTRPG(invaltrap),	/* 223 */
			MKKTRPG(invaltrap),	/* 224 */
			MKKTRPG(invaltrap),	/* 225 */
			MKKTRPG(invaltrap),	/* 226 */
			MKKTRPG(invaltrap),	/* 227 */
			MKKTRPG(invaltrap),	/* 228 */
			MKKTRPG(invaltrap),	/* 229 */
			MKKTRPG(invaltrap),	/* 230 */
			MKKTRPG(invaltrap),	/* 231 */
			MKKTRPG(invaltrap),	/* 232 */
			MKKTRPG(invaltrap),	/* 233 */
			MKKTRPG(invaltrap),	/* 234 */
			MKKTRPG(invaltrap),	/* 235 */
			MKKTRPG(invaltrap),	/* 236 */
			MKKTRPG(invaltrap),	/* 237 */
			MKKTRPG(invaltrap),	/* 238 */
			MKKTRPG(invaltrap),	/* 239 */
			MKKTRPG(invaltrap),	/* 240 */
			MKKTRPG(invaltrap),	/* 241 */
			MKKTRPG(invaltrap),	/* 242 */
			MKKTRPG(invaltrap),	/* 243 */
			MKKTRPG(invaltrap),	/* 244 */
			MKKTRPG(invaltrap),	/* 245 */
			MKKTRPG(invaltrap),	/* 246 */
			MKKTRPG(invaltrap),	/* 247 */
			MKKTRPG(invaltrap),	/* 248 */
			MKKTRPG(invaltrap),	/* 249 */
			MKKTRPG(invaltrap),	/* 250 */
			MKKTRPG(invaltrap),	/* 251 */
			MKKTRPG(invaltrap),	/* 252 */
			MKKTRPG(invaltrap),	/* 253 */
			MKKTRPG(invaltrap),	/* 254 */
			MKKTRPG(invaltrap),	/* 255 */
};

#ifdef VPIX
/*
**  Temporary kludge. This is a second copy of the IDT with the some
**  vectors changed to be task gates to the user TSS. It is loaded
**  into the IDT register when we go into user mode for dual mode
**  processes only. The vector that has a task gate to the user
**  TSS is: the invalid opcode exception (vector 6).
*/
struct gate_desc idt2[IDTSZ] = {
			MKKTRPG(div0trap),      /* 000 */
			MKKTRPG(dbgtrap),       /* 001 */
			MKINTG(nmiint),		/* 002 */
			MKUTRPG(brktrap),       /* 003 */
			MKUTRPG(ovflotrap),     /* 004 */
			MKKTRPG(boundstrap),	/* 005 */
			/* MKKTRPG(invoptrap), */    /* 006 */
			MKGATE(0, UTSSSEL, GATE_KACC|GATE_TSS), /* 006 */
			MKINTG(ndptrap3),	/* 007 */
			MKGATE(0, DFTSSSEL, GATE_KACC|GATE_TSS), /* 008 */
			MKKTRPG(overrun),	/* 009 */
			MKKTRPG(invtsstrap),	/* 010 */
			MKKTRPG(segnptrap),	/* 011 */
			MKKTRPG(stktrap),	/* 012 */
			/* MKKTRPG(gptrap),    */    /* 013 */
			MKINTG(v86gptrap),      /* 013 */
			MKKTRPG(pftrap),	/* 014 */
			MKKTRPG(resvtrap),	/* 015 */
			MKUTRPG(ndperr),	/* 016 */
			MKKTRPG(invaltrap),	/* 017 */
			MKKTRPG(invaltrap),	/* 018 */
			MKKTRPG(invaltrap),	/* 019 */
			MKKTRPG(invaltrap),	/* 020 */
			MKKTRPG(invaltrap),	/* 021 */
			MKKTRPG(invaltrap),	/* 022 */
			MKKTRPG(invaltrap),	/* 023 */
			MKKTRPG(invaltrap),	/* 024 */
			MKKTRPG(invaltrap),	/* 025 */
			MKKTRPG(invaltrap),	/* 026 */
			MKKTRPG(invaltrap),	/* 027 */
			MKKTRPG(invaltrap),	/* 028 */
			MKKTRPG(invaltrap),	/* 029 */
			MKKTRPG(invaltrap),	/* 030 */
			MKKTRPG(invaltrap),	/* 031 */
			MKUTRPG(ndptrap4),	/* 032 */ /* for fp emul */
			MKKTRPG(invaltrap),	/* 033 */
			MKKTRPG(invaltrap),	/* 034 */
			MKKTRPG(invaltrap),	/* 035 */
			MKKTRPG(invaltrap),	/* 036 */
			MKKTRPG(invaltrap),	/* 037 */
			MKKTRPG(invaltrap),	/* 038 */
			MKKTRPG(invaltrap),	/* 039 */
			MKKTRPG(invaltrap),	/* 040 */
			MKKTRPG(invaltrap),	/* 041 */
			MKKTRPG(invaltrap),	/* 042 */
			MKKTRPG(invaltrap),	/* 043 */
			MKKTRPG(invaltrap),	/* 044 */
			MKKTRPG(invaltrap),	/* 045 */
			MKKTRPG(invaltrap),	/* 046 */
			MKKTRPG(invaltrap),	/* 047 */
			MKKTRPG(invaltrap),	/* 048 */
			MKKTRPG(invaltrap),	/* 049 */
			MKKTRPG(invaltrap),	/* 050 */
			MKKTRPG(invaltrap),	/* 051 */
			MKKTRPG(invaltrap),	/* 052 */
			MKKTRPG(invaltrap),	/* 053 */
			MKKTRPG(invaltrap),	/* 054 */
			MKKTRPG(invaltrap),	/* 055 */
			MKKTRPG(invaltrap),	/* 056 */
			MKKTRPG(invaltrap),	/* 057 */
			MKKTRPG(invaltrap),	/* 058 */
			MKKTRPG(invaltrap),	/* 059 */
			MKKTRPG(invaltrap),	/* 060 */
			MKKTRPG(invaltrap),	/* 061 */
			MKKTRPG(invaltrap),	/* 062 */
			MKKTRPG(invaltrap),	/* 063 */
			MKINTG(ivctM0),		/* 064 */
			MKINTG(ivctM1),		/* 065 */
			MKINTG(ivctM2),		/* 066 */
			MKINTG(ivctM3),		/* 067 */
			MKINTG(ivctM4),		/* 068 */
			MKINTG(ivctM5),		/* 069 */
			MKINTG(ivctM6),		/* 070 */
			MKINTG(ivctM7),		/* 071 */
			MKINTG(ivctM0S0),	/* 072 */
			MKINTG(ivctM0S1),	/* 073 */
			MKINTG(ivctM0S2),	/* 074 */
			MKINTG(ivctM0S3),	/* 075 */
			MKINTG(ivctM0S4),	/* 076 */
			MKINTG(ivctM0S5),	/* 077 */
			MKINTG(ivctM0S6),	/* 078 */
			MKINTG(ivctM0S7),	/* 079 */
			MKINTG(ivctM1S0),	/* 080 */
			MKINTG(ivctM1S1),	/* 081 */
			MKINTG(ivctM1S2),	/* 082 */
			MKINTG(ivctM1S3),	/* 083 */
			MKINTG(ivctM1S4),	/* 084 */
			MKINTG(ivctM1S5),	/* 085 */
			MKINTG(ivctM1S6),	/* 086 */
			MKINTG(ivctM1S7),	/* 087 */
			MKINTG(ivctM2S0),	/* 088 */
			MKINTG(ivctM2S1),	/* 089 */
			MKINTG(ivctM2S2),	/* 090 */
			MKINTG(ivctM2S3),	/* 091 */
			MKINTG(ivctM2S4),	/* 092 */
			MKINTG(ivctM2S5),	/* 093 */
			MKINTG(ivctM2S6),	/* 094 */
			MKINTG(ivctM2S7),	/* 095 */
			MKINTG(ivctM3S0),	/* 096 */
			MKINTG(ivctM3S1),	/* 097 */
			MKINTG(ivctM3S2),	/* 098 */
			MKINTG(ivctM3S3),	/* 099 */
			MKINTG(ivctM3S4),	/* 100 */
			MKINTG(ivctM3S5),	/* 101 */
			MKINTG(ivctM3S6),	/* 102 */
			MKINTG(ivctM3S7),	/* 103 */
			MKINTG(ivctM4S0),	/* 104 */
			MKINTG(ivctM4S1),	/* 105 */
			MKINTG(ivctM4S2),	/* 106 */
			MKINTG(ivctM4S3),	/* 107 */
			MKINTG(ivctM4S4),	/* 108 */
			MKINTG(ivctM4S5),	/* 109 */
			MKINTG(ivctM4S6),	/* 110 */
			MKINTG(ivctM4S7),	/* 111 */
			MKINTG(ivctM5S0),	/* 112 */
			MKINTG(ivctM5S1),	/* 113 */
			MKINTG(ivctM5S2),	/* 114 */
			MKINTG(ivctM5S3),	/* 115 */
			MKINTG(ivctM5S4),	/* 116 */
			MKINTG(ivctM5S5),	/* 117 */
			MKINTG(ivctM5S6),	/* 118 */
			MKINTG(ivctM5S7),	/* 119 */
			MKINTG(ivctM6S0),	/* 120 */
			MKINTG(ivctM6S1),	/* 121 */
			MKINTG(ivctM6S2),	/* 122 */
			MKINTG(ivctM6S3),	/* 123 */
			MKINTG(ivctM6S4),	/* 124 */
			MKINTG(ivctM6S5),	/* 125 */
			MKINTG(ivctM6S6),	/* 126 */
			MKINTG(ivctM6S7),	/* 127 */
			MKINTG(ivctM7S0),	/* 128 */
			MKINTG(ivctM7S1),	/* 129 */
			MKINTG(ivctM7S2),	/* 130 */
			MKINTG(ivctM7S3),	/* 131 */
			MKINTG(ivctM7S4),	/* 132 */
			MKINTG(ivctM7S5),	/* 133 */
			MKINTG(ivctM7S6),	/* 134 */
			MKINTG(ivctM7S7),	/* 135 */
			MKKTRPG(invaltrap),	/* 136 */
			MKKTRPG(invaltrap),	/* 137 */
			MKKTRPG(invaltrap),	/* 138 */
			MKKTRPG(invaltrap),	/* 139 */
			MKKTRPG(invaltrap),	/* 140 */
			MKKTRPG(invaltrap),	/* 141 */
			MKKTRPG(invaltrap),	/* 142 */
			MKKTRPG(invaltrap),	/* 143 */
			MKKTRPG(invaltrap),	/* 144 */
			MKKTRPG(invaltrap),	/* 145 */
			MKKTRPG(invaltrap),	/* 146 */
			MKKTRPG(invaltrap),	/* 147 */
			MKKTRPG(invaltrap),	/* 148 */
			MKKTRPG(invaltrap),	/* 149 */
			MKKTRPG(invaltrap),	/* 150 */
			MKKTRPG(invaltrap),	/* 151 */
			MKKTRPG(invaltrap),	/* 152 */
			MKKTRPG(invaltrap),	/* 153 */
			MKKTRPG(invaltrap),	/* 154 */
			MKKTRPG(invaltrap),	/* 155 */
			MKKTRPG(invaltrap),	/* 156 */
			MKKTRPG(invaltrap),	/* 157 */
			MKKTRPG(invaltrap),	/* 158 */
			MKKTRPG(invaltrap),	/* 159 */
			MKKTRPG(invaltrap),	/* 160 */
			MKKTRPG(invaltrap),	/* 161 */
			MKKTRPG(invaltrap),	/* 162 */
			MKKTRPG(invaltrap),	/* 163 */
			MKKTRPG(invaltrap),	/* 164 */
			MKKTRPG(invaltrap),	/* 165 */
			MKKTRPG(invaltrap),	/* 166 */
			MKKTRPG(invaltrap),	/* 167 */
			MKKTRPG(invaltrap),	/* 168 */
			MKKTRPG(invaltrap),	/* 169 */
			MKKTRPG(invaltrap),	/* 170 */
			MKKTRPG(invaltrap),	/* 171 */
			MKKTRPG(invaltrap),	/* 172 */
			MKKTRPG(invaltrap),	/* 173 */
			MKKTRPG(invaltrap),	/* 174 */
			MKKTRPG(invaltrap),	/* 175 */
			MKKTRPG(invaltrap),	/* 176 */
			MKKTRPG(invaltrap),	/* 177 */
			MKKTRPG(invaltrap),	/* 178 */
			MKKTRPG(invaltrap),	/* 179 */
			MKKTRPG(invaltrap),	/* 180 */
			MKKTRPG(invaltrap),	/* 181 */
			MKKTRPG(invaltrap),	/* 182 */
			MKKTRPG(invaltrap),	/* 183 */
			MKKTRPG(invaltrap),	/* 184 */
			MKKTRPG(invaltrap),	/* 185 */
			MKKTRPG(invaltrap),	/* 186 */
			MKKTRPG(invaltrap),	/* 187 */
			MKKTRPG(invaltrap),	/* 188 */
			MKKTRPG(invaltrap),	/* 189 */
			MKKTRPG(invaltrap),	/* 190 */
			MKKTRPG(invaltrap),	/* 191 */
			MKKTRPG(invaltrap),	/* 192 */
			MKKTRPG(invaltrap),	/* 193 */
			MKKTRPG(invaltrap),	/* 194 */
			MKKTRPG(invaltrap),	/* 195 */
			MKKTRPG(invaltrap),	/* 196 */
			MKKTRPG(invaltrap),	/* 197 */
			MKKTRPG(invaltrap),	/* 198 */
			MKKTRPG(invaltrap),	/* 199 */
			MKKTRPG(invaltrap),	/* 200 */
			MKKTRPG(invaltrap),	/* 201 */
			MKKTRPG(invaltrap),	/* 202 */
			MKKTRPG(invaltrap),	/* 203 */
			MKKTRPG(invaltrap),	/* 204 */
			MKKTRPG(invaltrap),	/* 205 */
			MKKTRPG(invaltrap),	/* 206 */
			MKKTRPG(invaltrap),	/* 207 */
			MKKTRPG(invaltrap),	/* 208 */
			MKKTRPG(invaltrap),	/* 209 */
			MKKTRPG(invaltrap),	/* 210 */
			MKKTRPG(invaltrap),	/* 211 */
			MKKTRPG(invaltrap),	/* 212 */
			MKKTRPG(invaltrap),	/* 213 */
			MKKTRPG(invaltrap),	/* 214 */
			MKKTRPG(invaltrap),	/* 215 */
			MKKTRPG(invaltrap),	/* 216 */
			MKKTRPG(invaltrap),	/* 217 */
			MKKTRPG(invaltrap),	/* 218 */
			MKKTRPG(invaltrap),	/* 219 */
			MKKTRPG(invaltrap),	/* 220 */
			MKKTRPG(invaltrap),	/* 221 */
			MKKTRPG(invaltrap),	/* 222 */
			MKKTRPG(invaltrap),	/* 223 */
			MKKTRPG(invaltrap),	/* 224 */
			MKKTRPG(invaltrap),	/* 225 */
			MKKTRPG(invaltrap),	/* 226 */
			MKKTRPG(invaltrap),	/* 227 */
			MKKTRPG(invaltrap),	/* 228 */
			MKKTRPG(invaltrap),	/* 229 */
			MKKTRPG(invaltrap),	/* 230 */
			MKKTRPG(invaltrap),	/* 231 */
			MKKTRPG(invaltrap),	/* 232 */
			MKKTRPG(invaltrap),	/* 233 */
			MKKTRPG(invaltrap),	/* 234 */
			MKKTRPG(invaltrap),	/* 235 */
			MKKTRPG(invaltrap),	/* 236 */
			MKKTRPG(invaltrap),	/* 237 */
			MKKTRPG(invaltrap),	/* 238 */
			MKKTRPG(invaltrap),	/* 239 */
			MKKTRPG(invaltrap),	/* 240 */
			MKKTRPG(invaltrap),	/* 241 */
			MKKTRPG(invaltrap),	/* 242 */
			MKKTRPG(invaltrap),	/* 243 */
			MKKTRPG(invaltrap),	/* 244 */
			MKKTRPG(invaltrap),	/* 245 */
			MKKTRPG(invaltrap),	/* 246 */
			MKKTRPG(invaltrap),	/* 247 */
			MKKTRPG(invaltrap),	/* 248 */
			MKKTRPG(invaltrap),	/* 249 */
			MKKTRPG(invaltrap),	/* 250 */
			MKKTRPG(invaltrap),	/* 251 */
			MKKTRPG(invaltrap),	/* 252 */
			MKKTRPG(invaltrap),	/* 253 */
			MKKTRPG(invaltrap),	/* 254 */
			MKKTRPG(invaltrap),	/* 255 */
};
#endif

/*
 *	386 Global Descriptor Table
 */

struct seg_desc gdt[GDTSZ] = {
		MKDSCR(0L,0,0,0) ,		/* 00 */
	MKDSCR((char *)&gdt[0],GDTSZ*sizeof(struct seg_desc)-1,0x93,0),/* 01 */
	MKDSCR((char *)&idt[0],IDTSZ*sizeof(struct gate_desc)-1,0x92,0),/* 02 */
	MKDSCR(0x500L,0x5F,0x82,0) ,		/* 03 two dot */
	MKDSCR(0x640L,0x67,0x89,0) ,	/* 04 two dot */
	MKDSCR(0x000L,0x00,0x00,0) ,	/* 05 two dot */
	MKDSCR(0x5C0L,0x67,0xE9,0) , 	/* 06 two dot */
	MKDSCR(0xE00L,0x16B,0xF3,0) , 	/* 07 one ex */
	MKDSCR(0x0FFFF977A,0x87,0xFE,0) ,	/* 08 one ex */
	MKDSCR(0L,0,0,0) ,		/* 09 */
	MKDSCR(0L,0,0,0) ,		/* 10 */
	MKDSCR(0L,0L,0,0) ,		/* 11 */
	MKDSCR(0L,0L,0,0) ,		/* 12 */
	MKDSCR(0L,0L,0,0) ,		/* 13 */
	MKDSCR(0L,0L,0,0) ,		/* 14 */
	MKDSCR(0L,0L,0,0) ,		/* 15 */
	MKDSCR(0L,0L,0,0) ,		/* 16 */
	MKDSCR(0L,0L,0,0) ,		/* 17 */
	MKDSCR(0L,0L,0,0) ,		/* 18 */
	MKDSCR(0L,0L,0,0) ,		/* 19 */
	MKDSCR(0L,0L,0,0) ,		/* 20 */
	MKDSCR(0L,0L,0,0) ,		/* 21 */
	MKDSCR(0L,0L,0,0) ,		/* 22 */
	MKDSCR(0L,0L,0,0) ,		/* 23 */
	MKDSCR(0L,0L,0,0) ,		/* 24 */
	MKDSCR(0L,0L,0,0) ,		/* 25 */
	MKDSCR(0L,0L,0,0) ,		/* 26 */
	MKDSCR(0L,0L,0,0) ,		/* 27 */
	MKDSCR(0L,0L,0,0) ,		/* 28 */
	MKDSCR(0L,0L,0,0) ,		/* 29 */
	MKDSCR(0L,0L,0,0) ,		/* 30 */
	MKDSCR(0L,0L,0,0) ,		/* 31 */
	MKDSCR(0L,0L,0,0) ,		/* 32 */
	MKDSCR(0L,0L,0,0) ,		/* 33 */
	MKDSCR(0L,0L,0,0) ,		/* 34 */
	MKDSCR(0L,0L,0,0) ,		/* 35 */
	MKDSCR(0L,0L,0,0) ,		/* 36 */
	MKDSCR(0L,0L,0,0) ,		/* 37 */
	MKDSCR(0L,0L,0,0) ,		/* 38 */
	MKDSCR(0L,0L,0,0) ,		/* 39 */
	MKDSCR(0L,(MINLDTSZ+1)*sizeof(struct seg_desc)-1,LDT_KACC1,LDT_ACC2),/* 40 */
	MKDSCR((char *)&ktss,sizeof(struct tss386)-1,TSS3_KACC1,TSS_ACC2) ,/* 41 */
	MKDSCR((char *)&ktss,sizeof(struct tss386)-1,TSS3_KACC1,TSS_ACC2) ,/* 42 */
	MKDSCR(0L,0xFFFFF,KTEXT_ACC1,TEXT_ACC2) ,		/* 43 */
	MKDSCR(0L,0xFFFFF,KDATA_ACC1,DATA_ACC2) ,		/* 44 */
	MKDSCR((char *)&dftss,sizeof(struct tss386)-1,TSS3_KACC1,TSS_ACC2),/* 45 */
	MKDSCR((char *)&ltss,sizeof(struct tss386)-1,TSS3_KACC1,TSS_ACC2),/* 46 */
	MKDSCR(0L,0L,0,0) ,		/* 47 */
	MKDSCR(0L,0L,0,0) ,		/* 48 */
#ifdef VPIX
	MKDSCR(XTSSADDR,0L,TSS3_KACC1,TSS_ACC2),/* 49 */
#else
	MKDSCR(0L,0L,0,0) ,		/* 49 */
#endif
	MKDSCR(0L,0xFFFFF,KTEXT_ACC1,TEXT_ACC2_S) ,	/* 50 */ /* fp emul */
	MKDSCR(0L,0L,0,0) ,		/* 51 */
	MKDSCR(0L,0L,0,0) ,		/* 52 */
	MKDSCR(0L,0L,0,0) ,		/* 53 */
	MKDSCR(0L,0L,0,0) ,		/* 54 */
						/* other entries all zero */
};

int gdtend = 0;         /* used by uprt.s to find end of gdt */
struct gate_desc monidt[MONIDTSZ] = {
			MKKTRPG(0L),	/* 000 */
			MKKTRPG(0L),	/* 001 */
			MKKTRPG(0L),	/* 002 */
			MKKTRPG(0L),	/* 003 */
			MKKTRPG(0L),	/* 004 */
			MKKTRPG(0L),	/* 005 */
			MKKTRPG(0L),	/* 006 */
			MKKTRPG(0L),	/* 007 */
			MKKTRPG(0L),	/* 008 */
			MKKTRPG(0L),	/* 009 */
			MKKTRPG(0L),	/* 010 */
			MKKTRPG(0L),	/* 011 */
			MKKTRPG(0L),	/* 012 */
			MKKTRPG(0L),	/* 013 */
			MKKTRPG(0L),	/* 014 */
			MKKTRPG(0L),	/* 015 */
};

struct tss386 ltss = {0};	/* uprt.s dumps stuff here. It's never read */

struct tss386 ktss = {
			0L,
/*			(unsigned long) ((char *)&u + KSTKSZ),*/
			(unsigned long)(&df_stack+0xFFC),
			(unsigned long)KDSSEL,
			(unsigned long) ((char *)&u + KSTKSZ),
			(unsigned long)KDSSEL,
			(unsigned long) ((char *)&u + KSTKSZ),
			(unsigned long)KDSSEL,
			(unsigned long)(&kpd0[0])-KVSBASE,      /* cr3 */
			(unsigned long) vstart,
			0L,				/* flags */
			0L,
			0L,
			0L,
			0L,
/*			(unsigned long) ((char *)&u+KSTKSZ),*/
			(unsigned long)(&df_stack+0xFFC),
			0L,
			0L,
			0L,
			(unsigned long)KDSSEL,
			(unsigned long)KCSSEL,
			(unsigned long)KDSSEL,
			(unsigned long)KDSSEL,
			0L,
			0L,
			0L,
			0xDFFF0000L,
};

struct tss386 dftss = {
			0L,
			(unsigned long)(&df_stack+0xFFC),
			(unsigned long)KDSSEL,
			0L,
			0L,
			0L,
			0L,
			(unsigned long)(&kpd0[0])-KVSBASE,      /* cr3 */
			(unsigned long) syserrtrap,
			0L,				/* flags */
			0L,
			0L,
			0L,
			0L,
			(unsigned long)(&df_stack+0xFFC),
			0L,
			0L,
			0L,
			(unsigned long)KDSSEL,
			(unsigned long)KCSSEL,
			(unsigned long)KDSSEL,
			(unsigned long)KDSSEL,
			(unsigned long)KDSSEL,
			(unsigned long)KDSSEL,
			0L,                             /* LDT selector */
			0xDFFF0000L,
};

struct gate_desc scall_dscr = {
		(ulong)sys_call, KCSSEL, 1, GATE_UACC|GATE_386CALL
};

struct gate_desc sigret_dscr = {
		(ulong)sig_clean, KCSSEL, 1, GATE_UACC|GATE_386CALL
};

/* Here we will store the segment:offset values that we lifted */
/* out of the fake idt (monidt) that we had the monitor initialize. */
/* Note that if mon1sel and mon3sel are zero, there is no monitor */

unsigned short	mon1sel = 0;
unsigned short	mon3sel = 0;
unsigned long	mon1off = 0;
unsigned long	mon3off = 0;

char nmi_stack[KSTKSZ];		/* stack for nmi's during resume() */
