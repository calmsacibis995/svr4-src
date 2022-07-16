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

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/cci.d/dl.d/record.c	1.3"

#include "trans.h"
#include <stdio.h>
#include "ext.h"

extern FILE *ofile;
extern unsigned char *progname;
int b_index;
int new_i;

/****************************************************************************/
/*                                                                          */
/*                     DO PEDATA RECORD     		   		    */
/*                     ----------------				            */
/*                                                                          */
/****************************************************************************/

void do_pedata(data_buf,rec_len)
unsigned char *data_buf;	
unsigned short rec_len;
{
	unsigned long pe_addr;
	unsigned long temp_fn;

	temp_fn = *((unsigned short *)(&data_buf[3]));

	pe_addr = ((temp_fn << 4) | (data_buf[5]));

	do_download(&pe_addr,(&data_buf[6]),rec_len - 4);

}

/****************************************************************************/
/*                                                                          */
/*                     DO PIDATA RECORD     				   		        */
/*                     ----------------				                        */
/*                                                                          */
/****************************************************************************/

void do_pidata(data_buf,exp_buf)
unsigned char *data_buf;
unsigned char *exp_buf;
{
	unsigned long pi_addr;
	unsigned long temp_fn;
	unsigned short exp_len;


	temp_fn = *((unsigned short *)(&data_buf[3]));

	pi_addr = ((temp_fn << 4) | (data_buf[5]));
		
	b_index = PI_DATA_START;
	new_i = 0;
	exp_len = 0;
	expand(data_buf,exp_buf,0,&exp_len);
	do_download(&pi_addr,exp_buf,exp_len);
}

/****************************************************************************/
/*                                                                          */
/*                     DO MODEND RECORD    					   		        */
/*                     ----------------					                    */
/*                                                                          */
/****************************************************************************/

void do_modend(data_buf, addr_p, mod_flag_p, name_buf)
unsigned char *data_buf;
unsigned long *addr_p;
unsigned char *mod_flag_p;
unsigned char *name_buf;
{
	unsigned char conv_buf[MAX_CHARS];
	unsigned char me_mod_typ;
	unsigned short temp_fn;
	unsigned short temp_off;
	unsigned long me_addr;
	unsigned long pre_addr;
	
	string_move(name_buf, conv_buf);
	me_mod_typ = ((data_buf[3]) & 0xc0);

	switch (me_mod_typ) {

		case N_MAIN_N_START: {
			fprintf(ofile,"\nNon-Main Module  ");
			fprintf(ofile,"%s\n", conv_buf);
			*mod_flag_p = N_MAIN_N_START; 
			*addr_p = INVALID_ADDR;
			break;
		}

		case N_MAIN_START: {
			pre_addr = 0;
			temp_fn = *((unsigned short *)(&data_buf[4]));
			temp_off = *((unsigned short *)(&data_buf[6]));
			pre_addr = temp_fn;
			me_addr = ((pre_addr << 4) | temp_off);
			fprintf(ofile,"\n%s: Non-Main Module  %s Has Start Address\n", progname, conv_buf);
			*addr_p = me_addr;
			*mod_flag_p = N_MAIN_START; 
			break;
		}

		case INVALID: {
			fprintf(ofile,"\nInvalid Module Attribute  %s \n", conv_buf);
			*addr_p = INVALID_ADDR;
			break;
		}

		case MAIN_START: {
			pre_addr = 0;
			temp_fn = *((unsigned short *)(&data_buf[4]));
			temp_off = *((unsigned short *)(&data_buf[6]));
			pre_addr = temp_fn;
			me_addr = ((pre_addr << 4) | temp_off);
			fprintf(ofile,"\nMain Module %s\n", conv_buf);
			*addr_p = me_addr;
			*mod_flag_p = MAIN_START; 
			break;
		}
	}				
}
