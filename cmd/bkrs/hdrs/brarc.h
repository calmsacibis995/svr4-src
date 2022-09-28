/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:hdrs/brarc.h	1.4.2.1"

/* the archive hdr as it appears on media */

typedef struct br_arc {
	long	br_magic;	/* magic #, 0x918067de */
	long	br_length;	/* length of this hdr */
	time_t  br_date;	/* date-time of backup */
	int	br_seqno;	/* sequence num of this vol */
	long	br_blk_est;	/* estimate of 512 byte blks on archive */
	int	br_media_cap;	/* media capacity in 512 byte blks used */
	short	br_flags;	/* see definition(s) below */
	short	br_sysname_off;	/* offset to system originating the backup */
	short	br_method_off;	/* offset within br_data to method name */
	short	br_fsname_off;	/* offset to orginating file system name */
	short	br_dev_off;	/* offset to originating device */
	short	br_mname_off;	/* media name */
	short	br_fstype_off;	/* fstype string offset */
	char	br_data[1];	/* strings from above */
} br_arc_t;

#define BR_MAGIC	0x918067de

/*	flag definitions */

#define BR_IS_TOC	0x1	/* this is a table of contents volume */


/* archive info structure used to return archive info */

typedef struct archive_info {
	char *br_sysname;	/* system originating the backup */
	char *br_method;	/* method name */
	char *br_fsname;	/* orginating file system name */
	char *br_dev;		/* originating device */
	char *br_mname;		/* media name */
	char *br_fstype;	/* fstype string */
	long	br_length;	/* length of this hdr, once removed */
	time_t  br_date;	/* date-time of backup */
	int	br_seqno;	/* sequence num of this vol */
	int	br_media_cap;	/* media capacity in 512 byte blks used */
	long	br_blk_est;	/* num of blks in archive */
	short	br_flags;	/* see definition(s) below */
} archive_info_t;

/* struct used br methods to write backup/restore hdr */

struct wr_archive_hdr {
	struct  br_arc *br_hdr;		/* backup/restore hdr to write */
	int  	br_hdr_len;		/* length of b/r hdr */
	char 	*br_labelit_hdr;	/* labelit data on media */
	int  	br_lab_len;	/* len of labelit data on media */
} ;

/* struct used by br methods to build backup/restore hdr */

struct bld_archive_info {
	char	*br_method;		/* method name */
	char	*br_fsname;		/* file system name */
	char	*br_dev;		/* backup object device */
	char	*br_mname;		/* media name */
	char 	*br_fstype;		/* fstype string */
	time_t  br_date;		/* date-time of backup */
	int	br_seqno;		/* sequence num of this vol */
	int	br_media_cap;		/* media capacity in 512 byte blks */
	long	br_blk_est;		/* num of blks in archive */
	short	br_flags;		/* see definition(s) below */
} ;
