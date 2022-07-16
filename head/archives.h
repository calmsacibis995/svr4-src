/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Portions Copyright (c) 1988, Sun Microsystems, Inc.	*/
/*	All Rights Reserved.					*/

#ident	"@(#)head.usr:archives.h	1.7.2.1"

#include <tar.h>

/* Magic numbers */

#define CMN_ASC	0x070701	/* Cpio Magic Number for ASCii header */
#define CMN_BIN	070707		/* Cpio Magic Number for Binary header */
#define CMN_BBS	0143561		/* Cpio Magic Number for Byte-Swap header */
#define CMN_CRC	0x070702	/* Cpio Magic Number for CRC header */
#define CMS_ASC	"070701"	/* Cpio Magic String for ASCii header */
#define CMS_CHR	"070707"	/* Cpio Magic String for CHR (-c) header */
#define CMS_CRC	"070702"	/* Cpio Magic String for CRC header */
#define CMS_LEN	6		/* Cpio Magic String LENgth */

/* Various header and field lengths */

#define CHRSZ	76		/* -c hdr size minus filename field */
#define ASCSZ	110		/* ASC and CRC hdr size minus filename field */
#define TARSZ	512		/* TAR hdr size */

#define HNAMLEN	256	/* maximum filename length for binary and -c headers */
#define EXPNLEN	1024	/* maximum filename length for ASC and CRC headers */
#define HTIMLEN	2	/* length of modification time field */
#define HSIZLEN	2	/* length of file size field */

/* cpio binary header definition */

struct hdr_cpio {
	short	h_magic,		/* magic number field */
		h_dev;			/* file system of file */
	ushort	h_ino,			/* inode of file */
		h_mode,			/* modes of file */
		h_uid,			/* uid of file */
		h_gid;			/* gid of file */
	short	h_nlink,		/* number of links to file */
		h_rdev,			/* maj/min numbers for special files */
		h_mtime[HTIMLEN],	/* modification time of file */
		h_namesize,		/* length of filename */
		h_filesize[HSIZLEN];	/* size of file */
	char	h_name[HNAMLEN];	/* filename */
} ;

/* cpio ODC header format */

struct c_hdr {
	char	c_magic[CMS_LEN],
		c_dev[6],
		c_ino[6],
		c_mode[6],
		c_uid[6],
		c_gid[6],
		c_nlink[6],
		c_rdev[6],
		c_mtime[11],
		c_namesz[6],
		c_filesz[11],
		c_name[HNAMLEN];
} ;

/* -c and CRC header format */

struct Exp_cpio_hdr {
	char	E_magic[CMS_LEN],
		E_ino[8],
		E_mode[8],
		E_uid[8],
		E_gid[8],
		E_nlink[8],
		E_mtime[8],
		E_filesize[8],
		E_maj[8],
		E_min[8],
		E_rmaj[8],
		E_rmin[8],
		E_namesize[8],
		E_chksum[8],
		E_name[EXPNLEN];
} ;

/* Tar header structure and format */

#define TBLOCK	512	/* length of tar header and data blocks */
#define	TNAMLEN	100	/* maximum length for tar file names */
#define TMODLEN	8	/* length of mode field */
#define TUIDLEN	8	/* length of uid field */
#define TGIDLEN	8	/* length of gid field */
#define TSIZLEN	12	/* length of size field */
#define TTIMLEN	12	/* length of modification time field */
#define TCRCLEN	8	/* length of header checksum field */

/* tar header definition */

union tblock {
	char dummy[TBLOCK];
	struct tar_hdr {
		char	t_name[TNAMLEN],	/* name of file */
			t_mode[TMODLEN],	/* mode of file */
			t_uid[TUIDLEN],		/* uid of file */
			t_gid[TGIDLEN],		/* gid of file */
			t_size[TSIZLEN],	/* size of file in bytes */
			t_mtime[TTIMLEN],	/* modification time of file */
			t_cksum[TCRCLEN],	/* checksum of header */
			t_typeflag,
			t_linkname[TNAMLEN],	/* file this file linked with */
			t_magic[TMAGLEN],
			t_version[TVERSLEN],
			t_uname[32],
			t_gname[32],
			t_devmajor[8],
			t_devminor[8],
			t_prefix[155];
	} tbuf;
} ;

/* volcopy tape label format and structure */

#define VMAGLEN 8
#define VVOLLEN 6
#define VFILLEN 464

struct volcopy_label {
	char	v_magic[VMAGLEN],
		v_volume[VVOLLEN],
		v_reels,
		v_reel;
	long	v_time,
                v_length,
		v_dens,
		v_reelblks,	/* u370 added field */
		v_blksize,	/* u370 added field */
		v_nblocks;	/* u370 added field */
	char	v_fill[VFILLEN];
	long	v_offset;	/* used with -e and -reel options */
	int	v_type;		/* does tape have nblocks field? */
} ;
