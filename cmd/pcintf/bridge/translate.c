/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/translate.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)translate.c	3.12	LCC);	/* Modified: 16:42:47 11/17/89 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#include "log.h"
#include "const.h"
#include <memory.h>
#include <ctype.h>
#include <string.h>


#if	0
/*
 *	cleanup		-	'escapes' special shell characters in filenames
 *				by putting a frontslash in front of them.
 */

void
cleanup(ptr, cnt)
register char *ptr;
register int cnt;
{
    register char 
	*tptr;

    char 
	temp[MAX_PATH],
	*text;

    text = ptr;
    tptr = temp;

    for (; cnt > 0; cnt--) {
	switch (*ptr) {
	    case '$':
	    case '*':
	    case '&':
	    case '#':
	    case '@':
	    case '!':
	    case '?':
	    case '\\':
	    case '\'':
	    case '\`':
	    case '(':
	    case ')':
		*tptr++ = '\\';
		*tptr++ = *ptr++;
		break;

	    default:
		*tptr++ = *ptr++;
		break;
	}
    }
    (void) memset(text, 0, sizeof temp);
    (void) memcpy(text, temp, tptr-temp);
}
#endif	/* 0 */


/*
 *	bkslash		-	translates frontslash pathname delimiters
 *				into backslashes.
 */

void
bkslash(ptr)
register char *ptr;
{

    for (;*ptr;ptr++)
	if (*ptr == '\\')
	    *ptr = '/';
}


/*
 *	ftslash			Converts backslashes into frontslashes.
 */


void
ftslash(ptr, cnt)
register char *ptr;
register int cnt;
{
    register int 
	i;

    for (i = 0; i < cnt; i++, ptr++)
	if (*ptr == '/')
	    *ptr = '\\';
}




/*
 * uppercase		Converts strings from lowercase to uppercase.
 */


void 
uppercase(ptr, len)
register char *ptr;
register int  len;
{
    register int 
	i;

    for (i = 0; i < len; i++, ptr++)
	if ((*ptr >= 'a') && (*ptr <= 'z'))
	    *ptr -= 0x20;
}



/*
 * lowercase		Converts MS-DOS filenames from uppercase to lowercase.
 */

void 
lowercase(ptr, len)
register char *ptr;
register int  len;
{
    register int 
	i;

    for (i = 0; i < len; i++, ptr++)
	if ((*ptr >= 'A') && (*ptr <= 'Z'))
	    *ptr += 0x20;
}

/*
 * scan_illegal() -	Returns pointer upon encountering an illegal character.
 */

char 
*scan_illegal(ptr)
register char *ptr;
{
    char *prev_ptr;
    int prev_period = FALSE;

    if (*ptr == '.')		/* filenames may not begin with dot */
	return ptr;
    for (; *ptr; ptr++) {
	if ((*ptr >= '`') && (*ptr <= '{') ||
	    (*ptr == '}') ||
	    (*ptr == '~') ||
	    (*ptr >= '@') && (*ptr <= 'Z') ||
	    (*ptr >= '0') && (*ptr <= '9') ||
	    (*ptr >= '#') && (*ptr <= ')') ||
	    (*ptr == '!') ||
	    (*ptr == '-') ||
	    (*ptr == '^') ||
	    (*ptr == ' ') ||
	    (*ptr == '_'))
	    continue;
	else {		/* Handle multiple .'s too	*/
	    if (*ptr == '.')
	        if (prev_period == FALSE)
	        {   prev_period = TRUE;
		    prev_ptr = ptr;
		}
		else
		    return(prev_ptr);
	    else
		return(ptr);
	}
    }
    if (*--ptr == '.')	/* Check for a dot ending the filename	*/
	return(ptr);
    else
	return(NULL);
}

/*
 * cover_illegal() -    Changes illegal characters to underscores,
			Returns TRUE if it did so, and FALSE when
			not illegal characters were found.
 */

int
cover_illegal(ptr)
register char *ptr;
{
    char *prev_ptr;
    int prev_period = FALSE;
    register int changed = FALSE;

    if (*ptr == '.') {		/* filenames may not begin with dot */
	*ptr = '_';
	changed = TRUE;
    }
    for (; *ptr; ptr++) {
	if ((*ptr >= '`') && (*ptr <= '{') ||
	    (*ptr == '}') ||
	    (*ptr >= '@') && (*ptr <= 'Z') ||
	    (*ptr >= '0') && (*ptr <= '9') ||
	    (*ptr >= '#') && (*ptr <= ')') ||
	    (*ptr == '!') ||
	    (*ptr == '-') ||
	    (*ptr == '^') ||
	    (*ptr == ' ') ||
	    (*ptr == '_'))
	    continue;
	else	/* Handle multiple .'s too	*/
	{
	    if (*ptr == '.')
		if (prev_period == FALSE)
		{   prev_period = TRUE;
		    prev_ptr = ptr;
		}
		else
		{   *prev_ptr = '_';
		    changed = TRUE;
		    prev_ptr = ptr;
		}
	    else
	    {   *ptr = '_';
		changed = TRUE;
	    }
	}
    }
    if (*--ptr == '.')	/* Check for a dot ending the filename	*/
    {	*ptr = '_';
	changed = TRUE;
    }
    return(changed);
}




/*
 * scan_uppercase() -	Returns TRUE upon encountering an uppercase character.
 */

int
scan_uppercase(ptr)
register char *ptr;
{
    for (; *ptr; ptr++) {
	if ((*ptr >= 'A') && (*ptr <= 'Z'))
	    return(TRUE);
    }
    return(FALSE);
}

/*
 	This table is the OEM dependent table that is used to translate
	charaters with the upper (0x80) bit set.  Dos has an equivalent
	table that Dana found when we were checking all of this out.
*/
static char trTable[0x80] = {
/* 0x80 */ 'c', 'u', 'e', 'a', 'a', 'a', 'a', 'c',
           'e', 'e', 'e', 'i', 'i', 'i', 'a', 'a',
/* 0x90 */ 'e', 'a', 'a', 'o', 'o', 'o', 'u', 'u',
           'y', 'o', 'u', 'x', '$', 'y', 'r', 'f',
/* 0xA0 */ 'a', 'i', 'o', 'u', 'n', 'n', 'a', 'o',
	   'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x',
/* 0xB0 */ 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x',
	   'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x',
/* 0xC0 */ 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x',
	   'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x',
/* 0xD0 */ 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x',
	   'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x',
/* 0xE0 */ 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x',
	   'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x',
/* 0xF0 */ 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x',
	   'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x',
	   };

#ifdef D_CVT2UNIX
#define vfDbg(debugArgs)        debug(0x100, debugArgs)
#else
#define vfDbg(debugArgs) 
#endif

void
cvt2unix(ptr)
register char
	*ptr;
{
register char
	*op;

    vfDbg(("Translate %s to ", ptr));
    for (op = ptr; *ptr; ptr++) {
	if (*ptr == '\\')
		*op++ = '/';
	else if (isupper(*ptr & 0x7F))
		/* Translate characters with the upper bit on */
		*op++ = ('a' - 'A') +
			((*ptr & 0x80) ? trTable[*ptr & 0x7F] : *ptr);
	else
		/* Translate characters with the upper bit on */
		*op++ = ((*ptr & 0x80) ? trTable[*ptr & 0x7F] : *ptr);
    	vfDbg(("%x ", *ptr));
    }

    *op = '\0';
    vfDbg(("\n"));
}





/*
 *    legal_pathname()		Strips leading and trailing blanks from the
 *				argument string, then checks to see if the 
 *				remaining string is empty, terminated by a
 *				backslash, or if it includes any characters
 *				not allowed in DOS identifiers.  Returns
 *				a NULL pointer if any irregularities are found,
 *				or a pointer to the modified (stripped of
 *				blanks) string otherwise.
 */

char
*legal_pathname(pathname)
char *pathname;
{
    char *ptr;

	/* Remove leading blanks from name */
    while (*pathname == ' ')
	pathname++;

	/* Scan name for char's not allowed in DOS indentifiers */
    for (ptr = pathname; ptr = scan_illegal(ptr); ptr++)
	if (*ptr != '\\')
	    return (NULL);

	/* Now scan backwards from end of name to remove trailing blanks */
    for (ptr = pathname + strlen(pathname) - 1; *ptr == ' '; ptr--) ;

	/* Error if name is now empty or terminated by a '\' */
    if ((ptr < pathname) || (*ptr == '\\'))
	return (NULL);
    else
    	*++ptr = 0;	/* Mark new end (w/out end blanks */

    return (pathname);
}
