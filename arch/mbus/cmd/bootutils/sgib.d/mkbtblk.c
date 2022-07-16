/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988, 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:cmd/bootutils/sgib.d/mkbtblk.c	1.3"

#include <sys/types.h>
#include <stdio.h>
#include <memory.h>
#include <sys/fdisk.h>
#include <sys/ivlab.h>
#include "sgib.h"

extern int	debug;
extern int	delta_flag;
extern int	type;
extern ushort	dev_gran;
extern ushort	nfheads;
extern ushort	nrheads;
extern ushort	nsec;
extern ushort	ncyl;
extern ushort	ileave;
extern ushort	fsdelta;
extern uint	msa_text_size;
extern uint	msa_text_loc;
extern uint	msa_data_size;
extern char	*real_buf_1;
extern struct	btblk btblk;

extern char	*strcpy();
extern char	*strncpy();
extern void	exit();

void
mkbtblk()
{
	ulong	dsksz;
	ushort	nalt;
	ushort	nhead;
	char	iflags;
	struct btblk *bt;
	struct drtab *dt;

	bt = &btblk;
	dt = (struct drtab *)bt->ivlab.v_dspecial;

	/*********************/
	/*     Bootcode (1)  */
	/*********************/
	
	(void)memcpy(bt->bootcode, real_buf_1 , REAL_1_SIZE);

	/*********************/
	/*     Ipart 		 */
	/*********************/

	nhead = nfheads + nrheads;
	nalt = (((ulong)ncyl * nhead) / 50) / nhead;
	dsksz = (ulong)dev_gran * nsec * nhead;

	if ((int)nfheads > 0) {
		iflags = VF_AUTO | VF_MINI | VF_NOT_FLOPPY;
		dsksz = dsksz * (ncyl - nalt);
		if(delta_flag == 0)
			fsdelta = nsec;
	} else {
		iflags = VF_AUTO | VF_MINI | VF_DENSITY;
		if (nrheads > 1)
			iflags |= VF_SIDES;
		if(delta_flag == 0)
			fsdelta = (2048 / dev_gran) + nsec;
		dsksz = dsksz * ncyl;
	}

	(void)strncpy( bt->ivlab.v_name , "UNIX V4.0",10);
	bt->ivlab.v_flags =	iflags;
	bt->ivlab.v_fdriver =	UNIX_FD;
	bt->ivlab.v_gran =	dev_gran;
	bt->ivlab.v_size_l =	(ushort)(dsksz & 0xffff);
	bt->ivlab.v_size_h =	(ushort)((dsksz >> 16) & 0xffff);
	bt->ivlab.v_maxfnode =	0;
	bt->ivlab.v_stfnode_l = 2;
	bt->ivlab.v_stfnode_h = 2;
	bt->ivlab.v_szfnode =	32;
	bt->ivlab.v_rfnode =	2;
	bt->ivlab.v_devgran =	dev_gran;
	bt->ivlab.v_intl =	ileave;
	bt->ivlab.v_trskew =	0x0 ;
	bt->ivlab.v_sysid =	UNIX_SID;
	(void)strncpy(bt->ivlab.v_sysname , "MB V4.0 386",12);
	
	/* driver table ( v_dspecial )*/

	dt->dr_ncyl =	 ncyl;
	dt->dr_nfhead =  (char)nfheads;
	dt->dr_nrhead =  (char)nrheads;
	dt->dr_nsec = 	 (char)nsec;
	dt->dr_lsecsiz = (char)(dev_gran & 0xff);
	dt->dr_hsecsiz = (char)((dev_gran >> 8 ) & 0xff);
	dt->dr_nalt = 	 (char)nalt;

	bt->ivlab.v_fsdelta =	fsdelta;
	(void)strncpy(bt->ivlab.v_freespace , "UNIX",4);

	/*********************/
	/*     Ipart 	     */
	/*********************/

	(void)memset(bt->ipart, 0, IPART_SIZE);

	/*********************/
	/*     Ipart Sanity  */
	/*********************/

	bt->signature = BTBLK_MAGIC;

	/*********************/
	/* Bolt Area for MSA */
	/*********************/

	bt->bolt.reserved[0] = 0;
	bt->bolt.reserved[1] = 0;
	bt->bolt.reserved[2] = 0;
	bt->bolt.reserved[3] = 0;
	bt->bolt.magic_word_1 = (unsigned long)MAGIC_WORD_1;
	bt->bolt.magic_word_2 = 0;
	bt->bolt.version      = VERSION;
	bt->bolt.types	      = TYPES;
	bt->bolt.data_size    = msa_data_size;
	bt->bolt.num_entries  = 1;
	if (type == BSERVER)
		bt->bolt.tbl_entries.byte_offset = 0;
	else
		bt->bolt.tbl_entries.byte_offset = msa_text_loc;

	bt->bolt.tbl_entries.length = (unsigned short)msa_text_size;

	/* and file in the unused areas rest */

	(void)memset(bt->rsrvd1, 0, RSRVD_1_SIZE);
	(void)memset(bt->isovlab.vlab, 0, ISO_SIZE);
	(void)memset(bt->rsrvd2, 0, RSRVD_2_SIZE);
/*
*/
}
