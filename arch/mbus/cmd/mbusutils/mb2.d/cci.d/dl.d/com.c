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

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/cci.d/dl.d/com.c	1.3"

#include "trans.h"
#include <stdio.h>
#include "410.h"
#include "lit.h"
#include "ext.h"
extern unsigned int buf_size;
extern unsigned int delay;
extern short firmware_reg;
extern unsigned char *progname;

/****************************************************************************/
/*                                                                          */
/*                            EXPAND      			            */
/*                            ------				            */
/*                                                                          */
/****************************************************************************/

void expand(data_buf,new_buf,recur_level,exp_len_p)
unsigned char *data_buf;
unsigned char *new_buf;
int recur_level;
unsigned short *exp_len_p;
	
{
	int sav_i;
	int movb_i;
	unsigned char byt_count;
	unsigned short rpt_count;
	unsigned short blk_count;

	recur_level++;
	if (recur_level > MAX_RECUR_LEVEL) {
		fprintf(stderr,"\n%s: Recursion Level Too High\n",progname);
		exit(1);
	}
	
	rpt_count = *((unsigned short *)(&data_buf[b_index]));
	if (rpt_count == 0) {
		fprintf(stderr,"\n%s: Bad PIDATA Record\n",progname);
		exit(1);
	}
	sav_i = b_index + 2;
	while (rpt_count > 0) {
		b_index = sav_i;
		blk_count = *((unsigned short *)(&data_buf[b_index]));
		b_index = b_index + 2;
		
		if (blk_count != 0) {
			while (blk_count > 0) {
				expand(data_buf,new_buf,recur_level,exp_len_p);
				blk_count--;
			}
		}
		else {
			byt_count = data_buf[b_index];
			movb_i = b_index+1;
			b_index = b_index + byt_count + 1;
			while (byt_count > 0) {
				new_buf[new_i] = data_buf[movb_i];
				new_i++;				
				if (new_i >= buf_size) {
					fprintf(stderr,"\n%s: PIDATA Expands to > %d bytes\n", progname, buf_size);
					exit(1);
				}
				
				movb_i++;				
				byt_count--;
				(*exp_len_p)++;
			}
		}
		rpt_count--;
		recur_level--;
	}
}

/****************************************************************************/
/*                                                                          */
/*                            DOWNLOAD         					            */
/*                            --------	    	    	                     */
/*                                                                          */
/****************************************************************************/

void do_download(addr_p,data_buf_p,len)
	
unsigned long *addr_p;
unsigned char *data_buf_p;
unsigned short len;
{
	unsigned short fw_com_reg;
	BYTE src_id;
	WORD dest_id;	
	WORD port;
	BYTE value;
	DWORD dw = *addr_p;
	BYTE b0,b1,b2,b3;
	BYTE buf[11];
	int retry_cnt;
	int delay_cnt;

	b0 = BY0(dw);
	b1 = BY1(dw);
	b2 = BY2(dw);
	b3 = BY3(dw);

	src_id = get_slot_id();	
	port = get_portid();
	dest_id = (unsigned short)slot_num;

	fw_com_reg = firmware_reg + IC_FW_OFFSET;

	if (get_ic(slot_num, fw_com_reg) != READY) {
		fprintf(stderr,"\n%s: Board Not Ready - Use -r Option to Restart\n",progname);
		exit(1);
	}
	
	put_ic(slot_num, (fw_com_reg+1), 0);
	put_ic(slot_num, (fw_com_reg+2), src_id);
	put_ic(slot_num, (fw_com_reg+3), BY0(port));
	put_ic(slot_num, (fw_com_reg+4), BY1(port));
	put_ic(slot_num, (fw_com_reg+5), b0);
	put_ic(slot_num, (fw_com_reg+6), b1);
	put_ic(slot_num, (fw_com_reg+7), b2);
	put_ic(slot_num, (fw_com_reg+8), b3);
	put_ic(slot_num, (fw_com_reg+9), BY0(len));
	put_ic(slot_num, (fw_com_reg+10), BY1(len));
	put_ic(slot_num, fw_com_reg, DOWNLOAD_COM);
	do_sleep(delay);
	retry_cnt=0;
	do {
		do_send(data_buf_p, len);
		do_sleep(delay);
		delay_cnt=200;
		do {
			value = get_ic(slot_num, fw_com_reg);
			if (value == ERROR) {
				fprintf(stderr,"\n%s: Download Protocol Error\n",progname);
				exit(1);
			}
			if (value == READY) 
				return;
			do_sleep(delay);		
		} while (delay_cnt--);
	} while (retry_cnt++ < 20);
	fprintf(stderr,"\n%s: Download Protocol Error\n",progname);
	exit(1);
	
}

/****************************************************************************/
/*                                                                          */
/*                            EXECUTE           					            */
/*                            -------					                        */
/*                                                                          */
/****************************************************************************/

void do_execute(addr)
unsigned long addr;
{

	unsigned short fw_com_reg;
	DWORD dw = addr;
	BYTE b0,b1,b2,b3;

	b0 = BY0(dw);
	b1 = BY1(dw);
	b2 = BY2(dw);
	b3 = BY3(dw);

	fw_com_reg = firmware_reg + IC_FW_OFFSET;
	
	if (get_ic(slot_num, fw_com_reg) != READY) {
		fprintf(stderr,"\n%s: Board Not Ready - Use -r Option to Restart\n",progname);
		exit(1);
	}

	put_ic(slot_num, (fw_com_reg+4), b3);
	put_ic(slot_num, (fw_com_reg+3), b2);
	put_ic(slot_num, (fw_com_reg+2), b1);
	put_ic(slot_num, (fw_com_reg+1), b0);
	put_ic(slot_num, fw_com_reg, EXECUTE_COM);

}
