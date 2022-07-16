/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)scsi:cmd/scl.c	1.3"

/*  SCSI Command Library - This file contains a library of SCSI
 *  commands that the user utilities can use. The SCSI commands
 *  are created for the user and then sent to the Target Controller
 *  via pass-through.
 */

#include	"stdio.h"
#include	"errno.h"
#include	"fcntl.h"
#include	"sys/vtoc.h"		/* Included just to satisfy scsicomm.h */
#include	"scsicomm.h"
#include	"sys/mkdev.h"
#include	"sys/stat.h"

#if defined (i386) || defined (i486)
#include	"sys/sdi_edt.h"
#else
#include	"sys/pdi.h"
#include	"sys/xedtbus.h"
#endif  /* ix86 */

#include	"sys/scsi.h"
#include	"sys/sdi.h"
#include	"scl.h"
#include	"tokens.h"
#include	"string.h"
#include 	"signal.h"		/* used by format timer */


#define MARBLK	2
#define	TRUE	1
#define	FALSE	0
#define	MASK_FF	0xFF
#define	MASK_1E	0x1E
#define NORMEXIT 0

char		Cmdname[64];
char		Hostfile[128];	/* Host adapter pass thru file name */
extern int	Show;
int		Hostfdes;

#if defined (i386) || defined (i486)
dev_t		Hostdev=0;
#else
pdi_dev_t	Hostdev;
#endif  /* ix86 */


struct ident	Inquiry_data;
int 		Timer = 0;
int		Timerpid,		/* used by format timer */
		Stoptimer;
extern char	*malloc(),
		*realloc();
void		error(),
		warning();
extern void	qsort();

void
req_sense(sense_data)
struct sense	*sense_data;
{
	struct sb	req_sense_scb;
	struct scs	req_sense_cdb;

	if (Show)
		put_string(stderr, "Request Sense");

	/* Fill in the Request Sense CDB */
	req_sense_cdb.ss_op = SS_REQSEN;
	req_sense_cdb.ss_lun = LUN(Hostdev);
	req_sense_cdb.ss_addr1 = 0;
	req_sense_cdb.ss_addr  = 0;
	req_sense_cdb.ss_len = SENSE_SZ;
	req_sense_cdb.ss_cont = 0;

	/* Fill in the Request Sense SCB */
	req_sense_scb.sb_type = ISCB_TYPE;
	req_sense_scb.SCB.sc_comp_code = SDI_PROGRES;
	req_sense_scb.SCB.sc_int = NULL;
	req_sense_scb.SCB.sc_cmdpt = SCS_AD(&req_sense_cdb);
	req_sense_scb.SCB.sc_datapt = SENSE_AD(sense_data);
	req_sense_scb.SCB.sc_wd = 0;
	req_sense_scb.SCB.sc_time = 60 * 1000;	/* 1 Minute */
	req_sense_scb.SCB.sc_dev.sa_major = 0;
	req_sense_scb.SCB.sc_dev.sa_minor = 0;
	req_sense_scb.SCB.sc_dev.sa_lun = 0;
	req_sense_scb.SCB.sc_dev.sa_exlun = 0;
	req_sense_scb.SCB.sc_mode = SCB_READ;
	req_sense_scb.SCB.sc_status = 0;
	req_sense_scb.SCB.sc_link = (struct sb *) NULL;
	req_sense_scb.SCB.sc_cmdsz = SCS_SZ;
	req_sense_scb.SCB.sc_datasz = SENSE_SZ;
	req_sense_scb.SCB.sc_resid = 0;

	if (Show) {
		put_string(stderr, "SCB sent to Host Adapter");
		put_data(stderr, (char *) &req_sense_scb, sizeof(struct sb));
		put_string(stderr, "CDB sent to Host Adapter");
		put_data(stderr, req_sense_scb.SCB.sc_cmdpt, req_sense_scb.SCB.sc_cmdsz);
	}
	/* Send the Request Sense SCSI Control Block to the Host Adapter */
	if (ioctl(Hostfdes, SDI_SEND, &req_sense_scb) < 0)
		error("Request Sense ioctl failed\n");

	/* Check Completion Code of the job */
	if (req_sense_scb.SCB.sc_comp_code != SDI_ASW)
		error("Request Sense SDI_SEND failed (0x%X)\n", req_sense_scb.SCB.sc_comp_code);

	if (Show) {
		put_string(stderr, "Data received from Host Adapter");
		put_data(stderr, req_sense_scb.SCB.sc_datapt, req_sense_scb.SCB.sc_datasz);
	}
}	/* req_sense() */

send_scb(scb, sense_data)
struct sb	*scb;
struct sense	*sense_data;
{
	/* Complete the SCSI Control Block */
	scb->sb_type = ISCB_TYPE;
	scb->SCB.sc_comp_code = SDI_PROGRES;
	scb->SCB.sc_int = NULL;
	scb->SCB.sc_wd = 0;
	scb->SCB.sc_dev.sa_major = 0;
	scb->SCB.sc_dev.sa_minor = 0;
	scb->SCB.sc_dev.sa_lun = 0;
	scb->SCB.sc_dev.sa_exlun = 0;
	scb->SCB.sc_status = 0;
	scb->SCB.sc_link = (struct sb *) NULL;
	scb->SCB.sc_resid = 0;

	if (Show) {
		put_string(stderr, "SCB sent to Host Adapter");
		put_data(stderr, (char *) scb, sizeof(struct sb));
		put_string(stderr, "CDB sent to Host Adapter");
		put_data(stderr, scb->SCB.sc_cmdpt, scb->SCB.sc_cmdsz);
		if ((scb->SCB.sc_datasz > 0) && (~scb->SCB.sc_mode & SCB_READ)) {
			put_string(stderr, "Data sent to Host Adapter");
			put_data(stderr, scb->SCB.sc_datapt, scb->SCB.sc_datasz);
		}
	}

	/* Send the SCSI Control Block to the Host Adapter */
	if (ioctl(Hostfdes, SDI_SEND, scb) < 0)
		error("Send SCB Ioctl failed\n");

	/* Check Completion Code of the job */
	switch (scb->SCB.sc_comp_code & MASK_FF) {
	case (SDI_ASW & MASK_FF) :	/* Job completed normally           */
		break;
	case (SDI_CKSTAT & MASK_FF) :	/* Target returned check status     */

		if (Show)
			(void) fprintf(stderr, "Status: 0x%X\n", scb->SCB.sc_status);
		switch (scb->SCB.sc_status & MASK_1E) {
		case S_GOOD :	/* Job completed normally  */
			error("Good Status?\n");
		case S_CKCON :	/* Error executing command */
			req_sense(sense_data);

#if defined (i386) || defined (i486)
			sense_data->sd_ba = scl_swap32(sense_data->sd_ba);
#endif /* ix86 */

			/* Check Sense Key */

			if (Show) {
				(void) printf("send_scb(): Sense Key (0x%X)\n", sense_data->sd_key);
				(void) printf("send_scb(): Sense Code (0x%X)\n", sense_data->sd_sencode);
			}

			switch (sense_data->sd_key) {
			case SD_NOSENSE :
				break;
			case SD_VENUNI :
				error("Vendor Unique (0x%X)\n", sense_data->sd_sencode);
			case SD_RESERV :
				error("Reserved (0x%X)\n", sense_data->sd_sencode);
			default :
				switch (Inquiry_data.id_type) {
				case ID_DISK :
					switch (sense_data->sd_sencode) {
					case SC_NOSENSE :
						break;
					case SC_NOSGNL :
						error("No Index/Sector Signal\n");
					case SC_NOSEEK :
						error("No Seek Complete\n");
					case SC_WRFLT :	
						error("Write Fault\n");
					case SC_DRVNTRDY :
						error("Drive Not Ready\n");
					case SC_DRVNTSEL :
						error("Drive Not Selected\n");
					case SC_NOTRKZERO :
						error("No Track Zero found\n");
					case SC_MULTDRV :
						error("Multiple Drives Selected\n");
					case SC_LUCOMM :
						error("Logical Unit Communication Failure\n");
					case SC_TRACKERR :
						error("Track Following error\n");
					case SC_IDERR :	
					case SC_UNRECOVRRD :
					case SC_NOADDRID :
					case SC_NOADDRDATA :
					case SC_NORECORD :
					case SC_DATASYNCMK :
					case SC_RECOVRRD :
					case SC_RECOVRRDECC :
					case SC_CMPERR :
					case SC_RECOVRIDECC :
					case SC_MEDCHNG :
					case SC_RESET :	
						return(sense_data->sd_sencode);
					case SC_SEEKERR :
						error("Seek Positioning error\n");
					case SC_DFCTLSTERR :
						error("Defect List error\n");
					case SC_PARAMOVER :
						error("Paramater Overrun\n");
					case SC_SYNCTRAN :
						error("Synchronous Transfer error\n");
					case SC_NODFCTLST :
						error("Primary Defect List not found\n");
					case SC_INVOPCODE :
						error("Invalid Command Operation Code\n");
					case SC_ILLBLCK :
						error("Illegal Logical Block Address.\nAddress greater than the LBA returned by the READ CAPACITY data with PMI not set.\n");
					case SC_ILLFUNC :
						error("Illegal function for device type\n");
					case SC_ILLCDB :
						error("Illegal Field in CDB\n");
					case SC_INVLUN :
						error("Invalid LUN\n");
					case SC_INVPARAM :
						error("Invalid field in Parameter List\n");
					case SC_WRPROT :
						error("Write Protected\n");
					case SC_MDSELCHNG :
						error("Mode Select Parameters changed.\n");
					case SC_INCOMP :
						error("Incompatible Cartridge\n");
					case SC_FMTFAIL :
						error("Medium Format Corrupted\n");
					case SC_NODFCT :
						error("No Defect Spare Location Available\n");
					case SC_RAMFAIL :
						error("RAM Failure\n");
					case SC_DATADIAG :
						error("Data Path Diagnostic Failure\n");
					case SC_POWFAIL :
						error("Power On Diagnostic Failure\n");
					case SC_MSGREJCT :
						error("Message Reject Error\n");
					case SC_CONTRERR :
						error("Internal Controller Error\n");
					case SC_SELFAIL :
						error("Select/Reselect Failed\n");
					case SC_SOFTRESET :
						error("Unsuccessfule Soft Reset\n");
					case SC_PARITY :
						error("SCSI Interface Parity Error\n");
					case SC_INITERR :
						error("Initiator Detected Error\n");
					case SC_ILLMSG :
						error("Inappropriate/Illegal Message\n");
					default :
						error("Unknown sense code (0x%X)\n", sense_data->sd_sencode);
					}
					break;
				default :
					error("Unknown device type (0x%X)\n", Inquiry_data.id_type);
				}
				break;
			}
			break;
		case S_BUSY :		/* Controller busy	   */
			error("Controller busy\n");
		case S_RESER :		/* LUN Reserved		   */
			error("LUN Reserved\n");
		default :
			error("Unknown status (0x%X)\n", scb->SCB.sc_status);
		}
		break;
	case (SDI_NOALLOC & MASK_FF) :	/* This block is not allocated      */
		error("This block is not allocated\n");
	case (SDI_LINKF0 & MASK_FF) :	/* Linked command done without flag */
		error("Linked command done without flag\n");
	case (SDI_LINKF1 & MASK_FF) :	/* Linked command done with flag    */
		error("Linked command done with flag\n");
	case (SDI_QFLUSH & MASK_FF) :	/* Job was flushed                  */
		error("Job was flushed\n");
	case (SDI_ABORT & MASK_FF) :	/* Command was aborted              */
		error("Command was aborted\n");
	case (SDI_RESET & MASK_FF) :	/* Reset was detected on the bus    */
		error("Reset was detected on the bus\n");
	case (SDI_CRESET & MASK_FF) :	/* Reset was caused by this unit    */
		error("Reset was caused by this unit\n");
	case (SDI_V2PERR & MASK_FF) :	/* vtop failed                      */
		error("Virtual to Physical failed\n");
	case (SDI_TIME & MASK_FF) :	/* Job timed out                    */
		error("Job timed out\n");
	case (SDI_NOTEQ & MASK_FF) :	/* Addressed device not present     */
		error("Addressed device not present\n");
	case (SDI_HAERR & MASK_FF) :	/* Host adapter error               */
		error("Host Adapter error\n");
	case (SDI_MEMERR & MASK_FF) :	/* Memory fault                     */
		error("Memory fault\n");
	case (SDI_SBUSER & MASK_FF) :	/* SCSI bus error                   */
		error("SCSI bus error\n");
	case (SDI_SCBERR & MASK_FF) :	/* SCB error                        */
		error("SCB error\n");
	case (SDI_OOS & MASK_FF) :	/* Device is out of service         */
		error("Device is out of service\n");
	case (SDI_NOSELE & MASK_FF) :	/* The SCSI bus select failed       */
		error("The SCSI bus select failed\n");
	case (SDI_MISMAT & MASK_FF) :	/* parameter mismatch               */
		error("Parameter mismatch\n");
	case (SDI_PROGRES & MASK_FF) :	/* Job in progress                  */
		error("Job in progress\n");
	case (SDI_UNUSED & MASK_FF) :	/* Job not in use                   */
		error("Job not in use\n");
	case (SDI_ONEIC & MASK_FF) :	/* More than one immediate request */
		error("More than one immediate request\n");
	case (SDI_SFBERR & MASK_FF) :	/* SFB error			   */
		error("SFB error\n");
	default :
		error("Unknown completion code (0x%X)\n", scb->SCB.sc_comp_code);
	}

	if (Show && (scb->SCB.sc_datasz > 0) && (scb->SCB.sc_mode & SCB_READ)) {
		put_string(stderr, "Data received from Host Adapter");
		put_data(stderr, scb->SCB.sc_datapt, scb->SCB.sc_datasz);
	}
	return(0);
}	/* send_scb() */

void
format(format_cdb, format_bufpt, format_bufsz, format_time)
FORMAT_T	format_cdb;
char		*format_bufpt;
long		format_bufsz;
int		format_time;
{
	int		done = FALSE;
	struct sb	format_scb;
	struct sense	sense_data;
	void killtimer();
	void (*catchsig)();			/* used by format timer */
	long start, now, elapsed;		/* used by format timer */
	int hours, minutes, seconds;		/* used by format timer */
	(void) printf("Begin Format");
	if (format_time == 0)
		(void) printf("\n");
	else
		(void) printf(" (No more than %d minutes)\n", format_time / 16);

	if (Timer) {
		Timerpid = -1;
		/* Fork off timer child */
		switch (Timerpid=fork()) {
		case -1 :
			warning("format(): Can not fork timer display process\n");
			break;
		case 0 :
			/* then this is the child */

			(void)	signal(SIGHUP,SIG_DFL);
			(void) 	signal(SIGINT,SIG_DFL);
			(void)	signal(SIGQUIT,SIG_DFL);
			Stoptimer = FALSE;

			/* enable handling of SIGTERM from parent process */
			(void) signal(SIGTERM, catchsig);

			/* get and display starting time */
			(void) time(&start);
			(void) fprintf(stdout,"Elapsed time: 00:00:00");
			fflush(stdout);


			while ( Stoptimer == FALSE ) {

				(void) sleep(10);
				(void) time(&now);

				elapsed = now - start;
				hours = (int) elapsed / 3600;
				minutes = (int) (elapsed % 3600) / 60;
				seconds = (int) (elapsed % 3600) % 60;

				/* backspace over elapsed time display and update */
				(void) fprintf(stdout,"\015Elapsed time: %.2d:%.2d:%.2d",hours, minutes, seconds);
				fflush(stdout);

			}

			(void) fprintf(stdout,"\n");
			fflush(stdout);
			exit(NORMEXIT);
			break;

		default :
			/* this is the parent */
			break;
		}
	}

	/* Complete the Format SCSI Control Block */
	format_scb.SCB.sc_cmdpt = FORMAT_AD(&format_cdb);
	format_scb.SCB.sc_datapt = format_bufpt;
	format_scb.SCB.sc_mode = SCB_WRITE;
	format_scb.SCB.sc_cmdsz = FORMAT_SZ;
	format_scb.SCB.sc_datasz = format_bufsz;

	/* Send the Format SCSI Control Block to the Host Adapter */
	while (!done) {

		if (Show)
			put_string(stderr, "Format");

		/* Complete the Format SCSI Control Block */
		format_scb.SCB.sc_time = format_time * 4500;

		switch (send_scb(&format_scb, &sense_data)) {
		case SC_NOSENSE :
			done = TRUE;
			break;
		case SC_MEDCHNG :
			done = TRUE;
			break;
		case SC_RESET :
			done = FALSE;
			break;
		default :
			if (Timer)
				killtimer();
			error("format(): Unknown error (0x%X)\n", sense_data.sd_sencode);
		}
	}

	if (Timer)
		killtimer();
}	/* format() */

void
catchsig()
{
	(void) signal(SIGTERM, SIG_IGN);
	Stoptimer=TRUE;
}

void
mdselect(mdselect_cdb, mdselect_bufpt, mdselect_bufsz)
struct scs	mdselect_cdb;
char		*mdselect_bufpt;
long	 	mdselect_bufsz;
{
	int		done = FALSE;
	struct sb	mdselect_scb;
	struct sense	sense_data;

	/* Fill in the Mode Select SCSI Control Block */
	mdselect_scb.SCB.sc_cmdpt = SCS_AD(&mdselect_cdb);
	mdselect_scb.SCB.sc_datapt = mdselect_bufpt;
	mdselect_scb.SCB.sc_mode = SCB_WRITE;
	mdselect_scb.SCB.sc_cmdsz = SCS_SZ;
	mdselect_scb.SCB.sc_datasz = mdselect_bufsz;

	/* Send the Mode Select SCSI Control Block to the Host Adapter */
	while (!done) {

		if (Show)
			put_string(stderr, "Mode Select");

		/* Fill in the Mode Select SCSI Control Block */
		mdselect_scb.SCB.sc_time = 60 * 1000;	/* 1 Minute */

		switch (send_scb(&mdselect_scb, &sense_data)) {
		case SC_NOSENSE :
			done = TRUE;
			break;
		case SC_MEDCHNG :
			done = TRUE;
			break;
		case SC_RESET :
			done = FALSE;
			break;
		default :
			error("mdselect(): Unknown error (0x%X)\n", sense_data.sd_sencode);
		}
	}
}	/* mdselect() */

void
mdsense(mdsense_cdb, mdsense_bufpt, mdsense_bufsz)
struct scs	mdsense_cdb;
char		*mdsense_bufpt;
long		mdsense_bufsz;
{
	int		done = FALSE;
	struct sb	mdsense_scb;
	struct sense	sense_data;

	/* Fill in the Mode Sense SCSI Control Block */
	mdsense_scb.SCB.sc_cmdpt = SCS_AD(&mdsense_cdb);
	mdsense_scb.SCB.sc_datapt = mdsense_bufpt;
	mdsense_scb.SCB.sc_mode = SCB_READ;
	mdsense_scb.SCB.sc_cmdsz = SCS_SZ;
	mdsense_scb.SCB.sc_datasz = mdsense_bufsz;

	/* Send the Mode Sense SCSI Control Block to the Host Adapter */
	while (!done) {

		if (Show)
			put_string(stderr, "Mode Sense");

		/* Fill in the Mode Sense SCSI Control Block */
		mdsense_scb.SCB.sc_time = 60 * 1000;	/* 1 Minute */

		switch (send_scb(&mdsense_scb, &sense_data)) {
		case SC_NOSENSE :
			done = TRUE;
			break;
		case SC_MEDCHNG :
			done = TRUE;
			break;
		case SC_RESET :
			done = FALSE;
			break;
		default :
			error("mdsense(): Unknown error (0x%X)\n", sense_data.sd_sencode);
		}
	}
}	/* mdsense() */

void
scsi_write(write_start, write_bufpt, write_bufsz)
long		write_start;
char		*write_bufpt;
long		write_bufsz;
{
	int		done = FALSE;
	struct scs	write_cdb;
	struct sb	write_scb;
	struct sense	sense_data;

	/* Fill in the Write CDB */
	write_cdb.ss_op = SS_WRITE;
	write_cdb.ss_lun = LUN(Hostdev);
	write_cdb.ss_len = (write_bufsz + 511) / 512;
	write_cdb.ss_cont = 0;

#if defined (i386) || defined (i486)
	write_cdb.ss_addr1 = (write_start & 0x1F0000);
	write_cdb.ss_addr  = (write_start & 0xFFFF);
	write_cdb.ss_addr  = scl_swap16(write_cdb.ss_addr);
#else
	write_cdb.ss_addr  = write_start;
#endif /* ix86 */

	/* Fill in the Write SCSI Control Block */
	write_scb.SCB.sc_cmdpt = SCS_AD(&write_cdb);
	write_scb.SCB.sc_datapt = write_bufpt;
	write_scb.SCB.sc_mode = SCB_WRITE;
	write_scb.SCB.sc_cmdsz = SCS_SZ;
	write_scb.SCB.sc_datasz = write_bufsz;

	/* Send the Write SCSI Control Block to the Host Adapter */
	while (!done) {

		if (Show)
			put_string(stderr, "Write");


		/* Fill in the Write SCSI Control Block */
		write_scb.SCB.sc_time = 60 * 1000;	/* 1 Minute */

		switch (send_scb(&write_scb, &sense_data)) {
		case SC_NOSENSE :
			done = TRUE;
			break;
		case SC_IDERR :
		case SC_UNRECOVRRD :
		case SC_NOADDRID :
		case SC_NOADDRDATA :
		case SC_NORECORD :
		case SC_DATASYNCMK :
		case SC_RECOVRRD :
		case SC_RECOVRRDECC :
		case SC_CMPERR :
		case SC_RECOVRIDECC :
			if (sense_data.sd_valid) {
				int	block[2];

				block[0] = 4;
				block[1] = sense_data.sd_ba;
				(void) printf("Mapping Bad block 0x%X (0x%X)\n", block[1], sense_data.sd_sencode);
#if defined (i386) || defined (i486)
				block[0] = scl_swap32(block[0]);
				block[1] = scl_swap32(block[1]);
#endif /* ix86 */
				reassign((char *) block, 8);
				done = FALSE;
			} else
				done = TRUE;
			break;
		case SC_MEDCHNG :
			done = TRUE;
			break;
		case SC_RESET :
			done = FALSE;
			break;
		default :
			error("scsi_write(): Unknown error (0x%X)\n", sense_data.sd_sencode);
		}
	}
}	/* scsi_write() */

int
scsi_read(read_start, read_bufpt, read_bufsz)
long		read_start;
char		*read_bufpt;
long		read_bufsz;
{
	int		done = FALSE;
	int		rval = FALSE;
	struct scs	read_cdb;
	struct sb	read_scb;
	struct sense	sense_data;

	/* Fill in the Read CDB */
	read_cdb.ss_op = SS_READ;
	read_cdb.ss_lun = LUN(Hostdev);
	read_cdb.ss_len = (read_bufsz + 511) / 512;
	read_cdb.ss_cont = 0;

#if defined (i386) || defined (i486)
	read_cdb.ss_addr1 = (read_start & 0x1F0000);
	read_cdb.ss_addr  = (read_start & 0xFFFF);
	read_cdb.ss_addr  = scl_swap16(read_cdb.ss_addr);
#else
	read_cdb.ss_addr  = read_start;
#endif /* ix86 */

/*
	if (read_bufsz % 512) {
		char ibuf[read_cdb.ss_len];
		int i;

		for(i=0; i < read_bufsz; ++i);
			ibuf[i] =  *read_bufpt+i;
	}
*/
		

	/* Fill in the Read SCSI Control Block */
	read_scb.SCB.sc_cmdpt = SCS_AD(&read_cdb);
	read_scb.SCB.sc_datapt = read_bufpt;
	read_scb.SCB.sc_mode = SCB_READ;
	read_scb.SCB.sc_cmdsz = SCS_SZ;
	read_scb.SCB.sc_datasz = read_bufsz;

	/* Send the Read SCSI Control Block to the Host Adapter */
	while (!done) {

		if (Show)
			put_string(stderr, "Read");

		/* Fill in the Read SCSI Control Block */
		read_scb.SCB.sc_time = 60 * 1000;	/* 1 Minute */

		switch (send_scb(&read_scb, &sense_data)) {
		case SC_NOSENSE :
			done = TRUE;
			break;
		case SC_IDERR :
		case SC_UNRECOVRRD :
		case SC_NOADDRID :
		case SC_NOADDRDATA :
		case SC_NORECORD :
			if (sense_data.sd_valid) {
				char	*bufpt;
				int	block[2];

				bufpt = malloc(512);
				block[0] = 4;
				block[1] = sense_data.sd_ba;
				(void) printf("Mapping Bad block 0x%X (0x%X)\n", block[1], sense_data.sd_sencode);
#if defined (i386) || defined (i486)
				block[0] = scl_swap32(block[0]);
				block[1] = scl_swap32(block[1]);
#endif /* ix86 */
				reassign((char *) block, 8);
				block[1] = sense_data.sd_ba;
				scsi_write((long) block[1], bufpt, 512);
				free(bufpt);
				rval = TRUE;
			}
			done = TRUE;
			break;
		case SC_DATASYNCMK :
		case SC_RECOVRRD :
		case SC_RECOVRRDECC :
		case SC_CMPERR :
		case SC_RECOVRIDECC :
			if (sense_data.sd_valid) {
				int	block[2];

				block[0] = 4;
				block[1] = sense_data.sd_ba;
				(void) printf("Mapping Bad block 0x%X (0x%X)\n", block[1], sense_data.sd_sencode);
#if defined (i386) || defined (i486)
				block[0] = scl_swap32(block[0]);
				block[1] = scl_swap32(block[1]);
#endif /* ix86 */
				reassign((char *) block, 8);
				scsi_write(read_start, read_bufpt, read_bufsz);
#if defined (i386) || defined (i486)
				rval = MARBLK;
#else
				rval = TRUE;
#endif /* ix86 */
			}
			done = TRUE;
			break;
		case SC_MEDCHNG :
			done = TRUE;
			break;
		case SC_RESET :
			done = FALSE;
			break;
		default :
			error("scsi_read(): Unknown error (0x%X)\n", sense_data.sd_sencode);
		}
	}
	return(rval);
}	/* scsi_read() */

void
scsi_verify(verify_cdb, verify_start, verify_len, verify_size, no_map)
struct scm	verify_cdb;
int		verify_start;
int		verify_len;
int		verify_size;
int		no_map;
{
	int		last_start = verify_start;
	int		times = 0;
	struct sb	verify_scb;
	struct sense	sense_data;
	void killtimer();
	void (*catchsig)();		/* used by format timer */
	long start, now, elapsed;		/* used by format timer */
	int hours, minutes, seconds;		/* used by format timer */

	if (verify_size <= 0)
 		error("scsi_verify(): Non-negative verify size (%d)\n", verify_size);

	(void) printf("Begin Verify (No more than %d minutes)\n", ((verify_len * 2 / 1000) + 59) / 60);

	if (Timer) {
		Timerpid = -1;
		/* Fork off timer child */
		switch (Timerpid=fork()) {
		case -1 :
			warning("scsi_verify(): Can not fork timer display process\n");
			break;
		case 0 :
			/* then this is the child */

			(void)	signal(SIGHUP,SIG_DFL);
			(void) 	signal(SIGINT,SIG_DFL);
			(void)	signal(SIGQUIT,SIG_DFL);
			Stoptimer = FALSE;

			/* enable handling of SIGTERM from parent process */
			(void) signal(SIGTERM, catchsig);

			/* get and display starting time */
			(void) time(&start);
			(void) fprintf(stdout,"Elapsed time: 00:00:00");
			fflush(stdout);


			while ( Stoptimer == FALSE ) {

				(void) sleep(10);
				(void) time(&now);

				elapsed = now - start;
				hours = (int) elapsed / 3600;
				minutes = (int) (elapsed % 3600) / 60;
				seconds = (int) (elapsed % 3600) % 60;

				/* backspace over elapsed time display and update */
				(void) fprintf(stdout,"\015Elasped time: %.2d:%.2d:%.2d",hours, minutes, seconds);
				fflush(stdout);

			}

			(void) fprintf(stdout,"\n");
			fflush(stdout);
			exit(NORMEXIT);
			break;

		default :
			/* this is the parent */
			break;
		}
	}

	/* Fill in the Verify SCSI Control Block */
	verify_scb.SCB.sc_cmdpt = SCM_AD(&verify_cdb);
	verify_scb.SCB.sc_datapt = 0;
	verify_scb.SCB.sc_mode = SCB_WRITE;
	verify_scb.SCB.sc_cmdsz = SCM_SZ;
	verify_scb.SCB.sc_datasz = 0;

	while (verify_len > 0) {


		if (Show)
			put_string(stderr, "Verify");

		times += (verify_start == last_start);

		/* Fill in the Verify CDB */
		verify_cdb.sm_addr = verify_start;
		verify_cdb.sm_len = ((verify_len > verify_size) ? verify_size : verify_len);

		/* swap */
#if defined (i386) || defined (i486)
		verify_cdb.sm_addr = scl_swap32(verify_cdb.sm_addr);
		verify_cdb.sm_len  = scl_swap16(verify_cdb.sm_len);
#endif /* ix86 */

		/* Fill in the Verify SCSI Control Block */
		verify_scb.SCB.sc_time = ((verify_len > verify_size) ? verify_size : verify_len) * 2;

		/* Send the Verify SCSI Control Block to the Host Adapter */
		if (send_scb(&verify_scb, &sense_data) && (times < 5)) {
			switch (sense_data.sd_sencode) {
			case SC_NOSENSE :
				break;
			case SC_IDERR :
			case SC_UNRECOVRRD :
			case SC_NOADDRID :
			case SC_NOADDRDATA :
			case SC_NORECORD :
				if (sense_data.sd_valid) {
					char	*bufpt;
					int	block[2];

					block[0] = 4;
					block[1] = sense_data.sd_ba;
					if (!no_map) {
						bufpt = malloc(512);
						(void) printf("Mapping Bad block 0x%X (0x%X)\n", block[1], sense_data.sd_sencode);
#if defined (i386) || defined (i486)
						block[0] = scl_swap32(block[0]);
						block[1] = scl_swap32(block[1]);
#endif /* ix86 */
						reassign((char *) block, 8);
						block[1] = sense_data.sd_ba;
						scsi_write((long) block[1], bufpt, 512);
						free(bufpt);
					}
					else {
						(void) printf("Bad block 0x%X (0x%X)\n", block[1], sense_data.sd_sencode);
						verify_len -= ((verify_len > verify_size) ? verify_size : verify_len);
						verify_start += ((verify_len > verify_size) ? verify_size : verify_len);
						times = 0;
						last_start = verify_start;
					}
				}
				break;
			case SC_DATASYNCMK :
			case SC_RECOVRRD :
			case SC_RECOVRRDECC :
			case SC_CMPERR :
			case SC_RECOVRIDECC :
				if (sense_data.sd_valid) {
					char	*bufpt;
					int	block[2];

					block[0] = 4;
					block[1] = sense_data.sd_ba;
					if (!no_map) {
						bufpt = malloc(512);
#if defined (i386) || defined (i486)
						block[0] = scl_swap32(block[0]);
						block[1] = scl_swap32(block[1]);
#endif /* ix86 */
						block[1] = sense_data.sd_ba;

						if (scsi_read((long) block[1], bufpt, 512) == 0) {
							(void) printf("Mapping Bad Block 0x%X (0x%X)\n", block[1], sense_data.sd_sencode);
							reassign((char *) block, 8);
						}

						scsi_write((long) block[1], bufpt, 512);
						free(bufpt);
					}
					else {
						(void) printf("Bad block 0x%X (0x%X)\n", block[1], sense_data.sd_sencode);
						verify_len -= ((verify_len > verify_size) ? verify_size : verify_len);
						verify_start += ((verify_len > verify_size) ? verify_size : verify_len);
						times = 0;
						last_start = verify_start;
					}
				}
				break;
			case SC_MEDCHNG :
			case SC_RESET :
				break;
			default :
				if (Timer)
					killtimer();
				error("scsi_verify(): Unknown error (0x%X)\n", sense_data.sd_sencode);
			}
		} else {
			if (times >= 5)
				(void) printf("Skipping to next cylinder\n");
			verify_len -= ((verify_len > verify_size) ? verify_size : verify_len);
			verify_start += ((verify_len > verify_size) ? verify_size : verify_len);
			times = 0;
			last_start = verify_start;
		}
	}

	if (Timer) 
		killtimer();

}	/* scsi_verify() */
void
killtimer()
{
	(void) printf("\n");
	if(Timerpid != -1) {
		if (kill(Timerpid,SIGTERM) < 0)
			/* issue a warning to the user */
			warning("Could not kill timer display\n");
	}
}
void
readdefects(rdd_cdb, rdd_bufpt, rdd_bufsz)
struct scm	rdd_cdb;
char		**rdd_bufpt;
long		*rdd_bufsz;
{
	int		done = FALSE;
	struct sb	rdd_scb;
	struct sense	sense_data;

	/* Allocate memory to hold the defect list header */
	rdd_cdb.sm_len = DLH_SZ;
	*rdd_bufsz = rdd_cdb.sm_len;
	*rdd_bufpt = malloc(*rdd_bufsz);

	/* Fill in the READ DEFECT DATA SCB */
	rdd_scb.SCB.sc_cmdpt = SCM_AD(&rdd_cdb);
	rdd_scb.SCB.sc_datapt = *rdd_bufpt;
	rdd_scb.SCB.sc_mode = SCB_READ;
	rdd_scb.SCB.sc_cmdsz = SCM_SZ;
	rdd_scb.SCB.sc_datasz = *rdd_bufsz;

	/* Send the Read Defect Data SCB to the Host Adapter */
	while (!done) {

		if (Show)
			put_string(stderr, "Read Defect Data");

		/* Fill in the READ DEFECT DATA SCB */

		rdd_scb.SCB.sc_time = 60 * 1000;	/* 1 Minute */

		switch (send_scb(&rdd_scb, &sense_data)) {
		case SC_NOSENSE :
			done = TRUE;
			break;
		case SC_IDERR :
		case SC_UNRECOVRRD :
		case SC_NOADDRID :
		case SC_NOADDRDATA :
		case SC_NORECORD :
			if (sense_data.sd_valid) {
				char	*bufpt;
				int	block[2];

				bufpt = malloc(512);
				block[0] = 4;
				block[1] = sense_data.sd_ba;
				(void) printf("Mapping Bad block 0x%X (0x%X)\n", block[1], sense_data.sd_sencode);
#if defined (i386) || defined (i486)
				block[0] = scl_swap32(block[0]);
				block[1] = scl_swap32(block[1]);
#endif /* ix86 */
				reassign((char *) block, 8);
				block[1] = sense_data.sd_ba;
				scsi_write((long) block[1], bufpt, 512);
				free(bufpt);
			}
			done = TRUE;
			break;
		case SC_DATASYNCMK :
		case SC_RECOVRRD :
		case SC_RECOVRRDECC :
		case SC_CMPERR :
		case SC_RECOVRIDECC :
			if (sense_data.sd_valid) {
				char	*bufpt;
				int	block[2];

				bufpt = malloc(512);
				block[0] = 4;
				block[1] = sense_data.sd_ba;
#if defined (i386) || defined (i486)
				block[0] = scl_swap32(block[0]);
				block[1] = scl_swap32(block[1]);
#endif /* ix86 */
				block[1] = sense_data.sd_ba;

				if (scsi_read((long) block[1], bufpt, 512) == 0) {
					(void) printf("Mapping Bad Block 0x%X (0x%X)\n", block[1], sense_data.sd_sencode);
					reassign((char *) block, 8);
				}

				scsi_write((long) block[1], bufpt, 512);
				free(bufpt);
			}
			done = TRUE;
			break;
		case SC_MEDCHNG :
			done = TRUE;
			break;
		case SC_RESET :
			done = FALSE;
			break;
		default :
			error("readdefects(): Unknown error (0x%X)\n", sense_data.sd_sencode);
		}
	}

	if (((DLH_T *) *rdd_bufpt)->dlh_len) {
		/* Set up to get all Defect Data */
	  	*rdd_bufsz = DLH_SZ + scl_swap16(((DLH_T *) *rdd_bufpt)->dlh_len);
		rdd_cdb.sm_len = *rdd_bufsz;
		rdd_scb.SCB.sc_datasz = *rdd_bufsz;

		/* Allocate memory to hold the defect list header and defect list */
		*rdd_bufpt = realloc(*rdd_bufpt, *rdd_bufsz);
		rdd_scb.SCB.sc_datapt = *rdd_bufpt;

		/* Send the Read Defect Data SCSI Control Block to the Host Adapter */
		done = FALSE;
		while (!done) {
			/* Fill in the READ DEFECT DATA SCB */
			rdd_scb.SCB.sc_time = 60 * 1000;	/* 1 Minute */

			switch (send_scb(&rdd_scb, &sense_data)) {
			case SC_NOSENSE :
				done = TRUE;
				break;
			case SC_IDERR :
			case SC_UNRECOVRRD :
			case SC_NOADDRID :
			case SC_NOADDRDATA :
			case SC_NORECORD :
				if (sense_data.sd_valid) {
					char	*bufpt;
					int	block[2];

					bufpt = malloc(512);
					block[0] = 4;
					block[1] = sense_data.sd_ba;
					(void) printf("Mapping Bad block 0x%X (0x%X)\n", block[1], sense_data.sd_sencode);
#if defined (i386) || defined (i486)
					block[0] = scl_swap32(block[0]);
					block[1] = scl_swap32(block[1]);
#endif /* ix86 */
					reassign((char *) block, 8);
					block[1] = sense_data.sd_ba;
					scsi_write((long) block[1], bufpt, 512);
					free(bufpt);
				}
				done = TRUE;
				break;
			case SC_DATASYNCMK :
			case SC_RECOVRRD :
			case SC_RECOVRRDECC :
			case SC_CMPERR :
			case SC_RECOVRIDECC :
				if (sense_data.sd_valid) {
					char	*bufpt;
					int	block[2];

					bufpt = malloc(512);
					block[0] = 4;
					block[1] = sense_data.sd_ba;
#if defined (i386) || defined (i486)
					block[0] = scl_swap32(block[0]);
					block[1] = scl_swap32(block[1]);
#endif /* ix86 */
					block[1] = sense_data.sd_ba;

					if (scsi_read((long) block[1], bufpt, 512) == 0) {
						(void) printf("Mapping Bad Block 0x%X (0x%X)\n", block[1], sense_data.sd_sencode);
						reassign((char *) block, 8);
					}

					scsi_write((long) block[1], bufpt, 512);
					free(bufpt);
				}
				done = TRUE;
				break;
			case SC_MEDCHNG :
				done = TRUE;
				break;
			case SC_RESET :
				done = FALSE;
				break;
			default :
				error("readdefects(): Unknown error (0x%X)\n", sense_data.sd_sencode);
			}
		}
	}
}	/* readdefects() */

void
readcap(readcap_cdb, readcap_bufpt, readcap_bufsz)
struct scm	readcap_cdb;
char		*readcap_bufpt;
long		readcap_bufsz;
{
	int		done = FALSE;
	struct sb	readcap_scb;
	struct sense	sense_data;

	/* Fill in the READ CAPACITY SCB */
	readcap_scb.SCB.sc_cmdpt = SCM_AD(&readcap_cdb);
	readcap_scb.SCB.sc_datapt = readcap_bufpt;
	readcap_scb.SCB.sc_mode = SCB_READ;
	readcap_scb.SCB.sc_cmdsz = SCM_SZ;
	readcap_scb.SCB.sc_datasz = readcap_bufsz;

	/* Send the Read Capacity SCB to the Host Adapter */
	while (!done) {

		if (Show)
			put_string(stderr, "Read Capacity");

		/* Fill in the READ CAPACITY SCB */
		readcap_scb.SCB.sc_time = 60 * 1000;	/* 1 Minute */

		switch (send_scb(&readcap_scb, &sense_data)) {
		case SC_NOSENSE :
			done = TRUE;
			break;
		case SC_IDERR :
		case SC_UNRECOVRRD :
		case SC_NOADDRID :
		case SC_NOADDRDATA :
		case SC_NORECORD :
			if (sense_data.sd_valid) {
				char	*bufpt;
				int	block[2];

				bufpt = malloc(512);
				block[0] = 4;
				block[1] = sense_data.sd_ba;
				(void) printf("Mapping Bad block 0x%X (0x%X)\n", block[1], sense_data.sd_sencode);
#if defined (i386) || defined (i486)
				block[0] = scl_swap32(block[0]);
				block[1] = scl_swap32(block[1]);
#endif /* ix86 */
				reassign((char *) block, 8);
				block[1] = sense_data.sd_ba;
				scsi_write((long) block[1], bufpt, 512);
				free(bufpt);
			}
			done = TRUE;
			break;
		case SC_DATASYNCMK :
		case SC_RECOVRRD :
		case SC_RECOVRRDECC :
		case SC_CMPERR :
		case SC_RECOVRIDECC :
			if (sense_data.sd_valid) {
				char	*bufpt;
				int	block[2];

				bufpt = malloc(512);
				block[0] = 4;
				block[1] = sense_data.sd_ba;
#if defined (i386) || defined (i486)
				block[0] = scl_swap32(block[0]);
				block[1] = scl_swap32(block[1]);
#endif /* ix86 */
				block[1] = sense_data.sd_ba;

				if (scsi_read((long) block[1], bufpt, 512) == 0) {
					(void) printf("Mapping Bad Block 0x%X (0x%X)\n", block[1], sense_data.sd_sencode);
					reassign((char *) block, 8);
				}

				scsi_write((long) block[1], bufpt, 512);
				free(bufpt);
			}
			done = TRUE;
			break;
		case SC_MEDCHNG :
			done = TRUE;
			break;
		case SC_RESET :
			done = FALSE;
			break;
		default :
			error("readcap(): Unknown error (0x%X)\n", sense_data.sd_sencode);
		}
	}
}	/* readcap() */

void
reassign(rabl_bufpt, rabl_bufsz)
char		*rabl_bufpt;
long		rabl_bufsz;
{
	int		done = FALSE;
	struct sb	rabl_scb;
	struct scs	rabl_cdb;
	struct sense	sense_data;

	/* Fill in the Reassign Blocks CDB */
	rabl_cdb.ss_op = SS_REASGN;
	rabl_cdb.ss_lun = LUN(Hostdev);
	rabl_cdb.ss_addr1 = 0;
	rabl_cdb.ss_addr  = 0;
	rabl_cdb.ss_len = 0;
	rabl_cdb.ss_cont = 0;

	/* Fill in the Reassign Blocks SCB */
	rabl_scb.SCB.sc_cmdpt = SCS_AD(&rabl_cdb);
	rabl_scb.SCB.sc_datapt = rabl_bufpt;
	rabl_scb.SCB.sc_mode = SCB_WRITE;
	rabl_scb.SCB.sc_cmdsz = SCS_SZ;
	rabl_scb.SCB.sc_datasz = rabl_bufsz;

	/* Send the Reassign Blocks SCSI Control Block to the Host Adapter */
	while (!done) {

		if (Show)
			put_string(stderr, "Reassign Blocks");

		/* Fill in the Reassign Blocks SCB */
		rabl_scb.SCB.sc_time = rabl_bufsz * 60 * 1000;

		switch (send_scb(&rabl_scb, &sense_data)) {
		case SC_NOSENSE :
			done = TRUE;
			break;
		case SC_IDERR :
		case SC_UNRECOVRRD :
		case SC_NOADDRID :
		case SC_NOADDRDATA :
		case SC_NORECORD :
			if (sense_data.sd_valid) {
				char	*bufpt;
				int	block[2];

				bufpt = malloc(512);
				block[0] = 4;
				block[1] = sense_data.sd_ba;
				(void) printf("Mapping Bad block 0x%X (0x%X)\n", block[1], sense_data.sd_sencode);
#if defined (i386) || defined (i486)
				block[0] = scl_swap32(block[0]);
				block[1] = scl_swap32(block[1]);
#endif /* ix86 */
				reassign((char *) block, 8);
				block[1] = sense_data.sd_ba;
				scsi_write((long) block[1], bufpt, 512);
				free(bufpt);
			}
			done = TRUE;
			break;
		case SC_DATASYNCMK :
		case SC_RECOVRRD :
		case SC_RECOVRRDECC :
		case SC_CMPERR :
		case SC_RECOVRIDECC :
			if (sense_data.sd_valid) {
				char	*bufpt;
				int	block[2];

				bufpt = malloc(512);
				block[0] = 4;
				block[1] = sense_data.sd_ba;
#if defined (i386) || defined (i486)
				block[0] = scl_swap32(block[0]);
				block[1] = scl_swap32(block[1]);
#endif /* ix86 */
				block[1] = sense_data.sd_ba;

				if (scsi_read((long) block[1], bufpt, 512) == 0) {
					(void) printf("Mapping Bad Block 0x%X (0x%X)\n", block[1], sense_data.sd_sencode);
					reassign((char *) block, 8);
				}

				scsi_write((long) block[1], bufpt, 512);
				free(bufpt);
			}
			done = TRUE;
			break;
		case SC_MEDCHNG :
			done = TRUE;
			break;
		case SC_RESET :
			done = FALSE;
			break;
		default :
			error("reassign(): Unknown error (0x%X)\n", sense_data.sd_sencode);
		}
	}
}	/* reassign() */

void
inquiry(inquiry_data)
struct ident	*inquiry_data;
{
	int		done = FALSE;
	struct sb	inquiry_scb;
	struct scs	inquiry_cdb;
	struct sense	sense_data;

	/* Fill in the Inquiry CDB */
	inquiry_cdb.ss_op = SS_INQUIR;
	inquiry_cdb.ss_lun = LUN(Hostdev);
	inquiry_cdb.ss_addr1 = 0;
	inquiry_cdb.ss_addr  = 0;
	inquiry_cdb.ss_len = 5;
	inquiry_cdb.ss_cont = 0;

	/* Fill in the Inquiry SCB */
	inquiry_scb.SCB.sc_cmdpt = SCS_AD(&inquiry_cdb);
	inquiry_scb.SCB.sc_datapt = IDENT_AD(inquiry_data);
	inquiry_scb.SCB.sc_mode = SCB_READ;
	inquiry_scb.SCB.sc_cmdsz = SCS_SZ;
	inquiry_scb.SCB.sc_datasz = 5;

	/* Send the Inquiry SCSI Control Block to the Host Adapter */
	while (!done) {

		if (Show)
			put_string(stderr, "Inquiry");

		/* Fill in the Inquiry SCB */
		inquiry_scb.SCB.sc_time = 60 * 1000;	/* 1 Minute */

		switch (send_scb(&inquiry_scb, &sense_data)) {
		case SC_NOSENSE :
			done = TRUE;
			break;
		case SC_MEDCHNG :
			done = TRUE;
			break;
		case SC_RESET :
			done = FALSE;
			break;
		default :
			error("inquiry(): Unknown error (0x%X)\n", sense_data.sd_sencode);
		}
	}

	if ( (long) inquiry_data->id_len < (IDENT_SZ - 5))
		/* Not enough INQUIRY Data to name Target Controller */
		error("Insufficient INQUIRY Data\n");

	/* Set up to get all INQUIRY Data */
	inquiry_cdb.ss_len = IDENT_SZ;
	inquiry_scb.SCB.sc_datasz = IDENT_SZ;

	/* Send the Inquiry SCSI Control Block to the Host Adapter */
	done = FALSE;
	while (!done) {

		if (Show)
			put_string(stderr, "Inquiry");

		/* Fill in the Inquiry SCB */
		inquiry_scb.SCB.sc_time = 60 * 1000;	/* 1 Minute */

		switch (send_scb(&inquiry_scb, &sense_data)) {
		case SC_NOSENSE :
			done = TRUE;
			break;
		case SC_IDERR :
		case SC_UNRECOVRRD :
		case SC_NOADDRID :
		case SC_NOADDRDATA :
		case SC_NORECORD :
			if (sense_data.sd_valid) {
				char	*bufpt;
				int	block[2];

				bufpt = malloc(512);
				block[0] = 4;
				block[1] = sense_data.sd_ba;
				(void) printf("Mapping Bad block 0x%X (0x%X)\n", block[1], sense_data.sd_sencode);
#if defined (i386) || defined (i486)
				block[0] = scl_swap32(block[0]);
				block[1] = scl_swap32(block[1]);
#endif /* ix86 */
				reassign((char *) block, 8);
				block[1] = sense_data.sd_ba;
				scsi_write((long) block[1], bufpt, 512);
				free(bufpt);
			}
			done = TRUE;
			break;
		case SC_DATASYNCMK :
		case SC_RECOVRRD :
		case SC_RECOVRRDECC :
		case SC_CMPERR :
		case SC_RECOVRIDECC :
			if (sense_data.sd_valid) {
				char	*bufpt;
				int	block[2];

				bufpt = malloc(512);
				block[0] = 4;
				block[1] = sense_data.sd_ba;
#if defined (i386) || defined (i486)
				block[0] = scl_swap32(block[0]);
				block[1] = scl_swap32(block[1]);
#endif /* ix86 */
				block[1] = sense_data.sd_ba;

				if (scsi_read((long) block[1], bufpt, 512) == 0) {
					(void) printf("Mapping Bad Block 0x%X (0x%X)\n", block[1], sense_data.sd_sencode);
					reassign((char *) block, 8);
				}

				scsi_write((long) block[1], bufpt, 512);
				free(bufpt);
			}
			done = TRUE;
			break;
		case SC_MEDCHNG :
			done = TRUE;
			break;
		case SC_RESET :
			done = FALSE;
			break;
		default :
			error("inquiry(): Unknown error (0x%X)\n", sense_data.sd_sencode);
		}
	}
}	/* inquiry() */

void
get_defect(defectfile, bufpt, bufsz)
char	*defectfile;
char	**bufpt;
long	*bufsz;
{
	char	*defects;	/* Start of defect list		*/
	int	(*compar)();	/* Pointer to Defect List sorting function */
	int	defect_sz;	/* Size of each defect		*/
	short	number;		/* Total number of defects	*/
	FILE	*defectfp;	/* Defect file pointer		*/

	/* Open the defect file */
	if ((defectfp = fopen(defectfile, "r")) == NULL)
		/* Defect file cannot be opened */
		error("%s open failed\n", defectfile);

	switch (get_token(defectfp)) {
	case BLOCK :
		defect_sz = BLOCK_SZ;
		compar = blocksort;
		break;
	case BYTES :
		defect_sz = BYTES_SZ;
		compar = bytessort;
		break;
	case PHYSICAL :
		defect_sz = PHYSICAL_SZ;
		compar = physicalsort;
		break;
	default :
		error("Unknown token in %s\n", defectfile);
		break;
	}

	/* Read the number of defects in the defect list */
	if (get_data(defectfp, (char *) &number, 2) != 2)
		error("Cannot read number of defects in %s\n", defectfile);

	/* Allocate memory to hold the defect list header and defect list */
	*bufsz = defect_sz * number + DLH_SZ;
	*bufpt = realloc(*bufpt, *bufsz);

	((DLH_T *) *bufpt)->dlh_len = defect_sz * number;
	defects = *bufpt + DLH_SZ;

	/* Read the defect list from the defect file */
	if (get_data(defectfp, defects, scl_swap16(((DLH_T *) *bufpt)->dlh_len)) !=  scl_swap16(((DLH_T *) *bufpt)->dlh_len))
		error("Defect list is incomplete in %s\n", defectfile);

	/* Close the defect file */
	fclose(defectfp);
	
	/* Sort the defect list */
	qsort(defects, number, defect_sz, compar);
}	/* get_defect() */

void
put_defect(defectfp, bufpt)
FILE	*defectfp;
char	*bufpt;
{
	char	*defects;	/* Start of defect list		*/
	int	token;		/* Type of defects in the list	*/
	short	number;		/* Total number of defects	*/
	short	number1;
	int	i;

	if (Show)
		(void) fprintf (stderr, "DLF: (0x%X)\n", ((DLH_T *) bufpt)->dlh_dlf);

	switch (((DLH_T *) bufpt)->dlh_dlf) {
	case DLF_BLOCK :
		number = scl_swap16(((DLH_T *) bufpt)->dlh_len) / BLOCK_SZ;
		(void) printf("Defect List Length: %d Defective Logical Blocks\n", number);
		token = BLOCK;
		break;
	case DLF_BYTES :
		number = scl_swap16(((DLH_T *) bufpt)->dlh_len) / BYTES_SZ;
		(void) printf("Defect List Length: %d Defective Logical Blocks\n", number);
		token = BYTES;
		break;
	case DLF_PHYSICAL :
		number = scl_swap16(((DLH_T *) bufpt)->dlh_len) / PHYSICAL_SZ;
		(void) printf("Defect List Length: %d Defective Logical Blocks\n", number);
		token = PHYSICAL;
		break;
	default :
		error("Unknown Defect List Format (%d)\n", ((DLH_T *) bufpt)->dlh_dlf);
		break;
	}

	/* Put the type of defect in the defect file */
	if (defectfp != stdout)
		put_token(defectfp, token);

	/* Put the number of defects in the defect list in the defect file */
	if (defectfp != stdout) {
		number = scl_swap16(number);
		put_data(defectfp, (char *) &number, 2);
		number = scl_swap16(number);
	}

	/* Write the defect list to the defect file */
	defects = bufpt + DLH_SZ;
	switch (token) {
	int cur;
	case BLOCK :
		if (defectfp == stdout) {
			(void) printf("Logical Block Number\n");

			number1 = number;

			for (cur = 0; cur < number; cur+= 8) {
				for (i = 0; i < 8 && number1 > 0; ++i, --number1) {
					(void) fprintf(defectfp, "%.8X ", scl_swap32(((BLOCK_T *) defects)->dl_addr));
					defects += BLOCK_SZ;
				}
				(void) printf("\n");
			}
		}
		else {
			for (cur = 0; cur < number; cur++) {
				(void) fprintf(defectfp, "      %.8X\n", scl_swap32(((BLOCK_T *) defects)->dl_addr));
				defects += BLOCK_SZ;
			}
		}
		break;
	case BYTES :
		if (defectfp == stdout)
			(void) printf("Cylinder Track Bytes from Offset\n");
		for (cur = 0; cur < number; cur++) {
			(void) fprintf(defectfp, " %.6X   %.2X       %.8X\n",
				scl_swap24(((BYTES_T *) defects)->dl_cyl),
				((BYTES_T *) defects)->dl_head,
				scl_swap32(((BYTES_T *) defects)->dl_byte));
			defects += BYTES_SZ;
		}
		break;
	case PHYSICAL :
		if (defectfp == stdout)
			(void) printf("Cylinder Track  Sector\n");
		for (cur = 0; cur < number; cur++) {
			(void) fprintf(defectfp, " %.6X   %.2X   %.8X\n",
				scl_swap24(((PHYSICAL_T *) defects)->dl_cyl),
				((PHYSICAL_T *) defects)->dl_head,
				scl_swap32(((PHYSICAL_T *) defects)->dl_sec));
			defects += PHYSICAL_SZ;
		}
		break;
	}
}	/* put_defect() */

int
blocksort(defect1, defect2)
BLOCK_T	*defect1, *defect2;
{
	return(scl_swap32(defect1->dl_addr) - scl_swap32(defect2->dl_addr));
}	/* blocksort() */

int
bytessort(defect1, defect2)
BYTES_T	*defect1, *defect2;
{
	int cyldiff = scl_swap24(defect1->dl_cyl) - scl_swap24(defect2->dl_cyl);

	if (cyldiff == 0) {
		int headdiff = defect1->dl_head - defect2->dl_head;

		if (headdiff == 0)
			return(scl_swap32(defect1->dl_byte) - scl_swap32(defect2->dl_byte));
		else
			return(headdiff);
	} else
		return(cyldiff);
}	/* bytessort() */

int
physicalsort(defect1, defect2)
PHYSICAL_T	*defect1, *defect2;
{
	int cyldiff = scl_swap24(defect1->dl_cyl) - scl_swap24(defect2->dl_cyl);

	if (cyldiff == 0) {
		int headdiff = defect1->dl_head - defect2->dl_head;

		if (headdiff == 0)
			return(scl_swap32(defect1->dl_sec) - scl_swap32(defect2->dl_sec));
		else
			return(headdiff);
	} else
		return(cyldiff);
}	/* physicalsort() */

int
scsi_open(devicefile, bhostfile)
char	*devicefile;
char	*bhostfile;
{
	int	devicefdes;
	char	*ptr;
	

	/* Create the host adapter node in the same directory as
	/* the device node. */
	(void) strcpy(Hostfile, devicefile);
	if ((ptr = strrchr(Hostfile, '/')) != NULL)
		(void) strcpy(++ptr, bhostfile);
	else
		(void) strcpy(Hostfile, bhostfile);

	mktemp(Hostfile);
	errno = 0;

	if (Show)
		(void) fprintf(stderr, "Opening %s\n", devicefile);

	/* Open the special device file */
	if ((devicefdes = open(devicefile, O_RDONLY)) < 0) {
		/* Cannot continue if we cannot open the device */

		if (Show)
			(void) fprintf(stderr, "%s open failed\n", devicefile);

		return(TRUE);
	}

	/* Get the Host Adapter device number from the device driver */
	if (ioctl(devicefdes, B_GETDEV, &Hostdev) < 0) {

		if (Show)
			(void) fprintf(stderr, "B_GETDEV ioctl failed\n");

		close(devicefdes);
		return(TRUE);
	}

	/* Close the special device file */
	close(devicefdes);

	if (Show)
		(void) fprintf(stderr, "Creating %s\n", Hostfile);

	/* Create the Host Adapter special device file */
#if defined (i386) || defined (i486)
	if (mknod(Hostfile, (S_IFCHR | S_IREAD | S_IWRITE), Hostdev) < 0)
#else
	if (mknod(Hostfile, (S_IFCHR | S_IREAD | S_IWRITE), makedev(Hostdev.maj),(Hostdev.min))) < 0)
#endif /* ix86 */
	{
		if (Show)
			(void) fprintf(stderr, "%s mknod failed\n", Hostfile);
		return(TRUE);
	}

	sync(); sync(); sleep(2);

	if (Show)
		(void) fprintf(stderr, "Opening %s\n", Hostfile);

	/* Open the SCSI Host Adapter special device file */
	if ((Hostfdes = open(Hostfile, O_RDWR)) < 0) {
		/* Remove the Host Adapter special device file */
		unlink(Hostfile);

		/* Cannot continue if we cannot open the device */

		if (Show)
			(void) fprintf(stderr, "%s open failed\n", Hostfile);
		return(TRUE);
	}

	/* Get INQUIRY Data */
	inquiry(&Inquiry_data);

	return(FALSE);
}	/* scsi_open() */
