/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/cci.d/dl.d/ext.h	1.3"

extern void do_pedata();
extern void do_regint();
extern void do_pidata();
extern void do_modend();
extern int my_read();
extern void do_board_reset();

extern void do_download();
extern void expand();
extern void do_execute();
extern void print_buf();

extern void fill_buf();
extern void print_buf();
extern unsigned long short_to_long();

extern unsigned char get_slot_id();
extern unsigned char get_ic();
extern void catch_interrupt();
extern void put_ic();
extern short find_rec();
extern unsigned short get_portid();
extern void transport_init();
extern void do_send();
extern FILE *open_bit_bucket();
extern void string_move();
extern void exit();
extern void on_intr();
