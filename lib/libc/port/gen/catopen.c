/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/catopen.c	1.4"

#ifdef __STDC__
	#pragma weak catopen = _catopen
#endif
#include "synonyms.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <nl_types.h>
#include <locale.h>
#include <string.h>
#include <stdlib.h>

int _mmp_opened = 0;

static char *
cat_name(name, path)
  char *name;
  char *path;
{
  const char *nlspath;
  const char *lang, *ptr, *ptr3;
  char *ptr2, c, d;
  struct stat *buf;
  const char *def_nlspath = DEF_NLSPATH;
  struct stat buff;
    
  /*
   * A name that contains a slash is a pathname
   */
  if (strchr(name, '/') != 0)
    return name;
    
  /*
   * Get env variables
   */
  if ((nlspath = getenv(NL_PATH)) == 0){
    /*
     * No env var : use default
     */
    nlspath = def_nlspath;
  }
  
  if ((lang = getenv(NL_LANG)) == 0 && (lang = setlocale(LC_MESSAGES,(char*)NULL)) == 0)
    lang = NL_DEF_LANG;
    
  /*
   * Scan nlspath and replace if needed
   */
  ptr = nlspath;
  ptr2 = path;
  *ptr2 = '\0';
  for (;;){
    switch (c = *ptr++){
    case ':':
      if (ptr2 == path){
        /*
         * Leading ':' Check current directory
         */
        if (access(name, 4) == 0)
          return name;
        continue;
      }
    case '\0':
	/*
	 * Found a complete path
	 */
      	*ptr2 = '\0';
        /*
	 * Test to see that path is not a pure directory,
	 * if it is then attempt toopen it and append the
	 * filename "name" to it, in an attempt to succeed
	 * This syntax is not explicitly defined in XPG2, but
	 * is a logical extension to it. (XVS tests it too).
	 */
	
	buf = &buff;
	stat(path, buf);
	if ((buf -> st_mode) & S_IFDIR) {
		strcat(path, "/");
		if (access(strcat(path, name), 4) == 0)
			return path;
		} /* otherwise branch out to end */
	else
		if ( access(path, 4) == 0)
			return path;
	
	if (c == '\0'){
		/*
	 	 * File not found
	 	 */
	  	if (nlspath == def_nlspath)
		    return 0;
		nlspath = def_nlspath;
		ptr = nlspath;
        }
        ptr2 = path;
        *ptr2 = '\0';
        continue;
    case '%':
      /*
       * Expecting var expansion
       */
      switch(c = *ptr++){
      case 'L':
        ptr3 = lang;
        while ((d = *ptr3++) != 0)
          *ptr2++ = d;
        continue;
      case 'l':
        ptr3 = lang;
        while ((d = *ptr3++) != 0 && d != '.' && d != '_')
          *ptr2++ = d;
        continue;
      case 't':
        ptr3 = strchr(lang, '_');
        if (ptr3++ != 0){
          while ((d = *ptr3++) != 0 && d != '.')
            *ptr2++ = d;
        }
        continue;
      case 'c':
        ptr3 = strchr(lang, '.');
        if (ptr3++ != 0){
          while ((d = *ptr3++) != 0)
            *ptr2++ = d;
        }
        continue;
      case 'N':
        ptr3 = name;
        while ((d = *ptr3++) != 0)
          *ptr2++ = d;
        continue;
      case '%':
        *ptr2++ = '%';
        continue;
      default:
        *ptr2++ = '%';
        *ptr2++ = c;
        continue;
      }
    default:
      *ptr2++ = c;
      continue;
    }
  }
}

nl_catd
catopen(name, mode)
  const char *name;
  int mode;
{
  nl_catd catd;
  int fd;
  char path[NL_MAXPATHLEN];
  
  /*
   * Allocate space to hold the necessary data
   */
  if ((catd = (nl_catd)malloc(sizeof (nl_catd_t))) == 0)
      return (nl_catd)-1;
  
  /*
   * Get actual file name and open file
   */
  if ((name = cat_name(name,path)) != 0){
  
    /*
     * init internal data structures
     */
    if (_cat_init(name, catd))
      return catd;
  }

  free(catd);
  return (nl_catd)-1;
}
