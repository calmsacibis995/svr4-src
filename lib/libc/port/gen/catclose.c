/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/catclose.c	1.3"

#ifdef __STDC__
	#pragma weak catclose = _catclose
#endif
#include "synonyms.h"
#include <dirent.h>
#include <stdio.h>
#include <nl_types.h>
#include <locale.h>
#include <string.h>

extern char *setlocale();
extern int _mmp_opened;

catclose(catd)
  nl_catd catd;
{
char symb_path[MAXNAMLEN];
char old_locale[MAXNAMLEN];

  if ((int)catd != -1 || catd == (nl_catd)NULL) {
    
    if (catd->type == MALLOC) {
        free(catd->info.m.sets);
	catd->type = -1;
	return 0;
    } else
    if (catd->type == MKMSGS) {
        munmap(catd->info.g.sets,catd->info.g.size);

        /* sprintf(symb_path,"%s/%s",(const char *)XOPEN_DIRECTORY,catd->info.g.link); */
	strcpy(symb_path, (const char *)XOPEN_DIRECTORY);
	strcat(symb_path, "/");
	strcat(symb_path, catd->info.g.link);

        unlink(symb_path);
	close(catd->info.g.fd);
        free (catd->info.g.link);
	/*
	 * force gettxt to forget
	 * its xopen catalogs
	 */
        strcpy(old_locale,setlocale(LC_MESSAGES,(char*)NULL));
	setlocale(LC_MESSAGES,"XOPEN1");
	gettxt("XOPEN1:1","who cares if its not there");
	setlocale(LC_MESSAGES,old_locale);
	catd->type = -1;
	_mmp_opened--;
	return 0;
    }
  }
  /*
   * was a bad catd
   */
  return -1;
}
