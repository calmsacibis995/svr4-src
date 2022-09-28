/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*LINTLIBRARY*/

/*	from file delclass.c */
#include "class.h"

/**
 ** delclass() - WRITE CLASS OUT TO DISK
 **/
int delclass (const char * name)
{
    static int  _returned_value;
    return _returned_value;
}

/*	from file freeclass.c */


/**
 ** freeclass() - FREE SPACE USED BY CLASS STRUCTURE
 **/
void freeclass (CLASS * clsbufp)
{
}

/*	from file getclass.c */
/**
 ** getclass() - READ CLASS FROM TO DISK
 **/
CLASS *getclass (const char * name)
{
    static CLASS * _returned_value;
    return _returned_value;
}

/*	from file putclass.c */
/**
 ** putclass() - WRITE CLASS OUT TO DISK
 **/
int putclass (const char * name, const CLASS * clsbufp)
{
    static int  _returned_value;
    return _returned_value;
}
