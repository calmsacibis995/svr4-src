/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mbus:uts/i386/sys/bps.h	1.1.1.1"

#ifndef _SYS_BPS_H
#define _SYS_BPS_H

/*	Copyright (c) 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

/* 
 * NOTE: The bps structure is comprised of the bps_save_hdr, the 
 * bps_param_hdr, and the key_val structures.  This data structure is 
 * shared with the BPS manager in MSA f/w and should be consistent !!
 */

#ifndef lint
#pragma pack(1)			/* WARNING: All these structs must be PACKED !! */
#endif						

struct bps_save_hdr {
	unsigned long		reserved;
	unsigned short		total;	
	unsigned short		next_byte;
	unsigned short		next_param;
	unsigned short		count;
	unsigned char		state;
	unsigned char		terminator;
};

struct bps_param_hdr {
	unsigned short	total;
	unsigned char	source;
	unsigned char	value_offset;
	unsigned char	name_length;
};

struct bps	{
	struct		bps_save_hdr	bps_save_hdr;
	struct		bps_param_hdr	bps_param_hdr;
	unsigned	char 			parameters;			
};

struct	key_val		{
	unsigned	char	count;
	unsigned	char	value;
};

struct	bps_p_itbl	{
	char		*param_name;
	char		*param_value;
	char		*int_bps_param;
	char		*int_bps_value;
	int			param_val_len;
};

#ifndef lint
#pragma pack()
#endif

/* IOCTL definitions for BPS driver */

#define BPSINIT			1
#define BPSGETPV		2
#define BPSGETWCPV		3
#define BPSGETOPTS		4
#define BPSGETINTEGER	5
#define BPSGETSOCKET	6
#define BPSGETRANGE		7

struct	bps_ioctl	{
	int			str_len;		/* must include the null-terminator */
	char		*string_p;
	int			valbuf_len;		/* must include the null-terminator */
	char		*valbuf_p;
	int			value_len;		/* must include the null-terminator */
	char		*value_p;
	int			*state_p;		
	int			*config_code;
	int			*lo_return_p;
	int			*hi_return_p;
	int			status;			/* status for this call */
};

#ifdef	__STDC__
extern	int	bps_get_wcval(char *, int *, int, char *);
extern	int	bps_get_val(char *, int, char *);
extern	int	bps_get_opt(char *, int *, char *, int *, int, char *);
extern	int	bps_get_integer(char *, int *);
extern	int	bps_get_range(char *, int *, int *);
extern	int	bps_get_socket(char *, int *, int *);

#ifdef	_KERNEL
extern	int	bps_open();
extern	int	bps_init(char *);
#else	!_KERNEL
extern	int bpsopen();
extern	int bpsclose();
extern	int bpsinit(char *);
#endif	!_KERNEL

#else	!__STDC__
extern	int	bps_get_wcval();
extern	int	bps_get_val();
extern	int	bps_get_opt();
extern	int	bps_get_integer();
extern	int	bps_get_range();
extern	int	bps_get_socket();

#ifdef	_KERNEL
extern	int	bps_open();
extern	int	bps_init();
#else	!_KERNEL
extern	int	bpsopen();
extern	int	bpsclose();
extern	int	bpsinit();
#endif	!_KERNEL
#endif	!__STDC__

#endif	/* _SYS_BPS_H */
