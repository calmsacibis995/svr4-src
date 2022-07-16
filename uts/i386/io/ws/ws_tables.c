/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-io:ws/ws_tables.c	1.3.1.1"

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/at_ansi.h"
#include "sys/kd.h"
#include "sys/ascii.h"
#include "sys/termios.h"
#include "sys/stream.h"
#include "sys/strtty.h"
#include "sys/stropts.h"
#include "sys/proc.h"
#include "sys/xque.h"
#include "sys/ws/ws.h"

/*
 * This table is used to translate keyboard scan codes to ASCII character
 * sequences for the AT386 keyboard/display driver.  It is the default table,
 * and may be changed with system calls.
 */

pfxstate_t kdpfxstr = {0};

keymap_t kdkeymap = { 0x80, {		/* Number of scan codes */
/*                                                         ALT    SPECIAL    */
/* SCAN                        CTRL          ALT    ALT    CTRL   FUNC       */
/* CODE   BASE   SHIFT  CTRL   SHIFT  ALT    SHIFT  CTRL   SHIFT  FLAGS LOCK */
/*  0*/ {{K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP },0xff, L_O },
/*  1*/ {{A_ESC, A_ESC, A_ESC, A_ESC, A_ESC, A_ESC, A_ESC, A_ESC },0x00, L_O },
/*  2*/ {{'1',   '!',   '1',   '1',   K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_O },
/*  3*/ {{'2',   '@',   '2',   A_NUL, K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_O },
/*  4*/ {{'3',   '#',   '3',   '3',   K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_O },
/*  5*/ {{'4',   '$',   '4',   '4',   K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_O },
/*  6*/ {{'5',   '%',   '5',   '5',   K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_O },
/*  7*/ {{'6',   '^',   '6',   A_RS,  K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_O },
/*  8*/ {{'7',   '&',   '7',   '7',   K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_O },
/*  9*/ {{'8',   '*',   '8',   '8',   K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_O },
/*  a*/ {{'9',   '(',   '9',   '9',   K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_O },
/*  b*/ {{'0',   ')',   '0',   '0',   K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_O },
/*  c*/ {{'-',   '_',   '-',   A_US,  K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_O },
/*  d*/ {{'=',   '+',   '=',   '=',   K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_O },
/*  e*/ {{A_BS,  A_BS,  A_BS,  A_BS,  A_BS,  A_BS,  A_BS,  A_BS  },0x00, L_O },
/*  f*/ {{A_HT,  A_GS,  A_HT,  A_GS,  A_HT,  A_GS,  A_HT,  A_GS  },0x00, L_O },
/* 10*/ {{'q',   'Q',   A_DC1, A_DC1, K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_C },
/* 11*/ {{'w',   'W',   A_ETB, A_ETB, K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_C },
/* 12*/ {{'e',   'E',   A_ENQ, A_ENQ, K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_C },
/* 13*/ {{'r',   'R',   A_DC2, A_DC2, K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_C },
/* 14*/ {{'t',   'T',   A_DC4, A_DC4, K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_C },
/* 15*/ {{'y',   'Y',   A_EM,  A_EM,  K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_C },
/* 16*/ {{'u',   'U',   A_NAK, A_NAK, K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_C },
/* 17*/ {{'i',   'I',   A_HT,  A_HT,  K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_C },
/* 18*/ {{'o',   'O',   A_SI,  A_SI,  K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_C },
/* 19*/ {{'p',   'P',   A_DLE, A_DLE, K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_C },
/* 1a*/ {{'[',   '{',   A_ESC, K_NOP, K_ESN, K_ESN, K_NOP, K_NOP },0x1f, L_O },
/* 1b*/ {{']',   '}',   A_GS,  K_NOP, K_ESN, K_ESN, K_NOP, K_NOP },0x1f, L_O },
/* 1c*/ {{A_CR,  A_CR,  A_CR,  A_CR,  A_CR,  A_CR,  A_CR,  A_CR  },0x00, L_O },
/* 1d*/ {{K_LCT, K_LCT, K_LCT, K_LCT, K_LCT, K_LCT, K_LCT, K_LCT },0xff, L_O },
/* 1e*/ {{'a',   'A',   A_SOH, A_SOH, K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_C },
/* 1f*/ {{'s',   'S',   A_DC3, A_DC3, K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_C },
/* 20*/ {{'d',   'D',   A_EOT, A_EOT, K_ESN, K_ESN, K_DBG, K_NOP },0x0f, L_C },
/* 21*/ {{'f',   'F',   A_ACK, A_ACK, K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_C },
/* 22*/ {{'g',   'G',   A_BEL, A_BEL, K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_C },
/* 23*/ {{'h',   'H',   A_BS,  A_BS,  K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_C },
/* 24*/ {{'j',   'J',   A_LF,  A_LF,  K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_C },
/* 25*/ {{'k',   'K',   A_VT,  A_VT,  K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_C },
/* 26*/ {{'l',   'L',   A_FF,  A_FF,  K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_C },
/* 27*/ {{';',   ':',   ';',   ':',   K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_O },
/* 28*/ {{'\'',  '"',   '\'',  '"',   K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_O },
/* 29*/ {{'`',   '~',   '`',   '~',   K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_O },
/* 2a*/ {{K_LSH, K_LSH, K_LSH, K_LSH, K_LSH, K_LSH, K_LSH, K_LSH },0xff, L_O },
/* 2b*/ {{'\\',  '|',   A_FS,  '|',   K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_O },
/* 2c*/ {{'z',   'Z',   A_SUB, A_SUB, K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_C },
/* 2d*/ {{'x',   'X',   A_CAN, A_CAN, K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_C },
/* 2e*/ {{'c',   'C',   A_ETX, A_ETX, K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_C },
/* 2f*/ {{'v',   'V',   A_SYN, A_SYN, K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_C },
/* 30*/ {{'b',   'B',   A_STX, A_STX, K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_C },
/* 31*/ {{'n',   'N',   A_SO,  A_SO,  K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_C },
/* 32*/ {{'m',   'M',   A_CR,  A_CR,  K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_C },
/* 33*/ {{',',   '<',   ',',   '<',   K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_O },
/* 34*/ {{'.',   '>',   '.',   '>',   K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_O },
/* 35*/ {{'/',   '?',   '/',   A_US,  K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_O },
/* 36*/ {{K_RSH, K_RSH, K_RSH, K_RSH, K_RSH, K_RSH, K_RSH, K_RSH },0xff, L_O },
/* 37*/ {{'*',   '*',   '*',   '*'  , K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_O },
/* 38*/ {{K_LAL, K_LAL, K_LAL, K_LAL, K_LAL, K_LAL, K_LAL, K_LAL },0xff, L_O },
/* 39*/ {{' ',   ' ',   A_NUL, A_NUL, K_ESN, K_ESN, K_NOP, K_NOP },0x0f, L_O },
/* 3a*/ {{K_CLK, K_CLK, K_CLK, K_CLK, K_CLK, K_CLK, K_CLK, K_CLK },0xff, L_O },
/* 3b*/ {{KF+0,  KF+12, KF+24, KF+36, KF+0, KF+12, KF+24, KF+36 },0xff, L_O },
/* 3c*/ {{KF+1,  KF+13, KF+25, KF+37, KF+1, KF+13, KF+25, KF+37 },0xff, L_O },
/* 3d*/ {{KF+2,  KF+14, KF+26, KF+38, KF+2, KF+14, KF+26, KF+38 },0xff, L_O },
/* 3e*/ {{KF+3,  KF+15, KF+27, KF+39, KF+3, KF+15, KF+27, KF+39 },0xff, L_O },
/* 3f*/ {{KF+4,  KF+16, KF+28, KF+40, KF+4, KF+16, KF+28, KF+40 },0xff, L_O },
/* 40*/ {{KF+5,  KF+17, KF+29, KF+41, KF+5, KF+17, KF+29, KF+41 },0xff, L_O },
/* 41*/ {{KF+6,  KF+18, KF+30, KF+42, KF+6, KF+18, KF+30, KF+42 },0xff, L_O },
/* 42*/ {{KF+7,  KF+19, KF+31, KF+43, KF+7, KF+19, KF+31, KF+43 },0xff, L_O },
/* 43*/ {{KF+8,  KF+20, KF+32, KF+44, KF+8, KF+20, KF+32, KF+44 },0xff, L_O },
/* 44*/ {{KF+9,  KF+21, KF+33, KF+45, KF+9, KF+21, KF+33, KF+45 },0xff, L_O },
/* 45*/ {{K_NLK, K_NLK, K_NLK, K_NLK, K_NLK, K_NLK, K_NLK, K_NLK },0xff, L_O },
/* 46*/ {{K_SLK, K_SLK, K_SLK, K_SLK, K_SLK, K_SLK, K_SLK, K_SLK },0xff, L_O },
/* 47*/ {{KF+48, '7',   KF+48, '7'  , KF+48, K_ESN, K_NOP, K_NOP },0xaf, L_N },
/* 48*/ {{KF+49, '8',   KF+49, '8'  , KF+49, K_ESN, K_NOP, K_NOP },0xaf, L_N },
/* 49*/ {{KF+50, '9',   KF+50, '9'  , KF+50, K_ESN, K_NOP, K_NOP },0xaf, L_N },
/* 4a*/ {{KF+51, '-',   KF+51, '-'  , KF+51, K_ESN, K_NOP, K_NOP },0xaf, L_N },
/* 4b*/ {{KF+52, '4',   KF+52, '4'  , KF+52, K_ESN, K_NOP, K_NOP },0xaf, L_N },
/* 4c*/ {{KF+53, '5',   KF+53, '5'  , KF+53, K_ESN, K_NOP, K_NOP },0xaf, L_N },
/* 4d*/ {{KF+54, '6',   KF+54, '6'  , KF+54, K_ESN, K_NOP, K_NOP },0xaf, L_N },
/* 4e*/ {{KF+55, '+',   KF+55, '+'  , KF+55, K_ESN, K_NOP, K_NOP },0xaf, L_N },
/* 4f*/ {{KF+56, '1',   KF+56, '1'  , KF+56, K_ESN, K_NOP, K_NOP },0xaf, L_N },
/* 50*/ {{KF+57, '2',   KF+57, '2'  , KF+57, K_ESN, K_NOP, K_NOP },0xaf, L_N },
/* 51*/ {{KF+58, '3',   KF+58, '3'  , KF+58, K_ESN, K_NOP, K_NOP },0xaf, L_N },
/* 52*/ {{KF+59, '0',   KF+59, '0'  , KF+59, K_ESN, K_NOP, K_NOP },0xaf, L_N },
/* 53*/ {{A_DEL, '.',   A_DEL, '.'  , A_DEL, K_ESN, K_RBT, K_NOP },0x07, L_N },
/* 54*/ {{KF+59, KF+25, KF+59, K_NOP, K_SRQ, K_SRQ, K_SRQ, K_SRQ },0xff, L_O },
/* 55*/ {{KF+57, KF+57, KF+57, KF+57, KF+57, KF+57, KF+57, KF+57 },0xff, L_O },
/* 56*/ {{KF+52, KF+52, KF+52, KF+52, KF+52, KF+52, KF+52, KF+52 },0xff, L_O },
/* 57*/ {{KF+10, KF+22, KF+34, KF+46, KF+10, KF+22, KF+34, KF+46 },0xff, L_O },
/* 58*/ {{KF+11, KF+23, KF+35, KF+47, KF+11, KF+23, KF+35, KF+47 },0xff, L_O },
/* 59*/ {{K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP },0xff, L_O },
/* 5a*/ {{K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP },0xff, L_O },
/* 5b*/ {{K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP },0xff, L_O },
/* 5c*/ {{K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP },0xff, L_O },
/* 5d*/ {{K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP },0xff, L_O },
/* 5e*/ {{K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP },0xff, L_O },
/* 5f*/ {{K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP },0xff, L_O },
/* 60*/ {{K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP },0xff, L_O },
/* 61*/ {{K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP },0xff, L_O },
/* 62*/ {{K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP },0xff, L_O },
/* 63*/ {{K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP },0xff, L_O },
/* 64*/ {{K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP },0xff, L_O },
/* 65*/ {{K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP },0xff, L_O },
/* 66*/ {{K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP },0xff, L_O },
/* 67*/ {{K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP },0xff, L_O },
/* 68*/ {{K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP },0xff, L_O },
/* 69*/ {{K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP },0xff, L_O },
/* 6a*/ {{K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP },0xff, L_O },
/* 6b*/ {{KF+52, KF+52, KF+52, KF+52, KF+52, KF+52, KF+52, KF+52 },0xff, L_O },
/* 6c*/ {{K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP },0xff, L_O },
/* 6d*/ {{K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP },0xff, L_O },
/* 6e*/ {{K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP },0xff, L_O },
/* 6f*/ {{KF+50, KF+50, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP },0xff, L_O },
/* 70*/ {{K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP },0xff, L_O },
/* 71*/ {{K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP },0xff, L_O },
/* 72*/ {{K_RAL, K_RAL, K_RAL, K_RAL, K_RAL, K_RAL, K_RAL, K_RAL },0xff, L_O },
/* 73*/ {{K_RCT, K_RCT, K_RCT, K_RCT, K_RCT, K_RCT, K_RCT, K_RCT },0xff, L_O },
/* 74*/ {{A_NL,  A_NL,  A_NL,  A_NL,  A_NL,  A_NL,  A_NL,  A_NL  },0x00, L_O },
/* 75*/ {{'/',   '/',   K_NOP, K_NOP, K_ESN, K_ESN, K_NOP, K_NOP },0x3f, L_O },
/* 76*/ {{K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP },0xff, L_O },
/* 77*/ {{K_SLK, K_SLK, K_BRK, K_BRK, K_SLK, K_SLK, K_NOP, K_NOP },0xff, L_O },
/* 78*/ {{KF+49, KF+49, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP },0xff, L_O },
/* 79*/ {{A_DEL, A_DEL, A_DEL, A_DEL, A_DEL, A_DEL, A_DEL, A_DEL },0x00, L_O },
/* 7a*/ {{KF+56, KF+56, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP },0xff, L_O },
/* 7b*/ {{KF+59, KF+59, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP },0xff, L_O },
/* 7c*/ {{K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP },0xff, L_O },
/* 7d*/ {{KF+54, KF+54, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP },0xff, L_O },
/* 7e*/ {{KF+58, KF+58, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP },0xff, L_O },
/* 7f*/ {{KF+48, KF+48, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP, K_NOP },0xff, L_O }
}};

/*
 * Extended code support
 */

extkeys_t ext_keys = {
/* SCAN                               	     	*/
/* CODE       BASE    SHIFT   CTRL    ALT 	*/
/*   0 */  {  K_NOP,  K_NOP,  K_NOP,  K_NOP	},
/*   1 */  {  K_NOP,  K_NOP,  K_NOP,  K_NOP	},
/*   2 */  {  K_NOP,  K_NOP,  K_NOP,  0x78	},
/*   3 */  {  K_NOP,  K_NOP,  K_NOP,  0x79	},
/*   4 */  {  K_NOP,  K_NOP,  K_NOP,  0x7a	},
/*   5 */  {  K_NOP,  K_NOP,  K_NOP,  0x7b	},
/*   6 */  {  K_NOP,  K_NOP,  K_NOP,  0x7c	},
/*   7 */  {  K_NOP,  K_NOP,  K_NOP,  0x7d	},
/*   8 */  {  K_NOP,  K_NOP,  K_NOP,  0x7e	},
/*   9 */  {  K_NOP,  K_NOP,  K_NOP,  0x7f	},
/*  10 */  {  K_NOP,  K_NOP,  K_NOP,  0x80	},
/*  11 */  {  K_NOP,  K_NOP,  K_NOP,  0x81	},
/*  12 */  {  K_NOP,  K_NOP,  K_NOP,  0x82	},
/*  13 */  {  K_NOP,  K_NOP,  K_NOP,  0x83	},
/*  14 */  {  K_NOP,  K_NOP,  K_NOP,  K_NOP 	},
/*  15 */  {  K_NOP,  0xf,    K_NOP,  K_NOP  	},
/*  16 */  {  K_NOP,  K_NOP,  K_NOP,  0x10	},
/*  17 */  {  K_NOP,  K_NOP,  K_NOP,  0x11	},
/*  18 */  {  K_NOP,  K_NOP,  K_NOP,  0x12	},
/*  19 */  {  K_NOP,  K_NOP,  K_NOP,  0x13	},
/*  20 */  {  K_NOP,  K_NOP,  K_NOP,  0x14	},
/*  21 */  {  K_NOP,  K_NOP,  K_NOP,  0x15	},
/*  22 */  {  K_NOP,  K_NOP,  K_NOP,  0x16	},
/*  23 */  {  K_NOP,  K_NOP,  K_NOP,  0x17	},
/*  24 */  {  K_NOP,  K_NOP,  K_NOP,  0x18	},
/*  25 */  {  K_NOP,  K_NOP,  K_NOP,  0x19	},
/*  26 */  {  K_NOP,  K_NOP,  K_NOP,  K_NOP 	},
/*  27 */  {  K_NOP,  K_NOP,  K_NOP,  K_NOP 	},
/*  28 */  {  K_NOP,  K_NOP,  K_NOP,  K_NOP  	},
/*  29 */  {  K_NOP,  K_NOP,  K_NOP,  K_NOP	},
/*  30 */  {  K_NOP,  K_NOP,  K_NOP,  0x1e      },
/*  31 */  {  K_NOP,  K_NOP,  K_NOP,  0x1f	},
/*  32 */  {  K_NOP,  K_NOP,  K_NOP,  0x20	},
/*  33 */  {  K_NOP,  K_NOP,  K_NOP,  0x21	},
/*  34 */  {  K_NOP,  K_NOP,  K_NOP,  0x22	},
/*  35 */  {  K_NOP,  K_NOP,  K_NOP,  0x23	},
/*  36 */  {  K_NOP,  K_NOP,  K_NOP,  0x24	},
/*  37 */  {  K_NOP,  K_NOP,  K_NOP,  0x25	},
/*  38 */  {  K_NOP,  K_NOP,  K_NOP,  0x26	},
/*  39 */  {  K_NOP,  K_NOP,  K_NOP,  K_NOP	},
/*  40 */  {  K_NOP,  K_NOP,  K_NOP,  K_NOP 	},
/*  41 */  {  K_NOP,  K_NOP,  K_NOP,  K_NOP	},
/*  42 */  {  K_NOP,  K_NOP,  K_NOP,  K_NOP	},
/*  43 */  {  K_NOP,  K_NOP,  K_NOP,  K_NOP 	},
/*  44 */  {  K_NOP,  K_NOP,  K_NOP,  0x2c	},
/*  45 */  {  K_NOP,  K_NOP,  K_NOP,  0x2d	},
/*  46 */  {  K_NOP,  K_NOP,  K_NOP,  0x2e	},
/*  47 */  {  K_NOP,  K_NOP,  K_NOP,  0x2f	},
/*  48 */  {  K_NOP,  K_NOP,  K_NOP,  0x30	},
/*  49 */  {  K_NOP,  K_NOP,  K_NOP,  0x31	},
/*  50 */  {  K_NOP,  K_NOP,  K_NOP,  0x32	},
/*  51 */  {  K_NOP,  K_NOP,  K_NOP,  K_NOP	},
/*  52 */  {  K_NOP,  K_NOP,  K_NOP,  K_NOP	},
/*  53 */  {  K_NOP,  K_NOP,  K_NOP,  K_NOP	},
/*  54 */  {  K_NOP,  K_NOP,  K_NOP,  K_NOP	},
/*  55 */  {  K_NOP,  K_NOP,  0x72,   K_NOP 	},
/*  56 */  {  K_NOP,  K_NOP,  K_NOP,  K_NOP	},
/*  57 */  {  K_NOP,  K_NOP,  K_NOP,  K_NOP	},
/*  58 */  {  K_NOP,  K_NOP,  K_NOP,  K_NOP	},
/*  59 */  {  0x3b,   0x54,   0x5e,   0x68	}, /* f1, f13, f25, f37 */
/*  60 */  {  0x3c,   0x55,   0x5f,   0x69	}, /* f2, f14, f26, f38 */
/*  61 */  {  0x3d,   0x56,   0x60,   0x6a	}, /* f3, f15, f27, f39 */
/*  62 */  {  0x3e,   0x57,   0x61,   0x6b	}, /* f4, f16, f28, f40 */
/*  63 */  {  0x3f,   0x58,   0x62,   0x6c	}, /* f5, f17, f29, f41 */
/*  64 */  {  0x40,   0x59,   0x63,   0x6d	}, /* f6, f18, f30, f42 */
/*  65 */  {  0x41,   0x5a,   0x64,   0x6e	}, /* f7, f19, f31, f43 */
/*  66 */  {  0x42,   0x5b,   0x65,   0x6f	}, /* f8, f20, f32, f44 */
/*  67 */  {  0x43,   0x5c,   0x66,   0x70	}, /* f9, f21, f33, f45 */
/*  68 */  {  0x44,   0x5d,   0x67,   0x71	}, /* f10,f22, f34, f46 */
/*  69 */  {  K_NOP,  K_NOP,  K_NOP,  K_NOP	},
/*  70 */  {  K_NOP,  K_NOP,  0x00,   K_NOP	},
/*  71 */  {  0x47,   K_NOP,  0x77,   0x7	},
/*  72 */  {  0x48,   K_NOP,  K_NOP,  0x8	},
/*  73 */  {  0x49,   K_NOP,  0x84,   0x9	},
/*  74 */  {  K_NOP,  K_NOP,  K_NOP,  K_NOP	},
/*  75 */  {  0x4b,   K_NOP,  0x73,   0x4	},
/*  76 */  {  K_NOP,  K_NOP,  K_NOP,  0x5	},
/*  77 */  {  0x4d,   K_NOP,  0x74,   0x6	},
/*  78 */  {  K_NOP,  K_NOP,  K_NOP,  K_NOP	},
/*  79 */  {  0x4f,   K_NOP,  0x75,   0x1	},
/*  80 */  {  0x50,   K_NOP,  K_NOP,  0x2	},
/*  81 */  {  0x51,   K_NOP,  0x76,   0x3	},
/*  82 */  {  0x52,   K_NOP,  K_NOP,  0xb0	},
/*  83 */  {  0x53,   K_NOP,  K_NOP,  K_NOP	},
/*  84 */  {  K_NOP,  K_NOP,  K_NOP,  K_NOP	},
/*  85 */  {  0x85,   0x87,   0x89,   0x8b	}, /* f11, f23, f35, f47 */
/*  86 */  {  0x86,   0x88,   0x8a,   0x8c	}  /* f12, f24, f36, f48 */
};

/*
 * Table to translate 0xe0 prefixed scan codes to proper table indices
 */

esctbl_t kdesctbl = {
	0x1c, 0x74,	/* enter key */
	0x1d, 0x73,	/* right control key */
	0x2a, 0x00,	/* map to no key stroke */
	0x35, 0x75,	/* keypad '/' key */
	0x36, 0x00,	/* map to no key stroke */
	0x37, 0x54,	/* print screen key */
	0x38, 0x72,	/* right alt key */
	0x46, 0x77,	/* pause/break key */
	0x47, 0x7f,	/* home key */
	0x48, 0x78,	/* up arrow key */
	0x49, 0x6f,	/* page up key */
	0x4b, 0x6b,	/* left arrow key */
	0x4d, 0x7d,	/* right arrow key */
	0x4f, 0x7a,	/* end key */
	0x50, 0x55,	/* down arrow key */
	0x51, 0x7e,	/* page down key */
	0x52, 0x7b,	/* insert key */
	0x53, 0x79,	/* delete key */
};

/*
 * Function key mapping table.  The function key values can be any length, as
 * long as the total length of all the strings is <= STRTABLN, including NULLs.
 */

strmap_t kdstrbuf = {
/* Base function keys 1-12 */
/*  0 */	'\033', 'O', 'P', '\0',
/*  1 */	'\033', 'O', 'Q', '\0',
/*  2 */	'\033', 'O', 'R', '\0',
/*  3 */	'\033', 'O', 'S', '\0',
/*  4 */	'\033', 'O', 'T', '\0',
/*  5 */	'\033', 'O', 'U', '\0',
/*  6 */	'\033', 'O', 'V', '\0',
/*  7 */	'\033', 'O', 'W', '\0',
/*  8 */	'\033', 'O', 'X', '\0',
/*  9 */	'\033', 'O', 'Y', '\0',
/* 10 */	'\033', 'O', 'Z', '\0',
/* 11 */	'\033', 'O', 'A', '\0',
/* Shift function keys 1-12 */
/* 12 */	'\033', 'O', 'p', '\0',
/* 13 */	'\033', 'O', 'q', '\0',
/* 14 */	'\033', 'O', 'r', '\0',
/* 15 */	'\033', 'O', 's', '\0',
/* 16 */	'\033', 'O', 't', '\0',
/* 17 */	'\033', 'O', 'u', '\0',
/* 18 */	'\033', 'O', 'v', '\0',
/* 19 */	'\033', 'O', 'w', '\0',
/* 20 */	'\033', 'O', 'x', '\0',
/* 21 */	'\033', 'O', 'y', '\0',
/* 22 */	'\033', 'O', 'z', '\0',
/* 23 */	'\033', 'O', 'a', '\0',
/* Ctrl function keys 1-12 */
/* 24 */	'\033', 'O', 'P', '\0',
/* 25 */	'\033', 'O', 'Q', '\0',
/* 26 */	'\033', 'O', 'R', '\0',
/* 27 */	'\033', 'O', 'S', '\0',
/* 28 */	'\033', 'O', 'T', '\0',
/* 29 */	'\033', 'O', 'U', '\0',
/* 30 */	'\033', 'O', 'V', '\0',
/* 31 */	'\033', 'O', 'W', '\0',
/* 32 */	'\033', 'O', 'X', '\0',
/* 33 */	'\033', 'O', 'Y', '\0',
/* 34 */	'\033', 'O', 'Z', '\0',
/* 35 */	'\033', 'O', 'A', '\0',
/* Ctrl-shift function keys 1-12 */
/* The above comment was "Alt function keys 1-12" in M0 */
/* 36 */	'\033', 'O', 'p', '\0',
/* 37 */	'\033', 'O', 'q', '\0',
/* 38 */	'\033', 'O', 'r', '\0',
/* 39 */	'\033', 'O', 's', '\0',
/* 40 */	'\033', 'O', 't', '\0',
/* 41 */	'\033', 'O', 'u', '\0',
/* 42 */	'\033', 'O', 'v', '\0',
/* 43 */	'\033', 'O', 'w', '\0',
/* 44 */	'\033', 'O', 'x', '\0',
/* 45 */	'\033', 'O', 'y', '\0',
/* 46 */	'\033', 'O', 'z', '\0',
/* 47 */	'\033', 'O', 'a', '\0',
/* Keypad */
/* 48 */	'\033', '[', 'H', '\0',	/* 7 : Home */
/* 49 */	'\033', '[', 'A', '\0',	/* 8 : Up Arrow */
/* 50 */	'\033', '[', 'V', '\0',	/* 9 : Page Up */
/* 51 */	'-', '\0',		/* - : */
/* 52 */	'\033', '[', 'D', '\0',	/* 4 : Left Arrow */
/* 53 */	'\033', '[', 'G', '\0',	/* 5 : */
/* 54 */	'\033', '[', 'C', '\0',	/* 6 : Right Arrow */
/* 55 */	'+', '\0',		/* + : */
/* 56 */	'\033', '[', 'Y', '\0',	/* 1 : End */
/* 57 */	'\033', '[', 'B', '\0',	/* 2 : Down Arrow */
/* 58 */	'\033', '[', 'U', '\0',	/* 3 : Page Down */
/* 59 */	'\033', '[', '@', '\0',	/* 0 : Insert */
/* Extra string */
/* 60 */	'\033', '[', '2', '\0',	/* Insert on 101/102 key keyboard */
};

srqtab_t srqtab = {
/*  0*/ K_NOP, 
/*  1*/ K_NOP, 
/*  2*/ K_NOP,
/*  3*/ K_NOP, 
/*  4*/ K_NOP, 
/*  5*/ K_NOP,
/*  6*/ K_NOP,
/*  7*/ K_NOP,
/*  8*/ K_NOP,
/*  9*/ K_NOP,
/*  a*/ K_NOP,
/*  b*/ K_NOP,
/*  c*/ K_NOP,
/*  d*/ K_NOP,
/*  e*/ K_NOP,
/*  f*/ K_NOP,
/* 10*/ K_NOP,
/* 11*/ K_NOP,
/* 12*/ K_NOP,
/* 13*/ K_NOP,
/* 14*/ K_NOP,
/* 15*/ K_NOP,
/* 16*/ K_NOP,
/* 17*/ K_NOP,
/* 18*/ K_NOP,
/* 19*/ K_PREV,
/* 1a*/ K_NOP,
/* 1b*/ K_NOP,
/* 1c*/ K_NOP,
/* 1d*/ K_NOP,
/* 1e*/ K_NOP,
/* 1f*/ K_NOP,
/* 20*/ K_NOP,
/* 21*/ K_FRCNEXT,
/* 22*/ K_NOP,
/* 23*/ K_VTF,
/* 24*/ K_NOP,
/* 25*/ K_NOP,
/* 26*/ K_NOP,
/* 27*/ K_NOP,
/* 28*/ K_NOP,
/* 29*/ K_NOP,
/* 2a*/ K_NOP,
/* 2b*/ K_NOP,
/* 2c*/ K_NOP,
/* 2d*/ K_NOP,
/* 2e*/ K_NOP,
/* 2f*/ K_NOP,
/* 30*/ K_NOP,
/* 31*/ K_NEXT,
/* 32*/ K_NOP,
/* 33*/ K_NOP,
/* 34*/ K_NOP,
/* 35*/ K_NOP,
/* 36*/ K_NOP,
/* 37*/ K_NOP,
/* 38*/ K_NOP,
/* 39*/ K_NOP,
/* 3a*/ K_NOP,
/* 3b*/ K_VTF + 1,
/* 3c*/ K_VTF + 2,
/* 3d*/ K_VTF + 3,
/* 3e*/ K_VTF + 4,
/* 3f*/ K_VTF + 5,
/* 40*/ K_VTF + 6,
/* 41*/ K_VTF + 7,
/* 42*/ K_VTF + 8,
/* 43*/ K_VTF + 9,
/* 44*/ K_VTF + 10,
/* 45*/ K_NOP,
/* 46*/ K_NOP,
/* 47*/ K_NOP,
/* 48*/ K_NOP,
/* 49*/ K_NOP,
/* 4a*/ K_NOP,
/* 4b*/ K_NOP,
/* 4c*/ K_NOP,
/* 4d*/ K_NOP,
/* 4e*/ K_NOP,
/* 4f*/ K_NOP,
/* 50*/ K_NOP,
/* 51*/ K_NOP,
/* 52*/ K_NOP,
/* 53*/ K_NOP,
/* 54*/ K_NOP,
/* 55*/ K_NOP,
/* 56*/ K_NOP,
/* 57*/ K_VTF + 11,
/* 58*/ K_VTF + 12,
/* 59*/ K_NOP,
/* 5a*/ K_NOP,
/* 5b*/ K_NOP,
/* 5c*/ K_NOP,
/* 5d*/ K_NOP,
/* 5e*/ K_NOP,
/* 5f*/ K_NOP,
/* 60*/ K_NOP,
/* 61*/ K_NOP,
/* 62*/ K_NOP,
/* 63*/ K_NOP,
/* 64*/ K_NOP,
/* 65*/ K_NOP,
/* 66*/ K_NOP,
/* 67*/ K_NOP,
/* 68*/ K_NOP,
/* 69*/ K_NOP,
/* 6a*/ K_NOP,
/* 6b*/ K_NOP,
/* 6c*/ K_NOP,
/* 6d*/ K_NOP,
/* 6e*/ K_NOP,
/* 6f*/ K_NOP,
/* 70*/ K_NOP,
/* 71*/ K_NOP,
/* 72*/ K_NOP,
/* 73*/ K_NOP,
/* 74*/ K_NOP,
/* 75*/ K_NOP,
/* 76*/ K_NOP,
/* 77*/ K_NOP,
/* 78*/ K_NOP,
/* 79*/ K_NOP,
/* 7a*/ K_NOP,
/* 7b*/ K_NOP,
/* 7c*/ K_NOP,
/* 7d*/ K_NOP,
/* 7e*/ K_NOP,
/* 7f*/ K_NOP 
};

ushort kb_shifttab[] = {	/* Translate shifts for kb_state */
	0, 0, LEFT_SHIFT, RIGHT_SHIFT, CAPS_LOCK, NUM_LOCK, SCROLL_LOCK,
	ALTSET, 0, CTRLSET, LEFT_ALT, RIGHT_ALT, LEFT_CTRL, RIGHT_CTRL
};

struct kb_shiftmkbrk kb_mkbrk[] = {
	{ LEFT_SHIFT,	0x2a,	0xaa,	0 },
	{ LEFT_ALT,	0x38,	0xb8,	0 },
	{ LEFT_CTRL,	0x1d,	0x9d,	0 },
	{ RIGHT_SHIFT,	0x36,	0xb6,	0 },
	{ RIGHT_ALT,	0x38,	0xb8,	1 },
	{ RIGHT_CTRL,	0x1d,	0x9d,	1 },
	{ CAPS_LOCK,	0x3a,	0xba,	0 },
	{ NUM_LOCK,	0x45,	0xc5,	0 },
	{ SCROLL_LOCK,	0x46,	0xc6,	0 }, 
} ;
