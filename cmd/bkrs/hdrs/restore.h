/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:hdrs/restore.h	1.5.2.1"

/* Restore-specific stuff */

#define	RSLOGFILE	"rslog"

/* Type names for restore objects */
#define	R_FILE_TYPE	"F"
#define	R_PARTITION_TYPE	"P"
#define	R_DIRECTORY_TYPE	"D"
#define	R_FILESYS_TYPE	"S"
#define	R_DISK_TYPE	"A"

#define	R_N_TYPES	5

/* Stopat values */
#define	RSTM_ONE	1
#define	RSTM_ALL	2

/* rsspawn() flags */
#define	RS_ISTOC	0x1
#define	RS_SFLAG	0x2
#define	RS_VFLAG	0x4

/* Structure for describing requests */
typedef	struct	rs_rqst_s	{
	char *jobid;
	char *object;	/* Full path of item to restore */
	char *oname;	/* oname of item */
	char *odev;	/* odevice of item */
	time_t	date;	/* restoral date */
	char *type;	/* type of restore object */
	ino_t inode;
	int send_mail;	/* send mail to user when done? */
	char *re_oname;	/* Rename of oname */
	char *re_odev;	/* Rename of odev */
	time_t	tmdate;	/* Current Turing Machine Date */
	char *tmstate;	/* Current Turing Machine State */
	int	tmstimulus;	/* Current Turing Machine Stimulus */
} rs_rqst_t;

/* Return VALUES from Restore Strategy */
#define	RS_TARCHIVE	1	/* table of contents volumes */
#define	RS_DARCHIVE	2	/* data archive */
#define	RS_COMPLETE	3	/* Successfully done */

/* Values for flags in rs_entry_t */
#define	RS_NO_PRUNE	0x1

/* Structure containing information from rsstatus table entries */
typedef	struct rs_entry_s {
	int flags;
	unsigned char *jobid;
	unsigned char *type;
	unsigned char *object;
	ino_t inode;	/* Gotten from table of contents */
	time_t fdate;
	unsigned char *target;
	unsigned char *refsname;
	unsigned char *redev;
	uid_t muid;
	uid_t uid;
	unsigned char *method;
	unsigned char *moption;
	unsigned char *dgroup;
	unsigned char *dlabel;
	unsigned char *tlabel;
	unsigned char *dchar;
	unsigned char *tmstate;
	time_t tmdate;
	unsigned char *tmoname;
	unsigned char *tmodev;
	int	tmstimulus;
	int tmsucceeded;
	unsigned char *status;
	unsigned char *explanation;
	struct rs_entry_s *next;
} rs_entry_t;

/* Find oldest history entry with H_DATE >= date */
#define	rstm_dfloor( tid, date ) rstm_dfind( tid, date, TRUE )

/* Find newest history entry with H_DATE <= date */
#define	rstm_dceiling( tid, date ) rstm_dfind( tid, date, FALSE )

/* Is this entry a candidate? */
#define	IS_CANDIDATE( tid, entry, rqst, mtype ) \
	( \
		rstm_consistent( rqst, \
			(char *)TLgetfield( tid, entry, H_ONAME ), \
			(char *)TLgetfield( tid, entry, H_ODEVICE ) ) \
		&& (mtype = rss_mfind( \
			(char *)TLgetfield( tid, entry, H_METHOD ), \
			rqst->type ) ) \
		&& rstm_varch( (char *)TLgetfield( tid, entry, H_DMNAME ) ) )

