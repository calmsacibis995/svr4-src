/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)gencat:cat_mmp_dump.c	1.1.1.1"
#include <dirent.h>
#include <stdio.h>
#include <ctype.h>
#include <nl_types.h>

extern FILE *tempfile;
extern struct cat_set *sets;
static char *bldtmp();
extern char msg_buf[];

/*
 * dump a memory mapped gettxt library.
 */

cat_mmp_dump(catalog)
char *catalog;
{
  FILE *f_shdr;
  FILE *f_msg;
  struct cat_set *p_sets;
  struct cat_msg *p_msg;
  struct m_cat_set set;
  int status;
  int msg_len;
  int nmsg = 1;
  int i;
  int no_sets = 0;
  char *tmp_file;
  char message_file[MAXNAMLEN];
  

  f_shdr = fopen(catalog,"w+");
  if (f_shdr == NULL) {
    fprintf(stderr,"cat_mmp_dump : Cannot access %s\n",catalog);
    fatal();
  }
  if (fwrite((char *)&no_sets, sizeof (no_sets) , 1, f_shdr) != 1){
    fprintf(stderr, "cat_lib_dump : write error to %s\n",catalog);
    fatal();
  }
  /*
   * create a tempory for messages
   */
  tmp_file = bldtmp();
  if (tmp_file == (char *)NULL) {
    fprintf(stderr,"cat_mmp_dump : Cannot malloc. Out of Space\n");
    fatal();
  }
  f_msg = fopen(tmp_file,"w+");
  if (f_msg == NULL) {
    fprintf(stderr,"cat_mmp_dump : Cannot create tempory file %s\n",tmp_file);
    fatal();
  }
  /* 
   * for all the sets
   */
  p_sets = sets;
  nmsg = 1;
  while (p_sets != 0){
    no_sets++;
    /*
     * if set holes then
     * fill them
     */
    set.first_msg = 0;
    set.last_msg = 0;

    while (no_sets != p_sets->set_nr) {
      if (fwrite((char *)&set, sizeof (struct m_cat_set) , 1, f_shdr) != 1){
        fprintf(stderr, "cat_lib_dump : write error in shdr temp file\n");
        fatal();
      }
      no_sets++;
    }
    p_msg = p_sets->set_msg;
    
    /*
     * Keep offset in shdr temp file to mark the set's begin
     */
    if (p_msg) {
      set.last_msg = 0;
      set.first_msg = nmsg;
      while (p_msg != 0){
        msg_len = p_msg->msg_len;
        /*
         * Get message from main temp file
         */
        if (fseek(tempfile, p_msg->msg_off, 0) != 0){
          fprintf(stderr, "cat_lib_dump : Seek error in temp file\n");
          fatal();
        }
        if (fread(msg_buf, 1, msg_len, tempfile) != msg_len){
          fprintf(stderr, "cat_lib_dump : Read error in temp file\n");
          fatal();
        }
	msg_buf[msg_len-1] = '\n';
        
        /*
         * Put it in the messages temp file and keep offset
         */
        for (i=set.last_msg+1;i <p_msg->msg_nr;i++,nmsg++) {
          if (fwrite(DFLT_MSG, 1, strlen(DFLT_MSG), f_msg) != strlen(DFLT_MSG)){
            fprintf(stderr, "cat_lib_dump : Write error in msg temp file\n");
            fatal();
          }
          if (fwrite("\n", 1, 1, f_msg) != 1 ) {
            fprintf(stderr, "cat_lib_dump : Write error in msg temp file\n");
            fatal();
          }
	}
        set.last_msg  = p_msg->msg_nr;
        if (fwrite(msg_buf, 1, msg_len, f_msg) != msg_len){
          fprintf(stderr, "cat_lib_dump : Write error in msg temp file\n");
          fatal();
        }
        nmsg++;
        p_msg = p_msg->msg_next;
      }
    } else {
      set.first_msg = 0;
      set.last_msg = 0;
    }
    
    /*
     * Put set hdr into set temp file
     */
    if (fwrite((char *)&set, sizeof (struct m_cat_set) , 1, f_shdr) != 1){
      fprintf(stderr, "cat_lib_dump : write error in shdr temp file\n");
      fatal();
    }
    p_sets = p_sets->set_next;
  }
  
  /*
   * seek to begining of  file
   * and then write total sets
   */
  if (fseek(f_shdr, 0 , 0) != 0){
    fprintf(stderr, "cat_lib_dump : Seek error in %s \n",catalog);
    fatal();
  }
  if (fwrite((char *)&no_sets, sizeof (no_sets) , 1, f_shdr) != 1){
    fprintf(stderr, "cat_lib_dump : write error to %s\n",catalog);
    fatal();
  }

  fclose(tempfile);
  fclose(f_shdr);
  fclose(f_msg);

  sprintf(message_file,"%s%s",catalog,M_EXTENSION);
  if (fork() == 0) {
	
    execlp(BIN_MKMSGS,BIN_MKMSGS,"-o",tmp_file,message_file,(char*)NULL);
    fprintf(stderr,"Cannot Exec %s\n",MKMSGS);
    fatal();

  }
  wait(&status);
  unlink(tmp_file);
  if (status) {
    fprintf(stderr,"Mkmsgs Failed %s\n",MKMSGS);
    fatal();
  }
}

static 
char *
bldtmp()
{

  char *getenv();
  static char buf[MAXNAMLEN];
  char *tmp;
  char *mktemp();

  tmp = getenv("TMPDIR");
  if (tmp==(char *)NULL || *tmp == '\0')
    tmp="/tmp";

  sprintf(buf,"%s/%s",tmp,mktemp("gencat.XXXXXX"));
  return buf;
}
