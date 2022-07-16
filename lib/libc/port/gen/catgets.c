/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/catgets.c	1.3"

#ifdef __STDC__
	#pragma weak catgets = _catgets
#endif
#include "synonyms.h"
#include <dirent.h>
#include <locale.h>
#include <stdio.h>
#include <nl_types.h>
#include <string.h>
#define min(a,b)  (a>b?b:a)

extern char *gettxt();
extern char *setlocale();
extern char *_cat_itoa();

char *
catgets(catd, set_num, msg_num, s)
  nl_catd catd;
  int set_num, msg_num;
  char *s;
{
  register int i;
  struct cat_set_hdr *sets;
  struct cat_msg_hdr *msgs;
  struct m_cat_set *set;
  char *data;
  int first_msg;
  char *msg;
  char **msgs_addr;
  int msg_len;
  int message_no;
  char buf[MAXNAMLEN];
  char *p;
  char old_locale[MAXNAMLEN];
  unsigned int start,offset;
  
  if ( (int)catd == -1 || catd == (nl_catd)NULL)
    return s;


  switch (catd->type) {
  
    case MALLOC :
      /*
       * Locate set
       */
      sets = catd->info.m.sets;
      msgs = catd->info.m.msgs;
      data = catd->info.m.data;
      for (i = min(set_num, catd->set_nr) - 1 ; i >= 0 ; i--){
        if (set_num == sets[i].shdr_set_nr){
          first_msg = sets[i].shdr_msg;
  
          /*
           * Locate message in set
           */
          for (i = min(msg_num, sets[i].shdr_msg_nr) - 1 ; i >= 0 ; i--){
            if (msg_num == msgs[first_msg + i].msg_nr){
              i += first_msg;
              msg = data + msgs[i].msg_ptr;
              msg_len = msgs[i].msg_len;
              return msg;
            }
            if (msg_num > msgs[first_msg + i].msg_nr)
              return s;
          }
          return s;
        }
        if (set_num > sets[i].shdr_set_nr)
          return s;
      }
      return s;
  
  case MKMSGS :
      /*
       * get it from
       * a mkmsgs catalog
       */
      if (set_num > catd->set_nr)
        return s;
      set = &(catd->info.g.sets->sn[set_num -1]);
      if (msg_num > set->last_msg)
        return s;
      message_no = set->first_msg + msg_num -1;

      /* sprintf(buf,"%s:%d",catd->info.g.link,message_no); */
      strcpy(buf,catd->info.g.link);
      strcat(buf,":");
      if ((p = _cat_itoa(message_no, 10)) == NULL)
        return s;
      strcat(buf, p);

      strcpy(old_locale,setlocale(LC_MESSAGES,""));
      setlocale(LC_MESSAGES,"Xopen");
      p = gettxt(buf,DFLT_MSG);
      setlocale(LC_MESSAGES,old_locale);
      if (strcmp(p,DFLT_MSG) && strcmp(p,"Message not found!!\n"))
        return p;
      return s;
  
  default :
      return s;

  }
}
