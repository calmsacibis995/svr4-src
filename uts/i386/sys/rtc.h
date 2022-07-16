/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_RTC_H
#define _SYS_RTC_H

#ident	"@(#)head.sys:sys/rtc.h	1.1.2.1"

/*
 * Definitions for Real Time Clock driver (Motorola MC146818 chip).
 */

#define	RTC_ADDR	0x70	/* I/O port address of for register select */
#define	RTC_DATA	0x71	/* I/O port address for data read/write */

/*
 * Register A definitions
 */
#define	RTC_A		0x0a	/* register A address */
#define	RTC_UIP		0x80	/* Update in progress bit */
#define	RTC_DIV0	0x00	/* Time base of 4.194304 MHz */
#define	RTC_DIV1	0x10	/* Time base of 1.048576 MHz */
#define	RTC_DIV2	0x20	/* Time base of 32.768 KHz */
#define	RTC_RATE6	0x06	/* interrupt rate of 976.562 */

/*
 * Register B definitions
 */
#define	RTC_B		0x0b	/* register B address */
#define	RTC_SET		0x80	/* stop updates for time set */
#define	RTC_PIE		0x40	/* Periodic interrupt enable */
#define	RTC_AIE		0x20	/* Alarm interrupt enable */
#define	RTC_UIE		0x10	/* Update ended interrupt enable */
#define	RTC_SQWE	0x08	/* Square wave enable */
#define	RTC_DM		0x04	/* Date mode, 1 = binary, 0 = BCD */
#define	RTC_HM		0x02	/* hour mode, 1 = 24 hour, 0 = 12 hour */
#define	RTC_DSE		0x01	/* Daylight savings enable */

/* 
 * Register C definitions
 */
#define	RTC_C		0x0c	/* register C address */
#define	RTC_IRQF	0x80	/* IRQ flag */
#define	RTC_PF		0x40	/* PF flag bit */
#define	RTC_AF		0x20	/* AF flag bit */
#define	RTC_UF		0x10	/* UF flag bit */

/*
 * Register D definitions
 */
#define	RTC_D		0x0d	/* register D address */
#define	RTC_VRT		0x80	/* Valid RAM and time bit */

#define	RTC_NREG	0x0e	/* number of RTC registers */
#define	RTC_NREGP	0x0a	/* number of RTC registers to set time */

/*
 * Ioctl definitions for accessing RTC.
 */
#define	RTCIOC	('R' << 8)

#define	RTCRTIME	(RTCIOC | 0x01)		/* Read time from RTC */
#define	RTCSTIME	(RTCIOC | 0x02)		/* Set time into RTC */

struct	rtc_t {
	char	rtc_sec;
	char	rtc_asec;
	char	rtc_min;
	char	rtc_amin;
	char	rtc_hr;
	char	rtc_ahr;
	char	rtc_dow;
	char	rtc_dom;
	char	rtc_mon;
	char	rtc_yr;
	char	rtc_statusa;
	char	rtc_statusb;
	char	rtc_statusc;
	char	rtc_statusd;
};

#endif	/* _SYS_RTC_H */
