/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lxprof:hdr/filedata.h	1.1"
/************ STRUCTURE FOR PASSING INFO BETWEEN COVFILE OPERATORS ************/

struct caFILEDATA
{	FILE		*cov_obj_ptr;	/* file stream associated with object
				     	file info portion of a COVFILE  */
	FILE		*cov_data_ptr;	/* file stream associated with data
				     	portion of a COVFILE             */
	unsigned char	obj_cnt;	/* no. of object files that have been
				     	entered into this COVFILE        */
	unsigned char	use_flag;	/* usage flag:
                                     	CREATE--> building new COVFILE
                                     	UPDATE--> read/write existing COVFILE
                                                                      */
 };


#define	CREATE	0
#define UPDATE  1
