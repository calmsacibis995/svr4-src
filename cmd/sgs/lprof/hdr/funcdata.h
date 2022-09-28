/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lxprof:hdr/funcdata.h	1.1"
#include <syms.h>
#ifndef  _CACOVFILE
#include "covfile.h"
#endif


#define NOTCOVERED	0
#define	COVERED		1
#define	ALLCOVERED	~0

#define LINEMAX		64	/* no. of line numbers in one block for a function */

struct caCOV_STAT  /* one of these per line in a func */
{
	unsigned short	line_num;		/* line number */
	unsigned	block : 1;		/* logical block flag */
	    /* for XPROF */
	unsigned	status : 1;		/* coverage status bit */
	    /* for LPROF */
	unsigned long	count;
};

struct caSTAT_BLK
{
	struct caCOV_STAT    data[LINEMAX];	/* coverage status */
};

struct caDATA_BLK  /* for every LINEMAX lines in a func, there will be one struct */
{
	unsigned short       entry_cnt;		/* no of entries used */
	struct caDATA_BLK    *next_blk;		/* next block of line numbers */
	struct caSTAT_BLK    *stats;		/* coverage status */
};

struct caFUNC_DATA /* one for each function */
{
	char 		*func_name;		/* function name */
	unsigned short 	line_num;		/* absolute line no of beginning
							of function */
	long		func_idx;		/* symtbl index of function name */
	unsigned short	total_ent;		/* total no. of entries used for
						     storing line no. info */
	struct caDATA_BLK  *data;		/* coverage status */
	struct caFUNC_DATA *next_func;
};


struct caSRC_FILE
{
	char 		   *file_name; 		/* name of source file */
	long		   sym_idx;		/* symtbl index of file name */
	long		   last_idx;		/* symtbl index of last entry
						   belonging to this file */
	struct caFUNC_DATA *func_list;		/* list of fcns belonging to file */
	struct caSRC_FILE  *next_file;
};
