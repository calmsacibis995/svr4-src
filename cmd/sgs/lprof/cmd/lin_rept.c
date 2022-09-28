/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lxprof:cmd/lin_rept.c	1.3"
#include  <stdio.h>

#include "symint.h"
#include "funcdata.h"
#include "glob.h"
#include "env.h"

CAlinerept(func,cmd_pkt)
struct caFUNC_DATA *func;
struct command *cmd_pkt;
{
	char *strcat();

	char line[80];
	char line_out[80];

	int i;
	int start;	/*beginning of a group of contigously covered lines*/
	int end;	/* end of a group of contigously covered lines */
	int num_count;  /*number of numbers listed on one line or the report*/
	int prt_lines;
	struct caDATA_BLK *blk_buf;

	line_out[0] = '\0';
	i = 0;
	num_count = 0;
	start = 0;
	prt_lines = 0;
	blk_buf = func->data;


	printf("%s():\n",func->func_name);

	while ((blk_buf != NULL) && (i < (unsigned int)blk_buf->entry_cnt))  {

	    while((blk_buf != NULL) && (blk_buf->stats->data[i].status != cmd_pkt->cov_reqst)
	      && (i < (unsigned int)blk_buf->entry_cnt)) {
		/* cmd_pkt->cov_reqst  --->  1 =  covered    */
		/*			     0 = not covered */
		i++;
		if (i >= LINEMAX) {
		    blk_buf = blk_buf->next_blk;
		    i = 0;
		}
	    }

	    if((i >= (unsigned int)blk_buf->entry_cnt ) && (num_count == 0) && (prt_lines == 0))  {
		fprintf(stdout,"\t***NONE***\n");
		/*return(-1);*/  /* no coverage found as requested */
	    }


	    if( i != blk_buf->entry_cnt)  {
		start = blk_buf->stats->data[i].line_num;
		end = start;
		i++;
		if (i >= LINEMAX) {
		    blk_buf = blk_buf->next_blk;
		    i = 0;
		}

		while((blk_buf != NULL) && (blk_buf->stats->data[i].status == cmd_pkt->cov_reqst)
		  && (i < (unsigned int)blk_buf->entry_cnt)) {

		    end = blk_buf->stats->data[i].line_num;
		    i++;
		    if (i >= LINEMAX) {
			blk_buf = blk_buf->next_blk;
			i = 0;
		    }
		}

		if(start != end)  {
	
		    sprintf(line,"%d-%d ",start,end);
		    strcat(line_out,line);
	
		}

		else  {
	
		    sprintf(line,"%d ",start);
		    strcat(line_out,line);

		} 

		num_count ++;
	    }
	    if((num_count >= 5) || ((blk_buf != NULL) && (i == blk_buf->entry_cnt) ))
	    {
		strcat(line_out,"\n");
		fprintf(stdout,"\t%s\n",line_out);
		num_count = 0;
		line_out[0] = '\0'; 
		prt_lines++;
	    }
	}

}	/* end of main */
