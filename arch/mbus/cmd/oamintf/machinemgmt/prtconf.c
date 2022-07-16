/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:cmd/oamintf/machinemgmt/prtconf.c	1.3"

static char prtconf_copyright[] = "Copyright 1988 Intel Corp. 462677";

#include <stdio.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/ics.h>
#include <ctype.h>


#define	CACHEMEM	5
#define HOSTID		16
#define LOCALMEMREC	17
#define MEMREC		1
#define ENDREC		255


int	fr_type;
int	fr_len;
char	fr_buf[512];

int	offset;
char	*bnames[] = {
	"CSM/001", "Central Services Module",
	"386/258", "Disk Controller",
	"386/116", "16 Mhz CPU",
	"386/120", "20 Mhz CPU",
	"186/410", "Serial Communications Adapter",
	"186/530", "Ethernet Communications Adapter  ",
	0,0
	};

struct header {
	short	VendorID;
	char	BoardID[10];
	char	extra[20];
};

struct header hdr;

main(argc,argv)
int argc;
char *argv[];
{
	int fd;
	int slot;
	char *BoardDescrip, **btp;
	
	fd=open("/dev/ics",O_RDWR);
	
	if (fd == -1) {
		perror("/dev/ics");
		exit(1);
	}
	printf("\n  MULTIBUS II CONFIGURATION:          \n\n");
	for (slot=0; slot<20; slot++)
	{
	    ics_read(fd, slot, 0, &hdr, 32);
	    if (hdr.VendorID != 0)
	    {
	        BoardDescrip = "Board";
	        btp = bnames;
	        while ( *btp != 0 )
	        {
	            if (strcmp(hdr.BoardID, *btp++) == 0)
	            {
	                BoardDescrip = *btp;
	                btp++;
                        break;
                    }
	            btp++;
	        }
	        printf("    Slot %02d  %s %s\n", slot, hdr.BoardID, BoardDescrip);
                offset = 32;
	        ics_read(fd, slot, offset++, &fr_type, 1);
	        while (fr_type != 255)
	        {
	            ics_read(fd, slot, offset++, &fr_len, 1);
	            fr_buf[0] = fr_type;
	            fr_buf[1] = fr_len;
	            ics_read(fd, slot, offset, &fr_buf[2], fr_len);
	            offset = offset + fr_len;
	            fr_decode(fr_type, &fr_buf[0], fr_len);
	            ics_read(fd, slot, offset++, &fr_type, 1);
	        }
	    }
	}
	printf("\n");
	exit(0);
}


fr_decode(ftype, fbuf, flen)
int ftype;
int flen;
unsigned char fbuf[];
{

	int size;
	unsigned long start_addr, end_addr;

	switch (ftype)
	{
	    case MEMREC:
	        size = (pack16(&fbuf[2]) + 1) * 64;
	        printf("             MEMORY:\t%dK\n", size);
	        break;
/****************************
	    case LOCALMEMREC:
		start_addr = pack16(&fbuf[2]) << 16;
	        end_addr = pack16(&fbuf[4]) << 16;
	        printf("             ADDRESS:\t%08XH-%08XH\n", start_addr, end_addr);
	        break;
****************************/
	    case CACHEMEM:
	        size = (pack16(&fbuf[2]) + 1 ) * 256;
	        printf("             CACHE:\t%d bytes\n", size) ;
	        break;

	}
}


fr_dump(fbuf, flen)
unsigned char fbuf[];
int flen;
{
	int i;

	printf("    ");
	for (i=0; i<=flen; i++)
	{
	    printf(" %02X", fbuf[i]);
	}
	printf("\n");
}



pack16(buf)
unsigned char buf[];
{
	return (buf[0] + (buf[1] << 8));
}
