/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*LINTLIBRARY*/

#include "access.h"

/*	from file allowed.c */

/**
 ** is_user_admin() - CHECK IF CURRENT USER IS AN ADMINISTRATOR
 **/

int is_user_admin ( void )
{
    static int _returned_value;
    return _returned_value;
}

/**
 ** is_user_allowed() - CHECK USER ACCESS ACCORDING TO ALLOW/DENY LISTS
 **/
int is_user_allowed ( const char * user, const char ** allow,
		      const char ** deny )
{
    static int  _returned_value;
    return _returned_value;
}

/**
 ** is_user_allowed_form() - CHECK USER ACCESS TO FORM
 **/
int is_user_allowed_form ( const char * user, const char * form )
{
    static int _returned_value;
    return _returned_value;
}

/**
 ** is_user_allowed_printer() - CHECK USER ACCESS TO PRINTER
 **/
int is_user_allowed_printer ( const char * user, const char * printer)
{
    static int _returned_value;
    return _returned_value;
}

/**
 ** is_form_allowed_printer() - CHECK FORM USE ON PRINTER
 **/
int is_form_allowed_printer ( const char * form,  const char * printer)
{
    static int _returned_value;
    return _returned_value;
}

/**
 ** allowed() - GENERAL ROUTINE TO CHECK ALLOW/DENY LISTS
 **/
int allowed (const char * item, const char ** allow, const char ** deny)
{
    static int  _returned_value;
    return _returned_value;
}

/*	from file change.c */

/**
 ** deny_user_form() - DENY USER ACCESS TO FORM
 **/
int deny_user_form ( const char ** user_list,  const char * form)
{
    static int  _returned_value;
    return _returned_value;
}

/**
 ** allow_user_form() - ALLOW USER ACCESS TO FORM
 **/
int allow_user_form (const char ** user_list, const char * form)
{
    static int  _returned_value;
    return _returned_value;
}

/**
 ** deny_user_printer() - DENY USER ACCESS TO PRINTER
 **/
int deny_user_printer (const char ** user_list, const char * printer)
{
    static int  _returned_value;
    return _returned_value;
}

/**
 ** allow_user_printer() - ALLOW USER ACCESS TO PRINTER
 **/
int allow_user_printer (const char ** user_list, const char * printer)
{
    static int  _returned_value;
    return _returned_value;
}

/**
 ** deny_form_printer() - DENY FORM USE ON PRINTER
 **/
int deny_form_printer (const char ** form_list, const char * printer)
{
    static int  _returned_value;
    return _returned_value;
}

/**
 ** allow_form_printer() - ALLOW FORM USE ON PRINTER
 **/
int allow_form_printer (const char ** form_list, const char * printer)
{
    static int  _returned_value;
    return _returned_value;
}

/*	from file dumpaccess.c */

/**
 ** dumpaccess() - DUMP ALLOW OR DENY LISTS
 **/
int dumpaccess (const char * dir, const char * name, const char * prefix,
		char *** pallow, char *** pdeny)
{
    static int  _returned_value;
    return _returned_value;
}

/*	from file files.c */


/**
 ** getaccessfile() - BUILD NAME OF ALLOW OR DENY FILE
 **/
char *getaccessfile (const char * dir, const char * name,
		     const char * prefix, const char * base)
{
    static char * _returned_value;
    return _returned_value;
}

/*	from file loadaccess.c */
/**
 ** load_userform_access() - LOAD ALLOW/DENY LISTS FOR USER+FORM
 **/
int load_userform_access (const char *form, char *** pallow, char *** pdeny)
{
    static int  _returned_value;
    return _returned_value;
}

/**
 ** load_userprinter_access() - LOAD ALLOW/DENY LISTS FOR USER+PRINTER
 **/
int load_userprinter_access (const char * printer, char *** pallow,
			     char *** pdeny)
{
    static int  _returned_value;
    return _returned_value;
}

/**
 ** load_formprinter_access() - LOAD ALLOW/DENY LISTS FOR FORM+PRINTER
 **/
int load_formprinter_access (const char * printer, char *** pallow,
			     char *** pdeny)
{
    static int  _returned_value;
    return _returned_value;
}

/**
 ** loadaccess() - LOAD ALLOW OR DENY LISTS
 **/
int loadaccess (const char * dir, const char * name, const char * prefix,
		char *** pallow, char *** pdeny)
{
    static int  _returned_value;
    return _returned_value;
}
