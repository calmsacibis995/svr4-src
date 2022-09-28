/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)gencat:cat_build.c	1.1.1.1"

#include <dirent.h>
#include <locale.h>
#include <stdio.h>
#include "nl_types.h"
#include <malloc.h>

extern int list;
extern FILE *tempfile;
extern char msg_buf[];
extern char *gettxt();

struct cat_set *sets;
static void strcpy_conv();
/*
 * Read a catalog and build the internal structure
 */
cat_build(catname)
  char *catname;
{
  FILE *fd;
  long magic;
  
    
  /*
   * Test for mkmsgs
   * or old style malloc
   */

  if ((fd = fopen(catname, "r")) == 0){
    /*
     * does not exist
     */
    return 1;
  }

  if (fread(&magic, sizeof(long), 1, fd) != 1){
    fprintf(stderr, "%s : Cannot read catalog\n", catname);
    fatal();
  }
  if (magic == CAT_MAGIC) 
    cat_malloc_build(fd,catname);
  else 
    cat_mmp_build(fd,catname);

  fclose(fd);
  
  return 1;
}

static
cat_malloc_build (fd,catname)
  char  *catname;
  FILE *fd;
{
  struct cat_hdr hdr;
  struct cat_set *cur_set, *set_last;
  struct cat_msg *cur_msg, *msg_last;
  char *data;
  struct cat_set_hdr set_hdr;
  struct cat_msg_hdr msg_hdr;
  register int i;

    
  /*
   * Read malloc file header
   */
  rewind(fd);
  if (fread((char *)&hdr, sizeof(struct cat_hdr), 1, fd) != 1){
    fprintf(stderr, "%s : Cannot read header\n", catname);
    fatal();
  }
  /*
   * Read sets headers
   */
  for (i = 0 ; i < hdr.hdr_set_nr ; i++){
    struct cat_set *new;
    if ((new = (struct cat_set *)malloc(sizeof(struct cat_set))) == 0){
      fprintf(stderr, "Out of memory (cat_build.c (1))\n");
      fatal();
    }
    if (fread((char *)&set_hdr, sizeof(struct cat_set_hdr), 1, fd) != 1){
      fprintf(stderr, "Cannot read set headers\n");
      fatal();
    }
    new->set_nr = set_hdr.shdr_set_nr;
    new->set_msg_nr = set_hdr.shdr_msg_nr;
    new->set_next = 0;
    if (sets == 0)
      sets = new;
    else
      set_last->set_next = new;
    set_last = new;
  }

  /*
   * Read messages headers
   */
  for (cur_set = sets ; cur_set != 0 ; cur_set = cur_set->set_next){
    cur_set->set_msg = 0;
    for (i = 0 ; i < cur_set->set_msg_nr ; i++){
      struct cat_msg *new;

      if ((new = (struct cat_msg *)malloc(sizeof(struct cat_msg))) == 0){
        fprintf(stderr, "Out of memory (cat_build.c (2))\n");
        fatal();
      }
      if (fread((char *)&msg_hdr, sizeof(struct cat_msg_hdr), 1, fd) != 1){
        fprintf(stderr, "%s: Cannot read message headers\n", catname);
        fatal();
      }
      new->msg_nr = msg_hdr.msg_nr;
      new->msg_len = msg_hdr.msg_len;
      new->msg_next = 0;
      if (cur_set->set_msg == 0)
        cur_set->set_msg = new;
      else
        msg_last->msg_next = new;
      msg_last = new;
    }
  }
  
  /*
   * Read messages.
   */
  for (cur_set = sets ; cur_set != 0 ; cur_set = cur_set->set_next){
    for (cur_msg = cur_set->set_msg ;cur_msg!= 0;cur_msg= cur_msg->msg_next){

      if (fread(msg_buf, 1, cur_msg->msg_len, fd) != cur_msg->msg_len ){
        fprintf(stderr,"%s: Cannot read messages\n", catname);
        fatal();
      }

      /*
       * Put message in the temp file and keep offset
       */
      cur_msg->msg_off = ftell(tempfile);

      if (list)
	printf("Set %d,Message %d,Offset %d,Length %d\n%.*s\n*\n",
	  cur_set->set_nr,cur_msg->msg_nr,cur_msg->msg_off,cur_msg->msg_len,
	  cur_msg->msg_len,msg_buf);

      if (fwrite(msg_buf, 1, cur_msg->msg_len, tempfile) != cur_msg->msg_len){
        fprintf(stderr, "add_msg : Write error in temp file\n");
        fatal();
      }
    }
  }
}

static
cat_mmp_build (fd,catname)
  FILE *fd;
  char  *catname;
{
  struct cat_set *cur_set, *set_last;
  struct cat_msg *cur_msg, *msg_last;
  struct m_cat_set set;
  register int i;
  register int j;
  register int k;
  int no_sets;
  char *p;
  char temp_text[NL_TEXTMAX];
  struct cat_set *new;
  char symb_name[MAXNAMLEN];
  char *symb_catalog;
  char message_file[MAXNAMLEN];
  char old_locale[MAXNAMLEN];
  extern char *mktemp();
  char buf[MAXNAMLEN];



  /*
   * get the number of sets
   * of a set file
   */
  rewind(fd);
  if (fread(&no_sets, 1, sizeof(int), fd) != sizeof(int) ){
    fprintf(stderr, "%s: Cannot get number of sets\n", catname);
    fatal();
  }
  
  /*
   * Create the link for gettxt
   */
  symb_catalog =mktemp("gencat.XXXXXX");
  sprintf(symb_name,"%s/%s",XOPEN_DIRECTORY,symb_catalog);
  if (catname[0] == '/')
    sprintf(message_file,"%s%s",catname,M_EXTENSION);
  else {
    (void)getcwd(message_file,MAXNAMLEN);
    strcat(message_file,"/");
    strcat(message_file,catname);
    strcat(message_file,M_EXTENSION);
  }
  if (symlink(message_file,symb_name) < 0) {
    fprintf(stderr, "%s: Cannot create link\n", catname);
    fatal();
  }
  /*
   * get the messages
   */
  strcpy(old_locale,setlocale(LC_MESSAGES,(char*)NULL));

  for(i=1;i<=no_sets;i++) {
    if (fread(&set,1,sizeof(struct m_cat_set),fd) != sizeof(struct m_cat_set) ){
      fprintf(stderr, "%s: Cannot get set information\n", catname);
      unlink(symb_name);
      fatal();
    }
    if ((new = (struct cat_set *)malloc(sizeof(struct cat_set))) == 0){
      fprintf(stderr, "Out of memory (cat_build.c (10))\n");
      unlink(symb_name);
      fatal();
    }
    if (set.first_msg == 0)
      continue;
    new->set_nr = i;
    new->set_next = 0;
    if (sets == 0)
      sets = new;
    else
      set_last->set_next = new;
    set_last = new;

    /*
     * create message headers
     */
    new->set_msg = 0;
    for(j=1, k=set.first_msg; j <=set.last_msg && k != 0; k++, j++) {
        /*
         * check if message if so fill message header then put message
         * intempfile
         */
	sprintf(buf,"%s:%d",symb_catalog,k);
	setlocale(LC_MESSAGES,"Xopen");
	p = gettxt(buf,DFLT_MSG);
	strcpy_conv(temp_text,p);
	setlocale(LC_MESSAGES,old_locale);

	if (strcmp(temp_text,DFLT_MSG)) {
          struct cat_msg *new_msg;
          if ((new_msg=(struct cat_msg *)malloc(sizeof(struct cat_msg))) == 0){
            fprintf(stderr, "Out of memory (cat_build.c (20))\n");
	    unlink(symb_name);
            fatal();
	  }
          if (new->set_msg == 0) 
	    new->set_msg = new_msg;
	  else
	    msg_last->msg_next = new_msg;
	  new_msg->msg_nr = j;
	  new_msg->msg_len = strlen(temp_text)+1;
	  new_msg->msg_next = 0;
	  msg_last = new_msg;
	  /*
	   * write message to tempfile
	   */
          new_msg->msg_off = ftell(tempfile);
	  if (list)
	    printf("Set %d,Message %d,Text %s\n*\n",i,j,temp_text);
          if(fwrite(temp_text,1,new_msg->msg_len,tempfile)!=new_msg->msg_len){
            fprintf(stderr, "add_msg : Write error in temp file\n");
	    unlink(symb_name);
            fatal();
          }
	}
    }
  }
  unlink(symb_name);
}

/* 
 * mkmsgs cant handle the new line
 */
static void
strcpy_conv ( a , b )
char *a,*b;
{

  while (*b) {

    switch (*b) {

      case '\n' :
        *a++ = '\\';
        *a++ = 'n';
	b++;
        break;

      case '\\' :
        *a++ = '\\';
        *a++ = '\\';
	b++;
        break;

      default :
	*a++ = *b++;
    }
  }
  *a = '\0';
}
