/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lxprof:cmd/sum.c	1.8"
#include <stdio.h>

#include "symint.h"
#include <time.h>
#include <sys/stat.h>

#include "retcode.h"
#include "funcdata.h"
#include "glob.h"
#include "env.h"
#include "coredefs.h"

#include "sgs.h"

#ifdef __STDC__
#define DATESIZE        60
#endif


CAsumhdr(type,src,obj)
short type;
char *src;
char *obj;
{

    struct stat stat_ptr;

#ifdef __STDC__
    char   buf[DATESIZE];
#else
    extern char  *ctime( );
#endif

    if((src == NULL) || (obj == NULL))
	return(0);  /* no processor or no oject file name specified */
	
    if (stat(src, &stat_ptr) != 0)
	return(0);  /* cannot get status of file */

    fprintf(stdout,"Coverage Data Source: %s\n",src);
#ifdef __STDC__

    if ( strftime(buf,DATESIZE,"%a %b %d %H:%M:%S %Y", 
                  localtime((const time_t *) &(stat_ptr.st_mtime)))   ==  0 ) {
	(void) fprintf(stderr, "%slprof: insufficient space to store date\n", SGS);
	return(0);
       	}
    (void) printf("Date of Coverage Data Source: %s\n", buf );

#else

    (void) printf("Date of Coverage Data Source: %.24s\n", ctime(&(stat_ptr.st_mtime)));

#endif
    fprintf(stdout,"Object: %s\n\n",obj);

    if (type == SUM) {
	fprintf(stdout,"percent   lines   total\tfunction\n");
	fprintf(stdout,"covered  covered  lines\tname\n\n", "name");
    }

    return(1);   /* success */

}

static long total_lines = 0;
static long total_cov = 0;

CAsumrept(func)
struct caFUNC_DATA *func;
{

	struct caDATA_BLK *blk;
	float lst_percent, percent;
	long lines_in_func, count;	/* covered line counter */
	int i;

	count = 0;
	lines_in_func = 0;
	lst_percent = 0;
	percent = 0;
	i = 0;

	blk = func->data;
	while (blk != NULL) {
	    for(i=0;i < (unsigned int)blk->entry_cnt;i++)  {
		lines_in_func++;
		if(blk->stats->data[i].count > 0)
		    count++;
	    }
	    blk = blk->next_blk;
	}

	if(func->data != NULL)  {
	   if(count == 0)
	  	percent = 0;
	   else  {
		percent = (float)count/(float)lines_in_func;
		lst_percent = percent * 100;
	   }
	}
	else {
	   fprintf(stderr,"\nNo lines in function %s\n",func->func_name);
	   return(-1);   /* failure */
	}

	total_cov = total_cov + count;
	total_lines = total_lines + lines_in_func;


       fprintf(stdout,"%6.1f%8ld%8ld\t%s\n",
	    lst_percent, count, lines_in_func, func->func_name);
       return(1);  /* success */


}	/* end of main */

CAsum_tot()
{
    if (total_lines != 0) {
	fprintf(stdout, "\n%6.1f%8d%8d\tTOTAL\n",
	    100.0*(float)total_cov/(float)total_lines, total_cov, total_lines);
    }
}

