/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.sbin/in.named/db.h	1.1.2.1"

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */


/*
 * Global structures and variables for data base routines.
 */

#define INVBLKSZ	7	/* # of namebuf pointers per block */
#define INVHASHSZ	919	/* size of inverse hash table */

	/* max length of data in RR data field */
#define MAXDATA		256

/*
 * Hash table structures.
 */
struct databuf {
	struct	databuf *d_next;	/* linked list */
	u_long	d_ttl;			/* time to live */
	short	d_flags;
	short	d_zone;			/* zone number */
	short	d_class;		/* class number */
	short	d_type;			/* type number */
	short	d_mark;			/* place to mark data */
	short	d_size;			/* size of data area */
	u_long	d_nstime;		/* NS response time, milliseconds */
	char	d_data[MAXDATA]; 	/* the data is malloc'ed to size */
};
#define DATASIZE(n) (sizeof(struct databuf) - MAXDATA + n)

/*
 * d_flags definitions
 */
#define DB_F_HINT       0x01	/* databuf belongs to fcachetab */

struct namebuf {
	char	*n_dname;		/* domain name */
	u_int	n_hashval;		/* hash value of n_dname */
	struct	namebuf *n_next;	/* linked list */
	struct	databuf *n_data;	/* data records */
	struct	namebuf *n_parent;	/* parent domain */
	struct	hashbuf *n_hash;	/* hash table for children */
};

struct invbuf {
	struct	invbuf *i_next;		/* linked list */
	struct	namebuf	*i_dname[INVBLKSZ];	/* domain name */
};

struct hashbuf {
	int	h_size;			/* size of hash table */
	int	h_cnt;			/* number of entries */
	struct	namebuf	*h_tab[1];	/* malloc'ed as needed */
};
#define HASHSIZE(s) (s*sizeof(struct namebuf *) + 2*sizeof(int))

#define HASHSHIFT	3
#define HASHMASK	0x1f

/*
 * Flags to updatedb
 */
#define DB_NODATA	0x01	/* data should not exist */
#define DB_MEXIST	0x02	/* data must exist */
#define DB_DELETE	0x04	/* delete data if it exists */
#define DB_NOTAUTH	0x08	/* must not update authoritative data */
#define DB_NOHINTS      0x10	/* don't reflect update in fcachetab */

#define DB_Z_CACHE      (0)	/* cache-zone-only db_dump()  */
#define DB_Z_ALL        (-1)	/* normal db_dump() */

/*
 * Error return codes
 */
#define OK		0
#define NONAME		-1
#define NOCLASS		-2
#define NOTYPE		-3
#define NODATA		-4
#define DATAEXISTS	-5
#define NODBFILE	-6
#define TOOMANYZONES	-7
#define GOODDB		-8
#define NEWDB		-9
#define AUTH		-10

extern struct hashbuf *hashtab;		/* root hash table */
extern struct invbuf *invtab[];		/* inverse hash table */
extern struct hashbuf *fcachetab;	/* hash table for cache read from file*/

extern struct namebuf *nlookup();
extern struct namebuf *savename();
extern struct databuf *savedata();
extern struct databuf *rm_datum();
extern struct hashbuf *savehash();
extern struct invbuf *saveinv();
extern char *savestr();
extern char *malloc(), *realloc(), *calloc();
