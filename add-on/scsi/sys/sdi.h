/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)scsi:sys/sdi.h	1.3"

/*=================================================================*/
/* sdi.h
/*=================================================================*/

#define	SDI_EXLUN	0x80		/* Indicates extended logical unit # */

#define	SCB_TYPE	1
#define	ISCB_TYPE	2

		/* mode field */
#define	SCB_WRITE	0x00		/* Non-read job                       */
#define	SCB_READ	0x01		/* Read data job                      */
#define	SCB_LINK	0x02		/* SCSI command linking is used       */
#define	SCB_HAAD	0x04		/* Address supplied by HA             */
#define SCB_PARTBLK	0x08		/* Partial block transfer	      */

		/* completion code field */
#define	SDI_NOALLOC	0x00000000	/* This block is not allocated      */
#define	SDI_ASW		0x00000001	/* Job completed normally           */
#define	SDI_LINKF0	0x00000002	/* Linked command done without flag */
#define	SDI_LINKF1	0x00000003	/* Linked command done with flag    */
#define	SDI_QFLUSH	0xE0000004	/* Job was flushed                  */
#define	SDI_ABORT	0xF0000005	/* Command was aborted              */
#define	SDI_RESET	0xF0000006	/* Reset was detected on the bus    */
#define	SDI_CRESET	0xD0000007	/* Reset was caused by this unit    */
#define	SDI_V2PERR	0xA0000008	/* vtop failed                      */
#define	SDI_TIME	0xD0000009	/* Job timed out                    */
#define	SDI_NOTEQ	0x8000000A	/* Addressed device not present     */
#define	SDI_HAERR	0xE000000B	/* Host adapter error               */
#define	SDI_MEMERR	0xA000000C	/* Memory fault                     */
#define	SDI_SBUSER	0xA000000D	/* SCSI bus error                   */
#define	SDI_CKSTAT	0xD000000E	/* Check the status byte  	    */
#define	SDI_SCBERR	0x8000000F	/* SCB error                        */
#define	SDI_OOS		0xA0000010	/* Device is out of service         */
#define	SDI_NOSELE	0x90000011	/* The SCSI bus select failed       */
#define	SDI_MISMAT	0x90000012	/* parameter mismatch               */
#define	SDI_PROGRES	0x00000013	/* Job in progress                  */
#define	SDI_UNUSED	0x00000014	/* Job not in use                   */
#define	SDI_ONEIC	0x80000017	/* More than one immediate request  */
#define SDI_SFBERR	0x80000019	/* SFB error			    */
#define SDI_TCERR	0x9000001A	/* Target protocol error detected   */

#define	SDI_ERROR	0x80000000	/* An error was detected         */
#define	SDI_RETRY	0x40000000	/* Retry the job                 */
#define	SDI_MESS	0x20000000	/* A message has been sent       */
#define	SDI_SUSPEND	0x10000000	/* Processing has been suspended */

#define	SFB_TYPE	3

	/* Defines for the command field */
#define	SFB_NOPF	0x00		/* No op function                  */
#define	SFB_RESETM	0x01		/* Send a bus device reset message */
#define	SFB_ABORTM	0x02		/* Send an abort message           */
#define	SFB_FLUSHR	0x03		/* Flush queue request             */
#define SFB_RESUME	0x04		/* Resume the normal job queue	   */
#define SFB_SUSPEND	0x05		/* Suspend the normal job queue    */

#define	SDI_3B20	0x01
#define	SDI_3B15	0x02
#define	SDI_3B5		0x03
#define	SDI_3B2		0x04
#define	SDI_APACHE_FS	0x05
#define	SDI_386_AT	0x06

#define SDI_BASIC1              0x0001
#define SDI_FLT_HANDLING        0x0002

	/* Return values for sdi_send and sdi_icmd */
#define SDI_RET_OK 	0
#define SDI_RET_ERR 	-1
#define SDI_RET_RETRY	1

	/* Ioctl command codes */
#define SDI_SEND	0X0081		/* Send a SCSI command		*/
#define SDI_TRESET	0X0082		/* Reset a target controller	*/
#define SDI_BRESET	0X0084		/* Reset the SCSI bus		*/
#define HA_VER		0X0083		/* Get the host adapter version */
#define SDI_RESERVE     0X0085          /* Reserve the device           */
#define SDI_RELEASE     0X0086          /* Release the device           */
#define SDI_RESTAT      0X0087          /* Device Reservation Status    */

        /* Fault handler flags */
#define SDI_FLT_RESET   0x00000001      /* logical unit was reset       */
#define SDI_FLT_PTHRU   0x00000002      /* pass through was used        */

struct scsi_ad{
	long		sa_major;	/* Major number                 */
	long		sa_minor;	/* Minor number                 */
	unsigned char	sa_lun;		/* logical unit number          */
	unsigned char	sa_exlun;	/* extended logical unit number */
	short		sa_fill;	/* Fill word                    */
};

struct scb{
	unsigned long	sc_comp_code;	/* Current job status              */
	void		(*sc_int)();	/* Target Driver interrupt handler */
	caddr_t		sc_cmdpt;	/* Target command                  */
	caddr_t		sc_datapt;	/* Data area			   */
	long		sc_wd;		/* Target driver word              */
	time_t		sc_time;	/* Time limit for job              */
	struct scsi_ad	sc_dev;		/* SCSI device address             */
	unsigned short	sc_mode;	/* Mode flags for current job      */
	unsigned char	sc_status;	/* Target status byte              */
	char		sc_fill;	/* Fill byte                       */
	struct sb	*sc_link;	/* Link to next scb command        */
	long		sc_cmdsz;	/* Size of command                 */
	long		sc_datasz;	/* Size of data			   */
	long		sc_resid;	/* Bytes to xfer after data 	   */
};

struct sfb{
	unsigned long	sf_comp_code;	/* Current job status              */
	void		(*sf_int)();	/* Target Driver interrupt handler */
	struct scsi_ad	sf_dev;		/* SCSI device address             */
	unsigned long	sf_func;	/* Function to be performed        */
	int		sf_wd;		/* Target driver word		   */    
};

struct ver_no {
	unsigned char	sv_release;	/* The release number */
	unsigned char	sv_machine;	/* The running machine */
	short		sv_modes;	/* Supported modes     */
};

struct sb {
	unsigned long	sb_type;
	union{
		struct scb	b_scb;
		struct sfb	b_sfb;
	}sb_b;
};

#define SCB sb_b.b_scb
#define SFB sb_b.b_sfb

extern long		sdi_started;

extern struct ver_no	sdi_ver;

extern struct sb	*sdi_getblk ();
