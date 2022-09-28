/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Portions Copyright (c) 1988, Sun Microsystems, Inc.	*/
/*	All Rights Reserved.					*/

#ident	"@(#)cpio:cpio.h	1.3.7.1"

/* Option Character keys (OC#), where '#' is the option character specified. */

#define	OCa	0x1
#define	OCb	0x2
#define	OCc	0x4
#define	OCd	0x8
#define	OCf	0x10
#define	OCi	0x20
#define	OCk	0x40
#define	OCl	0x80
#define	OCm	0x100
#define	OCo	0x200
#define	OCp	0x400
#define	OCr	0x800
#define	OCs	0x1000
#define	OCt	0x2000
#define	OCu	0x4000
#define	OCv	0x8000
#define	OCA	0x10000
#define	OCB	0x20000
#define	OCC	0x40000
#define	OCE	0x80000
#define	OCH	0x100000
#define	OCI	0x200000
#define	OCL	0x400000
#define	OCM	0x800000
#define	OCO	0x1000000
#define	OCR	0x2000000
#define	OCS	0x4000000
#define	OCV	0x8000000
#define	OC6	0x10000000
#define	BSM	0x20000000

/* Invalid option masks for each action option (-i, -o or -p). */

#define INV_MSK4i	( OCo | OCp | OCA | OCL | OCO )

#define INV_MSK4o	( OCi | OCp | OCE | OCI | OCR )

#define INV_MSK4p	( OCf | OCi | OCo | OCr | OCt | OCA \
			| OCE | OCH | OCI | OCO )

/* Header types */

#define NONE	0	/* No header value verified */
#define	BIN	1	/* Binary */
#define	CHR	2	/* ASCII character (-c) */
#define	ASC	3	/* ASCII with expanded maj/min numbers */
#define	CRC	4	/* CRC with expanded maj/min numbers */
#define	TARTYP	5	/* Tar or USTAR */
#define	SEC	6	/* Secure system */

/* Differentiate between TAR and USTAR */

#define TAR	7	/* Regular tar */
#define	USTAR	8	/* IEEE data interchange standard */

/* the pathname lengths for the USTAR header */

#define MAXNAM	256	/* The maximum pathname length */
#define NAMSIZ	100	/* The maximum length of the name field */
#define PRESIZ	155	/* The maximum length of the prefix */

/* HDRSZ: header size minus filename field length */

#define HDRSZ (Hdr.h_name - (char *)&Hdr)

/*
 * IDENT: Determine if two stat() structures represent identical files.
 * Assumes that if the device and inode are the same the files are
 * identical (prevents the archive file from appearing in the archive).
 */

#define IDENT(a, b) ((a.st_ino == b.st_ino && a.st_dev == b.st_dev) ? 1 : 0)

/*
 * FLUSH: Determine if enough space remains in the buffer to hold
 * cnt bytes, if not, call bflush() to flush the buffer to the archive.
 */

#define FLUSH(cnt) if ((Buffr.b_end_p - Buffr.b_in_p) < cnt) bflush()

/*
 * FILL: Determine if enough bytes remain in the buffer to meet current needs,
 * if not, call rstbuf() to reset and refill the buffer from the archive.
 */

#define FILL(cnt) while (Buffr.b_cnt < cnt) rstbuf()

/*
 * VERBOSE: If x is non-zero, call verbose().
 */

#define VERBOSE(x, name) if (x) verbose(name)

/*
 * FORMAT: Date time formats
 * b - abbreviated month name
 * e - day of month (1 - 31)
 * H - hour (00 - 23)
 * M - minute (00 - 59)
 * Y - year as ccyy
 */

#define	FORMAT	"%b %e %H:%M %Y"

#define INPUT	0	/* -i mode (used for chgreel() */
#define OUTPUT	1	/* -o mode (used for chgreel() */
#define APATH	1024	/* maximum ASC or CRC header path length */
#define CPATH	256	/* maximum -c and binary path length */
#define BUFSZ	512	/* default buffer size for archive I/O */
#define CPIOBSZ	4096	/* buffer size for file system I/O */
#define LNK_INC	500	/* link allocation increment */
#define MX_BUFS	10	/* max. number of buffers to allocate */

#define F_SKIP	0	/* an object did not match the patterns */
#define F_LINK	1	/* linked file */
#define F_EXTR	2	/* extract non-linked object that matched patterns */

#define MX_SEEKS	10	/* max. number of lseek attempts after error */
#define	SEEK_ABS	0	/* lseek absolute */
#define SEEK_REL	1	/* lseek relative */

/*
 * xxx_CNT represents the number of sscanf items that will be matched
 * if the sscanf to read a header is successful.  If sscanf returns a number
 * that is not equal to this, an error occured (which indicates that this
 * is not a valid header of the type assumed.
 */

#define ASC_CNT	14	/* ASC and CRC headers */
#define CHR_CNT	11	/* CHR header */

/* These defines determine the severity of the message sent to the user. */

#define ERR	1	/* Error message (warning) - not fatal */
#define EXT	2	/* Error message - fatal, causes exit */
#define ERRN	3	/* Error message with errno (warning) - not fatal */
#define EXTN	4	/* Error message with errno - fatal, causes exit */
#define POST	5	/* Information message, not an error */
#define EPOST	6	/* Information message to stderr */

#define SIXTH	060000	/* UNIX 6th edition files */

#define	P_SKIP	0	/* File should be skipped */
#define P_PROC	1	/* File should be processed */

#define U_KEEP	0	/* Keep the existing version of a file (-u) */
#define U_OVER	1	/* Overwrite the existing version of a file (-u) */

/*
 * _20K: Allocate the maximum of (20K or (MX_BUFS * Bufsize)) bytes 
 * for the main I/O buffer.  Therefore if a user specifies a small buffer
 * size, they still get decent performance due to the buffering strategy.
 */

#define _20K	20480	

#define HALFWD	1	/* Pad headers/data to halfword boundaries */
#define FULLWD	3	/* Pad headers/data to word boundaries */
#define FULLBK	511	/* Pad headers/data to 512 byte boundaries */
