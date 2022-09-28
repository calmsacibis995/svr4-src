/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*LINTLIBRARY*/
# include	"stdio.h"

/*	from file conv.c */
# include	"filters.h"

/*	from file delfilter.c */

/**
 ** delfilter() - DELETE A FILTER FROM FILTER TABLE
 **/

int delfilter ( const char * name )
{
    static int _returned_value;
    return _returned_value;
}

/*	from file dumpfilters.c */

/**
 ** dumpfilters() - WRITE FILTERS FROM INTERNAL STRUCTURE TO FILTER TABLE
 **/
int dumpfilters (const char *file)
{
    static int  _returned_value;
    return _returned_value;
}

/*	from file freefilter.c */
/**
 ** freefilter() - FREE INTERNAL SPACE ALLOCATED FOR A FILTER
 **/
void freetempl (TEMPLATE * templ)
{
}
void free_filter (_FILTER * pf)
{
}
void freefilter (FILTER * pf)
{
}

/*	from file getfilter.c */

/**
 ** getfilter() - GET FILTER FROM FILTER TABLE
 **/
FILTER * getfilter ( const char * name)
{
    static FILTER * _returned_value;
    return _returned_value;
}

/*	from file filtertable.c */
/**
 ** get_and_load() - LOAD REGULAR FILTER TABLE
 **/
int get_and_load ( void )
{
    static int  _returned_value;
    return _returned_value;
}

/**
 ** open_filtertable( void )
 **/
FILE * open_filtertable (const char *file, const char *mode)
{
    static FILE * _returned_value;
    return _returned_value;
}

/**
 ** close_filtertable( void )
 **/
void close_filtertable (FILE * fp)
{
}

/*	from file insfilter.c */

/**
 ** insfilter( void )
 **/
FILTERTYPE insfilter ( char **pipes, char *input_type,
		       char *output_type, char *printer_type,
		       char *printer, char **parms,
		       unsigned short *flagsp)
{
    static FILTERTYPE _returned_value;
    return _returned_value;
}

/*	from file loadfilters.c */

int loadfilters (const char *file)
{
    static int _returned_value;
    return _returned_value;
}

/*	from file putfilter.c */

int putfilter (const char *name, const FILTER *flbufp)
{
    static int _returned_value;
    return _returned_value;
}

/*	from file search.c */
/**
 ** search_filter() - SEARCH INTERNAL FILTER TABLE FOR FILTER BY NAME
 **/
_FILTER *search_filter (const char *name)
{
    static _FILTER * _returned_value;
    return _returned_value;
}

/*	from file trash.c */

/**
 ** trash_filters() - FREE ALL SPACE ALLOCATED FOR FILTER TABLE
 **/
void trash_filters ( void )
{
}
