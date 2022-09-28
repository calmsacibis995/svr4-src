/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*LINTLIBRARY*/

/* from file anyrequests.c */
# include	<sys/types.h>
# include	"requests.h"

/**
 ** anyrequests() - SEE IF ANY REQUESTS ARE ``QUEUED''
 **/
int anyrequests ()
{
    static int  _returned_value;
    return _returned_value;
}

/* from file freerequest.c */

/**
 ** freerequest() - FREE STRUCTURE ALLOCATED FOR A REQUEST STRUCTURE
 **/
void freerequest ( REQUEST * reqbufp )
{
}

/* from file getrequest.c */

/**
 ** getrequest() - EXTRACT REQUEST STRUCTURE FROM DISK FILE
 **/
REQUEST * getrequest (const char * file)
{
    static REQUEST * _returned_value;
    return _returned_value;
}

/* from file mod32s.c */
char * mod32s ( const char * const file )
{
    static char *  _returned_value;
    return _returned_value;
}

/**
 ** getreqno() - GET NUMBER PART OF REQUEST ID
 **/
char * getreqno (const char * req_id)
{
    static char * _returned_value;
    return _returned_value;
}

/* from file putrequest.c */
/**
 ** putrequest() - WRITE REQUEST STRUCTURE TO DISK FILE
 **/
int putrequest ( const char * file, const REQUEST * reqbufp)
{
    static int  _returned_value;
    return _returned_value;
}
