/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/cat_init.c	1.3"

#include "synonyms.h"
#include <sys/types.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <nl_types.h>
#include <string.h>

extern	caddr_t mmap();
extern	void	munmap();
extern char *malloc();
extern int errno;

extern char *_cat_itoa();
static int cat_malloc_init();

/*
 * Read a catalog and init the internal structure
 */
_cat_init(name, res)
  char *name;
  nl_catd res;
{
  struct cat_hdr hdr;
  char *mem;
  int fd;
  long magic;

  /*
   * Read file header
   */
  if((fd=open(name,0)) < 0) {
    /*
     * Need read permission
     */
    return 0;
  }

  if (read(fd, (char *)&magic, sizeof(long)) == sizeof(long)){
    if (magic == CAT_MAGIC)
      return cat_malloc_init(fd,res);
    else
      return cat_mmp_init(fd,name,res);
  }
  return 0;
}

/*
 * Read a malloc catalog and init the internal structure
 */
static
cat_malloc_init(fd, res)
  int fd;
  nl_catd res;
{
  struct cat_hdr hdr;
  char *mem;

  lseek(fd,0L,0);
  if (read(fd, (char *)&hdr, sizeof(struct cat_hdr)) != sizeof(struct cat_hdr))
    return 0;
  if ((mem = malloc(hdr.hdr_mem)) != (char*)0){

    if (read(fd, mem, hdr.hdr_mem) == hdr.hdr_mem){
      res->info.m.sets = (struct cat_set_hdr*)mem;
      res->info.m.msgs = (struct cat_msg_hdr*)(mem + hdr.hdr_off_msg_hdr);
      res->info.m.data = mem + hdr.hdr_off_msg;
      res->set_nr = hdr.hdr_set_nr;
      res->type = MALLOC;
      close(fd);
      return 1;
    } else
      free(mem);
  }

  close(fd);
  return 0;
}


extern int _mmp_opened;

/*
 * Do the gettxt stuff
 */
static
cat_mmp_init (fd,catname,res)
  char  *catname;
  nl_catd res;
{
  struct m_cat_set *sets;
  int no_sets;
  char symb_name[MAXNAMLEN];
  char symb_path[MAXNAMLEN];
  char message_file[MAXNAMLEN];
  char buf[MAXNAMLEN];
  int bytes;
  struct stat sb;
  caddr_t addr;
  char *ptr;
  static int count = 1;
  extern char *getcwd();


  if (_mmp_opened == NL_MAX_OPENED) {
    close(fd);
    return 0;
  }
  res->type = MKMSGS;

  /*
   * get the number of sets
   * of a set file
   */
  if (fstat(fd, &sb) == -1) {
    close(fd);
    return 0;
  }

  addr = mmap(0, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
  
  if ( addr == (caddr_t)-1 ) {
    close(fd);
    return 0;
  }
  no_sets = *((int*)(addr));
  if (no_sets > NL_SETMAX) {
    munmap(addr, sb.st_size);
    close(fd);
    return 0;
  }

  res->set_nr = no_sets;
  res->info.g.sets = (struct set_info *)addr;
  res->info.g.size = sb.st_size;
  res->info.g.fd = fd;

  /*
   * Create the link for gettxt
   */

  /* sprintf(symb_name,"gencat.%x.%x",getpid(),count++); */
  strcpy(symb_name,"gencat.");
  if ((ptr = _cat_itoa(getpid(), 16)) == NULL) {
    munmap(addr, sb.st_size);
    close(fd);
    return 0;
  }
  strcat(symb_name, ptr);
  strcat(symb_name, ".");
  if ((ptr = _cat_itoa(count++, 16)) == NULL) {
    munmap(addr, sb.st_size);
    close(fd);
    return 0;
  }
  strcat(symb_name, ptr);

  /* sprintf(symb_path,"%s/%s",(const char *)XOPEN_DIRECTORY,symb_name); */
  strcpy(symb_path,(const char *)XOPEN_DIRECTORY);
  strcat(symb_path,"/");
  strcat(symb_path,symb_name);

  if (catname[0] == '/') {
    /* sprintf(message_file,"%s%s",catname,(const char *)M_EXTENSION); */
    strcpy(message_file, catname);
    strcat(message_file, (const char *)M_EXTENSION);
  } else  {
    /* sprintf(message_file,"%s/%s%s",getcwd(buf,MAXNAMLEN),catname,(const char *)M_EXTENSION); */
    strcpy(message_file, getcwd(buf,MAXNAMLEN));
    strcat(message_file, "/");
    strcat(message_file, catname);
    strcat(message_file, (const char *)M_EXTENSION);
  }
  if (symlink(message_file,symb_path) < 0)  {
    munmap(addr, sb.st_size);
    close(fd);
    return 0;

  }
  
  res->info.g.link = malloc(strlen(symb_name)+1);
  if (res->info.g.link == (char*)NULL ) {
    unlink(symb_name);
    munmap(addr, sb.st_size);
    close(fd);
    return 0;
  }
  strcpy(res->info.g.link,symb_name);
  if (fcntl(fd,F_SETFD,1) == -1) {
    unlink(symb_name);
    munmap(addr, sb.st_size);
    close(fd);
    return 0;
  }
  _mmp_opened++;
  return 1;
}

static const char *hex_digs = "0123456789abcdef";
static char *buf;

#define MAXDIGS 13	/* Max. length of an integer representation */

char *
_cat_itoa(num, base)
register int num;
register int base;
{
	register char	*ptr;
	register int	len = MAXDIGS;

	if (!buf) {
		if ((buf = malloc(MAXDIGS)) == NULL)
			return(NULL);
	}
	ptr = &(buf[MAXDIGS - 1]);
	*ptr = '\0';
	while (--len > 0 && num != 0) {
		if (base == 16) {
			*(--ptr) = hex_digs[num % 16];
			num /= 16;
		} else {
			*(--ptr) = num % 10 + '0';
			num /= 10;
		}
	}
	return(ptr);
}
