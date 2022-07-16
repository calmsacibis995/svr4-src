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

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/cci.d/dl.d/util.c	1.3"

#include "trans.h"
#include <stdio.h>
#include "ext.h"
#include "410.h"
extern unsigned char slot_num;
extern FILE *ofile;

/****************************************************************************/
/*                                                                          */
/*                     PRINT BUFFER    		   			    */
/*                     ------------					    */
/*                                                                          */
/****************************************************************************/

void print_buf(buffer,len)		
unsigned char *buffer;
unsigned short len;
{
	unsigned short i;
	
	fprintf(ofile,"\tPrint: %d Bytes\n",len);

	for (i=0; i<len; i++) {
		if (i % 16 == 0)
			fprintf(ofile,"\n\t");

		fprintf(ofile," %02xH",*buffer);
		buffer++;
	}
	fprintf(ofile,"\n\n");
}

/****************************************************************************/
/*                                                                          */
/*                     			MY READ    					   		        */
/*                    		 	-------					                    */
/*                                                                          */
/****************************************************************************/

int my_read(buf_p,len,fp)
unsigned char *buf_p;
int len;
FILE *fp;
{
	int eof_flg;

	eof_flg = 0;

	if (fread(buf_p,1,len,fp) != len)
		return(!eof_flg);
	else 
		return(eof_flg);

}

/****************************************************************************/
/*                                                                          */
/*                     DO BOARD RESET    		   					        */
/*                     --------------					                    */
/*                                                                          */
/****************************************************************************/

void do_board_reset()
{
	unsigned char value;

	value = RESET;
	put_ic(slot_num, RESET_REG, value);
	do_sleep(RESET_SLEEP);

	value = 0;
	put_ic(slot_num, RESET_REG, value);
	do_sleep(RESET_SLEEP);

	fprintf(ofile,"Resetting Target at slot %02d\n",slot_num);
}
