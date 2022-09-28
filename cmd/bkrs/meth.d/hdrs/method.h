/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:meth.d/hdrs/method.h	1.15.2.1"

#define	Aflag	0x1		/* automated operation */
#define dflag	0x2		/* do not put in backup history log */
#define lflag	0x4		/* long history log */
#define mflag	0x8		/* mount orig fs read only */
#define oflag	0x10		/* permit media label override */
#define rflag	0x20		/* include remote files */
#define tflag	0x40		/* create table of contents media */
#define vflag	0x80		/* validate archive as it is written */
#define Eflag	0x100		/* estimate media usage, proceed */
#define Nflag	0x200		/* estimate media usage, exit */
#define Sflag	0x400		/* print a dot per 100 512 byte recs */
#define Vflag	0x800		/* generate names of files */
#define iflag	0x1000		/* exclude files with inode change */
#define xflag	0x2000		/* ignore exception list */
#define cflag	0x4000		/* block count for data partition */
#define nflag	0x8000		/* unmount before image backup */
#define sflag	0x10000		/* do not save toc online */
#define pflag	0x20000		/* number of days prior for backups */
#define eflag	0x40000		/* exception list file */
#define qflag	0x80000		/* quick method - use g_copy for backups */

#define REST_IGNORE ( qflag | eflag | pflag | dflag | iflag | lflag | mflag | rflag | tflag | nflag | vflag | Eflag | Nflag | sflag )	/* restore ignores */

#define IS_BACKUP	0x1	/* -B was specified */
#define IS_RFILE	0x2	/* -RF was specified */
#define IS_RCOMP	0x4	/* -RC was specified */
#define IS_RESTORE	0x8	/* -R was specified */
#define IS_TOC		0x10	/* -T was specified */
#define IS_BTOC		0x11	/* -BT specified */
#define IS_RTOC		0x18	/* -RT specified */

#define IS_BOTH 	(IS_BACKUP | IS_RESTORE)
#define IS_FC		(IS_RCOMP | IS_RFILE)
#define IS_B(x)		(x & IS_BACKUP)
#define IS_R(x)		(x & IS_RESTORE)


/* the following must match the order of method determination
 * in bkopt.c
 */

#define	IS_FULL		0	/* is full filesystem request */
#define IS_INC		1	/* is incremental 	      */
#define IS_FDP		2	/* is full data partition     */
#define IS_FDISK	3	/* is full disk               */
#define IS_IMAGE	4	/* is filesystem image        */
#define IS_MIGRATION	5	/* is migration request       */

typedef struct method_info {
	char	*method_name;	/* invocation command name */
	char	*jobid;		/* backup or restore job id */
	char	*ofsname;	/* originating file system name */
	char	*ofsdev;	/* originating file system device */
	char	*ofslab;	/* originating file system label */
	char	*dgroup;	/* dest device dgroup description */
	char	*dname;		/* dest device dname description */
	char	*dchar;		/* dest device dchar description */
	char	*dmnames;	/* dest device dmnames description */
	unsigned  flags;	/* set from command options */
	short	noptarg;	/* number of cmd args */
	ushort	br_type;	/* backup or restore */
	int	c_count;	/* block count for fdp backup */
	int	n_names;	/* num file names if -RF */
	int	fnames;		/* offset in argv to first name */
	ushort	ofs_loc;	/* is ofs local or remote */
	char	*ofs_mntopts;	/* ofs mount options */
	short	mntinfo;	/* what we should do regarding mounts */
	short	meth_type;	/* index of type of method in use */
	char	**ex_tab;	/* exception list pointers */
	int	ex_count;	/* number of strings in exception list */
	char	*err;		/* error string for libbrmeth */
	long	blk_count;	/* blocks written */
	char	*nfsname;	/* new fs name for -RC */
	char	*nfsdev;	/* new fs dev for -RC */
	char	*nfsdevmnt;	/* if fs on nfsdev in mnttab, its name */
	char	*ofsdevmnt;	/* if fs on ofsdev in mnttab, its name */
	long	blks_per_vol;	/* 512 byte blocks per vol */
	long	lastfull;	/* date of last ffile -B for fs */
	int 	dtype;		/* device type for archive (see DTYPE) */
	long	estimate;	/* estimate of archive size (512 byte blks) */
	char	*volpromt;	/* what rmvbl media is called */
	time_t	bkdate;		/* one date for the archive */
	int	(*sndfp) ();	/* point to fname send function */
	char	*tocfname;	/* -BT toc name to save */
	char	*fstype;	/* fstype string */
} m_info_t;

/*
 *	values for ofs_loc
 */

#define IS_LOCAL	1	/* on local machine */
#define IS_REMOTE	2	/* on another machine */

/*
 *	values for mntinfo
 */

#define DONOTHING	0	/* we don't need to do any mounting */
#define DOUNMOUNT	1	/* umount ofsname from ofsdev */
#define DOREMOUNT	2	/* remount ofsname on ofsdev needed */

#define MN(x) 		x->method_name	/* macro for method name */
#define MNL(x)		((x->method_name) ? (strlen(x->method_name) + 1) : 0)
#define ME(x)		x->err		/* macro for method err */
#define NMEDIA(x)	((x->blk_count+(x->blks_per_vol-1)) / x->blks_per_vol)
#define OFS(x)		x->ofsname	/* macro for ofsname     */
#define FSTYPE(x)	x->fstype	/* macro for fstype     */
#define FSL(x)		((x->ofsname) ? (strlen(x->ofsname) + 1) : 0)
#define ODEV(x)		x->ofsdev	/* macro for ofsdev     */
#define FSDL(x)		((x->ofsdev) ? (strlen(x->ofsdev) + 1) : 0)
#define ST(x,y)		x->y ? x->y : "NULL"	/* string or NULL */
#define RCHAR(x)	((x->br_type & IS_RFILE) ? 'F' : 'C')
#define ISFILE_REST(x)	(x->br_type & IS_RFILE)

extern char	*sys_errlist[];
extern int	sys_nerr;
static char	*unk_sys_err = "errno out of range";
#define SE	(((errno>0)&&(errno<sys_nerr))?sys_errlist[errno]:unk_sys_err)

#define SAVTOC(x)	(((x->flags) & tflag) || (!((x->flags) & nflag)))

typedef struct rs {
	char	*name;			/* name on archive */
	char	*rename;		/* new name or NULL */
	char	*uid;			/* uid of requestor */
	char	*date;			/* date user requested */
	char	*jobid;			/* restore job id */
	char	*inode;			/* inode number */
	char	*rest_name;		/* full restore path name */
	char	*mall_name;		/* malloced restore path name */
	struct rs *rest_next;		/* next in chain for this file */
	struct rs *rest_prev;		/* previous in chain for this file */
	int	rest_fd;		/* open fd for restore */
	int	status;			/* per file status */
	int	rindx;			/* result msg index */
	int	name_len;		/* length of name */
	int	rename_len;		/* length of rename */
	int	ino;			/* atoi of inode above */
	long	ldate;			/* atol of date above */
	long	offset;			/* current offset for restore */
	char	*result_msg;		/* formatted result msg */
	short	type;			/* 1=directory , 0=file */
	short	file_count;		/* for dir, files restored */
	ushort	idnum;			/* numeric uid */
} file_rest_t;

#define RM(x)	(x->result_msg)
#define RI(x)	(x->rindx)
#define RFC(x)	(x->file_count)

#define F_NOTFOUND	0		/* file not processed yet */
#define F_SUCCESS	1		/* found and restored */
#define F_UNSUCCESS	2		/* found, not restored */

#define PAD_VAL		3	/* pad value to word align ASCII cpio header */

/* cpio header */

typedef
struct cpio_hdr {
	ulong	h_magic,	/* Magic number field */
		h_ino,		/* Inode number of file */
		h_mode,		/* Mode of file */
		h_uid,		/* Uid of file */
		h_gid,		/* Gid of file */
		h_nlink,	/* Number of links */
		h_mtime;	/* Modification time */
	long	h_filesize;	/* Length of file */
	ulong	h_dev,		/* File system of file */
		h_rdev,		/* Major/minor numbers of special files */
		h_namesize,	/* Length of filename */
		h_cksum;	/* Checksum of file */
	char	h_name[PATH_MAX+2];	/* Filename */
} Hdr_t;

typedef
struct cpio_hdr1 {
	ulong	h_magic,	/* Magic number field */
		h_ino,		/* Inode number of file */
		h_mode,		/* Mode of file */
		h_uid,		/* Uid of file */
		h_gid,		/* Gid of file */
		h_nlink,	/* Number of links */
		h_mtime;	/* Modification time */
	long	h_filesize;	/* Length of file */
	ulong	h_dev,		/* File system of file */
		h_rdev,		/* Major/minor numbers of special files */
		h_namesize,	/* Length of filename */
		h_cksum;	/* Checksum of file */
	char	h_name[4];	/* Filename */
} HHdr_t;

typedef struct ml{
	struct ml 	*next;		/* next label used */
	char		*label;		/* the label */
	short		type;		/* archive or toc */
} media_list_t;

typedef struct {
	long		bytes_left;	/* bytes current vol will still hold */
	media_list_t 	*first;		/* list of labels used */
	media_list_t 	*last;		/* last of list of labels used */
	media_list_t 	*cur;		/* current label, not on list yet */
} media_info_t;

struct dirs_made {
	struct dirs_made *next;    /* next directory created during restore*/
	struct dirs_made *prev;    /* prev directory created during restore*/
	char dirname[1];	   /* name of directory followed by null */
};

#define PATHSIZE	2048		/* maximum PATH length */

#ifdef ALONE

#define brlog		printf

#endif

#define BRMAGIC		0x579ee957

#define DTYPES		char *dtypes[] = {"dpart","file","dir"}
#define IS_DPART	0
#define IS_FILE		1
#define IS_DIR		2
#define NDEV		(int) (sizeof(dtypes) / sizeof(char *))

#define R_ULIMIT	0
#define R_NAMLEN	1
#define R_NOUID		2
#define R_NOTFOUND	3
#define R_SUCCESS	4
#define R_INCOMP	5
#define R_DATE		6
#define R_EXIST		7
#define R_NOTOWN	8
#define R_NOWRITE_PERM	9
#define R_NODIR		10

#define OVRIDE(x)	((int) ((x->flags & oflag) ? 1 : 0))
#define AUTOM(x)	((int) ((x->flags & Aflag) ? 1 : 0))
#define FNAMES(x)	(x->flags & Vflag)
#define DOTS(x)		(x->flags & Sflag)
#define LTOC(x)		(x->flags & lflag)

#define IMAGE_BKRS	"image_part"
