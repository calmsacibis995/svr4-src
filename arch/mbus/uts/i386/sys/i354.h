/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1986, 1987, 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/sys/i354.h	1.3.2.1"

#ifndef _SYS_I354_H
#define _SYS_I354_H

/* bits in the minor number */
#define	RAWMSK		0x80		/* bit 7, open as raw device */

/* flags in m_state */
#define INCLOSE		0x10		/* driver is currently in the
								 * close routine */
#define WANTLINE	0x20		/* driver has been called to
								 * open a line but the INCLOSE bit
								 * is set */
#define CARRF		0x40		/* actual carrier state */

#define	BRK_IN_PROGRESS	0x1		/* Break is in progress */

struct i354cfg {
	unsigned m_ctrl;		/* control port for this channel */
	unsigned m_data;		/* data port for this channel */
	uint m_speed;			/* speed line set at */
	uint m_reg3;			/* command to reg 3 to set proper state */
	uint m_reg5;			/* command to reg 5 to set proper state */
	char m_state;			/* state information for line */
	unchar m_mask;			/* mask for bits/char codes */
};

#define CH_A		0		/* index into i354cfg table for chan A */
#define CH_B		1		/* index into i354cfg table for chan B */

#endif	/* _SYS_I354_H */
