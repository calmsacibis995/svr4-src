/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _NL_TYPES_H
#define _NL_TYPES_H

#ident	"@(#)head:nl_types.h	1.4"

/*
 * allow for limits.h
 */
#ifndef NL_SETMAX
#define NL_SETMAX	1024
#define NL_MSGMAX	32767
#define NL_TEXTMAX	1024
#endif

#define NL_MAXPATHLEN	1024
#define NL_PATH		"NLSPATH"
#define NL_LANG		"LANG"
#define NL_DEF_LANG	"english"
#define NL_SETD		1
#define NL_MAX_OPENED	10


/*
 * gencat internal structures
 */
struct cat_msg {
  int msg_nr;			/* The message number */
  int msg_len;			/* The actual message len */
  long msg_off;			/* The message offset in the temporary file */
  char *msg_ptr;		/* A pointer to the actual message */
  struct cat_msg *msg_next;     /* Next element in list */
};

struct cat_set {
  int set_nr;			/* The set number */
  int set_msg_nr;		/* The number of messages in the set */
  struct cat_msg *set_msg;	/* The associated message list */
  struct cat_set *set_next;	/* Next set in list */
};



/*
 * mkmsgs format set
 * information
 */
struct m_cat_set {
  int first_msg;		/* The first message number */
  int last_msg;			/* The last message in the set */
};

/*
 * structure in file
 */
struct set_info {
	int no_sets;
	struct m_cat_set sn[1];
};

#define CMD_SET		"set"
#define CMD_SET_LEN	3
#define CMD_DELSET	"delset"
#define CMD_DELSET_LEN	6
#define CMD_QUOTE	"quote"
#define CMD_QUOTE_LEN	5

#define XOPEN_DIRECTORY "/usr/lib/locale/Xopen/LC_MESSAGES"
#define DFLT_MSG	"\01"   
#define M_EXTENSION	".m"
/*
 * Default search pathname
 */
#define DEF_NLSPATH	"./%N"

struct cat_hdr {
  long hdr_magic;		/* The magic number */
  int hdr_set_nr;		/* Set nr in file */
  int hdr_mem;			/* The space needed to load the file */
  long hdr_off_msg_hdr;		/* Position of messages headers in file */
  long hdr_off_msg;		/* Position of messages in file */
};

struct cat_set_hdr {
  int shdr_set_nr;		/* The set number */
  int shdr_msg_nr;		/* Number of messages in set */
  int shdr_msg;			/* Start offset of messages in messages list */
};

struct cat_msg_hdr{
  int msg_nr;			/* The messge number */
  int msg_len;			/* message len */
  int msg_ptr;			/* message offset in table */
};

#define CAT_MAGIC	0xFF88FF89

typedef int nl_item ;

typedef struct {
  char type;
  int set_nr;
  union {
    struct malloc_data {
      struct cat_set_hdr *sets;
      struct cat_msg_hdr *msgs;
      char *data;
    } m;
    struct gettxt_data {
      struct set_info *sets;
      int size;
      int fd;
      char *link;
    } g;
 } info;
} nl_catd_t;

typedef nl_catd_t *nl_catd;

/*
 * type fields for nl_catd_t
 */
#define MKMSGS		'M'	/* mkmsgs interfaces */
#define MALLOC		'm'	/* old style malloc  */

#define BIN_MKMSGS	"mkmsgs"

#ifdef __STDC__

int catclose(nl_catd);
char *catgets(nl_catd, int, int, char *);
nl_catd catopen(const char *, int);

#if !defined(_POSIX_SOURCE) && !defined(_XOPEN_SOURCE)
char *gettxt(const char *, const char *);
#endif

#endif
#endif /* _NL_TYPES_H */
