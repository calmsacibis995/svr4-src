/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1989 Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ifndef lint
static char libbpscopyright[] = "Copyright 1989 Intel Corporation 464726";
#endif /* lint */

#ident	"@(#)mbus:lib/libbps/libbps.c	1.1"

/*****************************************************************************
 *
 * TITLE:	Bootstrap Parameter String Library
 *
 ****************************************************************************/
#include <sys/types.h>
#include <sys/param.h> 
#include <sys/fcntl.h>
#include <sys/bps.h>
#include <string.h>

static dev_t 	Device;		/* Device is hidden in the module */
static int	opened = 0; 	/* opened is a state var. */

#ifdef DEBUG
#define	DEB_OPEN	0x0001
#define	DEB_INIT	0x0002
#define	DEB_CLOSE	0x0004
#define	DEB_GETVAL	0x0008
#define	DEB_GETWC	0x0010
#define	DEB_GETOPT	0x0020
#define	DEB_GETINT	0x0040
#define	DEB_GETRNG	0x0080
#define	DEB_GETSOC	0x0100

#define DEBPR(x,y)    	if(libbps_debug & (x)) printf y
int	libbps_debug = 0;

#else

#define	DEBPR(x,y)			
int	libbps_debug = 0;

#endif
/* These procedures hide system dependencies,etc */

/* bpsopen must be called before any other function can work.  */

int	
bpsopen()
{
int	oflag = O_RDONLY;

	DEBPR(DEB_OPEN,("bpsopen: entered\n"));

	if (!opened) 
		if ((Device = open("/dev/bps", oflag)) == -1) {
			DEBPR(DEB_OPEN,("bpsopen: failed\n"));
			return (-1);
		}
	opened++;
	DEBPR(DEB_OPEN, ("bpsopen: successful, Device %d\n", Device));
	return (0);
}

/* 
 * bpsclose should be called as many times as open, with the last call made
 * when user is finshed working with the BPS.  
 */
int	
bpsclose()
{
	DEBPR(DEB_CLOSE, ("bpsclose: entered\n"));

	if (!opened) {
		DEBPR(DEB_CLOSE, ("bpsclose: close called without open\n"));
		return (-1);
	}
	else {
		if (--opened == 0) {
			DEBPR(DEB_CLOSE, ("bpsclose: last close\n"));
			return(close (Device));
		}
		else
			return (0);
	} 
}

/* 
 * bpsinit is ONLY FOR TESTING.  It changes the value of the BPS to the
 * string Newbps 
 */
int	
bpsinit (Newbps)
char	*Newbps;
{
	struct bps_ioctl bps_cb;
	int		e_code;

	DEBPR(DEB_INIT, ("bpsinit: entered\n"));
	
	if (!opened) {
		DEBPR(DEB_INIT, ("bpsinit: bpsinit called without bpsopen\n"));
		return (-1);
	}
	/* 
	 * If Newbps is NULL, send no control block to init, signifying use 
	 * native bps 
	 */
	if (Newbps != NULL) {
		bps_cb.string_p = Newbps;
		bps_cb.str_len = strlen(Newbps) + 1;
		DEBPR(DEB_INIT, ("bpsinit: string %s, len %d\n, Device", 
			bps_cb.string_p, bps_cb.str_len, Device));
		e_code = ioctl(Device, BPSINIT, &bps_cb, 0);
	} else 
		e_code = ioctl(Device, BPSINIT, NULL, 0);

	DEBPR(DEB_INIT, ("bpsinit: completed, return code %x, status %x\n", 
		e_code, bps_cb.status));
	return(e_code);
}
/* 
 * bps_get_val makes a copy of the value of a BPS parameter in the
 * users space. 
 * ParamName - is the BPS parameter whose value is retrieved,
 * ptrValue  - is the space for the value to be stored.  
 * LenValue  - is the size of this space.  
 * If the parameter value is larger than the space
 * provided, no retrieval occurs, and an error is returned 
 */  
int	
bps_get_val (ParamName, LenValue, ptrValue)
char	*ParamName;
int		LenValue;
char	*ptrValue;
{
	struct bps_ioctl bps_cb;

	DEBPR(DEB_GETVAL, ("bps_get_val: entered\n"));

	if (!opened) {
		DEBPR(DEB_GETVAL, ("bps_get_val: called without bpsopen\n"));
		return (-1);
	}
	bps_cb.string_p = ParamName;
	bps_cb.str_len = strlen(ParamName) + 1;
	bps_cb.valbuf_p = ptrValue;
	bps_cb.valbuf_len = LenValue;
	if ((ioctl(Device, BPSGETPV, &bps_cb, 0)) == -1)
		return(-1);
	else 
		return(bps_cb.status);
}

/* 
 * bps_get_wcval retrieves the values of parameters that match a wild-card
 * pattern.  
 * ParamPattern - is the pattern the parameter names must match; 
 * StatePtr - counts how many times the pattern has been matched, and 
 * ptrValue and LenValue are as above.  
 */
int	
bps_get_wcval (ParamPattern, StatePtr, LenValue, ptrValue)
char	*ParamPattern;
int		*StatePtr;
int		LenValue;
char	*ptrValue;
{
	struct bps_ioctl bps_cb;

	DEBPR(DEB_GETWC, ("bps_get_wcval: entered\n"));

	if (!opened) {
		DEBPR(DEB_GETWC, ("bps_get_wcval: called without bpsopen\n"));
		return(-1);
	}
	bps_cb.string_p = ParamPattern;
	bps_cb.str_len = strlen(ParamPattern) + 1;
	bps_cb.state_p = StatePtr;
	bps_cb.valbuf_p = ptrValue;
	bps_cb.valbuf_len = LenValue;
	if ((ioctl(Device, BPSGETWCPV, &bps_cb, 0)) == -1)
		return(-1);
	else 
		return(bps_cb.status);
}

/* 
 * bps_get_opt retrieves subparameters from the values retrieved by the
 * bps_get_val or bps_get_wcval routines.  
 * bps_value - is the value to be parsed for subparameters; 
 * state_ptr - is for internal use only; 
 * pattern   - is the pattern of subparameter names to be retrieved, 
 * code_ptr  - is the index (1-origined ) of the subparameter name found; and 
 * len_value and value_ptr are as above 
 */
int	
bps_get_opt( bps_value, state_ptr, pattern, code_ptr, len_value, value_ptr)
char	*bps_value;
int		*state_ptr;
char	*pattern;
int		*code_ptr;
int		len_value;
char	*value_ptr;
{
	struct bps_ioctl bps_cb;

	DEBPR(DEB_GETOPT, ("bps_get_opt: entered\n"));

	if (!opened) {
		DEBPR(DEB_GETOPT, ("bps_get_opt: called without bpsopen\n"));
		return (-1);
	}
	bps_cb.str_len     = strlen(pattern) + 1;
	bps_cb.string_p    = pattern;
	bps_cb.valbuf_len  = strlen(bps_value) + 1;
	bps_cb.valbuf_p    = bps_value;
	bps_cb.value_len   = len_value;
	bps_cb.value_p     = value_ptr;
	bps_cb.state_p     = state_ptr;
	bps_cb.config_code = code_ptr;
	if ((ioctl(Device, BPSGETOPTS, &bps_cb, 0)) == -1)
		return(-1);
	else 
		return(bps_cb.status);
}
/* 
 * bps_get_integer translates the (character string) value_ptr into an 
 * integer returned in int_ptr 
 */
int	
bps_get_integer( value_ptr, int_ptr)
char	*value_ptr;
int		*int_ptr;
{
	struct bps_ioctl bps_cb;

	DEBPR(DEB_GETINT, ("bps_get_integer: entered\n"));
	
	if (!opened) {
		DEBPR(DEB_GETINT,("bps_get_integer: called without bpsopen\n"));
		return (-1);
	}
	bps_cb.str_len     = 0;
	bps_cb.string_p    = NULL ;
	bps_cb.valbuf_len  = 0;
	bps_cb.valbuf_p    = NULL ;
	bps_cb.value_len   = strlen(value_ptr) + 1;
	bps_cb.value_p     = value_ptr;
	bps_cb.state_p     = 0;
	bps_cb.lo_return_p = int_ptr;
	if ((ioctl(Device, BPSGETINTEGER, &bps_cb, 0)) == -1)
		return(-1);
	else 
		return (bps_cb.status);
}

/* 
 * bps_get_range translates the (character string) value_ptr into a pair of
 * integer values returned in lo_ptr and hi_ptr.  (No actual check is 
 * made on the relative sizes of the integers )  
 */
int	
bps_get_range( value_ptr, lo_ptr, hi_ptr)
char	*value_ptr;
int		*lo_ptr, *hi_ptr;
{
	struct bps_ioctl bps_cb;
	
	DEBPR(DEB_GETRNG, ("bps_get_range: entered\n"));

	if (!opened) {
		DEBPR(DEB_GETRNG, ("bps_get_range: called without bpsopen\n"));
		return (-1);
	}
	bps_cb.str_len     = 0;
	bps_cb.string_p    = NULL ;
	bps_cb.valbuf_len  = 0;
	bps_cb.valbuf_p    = NULL ;
	bps_cb.value_len   = strlen(value_ptr) + 1;
	bps_cb.value_p     = value_ptr;
	bps_cb.state_p     = 0;
	bps_cb.lo_return_p = lo_ptr;
	bps_cb.hi_return_p = hi_ptr;
	if ((ioctl(Device, BPSGETRANGE, &bps_cb, 0)) == -1)
		return(-1);
	else 
		return (bps_cb.status);
}

/* 
 * bps_get_socket translates the (character string) value_ptr into a pair of
 * integer values returned in port_ptr and host_ptr.  (No check is made 
 * on the integers )  
 */
int	
bps_get_socket ( value_ptr, port_ptr, host_ptr)
/* change in order is deliberate.  as specified */
char	*value_ptr;
int		*port_ptr, *host_ptr;
{
	struct bps_ioctl bps_cb;

	DEBPR(DEB_GETSOC, ("bps_get_socket: entered\n"));

	if (!opened) {
		DEBPR(DEB_GETSOC, ("bps_get_socket: called without bpsopen\n"));
		return (-1);
	}
	bps_cb.str_len     = 0;
	bps_cb.string_p    = NULL ;
	bps_cb.valbuf_len  = 0;
	bps_cb.valbuf_p    = NULL ;
	bps_cb.value_len   = strlen(value_ptr) + 1;
	bps_cb.value_p     = value_ptr;
	bps_cb.state_p     = NULL;
	bps_cb.lo_return_p = port_ptr;
	bps_cb.hi_return_p = host_ptr;
	if ((ioctl(Device, BPSGETSOCKET, &bps_cb, 0)) == -1)
		return(-1);
	else 
		return (bps_cb.status);
}

