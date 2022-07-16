/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988, 1989 Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:cmd/bootutils/sgib.d/util.c	1.3"

#include <a.out.h>
#include <sys/types.h>
#include <sys/fdisk.h>
#include <sys/ivlab.h>
#include <memory.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include "sgib.h"

extern int	debug;
extern uint	msa_text_size;
extern char	*rboot;
extern char	*boot;
extern char	*msa_text_buf;
extern char	*real_buf_2;
extern struct	btblk btblk;

extern int	write();
extern void	exit();
extern void	free();
extern void	sync();
extern int	close();

/* 
 * close the output file  to create a file mark prior to loading the 
 * the second stage image if its a tape. We use a close/open secquence,
 * so that if its not a tape device everything works ok.
 */
static int
make_filemark(device, dev)
char	*device;
int	dev;
{
	(void)close(dev) ;
	sync() ;

	/* Try to open read/write with append - if this fails */
	/* open read/write only.  This should now work on an  */
	/* AT system.                                         */

	if ( (dev = open(device, (O_RDWR | O_APPEND))) == -1 ) 
		if ( (dev = open(device, (O_RDWR))) == -1 ) {
			ERR(stderr, "failure to open %s\n", device);
			exit(1);
		}
	sync();
	return (dev);
}


void
writebuff(btype, device)
int	btype;
char	*device;
{
	int	dev;
	static int make_filemark();
	
	if ((dev = open(device, (O_RDWR | O_TRUNC | O_CREAT), 0644)) == -1 ) {
		ERR(stderr, "failure to open %s\n", device);
		exit(1);
	}
	if (debug)
		ERR(stderr, "opening %s, dev=%d\n", device, dev);

	switch (btype) {
		case BTAPE:	/* MSA Boot Tape */
			if ((write(dev, &btblk, BTBLK_SIZE)) == -1 ) {
				ERR(stderr, "write of boot buffer failed\n");
				exit(1);
			}

#ifdef MB1
			if ((write(dev, real_buf_2, REAL_2_SIZE)) == -1 ) {
				ERR(stderr, "write of '%s' failed\n", rboot);
				exit(1);
			}
#endif
			dev = make_filemark(device, dev);
	
			if ((write(dev, msa_text_buf, msa_text_size)) == -1 ) {
				ERR(stderr, "write of '%s' failed\n", boot);
				exit(1);
			}
			break;

		case BSERVER:
			/*
			 * if we're a boot server, set bolt offset to 0 and
			 * the buff size to bolt+text.
			 * the text offset is set to zero in the blot buffer 
			 * by mkbolt 
			 */
	
			if ((write(dev, &btblk.bolt, BOLT_SIZE)) == -1 ) {
				ERR(stderr, "write of bolt buffer failed\n");
				exit(1);
			}
			if ((write(dev, msa_text_buf, msa_text_size)) == -1 ) {
				ERR(stderr, "write of '%s' failed\n", boot);
				exit(1);
			}
			break;
		case BDISK:	/* Hard Disk */
			if ((write(dev, &btblk, BTBLK_SIZE)) == -1 ) {
				ERR(stderr, "write of boot buffer failed\n");
				exit(1);
			}
			if ((write(dev, real_buf_2 ,REAL_2_SIZE)) == -1 ) {
				ERR(stderr, "write of '%s' failed\n", rboot);
				exit(1);
			}
			if ((write(dev, msa_text_buf, msa_text_size)) == -1 ) {
				ERR(stderr, "write of '%s' failed\n", boot);
				exit(1);
			}
			break;
		case BFLOP:	/* Floppy  Disk */
			if ((write(dev, &btblk, BTBLK_SIZE)) == -1 ) {
				ERR(stderr, "write of boot buffer failed\n");
				exit(1);
			}
			if ((write(dev, real_buf_2,REAL_2_SIZE)) == -1 ) {
				ERR(stderr, "write of '%s' failed\n", rboot);
				exit(1);
			}
			if ((write(dev, msa_text_buf, msa_text_size)) == -1 ) {
				ERR(stderr, "write of '%s' failed\n", boot);
				exit(1);
			}
			break;
	}
	(void)close(dev) ;
	sync() ;
}
