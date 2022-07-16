/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)scsi:cmd/scl.h	1.3"

/*  Additional disk specific group zero op codes */

#define SS_FORMAT	0x04	/* Format Unit		*/

/* Additional op codes for group 1 commands */

#define SM_VERIFY	0x2F	/* Verify		*/

/*  Defect List Formats */

#define DLF_BLOCK	0x0	/* Block		*/
#define DLF_BYTES	0x4	/* Bytes from Index	*/
#define DLF_PHYSICAL	0x5	/* Physical Sector	*/

/*  Mode Page Codes */

#ifdef i386
#define	PC_TYPE(x)	(((x)->ss_addr) & 0x3F)
#else
#define	PC_TYPE(x)	((((x)->ss_addr) >> 5) & 0x3F)
#endif /* i386 */

#define	PC_ER		0x01	/* Error Recovery Parameters Page	*/
#define	PC_DRC		0x02	/* Disconnect/Reconnect Control Parameters Page	*/
#define	PC_DADF		0x03	/* Direct Access Device Format Page	*/
#define	PC_RDDG		0x04	/* Rigid Disk Drive Geometry Page	*/

/*  Because some of the bit fields would have overlapped 32 bit
 *  boundaries, they have been split into two parts.  An upper part and
 *  a lower part.  The following macros are used to read and write those
 *  fields.  The first argument is should be the address of the
 *  structure and the second argument is the value to read from or write
 *  to.
 */

#define RD_PG_CYL(p)		(((p)->pg_cylu << 8) | (p)->pg_cyll)
#define WR_PG_CYL(p,v)		(p)->pg_cyll = (v & 0xFF);\
				(p)->pg_cylu = ((v >> 8) & 0xFFFF)
#define RD_PG_WRPCOMP(p)	(((p)->pg_wrpcompu << 8) | (p)->pg_wrpcompl)
#define WR_PG_WRPCOMP(p,v)	(p)->pg_wrpcompl = (v & 0xFF);\
				(p)->pg_wrpcompu = ((v >> 8) & 0xFFFF)
#define RD_PG_LAND(p)		(((p)->pg_landu << 8) | (p)->pg_landl)
#define WR_PG_LAND(p,v)		(p)->pg_landl = (v & 0xFF);\
				(p)->pg_landu = ((v >> 8) & 0xFFFF)

/*  Because of the use of bit fields and data padding the addresses and
 *  sizes that are passed to the host adapter driver must be adjusted.
 *  The following macros provide the size and address of the structure
 *  for use by SCB structures.  For address the macro should be passed
 *  the address of the structure
 */

#define FORMAT_SZ	6
#define FORMAT_AD(x)	((char *) x + 1)
#define DLH_SZ		4
#define BLOCK_SZ	4
#define BYTES_SZ	8
#define PHYSICAL_SZ	8
#define	SENSE_PLH_SZ	4
#define	ER_SZ		8
#define	DRC_SZ		12
#define	DADF_SZ		24
#define	RDDG_SZ		20
#define	CAPACITY_SZ	8

/*  This structure defines the Format CDB.  There is a 8 bit pad to
 *  align the interleave field.  Note the host adapter driver must be
 *  passed the address of the structure plus 1.
 */

typedef struct format {
	unsigned char ss_pad0;
	unsigned char ss_op;	/* Opcode		*/
	int ss_dlf  : 3;	/* Defect List Format	*/
	int ss_cl   : 1;	/* Complete List	*/
	int ss_fd   : 1;	/* Format Data		*/
	int ss_lun  : 3;	/* Logical Unit Number	*/
	unsigned char ss_vu1;	/* Vendor Unique	*/
	int ss_intl : 16;	/* Interleave Field	*/
	int ss_cont : 6;	/* Control Field	*/
	int ss_vu2  : 2;	/* Vendor Unique	*/
} FORMAT_T;

/*  This structure defines the Defect List Header format.  It is a
 *  combination of the Headers used for reading,  writing and
 *  re-assigning defects.
 */

typedef struct dlh {
	unsigned char dlh_res1	: 8;	/* Reserved			     */
	int dlh_dlf	: 3;	/* Defect List Format		     */
	int dlh_g	: 1;	/* Grown List Flag		     */
	int dlh_stpf_p	: 1;	/* Stop Format and Primary List Flag */
	int dlh_crtf	: 1;	/* Certification		     */
	int dlh_prmy	: 1;	/* Primary			     */
	int dlh_epcs	: 1;	/* Enable P, C, and S		     */
	int dlh_len	: 16;	/* Defect List Length		     */
} DLH_T;

/*  This structure defines the Defect List Block data format. */

typedef struct block {
	unsigned int dl_addr;		/* Defect Block Address	*/
} BLOCK_T;

/*  This structure defines the Defect List Bytes from Index data format. */

typedef struct bytes {
	unsigned int dl_cyl  : 24;	/* Cylinder Number	*/
	unsigned char dl_head;		/* Head Number		*/
	unsigned int dl_byte;		/* Bytes from Index	*/
} BYTES_T;

/*  This structure defines the Defect List Physical Sector data format. */

typedef struct physical {
	unsigned int dl_cyl  : 24;	/* Cylinder Number	*/
	unsigned char dl_head;		/* Head Number		*/
	unsigned int dl_sec;		/* Sector Number	*/
} PHYSICAL_T;

/*  This structure defines the Mode Sense Parameter List Header format.  */

typedef struct sense_plh {
	unsigned char plh_len;		/* Data Length			*/
	unsigned char plh_type;		/* Medium Type			*/
	unsigned int  plh_res : 7;	/* Reserved			*/
	unsigned int  plh_wp : 1;	/* Write Protect		*/
	unsigned char plh_bdl;		/* Block Descriptor Length	*/
} SENSE_PLH_T;

/*  This structure defines the Error Recovery Paramaters Page format.
 */

typedef struct er {
	int pg_pc	: 6;	/* Page Code			*/
	int pg_res1	: 2;	/* Reserved			*/
	unsigned char pg_len;	/* Page Length			*/
	int pg_dcr	: 1;	/* Disable Correction		*/
	int pg_dte	: 1;	/* Disable Transfer on Error	*/
	int pg_per	: 1;	/* Post Error			*/
	int pg_eec	: 1;	/* Enable Early Correction	*/
	int pg_rc	: 1;	/* Read Continuous		*/
	int pg_tb	: 1;	/* Transfer Block		*/
	int pg_arre	: 1;	/* Automatic Read Reallocation Enabled */
	int pg_awre	: 1;	/* Automatic Write Reallocation Enabled */
	unsigned char pg_retry;	/* Retry Count			*/
	unsigned char pg_span;		/* Correction Span		*/
	unsigned char pg_hd_off;	/* Head Offset Count		*/
	unsigned char pg_ds_off;	/* Data Strobe Offset Count	*/
	unsigned char pg_rectl;	/* Recovery Time Limit		*/
} ER_T;

/*  This structure defines the Disconnect/Reconnect Control Parameters
 *  Page format.
 */

typedef struct drc {
	int pg_pc	: 6;	/* Page Code			*/
	int pg_res1	: 2;	/* Reserved			*/
	unsigned char pg_len;		/* Page Length			*/
	unsigned char pg_buffr;	/* Buffer Full Ratio		*/
	unsigned char pg_bufer;	/* Buffer Empty Ratio		*/
	int pg_busil	: 16;	/* Bus Inactivity Limit		*/
	int pg_distl	: 16;	/* Disconnect Time Limit	*/
	int pg_contl	: 16;	/* Connect Time Limit		*/
	int pg_res2	: 16;	/* Reserved			*/
} DRC_T;

/*  This structure defines the Direct Access Device Format Parameter
 *  Page format.
 */

typedef struct dadf {
	int pg_pc	: 6;	/* Page Code			*/
	int pg_res1	: 2;	/* Reserved			*/
	unsigned char pg_len;	/* Page Length			*/
	int pg_trk_z	: 16;	/* Tracks per Zone		*/
	int pg_asec_z	: 16;	/* Alternate Sectors per Zone	*/
	int pg_atrk_z	: 16;	/* Alternate Tracks per Zone	*/
	int pg_atrk_v	: 16;	/* Alternate Tracks per Volume	*/
	int pg_sec_t	: 16;	/* Sectors per Track		*/
	int pg_bytes_s	: 16;	/* Bytes per Physical Sector	*/
	int pg_intl	: 16;	/* Interleave Field		*/
	int pg_trkskew	: 16;	/* Track Skew Factor		*/
	int pg_cylskew	: 16;	/* Cylinder Skew Factor		*/
	int pg_res2	: 27;	/* Reserved			*/
	int pg_ins	: 1;	/* Inhibit Save			*/
	int pg_surf	: 1;	/* Allocate Surface Sectors	*/
	int pg_rmb	: 1;	/* Removable			*/
	int pg_hsec	: 1;	/* Hard Sector Formatting	*/
	int pg_ssec	: 1;	/* Soft Sector Formatting	*/
} DADF_T;

/*  This structure defines the Rigid Disk Drive Geometry Parameter
 *  Page format.
 */

typedef struct rddg {
	int pg_pc	: 6;	/* Page Code			 */
	int pg_res1	: 2;	/* Reserved			 */
	unsigned char pg_len;		/* Page Length			 */
	int pg_cylu	: 16;	/* Number of Cylinders (Upper)	 */
	unsigned char pg_cyll;		/* Number of Cylinders (Lower)	 */
	unsigned char pg_head;		/* Number of Heads		 */
	int pg_wrpcompu	: 16;	/* Write Precompensation (Upper) */
	unsigned char pg_wrpcompl;	/* Write Precompensation (Lower) */
	int pg_redwrcur	: 24;	/* Reduced Write Current	 */
	int pg_drstep	: 16;	/* Drive Step Rate		 */
	int pg_landu	: 16;	/* Landing Zone Cylinder (Upper) */
	unsigned char pg_landl;	/* Landing Zone Cylinder (Lower) */
	int pg_res2	: 24;	/* Reserved			 */
} RDDG_T;

 /*  This structure defines the Read Capacity Data format. */

typedef struct capacity {
	int cd_addr;		/* Logical Block Address	*/
	int cd_len;		/* Block Length			*/
} CAPACITY_T;

extern char		Cmdname[];
extern char		Hostfile[];
extern char		*sys_errlist[];
extern int		Hostfdes;

#ifndef i386
extern pdi_dev_t	Hostdev;
#else
extern dev_t		Hostdev;
#endif /* i386 */

extern struct ident	Inquiry_data;
extern int		Timer;
extern int		blocksort();
extern int		bytessort();
extern void		format();
extern void		get_defect();
extern void		inquiry();
extern void		mdselect();
extern void		mdsense();
extern int		physicalsort();
extern void		put_defect();
extern void		readcap();
extern void		readdefects();
extern void		reassign();
extern void		req_sense();
extern int		scsi_open();
extern int		scsi_read();
extern void		scsi_verify();
extern void		scsi_write();
extern int		send_scb();
extern void		warning();
